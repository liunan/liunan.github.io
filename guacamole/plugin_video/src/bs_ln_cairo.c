#include "bs_ln_cairo.h"
#include "bs_mem.h"


bs_ln_cairo_t*
bs_ln_cairo_st_create(bs_int32_t width, 
                    bs_int32_t height)
{
  bs_ln_cairo_t *pCairoSdl; 
  cairo_surface_t *pCairoSurface; 

  BS_MEM_ALLOC_TYPE(pCairoSdl, bs_ln_cairo_t, 1); 

  pCairoSdl->width = width; 
  pCairoSdl->height = height;

 /*
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
  {
    fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
    exit(1);     
  }
*/  
  //TODO: quit handler
  //atexit(SDL_Quit); 
  /*
  pCairoSdl->pScreen = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE); 
  if(pCairoSdl->pScreen == NULL)
  {
    fprintf(stderr, "SDL_SetVideoMode failed: %s\n", SDL_GetError()); 
    exit(1); 
  }*/

  //we should create an image surface with self controlled img buffer
  pCairoSurface = cairo_image_surface_create_for_data(NULL/* TODO: surface buffer ptr,pCairoSdl->pScreen->pixels*/, 
    CAIRO_FORMAT_ARGB32, pCairoSdl->width, pCairoSdl->height, pCairoSdl->width * 4); 

  //the surface created above is used for create a new cairo main object,and then destroy the cairo temp surface.
  pCairoSdl->pCairo = cairo_create(pCairoSurface);
  cairo_surface_destroy(pCairoSurface);
  
  return pCairoSdl; 
}                    

void bs_ln_cairo_st_destroy(bs_ln_cairo_t *pCairoSdl)
{
    //TODO: release the object created in the pCairoSdl object
}


