#include <Time.h>
#include <TimeLib.h>
#include <LiquidCrystal_I2C.h>
#include <DS1307RTC.h> //object pre-initialized as 'RTC' in header file 
#include <Wire.h>

LiquidCrystal_I2C  liquidcrystali2c(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  

//globals
tmElements_t       gRTC_reading;
tmElements_t       gLast_RTC_reading;



//all caps indicate a COMPILE TIME CONSTANT 
const byte QTY_IMPORTANT_DATES = 22;  //this must be initialized in global space instead
                                      //of inside Calendar.  
class Calendar
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
    const char* mMonthShortName[12] = {"Jan", "Feb", "Mar", "Apr",
                                       "May", "Jun", "Jul", "Aug",
                                       "Sep", "Oct", "Nov", "Dec"};
    //this compiler can't set array length so QTY_IMPORTANT_DATES (right above
    //this class) must be manually counted and set.     
    const ImportantDate mImportantDateList[QTY_IMPORTANT_DATES] = 
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
    const char* mDaySuffix[4] = {"st", "nd", "rd", "th"};
private: //variables continued
    byte mQtyImportantDatesToReport              {};
    ImportantDate const * mDatesToReportList[QTY_IMPORTANT_DATES] {};
    //mDatesToReportList array is large enough to hold pointers to every event if needed.
public:  //methods
    
    const char*    getDaySuffix            (byte day_number);
    const ImportantDate* getImportantDate        (const byte index);     
    const char*    getMonthShortName       (const byte month_number);
    byte           getWeekNumber           (tmElements_t date);
    byte           getQtyImportantDates    ();
    void           loadImportantDates      ();
    void           serialPrintImportantDate(const ImportantDate importantdate);

}calendar;

const char* Calendar::getDaySuffix(byte day_number)
{
    //test if number is in range
    if (day_number < 0 || day_number > 31)
    {
        Serial.print  (F("Calendar::printDateSuffix:  received parameter of "));
        Serial.print  (day_number);
        Serial.println(F(", but it should be between 1 - 31"));
        return mDaySuffix[3];
    }
    //Isolate the teens and be sure they get 'th' suffix (11th 12th 13th)
    //The non-teen 1, 2, 3,s get their special suffix (2nd, 22nd, 31st) 
    //Therefore, any number over 20 should be reduced by tens until it is 10 or less
    if (day_number >= 20)
    {
        while (day_number > 10)
        {
            day_number -= 10;
        }
    }
    //day_of_month should now be between (1 and 20)
    if (day_number == 1)
    {
        //mDaySuffix[4] = {"st", "nd", "rd", "th"}
        return mDaySuffix[0];
    }
    else if (day_number == 2)
    {
        return mDaySuffix[1];
    }
    else if (day_number == 3)
    {
        return mDaySuffix[2];
    }
    else
    {
        return mDaySuffix[3];
    }
}

const Calendar::ImportantDate* Calendar::getImportantDate(const byte index)
{
    return mDatesToReportList[index];
}

const char* Calendar::getMonthShortName(const byte month_number)
{
    return mMonthShortName[month_number];
}

byte Calendar::getWeekNumber(tmElements_t date)
{
    //convert date to the first moment of the year
    date.Hour   = 0;
    date.Minute = 0;
    date.Second = 0;
    date.Day    = 0;
    date.Month  = 0;
    //convert first moment of the year to unix time
    const time_t year_start {makeTime(date)};
    //how many seconds have elapsed since the year began?
    const time_t seconds_since_start_of_year {now() - year_start};
    //divide by seconds in a week to compute week number
    const time_t seconds_in_a_week {604800};
    const time_t week_number {seconds_since_start_of_year / seconds_in_a_week};
    return (week_number);
}

byte Calendar::getQtyImportantDates()
{
    return mQtyImportantDatesToReport;
}

void Calendar::loadImportantDates()
{
    //load all events in the next two weeks
    const time_t search_window_days {14};
    const time_t seconds_in_a_day   {86400};
    const time_t search_window_secs {search_window_days * seconds_in_a_day}; 
    for (int i = 0; i < QTY_IMPORTANT_DATES; i++)
    {
        //build anniversary date of event
        tmElements_t event_anniversary {};
        //clear Hours, Minutes, Seconds
        event_anniversary.Hour   = 0;
        event_anniversary.Minute = 0;
        event_anniversary.Second = 0;
        //align year
        event_anniversary.Year   = gRTC_reading.Year;
        //get day and month
        event_anniversary.Day    = mImportantDateList[i].day;
        event_anniversary.Month  = mImportantDateList[i].month;
        //convert tmElements_t to time_t
        time_t event = makeTime(event_anniversary);
        //see if date is in search window
        if (event > now() && event <= now() + search_window_secs)
        {
            serialPrintImportantDate(mImportantDateList[i]);
            //load a pointer to the important date into mDatesToReportList
            const ImportantDate *pointer {&mImportantDateList[i]};
            mDatesToReportList[mQtyImportantDatesToReport] = pointer;
            ++mQtyImportantDatesToReport;
            
        }
    }
}

