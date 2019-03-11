//*********************FUNCTION DIRECTORY IN ALPHABETICAL ORDER
//*************************************************************************************************************
//myAuxPowerfunction
//myBacklightfunction
//myBalancePowerBetweenChargerAndInverterfunction
//myCalculateWeekNumberfunction
//myClearMessageBoardfunction
//myIsItDaylightfunction
//myLoadUpcomingEventsfunction
//myMessageManagerfunction
//myMessageInverterRunTimefunction
//myMessageReminderfunction
//myMessageSunrisefunction()
//myMessageUpcomingEventsfunction()
//myMessageVoltageDailyHighfunction
//myMessageVoltageDailyLowfunction
//myMessageWeekNumberfunction
//myNumericalSuffixCalculatorfunction
//myPrintStatefunction
//myPrintDatetoLCDfunction
//myPrintLDRresultsToLCDfunction
//myPrintSerialTimestampfunction 
//myPrintTimetoLCDfunction
//myPrintVoltagetoLCDfunction
//myPulseGeneratorfunction
//myReadPotentiometerAndAdjustWorkbenchTrackLightsfunction
//myReturnDayofWeekFromUnixTimestampfunction
//myReturnDayofWeekfunction
//mySetSunriseSunsetTimesfunction()
//mySwitchPowerManagerModesfunction()
//myTestForChargeFunction()
//myVoltageCalculationfunction
//myVoltagePrintingAndRecordingfunction

//===============================
void myPrintStatefunction(char* text){
//========2.27.2018==============

// This code handles the state of operation LCD printing in the upper left 8 characters of the 4x20 display   
  Serial.print("  state changed to: ");
  Serial.print(text);
  lcd.setCursor (0,0);
  lcd.print (text);
  
}



//--------------------------
void myBacklightfunction() {
//--------------------------   

  if (hour() >= hour_to_turn_on_backlight && hour() < hour_to_turn_off_backlight) { 
    lcd.backlight();
  } else {
    lcd.noBacklight();
  }
}


//==============================================================
byte myCalculateWeekNumberfunction(TimeElements sixpartdate) {
//==============================================================

   //get unix time from start of current year by copying a six part date (tm_Elements_t) and zeroing everything but the year.
   //then subtract that from the unix time now, then convert to weeks.
   
   long unsigned        longdate;
   long unsigned        seconds_since_start_of_year;
   const long unsigned  seconds_in_a_week = 604800;
   byte                 week_number;
   
   sixpartdate.Hour = 0;
   sixpartdate.Minute = 0;
   sixpartdate.Second = 0;
   sixpartdate.Day = 0;
   sixpartdate.Month = 0;
   longdate = makeTime (sixpartdate);
   seconds_since_start_of_year = (now() - longdate);
   week_number = (seconds_since_start_of_year/seconds_in_a_week) + 1;
   return (week_number);
   
}

//---------------------------------
void myClearMessageBoardfunction(){
//---------------------------------
   //                              "12345678901234567890"   
   lcd.setCursor (0,1); lcd.print (F("                    "));
   lcd.setCursor (0,2); lcd.print (F("                    "));
}

//********************************
boolean myIsItDaylightfunction() {
//********************************
  int sunrise_in_minutes = today_sunrise_hour * 60 + today_sunrise_minute;
  int sunset_in_minutes = today_sunset_hour * 60 + today_sunset_minute;
  int now_in_minutes = hour() * 60 + minute();

  if (now_in_minutes >= sunrise_in_minutes && now_in_minutes < sunset_in_minutes) {
    return true; 
  } else {
    return false;
  }
}

//=========================================
boolean myIsItDeltaTimePastDawnfunction() {
//=============================5.10.2018===

  //The wake up delay ensures that the the inverter does not burn off the fresh battery charger energy
  //If the inverter is reset in the middle of the day however, this hastens the inverter activation 
  const int sunrise_in_minutes = today_sunrise_hour * 60 + today_sunrise_minute;
  const int delta_t = 15;  //minutes 
  const int now_in_minutes = hour() * 60 + minute();
  /*Serial.println();
  Serial.print("now: ");
  Serial.print(now_in_minutes);
  Serial.print(" sunrise: ");
  Serial.print(sunrise_in_minutes);
  Serial.print(" deltaT: ");
  Serial.print(delta_t);
  Serial.print(" -- ");
  */
  
  
  if (now_in_minutes >= sunrise_in_minutes + delta_t) {
    return true; 
  } else {
    return false;
  }
}

