#include <analogWrite.h>

#include "debug.h"
#include "spacial.h"
#include "motors.h"

#define LED_BUILTIN 1


/*
  SPECIAL NOTES:
  - 180 seems to be the motor power required to kick it off moving.
  - somewhere between 120 and 160 is a cutoff point where the power is not enough to move the weight of the robot
   - this is upped to 180 for climbing the second half of the rainbow bridge
*/


// wheel takes ~ 6 seconds to do a full rotation, so we can take 15 distance readings ~0.5 seconds apart to get a picture of a full rotation (and a bit)
const int wheelDistArrayLen = 15;

int distances[wheelDistArrayLen];
int distIndex = 0;


void setup() {

  Serial.begin(115200);   // setup debugging connection back to a computer
  Serial.println("ribbot online weewooweewoo");

  setupSpacial();         // setup spacial sensors and pins and whatnot, y'know
  setupMotors();          // setup motor pin modes

  delay(500);

}

void loop() {

  // TODO ? allignWithBridge();
  crossBridge();
  setLED(0, false);
  setLED(1, false);

  passSpinnyWheelWall();
  // TODO ? spot IR beacon to turn right, or similar

}


// START BRIDGE

const float upModMin = 1.1;
const float upModMax = 1.4;
const float downModMax = 0.9;
const float downModMin = 0.6;

float upMod;
float downMod;

// to cross the rainbow bridge without falling off the edge
void crossBridge() {
  bool success = false;
  bool warningLedOn = false;

  int forwardPower = 200;

  int t = 0;

  // TODO : make power mods more extreme with time spent with tracker sensors over the edge of the bridge

  while (!success) {

    if (!getTracker(0) && !getTracker(1)) {
      stopDriveMotors();
      warningLedOn = !warningLedOn;
      setLED(1, warningLedOn);
    }

    // left tracker
    else if (!getTracker(0)) {
      // turn away from edge slightly
      t++;
      setSpeedMod(t);
      setDriveMotors(forwardPower * upMod, forwardPower * downMod);
      //delay(200);
    }

    // right tracker
    else if (!getTracker(1)) {
      // turn away from edge slightly
      t++;
      setSpeedMod(t);
      setDriveMotors(forwardPower * downMod, forwardPower * upMod);
      //delay(200);
    }

    // set motors forward again if safe to move forward
    else if (getTracker(0) && getTracker(1)) {
      setDriveMotors(forwardPower, forwardPower);
      t = 0;
    }

    int d = getTof();
    if (d < 300) {
      success = wheelCheck();
      if (!success) {
        setDriveMotors(forwardPower, forwardPower);
      }
    }
    else {
      //setLED(0, true);
    }
  }
}


int setSpeedMod(int timerCount) {
  float t = timerCount / 10;
  // lerp
  upMod = 1.1;
  downMod = 0;
  //upMod = upModMin + (upModMax - upModMin) * t;
  //downMod = downModMax - (downModMax - downModMin) * t;
}


/*
   Attempt to tell the difference between wheel barrier and wall.
   Should return true if the obstacle moves and is probably the wheel.
   Shold return false if the obstacle doesn't move and is probably a wall.
*/
bool wheelCheck() {
  setLED(1, true);

  stopDriveMotors();
  int sum = 0;

  for (distIndex = 0; distIndex < wheelDistArrayLen; distIndex++) {
    int d = getTof();
    distances[distIndex] = d;
    sum += d;
    delay(250);
  }

  float avg = sum / wheelDistArrayLen;
  Serial.print("average ");
  Serial.println(avg);

  int wallCount = 0;
  int wheelCount = 0;

  // walls don't move - the distance won't vary too much
  // we're going to guess that they won't be more than 10mm from the average

  for (int i = 0 + 1; i < wheelDistArrayLen; i++) {

    if (abs(distances[i] - avg) > 10) {
      wheelCount++;
    }
    else {
      wallCount++;
    }

  setLED(1, false);
  }

  Serial.print("wheel ");
  Serial.println(wheelCount);
  Serial.print("wall ");
  Serial.println(wallCount);

  return wheelCount >= wallCount * 0.8;   // if more distances vary from the average than not (or rather, not times a little bit), then it's probably the wheel
}

// END BRIDGE



// START SPINNY WHEEL

void passSpinnyWheelWall() {
  Serial.println("passSpinnyWheelWall() running");
  bool success = false;
  bool stopped = true;

  int distChecks = 0;
  distIndex = 0;

  int closeIndex = -1;
  int farIndex = -1;

  while (!success) {

    for (int i = 0; i < wheelDistArrayLen; i++) {
      setLED(1, true);
      int d = distances[i];
      Serial.print(d);
      Serial.print(" , ");

      if (d < 300) {
        closeIndex = i;
      }
      else if (400 < d && d < 600) {
        farIndex = i;
      }
      setLED(1, false);
    }
    Serial.println();
    Serial.print("near ");
    Serial.println(closeIndex);
    Serial.println(distances[closeIndex]);
    Serial.print("far ");
    Serial.println(farIndex);
    Serial.println(distances[farIndex]);
    
    setLED(1, false);

    if (closeIndex != -1 && farIndex != -1) {
      setLED(0, true);
      // we think we have seen the wheel on both the close and far sides of its rotation
      if (closeIndex > wheelDistArrayLen * 0.7) {
        // we think the closer part of the rotation is near to the end of the array
        Serial.println("aaa");
      }
      else {
        stopDriveMotors();
        stopped = true;
        while (getTof() > 80) {
          delay(200);
        }
      }
    }
  }
}


// disgusting cumulative distances to the closest part of circumference and further part of circumference
// distance to end of 'library' not included because it exceeds the range of the time of flight dist sensor
// TODO : these are currently unused, I think. Use it or lose it
const int wayTooClose = 75;
const int wheelDistAMin = 100;
const int wheelDistAMax = 300;
const int wheelDistBMax = 300 + 400;

// END SPINNY WHEEL
