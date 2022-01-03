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
    boolean isCommunicationEstablished();
    long getLastMessageMillis();

    void sendPingRequest();
    boolean isPingRequest(const char* msg);
    void sendPingResponse();
    boolean isPingResponse(const char* msg);
    void sendLaser1AdjustmentInfo();
    boolean isLaser1AdjustmentInfo(const char* msg);
    void sendLaser1AdjustedInfo();
    boolean isLaser1AdjustedInfo(const char* msg);
    void sendLaser2AdjustmentRequest();
    boolean isLaser2AdjustmentRequest(const char* msg);
    void sendLaser2AdjustmentResponse();
    boolean isLaser2AdjustmentResponse(const char* msg);

   private:
    boolean _startDevice;
    long _establishedMessageSentMillis;
    long _lastMessageReceivedMillis;
};

#endif