#include <Time.h>
#include <TimeLib.h>
#include <LiquidCrystal_I2C.h>
#include <DS1307RTC.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  
class MyTimer
{
  public:
  MyTimer();
  byte counter;    // accessable from outside via (MyTimer).counter 
  void Update       ();
  void Set          (byte x); 
};

MyTimer::MyTimer()
{}

void MyTimer::Update()
{
    lcd.setCursor(8,0);
    if(counter)
    {
        if(counter <= 9)
        {
            lcd.print(" "); // clear the first space if a single digit
        }
        lcd.print(counter);                 // print the counter
        counter--;                          // counter decrements here
    }
    else
    {
        lcd.print("  ");             // no counter, so clear the board
    }
}

void MyTimer::Set(byte x)
{
    counter = x;
}


  
////jon lynch apr 15 1968
//                          
//                          // Event type 1) B-day 2) Aniversary                                   
//                                     //           1                      2                      3                      4                     5                       6                       7                      8                     9                      10                     11                     12                     13                     14                     15                     16                     17                      18                     19                     20                      21                     22
byte  number_of_records_to_scan = 22; // "12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890",,"12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890"     
const char* important_dates_string_array[] = {" Kathy"               ,"Jack(cat)"           ,"Katrina"             ,"  Alan"              ,"  Jack"              ," Joshua"              ,"  Dad"               ,"Christopher"         ,"  Mena"              ," Jacob"              ,"Elizabeth"           ,"Aunt Julie"          ," Dentist 8:00AM"     ,"Paul and Mena"       ,"Empty"               ," Cersei"             ," Mom and Dad"        ,"  Paul"              ,"  Mom"               ," Erik"               ,"Mathew"              ,"      Christmas"};
const byte  important_dates_month_array[]  = {1                      ,2                     ,2                     ,2                     ,2                     ,3                      ,3                     ,4                     ,4                     ,4                     ,4                     ,5                     ,10                    ,6                     ,0                     ,6                     ,9                     ,9                     ,10                    ,12                    ,12                    ,12};
const byte  important_dates_day_array[]    = {7                      ,6                     ,9                     ,10                    ,27                    ,23                     ,30                    ,3                     ,24                    ,26                    ,29                    ,31                    ,10                    ,29                    ,0                     ,15                    ,7                     ,24                    ,23                    ,4                     ,17                    ,25};
const int   important_dates_yob_array[]    = {1967                   ,2011                  ,2000                  ,1993                  ,2012                  ,1993                   ,1943                  ,1990                  ,1972                  ,2005                  ,2000                  ,1942                  ,0                     ,1991                  ,0                     ,2015                  ,1963                  ,1969                  ,1943                  ,1999                  ,1995                  ,0};   
const byte  event_type_to_print_array[]    = {1                      ,1                     ,1                     ,1                     ,1                     ,1                      ,1                     ,1                     ,1                     ,1                     ,1                     ,1                     ,0                     ,2                     ,0                     ,1                     ,2                     ,1                     ,1                     ,1                     ,1                     ,0};
                                     

