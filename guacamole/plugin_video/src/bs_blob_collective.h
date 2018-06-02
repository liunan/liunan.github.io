#ifndef __BS_BLOB_COLLECTIVE_H__
#define __BS_BLOB_COLLECTIVE_H__

#include "bs_types.h"
#include "bs_blob.h"
#include "bs_env.h"
#include "bs_vector.h"
#include "bs_list.h"

#include <cairo.h>

typedef struct bs_blob_collective_st 
{
  bs_list_factory_t *pFactory; 
  bs_list_t *pBlobs; 
} bs_blob_collective_t; 

bs_blob_collective_t* 
bs_blob_collective_create(bs_float_t xPos, 
                          bs_float_t yPos, 
                          bs_list_factory_t *pFactory); 

void 
bs_blob_collective_destroy(bs_blob_collective_t *pCollective); 

void 
bs_blob_collective_split(bs_blob_collective_t *pCollective); 

void 
bs_blob_collective_join(bs_blob_collective_t *pCollective); 

void 
bs_blob_collective_move(bs_blob_collective_t *pCollective, 
                        bs_float_t dt); 

void 
bs_blob_collective_sc(bs_blob_collective_t *pCollective, 
                      bs_env_t *pEnv); 
                      
//void 
//bs_blob_collective_merge_test_blobs(bs_blob_collective_t *pCollective); 

void 
bs_blob_collective_set_force(bs_blob_collective_t *pCollective, 
                             bs_vector_t *pForce); 

void 
bs_blob_collective_add_force(bs_blob_collective_t *pCollective, 
                             bs_vector_t *pForce); 

void 
bs_blob_collective_draw(bs_blob_collective_t *pCollective, 
                        cairo_t *pCairo, 
                        bs_float_t scaleFactor); 



void 
bs_blob_collective_larger_blobs(bs_blob_collective_t *pCollective);             

void 
bs_blob_collective_smaller_blobs(bs_blob_collective_t *pCollective);             

#endif
