#ifndef Button_h
#define Button_h

#include <Arduino.h>

class Button {
   public:
    Button(byte pin);
    void init();
    boolean isPressed();    

   private:
    byte _pin;    
    unsigned long _lastReportedMillis;
};

#endif