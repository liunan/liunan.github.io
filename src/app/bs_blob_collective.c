#include "bs_blob_collective.h"
#include "bs_mem.h"
#include "bs_pointmass.h"
#include "bs_util.h"
#include "bs_profiler.h"

#include <math.h>

static void 
bs_blob_collective_blob_destructor_cb(void *pUserData)
{
  bs_blob_t *pBlob; 
  
  BS_ASSERT(pUserData != NULL); 
  pBlob = (bs_blob_t*) pUserData; 
  bs_blob_destroy(pBlob); 
}

bs_blob_collective_t* 
bs_blob_collective_create(bs_float_t xPos, 
                          bs_float_t yPos, 
                          bs_list_factory_t *pFactory)
{
  bs_blob_collective_t *pCollective; 
  bs_blob_t *pBlob; 

  BS_MEM_ALLOC_TYPE(pCollective, bs_blob_collective_t, 1); 

  pCollective->pFactory = pFactory; 
  pCollective->pBlobs = bs_list_factory_get_list(pFactory); 
  
  pBlob = bs_blob_create(xPos, xPos, 0.4f); 
  bs_list_add_element_to_front(pCollective->pBlobs, (void*)pBlob, 
    bs_blob_collective_blob_destructor_cb); 
  
  return pCollective; 
}

void 
bs_blob_collective_destroy(bs_blob_collective_t *pCollective)
{
  BS_ASSERT(pCollective != NULL);
  BS_ASSERT(pCollective->pBlobs != NULL); 
  
  bs_list_factory_give_list(pCollective->pFactory, pCollective->pBlobs); 
}

typedef struct bs_blob_collective_find_largest_st
{
  bs_float_t maxRadius; 
  bs_blob_t *pMotherBlob; 
} bs_blob_collective_find_largest_t; 

static void 
bs_blob_collective_find_largest_cb(void *pUserData, 
                                   void *pAccumulator)
{
  bs_blob_t *pBlob; 
  bs_blob_collective_find_largest_t *pFindData;   
  bs_float_t radius; 
  
  BS_ASSERT(pUserData != NULL); 
  BS_ASSERT(pAccumulator != NULL); 
  
  pBlob = (bs_blob_t*) pUserData; 
  pFindData = (bs_blob_collective_find_largest_t*) pAccumulator; 
  
  radius = bs_blob_get_radius(pBlob); 
  if(radius > pFindData->maxRadius)
  {
    pFindData->maxRadius = radius; 
    pFindData->pMotherBlob = pBlob;  
  }
}

static void 
bs_blob_collective_add_new_blob(bs_blob_collective_t *pCollective, 
                                bs_float_t xPos, 
                                bs_float_t yPos, 
                                bs_float_t radius)
{
  bs_blob_t *pNewBlob; 
  
  pNewBlob = bs_blob_create(xPos, yPos, radius); 
  bs_list_add_element_to_front(pCollective->pBlobs, pNewBlob, 
    bs_blob_collective_blob_destructor_cb); 
}

static bs_bool_t 
bs_blob_collective_remove_blob_cb(void *pUserData, 
                                  void *pDataArg, 
                                  bs_bool_t *pCont)
{
  bs_blob_t *pCurrentBlob, *pBlobForRemoval; 
  
  BS_ASSERT(pUserData != NULL); 
  pCurrentBlob = (bs_blob_t*) pUserData; 
  BS_ASSERT(pDataArg != NULL);
  pBlobForRemoval = (bs_blob_t*) pDataArg;   
  
  if(pCurrentBlob == pBlobForRemoval)
  {
    *pCont = FALSE; 
    return TRUE; 
  }
  
  *pCont = TRUE; 
  return FALSE; 
}

static bs_float_t 
bs_blob_collective_split_get_radius(bs_float_t motherRadius, 
                                    bs_float_t partOfMother)
{
  bs_float_t motherVolume; 
  
  motherVolume = motherRadius * motherRadius * M_PI; 
  return sqrt(motherVolume * partOfMother / M_PI); 
}

