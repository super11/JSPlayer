/*
 * queuefuncs.h
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

#ifndef QUEUE_FUNCS_H
#define QUEUE_FUNCS_H

/**
 * \file queuefuncs.h
 * Packet queue related functions to initialize a packet queue, enqueue and
 * dequeue.
 *
 */

// Value of packet queue size in ffplay
#define MAX_QUEUE_SIZE (15 * 1024 * 1024)

#include "packetqueue.h"

/**
 *
 * Function: packet_queue_init
 * ----------------------------
 * Allocates memory for a struct PacketQueue and creates the required
 * mutex and condition variable and sets them to respective members in
 * the struct PacketQueue.
 *
 * \param       q       A queue header pointer variable which is pointed to
 *                      the actual queue header after the header is allocated
 *                      on the heap.
 *
 */

void packet_queue_init( PacketQueue *q );

/**
 *
 * Function:  packet_queue_put
 * ----------------------------
 * Puts (Enqueues) a given audio/video packet into the queue.
 *
 * \param       q       The queue (pointer to the queue header) into which
 *                      the audio/video packet is to be 'put' or 'enqueued'.
 *
 * \param       pkt     The audio/video packet (pointer) that is to be 'put'.
 *                      This packet of type AVPacket is enqueued into the
 *                      queue.
 *
 * \return      int     Returns -1 if out of memory.
 *                      Returns 0 on normal termination.
 *
 */

int packet_queue_put( PacketQueue *q, AVPacket *pkt );

/**
 *
 * Function: packet_queue_get
 * ---------------------------
 * Gets the audio/video packet from the queue.
 *
 * \param       q       The queue (pointer to the queue header) from which
 *                      the audio/video packet is to be 'get' or 'dequeued'.
 *
 * \param       pkt     The audio/video packet (pointer) that is dequeued is
 *                      copied into this packet. The dequeued packet is of
 *                      type AVPacketList. The actual packet information is
 *                      copied into this AVPacket.
 *
 * \return      int     Returns 0 if packets are over.
 *                      Returns 1 on normal termination, i.e when a packet
 *                      is copied to pkt.
 *
 */

int packet_queue_get( PacketQueue *q, AVPacket *pkt );

#endif /* QUEUE_FUNCS_H INCLUDED */




