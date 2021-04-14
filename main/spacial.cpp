#include "spacial.h"

#include <Wire.h>
#include <VL53L0X.h>

VL53L0X tof;


void setupTof() {
  Wire.begin();

  tof.setTimeout(500);
  if (!tof.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
  }

  /// uncomment next line to set tof time allowance to 20ms (from default 33ms) !! decreases accuracy. Can increase time to increase accuracy too
  // tof.setMeasurementTimingBudget(20000);
}


int getTof() {
  if (tof.timeoutOccurred()) {
    return 0;
  }
  
  return tof.readRangeSingleMillimeters();
}
