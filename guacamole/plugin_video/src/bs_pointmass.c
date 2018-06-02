#include <math.h>

#include "bs_pointmass.h"
#include "bs_mem.h"
#include "bs_assert.h"

bs_pointmass_t* 
bs_pointmass_create(bs_float_t xPos, 
                    bs_float_t yPos, 
                    bs_float_t mass)
{
  bs_pointmass_t *pPointmass; 

  BS_MEM_ALLOC_TYPE(pPointmass, bs_pointmass_t, 1); 

  pPointmass->pCurPos = bs_vector_create(xPos, yPos);   
  pPointmass->pPrevPos = bs_vector_create(xPos, yPos);   
  pPointmass->pForce = bs_vector_create(0.0f, 0.0f); 
  pPointmass->mass = mass; 
  pPointmass->friction = 0.99; 
  pPointmass->r = 1.0f; 
  pPointmass->g = 0.0f; 
  pPointmass->b = 0.0f; 

  return pPointmass;   
}

void 
bs_pointmass_destroy(bs_pointmass_t *pPointmass)
{
  BS_ASSERT(pPointmass != NULL);
  BS_MEM_FREE(pPointmass);   
}

bs_float_t 
bs_pointmass_get_x_pos(bs_pointmass_t *pPointmass)
{
  BS_ASSERT(pPointmass != NULL);   
  return BS_VECTOR_GET_X(pPointmass->pCurPos); 
}

bs_float_t 
bs_pointmass_get_y_pos(bs_pointmass_t *pPointmass) 
{
  BS_ASSERT(pPointmass != NULL);   
  return BS_VECTOR_GET_Y(pPointmass->pCurPos); 
}

bs_vector_t* 
bs_pointmass_get_pos(bs_pointmass_t *pPointmass)
{
  BS_ASSERT(pPointmass != NULL);   
  return pPointmass->pCurPos; 
}

bs_float_t 
bs_pointmass_get_x_prev_pos(bs_pointmass_t *pPointmass) 
{
  BS_ASSERT(pPointmass != NULL);   
  return BS_VECTOR_GET_X(pPointmass->pPrevPos); 
}

bs_float_t 
bs_pointmass_get_y_prev_pos(bs_pointmass_t *pPointmass)
{
  BS_ASSERT(pPointmass != NULL);   
  return BS_VECTOR_GET_Y(pPointmass->pPrevPos); 
}

bs_vector_t* 
bs_pointmass_get_prev_pos(bs_pointmass_t *pPointmass)
{
  BS_ASSERT(pPointmass != NULL);   
  return pPointmass->pPrevPos; 
}

void 
bs_pointmass_set_x_pos(bs_pointmass_t *pPointmass, 
                       bs_float_t x)
{
  BS_ASSERT(pPointmass != NULL);   
  BS_VECTOR_SET_X(pPointmass->pCurPos, x); 
}

void 
bs_pointmass_set_y_pos(bs_pointmass_t *pPointmass, 
                       bs_float_t y)
{
  BS_ASSERT(pPointmass != NULL);     
  BS_VECTOR_SET_Y(pPointmass->pCurPos, y); 
}

void 
bs_pointmass_set_pos(bs_pointmass_t *pPointmass, 
                     bs_vector_t *pPos)
{
  BS_ASSERT(pPointmass != NULL);   
  BS_VECTOR_SET(pPointmass->pCurPos, pPos); 
}

void 
bs_pointmass_add_x_pos(bs_pointmass_t *pPointmass, 
                       bs_float_t x)
{
  BS_ASSERT(pPointmass != NULL);   
  BS_VECTOR_ADD_X(pPointmass->pCurPos, x); 
}

void 
bs_pointmass_add_y_pos(bs_pointmass_t *pPointmass, 
                       bs_float_t y)
{
  BS_ASSERT(pPointmass != NULL);   
  BS_VECTOR_ADD_Y(pPointmass->pCurPos, y); 
}

void 
bs_pointmass_add_pos(bs_pointmass_t *pPointmass, 
                     bs_vector_t *pPos) 
{
  BS_ASSERT(pPointmass != NULL);   
  BS_VECTOR_ADD(pPointmass->pCurPos, pPos); 
}

void 
bs_pointmass_set_x_force(bs_pointmass_t *pPointmass, 
                         bs_float_t x) 
{
  BS_ASSERT(pPointmass != NULL);   
  BS_VECTOR_SET_X(pPointmass->pForce, x); 
}

void 
bs_pointmass_set_y_force(bs_pointmass_t *pPointmass, 
                         bs_float_t y) 
{
  BS_ASSERT(pPointmass != NULL);   
  BS_VECTOR_SET_Y(pPointmass->pForce, y); 
}

void 
bs_pointmass_set_force(bs_pointmass_t *pPointmass, 
                       bs_vector_t *pForce) 
{
  BS_ASSERT(pPointmass != NULL);   
  BS_VECTOR_SET(pPointmass->pForce, pForce); 
}

