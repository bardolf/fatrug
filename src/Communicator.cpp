#include "Communicator.h"

#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>
// #include <printf.h>

#define ESTABLISH_INTERVAL_MS 500
#define ESTABLISH_START_MESSAGE "EST_START"
#define ESTABLISH_FINISH_MESSAGE "EST_FINISH"
#define PING_MESSAGE_REQUEST "PING"
#define PING_MESSAGE_RESPONSE "PONG"
#define LASER_1_ADJUSTMENT_INFO "LASER1_ADJ_REQ"
#define LASER_1_ADJUSTED_INFO "LASER1_ADJ_RES"
#define LASER_2_ADJUSTMENT_REQUEST "LASER2_REQ"
#define LASER_2_ADJUSTMENT_RESPONSE "LASER2_ADJUSTED"

RF24 radio(7, 8);  // CE, CSN
const byte adresss[][6] = {"00001", "00002"};
const byte channel = 125;
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

    if (_startDevice) {
        radio.openWritingPipe(adresss[1]);
        radio.openReadingPipe(1, adresss[0]);
    } else {
        radio.openWritingPipe(adresss[0]);
        radio.openReadingPipe(1, adresss[1]);
    }

    radio.setAutoAck(false);
    radio.setRetries(2, 1);
    radio.setPayloadSize(16);
    radio.setPALevel(RF24_PA_MAX);
    radio.setDataRate(RF24_2MBPS);
    radio.setChannel(channel);
    radio.startListening();
}

void Communicator::send(const char* message) {
    radio.stopListening();
    radio.write(message, strlen(message));
    radio.startListening();
}

boolean Communicator::isMessageAvailable() {
    return radio.available();
}

long Communicator::getLastMessageMillis() {
    return _lastMessageReceivedMillis;
}

void Communicator::read(void* buf, uint8_t len) {
    radio.read(buf, len);
    _lastMessageReceivedMillis = millis();
}

boolean Communicator::isCommunicationEstablished() {
    if (_startDevice) {
        if ((millis() - _establishedMessageSentMillis) > ESTABLISH_INTERVAL_MS) {
            _establishedMessageSentMillis = millis();
            send(ESTABLISH_START_MESSAGE);
        }
        if (isMessageAvailable()) {
            read(&buffer, sizeof(buffer));
            if (strcmp(buffer, ESTABLISH_FINISH_MESSAGE) == 0) {
                return true;
            }
        }
    } else {
        if (isMessageAvailable()) {
            read(&buffer, sizeof(buffer));
            if (strcmp(buffer, ESTABLISH_START_MESSAGE) == 0) {
                send(ESTABLISH_FINISH_MESSAGE);
                return true;
            }
        }
    }
    return false;
}

void Communicator::sendPingRequest() {
    send(PING_MESSAGE_REQUEST);
}

boolean Communicator::isPingRequest(const char* msg) {
    return strcmp(msg, PING_MESSAGE_REQUEST) == 0;
}

void Communicator::sendPingResponse() {
    send(PING_MESSAGE_RESPONSE);
}

boolean Communicator::isPingResponse(const char* msg) {
    return strcmp(msg, PING_MESSAGE_RESPONSE) == 0;
}

void Communicator::sendLaser1AdjustmentInfo() {
    send(LASER_1_ADJUSTMENT_INFO);
}

boolean Communicator::isLaser1AdjustmentInfo(const char* msg) {
    return strcmp(msg, LASER_1_ADJUSTMENT_INFO) == 0;
}

void Communicator::sendLaser1AdjustedInfo() {
    send(LASER_1_ADJUSTED_INFO);
}

boolean Communicator::isLaser1AdjustedInfo(const char* msg) {
    return strcmp(msg, LASER_1_ADJUSTED_INFO) == 0;
}

void Communicator::sendLaser2AdjustmentRequest() {
    send(LASER_2_ADJUSTMENT_REQUEST);
}

boolean Communicator::isLaser2AdjustmentRequest(const char* msg) {
    return strcmp(msg, LASER_2_ADJUSTMENT_REQUEST) == 0;
}

void Communicator::sendLaser2AdjustmentResponse() {
    send(LASER_2_ADJUSTMENT_RESPONSE);
}

boolean Communicator::isLaser2AdjustmentResponse(const char* msg) {
    return strcmp(msg, LASER_2_ADJUSTMENT_RESPONSE) == 0;
}
