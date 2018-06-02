#ifndef __BS_TYPES_H__
#define __BS_TYPES_H__

typedef float     bs_float_t; 
typedef unsigned int   bs_uint32_t; 
typedef int     bs_int32_t; 
typedef int     bs_bool_t; 

#ifndef TRUE 
#define TRUE (1 == 1)
#endif

#ifndef FALSE
#define FALSE (1 == 0)
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

#endif
