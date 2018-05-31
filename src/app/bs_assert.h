#ifndef __BS_ASSERT_H__
#define __BS_ASSERT_H__

#define BS_USE_ASSERT
#ifdef BS_USE_ASSERT

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define BS_ASSERT(EXPRESSION) \
  (void)((EXPRESSION) || (printf("Assertion failed: %s, %s:%d\n", #EXPRESSION, __FILE__, __LINE__), FALSE) || (exit(1), TRUE)) 

#else /* BS_USE_ASSERT */ 

#define BS_ASSERT(EXPRESSION) 

#endif /* BS_USE_ASSERT */ 

#endif
