#include <stdio.h>
#include <string.h>
#include <math.h>
#include <values.h>

#include "bs_octree.h"
#include "bs_assert.h"
#include "bs_mem.h"
#include "bs_util.h"
#include "bs_vector_util.h"
#include "bs_profiler.h"

#define BUFFER_SIZE 4096
#define NODE_INCREMENT_STEP 3

static bs_float_t
bs_octree_get_box_half_size(bs_octree_node_t *pNode)
{
  return BS_VECTOR_DIST(&(pNode->boxStart), &(pNode->boxEnd)) * 0.5f; 
}

static bs_octree_node_t* 
bs_octree_node_create(bs_float_t xBoxStart, 
                      bs_float_t yBoxStart, 
                      bs_float_t xBoxEnd, 
                      bs_float_t yBoxEnd, 
                      bs_array_element_destroy_cb destroyCallback)
{
  bs_octree_node_t *pNewNode; 

  BS_MEM_ALLOC_TYPE(pNewNode, bs_octree_node_t, 1); 

  pNewNode->pUpperLeft = NULL; 
  pNewNode->pUpperRight = NULL; 
  pNewNode->pLowerLeft = NULL; 
  pNewNode->pLowerRight = NULL;

  BS_VECTOR_INIT(&(pNewNode->boxStart), xBoxStart, yBoxStart); 
  BS_VECTOR_INIT(&(pNewNode->boxEnd), xBoxEnd, yBoxEnd); 

  pNewNode->halfSize = bs_octree_get_box_half_size(pNewNode); 
  
  pNewNode->pSegments = bs_array_create(1, destroyCallback); 
  
  return pNewNode; 
}

static void 
bs_octree_node_destroy(bs_octree_node_t *pNode)
{
  BS_ASSERT(pNode != NULL);

  if(pNode->pUpperLeft != NULL)
  {
    bs_octree_node_destroy(pNode->pUpperLeft);   
    bs_octree_node_destroy(pNode->pUpperRight);   
    bs_octree_node_destroy(pNode->pLowerLeft);   
    bs_octree_node_destroy(pNode->pLowerRight);   
  }

  bs_array_destroy(pNode->pSegments); 
  
  BS_MEM_FREE(pNode); 
}

typedef struct bs_octree_line_segment_st
{
  bs_vector_t start, end; 
} bs_octree_line_segment_t;

static bs_octree_line_segment_t*
bs_octree_read_line_segment(FILE *pFptr)
{
  char buffer[BUFFER_SIZE]; 
  char *pToken; 
  bs_octree_line_segment_t *pNewSegment; 

  // no more data in the file
  if(fgets(buffer, BUFFER_SIZE - 1, pFptr) == NULL)
  {
    return NULL; 
  }

  BS_MEM_ALLOC_TYPE(pNewSegment, bs_octree_line_segment_t, 1); 

  pToken = strtok(buffer, ",\r\n"); 
  BS_VECTOR_SET_X(&(pNewSegment->start), (bs_float_t) atof(pToken));

  pToken = strtok(NULL, ",\r\n"); 
  BS_VECTOR_SET_Y(&(pNewSegment->start), (bs_float_t) atof(pToken)); 

  pToken = strtok(NULL, ",\r\n"); 
  BS_VECTOR_SET_X(&(pNewSegment->end), (bs_float_t) atof(pToken)); 

  pToken = strtok(NULL, ",\r\n"); 
  BS_VECTOR_SET_Y(&(pNewSegment->end), (bs_float_t) atof(pToken)); 

  return pNewSegment; 
}

