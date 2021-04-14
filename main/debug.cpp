#include "debug.h"

#include <analogWrite.h>

#define LED_BUILTIN 1
int leftLED = 18;
int rightLED = 19;


/* side: 0 for left, 1 for right */
void setLED(int side, bool _on) { 
  int brightness;
  _on ? brightness = 200 : brightness = 0;

  if (side == 0) {
    analogWrite(leftLED, brightness);
  }
  else if (side == 1) {
    analogWrite(rightLED, brightness);
  }
}

void doPrettyLEDs() {
  for (int i = 0; i < 255; i+=15) {
    analogWrite(leftLED, i);
    analogWrite(rightLED, 255 - i);
    delay(50);
  }
  for (int i = 0; i < 255; i+=25) {
    analogWrite(rightLED, i);
    analogWrite(leftLED, 255 - i);
    delay(50);
  }
}
