/**************************************************************************/
/*!
    @file     EQMon_ADS1015.cpp
    Changed name from Adafruit_ADS1015.cpp
    @author   K.Townsend (Adafruit Industries)
    @license  BSD (see license.txt)
    Driver for the ADS1015/ADS1115 ADC
    This is a library for the Adafruit MPL115A2 breakout
    ----> https://www.adafruit.com/products/???
    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!
    @section  HISTORY
    v1.0 - First release
    
    LB - Modified to handle ADS1115 - 16 bit chipset only
         Fixed method readRegister, originally it was hard coded to 
         read configuration register, even though a "reg" parameter was passed.
         Added methods:
         void      startContinuousADC_Differential_0_1(void);
         uint16_t  getLow(void);
         uint16_t  getHigh(void);
*/
/**************************************************************************/
#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <Wire.h>

#include "EQMon_ADS1115.h"

/**************************************************************************/
/*!
    @brief  Abstract away platform differences in Arduino wire library
*/
/**************************************************************************/
static uint8_t i2cread(void) {
  #if ARDUINO >= 100
  return Wire.read();
  #else
  return Wire.receive();
  #endif
}

/**************************************************************************/
/*!
    @brief  Abstract away platform differences in Arduino wire library
*/
/**************************************************************************/
static void i2cwrite(uint8_t x) {
  #if ARDUINO >= 100
  Wire.write((uint8_t)x);
  #else
  Wire.send(x);
  #endif
}

/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
*/
/**************************************************************************/
static void writeRegister(uint8_t i2cAddress, uint8_t reg, uint16_t value) {
  Wire.beginTransmission(i2cAddress);
  i2cwrite((uint8_t)reg);
  i2cwrite((uint8_t)(value>>8));
  i2cwrite((uint8_t)(value & 0xFF));
  Wire.endTransmission();
}

/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
*/
/**************************************************************************/
static uint16_t readRegister(uint8_t i2cAddress, uint8_t reg) {
  Wire.beginTransmission(i2cAddress);
// Possible bug here, seems hard coded to CONVERT register
//  i2cwrite(ADS1115_REG_POINTER_CONVERT);
  i2cwrite(reg);
  Wire.endTransmission();
  Wire.requestFrom(i2cAddress, (uint8_t)2);
  return ((i2cread() << 8) | i2cread());  
}

/**************************************************************************/
/*!
    @brief  Instantiates a new ADS1115 class w/appropriate properties
*/
/**************************************************************************/
Adafruit_ADS1115::Adafruit_ADS1115(uint8_t i2cAddress) 
{
   m_i2cAddress = i2cAddress;
   m_conversionDelay = ADS1115_CONVERSIONDELAY_250;
   m_bitShift = 0;
   m_gain = GAIN_TWOTHIRDS; /* +/- 6.144V range (limited to VDD +0.3V max!) */
}

/**************************************************************************/
/*!
    @brief  Sets up the HW (reads coefficients values, etc.)
*/
/**************************************************************************/
void Adafruit_ADS1115::begin() {
  Wire.begin();
}

/**************************************************************************/
/*!
    @brief  Sets the gain and input voltage range
*/
/**************************************************************************/
void Adafruit_ADS1115::setGain(adsGain_t gain)
{
  m_gain = gain;
}

/**************************************************************************/
/*!
    @brief  Gets a gain and input voltage range
*/
/**************************************************************************/
adsGain_t Adafruit_ADS1115::getGain()
{
  return m_gain;
}




/**************************************************************************/
/*!
    @brief  Gets conversion delay
*/
/**************************************************************************/
uint16_t Adafruit_ADS1115::getDelay()
{
  return m_conversionDelay;
}

/**************************************************************************/
/*!
    @brief  Sets conversion delay
*/
/**************************************************************************/
void Adafruit_ADS1115::setDelay(uint16_t delay)
{
  m_conversionDelay = delay;
}

