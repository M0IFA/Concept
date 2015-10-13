// My_WSPR_40M
// transmits WSPR on 40m
// use WSPR_Symbol_Generator sketch to make symbol tone table
// insert symbols and msgtxt below
// it is hard coded here for msgtxt " M6KWH IO92 20", note leading space, 3rd char must be number
// tuning in 10Hz steps
// output on CLK0, VFO_RTC_IQ shield "G0" pin
// Interface by I2C bus to RTC, init RTC time before use


// CONNECTIONS
// RTC
// SCL = A5
// SDA = A4
// I2C address 0x68
// -----
// DDS I2C SI5351
// SCL = A5
// SDA = A4
// I2C address 0x60
// ------
// display I2C LCD 16 * 2
// o A5 SCL (y)
// o A4 SDA (or)
// o +5     (r)
// o GND    (bwn)
// I2C address 0x27
// -----
// encoder KY-040
// o D2 DT  (y)
// o D3 CLK (g)
// o D4 SW  (or)
// o +5     (r)
// o GND    (bwn)
// -----

// libraries
#include "Wire.h"
#include "si5351.h"
#include "LiquidCrystal_I2C.h"
#include "Rotary.h"
#include "TimerOne.h"

// RTC I2C address
#define RTCADDR 0x68

// LCD
#define LCDADDR 0x27
#define LCDCOLS 16
#define LCDROWS 2

// rotary Encoder pins 2 & 3 (DT & CLK), button 4 (SW)
#define DT 2
#define CLK 3
#define SW 4

// dds object
Si5351 dds;

// LCD object
LiquidCrystal_I2C lcd(LCDADDR, LCDCOLS, LCDROWS);

// rotary sncoder object
Rotary rot = Rotary(DT, CLK);

uint8_t tone_ptr = 0; // Pointer to the current symbol

// start freq (mid band) cHz, step 10Hz, WSPR Tx is 704000000 - 704020000, 
// note standard is dial 70386000 + 150000 = 704010000 or "100" in WSPR.app software
uint32_t freq = 704010000;
uint32_t steps = 1000;

// ====================== insert your message text & symbol
char msgtxt[] = {" M6KWH IO92 20  "};
// symbols - use WSSPR Symbol Generator program to create/paste here
uint32_t WSPR_DATA[162] =
{ 3, 1, 2, 0, 0, 0, 2, 2, 1, 2, 0, 2, 3, 1, 3, 0, 2, 2, 1, 2, 0, 3, 0, 1, 3, 1, 1, 0, 2, 0, 0, 2,
  0, 0, 1, 0, 0, 1, 0, 1, 2, 0, 0, 0, 0, 2, 1, 2, 1, 3, 0, 0, 1, 3, 0, 1, 2, 0, 0, 1, 3, 2, 1, 0,
  2, 2, 0, 3, 3, 2, 3, 2, 3, 0, 3, 0, 3, 2, 0, 3, 2, 0, 1, 2, 1, 3, 0, 0, 0, 3, 1, 0, 1, 0, 1, 0,
  2, 2, 3, 0, 0, 0, 0, 0, 1, 0, 2, 1, 0, 2, 1, 3, 1, 2, 1, 1, 0, 0, 3, 3, 2, 1, 2, 0, 0, 1, 1, 1,
  2, 2, 2, 0, 0, 3, 0, 3, 2, 0, 3, 1, 2, 2, 2, 0, 2, 0, 2, 3, 1, 0, 1, 0, 1, 1, 2, 0, 0, 3, 1, 0,
  2, 0
};
// ======================

// RTC time seconds, minutes, hours
byte sec, mns, hr;

// repeat interval 2,4,6,8 min
uint8_t repeat;

