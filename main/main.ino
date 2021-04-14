#include <analogWrite.h>

#include "debug.h"

#define LED_BUILTIN 1

void setup() {
  analogWrite(LED_BUILTIN, 250);
}

void loop() {
  setLED(0, true);
  delay(1000);
  setLED(1, true);
  delay(1000);
  setLED(0, false);
  setLED(1, false);
  delay(1000);
}
