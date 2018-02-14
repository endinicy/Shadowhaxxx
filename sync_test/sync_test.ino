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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  sync();
  Serial.println("EMOWEMEOWMEOWEMOWEMWOMEOW");
}
