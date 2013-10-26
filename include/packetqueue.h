/*
 * packetqueue.h
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

#ifndef PACKET_QUEUE_H
#define PACKET_QUEUE_H

#include <libavformat/avformat.h>
#include <SDL2/SDL.h>

/**
 * \struct PacketQueue
 * \brief
 * A packet queue is headed by a structure defined by the struct PacketQueue.
 * This structure contains a pair of pointers, one to the first element in
 * the queue and the other to the last element in the queue.  The elements
 * are of type struct AVPacketList. New elements can be added to the queue
 * only at the head of the queue.
 *
 * \var PacketQueue::nb_packets
 * Number of packets currently present in the queue.
 *
 * \var PacketQueue::size
 * The size in bytes occupied by all the packets in the queue.
 *
 * \var PacketQueue::last_few_pkts
 * Last few packets of a stream.
 * The value of last_few_pkts is either 0 or 1.
 * Since we are queuing the packets of a unique stream into a unique queue,
 * last_few_pkts is 1, when the queue has the last few packets of that
 * stream i.e after av_read_frame() is done enqueing all the packets of a
 * stream onto it's corresponding queue, the last_few_pkts of that queue
 * header structure is set to 1.
 *
 */
typedef struct PacketQueue {
      AVPacketList *first_pkt , *last_pkt ;
      int nb_packets ;
      int size;
      int last_few_pkts;
      SDL_mutex *mutex;
      SDL_cond *cond;

} PacketQueue;

#endif /* PACKET_QUEUE_H INCLUDED */
