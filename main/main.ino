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
  else {
    doPrettyLEDs();
  }

}


// START BRIDGE

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
      setLED(1, false);
      // turn away from edge slightly
      t++;
      setDriveMotors(forwardPower * 0.95, forwardPower * 0);
      //delay(200);
    }

    // right tracker
    else if (!getTracker(1)) {
      setLED(1, false);
      // turn away from edge slightly
      t++;
      setDriveMotors(forwardPower * 0, forwardPower * 0.95);
      //delay(200);
    }

    // set motors forward again if safe to move forward
    else if (getTracker(0) && getTracker(1)) {
      setLED(1, false);
      setDriveMotors(forwardPower, forwardPower);
      t = 0;
    }

    int d = getTof();
    if (d < 300) {
      success = wheelCheck(true);
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


/*
   Attempt to tell the difference between wheel barrier and wall.
   Should return true if the obstacle moves and is probably the wheel.
   Shold return false if the obstacle doesn't move and is probably a wall.
*/
bool wheelCheck(bool getFreshDistances) {
  Serial.println("doing wheel check");
  Serial.print("getting fresh distances? ");
  Serial.println(getFreshDistances);
  setLED(1, true);

  if (getFreshDistances) {
    stopDriveMotors();
  }
  
  int sum = 0;

  for (distIndex = 0; distIndex < wheelDistArrayLen; distIndex++) {
    int d;

    if (getFreshDistances) {
      d = getTof();
      distances[distIndex] = d;
      delay(250);
    }
    else {
      d = distances[distIndex];
    }

    Serial.print(d);
    Serial.print(" , ");
    
    sum += d;
  }
  Serial.println("");

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
  Serial.println("determined that target is the wheel (not wall) - passing the spinny wheel wall");
  bool success = false;
  bool stopped = true;

  int distChecks = 0;
  distIndex = 0;

  int closeIndex = -1;
  int farIndex = -1;

  while (!success) {

    // replace the oldest of the TOF distance readings with a new one
    distances[distIndex] = getTof();
    distIndex++;
    if (distIndex >= wheelDistArrayLen) { distIndex = 0; }

    for (int i = 0; i < wheelDistArrayLen; i++) {
      setLED(1, true);
      int d = distances[i];
      Serial.print(d);
      Serial.print(" , ");

      if (d < 300) {
        // index of an entry where the wheel was probably in the closest part of its rotation
        closeIndex = i;
      }
      else if (400 < d && d < 700) {
        // index of an entry where the wheel was probably in the further part of its rotation
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

    // floodlight over course is really hot, so this should work
    float temp = getTemperature();
    Serial.print("temperature : ");
    Serial.print(temp);
    Serial.println("");
    if (temp > 35) {
      Serial.println("temperature is overthreshold so wheel section is success ?");
      setLED(0, true);
      setLED(1, true);
      setDriveMotors(-190, -190);
      delay(750);
      setDriveMotors(0, -190);
      delay(1200);
      stopDriveMotors();
      success = true;
      currentSection = 2;
    }
    else {
      doWheelCollisionStuff();
    }
  }
}

void doWheelCollisionStuff() {
  // wheel check based on the values in our distances array, but as we are gradually replacing old values with new ones (a few lines up)
  if (wheelCheck(false)) {
    
    // we are moving and/or what we are looking at is moving
    setLED(0, true);
    int d1 = getTof();
    if (d1 < 80) {
      setLED(0, true);
      stopDriveMotors();
      // wait for wheel to pass us by?
      delay(1250);
      setDriveMotors(-190, -190);
      delay(650);
      setDriveMotors(-190, 0);
      delay(400);
      stopDriveMotors();
      delay(50);
      setDriveMotors(200, 210);
    }
    else {
      setLED(0, false);
      setDriveMotors(200, 210);
    }
  }
  
  else {
    // we are sitting still and whatever we are looking at is also sitting still
    setLED(0, true);
    Serial.println("this is a wall aaaaa !!!");
    stopDriveMotors();
    // wait for wheel to pass us by?
    delay(1250);
    setDriveMotors(-190, -190);
    delay(650);
    setDriveMotors(-190, 0);
    delay(400);
    stopDriveMotors();
    delay(50);
    setDriveMotors(200, 210);
  }
}

// END SPINNY WHEEL

// START INFRARED

void followIRBeacon() {
  Serial.println("following IR beacon");
  bool driveToCorridorSuccess = false;
  bool finalSuccess = false;
  bool overallSuccess = false;

  while (!finalSuccess) {

    long ir = getIR();
    IRreadings[IRindex] = ir;
    IRindex++;
    if (IRindex >= IRarrayLen) { IRindex = 0; }

    char pos = checkIRCorridorPos();

    if (pos == 'l') {
      setLED(0, true);
      setLED(1, false);
      //pos = 'l';
      Serial.println(pos);
      setDriveMotors(190, 0);
    }
    else if (pos == 'r') {
      setLED(0, false);
      setLED(1, true);
      //pos = 'r';
      Serial.println(pos);
      setDriveMotors(0, 190);
    }
    else if (pos == 'c') {
      if (getTof() < 50) {
        stopDriveMotors();
        Serial.println("in IR section, centred, stopped");
        finalSuccess = true;
        delay(200);
      }
      else {
        setLED(0, true);
        setLED(1, true);
        //pos = 'c';
        Serial.println(pos);
        setDriveMotors(190, 190);
      }
      
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

  if (_max == 0) {
    if (wheelCheck(false)) {
      setLED(0, true);
      int d1 = getTof();
      if (d1 < 80) {
        setLED(0, true);
        stopDriveMotors();
        // wait for wheel to pass us by?
        delay(1250);
        setDriveMotors(-190, -190);
        delay(650);
        setDriveMotors(-190, 0);
        delay(400);
        stopDriveMotors();
        delay(50);
        setDriveMotors(200, 210);
        Serial.println("that was a wall aaaa");
      }
      else {
        setLED(0, false);
        setDriveMotors(200, 210);
      }
    }
  }
  else if (_max == leftCount) {
    return 'l';
  }
  else if (_max == rightCount) {
    return 'r';
  }
  else if (_max == centreCount) {
    return 'c';
  }
  
}


// END INFRARED