const char* month_short_name[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
//               WEEK      -1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53

byte sunrise_hour[53]   = { 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8 }; 
byte sunrise_minute[53] = {47,48,46,42,37,31,23,14, 4,54,43,32,21,10,59,48,38,29,20,13, 7, 3, 0,58,58, 0, 3, 7,12,18,24,30,37,43,50,56, 3, 9,16,23,29,36,44,51,59, 7,15,23,30,36,41,45,47 };

byte sunset_hour[53]    = {18,18,18,18,18,18,18,19,19,19,19,19,19,19,19,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,19,19,19,19,19,18,18,18,18,18,18,18,18,18,18,18,18,18 }; 
byte sunset_minute[53]  = {13,19,26,34,43,51,59, 7,15,23,30,37,44,51,58, 5,12,19,26,33,39,45,50,54,57,58,58,56,53,48,42,34,26,16,06,55,44,32,21, 9,58,48,38,29,21,14, 8, 4, 2, 2, 4, 7,13 };

boolean daylight_savings_time = true; // was false from fall to spring, now trying true for 
                                       // spring to fall

int  inverter_run_time  = 0;
byte power_manager_mode = 0; // 0 = night time, or before first charge
                             // 1 = balance mode first 14v charge complete, gomode 2 if power drops below 12.6 60s
                         //                                  gomode 3 if power rises above 13.5 60s
                         // 2 = aux charger trying to get us to 12.6                                  
                         // 3 = inverter cycling
float       reference_voltage = 5; // using the default operating voltage reference.  Measured between 5V pin and Ground
const float voltage_measurement_divided_by_voltage_divider_measurement = 4.31;  
                                      // Paul's pro tip:  Unplug the USB cable when tuning 
                                      //                  because the USB changes it.
                                      // Adjust displayed voltage by fine tuning this number             
                                      // the full voltage to be measured / the reduced voltage at voltage divider. 
                                      // My resistors:  positive terminal to divider intersection 150K 
                                      // from divider intersection to negative terminal 47k
                                      // My voltage measurement from pos to ground = 14
                                      // My measurement from ground to voltage divider intersection = 3.22
                                      // 12.28 / 2.90 = 4.355
float         stable_voltage = 12.5;  // the usable number to be displayed on LCD screen                     
float         voltage = 0;            
float         voltage_daily_max = 0;
float         voltage_daily_min = 15; // initial setting, a number higher than expected nominal voltage
int           voltage_trend                             = 0;
byte          am_pm = 0;

tmElements_t  RTC_reading;
tmElements_t  todays_low_voltage_timestamp;
tmElements_t  todays_high_voltage_timestamp;
byte          solar_week_number = 1;



                                      //  Mega wiring
                                      //  black eight wire cable (old ethernet cable)
                                      //  solid brown to digital #2 
                                      //  digital #3 to solid orange to (orange lead of 8 wire....
                                      //  digital #4 to solid green to (white lead of 5 wire brown tstat cable in local box) to (fuse e through h box -add wire color here-)
                                      //  digital #5 to solid blue to (yellow lead of 8 wire brown t-stat cable....
                                      //  digital #6 purple to striped blue to yellow lead of 5 wire tstat cable
                                      //  digital #8 green to striped green of black ethernet cable to light green wire in the brown 8 wire tstat cable
                                      //             From light green wire in metal box above back door to green striped wire in the gray 8 wire ethernet cable
                                      //             From handy box with light controls: striped green to white in the black 4 wire USB cable.
                                      //                          
                                      //  digital #9 orange to striped orange of black ethernet cable to dark green wire in the brown 5 wire tstat cable
                                      //             From dark green wire in metal box above back door to green solid in the gray 8 wire ethernet cable -
                                      //             From handy box with light controls: solid green to the solid green in the black 4 wire USB cable.
                                      //              
                                      // analog #1 yellow to striped brown

                                      // PIN ASSIGNMENT HERE
                                      
                                      // DIGITAL
                                      // Pins 0 and 1 are reserved for the USB
byte          the_pin_to_the_LDR_circuit                           = 2; //brown   wire lower cable              
byte          the_pin_to_the_LDR2_circuit                          = 3; //orange wire lower cable
byte          battery_charger_signal_pin                           = 11; //green wire upper cable  
byte          inverter_signal_pin                                  = 5; //blue wire upper cable
byte          workbench_lighting_MOSFET_signal_pin                 = 10; //purple wire upper cable* too fast PWM
byte          stage_one_inverter_relay                             = 8; //A
byte          stage_two_inverter_relay                             = 9; //S                                   
                                      // ANALOG
byte          the_analog_pin_to_the_voltage_divider                = A0; //green  wire lower cable
byte          potentiometer_input_pin                              = A1; //yellow wire


long unsigned a500ms_timestamp   = millis();
long unsigned a1000ms_timestamp   = millis();
long unsigned message_manager_timestamp    = millis();

//initializations: do not adjust
byte          today_sunrise_hour = 0;
byte          today_sunset_hour = 0;
byte          today_sunrise_minute = 0;
byte          today_sunset_minute = 0;
byte          message_manager_next_message = 1;
// power manager initializations
//new
byte          machine_state = 4;  //initiate balanced state
//old
byte          inverter_warm_up_timer = 0; //seconds
int           reusable_countdown_variable = 0;
int           balance_falling_countdown = 0;
int           balance_rising_countdown = 0;

byte          hour_to_turn_off_backlight = 21;
byte          hour_to_turn_on_backlight = 4;
boolean       LDR_data = false;
boolean       LDR2_data = false;
boolean       DST = true;
boolean       message_loaded [3]  = {false,false,false};
byte reminder_message_pointer [3] = {false,false,false};
byte dimmer_reference_number = 0;

const unsigned long seconds_in_a_week = 604800;  

//initialize my objects
MyTimer myTimer;
//Flasher led1(1,100,100);
//                                  
// 
void myPrintStatefunction(char* text);
void myBacklightfunction();
byte myCalculateWeekNumberfunction(TimeElements sixpartdate);
void myClearMessageBoardfunction();
boolean myIsItDaylightfunction();
boolean myIsItDeltaTimePastDawnfunction();
void myLoadUpcomingEventsfunction();
void myMessageManagerfunction();
void myMessageInverterRunTimefunction();
void myMessageReminderfunction();
void myMessageSunrisefunction();
void myMessageUpcomingEventsfunction(byte n);
void myMessageVoltageDailyHighfunction();
void myMessageVoltageDailyLowfunction();
void myMessageWeekNumberfunction();
char* myNumericalSuffixCalculatorfunction(byte x);
void myPrintDatetoLCDfunction(byte x, byte y);
void myPrintLDRresultsToLCDfunction();
void myPrintSerialTimestampfunction ();
void myPrintTimetoLCDfunction(TimeElements timestamp, byte x, byte y, boolean right_justify);
void myPrintVoltagetoLCDfunction(int x,int y,float v);
void myReadPotentiometerAndAdjustWorkbenchTrackLightsfunction();
char* myReturnDayofWeekfunction (byte x);
char* myReturnDayofWeekFromUnixTimestampfunction (TimeElements unzipped_time);
void mySetSunriseSunsetTimesfunction();
void myVoltagePrintingAndRecordingfunction();
void myVoltageCalculationfunction();
void myStateMachineInitSleepStatefunction();
void myStateMachineSleepStatefunction();
void myStateMachineInitWakeStatefunction();
void myStateMachineWakeStatefunction();
void myStateMachineInitBalancedStatefunction();
void myStateMachineBalancedStatefunction();
void myStateMachineInitWarmUpInverterStatefunction();
void myStateMachineWarmUpInverterStatefunction();
void myStateMachineInitStageOneInverterStatefunction();
void myStateMachineStageOneInverterStatefunction();
void myStateMachineInitStageTwoInverterStatefunction();
void myStateMachineStageTwoInverterStatefunction();
void myStateMachineInitDaytimeChargingfunction();
void myStateMachineDaytimeChargingfunction();
void myStateMachineInitInverterCooldownfunction();
void myStateMachineInverterCooldownfunction();
// SETUP 
void setup(){
   Serial.begin(250000);                // start the serial monitor
   Wire.begin();                      // start the Wire library
   lcd.begin(20, 4);                  // start the LiquidCrystal_I2C library
  

   // This code gets clock information from the external RTC module.  
   if (RTC.read(RTC_reading)){
     // This code sets the time and week number.
     setTime (RTC_reading.Hour,RTC_reading.Minute,RTC_reading.Second,RTC_reading.Day,RTC_reading.Month,RTC_reading.Year-30); // -30 years correction WTF?
     solar_week_number = myCalculateWeekNumberfunction(RTC_reading);
     myLoadUpcomingEventsfunction();
     mySetSunriseSunsetTimesfunction();
   } else {
     // This code handles errors if the time cannot be found.
     if (RTC.chipPresent()) {
         Serial.println();
         Serial.print(F("Time not set.  Possible RTC clock battery issue. Battery is LIR2032.  Battery pin on RTC clock usually reads 2.4v to GND when system is powered up.  Battery usually reads < 2.27v when detached."));
      } else {
         Serial.println();
         Serial.print(F("I can't find the clock through the I2C connection.  Investigate wiring A4,A5,5V,GND to RTC module."));
      } 
   }
   pinMode (inverter_signal_pin, OUTPUT);
   pinMode (battery_charger_signal_pin, OUTPUT);
   pinMode (the_analog_pin_to_the_voltage_divider, INPUT);
   pinMode (the_pin_to_the_LDR_circuit, INPUT);
   pinMode (the_pin_to_the_LDR2_circuit, INPUT);
   pinMode (workbench_lighting_MOSFET_signal_pin, OUTPUT);
   pinMode (stage_one_inverter_relay, OUTPUT);
   pinMode (stage_two_inverter_relay, OUTPUT);

//   
//
//   
//   
}
//
//                                      // MAIN LOOP  
void loop(){
//  //Run these functions as fast as possible
  myReadPotentiometerAndAdjustWorkbenchTrackLightsfunction();
  myVoltageCalculationfunction();
//
  //This code runs approximately every 1000ms
  if ((long unsigned)(millis() - a1000ms_timestamp) >= 1000){  a1000ms_timestamp = millis();
    //digitalWrite (battery_charger_signal_pin, LOW);
    //state_machine states
    //0 initiate sleep state
    //1 sleep state (the battery charger is always on
    //2 initiate wake state
    //3 wake state
    //4 initiate balanced state
    //5 balanced state
    //6 initiate inverter warm up state
    //7 inverter warm up state 
    //8 initiate stage one inverter state
    //9 stage one inverter state
    //10 initiate stage two inverter state
    //11 stage two inverter state
    //12 initiate daytime charging
    //13 daytime charging
    //14 initiate inverter cool down
    //15 inverter cool down 

    switch (machine_state) {
      case 0:
        myStateMachineInitSleepStatefunction();
        break;
      case 1:
        myStateMachineSleepStatefunction();
        break;
      case 2:
        myStateMachineInitWakeStatefunction();
        break;
      case 3:
        myStateMachineWakeStatefunction();
        break;
      case 4:
        myStateMachineInitBalancedStatefunction();
        break;
      case 5:
        myStateMachineBalancedStatefunction();
        break;
      case 6:
        myStateMachineInitWarmUpInverterStatefunction();
        break;
      case 7:
        myStateMachineWarmUpInverterStatefunction();
        break;
      case 8:
        myStateMachineInitStageOneInverterStatefunction();
        break;
      case 9:
        myStateMachineStageOneInverterStatefunction();
        break;
      case 10:
        myStateMachineInitStageTwoInverterStatefunction();
        break;
      case 11:
        myStateMachineStageTwoInverterStatefunction();
        break;
      case 12:
        myStateMachineInitDaytimeChargingfunction();
        break;
      case 13:
        myStateMachineDaytimeChargingfunction();
        break;
      case 14:
        myStateMachineInitInverterCooldownfunction();
        break;
      case 15:
        myStateMachineInverterCooldownfunction();
        break;    
        }
////    
//    if (myIsItDaylightfunction() == true) {
//      switch (power_manager_mode) {
//      case 0:
//        myAuxPowerfunction(13.20,30); 
//        break;
//      case 1:
//        myBalancePowerBetweenChargerAndInverterfunction(); 
//        break;  
//      case 2:
//        myAuxPowerfunction(13.20,5);
//        break;
//      case 3:
//        myStage01Inverterfunction();
//        break;
//      case 4:
//        myStage02Inverterfunction();
//        break;
//      }
//    } else {    //if it is not daytime
//      mySwitchPowerManagerModesfunction(0);
//    }

        
    myTimer.Update();
    
    
    Serial.print (" counter = "); 
    Serial.print (myTimer.counter);
    myBacklightfunction();
    myPrintSerialTimestampfunction();
    myPrintTimetoLCDfunction((RTC_reading),13, 3,true);
    myPrintDatetoLCDfunction(0, 3);
    myPrintLDRresultsToLCDfunction();
     
    // This code runs once or twice at 2am
    if (hour() == 0 && minute() == 0 && second() <= 1 ) {
      power_manager_mode = 0; //r
      inverter_run_time  = 0;
      voltage_daily_max = stable_voltage; 
      todays_high_voltage_timestamp = RTC_reading;
      voltage_daily_min = stable_voltage;
      todays_low_voltage_timestamp = RTC_reading;
      solar_week_number = myCalculateWeekNumberfunction(RTC_reading); //to read duskdawn data tables
      myLoadUpcomingEventsfunction();
      mySetSunriseSunsetTimesfunction();  
    }       
  }
//
//  //This code runs every 500ms
  if ((long unsigned)(millis() - a500ms_timestamp) >= 500){  //millis rollover safe    
    a500ms_timestamp = millis();
    if (RTC.read(RTC_reading)){
      setTime(RTC_reading.Hour,RTC_reading.Minute,RTC_reading.Second,RTC_reading.Day,RTC_reading.Month,RTC_reading.Year-30);
      //sync RTC with system clock every 500ms.
    }  
    myVoltagePrintingAndRecordingfunction();
  }

  //This code runs if MessageManager sets the message_manager_timestamp 
  if ((long unsigned)(message_manager_timestamp) <= millis()){ 
    myMessageManagerfunction(); 
  }
  
  
}

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


//state_machine states
//0 initiate sleep state
//1 sleep state (the battery charger is always on)
//2 initiate wake state
//3 wake state
//4 initiate balanced state
//5 balanced state
//6 initiate inverter warm up state
//7 inverter warm up state
//8 initiate stage one inverter state
//9 stage one inverter state
//10 initiate stage two inverter state
//11 stage two inverter state
//12 initiate daytime charging
//13 daytime charging
//14 initiate inverter cool down
//15 inverter cool down
//===========================================
void myStateMachineInitSleepStatefunction() {               //state 0
  //================================5.8.2018===
  myPrintStatefunction("Sleeping");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, HIGH);  //battery charger on
  digitalWrite (inverter_signal_pin, LOW);         //inverter off
  digitalWrite(stage_one_inverter_relay, LOW);    // relay one off
  digitalWrite(stage_two_inverter_relay, LOW);    // relay two off
  machine_state = 1; //go to sleep state
  return;
}

