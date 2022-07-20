void setup()
{
    Serial.begin(2400);
    Serial.print("Hello World!");
}

 
int input;
 
void loop()
{
    if(Serial.available()>0)
    {
        input=Serial.read();
        Serial.print(input);
        if(input==11){
          Serial.print("ACK");
        }
    }
}
