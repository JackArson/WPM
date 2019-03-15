#include <Time.h>
#include <TimeLib.h>
#include <LiquidCrystal_I2C.h>
#include <DS1307RTC.h> //object pre-initialized as 'RTC' in header file 
#include <Wire.h>

LiquidCrystal_I2C  liquidcrystali2c(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  
tmElements_t       gRTC_reading;
tmElements_t       gLast_RTC_reading;
//==================================================================================================================

class MyStateMachine
{
public:
    enum State
    {
        STATE_INIT_SLEEP, //this is always selected at 2am from the main loop
        STATE_SLEEP, //1
        STATE_INIT_WAKE, //2
        STATE_WAKE, //3
        STATE_INIT_BALANCED, //4
        STATE_BALANCED, //5
        STATE_INIT_INVERTER_WARM_UP, //6
        STATE_INVERTER_WARM_UP, //7
        STATE_INIT_INVERTER_STAGE_ONE, //8
        STATE_INVERTER_STAGE_ONE, //9
        STATE_INIT_INVERTER_STAGE_TWO, //10
        STATE_INVERTER_STAGE_TWO, //11
        STATE_INIT_DAY_CHARGE, //12
        STATE_DAY_CHARGE, //13
        STATE_INIT_INVERTER_COOL_DOWN, //14
        STATE_INVERTER_COOL_DOWN, //15
        STATE_ERROR,
        MAX_STATE
    };
private: //variables
    State mState {STATE_INIT_BALANCED}; //start balanced
public:  //methods
    State getState();
    void  setState(State);
}mystatemachine;

MyStateMachine::State MyStateMachine::getState()
{
    return mState;
}

void  MyStateMachine::setState(State state)
{
    mState = state;
}

//==================================================================================================================

class MySerial
{
private: //variables
    //choose the serial output 
    bool  mPrintStateMachineChanges {true};
    bool  mPrintTimestamp           {true};
public:
    void sprint(const char *string_ptr);
    void sprint(const byte numeral);
    void printLinefeed();
    void printStatus(); //runs every 1000 milliseconds. Called by main()
    void printTimestamp();
    void testClock();
private: //methods
    
    //void print
}myserial;

void MySerial::sprint(const char *string_ptr)
{
    Serial.print(string_ptr);   
}

void MySerial::sprint(const byte numeral)
{
    Serial.print(numeral);   
}

void MySerial::printLinefeed()
{
    Serial.println();
}

void MySerial::printStatus()
{
    if (mPrintTimestamp)
    {
        printTimestamp();
    }
    Serial.println();
}

void MySerial::printTimestamp()
{    
    if (gRTC_reading.Hour <= 9)
    {
        Serial.print("0");
    }
    Serial.print(gRTC_reading.Hour);
    Serial.print(":");
    if (gRTC_reading.Minute <= 9)
    {
        Serial.print("0");
    }
    Serial.print(gRTC_reading.Minute);
    Serial.print(":");
    if (gRTC_reading.Second <= 9)
    {
        Serial.print("0");
   }
   Serial.print(gRTC_reading.Second);
   Serial.print("  ");
}

void MySerial::testClock()
{
    if (RTC.read(gRTC_reading))
    {
        //setTime to be removed after all references deleted     
        setTime (gRTC_reading.Hour,gRTC_reading.Minute,gRTC_reading.Second,gRTC_reading.Day,gRTC_reading.Month,gRTC_reading.Year-30); // -30 years correction WTF?
    }
    else
    {
        //if the time cannot be found.
        if (RTC.chipPresent())
        {
            Serial.println(F("Time not set.  Possible RTC clock battery issue. Battery is LIR2032."));
        }
        else
        {
            Serial.println(F("I can't find the clock through the I2C connection, check wiring."));
        } 
    }
}

struct Coordinant
{
    byte x;
    byte y;
};

//=============================================================================================================
//  01234567890123456789 20 x 4        LCD Display
//0|Charging15 SA 13.02v               The number next to 'Charging' is a state change timer
//1|  message display                  'S' indicates 'S'ummer sensor is detecting light
//2|       box                         'A' indicates 'A'utumn sensor is detecting light
//3|Wed, Mar 15 *11:53am               '*' before clock indicates daylight savings time active


class MyLCD
{

private: //variables
    
public:
    void drawDisplay     ();
    void setCursor       (const Coordinant coordinant);
    void setCursor       (const byte         column,
                          const byte         row);
    void print           (const char        *string_ptr);
    void print           (const byte         numeral);
    void printDateSuffix (const byte         day_of_month);
public:  // <-make this private when old public references are removed
    void printClock      (const TimeElements time,
                          const Coordinant   coordinant,
                          const bool         right_justify);
    void printDate       (const Coordinant   coordinant);
    //void printDay
    void updateBacklight ();
}mylcd;

void MyLCD::drawDisplay()
{
    updateBacklight();
    Coordinant coordinant {13, 3};
    const bool right_justify    {true};
    printClock(gRTC_reading, coordinant, right_justify);
    coordinant = {0, 3};
    printDate(coordinant);
}

void MyLCD::setCursor(const Coordinant coordinant)
{
    liquidcrystali2c.setCursor(coordinant.x, coordinant.y);
}

void MyLCD::setCursor(const byte column, const byte row)
{
    liquidcrystali2c.setCursor(column, row);
}

void MyLCD::print(const char *string_ptr)
{
    liquidcrystali2c.print(string_ptr);   
}

void MyLCD::print(const byte numeral)
{
    liquidcrystali2c.print(numeral);   
}

void MyLCD::printDateSuffix(byte day_of_month)
{
    //test if number is in range
    if (day_of_month < 0 || day_of_month > 31)
    {
        Serial.print  (F("myLCD::printDateSuffix:  received parameter of "));
        Serial.print  (day_of_month);
        Serial.println(F(", but it should be between 1 - 31"));
        return;
    }
    //any number over 20 should be reduced by tens until it is 10 or less
    if (day_of_month >= 20)
    {
        while (day_of_month > 10)
        {
            day_of_month = day_of_month -10;
        }
    }
    //day_of_month should be between 1 and 10
    if (day_of_month == 1)
    {
        liquidcrystali2c.print("st");
    }
    else if (day_of_month == 2)
    {

        liquidcrystali2c.print("nd");
    }
    else if (day_of_month == 3)
    {
        liquidcrystali2c.print("rd");
    }
    else
    {
        liquidcrystali2c.print("th");
    }
}

//MyLCD private methods start here

void MyLCD::updateBacklight()
{
    byte hour_to_turn_on_backlight  {4};
    byte hour_to_turn_off_backlight {21};
    if (gRTC_reading.Hour >= hour_to_turn_on_backlight &&
        gRTC_reading.Hour < hour_to_turn_off_backlight)
    { 
        liquidcrystali2c.backlight();
    }
    else
    {
        liquidcrystali2c.noBacklight();
    }
}

void MyLCD::printClock(const TimeElements time, const Coordinant coordinant, const bool right_justify)
{
    setCursor(coordinant);                                   
    byte am_pm  {0};
    int  format {0};
    //change to 12 hour format
    if ((time.Hour) >= 12)
    {      
        format = 12;
    }
    if ((time.Hour) == 12 || (time.Hour) == 0)
    {
        print("12");
    }
    else
    {
        if (time.Hour - format < 10 && right_justify == true)
        {             
            print (" ");
        }
        print (time.Hour - format);
    }   
    print(":");
    if (time.Minute < 10)
    {  
        print ("0");
    }  
    print(time.Minute);
    if (time.Hour >= 12)
    {
        print("pm");
    }
    else
    {                          
        print("am");
    }                                      
}

void MyLCD::printDate(const Coordinant coordinant)
{
    setCursor(coordinant); 
    print(dayShortStr(weekday()));
    print (", ");
    const char* month_short_name[12] = {"Jan", "Feb", "Mar", "Apr",
                                        "May", "Jun", "Jul", "Aug",
                                        "Sep", "Oct", "Nov", "Dec"};
    print (month_short_name[gRTC_reading.Month-1]);    
    print(" ");
    print((gRTC_reading.Day));
    print(" ");  //this space to clear last digit when month rolls over (31 to 1) 
}

//=========================================================================================================

class MyTimer
{
private:
    byte mCounter;
  public:
    MyTimer();
    byte getCounter   ();
    void update       ();
    void set          (byte x); 
}myTimer;

MyTimer::MyTimer()
{}

byte MyTimer::getCounter()
{
    return mCounter;
}

void MyTimer::update()
{
    mylcd.setCursor(8, 0);
    if(mCounter) //if a countdown is running
    {
        if(mCounter <= 9)
        {
            mylcd.print(" "); // clear the first space if a single digit
        }
        mylcd.print(mCounter);                 // print the counter
        mCounter--;                          // counter decrements here
    }
    else
    {
        mylcd.print("  ");             // no counter, so clear the board
    }
}

void MyTimer::set(byte x)
{
    mCounter = x;
}



const byte QTY_IMPORTANT_DATES = 22;
class MyImportantDates
{
public:
    enum EventType
    {
        EVENTTYPE_ANNIVERSARY,
        EVENTTYPE_BIRTHDAY,
        EVENTTYPE_APPOINTMENT,
        EVENTTYPE_HOLIDAY,
        MAX_EVENTTYPE 
    };
    struct ImportantDate
    {
        const char *text;
        byte        month;
        byte        day;
        int         year;
        EventType   event_type; 
    };
    
private: //variables
    //this compiler can't set array length so QTY_IMPORTANT_DATES must be manually counted and set.     
    const ImportantDate importantdatelist[QTY_IMPORTANT_DATES] = 
    {
        {"Kathy",         1,  7, 1967, EVENTTYPE_BIRTHDAY},     //1
        {"Jack(cat)",     2,  6, 2011, EVENTTYPE_BIRTHDAY},     //2
        {"Katrina",       2,  9, 2000, EVENTTYPE_BIRTHDAY},     //3
        {"Alan",          2, 10, 1993, EVENTTYPE_BIRTHDAY},     //4
        {"Jack(dog)",     2, 27, 2012, EVENTTYPE_BIRTHDAY},     //5
        {"Joshua",        3, 23, 1993, EVENTTYPE_BIRTHDAY},     //6
        {"Dad",           3, 30, 1943, EVENTTYPE_BIRTHDAY},     //7
        {"Christopher",   4,  3, 1990, EVENTTYPE_BIRTHDAY},     //8
        {"Jon",           4, 15, 1968, EVENTTYPE_BIRTHDAY},     //9
        {"Mena",          4, 24, 1972, EVENTTYPE_BIRTHDAY},     //10
        {"Jacob",         4, 26, 2005, EVENTTYPE_BIRTHDAY},     //11
        {"Elizabeth",     4, 29, 2000, EVENTTYPE_BIRTHDAY},     //12
        {"Aunt Helen",    5, 23, 1946, EVENTTYPE_BIRTHDAY},     //13
        {"Aunt Julie",    5, 31, 1942, EVENTTYPE_BIRTHDAY},     //14
        {"Paul and Mena", 6, 29, 1991, EVENTTYPE_ANNIVERSARY},  //15
        {"Cersei",        6, 15, 2015, EVENTTYPE_BIRTHDAY},     //16
        {"Mom and Dad",   9,  7, 1963, EVENTTYPE_ANNIVERSARY},  //17
        {"Paul",          9, 24, 1969, EVENTTYPE_BIRTHDAY},     //18
        {"Mom",          10, 23, 1943, EVENTTYPE_BIRTHDAY},     //19
        {"Erik",         12,  4, 1999, EVENTTYPE_BIRTHDAY},     //20
        {"Mathew",       12, 17, 1995, EVENTTYPE_BIRTHDAY},     //21
        {"Christmas",    12, 25, 0,    EVENTTYPE_HOLIDAY}       //22
    };        
}myimportantdates;

class MyInfo //gains information and sends it to LCD and serial
{
public: //methods
    void printState(char const *text);   
}myinfo;

void MyInfo::printState(char const *text)
{
    //LCD printing in the upper left 8 characters of the 4x20 display   
    mylcd.setCursor (0,0);
    mylcd.print (text);
    myserial.printTimestamp();
    myserial.sprint(" state changed to: ");
    myserial.sprint(text);
    myserial.printLinefeed();
}

// PIN ASSIGNMENT HERE                                      
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


class Voltmeter
{
private: //variables
    float mVoltage {13.4};
    int           voltage_trend = 0;
public:  //methods
    void  main       ();
    float getVoltage ();
private: //methods
    void readVoltage();
}voltmeter;

void Voltmeter::main()
{
    readVoltage();
}

float Voltmeter::getVoltage()
{
    return mVoltage;
}

void Voltmeter::readVoltage()
{
    float         reference_voltage {5.0}; //using the default operating voltage reference.
                                           //which is measured between 5V pin and Ground

    //The controller cannot read voltage above 5 volts.
    //Instead, it measures a smaller voltage from a voltage divider and then multiplies that
    //result by a voltage ratio.
    //To establish the voltage ratio:
    //take two measurements with a portable voltmeter                                        
    //do not take these readings with the USB cable attached, they won't be accurate.
    //my battery reading  12.28
    //my divider reading    2.9
    //go ahead and use a calculator to get this number.
    //Since it is a constant, there is need to make the controller calculate it every time.   
    const float   voltage_ratio     {4.31};  // 12.28 / 2.9
    //my voltage divider is constructed as:  
    //150k resistor from positive battery terminal to voltage divider intersection.
    // 47k resistor from negative battery terminal to voltage divider intersection.
    
    const byte    range_of_voltage_trend {100};
    const int     pinValue {analogRead(the_analog_pin_to_the_voltage_divider)};  
    float raw_voltage = pinValue * voltage_ratio * reference_voltage / 1024;
   


                                      // convert analog reading to the useable voltage number
  //voltage = voltage + (USB_mode * 0.20);  // this line to correct voltage (add .2) when usb plugged in 
  //stabilizer alpha

  if (raw_voltage > mVoltage){  voltage_trend ++;  }
  if (raw_voltage < mVoltage){  voltage_trend --;  }
  if (voltage_trend >= range_of_voltage_trend){
     mVoltage = mVoltage + 0.01;
     voltage_trend = 0;
  }
  if (voltage_trend <= (0 - range_of_voltage_trend)){
     mVoltage = mVoltage - 0.01;
     voltage_trend = 0;
  }
}    
  

// 2                      3                      4                     5                       6                       7                      8                     9                      10                     11                     12                     13                     14                     15                     16                     17                      18                     19                     20                      21                     22
byte  number_of_records_to_scan = 23;      // "12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890",,"12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890","12345678901234567890"     
const char* important_dates_string_array[] = {" Kathy"               ,"Jack(cat)"           ,"Katrina"             ,"  Alan"              ,"  Jack"              ," Joshua"              ,"  Dad"               ,"Christopher"         ,"  Jon"                ,"  Mena"              ," Jacob"              ,"Elizabeth"           ,"Aunt Julie"          ," Dentist 8:00AM"     ,"Paul and Mena"       ,"Empty"               ," Cersei"             ," Mom and Dad"        ,"  Paul"              ,"  Mom"               ," Erik"               ,"Mathew"              ,"      Christmas"};
const byte  important_dates_month_array[]  = {1                      ,2                     ,2                     ,2                     ,2                     ,3                      ,3                     ,4                     ,4                      ,4                     ,4                     ,4                     ,5                     ,10                    ,6                     ,0                     ,6                     ,9                     ,9                     ,10                    ,12                    ,12                    ,12};
const byte  important_dates_day_array[]    = {7                      ,6                     ,9                     ,10                    ,27                    ,23                     ,30                    ,3                     ,15                     ,24                    ,26                    ,29                    ,31                    ,10                    ,29                    ,0                     ,15                    ,7                     ,24                    ,23                    ,4                     ,17                    ,25};
const int   important_dates_yob_array[]    = {1967                   ,2011                  ,2000                  ,1993                  ,2012                  ,1993                   ,1943                  ,1990                  ,1968                   ,1972                  ,2005                  ,2000                  ,1942                  ,0                     ,1991                  ,0                     ,2015                  ,1963                  ,1969                  ,1943                  ,1999                  ,1995                  ,0};   
const byte  event_type_to_print_array[]    = {1                      ,1                     ,1                     ,1                     ,1                     ,1                      ,1                     ,1                     ,1                      ,1                     ,1                     ,1                     ,1                     ,0                     ,2                     ,0                     ,1                     ,2                     ,1                     ,1                     ,1                     ,1                     ,0};
                                     

const char* month_short_name[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
//               WEEK      -1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53

byte sunrise_hour[53]   = { 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8 }; 
byte sunrise_minute[53] = {47,48,46,42,37,31,23,14, 4,54,43,32,21,10,59,48,38,29,20,13, 7, 3, 0,58,58, 0, 3, 7,12,18,24,30,37,43,50,56, 3, 9,16,23,29,36,44,51,59, 7,15,23,30,36,41,45,47 };

byte sunset_hour[53]    = {18,18,18,18,18,18,18,19,19,19,19,19,19,19,19,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,19,19,19,19,19,18,18,18,18,18,18,18,18,18,18,18,18,18 }; 
byte sunset_minute[53]  = {13,19,26,34,43,51,59, 7,15,23,30,37,44,51,58, 5,12,19,26,33,39,45,50,54,57,58,58,56,53,48,42,34,26,16,06,55,44,32,21, 9,58,48,38,29,21,14, 8, 4, 2, 2, 4, 7,13 };

boolean daylight_savings_time = true; // was false from fall to spring, now trying true for 
                                      // spring to fall

int  inverter_run_time  = 0;
//byte power_manager_mode = 0; // 0 = night time, or before first charge
                             // 1 = balance mode first 14v charge complete, gomode 2 if power drops below 12.6 60s
                         //                                  gomode 3 if power rises above 13.5 60s
                         // 2 = aux charger trying to get us to 12.6                                  
                         // 3 = inverter cycling


//float         stable_voltage = 13.4;  // the usable number to be displayed on LCD screen                     
//float         raw_voltage = 0;            
float         voltage_daily_max = 0;
float         voltage_daily_min = 15; // initial setting, a number higher than expected nominal voltage
                            
byte          am_pm = 0;


tmElements_t  todays_low_voltage_timestamp;
tmElements_t  todays_high_voltage_timestamp;
byte          solar_week_number = 1;



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


boolean       LDR_data = false;
boolean       LDR2_data = false;
boolean       DST = true;
boolean       message_loaded [3]  = {false,false,false};
byte reminder_message_pointer [3] = {false,false,false};
byte dimmer_reference_number = 0;

const unsigned long seconds_in_a_week = 604800;  


//void myPrintStatefunction(char const * text);
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
//void myMessageVoltageDailyLowfunction();
void myMessageWeekNumberfunction();
void myPrintDatetoLCDfunction(byte x, byte y);
void myPrintLDRresultsToLCDfunction();
//void myPrintSerialTimestampfunction ();
//void myPrintTimetoLCDfunction(TimeElements timestamp, byte x, byte y, boolean right_justify);
void myPrintVoltagetoLCDfunction(int x,int y,float v);
void myReadPotentiometerAndAdjustWorkbenchTrackLightsfunction();
//char const * myReturnDayofWeekfunction (byte x);
//char const * myReturnDayofWeekFromUnixTimestampfunction (TimeElements unzipped_time);
void mysetSunriseSunsetTimesfunction();
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
void setup()
{
    Serial.begin(250000);              // start the serial monitor
    Wire.begin();                      // start the Wire library
    liquidcrystali2c.begin(20, 4);     // start the LiquidCrystal_I2C library
    myserial.testClock();
    solar_week_number = myCalculateWeekNumberfunction(gRTC_reading);
    myLoadUpcomingEventsfunction();
    mysetSunriseSunsetTimesfunction();
    //set pins  
    pinMode (inverter_signal_pin, OUTPUT);
    pinMode (battery_charger_signal_pin, OUTPUT);
    pinMode (the_analog_pin_to_the_voltage_divider, INPUT);
    pinMode (the_pin_to_the_LDR_circuit, INPUT);
    pinMode (the_pin_to_the_LDR2_circuit, INPUT);
    pinMode (workbench_lighting_MOSFET_signal_pin, OUTPUT);
    pinMode (stage_one_inverter_relay, OUTPUT);
    pinMode (stage_two_inverter_relay, OUTPUT);
}
               
void loop()
{
    //This code runs as fast as possible
    myReadPotentiometerAndAdjustWorkbenchTrackLightsfunction();
    voltmeter.main();
    
    myVoltagePrintingAndRecordingfunction();
    //This code runs every second (1000ms)
    RTC.read(gRTC_reading);  //gathered from library by reference
    if (gLast_RTC_reading.Second != gRTC_reading.Second) 
    {
        gLast_RTC_reading = gRTC_reading;  //set up delay for the next loop
        //--------------to remove-------------------------------------------
        if (RTC.read(gRTC_reading)) //gathered by reference
        {
            //to be removed along with time lib
            setTime(gRTC_reading.Hour,gRTC_reading.Minute,gRTC_reading.Second,gRTC_reading.Day,gRTC_reading.Month,gRTC_reading.Year-30);
        }
        //--------------end of to remove-------------------------------------------
        myserial.printStatus();
        myTimer.update();
        mylcd.drawDisplay(); 
        
        //myPrintDatetoLCDfunction(0, 3);
        myPrintLDRresultsToLCDfunction();
        switch (mystatemachine.getState())
        {
        case MyStateMachine::STATE_INIT_SLEEP:
            myStateMachineInitSleepStatefunction();
            break;
        case MyStateMachine::STATE_SLEEP:
            myStateMachineSleepStatefunction();
            break;
        case MyStateMachine::STATE_INIT_WAKE:
            myStateMachineInitWakeStatefunction();
            break;
        case MyStateMachine::STATE_WAKE:
            myStateMachineWakeStatefunction();
            break;
        case MyStateMachine::STATE_INIT_BALANCED:
            myStateMachineInitBalancedStatefunction();
            break;
        case MyStateMachine::STATE_BALANCED:
            myStateMachineBalancedStatefunction();
            break;
        case MyStateMachine::STATE_INIT_INVERTER_WARM_UP:
            myStateMachineInitWarmUpInverterStatefunction();
            break;
        case MyStateMachine::STATE_INVERTER_WARM_UP:
            myStateMachineWarmUpInverterStatefunction();
            break;
        case MyStateMachine::STATE_INIT_INVERTER_STAGE_ONE:
            myStateMachineInitStageOneInverterStatefunction();
            break;
        case MyStateMachine::STATE_INVERTER_STAGE_ONE:
            myStateMachineStageOneInverterStatefunction();
            break;
        case MyStateMachine::STATE_INIT_INVERTER_STAGE_TWO:
            myStateMachineInitStageTwoInverterStatefunction();
            break;
        case MyStateMachine::STATE_INVERTER_STAGE_TWO:
            myStateMachineStageTwoInverterStatefunction();
            break;
        case MyStateMachine::STATE_INIT_DAY_CHARGE:
            myStateMachineInitDaytimeChargingfunction();
            break;
        case MyStateMachine::STATE_DAY_CHARGE:
            myStateMachineDaytimeChargingfunction();
            break;
        case MyStateMachine::STATE_INIT_INVERTER_COOL_DOWN:
            myStateMachineInitInverterCooldownfunction();
            break;
        case MyStateMachine::STATE_INVERTER_COOL_DOWN:
            myStateMachineInverterCooldownfunction();
            break;
        case MyStateMachine::STATE_ERROR:
        case MyStateMachine::MAX_STATE:
        default:
            break;
        }
        // This code runs at 2:00am 
        if (gRTC_reading.Hour   == 2 &&
            gRTC_reading.Minute == 0 &&
            gRTC_reading.Second == 0)
        {
            mystatemachine.setState(MyStateMachine::STATE_INIT_SLEEP);  //make sure machine is sleeping (redundant)
            inverter_run_time  = 0;
            voltage_daily_max = voltmeter.getVoltage(); 
            todays_high_voltage_timestamp = gRTC_reading;
            voltage_daily_min = voltmeter.getVoltage();
            todays_low_voltage_timestamp = gRTC_reading;
            solar_week_number = myCalculateWeekNumberfunction(gRTC_reading); //to read duskdawn data tables
            myLoadUpcomingEventsfunction();
            mysetSunriseSunsetTimesfunction();  
        }
        //This code only runs if MessageManager sets the message_manager_timestamp 
        if ((long unsigned)(message_manager_timestamp) <= millis())
        { 
            myMessageManagerfunction(); 
        }              
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
   liquidcrystali2c.setCursor (0,1); liquidcrystali2c.print (F("                    "));
   liquidcrystali2c.setCursor (0,2); liquidcrystali2c.print (F("                    "));
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
   
   TimeElements  record_date = gRTC_reading;
   TimeElements  reference_date = gRTC_reading;
   record_date.Year = gRTC_reading.Year - 30;        //*year error correction
   reference_date.Year = gRTC_reading.Year - 30;
   // check for events ocurring from now till the end of the scan window
   for (byte x = 0; x < QTY_IMPORTANT_DATES; x++)
   {
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
  liquidcrystali2c.setCursor (0,1);
  if (inverter_run_time == 0) {
               //"12345678901234567890"
     liquidcrystali2c.print(F("  Inverter Waiting"));
     return;  
  } 
  if (inverter_run_time > 0 && inverter_run_time < 60) {
               //"12345678901234567890"
     liquidcrystali2c.print(F(" Inverter harvested"));
     liquidcrystali2c.setCursor (8,2);
     liquidcrystali2c.print(inverter_run_time);
     liquidcrystali2c.print("s");
     return;  
  }
  if (inverter_run_time >= 60) {
               //"12345678901234567890"
     liquidcrystali2c.print(F(" Inverter harvested"));
     liquidcrystali2c.setCursor (8,2);
     liquidcrystali2c.print(inverter_run_time/60);
     liquidcrystali2c.print("m");
     return;  
  }
}

//-------------------------------
void myMessageReminderfunction(){
//-------------------------------
   myClearMessageBoardfunction();
   liquidcrystali2c.setCursor (0,1);
   //           "12345678901234567890"
   liquidcrystali2c.print (F("   Did you MED-X?"));
   liquidcrystali2c.setCursor (0,2);
   //liquidcrystali2c.print (F("   eggs floss med-x?"));
   
}
//*******************************
void myMessageSunrisefunction() {
//*******************************
  myClearMessageBoardfunction();
  liquidcrystali2c.setCursor (0,1);
  //           "12345678901234567890"
  liquidcrystali2c.print (F("   Sunrise "));
  liquidcrystali2c.print (today_sunrise_hour);
  liquidcrystali2c.print (F(":"));
  if (today_sunrise_minute <= 9){
   liquidcrystali2c.print(F("0"));
  }
  liquidcrystali2c.print (today_sunrise_minute);
  liquidcrystali2c.print (F("am"));
  liquidcrystali2c.setCursor (0,2);
  //           "12345678901234567890"
  liquidcrystali2c.print (F("   Sunset  ")); 
  liquidcrystali2c.print (today_sunset_hour - 12);
  liquidcrystali2c.print (F(":"));
  if (today_sunset_minute <= 9){
   liquidcrystali2c.print(F("0"));
  }
  liquidcrystali2c.print (sunset_minute[solar_week_number-1]);
  liquidcrystali2c.print (F("pm"));
}


  
//----------------------------------------
void myMessageUpcomingEventsfunction(byte n){
//----------------------------------------  
  TimeElements timex = gRTC_reading;
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
  if(gRTC_reading.Month == 12 && timex.Month == 1) {    //to protect myReturnDayofWeekFromUnixTimestampfunction
                                                        //from end of year rollover
    timex.Year++; 
  }
  if (message_loaded [n]){
    liquidcrystali2c.setCursor (0,1);
    if (important_dates_yob_array[reminder_message_pointer [n]]) {
    liquidcrystali2c.print (important_dates_string_array[reminder_message_pointer [n]]);
    liquidcrystali2c.print ("'s ");
    liquidcrystali2c.print (year() - important_dates_yob_array[reminder_message_pointer [n]]);
    const byte day_of_month = year() - important_dates_yob_array[reminder_message_pointer [n]];
    mylcd.printDateSuffix(day_of_month);
    if (event_type_to_print_array[reminder_message_pointer [n]] == 1) {
      liquidcrystali2c.print (" B-day");  
    }
    } else {
    liquidcrystali2c.print (important_dates_string_array[reminder_message_pointer [n]]);
    }
    liquidcrystali2c.setCursor (5,2);
    if (timex.Day == gRTC_reading.Day || timex.Day == gRTC_reading.Day + 1) {      
      if (timex.Day == gRTC_reading.Day) {
        //         "12345678901234567890"
        liquidcrystali2c.print ("   today.");  
      }
      if (timex.Day == gRTC_reading.Day+1) {
        //         "12345678901234567890"
        liquidcrystali2c.print (" tomorrow");  
      }
    } else {


      time_t long_int_time {makeTime(timex)};
      int myday {weekday(long_int_time)};
      const char *dayString {dayShortStr(myday)};
      mylcd.print(dayString);
      liquidcrystali2c.print (", ");
      liquidcrystali2c.print (month_short_name[(important_dates_month_array[reminder_message_pointer [n]])-1]);
      liquidcrystali2c.print (" ");
      liquidcrystali2c.print (important_dates_day_array[reminder_message_pointer [n]]);
    }  
  }
}
//


//-----------------------------------
void myMessageVoltageDailyHighfunction(){
//-----------------------------------  
   
   myClearMessageBoardfunction();
   liquidcrystali2c.setCursor (0,1);
   //         "12345678901234567890"
   myPrintVoltagetoLCDfunction(2,1,voltage_daily_max);
   liquidcrystali2c.print (F(" @"));
   Coordinant coordinant {11, 1};
   const bool right_justify {false};
   mylcd.printClock(todays_high_voltage_timestamp, coordinant, right_justify);
   myPrintVoltagetoLCDfunction(2,2,voltage_daily_min);
   liquidcrystali2c.print (F(" @"));
   coordinant = {11, 2};
   mylcd.printClock(todays_low_voltage_timestamp, coordinant, right_justify);   
}

void myMessageWeekNumberfunction() {
//----------------------------------

   myClearMessageBoardfunction();
   liquidcrystali2c.setCursor (0,1);
   //         "12345678901234567890"
   liquidcrystali2c.print (F("  Solar Week"));
   liquidcrystali2c.setCursor (0,2);
   liquidcrystali2c.print (F("        Number "));
   liquidcrystali2c.print (solar_week_number);   
}

////---------------------------------------------
//void myPrintDatetoLCDfunction(byte x, byte y) {
////--------------------------------------------- 
   //liquidcrystali2c.setCursor (x,y);
   //liquidcrystali2c.print(myReturnDayofWeekfunction(weekday() - 1));
   //liquidcrystali2c.print (", ");
   //liquidcrystali2c.print (month_short_name[gRTC_reading.Month-1]);    
   //liquidcrystali2c.print(" ");
   //liquidcrystali2c.print((gRTC_reading.Day));
   //liquidcrystali2c.print(" ");  //this space to clear last digit when month rolls over (31 to 1) 
//}
//-------------------------------------
void myPrintLDRresultsToLCDfunction() {
//-------------------------------------
   liquidcrystali2c.setCursor (11,0);
   //Serial.print("Summer: "); Serial.print(analogRead(the_pin_to_the_LDR2_circuit)); Serial.print(". ");
   if (digitalRead(the_pin_to_the_LDR2_circuit)) {
      liquidcrystali2c.print("S");
   } else {
      liquidcrystali2c.print(" ");
   }     
   if (digitalRead(the_pin_to_the_LDR_circuit)) { 
      liquidcrystali2c.print("A");
   } else {
      liquidcrystali2c.print(" ");
   }     
}



////---------------------------------------------   
//void myPrintTimetoLCDfunction(TimeElements timestamp, byte x, byte y, boolean right_justify) {
////---------------------------------------------  
                                      //// x and y are the LCD coordinants where the time is to be printed
                                      //// x can be 0 to 19
                                      //// y can be 0 to 3
   //liquidcrystali2c.setCursor (x,y);                                      
   //if ((timestamp.Hour) >= 12) {      
      //am_pm = 12;
   //} else {                           // set am_pm variable
      //am_pm = 0;
   //} 
   //if ((timestamp.Hour) == 12 || (timestamp.Hour) == 0) {
      //liquidcrystali2c.print ("12");
   //} else {
      //if ((timestamp.Hour)-am_pm < 10 && right_justify) {             
         //liquidcrystali2c.print (" ");
      //}
      //liquidcrystali2c.print ((timestamp.Hour)-(am_pm));
   //}   
   //liquidcrystali2c.print(":");
   //if ((timestamp.Minute) < 10 ) {  
      //liquidcrystali2c.print ("0");
   //}  
   //liquidcrystali2c.print (timestamp.Minute);
   //if ((timestamp.Hour) >= 12) {
      //liquidcrystali2c.print("pm");
   //} else {                          
      //liquidcrystali2c.print("am");
   //}                                      
//}
//----------------------------------------------------
void myPrintVoltagetoLCDfunction(int x,int y,float v){
//----------------------------------------------------
   liquidcrystali2c.setCursor (x,y);
   liquidcrystali2c.print (v);
   liquidcrystali2c.print ("v");      
}



//**************************************************************
void myReadPotentiometerAndAdjustWorkbenchTrackLightsfunction(){
//**************************************************************
  const float correction = -0.40;  // I arrived at this with direct measurement.  IDK why.  PN junction?
  const float maximum_voltage = 12.00 + correction; // do not allow the bulbs more voltage than this number.
  
  float scaling_ratio = 1;
  if (voltmeter.getVoltage() > maximum_voltage) {
    //Serial.print ("stable voltage: ");
    //Serial.print (stable_voltage);
    scaling_ratio = maximum_voltage / voltmeter.getVoltage();  
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


////***************************************
//char const * myReturnDayofWeekfunction (byte x){
////***************************************  
//char const * weekday_array[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
//return weekday_array[x];  
//}
////*********************************************************************************
//char const * myReturnDayofWeekFromUnixTimestampfunction (TimeElements unzipped_time){
////*********************************************************************************
  //long unsigned seconds_in_a_day = 86400;
  //long unsigned zipped_time      = makeTime(unzipped_time) / seconds_in_a_day ;
  //int x = 0;
  ////Serial.print (" - "); 
  ////Serial.print (zipped_time);
  //x = (zipped_time - (zipped_time / 7) * 7);  //detecting remainder with my integer math trick
  ////align week number with my array, because 4 <> Monday
  //x = x - 3;
  //if (x < 0){
    //x=x+7;
  //}
  ////Serial.print (x);
  ////Serial.print (" - ");
  ////Serial.print(myReturnDayofWeekfunction(x));
  //return myReturnDayofWeekfunction(x);  
//}

//======================================
void mysetSunriseSunsetTimesfunction() {
//==================12.4.2017===========

  
  today_sunrise_hour = sunrise_hour[solar_week_number-1]-1+daylight_savings_time;
  today_sunrise_minute = sunrise_minute[solar_week_number-1];
  today_sunset_hour = sunset_hour[solar_week_number-1]-1+daylight_savings_time;
  today_sunset_minute = sunset_minute[solar_week_number-1];
}


//-----------------------------------------
void myVoltagePrintingAndRecordingfunction() {
//-----------------------------------------  
   myPrintVoltagetoLCDfunction(14,0,voltmeter.getVoltage());                      
   //Serial.print (stable_voltage); Serial.print ("V "); 
   if (voltmeter.getVoltage() > voltage_daily_max) { 
    voltage_daily_max = voltmeter.getVoltage();
    todays_high_voltage_timestamp = gRTC_reading;
   }
   if (voltmeter.getVoltage() < voltage_daily_min) {
     voltage_daily_min = voltmeter.getVoltage(); 
     todays_low_voltage_timestamp = gRTC_reading;
   }
}

//===================================
//void myVoltageCalculationfunction() {
////===================================  
  //const byte    range_of_voltage_trend                    = 100;
  //raw_voltage = analogRead(the_analog_pin_to_the_voltage_divider) * voltage_measurement_divided_by_voltage_divider_measurement * reference_voltage / 1024;
                                      //// convert analog reading to the useable voltage number
  ////voltage = voltage + (USB_mode * 0.20);  // this line to correct voltage (add .2) when usb plugged in 
  ////stabilizer alpha

  //if (raw_voltage > stable_voltage){  voltage_trend ++;  }
  //if (raw_voltage < stable_voltage){  voltage_trend --;  }
  //if (voltage_trend >= range_of_voltage_trend){
     //stable_voltage = stable_voltage + 0.01;
     //voltage_trend = 0;
  //}
  //if (voltage_trend <= (0 - range_of_voltage_trend)){
     //stable_voltage = stable_voltage - 0.01;
     //voltage_trend = 0;
  //}
    
  

  ////end of stabilizer alpha 
//}

//===========================================
void myStateMachineInitSleepStatefunction() {               //state 0
  //================================5.8.2018===
  //mylcd.setCursor(0, 0);
  //mylcd.print("Sleeping");
  myinfo.printState("Sleeping");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, HIGH);  //battery charger on
  digitalWrite (inverter_signal_pin, LOW);         //inverter off
  digitalWrite(stage_one_inverter_relay, LOW);    // relay one off
  digitalWrite(stage_two_inverter_relay, LOW);    // relay two off
  mystatemachine.setState(MyStateMachine::STATE_SLEEP);
  return;
}

//=======================================
void myStateMachineSleepStatefunction() {                   //state 1
  //============================5.8.2018===
  if (myIsItDaylightfunction() == true) {        //switch to initiate wake state if light
    mystatemachine.setState(MyStateMachine::STATE_INIT_WAKE);
    return;
  }

  return;
}

//==========================================
void myStateMachineInitWakeStatefunction() {                //state 2
  //==============================5.8.2018====
  myinfo.printState("Waking  ");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, LOW);   // battery charger off
  digitalWrite (inverter_signal_pin, LOW);          // inverter off
  digitalWrite(stage_one_inverter_relay, LOW);      // relay one off
  digitalWrite(stage_two_inverter_relay, LOW);      // relay two off
  mystatemachine.setState(MyStateMachine::STATE_WAKE);
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
    mystatemachine.setState(MyStateMachine::STATE_INIT_SLEEP); //initiate sleep state
    return;
  }
  if (myIsItDeltaTimePastDawnfunction() == true) {
    mystatemachine.setState(MyStateMachine::STATE_INIT_BALANCED);  //initiate balanced
    return;
  }
}

//==============================================
void myStateMachineInitBalancedStatefunction() {            //state 4
  //===================================5.8.2018===


  myinfo.printState("Balanced");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, LOW);  //battery charger off
  digitalWrite (inverter_signal_pin, LOW);         //inverter off
  digitalWrite(stage_one_inverter_relay, LOW);    // relay one off
  digitalWrite(stage_two_inverter_relay, LOW);    // relay two off
  //
  mystatemachine.setState(MyStateMachine::STATE_BALANCED);                               // balanced initialization complete
  return;
}

//==========================================
void myStateMachineBalancedStatefunction() {                //state 5
  //===============================5.8.2018===

  const float voltage_to_start_inverter = 13.80;
  const float voltage_to_start_daytime_charging = 12.60;

  if (myIsItDaylightfunction() == false) {        //switch to initiate sleep mode if dark
    mystatemachine.setState(MyStateMachine::STATE_INIT_SLEEP); //initiate sleep state
    return;
  }
  if (voltmeter.getVoltage() >= voltage_to_start_inverter) {
    mystatemachine.setState(MyStateMachine::STATE_INIT_INVERTER_WARM_UP);     //init warm up inverter
    return;
  }
  if (voltmeter.getVoltage() <= voltage_to_start_daytime_charging) {
    mystatemachine.setState(MyStateMachine::STATE_INIT_DAY_CHARGE);    //init daytime charging
    return;
  }


  return;
}

//======================================================
void myStateMachineInitWarmUpInverterStatefunction() {    //state 6
  //===========================================5.8.2018===
  const byte seconds_to_warm_up = 4;

  myinfo.printState("Warm Up ");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, LOW);  //battery charger off
  digitalWrite (inverter_signal_pin, HIGH);         //inverter on
  digitalWrite(stage_one_inverter_relay, LOW);    // relay one off
  digitalWrite(stage_two_inverter_relay, LOW);    // relay two off
  myTimer.set(seconds_to_warm_up);
  mystatemachine.setState(MyStateMachine::STATE_INVERTER_WARM_UP);
}



//==================================================
void myStateMachineWarmUpInverterStatefunction() {        //state 7
  //=======================================5.8.2018===

  if (myTimer.getCounter()) {    //warming up during countdown
    return;
  }
  mystatemachine.setState(MyStateMachine::STATE_INIT_INVERTER_STAGE_ONE);

}
//======================================================
void myStateMachineInitStageOneInverterStatefunction() {    //state 8
  //===========================================5.8.2018===

  const byte  stage_two_switching_delay = 15;  //seconds.  this prevents stage two from engaging too soon

  myinfo.printState("Invert1 ");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, LOW);  //battery charger off
  digitalWrite (inverter_signal_pin, HIGH);         //inverter on
  digitalWrite(stage_one_inverter_relay, HIGH);    // relay one on
  digitalWrite(stage_two_inverter_relay, LOW);    // relay two off
  myTimer.set(stage_two_switching_delay);
  mystatemachine.setState(MyStateMachine::STATE_INVERTER_STAGE_ONE);
}

