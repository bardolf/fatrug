#ifndef SevenSegments_h
#define SevenSegments_h

#include <Arduino.h>

class SevenSegments {
   public:
    SevenSegments(byte sclk, byte rclk, byte dio);
    void init();
    void showMessage(const char* message);
    void showTime(unsigned long millis);

   private:
    byte _sclk;
    byte _rclk;
    byte _dio;
    void display(byte chars[4], byte dots);
};

#endif