#ifdef __cplusplus
#ifndef SoftPWM_H_
#define SoftPWM_H_
#include <Arduino.h>
#include <pthread.h>
#include <unistd.h>
#define SoftPWM_DELAY 600  //ms

class SoftPWM {
protected:
	static void *Handler(void* arg2);
public:
	SoftPWM();
	SoftPWM(int pin_number, int duty_cycle);
	~SoftPWM();
	void setDuty(int duty_cycle);
	void sleep(int time){
		usleep(1000 * time);
	}

private:
	pthread_t pwmThread;
	int* arg;
};

#endif
#endif