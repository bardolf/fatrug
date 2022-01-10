#include "Communicator.h"

#include <DebugLog.h>
#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>
// #include <printf.h>

RF24 radio(7, 8);  // CE, CSN
const byte adresss[][6] = {"00001", "00002"};
const byte channel = 120;
char buffer[16];

Communicator::Communicator(boolean startDevice) {
    _startDevice = startDevice;
}

void Communicator::init() {
    radio.begin();

    // printf_begin();             // need to have #include <printf.h>
    // radio.printDetails();
    // delay(1000);

    radio.setAutoAck(false);
    radio.setRetries(1, 1);
    radio.setPayloadSize(sizeof(Message));
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

void Communicator::read(Message *message) {
    radio.read(message, sizeof(Message));
}

void Communicator::send(Message *message) {
    radio.stopListening();
    radio.write(message, sizeof(Message));
    radio.startListening();
}