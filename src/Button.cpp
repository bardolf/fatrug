#include "Button.h"

#define REPORT_THRESHOLD_MS 300

Button::Button(byte pin) {
    _pin = pin;
    _lastReportedMillis = millis();
}

void Button::init() {
    pinMode(_pin, INPUT);
}

boolean Button::isPressed() {
    if (digitalRead(_pin) == HIGH && ((millis() - _lastReportedMillis) > REPORT_THRESHOLD_MS)) {
        _lastReportedMillis = millis();
        return true;
    }
    return false;
}
