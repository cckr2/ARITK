#include <tinyara/config.h>

#define pin 42


int main(int argc, char *argv[])
{

	for(;;){
		printf("Read Port %d Value : %d\n", pin, digitalRead(pin));
		up_mdelay(1000);
	}

	return 0;
}
