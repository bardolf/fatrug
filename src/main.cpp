#include <Arduino.h>
#include <SoftwareSerial.h>
// #define DEBUGLOG_DISABLE_LOG     //uncomment for disabling the log
#include <DebugLog.h>

#include "Button.h"
#include "Communicator.h"
#include "LaserDetector.h"
#include "SevenSegments.h"

#define DEVICE_TYPE 0   // defines whether is it start (0) or finish (1) device

#define STATE_START 0
#define STATE_LASER_1_ADJUSTMENT 1
#define STATE_LASER_2_ADJUSTMENT 2
#define STATE_PEERS_CONNECTION 3
#define STATE_READY 4
#define STATE_RUN 5
#define STATE_FINISH 6
#define STATE_WAIT_FOR_MESSAGE 7
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
#define ACK_REPETITION_MICROS 50
#define ACK_TIMEOUT_MICROS 2000



/* GLOBAL VARIABLES */
unsigned int currentState = STATE_START;
unsigned long runStartMillis;
unsigned long measuredTimeMillis = 0;
unsigned long changedStateMillis;

Payload sentPayload;
Payload receivedPayload;
unsigned long messageSentMicros;
unsigned long messageReSentMicros;
boolean waitingForAck = false;
uint8_t lastReceivedCounter = 0;
uint8_t sendReceivedCounter = 0;


SevenSegments sevenSegments(SEVEN_SEGMENTS_SCLK_PIN, SEVEN_SEGMENTS_RCLK_PIN, SEVEN_SEGMENTS_DIO_PIN);
LaserDetector laserDetector(LASER_DIODE_PIN, LASER_TRANSISTOR_PIN);
Communicator communicator(DEVICE_TYPE == 0);
Button button(BUTTON_PIN);
SoftwareSerial btSerial(10, 2); // RX, TX

char receivedMessage[16];
boolean messageAvailable = false;

void setup() {
    LOG_SET_LEVEL(DebugLogLevel::LVL_DEBUG);
    LOG_ERROR("this is error log");
    LOG_WARN("this is warn log");
    LOG_INFO("this is info log");
    LOG_DEBUG("this is debug log");
    LOG_TRACE("this is trace log");


    Serial.begin(115200);
    sevenSegments.init();
    laserDetector.init();
    communicator.init();
    changedStateMillis = millis();
    button.init();
    btSerial.begin(9600);
}

void loopStartDevice() {
     LOG_ERROR("this is error log");
    messageAvailable = false;
    if (communicator.isMessageAvailable()) {
        messageAvailable = true;
        communicator.read(receivedMessage);
    }

    switch (currentState) {
        case STATE_START:
            sevenSegments.showMessage("STAR");            
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
             if (messageAvailable) {                
                if (communicator.isEstablishRequest(receivedMessage)) {
                    currentState = STATE_PEERS_CONNECTION;
                    laserDetector.ledOff();
                }
            }
            break;
        case STATE_LASER_2_ADJUSTMENT:
            sevenSegments.showMessage("LAS2");
            if (messageAvailable) {                
                if (communicator.isLaser2AdjustmentResponse(receivedMessage)) {
                    Serial.println("Laser 2 adjusted.");
                    currentState = STATE_READY;
                    communicator.sendStateReadyRequest();
                    measuredTimeMillis = 0;
                    laserDetector.ledOn();
                    changedStateMillis = millis();                    
                }
                if (communicator.isEstablishRequest(receivedMessage)) {
                    currentState = STATE_PEERS_CONNECTION;
                    laserDetector.ledOff();
                }
            }
            break;
        case STATE_READY:
            sevenSegments.showTime(measuredTimeMillis);
            if (laserDetector.isSensorLow()) {
                currentState = STATE_RUN;
                laserDetector.ledOff();
                runStartMillis = millis();
                changedStateMillis = millis();
                communicator.sendStateRunRequest();
            }
            if (messageAvailable) {                
                if (communicator.isEstablishRequest(receivedMessage)) {
                    currentState = STATE_PEERS_CONNECTION;
                    laserDetector.ledOff();
                }
            }
            break;
        case STATE_RUN:
            measuredTimeMillis = millis() - runStartMillis;
            sevenSegments.showTime(measuredTimeMillis);
            if (messageAvailable) {                
                if (communicator.isLaser2Interrupted(receivedMessage)) {
                    currentState = STATE_FINISH;
                    changedStateMillis = millis();
                    communicator.sendStateFinishRequest(measuredTimeMillis);
                } else if (communicator.isEstablishRequest(receivedMessage)) {
                    currentState = STATE_PEERS_CONNECTION;
                    laserDetector.ledOff();
                }
            }
            if (button.isPressed()) {
                currentState = STATE_READY;
                measuredTimeMillis = 0;
                changedStateMillis = millis();
                laserDetector.ledOn();
                communicator.sendStateReadyRequest();
            }
            break;
        case STATE_FINISH:
            sevenSegments.showTime(measuredTimeMillis);
            if (millis() - changedStateMillis > 10000 || button.isPressed()) {
                currentState = STATE_READY;
                laserDetector.ledOn();
                measuredTimeMillis = 0;
                changedStateMillis = millis();
                communicator.sendStateReadyRequest();
            }
            if (messageAvailable) {                
                if (communicator.isEstablishRequest(receivedMessage)) {
                    currentState = STATE_PEERS_CONNECTION;
                    laserDetector.ledOff();
                }
            }
            break;
        default:
            break;
    }
}