void 
bs_blob_collective_split(bs_blob_collective_t *pCollective)
{
  bs_blob_collective_find_largest_t findData; 
  bs_float_t motherXPos, motherYPos;
  bs_float_t motherRadius; 
  bs_float_t aRadius, bRadius, largestRadius; 
  
  BS_ASSERT(pCollective != NULL);

  findData.maxRadius = -1.0f; 
  findData.pMotherBlob = NULL; 
  
  bs_list_map_acc(pCollective->pBlobs, bs_blob_collective_find_largest_cb, &findData); 
  
  BS_ASSERT(findData.pMotherBlob != NULL); 
  
  motherXPos = bs_blob_get_x_pos(findData.pMotherBlob); 
  motherYPos = bs_blob_get_y_pos(findData.pMotherBlob); 
  motherRadius = bs_blob_get_radius(findData.pMotherBlob); 
  
  aRadius = 5000.0f + 1000.0f - (bs_float_t)(random() % 2000); 
  aRadius /= 10000.0f;
  bRadius = 1.0f - aRadius; 
  
  largestRadius = BS_UTIL_MAX(aRadius, bRadius); 
  
  bs_blob_collective_add_new_blob(pCollective, motherXPos + largestRadius * 0.15f, motherYPos, 
    bs_blob_collective_split_get_radius(motherRadius, aRadius));
  bs_blob_collective_add_new_blob(pCollective, motherXPos - largestRadius * 0.15f, motherYPos, 
    bs_blob_collective_split_get_radius(motherRadius, bRadius)); 

  bs_list_unlink_p_cont(pCollective->pBlobs, bs_blob_collective_remove_blob_cb, findData.pMotherBlob); 
  
  printf("number of blobs: %d\n", bs_list_get_length(pCollective->pBlobs)); 
}

typedef struct bs_blob_collective_join_find_smallest_st
{
  bs_blob_t *pSmallest; 
  bs_float_t minRadius; 
} bs_blob_collective_join_find_smallest_t; 

static void 
bs_blob_collective_join_find_smallest_cb(void *pUserData, 
                                         void *pAccumulator)
{
  bs_blob_t *pBlob; 
  bs_blob_collective_join_find_smallest_t *pFindData; 
  bs_float_t radius; 
  
  BS_ASSERT(pUserData != NULL); 
  pBlob = (bs_blob_t*) pUserData; 
  BS_ASSERT(pAccumulator != NULL);
  pFindData = (bs_blob_collective_join_find_smallest_t*) pAccumulator; 

  radius = bs_blob_get_radius(pBlob); 
  if(radius < pFindData->minRadius)
  {
    pFindData->minRadius = radius; 
    pFindData->pSmallest = pBlob; 
  }    
}

typedef struct bs_blob_collective_join_find_closest_st
{
  bs_blob_t *pClosest; 
  bs_float_t minDist; 
  bs_blob_t *pSmallest; 
} bs_blob_collective_join_find_closest_t; 

static void 
bs_blob_collective_join_find_closest_cb(void *pUserData, 
                                        void *pAccumulator)
{
  bs_blob_t *pBlob; 
  bs_blob_collective_join_find_closest_t *pFindData; 
  bs_pointmass_t *pMiddle, *pSmallestMiddle; 
  bs_float_t aXbX, aYbY, dist; 
  
  BS_ASSERT(pUserData != NULL);
  pBlob = (bs_blob_t*) pUserData; 
  BS_ASSERT(pAccumulator != NULL);   
  pFindData = (bs_blob_collective_join_find_closest_t*) pAccumulator; 
  
  if(pBlob == pFindData->pSmallest)
  {
    return; 
  }
  
  pMiddle = bs_blob_get_middle(pBlob); 
  pSmallestMiddle = bs_blob_get_middle(pFindData->pSmallest); 
  
  aXbX = bs_pointmass_get_x_pos(pMiddle) - bs_pointmass_get_x_pos(pSmallestMiddle); 
  aYbY = bs_pointmass_get_y_pos(pMiddle) - bs_pointmass_get_y_pos(pSmallestMiddle); 
  
  dist = aXbX * aXbX + aYbY * aYbY; 
  
  if(dist < pFindData->minDist)
  {
    pFindData->minDist = dist; 
    pFindData->pClosest = pBlob; 
  }    
}

