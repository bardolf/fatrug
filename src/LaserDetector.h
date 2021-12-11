#ifndef LaserDetector_h
#define LaserDetector_h

#include <Arduino.h>

class LaserDetector {
   public:
    LaserDetector(byte out, byte in);
    void init();
    boolean isObjectDetected();
    boolean isLaserOffValid();

   private:
    byte _out;
    byte _in;
};

#endif