#include "LaserDetector.h"

#define ADJUSTMENT_TIME_MS 3000
#define THRESHOLD_SENSOR_HIGH 750
#define THRESHOLD_SENSOR_LOW 400

LaserDetector::LaserDetector(byte out, byte in) {
    _out = out;
    _in = in;
}

void LaserDetector::init() {
    pinMode(_out, OUTPUT);
}

void LaserDetector::ledOn() {
    digitalWrite(_out, HIGH);
}

void LaserDetector::ledOff() {
    digitalWrite(_out, LOW);
}

boolean LaserDetector::isSensorHigh() {
    unsigned int value = analogRead(_in);
    return value > THRESHOLD_SENSOR_HIGH;
}

boolean LaserDetector::isSensorLow() {
    unsigned int value = analogRead(_in);
    return value < THRESHOLD_SENSOR_LOW;
}

void LaserDetector::startLaserAdjustment() {
    _lastUnadjustedSensorMillis = millis();
}

boolean LaserDetector::isLaserAdjusted() {
    if ((millis() - _lastUnadjustedSensorMillis) > ADJUSTMENT_TIME_MS) {
        return true;
    }
    if (_adjustmentPhaseHigh) {
        digitalWrite(_out, HIGH);
        delayMicroseconds(200);
        if (!isSensorHigh()) {
            _lastUnadjustedSensorMillis = millis();            
        }
        _adjustmentPhaseHigh = false;
    } else {
        digitalWrite(_out, LOW);
        delayMicroseconds(200);
        if (!isSensorLow()) {
            _lastUnadjustedSensorMillis = millis();
        }
        _adjustmentPhaseHigh = true;
    }
    return false;
}