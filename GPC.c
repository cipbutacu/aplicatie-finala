//gps
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


void gps_init(void)
{
	system("sudo killall gpsd");
	system("sudo stty -F /dev/ttyAMA0 38400");
	system("sudo gpsd /dev/ttyAMA0 -F /var/run/gpsd.sock");
}

void gps_read(char *gps_lat,char *gps_long)
{
   FILE *fp;
   char buffer_temp[20];

   fp = popen("gpspipe -w -n 20 | grep -m 1 lat | jq '.lat'", "r");
	if (fp == NULL)
	{
		 printf("Failed to run command\n" );
		 exit(1);
	 }
	while (fgets(buffer_temp, sizeof(buffer_temp)-1, fp) != NULL)
	{

		strcpy(gps_lat,buffer_temp);
		//printf("LAT: (buffer_temp: %s, gps_lat: %s",buffer_temp, gps_lat);
	}
	usleep(500);
    pclose(fp);

    fp = popen("gpspipe -w -n 20 | grep -m 1 lat | jq '.lon'", "r");
    	if (fp == NULL)
    	{
    		 printf("Failed to run command\n" );
    		 exit(1);
    	 }
    	while (fgets(buffer_temp, sizeof(buffer_temp)-1, fp) != NULL)
    	{

    		strcpy(gps_long,buffer_temp);
    		//printf("LAT: (buffer_temp: %s, gps_long: %s",buffer_temp, gps_long);
    	}
    	usleep(500);
        pclose(fp);
        //printf("Coordordinates(%s, %s)",gps_long, gps_lat );
	//printf("rezultat: %s \n", buffer_temp);
	//gps_lat  = system("gpspipe -w -n 20 | grep -m 1 lat | jq '.lat'");
	//gps_long = system("gpspipe -w -n 20 | grep -m 1 lat | jq '.lon'");

    //system("sudo killall gpspipe ");
}
