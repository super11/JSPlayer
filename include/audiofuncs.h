/*
 * audiofuncs.h
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

#ifndef AUDIO_FUNCS_H
#define AUDIO_FUNCS_H

/**
 * \file audiofuncs.h
 * Audio packets decoding, resampling the decoded frames, creating channel
 * data from the resampled audio frames. Copies the channel data to a user
 * created buffer.
 *
 * Audio frames (one by one) are decoded from dequeued audio packets. Each
 * decoded frame is resampled and channel data is copied to a user buffer.
 * On every call to the callback function, the channel data is copied from
 * this user buffer to the SDL buffer provided to the callback function by
 * the SDL Audio thread.
 *
 */

#include <libavcodec/avcodec.h>

#define QUIT_AUDIO_EVENT (SDL_USEREVENT + 2)

/**
 *
 * Function: audio_callback
 * -------------------------
 * Copies the channel data of size 'len' from the user buffer to the SDL
 * buffer. If user buffer is empty, then it calls other functions to fill
 * the user buffer with channel data of an audio frame.
 *
 * Called by the SDL audio thread whenever the thread needs channel data.
 *
 *
 * \param       userdata        Data passed to the callback function by the
 *                              SDL audio thread. Our thread always passes
 *                              NULL.
 *
 * \param       stream          The SDL buffer to which the channel data is
 *                              to be copied. The format of the channel data
 *                              should match the audio format received in
 *                              the struct SDL_AudioSpec.
 *
 * \param       len             Length of the SDL buffer.
 *
 */

void audio_callback( void *userdata, uint8_t *stream, int len );

/**
*
* Function: audio_decode_frame
* -----------------------------
* Gets a packet from the audio packet queue. Decodes the first frame. Calls
* a function which resamples and then copies the channel data of the audio
* frame to a user buffer.
*
* \return       int     Returns -1 if the audio packets are over.
*                 On normal termination, returns the size occupied by
*                       the channel data.
*
*/

int audio_decode_frame( void );

/**
 *
 * Function: create_channel_data
 * ------------------------------
 * Resamples the decoded frame as per the resampling context in the struct
 * MediaFile, and copies the channel data of the resampled audio frame into
 * the buffer 'audio_buf' of struct MediaFile via global pointer gMedia.
 *
 * \param       pFrame          The decoded audio frame, which is to be
 *                              resampled. The pixel data of the resampled
 *                              frame is to be copied into a buffer which
 *                              is also accessible to the callback function.
 * \return      int             Returns the size of the channel data.
 *
 */

int create_channel_data( AVFrame *pFrame );

#endif /* AUDIO_FUNCS_H INCLUDED */
