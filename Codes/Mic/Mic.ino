int currentValue;
int maxValue;
int minValue;
unsigned long timer;
int sampleSpan = 5; // Amount in milliseconds to sample data
int volume; // this roughly goes from 0 to 700

void setup() 
{
    Serial.begin(9600); 
    resetValues();
}

void loop() 
{
    currentValue = analogRead(A0);

    if (currentValue < minValue) {
        minValue = currentValue;
    } 
    if (currentValue > maxValue) {
        maxValue = currentValue;
    }

    if (millis() - timer >= sampleSpan) {
        volume = maxValue - minValue;

        //Serial.println(volume);
        
        if(volume==1023)Serial.print("----------------");
        volume=volume/50;
        for(int i=0;i<=volume;i++)
          Serial.print("*");
        Serial.println();
        resetValues();
    }
}

void resetValues()
{
    maxValue = 0;
    minValue = 1024;
    timer = millis(); 
}
