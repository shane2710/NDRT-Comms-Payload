//These are the variables needed to define the temp sensor
#define temp_pin A0
byte bGlobalErr;
byte temp_data[5];
float temp_time;

//These are the variables for the shock sensor
#define shock_pin 7
int shock_data;
float shock_time;

// These two variables are used to store the temp and humidity data
int temp;
int humid;

// This stores the shock data
int shock;


float outtime;
// This variable is used for sending data to serial, not needed for actual operation

void setup() {
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

  // This is just for sending data to serial, not really needed
  Serial.begin(19200);
  delay(300);
  Serial.println("Sensor Comms online");
  delay(700);
}

void loop() {

  // This polls the humidity sensor and saves the data to 'temp' and 'humid'
  // To change the rate that it polls, change the number in the line below
  if (millis() - temp_time > 200) {
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
    temp_time = millis();
  }

  // This polls the shock sensor and saves the data to "shock".
  // It polls every loop of code, and saves data every 1000 milliseconds
  // The value is the number of times there has been shock in the last 1000 milliseconds 
  if (digitalRead(shock_pin) == LOW) {
    shock_data=shock_data+1;
  }
  if (millis() - shock_time > 1000) {
    shock=shock_data;
    shock_data=0;
    shock_time = millis();
  }


  // This sends data to serial, used for testing only
  if (millis() - outtime > 1000) {
    Serial.print(temp);
    Serial.print(",");
    Serial.print(humid);
    Serial.print(",");
    Serial.print(shock);
    Serial.println();
    outtime = millis();
  }


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