/**************************************************************************/
/*!
    @brief  Gets a single-ended ADC reading from the specified channel
*/
/**************************************************************************/
uint16_t Adafruit_ADS1115::readADC_SingleEnded(uint8_t channel) {
  if (channel > 3)
  {
    return 0;
  }
  
  // Start with default values
  uint16_t config = ADS1115_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
                    ADS1115_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
                    ADS1115_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1115_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1115_REG_CONFIG_DR_860SPS    | // 860 samples per second (default)
                    ADS1115_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // Set PGA/voltage range
  config |= m_gain;

  // Set single-ended input channel
  switch (channel)
  {
    case (0):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_0;
      break;
    case (1):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_1;
      break;
    case (2):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_2;
      break;
    case (3):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_3;
      break;
  }

  // Set 'start single-conversion' bit
  config |= ADS1115_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  writeRegister(m_i2cAddress, ADS1115_REG_POINTER_CONFIG, config);

  // Wait for the conversion to complete
  delay(m_conversionDelay);

  // Read the conversion results
  return readRegister(m_i2cAddress, ADS1115_REG_POINTER_CONVERT);  
}

/**************************************************************************/
/*! 
    @brief  Reads the conversion results, measuring the voltage
            difference between the P (AIN0) and N (AIN1) input.  Generates
            a signed value since the difference can be either
            positive or negative.
*/
/**************************************************************************/
int16_t Adafruit_ADS1115::readADC_Differential_0_1() {
  // Start with default values
  uint16_t config = ADS1115_REG_CONFIG_MUX_DIFF_0_1  | // AIN0 = P, AIN1 = N
                    ADS1115_REG_CONFIG_CQUE_NONE     | // Disable the comparator (default val)
                    ADS1115_REG_CONFIG_CLAT_NONLAT   | // Non-latching (default val)
                    ADS1115_REG_CONFIG_CPOL_ACTVLOW  | // Alert/Rdy active low   (default val)
                    ADS1115_REG_CONFIG_CMODE_TRAD    | // Traditional comparator (default val)
                    ADS1115_REG_CONFIG_DR_475SPS     | // 475 samples per second 
                    ADS1115_REG_CONFIG_OS_SINGLE     |
                    ADS1115_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)
//                    ADS1115_REG_CONFIG_MODE_CONTIN;    // Continuous mode
                    
  // Set PGA/voltage range
  config |= m_gain;
                   
  // Set 'start single-conversion' bit
  //config |= ADS1115_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  writeRegister(m_i2cAddress, ADS1115_REG_POINTER_CONFIG, config);

  // Wait for the conversion to complete
  // delay(m_conversionDelay);
  //
  // Changed to delayMicroseconds for better accuracy
  //delayMicroseconds(ADS1115_CONVERSIONDELAY_860);  // for 860 SPS
  delayMicroseconds(m_conversionDelay);
  

  // Read the conversion results
  uint16_t res = readRegister(m_i2cAddress, ADS1115_REG_POINTER_CONVERT);
  return (int16_t)res;
}
// --------------------
/**************************************************************************/
/*! 
    @brief  Starts ADC in continuous mode, measuring the voltage
            difference between the P (AIN0) and N (AIN1) input.  Generates
            a signed value since the difference can be either
            positive or negative.  ALERT/RDY pin indicates when conversion is ready
*/
/**************************************************************************/
void Adafruit_ADS1115::startContinuousADC_Differential_0_1() {
  // Start with default values
  uint16_t config = ADS1115_REG_CONFIG_CQUE_1CONV   | // Ready pin active after 1 conversion
                    ADS1115_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val) No function in continuous ADC mode
                    ADS1115_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1115_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val) No function in continuous ADC mode
                    ADS1115_REG_CONFIG_DR_250SPS    | // 250 samples per second 
                    ADS1115_REG_CONFIG_MODE_CONTIN;   // Continuous mode

// Setup ALERT/RDY Pin as conversion ready pin for use as interrupt trigger.
// Data sheet page 15
// The ALERT/RDY pin can also be configured as a conversion ready pin. This mode of operation can be 
// realized if the MSB of the high threshold register is set to '1' and the MSB of the low threshold 
// register is set to '0'
// High Threshold register MSB = 1
// Low  Threshold register MSB = 0     

// Set the high threshold register
  uint16_t value = 0x8000;
  writeRegister(m_i2cAddress, ADS1115_REG_POINTER_HITHRESH, value);
                
