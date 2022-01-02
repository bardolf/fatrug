#include "Communicator.h"

#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>
// #include <printf.h>

RF24 radio(7, 8);  // CE, CSN
const byte adresss[][6] = {"00001", "00002"};
const byte channel = 125;

Communicator::Communicator(boolean startDevice) {
    _startDevice = startDevice;
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

    radio.setPALevel(RF24_PA_MAX);
    radio.setDataRate(RF24_250KBPS);
    radio.setChannel(channel);
    radio.startListening(); 
}

void Communicator::send(const char* message) {
    radio.stopListening();    
    radio.write(message, sizeof(message));
    radio.startListening();
}

boolean Communicator::isMessageAvailable() {    
    return radio.available();
}

void Communicator::read(void* buf, uint8_t len) {
    radio.read(buf, len);
}