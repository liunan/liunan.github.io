#ifndef __BS_ARRAY_H__
#define __BS_ARRAY_H__

#include "bs_types.h"
#include "bs_assert.h"

typedef void (*bs_array_element_destroy_cb)(void*); 

typedef struct bs_array_st 
{
  bs_uint32_t size, capacity; 
  void **pArray;
  bs_array_element_destroy_cb destroyCallback; 
} bs_array_t; 

bs_array_t* 
bs_array_create(bs_uint32_t capacity, 
                bs_array_element_destroy_cb destroyCallback); 

void 
bs_array_destroy(bs_array_t *pArray); 

void 
bs_array_ensure_capacity(bs_array_t *pArray); 

void 
bs_array_trim(bs_array_t *pArray); 

#define BS_ARRAY_GET(ARRAY, INDEX) \
  (BS_ASSERT((ARRAY) != NULL), \
   BS_ASSERT((INDEX) < (ARRAY)->size), \
   BS_ASSERT((INDEX) >= 0), \
   (ARRAY)->pArray[INDEX]) 

#define BS_ARRAY_SET(ARRAY, INDEX, VALUE) \
  BS_ASSERT((ARRAY) != NULL); \
  BS_ASSERT((INDEX) >= 0); \
  BS_ASSERT((INDEX) < (ARRAY)->size); \
  (ARRAY)->pArray[INDEX] = (VALUE) 

#define BS_ARRAY_INSERT(ARRAY, VALUE) \
  BS_ASSERT((ARRAY) != NULL); \
  bs_array_ensure_capacity(ARRAY); \
  (ARRAY)->pArray[(ARRAY)->size] = (VALUE); \
  (ARRAY)->size++ 
  
#define BS_ARRAY_REMOVE(ARRAY, INDEX) \
  BS_ASSERT((ARRAY) != NULL); \
  BS_ASSERT((INDEX) >= 0); \
  BS_ASSERT((INDEX) < (ARRAY)->size); \
  BS_ASSERT((ARRAY)->size > 0); \
  if((ARRAY)->destroyCallback != NULL) \
  { \
    (ARRAY)->destroyCallback((ARRAY)[INDEX]); \
  } \
  (ARRAY)->size--; \
  (ARRAY)->pArray[INDEX] = (ARRAY)[(ARRAY)->size]

#define BS_ARRAY_GET_SIZE(ARRAY) \
  (BS_ASSERT((ARRAY) != NULL), (ARRAY)->size)
  
#define BS_ARRAY_GET_DATA(ARRAY) \
  (BS_ASSERT((ARRAY) != NULL), (ARRAY)->pArray)

#endif
