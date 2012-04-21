/*
*  Arduino 1.0 sketch
*
*  trafficlight_vu_serial
*  A traffic light controller that acts as a VU meter
*  serial messages can also be passed to control the trafficlight remotely
*/

#include "TimerOne.h"

#define ADC_BUFFLEN    10

int pin_red = 9;
int pin_yellow = 10;
int pin_green = 11;
int pin_led = 13;
int pin_audio = A0;

int adc_buff[ADC_BUFFLEN];
int adc_max = 0;
int peak_counter = 0;

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
  
  Timer1.initialize(2000);
  Timer1.attachInterrupt(timerIsr);
  
  Serial.begin(115200);
}

void loop(){
  if ((millis()%2000) == 0){
    adc_max = 0;
  }
  Serial.println(adc_max);
}

void timerIsr(){
  //For the moment, only using the first position of the adc buffer
  adc_buff[0] = analogRead(pin_audio);
  if (adc_buff[0] > adc_max){
    adc_max = adc_buff[0];  
  }
  digitalWrite(pin_led, digitalRead(pin_led) ^ 1);
}
