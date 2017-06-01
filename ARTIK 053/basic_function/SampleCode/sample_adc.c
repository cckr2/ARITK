#include <tinyara/config.h>
#include <stdio.h>
#include "basic_function/basic_function.h"
int main(int argc, char *argv[])
{
	analogInit();

	for(;;){
		printf("Read ADC Pin Value: %d\n", analogRead(0));
	}

	analogFinish();
	return 0;
}
