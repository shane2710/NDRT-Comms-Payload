char inbyte = 0;                 // Received byte
char buf[260];                   // Incoming data buffer
int buflen = 0;                  // Length of buffered ata

void setup()                     // Runs at startup
{
  Serial3.begin(4800);            // RadioShield runs at 4800 baud
  Serial.begin(19200);
  delay(3);                      // Allow time for RadioShield setup
}


void loop()                      // Runs constantly after startup
{
  while (Serial3.available() > 0) // Check for an incoming byte on the serial port
  {
    inbyte = Serial3.read();      // Get the byte
    if (inbyte == '\n')          // Check for end of line
    {
      Serial3.println("C");       // Clear screen
      delay(10);                 // Can take a bit to clear the LCD, so wait
      Serial.println(buf);
      buflen = 0;
    }
    else if (inbyte > 31 && buflen < 260)  // Only record printable characters
    {
      buf[buflen++] = inbyte;
      buf[buflen] = 0;
    }
  }  
}
