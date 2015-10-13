// My_VFO_40M_4
// 20 x 4 display
// for 40m, with TX/RX control and bandplan display
// Si5351 I2C bus
// SDA = A4
// SCL = A5
// LCD I2C bus
// SDA = A4
// SCL = A5
// rotary encoder pins§             
// DT = 2
// CLK = 3
// SW = 4

// I2C, Si5351, LCD and rotary Encoder libraries
#include "Wire.h"
#include "si5351.h"
#include "LiquidCrystal_I2C.h"
#include "Rotary.h"

// RTC I2C address
#define RTCADDR 0x68

// LCD
#define LCDADDR 0x27
#define LCDCOLS 20
#define LCDROWS 4

// rotary Encoder pins 2 & 3 (DT & CLK), step change pin 4 (SW)
#define DT 2
#define CLK 3
#define SW 4

// Rx & Tx signals
#define RX 13
#define TX 12
#define KEY 8

// number of band plans
#define PLANS 12

// dds object
Si5351 dds;

// LCD object
LiquidCrystal_I2C lcd(LCDADDR, LCDCOLS, LCDROWS);

// rotary Encoder object
Rotary rot = Rotary(DT, CLK);

// define plan structure
typedef struct {
  uint32_t lower;
  uint32_t upper;
  char alloc[30];
} plan;

// band plan array contents cHz/cHz/Text
plan bp[PLANS] = {
  {700000000, 700100000, "CW    QRSS   7000.7 "},
  {700100000, 703990000, "CW    PSK31 QRP 7030"},
  {703990000, 704690000, "NB    WSPR 7040     "},
  {704600000, 704990000, "NB    Auto          "},
  {704990000, 705290000, "All   Auto          "},
  {705290000, 705990000, "All   Digital       "},
  {705990000, 706990000, "All                 "},
  {706990000, 707990000, "All   HELL 7077     "},
  {707990000, 709990000, "All   SSB QRP 7090  "},
  {709990000, 712990000, "All   EMGCY 7110    "},
  {712990000, 717490000, "All   SSB CON 7165  "},
  {717490000, 720010000, "All   DX Intnl      "},
};

// start freq & step
uint32_t freq = 700000000; // cHz, start frequency
uint32_t step = 10000; // cHz, init 100Hz step

// RTC time and date
byte Second, prevSecond, Minute, Hour, DoW, Date, prevDate, Month, Year;

void setup() {
  // init LCD & backlight on
  lcd.init();
  lcd.backlight();

  // init dds si5351 module, "0" = default 25MHz XTAL
  dds.init(SI5351_CRYSTAL_LOAD_8PF, 0);

  // set 8mA output drive (max possible)
  dds.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);

  // can insert Si5351 calibration here if required

  // enable VFO output CLK0, disable CLK1 & 2
  dds.output_enable(SI5351_CLK0, 1);
  dds.output_enable(SI5351_CLK1, 0);
  dds.output_enable(SI5351_CLK2, 0);

  // encoder, button, RX, TX, band and KEY pins
  pinMode(DT, INPUT_PULLUP);
  pinMode(CLK, INPUT_PULLUP);
  pinMode(SW, INPUT_PULLUP);

  pinMode(RX, OUTPUT);
  pinMode(TX, OUTPUT);
  pinMode(KEY, INPUT_PULLUP);

  xmit(digitalRead(KEY)); // set RX|TX, KEY = LOW is TX
  getRTC(); // get time

  freqOut(freq); // cHz, output freq

  dispDate(0, 0);
  dispTime(12, 0);
  dispFreq(6, 2, freq, 1); // display freq kHz col 4 row 2
  dispMsg(0, 3, scanPlan()); // display band plan col 0 row 3
}

void loop() {
  getRTC(); // get time
  if (Date != prevDate) {
    dispDate(0, 0);
    prevDate = Date;
  }
  if (Second != prevSecond) {
    dispTime(12, 0); // display it, if changed
    prevSecond = Second;
  }

  // tune?
  if (tune()) {
    freqOut(freq); // output freq
    dispFreq(6, 2, freq, 1); // update freq display
    dispMsg(0, 3, scanPlan()); // update band plan display
  }

  // step?
  if (button()) {
    dispStep(step, 17, 2);
  }
  xmit(digitalRead(KEY)); // RX|TX
}

// tune?
bool tune() {
  unsigned char dir; // tuning direction CW/CCW

  dir = rot.process(); // read encoder
  if (dir != DIR_NONE) { // turned?
    if (dir == DIR_CW && (freq < bp[PLANS - 1].upper - step)) freq += step;
    if (dir == DIR_CCW && (freq >= bp[0].lower + step)) freq -= step;
    return true;
  }
  return false;
}

