#ifndef __BS_POINTMASS_H__
#define __BS_POINTMASS_H__

#include "bs_types.h"
#include "bs_vector.h"

#include <cairo.h>

typedef struct bs_pointmass_st
{
  bs_vector_t *pCurPos; 
  bs_vector_t *pPrevPos; 
  bs_vector_t *pForce; 
  bs_float_t mass;   
  bs_float_t friction; 
  bs_float_t r, g, b; 
} bs_pointmass_t; 

#define BS_POINTMASS_GET_X_POS(PM) \
  BS_VECTOR_GET_X((PM)->pCurPos)

#define BS_POINTMASS_GET_Y_POS(PM) \
  BS_VECTOR_GET_Y((PM)->pCurPos)

#define BS_POINTMASS_GET_CUR_POS(PM) \
  ((PM)->pCurPos)

#define BS_POINTMASS_GET_PREV_POS(PM) \
  ((PM)->pPrevPos)

bs_pointmass_t* 
bs_pointmass_create(bs_float_t xPos, 
                    bs_float_t yPos, 
                    bs_float_t mass);

void 
bs_pointmass_destroy(bs_pointmass_t *pPointmass); 

bs_float_t 
bs_pointmass_get_x_pos(bs_pointmass_t *pPointmass); 

bs_float_t 
bs_pointmass_get_y_pos(bs_pointmass_t *pPointmass); 

bs_vector_t* 
bs_pointmass_get_pos(bs_pointmass_t *pPointmass); 

bs_float_t 
bs_pointmass_get_x_prev_pos(bs_pointmass_t *pPointmass); 

bs_float_t 
bs_pointmass_get_y_prev_pos(bs_pointmass_t *pPointmass); 

bs_vector_t* 
bs_pointmass_get_prev_pos(bs_pointmass_t *pPointmass); 

void 
bs_pointmass_set_x_pos(bs_pointmass_t *pPointmass, 
                       bs_float_t x); 

void 
bs_pointmass_set_y_pos(bs_pointmass_t *pPointmass, 
                       bs_float_t y); 

void 
bs_pointmass_set_pos(bs_pointmass_t *pPointmass, 
                     bs_vector_t *pPos); 

void 
bs_pointmass_add_x_pos(bs_pointmass_t *pPointmass, 
                       bs_float_t x); 

void 
bs_pointmass_add_y_pos(bs_pointmass_t *pPointmass, 
                       bs_float_t y); 

void 
bs_pointmass_add_pos(bs_pointmass_t *pPointmass, 
                     bs_vector_t *pPos); 

void 
bs_pointmass_set_x_force(bs_pointmass_t *pPointmass, 
                         bs_float_t x); 

void 
bs_pointmass_set_y_force(bs_pointmass_t *pPointmass, 
                         bs_float_t y); 

void 
bs_pointmass_set_force(bs_pointmass_t *pPointmass, 
                       bs_vector_t *pForce); 

void 
bs_pointmass_add_x_force(bs_pointmass_t *pPointmass, 
                         bs_float_t x); 

void 
bs_pointmass_add_y_force(bs_pointmass_t *pPointmass, 
                         bs_float_t y); 

void 
bs_pointmass_add_force(bs_pointmass_t *pPointmass, 
                       bs_vector_t *pForce); 

bs_float_t
bs_pointmass_get_mass(bs_pointmass_t *pPointmass);

void 
bs_pointmass_set_mass(bs_pointmass_t *pPointmass, 
                      bs_float_t mass);

void 
bs_pointmass_set_friction(bs_pointmass_t *pPointmass, 
                          bs_float_t friction); 

void 
bs_pointmass_move(bs_pointmass_t *pPointmass,
                  bs_float_t dt); 

bs_float_t 
bs_pointmass_get_square_vel(bs_pointmass_t *pPointmass); 

bs_float_t 
bs_pointmass_get_dist(bs_pointmass_t *pPointmassA, 
                      bs_pointmass_t *pPointmassB); 

void 
bs_pointmass_draw(bs_pointmass_t *pPointmass, 
                  cairo_t *pCairo, 
                  bs_float_t scaleFactor);

void 
bs_pointmass_set_rgb(bs_pointmass_t *pPointmass, 
                     bs_float_t r, 
                     bs_float_t g, 
                     bs_float_t b); 

#endif
