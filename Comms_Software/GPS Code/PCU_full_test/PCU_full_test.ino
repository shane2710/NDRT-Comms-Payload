#include <Flash.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>


//SET UP PINS

const byte  gpsCommRX=2;
const byte  gpsCommTX=3;
const byte  gpsPwr=4;
const byte  redLed=5;
const byte  greenLed=6;
const byte  sdCSpin=10;
const byte  PCUVoltpin=3;
const byte  VTXVoltpin=2;
const byte  IMGprocENP=7;
const byte  VIDsourceP=A1;
const byte  MainDeployP=9;
const byte  DrogueDeployP=8;
const byte  VTXonP=A0;


//Create binaryFloat type for sending floats using binary!
typedef union {
  float floatingPoint;
  byte binary[4];
} 
binaryFloat;

// GPS VARIABLES
SoftwareSerial gpsComm(gpsCommRX,gpsCommTX);
boolean gpsRun=false;  //allow GPS to run
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

//OTHER TELEMETRY VARIABLES
byte PCUVolt[2];
byte VTXVolt[2];
int aread;
boolean mainDeployed=false;
boolean drogueDeployed=false;

// SD CARD VARIABLES
boolean logging=false;  //if we are logging data
boolean card;  //SD card present
boolean logSuccess=false;  //if we are successfully logging
File posLog;

//RADIO VARIABLES
boolean highRate=false;  //transmit every 3 seconds if on, 1minute otherwise
boolean waitAck=false;  //system waiting for acknowledgement from ground
boolean txAllow=false;  //whether to transmit at all
boolean txAuto=false;  //Transmmitter control in automatic mode
char lastCommand;  //last command waiting to be acknowledged
//TNC receiver variables
byte msgIn[64];  //the incoming packet
char source[7];
char dest[7];
byte sourceSSID;
byte destSSID;
boolean receiving=false;  //are we getting a message from the TNC?
char cbuff[6];  //character buffer
byte inByte;
byte recvIndex=0;
static unsigned short	crc,crcrecv;
char pbuff[5];
char pktMode;
//KC9PXM-11 to KC9PXM-1
//                                   K          C         9        T         M          D    -   1
FLASH_ARRAY(byte,HEADER,0xc0,0x00,B10010110,B10000110,B01110010,B10101000,B10011010,B10001000,B01100010,
// K         C         9         T          M        D     -   11
B10010110,B10000110,B01110010,B10101000,B10011010,B10001000,B01110111,0x03,0xf0);

//VIDEO VARIABLES


//CONTROL VARIABLES
boolean MainDeployed, DrogueDeployed;
byte CommandByte1,CommandByte2,StatusByte1;
boolean setChanged=false;  //bump out of loop if settings changed

void setup(){
  pinMode(gpsPwr,OUTPUT);
  pinMode(sdCSpin,OUTPUT);
  pinMode(redLed,OUTPUT);
  pinMode(greenLed,OUTPUT);
    pinMode(IMGprocENP,OUTPUT);
  pinMode(VIDsourceP,OUTPUT);
  pinMode(MainDeployP,INPUT);
  pinMode(DrogueDeployP,INPUT);
  pinMode(VTXonP, OUTPUT);
  //enable pull-ups on deployment sensors
  digitalWrite(MainDeployP,HIGH);
  digitalWrite(DrogueDeployP,HIGH);
  CommandByte2=2;  //put in lowest power consumption mode (all relays off)
  //Start modem Serial Connection
  Serial.begin(19200);
  //Start GPS
  digitalWrite(gpsPwr,LOW);
  gpsComm.begin(4800);
  delay(250);  //let the GPS get fully powered up
  GPSCMP.print(gpsComm);    //Set up GPS comm parameters
  delay(250);
  GPSGGA.print(gpsComm);  //GGA has altidue and position so get it as fast as possible
  delay(250);
  GPSRMC.print(gpsComm); // RMC every 20 seconds only because we want messages with altitude data, but RMC has date which is useful
  delay(250);
  GPSWAAS.print(gpsComm);    //WAAS on- slower fix but more accuracy

  //START SD CARD LOGGING
  card=SD.begin(sdCSpin);

  if(gpsRun){
    digitalWrite(gpsPwr,LOW); 
  }
  else{
    digitalWrite(gpsPwr,HIGH);
  }
  logSuccess=logging&&card;
    
    StatusByte1=fix+logSuccess*2+DrogueDeployed*4+MainDeployed*8;
  
  smartDelay(3000);
}

