// Modul pentru motoare
// Modul pentru motoare
#include "Motors.h"
#include "WebConnect.h"
#include  <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define PIFACE  200


// resursa semafor intre citire si scriere
int DirectionOfMotor = 200;
int inaite_old,inapoi_old,inainte_ev,inapoi_ev;
void Init_comands(void)
{
	buf_control_comands.rbi_dreapta = 0;
	buf_control_comands.rbi_stanga  = 0;
	buf_control_comands.rbi_inainte = 0;
	buf_control_comands.rbi_inapoi  = 0;
	buf_control_comands.rbi_stop = 0;
	buf_control_comands.DirectionOfMotor = 50;
	buf_control_comands.rub_takephoto = 1;
	strcpy(buf_control_comands.rezultat_analiza,"");
}
void Motors_comands_void(void)
 {

	 static int debinainte_in  = 0;
	 static int debinainte_out = 0;
	 static int debinapoi_in   = 0;
	 static int debinapoi_out  = 0;
	 static int pwm_out1       = 0;
	 static int pwm_out2       = 0;
	 int i = 0;

	 if (buf_control_comands.DirectionOfMotor > 50)
	 {
		 pwm_out1 = (100 *(buf_control_comands.DirectionOfMotor - 50));
		 pwm_out2 = 0;
		 softPwmWrite (PIFACE,pwm_out1);
		 softPwmWrite (PIFACE+1,pwm_out2) ;
	 }else if (buf_control_comands.DirectionOfMotor < 50)
	 {
		 pwm_out1 = 0 ;
		 pwm_out2 = 100 *(50 - buf_control_comands.DirectionOfMotor);
		 softPwmWrite (PIFACE,pwm_out1);
		 softPwmWrite (PIFACE+1,pwm_out2) ;
	 }
	 else
	 {
		 pwm_out1=0;
		 pwm_out2=0;
		 softPwmWrite (PIFACE,pwm_out1);
		 softPwmWrite (PIFACE+1,pwm_out2) ;
	 }

	 if (buf_control_comands.rbi_dreapta  == 1 )
	 {
		 pwmWrite(1,45);
	 }
	 else if (buf_control_comands.rbi_stanga  == 1)
	 {
		 pwmWrite(1,105);

	 }
	 else
	 {
		 pwmWrite(1,75);
	 }


 }
