#include "bs_stick.h"
#include "bs_mem.h"
#include "bs_assert.h"

bs_stick_t* 
bs_stick_create(bs_pointmass_t *pPointmassA, 
                bs_pointmass_t *pPointmassB)
{
  bs_float_t length; 
  bs_stick_t *pStick; 
  
  BS_MEM_ALLOC_TYPE(pStick, bs_stick_t, 1); 
  
  pStick->pAPos = bs_pointmass_get_pos(pPointmassA); 
  pStick->pBPos = bs_pointmass_get_pos(pPointmassB); 
  length = bs_vector_dist(pStick->pAPos, pStick->pBPos); 
  pStick->lengthSquared = length * length; 
  
  return pStick; 
}

void 
bs_stick_destroy(bs_stick_t *pStick)
{
  BS_ASSERT(pStick != NULL); 
  BS_MEM_FREE(pStick); 
}

void 
bs_stick_scale(bs_stick_t *pStick, 
               bs_float_t scaleFactor)
{
  BS_ASSERT(pStick != NULL);
  pStick->lengthSquared *= scaleFactor * scaleFactor;   
}

void 
bs_stick_set_length(bs_stick_t *pStick, 
                    bs_float_t length)
{
  BS_ASSERT(pStick != NULL);
  pStick->lengthSquared = length * length;   
}

void 
bs_stick_sc(bs_stick_t *pStick)
{
  bs_vector_t delta;   
  bs_float_t dotProd, scaleFactor; 
    
  BS_VECTOR_SET(&delta, pStick->pBPos); 
  BS_VECTOR_SUB(&delta, pStick->pAPos); 

  dotProd = BS_VECTOR_DOT_PROD(&delta, &delta); 
  scaleFactor = pStick->lengthSquared / (dotProd + pStick->lengthSquared) - 0.5f; 
  BS_VECTOR_SCALE(&delta, scaleFactor); 
  
  BS_VECTOR_SUB(pStick->pAPos, &delta); 
  BS_VECTOR_ADD(pStick->pBPos, &delta);
}
