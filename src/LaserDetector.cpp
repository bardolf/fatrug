#include "LaserDetector.h"

#define MEASUREMENTS 3
#define THRESHOLD_ON 750
#define THRESHOLD_OFF 500

LaserDetector::LaserDetector(byte out, byte in) {
    _out = out;
    _in = in;
}

void LaserDetector::init() {
    pinMode(_out, OUTPUT);
    Serial.begin(9600);
}

boolean LaserDetector::isObjectDetected() {
    byte detectedCount = 0;
    for (int i = 0; i < MEASUREMENTS; i++) {
        digitalWrite(_out, HIGH);                
        delayMicroseconds(10);
        unsigned int valueOn = analogRead(_in);
        Serial.println(valueOn);

        digitalWrite(_out, LOW);
        delayMicroseconds(90);
        unsigned int valueOff = analogRead(_in);

        if (valueOn > THRESHOLD_ON && valueOff < THRESHOLD_OFF) {
            detectedCount++;
        }
    }
    return detectedCount > MEASUREMENTS / 2;
}

boolean LaserDetector::isLaserOffValid() {
    for (int i = 0; i < 3; i++) {
        unsigned int value = analogRead(_in);
        if (value < THRESHOLD_OFF) {
            return true;
        }
    }
    return false;
}