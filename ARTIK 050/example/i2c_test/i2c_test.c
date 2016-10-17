	#include <stdio.h>
	#include "wiced.h"
void application_start( )
	{
		uint8_t buffer[7];

		wiced_init();

		wiced_i2c_device_t s_gac9;
		s_gac9.port = WICED_I2C_1;
		s_gac9.address = 0x68; //1101000
		s_gac9.address_width = I2C_ADDRESS_WIDTH_7BIT;
		s_gac9.flags = 1;
		s_gac9.speed_mode = I2C_STANDARD_SPEED_MODE;
		wiced_i2c_init(&s_gac9);

		int16_t ax,ay,az;
int i;
		while( 1 ){
			memset(&buffer,0,sizeof(buffer));
			wiced_i2c_read(&s_gac9,WICED_I2C_START_FLAG,buffer,sizeof(buffer));

//			ax = (((int16_t)buffer[0]) << 8) | buffer[1];
//			ay = (((int16_t)buffer[2]) << 8) | buffer[3];
//			az = (((int16_t)buffer[4]) << 8) | buffer[5];

			for(i=0;i<7;i++)
				printf("b%d : %d ",i,buffer[i]);
			printf("\n");
			wiced_rtos_delay_milliseconds( 100 );
		}

		wiced_i2c_deinit(&s_gac9);
	}

