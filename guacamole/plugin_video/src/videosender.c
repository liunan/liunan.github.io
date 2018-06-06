#include "videosender.h"

#include <guacamole/client.h>
#include <guacamole/protocol.h>
#include <guacamole/socket.h>
#include <guacamole/user.h>

#include <guacamole/layer.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libavcodec/avcodec.h>

#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>

#include <libswscale/swscale.h>

static void
bs_main_clear(cairo_t *pCairo,
              double width,
              double height)
{
    cairo_set_source_rgb(pCairo, .5, .5, .5);

    cairo_new_path(pCairo);
    cairo_move_to(pCairo, 0.0, 0.0);
    cairo_line_to(pCairo, width, 0.0);
    cairo_line_to(pCairo, width, height);
    cairo_line_to(pCairo, 0.0, height);
    cairo_close_path(pCairo);

    cairo_fill(pCairo);
}

static void
bs_main_redraw(bs_main_data_t *pMainData, guac_socket *socket, video_client_data *pClientData)
{
    cairo_t *pCairo;

    //bs_profiler_start(BS_PROFILER_TASK_DRAW);

    //pCairo = bs_cairo_sdl_get_cairo(pMainData->pCairoSdl);

    pCairo = pMainData->pCairoSdl->pCairo;

    //bs_cairo_sdl_lock_surface(pMainData->pCairoSdl);

    bs_main_clear(pCairo, pMainData->width, pMainData->height);
    bs_env_draw(pMainData->pEnv, pCairo, 100.0);
    bs_blob_collective_draw(pMainData->pCollective, pCairo, 100.0);

    //TODO: removed fps rendering temporly
    //bs_main_draw_fps(pCairo, pMainData);

    //TODO: no sdl update ,we need get rendering result to the video stream
    //SDL_UpdateRect(pMainData->pCairoSdl->pScreen, 0, 0, 0, 0);

    //flush the pending rendering instructions

    //bs_cairo_sdl_unlock_surface(pMainData->pCairoSdl);

    //bs_profiler_stop(BS_PROFILER_TASK_DRAW);
}

static void
bs_main_update_simulation(bs_main_data_t *pMainData)
{
    //bs_profiler_start(BS_PROFILER_TASK_SIMULATION);

    bs_blob_collective_move(pMainData->pCollective, 0.05);
    bs_blob_collective_sc(pMainData->pCollective, pMainData->pEnv);
    bs_blob_collective_set_force(pMainData->pCollective, pMainData->pGravity);
    
    //bs_profiler_stop(BS_PROFILER_TASK_SIMULATION);
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

/**
 * guacamole plugin key handler callback
*/
int key_handler(guac_user *user, int keysym, int pressed)
{

    video_client_data *pClientData = (video_client_data *)user->client->data;
    bs_main_data_t *pMainData = pClientData->pMainData;
    bs_vector_t force;
    if (pressed)
    {
        switch (keysym)
        {
        case XK_Left:
            bs_vector_init(&force, -50.0, 0.0);
            bs_blob_collective_set_force(pMainData->pCollective, &force);
            break;
        case XK_Right:
            bs_vector_init(&force, 50.0, 0.0);
            bs_blob_collective_set_force(pMainData->pCollective, &force);
            break;
        case XK_Up:
            bs_vector_init(&force, 0.0, -50.0);
            bs_blob_collective_set_force(pMainData->pCollective, &force);
            break;
        case XK_Down:
            bs_vector_init(&force, 0.0, 50.0);
            bs_blob_collective_set_force(pMainData->pCollective, &force);
            break;

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
            if (bs_vector_get_y(pMainData->pGravity) > 0.0)
            {
                bs_vector_set_y(pMainData->pGravity, 0.0);
            }
            else
            {
                bs_vector_set_y(pMainData->pGravity, 10.0);
            }
            break;
        }
    }

    return 0;
}

/**
 * context data is stored in the enc_ctx and current frame data is in the frame parameter,
 * we will get the stream buf result via the pkt
 * 
 * finally we send all the bytes to the video stream in the guacamole user struct.
 * */
static void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, guac_user *user)
{
    int ret;

    ret = avcodec_send_frame(enc_ctx, frame);

    guac_client *client = user->client;
    /* Get user-specific socket */
    guac_socket *socket = user->socket;
    video_client_data *client_data = (video_client_data *)client->data;

    while (ret >= 0)
    {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0)
        {
            fprintf(stderr, "Error during encoding\n");            
            break;
        }
        
        guac_protocol_send_blob(socket,
                                client_data->video_stream,
                                pkt->data, pkt->size);
        guac_socket_flush(socket);

        av_packet_unref(pkt);
    }
}

