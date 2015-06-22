//define and stuf
#include <libwebsockets.h>
#include "pthread.h"
#include "Motors.h"

extern unsigned char applbuf_in[10];
//extern struct libwebsocket_protocols protocols[] ;
extern struct libwebsocket_context *context;
extern pthread_mutex_t Semaphore_read_web_buffer;

typedef struct
{
	char rbi_inainte :1;
	char rbi_inapoi  :1;
	char rbi_dreapta :1;
	char rbi_stanga  :1;
	char rbi_stop 	 :1;
	int  DirectionOfMotor;
	char rub_takephoto;
	char prepare_photo;
	char rezultat_analiza[255];
	char latitudine[20];
	char logitudine[20];
}WEB_BUFFER_COMUNICATION;

extern WEB_BUFFER_COMUNICATION buf_control_comands;

void init_libwebsocket_parameters_void ();
extern void read_input(void);
extern void write_output(void);
