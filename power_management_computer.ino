#include <Time.h>
#include <TimeLib.h>
#include <LiquidCrystal_I2C.h>
#include <DS1307RTC.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  
class MyTimer
{
  // Class Member Variables
  // These are initialized at startup
  boolean double_digit;
  
  
  // Constructor initializes the member variables and state
  public:
  //MyTimer(byte input)  //import data here
  //{
  //counter = input;
  //}
  MyTimer()
  {
  }
  byte counter;    // accessable from outside via (MyTimer).counter 
  void Update() {
    lcd.setCursor(8,0);
    if(counter) {
      if(counter <= 9) lcd.print(" "); // clear the first space if a single digit
    lcd.print(counter);                 // print the counter
    counter--;                          // counter decrements here
    } else lcd.print("  ");             // no counter, so clear the board    
  }
  void Set(byte x) {

    counter = x;
  }
};
