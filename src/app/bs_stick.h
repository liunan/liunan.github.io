#ifndef __BS_STICK_H__
#define __BS_STICK_H__

#include "bs_types.h"
#include "bs_pointmass.h"
#include "bs_vector.h"

typedef struct bs_stick_st
{
  bs_float_t length, lengthSquared; 
  bs_vector_t *pAPos, *pBPos; 
} bs_stick_t;


#define BS_STICK_GET_A_POS(S) \
  (S)->pAPos

#define BS_STICK_GET_B_POS(S) \
  (S)->pBPos

bs_stick_t* 
bs_stick_create(bs_pointmass_t *pPointmassA, 
                bs_pointmass_t *pPointmassB); 

void 
bs_stick_destroy(bs_stick_t *pStick); 

void 
bs_stick_scale(bs_stick_t *pStick, 
               bs_float_t scaleFactor); 

void 
bs_stick_set_length(bs_stick_t *pStick, 
                    bs_float_t length); 

void 
bs_stick_sc(bs_stick_t *pStick); 

#endif
