int CRrate = 0;      //Command signal rate
int CGrate = 0;
int rate;      //Usable rate

float tSendRate;

char inbyte = 0;
char buf[260];
int buflen = 0;               //Radio variables


void setup() {
  Serial.begin(4800);    //Start comms with radio shield
  delay(5);
  Serial.print("MKC9TMD\r\n");  //Set callsign
  delay(10);
  Serial.print("PWIDE1-1,WIDE2-1\r\n");    //Set Digipath
  delay(10);
  pinMode(13, OUTPUT);
  tSendRate = millis();
}

void loop() {

  while (Serial.available()) { //While radio data is coming in
    inbyte = Serial.read();    //Get the byte
    if (inbyte == '\n') {       //If its the end of the packet,
      stripKISSR(buf);          //Strip, store to variables
      updateRocketVars();
      updateRocketSettings();
      tSendRate = millis() - rate + 500;
      buflen = 0;
    }
    else if ((inbyte > 31 && buflen < 260)) {   //If its a nonprintable character, don't record it
      buf[buflen++] = inbyte;
      buf[buflen] = 0;
    }
  }  //If data avaliable, read it in, strip and assign variables


  if ((millis() - tSendRate) > rate) {
    sendPacket();
    tSendRate = millis();    //Set current time for looping
  }  //Send message over and over again


  updateRocketVars();
  updateRocketSettings();

}


void stripKISSR(char buff[]) {
  CGrate = buff[16] - '0';
  if (CGrate == 0) {
    digitalWrite(13, 1);
  }
  else if (CGrate == 1) {
    digitalWrite(13, 0);
  }
}    //Strip incoming packet, store to variables


void sendPacket() {
  Serial.print("!");          //Start packet
  Serial.print(CRrate);      //Print rate control signal
  Serial.print("125,");       //Add numbers
  Serial.print(buf);
  Serial.print(",");
  Serial.print(buf[14]);
  Serial.print(",");
  Serial.print(buf[16]);
  Serial.print("\r\n");       //Close packet, send
} //Send packet to ground station


void updateRocketVars() {
  if (CGrate != CRrate) {
    CRrate = CGrate;
  }
} //If incoming is different than rocket, update rocket


void updateRocketSettings() {
  if (CRrate) {
    rate = 4500;
  }
  else {
    rate = 8000;
  }   //Update rate based on rocket state
}