//-------------------------------
void myLoadUpcomingEventsfunction(){
//-------------------------------
   
   byte scan_window = 14; // get records this many days from today
   
   long unsigned      seconds_in_a_day = 86400;
   long unsigned      record_zipped_time;
   long unsigned      now_zipped_time;
   message_loaded [1] = false;
   message_loaded [2] = false;
   
   TimeElements  record_date = RTC_reading;
   TimeElements  reference_date = RTC_reading;
   record_date.Year = RTC_reading.Year - 30;        //*year error correction
   reference_date.Year = RTC_reading.Year - 30;
   // check for events ocurring from now till the end of the scan window
   for (byte x=0; x<number_of_records_to_scan; x++){
      record_date.Month = important_dates_month_array[x];
      record_date.Day = important_dates_day_array[x];      
      record_zipped_time = makeTime(record_date) / seconds_in_a_day;
      now_zipped_time = makeTime(reference_date) / seconds_in_a_day;
      if (record_zipped_time - now_zipped_time <= scan_window) {
        //flag the first two records of the next scan_window
        if (!message_loaded [1]){
          reminder_message_pointer [1] = x;
          message_loaded [1] = true;
        } else {
          if (!message_loaded [2]){
            reminder_message_pointer [2] = x;
            message_loaded [2] = true;
          }
        }
        
        //print record
        //Serial.print (important_dates_string_array[x]);
        //Serial.print (" - ");
        //Serial.print (important_dates_month_array[x]);
        //Serial.print (" - ");
        //Serial.print (important_dates_day_array[x]);
        //Serial.print (" - reminder_message_pointer [1] = ");
        //Serial.println (reminder_message_pointer [1]);
        //Serial.print (" - reminder_message_pointer [2] = ");
        //Serial.println (reminder_message_pointer[2]);

      }  
   }
                                               
}
//===============================
void myMessageManagerfunction() {
//=========rewritten 12.3.2017====

  if (message_manager_next_message <= 1) {
    myMessageSunrisefunction();
    message_manager_timestamp    = millis()+3000; 
  }
  if (message_manager_next_message == 2) {
    myMessageWeekNumberfunction();
    message_manager_timestamp    = millis()+3000;
  }   
  if (message_manager_next_message == 3) {
    myMessageReminderfunction();
    message_manager_timestamp    = millis()+3000;
  }
  if (message_manager_next_message == 4) {
    myMessageVoltageDailyHighfunction();
    message_manager_timestamp    = millis()+6000;
  }
  if (message_manager_next_message == 5) { 
    myMessageInverterRunTimefunction();
    message_manager_timestamp    = millis()+2000;
  }
  if (message_manager_next_message == 6) {
    myMessageUpcomingEventsfunction(1);
    message_manager_timestamp    = millis()+3000;
  }
  if (message_manager_next_message == 7) {
    myMessageUpcomingEventsfunction(2);
    message_manager_timestamp    = millis()+3000;  
  }
  message_manager_next_message++;
  if (message_manager_next_message == 8) {
    message_manager_next_message = 1;   
  }
}





//-------------------------------
void myMessageInverterRunTimefunction(){
//-------------------------------   
  myClearMessageBoardfunction();
  lcd.setCursor (0,1);
  if (inverter_run_time == 0) {
               //"12345678901234567890"
     lcd.print(F("  Inverter Waiting"));
     return;  
  } 
  if (inverter_run_time > 0 && inverter_run_time < 60) {
               //"12345678901234567890"
     lcd.print(F(" Inverter harvested"));
     lcd.setCursor (8,2);
     lcd.print(inverter_run_time);
     lcd.print("s");
     return;  
  }
  if (inverter_run_time >= 60) {
               //"12345678901234567890"
     lcd.print(F(" Inverter harvested"));
     lcd.setCursor (8,2);
     lcd.print(inverter_run_time/60);
     lcd.print("m");
     return;  
  }
}

