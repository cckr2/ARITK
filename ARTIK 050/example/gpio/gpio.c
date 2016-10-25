#include "wiced.h"


void gpio_input_handler(uint32_t *arg){ // GPIO �ڵ鷯
	wiced_bool_t input;
	input = wiced_gpio_input_get(WICED_GPIO_2);
	printf("gpio [%d] input : [%d]\n", WICED_GPIO_2, input);
}

void loop(){
	wiced_gpio_output_high(WICED_GPIO_1);
	printf("off\n");
	wiced_rtos_delay_milliseconds(1000);
	wiced_gpio_output_low(WICED_GPIO_1);
	printf("on\n");
	wiced_rtos_delay_milliseconds(1000);
}

void GPIO_test(void){
	uint8_t i;

	wiced_gpio_init(WICED_GPIO_1, OUTPUT_PUSH_PULL);
	wiced_gpio_init(WICED_GPIO_2, INPUT_PULL_UP);

	wiced_gpio_input_irq_enable(WICED_GPIO_2, IRQ_TRIGGER_RISING_EDGE,gpio_input_handler,NULL);
	wiced_gpio_output_high(WICED_GPIO_1);
	for(;;){
		//loop();
	}

	wiced_gpio_deinit(WICED_GPIO_1);
	wiced_gpio_input_irq_disable(WICED_GPIO_2);
	wiced_gpio_deinit(WICED_GPIO_2);
}

void application_start( )
{
	wiced_init();
	GPIO_test();
}



