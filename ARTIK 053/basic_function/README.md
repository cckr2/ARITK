# ARITK 053
Function of Control Pin
I don't finish Pin aliasing, so port number is number of File(ex.gpio53, pwm0)

## ADC
 - int analogInit(void);
 - analogRead(int port);
    1. port : 0 ~ 3(A0 ~ A3)
 - analogFinish(void);

## GPIO
 - void digitalWrite(int port, int value);
 - int digitalRead(int port);
    1. port : 30, 31, 32, 37, 38, 39, 40, 41, 42, 44, 45, 47, 48, 49, 50, 51, 52, 53, 54, 55
            : 57, 58, 59(Interrupt Pin, so Only Read)
    2. value : HIGH(1), LOW(0)

## PWM
 - int pwmInit(int port, int frequency, int duty);
 - void pwmSet(int port, int duty);
 - void pwmFinish(int port);
    1. port : 1 ~ 6
    2. duty : 0 ~ 100 (%)

