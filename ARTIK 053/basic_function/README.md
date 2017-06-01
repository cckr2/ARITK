# ARITK 053
## 1. summary
    1. Function of Control Pin.
    2. I don't finish Pin aliasing, so port number is file's number(ex.gpio53, pwm0).

## 2. How to use
	1. Copy two File(basic_function.c, basic_function.h) into your's project
	2. Use Function
	3. You want to test samplecode, copy only source code in main.c

## 3. Fumction and parameter
### ADC
 - int analogInit(void);
 - analogRead(int port);
    1. port : 0 ~ 3(A0 ~ A3)
 - analogFinish(void);

### GPIO
 - void digitalWrite(int port, int value);
 - int digitalRead(int port);
    1. port : 30, 31, 32, 37, 38, 39, 40, 41, 42, 44, 45, 47, 48, 49, 50, 51, 52, 53, 54, 55
            : 57, 58, 59(Interrupt Pin, so Only Read)
    2. value : HIGH(1), LOW(0)

### PWM
 - int pwmInit(int port, int frequency, int duty);
 - void pwmSet(int port, int duty);
 - void pwmFinish(int port);
    1. port : 1 ~ 6
    2. duty : 0 ~ 100 (%)

