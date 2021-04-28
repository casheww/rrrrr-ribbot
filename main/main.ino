#include <analogWrite.h>

#include "debug.h"
#include "spacial.h"
#include "motors.h"

#define LED_BUILTIN 1


/*
 * SPECIAL NOTES:
 * - 180 seems to be the motor power required to kick it off moving.
 * - somewhere between 120 and 160 is a cutoff point where the power is not enough to move the weight of the robot
 */



void setup() {
  Serial.begin(115200);   // setup debugging connection back to a computer
  setupSpacial();         // setup spacial sensors and pins and whatnot, y'know
  setupMotors();          // setup motor pin modes
}


void loop() {
  crossBridge();
}


void crossBridge() {
  bool success = false;
  bool backedThisLoop = true;
  
  while (!success) {
    // check left tracker sensor
    if (!getTracker(0)) {
      backedThisLoop = true;
      stopDriveMotors();
      delay(100);
      setDriveMotors(-180, -180);
      delay(200);
      setDriveMotors(-140, 0);
      delay(1000);
      stopDriveMotors();
    }
    // check right tracker sensor
    else if (!getTracker(1)) {
      backedThisLoop = true;
      stopDriveMotors();
      delay(100);
      setDriveMotors(-180, -180);
      delay(200);
      setDriveMotors(0, -140);
      delay(1000);
      stopDriveMotors();
    }

    // set motors forward again if safe to move forward
    if (backedThisLoop && getTracker(0) && getTracker(1)) {
      setDriveMotors(180, 180);
      delay(300);
      setDriveMotors(160, 160);
      backedThisLoop = false;
    }
  }
}
