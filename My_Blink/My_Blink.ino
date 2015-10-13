// My_Blink
// flashes a LED on pin 13

// pin number
#define LED 13

// the setup routine runs once when you upload (->) the sketch
void setup() {
  // initialise the digital pin 13 as an output
  pinMode(LED, OUTPUT);
}

// the loop runs over and over again, forever
void loop() {
  digitalWrite(LED, HIGH);  // turn the LED on (HIGH voltage level)
  delay(1000);              // wait for 1 second (1000ms)
  digitalWrite(LED, LOW);   // turn the LED off (LOW voltage leve)
  delay(1000);              // wait for 1 second
}