void loop(){
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
  //get voltages
  aread=analogRead(PCUVoltpin);
  PCUVolt[0]=aread/256;
  PCUVolt[1]=aread % 256;
  aread=analogRead(VTXVoltpin);
  VTXVolt[0]=aread/256;
  VTXVolt[1]=aread % 256;
  //find deployment status of chutes
  DrogueDeployed=digitalRead(DrogueDeployP);
  MainDeployed=digitalRead(MainDeployP);
  //update status variables and LEDs
  if(sats>=4){
    digitalWrite(greenLed,HIGH); 
    fix=true;
  }
  else{
    digitalWrite(greenLed,LOW);
    fix=false;
  }
  logSuccess=logging&&card;
  if(logSuccess){
    digitalWrite(redLed,HIGH); 
  }
  else{
    digitalWrite(redLed,LOW);
  }
  StatusByte1=fix+logSuccess*2+DrogueDeployed*4+MainDeployed*8;
  //CONTROL SWITCHES AND RELAYS
  //GPS
  if(gpsRun){
    digitalWrite(gpsPwr,LOW); 
  }
  else{
    digitalWrite(gpsPwr,HIGH);
  }
  //VTX
  if(CommandByte2&1){
    digitalWrite(VTXonP,HIGH); 
  }
  else{
    digitalWrite(VTXonP,LOW);
  }
  //VIDEO SOURCE
  if(((CommandByte2&4)&&MainDeployed)||((CommandByte2^4)&&(CommandByte2&2))){
    digitalWrite(VIDsourceP,LOW);  //Raspberry Pi- if pin/power fails default is the Pi since it is mission critical
  }else{  
    digitalWrite(VIDsourceP,HIGH);  //External Camera  
  }
  //IMAGE PROCESSING
  if(((CommandByte2&16)&&MainDeployed)||((CommandByte2^16)&&(CommandByte2&8))){
    digitalWrite(IMGprocENP,HIGH);
  }else{  
    digitalWrite(IMGprocENP,LOW);
  }
  //DATA TRANSMISSION
  // transmission controlled by drogue deployment if in AUTO otherwise controlled by TX switch on ground station
  if((txAuto&&DrogueDeployed)||((!txAuto)&&txAllow)){  
    sendTelem();
  }
  if(highRate){
    smartDelay(3000);
  }
  else{
    smartDelay(60000);
  }
}

void sendTelem(){
  byte i;
  //send the AX.25 header
  for(i=0;i<HEADER.count();i++){
    Serial.write(HEADER[i]);
  } 
  //now send the individual values
  Serial.write('T');
  Serial.write(tmonth);
  Serial.write(tday);
  Serial.write(tyear);
  Serial.write(thour);
  Serial.write(tminute);
  Serial.write(tsecond);
  Serial.write(latitude.binary,4);
  Serial.write(longitude.binary,4);
  Serial.write(alt.binary,4);
  Serial.write(sats);
  Serial.write(PCUVolt,2);
  Serial.write(VTXVolt,2);
  Serial.write(CommandByte1);
  Serial.write(CommandByte2);
  Serial.write(StatusByte1);
  //send the KISS frame end character
  Serial.write((byte) 0xc0);
}

void sendAck(){
  byte i;
  //send the AX.25 header
  for(i=0;i<HEADER.count();i++){
    Serial.write(HEADER[i]);
  } 
  Serial.write('A');
  Serial.write(CommandByte1);
  Serial.write(CommandByte2);
  Serial.write(StatusByte1);
  Serial.write((byte) 0xc0);
}