//=======================================
void myStateMachineSleepStatefunction() {                   //state 1
  //============================5.8.2018===
  if (myIsItDaylightfunction() == true) {        //switch to initiate wake state if light
    machine_state = 2; //initiate wake state
    return;
  }

  return;
}

//==========================================
void myStateMachineInitWakeStatefunction() {                //state 2
  //==============================5.8.2018====
  myPrintStatefunction("Waking  ");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, LOW);   // battery charger off
  digitalWrite (inverter_signal_pin, LOW);          // inverter off
  digitalWrite(stage_one_inverter_relay, LOW);      // relay one off
  digitalWrite(stage_two_inverter_relay, LOW);      // relay two off
  machine_state = 3;
}

//======================================
void myStateMachineWakeStatefunction() {                    //state 3
  //===========================5.8.2018===
  //check to see if it is time to go back to sleep
  //check to see how long it is past dawn.
  //if it is delta_t, or 15 minutes, past dawn switch to state 4 (init balanced)
  //The wake up delay ensures that the the inverter does not burn off the fresh battery charger energy
  //Time of day used here so the machine can begin inverting quickly if reset mid-day
  //I'm planning on starting in balanced mode however, so should never come up
  if (myIsItDaylightfunction() == false) {        //return to initiate sleep mode if dark, unlikely
    machine_state = 0; //initiate sleep state
    return;
  }
  if (myIsItDeltaTimePastDawnfunction() == true) {
    machine_state = 4;  //initiate balanced
    return;
  }
}

