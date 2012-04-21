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
int pin_led = 13;

void setup(){
  //Configure necessary I/O pins
  pinMode(pin_red, OUTPUT);
  pinMode(pin_yellow, OUTPUT);
  pinMode(pin_green, OUTPUT);
  pinMode(pin_led, OUTPUT);
  digitalWrite(pin_red, HIGH);
  digitalWrite(pin_yellow, HIGH);
  digitalWrite(pin_green, HIGH);
  digitalWrite(pin_led, LOW);
  
  
}

void loop(){
  
}
