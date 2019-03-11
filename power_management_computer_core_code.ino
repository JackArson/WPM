
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
byte          battery_charger_signal_pin                              = 4; //green wire upper cable  
byte          inverter_signal_pin                                  = 5; //blue wire upper cable
byte          workbench_lighting_MOSFET_signal_pin                 = 6; //purple wire upper cable* too fast PWM
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
