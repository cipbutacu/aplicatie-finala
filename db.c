/*
 * db.c:
 *      Homey DB related  Application.
  ***********************************************************************
 */

/* *************************************************************************
 * 	   Activate device associated with
	   the ID indicated by parameter

	   @param[in]     none
	   @return 		  none
 * *************************************************************************/
#include "db.h"
#include "db-EXT.h"
#include "homey-EXT.h"
#include "out-EXT.h"


//stdin , printf and stuff
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

//for mysql
#include <mysql.h>

//sleep manipulations
#include <string.h>




/*GLOBAL VARIABLES*/

BufferDB Buffer_DB;

/*mysql connection details*/
MysqlDB_st mysqlConnect_st;
/*result pointer*/
MYSQL_RES *result_ptr = NULL;
/*number of fields in a query*/
uint32_t numOfFields_u16 = 0;
/*structure containing row data*/
MYSQL_ROW row_st;

char queryStr_au8[MAX_QUERY_STRING_LENGTH];
char tempStr_au8[MAX_QUERY_STRING_LENGTH];
/*flags for thread sync*/
uint8_t refreshQuerryRunning_b = FALSE;
/*indicates if a query is already running*/
uint8_t updateQuerryRunning_b = FALSE;

uint8_t updatePerformedSinceLastCheck_u8 = FALSE;

/* this variable is our reference to the second thread */
pthread_t pthreadRefreshFromDB_st;
/*thread will update db and send heyu command to relays*/
pthread_t pthreadUpdateDB_st;

/*global variable used to pass as an argument to the thread. It's value is modified once before the thread is created, so there's no need to protect it with semaphores*/
volatile uint8_t threadArgumentArr_au8[MAX_NR_OF_THREAD_PARAMS];

/*FUNCTION PROTOTYPES*/



void *threadUpdateDB()
{

	uint8_t local_update_flag;
	uint8_t local_Flag;
	int8_t result_lu8 = 0;
    BufferDB local_BufferDB;
    char longitudine_sting[20];
    char latitudine_sting[20];


    while(1){
    	pthread_mutex_lock( &Semafore_exit_flag );
    	local_Flag = flag_req_to_stop_Thread;
    	pthread_mutex_unlock( &Semafore_exit_flag );
    	if (local_Flag)
    	{
    		//cleanup la resursele alocate doar in thread
    		//inchid conexiunea pe websocket
    		return 0;
    	}

    	pthread_mutex_lock( &SemaforeDB_exit_flag );
    	local_update_flag = flag_notify_db_update;

    	pthread_mutex_unlock( &SemaforeDB_exit_flag );
    	if(local_update_flag)
    	{

    		printf("thread db1 \n");
    		pthread_mutex_lock( &SemaforeDB_exit_flag );
    		strcpy(local_BufferDB.denumire_obiect,Buffer_DB.denumire_obiect);
    		strcpy(local_BufferDB.latitudine, Buffer_DB.latitudine);
    		strcpy(local_BufferDB.longitudine,Buffer_DB.longitudine);
    		pthread_mutex_unlock( &SemaforeDB_exit_flag );
    		printf("thread db2 \n");
    		mysqlConnect_st.conn_ptrst = mysql_init(NULL);
    		if (!mysqlConnect_st.conn_ptrst )
    		{
    			/*connect error*/
    			fprintf(stdout,"\nMySQL null pointer error: %s\n", mysql_error(mysqlConnect_st.conn_ptrst));
    			result_lu8 = -1;
    		}
    		else
    		{
    			if (mysql_real_connect(mysqlConnect_st.conn_ptrst, mysqlConnect_st.server_au8, mysqlConnect_st.user_au8,
    					mysqlConnect_st.password_au8,
						mysqlConnect_st.database_au8, 0, NULL, 0) == NULL)
    			{
    				/*connect error*/
    				fprintf(stdout,"\nMySQL connect error: %s\n", mysql_error(mysqlConnect_st.conn_ptrst));
    				result_lu8 = -1;
    			}
    			else
    			{
    				fprintf(stdout,"\nMySQL connect succes\n");
    				strcpy (queryStr_au8, "INSERT INTO ");
    				strcat (queryStr_au8, MYSQL_TABLE_NAME);
    				strcat (queryStr_au8, " VALUES( ,");
    				strcat (queryStr_au8, local_BufferDB.denumire_obiect);
    				strcat (queryStr_au8, ",");
    				strcat (queryStr_au8, "image.jpg");
    				strcat (queryStr_au8, ",");
    				strcat (queryStr_au8, longitudine_sting);
    				strcat (queryStr_au8, ",");
    				strcat (queryStr_au8, longitudine_sting);
    				strcat (queryStr_au8, ")");
    				fprintf(stdout,"\n querry : %s",queryStr_au8);

    				/*if(mysql_query(mysqlConnect_st.conn_ptrst,queryStr_au8))
    				{

    					fprintf(stdout,"\nMySQL connect error: %s\n", mysql_error(mysqlConnect_st.conn_ptrst));
    					result_lu8 = -1;
    				}
    				else
    				{
    					printf("doamne ajuta!");
    				}*/
    			}

    		}

    		//close connection
    		mysql_close(mysqlConnect_st.conn_ptrst);
    		pthread_mutex_lock( &SemaforeDB_exit_flag );
    		flag_notify_db_update = 0;
    		pthread_mutex_unlock( &SemaforeDB_exit_flag );
    	}
    	else
    	{

    	}
    }
    	updatePerformedSinceLastCheck_u8 = FALSE;
    	usleep(1);

}


/* *************************************************************************
 * 	   Init function of DB handling
	   This function will be called at init, after board and IO init

	   @param[in]     none
	   @return 		  none
 * *************************************************************************/
void InitDB(void)
{
	strcpy (mysqlConnect_st.server_au8, MYSQL_HOST);
	strcpy (mysqlConnect_st.user_au8, MYSQL_USERNAME);
	strcpy (mysqlConnect_st.password_au8, MYSQL_PASSWORD);
	strcpy (mysqlConnect_st.database_au8, MYSQL_DB);
	strcpy (mysqlConnect_st.table_au8, MYSQL_TABLE_NAME);


	/*Semaphore init*/
	//sem_init(&mutexSemaphoreQueryDB_st, 0, 1);      /* initialize mutex to 1 - binary semaphore */
	                                 /* second param = 0 - semaphore is local */
	//sem_init(&mutexSemaphoreUpdateDB_st, 0, 1);      /* initialize mutex to 1 - binary semaphore */
	                                 /* second param = 0 - semaphore is local */


}


/* *************************************************************************
 *   Cleanup at shutdown

      @param[in]      none
      @return         none
 * *************************************************************************/
void CleanupDB(void)
{
	  /* wait for thread (if any) to  finish */
	pthread_join(pthreadRefreshFromDB_st, NULL);
	pthread_join(pthreadUpdateDB_st, NULL);

	/*destroy semaphores*/
	sem_destroy(&mutexSemaphoreQueryDB_st); /* destroy semaphore */
	sem_destroy(&mutexSemaphoreUpdateDB_st); /* destroy semaphore */

}
