#include <Arduino.h>
#include <pthread.h>
#include "SoftPWM.h"

void * SoftPWM::Handler(void* arg2) {
	int duty;
	int pin = ((int*)arg2)[0];
	while (true){
		duty = SoftPWM_DELAY * ((int*)arg2)[1]/100;
		digitalWrite(((int*)arg2)[0], HIGH);
		delayMicroseconds(duty);
		digitalWrite(((int*)arg2)[0], LOW);
		delayMicroseconds(SoftPWM_DELAY - duty);
	}
}

SoftPWM::SoftPWM(int pin_number, int duty_cycle) {
	this->arg = new int[2];
	this->arg[0] = pin_number;
	this->arg[1] = duty_cycle;
	pthread_create(&pwmThread, NULL, &SoftPWM::Handler, (void*)arg);
	pthread_detach(pwmThread);

	pinMode(arg[0], OUTPUT);
}

SoftPWM::SoftPWM() {

}
void SoftPWM::setDuty(int duty_cycle){
	this->arg[1] = duty_cycle;
}

SoftPWM::~SoftPWM() {
	pthread_cancel(pwmThread);
}