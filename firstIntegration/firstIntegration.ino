#include <Wire.h>
// for the OLED display
#include <SeeedOLED.h>

// Matrix LED
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
bool ifPrinted = false;
Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();
static const uint8_t PROGMEM
  smile_bmp[] =
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10100101,
    B10011001,
    B01000010,
    B00111100 },
  neutral_bmp[] =
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10111101,
    B10000001,
    B01000010,
    B00111100 },
  frown_bmp[] =
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10011001,
    B10100101,
    B01000010,
    B00111100 };



// Sound sensor
#define pinAdc A0
int lastValue = 0;

//Distance sensors:
#define LtrigPin 4
#define LechoPin 5

#define RtrigPin 6
#define RechoPin 7

//Temp and Humidity
#define THPin 8


//Touch switch
#define Touch 9
boolean isOn = false;



void setup()
{
    Serial.begin(115200);
    

    // OLED display
    Wire.begin();
    SeeedOled.init();  //initialze SEEED OLED display
    SeeedOled.clearDisplay();          //clear the screen and set start position to top left corner
    SeeedOled.setNormalDisplay();      //Set display to normal mode (i.e non-inverse mode)
    SeeedOled.setHorizontalMode();           //Set addressing mode to Page Mode
    SeeedOled.setTextXY(0,0);          //Set the cursor to Xth Page, Yth Column  

    // Matrix display 
    matrix.begin(0x70);  // pass in the address

}

void loop()
{
    // Switch
    isOn = (digitalRead(Touch) == 1) ? (!isOn) : isOn;


    if (isOn){

        //Sound sensor
        long values[4];
        lastValue = 0;
        for(int j = 0; j < 4; j++){
            long sum = 0;
            for(int i = 0; i < 16; i++){
                sum += analogRead(pinAdc);
            }
            sum >>= 4;
            values[j] = sum;
            if (sum - lastValue > 200){
                SeeedOled.putString("LOUD!!\n");
            }
            lastValue = sum;
            Serial.println(sum);
            // use a counter instead of for loop
            // add median
            delay(50);
        }

        // Matrix display
        if(!ifPrinted){

        }
        matrix.clear();
        matrix.drawBitmap(0, 0, smile_bmp, 8, 8, LED_GREEN);
        matrix.writeDisplay();}
    
    
    
    delay(500);


    }

    else{
            // Matrix display
    matrix.clear();
    matrix.drawBitmap(0, 0, frown_bmp, 8, 8, LED_RED);
    matrix.writeDisplay();
    delay(500);
    }

  





    
    delay(50);
    

}

