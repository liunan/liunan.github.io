#include "videosender.h"

#include <guacamole/client.h>
#include <guacamole/protocol.h>
#include <guacamole/socket.h>
#include <guacamole/user.h>

#include <guacamole/layer.h>

#include <cairo/cairo.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "bs_ln_cairo.h"
#include "bs_blob_collective.h"
#include "bs_assert.h"
#include "bs_mem.h"
#include "bs_list.h"
#include "bs_profiler.h"
#include "bs_rubberband.h"

#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>

#define M_PI 3.1415926


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
bs_main_redraw(bs_main_data_t *pMainData)
{
  cairo_t *pCairo;

  bs_profiler_start(BS_PROFILER_TASK_DRAW);

  pCairo = bs_cairo_sdl_get_cairo(pMainData->pCairoSdl);

  //bs_cairo_sdl_lock_surface(pMainData->pCairoSdl);

        bs_main_clear(pCairo, pMainData->width, pMainData->height);
        bs_env_draw(pMainData->pEnv, pCairo, 100.0);
        bs_blob_collective_draw(pMainData->pCollective, pCairo, 100.0);
        
        //TODO: removed fps rendering temporly
        //bs_main_draw_fps(pCairo, pMainData);

        //TODO: no sdl update ,we need get rendering result to the video stream
        //SDL_UpdateRect(pMainData->pCairoSdl->pScreen, 0, 0, 0, 0);

  //bs_cairo_sdl_unlock_surface(pMainData->pCairoSdl);

  bs_profiler_stop(BS_PROFILER_TASK_DRAW);
}

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



// keep the fps via an changable sleep length return value
static bs_uint32_t
bs_main_sdl_timer_callback(bs_uint32_t timerInterval,
                           void *pUserData)
{
  //SDL_Event event;
  //SDL_UserEvent userevent;
  bs_main_data_t *pMainData;

  pMainData = (bs_main_data_t*) pUserData;

  /*userevent.type = SDL_USEREVENT;
  userevent.code = 1;
  userevent.data1 = NULL;
  userevent.data2 = NULL;

  event.type = SDL_USEREVENT;
  event.user = userevent;

  SDL_PushEvent(&event);
  */

  return pMainData->newTimerInterval;
}




char isNALU(unsigned char type)
{
    char bRet = 0;
    switch (type)
    {
    case 0x65:
    case 0x67:
    case 0x68:
    case 0x41:
        bRet = type;
    }
    return bRet;
}
/*
sending h264 nalu while user press mouse left button 
*/
int mouse_handler(guac_user *user, int x, int y, int button_mask)
{
    /* Do something */

    //if (button_mask & GUAC_CLIENT_MOUSE_LEFT)
    {
        guac_client *client = user->client;

        /* Get user-specific socket */
        guac_socket *socket = user->socket;

        video_client_data *client_data = (video_client_data *)client->data;
        

        //get the destnation layer,and send the video_stream

        char splitter[] = {0x0, 0x0, 0x1};
        while (client_data->current_idx < client_data->video_buf_len - 3)
        {
            if (memcmp(&client_data->video_buf[client_data->current_idx], splitter, sizeof(splitter)) == 0)
            {
                unsigned char type = client_data->video_buf[client_data->current_idx + 3];

                if (isNALU(type) != 0)
                {
                    if (client_data->nal_start != 0)
                    {
                        int nalu_end = client_data->current_idx + (client_data->video_buf[client_data->current_idx - 1] == 0 ? -1 : 0);
                        //var nalu = bufData.subarray(nal_start,nalu_end );//2 or 3 zero char
                        // send data from [nal_start,nalu_end)
                        guac_protocol_send_blob(socket, client_data->video_stream, client_data->video_buf + client_data->nal_start, nalu_end - client_data->nal_start /*length from end to start*/);
                        guac_socket_flush(socket);
                    }

                    client_data->current_idx += 3;
                    client_data->nal_start = client_data->current_idx;
                    //one frame end exit
                    return 0;
                }
                else
                {
                    ++client_data->current_idx;
                }
            }
            else
            {
                client_data->current_idx++;
            }
        }

        //if the stream is over,restart again
        client_data->current_idx = client_data->nal_start = 0;

        //guac_protocol_send_blob(socket,client_data->video_stream,client_data->video_buf,client_data->video_buf_len);
        /* Flush buffer */
        //guac_socket_flush(socket);

        //guac_protocol_send_end(socket,client_data->video_stream);
        //guac_socket_flush(socket);
    }
  

    return 0;
}

