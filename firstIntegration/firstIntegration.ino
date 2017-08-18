#include <Wire.h>
// for the OLED display
#include <SeeedOLED.h>

// Matrix LED
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
bool ifPrinted = false;
Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();
static const uint8_t PROGMEM smile_bmp[] = {B00111100, B01000010, B10100101,
                                            B10000001, B10100101, B10011001,
                                            B01000010, B00111100},
                             neutral_bmp[] = {B00111100, B01000010, B10100101,
                                              B10000001, B10111101, B10000001,
                                              B01000010, B00111100},
                             frown_bmp[] = {B00111100, B01000010, B10100101,
                                            B10000001, B10011001, B10100101,
                                            B01000010, B00111100},
                             exclamation_bmp[] = {
                                 B00011000, B00011000, B00011000, B00011000,
                                 B00011000, B00000000, B00011000, B00011000,
};

// sort algorithm
#include <ArduinoSort.h>

// Sound sensor
#define pinAdc A0
int lastValue = 0;

// Distance sensors:
#define LtrigPin 4
#define LechoPin 5

#define RtrigPin 6
#define RechoPin 7

#include <NewPing.h>
#define MAX_DISTANCE 200

NewPing sonarLeft(LtrigPin, LechoPin, MAX_DISTANCE);
NewPing sonarRight(RtrigPin, RechoPin, MAX_DISTANCE);

int LDefaultDistance;
int RDefaultDistance;

// Temp and Humidity
#include <dht.h>
dht DHT;
#define DHT22_PIN 8
#define TEMP_LOWER_BOUND 22
#define TEMP_UPPER_BOUND 27

// Touch switch
#define Touch 9
boolean isOn = false;

void setup() {
  Serial.begin(115200);

  // OLED display
  Wire.begin();
  SeeedOled.init();          // initialze SEEED OLED display
  SeeedOled.clearDisplay();  // clear the screen and set start position to top
                             // left corner
  SeeedOled
      .setNormalDisplay();  // Set display to normal mode (i.e non-inverse mode)
  SeeedOled.setHorizontalMode();  // Set addressing mode to Page Mode
  SeeedOled.setTextXY(0, 0);      // Set the cursor to Xth Page, Yth Column

  // Matrix display
  matrix.begin(0x70);  // pass in the address

  // Distance sensors
  LDefaultDistance = sonarLeft.ping_cm();
  RDefaultDistance = sonarRight.ping_cm();
}

void loop() {
  // Switch
  if (digitalRead(Touch) == 1) {
    isOn = !isOn;
    ifPrinted = false;
  }

  if (isOn) {
    SeeedOled.clearDisplay();
    // Matrix display
    if (!ifPrinted) {
      ifPrinted = true;
      matrix.clear();
      matrix.drawBitmap(0, 0, smile_bmp, 8, 8, LED_GREEN);
    }

    bool ifNormal = true;

    // Sound sensor

    long sum = 0;
    for (int i = 0; i < 16; i++) {
      sum += analogRead(pinAdc);
    }
    sum >>= 4;

    if (sum - lastValue > 180) {
      SeeedOled.putString("LOUD!!\n");
      matrix.clear();
      matrix.drawBitmap(0, 0, exclamation_bmp, 8, 8, LED_RED);
      ifPrinted = false;
      ifNormal = false;
    }
    lastValue = sum;

    Serial.println(sum);
    // use a counter instead of for loop
    delay(50);

    // Distance sensors
    int LCurrentDistance = sonarLeft.ping_cm();
    int RCurrentDistance = sonarRight.ping_cm();
    if (LCurrentDistance <= LDefaultDistance - 10 ||
        RCurrentDistance <= RDefaultDistance - 10) {
      SeeedOled.putString("The Baby is trying to crawl out!");
      matrix.clear();
      matrix.drawBitmap(0, 0, exclamation_bmp, 8, 8, LED_RED);
      ifPrinted = false;
      ifNormal = false;
    }

    // Temperature and Humidity
    DHT.read22(DHT22_PIN);
    if (DHT.temperature < TEMP_LOWER_BOUND ||
        DHT.temperature > TEMP_UPPER_BOUND) {
          SeeedOled.putString("The temperature is not comfortable!");
          matrix.clear();
          matrix.drawBitmap(0, 0, exclamation_bmp, 8, 8, LED_RED);
          ifPrinted = false;
          ifNormal = false;
    }

    if(ifNormal){
      SeeedOled.putString("All is OK.");
    }

    // Write display at the end after deciding what to display
    matrix.writeDisplay();
  } else {
    // OLED
    SeeedOled.clearDisplay();
    SeeedOled.putString("Monitoring is off.");
    // Matrix display
    if (!ifPrinted) {
      ifPrinted = true;
      matrix.clear();
      matrix.drawBitmap(0, 0, frown_bmp, 8, 8, LED_GREEN);
      matrix.writeDisplay();
    }
  }
}
