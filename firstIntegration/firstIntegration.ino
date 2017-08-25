// define the state of the system
#define NORMAL 0
#define TEMP_ALERT 1
#define SOUND_ALERT (1 << 1)
#define DIS_ALERT (1 << 2)

// To ensure there is a change of state at startup.
int old_state = -1;

#define MAX_LOOP_BEFORE_ALERT 5
#define LOOP_OF_ALERT 5

int currentErrorLoopCount = 0;
int remainingAlertLoops = LOOP_OF_ALERT;

#include <Wire.h>
// for the OLED display
#include <SeeedOLED.h>

// Matrix LED
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
bool ifPrinted = false;
Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();
static const uint8_t PROGMEM on_bmp[] = {B00000000, 
                                            B01001001, 
                                            B10101101,
                                            B10101101, 
                                            B10101011, 
                                            B10101011,
                                            B01001001, 
                                            B00000000},                            
                             off_bmp[] = {B00000000, 
                                            B01011011, 
                                            B10110010,
                                            B10110010, 
                                            B10111011, 
                                            B10110010,
                                            B01010010, 
                                            B00000000},
                             alert_bmp[] = {
                                 B11111111, B11111111, B11111111, B11111111,
                                 B11111111, B11111111, B11111111, B11111111
};
// control the blinking of matrix when there is an alert
bool blinkOn = true;

// Sound sensor
#define pinAdc A0
int lastSoundValue = 0;
#define SOUND_THRESHOLD 100

// Distance sensors:
#define LtrigPin 4
#define LechoPin 5

#define RtrigPin 6
#define RechoPin 7

#include <NewPing.h>
#define MAX_DISTANCE 200
#define DISTANCE_THRESHOLD 5

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
  matrix.clear();
  matrix.writeDisplay();

  // Distance sensors
  LDefaultDistance = sonarLeft.ping_cm();
  RDefaultDistance = sonarRight.ping_cm();
}

void printData(long soundVal, int leftDist, int rightDist, float temperature) {
  Serial.print("Sound:");
  Serial.println(soundVal);
  Serial.print("Left distance:");
  Serial.println(leftDist);
  Serial.print("Right distance");
  Serial.println(rightDist);
  Serial.print("Temperature:");
  Serial.println(temperature);
  Serial.println();
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
      matrix.drawBitmap(0, 0, off_bmp, 8, 8, LED_YELLOW);
      matrix.writeDisplay();
      // turn off the display
      delay(2000);
      matrix.clear();
      matrix.writeDisplay();
    }
    // or it is just turned on
    else {
      // clear the OLED
      SeeedOled.clearDisplay();

      // also clear the matrix
      matrix.clear();

      // to show the system is turned on
      matrix.drawBitmap(0, 0, on_bmp, 8, 8, LED_YELLOW);
      matrix.writeDisplay();
      delay(2000);
      matrix.clear();

      // set the old state to be normal
      old_state = NORMAL;
    }
  }

  if (isOn) {
    int current_state = NORMAL;

    // Sound sensor
    long avgSoundVal = 0;
    for (int i = 0; i < 16; i++) {
      avgSoundVal += analogRead(pinAdc);
    }
    avgSoundVal >>= 4;
    // if the volume is large
    if (avgSoundVal - lastSoundValue > SOUND_THRESHOLD) {
      // ifPrinted = false;
      current_state = current_state | SOUND_ALERT;
    }
    lastSoundValue = avgSoundVal;

    // Distance sensors
    int LCurrentDistance = sonarLeft.ping_cm();
    int RCurrentDistance = sonarRight.ping_cm();
    if (LDefaultDistance - LCurrentDistance >= DISTANCE_THRESHOLD ||
        RDefaultDistance - RCurrentDistance >= DISTANCE_THRESHOLD) {
      current_state = current_state | DIS_ALERT;
    }

    // Temperature
    DHT.read22(DHT22_PIN);
    if (DHT.temperature < TEMP_LOWER_BOUND ||
        DHT.temperature > TEMP_UPPER_BOUND) {
      current_state = current_state | TEMP_ALERT;
    }

    printData(avgSoundVal, LCurrentDistance, RCurrentDistance, DHT.temperature);

    // display
    if (old_state != current_state) {
      SeeedOled.clearDisplay();
      // the current state is normal
      if (current_state == 0) {
        currentErrorLoopCount = 0;
        remainingAlertLoops = 0;
        SeeedOled.putString("All is OK.");
        matrix.clear();
      }
      // something went wrong
      else {
        String alert = "";
        if (current_state & SOUND_ALERT) {
          alert = alert + "The Baby is crying!\n";
          remainingAlertLoops = LOOP_OF_ALERT;
          blinkOn = true;
        }
        if (current_state & TEMP_ALERT || current_state & DIS_ALERT) {
          if (current_state & TEMP_ALERT) {
            alert = alert + "The temperature is not comfortable!\n";
          }
          if (current_state & DIS_ALERT) {
            alert = alert + "The Baby is trying to climb out!";
          }
          currentErrorLoopCount++;
          if (currentErrorLoopCount >= MAX_LOOP_BEFORE_ALERT) {
            currentErrorLoopCount = 0;
            remainingAlertLoops = LOOP_OF_ALERT;
            blinkOn = true;
          }
        }

        int alertLength = alert.length();
        char alertCharArray[alertLength];
        alert.toCharArray(alertCharArray, alertLength);
        SeeedOled.putString(alertCharArray);
        matrix.clear();
        matrix.drawBitmap(0, 0, alert_bmp, 8, 8, LED_RED);
      }
    }
    // to blink the matrix
    if (remainingAlertLoops > 0) {
      if (blinkOn) {
        matrix.drawBitmap(0, 0, alert_bmp, 8, 8, LED_RED);
      } else {
        matrix.clear();
      }
      blinkOn = !blinkOn;
      remainingAlertLoops--;
    } else {
      matrix.clear();
    }
    // Write display at the end after deciding what to display
    matrix.writeDisplay();
    // set the old state to the current state
    old_state = current_state;
  }
}
