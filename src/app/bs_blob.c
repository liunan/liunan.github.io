#include <math.h>

#include "bs_blob.h"
#include "bs_mem.h"
#include "bs_assert.h"
#include "bs_util.h"
#include "bs_vector.h"

static bs_int32_t 
bs_blob_get_num_pointmasses(bs_float_t radius)
{  
  return 40;
}

static bs_int32_t 
bs_blob_clamp_index(bs_int32_t index, 
                    bs_int32_t highBound)
{
  while(index < 0)
  {
    index = highBound + index; 
  }
  while(index >= highBound)
  {
    index = index - highBound; 
  }
  
  return index; 
}

bs_blob_t* 
bs_blob_create(bs_float_t xPos, 
               bs_float_t yPos, 
               bs_float_t radius)
{
  bs_blob_t *pBlob; 
  bs_int32_t i, p; 
  
  BS_MEM_ALLOC_TYPE(pBlob, bs_blob_t, 1);   

  pBlob->pMiddle = bs_pointmass_create(xPos, yPos, 1.0); 
  pBlob->pComputedMiddle = bs_vector_create(xPos, yPos); 
  pBlob->radius = radius; 
  pBlob->faceStyle = BS_BLOB_FACE_STYLE_MOUTH_CLOSED; 
  pBlob->eyeStyle = BS_BLOB_EYE_STYLE_OPEN; 

  pBlob->pMiddle = bs_pointmass_create(xPos, yPos, 1.0f); 
  pBlob->spA = bs_vector_create(0.1f, 0.1f); 
  pBlob->spB = bs_vector_create(0.2f, 0.2f); 

  pBlob->numPointmasses = bs_blob_get_num_pointmasses(radius); 
  pBlob->numSticks = pBlob->numPointmasses; 
  pBlob->numJoints = pBlob->numPointmasses * 5; 
  
  BS_MEM_ALLOC_TYPE(pBlob->pPointmasses, bs_pointmass_t*, pBlob->numPointmasses); 
  for(i = 0; i < pBlob->numPointmasses; i++)
  {
    bs_float_t t; 
    
    t = (bs_float_t) i / (bs_float_t) pBlob->numPointmasses * M_PI * 2.0f; 
    pBlob->pPointmasses[i] = bs_pointmass_create(xPos + cos(t) * radius, yPos + sin(t) * radius, 1.0f); 
  }
  
  BS_MEM_ALLOC_TYPE(pBlob->pPointmassesScAccess, bs_pointmass_t*, pBlob->numPointmasses); 
  for(i = 0; i < pBlob->numPointmasses; i++)
  {
    pBlob->pPointmassesScAccess[i] = NULL; 
  }
  for(i = 0; i < pBlob->numPointmasses; i++)
  {
    bs_int32_t j; 
    
    do
    {
      j = random() % pBlob->numPointmasses;    
    } while(pBlob->pPointmassesScAccess[j] != NULL);  
    
    pBlob->pPointmassesScAccess[j] = pBlob->pPointmasses[i]; 
  } 
  
  
  BS_MEM_ALLOC_TYPE(pBlob->pSticks, bs_stick_t*, pBlob->numSticks); 
  for(i = 1; i < pBlob->numSticks; i++)
  {
    pBlob->pSticks[i] = bs_stick_create(pBlob->pPointmasses[i - 1], pBlob->pPointmasses[i]); 
  }
  pBlob->pSticks[0] = bs_stick_create(pBlob->pPointmasses[0], pBlob->pPointmasses[pBlob->numSticks - 1]); 

  BS_MEM_ALLOC_TYPE(pBlob->pSticksScAccess, bs_stick_t*, pBlob->numSticks); 
  for(i = 0; i < pBlob->numSticks; i++)
  {
    pBlob->pSticksScAccess[i] = NULL; 
  }
  for(i = 0; i < pBlob->numPointmasses; i++)
  {
    bs_int32_t j; 
    
    do 
    {
      j = random() % pBlob->numSticks;    
    } while(pBlob->pSticksScAccess[j] != NULL);      
    
    pBlob->pSticksScAccess[j] = pBlob->pSticks[i]; 
  } 
  
  
  pBlob->shortConst = 1.07; 
  pBlob->longConst = 1.45f; 
  BS_MEM_ALLOC_TYPE(pBlob->pJoints, bs_joint_t*, pBlob->numJoints); 
  for(i = 0, p = 0; i < pBlob->numPointmasses; i++)
  {
    bs_int32_t t, k; 

    k = pBlob->numPointmasses / 2; 

    t = bs_blob_clamp_index(i - 5, pBlob->numPointmasses); 
    pBlob->pJoints[p++] = bs_joint_create(pBlob->pPointmasses[i], 
      pBlob->pPointmasses[t], pBlob->shortConst, pBlob->longConst); 
    t = bs_blob_clamp_index(i + 5, pBlob->numPointmasses); 
    pBlob->pJoints[p++] = bs_joint_create(pBlob->pPointmasses[i],  
      pBlob->pPointmasses[t], pBlob->shortConst, pBlob->longConst); 

    t = bs_blob_clamp_index(i + k - 6, pBlob->numPointmasses); 
    pBlob->pJoints[p++] = bs_joint_create(pBlob->pPointmasses[i], 
      pBlob->pPointmasses[t], pBlob->shortConst, pBlob->longConst); 
    t = bs_blob_clamp_index(i + k + 6, pBlob->numPointmasses); 
    pBlob->pJoints[p++] = bs_joint_create(pBlob->pPointmasses[i], 
      pBlob->pPointmasses[t], pBlob->shortConst, pBlob->longConst); 
    
    pBlob->pJoints[p++] = bs_joint_create(pBlob->pPointmasses[i], 
      pBlob->pMiddle, pBlob->shortConst, pBlob->longConst); 
  }

  BS_MEM_ALLOC_TYPE(pBlob->pJointsScAccess, bs_joint_t*, pBlob->numJoints); 
  for(i = 0; i < pBlob->numJoints; i++)
  {
    pBlob->pJointsScAccess[i] = NULL; 
  }
  for(i = 0; i < pBlob->numJoints; i++)
  {
    bs_int32_t j; 
    
    do 
    {
      j = random() % pBlob->numJoints;    
    } while(pBlob->pJointsScAccess[j] != NULL); 
    
    pBlob->pJointsScAccess[j] = pBlob->pJoints[i]; 
  } 

  return pBlob; 
}

