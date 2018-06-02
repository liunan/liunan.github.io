#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <SDL.h>

#include "bs_cairo.h"
#include "bs_blob_collective.h"
#include "bs_assert.h"
#include "bs_mem.h"
#include "bs_list.h"
#include "bs_profiler.h"
#include "bs_rubberband.h"

typedef struct bs_main_data_st
{
  bs_cairo_sdl_t *pCairoSdl;
  bs_blob_collective_t *pCollective;
  bs_rubberband_t *pRubberband;
  bs_list_factory_t *pListFactory;
  bs_env_t *pEnv;
  bs_vector_t *pGravity;
  double width, height;
  bs_uint32_t newTimerInterval;
  bs_float_t fps;
  bs_int32_t lastFrameTimeStamp;
  bs_bool_t running;
} bs_main_data_t;


/**
 * fill the whole screen with color(1.0,1.0,1.0) white
 * */
static void
bs_main_clear(cairo_t *pCairo,
              double width,
              double height)
{
  cairo_set_source_rgb(pCairo, 1.0, 1.0, 1.0);

  cairo_new_path(pCairo);
  cairo_move_to(pCairo, 0.0, 0.0);
  cairo_line_to(pCairo, width, 0.0);
  cairo_line_to(pCairo, width, height);
  cairo_line_to(pCairo, 0.0, height);
  cairo_close_path(pCairo);

  cairo_fill(pCairo);
}

static void
bs_main_draw_fps(cairo_t *pCairo,
                 bs_main_data_t *pMainData)
{
  char fpsBuffer[32];
  struct timeval timeStamp;
  bs_int32_t thisFrameTimeStamp;

  gettimeofday(&timeStamp, NULL);
  thisFrameTimeStamp = timeStamp.tv_sec * 1000 + timeStamp.tv_usec / 1000;

  pMainData->fps = pMainData->fps * 0.9f +
    0.1f * 1000.0f / (thisFrameTimeStamp - pMainData->lastFrameTimeStamp + 1.0f);
  sprintf(fpsBuffer, "FPS: %.2f", pMainData->fps);
  pMainData->lastFrameTimeStamp = thisFrameTimeStamp;

  cairo_set_source_rgb(pCairo, 0.0, 0.0, 0.0);

  cairo_move_to(pCairo, 10.0, 20.0);
  cairo_show_text(pCairo, fpsBuffer);
}

/** rendering thread
 * 1. start profiler
 * 2. get rendering context pCario
 * 3. lock the rendering SDL surface
    * 4. clear the whole screen
    * 5. rendering screen with function [bs_env_draw]
    * 6. rendering collective []
    * 7. draw screen fps
    * 8. update the in-memory rendering resut to the real window output
 * 9. unlocakthe SDL surface
 * 10. stop the profiler
 * 
*/
static void
bs_main_redraw(bs_main_data_t *pMainData)
{
  cairo_t *pCairo;

  bs_profiler_start(BS_PROFILER_TASK_DRAW);

      pCairo = bs_cairo_sdl_get_cairo(pMainData->pCairoSdl);

      bs_cairo_sdl_lock_surface(pMainData->pCairoSdl);

          bs_main_clear(pCairo, pMainData->width, pMainData->height);
          bs_env_draw(pMainData->pEnv, pCairo, 100.0);
          bs_blob_collective_draw(pMainData->pCollective, pCairo, 100.0);
          //bs_rubberband_draw(pMainData->pRubberband, pCairo, 100.0);
          bs_main_draw_fps(pCairo, pMainData);
          SDL_UpdateRect(pMainData->pCairoSdl->pScreen, 0, 0, 0, 0);

      bs_cairo_sdl_unlock_surface(pMainData->pCairoSdl);

  bs_profiler_stop(BS_PROFILER_TASK_DRAW);
}

/*
the physicss simulation proc
*/
static void
bs_main_update_simulation(bs_main_data_t *pMainData)
{
  bs_profiler_start(BS_PROFILER_TASK_SIMULATION);

  bs_blob_collective_move(pMainData->pCollective, 0.05);
  bs_blob_collective_sc(pMainData->pCollective, pMainData->pEnv);
  bs_blob_collective_set_force(pMainData->pCollective, pMainData->pGravity);

  /*
  bs_rubberband_move(pMainData->pRubberband, 0.05);
  bs_rubberband_sc(pMainData->pRubberband, pMainData->pEnv);
  bs_rubberband_set_force(pMainData->pRubberband, pMainData->pGravity);
  */

  bs_profiler_stop(BS_PROFILER_TASK_SIMULATION);
}

/**
 * main time based event loop handler
 * make a user event wrapper with the SDL_Event,then propgate the event with SDL_PushEvent
*/
static bs_uint32_t
bs_main_sdl_timer_callback(bs_uint32_t timerInterval,
                           void *pUserData)
{
  SDL_Event event;
  SDL_UserEvent userevent;
  bs_main_data_t *pMainData;

  pMainData = (bs_main_data_t*) pUserData;

  userevent.type = SDL_USEREVENT;
  userevent.code = 1;
  userevent.data1 = NULL;
  userevent.data2 = NULL;

  event.type = SDL_USEREVENT;
  event.user = userevent;

  // trigger the user event 
  SDL_PushEvent(&event);

  return pMainData->newTimerInterval;
}

