/*
 * renderfuncs.c
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
 * Rendering related functions
 */

#include <SDL2/SDL.h>
#include "dimensions.h"
#include "mediafile.h"
#include "globalvar.h"
#include "renderfuncs.h"

int create_win_ren_tex()
{
        SDL_Window *movieWindow = SDL_CreateWindow( gMedia->filename,
                                                    SDL_WINDOWPOS_UNDEFINED,
                                                    SDL_WINDOWPOS_UNDEFINED,
                                                    SCREEN_WIDTH,
                                                    SCREEN_HEIGHT,
                                                    SDL_WINDOW_SHOWN );

        if ( movieWindow == NULL ) {
                printf( "Window could not be created! SDL_Error: %s\n",
                        SDL_GetError() );
                exit( EXIT_FAILURE );
        }

        gMedia->movieWindow = movieWindow;
        SDL_Renderer *movieRenderer = SDL_CreateRenderer
                                      ( movieWindow,
                                        -1,
                                        SDL_RENDERER_ACCELERATED );

        if ( movieRenderer == NULL ) {
                printf( "Renderer could not be created! SDL_Error: %s\n",
                        SDL_GetError() );
                exit( EXIT_FAILURE );
        }

        gMedia->movieRenderer = movieRenderer;
        SDL_RendererInfo movieRendererInfo;
        SDL_GetRendererInfo( movieRenderer, &movieRendererInfo );
        printf( "Name of the Renderer is %s\n", movieRendererInfo.name );
        SDL_Texture *movieTexture = SDL_CreateTexture
                                    ( movieRenderer,
                                      SDL_PIXELFORMAT_YV12,
                                      SDL_TEXTUREACCESS_STREAMING,
                                      SCREEN_WIDTH,
                                      SCREEN_HEIGHT );

        if ( movieTexture == NULL ) {
                printf( "Texture could not be created! SDL_Error: %s\n",
                        SDL_GetError() );
                exit( EXIT_FAILURE );
        }

        gMedia->movieTexture = movieTexture;
        return 1;
}

int display_frame_on_screen( unsigned char *frame_data )
{
        //printf( "In Display frame on screen : Thread\n" );
        SDL_Renderer *movieRenderer = gMedia->movieRenderer;
        SDL_Texture *movieTexture = gMedia->movieTexture;
        const SDL_Rect textureArea = {0, 0,
                                      gMedia->render_pic_width,
                                      gMedia->render_pic_height
                                     };
        SDL_UpdateTexture( movieTexture,
                           &textureArea,
                           frame_data,
                           gMedia->video_linesize[0] );
        SDL_RenderCopy( movieRenderer, movieTexture, NULL, NULL );
        SDL_RenderPresent( movieRenderer );
        //printf( "Picture Displayed\n" );
        return 1;
}

int display_atwb_on_screen()
{
        TwDraw();
        SDL_GL_SwapWindow( gMedia->movieWindow );
}

void destroy_win_ren_tex()
{
        SDL_DestroyRenderer( gMedia->movieRenderer );
        SDL_DestroyWindow( gMedia->movieWindow );
}


