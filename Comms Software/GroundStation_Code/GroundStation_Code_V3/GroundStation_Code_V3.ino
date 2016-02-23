// These are the variables needed to control the rate
#define rate_pin 8 //Rate switch
int CRrate = 0; //Rate according to the rocket
int CGrate = 0; //Rate according to the ground

int packetOut = 0;
float tPacketOut = 0;

// These are variables used for the TNC
char inbyte = 0;
char buf[260];
int buflen = 0;

void setup() {

  //Set up comms with computer, wait
  Serial.begin(19200); 
  delay(5);
  Serial.print("Comms Online");

  //Start comms with radio shield, wait
  Serial3.begin(4800);        //Set up comms with Radio shield
  delay(5);

  //Set radio stuff
  Serial3.print("MKC9TMD\r\n");    //Set callsign
  delay(10);
  Serial3.print("PWIDE1-1,WIDE2-1\r\n");  //Set Digipath. We don't really need it for this, but the TNC won't work without it
  delay(10);
  //WHAT THE FUUUCCCKKKKKK

}

void loop() {

  //This while loop waits for a packet to be coming from the rocket, then does stuff
  while (Serial3.available()) { //While radio data is coming in
    inbyte = Serial3.read();    //Get the byte
    if (inbyte == '\n') {       //If its the end of the packet, do the following:
      Serial.println(buf);     //Print to computer
      stripKISS(buf);         //Strip, store to variables
      //If a packet needs to go out, set it to be sent in 500ms
      if (checkState()) {
          packetOut=1;
          tPacketOut = millis();
      }                        
      
      buflen = 0;
    }
    else if ((inbyte > 31 && buflen < 260)) {   //If its a nonprintable character, don't record it
      buf[buflen++] = inbyte;
      buf[buflen] = 0;
    }
  }

  // If a packet needs to go out, send it
  if (packetOut) {
    if (millis()-tPacketOut > 500) {
      sendPacket();
      packetOut=0;
    }
  }

  //check switches, update variable states
  checkSwitches();    

}

// Check switches and update variables
void checkSwitches() {
  //Set rate variable
  if (digitalRead(rate_pin)) {
    CGrate = 0;
  }
  else {
    CGrate = 1;
  }
}

//This strips the crap out of the packet and stores stuff to the CR variables and data variables
void stripKISS(char buff[]) {

  //Take the 14th character, store to rate variable
  CRrate = buff[14] - '0';

}

//Check if rocket and ground variables match
int checkState() {

  //Check rate variable
  if (CRrate != CGrate) {
    return 1;
  }
  return 0;
}

// This is the function that actually sends the packet
void sendPacket() {
  Serial.println("Sending Update"); //Tell computer that a packet is going out, not needed
  Serial.println(CGrate);
  Serial.println(CRrate);
  Serial3.print("!"); //Open packet
  Serial3.print(CGrate); //Send rate
  Serial3.print("\r\n"); //Close packet

}
