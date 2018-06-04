#include "bs_env.h"
#include "bs_mem.h"
#include "bs_array.h"

bs_env_t*
bs_env_create(bs_float_t x, 
              bs_float_t y, 
              bs_float_t w, 
              bs_float_t h)
{
  bs_env_t *pEnv; 

  BS_MEM_ALLOC_TYPE(pEnv, bs_env_t, 1); 
  
  pEnv->left = x; 
  pEnv->right = x + w; 
  pEnv->top = y; 
  pEnv->bottom = y + h; 
  
  pEnv->pOctree = bs_octree_create("/usr/local/lib/maps/level1.txt"); 
  
  return pEnv; 
}

void 
bs_env_destroy(bs_env_t *pEnv)
{
  BS_ASSERT(pEnv != NULL);
  BS_MEM_FREE(pEnv); 
}

bs_bool_t 
bs_env_collision(bs_env_t *pEnv, 
                 bs_vector_t *pInsidePos, 
                 bs_vector_t *pTestedPos)
{
  bs_bool_t collision = FALSE; 
  bs_float_t x, y; 
  bs_vector_t result; 

  x = bs_vector_get_x(pTestedPos); 
  y = bs_vector_get_y(pTestedPos); 

  if(x < pEnv->left)
  {
    bs_vector_set_x(pTestedPos, pEnv->left); 
    collision = TRUE;     
  }    
  else if(x > pEnv->right)
  {
    bs_vector_set_x(pTestedPos, pEnv->right); 
    collision = TRUE;     
  }
  if(y < pEnv->top)
  {
    bs_vector_set_y(pTestedPos, pEnv->top); 
    collision = TRUE;     
  }    
  else if(y > pEnv->bottom)
  {
    bs_vector_set_y(pTestedPos, pEnv->bottom); 
    collision = TRUE;     
  }

  if(bs_octree_test_line_segment(pEnv->pOctree, pInsidePos, pTestedPos, &result) == TRUE)
  {
    BS_VECTOR_SET(pTestedPos, &result); 
    collision = TRUE;   
  }    
  
  return collision; 
}

void 
bs_env_draw(bs_env_t *pEnv, 
            cairo_t *pCairo, 
            bs_float_t scaleFactor)
{
  BS_ASSERT(pEnv != NULL);
  bs_octree_draw(pEnv->pOctree, pCairo, scaleFactor);   
}