//-------------------------------
void myMessageReminderfunction(){
//-------------------------------
   myClearMessageBoardfunction();
   lcd.setCursor (0,1);
   //           "12345678901234567890"
   lcd.print (F("   Did you MED-X?"));
   lcd.setCursor (0,2);
   //lcd.print (F("   eggs floss med-x?"));
   
}
//*******************************
void myMessageSunrisefunction() {
//*******************************
  myClearMessageBoardfunction();
  lcd.setCursor (0,1);
  //           "12345678901234567890"
  lcd.print (F("   Sunrise "));
  lcd.print (today_sunrise_hour);
  lcd.print (F(":"));
  if (today_sunrise_minute <= 9){
   lcd.print(F("0"));
  }
  lcd.print (today_sunrise_minute);
  lcd.print (F("am"));
  lcd.setCursor (0,2);
  //           "12345678901234567890"
  lcd.print (F("   Sunset  ")); 
  lcd.print (today_sunset_hour - 12);
  lcd.print (F(":"));
  if (today_sunset_minute <= 9){
   lcd.print(F("0"));
  }
  lcd.print (sunset_minute[solar_week_number-1]);
  lcd.print (F("pm"));
}


  
//----------------------------------------
void myMessageUpcomingEventsfunction(byte n){
//----------------------------------------  
  TimeElements timex = RTC_reading;
  myClearMessageBoardfunction();
  timex.Day = important_dates_day_array[reminder_message_pointer [n]];
  //Serial.print ("reminder_message_pointer [n]");
  //Serial.println (reminder_message_pointer [n]);
  //Serial.print ("timex.Day");
  //Serial.println (timex.Day);
  timex.Month = important_dates_month_array[reminder_message_pointer [n]];
  //Serial.print ("reminder_message_pointer [n]");
  //Serial.println (reminder_message_pointer [n]);
  //Serial.print ("timex.Month");
  //Serial.println (timex.Month);
  if(RTC_reading.Month == 12 && timex.Month == 1) {    //to protect myReturnDayofWeekFromUnixTimestampfunction
                                                        //from end of year rollover
    timex.Year++; 
  }
  if (message_loaded [n]){
    lcd.setCursor (0,1);
    if (important_dates_yob_array[reminder_message_pointer [n]]) {
    lcd.print (important_dates_string_array[reminder_message_pointer [n]]);
    lcd.print ("'s ");
    lcd.print (year() - important_dates_yob_array[reminder_message_pointer [n]]);
    lcd.print(myNumericalSuffixCalculatorfunction(year() - important_dates_yob_array[reminder_message_pointer [n]]));
    if (event_type_to_print_array[reminder_message_pointer [n]] == 1) {
      lcd.print (" B-day");  
    }
    } else {
    lcd.print (important_dates_string_array[reminder_message_pointer [n]]);
    }
    lcd.setCursor (5,2);
    if (timex.Day == RTC_reading.Day || timex.Day == RTC_reading.Day + 1) {      
      if (timex.Day == RTC_reading.Day) {
        //         "12345678901234567890"
        lcd.print ("   today.");  
      }
      if (timex.Day == RTC_reading.Day+1) {
        //         "12345678901234567890"
        lcd.print (" tomorrow");  
      }
    } else {
      lcd.print (myReturnDayofWeekFromUnixTimestampfunction(timex));
      lcd.print (", ");
      lcd.print (month_short_name[(important_dates_month_array[reminder_message_pointer [n]])-1]);
      lcd.print (" ");
      lcd.print (important_dates_day_array[reminder_message_pointer [n]]);
    }  
  }
}
//


//-----------------------------------
void myMessageVoltageDailyHighfunction(){
//-----------------------------------  
   
   myClearMessageBoardfunction();
   lcd.setCursor (0,1);
   //         "12345678901234567890"
   myPrintVoltagetoLCDfunction(2,1,voltage_daily_max);
   lcd.print (F(" @"));
   myPrintTimetoLCDfunction(todays_high_voltage_timestamp,12,1,false);
   myPrintVoltagetoLCDfunction(2,2,voltage_daily_min);
   lcd.print (F(" @")); myPrintTimetoLCDfunction(todays_low_voltage_timestamp,12,2,false);
   
}


//