//forward declaration
void *video_render_thread(void *arg);

int video_join_handler(guac_user *user, int argc, char **argv)
{

    /* Get client associated with user */
    guac_client *client = user->client;

    //user->mouse_handler = mouse_handler;
    user->key_handler = key_handler;

    /* Get user-specific socket */
    guac_socket *socket = user->socket;

    /* Send the display size */
    guac_protocol_send_size(socket, GUAC_DEFAULT_LAYER, 1920, 1080);

    /* Prepare a curve which covers the entire layer */
    guac_protocol_send_rect(socket, GUAC_DEFAULT_LAYER,
                            0, 0, 1024, 768);

    /* Fill background with solid color white */
    guac_protocol_send_cfill(socket,
                             GUAC_COMP_OVER, GUAC_DEFAULT_LAYER,
                             0x7f, 0x7f, 0x7f, 0xFF);

    /* Mark end-of-frame */
    guac_protocol_send_sync(socket, client->last_sent_timestamp);

    /* Flush buffer */
    guac_socket_flush(socket);

    // allocate user stream for the video tunnel
    video_client_data *pClientData = (video_client_data *)client->data;
    pClientData->video_stream = guac_user_alloc_stream(user);

    // open the video transmit tunnel for the screen output
    guac_protocol_send_video(socket,
                             pClientData->video_stream, //const guac_stream * 	stream,
                             GUAC_DEFAULT_LAYER,
                             "video/mp4" /*stream type*/);
    guac_socket_flush(socket);

    //make the time-based callback
    pthread_create(&pClientData->render_thread, NULL, video_render_thread, user);


    return 0;
}

int leave_handler(guac_user *user)
{
    video_client_data *data = (video_client_data *)user->client->data;
    //TODO free user stream
    guac_user_free_stream(user, data->video_stream);
    return 0;
}

int video_free_handler(guac_client *client)
{
    video_client_data *data = (video_client_data *)client->data;    

    /* Wait for render thread to terminate */
    pthread_join(data->render_thread, NULL);

    
    /* Free client-specific data */
    free(data);

    /* Data successfully freed */
    return 0;
}

