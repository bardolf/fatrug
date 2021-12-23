#include <Arduino.h>

#include "Communicator.h"
#include "LaserDetector.h"
#include "SevenSegments.h"

#define STATE_START 0
#define STATE_LASER_ADJUSTMENT 1
#define STATE_PEERS_CONNECTED 2
#define STATE_READY 3
#define STATE_RUN 4
#define STATE_FINISH 5

/* HW CONFIGURATION */
#define SEVEN_SEGMENTS_SCLK_PIN 4
#define SEVEN_SEGMENTS_RCLK_PIN 6
#define SEVEN_SEGMENTS_DIO_PIN 5
#define LASER_DIODE_PIN 3
#define LASER_TRANSISTOR_PIN A5

/* TIME CONFIGURATION */
#define PING_INTERVAL_MS 500
#define UPDATE_LED_INTERVAL_MS 4

#define DEVICE_TYPE 0  // defines whether is it start (0) or finish (1) device

unsigned int currentState = STATE_START;
unsigned long runStartMillis;
unsigned long measuredTimeMillis = 0;
unsigned long changedStateMillis;

SevenSegments sevenSegments(SEVEN_SEGMENTS_SCLK_PIN, SEVEN_SEGMENTS_RCLK_PIN, SEVEN_SEGMENTS_DIO_PIN);
LaserDetector laserDetector(LASER_DIODE_PIN, LASER_TRANSISTOR_PIN);
Communicator communicator(DEVICE_TYPE == 0);

void setup() {
    Serial.begin(115200);
    sevenSegments.init();
    laserDetector.init();
    communicator.init();
    changedStateMillis = millis();
}

void loop() {
    switch (currentState) {
        case STATE_START:
            sevenSegments.showMessage("STAR");
            Serial.println("START");
            currentState = STATE_PEERS_CONNECTED;
            break;
        case STATE_PEERS_CONNECTED:
            sevenSegments.showMessage("CONN");
            if (millis() - changedStateMillis > 2000) {
                currentState = STATE_LASER_ADJUSTMENT;
                changedStateMillis = millis();
            }
            break;
        case STATE_LASER_ADJUSTMENT:
            sevenSegments.showMessage("LASE");
            Serial.println("Laser adjustment");
            laserDetector.ledOn();
            if (laserDetector.isLaserAdjusted()) {
                currentState = STATE_READY;
                changedStateMillis = millis();
            }
            break;        
        case STATE_READY:
            sevenSegments.showTime(measuredTimeMillis);
            if (millis() - changedStateMillis > 2000) {
                currentState = STATE_RUN;
                runStartMillis = millis();
                changedStateMillis = millis();
            }
            break;
        case STATE_RUN:
            laserDetector.ledOff();
            measuredTimeMillis = millis() - runStartMillis;
            sevenSegments.showTime(measuredTimeMillis);
            if (millis() - changedStateMillis > 10000) {
                currentState = STATE_FINISH;
                changedStateMillis = millis();
            }
            break;
        case STATE_FINISH:
            laserDetector.ledOn();
            sevenSegments.showTime(measuredTimeMillis);
            if (millis() - changedStateMillis > 10000) {
                currentState = STATE_LASER_ADJUSTMENT;
                measuredTimeMillis = 0;
                changedStateMillis = millis();
            }
            break;
        default:
            break;
    }

    // sevenSegments.showTime(millis());
    // if (laserDetector.isObjectDetected()) {
    //   sevenSegments.showMessage("EMPT");
    // } else {
    //   sevenSegments.showMessage("OBJ");
    // }

    //  Serial.println("111");
    // // communicator.send("X");
    // if (communicator.isMessageAvailable()) {
    //   Serial.println("xxx");
    //   char text[32] = "";
    //   communicator.read(&text, strlen(text));
    //   Serial.println(text);
    // }

    // Serial.println("1");
    // delay(1000);
}

/**
 * Specifies whether is the start (master) device or not.
 * Usually based on harware configuration.
 */
boolean isStartDevice() {
    return true;
}

/**
 * Specifies whether is the finish (slave) device or not.
 * Usually based on harware configuration.
 */
boolean isFinishDevice() {
    return false;
}

// boolean isMessageFromPeerAvailable() {

// }

// char* receiveMessageFromPeer() {

// }

// void sendMessageToPeer(String message) {

// }