int rateSwitch = 8;
int CRrate = 0;               //Command Signal Rate
int CGrate = 0;

int packetOut = 0;
float tPacketOut = 0;

char inbyte = 0;
char buf[260];
int buflen = 0;               //Radio variables

void setup() {
  pinMode(rateSwitch, HIGH);
  digitalWrite(rateSwitch, HIGH);

  Serial3.begin(4800);        //Set up comms with Radio shield
  delay(5);
  Serial.print("MKC9TMD\r\n");    //Set callsign
  delay(10);
  Serial.print("PWIDE1-1,WIDE2-1\r\n");    //Set Digipath
  delay(10);

  Serial.begin(19200);         //Set up comms with computer
  delay(5);
  pinMode(7, INPUT);
  digitalWrite(7, HIGH);
}

void loop() {

  while (Serial3.available()) { //While radio data is coming in
    inbyte = Serial3.read();    //Get the byte
    if (inbyte == '\n') {       //If its the end of the packet,
      Serial.println(buf);     //Print to computer
      stripKISS(buf);         //Strip, store to variables
      if (checkState()) {
        packetOut = 1;
        tPacketOut = millis();
      }                        //If a packet needs to go out, send it to send asap
      buflen = 0;
    }
    else if ((inbyte > 31 && buflen < 260)) {   //If its a nonprintable character, don't record it
      buf[buflen++] = inbyte;
      buf[buflen] = 0;
    }
  }

  if (digitalRead(7) == 0) {
    sendPacket();
  }
  else if (packetOut == 1) {
    if (millis() - tPacketOut > 1200) {
      sendPacket();
      packetOut = 0;
    }
  } //If packet needs to get sent right away or delayed slightly, do that


  checkSwitches();    //check switches, update variable states

}

void checkSwitches() {
  if (digitalRead(rateSwitch)) {
    CGrate = 0;
  }
  else {
    CGrate = 1;
  }          //Set rate variable
}

void stripKISS(char buff[]) {
  CRrate = buff[14] - '0';
  Serial.println(CRrate);
}

int checkState() {
  if (CRrate != CGrate) {
    return 1;
  }
  return 0;
}

void sendPacket() {

  Serial3.print("!");
  Serial3.print(CGrate);
  Serial3.print("\r\n");
  Serial.println(" ");
  Serial.println(CGrate);

}
