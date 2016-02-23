//Setup LED pin for testing purposes
#define LED_pin 13

// These are variables used for the TNC
char inbyte = 0;
char buf[260];
int buflen = 0;

// These are the variables needed to control the rate
int CRrate = 0; //Rate according to the Rocket
int CGrate = 0; //Rate according to the ground
int rate = 8000; //Working rate, in ms
float rate_time;  //Timing variable for rate


int packetStart = 0; //This is the start variable for the stripkiss function
int i; //This is a generic looping variable

void setup() {
  //Start comms with radio shield, wait
  Serial.begin(4800);
  delay(5);

  //Set radio stuff
  Serial.print("MKC9TMD\r\n");  //Set callsign
  delay(10);
  Serial.print("PWIDE1-1,WIDE2-1\r\n"); //Set Digipath. We don't really need it for this, but the TNC won't work without it
  delay(10);

  //Set LED as output, again just for testing purposes
  pinMode(LED_pin, OUTPUT);

  //Initialize timing stuff
  rate_time = millis();
}

void loop() {

  //This while loop waits for a packet to be coming from the ground station, then does stuff
  while (Serial.available()) {  //While radio data is coming in
    inbyte = Serial.read();    //Get the byte
    if (inbyte == '\n') {       //If its the end of the packet, do the following:
      stripKISSR(buf);          //Strip, store to variables
      updateRocketVars();       //Update the rocket with the new variables
      updateRocketSettings();   //Update the settings on the rocket as well
      rate_time = millis() - rate + 1000;   //Set the rate time to send a packet back in half a second
      buflen = 0;
    }
    else if ((inbyte > 31 && buflen < 260)) {   //If its a nonprintable character, don't record it
      buf[buflen++] = inbyte;
      buf[buflen] = 0;
    }
  }

  // Send the message over and over again based on the rate
  if ((millis() - rate_time) > rate) {
    sendPacket(); //Send the packet
    rate_time = millis(); //Set current time for looping purposes
  }

  // Every iteration, update the variables and settings in case something happened on therocket side of things
  updateRocketVars();
  updateRocketSettings();

}

// This strips the crap out of the packet and stores stuff to the CG variables
void stripKISSR(char buff[]) {

  // Loop through the packet, look for the : that starts the actual info
  for (i=0;i<20;i=i+1) { //Only look to 20. if its not found by then, set start to 0 to show that its a bad packet.
    if (buff[i]==58) { //58 is the ascii code for : that it is looking for
      packetStart=i+1; //Set the packet start variable to be one after the :
      break; //Break out
    }

    //If we got to the end and didn't find it, set to 0 to indicate bad packet
    if (i==19) {
      packetStart=0;
    }
  }

  // If bad packet, break out of stripkiss
  if (packetStart==0) {
    return;
  }
  
  //Store rate variable
  CGrate = buff[packetStart] - '0';
}

// This is the function that actually sends the packet
void sendPacket() {
  Serial.print("!");          //Start packet
  Serial.print(CRrate);      //Print rate control signal
  Serial.print(",00");       //Everything from here...
  Serial.print(",");          // ... to here is testing crap, not needed in final version
  Serial.print("\r\n");       //Close packet, send
}


// This updates the rocket variables to match that coming from the ground station
void updateRocketVars() {

  if (CGrate != CRrate) {
    CRrate = CGrate;
  } // Update rate
}

// This updates the rocket settings based on the rocket variables
void updateRocketSettings() {

  if (CRrate) {
    rate = 3000;
  }
  else {
    rate = 6000;
  }   //Update rate based on rocket state

}

