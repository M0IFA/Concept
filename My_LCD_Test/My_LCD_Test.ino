// My_LCD_Test 
// displays a test message on the LCD

// include libraries for I2C comms and LCD driver
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD I2C address, cols, rows
#define LCDADDR 0x27
#define LCDCOLS 16
#define LCDROWS 2

// create an LCD object "lcd"
LiquidCrystal_I2C lcd(LCDADDR, LCDCOLS, LCDROWS);

// setup runs once on upload
void setup() {
  // initialise the LCD & switch the backlight on
  lcd.init();
  lcd.backlight();
  
  // move the cursor to col, row and output text
  lcd.setCursor(3, 0);
  lcd.print(" Concept ");

  // wait 2 sec (2000ms)
  delay(2000);
  
  // move the cursor to col, row and output text
  lcd.setCursor(3, 1);
  lcd.print(" Program ");
}

// loop does nothing, but must be here
void loop() {
  
}
