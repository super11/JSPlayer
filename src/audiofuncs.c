/*
 * audiofuncs.c
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
 * Audio related functions
 */

#include <libswresample/swresample.h>
#include <SDL2/SDL.h>
#include "mediafile.h"
#include "globalvar.h"
#include "queuefuncs.h"
#include "audiofuncs.h"

void audio_callback( void *userdata, uint8_t *stream, int len )
{
        //printf( "In Audio callback : Thread\n" );
        //printf("Length is %d\n",len);
        static int stream_buf_index = 0;
        static int audio_data_size = 0;
        static int audio_buf_index = 0;
        int free_flag = 0;

        while ( len > 0 && gMedia->quit[2] == no ) {
                if ( audio_data_size == 0 ) {
                        audio_buf_index = 0;
                        audio_data_size = audio_decode_frame();
                        //printf("%d\n",audio_data_size);
                        free_flag = 1;
                }

                if ( audio_data_size == -1 ) {
                        return;
                }

                if ( len <= audio_data_size ) {
                        memcpy( stream + stream_buf_index,
                                *( gMedia->audio_buf ) + audio_buf_index,
                                len );
                        //printf("%d bytes copied : %d bytes left in buffer\t",len,audio_data_size);
                        audio_data_size -= len;
                        audio_buf_index += len;
                        //     printf("%d\n",audio_buf_index);
                        len = 0;
                        stream_buf_index = 0;
                        break;
                }

                if ( len > audio_data_size ) {
                        memcpy( stream + stream_buf_index,
                                *( gMedia->audio_buf ) + audio_buf_index,
                                audio_data_size );
                        len -= audio_data_size;
                        stream_buf_index += audio_data_size;
                        audio_data_size = 0;
                        audio_buf_index = 0;
                        continue;
                }
        }

        if ( free_flag && !audio_data_size ) {
                av_free( gMedia->audio_buf[0] );
        }
}

int audio_decode_frame( void )
{
        //printf( "In Audio decode frame : Thread \n" );
        SDL_Event quit_audio_event;

        quit_audio_event.type = QUIT_AUDIO_EVENT;

        PacketQueue *pAQueue = &( gMedia->audioQueue ) ;
        static AVPacket packet;
        AVFrame *pFrame = avcodec_alloc_frame() ;
        int pkt_bytes_decd = 0;
        int audio_data_size = 0;
        int frame_fin = 0;

        if ( packet.size == 0 ) {
                if ( !packet_queue_get( pAQueue, &packet ) ) {
                        SDL_PushEvent( &quit_audio_event );
                        av_free( pFrame );
                        return -1;
                }
        }

        while ( packet.size > 0 ) {
                // printf("Size of packet is %d\n",packet.size);
                pkt_bytes_decd = avcodec_decode_audio4( gMedia->aCodecContext,
                                                        pFrame,
                                                        &frame_fin,
                                                        &packet );

                printf( "%d bytes from packet decoded\n", pkt_bytes_decd );
                // printf("Format of Decoded frame is %d\n",pFrame->format);
                // printf("Format of audio is %d\n",pFrame->nb_samples);

                //               aud_frame_pts = pFrame->pkt_pts ;
                //printf( " audio frame : pts is %" PRId64 "\n", aud_frame_pts );

                if ( pkt_bytes_decd < 0 ) {
                        /* if error, skip packet */
                        break;
                }

                if ( frame_fin ) {
                        audio_data_size = create_channel_data( pFrame );
                        packet.size -= pkt_bytes_decd;
                        av_free( pFrame );
                        return audio_data_size ;
                }
        }

        /*        if ( pkt->pts != AV_NOPTS_VALUE ) {
                        gMedia->audio_clock = av_q2d( gMedia->pFormatContext->
                                                      streams[aud_stream_index] )
                                              * pkt->pts;
                }
        */
        return 1; //Never comes here
}

int create_channel_data( AVFrame *pFrame )
{
        //printf( "In Create Channel data : Thread 1\n" );
        int dst_nb_channels = av_get_channel_layout_nb_channels
                              ( AV_CH_LAYOUT_STEREO );
        int dst_linesize;
        int delay = swr_get_delay( gMedia->pSwrContext,
                                   pFrame->sample_rate );
        int dst_nb_samples = av_rescale_rnd( pFrame->nb_samples + delay,
                                             pFrame->sample_rate,
                                             pFrame->sample_rate,
                                             AV_ROUND_UP );
        //printf("Destination channels = %d\n",dst_nb_channels);
        //printf("Destination samples = %d\n",dst_nb_samples);
        int error_check = av_samples_alloc_array_and_samples
                          ( &gMedia->audio_buf,
                            &dst_linesize,
                            dst_nb_channels,
                            dst_nb_samples,
                            AV_SAMPLE_FMT_FLT,
                            1 );

        if ( error_check < 0 ) {
                fprintf( stderr, "Could not allocate destination samples\n" );
        }

        int data_size = av_samples_get_buffer_size
                        ( NULL,
                          pFrame->channels,
                          pFrame->nb_samples,
                          pFrame->format,
                          0 );
        /*      printf("Number of samples = %d\n",pFrame->nb_samples);
                printf("Number of bytes = %d\n",pFrame->nb_samples*2*4);
                printf("Linesize per channel is %d\n",pFrame->linesize[0]);
                printf("Calculated datasize is %d\n",data_size);
        */
        swr_convert( gMedia->pSwrContext,
                     ( uint8_t ** )( gMedia->audio_buf ) ,
                     pFrame->nb_samples,
                     ( const uint8_t ** )pFrame->data,
                     pFrame->nb_samples );
        return data_size;
}
