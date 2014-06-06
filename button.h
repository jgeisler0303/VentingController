#ifndef BUTTON_H
#define BUTTON_H

const int buttonPin = 3;     // the number of the pushbutton pin

inline boolean buttonPressed() {
  return (digitalRead(buttonPin) == LOW);
}

#endif