static bs_float_t
bs_blob_collective_join_get_radius(bs_float_t smallestRadius, 
                                   bs_float_t closestRadius)
{
  bs_float_t newRadius; 
  bs_float_t smallestVolume, closestVolume; 
  
  smallestVolume = smallestRadius * smallestRadius * M_PI; 
  closestVolume = closestRadius * closestRadius * M_PI; 
  newRadius = sqrt((smallestVolume + closestVolume) / M_PI); 
  
  return newRadius; 
}

static void 
bs_blob_collective_join_blobs(bs_blob_collective_t *pCollective, 
                              bs_blob_t *pClosest, 
                              bs_blob_t *pSmallest)
{
  bs_float_t newRadius, newXPos, newYPos; 
  
  newXPos = bs_blob_get_x_pos(pClosest); 
  newYPos = bs_blob_get_y_pos(pClosest); 

  newRadius = bs_blob_collective_join_get_radius(
    bs_blob_get_radius(pClosest), bs_blob_get_radius(pSmallest)); 
   
  bs_list_unlink_p_cont(pCollective->pBlobs, bs_blob_collective_remove_blob_cb, pClosest); 
  bs_list_unlink_p_cont(pCollective->pBlobs, bs_blob_collective_remove_blob_cb, pSmallest); 
  
  bs_blob_collective_add_new_blob(pCollective, newXPos, newYPos, newRadius); 
}

void 
bs_blob_collective_join(bs_blob_collective_t *pCollective)
{
  bs_blob_collective_join_find_smallest_t findSmallest; 
  bs_blob_collective_join_find_closest_t findClosest; 
  
  BS_ASSERT(pCollective != NULL);  
  BS_ASSERT(bs_list_get_length(pCollective->pBlobs) > 0);   

  if(bs_list_get_length(pCollective->pBlobs) == 1)
  {
    return; 
  }
  
  findSmallest.pSmallest = NULL; 
  findSmallest.minRadius = 10000.0f; 
  bs_list_map_acc(pCollective->pBlobs, bs_blob_collective_join_find_smallest_cb, &findSmallest); 
  BS_ASSERT(findSmallest.pSmallest != NULL); 
  
  findClosest.pClosest = NULL; 
  findClosest.minDist = 10000.0f; 
  findClosest.pSmallest = findSmallest.pSmallest;
  bs_list_map_acc(pCollective->pBlobs, bs_blob_collective_join_find_closest_cb, &findClosest); 
  BS_ASSERT(findClosest.pClosest != NULL); 
  
  bs_blob_collective_join_blobs(pCollective, findClosest.pClosest, findSmallest.pSmallest); 
}

static void 
bs_blob_collective_move_cb(void *pUserData, 
                           bs_float_t dt)
{
  bs_blob_move((bs_blob_t*)pUserData, dt);   
}

void 
bs_blob_collective_move(bs_blob_collective_t *pCollective, 
                        bs_float_t dt)
{
  BS_ASSERT(pCollective != NULL); 
  bs_list_map_f(pCollective->pBlobs, bs_blob_collective_move_cb, dt);
}

static void 
bs_blob_colelctive_blob_sc_cb(void *pUserData, 
                              void *pDataArg)
{
  bs_blob_sc((bs_blob_t*)pUserData, (bs_env_t*)pDataArg);   
}

