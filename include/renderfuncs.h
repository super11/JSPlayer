/*
 * renderfuncs.h
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

#ifndef RENDER_FUNCS_H
#define RENDER_FUNCS_H

/**
 * \file renderfuncs.h
 * Video Rendering (SDL) related functions
 */

/**
 *
 * Function: create_win_ren_tex
 * -----------------------------
 * Creates the following :
 *
 * SDL_Window   - "An application window".
 * SDL_Renderer - "A 2D rendering context for a window".
 * SDL_Texture  - "A structure that contains an efficient, driver-specific
 *                 representation of pixel data".
 *
 * The pointers to the created window, renderer and texture are stored in
 * the struct MediaFile and can be accessed through the global pointer.
 *
 * \return         Returns a value of 1, on normal termination.
 *
 */

int create_win_ren_tex();

/**
 *
 * Function: display_frame_on_screen
 * ----------------------------------
 * Renders the video frame onto the window. The information (pixel data)
 * of the video frame is present in the buffer frame_data.
 *
 * \param          The pixel data (pointer) of the frame to be displayed.
 *
 * \return         Returns a value of 1, on normal termination.
 *
 */

int display_frame_on_screen( unsigned char *frame_data );

/**
 *
 * Function: display_atwb_on_screen
 * ---------------------------------
 *
 * \return
 *
 */

int display_atwb_on_screen();

/**
 *
 * Function: destroy_win_ren_tex
 * ------------------------------
 * "Destroys the rendering context for the window and frees associated
 * textures".
 * "Destroys the window".
 *
 */

void destroy_win_ren_tex();

#endif /* RENDER_FUNCS_H INCLUDED */