void processPacket(void){
  byte index=0;  //where we are in the incoming message
  boolean lastCall=false;
  // RESET EVERYTHING
  memset(source,'\0',sizeof(source));
  memset(dest,'\0',sizeof(dest));
  memset(pbuff,'\0',sizeof(pbuff));
  sourceSSID=0;
  destSSID=0;
  //Serial.println(" ");

  if(recvIndex<16){  //bad packet- not enough data
    return; 
  }
  // get destination address
  for(byte i=0;i<6;i++){
    dest[i]=char(msgIn[index]>>1);
    index++;
  }
  destSSID=(msgIn[index]& B00011110)>>1;
  index++;
  if((destSSID!=11) ||(strcmp(dest,"KC9TMD")!=0) ){   // message not to us, ignore and exit
    return;
  }
  //get source address
  for(byte i=0;i<6;i++){
    source[i]=char(msgIn[index]>>1);
    index++;
  }
  sourceSSID=(msgIn[index]& B00011110)>>1;
  // the last callsign has the LSB set to 1, so loop and find it.
  while ((msgIn[index]& B00000001)!=1){  //who cares about the routing, just find the start of content
    index+=7; 
  }
  index+=3;  //ignore control characters (we know this is going to be a UI frame)
  if((sourceSSID!=1) && (strcmp(source,"KC9TMD")!=0) ){  // make sure we have data from the right call sign, if not exit
    return;
  }
  pktMode=msgIn[index];
  index++;
  //bsiness end of stuff!
  switch (pktMode){
  case 'S':    // Command to send switching bits to rocket
    CommandByte1=msgIn[index];
    index++;
    CommandByte2=msgIn[index];    
    //set the requisite variables
    gpsRun=(CommandByte1&1)/1;
    logging=(CommandByte1&2)/2;
    highRate=(CommandByte1&4)/4;
    txAllow=(CommandByte1&8)/8;
    txAuto=(CommandByte1&16)/16;
    
    delay(600);
    sendAck();  //acknowledge command
    setChanged=true;
    break;
  case 'P':
    delay(600);
    sendTelem();
  break;
  default:
    //do nothing
    break;
  }
}


static byte prevdata; 
void stripKiss(byte data)
{

  if (0xC0 == prevdata)
  {
    if ((0x00 == data) )
    { //we have the start of a new message
      receiving=true;
    }
    //TODO setup other modem parameters here... such as txtail etc....
  }
  else if((0xC0 == data)&& receiving)// now we have the end of a message
  {
    receiving=false;
    if(fcsCheck()){
      processPacket();    //extract useful data from the packet
    }
    //RESET FOR NEXT PACKET
    memset(msgIn,0,sizeof(msgIn));  //reset the incoming message buffer
    recvIndex=0;
  }
  else if(0xDC == data) // we may have an escape byte
  {  
    if(0xDB == prevdata)
    {
      //add 0xC0 to message
      msgIn[recvIndex]=0xC0;
      recvIndex++;
    }
  }
  else if(0xDD == data) // we may have an escape byte
  {
    if(0xDB == prevdata)
    {
      //add 0xDB to message
      msgIn[recvIndex]=0xDB;
      recvIndex++;
    }
  }  
  else if(0xDB == data) {
  }  // 0xDB shouldnt appear in incoming message except as escape byte so ignore it and go to the next character
  else if (receiving){
    msgIn[recvIndex]=data;
    //Serial.print(data,HEX);Serial.print(" ");
    recvIndex++;
  }

  prevdata = data; // copy the data for our state machine 
}


boolean fcsCheck(){    //check incoming FCS return true if it passes (this adapted from the arduino_tnc code used on board the TNC shield)
  static unsigned short	xor_int;
  crc = 0xFFFF;	
  byte bitbyte;
  for(byte i=0;i<(recvIndex-2);i++){    //loop through the entire message except the FCS to compute received checksum
    bitbyte=msgIn[i];                    //loop through the bits of each byte
    for (byte mloop = 0 ; mloop < 8 ; mloop++){
      xor_int = crc ^ (bitbyte& 0x01);				// XOR lsb of CRC with the latest bit
      crc >>= 1;						// Shift 16-bit CRC one bit to the right

      if (xor_int & 0x0001)					// If XOR result from above has lsb set
      {
        crc ^= 0x8408;					// Shift 16-bit CRC one bit to the right
      }
      bitbyte >>= 1;
    }

  }
  if((((crc >> 8)^0xFF)==msgIn[recvIndex-1])&&(((byte)crc^0xFF)==msgIn[recvIndex-2])){  //test the high and low bits of the checksum against what was received
    return true;
  }
  else{
    return false;
  }
}

//delay for a given time while still encoding GPS data
void smartDelay(unsigned long ms){
  if(logSuccess){
    posLog=SD.open("FLTLOG.TXT",FILE_WRITE);
  }
  unsigned long lastMs=millis();
  while (((millis()-lastMs)<ms)&& (!setChanged)){
    if(gpsComm.available()){
      c = gpsComm.read(); 
      //feed tinyGPS
      gps.encode(c);
      //Serial.write(c);
      //write raw nmea sentences to SD Card
      if(logSuccess){
        posLog.write(c); 
      }
    } 
    if (Serial.available())  //there's data coming in from the TNC
    {
      c=Serial.read();
      stripKiss(c);   //remove the KISS protocol stuff
    }
  }
  if(logSuccess){
    posLog.close();
  }
  setChanged=false;  //reset end-of-loop trigger for next time
}






