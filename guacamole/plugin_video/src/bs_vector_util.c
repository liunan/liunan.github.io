#include "bs_vector_util.h"

bs_bool_t 
bs_vector_util_intersect(bs_vector_t *pA, 
                         bs_vector_t *pB, 
                         bs_vector_t *pC, 
                         bs_vector_t *pD, 
                         bs_vector_t *pR)
{
  bs_float_t r, s, k; 
  bs_float_t bXaX, aYcY, dYcY, bYaY, dXcX, aXcX; 
  
  bXaX = BS_VECTOR_GET_X(pB) - BS_VECTOR_GET_X(pA); 
  bYaY = BS_VECTOR_GET_Y(pB) - BS_VECTOR_GET_Y(pA); 
  aXcX = BS_VECTOR_GET_X(pA) - BS_VECTOR_GET_X(pC); 
  aYcY = BS_VECTOR_GET_Y(pA) - BS_VECTOR_GET_Y(pC); 
  dXcX = BS_VECTOR_GET_X(pD) - BS_VECTOR_GET_X(pC); 
  dYcY = BS_VECTOR_GET_Y(pD) - BS_VECTOR_GET_Y(pC); 
  
  k = (bXaX * dYcY - bYaY * dXcX); 
  if(k == 0.0f)
  {
    return FALSE; 
  }

  r = (aYcY * dXcX - aXcX * dYcY) / k; 
  s = (aYcY * bXaX - aXcX * bYaY) / k; 

  if(r < 0.0f || r > 1.0f || s < 0.0f || s > 1.0f)
  {
    return FALSE; 
  }

  if(pR != NULL)
  {
    BS_VECTOR_SET(pR, pB); 
    BS_VECTOR_SUB(pR, pA);
    BS_VECTOR_SCALE(pR, r);
    BS_VECTOR_ADD(pR, pA);     
  }

  return TRUE; 
}

bs_bool_t
bs_vector_util_test_segments(bs_vector_t *pSegmentA, 
                             bs_vector_t *pSegmentB, 
                             bs_vector_t *pSegments, 
                             bs_int32_t segmentsLength, 
                             bs_int32_t stride)
{
  bs_int32_t i; 
  bs_float_t r, s, k; 
  bs_float_t bXaX, aYcY, dYcY, bYaY, dXcX, aXcX; 
  
  bXaX = BS_VECTOR_GET_X(pSegmentB) - BS_VECTOR_GET_X(pSegmentA); 
  bYaY = BS_VECTOR_GET_Y(pSegmentB) - BS_VECTOR_GET_Y(pSegmentA); 
  
  for(i = 0; i < segmentsLength; i += stride)
  {
    aXcX = BS_VECTOR_GET_X(pSegmentA) - BS_VECTOR_GET_X(pSegments + i); 
    aYcY = BS_VECTOR_GET_Y(pSegmentA) - BS_VECTOR_GET_Y(pSegments + i); 
    dXcX = BS_VECTOR_GET_X(pSegments + i + 1) - BS_VECTOR_GET_X(pSegments + i); 
    dYcY = BS_VECTOR_GET_Y(pSegments + i + 1) - BS_VECTOR_GET_Y(pSegments + i); 
  
    k = (bXaX * dYcY - bYaY * dXcX); 
    if(k == 0.0f)
    {
      continue; 
    }

    r = (aYcY * dXcX - aXcX * dYcY) / k; 
    s = (aYcY * bXaX - aXcX * bYaY) / k; 
    if(r < 0.0f || r > 1.0f || s < 0.0f || s > 1.0f)
    {
      continue; 
    }

    return TRUE;     
  } 
  return FALSE;   
}

bs_bool_t 
bs_vector_util_point_inside_triangle(bs_vector_t *pPoint, 
                                     bs_vector_t *pTriangleA, 
                                     bs_vector_t *pTriangleB, 
                                     bs_vector_t *pTriangleC) 
{
  bs_float_t b0, b1, b2, b3; 
  bs_float_t x0, y0, x1, y1, x2, y2, x3, y3; 
  
  x0 = BS_VECTOR_GET_X(pPoint); 
  y0 = BS_VECTOR_GET_Y(pPoint); 
  x1 = BS_VECTOR_GET_X(pTriangleA); 
  y1 = BS_VECTOR_GET_Y(pTriangleA); 
  x2 = BS_VECTOR_GET_X(pTriangleB); 
  y2 = BS_VECTOR_GET_Y(pTriangleB); 
  x3 = BS_VECTOR_GET_X(pTriangleC); 
  y3 = BS_VECTOR_GET_Y(pTriangleC);
  
  b0 = ((x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1)); 
  b1 = (((x2 - x0) * (y3 - y0) - (x3 - x0) * (y2 - y0)) / b0); 
  b2 = (((x3 - x0) * (y1 - y0) - (x1 - x0) * (y3 - y0)) / b0); 
  b3 = 1.0f - b1 - b2; 
  
  if(b1 >= 0.0f && b2 >= 0.0f && b3 >= 0.0f)
  {
    return TRUE; 
  }
  
  return FALSE; 
}
