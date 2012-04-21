/*
*  Arduino 1.0 sketch
*
*  trafficlight_vu_serial
*  A traffic light controller that acts as a VU meter
*  
*  On startup, this will continuously sample from analog pin A0 at 500Hz
*  A trafficlight with relays (ideally solid state) is connected to
*  pins dictated by pin_red, pin_yellow and pin_green.
*  The program will attempt to adapt to varying volume levels and
*  will flash the lights roughly in proportion to the signal level.
*
*  If any serial characters are written to the UART, the VU meter will be
*  suspended for a given timeout (SERIAL_TIMEOUT).  During this time, the 
*  traffic light can be controlled with a few simple serial commands.
*  The light will automatically revert to VU meter mode.
*/

#include "TimerOne.h"

//Defines for the VU meter
#define PEAK_PERIOD        1000  //Measured in samples at 500Hz
#define PEAK_BUFFLEN       5     //number of peak samples to average
#define TRAFFIC_HOLD       50    //Debounce time for the poor SCRs, in ms
#define TRAFFIC_HOLD_OFF   60    //OFF time debounce

//Defines for the serial port
#define SERIAL_TIMEOUT     10000  //Duration to wait before resuming VU mode
#define SERIAL_BUFFLEN     50

//Arduino pin definitions
int pin_red = 9;
int pin_yellow = 10;
int pin_green = 11;
int pin_led = 13;
int pin_audio = A0;

//relevant audio variables
int adc_value = 0;    //the most recent sample
int peak_avg = 0;     //the average peak (envelope) value

//variables internal to the envelope detection feature
int last_value = 0;
int peak_short = 0;
int peak_long = 0;
int peak_counter = 0;
int peak_sum = 0;
int peak_buff[PEAK_BUFFLEN];

//threshold values for the Green, Yellow and Red lights
int thresh0;
int thresh1;
int thresh2;

//time storage (millis)
unsigned long last_update = 0;
unsigned long last_serial = 0;

//Incoming serial message buffer
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
  
  //fire timerIsr every 2000us (500Hz)
  Timer1.initialize(2000);
  Timer1.attachInterrupt(timerIsr);
  
  Serial.begin(115200);
}

void loop(){
  //If a serial character is waiting, DO NOT run the VU meter
  //Clear the traffic light
  if (Serial.available()){
    digitalWrite(pin_red, LOW);
    digitalWrite(pin_yellow, LOW);
    digitalWrite(pin_green, LOW);    
    last_serial = millis();
    //Stay within this loop as long as serial data has been recieved
    //within SERIAL_TIMEOUT milliseconds
    while(millis() - last_serial < SERIAL_TIMEOUT){
      if (Serial.available()){
        last_serial = millis();
        //Read an entire line into buff[], and parse it.
        readLine();
        parseLine();
      }
    }
  }
  //When no serial data is waiting, update the lights for the VU meter
  doVUMeter();
}

void parseLine(){
  Serial.println(buff);
  switch (buff[0]){
  //(S)et a light (turn it on).  Eg: 'sy' will turn on yellow.
  case 'S':
  case 's':
    digitalWrite(findTarget(buff[1]), HIGH);
    return;
  //(C)lear a light (turn it off).  Eg: 'CG' will turn off green.
  case 'C':
  case 'c':
    digitalWrite(findTarget(buff[1]), LOW);
    return;
  //(Q)uit - return to VU meter mode immediately with no timeout
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
  return -1; 
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
  //peak_short is *always* updated with the maximum value
  if (adc_value > peak_short){
    peak_short = adc_value;
  }
  //at the end of each PEAK_PERIOD, calculate the weighted envelope (peak_avg)
  //also update the threshold values for this new value of peak_avg
  if (++peak_counter > PEAK_PERIOD){
    peak_counter = 0;
    doPeakCalc();
    findThresh();
  }
  //Toggle pin 13 - if the ISR starts falling behind, we will see distruptions
  //in the square wave on pin 13
  digitalWrite(pin_led, digitalRead(pin_led) ^ 1);
}

void doVUMeter(){
  //Skip any 0 (read: probably negative) values immediately
  if (adc_value == 0){
    return;
  }
  //If we last changed state within TRAFFIC_HOLD, abort immediately (trying to stay lean)
  if (millis() - last_update < TRAFFIC_HOLD){
    return;
  }
  last_value = adc_value;
  //Enable for serial debug of current value VS peak value
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
  thresh0 = (int)((double)peak_avg)*0.08;
  thresh1 = (int)((double)peak_avg)*0.25;
  thresh2 = (int)((double)peak_avg)*0.50;
}

void doPeakCalc(){
    //If the new peak is greater than the last, do not weight
    //This keeps the VU responsive to loudness
    if (peak_short > peak_long){
      peak_long = peak_short;
    //If the new peak is falling, slow the rate of descent (by 1/4)
    } else {
      peak_long = peak_long-((peak_long-peak_short)/4);
    }
    //Reset the short-term peak so it can accumulate over the next period
    peak_short = 0;
    //average the last PEAK_BUFFLEN values into peak_avg
    peak_sum = 0;
    int i;
    for (int i=1;i<PEAK_BUFFLEN;i++){
      peak_buff[i] = peak_buff[i-1];
      peak_sum += peak_buff[i-1];
    }
    peak_buff[0] = peak_long;
    peak_avg = (peak_sum+peak_long)/PEAK_BUFFLEN;
}