//-----------------------------------
void myMessageVoltageDailyLowfunction(){
//-----------------------------------  
   //Serial.print (" ");
   //Serial.print (message_manager_current_message);
   //Serial.print ("Todays lowest voltage was ");
   //Serial.print (voltage_daily_min);
   //Serial.print (" volts at ");
   //myPrintSerialTimestampfunction (todays_low_voltage_timestamp);
   myClearMessageBoardfunction();
   lcd.setCursor (0,1);
   //         "12345678901234567890"
   lcd.print (F("  Today's low was")); myPrintVoltagetoLCDfunction(2,2,voltage_daily_min);
   lcd.print (F(" at")); myPrintTimetoLCDfunction(todays_low_voltage_timestamp,12,2,false);
}
//----------------------------------
void myMessageWeekNumberfunction() {
//----------------------------------

   myClearMessageBoardfunction();
   lcd.setCursor (0,1);
   //         "12345678901234567890"
   lcd.print (F("  Solar Week"));
   lcd.setCursor (0,2);
   lcd.print (F("        Number "));
   lcd.print (solar_week_number);
   
}

//*************************************************
char* myNumericalSuffixCalculatorfunction(byte x) {
//*************************************************  
  const char* suffix = "th";
  // first, any number over 20 should be reduced by tens until it is 10 or less
  if (x >= 20) {
    while (x > 10) {
      x = x -10;
    }
  }
  // now, x should be between 1 and 20
  if (x == 1) { suffix = "st"; }
  if (x == 2) { suffix = "nd"; }
  if (x == 3) { suffix = "rd"; }
  return suffix;
}


//---------------------------------------------
void myPrintDatetoLCDfunction(byte x, byte y) {
//--------------------------------------------- 
   lcd.setCursor (x,y);
   lcd.print(myReturnDayofWeekfunction(weekday() - 1));
   lcd.print (", ");
   lcd.print (month_short_name[RTC_reading.Month-1]);    
   lcd.print(" ");
   lcd.print((RTC_reading.Day));
   lcd.print(" ");  //this space to clear last digit when month rolls over (31 to 1) 
}
//-------------------------------------
void myPrintLDRresultsToLCDfunction() {
//-------------------------------------
   lcd.setCursor (11,0);
   //Serial.print("Summer: "); Serial.print(analogRead(the_pin_to_the_LDR2_circuit)); Serial.print(". ");
   if (digitalRead(the_pin_to_the_LDR2_circuit)) {
      lcd.print("S");
   } else {
      lcd.print(" ");
   }     
   if (digitalRead(the_pin_to_the_LDR_circuit)) { 
      lcd.print("A");
   } else {
      lcd.print(" ");
   }     
}

//***********************************************************
void myPrintSerialTimestampfunction (){
//***********************************************************
   Serial.println();
   if (hour() <= 9) {
      Serial.print("0");
   }
   Serial.print(hour());
   Serial.print(":");
   if (minute() <= 9) {
      Serial.print("0");
   }
   Serial.print(minute());
   Serial.print(":");
   if (second() <= 9) {
      Serial.print("0");
   }
   Serial.print(second());
   Serial.print(" "); 
}

//---------------------------------------------   
void myPrintTimetoLCDfunction(TimeElements timestamp, byte x, byte y, boolean right_justify) {
//---------------------------------------------  
                                      // x and y are the LCD coordinants where the time is to be printed
                                      // x can be 0 to 19
                                      // y can be 0 to 3
   lcd.setCursor (x,y);                                      
   if ((timestamp.Hour) >= 12) {      
      am_pm = 12;
   } else {                           // set am_pm variable
      am_pm = 0;
   } 
   if ((timestamp.Hour) == 12 || (timestamp.Hour) == 0) {
      lcd.print ("12");
   } else {
      if ((timestamp.Hour)-am_pm < 10 && right_justify) {             
         lcd.print (" ");
      }
      lcd.print ((timestamp.Hour)-(am_pm));
   }   
   lcd.print(":");
   if ((timestamp.Minute) < 10 ) {  
      lcd.print ("0");
   }  
   lcd.print (timestamp.Minute);
   if ((timestamp.Hour) >= 12) {
      lcd.print("pm");
   } else {                          
      lcd.print("am");
   }                                      
}
//----------------------------------------------------
void myPrintVoltagetoLCDfunction(int x,int y,float v){
//----------------------------------------------------
   lcd.setCursor (x,y);
   lcd.print (v);
   lcd.print ("v");      
}



