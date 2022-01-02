#include "Communicator.h"

#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>
// #include <printf.h>

#define ESTABLISH_INTERVAL_MS 500
#define ESTABLISH_START_MESSAGE "EST_START"
#define ESTABLISH_FINISH_MESSAGE "EST_FINISH"
#define PING_MESSAGE "PING"
#define PONG_MESSAGE "PONG"

RF24 radio(7, 8);  // CE, CSN
const byte adresss[][6] = {"00001", "00002"};
const byte channel = 125;
char buffer[32];

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
    radio.setRetries(2,1);  
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

void Communicator::read(void* buf, uint8_t len) {
    radio.read(buf, len);
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

void Communicator::sendPing() {
    send(PING_MESSAGE);
}

