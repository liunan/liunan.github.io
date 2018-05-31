#include "bs_rubberband.h"
#include "bs_mem.h"
#include "bs_assert.h"

#include <math.h>

bs_rubberband_t*
bs_rubberband_create(bs_float_t xPos, 
                     bs_float_t yPos)
{
  bs_rubberband_t *pRubberband; 
  bs_float_t radius = 0.65f; 
  bs_int32_t i, j, p; 
  
  BS_MEM_ALLOC_TYPE(pRubberband, bs_rubberband_t, 1); 
  BS_MEM_ALLOC_TYPE(pRubberband->pPointmasses, bs_pointmass_t*, 40);
  BS_MEM_ALLOC_TYPE(pRubberband->pSticks, bs_stick_t*, 40); 
  BS_MEM_ALLOC_TYPE(pRubberband->pBallToBandJoint, bs_joint_t*, 120); 
  
  pRubberband->numPointmasses = 40; 
  pRubberband->numSticks = 40; 

  for(i = 0; i < 40; i++)
  {
    bs_float_t t; 
    
    t = (bs_float_t) i / 40.0 * M_PI * 2.0f; 

    pRubberband->pPointmasses[i] = bs_pointmass_create(xPos + cos(t) * radius, yPos + sin(t) * radius, 1.0f);
  }
  for(j = 39, i = 0; i < 40; j = i, i++)
  {
    pRubberband->pSticks[i] = bs_stick_create(pRubberband->pPointmasses[j], pRubberband->pPointmasses[i]); 
  }
  
  BS_MEM_ALLOC_TYPE(pRubberband->pBalls, bs_pointmass_t*, 3);   

  pRubberband->pBalls[0] = bs_pointmass_create(xPos + 0.3f, yPos - 0.1f, 1.0f); 
  pRubberband->pBalls[1] = bs_pointmass_create(xPos - 0.3f, yPos - 0.1f, 1.0f);
  pRubberband->pBalls[2] = bs_pointmass_create(xPos, yPos + 0.3f, 1.0f);   
  
  BS_MEM_ALLOC_TYPE(pRubberband->pBallJoints, bs_joint_t*, 3); 
  
  pRubberband->pBallJoints[0] = bs_joint_create(pRubberband->pBalls[0], pRubberband->pBalls[1], 0.8f, 3.0f); 
  pRubberband->pBallJoints[1] = bs_joint_create(pRubberband->pBalls[1], pRubberband->pBalls[2], 0.8f, 3.0f); 
  pRubberband->pBallJoints[2] = bs_joint_create(pRubberband->pBalls[2], pRubberband->pBalls[0], 0.8f, 3.0f); 
  
  return pRubberband;   
}

#define RADIUS 0.4f

void 
bs_rubberband_sc(bs_rubberband_t *pRubberband, 
                 bs_env_t *pEnv)
{
  bs_int32_t i, j, k; 

  for(i = 0; i < 20; i++)
  {
    for(j = 0; j < 40; j++)
    {
      if(bs_env_collision(pEnv, NULL, 
        bs_pointmass_get_pos(pRubberband->pPointmasses[j])) == TRUE)
      {
        bs_pointmass_set_friction(pRubberband->pPointmasses[j], 0.5f); 
      }
      else 
      {
        bs_pointmass_set_friction(pRubberband->pPointmasses[j], 0.01f);         
      }
    }  

    bs_env_collision(pEnv, NULL, bs_pointmass_get_pos(pRubberband->pBalls[0])); 
    bs_env_collision(pEnv, NULL, bs_pointmass_get_pos(pRubberband->pBalls[1])); 
    bs_env_collision(pEnv, NULL, bs_pointmass_get_pos(pRubberband->pBalls[2])); 
    
    bs_joint_sc(pRubberband->pBallJoints[0]); 
    bs_joint_sc(pRubberband->pBallJoints[1]); 
    bs_joint_sc(pRubberband->pBallJoints[2]); 
        
    for(j = 0; j < 40; j++)
    {
      bs_stick_sc(pRubberband->pSticks[j]); 
    }    
        
    for(j = 0; j < 40; j++)
    {
      bs_float_t dist, iDist, length; 
      bs_vector_t *pCurPos, *pBallPos; 
      bs_vector_t t;         
      
      pCurPos = BS_POINTMASS_GET_CUR_POS(pRubberband->pPointmasses[j]); 
      
      pBallPos = BS_POINTMASS_GET_CUR_POS(pRubberband->pBalls[0]); 
      dist = BS_VECTOR_DIST(pCurPos, pBallPos); 
      if(dist < RADIUS)
      {
        BS_VECTOR_SET(&t, pCurPos); 
        BS_VECTOR_SUB(&t, pBallPos); 
        length = BS_VECTOR_LENGHT(&t); 
        iDist = RADIUS - dist; 
        BS_VECTOR_SCALE(&t, 0.5f * iDist/length); 
        BS_VECTOR_ADD(pCurPos, &t); 
        BS_VECTOR_SUB(pBallPos, &t); 
      }
      pBallPos = BS_POINTMASS_GET_CUR_POS(pRubberband->pBalls[1]); 
      dist = BS_VECTOR_DIST(pCurPos, pBallPos); 
      if(dist < RADIUS)
      {
        BS_VECTOR_SET(&t, pCurPos); 
        BS_VECTOR_SUB(&t, pBallPos); 
        length = BS_VECTOR_LENGHT(&t); 
        iDist = RADIUS - dist; 
        BS_VECTOR_SCALE(&t, 0.5f * iDist/length); 
        BS_VECTOR_ADD(pCurPos, &t); 
        BS_VECTOR_SUB(pBallPos, &t); 
      }
      pBallPos = BS_POINTMASS_GET_CUR_POS(pRubberband->pBalls[2]); 
      dist = BS_VECTOR_DIST(pCurPos, pBallPos); 
      if(dist < RADIUS)
      {
        BS_VECTOR_SET(&t, pCurPos); 
        BS_VECTOR_SUB(&t, pBallPos); 
        length = BS_VECTOR_LENGHT(&t); 
        iDist = RADIUS - dist; 
        BS_VECTOR_SCALE(&t, 0.5f * iDist/length); 
        BS_VECTOR_ADD(pCurPos, &t); 
        BS_VECTOR_SUB(pBallPos, &t); 
      }
    }    
  }    
}