static void 
bs_blob_collective_collision_blob_blob_cb(void *pUserData, 
                                          void *pDataArg)
{
  bs_blob_t *pAgainst, *pTested; 
  
  BS_ASSERT(pUserData != NULL);
  pAgainst = (bs_blob_t*) pUserData; 
  BS_ASSERT(pDataArg != NULL);   
  pTested = (bs_blob_t*) pDataArg;   
  
  if(pAgainst == pTested)
  {
    return; 
  }
  
  bs_blob_collision_test(pAgainst, pTested); 
}

static void 
bs_blob_collective_collision_list_blob_cb(void *pUserData, 
                                          void *pDataArg)
{
  bs_blob_collective_t *pCollective; 
  
  BS_ASSERT(pDataArg != NULL);     
  pCollective = (bs_blob_collective_t*) pDataArg; 
  
  bs_list_map_p(pCollective->pBlobs, bs_blob_collective_collision_blob_blob_cb, pUserData); 
}

void 
bs_blob_collective_sc(bs_blob_collective_t *pCollective, 
                      bs_env_t *pEnv)
{
  BS_ASSERT(pCollective != NULL);
  
  bs_profiler_start(BS_PROFILER_TASK_BLOB_COLLECTIVE); 

  bs_list_map_p(pCollective->pBlobs, bs_blob_colelctive_blob_sc_cb, (void*) pEnv);
  bs_list_map_p(pCollective->pBlobs, bs_blob_collective_collision_list_blob_cb, 
    (void*) pCollective);

  bs_profiler_stop(BS_PROFILER_TASK_BLOB_COLLECTIVE); 
}

static void 
bs_blob_collective_set_force_cb(void *pUserData, 
                                void *pDataArg)
{
  bs_blob_set_force((bs_blob_t*) pUserData, (bs_vector_t*) pDataArg);
}

void 
bs_blob_collective_set_force(bs_blob_collective_t *pCollective,
                             bs_vector_t *pForce)
{
  BS_ASSERT(pCollective != NULL);
  bs_list_map_p(pCollective->pBlobs, bs_blob_collective_set_force_cb, (void*) pForce);
}

static void 
bs_blob_collective_add_force_cb(void *pUserData, 
                                void *pDataArg)
{
  bs_blob_add_force((bs_blob_t*)pUserData, (bs_vector_t*) pDataArg); 
}

void 
bs_blob_collective_add_force(bs_blob_collective_t *pCollective, 
                             bs_vector_t *pForce)
{
  BS_ASSERT(pCollective != NULL); 
  bs_list_map_p(pCollective->pBlobs, bs_blob_collective_add_force_cb, (void*) pForce); 
}

static void 
bs_blob_collective_draw_blob_cb(void *pUserData, 
                                bs_float_t scaleFactor, 
                                void *pDataArg)
{
  bs_blob_draw((bs_blob_t*)pUserData, (cairo_t*) pDataArg, scaleFactor); 
}

void 
bs_blob_collective_draw(bs_blob_collective_t *pCollective, 
                        cairo_t *pCairo, 
                        bs_float_t scaleFactor)
{
  bs_list_map_fp(pCollective->pBlobs, bs_blob_collective_draw_blob_cb, 
    scaleFactor, (void*) pCairo);  
}

static void 
bs_blob_collective_larger_blobs_cb(void *pUserData)
{
  bs_blob_scale((bs_blob_t*)pUserData, 1.1f); 
}

static void 
bs_blob_collective_smaller_blobs_cb(void *pUserData)
{
  bs_blob_scale((bs_blob_t*)pUserData, 0.9f); 
}

void 
bs_blob_collective_larger_blobs(bs_blob_collective_t *pCollective)
{
  bs_list_map(pCollective->pBlobs, bs_blob_collective_larger_blobs_cb); 
}

void 
bs_blob_collective_smaller_blobs(bs_blob_collective_t *pCollective)
{
  bs_list_map(pCollective->pBlobs, bs_blob_collective_smaller_blobs_cb);   
}