void 
bs_pointmass_add_x_force(bs_pointmass_t *pPointmass, 
                         bs_float_t x) 
{
  BS_ASSERT(pPointmass != NULL);   
  BS_VECTOR_ADD_X(pPointmass->pForce, x); 
}

void 
bs_pointmass_add_y_force(bs_pointmass_t *pPointmass, 
                         bs_float_t y) 
{
  BS_ASSERT(pPointmass != NULL);   
  BS_VECTOR_ADD_Y(pPointmass->pForce, y); 
}

void 
bs_pointmass_add_force(bs_pointmass_t *pPointmass, 
                       bs_vector_t *pForce) 
{
  BS_ASSERT(pPointmass != NULL);   
  BS_VECTOR_ADD(pPointmass->pForce, pForce); 
}

bs_float_t
bs_pointmass_get_mass(bs_pointmass_t *pPointmass)
{
  BS_ASSERT(pPointmass != NULL);   
  return pPointmass->mass; 
}

void 
bs_pointmass_set_mass(bs_pointmass_t *pPointmass, 
                      bs_float_t mass)
{
  BS_ASSERT(pPointmass != NULL); 
  pPointmass->mass = mass;   
}

void 
bs_pointmass_set_friction(bs_pointmass_t *pPointmass, 
                          bs_float_t friction)
{
  BS_ASSERT(pPointmass != NULL);
  pPointmass->friction = 1.0f - friction;   
}

void 
bs_pointmass_move(bs_pointmass_t *pPointmass, 
                  bs_float_t dt)
{
  bs_float_t t, a, c, dtdt, f1, f2; 
  
  BS_ASSERT(pPointmass != NULL);   
  
  dtdt = dt * dt; 
  f1 = pPointmass->friction; 
  f2 = 1.0f + f1; 
  
  a = BS_VECTOR_GET_X(pPointmass->pForce) / pPointmass->mass; 
  c = BS_VECTOR_GET_X(pPointmass->pCurPos); 
  t = f2 * c - f1 * BS_VECTOR_GET_X(pPointmass->pPrevPos) + a * dtdt; 
  BS_VECTOR_SET_X(pPointmass->pPrevPos, c); 
  BS_VECTOR_SET_X(pPointmass->pCurPos, t); 

  a = BS_VECTOR_GET_Y(pPointmass->pForce) / pPointmass->mass; 
  c = BS_VECTOR_GET_Y(pPointmass->pCurPos); 
  t = f2 * c - f1 * BS_VECTOR_GET_Y(pPointmass->pPrevPos) + a * dtdt; 
  BS_VECTOR_SET_Y(pPointmass->pPrevPos, c); 
  BS_VECTOR_SET_Y(pPointmass->pCurPos, t);   
}

bs_float_t 
bs_pointmass_get_square_vel(bs_pointmass_t *pPointmass)
{
  bs_float_t cXpX, cYpY; 
  
  BS_ASSERT(pPointmass != NULL);

  cXpX = BS_VECTOR_GET_X(pPointmass->pCurPos) - BS_VECTOR_GET_X(pPointmass->pPrevPos);   
  cYpY = BS_VECTOR_GET_Y(pPointmass->pCurPos) - BS_VECTOR_GET_Y(pPointmass->pPrevPos);   

  return cXpX * cXpX + cYpY * cYpY;   
}

bs_float_t 
bs_pointmass_get_dist(bs_pointmass_t *pPointmassA, 
                      bs_pointmass_t *pPointmassB)
{
  BS_ASSERT(pPointmassA != NULL);
  BS_ASSERT(pPointmassB != NULL);   

  return BS_VECTOR_DIST(pPointmassA->pCurPos, pPointmassB->pCurPos);   
}

void 
bs_pointmass_draw(bs_pointmass_t *pPointmass, 
                  cairo_t *pCairo, 
                  bs_float_t scaleFactor)
{
  BS_ASSERT(pPointmass != NULL); 
  BS_ASSERT(pCairo != NULL);

  cairo_set_source_rgb(pCairo, pPointmass->r, pPointmass->g, pPointmass->b); 
  cairo_set_line_width(pCairo, 1.0f); 
  
  cairo_new_path(pCairo); 
  cairo_arc(pCairo, BS_VECTOR_GET_X(pPointmass->pCurPos) * scaleFactor, 
    BS_VECTOR_GET_Y(pPointmass->pCurPos) * scaleFactor, 4.0f, 0.0f, M_PI * 2.0f); 
  cairo_fill(pCairo); 
}

void 
bs_pointmass_set_rgb(bs_pointmass_t *pPointmass, 
                     bs_float_t r, 
                     bs_float_t g, 
                     bs_float_t b)
{
  BS_ASSERT(pPointmass != NULL);

  pPointmass->r = r;   
  pPointmass->g = g;   
  pPointmass->b = b;   
}
