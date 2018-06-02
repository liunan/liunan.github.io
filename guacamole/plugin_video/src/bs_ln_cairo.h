#ifndef BS_LN_CAIRO__H
#define BS_LN_CAIRO__H


#include "bs_types.h"
#include <cairo.h>


typedef struct bs_ln_cairo_st
{
  bs_int32_t width, height; 
  //SDL_Surface *pScreen; 
  cairo_t *pCairo;
} bs_ln_cairo_t; 

bs_ln_cairo_t*
bs_ln_cairo_st_create(bs_int32_t width, 
                    bs_int32_t height);


void 
bs_ln_cairo_st_destroy(bs_ln_cairo_t *pCairoSdl);                     

#endif
