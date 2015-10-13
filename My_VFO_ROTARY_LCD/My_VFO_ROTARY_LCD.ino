// My_VFO_ROTARY_LCD controls the freq by rotary encoder, freq display on LCD
// button changes band
// Si5351 I2C bus 
// SDA = A4 
// SCL = A5
// LCD I2C bus 
// SDA = A4 
// SCL = A5
// rotary encoder pins 
// DT = 2 
// CLK = 3
// SW = 4 

// I2C, Si5351, LCD and rotary Encoder libraries
#include "Wire.h"
#include "si5351.h"
#include "Rotary.h"
#include "LiquidCrystal_I2C.h"

// tuning freq STEPS (cHz), 100Hz
#define STEPS 10000

// rotary Encoder pins 2 & 3 (DT & CLK), band change pin 4 (SW)
#define DT 2
#define CLK 3
#define SW 4

// dds object
Si5351 dds;

// rotary Encoder object
Rotary rot = Rotary(DT, CLK);

// lcd object
LiquidCrystal_I2C lcd(0x27, 16, 2);

// start frequencies (cHz), band names
uint32_t freqStart[3] = {
  710000000, 1014000000, 1410000000};

// band, freq (cHz)
byte band = 0;
uint32_t freq = freqStart[band];

void setup() {
        
  // init LCD & backlight on
  lcd.init();
  lcd.backlight();
  
  // init dds si5351 module, "0" = default 25MHz XTAL
  dds.init(SI5351_CRYSTAL_LOAD_8PF, 0);

  // set 8mA output drive
  dds.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);

  // enable VFO output CLK0, disable CLK1 & 2
  dds.output_enable(SI5351_CLK0, 1);
  dds.output_enable(SI5351_CLK1, 0);
  dds.output_enable(SI5351_CLK2, 0);

  // encoder, button, RX, TX, band and XMIT pins
  pinMode(DT, INPUT_PULLUP);
  pinMode(CLK, INPUT_PULLUP);
  pinMode(SW, INPUT_PULLUP);
  
  freqOut(freq); // output freq
  dispFreq(freq); // display freq
}

void loop() {
  // tune?
  if (tune()) {
    freqOut(freq);
    dispFreq(freq);
  }
  
  // band?
  if (button()) {
    freq = freqStart[band];
    freqOut(freq);
    dispFreq(freq);
  }
}

// tune?
bool tune() {
  unsigned char dir; // tuning direction CW/CCW

  // turned?
  dir = rot.process(); // read encoder
  if (dir != DIR_NONE) { // turned?
    if (dir == DIR_CW) freq += STEPS; // increment freq +/- STEPS
    if (dir == DIR_CCW) freq -= STEPS;
    return true;
  }
  return false;
}

// band?
bool button() {
  if (digitalRead(SW) == LOW) { // button pressed?
    while (!digitalRead(SW)); // wait for release
    if (band == 2) band = 0; // loop
    else band++;
    return true;
  }
  return false;
}

// frequency (in cHz) for VFO, on CLK0
void freqOut(uint32_t f) {
    dds.set_freq(f, 0ULL, SI5351_CLK0); // converted to cHz
}

// display freq in cHz
void dispFreq(uint32_t f) {
  lcd.setCursor(0, 0);
  lcd.print("VFO                 ");
  lcd.setCursor(4, 0);
  lcd.print((float)f / 100000, 1); // convert to float for print function
  lcd.setCursor(13, 0);
  lcd.print("kHz");
}
