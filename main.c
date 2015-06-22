/*
 * homey.c:
 *      Homey Main Application.
  ***********************************************************************
 */
 

#include <softPwm.h>

//piface board
#include <wiringPi.h>
#include <piFace.h>
#include <libwebsockets.h>

//for catching system signals (SIGINT)
#include  <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//sttring manipulations
#include <string.h>
//for sleep and stuff
#include <unistd.h>
#include <time.h>
//for threads
#include "pthread.h"
#include "main.h"
#include "db-EXT.h"
#include "WebConnect.h"
#include "Motors.h"
#include "GPS.h"
#include "db.h"
/*DEFINES*/



/*Undefine this to disable debug output to console*/
#define _DEBUG_ENABLED


//FUNCTION LIKE MACROS


/*GLOBAL VARIABLES*/

//if CTRL+C is pressed, this is set
uint8_t  sigINTReceived = 0;
uint8_t flag_req_to_stop_Thread = 0;
uint8_t g_TaskCounter_u8 =0u;
pthread_mutex_t Semafore_exit_flag = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t SemaforeDB_exit_flag = PTHREAD_MUTEX_INITIALIZER;

uint8_t take_photo_old = 1;
uint8_t video_stopped;
uint8_t flag_notify_db_update;
unsigned char buff_position = 0;


/*DEBUG enabled declarations*/


#ifdef _DEBUG_ENABLED
#endif


/*FUNCTION DECLARATIONS*/

void *Thread_1();
void *Thread_2();
void *Thread_3();
void strip(char *s);
void InitBoard(void)
{
	InitIO();
}

/* *************************************************************************
 * 	   Init SW board function
	   This function will be called at init, after board init

	   @param[in]     none
	   @return 		  none
 * *************************************************************************/
void InitSW(void)
{
	//InitShm();
	InitDB();
	//init library for websocket comunication
	init_libwebsocket_parameters_void();
	//gps init
	gps_init();
	Init_comands();

}




/* *************************************************************************
 * 	   Main.c function
	   This is the main function of the module

	   @param[in]     _argc number of command line arguments
	   @param[out]    *argv array containing command line parameters
	   @return error code.
 * *************************************************************************/


int main (int argc, char **argv)
{
	//int i;
	int thread_1_flag,thread_2_flag,thread_3_flag,thread_4_flag;
	pthread_t thread1;
	pthread_t thread2;
	pthread_t thread3;
	pthread_t thread4;

	signal(SIGINT, intHandler);
	signal(SIGKILL, intHandler);
	signal(SIGQUIT, intHandler);
   /*Initialize the Rapsberry board*/
    InitBoard();
   /*init sw variables*/
    InitSW();

  //entering main loops
    thread_1_flag = pthread_create(&thread1,NULL,Thread_1,NULL);
    if(thread_1_flag)
    {
    	fprintf(stderr,"Error - pthread_create() return code: %d\n",thread_1_flag);
    	exit(EXIT_FAILURE);
    }
    thread_2_flag = pthread_create(&thread2,NULL,Thread_2,NULL) ;
   if(thread_2_flag)
    {
    fprintf(stderr,"Error - pthread_create() return code: %d\n",thread_2_flag);
   	exit(EXIT_FAILURE);
    }
    thread_3_flag =  pthread_create(&thread3,NULL,Thread_3,NULL) ;
    if(thread_3_flag)
    {
    	fprintf(stderr,"Error - pthread_create() return code: %d\n",thread_3_flag);
    	exit(EXIT_FAILURE);
    }
   thread_4_flag =  pthread_create(&thread4,NULL,threadUpdateDB,NULL);
    if(thread_4_flag)
   {
    	fprintf(stderr,"Error - pthread_create() return code: %d\n",thread_4_flag);
    	exit(EXIT_FAILURE);
    }




  while(1)
  {

	 if (sigINTReceived)
	 {
		 // trebuie sa iesim din program
		 //ENTER_CRIT_SECTION
		 pthread_mutex_lock( &Semafore_exit_flag );
		 flag_req_to_stop_Thread = true;
		 pthread_mutex_unlock( &Semafore_exit_flag );
	     //EXIT_CRIT_SECTION
		 break;
	 }

	 read_input();
	 Motors_comands_void();
     digitalWrite(PIFACE+4,1);
     write_output();
	 // softPwmWrite(PIFACE+3,4000);
	 // delay(25); //sleep not delay
     usleep(1000);

  }
  pthread_join(thread1,NULL); //blocheaza pana cand thread is termina executia
  pthread_join(thread2,NULL); //blocheaza pana cand thread is termina executia
  pthread_join(thread3,NULL);
  libwebsocket_context_destroy(context);
  CleanupAfterSigINT();
  return 0;
}

void *Thread_1()
{
  int local_Flag = false;
	while (1)
  {
	//	fprintf(stderr,"  additional thread");
	  //enter_crit_Section
	  pthread_mutex_lock( &Semafore_exit_flag );
	  local_Flag = flag_req_to_stop_Thread;
	  pthread_mutex_unlock( &Semafore_exit_flag );
	  //exit crit_section
	  if (local_Flag)
	  {
		 //cleanup la resursele alocate doar in thread
		  //inchid conexiunea pe websocket
		  return 0;
	  }
	  // libwebsocket_service will process all waiting events with their
	  // callback functions and then wait 50 ms.
	  // (this is single threaded webserver and this will keep
	  // our server from generating load while there are not
	  // requests to process)
	  libwebsocket_service(context, 50);
      usleep (1000);
  }
}

