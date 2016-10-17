#include <stdio.h>
#include "wiced.h"

void application_start( )
{
	int i, check = 0;;
	int max[3],min[3];
	uint8_t txb[3][3];
	uint8_t rxb[3][3];
	uint16_t readValue[3];

	wiced_spi_device_t mcp3208_spi;
	wiced_spi_message_segment_t mcp3208_msg[3];

	for(i=0;i<3;i++){
		txb[i][0] = (0x06 | ((i & 0x07) >> 2));
		txb[i][1] = ((i & 0x07) << 6);
		txb[i][2] = 0x0;
		max[i] = 0;
		min[i] = 10000;
	}
	wiced_init();

	mcp3208_spi.port = WICED_SPI_1;
	mcp3208_spi.chip_select = WICED_GPIO_22;
	mcp3208_spi.speed = 1000000;
	mcp3208_spi.mode = (SPI_CLOCK_RISING_EDGE | SPI_CLOCK_IDLE_HIGH | SPI_NO_DMA | SPI_MSB_FIRST);
	mcp3208_spi.bits = 8;

	wiced_spi_init(&mcp3208_spi);
	WPRINT_APP_INFO(("Get Joystick Location & MCM3208 ADC\n"));


	while( 1 ){
		check = 0;
		for(i=0;i<3;i++){
			memset(&mcp3208_msg[i],0,sizeof(mcp3208_msg[i]));
			mcp3208_msg[i].tx_buffer = txb[i];
			mcp3208_msg[i].rx_buffer = rxb[i];
			mcp3208_msg[i].length = sizeof(txb[i]);
			wiced_spi_transfer(&mcp3208_spi, &mcp3208_msg[i],1);
			readValue[i] = (((rxb[i][1] & 0x0F) << 8) | rxb[i][2]);
			wiced_rtos_delay_milliseconds( 10 );
		}
		for(i=0;i<3;i++){
			if(readValue[i]>max[i]){
				max[i] = readValue[i];
				check = 1;
			}
			if(readValue[i]<min[i]){
				min[i] = readValue[i];
				check = 1;
			}
			if(check){
				if(i==0)
					printf("z :");
				else if(i==1)
					printf("y :");
				else if(i==2)
					printf("z :");
				printf(" min : %d, max : %d\n", min[i], max[i]);
			}
		}
	}

	wiced_spi_deinit(&mcp3208_spi);

}
