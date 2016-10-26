#include "wiced.h"

#define MAXTIMINGS 85
#define DHTPIN WICED_GPIO_1

int dht11_data[] = {0,0,0,0,0};
static int read_dht11_data();

void application_start( )
{
	wiced_init();
	WPRINT_APP_INFO(("Temperature & Humidity\n"));

	while(1){
		read_dht11_data();
		wiced_rtos_delay_milliseconds(2000);
	}
}

static int read_dht11_data(){
	uint8_t laststate = 1;
	uint8_t counter = 9;
	uint8_t i,j = 0;

	dht11_data[0] = dht11_data[1] = dht11_data[2] = dht11_data[3] = dht11_data[4] = 0;

	/* Single-Wire Two-Way Communication */
	wiced_gpio_init(DHTPIN, OUTPUT_PUSH_PULL);

	/* MCU->DHT, start signal */
	wiced_gpio_output_high(DHTPIN);
	wiced_rtos_delay_milliseconds(10);

	/* MCU pull down, DHT detect a start signal from MCU */
	wiced_gpio_output_low(DHTPIN);
	wiced_rtos_delay_milliseconds(20);

	/* MCU pull up to receive response signal from DHT */
	wiced_gpio_output_high(DHTPIN);
	wiced_rtos_delay_microseconds(30);

	/* MCU read response data consist of temperature and humidity */
	wiced_gpio_init(DHTPIN, INPUT_PULL_UP);

	for(i=0; i<MAXTIMINGS; i++){
		counter = 0;
		while(wiced_gpio_input_get(DHTPIN)==laststate){
			counter++;
			wiced_rtos_delay_microseconds(1);
			if(counter == 255)
				break;
		}

		laststate = wiced_gpio_input_get(DHTPIN);

		if(counter == 255)
			break;

		if((i >= 4) && (i % 2 == 0)){
			dht11_data[j/8] <<= 1;
			//dht11_data[j/8] >>= 1;

			if(counter > 30)
				dht11_data[j/8] |= 1;
				//dht11_data[j/8] |= 128;
			j++;
		}
	}


	if((j >= 40) && (dht11_data[4] == ((dht11_data[0] + dht11_data[1] + dht11_data[2] + dht11_data[3]) & 0xFF))){
		float t, h;
		h = (float)dht11_data[0] + ((float)dht11_data[1]/100);
		t = (float)dht11_data[2] + ((float)dht11_data[3]/100);
		printf("Humidity = %.2f %% Temperature = %.2f *C\n",h,t);
		return 1;
	}else{
		printf("Data not good, skip (read bytes: %d).\n",j);
		return 0;
	}
}