void Calendar::serialPrintImportantDate(const ImportantDate importantdate)
{
    //print the information to the serial monitor
    Serial.print("Calendar::loadImportantDates: Detected ");
    Serial.print(importantdate.text);
    switch (importantdate.event_type)
    {
    case EVENTTYPE_ANNIVERSARY:
        Serial.print(" anniversary ");
        break;
    case EVENTTYPE_BIRTHDAY:
        Serial.print(" birthday ");
        break;
    case EVENTTYPE_APPOINTMENT:
        Serial.print(" appointment ");
        break;
    case EVENTTYPE_HOLIDAY:
        Serial.print(" holiday ");
        break;
    case MAX_EVENTTYPE:
    default:
        Serial.print(" event error ");
        break;
    };
    Serial.print("on ");
    const byte month_number  {importantdate.month};
    const char *month_string_ptr {getMonthShortName(month_number)};
    Serial.print(month_string_ptr);
    Serial.print(", ");
    Serial.print(importantdate.day);
    const char *day_suffix {getDaySuffix(importantdate.day)};
    Serial.print(day_suffix);
    Serial.println();
}

//==end of Calendar==========================================================================

class MyStateMachine
{
public:
    enum State
    {
        STATE_INIT_SLEEP, //0
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
    const char *start_string {" state set to "};
    const char *finish_string {""};
    liquidcrystali2c.setCursor (0,0);
    switch (mState)
    {
    case STATE_INIT_SLEEP:
    case STATE_INIT_WAKE:
    case STATE_INIT_BALANCED:
    case STATE_INIT_INVERTER_WARM_UP:
    case STATE_INIT_INVERTER_STAGE_ONE:
    case STATE_INIT_INVERTER_STAGE_TWO:
    case STATE_INIT_DAY_CHARGE:
    case STATE_INIT_INVERTER_COOL_DOWN:
        break;
    case STATE_SLEEP:
        finish_string = "sleeping";
        liquidcrystali2c.print("Sleeping");
        break;
    case STATE_WAKE:
        finish_string = "waking";
        liquidcrystali2c.print("Waking  ");
        break;
    case STATE_BALANCED:
        finish_string = "balanced";
        liquidcrystali2c.print("Balanced");
        break;
    case STATE_INVERTER_WARM_UP:
        finish_string = "warm up";
        liquidcrystali2c.print("Warm up ");
        break;
    case STATE_INVERTER_STAGE_ONE:
        finish_string = "stage one inverter";
        liquidcrystali2c.print("Invert1 ");
        break;
    case STATE_INVERTER_STAGE_TWO:
        finish_string = "stage two inverter";
        liquidcrystali2c.print("Invert2 ");
        break;
    case STATE_DAY_CHARGE:
        finish_string = "charging";
        liquidcrystali2c.print("Charging");
        break;
    case STATE_INVERTER_COOL_DOWN:
        finish_string = "inverter cool down";
        liquidcrystali2c.print("Cooldown");
        break;
    case MAX_STATE:
    default:
        break;
    }
    if (finish_string != "")
    {
        Serial.print(start_string);
        Serial.print(finish_string);
    }
}

//==end of MyStateMachine====================================================================

class MySerial
{
private: //variables
    //choose the serial output 
    bool mUseLaptopOperatingVoltage{true};    
public:
    void checkInput();
    void printState(char const *text);
    void printTimestamp();
    void setClock();
    bool usingLaptopOperatingVoltage();
private: //methods
    
