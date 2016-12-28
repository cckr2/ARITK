/*  Constructor's parameters are 'Pin Number' and 'Duty Cycle'
 *  'Duty Cycle' is 0 to 100
 *  also Function 'setDuty' has parameter that Duty Cycle(0 to 100).
 */

#include <DebugSerial.h>
#include <SoftPWM.h>
#define R 2
#define G 3
#define B 4

SoftPWM Red,Green,Blue;

void setColor(int color_R, int color_G,int color_B){
    int color_Red = (color_R + 1) * 100 / 255;
    int color_Green = (color_G + 1) * 100 / 255;
    int color_Blue = (color_B + 1) * 100 / 255;
    Red.setDuty(color_Red);  //Change Duty Cycle(0~100(%))
    Green.setDuty(color_Green);
    Blue.setDuty(color_Blue);
}

void setup() {
  // put your setup code here, to run once:
  DebugSerial.begin(115200);
  Red = SoftPWM(R, 0);    //Init Pin Number and Duty Cycle(0~100(%))
  Green = SoftPWM(G, 0);
  Blue = SoftPWM(B, 0);
}

void loop() {
    setColor(random(255),random(255),random(255));
    delay(1000);
}