/* can be optimized a whole lot more */ 
static bs_bool_t 
bs_octree_box_contains_segment(bs_vector_t *pSegmentStart, 
                               bs_vector_t *pSegmentEnd,
                               bs_vector_t *pBoxStart, 
                               bs_vector_t *pBoxEnd)
{
  bs_float_t segmentXStart, segmentYStart, segmentXEnd, segmentYEnd; 
  bs_float_t boxXStart, boxYStart, boxXEnd, boxYEnd; 
  bs_vector_t segments[5]; 

  segmentXStart = BS_VECTOR_GET_X(pSegmentStart);
  segmentYStart = BS_VECTOR_GET_Y(pSegmentStart);
  segmentXEnd = BS_VECTOR_GET_X(pSegmentEnd);
  segmentYEnd = BS_VECTOR_GET_Y(pSegmentEnd); 

  boxXStart = BS_VECTOR_GET_X(pBoxStart); 
  boxYStart = BS_VECTOR_GET_Y(pBoxStart); 
  boxXEnd = BS_VECTOR_GET_X(pBoxEnd); 
  boxYEnd = BS_VECTOR_GET_Y(pBoxEnd); 

  if(segmentXStart < boxXStart && segmentXEnd < boxXStart)
  {
    return FALSE; 
  }
  if(segmentXStart > boxXEnd && segmentXEnd > boxXEnd)
  {
    return FALSE; 
  }
  if(segmentYStart < boxYStart && segmentYEnd < boxYStart)
  {
    return FALSE; 
  }
  if(segmentYStart > boxYEnd && segmentYEnd > boxYEnd)
  {
    return FALSE; 
  }
  
  if(segmentXStart >= boxXStart && segmentXStart <= boxXEnd && 
     segmentYStart >= boxYStart && segmentYStart <= boxYEnd)
  {
    return TRUE; 
  }
  if(segmentXEnd >= boxXStart && segmentXEnd <= boxXEnd && 
     segmentYEnd >= boxYStart && segmentYEnd <= boxYEnd)
  {
    return TRUE; 
  }

  BS_VECTOR_INIT(&(segments[0]), boxXStart, boxYStart); 
  BS_VECTOR_INIT(&(segments[1]), boxXEnd, boxYStart); 
  BS_VECTOR_INIT(&(segments[2]), boxXEnd, boxYEnd); 
  BS_VECTOR_INIT(&(segments[3]), boxXStart, boxYEnd); 
  BS_VECTOR_INIT(&(segments[4]), boxXStart, boxYStart); 
  
  if(bs_vector_util_test_segments(pSegmentStart, pSegmentEnd, segments, 4, 1) == TRUE)
  {
    return TRUE; 
  }
  
  return FALSE; 
}

static void 
bs_octree_child_node_claim_segments(bs_octree_node_t *pRoot, 
                                    bs_octree_node_t *pChild)
{
  bs_int32_t i; 
  bs_octree_line_segment_t *pSegment; 
  
  BS_ASSERT(pRoot != NULL);
  BS_ASSERT(pChild != NULL);   

  for(i = 0; i < BS_ARRAY_GET_SIZE(pRoot->pSegments); i++)
  {
    pSegment = (bs_octree_line_segment_t*) BS_ARRAY_GET(pRoot->pSegments, i); 
    
    if(bs_octree_box_contains_segment(&(pSegment->start), &(pSegment->end), 
      &(pChild->boxStart), &(pChild->boxEnd)) == TRUE)
    {
      BS_ARRAY_INSERT(pChild->pSegments, pSegment); 
    }
  }    
}

static void 
bs_octree_branch_tree(bs_octree_node_t *pRoot) 
{
  bs_float_t xBoxStart, yBoxStart, xBoxEnd, yBoxEnd; 
  bs_float_t xBoxSize, yBoxSize; 
  bs_octree_node_t *pTmpNode; 

  if(BS_ARRAY_GET_SIZE(pRoot->pSegments) < BS_OCTREE_MIN_SEGMENTS_PER_NODE)
  {
    return; 
  }

  xBoxStart = BS_VECTOR_GET_X(&(pRoot->boxStart)); 
  yBoxStart = BS_VECTOR_GET_Y(&(pRoot->boxStart)); 
  xBoxEnd = BS_VECTOR_GET_X(&(pRoot->boxEnd)); 
  yBoxEnd = BS_VECTOR_GET_Y(&(pRoot->boxEnd)); 

  xBoxSize = xBoxEnd - xBoxStart; 
  yBoxSize = yBoxEnd - yBoxStart; 

  if(xBoxSize < BS_OCTREE_MIN_NODE_SIZE)
  {
    return;
  }
  if(yBoxSize < BS_OCTREE_MIN_NODE_SIZE)
  {
    return;
  }

  pTmpNode = bs_octree_node_create(xBoxStart, yBoxStart, 
    xBoxStart + xBoxSize / 2.0f, yBoxStart + yBoxSize / 2.0f, NULL); 
  bs_octree_child_node_claim_segments(pRoot, pTmpNode); 
  bs_octree_branch_tree(pTmpNode);  
  pRoot->pUpperLeft = pTmpNode; 

  pTmpNode = bs_octree_node_create(xBoxStart + xBoxSize / 2.0f, 
    yBoxStart, xBoxEnd, yBoxStart + yBoxSize / 2.0f, NULL); 
  bs_octree_child_node_claim_segments(pRoot, pTmpNode); 
  bs_octree_branch_tree(pTmpNode); 
  pRoot->pUpperRight = pTmpNode; 

  pTmpNode = bs_octree_node_create(xBoxStart, yBoxStart + yBoxSize / 2.0f, 
    xBoxStart + xBoxSize / 2.0f, yBoxEnd, NULL); 
  bs_octree_child_node_claim_segments(pRoot, pTmpNode); 
  bs_octree_branch_tree(pTmpNode); 
  pRoot->pLowerLeft = pTmpNode; 

  pTmpNode = bs_octree_node_create(xBoxStart + xBoxSize / 2.0f, 
    yBoxStart + yBoxSize / 2.0f, xBoxEnd, yBoxEnd, NULL); 
  bs_octree_child_node_claim_segments(pRoot, pTmpNode); 
  bs_octree_branch_tree(pTmpNode); 
  pRoot->pLowerRight = pTmpNode; 
}

