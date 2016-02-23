//Setup LED pin for testing purposes
#define LED_pin 13

//These are the variables needed to define the temp sensor
#define temp_pin A0 //This can change
byte bGlobalErr;
byte temp_data[5];
float temp_time;

//These are the variables for the shock sensor
#define shock_pin 7 //This can change
int shock_data;
float shock_time;

// These are variables used for the TNC
char inbyte = 0;
char buf[260];
int buflen = 0;

// These two variables are used to store the temp and humidity data
int temp;
int humid;

// This stores the shock data
int shock;

// These are the variables needed to control the rate, 1 is high 0 is low
int CRrate = 1; //Rate according to the Rocket
int CGrate = 1; //Rate according to the ground
int rate = 8000; //Working rate, in ms
float rate_time;  //Timing variable for rate

//These are the variables needed to control transmission, 1 is on 0 is off
int CRtx = 1;  //tx according to the Rocket
int CGtx = 1;  //tx according to the Rocket
int txBypass = 1; // this can be used to bypass a disabled tx
// It's initially a 1 so that a single info packet gets sent on boot


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

  //These lines are for setting up the power and ground for the humidity sensor, they're not needed in actual operation
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  digitalWrite(A1, HIGH);
  digitalWrite(A2, LOW);

  //These lines are for setting up the power and ground for the shock sensor, they're not needed in actual operation
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite(6, HIGH);
  digitalWrite(5, LOW);

  // This sets up the shock sensor
  pinMode(shock_pin, INPUT);

  // This sets up the humidity sensor
  pinMode(temp_pin, OUTPUT);
  digitalWrite(temp_pin, HIGH);

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
      rate_time = millis() - rate + 1000;   //Set the rate time to send a packet back in one second
      txBypass = 1; //Tells PCU to reply even if TX is disabled
      buflen = 0;
    }
    else if ((inbyte > 31 && buflen < 260)) {   //If its a nonprintable character, don't record it
      buf[buflen++] = inbyte;
      buf[buflen] = 0;
    }
  }

  // If transmit is enabled, send the message over and over again based on the rate
  if (CRtx || txBypass) {
    if ((millis() - rate_time) > rate) {
      sendPacket(); //Send the packet
      rate_time = millis(); //Set current time for looping purposes
      txBypass = 0;
    }
  }

  // Every iteration, update the variables and settings in case something happened on therocket side of things
  updateRocketVars();
  updateRocketSettings();

  // This polls the humidity sensor and saves the data to 'temp' and 'humid'
  // To change the rate that it polls, change the number in the line below
  if (millis() - temp_time > 200) {
    pollTemp();
    temp_time = millis();
  }

  // This polls the shock sensor and saves the data to "shock".
  // It polls every loop of code, and saves data every 1000 milliseconds
  // The value is the number of times there has been shock in the last 1000 milliseconds divided by five (the average amount per 200 ms)
  if (digitalRead(shock_pin) == LOW) {
    shock_data = shock_data + 1;
  }
  if (millis() - shock_time > 2000) {
    shock = shock_data;
    shock_data = 0;
    shock_time = millis();
  }

}

void pollTemp() {
  ReadDHT();
  switch (bGlobalErr) {
    case 0:
      humid = temp_data[0];
      temp = (temp_data[2] * 9 / 5) + 32;
      break;
    case 1:
      humid = 99;
      temp = 91;
      break;
    case 2:
      humid = 99;
      temp = 92;
      break;
    case 3:
      humid = 99;
      temp = 93;
      break;
    default:
      humid = 99;
      temp = 94;
      break;
  }
}


// This strips the crap out of the packet and stores stuff to the CG variables
void stripKISSR(char buff[]) {

  // Loop through the packet, look for the : that starts the actual info
  for (i = 0; i < 20; i = i + 1) { //Only look to 20. if its not found by then, set start to 0 to show that its a bad packet.
    if (buff[i] == 58) { //58 is the ascii code for : that it is looking for
      packetStart = i + 1; //Set the packet start variable to be one after the :
      break; //Break out
    }

    //If we got to the end and didn't find it, set to 0 to indicate bad packet
    if (i == 19) {
      packetStart = 0;
    }
  }

  // If bad packet, break out of stripkiss
  if (packetStart == 0) {
    return;
  }

  //Store rate variable
  CGrate = buff[packetStart] - '0';
}

// This is the function that actually sends the packet
void sendPacket() {
  Serial.print("!");          //Start packet
  Serial.print(CRrate);     //Rate control signal
  Serial.print(CRtx);       //TX control signal
  Serial.print(",");
  Serial.print(temp);       //temperature data
  Serial.print(",");
  Serial.print(humid);      //humidity data
  Serial.print(",");
  Serial.print(shock);      //shock dat
  Serial.print(",");
  Serial.print("nn");       //signals end of packet (to benny), not needed
  Serial.print("\r\n");       //Close packet, send
}


// This updates the rocket variables to match that coming from the ground station
void updateRocketVars() {

  if (CGrate != CRrate) {
    CRrate = CGrate;
  } // Update rate

  if (CGtx != CRtx) {
    CRtx = CGtx;
  } //Update tx
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

// These next two functions are for polling the humidity sensor, in the code they implement as just ReadDHT()
// It must be run at most every 200 milliseconds, but any slower is fine
void ReadDHT() {
  bGlobalErr = 0;
  byte dht_in;
  byte i;
  digitalWrite(temp_pin, LOW);
  delay(20);
  digitalWrite(temp_pin, HIGH);
  delayMicroseconds(40);
  pinMode(temp_pin, INPUT);
  //delayMicroseconds(40);
  dht_in = digitalRead(temp_pin);
  if (dht_in) {
    bGlobalErr = 1;
    return;
  }
  delayMicroseconds(80);
  dht_in = digitalRead(temp_pin);
  if (!dht_in) {
    bGlobalErr = 2;
    return;
  }
  delayMicroseconds(80);
  for (i = 0; i < 5; i++)
    temp_data[i] = read_temp_data();
  pinMode(temp_pin, OUTPUT);
  digitalWrite(temp_pin, HIGH);
  byte dht_check_sum =
    temp_data[0] + temp_data[1] + temp_data[2] + temp_data[3];
  if (temp_data[4] != dht_check_sum)
  {
    bGlobalErr = 3;
  }
};
byte read_temp_data() {
  byte i = 0;
  byte result = 0;
  for (i = 0; i < 8; i++) {
    while (digitalRead(temp_pin) == LOW);
    delayMicroseconds(30);
    if (digitalRead(temp_pin) == HIGH)
      result |= (1 << (7 - i));
    while (digitalRead(temp_pin) == HIGH);
  }
  return result;
}


