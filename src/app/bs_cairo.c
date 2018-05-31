#include <stdio.h>

#include "bs_cairo.h"
#include "bs_mem.h"
#include "bs_assert.h"

#define BS_CAIRO_START_X 100
#define BS_CAIRO_START_Y 100

bs_cairo_sdl_t*
bs_cairo_sdl_create(bs_int32_t width, 
                    bs_int32_t height)
{
  bs_cairo_sdl_t *pCairoSdl; 
  cairo_surface_t *pCairoSurface; 

  BS_MEM_ALLOC_TYPE(pCairoSdl, bs_cairo_sdl_t, 1); 

  pCairoSdl->width = width; 
  pCairoSdl->height = height;

  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
  {
    fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
    exit(1);     
  }
  
  atexit(SDL_Quit); 
  pCairoSdl->pScreen = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE); 
  if(pCairoSdl->pScreen == NULL)
  {
    fprintf(stderr, "SDL_SetVideoMode failed: %s\n", SDL_GetError()); 
    exit(1); 
  }

  pCairoSurface = cairo_image_surface_create_for_data(pCairoSdl->pScreen->pixels, 
    CAIRO_FORMAT_ARGB32, pCairoSdl->width, pCairoSdl->height, pCairoSdl->width * 4); 
  pCairoSdl->pCairo = cairo_create(pCairoSurface);
  cairo_surface_destroy(pCairoSurface);
  
  return pCairoSdl; 
}

void 
bs_cairo_sdl_destroy(bs_cairo_sdl_t *pCairoSdl)
{
  
}

void 
bs_cairo_sdl_lock_surface(bs_cairo_sdl_t *pCairoSdl)
{
  if (SDL_MUSTLOCK(pCairoSdl->pScreen))
  {
    SDL_LockSurface(pCairoSdl->pScreen); 
  }
}

void 
bs_cairo_sdl_unlock_surface(bs_cairo_sdl_t *pCairoSdl)
{
  if (SDL_MUSTLOCK(pCairoSdl->pScreen))
  {
    SDL_UnlockSurface(pCairoSdl->pScreen);
  }
}
