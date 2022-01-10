#include <Arduino.h>
#include <SoftwareSerial.h>
#define DEBUGLOG_DISABLE_LOG  // uncomment for disabling the log
#include <DebugLog.h>

#include "Button.h"
#include "Communicator.h"
#include "LaserDetector.h"
#include "SevenSegments.h"

#define DEVICE_TYPE 0  // defines whether is it start (0) or finish (1) device

#define STATE_START 0
#define STATE_LASER_1_ADJUSTMENT 1
#define STATE_LASER_2_ADJUSTMENT 2
#define STATE_CONNECTION 3
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
#define ACK_REPETITION_MICROS 30000
#define ACK_TIMEOUT_MICROS 500000

/* MESSAGES */
#define MSG_ACK "ACK"
#define MSG_CONNECTION_REQUEST "CONN_RQ"
#define MSG_CONNECTION_RESPONSE "CONN_RESP"
#define MSG_LASER_1_ADJUSTMENT "LASER1"
#define MSG_LASER_2_ADJUSTMENT_REQUEST "LASER2_RQ"
#define MSG_LASER_2_ADJUSTMENT_RESPONSE "LASER2_RESP"
#define MSG_STATE_READY_REQUEST "READY_RQ"
#define MSG_STATE_RUN_REQUEST "RUN_RQ"
#define MSG_STATE_FINISH_REQUEST "FIN_RQ"
#define MSG_LASER_2_INTERRUPTED "LASER2_INTR"

Message *sentMessage;
Message *receivedMessage = (Message *)malloc(sizeof(Message));
Message *ackMessage = (Message *)malloc(sizeof(Message));
Message *connectionRequestMessage = (Message *)malloc(sizeof(Message));
Message *connectionResponseMessage = (Message *)malloc(sizeof(Message));
Message *laser1AdjustmentMessage = (Message *)malloc(sizeof(Message));
Message *laser2AdjustmentRequestMessage = (Message *)malloc(sizeof(Message));
Message *laser2AdjustmentResponseMessage = (Message *)malloc(sizeof(Message));
Message *stateReadyRequestMessage = (Message *)malloc(sizeof(Message));
Message *stateRunRequestMessage = (Message *)malloc(sizeof(Message));
Message *stateFinishRequestMessage = (Message *)malloc(sizeof(Message));
Message *laser2InterrupedtMessage = (Message *)malloc(sizeof(Message));

/* GLOBAL VARIABLES */
unsigned int currentState = STATE_START;
unsigned long runStartMillis;
unsigned long measuredTimeMillis = 0;

boolean unsentMessage = false;
unsigned long messageSentMicros;
unsigned long messageResentMicros;

boolean unprocessedMessage = false;
boolean waitingForAck = false;

uint8_t lastReceivedCounter = 0;
uint8_t lastSentCounter = 0;

unsigned long finishStateMillis = 0;

SevenSegments sevenSegments(SEVEN_SEGMENTS_SCLK_PIN, SEVEN_SEGMENTS_RCLK_PIN, SEVEN_SEGMENTS_DIO_PIN);
LaserDetector laserDetector(LASER_DIODE_PIN, LASER_TRANSISTOR_PIN);
Communicator communicator(DEVICE_TYPE == 0);
Button button(BUTTON_PIN);
SoftwareSerial btSerial(10, 2);  // RX, TX

/* FUNCTIONS DECLARATION */
static boolean processCommunication();
static void initMessages();
static boolean isAckMessage(Message *message);
static boolean isConnectionRequestMessage(Message *message);
static boolean isConnectionResponseMessage(Message *message);
static boolean isLaser1AdjustmentMessage(Message *message);
static boolean isLaser2AdjustmentRequestMessage(Message *message);
static boolean isLaser2AdjustmentResponseMessage(Message *message);
static boolean isStateReadyRequestMessage(Message *message);
static boolean isStateRunRequestMessage(Message *message);
static boolean isStateFinishRequestMessage(Message *message);
static boolean isLaser2InterrupedtMessage(Message *message);
static long getFinishTime(Message *message);