//==============================================
void myStateMachineInitBalancedStatefunction() {            //state 4
  //===================================5.8.2018===


  myPrintStatefunction("Balanced");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, LOW);  //battery charger off
  digitalWrite (inverter_signal_pin, LOW);         //inverter off
  digitalWrite(stage_one_inverter_relay, LOW);    // relay one off
  digitalWrite(stage_two_inverter_relay, LOW);    // relay two off
  //
  machine_state = 5;                               // balanced initialization complete
  return;
}

//==========================================
void myStateMachineBalancedStatefunction() {                //state 5
  //===============================5.8.2018===

  const float voltage_to_start_inverter = 13.80;
  const float voltage_to_start_daytime_charging = 12.60;

  if (myIsItDaylightfunction() == false) {        //switch to initiate sleep mode if dark
    machine_state = 0; //initiate sleep state
    return;
  }
  if (stable_voltage >= voltage_to_start_inverter) {
    machine_state = 6;     //init warm up inverter
    return;
  }
  if (stable_voltage <= voltage_to_start_daytime_charging) {
    machine_state = 12;    //init daytime charging
    return;
  }


  return;
}

//======================================================
void myStateMachineInitWarmUpInverterStatefunction() {    //state 6
  //===========================================5.8.2018===
  const byte seconds_to_warm_up = 4;

  myPrintStatefunction("Warm Up ");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, LOW);  //battery charger off
  digitalWrite (inverter_signal_pin, HIGH);         //inverter on
  digitalWrite(stage_one_inverter_relay, LOW);    // relay one off
  digitalWrite(stage_two_inverter_relay, LOW);    // relay two off
  myTimer.Set(seconds_to_warm_up);
  machine_state = 7;
}



