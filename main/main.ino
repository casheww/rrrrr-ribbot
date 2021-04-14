#include <analogWrite.h>

#include "debug.h"
#include "spacial.h"

#define LED_BUILTIN 1


void setup() {
  Serial.begin(115200);   // setup debugging connection back to a computer
  setupTof();             // setup TOF things
}

void loop() {
  setLED(0, true);
  delay(1000);
  setLED(1, true);
  delay(1000);
  setLED(0, false);
  setLED(1, false);
  delay(1000);

  int tofDist = getTof();
  Serial.print(tofDist);
  Serial.println();
}
