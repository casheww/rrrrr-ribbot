
/*
 * This was just going to be for spacial sensors, but at this point why not have the IR receiver here too. Or all of the sensors, that works too
 */

#include "spacial.h"

#include <stdint.h>

#include <Wire.h>
#include <VL53L0X.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>
#include <OneWire.h>
#include <DallasTemperature.h>



// time of flight var
VL53L0X tof;

// tracker sensor pins
const int rightTracker = 13;
const int leftTracker = 16;

// usound
const int usoundTrigger = 33;
const int usoundEcho = 32;

// accel & gyro
MPU6050 accelgyro;
int16_t ax, ay, az;
int16_t gx, gy, gz;

// ir receiver
const int irPin = 14;
IRrecv irrecv(irPin);

// thermometer
#define ONE_WIRE_BUS 17
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature thermo(&oneWire);


// ===================================


void setupSpacial() {
  // start TOF setup
  
  Wire.begin();

  tof.setTimeout(500);
  if (!tof.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
  }

  /// uncomment next line to set tof time allowance to 20ms (from default 33ms) !! decreases accuracy. Can increase time to increase accuracy too
  tof.setMeasurementTimingBudget(40000);    // 40000 = 40ms , so higher accuracy

  // end TOF setup

  // trackers
  pinMode(leftTracker, INPUT);
  pinMode(rightTracker, INPUT);

  // ultrasound distance sensor
  pinMode(usoundTrigger, OUTPUT);
  pinMode(usoundEcho, INPUT);

  // accel & gyro
  accelgyro.initialize();

  // ir receiver
  irrecv.enableIRIn();

  // thermometer
  thermo.begin();
}

/* gets a distance reading from the front-facing time of flight sensor (in mm) */
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

/* gets a time reading from the back-facing ultrasound sensor... haven't been bothered to convert to distance yet */
int getUsound() {
  // send trigger signal - request for a reading
  digitalWrite(usoundTrigger, HIGH);
  delay(0.01);
  digitalWrite(usoundTrigger, LOW);

  // read echo / return from sensor
  return pulseIn(usoundEcho, HIGH);
}

/* maybe functional... we're not really using this */
short* getGyro() {
  // I have no clue how pointers work aaAAAA
  
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  short motion[6] = { ax, ay, az, gx, gy, gz };

  short* ptr = (short*)malloc(6 * sizeof(short));

  for (int i = 0; i < 6; i++) {
    ptr[i] = motion[i];
  }

  return ptr;
}

/* gets a reading from the infrared receiver */
long getIR() {
  // I think this works???
  decode_results res;
  // check that the result can be decoded
  if (irrecv.decode(&res)) {
    irrecv.resume();
    return res.value;
  }
}

/* gets a temperature reading from the thermometer */
float getTemperature() {
  thermo.requestTemperatures();
  float t = thermo.getTempCByIndex(0);
  
  if(t != DEVICE_DISCONNECTED_C) {
    return t;
  } 
  else {
    return 0;
  }
}