static void 
bs_octree_parse_line_segments(bs_octree_t *pOctree, 
                              FILE *pFptr)
{
  bs_octree_line_segment_t *pLineSegment; 
  bs_float_t tmp, minX, maxX, minY, maxY; 

  pLineSegment = bs_octree_read_line_segment(pFptr);
  if(pLineSegment == NULL)
  {
    return; 
  }

  minX = maxX = BS_VECTOR_GET_X(&(pLineSegment->start)); 
  minY = maxY = BS_VECTOR_GET_Y(&(pLineSegment->start)); 
  
  do
  {
    BS_ARRAY_INSERT(pOctree->pRoot->pSegments, pLineSegment); 

    tmp = BS_UTIL_MIN(BS_VECTOR_GET_X(&(pLineSegment->start)), 
      BS_VECTOR_GET_X(&(pLineSegment->end))); 
    minX = BS_UTIL_MIN(minX, tmp); 
    tmp = BS_UTIL_MAX(BS_VECTOR_GET_X(&(pLineSegment->start)), 
      BS_VECTOR_GET_X(&(pLineSegment->end))); 
    maxX = BS_UTIL_MAX(maxX, tmp); 
    tmp = BS_UTIL_MIN(BS_VECTOR_GET_Y(&(pLineSegment->start)), 
      BS_VECTOR_GET_Y(&(pLineSegment->end))); 
    minY = BS_UTIL_MIN(minY, tmp); 
    tmp = BS_UTIL_MAX(BS_VECTOR_GET_Y(&(pLineSegment->start)), 
      BS_VECTOR_GET_Y(&(pLineSegment->end))); 
    maxY = BS_UTIL_MAX(maxY, tmp); 

    pLineSegment = bs_octree_read_line_segment(pFptr);    
  }  while(pLineSegment != NULL); 

  BS_VECTOR_INIT(&(pOctree->pRoot->boxStart), minX, minY); 
  BS_VECTOR_INIT(&(pOctree->pRoot->boxEnd), maxX, maxY); 
  
  pOctree->pRoot->halfSize = bs_octree_get_box_half_size(pOctree->pRoot);

  bs_octree_branch_tree(pOctree->pRoot); 
}

static void 
bs_octree_segment_destroy_cb(void *pUserData)
{
  BS_ASSERT(pUserData != NULL);
  BS_MEM_FREE(pUserData);   
}

bs_octree_t*
bs_octree_create(char *pMapPath)
{
  FILE *pFptr; 
  bs_octree_t *pOctree; 

  BS_ASSERT(pMapPath != NULL); 

  pFptr = fopen(pMapPath, "r"); 
  if(pFptr == NULL)
  {
    fprintf(stderr, "Cannot open map file: \"%s\"\n", pMapPath);
    exit(1); 
  }

  BS_MEM_ALLOC_TYPE(pOctree, bs_octree_t, 1);

  pOctree->pRoot = bs_octree_node_create(0.0f, 0.0f, 0.0f, 0.0f, bs_octree_segment_destroy_cb); 
  bs_octree_parse_line_segments(pOctree, pFptr); 

  return pOctree; 
}

void
bs_octree_destroy(bs_octree_t *pOctree)
{
  BS_ASSERT(pOctree != NULL);

  bs_octree_node_destroy(pOctree->pRoot); 
  BS_MEM_FREE(pOctree); 
}

typedef struct bs_octree_serach_data_st
{
  bs_vector_t *pSegmentStart;  
  bs_vector_t *pSegmentEnd; 
  bs_float_t segmentLength; 
  bs_vector_t *pResult; 
  bs_float_t resultDist; 
  bs_bool_t intersect; 
} bs_octree_serach_data_t;