    //void print
}myserial;

void MySerial::checkInput()
{
    //check if the user wants to toggle 'c'orrected voltage    
    if (Serial.available())
    {
        const char input {static_cast<char>(Serial.read())};
        if (input == 'c' || input == 'C')
        {
            if (mUseLaptopOperatingVoltage == true)
            {
                mUseLaptopOperatingVoltage = false;
                Serial.println(F("Switched to normal operating voltage."));
            }
            else
            {
                mUseLaptopOperatingVoltage = true;
                Serial.println(F("Switched to laptop operating voltage."));
            }
        }
    }
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

void MySerial::setClock()
{
    if (RTC.read(gRTC_reading))
    {
        setTime (gRTC_reading.Hour,
                 gRTC_reading.Minute,
                 gRTC_reading.Second,
                 gRTC_reading.Day,
                 gRTC_reading.Month,
                 gRTC_reading.Year - 30);
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

bool MySerial::usingLaptopOperatingVoltage()
{
    return mUseLaptopOperatingVoltage;
}

//==end of MySerial=========================================================================

struct Coordinant
{
    byte x;
    byte y;
};

namespace Pin
{                              
    //digital pins
    const byte light_sensor_one         {2};  //brown   wire lower cable              
    const byte light_sensor_two         {3};  //orange wire lower cable
    const byte battery_charger          {11}; //green wire upper cable  
    const byte inverter                 {5};  //blue wire upper cable
    const byte workbench_lighting       {10}; //purple wire upper cable* too fast PWM
    const byte stage_one_inverter_relay {8};  //A
    const byte stage_two_inverter_relay {9};  //S
    //analog pins
    const byte voltage_divider          {A0}; //green  wire lower cable
    const byte potentiometer            {A1}; //yellow wire
}

//==end of namespace Pin======================================================================

class MyLCD
{

//  01234567890123456789 20 x 4        LCD Display
//0|Charging15 SA 13.02v               The number next to 'Charging' is a state change timer
//1|  message display                  'S' indicates 'S'ummer sensor is detecting light
//2|       box                         'A' indicates 'A'utumn sensor is detecting light
//3|Wed, Mar 15 *11:53am               '*' before clock indicates daylight savings time active

private: //variables
    const byte mLCD_Width {20};
public:
    void drawDisplay        ();
    void print              (const char        *string_ptr);
    void print              (const byte         numeral);
    void printDateSuffix    (const byte         day_of_month);
    void printImportantDate (const Calendar::ImportantDate* importantdate);
public:  // <-make this private when old public references are removed
    void printClock         (const TimeElements time,
                             const Coordinant   coordinant,
                             const bool         right_justify);
    void printDate          (const Coordinant   coordinant);
    void printLDRresults    ();
    void updateBacklight    ();
private: //methods
    void centerText         (String &text);
}mylcd;

void MyLCD::drawDisplay()
{
    updateBacklight();
    Coordinant coordinant {13, 3};
    const bool right_justify    {true};
    printClock(gRTC_reading, coordinant, right_justify);
    coordinant = {0, 3};
    printDate(coordinant);
    printLDRresults();
    //myPrintLDRresultsToLCDfunction();
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
    const char *suffix {calendar.getDaySuffix(day_of_month)};
    liquidcrystali2c.print(suffix);
}

void MyLCD::printImportantDate(const Calendar::ImportantDate* importantdate)
{
    //clear top line
    liquidcrystali2c.setCursor(0, 1);
    liquidcrystali2c.print(F("                    "));
    liquidcrystali2c.setCursor(0, 1);
    //format top line
    String topline(importantdate->text); //Paul
    bool use_possessive_suffix {false};
    bool use_day_suffix {false};
    String possessive {"'s"};
    switch (importantdate->event_type)
    {
        case Calendar::EVENTTYPE_ANNIVERSARY:
            use_possessive_suffix = true;
            use_day_suffix        = true;
            break;
        case Calendar::EVENTTYPE_BIRTHDAY:
            use_possessive_suffix = true;
            use_day_suffix        = true;
            break;
        case Calendar::EVENTTYPE_APPOINTMENT:
            break;
        case Calendar::EVENTTYPE_HOLIDAY:
            break;
        case Calendar::MAX_EVENTTYPE:        
        default:
            break;
    }
    if (use_possessive_suffix)
    {
        topline += possessive; //Paul's
    }
    topline += ' ';
    //add day of month
    const byte day_of_month {importantdate->day};
    String day {day_of_month};
    topline += day; //Paul's 50
    //add day suffix
    if (use_day_suffix)
    {
        topline += calendar.getDaySuffix(day_of_month); //Paul's 50th
    }
    
    centerText(topline);
    Serial.print("MyLCD::printImportantDate  topline: ");
    Serial.println(topline);
    
    
}

//MyLCD private methods start here

void MyLCD::centerText(String &text)
{
    //center the text in a 20 char (mLCD_Width) string by
    //padding the front and rear with spaces
    const byte string_length (text.length());
    if (string_length > mLCD_Width)
    {
        //string is too long
        Serial.println("MyLCD::centerText  String is larger than screen width");
        String end {text.substring(mLCD_Width)};
        text.remove(mLCD_Width);
        Serial.print("MyLCD::centerText  Removed \"");
        Serial.print(end);
        Serial.println("\"");
    }
    else if (string_length <= 0)
    {
        //string empty
        Serial.println("MyLCD::centerText  no string error");
    }
    else
    {
        //string good
        const byte extra_chars (mLCD_Width - string_length);
        const byte extra_spaces_in_front (extra_chars / 2);  //remainder dropped
        const byte extra_spaces_in_rear  (extra_chars - extra_spaces_in_front);
        String front {""};
        for (int i = 0; i < extra_spaces_in_front; i++)
        {
            front += '*';
        }
        String rear {""};
        for (int i = 0; i < extra_spaces_in_rear; i++)
        {
            rear += '*';
        }
        text = front + text + rear;
    }
    
}
void MyLCD::printLDRresults()
{
    liquidcrystali2c.setCursor (11,0);
    if (digitalRead(Pin::light_sensor_one))
    { 
        liquidcrystali2c.print("A");
    }
    else
    {
        liquidcrystali2c.print(" ");
    }

    if (digitalRead(Pin::light_sensor_two))
    {
        liquidcrystali2c.print("S");
    }
    else
    {
        liquidcrystali2c.print(" ");
    }
}

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
    liquidcrystali2c.setCursor(coordinant.x, coordinant.y);                                   
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
    liquidcrystali2c.setCursor(coordinant.x, coordinant.y); 
    print(dayShortStr(weekday()));
    print (", ");
    print (calendar.getMonthShortName(gRTC_reading.Month-1));    
    print(" ");
    print((gRTC_reading.Day));
    print(" ");  //this space to clear last digit when month rolls over (31 to 1) 
}

//=========================================================================================================

class CoundownTimer
{
private:
    byte mCounter;
  public:
    CoundownTimer();
    byte getCounter   ();
    void update       ();
    void set          (byte x); 
}coundowntimer;

CoundownTimer::CoundownTimer()
{}

byte CoundownTimer::getCounter()
{
    return mCounter;
}

void CoundownTimer::update()
{
    liquidcrystali2c.setCursor(8, 0);
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

void CoundownTimer::set(byte x)
{
    mCounter = x;
}





class Voltmeter
{
public:  //struct
    struct VoltRecord
    {
        float voltage;
        tmElements_t timestamp;
    };
private: //variables
    const float mLaptopOperatingVoltage     {5.125};
    bool        mIsFirstReadingCompleted    {false};
    VoltRecord  mMin                        {};
    VoltRecord  mMax                        {};
    float       mVoltage                    {0.0}; 
    
public:  //methods
    void       main                ();
    VoltRecord getMin              ();
    VoltRecord getMax              ();
    float      getVoltage          ();
    void       initDailyStatistics ();
    void       readVoltage         ();
private: //methods
    
}voltmeter;

void Voltmeter::main()
{
    //print results to serial 
    Serial.print(mVoltage);
    //print results to LCD
    liquidcrystali2c.setCursor(14, 0);
    liquidcrystali2c.print(mVoltage);
    liquidcrystali2c.setCursor(19, 0);
    if (myserial.usingLaptopOperatingVoltage())
    {
        //print a 'c' instead of a 'v' as a reminder that program is using
        //'c'orrective voltage
        liquidcrystali2c.print("c");
        Serial.print("c ");
    }
    else
    {
        //'v' for voltage
        liquidcrystali2c.print("v");
        Serial.print("v ");
    }
    if (mVoltage > mMax.voltage)
    { 
        mMax.voltage   = mVoltage;
        mMax.timestamp = gRTC_reading;
        Serial.print(" New daily high just set.");
    }
    if (mVoltage < mMin.voltage)
    {
        mMin.voltage   = mVoltage; 
        mMin.timestamp = gRTC_reading;
        Serial.print(" New daily low just set.");
    }   
}

Voltmeter::VoltRecord Voltmeter::getMin()
{
    return mMin;
}

Voltmeter::VoltRecord Voltmeter::getMax()
{
    return mMax;
}

float Voltmeter::getVoltage()
{
    return mVoltage;
}

void Voltmeter::initDailyStatistics()
{
    if (mVoltage)
    {
        mMin.voltage = mVoltage;
        mMin.timestamp = gRTC_reading;
        mMax.voltage = mVoltage;
        mMax.timestamp = gRTC_reading;
    }
    else
    {
        Serial.print("Voltmeter::initDailyStatistics  no voltage detected");
    }
    
}
void Voltmeter::readVoltage()
{
    //The 5 volt Arduino controller cannot read voltage above 5 volts.
    //If you put more than 5 volts on an analog input pin, you will probably
    //ruin your controller. 
    //Instead, measure a smaller voltage from a voltage divider and then multiply that
    //result by a voltage ratio.

    //my voltage divider is constructed as:  
    //150k resistor from positive battery terminal to voltage divider intersection.
    // 47k resistor from negative battery terminal to voltage divider intersection.
    // a wire from the voltage divider intersection to a controller analog pin

    //To establish the voltage ratio:
    //take two measurements with a portable voltmeter                                        
    //DO NOT TAKE THESE READINGS WITH THE USB CABLE ATTACHED, THEY WON'T BE ACCURATE.
    //my battery reading  12.28volts
    //my divider reading    2.9volts
    //Use a calculator to get this number. 12.28 / 2.84 = 4.32
    //Since it is a constant, there is no need to make the controller calculate it every time.
    float         voltage_divider_ratio { 4.32};  
    //if the program displays the wrong voltage, you can fine tune it here.
    const float   error_correction      {-0.07};  //This is the actual adjustment to my
    //own voltmeter.  you would put a 0.00 here unless you have reason to believe
    //your Arduino voltmeter is reading too high, or too low.  I found my error correction by
    //adjusting this number and examining the result
    voltage_divider_ratio += error_correction;
      
    //An important note about errors and operating voltage.  I am using the full operating voltage
    //of my Arduino as a voltage reference for simplicity.  There are other ways to set up a more 
    //accurate voltage reference, but this method works well for me.
    //I power my Arduino with 9 volts, and have a nice even 5 volts measured between my 5 volt
    //pin and my ground pin.  When I plug my Arduino into my laptop for uploading, the voltage changes
    //slightly.  What that means: My program displays the wrong voltage with the USB cable attached.
    //The difference is about 0.2 volts.  I must remind myself to unplug the USB cable after uploading
    //my sketch and before I wonder why the voltage is wrong.
    //Be sure you have a steady power supply to your Arduino (or clone, or other thing 
    //that reads code)
    
    //the range of pin values starts at 0 for 0.0 volts and top out
    //at 1023 when the pin is at full operating voltage
    const int   pin_reading     {analogRead(Pin::voltage_divider)};
    
    const float pin_value       {static_cast<float>(pin_reading)};
    const float max_pin_value   {1024.0};
    const float pin_value_ratio {pin_value / max_pin_value};
    //the pin value ratio is a number between 0.0 and 1.0  It is a decimal percentage
    //of the controller's operating_voltage. 
    float operating_voltage {5.0}; //this is a number you need to measure, my 
    //portable voltmeter mesures 5.0 volts between the 5V pin and ground (with
    //the USB cable unplugged.  See note on my USB cable trouble above.)
    //there is enough information for the micro-controller to read the voltage at the pin
    if (myserial.usingLaptopOperatingVoltage())
    {
        operating_voltage = mLaptopOperatingVoltage;
    }
    
    const float pin_voltage       {pin_value_ratio * operating_voltage};
    //the 'real' voltage is the pin voltage multiplied by the voltage ratio.  
    const float raw_voltage       {pin_voltage * voltage_divider_ratio};
    //set the mVoltage value to raw_voltage if this is the first run.
    if (mIsFirstReadingCompleted == false)
    {
        mVoltage = raw_voltage;
        mIsFirstReadingCompleted = true;
    }

    //My voltmeter gets jumpy when my solar chargers are processing a lot of energy
    //Use the code below to help stabilize a jumpy reading (if needed.)
    //My voltmeter display runs well with a max deviation setting of 0.01
    const float max_deviation {0.01};
    //a lower number produces a more consistent reading, at the
    //expense of reaction time.
    //Explained another way, with a small max_deviation
    //the reading would take longer to drop to 0.0 volts if
    //the battery was disconnected.
    
    //I am not an electronics expert, but my experiments to stabilize
    //my voltmeter reading by using capacitors were ineffective.  This snippet below
    //keeps my number averaged well enough though
    //STABILIZE
    if (raw_voltage > mVoltage)
    {
        mVoltage += max_deviation;
    }
    else if (raw_voltage < mVoltage)
    {
        mVoltage -= max_deviation;
    }
}

//==end of Voltmeter=========================================================================

class MessageManager
{
private: //variables
    byte         mCurrentMessageIndex   {0};
    time_t       mNextMessageTimestamp  {0};
    const time_t mMessageDuration       {5000}; //milliseconds
    const byte   mQtySystemMessages     {0};        
public:  //methods
    void main();
}messagemanager;

void MessageManager::main()
{
    if (mNextMessageTimestamp <= millis())
    { 
        //set up delay for next message
        mNextMessageTimestamp += mMessageDuration;
        //print message
        if (mCurrentMessageIndex < calendar.getQtyImportantDates()) //calendar message
        {
            const Calendar::ImportantDate *importantdate{};
            importantdate = calendar.getImportantDate(mCurrentMessageIndex);
            mylcd.printImportantDate(importantdate);
            //myserial.printImportantDate(importantdate);
        }
        else //system messages
        {
            
        }
        const byte calendar_messages {calendar.getQtyImportantDates()};
        const byte system_messages   {mQtySystemMessages};
        const byte total_messages    (system_messages + calendar_messages);
        //adjust index
        mCurrentMessageIndex++;
        if (mCurrentMessageIndex == total_messages)
        {
            mCurrentMessageIndex = 0;
        }
        
    }
}


//==end of MessageManager====================================================================



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

byte          today_sunrise_hour = 0;
byte          today_sunset_hour = 0;
byte          today_sunrise_minute = 0;
byte          today_sunset_minute = 0;
byte          message_manager_next_message = 1;
// power manager initializations
//new
//byte          machine_state = 4;  //initiate balanced state
//old
byte          inverter_warm_up_timer = 0; //seconds
int           balance_falling_countdown = 0;
int           balance_rising_countdown = 0;


boolean       LDR_data = false;
boolean       LDR2_data = false;
boolean       DST = true;
boolean       message_loaded [3]  = {false,false,false};
byte reminder_message_pointer [3] = {false,false,false};
byte dimmer_reference_number = 0;

const unsigned long seconds_in_a_week = 604800;  

void myClearMessageBoardfunction();
boolean myIsItDaylightfunction();
boolean myIsItDeltaTimePastDawnfunction();
//void myLoadUpcomingEventsfunction();
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
//void myVoltagePrintingAndRecordingfunction();
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
    Serial.println(F("If you are reading this, you are connected to the serial port via"));
    Serial.println(F("USB.  Your laptop raises the operating voltage of your"));
    Serial.println(F("microcontroller.  If you plan to leave this cable plugged in,"));
    Serial.println(F("you need to enter a 'c' in the entry field to switch to 'corrected'"));
    Serial.println(F("operating voltage.  Your LCD will display 12.97c instead of 12.97v"));
    Serial.println(F("as a reminder.  Enter a 'c' in the entry field again to toggle back"));
    Serial.println(F("to normal before you pull the cable.  If you forget, just reset the"));
    Serial.println(F("controller."));
    Serial.println(F("                                                   -Paul 03.22.2019"));
    Wire.begin();                      // start the Wire library
    liquidcrystali2c.begin(20, 4);     // start the LiquidCrystal_I2C library
    myserial.setClock();
    voltmeter.readVoltage();
    voltmeter.initDailyStatistics();
    calendar.loadImportantDates();
    mysetSunriseSunsetTimesfunction();
    //set pins  
    pinMode (Pin::inverter, OUTPUT);
    pinMode (Pin::battery_charger, OUTPUT);
    pinMode (Pin::voltage_divider, INPUT);
    pinMode (Pin::light_sensor_one, INPUT);
    pinMode (Pin::light_sensor_two, INPUT);
    pinMode (Pin::workbench_lighting, OUTPUT);
    pinMode (Pin::stage_one_inverter_relay, OUTPUT);
    pinMode (Pin::stage_two_inverter_relay, OUTPUT);
}
               
void loop()
{
    //This code runs as fast as possible
    myReadPotentiometerAndAdjustWorkbenchTrackLightsfunction();
    voltmeter.readVoltage();
    myserial.checkInput();
    //This code runs every second (1000ms)
    RTC.read(gRTC_reading);  //gathered from library by reference
    if (gLast_RTC_reading.Second != gRTC_reading.Second) 
    {
        gLast_RTC_reading = gRTC_reading;  //set up 1000ms delay for the next loop
        if (RTC.read(gRTC_reading))        //gRTC_reading is gathered by reference
        {
            setTime(gRTC_reading.Hour,gRTC_reading.Minute,gRTC_reading.Second,gRTC_reading.Day,gRTC_reading.Month,gRTC_reading.Year-30);
        }
        coundowntimer.update();
        mylcd.drawDisplay();
        messagemanager.main();
        
        //begin serial report
        myserial.printTimestamp();
        voltmeter.main();
        
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
        case MyStateMachine::MAX_STATE:
        default:
            break;
        }
        
        // This code runs at 2:00am 
        if (gRTC_reading.Hour   == 2 &&
            gRTC_reading.Minute == 0 &&
            gRTC_reading.Second == 0)
        {
            inverter_run_time  = 0;
            voltmeter.initDailyStatistics();            
            calendar.loadImportantDates();
            mysetSunriseSunsetTimesfunction();  
        }
        
        
    Serial.println();
    //end serial report
    }
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
  
  if (now_in_minutes >= sunrise_in_minutes + delta_t) {
    return true; 
  } else {
    return false;
  }
}


////===============================
//void myMessageManagerfunction() {
////=========rewritten 12.3.2017====

  //if (message_manager_next_message <= 1) {
    //myMessageSunrisefunction();
    //message_manager_timestamp    = millis()+3000; 
  //}
  //if (message_manager_next_message == 2) {
    //myMessageWeekNumberfunction();
    //message_manager_timestamp    = millis()+3000;
  //}   
  //if (message_manager_next_message == 3) {
    //myMessageReminderfunction();
    //message_manager_timestamp    = millis()+3000;
  //}
  //if (message_manager_next_message == 4) {
    //myMessageVoltageDailyHighfunction();
    //message_manager_timestamp    = millis()+6000;
  //}
  //if (message_manager_next_message == 5) { 
    //myMessageInverterRunTimefunction();
    //message_manager_timestamp    = millis()+2000;
  //}
  //if (message_manager_next_message == 6) {
    //myMessageUpcomingEventsfunction(1);
    //message_manager_timestamp    = millis()+3000;
  //}
  //if (message_manager_next_message == 7) {
    //myMessageUpcomingEventsfunction(2);
    //message_manager_timestamp    = millis()+3000;  
  //}
  //message_manager_next_message++;
  //if (message_manager_next_message == 8) {
    //message_manager_next_message = 1;   
  //}
//}





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
  
  liquidcrystali2c.print (sunset_minute[calendar.getWeekNumber(gRTC_reading)]);
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
    Voltmeter::VoltRecord high {};
    high = voltmeter.getMax();
    myPrintVoltagetoLCDfunction(2,1,high.voltage);
    liquidcrystali2c.print (F(" @"));
    Coordinant coordinant {11, 1};
    const bool right_justify {false};
    mylcd.printClock(high.timestamp, coordinant, right_justify);


    Voltmeter::VoltRecord low {};
    low = voltmeter.getMin();
    myPrintVoltagetoLCDfunction(2, 2, low.voltage);
    liquidcrystali2c.print (F(" @"));
    coordinant = {11, 2};
    mylcd.printClock(low.timestamp, coordinant, right_justify);   
}

void myMessageWeekNumberfunction() {
//----------------------------------

   myClearMessageBoardfunction();
   liquidcrystali2c.setCursor (0,1);
   //         "12345678901234567890"
   liquidcrystali2c.print (F("  Solar Week"));
   liquidcrystali2c.setCursor (0,2);
   liquidcrystali2c.print (F("        Number "));
   liquidcrystali2c.print (calendar.getWeekNumber(gRTC_reading));   
}

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
     potentiometer_reading = potentiometer_reading + (analogRead (Pin::potentiometer));
  }
  potentiometer_reading = potentiometer_reading / 40;
  if (potentiometer_reading <= dimmer_reference_number - stray_distance) {
    dimmer_reference_number = potentiometer_reading; 
  }
  if (potentiometer_reading >= dimmer_reference_number + stray_distance) {
    dimmer_reference_number = potentiometer_reading; 
  }
  analogWrite (Pin::workbench_lighting, (255 - dimmer_reference_number) * scaling_ratio);  
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

  const byte week_number {calendar.getWeekNumber(gRTC_reading)};
  today_sunrise_hour = sunrise_hour[week_number]-1+daylight_savings_time;
  today_sunrise_minute = sunrise_minute[week_number];
  today_sunset_hour = sunset_hour[week_number]-1+daylight_savings_time;
  today_sunset_minute = sunset_minute[week_number];
}


