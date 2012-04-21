/*
*  Arduino 1.0 sketch
*
*  trafficlight_vu_serial
*  A traffic light controller that acts as a VU meter
*  serial messages can also be passed to control the trafficlight remotely
*/

#include <TimerOne.h>

int pin_red = 9;
int pin_yellow = 10;
int pin_green = 11;

void setup(){
  pinMode(pin_red, OUTPUT);
  pinMode(pin_yellow, OUTPUT);
  pinMode(pin_green, OUTPUT);
  digitalWrite(pin_red, LOW);
  digitalWrite(pin_yellow, OUTPUT);
  digitalWrite(pin_green, OUTPUT);
}

void loop(){
  
}
