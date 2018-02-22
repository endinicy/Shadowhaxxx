#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
// Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */


String inputString ="";
int startheard=0;


byte Get2Bytes(uint8_t address, uint8_t registerNum) {  // make function to read data off I2C device
//  Serial.println("begin");
  Wire.beginTransmission(address);    // Get the slave's attention, tell it we're sending a command byte
//  Serial.println("write");
  Wire.write(registerNum); //  The command byte, sets pointer to register address
//  Serial.println("end1");
  Wire.endTransmission(); 
//  Serial.println("request");
  Serial.print("Requesting: ");
  Serial.println(Wire.requestFrom(address,(uint8_t)2));          // Tell slave we need to read byte from the current register
//  Serial.println("reads");
//  Serial.println(Wire.available());
  byte read1=Wire.read();
  byte read2=Wire.read();
  Serial.print("byte 1: ");
  Serial.println(read1);
  Serial.print("byte 2: ");
  Serial.println(read2);
//  return uint16_t ((Wire.read() << 8) | Wire.read());
//  return Wire.read();
//  Serial.println("end2");
  Wire.endTransmission();
}



// BEGIN: THIS SECTION DEALS WITH START TRIGGERS
void serialEvent(){               // In event of incoming serial port data
  while(Serial.available()){      // while there is still data in the buffer
    char inChar=(char)Serial.read();  // take the characters one at a time
    inputString = inChar;            // and stack them to form the string

  }
  if (inputString =="s" && startheard!=1){ // if user types "s"
    // startmicros=micros();                  // set start time   gotta be initiated within 4294 seconds of powering on otherwise startmicros will overflow 
    startheard=1;                         // and set the data in motion
    Serial.println("k");
  }
  if (inputString =="x"){ // && stopheard!=1) {  // if user types "x"
    startheard=0;                           // NOT USING stopheard
  }
  inputString="";                           // clear input buffer (putting this here makes max string length one (I don't really get it)
}                                           // i don't want to put this in the loop cuz it would slow down data taking.
// END: THIS SECTION DEALS WITH START TRIGGERS


void setup(void) 
{
  Serial.begin(9600);

////  Just checking default config
//  uint16_t config_bin = ADS1015_REG_CONFIG_CQUE_1CONV   | // Comparator enabled and asserts on 1 match
//                    ADS1015_REG_CONFIG_CLAT_LATCH   | // Latching mode
//                    ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
//                    ADS1015_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
//                    ADS1015_REG_CONFIG_DR_1600SPS   | // 1600 samples per second (default)
//                    ADS1015_REG_CONFIG_MODE_CONTIN  | // Continuous conversion mode
//                    ADS1015_REG_CONFIG_MUX_SINGLE_0  |   // Single-ended AIN0
//                    ADS1015_REG_CONFIG_PGA_4_096V;      // +/-4.096V range = Gain 1
//                    
//
//  Serial.println(config_bin);


//  Serial.println("Hello!");
//  
//  Serial.println("Getting single-ended readings from AIN0..3");
//  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");
  
  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
//  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  ads.begin();
  Serial.print("{");
}

float multiplier = 0.1875;

void loop(void) 
{
  if (startheard==1){
    int16_t adc0, adc1, adc2, adc3;

//    Serial.println(m_i2cAddress);

    adc0 = ads.readADC_SingleEnded(0);
    adc1 = ads.readADC_SingleEnded(1);
    Serial.print(adc0 * multiplier); 
//    Serial.print(",");
//    Serial.print(adc1); 
    Serial.print(",");
    // DOESN'T WORK FOR NEGATIVES
    //delay(100);

    uint8_t adc_address = 0x48;
    uint8_t config_reg = 0x01;
    
//    Serial.println(Get2Bytes(adc_address,config_reg));

  }
  else {
    if (startheard == 0){
      Serial.write("g");
      delay(10);
  }
  }
}
  


//  adc1 = ads.readADC_SingleEnded(1);
//  adc2 = ads.readADC_SingleEnded(2);
//  adc3 = ads.readADC_SingleEnded(3);
//  Serial.print("AIN0: "); 

// Serial.print("AIN0: "); Serial.println(adc0 * multiplier);
//  Serial.print("AIN1: "); Serial.println(adc1);
//  Serial.print("AIN2: "); Serial.println(adc2);
//  Serial.print("AIN3: "); Serial.println(adc3);
//  Serial.println(" ");

