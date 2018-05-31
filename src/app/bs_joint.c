#include "bs_joint.h"
#include "bs_mem.h"
#include "bs_assert.h"

bs_joint_t* 
bs_joint_create(bs_pointmass_t *pPointmassA, 
                bs_pointmass_t *pPointmassB, 
                bs_float_t shortConst, 
                bs_float_t longConst)
{
  bs_joint_t *pJoint; 
  bs_vector_t delta; 
  bs_float_t pointDist; 

  BS_MEM_ALLOC_TYPE(pJoint, bs_joint_t, 1); 
  
  pJoint->aIsMovable = TRUE; 
  pJoint->bIsMovable = TRUE; 
  pJoint->pAPos = bs_pointmass_get_pos(pPointmassA);   
  pJoint->pBPos = bs_pointmass_get_pos(pPointmassB); 

  bs_vector_set(&delta, pJoint->pBPos); 
  bs_vector_sub(&delta, pJoint->pAPos); 
  
  pointDist = bs_vector_dot_prod(&delta, &delta); 
  pJoint->pointDist = pointDist; 
  
  pJoint->shortConstSquared = pointDist * shortConst * shortConst; 
  pJoint->longConstSquared = pointDist * longConst * longConst; 
  
  pJoint->shortIsUsed = FALSE;   
  if(shortConst != BS_JOINT_NO_CONST)
  {
    pJoint->shortIsUsed = TRUE; 
  }
  
  pJoint->longIsUsed = FALSE;   
  if(longConst != BS_JOINT_NO_CONST)
  {
    pJoint->longIsUsed = TRUE; 
  }
  
  return pJoint; 
}

void 
bs_joint_destroy(bs_joint_t *pJoint)
{
  BS_ASSERT(pJoint != NULL);
  BS_MEM_FREE(pJoint);   
}
  
void 
bs_joint_set_consts(bs_joint_t *pJoint, 
                    bs_float_t shortConst, 
                    bs_float_t longConst)
{
  BS_ASSERT(pJoint != NULL); 
  pJoint->shortConstSquared = shortConst * shortConst; 
  pJoint->longConstSquared = longConst * longConst;   

  pJoint->shortIsUsed = FALSE;   
  if(shortConst != BS_JOINT_NO_CONST)
  {
    pJoint->shortIsUsed = TRUE; 
  }
  
  pJoint->longIsUsed = FALSE;   
  if(longConst != BS_JOINT_NO_CONST)
  {
    pJoint->longIsUsed = TRUE; 
  }
}

void 
bs_joint_set_movability(bs_joint_t *pJoint, 
                        bs_bool_t aIsMovable, 
                        bs_bool_t bIsMovable)
{
  BS_ASSERT(pJoint != NULL);
  pJoint->aIsMovable = aIsMovable; 
  pJoint->bIsMovable = bIsMovable;
}

void 
bs_joint_set_short_const(bs_joint_t *pJoint, 
                         bs_float_t shortConst)
{
  BS_ASSERT(pJoint != NULL);
  pJoint->shortConstSquared = pJoint->pointDist * shortConst * shortConst;   
}

void 
bs_joint_set_long_const(bs_joint_t *pJoint, 
                        bs_float_t longConst)
{
  BS_ASSERT(pJoint != NULL);
  pJoint->longConstSquared = pJoint->pointDist * longConst * longConst;   
}

void 
bs_joint_scale(bs_joint_t *pJoint, 
               bs_float_t scaleFactor)
{
  bs_float_t scaleFactorSquared; 
  
  BS_ASSERT(pJoint != NULL); 

  scaleFactorSquared = scaleFactor * scaleFactor; 
  pJoint->shortConstSquared *= scaleFactorSquared; 
  pJoint->longConstSquared *= scaleFactorSquared;    
}

static void 
bs_joint_move_positions(bs_joint_t *pJoint, 
                        bs_vector_t *pDelta)
{
  if(pJoint->aIsMovable == TRUE && pJoint->bIsMovable == TRUE)
  {
    BS_VECTOR_SUB(pJoint->pAPos, pDelta); 
    BS_VECTOR_ADD(pJoint->pBPos, pDelta);       
  }
  else if(pJoint->aIsMovable == TRUE)
  {
    BS_VECTOR_SCALE(pDelta, 2.0f); 
    BS_VECTOR_SUB(pJoint->pAPos, pDelta);       
  }
  else if(pJoint->bIsMovable == TRUE)
  {
    BS_VECTOR_SCALE(pDelta, 2.0f); 
    BS_VECTOR_ADD(pJoint->pBPos, pDelta);             
  }
}

void 
bs_joint_sc(bs_joint_t *pJoint)
{
  bs_float_t dotProd, scaleFactor; 
  bs_vector_t delta; 

  BS_ASSERT(pJoint != NULL); 
  
  BS_VECTOR_SET(&delta, pJoint->pBPos); 
  BS_VECTOR_SUB(&delta, pJoint->pAPos); 
  
  dotProd = BS_VECTOR_DOT_PROD(&delta, &delta); 
  
  if(pJoint->shortIsUsed == TRUE && 
     dotProd < pJoint->shortConstSquared)
  {
    scaleFactor = pJoint->shortConstSquared / (dotProd + pJoint->shortConstSquared) - 0.5f; 
    
    BS_VECTOR_SCALE(&delta, scaleFactor); 
    bs_joint_move_positions(pJoint, &delta); 
  }
  else if(pJoint->longIsUsed == TRUE && 
          dotProd > pJoint->longConstSquared)
  {
    scaleFactor = pJoint->longConstSquared / (dotProd + pJoint->longConstSquared) - 0.5f; 
    
    BS_VECTOR_SCALE(&delta, scaleFactor);     
    bs_joint_move_positions(pJoint, &delta); 
  }
}

void 
bs_joint_draw(bs_joint_t *pJoint, 
              cairo_t *pCairo, 
              bs_float_t scaleFactor)
{
  BS_ASSERT(pJoint != NULL); 
  BS_ASSERT(pCairo != NULL);

  cairo_set_source_rgb(pCairo, 1.0f, 0.0f, 0.0f); 
  cairo_set_line_width(pCairo, 1.0f); 
  
  cairo_new_path(pCairo); 
  cairo_move_to(pCairo, BS_VECTOR_GET_X(pJoint->pAPos) * scaleFactor, 
    BS_VECTOR_GET_Y(pJoint->pAPos) * scaleFactor);
  cairo_line_to(pCairo, BS_VECTOR_GET_X(pJoint->pBPos) * scaleFactor, 
    BS_VECTOR_GET_Y(pJoint->pBPos) * scaleFactor);
  cairo_stroke(pCairo); 
}
