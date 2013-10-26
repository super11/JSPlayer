/**************************************************************************
 *  2013/09/16 09:29:33                                                   *
 *  jsplayer.c                                                            *
 *                                                                        *
 *  Copyright 2013 - Prashanth Josyula                                    *
 *                                                                        *
 *  This program is free software; you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation; either version 2 of the License, or     *
 *  (at your option) any later version.                                   *
 *                                                                        *
 *  This program is distributed in the hope that it will be useful,       *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with this program; if not, write to the Free Software           *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,            *
 *  MA 02110-1301, USA.                                                   *
 *                                                                        *
 **************************************************************************
 *                        "Code HOW Comment WHY"                          *
 *************************************************************************/

// Size of audio buffer in ffplay (Practical size)
#define SDL_AUDIO_BUFFER_SIZE 1024

#include <stdio.h>
#include <stdlib.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/avstring.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>

#include "AntTweakBar.h"
#include "dimensions.h"
#include "queuefuncs.h"
#include "mediafile.h"
#include "videofuncs.h"
#include "renderfuncs.h"
#include "audiofuncs.h"

struct MediaFile *gMedia = NULL;

int demux_input_file( void *pFormatContext )
{
        printf( "Thread : Demux input file : Waiting for the mutex\n" );
        SDL_LockMutex( gMedia->demuxth_mutex );
        printf( "Thread : Demux input file : Mutex locked\n" );

        static AVPacket packet;

        PacketQueue *pAudioQ = &gMedia->audioQueue;
        PacketQueue *pVideoQ = &gMedia->videoQueue;

        pVideoQ->last_few_pkts = 0;
        pAudioQ->last_few_pkts = 0;

        while ( av_read_frame( pFormatContext, &packet ) >= 0 ) {

                if ( packet.stream_index == gMedia->aud_stream_index ) {
                        if ( pAudioQ->nb_packets >= MAX_QUEUE_SIZE ) {
                                SDL_CondWait( gMedia->demuxth_cond,
                                              gMedia->demuxth_mutex );
                        }

                        packet_queue_put( pAudioQ, &packet );
                        //printf("Audio packet put\n");

                } else if ( packet.stream_index == gMedia->vid_stream_index ) {
                        if ( pVideoQ->nb_packets >= MAX_QUEUE_SIZE ) {
                                SDL_CondWait( gMedia->demuxth_cond,
                                              gMedia->demuxth_mutex );
                        }

                        packet_queue_put( pVideoQ, &packet );
                        //printf("Video packet put\n");
                }
        }

        pVideoQ->last_few_pkts = 1;
        pAudioQ->last_few_pkts = 1;

        SDL_UnlockMutex( gMedia->demuxth_mutex );
        printf( "Thread : Demux input file : Mutex Unlocked\n" );
        return 1; //
}


uint32_t video_refresh( uint32_t interval, void *param )
{
        SDL_LockMutex( gMedia->refresht_mutex );

        printf( "Timer is called\n" );
        static SDL_Event render_pict_event;
        render_pict_event.type = RENDER_PICT_EVENT;
        uint32_t next_timer;

        SDL_LockMutex( gMedia->pictq_mutex );

        if ( TAILQ_EMPTY( &gMedia->pictq_head ) ) {
                SDL_CondWait( gMedia->pictq_cond, gMedia->pictq_mutex );
        }

        static FramePicture *pict;
        static FramePicture *next_pict;

        pict = TAILQ_FIRST( &gMedia->pictq_head );
        render_pict_event.user.data1 = ( void * )pict ;
        TAILQ_REMOVE( &gMedia->pictq_head, pict, pict_ptrs );
        gMedia->pictq_size--;

        if ( TAILQ_EMPTY( &gMedia->pictq_head ) ) {
                SDL_CondWaitTimeout( gMedia->pictq_cond,
                                     gMedia->pictq_mutex,
                                     100 );
                //Intended to timeout for the last packet of the stream
        }

        next_pict = TAILQ_FIRST( &gMedia->pictq_head );
        /*      printf(" Present picture pts is %" PRIu64 "\n", pict->pts);
                printf(" Next picture pts is %" PRIu64 "\n", next_pict->pts);
                printf(" Value of timebase is %f\n", gMedia->timebase); */
        next_timer = ( next_pict->pts - pict->pts ) * ( gMedia->timebase );

        SDL_PushEvent( &render_pict_event );

        SDL_CondSignal( gMedia->pictq_cond );
        SDL_UnlockMutex( gMedia->pictq_mutex );
        printf( " Value of next timer is %" PRIu32 "\n", next_timer );

        SDL_UnlockMutex( gMedia->refresht_mutex );
        return next_timer;
}


