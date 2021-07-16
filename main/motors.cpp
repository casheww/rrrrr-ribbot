
/*
 * As the name suggests, this is just motor control for our pair of DC motors. Used a DRV8833 motor bridge.
 */

#include "motors.h"

#include <analogWrite.h>
#include <cmath>
using namespace std;

const int left1 = 27;
const int left2 = 26;
const int right1 = 23;
const int right2 = 25;


void setupMotors() {
  pinMode(left1, OUTPUT);
  pinMode(left2, OUTPUT);
  pinMode(right1, OUTPUT);
  pinMode(right2, OUTPUT);
}


void stopDriveMotors() {
  analogWrite(left1, 0);
  analogWrite(left2, 0);
  analogWrite(right1, 0);
  analogWrite(right2, 0);
}


/* 
 *  Our motors are driven by the effective potential difference / voltage between each of their two pins.
 *  Pins are opperated using PWM. I think 255 doesn't work for some reason, but having tested the 180..200 range, it works fine.
 *  Just pass a value in that range for each motor. Negative values will be used to drive the motor backwards.
 *  
 *  https://www.ti.com/lit/ds/symlink/drv8833.pdf
 *  Tables 1 & 2 from section 7.3.2 of the above doc were useful here.
 */
void setDriveMotors(int powerL, int powerR) {
  // right motor
  if (powerR > 0) {
    // forwards
    analogWrite(right1, 0);
    analogWrite(right2, abs(powerR));
  }
  else {
    // backwards
    analogWrite(right1, abs(powerR));
    analogWrite(right2, 0);
  }

  // left motor
  if (powerL > 0) {
    // forwards
    analogWrite(left1, 0);
    analogWrite(left2, abs(powerL));
  }
  else {
    // backwards
    analogWrite(left1, abs(powerL));
    analogWrite(left2, 0);
  }
}