void 
bs_blob_destroy(bs_blob_t *pBlob)
{
  int i; 
  
  BS_ASSERT(pBlob != NULL); 

  bs_pointmass_destroy(pBlob->pMiddle); 
  
  BS_ASSERT(pBlob->pPointmasses != NULL);   
  for(i = 0; i < pBlob->numPointmasses; i++)
  {
    bs_pointmass_destroy(pBlob->pPointmasses[i]); 
  }
  BS_MEM_FREE(pBlob->pPointmasses); 
  
  BS_ASSERT(pBlob->pSticks != NULL); 
  for(i = 0; i < pBlob->numSticks; i++)
  {
    bs_stick_destroy(pBlob->pSticks[i]); 
  }
  BS_MEM_FREE(pBlob->pSticks); 
  
  BS_ASSERT(pBlob->pJoints != NULL); 
  for(i = 0; i < pBlob->numJoints; i++)
  {
    bs_joint_destroy(pBlob->pJoints[i]); 
  }
  BS_MEM_FREE(pBlob->pJoints); 
  
  BS_MEM_FREE(pBlob); 
}

bs_float_t 
bs_blob_get_radius(bs_blob_t *pBlob)
{
  BS_ASSERT(pBlob != NULL); 
  return pBlob->radius;   
}

void
bs_blob_scale(bs_blob_t *pBlob, 
              bs_float_t scaleFactor)
{
  int i; 

  BS_ASSERT(pBlob != NULL); 
  BS_ASSERT(scaleFactor > 0.0f); 

  for(i = 0; i < pBlob->numJoints; i++)
  {
    bs_joint_scale(pBlob->pJoints[i], scaleFactor); 
  }    
  for(i = 0; i < pBlob->numSticks; i++)
  {
    bs_stick_scale(pBlob->pSticks[i], scaleFactor); 
  }
  pBlob->radius *= scaleFactor;
}

bs_pointmass_t* 
bs_blob_get_middle(bs_blob_t *pBlob)
{
  BS_ASSERT(pBlob != NULL);
  return pBlob->pMiddle;   
}

bs_float_t 
bs_blob_get_x_pos(bs_blob_t *pBlob)
{
  BS_ASSERT(pBlob != NULL);
  return bs_pointmass_get_x_pos(pBlob->pMiddle);   
}

bs_float_t 
bs_blob_get_y_pos(bs_blob_t *pBlob)
{
  BS_ASSERT(pBlob != NULL);
  return bs_pointmass_get_y_pos(pBlob->pMiddle);   
}