void 
bs_rubberband_move(bs_rubberband_t *pRubberband, 
                   bs_float_t dt)
{
  bs_int32_t i; 

  for(i = 0; i < 40; i++)
  {
    bs_pointmass_move(pRubberband->pPointmasses[i], dt); 
  }    
  bs_pointmass_move(pRubberband->pBalls[0], dt); 
  bs_pointmass_move(pRubberband->pBalls[1], dt); 
  bs_pointmass_move(pRubberband->pBalls[2], dt); 
}

void 
bs_rubberband_set_force(bs_rubberband_t *pRubberband, 
                        bs_vector_t *pForce)
{
  bs_int32_t i; 

  for(i = 0; i < 40; i++)
  {
    bs_pointmass_set_force(pRubberband->pPointmasses[i], pForce); 
  }    
  bs_pointmass_set_force(pRubberband->pBalls[0], pForce); 
  bs_pointmass_set_force(pRubberband->pBalls[1], pForce); 
  bs_pointmass_set_force(pRubberband->pBalls[2], pForce); 
}

void 
bs_rubberband_add_force(bs_rubberband_t *pRubberband, 
                        bs_vector_t *pForce)
{
  bs_int32_t i; 
  static int whichBall = 0; 
  
  for(i = 0; i < 40; i++)
  {
    bs_pointmass_add_force(pRubberband->pPointmasses[i], pForce); 
  }    
  bs_pointmass_add_force(pRubberband->pBalls[whichBall], pForce); 
  whichBall = random() % 3; 
}

void 
bs_rubberband_draw(bs_rubberband_t *pRubberband, 
                   cairo_t *pCairo, 
                   bs_float_t scaleFactor)
{
  bs_int32_t i; 

  cairo_new_path(pCairo); 

  cairo_move_to(pCairo, 
    (bs_pointmass_get_x_pos(pRubberband->pPointmasses[0]) * scaleFactor), 
    (bs_pointmass_get_y_pos(pRubberband->pPointmasses[0]) * scaleFactor));      

  for(i = 1; i < 40; i++)
  {
    cairo_line_to(pCairo, 
      (bs_pointmass_get_x_pos(pRubberband->pPointmasses[i]) * scaleFactor), 
      (bs_pointmass_get_y_pos(pRubberband->pPointmasses[i]) * scaleFactor));      
  }
    
  cairo_close_path(pCairo); 

  cairo_set_line_width(pCairo, 3.0);   
  cairo_set_source_rgb(pCairo, 0.0, 0.0, 0.0); 
  cairo_stroke(pCairo);     

  bs_pointmass_draw(pRubberband->pBalls[0], pCairo, scaleFactor);   
  bs_pointmass_draw(pRubberband->pBalls[1], pCairo, scaleFactor);   
  bs_pointmass_draw(pRubberband->pBalls[2], pCairo, scaleFactor);   
}
