#include<LiquidCrystal.h>
LiquidCrystal lcd(12,11,5,4,3,2);
const int switchPin=6;
int switchState=0;
int prevSwitchState=0;
int reply;
void setup(){
  lcd.begin(16,2);
  pinMode(switchPin,INPUT);
  lcd.print("Ask the");
  lcd.setCursor(0,1);
  lcd.print("Cystal Ball!");
}
void loop(){
  switchState=digitalRead(switchPin);
  if(switchState!=prevSwitchState){
    if(switchState==LOW){
      reply=random(8);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("The ball says:");
      lcd.setCursor(0,1);
      switch(reply){
        case 0:
          lcd.print("1");
          break;
        case 1:
          lcd.print("2");
          break;
        case 2:
          lcd.print("3");
          break;
        case 3:
          lcd.print("4");
          break;
        case 4:
          lcd.print("5");
          break;
        case 5:
          lcd.print("6");
          break;
        case 6:
          lcd.print("7");
          break;
        case 7:
          lcd.print("8");
          break;
      }
    }
  }
  prevSwitchState=switchState;
}