void loopFinishDevice() {
    messageAvailable = false;
    if (communicator.isMessageAvailable()) {
        messageAvailable = true;
        communicator.read(receivedMessage, sizeof(receivedMessage));
    }

    switch (currentState) {
        case STATE_START:
            sevenSegments.showMessage("STAR");
            btSerial.println("Start");
            currentState = STATE_PEERS_CONNECTION;
            btSerial.println("Peers connecting...");
            break;
        case STATE_PEERS_CONNECTION:
            sevenSegments.showMessage("CONN");
            if (communicator.isCommunicationEstablished()) {
                currentState = STATE_WAIT_FOR_MESSAGE;
                changedStateMillis = millis();
                btSerial.println("Peers connected.");
            }
            break;
        case STATE_WAIT_FOR_MESSAGE:
            sevenSegments.showMessage("WAIT");
            if (messageAvailable) {                
                btSerial.println(receivedMessage);
                if (communicator.isLaser1AdjustmentInfo(receivedMessage)) {
                    currentState = STATE_LASER_1_ADJUSTMENT;
                } else if (communicator.isLaser2AdjustmentRequest(receivedMessage)) {
                    laserDetector.startLaserAdjustment();
                    currentState = STATE_LASER_2_ADJUSTMENT;
                    changedStateMillis = millis();
                } else if (communicator.isStateReadyRequest(receivedMessage)) {
                    measuredTimeMillis = 0;
                    currentState = STATE_READY;
                    changedStateMillis = millis();
                    laserDetector.ledOff();
                } else if (communicator.isStateFinishRequest(receivedMessage)) {
                    measuredTimeMillis = communicator.getFinishTime(receivedMessage);
                    btSerial.print(measuredTimeMillis/1000.0);
                    btSerial.println("s");
                    Serial.println(1.0 * measuredTimeMillis / 1000.0);
                    currentState = STATE_FINISH;
                    changedStateMillis = millis();
                } else if (communicator.isEstablishRequest(receivedMessage)) {
                    laserDetector.ledOff();
                    currentState = STATE_PEERS_CONNECTION;
                    changedStateMillis = millis();
                } else if (communicator.isStateRunRequest(receivedMessage)) {   // shouldn't be, could happen though
                    currentState = STATE_RUN;
                    laserDetector.ledOn();
                    changedStateMillis = millis();
                    runStartMillis = millis();
                } 
            }
            break;
        case STATE_LASER_1_ADJUSTMENT:
            sevenSegments.showMessage("LAS1");
            if (messageAvailable) {                
                if (communicator.isLaser1AdjustedInfo(receivedMessage)) {
                    btSerial.println("Laser 1 adjusted.");
                    currentState = STATE_WAIT_FOR_MESSAGE;
                    changedStateMillis = millis();
                } else if (communicator.isEstablishRequest(receivedMessage)) {
                    laserDetector.ledOff();
                    currentState = STATE_PEERS_CONNECTION;
                    changedStateMillis = millis();
                }
            }
            break;
        case STATE_LASER_2_ADJUSTMENT:
            sevenSegments.showMessage("LAS2");
            if (laserDetector.isLaserAdjusted()) {
                btSerial.println("Laser 2 adjusted.");                
                currentState = STATE_WAIT_FOR_MESSAGE;
                changedStateMillis = millis();
                communicator.sendLaser2AdjustmentResponse();
            }
            if (messageAvailable) {                
                if (communicator.isEstablishRequest(receivedMessage)) {
                    laserDetector.ledOff();
                    currentState = STATE_PEERS_CONNECTION;
                    changedStateMillis = millis();
                }
            }
            break;
        case STATE_READY:
            sevenSegments.showTime(measuredTimeMillis);
            if (messageAvailable) {                
                if (communicator.isStateRunRequest(receivedMessage)) {
                    currentState = STATE_RUN;
                    laserDetector.ledOn();
                    changedStateMillis = millis();
                    runStartMillis = millis();
                } else if (communicator.isEstablishRequest(receivedMessage)) {
                    laserDetector.ledOff();
                    currentState = STATE_PEERS_CONNECTION;
                    changedStateMillis = millis();
                }
            }
            break;
        case STATE_RUN:
            measuredTimeMillis = millis() - runStartMillis;
            sevenSegments.showTime(measuredTimeMillis);
            if (laserDetector.isSensorLow()) {
                laserDetector.ledOff();
                communicator.sendLaser2Interrupted();
                currentState = STATE_WAIT_FOR_MESSAGE;
                changedStateMillis = millis();
            }
            if (messageAvailable) {                
                if (communicator.isEstablishRequest(receivedMessage)) {
                    laserDetector.ledOff();
                    btSerial.println("Peers connecting...");
                    currentState = STATE_PEERS_CONNECTION;

                    changedStateMillis = millis();
                } else if (communicator.isStateReadyRequest(receivedMessage)) {
                    laserDetector.ledOff();
                    measuredTimeMillis = 0;
                    currentState = STATE_READY;
                }
            }
            break;
        case STATE_FINISH:
            sevenSegments.showTime(measuredTimeMillis);
            if (messageAvailable) {                
                if (communicator.isStateReadyRequest(receivedMessage)) {
                    measuredTimeMillis = 0;
                    currentState = STATE_READY;
                    changedStateMillis = millis();
                    laserDetector.ledOff();
                } else if (communicator.isStateFinishEndRequest(receivedMessage)) {
                    currentState = STATE_WAIT_FOR_MESSAGE;
                    changedStateMillis = millis();
                } else if (communicator.isEstablishRequest(receivedMessage)) {
                    laserDetector.ledOff();
                    btSerial.println("Peers connecting...");
                    currentState = STATE_PEERS_CONNECTION;
                    changedStateMillis = millis();
                } else if (communicator.isStateRunRequest(receivedMessage)) {   // shouldn't be, could happen though
                    currentState = STATE_RUN;
                    laserDetector.ledOn();
                    changedStateMillis = millis();
                    runStartMillis = millis();
                } 
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

boolean isCommuncationSolved() {
    if (waitingForAck) {
        if (communicator.isMessageAvailable()) {
            communicator.read(receivedPayload);
                        
        } else {

        }
        


    }
}

void loop() {
    if (isStartDevice()) {
        loopStartDevice();
    } else {
        loopFinishDevice();
    }
}