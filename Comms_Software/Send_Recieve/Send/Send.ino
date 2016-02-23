int counter = 1;

void setup()
{
  Serial.begin(4800);                      //OPEN SERIAL LINE AT 4800
  delay(3);
  Serial.print("KC9TMD\r\n");               //SET YOUR CALLSIGN HERE, HERE YOU SEE W1AW
  delay(10);                       
  Serial.print("PWIDE1-1,WIDE2-1\r\n");    //SET DIGIPATH HERE
  delay(10);
}

void loop()
{
  Serial.print("!>This is RadioShield test message #");     //BEGIN MESSAGE BUT DON'T SEND YET...
  Serial.print(counter);                                    //  ...CONCATENATE VALUE OF count TO OUTPUT...
  Serial.print("\r\n");                                     //    ...SEND CR/LF TO COMPLETE AND TRANSMIT PACKET.
  counter++;
  delay(3000);                                             //3000ms = 3sec
}