////-----------------------------------------
//void myVoltagePrintingAndRecordingfunction() {
////-----------------------------------------  
   //myPrintVoltagetoLCDfunction(14,0,voltmeter.getVoltage());                      
   ////Serial.print (stable_voltage); Serial.print ("V "); 
   //if (voltmeter.getVoltage() > voltage_daily_max) { 
    //voltage_daily_max = voltmeter.getVoltage();
    //todays_high_voltage_timestamp = gRTC_reading;
   //}
   //if (voltmeter.getVoltage() < voltage_daily_min) {
     //voltage_daily_min = voltmeter.getVoltage(); 
     //todays_low_voltage_timestamp = gRTC_reading;
   //}
//}

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
  
  digitalWrite (Pin::battery_charger, HIGH);  //battery charger on
  digitalWrite (Pin::inverter, LOW);         //inverter off
  digitalWrite(Pin::stage_one_inverter_relay, LOW);    // relay one off
  digitalWrite(Pin::stage_two_inverter_relay, LOW);    // relay two off
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
  digitalWrite (Pin::battery_charger, LOW);   // battery charger off
  digitalWrite (Pin::inverter, LOW);          // inverter off
  digitalWrite(Pin::stage_one_inverter_relay, LOW);      // relay one off
  digitalWrite(Pin::stage_two_inverter_relay, LOW);      // relay two off
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

  
  digitalWrite (Pin::battery_charger, LOW);  //battery charger off
  digitalWrite (Pin::inverter, LOW);         //inverter off
  digitalWrite(Pin::stage_one_inverter_relay, LOW);    // relay one off
  digitalWrite(Pin::stage_two_inverter_relay, LOW);    // relay two off
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
  
  digitalWrite (Pin::battery_charger, LOW);  //battery charger off
  digitalWrite (Pin::inverter, HIGH);         //inverter on
  digitalWrite(Pin::stage_one_inverter_relay, LOW);    // relay one off
  digitalWrite(Pin::stage_two_inverter_relay, LOW);    // relay two off
  coundowntimer.set(seconds_to_warm_up);
  mystatemachine.setState(MyStateMachine::STATE_INVERTER_WARM_UP);
}



