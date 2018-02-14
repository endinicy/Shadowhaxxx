#include "EQMon_ADS1115.h"

#include <Wire.h>

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */

// Global variable for the ADC conversion
// Used to pass value from interrupt routine to main routine
volatile int16_t adcValue;       // ADC conversion value
volatile int8_t newValueFlag=0;  // Flag to indicate new value, 0=False, 1=True
volatile int16_t intCount=0;     // Count the number of interrupts, testing purposes only

const float multiplier = 0.1875F;

// Interrupt routine to get ADC result
void readADC(void){
  //noInterrupts();
  adcValue = ads.getLastConversionResults();
  intCount++;
  newValueFlag = 1;
  //interrupts();
}



void setup(void){
  
  
  attachInterrupt(digitalPinToInterrupt(2), readADC, RISING);
  
  
  Serial.begin(9600);
  Serial.println("Setup Routine Start");
  
  Serial.println("Hello!");
  
  Serial.println("Getting differential reading from AIN0 (P) and AIN1 (N)");
  
  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                  ADS1115
  //                                                                  -------
   ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 0.1875mV
   //ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 0.125mV
   //ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 0.0625mV
   //ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.03125mV
   //ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.015625mV
   //ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.0078125mV

  //const float multiplier = 0.1875F;      /* ADS1115  @ +/- 6.144V gain 2/3x (16-bit results) */
  //float multiplier = 0.1250F;      /* ADS1115  @ +/- 4.096V gain 1    (16-bit results) */
  //float multiplier = 0.0625F;      /* ADS1115  @ +/- 2.048V gain 2    (16-bit results) */
  //float multiplier = 0.03125F;     /* ADS1115  @ +/- 1.024V gain 4    (16-bit results) */
  //float multiplier = 0.015625F;    /* ADS1115  @ +/- 0.512V gain 8    (16-bit results) */
  //float multiplier = 0.0078125F;   /* ADS1115  @ +/- 0.256V gain 16   (16-bit results) */

  
  ads.begin();

  ads.startContinuousADC_Differential_0_1();
  
  Serial.println("Started ADS1115");

  Serial.println("Setup Routine END");
}


void loop(void)
{
  
  int16_t results;
  int16_t intCountCopy;


  if(newValueFlag) {
    noInterrupts();
    results = adcValue;
    intCountCopy = intCount;
    newValueFlag = 0;
    interrupts();
    
    float mv = results * multiplier;
    Serial.print("mV = ");
    Serial.print(mv);
    Serial.print("interrupt counts = ");
    Serial.println(intCountCopy);
    }

  
  // Wait about 1/2 sample and loop again
  delayMicroseconds(2000);
}

