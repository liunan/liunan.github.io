#ifndef __BS_RUBBERBAND_H__
#define __BS_RUBBERBAND_H__

#include "bs_types.h"
#include "bs_pointmass.h"
#include "bs_stick.h"
#include "bs_joint.h"
#include "bs_vector.h"
#include "bs_env.h"

#include <cairo.h>

typedef struct bs_rubberband_st
{
  bs_pointmass_t **pPointmasses;
  bs_stick_t **pSticks; 
  bs_int32_t numPointmasses; 
  bs_int32_t numSticks; 
  bs_pointmass_t **pBalls; 
  bs_int32_t numBalls; 
  bs_joint_t **pBallJoints; 
  bs_joint_t **pBallToBandJoint; 
  bs_env_t *pBallEnv; 
} bs_rubberband_t;

bs_rubberband_t*
bs_rubberband_create(bs_float_t xPos, 
                     bs_float_t yPos); 

void 
bs_rubberband_move(bs_rubberband_t *pRubberband, 
                   bs_float_t dt); 

void 
bs_rubberband_sc(bs_rubberband_t *pRubberband, 
                 bs_env_t *pEnv); 

void 
bs_rubberband_set_force(bs_rubberband_t *pRubberband, 
                        bs_vector_t *pForce); 

void 
bs_rubberband_add_force(bs_rubberband_t *pRubberband, 
                        bs_vector_t *pForce);

void 
bs_rubberband_draw(bs_rubberband_t *pRubberband, 
                   cairo_t *pCairo, 
                   bs_float_t scaleFactor); 


#endif