void setup() {
    LOG_SET_LEVEL(DebugLogLevel::LVL_TRACE);
    initMessages();
    Serial.begin(115200);
    sevenSegments.init();
    laserDetector.init();
    communicator.init();
    button.init();
    btSerial.begin(9600);
}

void sendMessage(Message *message) {
    message->counter = ++lastSentCounter;
    sentMessage = message;
    messageSentMicros = micros();
    unsentMessage = true;
}

void loopStartDevice() {
    switch (currentState) {
        case STATE_START:
            sendMessage(connectionRequestMessage);
            currentState = STATE_CONNECTION;
            break;
        case STATE_CONNECTION:
            sevenSegments.showMessage("CONN");
            if (unprocessedMessage) {
                if (isConnectionResponseMessage(receivedMessage)) {
                    currentState = STATE_LASER_1_ADJUSTMENT;
                    sendMessage(laser1AdjustmentMessage);
                    laserDetector.startLaserAdjustment();
                }
                unprocessedMessage = false;
            }
            break;
        case STATE_LASER_1_ADJUSTMENT:
            sevenSegments.showMessage("LAS1");
            if (laserDetector.isLaserAdjusted()) {
                currentState = STATE_LASER_2_ADJUSTMENT;
                sendMessage(laser2AdjustmentRequestMessage);
                laserDetector.ledOff();
            }
            break;
        case STATE_LASER_2_ADJUSTMENT:
            sevenSegments.showMessage("LAS2");
            if (unprocessedMessage) {
                if (isLaser2AdjustmentResponseMessage(receivedMessage)) {
                    laserDetector.ledOn();
                    currentState = STATE_READY;
                    sendMessage(stateReadyRequestMessage);
                }
                unprocessedMessage = false;
            }
            break;
        case STATE_READY:
            sevenSegments.showTime(measuredTimeMillis);
            if (laserDetector.isSensorLow()) {
                currentState = STATE_RUN;
                laserDetector.ledOff();
                runStartMillis = millis();
                sendMessage(stateRunRequestMessage);
            }
            break;
        case STATE_RUN:
            measuredTimeMillis = millis() - runStartMillis;
            sevenSegments.showTime(measuredTimeMillis);
            if (unprocessedMessage) {
                if (isLaser2InterrupedtMessage(receivedMessage)) {
                    currentState = STATE_FINISH;
                    sprintf(&stateFinishRequestMessage->text[strlen(MSG_STATE_FINISH_REQUEST)], "%08ld", measuredTimeMillis);
                    sendMessage(stateFinishRequestMessage);
                    finishStateMillis = millis();
                }
                unprocessedMessage = false;
            }
            if (button.isPressed()) {
                currentState = STATE_READY;
                measuredTimeMillis = 0;
                laserDetector.ledOn();
                sendMessage(stateReadyRequestMessage);
            }
            break;
        case STATE_FINISH:
            sevenSegments.showTime(measuredTimeMillis);
            if (millis() - finishStateMillis > 10000 || button.isPressed()) {
                currentState = STATE_READY;
                laserDetector.ledOn();
                measuredTimeMillis = 0;
                sendMessage(stateReadyRequestMessage);
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
            btSerial.println("Start");
            btSerial.println("Peers connecting...");
            // sendMessage(connectionRequestMessage);
            currentState = STATE_CONNECTION;
            break;
        case STATE_CONNECTION:
            sevenSegments.showMessage("CONN");
            if (unprocessedMessage) {
                if (isConnectionRequestMessage(receivedMessage)) {
                    btSerial.println("Peers connected.");
                    currentState = STATE_WAIT_FOR_MESSAGE;
                    sendMessage(connectionResponseMessage);
                }
                unprocessedMessage = false;
            }
            break;
        case STATE_WAIT_FOR_MESSAGE:
            sevenSegments.showMessage("WAIT");
            if (unprocessedMessage) {
                if (isLaser1AdjustmentMessage(receivedMessage)) {
                    currentState = STATE_LASER_1_ADJUSTMENT;
                } else if (isStateReadyRequestMessage(receivedMessage)) {
                    measuredTimeMillis = 0;
                    currentState = STATE_READY;
                } else if (isStateFinishRequestMessage(receivedMessage)) {
                    measuredTimeMillis = getFinishTime(receivedMessage);
                    btSerial.print(measuredTimeMillis / 1000.0);
                    btSerial.println("s");
                    LOG_INFO(millis(), measuredTimeMillis, "ms");
                    currentState = STATE_FINISH;
                }
                unprocessedMessage = false;
            }
            break;
        case STATE_LASER_1_ADJUSTMENT:
            sevenSegments.showMessage("LAS1");
            if (unprocessedMessage) {
                if (isLaser2AdjustmentRequestMessage(receivedMessage)) {
                    LOG_INFO(millis(), "Laser 1 adjusted.");
                    btSerial.println("Laser 1 adjusted.");
                    laserDetector.ledOn();
                    laserDetector.startLaserAdjustment();
                    currentState = STATE_LASER_2_ADJUSTMENT;
                }
                unprocessedMessage = false;
            }
            break;
        case STATE_LASER_2_ADJUSTMENT:
            sevenSegments.showMessage("LAS2");
            if (laserDetector.isLaserAdjusted()) {
                LOG_INFO(millis(), "Laser 2 adjusted.");
                btSerial.println("Laser 2 adjusted.");
                laserDetector.ledOff();
                currentState = STATE_WAIT_FOR_MESSAGE;
                sendMessage(laser2AdjustmentResponseMessage);
            }
            break;
        case STATE_READY:
            sevenSegments.showTime(measuredTimeMillis);
            if (unprocessedMessage) {
                if (isStateRunRequestMessage(receivedMessage)) {
                    LOG_INFO(millis(), "Run...");
                    runStartMillis = millis();
                    laserDetector.ledOn();
                    currentState = STATE_RUN;
                }
                unprocessedMessage = false;
            }
            break;
        case STATE_RUN:
            measuredTimeMillis = millis() - runStartMillis;
            sevenSegments.showTime(measuredTimeMillis);
            if (laserDetector.isSensorLow()) {
                laserDetector.ledOff();
                sendMessage(laser2InterrupedtMessage);
                currentState = STATE_WAIT_FOR_MESSAGE;
                LOG_INFO(millis(), "Finish...");
            }
            if (unprocessedMessage) {
                if (isStateReadyRequestMessage(receivedMessage)) {
                    measuredTimeMillis = 0;
                    currentState = STATE_READY;
                }
                unprocessedMessage = false;
            }
            break;
        case STATE_FINISH:
            sevenSegments.showTime(measuredTimeMillis);
            if (unprocessedMessage) {
                if (isStateReadyRequestMessage(receivedMessage)) {
                    measuredTimeMillis = 0;
                    currentState = STATE_READY;
                }
                unprocessedMessage = false;
            }
            break;
        default:
            break;
    }
}

static boolean isStartDevice() {
    return DEVICE_TYPE == 0;
}

void loop() {
    if (!processCommunication()) {
        if (currentState == STATE_CONNECTION) {
            sevenSegments.showMessage("CONN");
        }
        return;
    }
    if (isStartDevice()) {
        loopStartDevice();
    } else {
        loopFinishDevice();
    }
}

boolean processCommunication() {
    if (waitingForAck) {
        if (communicator.isMessageAvailable()) {
            communicator.read(receivedMessage);
            LOG_TRACE(millis(), "received message while waiting for ack", receivedMessage->counter, receivedMessage->text);
            if (isAckMessage(receivedMessage)) {
                LOG_DEBUG(millis(), "ack received");
                waitingForAck = false;
            } else {
                LOG_WARN(millis(), "waiting for ack, different message received, just ack", receivedMessage->counter, receivedMessage->text);
                communicator.send(ackMessage);
                // waitingForAck = false;
            }
            return false;
        } else {
            if (messageResentMicros < messageSentMicros) {
                messageResentMicros = messageSentMicros;
            }
            if ((micros() - messageSentMicros) > ACK_TIMEOUT_MICROS) {
                LOG_INFO("Ack not received whatsoever, change state to START");
                currentState = STATE_START;
                waitingForAck = false;
                return true;
            }
            if ((micros() - messageResentMicros) > ACK_REPETITION_MICROS && !isConnectionRequestMessage(sentMessage)) {
                LOG_DEBUG(millis(), "resending a message, ack not received yet for", sentMessage->counter, sentMessage->text);
                communicator.send(sentMessage);
                messageResentMicros = micros();
            }
        }
        return false;
    }
    if (unsentMessage) {
        LOG_DEBUG(millis(), "sending message", sentMessage->counter, sentMessage->text);
        communicator.send(sentMessage);
        waitingForAck = true;
        unsentMessage = false;
        messageSentMicros = micros();
        return false;
    }
    if (communicator.isMessageAvailable()) {
        communicator.read(receivedMessage);
        if (isAckMessage(receivedMessage)) {
            LOG_TRACE(millis(), "ignore ack received");
            return false;
        }
        LOG_DEBUG(millis(), "received message", receivedMessage->text, receivedMessage->counter);
        // anytime we got connection request, let's start from beginning
        if (isConnectionRequestMessage(receivedMessage)) {
            ackMessage->counter = lastSentCounter;
            communicator.send(ackMessage);
            LOG_TRACE(millis(), "sending ack");
            if (isStartDevice()) {
                currentState = STATE_START;
            } else {
                currentState = STATE_CONNECTION;
            }
            lastReceivedCounter = receivedMessage->counter;
            unprocessedMessage = true;
            return true;
        }
        if ((receivedMessage->counter > lastReceivedCounter) || (receivedMessage->counter < 5 && lastReceivedCounter > 250)) {
            ackMessage->counter = lastSentCounter;
            communicator.send(ackMessage);
            LOG_TRACE(millis(), "sending ack");
            lastReceivedCounter = receivedMessage->counter;
            unprocessedMessage = true;
        } else {
            LOG_DEBUG(millis(), "received message repeateadly, just ack it.");
            ackMessage->counter = lastSentCounter;
            communicator.send(ackMessage);
            unprocessedMessage = false;
        }
        return true;
    }
    return true;
}

static void initMessages() {
    memcpy(ackMessage->text, MSG_ACK, strlen(MSG_ACK));
    ackMessage->text[strlen(MSG_ACK)] = '\0';
    memcpy(connectionRequestMessage->text, MSG_CONNECTION_REQUEST, strlen(MSG_CONNECTION_REQUEST));
    connectionRequestMessage->text[strlen(MSG_CONNECTION_REQUEST)] = '\0';
    memcpy(connectionResponseMessage->text, MSG_CONNECTION_RESPONSE, strlen(MSG_CONNECTION_RESPONSE));
    connectionResponseMessage->text[strlen(MSG_CONNECTION_RESPONSE)] = '\0';
    memcpy(laser1AdjustmentMessage->text, MSG_LASER_1_ADJUSTMENT, strlen(MSG_LASER_1_ADJUSTMENT));
    laser1AdjustmentMessage->text[strlen(MSG_LASER_1_ADJUSTMENT)] = '\0';
    memcpy(laser2AdjustmentRequestMessage->text, MSG_LASER_2_ADJUSTMENT_REQUEST, strlen(MSG_LASER_2_ADJUSTMENT_REQUEST));
    laser2AdjustmentRequestMessage->text[strlen(MSG_LASER_2_ADJUSTMENT_REQUEST)] = '\0';
    memcpy(laser2AdjustmentResponseMessage->text, MSG_LASER_2_ADJUSTMENT_RESPONSE, strlen(MSG_LASER_2_ADJUSTMENT_RESPONSE));
    laser2AdjustmentResponseMessage->text[strlen(MSG_LASER_2_ADJUSTMENT_RESPONSE)] = '\0';
    memcpy(stateReadyRequestMessage->text, MSG_STATE_READY_REQUEST, strlen(MSG_STATE_READY_REQUEST));
    stateReadyRequestMessage->text[strlen(MSG_STATE_READY_REQUEST)] = '\0';
    memcpy(stateRunRequestMessage->text, MSG_STATE_RUN_REQUEST, strlen(MSG_STATE_RUN_REQUEST));
    stateRunRequestMessage->text[strlen(MSG_STATE_RUN_REQUEST)] = '\0';
    memcpy(stateFinishRequestMessage->text, MSG_STATE_FINISH_REQUEST, strlen(MSG_STATE_FINISH_REQUEST));
    stateFinishRequestMessage->text[strlen(MSG_STATE_FINISH_REQUEST)] = '\0';
    memcpy(laser2InterrupedtMessage->text, MSG_LASER_2_INTERRUPTED, strlen(MSG_LASER_2_INTERRUPTED));
    laser2InterrupedtMessage->text[strlen(MSG_LASER_2_INTERRUPTED)] = '\0';
}

static boolean isAckMessage(Message *message) {
    return strncmp(message->text, MSG_ACK, strlen(MSG_ACK)) == 0;
}

static boolean isConnectionRequestMessage(Message *message) {
    return strncmp(message->text, MSG_CONNECTION_REQUEST, strlen(MSG_CONNECTION_REQUEST)) == 0;
}

static boolean isConnectionResponseMessage(Message *message) {
    return strncmp(message->text, MSG_CONNECTION_RESPONSE, strlen(MSG_CONNECTION_RESPONSE)) == 0;
}

static boolean isLaser1AdjustmentMessage(Message *message) {
    return strncmp(message->text, MSG_LASER_1_ADJUSTMENT, strlen(MSG_LASER_1_ADJUSTMENT)) == 0;
}

static boolean isLaser2AdjustmentRequestMessage(Message *message) {
    return strncmp(message->text, MSG_LASER_2_ADJUSTMENT_REQUEST, strlen(MSG_LASER_2_ADJUSTMENT_REQUEST)) == 0;
}

static boolean isLaser2AdjustmentResponseMessage(Message *message) {
    return strncmp(message->text, MSG_LASER_2_ADJUSTMENT_RESPONSE, strlen(MSG_LASER_2_ADJUSTMENT_RESPONSE)) == 0;
}

static boolean isStateReadyRequestMessage(Message *message) {
    return strncmp(message->text, MSG_STATE_READY_REQUEST, strlen(MSG_STATE_READY_REQUEST)) == 0;
}

static boolean isStateRunRequestMessage(Message *message) {
    return strncmp(message->text, MSG_STATE_RUN_REQUEST, strlen(MSG_STATE_RUN_REQUEST)) == 0;
}

static boolean isStateFinishRequestMessage(Message *message) {
    return strncmp(message->text, MSG_STATE_FINISH_REQUEST, strlen(MSG_STATE_FINISH_REQUEST)) == 0;
}

static boolean isLaser2InterrupedtMessage(Message *message) {
    return strncmp(message->text, MSG_LASER_2_INTERRUPTED, strlen(MSG_LASER_2_INTERRUPTED)) == 0;
}

static long getFinishTime(Message *message) {
    char sub[9];
    memcpy(sub, &message->text[strlen(MSG_STATE_FINISH_REQUEST)], 8);
    return atol(sub);
}
