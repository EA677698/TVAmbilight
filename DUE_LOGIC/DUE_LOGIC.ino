void setup()
{
    Serial.begin(2400);
    Serial.print("Hello World!\n");
    Serial.print("Hello World!\n");
}

 
int input;
 
void loop()
{
    if(Serial.available()>0)
    {
        Serial.print("PASSED");
        input=Serial.read();
        Serial.println(input, DEC);
        if(input==11){
          Serial.print("ACK");
        }
    }
}
