#ifndef __BS_OCTREE__
#define __BS_OCTREE__

#include <assert.h>
#include <cairo.h>

#include "bs_types.h"
#include "bs_vector.h"
#include "bs_list.h"
#include "bs_array.h"

#define BS_OCTREE_MIN_SEGMENTS_PER_NODE 1
#define BS_OCTREE_MIN_NODE_SIZE 0.1f

typedef struct bs_octree_node_st bs_octree_node_t;
struct bs_octree_node_st
{
  bs_octree_node_t *pUpperLeft, *pUpperRight, *pLowerLeft, *pLowerRight; 
  bs_vector_t boxStart, boxEnd; 
  bs_float_t halfSize; 
  bs_array_t *pSegments; 
}; 

typedef struct bs_octree_st
{
  bs_octree_node_t *pRoot; 
} bs_octree_t;

bs_octree_t*
bs_octree_create(char *pMapPath);

void
bs_octree_destroy(bs_octree_t *pOctree); 

void 
bs_octree_print(bs_octree_t *pOctree);

void 
bs_octree_print_svg(bs_octree_t *pOctree);

bs_bool_t
bs_octree_test_line_segment(bs_octree_t *pOctree, 
                            bs_vector_t *pLineStart, 
                            bs_vector_t *pLineEnd, 
                            bs_vector_t *pResult); 

void 
bs_octree_draw(bs_octree_t *pOctree, 
               cairo_t *pCairo, 
               bs_float_t scaleFactor); 

#endif