// change step?
bool button() {
  if (digitalRead(SW) == LOW) { // button pressed?
    while (!digitalRead(SW)); // wait for release
    if (step == 1000000) step = 10000; // reset
    else step = step * 10; // or increment by x10
    return true;
  }
  return false;
}

// search for band info
char *scanPlan() {
  for (int i = 0; i < 15; i++) {
    if (freq >= bp[i].lower && freq < bp[i].upper) // find plan
      return bp[i].alloc; // return when found
  }
}

// Output Freq for VFO, on CLK0, f cHz
void freqOut(uint32_t f) {
  dds.set_freq(f, 0ULL, SI5351_CLK0);
}

// Tx/Rx KEY HIGH = RX, LOW = TX
void xmit(bool x)
{
  if (x == LOW) // TX
  {
    dispMsg(0, 2, "TX ");
    digitalWrite(RX, HIGH); // Rx off
    digitalWrite(TX, LOW); // Tx on
  }
  else
  {
    dispMsg(0, 2, "VFO");
    digitalWrite(RX, LOW); // Rx on
    digitalWrite(TX, HIGH); // Tx off
  }
}

// display char msg at col c, row r
void dispMsg(uint8_t c, uint8_t r, char *m) {
  lcd.setCursor(c, r);
  lcd.print(m);
}

// get time from RTC, convert bcd to decimal
void getRTC() {
  // Reset the RTC register pointer
  Wire.beginTransmission(RTCADDR);
  byte zero = 0x00;
  Wire.write(zero);
  Wire.endTransmission();

  // request 7 bytes from the RTC address
  Wire.requestFrom(RTCADDR, 7);

  // get the time data
  Second = bcdToDec(Wire.read()); // 0 - 59
  Minute = bcdToDec(Wire.read()); // 0 - 59
  Hour = bcdToDec(Wire.read() & 0b111111); // mask 12/24 bit
  DoW = bcdToDec(Wire.read()); //0 - 6 = Sunday - Saturday
  Date = bcdToDec(Wire.read()); // 1 - 31
  Month = bcdToDec(Wire.read()); // 0 = jan
  Year = bcdToDec(Wire.read()); // 20xx
}

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val) {
  return ( (val / 16 * 10) + (val % 16) );
}

// display freq in kHz at col c, row r, f cHz, d decimal places
void dispFreq(uint8_t c, uint8_t r, uint32_t f, uint8_t d) {
  lcd.setCursor(c, r);
  lcd.print((float)f / 100000, d); // convert to float & kHz
  lcd.print("kHz ");
}

// display step
void dispStep(uint32_t s, byte c, byte r)
{
  switch (s) // display step
  {
    case 10000:
      dispMsg(c, r, "  ");
      break;
    case 100000:
      dispMsg(c, r, " +");
      break;
    case 1000000:
      dispMsg(c, r, "++");
      break;
  }
}


// display date and time
void dispDate(byte c, byte r) {
  lcd.setCursor(c, r);
  switch (DoW) {
    case 1:
      lcd.print("Mon");
      break;
    case 2:
      lcd.print("Tue");
      break;
    case 3:
      lcd.print("Wed");
      break;
    case 4:
      lcd.print("Thu");
      break;
    case 5:
      lcd.print("Fri");
      break;
    case 6:
      lcd.print("Sat");
      break;
    case 7:
      lcd.print("Sun");
      break;
  }

  lcd.print(" ");
  lcd.print(Date);

  lcd.print(" ");
  switch (Month)
  {
    case 1:
      lcd.print("Jan");
      break;
    case 2:
      lcd.print("Feb");
      break;
    case 3:
      lcd.print("Mar");
      break;
    case 4:
      lcd.print("Apr");
      break;
    case 5:
      lcd.print("May");
      break;
    case 6:
      lcd.print("Jun");
      break;
    case 7:
      lcd.print("Jul");
      break;
    case 8:
      lcd.print("Aug");
      break;
    case 9:
      lcd.print("Sep");
      break;
    case 10:
      lcd.print("Oct");
      break;
    case 11:
      lcd.print("Nov");
      break;
    case 12:
      lcd.print("Dec");
      break;
  }
}

void dispTime(byte c, byte r) {
  lcd.setCursor(c, r);
  if (Hour < 10)
    lcd.print("0");
  lcd.print(Hour);
  lcd.print(":");
  if (Minute < 10)
    lcd.print("0");
  lcd.print(Minute);
  lcd.print(":");
  if (Second < 10)
    lcd.print("0");
  lcd.print(Second);
}

