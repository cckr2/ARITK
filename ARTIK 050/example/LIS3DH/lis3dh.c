#include <stdio.h>
#include "wiced.h"

#define LIS3DH_CTRL_REG1 0x20
#define LIS3DH_CTRL_REG2 0x21
#define LIS3DH_CTRL_REG3 0x22
#define LIS3DH_CTRL_REG4 0x23
#define LIS3DH_CTRL_REG5 0x24
#define LIS3DH_CTRL_REG6 0x25
#define LIS3DH_STATUS_REG 0x27
#define LIS3DH_OUT_X_L 0x28
#define LIS3DH_OUT_X_H 0x29
#define LIS3DH_OUT_Y_L 0x2A
#define LIS3DH_OUT_Y_H 0x2B
#define LIS3DH_OUT_Z_L 0x2C
#define LIS3DH_OUT_Z_H 0x2D


void init_lis3dh();
uint8_t tx_buffer[2];
uint8_t rx_buffer[2];
uint16_t readValue[3];

wiced_spi_device_t lis3dh_spi;
wiced_spi_message_segment_t lis3dh_msg;
void application_start( )
{


	wiced_init();

	lis3dh_spi.port = WICED_SPI_1;
	lis3dh_spi.chip_select = WICED_GPIO_22;
	lis3dh_spi.speed = 100000; //4Mhz
	lis3dh_spi.mode = (SPI_CLOCK_RISING_EDGE | SPI_CLOCK_IDLE_HIGH | SPI_NO_DMA | SPI_MSB_FIRST);
	lis3dh_spi.bits = 16;

	wiced_spi_init(&lis3dh_spi);

	init_lis3dh();
	for(;;){
		read_Data();
	}

//	wiced_spi_deinit(&lis3dh_spi);

}

void init_lis3dh() {
	write_register(LIS3DH_CTRL_REG1, 0x57); //All axes, normal, 100Hz
	write_register(LIS3DH_CTRL_REG2, 0x00); // No HighPass filter
	write_register(LIS3DH_CTRL_REG3, 0x00); // No interrupts
	write_register(LIS3DH_CTRL_REG4, 0x0); // all defaults
	write_register(LIS3DH_CTRL_REG5, 0x0); // all defaults
	write_register(LIS3DH_CTRL_REG6, 0x0); // all defaults
}

void write_register (uint8_t reg, uint8_t value)
{
	memset(tx_buffer,0,sizeof(tx_buffer));
	tx_buffer[0] = reg & ~0xc0;		//reg & 0011 1111
	//tx_buffer[0] = 0xAA;
	tx_buffer[1] = value;

	memset(&lis3dh_msg,0,sizeof(lis3dh_msg));
	lis3dh_msg.tx_buffer = tx_buffer;
	lis3dh_msg.rx_buffer = rx_buffer;
	lis3dh_msg.length = sizeof(tx_buffer);

	wiced_spi_transfer(&lis3dh_spi, &lis3dh_msg,1);
	wiced_rtos_delay_milliseconds( 1 );

}

uint8_t read_register ( uint16_t reg )
{
	memset(tx_buffer,0,sizeof(tx_buffer));
	memset(rx_buffer,0,sizeof(rx_buffer));
	tx_buffer[0] = reg | 0x80;  //0x80 is first bit and read bit, second bit is multiple reading set bit
	tx_buffer[1] = 0;

	memset(&lis3dh_msg,0,sizeof(lis3dh_msg));
	lis3dh_msg.tx_buffer = tx_buffer;
	lis3dh_msg.rx_buffer = rx_buffer;
	lis3dh_msg.length = sizeof(tx_buffer);

	wiced_spi_transfer(&lis3dh_spi, &lis3dh_msg,1);
	wiced_rtos_delay_microseconds( 10 );

	return rx_buffer[1];
}

void read_Data()
{
	int16_t x,y,z;
	double dx, dy, dz;
	while(read_register(LIS3DH_STATUS_REG) & 0x8 == 0 ){

	};
	x = ((read_register(LIS3DH_OUT_X_H) << 8 ) | read_register(LIS3DH_OUT_X_L));
	y = ((read_register(LIS3DH_OUT_Y_H) << 8 ) | read_register(LIS3DH_OUT_Y_L));
	z = ((read_register(LIS3DH_OUT_Z_H) << 8 ) | read_register(LIS3DH_OUT_Z_L));
	dx = x/163.0;
	dy = y/163.0;
	dz = z/163.0;
	wiced_rtos_delay_milliseconds( 500 );
	printf("X: %f; Y: %f; Z: %f; sum: %f; shock: %f\n\r", dx , dy, dz, dx+dy+ dz,sqrt(dx*dx + dy*dy + dz*dz));
	}
