#include "EQMon_ADS1115.h"

#include <Wire.h>

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */

// Global variable for the ADC conversion
// Used to pass value from interrupt routine to main routine
volatile int8_t newValueFlag=0;  // Flag to indicate new value, 0=False, 1=True
volatile int16_t intCount=0;     // Count the number of interrupts, testing purposes only


int lasttime=micros();
int sampledelay=0;

void readADC(void){
  //noInterrupts();
  intCount++;
  newValueFlag++;
  //interrupts();
}



void setup(void){
  
  
  attachInterrupt(digitalPinToInterrupt(2), readADC, RISING);
  
  
  Serial.begin(115200);
  Serial.println("Setup Routine Start");
  
  Serial.println("Hello!");
  
  ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 0.1875mV

  ads.begin();
  ads.startContinuousADC_Differential_0_1();
  
  Serial.println("Started ADS1115");
  Serial.println("Setup Routine END");
}


void loop(void)
{
  if(newValueFlag) {
    noInterrupts();
    if(newValueFlag == 1){
      sampledelay=micros()-lasttime;
      lasttime=micros();
      Serial.println(sampledelay);
    }
    else{
      Serial.println("OH NOES WE MISSED ONE!!!!!");
    }
    newValueFlag = 0;
    interrupts();

    }
  // LOOKS LIKE THE TIMING IS ACTUALLY LIKE 1116.
  // I'M GETTING SOME 92'S AND SOME MISSED ONES
}

