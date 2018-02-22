

#include "EQMon_ADS1115.h"  // This is an ADS1115 library that I found on the internet. Don't know why it works so much better than the vanilla one
#include <Wire.h> // This includes all of the i2c 'wire' commands, used by "EQMon_ADS1115.h" 

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
const uint8_t ADS_i2cAddress = 0x48;

volatile int16_t adcValue;       // variable to store ADC conversion outputs
volatile int8_t restartFlag = 0;  // Flag to indicate restart signal
volatile int8_t newValueFlag = 0; // Flag to indicate ADC conversion register has a new value to read, 0=False, 1=True
volatile unsigned long timeLastSample;  // variable to test how long it has been since the last read of conversion register

volatile int8_t nextport = 01; // variable to keep track of which ADC port to convert

volatile uint16_t config_0_1; // the config register value for taking continuous differential measurements between ports 0 and 1
volatile uint16_t config_2_3; // this one is for ports 2 and 3, these config values are defined in the void(setup)

char inputString = "";  // variable to store serial input values 


//=============================================================

/*
    FUNCTIONS START
*/

void writeReg(uint8_t i2cAddress, uint8_t reg, uint16_t value) {  // function to write 'value' to register # 'reg' at i2c address 'i2cAddress'0
  Wire.beginTransmission(i2cAddress);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)(value >> 8));
  Wire.write((uint8_t)(value & 0xFF));
  Wire.endTransmission();
}

uint16_t readRegister(uint8_t i2cAddress, uint8_t reg) {  // function to read 2 bytes from register # 'reg' at i2c address 'i2cAddress'0
  Wire.beginTransmission(i2cAddress);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(i2cAddress, (uint8_t)2);
  return ((Wire.read() << 8) | Wire.read());  
}

void sendRegister(uint8_t i2cAddress, uint8_t reg) {  // function writes 2 bytes from register # 'reg' at i2c address 'i2cAddres' STRAIGHT TO THE SERIAL BUFFER
  Wire.beginTransmission(i2cAddress);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(i2cAddress, (uint8_t)2);
  Serial.write(Wire.read());
  Serial.write(Wire.read());
}

void readADC(void) { // Interrupt routine to set read flag. (reading not performed in interrupt because printing and long operations tend to break here)
  //noInterrupts();
  newValueFlag++;
  //interrupts();
}

void restartInterrupt(void) { // Interrupt routine to set restart flag.
  //noInterrupts();
  restartFlag = 1;
  //interrupts();
}

void choosePort(uint8_t newport) {  // function changes config settings for next conversion. 
  if (newport == 01) {
    writeReg(ADS_i2cAddress, ADS1115_REG_POINTER_CONFIG, config_0_1); // continuous differential between ports 0 and 1
    nextport = 23;  // after reading 0-1, read 2-3 next
  }
  else if (newport == 23) {
    writeReg(ADS_i2cAddress, ADS1115_REG_POINTER_CONFIG, config_2_3); // continuous differential between ports 2 and 3
    nextport = 01;  // after reading 2-3, read 0-1 next
  }
}

void sync(void) { // function that syncs with computer. sends 'g' ready signal until recieving anything, then sends 'k' before starting transmission
  while (Serial.available()) {  // this loop clears the input buffer
    Serial.read();  // throwing away items in buffer
  }
  while (true) {

    Serial.print("g");
    delay(100);
    //    Serial.print(Serial.available());
    if (Serial.available() > 0) {
      //      char inChar=(char)Serial.read();  // take the characters one at a time
      //      if(inputString=='s'){
      delay(100);
      Serial.print("k");    // signal to computer to expect data

        while (Serial.available()) {  // this loop clears the input buffer
          Serial.read();  // throwing away items in buffer
        }
      break; 
      //      }
    }
  }
}



//==============================================================================


void setup(void) {

  Serial.begin(115200); // start serial communication with computer
  Wire.begin(); // start i2c communication

  ads.startContinuousADC_Differential_0_1();

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

  choosePort(nextport); // start with continuous differential conversian between ports 0 and 1
  attachInterrupt(digitalPinToInterrupt(2), readADC, RISING); // setting up the new conversion interrupt to pin 2
  // attachInterrupt(digitalPinToInterrupt(3), restartInterrupt, RISING); // setting up the restart interrupt to pin 3
}


void loop(void)
{
  sync(); // running the sync code
  
  // unsigned long stopmicros = micros() + (30 * 1000000); // runs for 30 seconds 
  newValueFlag = 0;
  // restartFlag = 0;
  
  while (Serial.available() == 0) {
    if (newValueFlag == 1) {
      sendRegister(ADS_i2cAddress, ADS1115_REG_POINTER_CONVERT);
      choosePort(nextport);
      newValueFlag = 0;
    }
    else if (newValueFlag > 1) {
      Serial.print("WOWWWEEEEE!!!");
      break;
    }
    // else if (restartFlag > 0){
    //   break;
    // }
    //    Serial.println(results);
    //    if((stopmicros - micros()) % 29 == 0) {
    //      Serial.write(0x0000);
    //      Serial.write(0x0000);
    //    }
    //    intCount++
  }
  //  Serial.println(
}

