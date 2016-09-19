	#include <stdio.h>
	#include "wiced.h"
	#define CHANNEL 0

	void application_start( )
	{
		uint8_t txb[3];
		uint8_t rxb[3];
		uint16_t readValue=0;

		wiced_spi_device_t mcp3208_spi;
		wiced_spi_message_segment_t mcp3208_msg;

		txb[0] = (0x06 | ((CHANNEL & 0x07) >> 2));
		txb[1] = ((CHANNEL & 0x07) << 6);
		txb[2] = 0x0;

		wiced_init();

		mcp3208_spi.port = WICED_SPI_1;
		mcp3208_spi.chip_select = WICED_GPIO_22;
		mcp3208_spi.speed = 1000000;
		mcp3208_spi.mode = (SPI_CLOCK_RISING_EDGE | SPI_CLOCK_IDLE_HIGH | SPI_NO_DMA | SPI_MSB_FIRST);
		mcp3208_spi.bits = 8;

		wiced_spi_init(&mcp3208_spi);
		WPRINT_APP_INFO(("Get Joystick Location & MCM3208 ADC\n"));

		wiced_gpio_init(WICED_GPIO_10,OUTPUT_PUSH_PULL);
		wiced_gpio_output_high(WICED_GPIO_10);

		while( 1 ){
			memset(&mcp3208_msg,0,sizeof(mcp3208_msg));
			mcp3208_msg.tx_buffer = txb;
			mcp3208_msg.rx_buffer = rxb;
			mcp3208_msg.length = sizeof(txb);

			wiced_spi_transfer(&mcp3208_spi, &mcp3208_msg,CHANNEL+1);
			readValue = (((rxb[1] & 0x0F) << 8) | rxb[2]);

			printf("brightness: %d, %f(V)\n", readValue, ((3.3/4096) * readValue));

			wiced_rtos_delay_milliseconds( 100 );
		}

		wiced_spi_deinit(&mcp3208_spi);
		wiced_gpio_deinit(WICED_GPIO_10);
	}

