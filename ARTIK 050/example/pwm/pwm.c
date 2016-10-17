#include "wiced.h"
#include <stdlib.h>
wiced_pwm_t RGB_R_PWM =  WICED_PWM_1;
wiced_pwm_t RGB_G_PWM =  WICED_PWM_2;
wiced_pwm_t RGB_B_PWM =  WICED_PWM_3;

void RGB_change(int color_R, int color_G, int color_B){
	int i,j,k;
	for(i=0,j=0,k=0;;){
		if(i<color_R){
			i+=10;
		}else{
			i= color_R;
		}
		if(j<color_G){
			j+=10;
		}else{
			j= color_G;
		}
		if(k<color_B){
			k+=10;
		}else{
			k= color_B;
		}

		wiced_pwm_init( RGB_R_PWM, 1000, i );
		wiced_pwm_init( RGB_G_PWM, 1000, j );
		wiced_pwm_init( RGB_B_PWM, 1000, k );
		wiced_pwm_start( RGB_R_PWM );
		wiced_pwm_start( RGB_G_PWM );
		wiced_pwm_start( RGB_B_PWM );

		if((i==color_R)&&(j==color_G)&&(k==color_B)){
			break;
		}
	}
}

void application_start( )
{
	/* Initialise the WICED device */
	wiced_init();

	while ( 1 )
	{
		RGB_change(rand()%256, rand()%256, rand()%256);

		wiced_rtos_delay_milliseconds(1000 );
	}
}



