#include <tinyara/config.h>
#include <stdio.h>
#include <unistd.h>

#define pin 45

int main(int argc, char *argv[])
{
	for(;;){
		digitalWrite(pin,HIGH);
		up_mdelay(1000);

		digitalWrite(pin,LOW);
		up_mdelay(1000);
	}
	return 0;
}
