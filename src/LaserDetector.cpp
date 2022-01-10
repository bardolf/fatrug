#include <DebugLog.h>
#include "LaserDetector.h"

#define ADJUSTMENT_TIME_MS 3000
#define THRESHOLD_SENSOR_HIGH 150
#define THRESHOLD_SENSOR_LOW 70

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
    // LOG_INFO(value);
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
        delayMicroseconds(500);
        if (!isSensorHigh()) {
            _lastUnadjustedSensorMillis = millis();            
        }
        _adjustmentPhaseHigh = false;
    } else {
        digitalWrite(_out, LOW);
        delayMicroseconds(500);
        if (!isSensorLow()) {
            _lastUnadjustedSensorMillis = millis();
        }
        _adjustmentPhaseHigh = true;
    }
    return false;
}