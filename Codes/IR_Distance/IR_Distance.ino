int sensorIRPin=A1;
void setup(){
  Serial.begin(9600);
}
void loop(){
  float IRvalue=analogRead(sensorIRPin);
  IRvalue=10650.08*pow(IRvalue,-0.935)-10;
  Serial.print("Distance in cm: ");
  Serial.println(IRvalue);
  delay(500);
}
