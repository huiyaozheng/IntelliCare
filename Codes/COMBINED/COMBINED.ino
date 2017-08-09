#define micPin A0
#define sensorIRPin1 A1
#define sensorIRPin2 A2
#define tempSensorPin 10
#define tempLEDpin 1
#define tempBuzzerpin 11
//definitions for alearting system
bool alerting=false;
bool alertFirstTime=false;
//
//definitons for temperature sensor
#define buzzerFreq 1000
int cntTemp=0;  // cycle counter
#define cycle 50 // how many cycles to average
#define thresholdTemp 32 //above which temperature alert starts
double sumTemp=0;
double currentTemp;
char buf[10];
double prevTemp=0;
//

//definitions for GSM
#include <GSM.h>
#define PINNUMBER ""
GSM gsmAccess; // include a 'true' parameter for debug enabled
GSM_SMS sms;
char remoteNumber[20]= "6591177498"; //remote number
unsigned long SMStimer;
//

//definitions for IR
float IRvalue;
float baseDistance1,baseDistance2;
//

//definition for LCD
#include<LiquidCrystal.h>
LiquidCrystal LCD(13,12,5,6,9,8);

//

//definition for Mic
int currentValue;
int maxValue;
int minValue;
unsigned long timer;
#define sampleSpan 5 // Amount in milliseconds to sample data
int volume; // this roughly goes from 0 to 700
int volThreshold=1000;
//----------------------------------------------------------------
void setup(){
  //Serial.begin(9600);
  Serial.println("Initializing");
  LCD.begin(16,2);//setup for LCD
  LCDprint("Initializing");
  //setup for temperature sensor
  pinMode(tempSensorPin,INPUT);
  pinMode(tempLEDpin,OUTPUT);
  digitalWrite(tempLEDpin,LOW);
  //
  
  //setup for GSM shield
  boolean notConnected = true;
  // If your SIM has PIN, pass it as a parameter of begin() in quotes
  /*
  LCDprint("GSM initializing");
  while(notConnected)
  {
    if(gsmAccess.begin(PINNUMBER)==GSM_READY)
      notConnected = false;
    else
    {
      Serial.println("GSM Not connected");
      delay(1000);
    }
  }
  SMStimer=0;
  Serial.println("GSM initialized");
  //*/
  
  resetValues();//setup for mic
  
  //setup for IR;
  baseDistance1=analogRead(sensorIRPin1);
  baseDistance1=10650.08*pow(baseDistance1,-0.935)-10;
  baseDistance2=analogRead(sensorIRPin2);
  baseDistance2=10650.08*pow(baseDistance2,-0.935)-10;
  //
  
  LCDprint("Working");
  for(int i=1;i<=cycle;i++)
    readTemp();
}
//----------------------------------------------------------------
void loop(){
  alerting=false;
  alertFirstTime=false;
  readTemp();
  alertFirstTime=false;
  readIR();
  alertFirstTime=false;
  readMic();
  if(alerting){
    tone(tempBuzzerpin,buzzerFreq);
    digitalWrite(tempLEDpin,HIGH);
    alerting=false;
  }else{
    noTone(tempBuzzerpin);
    digitalWrite(tempLEDpin,LOW);
    LCDprint("Monitoring");
  }
  delay(2000);
}
//----------------------------------------------------------------
void readTemp(){
  //dealing with temperature
  currentTemp=readPWM(tempSensorPin); //current reading
  if(currentTemp!=-1){
    for(int i=1;i<=cycle;i++){ 
      cntTemp++;
      currentTemp=(currentTemp-0.32)/0.0047;
      sumTemp+=currentTemp;
      currentTemp=readPWM(tempSensorPin); 
    }
    if(cntTemp>=cycle){
      sumTemp=sumTemp/cycle;
      Serial.print("current temperature:");
      Serial.println(sumTemp);
      prevTemp=sumTemp;
      if(round(sumTemp)>thresholdTemp){ //alerting
        if(alertFirstTime){
          alerting=true;
          LCDprint("Temp Too High!");
          sendSMS("Temperature too high!");
        }else{
          alertFirstTime=true;
          cntTemp=0;
          sumTemp=0;
          readTemp();
        }
      }
      cntTemp=0;
      sumTemp=0;
    }
  }
}
//----------------------------------------------------------------
double readPWM(int pin){  //temperature sensor reader
  long highTime=0;
  long lowTime=0;
  long tempPulse=pulseIn(pin,HIGH);
  if(tempPulse==0){  //testing for switch
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
//----------------------------------------------------------------
void readIR(){
  analogReference(EXTERNAL);
  IRvalue=analogRead(sensorIRPin1);
  IRvalue=10650.08*pow(IRvalue,-0.935)-10;
  if(IRvalue==6.34)return;
  Serial.print("Distance1 in cm: ");
  Serial.println(IRvalue);
  if(((baseDistance1-IRvalue)/baseDistance1)>0.2){
    if(alertFirstTime){
      LCDprint("Baby moving!");
      sendSMS("Your baby is moving!");
      alerting=true;}
    else{
      alertFirstTime=true;
      readIR();
    }
  }
  IRvalue=analogRead(sensorIRPin2);
  IRvalue=10650.08*pow(IRvalue,-0.935)-10;
  Serial.print("Distance2 in cm: ");
  Serial.println(IRvalue);
  if(((baseDistance2-IRvalue)/baseDistance2)>0.2){
    if(alertFirstTime){
      LCDprint("Baby moving!");
      sendSMS("Your baby is moving!");
      alerting=true;
    }else{
      alertFirstTime=true;
      readIR();
    }
  }
  analogReference(DEFAULT);
}
//----------------------------------------------------------------
void readMic(){
  analogReference(DEFAULT);
  for(int i=0;i<200;i++){
  currentValue = analogRead(micPin);

  if(currentValue<minValue){
    minValue=currentValue;
  } 
  if(currentValue>maxValue){
    maxValue=currentValue;
  }
  if(millis()-timer>=sampleSpan){
    volume=maxValue-minValue;
    Serial.print("Volume:");
    Serial.println(volume);
    if(volume>=volThreshold){
      if(alertFirstTime){
        Serial.println("------------------------------------");
        alerting=true;
        LCDprint("Baby crying!");
        sendSMS("Your baby is crying!");
      }else{
        alertFirstTime=true;
        readMic();
      }
    }
    resetValues();
  }
  }
}
//----------------------------------------------------------------
void resetValues() // for mic
{
  maxValue = 0;
  minValue = 1024;
  timer = millis(); 
}
//----------------------------------------------------------------
void sendSMS(char txt[]){ //sending SMS
  return;
  if(SMStimer!=0){
    if(millis()-SMStimer<300000)return;
    else SMStimer=millis();
  }else SMStimer=millis();
  Serial.print("Message to mobile number: ");
  Serial.println(remoteNumber);

  // sms text
  Serial.println("SENDING");
  Serial.println();
  Serial.println("Message:");
  Serial.println(txt);

  // send the message
  sms.beginSMS(remoteNumber);
  sms.print(txt);
  sms.endSMS(); 
  Serial.println("\nCOMPLETE!\n");  
}
//----------------------------------------------------------------
void LCDprint(char txt[]){
  LCD.clear();
  LCD.setCursor(0,0);
  LCD.print(txt);
  LCD.setCursor(0,1);
  LCD.print("Temp:");
  LCD.print(round(prevTemp));
}