void *Thread_2()
{
	int local_Flag = false;
	while (1)
	  {
		//	fprintf(stderr,"  additional thread");
		  //enter_crit_Section
		  pthread_mutex_lock( &Semafore_exit_flag );
		  local_Flag = flag_req_to_stop_Thread;
		  pthread_mutex_unlock( &Semafore_exit_flag );
		  //exit crit_section
		  if (local_Flag ||(buf_control_comands.latitudine == 0 || buf_control_comands.logitudine == 0))
		  {
			 //cleanup la resursele alocate doar in thread
			  //inchid conexiunea pe websocket
			  return 0;
		  }
		  gps_read(buf_control_comands.latitudine,buf_control_comands.logitudine);
		//  printf("%s \n" , buf_control_comands.latitudine);
		 // printf("%s \n" , buf_control_comands.logitudine);
		//  snprintf(buf_control_comands.latitudine,50,"%f",gps_lat);
		 // snprintf(buf_control_comands.logitudine,50,"%f",gps_long);

		//  fprintf(stderr,"\n %f",gps_lat);
		//  fprintf(stderr,"\n %f",gps_long);
	      sleep(10);
	  }

}
void *Thread_3()
{
	int local_Flag = false;
	int pid;
	char command[100];
	time_t unix_t;
	char img_name[20];
	char buffer_temp[255];
	FILE *fp;
	//system("raspivid -o - -t 0 -n | cvlc -vvv stream:///dev/stdin --sout '#rtp{sdp=rtsp://:8554/}' :demux=h264 &");
	while (1)
	{
		//enter_crit_Section
		pthread_mutex_lock( &Semafore_exit_flag );
		local_Flag = flag_req_to_stop_Thread;
		pthread_mutex_unlock( &Semafore_exit_flag );
		 if (local_Flag)
		 {
			 //cleanup la resursele alocate doar in thread
			 system("./killvideo.sh");
			 return 0;

		 }
		if((buf_control_comands.prepare_photo == 1) && (!video_stopped))
		{
			 system("./killvideo.sh");
			 video_stopped = 1;
		}
		else if((buf_control_comands.prepare_photo == 0) && (video_stopped))
		{
			system("./killvideo.sh");
			system("raspivid -o - -t 0 -n | cvlc -vvv stream:///dev/stdin --sout '#rtp{sdp=rtsp://:8554/}' :demux=h264 > /dev/null 2>&1 &");
			 video_stopped = 0;
		}
		 if(buf_control_comands.rub_takephoto && (!take_photo_old))
		 {

			 strcpy(command,"raspistill -t 2000 -o input");
			  unix_t = time(NULL);

			 sprintf(buffer_temp, "%ju", (uintmax_t)unix_t);
			// itoa(time,buffer_temp,10);
			 strcat(command,buffer_temp);
			 strcat(command ,".jpg -w 1920 -h 1080");
			 //printf("%s",command);
			 system(command);

			 strcpy(img_name,"./cIdentify.sh input");
			 strcat(img_name,buffer_temp);
			 strcat(img_name,".jpg");
			 printf("executing image recognition : \n");
			 fp = popen(img_name, "r");
			 if (fp == NULL) {
			     printf("Failed to run command\n" );
			     exit(1);
			   }
			 while (fgets(buffer_temp, sizeof(buffer_temp)-1, fp) != NULL) {
				 if (strstr(buffer_temp,"Concept"))
				 {
				 strip(buffer_temp);
				 strcpy(buf_control_comands.rezultat_analiza,buffer_temp);
			     printf("rezultat: %s \n", buffer_temp);
			     //enter protected section
			     pthread_mutex_lock( &SemaforeDB_exit_flag );
			     flag_notify_db_update=1;
			     strcpy(Buffer_DB.denumire_obiect,buffer_temp);
			     strcpy(Buffer_DB.latitudine,buf_control_comands.latitudine);
			     strcpy(Buffer_DB.longitudine,buf_control_comands.logitudine);
			    ///copy rezultate -> BUFFER_PT_DB
			     //copy coordontate -> buffer_PT_DB
			     //copy filenam -> BUFFEr_PT_DB
			    pthread_mutex_unlock(&SemaforeDB_exit_flag);
			     //buffer_temp = system(img_name);
			     //exit protected session
			     usleep(100);
				 }
			   }
			 pclose(fp);


			// printf("%s",buffer_temp);

			// usleep (2000);
		 }
		 else
		 {

		 }
		//exit crit_section
		take_photo_old = buf_control_comands.rub_takephoto;
		usleep (100);
	}
}
/* *************************************************************************
 * 	   System Signal handler function
	   This function will be called after user presses CTRL+C (SIGINT)

	   @param[in]     none
	   @return 		  none
 * *************************************************************************/
void intHandler(int signal_s32)
{
	sigINTReceived = TRUE;
}

/* *************************************************************************
 * 	   Cleanup function
	   This function will be called after user presses CTRL+C (SIGINT)

	   @param[in]     none
	   @return 		  none
 * *************************************************************************/
void CleanupAfterSigINT(void)
{


	 CleanupIO();
	 /* exit */
	 exit(0);
}

void strip(char *s) {
    char *p2 = s;
    while(*s != '\0') {
    	if(*s != '\t' && *s != '\n') {
    		*p2++ = *s++;
    	} else {
    		++s;
    	}
    }
    *p2 = '\0';
}
