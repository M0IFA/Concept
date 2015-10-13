// My_Morse_KB is a morse morse sender with KB input
// active piezo buzzer on pin 7

// include the special llibrary to generate morse dit/dah
#include "MorseEnDecoder.h"

// define a constant for words per minute
#define WPM 5

// define a constant for the buzzer pin
#define BUZZER 7

// create a morseOut object
morseEncoder morseOut(BUZZER);

// setup runs once on upload
void setup() {
  // set BUZZER pin as an output
  pinMode(BUZZER, OUTPUT);

  // start serial comms with Arduino IDE monitor window, over USB
  Serial.begin(9600);
  while (!Serial); // wait for USB connection

  // call morseOUT object to set the morse speed
  morseOut.setspeed(WPM);
}

// loop repeats over and over
void loop() {
  // char variable to hold a character of input
  char text;

  // set morseOut encode mode
  morseOut.encode();

  // if characters in the input buffer & morse object available
  if (Serial.available() && morseOut.available()) {
    text = Serial.read(); // read a character
    Serial.write(text); // echo it
    morseOut.write(text); // send out as morse
  }
}



