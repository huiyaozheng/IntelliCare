// define the state of the system
#define NORMAL 0
#define TEMP_ALERT 1
#define SOUND_ALERT (1 << 1)
#define DIS_ALERT (1 << 2)

int old_state = NORMAL;

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
  SeeedOled.init();          // initialise SEEED OLED display
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
  delay(200);

  // if the button is touched
  if (digitalRead(Touch) == 1) {
    isOn = !isOn;
    if (!isOn) {
      // and it is turned off
      // clear the OLED
      SeeedOled.clearDisplay();
      // display it is off
      SeeedOled.putString("Monitoring is off.");

      // also for the matrix
      matrix.clear();
      matrix.drawBitmap(0, 0, frown_bmp, 8, 8, LED_YELLOW);
      matrix.writeDisplay();
    }
    // or it is just turned on
    else {
      // clear the OLED
      SeeedOled.clearDisplay();

      // also for the matrix
      matrix.clear();

      // set the old state to be normal
      old_state = NORMAL;
    }
  }

  if (isOn) {
    int current_state = NORMAL;

    // Sound sensor
    long sum = 0;
    for (int i = 0; i < 16; i++) {
      sum += analogRead(pinAdc);
    }
    sum >>= 4;

    if (sum - lastValue > 180) {
      // ifPrinted = false;
      current_state = current_state | SOUND_ALERT;
    }
    lastValue = sum;

    //Serial.println(sum);

    // Distance sensors
    int LCurrentDistance = sonarLeft.ping_cm();
    int RCurrentDistance = sonarRight.ping_cm();
    if (LCurrentDistance <= LDefaultDistance - 10 ||
        RCurrentDistance <= RDefaultDistance - 10) {
      current_state = current_state | DIS_ALERT;
    }

    // Temperature and Humidity
    DHT.read22(DHT22_PIN);
    if (DHT.temperature < TEMP_LOWER_BOUND ||
        DHT.temperature > TEMP_UPPER_BOUND) {
      current_state = current_state | TEMP_ALERT;
    }

    // display
    if (old_state != current_state) {
      // the current state is normal
      if (current_state == 0) {
        SeeedOled.putString("All is OK.");
        matrix.clear();
        matrix.drawBitmap(0, 0, smile_bmp, 8, 8, LED_GREEN);
      }
      // something went wrong
      else {
        // if current_state contains TEMP_ALERT
        string alert = "";
        if (current_state & TEMP_ALERT) {
          alert = alert + "The temperature is not comfortable!\n";
        }
        if (current_state & SOUND_ALERT) {
          alert = alert + "The Baby is crying!\n";
        }
        if (current_state & DIS_ALERT) {
          alert = alert + "The Baby is trying to climb out!";
        }
        SeeedOled.putString(alert);
        matrix.clear();
        matrix.drawBitmap(0, 0, exclamation_bmp, 8, 8, LED_RED);
      }
    }
    // Write display at the end after deciding what to display
    matrix.writeDisplay();
  }
}
