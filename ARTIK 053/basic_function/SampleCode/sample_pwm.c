#include <tinyara/config.h>
#include <stdio.h>
#include <unistd.h>

#define pin 5
int main(int argc, char *argv[])
{
	int i;

	pwmInit(pin, 1000, 0);

	while(1){
		for(i=0;i<100;i++){
			pwmSet(pin, i);
			up_mdelay(5);
		}
	}

	pwmFinish(pin);
	return 0;
}
