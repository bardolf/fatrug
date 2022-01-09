#include "Communicator.h"

#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>
// #include <printf.h>

#define ESTABLISH_INTERVAL_MS 1000
#define ESTABLISH_REQUEST "EST_START"
#define ESTABLISH_RESPONSE "EST_FINISH"
#define PING_MESSAGE_REQUEST "PING"
#define PING_MESSAGE_RESPONSE "PONG"
#define LASER_1_ADJUSTMENT_INFO "LASER1_ADJ_RQ"
#define LASER_1_ADJUSTED_INFO "LASER1_ADJ_RES"
#define LASER_2_ADJUSTMENT_REQUEST "LASER2_RQ"
#define LASER_2_ADJUSTMENT_RESPONSE "LASER2_ADJUSTED"
#define STATE_READY_REQUEST "READY_RQ"
#define STATE_RUN_REQUEST "RUN_RQ"
#define STATE_FINISH_REQUEST "FIN_R"
#define STATE_FINISH_END_REQUEST "F_E_RQ"
#define LASER_2_INTERRUPTED "LASER2_INTR"

RF24 radio(7, 8);  // CE, CSN
const byte adresss[][6] = {"00001", "00002"};
const byte channel = 120;
char buffer[16];

Communicator::Communicator(boolean startDevice) {
    _startDevice = startDevice;
    _establishedMessageSentMillis = 0;
}

void Communicator::init() {
    radio.begin();

    // printf_begin();             // need to have #include <printf.h>
    // radio.printDetails();
    // delay(1000);

    radio.setAutoAck(false);
    radio.setRetries(1, 1);
    radio.setPayloadSize(sizeof(Payload));
    radio.setPALevel(RF24_PA_MAX);
    radio.setDataRate(RF24_250KBPS);
    radio.setChannel(channel);
    
    if (_startDevice) {
        radio.openWritingPipe(adresss[1]);
        radio.openReadingPipe(1, adresss[0]);
    } else {
        radio.openWritingPipe(adresss[0]);
        radio.openReadingPipe(1, adresss[1]);
    }    
    radio.startListening();
}

boolean Communicator::isMessageAvailable() {
    return radio.available();
}

void Communicator::read(Payload payload) {    
    radio.read(&payload, sizeof(payload));    
}

void Communicator::send(Payload payload) {    
    radio.stopListening();
    boolean success = radio.write(&payload, sizeof(payload));    
    radio.startListening();    
}

long Communicator::getLastMessageMillis() {
    return _lastMessageReceivedMillis;
}



// boolean Communicator::isCommunicationEstablished() {
//     if (_startDevice) {
//         if (isMessageAvailable()) {
//             read(buffer);
//             return isEstablishResponse(buffer);
//         }
//         if ((millis() - _establishedMessageSentMillis) > ESTABLISH_INTERVAL_MS) {
//             _establishedMessageSentMillis = millis();
//             sendEstablishRequest();
//         }
//     } else {
//         // reconnect
//          if (isMessageAvailable()) {
//             read(buffer);
//             if (isEstablishRequest(buffer)) {
//                 sendEstablishResponse();
//                 return true;
//             }
//         }
//         if ((millis() - _establishedMessageSentMillis) > ESTABLISH_INTERVAL_MS) {
//             _establishedMessageSentMillis = millis();
//             sendEstablishRequest();
//         }       
//     }
//     return false;
// }

void Communicator::sendEstablishRequest(uint8_t counter) {
    Payload payload;
    payload.counter = counter;
    memcpy(payload.message, ESTABLISH_REQUEST, strlen(ESTABLISH_REQUEST));
    send(payload);
}

boolean Communicator::isEstablishRequest(Payload payload) {
    return strcmp(payload.message, ESTABLISH_REQUEST) == 0;
}

// void Communicator::sendEstablishResponse() {
//     send(ESTABLISH_RESPONSE);
// }

// boolean Communicator::isEstablishResponse(const char* msg) {
//     return strcmp(msg, ESTABLISH_RESPONSE) == 0;
// }

// void Communicator::sendPingRequest() {
//     send(PING_MESSAGE_REQUEST);
// }

// boolean Communicator::isPingRequest(const char* msg) {
//     return strcmp(msg, PING_MESSAGE_REQUEST) == 0;
// }

// void Communicator::sendPingResponse() {
//     send(PING_MESSAGE_RESPONSE);
// }

// boolean Communicator::isPingResponse(const char* msg) {
//     return strcmp(msg, PING_MESSAGE_RESPONSE) == 0;
// }

// void Communicator::sendLaser1AdjustmentInfo() {
//     send(LASER_1_ADJUSTMENT_INFO);
// }

// boolean Communicator::isLaser1AdjustmentInfo(const char* msg) {
//     return strcmp(msg, LASER_1_ADJUSTMENT_INFO) == 0;
// }

// void Communicator::sendLaser1AdjustedInfo() {
//     send(LASER_1_ADJUSTED_INFO);
// }

// boolean Communicator::isLaser1AdjustedInfo(const char* msg) {
//     return strcmp(msg, LASER_1_ADJUSTED_INFO) == 0;
// }

// void Communicator::sendLaser2AdjustmentRequest() {
//     send(LASER_2_ADJUSTMENT_REQUEST);
// }

// boolean Communicator::isLaser2AdjustmentRequest(const char* msg) {
//     return strcmp(msg, LASER_2_ADJUSTMENT_REQUEST) == 0;
// }

// void Communicator::sendLaser2AdjustmentResponse() {
//     send(LASER_2_ADJUSTMENT_RESPONSE);
// }

// boolean Communicator::isLaser2AdjustmentResponse(const char* msg) {
//     return strcmp(msg, LASER_2_ADJUSTMENT_RESPONSE) == 0;
// }

// void Communicator::sendStateReadyRequest() {
//     send(STATE_READY_REQUEST);
// }

// boolean Communicator::isStateReadyRequest(const char* msg) {
//     return strcmp(msg, STATE_READY_REQUEST) == 0;
// }

// void Communicator::sendStateRunRequest() {
//     send(STATE_RUN_REQUEST);
// }

// boolean Communicator::isStateRunRequest(const char* msg) {
//     return strcmp(msg, STATE_RUN_REQUEST) == 0;
// }

// void Communicator::sendStateFinishRequest(long finishTime) {
//     char buffer[16];
//     sprintf(buffer, "%s%08ld", STATE_FINISH_REQUEST, finishTime);
//     send(buffer);
// }

// boolean Communicator::isStateFinishRequest(const char* msg) {
//     return strncmp(msg, STATE_FINISH_REQUEST, strlen(STATE_FINISH_REQUEST)) == 0;
// }

// long Communicator::getFinishTime(const char* msg) {
//     char sub[9];
//     memcpy(sub, &msg[strlen(STATE_FINISH_REQUEST)], 8);
//     return atol(sub);
// }

// void Communicator::sendStateFinishEndRequest() {
//     send(STATE_FINISH_END_REQUEST);
// }

// boolean Communicator::isStateFinishEndRequest(const char* msg) {
//     return strcmp(msg, STATE_FINISH_END_REQUEST) == 0;
// }

// void Communicator::sendLaser2Interrupted() {
//     send(LASER_2_INTERRUPTED);
// }

// boolean Communicator::isLaser2Interrupted(const char* msg) {
//     return strcmp(msg, LASER_2_INTERRUPTED) == 0;
// }