/**
 * guacamole plugin key handler callback
*/
int key_handler(guac_user* user, int keysym, int pressed)
{
    bs_main_data_t *pMainData;
    pMainData = (bs_main_data_t*) user->data;
    switch(keysym){
        
/*        
        case XK_Left:
            bs_vector_init(&force, -50.0, 0.0);
            bs_blob_collective_set_force(pMainData->pCollective, &force);
            break;
        case XK_Right:
            bs_vector_init(&force,  50.0, 0.0);
            bs_blob_collective_set_force(pMainData->pCollective, &force);
            break;
        case XK_Up:
            bs_vector_init(&force,  0.0, -50.0);
            bs_blob_collective_set_force(pMainData->pCollective, &force);
            break;
        case XK_Down:
            bs_vector_init(&force,  0.0, 50.0);
            bs_blob_collective_set_force(pMainData->pCollective, &force);
            break;
*/            
        
        //
        case XK_h:
            bs_blob_collective_split(pMainData->pCollective);
            break;
        case XK_j:
            bs_blob_collective_join(pMainData->pCollective);
            break;
        case XK_e:
            bs_blob_collective_larger_blobs(pMainData->pCollective);
            break;
        case XK_r:
            bs_blob_collective_smaller_blobs(pMainData->pCollective);
            break;
        case XK_g:
            if(bs_vector_get_y(pMainData->pGravity) > 0.0)
            {
              bs_vector_set_y(pMainData->pGravity, 0.0);
            }
            else
            {
              bs_vector_set_y(pMainData->pGravity, 10.0);
            }
            break;
                    
    }

    return 0;
}

int video_join_handler(guac_user *user, int argc, char **argv)
{

    /* Get client associated with user */
    guac_client *client = user->client;

    user->mouse_handler = mouse_handler;
    user->key_handler = key_handler;

    /* Get user-specific socket */
    guac_socket *socket = user->socket;

    /* Send the display size */
    guac_protocol_send_size(socket, GUAC_DEFAULT_LAYER, 1920, 1080);

    /* Prepare a curve which covers the entire layer */
    guac_protocol_send_rect(socket, GUAC_DEFAULT_LAYER,
                            0, 0, 1920, 1080);

    /* Fill background with solid color white */
    guac_protocol_send_cfill(socket,
                             GUAC_COMP_OVER, GUAC_DEFAULT_LAYER,
                             0x7f, 0x7f, 0x7f, 0xFF);


    /* Mark end-of-frame */
    guac_protocol_send_sync(socket, client->last_sent_timestamp);

    /* Flush buffer */
    guac_socket_flush(socket);

    video_client_data *pClientData = (video_client_data *)client->data;
    pClientData->video_stream = guac_user_alloc_stream(user);

    // open the video transmit tunnel for the screen output
    guac_protocol_send_video(socket,
                                     pClientData->video_stream, //const guac_stream * 	stream,
                                     GUAC_DEFAULT_LAYER,
                                     "video/mp4" /*stream type*/);
    guac_socket_flush(socket);

    /* User successfully initialized */
    return 0;
}


int leave_handler(guac_user* user)
{
    return 0;
}

int video_free_handler(guac_client *client)
{

    video_client_data *data = (video_client_data *)client->data;

    /* Wait for render thread to terminate */
    //pthread_join(data->render_thread, NULL);

    /* Free client-level ball layer */
    guac_client_free_layer(client, data->video_lyr);

    // free video buf data
    free(data->video_buf);

    //free the used stream
    //TODO free user stream
    //guac_user_free_stream(client->user, data->video_stream);

    /* Free client-specific data */
    free(data);

    /* Data successfully freed */
    return 0;
}


void* video_render_thread(void* arg) {

    /* Get data */
    guac_client* client = (guac_client*) arg;
    //ball_client_data* data = (ball_client_data*) client->data;

    /* Update ball position as long as client is running */
    while (client->state == GUAC_CLIENT_RUNNING) {

        /* Sleep a bit */
        // TODO: variant sleep for balanced fps
        //usleep(30000);

        /* Update position */
       

    }

    return NULL;
}

/* Client plugin arguments (empty) */
const char *TUTORIAL_ARGS[] = {NULL};

int guac_client_init(guac_client *client)
{

    /* This example does not implement any arguments */
    client->args = TUTORIAL_ARGS;

    /* Client-level handlers */
    client->join_handler = video_join_handler;    
    client->leave_handler = leave_handler;
    client->free_handler = video_free_handler;



    video_client_data *pClientData = malloc(sizeof(video_client_data));

    /* Set up client data and handlers */
    client->data = pClientData;
    /* Allocate video layer at the client level */
    pClientData->video_lyr = guac_client_alloc_layer(client);
    
    pClientData->current_idx = pClientData->nal_start = 0;

    

    //////////////////////////////////////////
    bs_main_data_t *pMainData;
    BS_MEM_ALLOC_TYPE(pMainData, bs_main_data_t, 1);

    bs_vector_t force;
    int usedTime;
    struct timeval startTime, stopTime;


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

    gettimeofday(&startTime, NULL);
    pMainData->lastFrameTimeStamp = startTime.tv_sec * 1000 + startTime.tv_usec / 1000;


    /* Start render thread for client level */
    pthread_create(&pClientData->render_thread, NULL, video_render_thread, client);

    //make the time-based callback





    return 0;
}