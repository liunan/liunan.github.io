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

#define M_PI 3.1415926

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

int video_join_handler(guac_user *user, int argc, char **argv)
{

    /* Get client associated with user */
    guac_client *client = user->client;

    user->mouse_handler = mouse_handler;

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

#if 0
    cairo_surface_t *surface;

    cairo_t *cr;

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 256, 256);
    cr = cairo_create (surface);
    double xc = 128.0;
    double yc = 128.0;
    double radius = 100.0;
    double angle1 = 45.0  * (M_PI/180.0);  /* angles are specified */
    double angle2 = 180.0 * (M_PI/180.0);  /* in radians           */

    cairo_set_line_width (cr, 10.0);
    cairo_arc (cr, xc, yc, radius, angle1, angle2);
    cairo_stroke (cr);

    /* draw helping lines */
    cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
    cairo_set_line_width (cr, 6.0);

    cairo_arc (cr, xc, yc, 10.0, 0, 2*M_PI);
    cairo_fill (cr);

    cairo_arc (cr, xc, yc, radius, angle1, angle1);
    cairo_line_to (cr, xc, yc);
    cairo_arc (cr, xc, yc, radius, angle2, angle2);
    cairo_line_to (cr, xc, yc);
    cairo_stroke (cr);

    guac_protocol_send_size(socket, data->video_lyr, 256, 256);

    guac_client_stream_png(client,socket,
                                GUAC_COMP_OVER,
                                data->video_lyr,
                                0,0,
                                surface);

    surface = cairo_image_surface_create_from_png("/home/liunan/Videos/images00515.png");
    //video_client_data* data = (video_client_data*) client->data;
    guac_client_stream_png(client,socket,
                                GUAC_COMP_OVER,
                                GUAC_DEFAULT_LAYER,
                                0,0,
                                surface);
    // dispose cairo 
    //cairo_destroy(cr);
    // dispose surface
    cairo_surface_destroy(surface);
#endif
    /* Mark end-of-frame */
    guac_protocol_send_sync(socket, client->last_sent_timestamp);

    /* Flush buffer */
    guac_socket_flush(socket);

    video_client_data *pClientData = (video_client_data *)client->data;
    pClientData->video_stream = guac_user_alloc_stream(user);

    guac_protocol_send_video(socket,
                                     pClientData->video_stream, //const guac_stream * 	stream,
                                     GUAC_DEFAULT_LAYER,
                                     "video/mp4" /*stream type*/);
    guac_socket_flush(socket);

    //added by ln 20180522
    /*attemp to send a video*/

    /* User successfully initialized */
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

/* Client plugin arguments (empty) */
const char *TUTORIAL_ARGS[] = {NULL};

int guac_client_init(guac_client *client)
{

    /* This example does not implement any arguments */
    client->args = TUTORIAL_ARGS;

    /* Client-level handlers */
    client->join_handler = video_join_handler;
    client->free_handler = video_free_handler;

    video_client_data *pClientData = malloc(sizeof(video_client_data));

    /* Set up client data and handlers */
    client->data = pClientData;
    /* Allocate video layer at the client level */
    pClientData->video_lyr = guac_client_alloc_layer(client);
    
    pClientData->current_idx = pClientData->nal_start = 0;

    /* Start render thread for client level */
    //pthread_create(&pClientData->render_thread, NULL, videoplayer_render_thread, client);

    FILE *fp = fopen("/home/liunan/3d_demo.264", "rb");
    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_END);
        pClientData->video_buf_len = ftell(fp);

        
        fseek(fp, 0, SEEK_SET);
        if (pClientData->video_buf_len > 0)
        {
            pClientData->video_buf = (unsigned char *)malloc(pClientData->video_buf_len);

            int bytes_read = fread(pClientData->video_buf, pClientData->video_buf_len, 1, fp);

            if (bytes_read > 0)
                printf("read successfuly!\n");

            

            fclose(fp);
        }
    }

    return 0;
}