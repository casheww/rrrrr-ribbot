#include "spacial.h"

#include <Wire.h>
#include <VL53L0X.h>

// time of flight var
VL53L0X tof;

// tracker sensor pins
int rightTracker = 13;
int leftTracker = 16;


void setupSpacial() {
  // start TOF setup
  
  Wire.begin();

  tof.setTimeout(500);
  if (!tof.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
  }

  /// uncomment next line to set tof time allowance to 20ms (from default 33ms) !! decreases accuracy. Can increase time to increase accuracy too
  // tof.setMeasurementTimingBudget(20000);

  // end TOF setup

  // trackers
  pinMode(leftTracker, INPUT);
  pinMode(rightTracker, INPUT);
}


int getTof() {
  if (tof.timeoutOccurred()) {
    return 0;
  }
  
  return tof.readRangeSingleMillimeters();
}


/* side: 0 for left, 1 for right
 * return: True if tracker does yes thing
 */
bool getTracker(int side) {
  return digitalRead(side == 0 ? leftTracker : rightTracker) == 1;
}
