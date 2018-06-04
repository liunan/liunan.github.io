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

#include <libavcodec/avcodec.h>


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
bs_main_redraw(bs_main_data_t *pMainData,guac_socket* socket,video_client_data* pClientData)
{  
  cairo_t *pCairo;


  //bs_profiler_start(BS_PROFILER_TASK_DRAW);

  //pCairo = bs_cairo_sdl_get_cairo(pMainData->pCairoSdl);

  pCairo = pMainData->pCairoSdl->pCairo;

  guac_protocol_send_blob(socket, pClientData->video_stream,
                             "123456778888", 3 /*length from end to start*/);
  guac_socket_flush(socket);

  //bs_cairo_sdl_lock_surface(pMainData->pCairoSdl);

        bs_main_clear(pCairo, pMainData->width, pMainData->height);
        bs_env_draw(pMainData->pEnv, pCairo, 100.0);
        bs_blob_collective_draw(pMainData->pCollective, pCairo, 100.0);
        
        //TODO: removed fps rendering temporly
        //bs_main_draw_fps(pCairo, pMainData);

        //TODO: no sdl update ,we need get rendering result to the video stream
        //SDL_UpdateRect(pMainData->pCairoSdl->pScreen, 0, 0, 0, 0);

        //flush the pending rendering instructions
        cairo_surface_flush(cairo_get_target(pCairo));

        guac_protocol_send_blob(socket, pClientData->video_stream,
                             "123456778888", 4 /*length from end to start*/);
        guac_socket_flush(socket);

        unsigned char * imgBuf = cairo_image_surface_get_data((cairo_get_target(pCairo)));
        static int outted = 0;
        if(imgBuf && !outted)
        {
            outted = 1;
            FILE* fp  =fopen("/home/liunan/output.txt","wb");
            char meta[260] = {0};
            sprintf(meta,"P6\n%d %d\n%d\n",cairo_image_surface_get_stride(cairo_get_target(pCairo)),768,65536);
            fwrite(meta,strlen(meta),1,fp);
            fflush(fp);
            fclose(fp);
            
            cairo_surface_write_to_png(cairo_get_target(pCairo),"/home/liunan/output.png");
            
        }
    
    guac_protocol_send_blob(socket, pClientData->video_stream,
                             "123456778888", 5 /*length from end to start*/);
    guac_socket_flush(socket);

  //bs_cairo_sdl_unlock_surface(pMainData->pCairoSdl);

  //bs_profiler_stop(BS_PROFILER_TASK_DRAW);
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


/**
 * context data is stored in the enc_ctx and current frame data is in the frame parameter,
 * we will get the stream buf result via the pkt
 * 
 * finally we send all the bytes to the video stream in the guacamole user struct.
 * */
static void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt,guac_user *user)
{
    int ret;

    /* send the frame to the encoder */
    // if (frame)
    //     printf("Send frame %3"PRId64"\n", frame->pts);

    ret = avcodec_send_frame(enc_ctx, frame);
    // if (ret < 0) {
    //     fprintf(stderr, "Error sending a frame for encoding\n");
    //     exit(1);
    // }

    guac_client *client = user->client;
    /* Get user-specific socket */
    guac_socket *socket = user->socket;
    video_client_data *client_data = (video_client_data *)client->data;

    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }

        //printf("Write packet %3"PRId64" (size=%5d)\n", pkt->pts, pkt->size);
        //fwrite(pkt->data, 1, pkt->size, outfile);
        guac_protocol_send_blob(socket, client_data->video_stream,
                             pkt->data, pkt->size /*length from end to start*/);
        guac_socket_flush(socket);

        av_packet_unref(pkt);
    }
}

//forward declation
void* video_render_thread(void* arg);

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
   


    
#if 0    
    int i, ret, x, y;

    for (i = 0; i < 200; i++)
    {
        
        av_frame_make_writable(frame);
        /* prepare a dummy image */
        /* Y */
        for (y = 0; y < c->height; y++) {
            for (x = 0; x < c->width; x++) {
                frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
            }
        }

        /* Cb and Cr */
        for (y = 0; y < c->height/2; y++) {
            for (x = 0; x < c->width/2; x++) {
                frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
                frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
            }
        }

        frame->pts = i;

        guac_protocol_send_blob(socket, pClientData->video_stream,
                             "video/mp4", 7 );
            guac_socket_flush(socket);

        /* encode the image */
        encode(c, frame, pkt,user);// now ready to transmit the encoded buffer resut

    }

