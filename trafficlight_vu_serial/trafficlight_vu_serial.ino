/*
*  Arduino 1.0 sketch
*
*  trafficlight_vu_serial
*  A traffic light controller that acts as a VU meter
*  serial messages can also be passed to control the trafficlight remotely
*/

#include "TimerOne.h"

#define PEAK_PERIOD    1000
#define PEAK_BUFFLEN   2

int pin_red = 9;
int pin_yellow = 10;
int pin_green = 11;
int pin_led = 13;
int pin_audio = A0;

int adc_value = 0;
int peak_short = 0;
int peak_long = 0;

int peak_counter = 0;
int peak_sum = 0;
int peak_avg = 0;
int peak_buff[PEAK_BUFFLEN];

int peak_long_old = 0;

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
  //Print the result of each peak-detected chunk.
  //(we will average this)
  if (peak_avg != peak_long_old){
    Serial.println(peak_avg);
    peak_long_old = peak_avg;
  }
}

void timerIsr(){
  adc_value = analogRead(pin_audio);
  //peak_short is *always* updated with the peak value
  if (adc_value > peak_short){
    peak_short = adc_value;
  }
  //after a certian timeout, reset peak_short
  //this defines the smaller chunk of time in which we find peaks
  if (++peak_counter > PEAK_PERIOD){
    peak_counter = 0;
    if (peak_long > peak_short){
      peak_long = peak_short;
    } else {
      peak_long = peak_long-((peak_long-peak_short)/4);
    }
    peak_long = peak_short;
    peak_short = 0;
    peak_sum = 0;
    int i;
    for (int i=1;i<PEAK_BUFFLEN;i++){
      peak_buff[i] = peak_buff[i-1];
      peak_sum += peak_buff[i-1];
    }
    peak_buff[0] = peak_long;
    peak_avg = (peak_sum+peak_long)/PEAK_BUFFLEN;
  }
  digitalWrite(pin_led, digitalRead(pin_led) ^ 1);
}
