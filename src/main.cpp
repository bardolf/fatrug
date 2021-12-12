#include <Arduino.h>

#include "SevenSegments.h"
#include "LaserDetector.h"
#include "Communicator.h"

#define STATE_START 0
#define STATE_LASER_ADJUSTMENT 1
#define STATE_PEERS_CONNECTED 2
#define STATE_READY 3
#define STATE_RUN 4
#define STATE_FINISH 5

#define PING_INTERVAL_MS 500

/* HW CONFIGURATION */
#define SEVEN_SEGMENTS_SCLK_PIN 4
#define SEVEN_SEGMENTS_RCLK_PIN 6
#define SEVEN_SEGMENTS_DIO_PIN 5

#define LASER_DIODE_PIN 3
#define LASER_TRANSISTOR_PIN A5

#define DEVICE_TYPE 0    //defines whether is it start (0) or finish (1) device

unsigned int currentState = STATE_START;

SevenSegments sevenSegments(SEVEN_SEGMENTS_SCLK_PIN, SEVEN_SEGMENTS_RCLK_PIN, SEVEN_SEGMENTS_DIO_PIN);
LaserDetector laserDetector(LASER_DIODE_PIN, LASER_TRANSISTOR_PIN);
Communicator communicator;

void setup() {    
    sevenSegments.init();
    laserDetector.init();
    Serial.begin(9600);
}

void loop() {
    // sevenSegments.showTime(millis());
    // if (laserDetector.isObjectDetected()) {
    //   sevenSegments.showMessage("EMPT");
    // } else {
    //   sevenSegments.showMessage("OBJ");
    // }
    
     Serial.println("111");
    // communicator.send("X");
    if (communicator.isMessageAvailable()) {
      Serial.println("xxx");
      char text[32] = "";
      communicator.read(&text, strlen(text));
      Serial.println(text);
    }




    delay(1000);
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