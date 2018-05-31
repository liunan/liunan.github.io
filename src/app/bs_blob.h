#ifndef __BS_BLOB_H__
#define __BS_BLOB_H__

#include "bs_types.h"
#include "bs_stick.h"
#include "bs_joint.h"
#include "bs_pointmass.h"
#include "bs_env.h"

#include <cairo.h>

#define BS_BLOB_DEBUG_FILL_BLOB 
//#define BS_BLOB_DEBUG_DRAW_SEPATATION_PLANE
//#define BS_BLOB_DEBUG_DRAW_JOINTS 
// #define BS_BLOB_DEBUG_DRAW_POINTMASSES

typedef enum bs_blob_face_style_e
{
  BS_BLOB_FACE_STYLE_MOUTH_OPEN, 
  BS_BLOB_FACE_STYLE_MOUTH_CLOSED 
} bs_blob_face_style_t; 

typedef enum bs_blob_eye_style_e
{
  BS_BLOB_EYE_STYLE_OPEN, 
  BS_BLOB_EYE_STYLE_BLINK 
} bs_blob_eye_style_t; 

/* The satisfy constraint algorithm use in this program is similar to stochstic */
/* gradient decent approaches in the sense that is will not collect the error */
/* vector. It will apply deltas to every pointmass and joint when evaluated. */
/* This means that the previously evaluated pointmasses will most likley */
/* influence the one that is currently under evaluation. Now if you evaluate */
/* every joint and pointmass in some particular order we might get some */
/* culminative added effect which in this case makes the blob 'roll' */
/* To solve this I use pJointsScAccess and pPointmassesScAccess which maps */
/* elements in pJoints and pPointmasses in a random order, which later is */
/* used when running bs_blob_sc. */ 

typedef struct bs_blob_st 
{
  bs_pointmass_t *pMiddle; 
  bs_stick_t **pSticks; 
  bs_stick_t **pSticksScAccess;
  bs_joint_t **pJoints; 
  bs_joint_t **pJointsScAccess; 
  bs_pointmass_t **pPointmasses; 
  bs_pointmass_t **pPointmassesScAccess; 
  bs_int32_t numPointmasses; 
  bs_int32_t numSticks; 
  bs_int32_t numJoints; 
  bs_float_t longConst, shortConst; 
  bs_float_t radius; 
  bs_vector_t *pComputedMiddle; 
  bs_blob_face_style_t faceStyle; 
  bs_blob_eye_style_t eyeStyle; 
  
  /* used for debugging, separation plane */ 
  bs_vector_t *spA, *spB; 
} bs_blob_t; 

bs_blob_t* 
bs_blob_create(bs_float_t xPos, 
               bs_float_t yPos, 
               bs_float_t radius); 

void 
bs_blob_destroy(bs_blob_t *pBlob); 

bs_float_t 
bs_blob_get_radius(bs_blob_t *pBlob); 

void
bs_blob_scale(bs_blob_t *pBlob, 
              bs_float_t scaleFactor); 

bs_pointmass_t* 
bs_blob_get_middle(bs_blob_t *pBlob); 

bs_float_t 
bs_blob_get_x_pos(bs_blob_t *pBlob); 

bs_float_t 
bs_blob_get_y_pos(bs_blob_t *pBlob); 

void 
bs_blob_move(bs_blob_t *pBlob, 
             bs_float_t dt); 

void 
bs_blob_sc(bs_blob_t *pBlob, 
           bs_env_t *pEnv); 

void 
bs_blob_set_force(bs_blob_t *pBlob, 
                  bs_vector_t *pForce); 

void 
bs_blob_add_force(bs_blob_t *pBlob, 
                  bs_vector_t *pForce); 

void 
bs_blob_collision_test(bs_blob_t *pA, 
                       bs_blob_t *pB);

void 
bs_blob_collision_test_(bs_blob_t *pA, 
                       bs_blob_t *pB);

void 
bs_blob_draw(bs_blob_t *pBlob, 
             cairo_t *pCairo, 
             bs_float_t scaleFactor); 
                                       
#endif