#endif    
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

void sendframe()
{

}

void* video_render_thread(void* arg) {

    /* Get data */
    guac_user* user = (guac_user*) arg;
    guac_client *client = user->client;    
    guac_socket* socket = client->socket;
    video_client_data* pClientData = (video_client_data*) client->data;

    ////////////////////////////////////////
    // ffmpeg context
    int i = 0;

    // send init frame 
    const AVCodec *codec;
    codec = avcodec_find_encoder_by_name("libx264");
    


    AVCodecContext *c= NULL;
    c = avcodec_alloc_context3(codec);

    AVFrame *frame;
    AVPacket *pkt;
    
    pkt = av_packet_alloc();

    /* put sample parameters */
    c->bit_rate = 20000000;
    /* resolution must be a multiple of two */
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
        av_opt_set(c->priv_data, "preset", "slow", 0);
        av_opt_set(c->priv_data, "profile", "baseline", 0);

    }
    

    avcodec_open2(c, codec, NULL);//TODO: get open result

    frame = av_frame_alloc();
    frame->format = c->pix_fmt;
    frame->width  = c->width;
    frame->height = c->height;

    av_frame_get_buffer(frame, 32);//TODO: check result



        //////////////////////////////////////////    
    bs_main_data_t *pMainData;
    BS_MEM_ALLOC_TYPE(pMainData, bs_main_data_t, 1);

    bs_vector_t force;
    int usedTime;
    struct timeval startTime, stopTime;

    int screenW,screenH;
    screenW = 1024;
    screenH = 768;

    pMainData->pListFactory = bs_list_factory_create();
    pMainData->pCollective = bs_blob_collective_create(1.0f, 1.0f, pMainData->pListFactory);    
    pMainData->pEnv = bs_env_create(0.0f, 0.0f, 6.0f, 4.0f);
    pMainData->pGravity = bs_vector_create(0.0f, 10.0f);

    pMainData->pCairoSdl = bs_ln_cairo_st_create(screenW, screenH);
    pMainData->width = screenW;
    pMainData->height = screenH;
    pMainData->newTimerInterval = 50;
    pMainData->fps = 20.0;
    pMainData->running = TRUE;

    //bs_profiler_init();

    gettimeofday(&startTime, NULL);
    pMainData->lastFrameTimeStamp = startTime.tv_sec * 1000 + startTime.tv_usec / 1000;
    


    int ret,x,y;

    /* Update ball position as long as client is running */
    while (client->state == GUAC_CLIENT_RUNNING) {

        /* Sleep a bit */
        // TODO: variant sleep for balanced fps
        //usleep(30000);

        /* Update position */


        {
        
            av_frame_make_writable(frame);
            /* prepare a dummy image */


            guac_protocol_send_blob(socket, pClientData->video_stream,
                             "123", 1 /*length from end to start*/);
            guac_socket_flush(socket);

            ////////////////////////////
            gettimeofday(&startTime, NULL);
            bs_main_update_simulation(pMainData);
            bs_main_redraw(pMainData,socket,pClientData);
            sendframe();
            
            guac_protocol_send_blob(socket, pClientData->video_stream,
                             "123", 2 /*length from end to start*/);
            guac_socket_flush(socket);

            gettimeofday(&stopTime, NULL);

            usedTime  = (stopTime.tv_sec - startTime.tv_sec) * 1000;
            usedTime += (stopTime.tv_usec - startTime.tv_usec) / 1000;

            if(usedTime < 50)
            {
                usedTime = 50;
            }

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

    /* Set up client data and handlers */
    client->data = pClientData;
    /* Allocate video layer at the client level */
    pClientData->video_lyr = guac_client_alloc_layer(client);
    
    pClientData->current_idx = pClientData->nal_start = 0;

    
    return 0;
}