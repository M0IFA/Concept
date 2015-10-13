// My_RTC_LCD
// display time on LCD I2C

// CONNECTIONS
// RTC DS1307 or DS3231
// SCL = A5
// SDA = A4
// I2C address 0x57
// ------
// display LCD I2C
// o A5 SCL
// o A4 SDA
// o +5
// o GND
// I2C address 0x27

// libraries
#include "Wire.h"
#include "LiquidCrystal_I2C.h"

// RTC I2C address
#define RTCADDR 0x68

// LCD
#define LCDADDR 0x27
#define LCDCOLS 16
#define LCDROWS 2

// LCD object
LiquidCrystal_I2C lcd(LCDADDR, LCDCOLS, LCDROWS);

// RTC time and date
byte Second, prevSecond, Minute, Hour, DoW, Date, prevDate, Month, Year;

void setup() {
  // initialise the wire library for I2C comms
  Wire.begin();

  // init LCD & backlight on
  lcd.init();
  lcd.backlight();

  getRTC();
  dispDate(0, 0); //  display date & time
  dispTime(4, 1);
  prevSecond = Second; // save current second & date
  prevDate = Date;
}

void loop() {
  getRTC(); // get time
  if (Second != prevSecond) {
    dispTime(4, 1); // display it, if changed
    prevSecond = Second;
  }
  if (Date != prevDate) {
    dispDate(0, 0);
    prevDate = Date;
  }
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

// display date and time
void dispDate(byte c, byte r) {
  lcd.clear();
  
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
  lcd.print(" ");
  lcd.print("20");
  lcd.print(Year);
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