#define BS_OCTREE_NODE_HAVE_CHILDREN(NODE) \
  ((NODE)->pUpperLeft != NULL)

static bs_float_t
bs_octree_test_segments(bs_vector_t *pSegmentStart, 
                        bs_vector_t *pSegmentEnd, 
                        bs_array_t *pSegments, 
                        bs_vector_t *pResult)
{
  bs_int32_t i; 
  bs_float_t r, s, k; 
  bs_float_t bXaX, aYcY, dYcY, bYaY, dXcX, aXcX; 
  bs_octree_line_segment_t *pSegment; 
  bs_vector_t tResult; 
  bs_float_t closestIntersection = MAXFLOAT, tDist; 
  
  bs_profiler_start(BS_PROFILER_TASK_OCTREE_TEST_LINE_SEGMENT); 
  
  bXaX = BS_VECTOR_GET_X(pSegmentEnd) - BS_VECTOR_GET_X(pSegmentStart); 
  bYaY = BS_VECTOR_GET_Y(pSegmentEnd) - BS_VECTOR_GET_Y(pSegmentStart); 
  
  for(i = 0; i < BS_ARRAY_GET_SIZE(pSegments); i++)
  {
    pSegment = BS_ARRAY_GET(pSegments, i); 
    
    dXcX = BS_VECTOR_GET_X(&(pSegment->end)) - BS_VECTOR_GET_X(&(pSegment->start)); 
    dYcY = BS_VECTOR_GET_Y(&(pSegment->end)) - BS_VECTOR_GET_Y(&(pSegment->start)); 
  
    k = (bXaX * dYcY - bYaY * dXcX); 
    if(k == 0.0f)
    {
      continue; 
    }

    aXcX = BS_VECTOR_GET_X(pSegmentStart) - BS_VECTOR_GET_X(&(pSegment->start)); 
    aYcY = BS_VECTOR_GET_Y(pSegmentStart) - BS_VECTOR_GET_Y(&(pSegment->start)); 

    r = (aYcY * dXcX - aXcX * dYcY) / k; 
    s = (aYcY * bXaX - aXcX * bYaY) / k; 
    if(r < 0.0f || r > 1.0f || s < 0.0f || s > 1.0f)
    {
      continue; 
    }
    
    BS_VECTOR_SET(&tResult, pSegmentEnd); 
    BS_VECTOR_SUB(&tResult, pSegmentStart);
    BS_VECTOR_SCALE(&tResult, r);
    BS_VECTOR_ADD(&tResult, pSegmentStart);     
    
    tDist = BS_VECTOR_DOT_PROD(&tResult, pSegmentStart);
    if(tDist < closestIntersection)
    {
      closestIntersection = tDist; 
      BS_VECTOR_SET(pResult, &tResult); 
    }
  } 
  
  bs_profiler_stop(BS_PROFILER_TASK_OCTREE_TEST_LINE_SEGMENT); 
  
  return closestIntersection;   
}

static void
bs_octree_test_line_segment_r(bs_octree_node_t *pNode, 
                              bs_octree_serach_data_t *pSearchData)
{
  BS_ASSERT(pNode != NULL);
  BS_ASSERT(pSearchData != NULL); 

  if(BS_ARRAY_GET_SIZE(pNode->pSegments) == 0)
  {
    return; 
  }

  if(bs_octree_box_contains_segment(pSearchData->pSegmentStart, 
    pSearchData->pSegmentEnd, &(pNode->boxStart), &(pNode->boxEnd)) == FALSE)
  {
    return; 
  }

  if(BS_OCTREE_NODE_HAVE_CHILDREN(pNode) == FALSE || 
     pNode->halfSize < pSearchData->segmentLength)
  {
    bs_vector_t result; 
    bs_float_t resultDist; 

    BS_VECTOR_INIT(&result, 0.0f, 0.0f); 
    resultDist = bs_octree_test_segments(pSearchData->pSegmentStart, 
      pSearchData->pSegmentEnd, pNode->pSegments, &result); 
    
    if(resultDist < pSearchData->resultDist)
    {
      pSearchData->resultDist = resultDist; 
      BS_VECTOR_SET(pSearchData->pResult, &result); 
      pSearchData->intersect = TRUE; 
    }
  }
  else 
  {
    bs_octree_test_line_segment_r(pNode->pUpperLeft, pSearchData); 
    bs_octree_test_line_segment_r(pNode->pUpperRight, pSearchData); 
    bs_octree_test_line_segment_r(pNode->pLowerLeft, pSearchData); 
    bs_octree_test_line_segment_r(pNode->pLowerRight, pSearchData); 
  }
}


