#ifndef __BS_MEM_H__
#define __BS_MEM_H__

#include "bs_assert.h"

#include <stdlib.h>

#define BS_MEM_ALLOC_TYPE(PTR, TYPE, NUM) \
	(PTR) = (TYPE*) malloc(sizeof(TYPE) * (NUM)); \
	BS_ASSERT((PTR) != NULL); 

#define BS_MEM_REALLOC_TYPE(PTR, TYPE, NUM) \
  (PTR) = (TYPE*) realloc(PTR, sizeof(TYPE) * (NUM)); \
  BS_ASSERT((PTR) != NULL); 

#define BS_MEM_FREE(PTR) \
  BS_ASSERT((PTR) != NULL); \
	free(PTR)

#endif
