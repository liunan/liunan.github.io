#ifndef __BS_VECTOR_UTIL_H__
#define __BS_VECTOR_UTIL_H__

#include "bs_vector.h"
#include "bs_types.h"

/* Test is two line segments (pA, pB) (pC, pD) intersect
 * return TRUE if so, then result position is stored in 
 * in pR. FALSE if not, contents of pR is then undefined. */  
bs_bool_t
bs_vector_util_intersect(bs_vector_t *pA, 
                         bs_vector_t *pB, 
                         bs_vector_t *pC, 
                         bs_vector_t *pD, 
                         bs_vector_t *pR); 

bs_bool_t
bs_vector_util_test_segments(bs_vector_t *pSegmentA, 
                             bs_vector_t *pSegmentB, 
                             bs_vector_t *pSegments, 
                             bs_int32_t segmentsLength, 
                             bs_int32_t stride); 

bs_bool_t 
bs_vector_util_point_inside_triangle(bs_vector_t *pPoint, 
                                     bs_vector_t *pTriangleA, 
                                     bs_vector_t *pTriangleB, 
                                     bs_vector_t *pTriangleC); 

#endif
