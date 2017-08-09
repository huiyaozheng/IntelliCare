int tempsensorpin=8;
int tempLEDpin=9;
int tempBuzzerpin=11;
int buzzerFreq=1000;
int cnt=0;
int thresholdTemp=30;
double sum=0;
void setup(){
  Serial.begin(9600);
  pinMode(tempsensorpin,INPUT);
  pinMode(tempLEDpin,OUTPUT);
  digitalWrite(tempLEDpin,LOW);
}
void loop(){
  double temp;
  temp=readPWM(tempsensorpin);
  if(temp!=-1){
    cnt++;
    temp=(temp-0.32)/0.0047;
    sum+=temp;
    if(cnt==100){
      sum=sum/100;
      Serial.println(sum);
      if(sum>thresholdTemp){
        tone(tempBuzzerpin,buzzerFreq);
        digitalWrite(tempLEDpin,HIGH);
      }else{ 
        digitalWrite(tempLEDpin,LOW);
        noTone(tempBuzzerpin);
      }
      cnt=0;
      sum=0;
    }
    delay(10);
  }else{ 
    digitalWrite(tempLEDpin,LOW);
    noTone(tempBuzzerpin);
  }
}

double readPWM(int pin){
  long highTime=0;
  long lowTime=0;
  long tempPulse=pulseIn(pin,HIGH);
  if(tempPulse==0){
    Serial.println("Temperature Sensor Disabled!");
    return -1;
  }
  if(tempPulse>highTime){
    highTime=tempPulse;
  }
  tempPulse=pulseIn(pin,LOW);
  if(tempPulse>lowTime){
    lowTime=tempPulse;
  }
  return ((highTime/(double(lowTime+highTime))));
}

