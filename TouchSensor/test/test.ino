const int TouchPin=5;

void setup() {
    pinMode(TouchPin, INPUT);
}

void loop() {
    int sensorValue = digitalRead(TouchPin);
    if(sensorValue==1)
    {
        Serial.println("touched");
    }
    delay(1000);
}
