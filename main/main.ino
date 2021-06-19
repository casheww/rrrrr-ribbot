#include <analogWrite.h>
#include "debug.h"
#include "spacial.h"
#include "motors.h"

#define LED_BUILTIN 1


/*
 SPECIAL NOTES:
 - 180 seems to be the motor power required to kick it off moving.
 - somewhere between 120 and 160 is a cutoff point where the power is not enough to move the weight of the robot
 - each 
*/



void setup() {
  Serial.begin(115200);   // setup debugging connection back to a computer
  Serial.println("ribbot online weewooweewoo");
  setupSpacial();         // setup spacial sensors and pins and whatnot, y'know
  setupMotors();          // setup motor pin modes

  delay(500);
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

  // TODO ? allignWithBridge();
  //crossBridge();
  //passSpinningWheelWall();

  
}

// to cross the rainbow bridge without falling off the edge
void crossBridge() {
  bool success = false;
  bool backedThisLoop = true;

  while (!success) {
    // left tracker
    if (!getTracker(0)) {
      backedThisLoop = true;
      // turn away from edge slightly
      setDriveMotors(200, 160);
      delay(500);
    }
    // right tracker
    else if (!getTracker(1)) {
      backedThisLoop = true;
      // turn away from edge slightly
      setDriveMotors(160, 200);
      delay(500);
    }

    // set motors forward again if safe to move forward
    if (backedThisLoop && getTracker(0) && getTracker(1)) {
      setDriveMotors(180, 180);
      backedThisLoop = false;
    }

    // hopefully we can see the laminated sheet from the next stage
    // if not, it may be about to send us into a wall
    success = getTof() < 100;
  }
}

// to pass the spinning bike wheel with a reinforced laminated sheet as a barrier
void passSpinningWheelWall() {
  bool success = false;
  bool stopped = false;

  while (!success) {
    int dist = getTof();

    if (dist < 30) {          // <3cm - way too close! reverse
      setLED(0, false);
      setLED(1, true);
      setDriveMotors(-180, -180);
      delay(200);
      setDriveMotors(-140, -140);
      delay(400);
      stopDriveMotors();
      delay(100);
    }
    
    else if (dist < 60) {     // <6cm - wait
      stopDriveMotors();
      stopped = true;
      setLED(0, true);
      setLED(1, false);
    }
    
    else if (stopped) {           // go!
      setDriveMotors(180, 180);   // higher power to start motors
      delay(200);
      setDriveMotors(160, 160);
      stopped = false;
      setLED(0, true);
      setLED(1, true);
    }
  }
}