//**************************************************************
void myReadPotentiometerAndAdjustWorkbenchTrackLightsfunction(){
//**************************************************************
  const float correction = -0.40;  // I arrived at this with direct measurement.  IDK why.  PN junction?
  const float maximum_voltage = 12.00 + correction; // do not allow the bulbs more voltage than this number.
  
  float scaling_ratio = 1;
  if (stable_voltage > maximum_voltage) {
    //Serial.print ("stable voltage: ");
    //Serial.print (stable_voltage);
    scaling_ratio = maximum_voltage / stable_voltage;  
  }
  
  int potentiometer_reading = 0;
  const byte stray_distance = 3; // increase to prevent flicker from potentiometer attenuation, 
                                 // decrease for smoother transition 
  for (byte x = 0; x <=9; x++){
     potentiometer_reading = potentiometer_reading + (analogRead (potentiometer_input_pin));
  }
  potentiometer_reading = potentiometer_reading / 40;
  if (potentiometer_reading <= dimmer_reference_number - stray_distance) {
    dimmer_reference_number = potentiometer_reading; 
  }
  if (potentiometer_reading >= dimmer_reference_number + stray_distance) {
    dimmer_reference_number = potentiometer_reading; 
  }
  analogWrite (workbench_lighting_MOSFET_signal_pin, (255 - dimmer_reference_number) * scaling_ratio);  
  //diagnostic
  //Serial.println ();
  //Serial.print ("potentiometer_reading ");
  //Serial.print (potentiometer_reading);
  //Serial.print (". scaling_ratio = ");
  //Serial.print (scaling_ratio);
  //Serial.print ("I am writing number ");
  //Serial.print ((255 - dimmer_reference_number) * scaling_ratio);
  }


//***************************************
char* myReturnDayofWeekfunction (byte x){
//***************************************  
char* weekday_array[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
return weekday_array[x];  
}
//*********************************************************************************
char* myReturnDayofWeekFromUnixTimestampfunction (TimeElements unzipped_time){
//*********************************************************************************
  long unsigned seconds_in_a_day = 86400;
  long unsigned zipped_time      = makeTime(unzipped_time) / seconds_in_a_day ;
  int x = 0;
  //Serial.print (" - "); 
  //Serial.print (zipped_time);
  x = (zipped_time - (zipped_time / 7) * 7);  //detecting remainder with my integer math trick
  //align week number with my array, because 4 <> Monday
  x = x - 3;
  if (x < 0){
    x=x+7;
  }
  //Serial.print (x);
  //Serial.print (" - ");
  //Serial.print(myReturnDayofWeekfunction(x));
  return myReturnDayofWeekfunction(x);  
}

//======================================
void mySetSunriseSunsetTimesfunction() {
//==================12.4.2017===========

  
  today_sunrise_hour = sunrise_hour[solar_week_number-1]-1+daylight_savings_time;
  today_sunrise_minute = sunrise_minute[solar_week_number-1];
  today_sunset_hour = sunset_hour[solar_week_number-1]-1+daylight_savings_time;
  today_sunset_minute = sunset_minute[solar_week_number-1];
}


//-----------------------------------------
void myVoltagePrintingAndRecordingfunction() {
//-----------------------------------------  
   myPrintVoltagetoLCDfunction(14,0,stable_voltage);                      
   //Serial.print (stable_voltage); Serial.print ("V "); 
   if (stable_voltage > voltage_daily_max) { 
    voltage_daily_max = stable_voltage;
    todays_high_voltage_timestamp = RTC_reading;
   }
   if (stable_voltage < voltage_daily_min) {
     voltage_daily_min = stable_voltage; 
     todays_low_voltage_timestamp = RTC_reading;
   }
}

//===================================
void myVoltageCalculationfunction() {
//===================================  
  const byte    range_of_voltage_trend                    = 100;
  voltage = analogRead(the_analog_pin_to_the_voltage_divider) * voltage_measurement_divided_by_voltage_divider_measurement * reference_voltage / 1024;
                                      // convert analog reading to the useable voltage number
  //voltage = voltage + (USB_mode * 0.20);  // this line to correct voltage (add .2) when usb plugged in 
  //stabilizer alpha

  if (voltage > stable_voltage){  voltage_trend ++;  }
  if (voltage < stable_voltage){  voltage_trend --;  }
  if (voltage_trend >= range_of_voltage_trend){
     stable_voltage = stable_voltage + 0.01;
     voltage_trend = 0;
  }
  if (voltage_trend <= (0 - range_of_voltage_trend)){
     stable_voltage = stable_voltage - 0.01;
     voltage_trend = 0;
  }
    
  

  //end of stabilizer alpha 
}
