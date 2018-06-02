#ifndef __BS_UTIL_H__
#define __BS_UTIL_H__

#include "bs_types.h"

bs_float_t 
bs_util_random_float(); 

#define BS_UTIL_MIN(A, B) \
  ((A) < (B) ? (A) : (B))

#define BS_UTIL_MAX(A, B) \
  ((A) > (B) ? (A) : (B))

#define BS_UNUSED_PARAMETER(A) ((A) = (A))

#define BS_UTIL_CLAMP(V, L, U) (V) > (U) ? (U) : ((V) < (L) ? (L) : (V))

#endif