int init_SDL()
{
        if ( SDL_Init( SDL_INIT_VIDEO |
                       SDL_INIT_AUDIO |
                       SDL_INIT_TIMER ) < 0 ) {
                printf( "SDL could not be intialized! SDL_Error: %s\n",
                        SDL_GetError() );
                return EXIT_FAILURE;
        }

        printf( "SDL Successfully Initialized\n" );
        return 1;
}

int main( int argc, char **argv )
{
        AVFormatContext *pFormatContext = NULL;
        AVCodecContext *aCodecContext = NULL;
        AVCodecContext *vCodecContext = NULL;
        AVFrame *pFrame = avcodec_alloc_frame();
        SDL_AudioSpec wantedSpecs, availSpecs;
        SDL_AudioDeviceID dev_id;

        // ret_val is used to store the value returned by functions.
        int ret_val;

        if ( argc < 2 ) {
                printf( "Please provide an input media file\n" );
                return EXIT_FAILURE;
        }

        gMedia = av_mallocz( sizeof( struct MediaFile ) );
        av_strlcpy( gMedia->filename, argv[1], 1024 );

        // Initialize libavformat and register all the muxers,
        // demuxers and protocols.
        av_register_all();

        // Initialize SDL
        init_SDL();

        ret_val = avformat_open_input( &pFormatContext, argv[1],
                                       NULL, NULL );

        if ( ret_val < 0 ) {
                printf( "Could not: Open the input media file\n" );
                return EXIT_FAILURE;
        }

        gMedia->pFormatContext = pFormatContext;

        ret_val = avformat_find_stream_info( pFormatContext, NULL );

        if ( ret_val < 0 ) {
                printf( "Could not: Read packets of a media file to get"
                        "stream information\n" );
                return EXIT_FAILURE;
        }

        av_dump_format( pFormatContext, 0, argv[1], 0 );

        int aud_stream_index = -1, vid_stream_index = -1;

        for ( int i = 0; i < pFormatContext->nb_streams; i++ ) {
                if ( ( pFormatContext->streams[i]->codec->codec_type
                       == AVMEDIA_TYPE_VIDEO )
                     && ( vid_stream_index < 0 ) ) {
                        vid_stream_index = i;
                }

                if ( ( pFormatContext->streams[i]->codec->codec_type
                       == AVMEDIA_TYPE_AUDIO )
                     && ( aud_stream_index < 0 ) ) {
                        aud_stream_index = i;
                }
        }

        // Filling the struct MediaFile with the pointers to audio and video
        // streams, and indexes of the streams.
        gMedia->pAudioStream = pFormatContext->streams[aud_stream_index];
        gMedia->pVideoStream = pFormatContext->streams[vid_stream_index];
        gMedia->aud_stream_index = aud_stream_index;
        gMedia->vid_stream_index = vid_stream_index;

        // If an audio stream is present.
        if ( aud_stream_index >= 0 ) {
                aCodecContext = pFormatContext
                                ->streams[aud_stream_index]->codec;
                // Find the corresponding decoder for the codec ID.
                aCodecContext->codec = avcodec_find_decoder
                                       ( aCodecContext->codec_id );
                avcodec_open2( aCodecContext, aCodecContext->codec, NULL );
                gMedia->aCodecContext = aCodecContext;

                // Set audio settings from codec information
                wantedSpecs.callback = audio_callback;
                wantedSpecs.channels = aCodecContext->channels;
                wantedSpecs.format = AUDIO_F32SYS;
                wantedSpecs.freq = aCodecContext->sample_rate;
                wantedSpecs.samples = SDL_AUDIO_BUFFER_SIZE;
                wantedSpecs.silence = 0;
                wantedSpecs.userdata = NULL;

                // Opens the audio device and pauses it.
                dev_id = SDL_OpenAudioDevice( NULL, 0, &wantedSpecs,
                                              &availSpecs, 0 );

                if ( dev_id < 0 ) {
                        fprintf( stderr, "SDL_OpenAudioDevice: %s\n",
                                 SDL_GetError() );
                        return EXIT_FAILURE;
                }

                // Setting the resampling context

                int src_ch_layout = av_get_default_channel_layout
                                    ( aCodecContext->channels );

                int dst_ch_layout = src_ch_layout;

                // Allocating a resampling context
                gMedia->pSwrContext = swr_alloc();

                av_opt_set_int( gMedia->pSwrContext, "in_channel_layout",
                                src_ch_layout, 0 );

                av_opt_set_int( gMedia->pSwrContext, "in_sample_rate",
                                aCodecContext->sample_rate, 0 );

                // Format of decoded audio frame
                av_opt_set_sample_fmt( gMedia->pSwrContext, "in_sample_fmt",
                                       aCodecContext->sample_fmt, 0 );

                av_opt_set_int( gMedia->pSwrContext, "out_channel_layout",
                                dst_ch_layout, 0 );

                av_opt_set_int( gMedia->pSwrContext, "out_sample_rate",
                                aCodecContext->sample_rate, 0 );

                // Format expected by SDL
                // (AUDIO_F32SYS same as AV_SAMPLE_FMT_FLT )
                av_opt_set_sample_fmt( gMedia->pSwrContext, "out_sample_fmt",
                                       AV_SAMPLE_FMT_FLT, 0 );

                ret_val = swr_init( gMedia->pSwrContext );

                if ( ret_val < 0 ) {
                        fprintf( stderr, "Failed to initialize the"
                                 "resampling context\n" );
                        return EXIT_FAILURE;
                }

                // Initialize the audio packet queue so that packets can be
                // enqueued and dequeued.
                packet_queue_init( &gMedia->audioQueue );

                // Unpauses the previously opened audio device.
                SDL_PauseAudioDevice( dev_id, 0 );
        }

        if ( vid_stream_index >= 0 ) {
                vCodecContext = pFormatContext
                                ->streams[vid_stream_index]->codec;
                // Find the corresponding decoder for the codec ID.
                vCodecContext->codec = avcodec_find_decoder
                                       ( vCodecContext->codec_id );
                avcodec_open2( vCodecContext, vCodecContext->codec, NULL );
                gMedia->vCodecContext = vCodecContext;

                // To be scaled picture dimensions
                gMedia->render_pic_width = 1366;
                gMedia->render_pic_height = 768;

                // Setting the scaling context

                gMedia->pSwsContext = sws_getContext
                                      ( vCodecContext->width,
                                        vCodecContext->height,
                                        vCodecContext->pix_fmt,
                                        gMedia->render_pic_width,
                                        gMedia->render_pic_height,
                                        AV_PIX_FMT_YUV420P,
                                        SWS_BILINEAR,
                                        NULL,
                                        NULL,
                                        NULL );

                // Calculates the size occupied by a frame of format
                // AV_PIX_FMT_YUV420P with dimensions SCREEN_WIDTH by
                // SCREEN_HEIGHT and allocates it.
                ret_val = av_image_alloc( gMedia->video_data,
                                          gMedia->video_linesize,
                                          gMedia->render_pic_width,
                                          gMedia->render_pic_height,
                                          AV_PIX_FMT_YUV420P,
                                          1 );

                if ( ret_val < 0 ) {
                        fprintf( stderr,
                                 "Could not allocate destination image\n" );
                        return EXIT_FAILURE;
                }

                // Initialize the audio packet queue so that packets can be
                // enqueued and dequeued.
                packet_queue_init( &gMedia->videoQueue );
        }

        // Initialize AntTweakBar
        TwInit( TW_OPENGL, NULL );
        // Inform ATWB the size of application graphics window
        TwWindowSize( gMedia->render_pic_width, gMedia->render_pic_height );
        // Create a Tweak Bar
        TwBar *myBar = TwNewBar( "JSPlayer" );

        TwAddVarRW( myBar, "Width", TW_TYPE_INT32,
                    &gMedia->render_pic_width,
                    " label='Picture width'\
                    min=gMedia->render_pic_width\
                    max=SCREEN_WIDTH keyIncr=d keyDecr=a\
                    help='Width of the rendered picture (in pixels)' " );

        TwAddVarRW( myBar, "Height", TW_TYPE_INT32,
                    &gMedia->render_pic_height,
                    " label='Picture height'\
                    min=gMedia->render_pic_height\
                    max=SCREEN_HEIGHT keyIncr=w keyDecr=s\
                    help='Height of the rendered picture (in pixels)' " );

        // Calculate the size of a yuv420 picture of scaled dimensions
        int num_bytes_yPlane = ( gMedia->video_linesize[0]
                                 * SCREEN_HEIGHT );
        int num_bytes_vPlane = ( gMedia->video_linesize[1]
                                 * SCREEN_HEIGHT ) / 2;
        int num_bytes_uPlane = ( gMedia->video_linesize[2]
                                 * SCREEN_HEIGHT ) / 2;
        int num_bytes_total = num_bytes_yPlane +
                              num_bytes_vPlane +
                              num_bytes_uPlane ;

        // Allocate a buffer for yuv420 picture
        // (to be filled with scaled frame data)
        unsigned char *const frame_data = ( unsigned char * )
                                          malloc( num_bytes_total );

        // Making the value of num_bytes_total accessible to other
        // functions.
        gMedia->pict_size = num_bytes_total;

        // Video Decoding thread starts running.
        gMedia->video_tid =  SDL_CreateThread( video_decode_frame,
                                               "decode video frame",
                                               ( void * )frame_data );

        // Demuxing thread starts running.
        // The packets from the streams are enqueued in their respective
        // queues.
        gMedia->demux_tid =  SDL_CreateThread( demux_input_file,
                                               "demux input file",
                                               ( void * )pFormatContext );
        gMedia->demuxth_mutex = SDL_CreateMutex();
        gMedia->demuxth_cond = SDL_CreateCond();

        // Creates window, renderer and texture for displaying the frame.
        create_win_ren_tex();

        // Quit flags array.
        gMedia->quit[0] = gMedia->quit[1] = gMedia->quit[2] =  no;

        SDL_Event event;
        enum parity { even = 0, odd = 1 } times_pressed = even;

        //int handled;

        while ( gMedia->quit[0] == no ) {

                SDL_WaitEvent( &event );

                // handled = TwEventSDL( &event,
                //                       SDL_MAJOR_VERSION,
                //                       SDL_MINOR_VERSION );

                switch ( event.type ) {
                case SDL_QUIT:
                        SDL_SetThreadPriority( SDL_THREAD_PRIORITY_HIGH );

                        if ( gMedia->quit[1] != yes ) {
                                SDL_RemoveTimer( gMedia->refresh_tid );
                                SDL_KillThread( gMedia->video_tid );
                        }

                        //SDL_KillThread( gMedia->demux_tid );

                        if ( !gMedia->quit[2] != yes ) {
                                SDL_LockAudioDevice( dev_id );
                                SDL_CloseAudioDevice( dev_id );
                        }

                        printf( "SDL Quit event recorded\n" );
                        gMedia->quit[2]
                                = gMedia->quit[1]
                                  = gMedia->quit[0] = yes;
                        break;

                case QUIT_VIDEO_EVENT:
                        SDL_RemoveTimer( gMedia->refresh_tid );
                        SDL_KillThread( gMedia->video_tid );
                        printf( "Video Quit event recorded\n" );
                        gMedia->quit[1] = yes;
                        break;

                case QUIT_AUDIO_EVENT:
                        printf( "Audio Quit event recorded\n" );
                        SDL_LockAudioDevice( dev_id );
                        SDL_CloseAudioDevice( dev_id );
                        gMedia->quit[2] = yes;
                        break;

                case SDL_KEYDOWN:
                        times_pressed++;
                        times_pressed %= 2;

                        if ( event.key.keysym.sym == SDLK_SPACE )  {
                                if ( times_pressed == odd ) {
                                        SDL_PauseAudioDevice( dev_id, 1 );
                                        SDL_LockMutex( gMedia->demuxth_mutex );
                                        SDL_LockMutex( gMedia->refresht_mutex );
                                        printf( "Space Bar pressed:"
                                                "Audio Paused\n" );

                                } else if ( times_pressed == even ) {
                                        SDL_PauseAudioDevice( dev_id, 0 );
                                        SDL_UnlockMutex( gMedia->demuxth_mutex );
                                        SDL_UnlockMutex( gMedia->refresht_mutex );
                                        printf( "Space Bar pressed:"
                                                "Audio Unpaused\n" );
                                }
                        }

                        break;

                case ADD_TIMER_EVENT:
                        //Thread (4) starts running
                        gMedia->refresh_tid = SDL_AddTimer
                                              ( 40, video_refresh, NULL );
                        gMedia->refresht_mutex = SDL_CreateMutex();
                        gMedia->refresht_cond = SDL_CreateCond();
                        break;

                case RENDER_PICT_EVENT: {
                        FramePicture *pict = event.user.data1;
                        display_frame_on_screen( pict->pict_data );
                        //display_atwb_on_screen();
                        free( pict->pict_data );
                        free( pict );
                        break;
                }
                }


                if ( gMedia->quit[1] && gMedia->quit[2] ) {
                        printf( "SDL quit set to 0\n" );
                        gMedia->quit[0] = yes;
                        break;
                }

        }

        // Cleaning up before quitting.
        printf( "Quitting ...\n" );
        SDL_CloseAudioDevice( dev_id );
        avcodec_close( vCodecContext );
        avcodec_close( aCodecContext );
        av_free( pFrame );
        avformat_close_input( &pFormatContext );
        av_freep( &gMedia->video_data[0] );
        av_free( gMedia );
        destroy_win_ren_tex();
        SDL_Quit();

        return EXIT_SUCCESS;


}