void setup()
{
  // initialise the wire library for I2C comms
  Wire.begin();

  // init LCD & backlight on
  lcd.init();
  lcd.backlight();

  // init dds si5351 module, "0" = default 25MHz XTAL
  dds.init(SI5351_CRYSTAL_LOAD_8PF, 0);

  // put calibration here...

  // set 8mA output drive
  dds.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);

  // disable all outputs
  dds.output_enable(SI5351_CLK0, 0);
  dds.output_enable(SI5351_CLK1, 0);
  dds.output_enable(SI5351_CLK2, 0);

  // encoder, button
  pinMode(DT, INPUT_PULLUP);
  pinMode(CLK, INPUT_PULLUP);
  pinMode(SW, INPUT_PULLUP);

  repeat = 2; // init to send every 2 mins

  dispMsg(0, 0, "WSPR            ");
  dispFreq(5, 0, freq, 2); // display freq b
  dispMsg(1, 1, " min   ");
  dispNum(0, 1, repeat);
}

void loop()
{
  if (tune()) {
    dispFreq(5, 0, freq, 2); // display freq;
  }

  if (event()) {
    dispMsg(1, 1, " min");
    dispNum(0, 1, repeat);
  }

  getRTC(); // read RTC, display time
  dispTime(8, 1);

  // send WSPR now?
  if (mns % repeat == 0 && sec == 0)
  {
    // display message
    dispMsg(0, 0, msgtxt);
    dispMsg(8, 1, "     TX "); // show transmit

    txWspr(); // transmit on freq

    // restore display
    dispMsg(0, 0, "WSPR           ");
    dispFreq(5, 0, freq, 2); // display freq
    dispMsg(1, 1, " min           ");
    dispNum(0, 1, repeat);
  }
}

// tune freq
bool tune()
{
  unsigned char dir; // tuning direction CW/CCW

  dir = rot.process(); // read encoder
  if (dir != DIR_NONE) // turned?
  {
    if (dir == DIR_CW) freq += steps;; // increment freq
    if (dir == DIR_CCW) freq -= steps;
    return true;
  }
  return false;
}

// set event time 2,4,6,8 min
bool event()
{
  if (digitalRead(SW) == LOW)
  {
    while (!digitalRead(SW)); // wait for release
    if (repeat == 8) repeat = 2;
    else repeat += 2;
    return true;
  }
  return false;
}


// get time from RTC, convert bcd to decimal
void getRTC()
{
  // Reset the register pointer
  Wire.beginTransmission(RTCADDR);
  byte zero = 0x00;
  Wire.write(zero);
  Wire.endTransmission();

  // request 1st 3 bytes from the RTC address
  Wire.requestFrom(RTCADDR, 3);

  // get the s/m/h time data
  sec = bcdToDec(Wire.read());
  mns = bcdToDec(Wire.read());
  hr = bcdToDec(Wire.read() & 0b111111); // mask 24 hour time bit
}

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val / 16 * 10) + (val % 16) );
}

void txWspr() {
  int i;
  
  dds.output_enable(SI5351_CLK0, 1); // CLK0 on
  
  for(i = 0; i < 162; i++) {
    freqOut(freq + WSPR_DATA[i] * 146ULL);
    delay(682); // set empirically for TX lasting 110sec
  }
  
  dds.output_enable(SI5351_CLK0, 0); // CLK0 off
}

// output freq cHz
void freqOut(uint32_t f) {
    dds.set_freq(f, 0ULL, SI5351_CLK0);
}

// display char msg at col c, row r
void dispMsg(uint8_t c, uint8_t r, char *m)
{
  lcd.setCursor(c, r);
  lcd.print(m);
}

// display a number at col c, row r
void dispNum(uint8_t c, uint8_t r, uint16_t n)
{
  lcd.setCursor(c, r);
  lcd.print(n);
}

// display freq in kHz,col c, row r, d decimal places
void dispFreq(uint8_t c, uint8_t r, uint32_t f, uint8_t d)
{
  lcd.setCursor(c, r); // clear last freq display
  lcd.print((float)f / 100000, d); // convert to float for print function
  lcd.print("kHz  "); // + trailing spaces to clear previous display
}

// display time at col, row
void dispTime(byte c, byte r) {
  lcd.setCursor(c, r);
  if (hr < 10)
    lcd.print("0");
  lcd.print(hr);
  lcd.print(":");
  if (mns < 10)
    lcd.print("0");
  lcd.print(mns);
  lcd.print(":");
  if (sec < 10)
    lcd.print("0");
  lcd.print(sec);
}