void *video_render_thread(void *arg)
{

    /* Get data */
    guac_user *user = (guac_user *)arg;
    guac_client *client = user->client;
    guac_socket *socket = client->socket;
    video_client_data *pClientData = (video_client_data *)client->data;

    ////////////////////////////////////////
    // ffmpeg context

    guac_client_log(client, GUAC_LOG_WARNING, "rendering thread started!");

    // send init frame
    const AVCodec *codec;
    codec = avcodec_find_encoder_by_name("libx264");

    AVCodecContext *c = NULL;
    c = avcodec_alloc_context3(codec);

    AVFrame *frame;
    AVPacket *pkt;

    pkt = av_packet_alloc();

    /* put sample parameters */
    c->bit_rate = 10000000;
    /* resolution must be a multiple of two */
    //TODO: get width & height from client side
    c->width = 1024;
    c->height = 768;
    /* frames per second */
    c->time_base = (AVRational){1, 25};
    c->framerate = (AVRational){25, 1};

    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    c->gop_size = 10;
    c->max_b_frames = 1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;

    if (codec->id == AV_CODEC_ID_H264)
    {
        //av_opt_set(c->priv_data, "preset", "slow", 0);
        av_opt_set(c->priv_data, "profile", "baseline", 0);
    }

    avcodec_open2(c, codec, NULL); //TODO: get open result

    frame = av_frame_alloc();
    frame->format = c->pix_fmt;
    frame->width = c->width;
    frame->height = c->height;

    //av_frame_get_buffer(frame, 32);//TODO: check result
    av_image_alloc(frame->data, frame->linesize, c->width, c->height, c->pix_fmt, 32);

    //////////////////////////////////////////
    bs_main_data_t *pMainData;
    

    pMainData = pClientData->pMainData;
    
    int usedTime;
    struct timeval startTime, stopTime;
    
    //bs_profiler_init();

    gettimeofday(&startTime, NULL);
    pMainData->lastFrameTimeStamp = startTime.tv_sec * 1000 + startTime.tv_usec / 1000;

    struct SwsContext *ctx = NULL;
    ctx = sws_getCachedContext(ctx,
                               //c->width, c->height,AV_PIX_FMT_RGB24,
                               c->width, c->height, AV_PIX_FMT_RGB32,
                               c->width, c->height, AV_PIX_FMT_YUV420P,
                               0, 0, 0, 0);

    
    int i = 0;
    /* Update ball position as long as client is running */
    while (client->state == GUAC_CLIENT_RUNNING)
    {

        /* Sleep a bit */
        // TODO: variant sleep for balanced fps
        //usleep(30000);

        /* Update position */

        {

            av_frame_make_writable(frame);
            /* prepare a dummy image */

            // guac_protocol_send_blob(socket, pClientData->video_stream,
            //                  "123", 1 /*length from end to start*/);
            // guac_socket_flush(socket);

            ////////////////////////////
            gettimeofday(&startTime, NULL);
            bs_main_update_simulation(pMainData);
            bs_main_redraw(pMainData, socket, pClientData);

            // guac_protocol_send_blob(socket, pClientData->video_stream,
            //                  "123", 2 /*length from end to start*/);
            // guac_socket_flush(socket);

            cairo_t *pCairo;

            //bs_profiler_start(BS_PROFILER_TASK_DRAW);

            

            pCairo = pMainData->pCairoSdl->pCairo;
            cairo_surface_flush(cairo_get_target(pCairo));

            unsigned char *imgBuf = cairo_image_surface_get_data((cairo_get_target(pCairo)));
            
            if (imgBuf)
            {

                int inLinesize[1] = {4 * c->width}; // RGB stride
                sws_scale(ctx, &imgBuf, inLinesize, 0,
                          c->height, frame->data, frame->linesize);
                frame->pts = i;
                ++i;
                encode(c, frame, pkt, user);
            }

            gettimeofday(&stopTime, NULL);

            usedTime = (stopTime.tv_sec - startTime.tv_sec) * 1000;
            usedTime += (stopTime.tv_usec - startTime.tv_usec) / 1000;
            int sleepTime = 0;
            if (usedTime < 50)
            {
                sleepTime = 50-usedTime;
            }
            if(sleepTime)
                usleep(sleepTime*1000);
            //pMainData->newTimerInterval = usedTime;
        }
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

    BS_MEM_ALLOC_TYPE(pClientData->pMainData, bs_main_data_t, 1);
    bs_main_data_t* pMainData = pClientData->pMainData;
    pMainData->pListFactory = bs_list_factory_create();
    pMainData->pCollective = bs_blob_collective_create(1.0f, 1.0f, pMainData->pListFactory);
    pMainData->pEnv = bs_env_create(0.0f, 0.0f, 6.0f, 4.0f);
    pMainData->pGravity = bs_vector_create(0.0f, 10.0f);

    int screenW = 1024;
    int screenH = 768;
    pMainData->pCairoSdl = bs_ln_cairo_st_create(screenW, screenH);
    pMainData->width = screenW;
    pMainData->height = screenH;
    pMainData->newTimerInterval = 50;
    pMainData->fps = 20.0;
    pMainData->running = TRUE;

    /* Set up client data and handlers */
    client->data = pClientData;
    /* Allocate video layer at the client level */
    

    

    return 0;
}