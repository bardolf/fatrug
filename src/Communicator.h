#ifndef Communicator_h
#define Communicator_h

#include <Arduino.h>

struct Message {
    uint8_t counter;
    char text[16];
};

class Communicator {
   public:
    Communicator(boolean startDevice);
    void init();
    boolean isMessageAvailable();
    void read(Message *message);
    void send(Message *message);

   private:
    boolean _startDevice;
};

#endif