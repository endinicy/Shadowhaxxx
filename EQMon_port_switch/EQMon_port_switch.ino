#include "EQMon_ADS1115.h"

#include <Wire.h>

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
const uint8_t ADS_i2cAddress=0x48;

// Global variable for the ADC conversion
// Used to pass value from interrupt routine to main routine
volatile int16_t adcValue;       // ADC conversion value
volatile int8_t newValueFlag=0;  // Flag to indicate new value, 0=False, 1=True
volatile int16_t intCount=0;     // Count the number of interrupts, testing purposes only
volatile unsigned long timeLastSample;


volatile  int16_t results;
volatile  int16_t intCountCopy;

volatile int8_t port=01;

unsigned long lasttime=micros();
unsigned long sampledelay=0;

const float multiplier = 0.1875F;

volatile uint16_t config_0_1;
volatile uint16_t config_2_3;

void writeReg(uint8_t i2cAddress, uint8_t reg, uint16_t value) {
  Wire.beginTransmission(i2cAddress);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)(value>>8));
  Wire.write((uint8_t)(value & 0xFF));
  Wire.endTransmission();
}

// Interrupt routine to get ADC result
void readADC(void){
  //noInterrupts();
  // adcValue = ads.getLastConversionResults();
  intCount++;
  newValueFlag = 1;
  //interrupts();
}

void choosePort(uint8_t newport) {
  if(newport==01){
    writeReg(ADS_i2cAddress, ADS1115_REG_POINTER_CONFIG, config_0_1);
    port=01;
  }
  else if(newport==23){
    writeReg(ADS_i2cAddress, ADS1115_REG_POINTER_CONFIG, config_2_3);
    port=23;
  }
}

void setup(void){
  
  
  //attachInterrupt(digitalPinToInterrupt(2), readADC, RISING);
  
  
  Serial.begin(9600);
  Serial.println("Setup Routine Start");
  
  Serial.println("Hello!");
  
  Serial.println("Getting differential reading from AIN0 (P) and AIN1 (N)");
    
  ads.begin();

  ads.startContinuousADC_Differential_0_1();

  
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

  config_0_1 = ADS1115_REG_CONFIG_CQUE_1CONV   | // Ready pin active after 1 conversion
              ADS1115_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val) No function in continuous ADC mode
              ADS1115_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
              ADS1115_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val) No function in continuous ADC mode
              ADS1115_REG_CONFIG_DR_250SPS    | // 250 samples per second 
              ADS1115_REG_CONFIG_MODE_CONTIN  |   // Continuous mode
              ADS1115_REG_CONFIG_MUX_DIFF_0_1 |
              ADS1115_REG_CONFIG_OS_SINGLE;

  config_2_3 = ADS1115_REG_CONFIG_CQUE_1CONV   | // Ready pin active after 1 conversion
              ADS1115_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val) No function in continuous ADC mode
              ADS1115_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
              ADS1115_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val) No function in continuous ADC mode
              ADS1115_REG_CONFIG_DR_250SPS    | // 250 samples per second 
              ADS1115_REG_CONFIG_MODE_CONTIN  |   // Continuous mode
              ADS1115_REG_CONFIG_MUX_DIFF_2_3 |
              ADS1115_REG_CONFIG_OS_SINGLE;
  
  Serial.println("Started ADS1115");
  Serial.println("Setup Routine END");
}


void loop(void)
{  
  if(micros()-timeLastSample > 00000) {
    sampledelay=micros()-timeLastSample;
    Serial.println(sampledelay);
//    Serial.print(":  ");

    results = ads.getLastConversionResults();
    if(port==01){
      choosePort(23);
      Serial.println("port 2-3");
    }
    else if(port==23){
      choosePort(01);
      Serial.println("port 0-1");
    }
    
    timeLastSample=micros();
    //Serial.print("  ");
    Serial.println(results);
    }

  
  // Wait about 1/2 sample and loop again
  //delayMicroseconds(1000);
}

