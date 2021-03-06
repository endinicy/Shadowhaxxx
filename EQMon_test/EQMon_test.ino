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

uint16_t readRegister(uint8_t i2cAddress, uint8_t reg) {
  Wire.beginTransmission(i2cAddress);
// Possible bug here, seems hard coded to CONVERT register
//  i2cwrite(ADS1115_REG_POINTER_CONVERT);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(i2cAddress, (uint8_t)2);
  return ((Wire.read() << 8) | Wire.read());  
}

void sendRegister(uint8_t i2cAddress, uint8_t reg) {
  Wire.beginTransmission(i2cAddress);
// Possible bug here, seems hard coded to CONVERT register
//  i2cwrite(ADS1115_REG_POINTER_CONVERT);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(i2cAddress, (uint8_t)2);
//  byte read1 = Wire.read();
  Serial.write(Wire.read());
  Serial.write(Wire.read());
//  Serial.write(read1);
}


// Interrupt routine to get ADC result
void readADC(void){
  //noInterrupts();
  // adcValue = ads.getLastConversionResults();
  intCount++;
  newValueFlag = 1;
  //interrupts();
}

char inputString="";
int startheard=0;


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

void sync(void){  // function that syncs with computer. waits to 
  while(Serial.available()){
    Serial.read();
  }
  while(true) {
    delay(100);
    Serial.print("g");
//    Serial.print(Serial.available());
    if(Serial.available() > 0){  
//      char inChar=(char)Serial.read();  // take the characters one at a time
//      if(inputString=='s'){  
        Serial.print("k");    // signal to computer to expect data
        delay(100);
        break;
//      }
    }
  }
}
    
uint16_t getConversion(void) {
  uint16_t res = readRegister(ADS_i2cAddress, ADS1115_REG_POINTER_CONVERT);
  return (int16_t)res;
}


void setup(void){
  
  
  //attachInterrupt(digitalPinToInterrupt(2), readADC, RISING);
  
  
  Serial.begin(115200);
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
}


void loop(void)
{  
  sync();

  attachInterrupt(digitalPinToInterrupt(2), readADC, RISING);
//    results = ads.getLastConversionResults();
//    choosePort(23);
//    Serial.println(results);
  unsigned long stopmicros = micros() + (30 * 1000000);
  while(micros() < stopmicros) {
//    Serial.print(stopmicros - micros());
//    results = ads.getLastConversionResults(); // THIS CAN BE OPTIMIZED, CONVERTING BYTES 
//    results = readRegister(ADS_i2cAddress, ADS1115_REG_POINTER_CONVERT);
    sendRegister(ADS_i2cAddress, ADS1115_REG_POINTER_CONVERT);
    choosePort(23);
    sendRegister(ADS_i2cAddress, ADS1115_REG_POINTER_CONVERT);
    choosePort(23);
//    Serial.println(results);
//    if((stopmicros - micros()) % 29 == 0) {
//      Serial.write(0x0000);
//      Serial.write(0x0000);
//    }
//    intCount++ 
  }
//  Serial.println(
}