static bs_float_t 
bs_blob_compute_volume(bs_blob_t *pBlob)
{
  bs_int32_t i; 
  bs_float_t volume, t; 
  bs_vector_t *pA, *pB, *pM; 
  
  pM = bs_pointmass_get_pos(pBlob->pMiddle); 
  volume = 0.0f; 
  for(i = 0; i < pBlob->numSticks; i++)
  {
    pA = BS_STICK_GET_A_POS(pBlob->pSticks[i]); 
    pB = BS_STICK_GET_B_POS(pBlob->pSticks[i]); 
    
    t  = BS_VECTOR_GET_X(pA) * (BS_VECTOR_GET_Y(pB) - BS_VECTOR_GET_Y(pM)); 
    t += BS_VECTOR_GET_X(pB) * (BS_VECTOR_GET_Y(pM) - BS_VECTOR_GET_Y(pA)); 
    t += BS_VECTOR_GET_X(pM) * (BS_VECTOR_GET_Y(pA) - BS_VECTOR_GET_Y(pB)); 
    
    volume += fabs(t); 
  }
  
  volume *= 0.5f; 
  
  return volume; 
}

static void 
bs_blob_compute_middle(bs_blob_t *pBlob, 
                       bs_vector_t *pMiddle)
{
  bs_int32_t i; 

  BS_VECTOR_SET_X(pMiddle, 0.0f); 
  BS_VECTOR_SET_Y(pMiddle, 0.0f); 
  for(i = 0; i < pBlob->numPointmasses; i++)
  {
    BS_VECTOR_ADD(pMiddle, BS_POINTMASS_GET_CUR_POS(pBlob->pPointmasses[i])); 
  }    
  BS_VECTOR_SCALE(pMiddle, 1.0f / (bs_float_t) pBlob->numPointmasses);
}

static void 
bs_blob_pump(bs_blob_t *pBlob)
{
  bs_float_t targetVolume, actualVolume, scaleFactor; 
  bs_int32_t i; 
  bs_vector_t middle;
  
  targetVolume = pBlob->radius * pBlob->radius * M_PI * 1.5f; 
  actualVolume = bs_blob_compute_volume(pBlob);

  if(actualVolume < targetVolume * 0.0001f)
  {
    actualVolume = targetVolume * 0.0001f; 
  }
  
  scaleFactor = targetVolume / actualVolume; 
  scaleFactor -= 1.0f; 
  scaleFactor *= 0.1f; 
  
  bs_blob_compute_middle(pBlob, &middle); 
  
  for(i = 0; i < pBlob->numPointmasses; i++)
  {
    bs_vector_t t, *pPM; 
    
    pPM = bs_pointmass_get_pos(pBlob->pPointmasses[i]); 
    BS_VECTOR_SET(&t, pPM);
    BS_VECTOR_SUB(&t, &middle);
    BS_VECTOR_SCALE(&t, scaleFactor); 
    BS_VECTOR_ADD(pPM, &t);     
  }
}

void 
bs_blob_move(bs_blob_t *pBlob, 
             bs_float_t dt)
{
  int i; 

  BS_ASSERT(pBlob != NULL); 
  BS_ASSERT(pBlob->pPointmasses != NULL); 
  BS_ASSERT(pBlob->pMiddle != NULL); 
  
  bs_blob_compute_middle(pBlob, pBlob->pComputedMiddle);
  
  for(i = 0; i < pBlob->numPointmasses; i++)
  {
    bs_pointmass_move(pBlob->pPointmassesScAccess[i], dt); 
  }  
  
  bs_pointmass_move(pBlob->pMiddle, dt); 
}

void 
bs_blob_sc(bs_blob_t *pBlob, 
           bs_env_t *pEnv)
{
  int i, j;

  BS_ASSERT(pBlob != NULL); 
  BS_ASSERT(pBlob->pPointmasses != NULL); 
  BS_ASSERT(pBlob->pSticks != NULL); 
  BS_ASSERT(pBlob->pJoints != NULL); 

  for(j = 0; j < 10; j++)
  {
    bs_blob_pump(pBlob); 
    
    for(i = 0; i < pBlob->numJoints; i++)
    {
      bs_joint_sc(pBlob->pJointsScAccess[i]); 
    }
    for(i = 0; i < pBlob->numSticks; i++)
    {
      bs_stick_sc(pBlob->pSticksScAccess[i]); 
    }  

    for(i = 0; i < pBlob->numPointmasses; i++)
    {
      if(bs_env_collision(pEnv, pBlob->pComputedMiddle, 
        bs_pointmass_get_pos(pBlob->pPointmassesScAccess[i])) == TRUE)
      {
        bs_pointmass_set_friction(pBlob->pPointmassesScAccess[i], 0.5f); 
      }
      else 
      {
        bs_pointmass_set_friction(pBlob->pPointmassesScAccess[i], 0.01f);         
      }
    }  
  
    bs_blob_pump(pBlob); 
    
    for(i = 0; i < pBlob->numJoints; i++)
    {
      bs_joint_sc(pBlob->pJointsScAccess[i]); 
    }
    for(i = 0; i < pBlob->numSticks; i++)
    {
      bs_stick_sc(pBlob->pSticksScAccess[i]); 
    }  

    for(i = pBlob->numPointmasses - 1; i >= 0; i--)
    {
      if(bs_env_collision(pEnv, pBlob->pComputedMiddle, 
        bs_pointmass_get_pos(pBlob->pPointmassesScAccess[i])) == TRUE)
      {
        bs_pointmass_set_friction(pBlob->pPointmassesScAccess[i], 0.5f); 
      }
      else 
      {
        bs_pointmass_set_friction(pBlob->pPointmassesScAccess[i], 0.01f);         
      }
    }  
  }   
}

