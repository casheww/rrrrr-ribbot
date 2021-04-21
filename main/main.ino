#include <analogWrite.h>

#include "debug.h"
#include "spacial.h"

#define LED_BUILTIN 1


void setup() {
  Serial.begin(115200);   // setup debugging connection back to a computer
  setupSpacial();         // setup spacial sensors and pins and whatnot, y'know
}

void loop() {
  setLED(0, true);
  delay(1000);
  setLED(1, true);
  delay(1000);
  setLED(0, false);
  setLED(1, false);
  delay(1000);

  getTracker(0);

  int tofDist = getTof();
  Serial.print(tofDist);
  Serial.println();

  Serial.print("tracker:");
  Serial.print(getTracker(0));
  Serial.println();
}
