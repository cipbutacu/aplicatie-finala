//libwebsockend and stuf
#include "WebConnect.h"
#include "libwebsockets.h"


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "pthread.h"
#include <string.h>

WEB_BUFFER_COMUNICATION buf_control_comands;

pthread_mutex_t Semaphore_read_web_buffer = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Semaphore_write_web_buffer = PTHREAD_MUTEX_INITIALIZER;

unsigned char applbuf_in[10];
unsigned char applbuf_out[255];

struct libwebsocket_context *context;
static int callback_http(struct libwebsocket_context * this,
                         struct libwebsocket *wsi,
                         enum libwebsocket_callback_reasons reason, void *user,
                         void *in, size_t len);
static int callback_dumb_increment(struct libwebsocket_context * this,
                                   struct libwebsocket *wsi,
                                   enum libwebsocket_callback_reasons reason,
                                   void *user, void *in, size_t len);

struct libwebsocket_protocols protocols[] = {
   /* first protocol must always be HTTP handler */
   {
       "http-only",   // name
       callback_http, // callback
       0              // per_session_data_size
   },
   {
       "dumb-increment-protocol", // protocol name - very important!
       callback_dumb_increment,   // callback
       0                          // we don't use any per session data

   },
   {
       NULL, NULL, 0   /* End of list */
   }
};


static int callback_http(struct libwebsocket_context * this,
                         struct libwebsocket *wsi,
                         enum libwebsocket_callback_reasons reason, void *user,
                         void *in, size_t len)
{
    return 0;
}

static int callback_dumb_increment(struct libwebsocket_context * this,
                                   struct libwebsocket *wsi,
                                   enum libwebsocket_callback_reasons reason,
                                   void *user, void *in, size_t len)
{

    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED: // just log message that someone is connecting
            printf("connection established\n");
            break;
        case LWS_CALLBACK_RECEIVE: {  // the funny part
            // create a buffer to hold our response
            // it has to have some pre and post padding. You don't need to care
            // what comes there, libwebsockets will do everything for you. For more info see
            // http://git.warmcat.com/cgi-bin/cgit/libwebsockets/tree/lib/libwebsockets.h#n597
            unsigned char *buf = (unsigned char*) malloc(LWS_SEND_BUFFER_PRE_PADDING + len +
                                                         LWS_SEND_BUFFER_POST_PADDING);
            unsigned char *buf_out = (unsigned char*) malloc(LWS_SEND_BUFFER_PRE_PADDING + 50+
                                                                     LWS_SEND_BUFFER_POST_PADDING);
            int i;

            // pointer to `void *in` holds the incomming request
            // we're just going to put it in reverse order and put it in `buf` with
            // correct offset. `len` holds length of the request.
            for (i=0; i < len; i++)
            {
                buf[LWS_SEND_BUFFER_PRE_PADDING + i ] = ((char *) in)[i];
                pthread_mutex_lock( &Semaphore_read_web_buffer);
                applbuf_in[i] = ((char *) in)[i];
               	pthread_mutex_unlock( &Semaphore_read_web_buffer);
            }

            // log what we recieved and what we're going to send as a response.
            // that disco syntax `%.*s` is used to print just a part of our buffer
            // http://stackoverflow.com/questions/5189071/print-part-of-char-array
         //   printf("received data: %s, replying: %.*s\n", (char *) in, (int) len,
                // buf + LWS_SEND_BUFFER_PRE_PADDING);

            for (i=0; i < 50; i++)
            {
            	buf_out[LWS_SEND_BUFFER_PRE_PADDING +i ] = applbuf_out[i];
            }

            // send response
            // just notice that we have to tell where exactly our response starts. That's
            // why there's `buf[LWS_SEND_BUFFER_PRE_PADDING]` and how long it is.
            // we know that our response has the same length as request because
            // it's the same message in reverse order.
            libwebsocket_write(wsi, &buf_out[LWS_SEND_BUFFER_PRE_PADDING], 50, LWS_WRITE_TEXT);

            // release memory back into the wild
            for (i=0; i < (10+LWS_SEND_BUFFER_PRE_PADDING+LWS_SEND_BUFFER_POST_PADDING); i++)
            {
            	//printf("\n out: buf_out[%d]:%d" , i,buf_out[i]);
            }
            free(buf);
            free(buf_out);
            break;
        }
        default:
            break;
    }

    return 0;
}
void init_libwebsocket_parameters_void ()
{
	// server url will be http://localhost:9001pi@raspberrypi ~/program/APP $ nano makefile

	struct lws_context_creation_info info;
	//libwebsocket_protocols protocols[];
	memset(&info, 0, sizeof info);
	info.port = 9003;
	const char *interface = NULL;
	info.iface = interface;
	info.protocols = protocols;
	//info.extensions = libwebsocket_get_internal_extensions();

	info.ssl_cert_filepath = NULL;
	info.ssl_private_key_filepath = NULL;

	info.gid = -1;
	info.uid = -1;
	info.options = 0;

	context = libwebsocket_create_context(&info);
	//------
	if (context == NULL)
	{
	         fprintf(stderr, "libwebsocket init failed\n");
	         exit(EXIT_FAILURE);
	 }
	else
	{
	     printf("starting server...\n");
	     printf("starting server...\n");
	}
}
void read_input()
{
	pthread_mutex_lock( &Semaphore_read_web_buffer);
	buf_control_comands.prepare_photo = (applbuf_in[15] - '0');
    if(!buf_control_comands.prepare_photo)
    {
	buf_control_comands.rbi_inainte = applbuf_in[0];
	buf_control_comands.rbi_inapoi  = applbuf_in[2];
	buf_control_comands.rbi_dreapta = applbuf_in[4];
	buf_control_comands.rbi_stanga  = applbuf_in[6];
	buf_control_comands.rbi_stop 	= applbuf_in[8];

	buf_control_comands.DirectionOfMotor = (int)((applbuf_in[10]-'0')*10) +((applbuf_in[11]-'0'));
	buf_control_comands.rub_takephoto  = (applbuf_in[13]-'0');
    }
    else
    {
    	buf_control_comands.rbi_inainte = 0;
    	buf_control_comands.rbi_inapoi  = 0;
    	buf_control_comands.rbi_dreapta = 0;
    	buf_control_comands.rbi_stanga  = 0;
    	buf_control_comands.rbi_stop 	= 0;
    	buf_control_comands.DirectionOfMotor = 50;
    	buf_control_comands.rub_takephoto  = (applbuf_in[13]-'0');
    }

	pthread_mutex_unlock( &Semaphore_read_web_buffer);
}

void write_output(void)
{
	strcpy(applbuf_out,buf_control_comands.rezultat_analiza);
	strcat(applbuf_out,",");
	strcat(applbuf_out,buf_control_comands.logitudine);
	strcat(applbuf_out,",");
	strcat(applbuf_out,buf_control_comands.latitudine);

}

