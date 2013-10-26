/*
 * mediafile.h
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

#ifndef MEDIA_FILE_H
#define MEDIA_FILE_H

//#define debug(fmt, ...) printf("%s:%d: " fmt, __FILE__, __LINE__, __VA_ARGS__);

#include <SDL2/SDL.h>
#include "packetqueue.h"
#include "sys_queue.h"

/**
 * \file mediafile.h
 * This file contains the structure (MediaFile) which contains the
 * information about the input media file.
 *
 */

/**
 * \struct FramePicture
 * \brief  The 'type' of a picture queue element.
 *
 */
typedef struct FramePicture {
      unsigned char *pict_data;
      uint64_t pts;
      TAILQ_ENTRY( FramePicture ) pict_ptrs;
} FramePicture;

/**
 * \enum  boolean
 * \brief boolean variable for quit flags. Used by the quit[] array.
 *
 */
typedef enum boolean {
      no = 0, yes = 1
} boolean;

/**
 * \struct MediaFile
 * \brief  This structure holds necessary information about the input
 *         media file.
 *
 * \var MediaFile::filename
 * Filename of the input media file. Same as argv[1]
 *
 * \var MediaFile::pFormatContext
 * Pointer to the format context of the input file.
 *
 * \var MediaFile::pAudioStream
 * Pointer to the first audio stream in the file.
 *
 * \var MediaFile::pVideoStream
 * Pointer to the first video stream in the file.
 *
 * \var Mediafile::apkts_queued
 * This value is set to 1, after all the packets in the first audio stream
 * are enqueued into the audio packet queue, otherwise 0.
 *
 * \var Mediafile::vpkts_queued
 * This value is set to 1, after all the packets in the first video stream
 * are enqueued into the audio packet queue, otherwise 0.
 *
 * \var Mediafile::aCodecContext
 * Pointer
 *
 * \var Mediafile::vCodecContext
 *
 *
 * \var Mediafile::pSwrContext
 *
 *
 * \var Mediafile::pSwsContext
 *
 *
 * \var Mediafile::audioQueue
 *
 *
 * \var Mediafile::videoQueue
 *
 *
 * \var Mediafile::audio_buf
 *
 *
 * \var Mediafile::timebase
 * Timebase in milliseconds.
 *
 * \var Mediafile::quit[3]
 * Quit flags.
 * quit[0] Main quit flag
 * quit[1] Video quit flag
 * quit[2] Audio quit flag
 *
 * \var MediaFile::pictq_size
 * Size of video picture queue.
 *
 *
 */
struct MediaFile {
      char filename[1024];
      AVFormatContext *pFormatContext;
      AVStream *pAudioStream;
      AVStream *pVideoStream;
      int aud_stream_index;
      int vid_stream_index;

      int render_pic_width;
      int render_pic_height;

      int apkts_queued;
      int vpkts_queued;

      AVCodecContext *aCodecContext;
      struct SwrContext *pSwrContext;
      PacketQueue audioQueue;
      uint8_t **audio_buf;

      AVCodecContext *vCodecContext;
      struct SwsContext *pSwsContext;
      PacketQueue videoQueue;
      uint8_t *video_data[4];
      int video_linesize[4];
      int pict_size;

      SDL_Thread *video_tid;

      SDL_Thread *demux_tid;
      SDL_mutex *demuxth_mutex;
      SDL_cond *demuxth_cond;

      SDL_TimerID refresh_tid;
      SDL_mutex *refresht_mutex;
      SDL_cond *refresht_cond;

      TAILQ_HEAD( PictureHead, FramePicture ) pictq_head;
      int pictq_size;
      SDL_mutex *pictq_mutex;
      SDL_cond *pictq_cond;

      SDL_Window *movieWindow;
      SDL_Renderer *movieRenderer;
      SDL_Texture *movieTexture;

      double timebase;
      boolean quit[3];
};

#endif /* MEDIA_FILE_H INCLUDED */