void 
bs_blob_set_force(bs_blob_t *pBlob, 
                  bs_vector_t *pForce)
{
  int i; 
  
  BS_ASSERT(pBlob != NULL); 
  BS_ASSERT(pBlob->pPointmasses != NULL); 
  BS_ASSERT(pBlob->pMiddle != NULL);  
  
  for(i = 0; i < pBlob->numPointmasses; i++)
  {
    bs_pointmass_set_force(pBlob->pPointmasses[i], pForce);  
  }    
  bs_pointmass_set_force(pBlob->pMiddle, pForce);  
}

void 
bs_blob_add_force(bs_blob_t *pBlob, 
                  bs_vector_t *pForce)
{
  int i; 

  BS_ASSERT(pBlob != NULL); 
  BS_ASSERT(pBlob->pPointmasses != NULL); 
  BS_ASSERT(pBlob->pMiddle != NULL);  
  
  for(i = 0; i < pBlob->numPointmasses; i++)
  {
    bs_pointmass_add_force(pBlob->pPointmasses[i], pForce);  
  }    
  bs_pointmass_add_force(pBlob->pMiddle, pForce);  
}

/* Compute the separation plane between two blobs, result is defined by 
 * pResultA and pResultB. This function is the reason why blobs interesct 
 * in 'strange' fashion. The separation plane tries to find the line at which 
 * two blobs start overlapping ewach other. This would work if the blobs where 
 * perfect circles, but they are blobs, thus deforming and the separation plane 
 * becomes a mere approximation. 
 */ 
static void 
bs_blob_find_separation_plane(bs_blob_t *pBlobA, 
                              bs_blob_t *pBlobB, 
                              bs_vector_t *pResultA, 
                              bs_vector_t *pResultB)
{
  bs_float_t tx, ty, px, py; 
  bs_float_t ax, ay; 
  bs_float_t k, kA, kB; 

  /* M_PI constant removed, kA and kB are only valid for k computation */ 
  /* FIXME, get rid of sqrt by using one step taylor or something like that */ 
  kA = sqrt(bs_blob_compute_volume(pBlobA)); 
  kB = sqrt(bs_blob_compute_volume(pBlobB)); 
  k = kA / (kA + kB); 
  
  ax = BS_VECTOR_GET_X(pBlobA->pComputedMiddle); 
  ay = BS_VECTOR_GET_Y(pBlobA->pComputedMiddle); 
  
  tx = BS_VECTOR_GET_X(pBlobB->pComputedMiddle) - ax; 
  ty = BS_VECTOR_GET_Y(pBlobB->pComputedMiddle) - ay; 
    
  px = - ty;   
  py = tx; 
  
  /* reuse tx, ty */
  tx = tx * k + ax;   
  ty = ty * k + ay;   

  /* reuse px, py */
  px = px * 0.5f; 
  py = py * 0.5f; 
  
  BS_VECTOR_SET_X(pResultA, tx + px);
  BS_VECTOR_SET_Y(pResultA, ty + py);

  BS_VECTOR_SET_X(pResultB, tx - px); 
  BS_VECTOR_SET_Y(pResultB, ty - py); 
}

/* Test is the line segment starting in pSegmentA ending in pSegmentB 
 * intersects with an infinite line defined by pLineA and pLineB. 
 * returns FALSE if they do not intersect, in that case 
 * the value of pResult is undefined. 
 */ 
