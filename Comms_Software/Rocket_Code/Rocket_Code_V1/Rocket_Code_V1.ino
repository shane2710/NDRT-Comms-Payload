int CRrate = 0;      //Command signal rate
int CGrate = 0;
int rate = 4000;      //Usable rate
float TSendRate;

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

  TSendRate = millis();
}

void loop() {

  while (Serial.available()) { //While radio data is coming in
    inbyte = Serial.read();    //Get the byte
    if (inbyte == '\n') {       //If its the end of the packet,
      stripKISSR(buf);
      buflen = 0;
    }
    else if ((inbyte > 31 && buflen < 260)) {   //If its a nonprintable character, don't record it
      buf[buflen++] = inbyte;
      buf[buflen] = 0;
    }
  }  //If data avaliable, read it in, strip and assign variables


  if ((millis() - TSendRate) > rate) {
    Serial.print("!");          //Start packet
    Serial.print(CRrate);      //Print rate control signal
    Serial.print("12345,");       //Add numbers
    Serial.print("\r\n");       //Close packet, send
    TSendRate = millis();    //Set current time for looping
  }  //Send message over and over again


  if (CGrate != CRrate) {
    CRrate = CGrate;
  }  //If incoming rate is different than rocket rate, update rocket


  if (CRrate) {
    rate = 4000;
  }
  else {
    rate = 8000;
  }

}


void stripKISSR(char buff[]) {
  CGrate = buff[16] - '0';
}    //Strip incoming packet, store to variables
