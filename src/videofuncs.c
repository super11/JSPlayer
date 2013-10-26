/*
 * videofuncs.c
 *
 * Copyright 2013 - Prashanth Josyula
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

/**
 * @file
 * Video packets decoding, creating pixel data from decoded frame.
 */

#include <libswscale/swscale.h>
#include <SDL2/SDL.h>
#include "queuefuncs.h"
#include "dimensions.h"
#include "mediafile.h"
#include "globalvar.h"
#include "videofuncs.h"


void create_pixel_data( AVFrame *pFrame, unsigned char *frame_data )
{
        //printf( "In Create Pixel Data : Thread\n" );
        sws_scale( gMedia->pSwsContext,
                   ( uint8_t const * const * )pFrame->data,
                   pFrame->linesize,
                   0,
                   pFrame->height,
                   gMedia->video_data,
                   gMedia->video_linesize );

        int num_bytes_yPlane = ( gMedia->video_linesize[0]
                                 * gMedia->render_pic_height );
        int num_bytes_vPlane = ( gMedia->video_linesize[1]
                                 * gMedia->render_pic_height ) / 2;
        int num_bytes_uPlane = ( gMedia->video_linesize[2]
                                 * gMedia->render_pic_height ) / 2;

        unsigned char *frame_data_iter = ( unsigned char * )frame_data ;

        //Copy the pixel information stored in the planes into a buffer
        memcpy( frame_data_iter,
                gMedia->video_data[0],
                num_bytes_yPlane );
        frame_data_iter = frame_data_iter + num_bytes_yPlane;

        memcpy( frame_data_iter,
                gMedia->video_data[2],
                num_bytes_uPlane );
        frame_data_iter = frame_data_iter + num_bytes_uPlane;

        memcpy( frame_data_iter,
                gMedia->video_data[1],
                num_bytes_vPlane );
}

int video_decode_frame( void *frame_data )
{
        //printf("In Video Decode Frame : Thread 2\n");
        gMedia->pictq_size = 0;
        TAILQ_INIT( &gMedia->pictq_head );
        gMedia->pictq_mutex = SDL_CreateMutex();
        gMedia->pictq_cond = SDL_CreateCond();

        SDL_Event quit_video_event;
        quit_video_event.type = QUIT_VIDEO_EVENT;

        PacketQueue *pVQueue = &( gMedia->videoQueue ) ;
        AVPacket packet;
        AVFrame *pFrame = avcodec_alloc_frame();

        int frame_fin = 0;

        AVRational timebase; // in milliseconds

        timebase = gMedia->pVideoStream->time_base;
        timebase.num *= 1000;

        gMedia->timebase =  av_q2d( timebase );

        printf( "Video timebase in milliseconds is %f\n", gMedia->timebase );

        while ( gMedia->quit[1] == no ) {
                printf( "-------------------------------------------\n" );

                if ( !packet_queue_get( pVQueue, &packet ) ) {
                        SDL_PushEvent( &quit_video_event );
                        printf( "Video quit event pushed\n" );
                        avcodec_free_frame( &pFrame );
                        return 0;
                }

                avcodec_decode_video2( gMedia->vCodecContext,
                                       pFrame,
                                       &frame_fin,
                                       &packet );

                printf( "\t %c frame\n",
                        av_get_picture_type_char( pFrame->pict_type ) );

                if ( frame_fin ) {

                        static int first_fin_frame = 1;

                        printf( "PTS of frame->pkt_pts is %" PRIu64 "\n", pFrame->pkt_pts );
                        printf( "PTS of frame->pkt_dts is %" PRIu64 "\n", pFrame->pkt_dts );

                        if ( first_fin_frame ) {
                                pFrame->pkt_pts = 0;
                                first_fin_frame--;
                        }

                        if ( pFrame->pkt_pts == AV_NOPTS_VALUE ) {
                                // This is for I and P frames that do not
                                // have pkt_pts value.
                                pFrame->pkt_pts = pFrame->pkt_dts;
                        }

                        printf( "PTS of frame->pkt_pts is %" PRIu64 "\n", pFrame->pkt_pts );

                        if ( pFrame->pts != pFrame->pkt_pts ) {
                                pFrame->pts = pFrame->pkt_pts;
                        }

                        printf( "PTS of frame is %" PRIu64 "\n", pFrame->pts );
                        create_pixel_data( pFrame, frame_data );
                        enqueue_picture( frame_data, pFrame->pts );

                } else {
                        printf( "Video frame is not finished\n" );
                }
        }

        printf( "Video thread returned\n" );
}

int enqueue_picture( unsigned char *frame_data, uint64_t pts )
{
        FramePicture *pict;
        pict = malloc( sizeof( *pict ) );
        pict->pict_data = malloc( gMedia->pict_size );

        memcpy( pict->pict_data, frame_data, gMedia->pict_size );
        pict->pts = pts;

        SDL_LockMutex( gMedia->pictq_mutex );

        if ( gMedia->pictq_size >= PICTQ_MAX_SIZE )    {

                static int first_time = 1;

                // Add a Timer for the first time when queue becomes full
                if ( first_time ) {
                        SDL_Event add_timer_event;
                        add_timer_event.type = ADD_TIMER_EVENT;
                        SDL_PushEvent( &add_timer_event );
                        printf( "SDL ADD_TIMER_EVENT pushed\n" );
                        first_time = 0;
                }

                SDL_CondWait( gMedia->pictq_cond, gMedia->pictq_mutex );
        }

        TAILQ_INSERT_TAIL( &( gMedia->pictq_head ), pict, pict_ptrs );
        //printf("Picture inserted in the queue\n");
        gMedia->pictq_size++;
        SDL_CondSignal( gMedia->pictq_cond );

        SDL_UnlockMutex( gMedia->pictq_mutex );
        return 0;
}



