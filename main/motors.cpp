#include "motors.h"

#include <analogWrite.h>
#include <cmath>
using namespace std;

const int left1 = 23;
const int left2 = 25;
const int right1 = 27;
const int right2 = 26;


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


/* motors can be driven backwards using negative power values */
void setDriveMotors(int powerL, int powerR) {
  if (powerR > 0){
    analogWrite(right1, 0);
    analogWrite(right2, abs(powerR));
  }
  else {
    analogWrite(right1, abs(powerR));
    analogWrite(right2, 0);
  }

  if (powerL > 0) {
    analogWrite(left1, 0);
    analogWrite(left2, abs(powerL));
  }
  else {
    analogWrite(left1, abs(powerL));
    analogWrite(left2, 0);
  }
}
