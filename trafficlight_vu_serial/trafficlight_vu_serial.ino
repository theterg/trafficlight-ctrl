/*
*  Arduino 1.0 sketch
*
*  trafficlight_vu_serial
*  A traffic light controller that acts as a VU meter
*  serial messages can also be passed to control the trafficlight remotely
*/

#include "TimerOne.h"

#define PEAK_PERIOD        1000  //Measured in samples at 500Hz
#define PEAK_BUFFLEN       5     //number of peak samples to average
#define TRAFFIC_HOLD       50    //Debounce time for the poor SCRs, in ms
#define TRAFFIC_HOLD_OFF   60    //OFF time debounce

#define SERIAL_TIMEOUT     10000  //Duration to wait before resuming VU mode
#define SERIAL_BUFFLEN     200

int pin_red = 9;
int pin_yellow = 10;
int pin_green = 11;
int pin_led = 13;
int pin_audio = A0;

int adc_value = 0;
int last_value = 0;
int peak_short = 0;
int peak_long = 0;

int peak_counter = 0;
int peak_sum = 0;
int peak_avg = 0;
int peak_buff[PEAK_BUFFLEN];

int thresh0;
int thresh1;
int thresh2;
unsigned long last_update = 0;
unsigned long last_serial = 0;

int peak_long_old = 0;

char buff[SERIAL_BUFFLEN];

void setup(){
  //Configure necessary I/O pins
  pinMode(pin_red, OUTPUT);
  pinMode(pin_yellow, OUTPUT);
  pinMode(pin_green, OUTPUT);
  pinMode(pin_led, OUTPUT);
  digitalWrite(pin_red, LOW);
  digitalWrite(pin_yellow, LOW);
  digitalWrite(pin_green, LOW);
  digitalWrite(pin_led, LOW);
  
  Timer1.initialize(2000);
  Timer1.attachInterrupt(timerIsr);
  
  Serial.begin(115200);
}

void loop(){
  if (Serial.available()){
    digitalWrite(pin_red, LOW);
    digitalWrite(pin_yellow, LOW);
    digitalWrite(pin_green, LOW);    
    last_serial = millis();
    while(millis() - last_serial < SERIAL_TIMEOUT){
      if (Serial.available()){
        last_serial = millis();
        readLine();
        parseLine();
      }
    }
  }
  doVUMeter();
}

void parseLine(){
  Serial.println(buff);
  switch (buff[0]){
  case 'S':
  case 's':
    digitalWrite(findTarget(buff[1]), HIGH);
    return;
  case 'C':
  case 'c':
    digitalWrite(findTarget(buff[1]), LOW);
    return;
  case 'Q':
  case 'q':
    last_serial = 0;
    return;
  }
}

int findTarget(char c){
  switch(c){
  case 'R':
  case 'r':
    return pin_red;
  case 'Y':
  case 'y':
    return pin_yellow;
  case 'G':
  case 'g':
    return pin_green;
  }
  return 14;
}

void readLine(){
  memset(buff, '\0', SERIAL_BUFFLEN);
  int idx = 0;
  int c;
  do {
    c = Serial.read();
    if ((c > 0) && (c != '\r') && (c != '\n')){
      buff[idx++] = (char)c;
    }
  } while (c != '\n');
}

void timerIsr(){
  adc_value = analogRead(pin_audio);
  //peak_short is *always* updated with the peak value
  if (adc_value > peak_short){
    peak_short = adc_value;
  }
  //at the end of each PEAK_PERIOD, calculate the weighted average
  //peak_avg (see below) will be used as the upper bound for the trafficlight.
  if (++peak_counter > PEAK_PERIOD){
    doPeakCalc();
    findThresh();
  }
  digitalWrite(pin_led, digitalRead(pin_led) ^ 1);
}

void doVUMeter(){
  if (adc_value == 0){
    return;
  }
  //If the value is greater than the last, go through
  //if ((adc_value < last_value)&&(millis() - last_update < TRAFFIC_HOLD)){
  if (millis() - last_update < TRAFFIC_HOLD){
    return;
  }
  last_value = adc_value;
  //Serial.print(adc_value);
  //Serial.print(" < ");
  //Serial.println(peak_avg);
  last_update = millis();
  if ((adc_value < thresh0)){
    //all off
    digitalWrite(pin_red, LOW);
    digitalWrite(pin_yellow, LOW);
    digitalWrite(pin_green, LOW);
  } else if (adc_value < thresh1){
    //only green
    digitalWrite(pin_red, LOW);
    digitalWrite(pin_yellow, LOW);
    digitalWrite(pin_green, HIGH);
  } else if (adc_value < thresh2){
    //green + yellow
    digitalWrite(pin_red, LOW);
    digitalWrite(pin_yellow, HIGH);
    digitalWrite(pin_green, HIGH);
  } else {
    //all on
    digitalWrite(pin_red, HIGH);
    digitalWrite(pin_yellow, HIGH);
    digitalWrite(pin_green, HIGH);
  }
}

void findThresh(){
  //Truncate the number back to an integer for comparison...
  thresh0 = (int)((double)peak_avg)*0.080;
  thresh1 = (int)((double)peak_avg)*0.250;
  thresh2 = (int)((double)peak_avg)*0.50;
}

void doPeakCalc(){
    peak_counter = 0;
    if (peak_short > peak_long){
      peak_long = peak_short;
    } else {
      peak_long = peak_long-((peak_long-peak_short)/4);
    }
    //peak_long = peak_short;
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
