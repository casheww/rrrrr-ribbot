#include <analogWrite.h>
#include "debug.h"
#include "spacial.h"
#include "motors.h"

#define LED_BUILTIN 1


/*
 SPECIAL NOTES:
 - 180 seems to be the motor power required to kick it off moving.
 - somewhere between 120 and 160 is a cutoff point where the power is not enough to move the weight of the robot
*/



void setup() {
  Serial.begin(115200);   // setup debugging connection back to a computer
  Serial.println("ribbot online weewooweewoo");
  setupSpacial();         // setup spacial sensors and pins and whatnot, y'know
  setupMotors();          // setup motor pin modes

  delay(250);
}


void loop() {
  /*
  Serial.println("a");
  short* gyroP = getGyro();
  Serial.println("b : gyro get done");
  for (int i = 0; i < 3; i++) {
    Serial.println(*(gyroP + i));
  }
  Serial.println("c : passed values printed?");*/
  
  crossBridge();
}


void crossBridge() {
  bool success = false;
  bool backedThisLoop = true;

  while (!success) {
    // left tracker
    if (!getTracker(0)) {
      backedThisLoop = true;
      // turn away from edge slightly
      setDriveMotors(210, 160);
      delay(500);
    }
    // right tracker
    else if (!getTracker(1)) {
      backedThisLoop = true;
      // turn away from edge slightly
      setDriveMotors(160, 210);
      delay(500);
    }

    // set motors forward again if safe to move forward
    if (backedThisLoop && getTracker(0) && getTracker(1)) {
      setDriveMotors(190, 190);
      backedThisLoop = false;
    }
  }
}