//==================================================
void myStateMachineStageOneInverterStatefunction() {        //state 9
  //=======================================5.8.2018===

  const float voltage_to_turn_inverter_off = 12.55;
  const float voltage_to_switch_to_stage_two = 13.80;

  inverter_run_time++;

  if (myIsItDaylightfunction() == false) {        //switch to initiate sleep mode if dark
    mystatemachine.setState(MyStateMachine::STATE_INIT_SLEEP); //initiate sleep state -   //might be helpful for a charger stuck in "on" state
    return;
  }

  if (voltmeter.getVoltage() >= voltage_to_switch_to_stage_two && !myTimer.getCounter()) {
    mystatemachine.setState(MyStateMachine::STATE_INIT_INVERTER_STAGE_TWO);  //initiate stage two inverter
    return;
  }

  if (voltmeter.getVoltage() <= voltage_to_turn_inverter_off) {
    mystatemachine.setState(MyStateMachine::STATE_INIT_INVERTER_COOL_DOWN); //initiate inverter cooldown state, so a burst of solar energy does not short
    //cycle the inverter
    return;
  }

}
//======================================================
void myStateMachineInitStageTwoInverterStatefunction() {    //state 10
  //==============================5.8.2018================
  myinfo.printState("Invert2 ");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, LOW);  //battery charger off
  digitalWrite (inverter_signal_pin, HIGH);         //inverter on
  digitalWrite(stage_one_inverter_relay, HIGH);    // relay one on
  digitalWrite(stage_two_inverter_relay, HIGH);    // relay two on
  mystatemachine.setState(MyStateMachine::STATE_INIT_INVERTER_STAGE_TWO);
}

