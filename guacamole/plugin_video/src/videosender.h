#ifndef VIDEOSENDER__H
#define VIDEOSENDER__H

#include <guacamole/layer.h>
#include <pthread.h>

#include <cairo/cairo.h>
#include <guacamole/stream.h>

typedef struct video_client_data {

    guac_layer* video_lyr;

    int current_idx;  
    int nal_start;  
    guac_stream* video_stream;
    unsigned char* video_buf;
    unsigned int video_buf_len;

    
    pthread_t render_thread;

} video_client_data;


#endif
