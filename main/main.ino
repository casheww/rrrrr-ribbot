#include <analogWrite.h>

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

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

const int IRarrayLen = 5;
long IRreadings[IRarrayLen];
int IRindex = 0;



void setup() {

  Serial.begin(115200);   // setup debugging connection back to a computer
  Serial.println("ribbot online weewooweewoo");

  setupSpacial();         // setup spacial sensors and pins and whatnot, y'know
  setupMotors();          // setup motor pin modes

  delay(500);

}


// used to track which obstacle we're on
int currentSection = 0;

void loop() {

  // TODO ? allignWithBridge();

  if (currentSection == 0){
    crossBridge();
    setLED(0, false);
    setLED(1, false);
  }
  else if (currentSection == 1) {
    passSpinnyWheelWall();
    setLED(0, false);
    setLED(1, false);
  }
  else if (currentSection == 2) {
    followIRBeacon();
  }
  else if (currentSection >= 3) {
    doPrettyLEDs();
  }

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
  Serial.println("crossing bridge");
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
      else {
        currentSection = 1;
      }
    }
    else {
      //setLED(0, true);
    }
  }
}


// this doesn't actually do anything interesting anymore
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
  Serial.println("doing wheel check");
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
  Serial.println("passing the spinny wheel wall");
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
    
    if (closeIndex != -1 && farIndex != -1) {
      setLED(0, true);
      // we think we have seen the wheel on both the close and far sides of its rotation
      if (closeIndex > wheelDistArrayLen * 0.7) {
        // we think the closer part of the rotation is near to the end of the array
        success = true;
        currentSection = 2;
      }
      else {
        stopDriveMotors();
        stopped = true;
        int d;
        do {
          d = getTof();
          delay(100);
        } while (d > 80);
      }
    }
    else {
      setDriveMotors(180,180);
      delay(100);
      wheelCheck();
    }
  }
}

// END SPINNY WHEEL



// START INFRARED

void followIRBeacon() {
  Serial.println("following IR beacon");
  bool driveToCorridorSuccess = false;
  bool finalSuccess = false;
  bool overallSuccess = false;

  // blindly drive forward and hope that everything is okay
  setDriveMotors(180, 180);
  delay(1000);
  stopDriveMotors();
  delay(100);
  setDriveMotors(180,-180);
  delay(500);
  stopDriveMotors();
  delay(100);
  setDriveMotors(180, 180);

  while (!finalSuccess) {

    long ir = getIR();
    IRreadings[IRindex] = ir;
    IRindex++;
    if (IRindex >= IRarrayLen) { IRindex = 0; }

    char pos = checkIRCorridorPos();

    if (pos == 'l') {
      setLED(0, true);
      setLED(1, false);
      pos = 'l';
      Serial.println(pos);
      setDriveMotors(190, 0);
    }
    else if (pos == 'r') {
      setLED(0, false);
      setLED(1, true);
      pos = 'r';
      Serial.println(pos);
      setDriveMotors(0, 190);
    }
    else if (pos = 'c') {
      setLED(0, true);
      setLED(1, true);
      pos = 'c';
      Serial.println(pos);
      setDriveMotors(190, 190);
    }

    delay(400);
    
  }

  currentSection = 10;
}

/* 
 *  Uses an existing array of IR readings to determine where in the IR corridor we are.
 *  The return can be:
 *    'l' when too far left
 *    'r' when too far right
 *    'c' when in the centre of the corridor.
 *  Due to the use of an array that's built up and updated over time, there is a little delay (see followIRBeacon)
 */
char checkIRCorridorPos() {

  /*
   * IR transmitter codes:
   * too far left:  FF04FB
   * centre:        FF01FE
   * too far right: FF02FD
   */

  int leftCount = 0;
  int centreCount = 0;
  int rightCount = 0;
  
  for (int i = 0; i < IRarrayLen; i++) {
    serialPrintUint64(IRreadings[i], HEX);
    Serial.print(" , ");
    if (IRreadings[i] == 0xFF04FB) { leftCount++; }
    else if (IRreadings[i] == 0xFF01FE) { centreCount++; }
    else if (IRreadings[i] == 0xFF02FD) { rightCount++; }
  }
  Serial.println("");

  int _max = max(max(leftCount, centreCount), rightCount);

  if (_max == leftCount) {
    return 'l';
  }
  else if (_max == rightCount) {
    return 'r';
  }
  if (_max == centreCount) {
    return 'c';
  }
  
}


// END INFRARED