static bs_bool_t
bs_blob_plane_intersect(bs_vector_t *pLineA, 
                        bs_vector_t *pLineB, 
                        bs_vector_t *pSegmentA, 
                        bs_vector_t *pSegmentB, 
                        bs_vector_t *pResult)
{
  bs_float_t s, k; 
  bs_float_t aX, aY, bX, bY, cX, cY, dX, dY; 
  
  aX = BS_VECTOR_GET_X(pLineA); 
  aY = BS_VECTOR_GET_Y(pLineA); 
  bX = BS_VECTOR_GET_X(pLineB); 
  bY = BS_VECTOR_GET_Y(pLineB); 
  cX = BS_VECTOR_GET_X(pSegmentA); 
  cY = BS_VECTOR_GET_Y(pSegmentA); 
  dX = BS_VECTOR_GET_X(pSegmentB); 
  dY = BS_VECTOR_GET_Y(pSegmentB);
  
  /* the lines are paralle if k is zero */ 
  k = ((bX-aX) * (dY-cY) - (bY-aY) * (dX-cX)); 
  if(k == 0.0f)
  {
    return FALSE; 
  }

  s = ((aY-cY) * (bX-aX) - (aX-cX) * (bY-aY)) / k; 
  if(s < 0.0f || s > 1.0f) 
  {
    return FALSE; 
  }

  BS_VECTOR_SET(pResult, pSegmentB); 
  BS_VECTOR_SUB(pResult, pSegmentA);
  BS_VECTOR_SCALE(pResult, s);
  BS_VECTOR_ADD(pResult, pSegmentA); 
  
  return TRUE; 
}

/* Test is two blobs can possibly intersect */ 
static bs_bool_t 
bs_blob_simple_distance_test(bs_blob_t *pBlobA, 
                             bs_blob_t *pBlobB)
{
  bs_float_t aRadius, bRadius, dist; 
  
  aRadius = bs_blob_get_radius(pBlobA); 
  bRadius = bs_blob_get_radius(pBlobB); 
  dist = BS_VECTOR_DIST(pBlobA->pComputedMiddle, pBlobB->pComputedMiddle); 
  if(dist > 1.5f * (aRadius + bRadius))
  {
    return FALSE;; 
  }
  
  return TRUE; 
}

void 
bs_blob_collision_test(bs_blob_t *pBlobA, 
                       bs_blob_t *pBlobB)
{
  bs_int32_t i; 
  
  if(bs_blob_simple_distance_test(pBlobA, pBlobB) == FALSE)
  {
    return; 
  }
  
  bs_blob_compute_middle(pBlobA, pBlobA->pComputedMiddle); 
  bs_blob_compute_middle(pBlobB, pBlobB->pComputedMiddle); 
  
  bs_blob_find_separation_plane(pBlobA, pBlobB, pBlobA->spA, pBlobA->spB);   
  
  for(i = 0; i < pBlobA->numPointmasses; i++)
  {
    bs_vector_t r; 
    bs_vector_t *pTmp; 
    
    pTmp = BS_POINTMASS_GET_CUR_POS(pBlobA->pPointmassesScAccess[i]); 
    if(bs_blob_plane_intersect(pBlobA->spA, pBlobA->spB, pBlobA->pComputedMiddle, pTmp, &r) == TRUE)
    {
      BS_VECTOR_SET(pTmp, &r); 
      bs_pointmass_set_friction(pBlobA->pPointmassesScAccess[i], 0.5f);        
    }      
  }
}

static void 
bs_blob_randomize_face_style(bs_blob_t *pBlob)
{
  BS_ASSERT(pBlob != NULL);

  if(pBlob->faceStyle == BS_BLOB_FACE_STYLE_MOUTH_CLOSED && bs_util_random_float() < 0.05f)
  {
    pBlob->faceStyle = BS_BLOB_FACE_STYLE_MOUTH_OPEN; 
  }
  else if(pBlob->faceStyle == BS_BLOB_FACE_STYLE_MOUTH_OPEN && bs_util_random_float() < 0.1f)
  {
    pBlob->faceStyle = BS_BLOB_FACE_STYLE_MOUTH_CLOSED; 
  }
  
  if(pBlob->eyeStyle == BS_BLOB_EYE_STYLE_OPEN && bs_util_random_float() < 0.025f)
  {
    pBlob->eyeStyle = BS_BLOB_EYE_STYLE_BLINK; 
  }
  else if(pBlob->eyeStyle == BS_BLOB_EYE_STYLE_BLINK && bs_util_random_float() < 0.3f)
  {
    pBlob->eyeStyle = BS_BLOB_EYE_STYLE_OPEN; 
  }
}

static void 
bs_blob_draw_mouth_closed(bs_blob_t *pBlob, 
                          cairo_t *pCairo, 
                          bs_float_t rsf)
{
  BS_ASSERT(pBlob != NULL);
  BS_ASSERT(pCairo != NULL);

  cairo_new_path(pCairo); 
  cairo_set_line_width(pCairo, 2.0 / rsf);   
  cairo_arc(pCairo, 0.0, 0.0, 0.25f, M_PI, FALSE); 
  cairo_stroke(pCairo); 
}