//==================================================
void myStateMachineStageTwoInverterStatefunction() {        //state 11
  //===========================5.8.2018===============

  const float voltage_to_drop_back_to_stage_one = 12.70;

  if (myIsItDaylightfunction() == false) {        //switch to initiate sleep mode if dark
    mystatemachine.setState(MyStateMachine::STATE_INIT_SLEEP); //initiate sleep state -   //might be helpful for a charger stuck in "on" state
    return;
  }

  inverter_run_time++;

  if (voltmeter.getVoltage() <= voltage_to_drop_back_to_stage_one) {
    mystatemachine.setState(MyStateMachine::STATE_INIT_INVERTER_STAGE_ONE);
    return;
  }

}

//================================================
void myStateMachineInitDaytimeChargingfunction() {    //state 12
  //====================================5.11.2018===
  myinfo.printState("Charging");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, HIGH);  //battery charger on
  digitalWrite (inverter_signal_pin, LOW);         //inverter off
  digitalWrite(stage_one_inverter_relay, LOW);    // relay one off
  digitalWrite(stage_two_inverter_relay, LOW);    // relay two off
  mystatemachine.setState(MyStateMachine::STATE_DAY_CHARGE);
}
//============================================
void myStateMachineDaytimeChargingfunction() {  //state 13
  //================================5.11.2018===

  const float voltage_to_switch_off_charger = 13.40;

  if (voltmeter.getVoltage() >= voltage_to_switch_off_charger) {
    mystatemachine.setState(MyStateMachine::STATE_INIT_BALANCED); //switch to initbalanced
    return;
  }
}

//================================================
void myStateMachineInitInverterCooldownfunction() {    //state 14
  //====================================5.11.2018===

  byte inverter_cooldown_time = 30;  //seconds

  myinfo.printState("Cooldown");
  //initialize relevant pins with redundancy
  digitalWrite (battery_charger_signal_pin, LOW);  //battery charger off
  digitalWrite (inverter_signal_pin, LOW);         //inverter off
  digitalWrite(stage_one_inverter_relay, LOW);    // relay one off
  digitalWrite(stage_two_inverter_relay, LOW);    // relay two off
  myTimer.set(inverter_cooldown_time);
  mystatemachine.setState(MyStateMachine::STATE_INVERTER_COOL_DOWN);
}

//================================================
void myStateMachineInverterCooldownfunction() {    //state 15
  //====================================5.11.2018===

  if (!myTimer.getCounter()) {
    mystatemachine.setState(MyStateMachine::STATE_INIT_BALANCED);  //init balanced
  }
}
