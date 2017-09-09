// define the state of the system
const int NORMAL = 0;
const int TEMP_ALERT = 1;
const int SOUND_ALERT = 2;
const int DIS_ALERT = 4;

// To ensure there is a change of state at startup.
int old_state = -1;

// Save consecutive states
const int NO_OF_STATES = 5;
int states[NO_OF_STATES];

#define MAX_LOOP_BEFORE_ALERT 5
const int LOOP_OF_ALERT = 5;

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
static const uint8_t PROGMEM on_bmp[] = {B00000000, B01001001, B10101101,
                                         B10101101, B10101011, B10101011,
                                         B01001001, B00000000},
                             off_bmp[] = {B00000000, B01011011, B10110010,
                                          B10110010, B10111011, B10110010,
                                          B01010010, B00000000},
                             alert_bmp[] = {B11111111, B11111111, B11111111,
                                            B11111111, B11111111, B11111111,
                                            B11111111, B11111111};
// control the blinking of matrix when there is an alert
bool blinkOn = true;

// Sound sensor
#define pinAdc A0
int lastSoundValue = 0;
const int SOUND_THRESHOLD = 30;

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
#define TEMP_UPPER_BOUND 25

// Touch switch
#define Touch 9
boolean isOn = false;

// Refresh the thresholds and states.
void initialize(){
  // Distance sensors
  LDefaultDistance = sonarLeft.ping_cm();
  RDefaultDistance = sonarRight.ping_cm();
  
  // Fill the initial states
  for (int i = 0; i < NO_OF_STATES; ++i) {
    states[i] = checkSensors();
  }
}

void setup() {
  Serial.begin(115200);

  // OLED display
  Wire.begin();
  SeeedOled.init();          // initialise SEEED OLED display
  SeeedOled.clearDisplay();  // clear the screen and set start position to top left corner
  SeeedOled.setNormalDisplay();  // Set display to normal mode (i.e non-inverse mode)
  SeeedOled.setHorizontalMode();  // Set addressing mode to Page Mode
  SeeedOled.setTextXY(0, 0);      // Set the cursor to Xth Page, Yth Column

  // Matrix display
  matrix.begin(0x70);  // pass in the address
  matrix.clear();
  matrix.writeDisplay();
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

// Read the sensors.
int checkSensors() {
  int current_state = NORMAL;

  // Sound sensor
  long avgSoundVal = 0;
  for (int i = 0; i < 8; i++) {
    avgSoundVal += analogRead(pinAdc);
  }
  avgSoundVal >>= 4;
  // if the volume is large
  if (abs(avgSoundVal - lastSoundValue) > SOUND_THRESHOLD) {
    current_state = current_state | SOUND_ALERT;
  }
  lastSoundValue = avgSoundVal;

  // Distance sensors
  int LCurrentDistance = sonarLeft.ping_cm();
  int RCurrentDistance = sonarRight.ping_cm();
  if (abs(LDefaultDistance - LCurrentDistance) >= DISTANCE_THRESHOLD ||
      abs(RDefaultDistance - RCurrentDistance) >= DISTANCE_THRESHOLD) {
    current_state = current_state | DIS_ALERT;
  }

  // Temperature
  DHT.read22(DHT22_PIN);
  if (DHT.temperature < TEMP_LOWER_BOUND ||
      DHT.temperature > TEMP_UPPER_BOUND) {
    current_state = current_state | TEMP_ALERT;
  }

  printData(avgSoundVal, LCurrentDistance, RCurrentDistance, DHT.temperature);

  return current_state;
}

// Check if abnormal state persists for some time.
int checkStates() {
  int ret;
  if (states[NO_OF_STATES - 1] & SOUND_ALERT != 0) {
    ret = SOUND_ALERT;
  } else {
    ret = 0;
  }
  int temp = 0;
  int dis = 0;
  for (int i = 0; i < NO_OF_STATES; ++i) {
    if (states[i] & TEMP_ALERT) temp++;
    if (states[i] & DIS_ALERT) dis = dis+1;
  } 
  if (temp + 2 >= NO_OF_STATES) ret = ret | TEMP_ALERT;
  if (dis + 2 >= NO_OF_STATES) ret = ret | DIS_ALERT;
  return ret;
}

void loop() {
  delay(100);

  // If the button is touched
  if (digitalRead(Touch) == 1) {
    isOn = !isOn;
    if (!isOn) {
      // and it is turned off
      // Clear the OLED.
      SeeedOled.clearDisplay();
      SeeedOled.putString("Monitoring is off.");

      // Clear the matrix.
      matrix.clear();
      matrix.drawBitmap(0, 0, off_bmp, 8, 8, LED_YELLOW);
      matrix.writeDisplay();

      delay(2000);
      matrix.clear();
      matrix.writeDisplay();
    }
    // or it is just turned on
    else {
      // Clear the OLED.
      SeeedOled.clearDisplay();

      // Clear the matrix.
      matrix.clear();

      matrix.drawBitmap(0, 0, on_bmp, 8, 8, LED_YELLOW);
      matrix.writeDisplay();
      delay(2000);
      initialize();
      matrix.clear();

      // Set the old state such that there is always change of output when it is turned on.
      old_state = -1;
    }
  }

  if (isOn) {
    // Rolling states
    for (int i = 0; i < NO_OF_STATES - 1; ++i) {
      states[i] = states[i + 1];
    }
    states[NO_OF_STATES - 1] = checkSensors();

    int current_state = checkStates();    

    String alert = "";
    if (current_state != 0) {
      remainingAlertLoops = LOOP_OF_ALERT;
      if (current_state & SOUND_ALERT) {
        alert = alert + "The Baby is crying!\n";
      }
      if (current_state & TEMP_ALERT || current_state & DIS_ALERT) {
        if (current_state & TEMP_ALERT) {
          alert = alert + "The temperature is not comfortable!\n";
        }
        if (current_state & DIS_ALERT) {
          alert = alert + "The Baby is trying to climb out!";
        }
      }
    }
    if (current_state != old_state) {
      SeeedOled.clearDisplay();
      if (remainingAlertLoops > 0) {
        int alertLength = alert.length();
        char alertCharArray[alertLength];
        alert.toCharArray(alertCharArray, alertLength);
        SeeedOled.putString(alertCharArray);
      } else {
        SeeedOled.putString("All is OK.");
      }
    }

    // Blink the matrix.
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
    // Write display at the end after deciding what to display.
    matrix.writeDisplay();
    // Set the old state to the current state.
    old_state = current_state;
  }
}