// Set the low threshold register
  value = 0x0000;
  writeRegister(m_i2cAddress, ADS1115_REG_POINTER_LOWTHRESH, value);
  
                    
                                        
  // Set PGA/voltage range
  config |= m_gain;
                    
  // Set channels
  config |= ADS1115_REG_CONFIG_MUX_DIFF_0_1;          // AIN0 = P, AIN1 = N

  // Set 'start single-conversion' bit
  config |= ADS1115_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  writeRegister(m_i2cAddress, ADS1115_REG_POINTER_CONFIG, config);
}


// ************* Debug ****************
// Read the Threshold registers to verify the contents
uint16_t Adafruit_ADS1115::getLow(void){
 // uint16_t res = readRegister(m_i2cAddress, ADS1115_REG_POINTER_LOWTHRESH);
//  return (int16_t)res;
}
  

uint16_t Adafruit_ADS1115::getHigh(void){
 // uint16_t res = readRegister(m_i2cAddress, ADS1115_REG_POINTER_HIGHTHRESH);
//  return (int16_t)res;
}
  






/**************************************************************************/
/*! 
    @brief  Reads the conversion results, measuring the voltage
            difference between the P (AIN2) and N (AIN3) input.  Generates
            a signed value since the difference can be either
            positive or negative.
*/
/**************************************************************************/
int16_t Adafruit_ADS1115::readADC_Differential_2_3() {
  // Start with default values
  uint16_t config = ADS1115_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
                    ADS1115_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
                    ADS1115_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1115_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1115_REG_CONFIG_DR_860SPS    | // 860 samples per second (default)
                    ADS1115_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // Set PGA/voltage range
  config |= m_gain;

  // Set channels
  config |= ADS1115_REG_CONFIG_MUX_DIFF_2_3;          // AIN2 = P, AIN3 = N

  // Set 'start single-conversion' bit
  config |= ADS1115_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  writeRegister(m_i2cAddress, ADS1115_REG_POINTER_CONFIG, config);

  // Wait for the conversion to complete
  delayMicroseconds(m_conversionDelay);

  // Read the conversion results
  uint16_t res = readRegister(m_i2cAddress, ADS1115_REG_POINTER_CONVERT);
  return (int16_t)res;
}

/**************************************************************************/
/*!
    @brief  Sets up the comparator to operate in basic mode, causing the
            ALERT/RDY pin to assert (go from high to low) when the ADC
            value exceeds the specified threshold.
            This will also set the ADC in continuous conversion mode.
*/
/**************************************************************************/
void Adafruit_ADS1115::startComparator_SingleEnded(uint8_t channel, int16_t threshold)
{
  // Start with default values
  uint16_t config = ADS1115_REG_CONFIG_CQUE_1CONV   | // Comparator enabled and asserts on 1 match
                    ADS1115_REG_CONFIG_CLAT_LATCH   | // Latching mode
                    ADS1115_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1115_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1115_REG_CONFIG_DR_860SPS    | // 1600 samples per second (default)
                    ADS1115_REG_CONFIG_MODE_CONTIN  | // Continuous conversion mode
                    ADS1115_REG_CONFIG_MODE_CONTIN;   // Continuous conversion mode

  // Set PGA/voltage range
  config |= m_gain;
                    
  // Set single-ended input channel
  switch (channel)
  {
    case (0):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_0;
      break;
    case (1):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_1;
      break;
    case (2):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_2;
      break;
    case (3):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_3;
      break;
  }

  // Set the high threshold register
  writeRegister(m_i2cAddress, ADS1115_REG_POINTER_HITHRESH, threshold);

  // Write config register to the ADC
  writeRegister(m_i2cAddress, ADS1115_REG_POINTER_CONFIG, config);
}

/**************************************************************************/
/*!
    @brief  In order to clear the comparator, we need to read the
            conversion results.  This function reads the last conversion
            results without changing the config value.
*/
/**************************************************************************/
int16_t Adafruit_ADS1115::getLastConversionResults()
{
  // Wait for the conversion to complete
  delayMicroseconds(m_conversionDelay);

  // Read the conversion results
  uint16_t res = readRegister(m_i2cAddress, ADS1115_REG_POINTER_CONVERT);
  return (int16_t)res;
}