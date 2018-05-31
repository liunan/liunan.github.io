#include <math.h>

#include "bs_vector.h"
#include "bs_mem.h"
#include "bs_assert.h"

void 
bs_vector_init(bs_vector_t *pVector, 
               bs_float_t x, 
               bs_float_t y)
{
  BS_ASSERT(pVector != NULL);
  pVector->x = x; 
  pVector->y = y;   
}

bs_vector_t*
bs_vector_create(bs_float_t x, 
                 bs_float_t y)
{
  bs_vector_t *pVector; 

  BS_MEM_ALLOC_TYPE(pVector, bs_vector_t, 1); 
  
  pVector->x = x; 
  pVector->y = y; 
  
  return pVector; 
}

void 
bs_vector_destroy(bs_vector_t *pVector)
{
  BS_ASSERT(pVector != NULL); 
  BS_MEM_FREE(pVector); 
}

bs_bool_t 
bs_vector_equals(bs_vector_t *pA, 
                 bs_vector_t *pB)
{
  BS_ASSERT(pA != NULL); 
  BS_ASSERT(pB != NULL); 
  return pA->x == pB->x && pA->y == pB->y; 
}

bs_float_t 
bs_vector_get_x(bs_vector_t *pVector)
{
  BS_ASSERT(pVector != NULL); 
  return pVector->x; 
}

bs_float_t
bs_vector_get_y(bs_vector_t *pVector)
{
  BS_ASSERT(pVector != NULL); 
  return pVector->y; 
}

void 
bs_vector_set_x(bs_vector_t *pVector, 
                bs_float_t x)
{
  BS_ASSERT(pVector != NULL); 
  pVector->x = x; 
}

void 
bs_vector_set_y(bs_vector_t *pVector, 
                bs_float_t y)
{
  BS_ASSERT(pVector != NULL); 
  pVector->y = y; 
}

void
bs_vector_set(bs_vector_t *pDst, 
              bs_vector_t *pSrc)
{
  BS_ASSERT(pDst != NULL); 
  BS_ASSERT(pSrc != NULL); 
  pDst->x = pSrc->x; 
  pDst->y = pSrc->y; 
}

void 
bs_vector_add_x(bs_vector_t *pVector, 
                bs_float_t x)
{
  BS_ASSERT(pVector != NULL); 
  pVector->x += x;   
}

void 
bs_vector_add_y(bs_vector_t *pVector, 
                bs_float_t y)
{
  BS_ASSERT(pVector != NULL); 
  pVector->y += y;   
}

void 
bs_vector_add(bs_vector_t *pDst, 
              bs_vector_t *pSrc)
{
  BS_ASSERT(pDst != NULL);
  BS_ASSERT(pSrc != NULL); 
  pDst->x += pSrc->x; 
  pDst->y += pSrc->y;   
}

void 
bs_vector_sub_x(bs_vector_t *pVector, 
                bs_float_t x)
{
  BS_ASSERT(pVector != NULL);
  pVector->x -= x;   
}

void 
bs_vector_sub_y(bs_vector_t *pVector, 
                bs_float_t y)
{
  BS_ASSERT(pVector != NULL);
  pVector->y -= y;   
}

void 
bs_vector_sub(bs_vector_t *pDst, 
              bs_vector_t *pSrc)
{
  BS_ASSERT(pDst != NULL); 
  BS_ASSERT(pSrc != NULL);
  pDst->x -= pSrc->x; 
  pDst->y -= pSrc->y;   
}

bs_float_t 
bs_vector_dot_prod(bs_vector_t *pA,
                   bs_vector_t *pB)
{
  BS_ASSERT(pA != NULL);
  BS_ASSERT(pB != NULL);
  return pA->x * pB->x + pA->y * pB->y; 
}

bs_float_t 
bs_vector_dist(bs_vector_t *pA, 
               bs_vector_t *pB)
{
  bs_float_t aXbX, aYbY; 
  
  BS_ASSERT(pA != NULL);
  BS_ASSERT(pB != NULL);
  
  aXbX = pA->x - pB->x; 
  aYbY = pA->y - pB->y; 
  return sqrt(aXbX * aXbX + aYbY * aYbY);   
}

bs_float_t 
bs_vector_length(bs_vector_t *pVector)
{
  BS_ASSERT(pVector != NULL); 
  return sqrt(pVector->x * pVector->x + pVector->y * pVector->y);     
}

void 
bs_vector_scale(bs_vector_t *pVector, 
                bs_float_t scaleFactor)
{
  BS_ASSERT(pVector != NULL); 
  pVector->x *= scaleFactor; 
  pVector->y *= scaleFactor; 
}

void 
bs_vector_normalize(bs_vector_t *pVector)
{
  bs_float_t scaleFactor; 
  
  BS_ASSERT(pVector != NULL);
  
  scaleFactor = BS_VECTOR_LENGHT(pVector); 
  scaleFactor = 1.0f / scaleFactor; 
  BS_VECTOR_SCALE(pVector, scaleFactor);   
}

void
bs_vector_print(bs_vector_t *pVector)
{
  BS_ASSERT(pVector != NULL); 
  printf("X: %f, Y %f\n", pVector->x, pVector->y); 
}
