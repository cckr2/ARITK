# ARITK 053
Function of Control Pin

## ADC
 - int analogInit(void);
 
   1.
 - analogRead(int port);
 - analogFinish(void);

## GPIO
 - void digitalWrite(int port, int value);  //Available GPIO Pin Number : 39, 41, 52, 53, 54, 55
 - int digitalRead(int port);

## PWM
 - int pwmInit(int port, int frequency, int duty);
 - void pwmSet(int port, int duty);
 - void pwmFinish(int port);