//==================================================
void myStateMachineWarmUpInverterStatefunction() {        //state 7
  //=======================================5.8.2018===

  if (coundowntimer.getCounter()) {    //warming up during countdown
    return;
  }
  mystatemachine.setState(MyStateMachine::STATE_INIT_INVERTER_STAGE_ONE);

}
//======================================================
void myStateMachineInitStageOneInverterStatefunction() {    //state 8
  //===========================================5.8.2018===

  const byte  stage_two_switching_delay = 15;  //seconds.  this prevents stage two from engaging too soon    
  digitalWrite (Pin::battery_charger, LOW);  //battery charger off
  digitalWrite (Pin::inverter, HIGH);         //inverter on
  digitalWrite(Pin::stage_one_inverter_relay, HIGH);    // relay one on
  digitalWrite(Pin::stage_two_inverter_relay, LOW);    // relay two off
  coundowntimer.set(stage_two_switching_delay);
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

  if (voltmeter.getVoltage() >= voltage_to_switch_to_stage_two && !coundowntimer.getCounter()) {
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
  digitalWrite (Pin::battery_charger, LOW);  //battery charger off
  digitalWrite (Pin::inverter, HIGH);         //inverter on
  digitalWrite(Pin::stage_one_inverter_relay, HIGH);    // relay one on
  digitalWrite(Pin::stage_two_inverter_relay, HIGH);    // relay two on
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
  digitalWrite (Pin::battery_charger, HIGH);  //battery charger on
  digitalWrite (Pin::inverter, LOW);         //inverter off
  digitalWrite(Pin::stage_one_inverter_relay, LOW);    // relay one off
  digitalWrite(Pin::stage_two_inverter_relay, LOW);    // relay two off
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
  digitalWrite (Pin::battery_charger, LOW);  //battery charger off
  digitalWrite (Pin::inverter, LOW);         //inverter off
  digitalWrite(Pin::stage_one_inverter_relay, LOW);    // relay one off
  digitalWrite(Pin::stage_two_inverter_relay, LOW);    // relay two off
  coundowntimer.set(inverter_cooldown_time);
  mystatemachine.setState(MyStateMachine::STATE_INVERTER_COOL_DOWN);
}

//================================================
void myStateMachineInverterCooldownfunction() {    //state 15
  //====================================5.11.2018===

  if (!coundowntimer.getCounter()) {
    mystatemachine.setState(MyStateMachine::STATE_INIT_BALANCED);  //init balanced
  }
}
