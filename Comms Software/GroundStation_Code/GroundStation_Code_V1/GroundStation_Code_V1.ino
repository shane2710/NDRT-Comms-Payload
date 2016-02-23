int rateSwitch = 8;
int CRrate = 0;               //Command Signal Rate
int CGrate = 0;

float Tpacket = 0;

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
}

void loop() {

  while (Serial3.available()) { //While radio data is coming in
    inbyte = Serial3.read();    //Get the byte
    if (inbyte == '\n') {       //If its the end of the packet,
      Serial.println(buf);     //Print to computer
      stripKISS(buf);
      buflen = 0;
    }
    else if ((inbyte > 31 && buflen < 260)) {   //If its a nonprintable character, don't record it
      buf[buflen++] = inbyte;
      buf[buflen] = 0;
    }
  }




  if (digitalRead(rateSwitch)) {
    CGrate = 0;
  }
  else {
    CGrate = 1;
  }                               //Set rate variable


  if (CRrate != CGrate) {
    sendPacket(CGrate);
  }

}



void stripKISS(char buff[]) {
  CRrate = buff[14] - '0';
  Serial.println(CRrate);
}


void sendPacket(int packet) {
  if (millis() - Tpacket > 2600)  {
      Serial3.print("!");
      Serial3.print(packet);
      Serial3.print("\r\n");
      Tpacket = millis();
      Serial.println(" ");
      Serial.println(CGrate);
  }
}