/**
app entrance application
*/
int
main(int argc,
     char *argv[])
{
  // all the app data are stored in the bs_main_data_t object
  bs_main_data_t *pMainData;
  
  SDL_Event event;
  bs_vector_t force;//
  
  int usedTime;
  struct timeval startTime, stopTime;

  BS_MEM_ALLOC_TYPE(pMainData, bs_main_data_t, 1);

  pMainData->pListFactory = bs_list_factory_create();
  pMainData->pCollective = bs_blob_collective_create(1.0f, 1.0f, pMainData->pListFactory);
  //pMainData->pRubberband = bs_rubberband_create(2.5f, 3.0f);
  pMainData->pEnv = bs_env_create(0.0f, 0.0f, 6.0f, 4.0f);
  pMainData->pGravity = bs_vector_create(0.0f, 10.0f);
  pMainData->pCairoSdl = bs_cairo_sdl_create(600, 400);
  pMainData->width = 600.0;
  pMainData->height = 400.0;
  pMainData->newTimerInterval = 50;
  pMainData->fps = 20.0;
  pMainData->running = TRUE;

  bs_profiler_init();

  //init timer for the first callback
  gettimeofday(&startTime, NULL);
  pMainData->lastFrameTimeStamp = startTime.tv_sec * 1000 + startTime.tv_usec / 1000;

  //a timer used for check status at very 50 ms
  SDL_AddTimer(50, bs_main_sdl_timer_callback, pMainData);

  for(;;)
  {
    //to avoid meaningless event-busy-loop,proceed when there is an event
    SDL_WaitEvent(&event);

    switch(event.type)// handle for the key press event ,need to be ported
    {
      //jklfasdndd
      case SDL_KEYDOWN:
        switch(event.key.keysym.sym)
        {
          case SDLK_q:
          case SDLK_ESCAPE:
            exit(0);
            break;

          case SDLK_h:
            bs_blob_collective_split(pMainData->pCollective);
            break;

          case SDLK_j:
            bs_blob_collective_join(pMainData->pCollective);
            break;

          case SDLK_e:
            bs_blob_collective_larger_blobs(pMainData->pCollective);
            break;
          case SDLK_r:
            bs_blob_collective_smaller_blobs(pMainData->pCollective);
            break;

          case SDLK_g:
            if(bs_vector_get_y(pMainData->pGravity) > 0.0)
            {
              bs_vector_set_y(pMainData->pGravity, 0.0);
            }
            else
            {
              bs_vector_set_y(pMainData->pGravity, 10.0);
            }
            break;

          case SDLK_LEFT:
            bs_vector_init(&force, -50.0, 0.0);
            bs_blob_collective_set_force(pMainData->pCollective, &force);
            //bs_rubberband_add_force(pMainData->pRubberband, &force);
            break;

          case SDLK_RIGHT:
            bs_vector_init(&force,  50.0, 0.0);
            bs_blob_collective_set_force(pMainData->pCollective, &force);
            //bs_rubberband_add_force(pMainData->pRubberband, &force);
            break;

          case SDLK_UP:
            bs_vector_init(&force,  0.0, -50.0);
            bs_blob_collective_set_force(pMainData->pCollective, &force);
            //bs_rubberband_add_force(pMainData->pRubberband, &force);
            break;

          case SDLK_DOWN:
            bs_vector_init(&force,  0.0, 50.0);
            bs_blob_collective_set_force(pMainData->pCollective, &force);
            //bs_rubberband_add_force(pMainData->pRubberband, &force);
            break;

          case SDLK_p:
            if(pMainData->running == TRUE)
            {
              pMainData->running = FALSE;
              pMainData->newTimerInterval = 0;
            }
            else
            {
              pMainData->running = TRUE;
              pMainData->newTimerInterval = 50;
              SDL_AddTimer(50, bs_main_sdl_timer_callback, pMainData);
            }
            break;

          case SDLK_v:
            bs_profiler_report();
            break;

          case SDLK_b:
            bs_main_update_simulation(pMainData);
            bs_main_redraw(pMainData);
            break;

          default:
            break;
        }
        break;

      case SDL_USEREVENT: // user defined event,in this case triggered by the timer
        if(event.user.code == 1)
        {
          gettimeofday(&startTime, NULL);
          
                  //update objects physical status
                  bs_main_update_simulation(pMainData);
                  //rendering the scence
                  bs_main_redraw(pMainData);
          
          gettimeofday(&stopTime, NULL);

          //get time used
          usedTime  = (stopTime.tv_sec - startTime.tv_sec) * 1000;
          usedTime += (stopTime.tv_usec - startTime.tv_usec) / 1000;

          if(usedTime < 50)//keep the scence frame rate @ 20 
          {
            usedTime = 50;
          }

          pMainData->newTimerInterval = usedTime;
        }
        break;

      // application level quit event, such as Ctrl+C
      case SDL_QUIT:
        exit(0);
        break;

      default:
        break;
    }
  }

  return 0;
}
