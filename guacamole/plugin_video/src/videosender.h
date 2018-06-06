#ifndef VIDEOSENDER__H
#define VIDEOSENDER__H

#include <guacamole/layer.h>
#include <pthread.h>

#include <cairo/cairo.h>
#include <guacamole/stream.h>
#include "bs_ln_cairo.h"
#include "bs_blob_collective.h"
#include "bs_assert.h"
#include "bs_mem.h"
#include "bs_list.h"
#include "bs_profiler.h"
#include "bs_rubberband.h"

typedef struct bs_main_data_st
{
  bs_ln_cairo_t *pCairoSdl;
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

typedef struct video_client_data {

    guac_stream* video_stream;  

    bs_main_data_t* pMainData;
    
    pthread_t render_thread;
} video_client_data;


#endif
