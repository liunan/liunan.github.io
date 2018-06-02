#ifndef __BS_ENV_H__
#define __BS_ENV_H__

#include "bs_types.h"
#include "bs_vector.h"
#include "bs_octree.h"

typedef struct bs_env_st 
{
  bs_float_t left, right, top, bottom; 
  bs_octree_t *pOctree;
} bs_env_t; 

bs_env_t*
bs_env_create(bs_float_t x, 
              bs_float_t y, 
              bs_float_t w, 
              bs_float_t h); 

void 
bs_env_destroy(bs_env_t *pEnv); 

// pInsidePos is assumed to be inside the environment 
// pTestedPos is the point that we might move if collision occurs
bs_bool_t 
bs_env_collision(bs_env_t *pEnv, 
                 bs_vector_t *pInsidePos, 
                 bs_vector_t *pTestedPos); 

void 
bs_env_draw(bs_env_t *pEnv, 
            cairo_t *pCairo, 
            bs_float_t scaleFactor); 

#endif
