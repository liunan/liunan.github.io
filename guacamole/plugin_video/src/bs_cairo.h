#ifndef __BS_CAIRO_H__
#define __BS_CAIRO_H__

#include "bs_types.h"

#include <cairo.h>
#include <SDL.h>

typedef struct bs_cairo_sdl_st
{
  bs_int32_t width, height; 
  SDL_Surface *pScreen; 
  cairo_t *pCairo;
} bs_cairo_sdl_t; 

bs_cairo_sdl_t*
bs_cairo_sdl_create(bs_int32_t width, 
                    bs_int32_t height);

void 
bs_cairo_sdl_destroy(bs_cairo_sdl_t *pCairoSdl); 

#define bs_cairo_sdl_get_cairo(CAIROSDL) \
  (CAIROSDL)->pCairo

void 
bs_cairo_sdl_lock_surface(bs_cairo_sdl_t *pCairoSdl); 

void 
bs_cairo_sdl_unlock_surface(bs_cairo_sdl_t *pCairoSdl); 

#endif