static void 
bs_blob_draw_mouth_open(bs_blob_t *pBlob, 
                        cairo_t *pCairo, 
                        bs_float_t rsf)
{
  BS_ASSERT(pBlob != NULL);
  BS_ASSERT(pCairo != NULL);

  cairo_new_path(pCairo); 
  cairo_set_line_width(pCairo, 2.0 / rsf);   
  cairo_arc(pCairo, 0.0, 0.0, 0.25f, M_PI, FALSE); 
  cairo_close_path(pCairo); 
  cairo_fill(pCairo); 
}

static void 
bs_blob_draw_eyes_open(bs_blob_t *pBlob, 
                       cairo_t *pCairo, 
                       bs_float_t rsf)
{
  BS_ASSERT(pBlob != NULL);
  BS_ASSERT(pCairo != NULL);

  cairo_set_line_width(pCairo, 1.0 / rsf);
  
  cairo_new_path(pCairo); 
  cairo_arc(pCairo, -0.15, 0.20, 0.12, 0.0, 2.0 * M_PI); 
  cairo_close_path(pCairo); 
  cairo_set_source_rgb(pCairo, 1.0, 1.0, 1.0);   
  cairo_fill_preserve(pCairo); 
  cairo_set_source_rgb(pCairo, 0.0, 0.0, 0.0);   
  cairo_stroke(pCairo); 
    
  cairo_new_path(pCairo); 
  cairo_arc(pCairo, 0.15, 0.20, 0.12, 0.0, 2.0 * M_PI); 
  cairo_close_path(pCairo); 
  cairo_set_source_rgb(pCairo, 1.0, 1.0, 1.0);   
  cairo_fill_preserve(pCairo); 
  cairo_set_source_rgb(pCairo, 0.0, 0.0, 0.0);   
  cairo_stroke(pCairo); 
  
  cairo_new_path(pCairo);
  cairo_arc(pCairo, -0.15, 0.17, 0.06, 0.0, 2.0 * M_PI); 
  cairo_close_path(pCairo);
  cairo_fill(pCairo);   

  cairo_new_path(pCairo);
  cairo_arc(pCairo, 0.15, 0.17, 0.06, 0.0, 2.0 * M_PI); 
  cairo_close_path(pCairo);
  cairo_fill(pCairo);   
}

static void 
bs_blob_draw_eyes_blink(bs_blob_t *pBlob, 
                        cairo_t *pCairo, 
                        bs_float_t rsf)
{
  BS_ASSERT(pBlob != NULL);
  BS_ASSERT(pCairo != NULL);

  cairo_set_line_width(pCairo, 1.0 / rsf);

  cairo_new_path(pCairo); 
  cairo_arc(pCairo, -0.15, 0.20, 0.12, 0.0, 2.0 * M_PI); 
  cairo_set_source_rgb(pCairo, 1.0, 1.0, 1.0);   
  cairo_fill_preserve(pCairo); 
  cairo_set_source_rgb(pCairo, 0.0, 0.0, 0.0);   
  cairo_stroke(pCairo); 
    
  cairo_new_path(pCairo); 
  cairo_arc(pCairo, 0.15, 0.20, 0.12, 0.0, 2.0 * M_PI); 
  cairo_close_path(pCairo);
  cairo_set_source_rgb(pCairo, 1.0, 1.0, 1.0);   
  cairo_fill_preserve(pCairo); 
  cairo_set_source_rgb(pCairo, 0.0, 0.0, 0.0);   
  cairo_stroke(pCairo); 

  cairo_new_path(pCairo); 
  cairo_move_to(pCairo, -0.25, 0.20);
  cairo_line_to(pCairo, -0.05, 0.20); 
  cairo_stroke(pCairo);  

  cairo_new_path(pCairo); 
  cairo_move_to(pCairo,  0.25, 0.20);
  cairo_line_to(pCairo,  0.05, 0.20); 
  cairo_stroke(pCairo);   
}

static void 
bs_blob_draw_ooh_face(bs_blob_t *pBlob, 
                      cairo_t *pCairo, 
                      bs_float_t rsf)
{
  BS_ASSERT(pBlob != NULL);
  BS_ASSERT(pCairo != NULL);
  
  bs_blob_draw_mouth_open(pBlob, pCairo, rsf); 
  
  cairo_set_line_width(pCairo, 2.0 / rsf); 
  
  cairo_new_path(pCairo); 
  cairo_move_to(pCairo, -0.25, 0.3); 
  cairo_line_to(pCairo, -0.05, 0.2); 
  cairo_line_to(pCairo, -0.25, 0.1); 
  cairo_stroke(pCairo); 

  cairo_new_path(pCairo); 
  cairo_move_to(pCairo,  0.25, 0.3); 
  cairo_line_to(pCairo,  0.05, 0.2); 
  cairo_line_to(pCairo,  0.25, 0.1); 
  cairo_stroke(pCairo); 
}