//==================================================
void myStateMachineWarmUpInverterStatefunction() {        //state 7
  //=======================================5.8.2018===

  if (myTimer.counter) {    //warming up during countdown
    return;
  }
  machine_state = 8;

}
//======================================================
void myStateMachineInitStageOneInverterStatefunction() {    //state 8
  //===========================================5.8.2018===

  const byte  stage_two_switching_delay = 15;  //seconds.  this prevents stage two from engaging too soon

  myPrintStatefunction("Invert1 ");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, LOW);  //battery charger off
  digitalWrite (inverter_signal_pin, HIGH);         //inverter on
  digitalWrite(stage_one_inverter_relay, HIGH);    // relay one on
  digitalWrite(stage_two_inverter_relay, LOW);    // relay two off
  myTimer.Set(stage_two_switching_delay);
  machine_state = 9;
}

//==================================================
void myStateMachineStageOneInverterStatefunction() {        //state 9
  //=======================================5.8.2018===

  const float voltage_to_turn_inverter_off = 12.55;
  const float voltage_to_switch_to_stage_two = 13.80;

  inverter_run_time++;

  if (myIsItDaylightfunction() == false) {        //switch to initiate sleep mode if dark
    machine_state = 0; //initiate sleep state -   //might be helpful for a charger stuck in "on" state
    return;
  }

  if (stable_voltage >= voltage_to_switch_to_stage_two && !myTimer.counter) {
    machine_state = 10;  //initiate stage two inverter
    return;
  }

  if (stable_voltage <= voltage_to_turn_inverter_off) {
    machine_state = 14; //initiate inverter cooldown state, so a burst of solar energy does not short
    //cycle the inverter
    return;
  }

}
//======================================================
void myStateMachineInitStageTwoInverterStatefunction() {    //state 10
  //==============================5.8.2018================
  myPrintStatefunction("Invert2 ");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, LOW);  //battery charger off
  digitalWrite (inverter_signal_pin, HIGH);         //inverter on
  digitalWrite(stage_one_inverter_relay, HIGH);    // relay one on
  digitalWrite(stage_two_inverter_relay, HIGH);    // relay two on
  machine_state = 11;
}