bs_bool_t
bs_octree_test_line_segment(bs_octree_t *pOctree, 
                            bs_vector_t *pSegmentStart, 
                            bs_vector_t *pSegmentEnd, 
                            bs_vector_t *pResult)
{
  bs_octree_serach_data_t searchData; 

  bs_profiler_start(BS_PROFILER_TASK_OCTREE); 
  
  searchData.pResult = pResult; 
  searchData.resultDist = MAXFLOAT;  
  searchData.pSegmentStart = pSegmentStart; 
  searchData.pSegmentEnd = pSegmentEnd; 
  searchData.segmentLength = BS_VECTOR_DIST(pSegmentStart, pSegmentEnd); 
  searchData.intersect = FALSE; 

  bs_octree_test_line_segment_r(pOctree->pRoot, &searchData); 

  bs_profiler_stop(BS_PROFILER_TASK_OCTREE); 

  return searchData.intersect; 
}

static void 
bs_octree_draw_segments(bs_octree_node_t *pNode, 
                        cairo_t *pCairo, 
                        bs_float_t scaleFactor)
{
  bs_int32_t i; 
  bs_octree_line_segment_t *pSegment; 
  bs_vector_t *pStart, *pEnd; 
  
  cairo_set_source_rgb(pCairo, 0.0, 0.0, 0.0); 
  cairo_set_line_width(pCairo, 2.0); 
  
  for(i = 0; i < BS_ARRAY_GET_SIZE(pNode->pSegments); i++)
  {
    pSegment = (bs_octree_line_segment_t*) BS_ARRAY_GET(pNode->pSegments, i); 
    
    pStart = &(pSegment->start); 
    pEnd = &(pSegment->end); 
  
    cairo_new_path(pCairo); 
    cairo_move_to(pCairo, BS_VECTOR_GET_X(pStart) * scaleFactor, 
      BS_VECTOR_GET_Y(pStart) * scaleFactor); 
    cairo_line_to(pCairo, BS_VECTOR_GET_X(pEnd) * scaleFactor, 
      BS_VECTOR_GET_Y(pEnd) * scaleFactor); 
    cairo_stroke(pCairo);     
  }
}

static void
bs_octree_draw_nodes(bs_octree_node_t *pNode, 
                     cairo_t *pCairo, 
                     bs_float_t scaleFactor)
{
  bs_float_t x1, y1, x2, y2; 
  
  BS_ASSERT(pNode != NULL); 

  x1 = BS_VECTOR_GET_X(&(pNode->boxStart)) * scaleFactor; 
  y1 = BS_VECTOR_GET_Y(&(pNode->boxStart)) * scaleFactor; 
  x2 = BS_VECTOR_GET_X(&(pNode->boxEnd)) * scaleFactor; 
  y2 = BS_VECTOR_GET_Y(&(pNode->boxEnd)) * scaleFactor; 
  
  cairo_new_path(pCairo);
  cairo_set_line_width(pCairo, 1.0f);   
  cairo_move_to(pCairo, x1, y1); 
  cairo_line_to(pCairo, x2, y1); 
  cairo_line_to(pCairo, x2, y2); 
  cairo_line_to(pCairo, x1, y2); 
  cairo_close_path(pCairo); 
  cairo_stroke(pCairo); 

  if(pNode->pUpperLeft != NULL)
  {
    bs_octree_draw_nodes(pNode->pUpperLeft, pCairo, scaleFactor); 
    bs_octree_draw_nodes(pNode->pUpperRight, pCairo, scaleFactor); 
    bs_octree_draw_nodes(pNode->pLowerLeft, pCairo, scaleFactor); 
    bs_octree_draw_nodes(pNode->pLowerRight, pCairo, scaleFactor); 
  }
}

void 
bs_octree_draw(bs_octree_t *pOctree, 
               cairo_t *pCairo, 
               bs_float_t scaleFactor)
{
  BS_ASSERT(pOctree != NULL);
  BS_ASSERT(pCairo != NULL);

  bs_octree_draw_segments(pOctree->pRoot, pCairo, scaleFactor); 
  //bs_octree_draw_nodes(pOctree->pRoot, pCairo, scaleFactor); 
}
