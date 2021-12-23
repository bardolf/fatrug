#ifndef Communicator_h
#define Communicator_h

#include <Arduino.h>

class Communicator {
   public:
    Communicator(boolean startDevice);
    void init();
    void send(const char* message);
    boolean isMessageAvailable();
    void read(void* buf, uint8_t len);

   private:
    boolean _startDevice;
};

#endif