static void 
bs_blob_draw_face(bs_blob_t *pBlob, 
                  cairo_t *pCairo, 
                  bs_float_t scaleFactor)
{
  bs_float_t xOff, yOff; 
  bs_vector_t up, orientation; 
  bs_float_t angle; 
  bs_float_t rsf; 

  BS_ASSERT(pBlob != NULL);
  BS_ASSERT(pCairo != NULL);

  rsf = pBlob->radius * scaleFactor; 
  
  cairo_save(pCairo); 
  cairo_set_source_rgb(pCairo, 0.0, 0.0, 0.0); 
  
  BS_VECTOR_SET(&orientation, bs_pointmass_get_pos(pBlob->pPointmasses[0])); 
  BS_VECTOR_ADD(&orientation, bs_pointmass_get_pos(pBlob->pPointmasses[1])); 
  BS_VECTOR_SUB(&orientation, bs_pointmass_get_pos(pBlob->pPointmasses[2])); 
  BS_VECTOR_SUB(&orientation, bs_pointmass_get_pos(pBlob->pPointmasses[3])); 
  
  xOff = (bs_pointmass_get_x_pos(pBlob->pMiddle) + BS_VECTOR_GET_X(&orientation)) * scaleFactor; 
  yOff = (bs_pointmass_get_y_pos(pBlob->pMiddle) + BS_VECTOR_GET_Y(&orientation)) * scaleFactor;
  cairo_translate(pCairo, xOff, yOff); 
  
  bs_vector_init(&up, 0.0f, 1.0f); 
  angle = acos(bs_vector_dot_prod(&orientation, &up) / bs_vector_length(&orientation)); 
  if(bs_vector_get_x(&orientation) > 0.0f)
  {
    angle = -angle; 
  }
  cairo_rotate(pCairo, angle); 
 
  /* face offset relaitve middle position */
  /* face off is a crap movie btw */   
  cairo_scale(pCairo, rsf, rsf); 
  cairo_translate(pCairo, 0.0, -0.30); 

  bs_blob_randomize_face_style(pBlob); 
  
  if(bs_pointmass_get_square_vel(pBlob->pMiddle) > 0.004f)
  {
    bs_blob_draw_ooh_face(pBlob, pCairo, rsf); 
  }
  else 
  {
    if(pBlob->faceStyle == BS_BLOB_FACE_STYLE_MOUTH_CLOSED)
    {
      bs_blob_draw_mouth_closed(pBlob, pCairo, rsf); 
    }
    else if(pBlob->faceStyle == BS_BLOB_FACE_STYLE_MOUTH_OPEN)
    {
      bs_blob_draw_mouth_open(pBlob, pCairo, rsf);       
    }
    else 
    {
      BS_ASSERT(FALSE);   
    }
    
    if(pBlob->eyeStyle == BS_BLOB_EYE_STYLE_BLINK)
    {    
      bs_blob_draw_eyes_blink(pBlob, pCairo, rsf); 
    }
    else if(pBlob->eyeStyle == BS_BLOB_EYE_STYLE_OPEN)
    {
      bs_blob_draw_eyes_open(pBlob, pCairo, rsf);       
    }
    else 
    {
      BS_ASSERT(FALSE);   
    }
  }
  
  cairo_restore(pCairo); 
}

static void 
bs_blob_draw_simple_body(bs_blob_t *pBlob, 
                         cairo_t *pCairo, 
                         bs_float_t scaleFactor)
{
  bs_int32_t i; 
  
  BS_ASSERT(pBlob != NULL);
  BS_ASSERT(pCairo != NULL);
  
  cairo_new_path(pCairo); 

  cairo_move_to(pCairo, 
    (bs_pointmass_get_x_pos(pBlob->pPointmasses[0]) * scaleFactor), 
    (bs_pointmass_get_y_pos(pBlob->pPointmasses[0]) * scaleFactor));      

  for(i = 1; i < pBlob->numPointmasses; i++)
  {
    cairo_line_to(pCairo, 
      (bs_pointmass_get_x_pos(pBlob->pPointmasses[i]) * scaleFactor), 
      (bs_pointmass_get_y_pos(pBlob->pPointmasses[i]) * scaleFactor));      
  }
    
  cairo_close_path(pCairo); 

#ifdef BS_BLOB_DEBUG_FILL_BLOB
  cairo_set_source_rgb(pCairo, 1.0, 1.0, 1.0); 
  cairo_fill_preserve(pCairo); 
#endif /* BS_BLOB_DEBUG_FILL_BLOB */ 
  
  cairo_set_line_width(pCairo, 3.0);   
  cairo_set_source_rgb(pCairo, 0.0, 0.0, 0.0); 
  cairo_stroke(pCairo);   
}