//==================================================
void myStateMachineStageTwoInverterStatefunction() {        //state 11
  //===========================5.8.2018===============

  const float voltage_to_drop_back_to_stage_one = 12.70;

  if (myIsItDaylightfunction() == false) {        //switch to initiate sleep mode if dark
    machine_state = 0; //initiate sleep state -   //might be helpful for a charger stuck in "on" state
    return;
  }

  inverter_run_time++;

  if (stable_voltage <= voltage_to_drop_back_to_stage_one) {
    machine_state = 8;
    return;
  }

}

//================================================
void myStateMachineInitDaytimeChargingfunction() {    //state 12
  //====================================5.11.2018===
  myPrintStatefunction("Charging");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, HIGH);  //battery charger on
  digitalWrite (inverter_signal_pin, LOW);         //inverter off
  digitalWrite(stage_one_inverter_relay, LOW);    // relay one off
  digitalWrite(stage_two_inverter_relay, LOW);    // relay two off
  machine_state = 13;
}
//============================================
void myStateMachineDaytimeChargingfunction() {  //state 13
  //================================5.11.2018===

  const float voltage_to_switch_off_charger = 13.40;

  if (stable_voltage >= voltage_to_switch_off_charger) {
    machine_state = 4;  //switch to initbalanced
    return;
  }
}

//================================================
void myStateMachineInitInverterCooldownfunction() {    //state 14
  //====================================5.11.2018===

  byte inverter_cooldown_time = 30;  //seconds

  myPrintStatefunction("Cooldown");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, LOW);  //battery charger off
  digitalWrite (inverter_signal_pin, LOW);         //inverter off
  digitalWrite(stage_one_inverter_relay, LOW);    // relay one off
  digitalWrite(stage_two_inverter_relay, LOW);    // relay two off
  myTimer.Set(inverter_cooldown_time);
  machine_state = 15;
}

//================================================
void myStateMachineInverterCooldownfunction() {    //state 15
  //====================================5.11.2018===

  if (!myTimer.counter) {
    machine_state = 4;  //init balanced
  }
}
