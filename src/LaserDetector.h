#ifndef LaserDetector_h
#define LaserDetector_h

#include <Arduino.h>

class LaserDetector {
   public:
    LaserDetector(byte out, byte in);
    void init();
    boolean isSensorHigh();
    boolean isSensorLow();
    boolean isLaserOffValid();
    
    void startLaserAdjustment();
    boolean isLaserAdjusted();
    void ledOn();
    void ledOff();

   private:
    byte _out;
    byte _in;
    long _lastUnadjustedSensorMillis;
    boolean _adjustmentPhaseHigh;
};

#endif