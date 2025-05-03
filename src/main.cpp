#include <Arduino.h>
#include <NewPing.h>
#include <LiquidCrystal_I2C.h>
#include <BleKeyboard.h>

#define TRIGGER_PIN  12
#define ECHO_PIN     14

int MAX_DISTANCE = 30;
int POLL_RATE = 50;
int DET_POLL_RATE = 29;

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
    unsigned int distance = sonar.ping_cm();

    if (distance == 0) {
        delay(POLL_RATE);
        return;
    }

    Serial.println("Object detected");

    unsigned long startMillis = millis();
    unsigned int detections = 0;
    unsigned int minDistance = MAX_DISTANCE;

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