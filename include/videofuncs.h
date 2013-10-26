/*
 * videofuncs.h
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

#ifndef VIDEO_FUNCS_H
#define VIDEO_FUNCS_H

/**
 * \file videofuncs.h
 * Video packets decoding, scaling the decoded frame, creating pixel data
 * from the scaled frame, and queuing up the pixel data of the scaled frames.
 *
 * The frame is decoded from the dequeued video packet.
 * The decoded frame is scaled as per the scaling context.
 * The pixel information from the frame is copied to the buffer.
 * The buffer information is made into a queue element.
 * The element is enqueued into the picture queue.
 *
 */

#include <libavutil/frame.h>

#define QUIT_VIDEO_EVENT    (SDL_USEREVENT + 1)
#define ADD_TIMER_EVENT     (SDL_USEREVENT + 3)
#define RENDER_PICT_EVENT   (SDL_USEREVENT + 4)

#define PICTQ_MAX_SIZE       3

/**
 *
 * Function : video_decode_frame
 * -----------------------------
 * Initializes the picture queue, creates the required mutex and condition
 * variable. Gets a packet from the video packet queue, decodes the frame,
 * and calls other functions to scale the frame, create the pixel data from
 * the scaled frame and to enqueue the created data into the picture queue.
 *
 * Runs as a seperate thread. Returns only when video packets are over.
 * The thread id is stored in the struct MediaFile.
 * The thread is killed when the event SDL_QUIT is recorded.
 *
 * \param       frame_data      The buffer to fill with the pixel data of
 *                              the scaled frame. This buffer is later
 *                              copied into an element of the picture queue.
 *                              Then the buffer is reused for the next frame.
 * \return      int             Returns 0 on normal termination.
 *
 */

int video_decode_frame( void *frame_data );

/**
 *
 * Function: create_pixel_data
 * ----------------------------
 * Scales the decoded frame as per the scaling context in the struct
 * Mediafile, and copies the pixel data of the scaled frame to the buffer
 * that is provided.
 *
 * \param       pFrame          The decoded frame, which is to be scaled.
 *                              The pixel data of the scaled frame is to
 *                              be copied into the buffer.
 * \param       frame_data      The buffer to which the pixel data of the
 *                              scaled frame is to be copied.
 *
 */

void create_pixel_data( AVFrame *pFrame, unsigned char *frame_data );

/**
 *
 * Function: enqueue_picture
 * --------------------------
 * Allocates memory for a struct FramePicture and copies the data in the
 * buffer into the struct and then pushes this struct on the picture queue.
 *
 * \param       frame_data      The buffer which contains the pixel data of
 *                              the scaled frame. This data is copied into
 *                              a picture queue element (struct FramePicture)
 *                              and that element is pushed onto the queue.
 * \return      int             Returns 0 on normal termination.
 *
 */

int enqueue_picture( unsigned char *frame_data, uint64_t pts );

#endif /* VIDEO_FUNCS_H INCLUDED */




