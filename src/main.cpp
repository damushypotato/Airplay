#include <Arduino.h>
#include <NewPing.h>
#include <LiquidCrystal_I2C.h>
#include <BleKeyboard.h>

// Define the pins for the ultrasonic sensor
// Trigger pin is connected to the ultrasonic sensor's trigger pin
// Echo pin is connected to the ultrasonic sensor's echo pin
// The ultrasonic sensor is powered by the ESP32's 5V pin
// The ultrasonic sensor's ground pin is connected to the ESP32's ground pin
#define TRIGGER_PIN  12
#define ECHO_PIN     14

// Maximum distance to measure in cm
// The ultrasonic sensor can measure distances up to 400 cm, but we set it to 30 cm for this project
int MAX_DISTANCE = 30;
// Polling rate for the ultrasonic sensor
// The ultrasonic sensor is polled every 50 milliseconds
int POLL_RATE = 50;
// Polling rate for the detection loop
// The detection loop is polled every 29 milliseconds
int DET_POLL_RATE = 29;

// these values can be adjusted to suit your needs
// skipHoldThreshold: number of detections to trigger skip
// pauseHoldThreshold: number of detections to trigger pause
// pauseMaxDistanceThreshold: maximum distance to trigger pause (in cm)
// prevHoldThreshold: number of detections to trigger previous track
// prevMaxDistanceThreshold: maximum distance to trigger previous track (in cm)
int skipHoldThreshold = 9;
int pauseHoldThreshold = 18;
int pauseMaxDistanceThreshold = 6;
int prevHoldThreshold = 20;
int prevMaxDistanceThreshold = 4;

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

LiquidCrystal_I2C lcd(0x27, 16, 2);

BleKeyboard bleKeyboard("Airplay", "KM Defence Corp", 100);

void setup() {
    Serial.begin(115200);

    bleKeyboard.begin();

    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);

    lcd.print("Airplay");
}

void loop() {
    // get the distance from the ultrasonic sensor
    unsigned int distance = sonar.ping_cm();

    // if the distance is 0, it means no object is detected
    if (distance == 0) {
        delay(POLL_RATE);
        return;
    }

    Serial.println("Object detected");

    unsigned long startMillis = millis();
    unsigned int detections = 0;
    unsigned int minDistance = MAX_DISTANCE;

    // if an object is detected, start the detection loop
    // the detection loop runs for 1 second and counts the number of detections
    while (millis() - startMillis < 1000) {
        unsigned int newDistance = sonar.ping_cm();

        if (newDistance != 0) {
            detections++;
        }
        if (newDistance < minDistance && newDistance != 0) {
            minDistance = newDistance;
        }

        delay(DET_POLL_RATE);
    }

    Serial.println("Number of detections: " + String(detections));
    Serial.println("Minimum distance: " + String(minDistance) + "cm");

    Serial.println("Object detection complete");

    //state - none = 0, SKIP = 1, PAUSE = 2, PREV = 3
    int state = 0;

    if (detections <= skipHoldThreshold) {
        state = 1;
    }
    else if (minDistance <= prevMaxDistanceThreshold) {
        if (detections >= prevHoldThreshold) {
            state = 3;
        }
    }
    if (detections >= pauseHoldThreshold && minDistance >= pauseMaxDistanceThreshold) {
        state = 2;
    }

    if (bleKeyboard.isConnected()) {
        switch (state)
        {
        case 1:
            bleKeyboard.write(KEY_MEDIA_NEXT_TRACK);
            break;
        case 2:
            bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
            break;
        case 3:
            bleKeyboard.write(KEY_MEDIA_PREVIOUS_TRACK);
            break;
        default:
            break;
        }
    }

    if (state == 1) {
        Serial.println("State: SKIP");

        lcd.setCursor(0, 1);
        lcd.print("SKIP");
    }
    else if (state == 2) {
        Serial.println("State: PAUSE");

        lcd.setCursor(0, 1);
        lcd.print("PAUSE");
    }
    else if (state == 3) {
        Serial.println("State: PREV");

        lcd.setCursor(0, 1);
        lcd.print("PREVIOUS");
    }
    else {
        Serial.println("State: NONE");
    }

    delay(1500);

    Serial.println("Ready...");
    lcd.setCursor(0, 1);
    lcd.print("                ");
}