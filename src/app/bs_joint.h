#ifndef __BS_JOINT_H__
#define __BS_JOINT_H__

#include "bs_vector.h"
#include "bs_pointmass.h"
#include "bs_types.h"

#include <cairo.h>

#define BS_JOINT_NO_CONST (-1.0f)

typedef struct bs_joint_st 
{
  bs_bool_t aIsMovable, bIsMovable; 
  bs_bool_t shortIsUsed, longIsUsed; 
  bs_vector_t *pAPos, *pBPos;
  bs_float_t pointDist; 
  bs_float_t shortConstSquared;   
  bs_float_t longConstSquared;   
} bs_joint_t; 

bs_joint_t* 
bs_joint_create(bs_pointmass_t *pPointmassA, 
                bs_pointmass_t *pPointmassB, 
                bs_float_t shortConst, 
                bs_float_t longConst); 

void 
bs_joint_destroy(bs_joint_t *pJoint); 

void 
bs_joint_set_consts(bs_joint_t *pJoint, 
                    bs_float_t shortConst, 
                    bs_float_t longConst); 

void 
bs_joint_set_short_const(bs_joint_t *pJoint, 
                         bs_float_t shortConst); 

void 
bs_joint_set_long_const(bs_joint_t *pJoint, 
                        bs_float_t longConst); 

void 
bs_joint_scale(bs_joint_t *pJoint, 
               bs_float_t scaleFactor); 

void 
bs_joint_sc(bs_joint_t *pJoint); 

void 
bs_joint_draw(bs_joint_t *pJoint, 
              cairo_t *pCairo, 
              bs_float_t scaleFactor);

#endif
