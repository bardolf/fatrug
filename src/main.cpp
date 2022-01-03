#include <Arduino.h>

#include "Button.h"
#include "Communicator.h"
#include "LaserDetector.h"
#include "SevenSegments.h"

#define STATE_START 0
#define STATE_LASER_1_ADJUSTMENT 1
#define STATE_LASER_2_ADJUSTMENT 2
#define STATE_PEERS_CONNECTION 3
#define STATE_READY 4
#define STATE_RUN 5
#define STATE_FINISH 6
#define STATE_REACT_TO_MESSAGE 7
#define STATE_PROCESS_MESSAGE 8
#define STATE_CHECK_SENSORS 9

/* HW CONFIGURATION */
#define SEVEN_SEGMENTS_SCLK_PIN 4
#define SEVEN_SEGMENTS_RCLK_PIN 6
#define SEVEN_SEGMENTS_DIO_PIN 5
#define LASER_DIODE_PIN 3
#define LASER_TRANSISTOR_PIN A5
#define BUTTON_PIN 9

/* TIME CONFIGURATION */
#define PING_INTERVAL_MS 500
#define UPDATE_LED_INTERVAL_MS 4

#define DEVICE_TYPE 1  // defines whether is it start (0) or finish (1) device

/* GLOBAL VARIABLES */
unsigned int currentState = STATE_START;
unsigned long runStartMillis;
unsigned long measuredTimeMillis = 0;
unsigned long changedStateMillis;

SevenSegments sevenSegments(SEVEN_SEGMENTS_SCLK_PIN, SEVEN_SEGMENTS_RCLK_PIN, SEVEN_SEGMENTS_DIO_PIN);
LaserDetector laserDetector(LASER_DIODE_PIN, LASER_TRANSISTOR_PIN);
Communicator communicator(DEVICE_TYPE == 0);
Button button(BUTTON_PIN);
char receivedMessage[16];

void setup() {
    Serial.begin(115200);
    sevenSegments.init();
    laserDetector.init();
    communicator.init();
    changedStateMillis = millis();
    button.init();
}

void loopStartDevice() {
    switch (currentState) {
        case STATE_START:
            sevenSegments.showMessage("STAR");
            Serial.println("Start");
            currentState = STATE_PEERS_CONNECTION;
            break;
        case STATE_PEERS_CONNECTION:
            sevenSegments.showMessage("CONN");
            if (communicator.isCommunicationEstablished()) {
                currentState = STATE_LASER_1_ADJUSTMENT;
                laserDetector.startLaserAdjustment();
                changedStateMillis = millis();
                communicator.sendLaser1AdjustmentInfo();
            }
            break;
        case STATE_LASER_1_ADJUSTMENT:
            sevenSegments.showMessage("LAS1");
            if (laserDetector.isLaserAdjusted()) {
                currentState = STATE_LASER_2_ADJUSTMENT;
                changedStateMillis = millis();
                communicator.sendLaser1AdjustedInfo();
                communicator.sendLaser2AdjustmentRequest();
            }
            break;
        case STATE_LASER_2_ADJUSTMENT:
            sevenSegments.showMessage("LAS2");
            if (communicator.isMessageAvailable()) {
                communicator.read(&receivedMessage, sizeof(receivedMessage));
                if (communicator.isLaser2AdjustmentResponse(receivedMessage)) {
                    Serial.println("Laser 2 adjusted.");
                    currentState = STATE_READY;
                    changedStateMillis = millis();
                }
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
            measuredTimeMillis = millis() - runStartMillis;
            sevenSegments.showTime(measuredTimeMillis);
            if (millis() - changedStateMillis > 10000) {
                currentState = STATE_FINISH;
                changedStateMillis = millis();
            }
            break;
        case STATE_FINISH:            
            sevenSegments.showTime(measuredTimeMillis);
            if (millis() - changedStateMillis > 10000) {
                currentState = STATE_READY;
                measuredTimeMillis = 0;
                changedStateMillis = millis();
            }
            break;
        default:
            break;
    }
}

void loopFinishDevice() {
    switch (currentState) {
        case STATE_START:
            sevenSegments.showMessage("STAR");
            Serial.println("Start");
            currentState = STATE_PEERS_CONNECTION;
            Serial.println("Peers connecting...");
            break;
        case STATE_PEERS_CONNECTION:
            sevenSegments.showMessage("CONN");
            if (communicator.isCommunicationEstablished()) {
                currentState = STATE_REACT_TO_MESSAGE;
                changedStateMillis = millis();
                Serial.println("Peers connected.");
            }
            break;
        case STATE_REACT_TO_MESSAGE:
            sevenSegments.showMessage("READ");
            if (communicator.isMessageAvailable()) {
                communicator.read(&receivedMessage, sizeof(receivedMessage));
                if (communicator.isLaser1AdjustmentInfo(receivedMessage)) {
                    currentState = STATE_LASER_1_ADJUSTMENT;
                } else if (communicator.isLaser2AdjustmentRequest(receivedMessage)) {
                    laserDetector.startLaserAdjustment();
                    currentState = STATE_LASER_2_ADJUSTMENT;
                    changedStateMillis = millis();
                }
            }
            break;
        case STATE_LASER_1_ADJUSTMENT:
            sevenSegments.showMessage("LAS1");
            if (communicator.isMessageAvailable()) {
                communicator.read(&receivedMessage, sizeof(receivedMessage));
                if (communicator.isLaser1AdjustedInfo(receivedMessage)) {
                    currentState = STATE_REACT_TO_MESSAGE;
                    changedStateMillis = millis();
                }
            }
            break;
        case STATE_LASER_2_ADJUSTMENT:
            sevenSegments.showMessage("LAS2");
            if (laserDetector.isLaserAdjusted()) {
                currentState = STATE_REACT_TO_MESSAGE;
                changedStateMillis = millis();
            }
            break;
        default:
            break;
    }
}

/**
 * Specifies whether is the start (master) device or not.
 * Usually based on harware configuration.
 */
boolean isStartDevice() {
    return DEVICE_TYPE == 0;
}

/**
 * Specifies whether is the finish (slave) device or not.
 * Usually based on harware configuration.
 */
boolean isFinishDevice() {
    return DEVICE_TYPE == 1;
}

void loop() {
    if (isStartDevice()) {
        loopStartDevice();
    } else {
        loopFinishDevice();
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

// boolean isMessageFromPeerAvailable() {

// }

// char* receiveMessageFromPeer() {

// }

// void sendMessageToPeer(String message) {

// }