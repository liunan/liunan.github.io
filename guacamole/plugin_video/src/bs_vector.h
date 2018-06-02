#ifndef __BS_VECTOR_H__
#define __BS_VECTOR_H__

#include "bs_types.h"

typedef struct bs_vector_st 
{
  bs_float_t x, y; 
} bs_vector_t;

void 
bs_vector_init(bs_vector_t *pVector, 
               bs_float_t x, 
               bs_float_t y); 

bs_vector_t*
bs_vector_create(bs_float_t x, 
                 bs_float_t y); 

void 
bs_vector_destroy(bs_vector_t *pVector); 

bs_bool_t 
bs_vector_equals(bs_vector_t *pA, 
                 bs_vector_t *pB);

bs_float_t 
bs_vector_get_x(bs_vector_t *pVector); 

bs_float_t
bs_vector_get_y(bs_vector_t *pVector); 

void 
bs_vector_set_x(bs_vector_t *pVector, 
                bs_float_t x); 

void 
bs_vector_set_y(bs_vector_t *pVector, 
                bs_float_t y); 

void
bs_vector_set(bs_vector_t *pDst, 
              bs_vector_t *pSrc); 

void 
bs_vector_add_x(bs_vector_t *pVector, 
                bs_float_t x); 

void 
bs_vector_add_y(bs_vector_t *pVector, 
                bs_float_t y); 

void 
bs_vector_add(bs_vector_t *pDst, 
              bs_vector_t *pSrc); 

void 
bs_vector_sub_x(bs_vector_t *pVector, 
                bs_float_t x); 

void 
bs_vector_sub_y(bs_vector_t *pVector, 
                bs_float_t y); 

void 
bs_vector_sub(bs_vector_t *pDst, 
              bs_vector_t *pSrc); 

bs_float_t 
bs_vector_dot_prod(bs_vector_t *pA,
                   bs_vector_t *pB); 
                   
bs_float_t 
bs_vector_dist(bs_vector_t *pA, 
               bs_vector_t *pB); 

bs_float_t 
bs_vector_length(bs_vector_t *pVector); 

void 
bs_vector_scale(bs_vector_t *pVector, 
                bs_float_t scaleFactor); 

void 
bs_vector_normalize(bs_vector_t *pVector); 

void 
bs_vector_print(bs_vector_t *pVector); 


#define BS_VECTOR_INIT(A, X, Y) \
  (A)->x = (X); \
  (A)->y = (Y); 

#define BS_VECTOR_EQUALS(A, B) \
  ((A)->x == (B)->x && (A)->y == (B)->y)

#define BS_VECTOR_GET_X(A) \
  (A)->x

#define BS_VECTOR_GET_Y(A) \
  (A)->y

#define BS_VECTOR_SET_X(A, X) \
  (A)->x = (X)

#define BS_VECTOR_SET_Y(A, Y) \
  (A)->y = (Y)

#define BS_VECTOR_SET(A, B) \
  (A)->x = (B)->x; \
  (A)->y = (B)->y; 

#define BS_VECTOR_ADD_X(A, X) \
  (A)->x += (X); 

#define BS_VECTOR_ADD_Y(A, Y) \
  (A)->y += (Y); 

#define BS_VECTOR_ADD(A, B) \
  (A)->x += (B)->x; \
  (A)->y += (B)->y; 

#define BS_VECTOR_SUB_X(A, X) \
  (A)->x -= (X); 

#define BS_VECTOR_SUB_Y(A, Y) \
  (A)->y -= (Y); 

#define BS_VECTOR_SUB(A, B) \
  (A)->x -= (B)->x; \
  (A)->y -= (B)->y; 

#define BS_VECTOR_DOT_PROD(A, B) \
  ((A)->x * (B)->x + (A)->y *(B)->y)

#define BS_VECTOR_DIST(A, B) \
  sqrt(((A)->x - (B)->x) * ((A)->x - (B)->x) + ((A)->y - (B)->y) * ((A)->y - (B)->y))

#define BS_VECTOR_LENGHT(A) \
  sqrt(BS_VECTOR_DOT_PROD(A, A))

#define BS_VECTOR_SCALE(A, SF) \
  (A)->x *= (SF); \
  (A)->y *= (SF); 
  
#define BS_VECTOR_NORMALIZE(A) \
  { \
    bs_float_t ALENGTH = BS_VECTOR_LENGHT(A); \
    BS_VECTOR_SCALE(A, 1.0f / ALENGTH); \
  }

#endif
