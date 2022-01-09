#ifndef Communicator_h
#define Communicator_h

#include <Arduino.h>

struct Payload {
  uint8_t counter;
  char message[16];
};

class Communicator {
   public:
    Communicator(boolean startDevice);
    void init();
    boolean isMessageAvailable();
    void read(Payload payload);
    void send(Payload payload);
    boolean isCommunicationEstablished();
    long getLastMessageMillis();

    void sendEstablishRequest(uint8_t counter);
    boolean isEstablishRequest(Payload payload);
    // void sendEstablishResponse();
    // boolean isEstablishResponse(const char* msg); 
    // void sendPingRequest();
    // boolean isPingRequest(const char* msg);
    // void sendPingResponse();
    // boolean isPingResponse(const char* msg);
    
    
    // void sendLaser1AdjustmentInfo();
    // boolean isLaser1AdjustmentInfo(const char* msg);
    // void sendLaser1AdjustedInfo();
    // boolean isLaser1AdjustedInfo(const char* msg);
    // void sendLaser2AdjustmentRequest();
    // boolean isLaser2AdjustmentRequest(const char* msg);
    // void sendLaser2AdjustmentResponse();
    // boolean isLaser2AdjustmentResponse(const char* msg);
    // void sendStateReadyRequest();
    // boolean isStateReadyRequest(const char* msg);
    // void sendStateRunRequest();
    // boolean isStateRunRequest(const char* msg);
    // void sendStateFinishRequest(long finishTime);
    // boolean isStateFinishRequest(const char* msg);
    // long getFinishTime(const char* msg);
    // void sendStateFinishEndRequest();
    // boolean isStateFinishEndRequest(const char* msg);
    // void sendLaser2Interrupted();
    // boolean isLaser2Interrupted(const char* msg);

   private:
    boolean _startDevice;
    long _establishedMessageSentMillis;
    long _lastMessageReceivedMillis;
};

#endif