// My_RTC set
// enter YYMMDDwHHMMSS on Monitor, w = week day
// note 1 = mon. 01 = Jan
// hit ENTER exactly on the time you want to set

#include "Wire.h"
#include "DS3231.h"
#include "LiquidCrystal_I2C.h"

// LCD address, cols, rows
#define LCDADDR 0x27
#define LCDCOLS 16
#define LCDROWS 2

// RTC address
#define RTCADDR 0x68

// LCD object
LiquidCrystal_I2C lcd(LCDADDR, LCDCOLS, LCDROWS);

// rtc object
DS3231 rtc;

// RTC time and date
byte Second, prevSecond, Minute, Hour, DoW, Date, prevDate, Month, Year;

bool gotString;

void setup() {
  // Start the serial port
  Serial.begin(9600);

  // init LCD & backlight on
  lcd.init();
  lcd.backlight();

  // Start the I2C interface
  Wire.begin();

  dispMsg(0, 0, "Enter time:           ");
  dispMsg(0, 1, "\"YYMMDDwHHMMSS\"");

  gotString = false;
}

void loop() {
  char inString[20] = "";
  byte j = 0;
  
  while (!gotString) {
    if (Serial.available()) {
      inString[j] = Serial.read();

      if (inString[j] == '\n') {
        gotString = true;

        Serial.println(inString);

        // convert ASCII codes to bytes
        Year = ((byte)inString[0] - 48) * 10 + (byte)inString[1] - 48;
        Month = ((byte)inString[2] - 48) * 10 + (byte)inString[3] - 48;
        Date = ((byte)inString[4] - 48) * 10 + (byte)inString[5] - 48;
        DoW = ((byte)inString[6] - 48);
        Hour = ((byte)inString[7] - 48) * 10 + (byte)inString[8] - 48;
        Minute = ((byte)inString[9] - 48) * 10 + (byte)inString[10] - 48;
        Second = ((byte)inString[11] - 48) * 10 + (byte)inString[12] - 48;

        rtc.setYear(Year);
        rtc.setMonth(Month);
        rtc.setDate(Date);
        rtc.setDoW(DoW);
        rtc.setHour(Hour);
        rtc.setMinute(Minute);
        rtc.setSecond(Second);
      }
      j += 1;
    }
  }

  getRTC(); // get time
  
  if(Second != prevSecond) {
      dispTime(4, 1); // display it, if changed
      prevSecond = Second;
  }
  if(Date != prevDate) {
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
  Hour = bcdToDec(Wire.read() & 0b111111); //mask 12/24 bit
  DoW = bcdToDec(Wire.read()); //0 - 6 = Sunday - Saturday
  Date = bcdToDec(Wire.read()); // 1 - 31
  Month = bcdToDec(Wire.read()); // 0 = jan
  Year = bcdToDec(Wire.read()); // 20xx
}

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val) {
  return ( (val / 16 * 10) + (val % 16) );
}

// display char msg at col c, row r
void dispMsg(uint8_t c, uint8_t r, char *m) {
  lcd.setCursor(c, r);
  lcd.print(m);
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