static void 
bs_blob_debug_draw_joints(bs_blob_t *pBlob, 
                          cairo_t *pCairo, 
                          bs_float_t scaleFactor)
{
#ifdef BS_BLOB_DEBUG_DRAW_JOINTS

  bs_int32_t i; 

  BS_ASSERT(pBlob != NULL);
  BS_ASSERT(pBlob->pJoints != NULL);
  BS_ASSERT(pCairo != NULL);   

  for(i = 0; i < pBlob->numJoints; i++)
  {
    bs_joint_draw(pBlob->pJoints[i], pCairo, scaleFactor); 
  }
  
#else
  BS_UNUSED_PARAMETER(pBlob);
  BS_UNUSED_PARAMETER(pCairo); 
  BS_UNUSED_PARAMETER(scaleFactor);   
#endif /* BS_BLOB_DEBUG_DRAW_JOINTS */ 
}

static void 
bs_Blob_debug_draw_pointmasses(bs_blob_t *pBlob, 
                               cairo_t *pCairo, 
                               bs_float_t scaleFactor)
{
#ifdef BS_BLOB_DEBUG_DRAW_POINTMASSES  
  
  bs_int32_t i; 
  
  BS_ASSERT(pBlob != NULL);
  BS_ASSERT(pBlob->pPointmasses != NULL);
  BS_ASSERT(pCairo != NULL);   

  for(i = 0; i < pBlob->numPointmasses; i++)
  {
    bs_pointmass_draw(pBlob->pPointmasses[i], pCairo, scaleFactor); 
    bs_pointmass_set_rgb(pBlob->pPointmasses[i], 0.2f, 1.0f, 0.0f);
  }
  
#else
  BS_UNUSED_PARAMETER(pBlob);
  BS_UNUSED_PARAMETER(pCairo); 
  BS_UNUSED_PARAMETER(scaleFactor);   
#endif /* BS_BLOB_DEBUG_DRAW_POINTMASSES */    
}

static void 
bs_blob_debug_draw_separation_plane(bs_blob_t *pBlob, 
                                    cairo_t *pCairo, 
                                    bs_float_t scaleFactor)
{
#ifdef BS_BLOB_DEBUG_DRAW_SEPATATION_PLANE
  
  cairo_new_path(pCairo); 
  cairo_move_to(pCairo, BS_VECTOR_GET_X(pBlob->spA) * scaleFactor, 
    BS_VECTOR_GET_Y(pBlob->spA) * scaleFactor);  
  cairo_line_to(pCairo, BS_VECTOR_GET_X(pBlob->spB) * scaleFactor, 
    BS_VECTOR_GET_Y(pBlob->spB) * scaleFactor);  
  cairo_close_path(pCairo); 
    
  cairo_set_line_width(pCairo, 1.0);   
  cairo_set_source_rgb(pCairo, 0.0, 0.8, 0.8); 
  cairo_stroke(pCairo);   
  
#else
  BS_UNUSED_PARAMETER(pBlob);
  BS_UNUSED_PARAMETER(pCairo); 
  BS_UNUSED_PARAMETER(scaleFactor);   
#endif /* BS_BLOB_DEBUG_DRAW_SEPATATION_PLANE */   
}

void 
bs_blob_draw(bs_blob_t *pBlob, 
             cairo_t *pCairo, 
             bs_float_t scaleFactor)
{
  BS_ASSERT(pBlob != NULL);
  BS_ASSERT(pCairo != NULL);
  
  bs_blob_draw_simple_body(pBlob, pCairo, scaleFactor); 

  bs_blob_debug_draw_joints(pBlob, pCairo, scaleFactor); 
  bs_Blob_debug_draw_pointmasses(pBlob, pCairo, scaleFactor); 
  bs_blob_debug_draw_separation_plane(pBlob, pCairo, scaleFactor); 
  
  bs_blob_draw_face(pBlob, pCairo, scaleFactor); 

  if (cairo_status(pCairo)) 
  {
    printf("Cairo is unhappy: %s\n",
      cairo_status_to_string (cairo_status (pCairo)));
    exit(0);
  }
}
