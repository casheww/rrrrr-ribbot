#include <analogWrite.h>

#include "debug.h"

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
