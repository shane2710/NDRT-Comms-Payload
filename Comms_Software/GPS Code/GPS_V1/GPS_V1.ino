#include <TinyGPS++.h>
#include <Flash.h>
#include <SoftwareSerial.h>

// Pins
const byte  gpsCommRX=2;
const byte  gpsCommTX=3;
const byte  gpsPwr=4;
const byte  redLed=5;
const byte  greenLed=6;


//Create binaryFloat type for sending floats using binary!
typedef union {
  float floatingPoint;
  byte binary[4];
} 
binaryFloat;


// GPS VARIABLES
SoftwareSerial gpsComm(gpsCommRX,gpsCommTX);
boolean gpsRun=true;  //allow GPS to run
TinyGPSPlus gps;
char c;
FLASH_STRING(GPSCMP,"$PSRF100,01,4800,08,01,00*0E\r\n");
FLASH_STRING(GPSGGA,"$PSRF103,00,00,01,01*25\r\n");
FLASH_STRING(GPSRMC,"$PSRF103,04,00,10,01*21\r\n");
FLASH_STRING(GPSWAAS,"$PSRF151,01*3F\r\n");


//GPS LOCATION VARIABLES
binaryFloat latitude;
binaryFloat longitude;
binaryFloat alt;
byte sats,tmonth,tday,thour,tminute,tsecond,tyear;
boolean fix=false;



void setup() {
  // Set up pins
  pinMode(gpsPwr,OUTPUT);
  pinMode(redLed,OUTPUT);
  pinMode(greenLed,OUTPUT);
  
  //Start comm with computer
  Serial.begin(19200);
  Serial.println("-------Starting serial interface--------");
  
  // Start GPS
  digitalWrite(gpsPwr,LOW);
  gpsComm.begin(4800);
  delay(250); // Let GPS warm up
  GPSCMP.print(gpsComm); // Set up comm parameters
  delay(250);
  GPSGGA.print(gpsComm);  //GGA has altidue and position so get it as fast as possible
  delay(250);
  GPSRMC.print(gpsComm); // RMC every 20 seconds only because we want messages with altitude data, but RMC has date which is useful
  delay(250);
  GPSWAAS.print(gpsComm); //WAAS on- slower fix but more accuracy
  
  
  if(gpsRun){
    digitalWrite(gpsPwr,LOW); 
  }
  else{
    digitalWrite(gpsPwr,HIGH);
  }
  
  
}



void loop() {
  //get all the values
  tyear=(byte)(gps.date.year()-2000);
  tmonth=gps.date.month();
  tday=gps.date.day();
  thour=gps.time.hour();
  tminute=gps.time.minute();
  tsecond=gps.time.second();
  latitude.floatingPoint=gps.location.lat();
  longitude.floatingPoint=gps.location.lng();
  alt.floatingPoint=gps.altitude.feet();
  sats=(byte)gps.satellites.value();

  
  // Set LEDs
  if(sats>=3){
    digitalWrite(greenLed,HIGH); 
    fix=true;
  }
  else{
    digitalWrite(greenLed,LOW);
    fix=false;
  }
  

  
  if(gpsRun){
    digitalWrite(gpsPwr,LOW); 
  }
  else{
    digitalWrite(gpsPwr,HIGH);
  }
  
  smartDelay(3000);
  }
  
  void smartDelay(unsigned long ms){
  unsigned long lastMs=millis();
  while (((millis()-lastMs)<ms)){
    if(gpsComm.available()){
      c = gpsComm.read(); 
      //feed tinyGPS
      gps.encode(c);
      Serial.write(c);
    } 
  }
}
