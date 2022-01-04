#include "SevenSegments.h"

#define DOT1 1
#define DOT2 2
#define DOT3 4
#define DOT4 8

const byte digit[10] = {
    0b11000000,  // 0
    0b11111001,  // 1
    0b10100100,  // 2
    0b10110000,  // 3
    0b10011001,  // 4
    0b10010010,  // 5
    0b10000010,  // 6
    0b11111000,  // 7
    0b10000000,  // 8
    0b10010000,  // 9
};

const byte letter[44] = {
    0b11000000,  // 0
    0b11111001,  // 1
    0b10100100,  // 2
    0b10110000,  // 3
    0b10011001,  // 4
    0b10010010,  // 5
    0b10000010,  // 6
    0b11111000,  // 7
    0b10000000,  // 8
    0b10010000,  // 9

    0b11111111,  // EMPTY
    0b11111111,  // EMPTY
    0b11111111,  // EMPTY
    0b11111111,  // EMPTY
    0b11111111,  // EMPTY
    0b11111111,  // EMPTY
    0b11111111,  // EMPTY

    0b10001000,  // A
    0b10000011,  // B
    0b11000110,  // C
    0b10100001,  // D
    0b10000110,  // E
    0b10001110,  // F
    0b11000010,  // G
    0b10001011,  // H
    0b11001111,  // I
    0b11100001,  // J
    0b10001010,  // K
    0b11000111,  // L
    0b11101010,  // M
    0b11001000,  // N
    0b11000000,  // O
    0b10001100,  // P
    0b10011000,  // Q
    0b10101111,  // R
    0b10010010,  // S
    0b10000111,  // T
    0b11000001,  // U
    0b11010001,  // V
    0b11010101,  // W
    0b10001001,  // X
    0b10010001,  // Y
    0b10110100,  // Z
    0b11111111,  // EMPTY
};

const byte chr[4] = {
    0b00001000,
    0b00000100,
    0b00000010,
    0b00000001};

SevenSegments::SevenSegments(byte sclk, byte rclk, byte dio) {
    _sclk = sclk;
    _rclk = rclk;
    _dio = dio;
}

void SevenSegments::init() {
    pinMode(_rclk, OUTPUT);
    pinMode(_sclk, OUTPUT);
    pinMode(_dio, OUTPUT);    
}

void SevenSegments::showMessage(const char* message) {
    byte buffer[4];    
    for (int i = 0; i < 4; i++) {
        if (i >= strlen(message)) {
            buffer[i] = letter[43];
        } else {
            buffer[i] = letter[message[i] - '0'];
        }        
    }
    display(buffer, 0);
}

void SevenSegments::showTime(unsigned long millis) {    
    byte buffer[4];
    buffer[3] = digit[(millis / 10) % 10];
    buffer[2] = digit[(millis / 100) % 10];
    buffer[1] = digit[(millis / 1000) % 10];
    buffer[0] = digit[(millis / 10000) % 10];    
    display(buffer, DOT2);
}

void SevenSegments::display(byte chars[4], byte dots) {
    for (byte i = 0; i < 4; i++) {
        digitalWrite(_rclk, LOW);
        if (((1 << i) & dots) != 0) {
            shiftOut(_dio, _sclk, MSBFIRST, chars[i] & 0b01111111);
        } else {
            shiftOut(_dio, _sclk, MSBFIRST, chars[i]);
        }
        shiftOut(_dio, _sclk, MSBFIRST, chr[i]);
        digitalWrite(_rclk, HIGH);
    }
}
