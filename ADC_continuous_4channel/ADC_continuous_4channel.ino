#include <Wire.h>

const uint8_t adc_address = 0x48;
const uint8_t convert_reg = 0x00;
const uint8_t config_reg = 0x01;
int cont_port0_config = 0b100001010001011;
const byte interruptPin = 2;
volatile int yup;

void SendData(int address, int registerNum, int data) { // function to write data to I2C device
  Wire.beginTransmission(address);  // Get the slave's attention, tell it we're sending a command byte
  Wire.write(registerNum);          // set pointer to register of interest
  Wire.write(data);                 // write it in there!
  Wire.endTransmission();
}

void ReadConvReg() {
  Wire.beginTransmission(adc_address);    // Get the slave's attention, tell it we're sending a command byte
  Wire.write(convert_reg); //  The command byte, sets pointer to register address
  Wire.endTransmission(); 
  Wire.requestFrom(adc_address,(uint8_t)2);
  Serial.print(Wire.read());
  Serial.print(Wire.read());
  Wire.endTransmission();
}

void meow() {
  yup=1;
}

void setup() {

  Serial.begin(9600);
  Serial.println("Begin Setup");
  Wire.begin();
  pinMode(interruptPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin), meow, RISING);
  SendData(adc_address,config_reg,cont_port0_config);
  Serial.println("End Setup");
}

void loop() {
  if (yup == 1){
//    yup = 0;
//    Serial.print("1");
    ReadConvReg();
  }
}
