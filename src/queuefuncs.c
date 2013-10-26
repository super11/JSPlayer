/*
 * queuefuncs.c
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
 * \file queuefuncs.c
 * Packet queue related functions to initialize a packet queue, enqueue and
 * dequeue.
 *
 */

#include "queuefuncs.h"
#include "mediafile.h"
#include "globalvar.h"

void packet_queue_init( PacketQueue *q )
{
        memset( q, 0, sizeof( PacketQueue ) );
        q->mutex = SDL_CreateMutex();
        q->cond = SDL_CreateCond();
}

int packet_queue_put( PacketQueue *q, AVPacket *pkt )
{
        // Packet queue element is of type AVPacketList
        AVPacketList  *pkt1;
        pkt1 = av_malloc( sizeof( AVPacketList ) );

        pkt1->pkt = *pkt;
        pkt1->next = NULL;

        SDL_LockMutex( q->mutex );

        // Queue is empty
        if ( !q->last_pkt ) {
                q->first_pkt = pkt1;

        } else  { // Queue is non-empty
                q->last_pkt->next = pkt1;
        }

        q->last_pkt = pkt1;
        q->nb_packets++;
        q->size += pkt1->pkt.size;

        SDL_CondSignal( q->cond );
        SDL_UnlockMutex( q->mutex );
        return 0;
}

int packet_queue_get( PacketQueue *q, AVPacket *pkt )
{
        // Packet queue element is of type AVPacketList
        AVPacketList *pkt1;
        SDL_LockMutex( q->mutex );

        pkt1 = q->first_pkt;

        // Queue is empty
        if ( !pkt1 ) {

                // Stream has packets to be enqueued
                if ( !q->last_few_pkts )  {
                        SDL_CondWait( q->cond, q->mutex );
                        printf( "Condition Signalled\n" );

                } else   {  // Stream has no more packets to be enqueued
                        SDL_UnlockMutex( q->mutex );
                        return 0;
                }

                // Queue has a packet now!
                pkt1 = q->first_pkt;
        }

        // Queue is non-empty

        q->first_pkt = pkt1->next;

        // Queue has a single packet
        if ( !q->first_pkt ) {
                q->last_pkt = NULL;
        }

        q->nb_packets--;
        q->size -= pkt1->pkt.size;
        *pkt = pkt1->pkt;
        av_free( pkt1 );

        SDL_CondSignal( gMedia->demuxth_cond );
        SDL_UnlockMutex( q->mutex );
        return 1;
}


