/* Workshop Power Manager controls my 12 volt solar power system in my workshop.
 * It monitors system voltage and can activate a battery charger or an inverter.
 * It also controls 2, 120 volt circuits and can switch them away from the grid
 * to the inverter as needed.
 *     The hardware is an Arduino Mega, (although an Uno would have been enough.)
 * Attached to that is a battery backed up RTC clock, and a 4 x 20 LCD screen.  These
 * items communicate to the Mega through the i2c protocol.  I also built a 9 volt power
 * supply (drops to 5 volts after the Mega's regulator.)  I also built a voltage
 * divider.  The divider allows my 5 volt Mega to measure up to 20 volts.
 *     In addition to it's primary functions, it runs a small message system, and
 * a track lighting system.  The message system display's system statistics and
 * reminds me of important dates.  I converted the track lighting to 12 volts DC.
 * The controller keeps the LED lights below 12 volts using pulse width modulation.
 * It also reads a potentiometer I am using as a dimmer switch.       
*/

#include <Time.h>
#include <TimeLib.h>
#include <LiquidCrystal_I2C.h>
#include <DS1307RTC.h> //object pre-initialized as 'RTC' in header file 
#include <Wire.h>

LiquidCrystal_I2C  liquidcrystali2c(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  

//globals
tmElements_t       gRTC_reading;
tmElements_t       gLast_RTC_reading;        //to control 1000ms loop
time_t             mDissolveTimestamp  {0};  //for LCD 'dissolve' effect loop

namespace Pin
{                              
    //digital pins
    const byte light_sensor_one         {2};  //brown   wire lower cable              
    const byte light_sensor_two         {3};  //orange wire lower cable
    const byte battery_charger          {11}; //green wire upper cable  
    const byte inverter                 {5};  //blue wire upper cable
    const byte workbench_lighting       {10}; //purple wire upper cable
    const byte stage_one_inverter_relay {8};  //circuit 'A'
    const byte stage_two_inverter_relay {9};  //circuit 'S'
    //analog pins
    const byte voltage_divider          {A0}; //green  wire lower cable
    const byte potentiometer            {A1}; //yellow wire
}

//==end of namespace Pin======================================================================

void myReadPotentiometerAndAdjustWorkbenchTrackLightsfunction();

//all caps indicate a COMPILE TIME CONSTANT 
const byte QTY_IMPORTANT_DATES = 23;  //this must be initialized in global space instead
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
    const char* mMonthShortName[13] = {"-0-", "Jan", "Feb", "Mar", "Apr",
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
        {"Chris",         4,  3, 1990, EVENTTYPE_BIRTHDAY},     //8
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
        {"Christmas",    12, 25, 0,    EVENTTYPE_HOLIDAY},      //22
        {"Paul Crowned King", 3, 28, 2019, EVENTTYPE_APPOINTMENT} //23
    };
    const char* mDaySuffix[4] = {"st", "nd", "rd", "th"};
    //Sometimes there are more than 52 weeks in a year.
    //These tables are close enough to use year after year

    //Why is there a 4:58am in this list when know the sun has never risen at 4:58am?
    //Because this list has not had the Daylight Savings Time + 1 (spring forward)
    //applied yet.  Near summer solstice it does rise at 5:58am!
    const byte sunrise_hour[53]   = { 7,  7,  7,  7,  7,  7,  7,  7,  7,  6,
                                      6,  6,  6,  6,  5,  5,  5,  5,  5,  5,
                                      5,  5,  5,  4,  4,  5,  5,  5,  5,  5,
                                      5,  5,  5,  5,  5,  5,  6,  6,  6,  6,
                                      6,  6,  6,  6,  6,  7,  7,  7,  7,  7,
                                      7,  7,  7}; 
    const byte sunrise_minute[53] = {47, 48, 46, 42, 37, 31, 23, 14,  4, 54,
                                     43, 32, 21, 10, 59, 48, 38, 29, 20, 13,
                                      7,  3,  0, 58, 58,  0,  3,  7, 12, 18,
                                     24, 30, 37, 43, 50, 56,  3,  9, 16, 23,
                                     29, 36, 44, 51, 59,  7, 15, 23, 30, 36,
                                     41, 45, 47};
    const byte sunset_hour[53]    = {17, 17, 17, 17, 17, 17, 17, 18, 18, 18,
                                     18, 18, 18, 18, 18, 19, 19, 19, 19, 19,
                                     19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
                                     19, 19 ,19, 19, 19, 18, 18, 18, 18, 18,
                                     17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
                                     17, 17, 17}; 
    const byte sunset_minute[53]  = {13, 19, 26, 34, 43, 51, 59,  7, 15, 23,
                                     30, 37, 44, 51, 58,  5, 12, 19, 26, 33,
                                     39, 45, 50, 54, 57, 58, 58, 56, 53, 48,
                                     42, 34, 26, 16, 06, 55, 44, 32, 21,  9,
                                     58, 48, 38, 29, 21, 14,  8,  4,  2,  2,
                                      4,  7, 13};

private: //variables continued
    byte mQtyImportantDatesToReport              {};
    ImportantDate const * mDatesToReportList[QTY_IMPORTANT_DATES] {};
    //mDatesToReportList array is large enough to hold pointers to every event if needed.
    byte mTodaySunriseHour   {0};
    byte mTodaySunriseMinute {0};
    byte mTodaySunsetHour    {0};
    byte mTodaySunsetMinute  {0};
    bool mDaylightSavingsTime {true};
public:  //methods
    void           init                    ();
    String         getClockString          (const tmElements_t time,
                                            const bool right_justify = false);
    const char*    getDaySuffix            (byte day_number);
    const ImportantDate* getImportantDate  (const byte index);     
    const char*    getMonthShortName       (const byte month_number);
    String         getSunriseClockString   ();
    String         getSunsetClockString    ();
    byte           getWeekNumber           (tmElements_t date);
    byte           getQtyImportantDates    ();
    bool           isAM                    (const tmElements_t time);
    bool           isDaylight              ();
    bool           isWakeUpComplete        ();
    void           loadImportantDates      ();
    void           serialPrintImportantDate(const ImportantDate importantdate);
    void           setSunriseSunset        ();

}calendar;

void Calendar::init()
{
    loadImportantDates();
    setSunriseSunset();
}

String Calendar::getClockString(const tmElements_t time, const bool right_justify)
{
    int  format {0};
    String clock_string ("");
    //change to 12 hour format
    if ((time.Hour) >= 12)
    {      
        format = 12;
    }
    if ((time.Hour) == 12 || (time.Hour) == 0)
    {
        clock_string = "12";
    }
    else
    {
        //if a single digit, add a space
        if (time.Hour - format < 10 && right_justify == true)
        {             
            clock_string = " ";
        }
        const String hour_string (time.Hour - format); 
        clock_string += hour_string;
    }   
    clock_string += ":";
    //if a single digit, add a zero
    if (time.Minute < 10)
    {  
        clock_string += "0";
    }
    const String minute_string {time.Minute};   
    clock_string += minute_string;
    if (calendar.isAM(time))
    {
        clock_string += "am";
    }
    else
    {                          
        clock_string += "pm";
    }
    return clock_string;    
}

bool Calendar::isWakeUpComplete()
{
    const int delay = 15;  //minutes 
    //The wake up delay ensures that the the inverter does not burn off the fresh
    //battery charger energy.

    const int sunrise_minutes = (mTodaySunriseHour * 60) + mTodaySunriseMinute;
    const int now_minutes     = (hour() * 60)            + minute();  
    if (now_minutes >= sunrise_minutes + delay)
    {
        return true; 
    }
    else
    {
        return false;
    }
}


const char* Calendar::getDaySuffix(byte day_number)
{
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

String Calendar::getSunriseClockString()
{
    tmElements_t sunrise {0};
    sunrise.Hour   = mTodaySunriseHour;
    sunrise.Minute = mTodaySunriseMinute;
    return getClockString(sunrise);
}

String Calendar::getSunsetClockString()
{
    tmElements_t sunset {0};
    sunset.Hour   = mTodaySunsetHour;
    sunset.Minute = mTodaySunsetMinute;
    return getClockString(sunset);
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

bool Calendar::isAM(const tmElements_t time)
{
    //the moment after the clock hits 12 noon the time is post merīdiem
    if (time.Hour < 12)
    {
        return true;
    }
    else
    {
        return false;
    }      
}

bool Calendar::isDaylight()
{
    const int sunrise_minutes {(mTodaySunriseHour * 60) + mTodaySunriseMinute};
    const int sunset_minutes  {(mTodaySunsetHour  * 60) + mTodaySunsetMinute};
    const int now_minutes     {(hour()            * 60) + minute()};
    if (now_minutes >= sunrise_minutes &&
        now_minutes <  sunset_minutes)
    {
        return true; 
    }
    else
    {
        return false;
    }
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

void Calendar::setSunriseSunset()
{
    const int week_number {getWeekNumber(gRTC_reading)};
    mTodaySunriseHour   = sunrise_hour[week_number] + mDaylightSavingsTime;
    mTodaySunriseMinute = sunrise_minute[week_number];
    mTodaySunsetHour    = sunset_hour[week_number] + mDaylightSavingsTime;
    mTodaySunsetMinute  = sunset_minute[week_number]; 
}

//==end of Calendar==========================================================================


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

class MyLCD
{

//  01234567890123456789 20 x 4        LCD Display
//0|Charging15 SA 13.02v               The number next to 'Charging' is a state change timer
//1|  message display                  'S' indicates 'S'ummer sensor is detecting light
//2|       box                         'A' indicates 'A'utumn sensor is detecting light
//3|Wed, Mar 15 *11:53am               '*' before clock indicates daylight savings time active

private: //variables
    const byte mLCD_Width {20};
    String mMessageTopLine    {""};
    String mMessageBottomLine {""};
    byte   mDissolveCountdown {0};
public:
    void   drawDisplay        ();
    void   dissolveEffect     ();
    void   dissolveThis       (String top_line, String bottom_line);
    void   printDateSuffix    (const byte         day_of_month);
    void   printImportantDate (const Calendar::ImportantDate* importantdate);
    void   printClock         (const TimeElements time,
                               const Coordinant   coordinant,
                               const bool         right_justify);
    void   printDate          (const Coordinant   coordinant);
    void   printLDRresults    ();
    void   updateBacklight    ();
    String centerText         (const String text);
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
}

void MyLCD::dissolveEffect()
{
    const int top_line_row    {1}; //messages start in row 1
    const int bottom_line_row {2};
    if (mDissolveCountdown) //range: mLCD_Width to 1
    {
        --mDissolveCountdown;//mLCD_Width - 1 to 1 - 1  = range: (19 to 0)
        //top line dissolves from right to left
        const char top_char {mMessageTopLine.charAt(mDissolveCountdown)};
        liquidcrystali2c.setCursor(mDissolveCountdown, top_line_row);
        liquidcrystali2c.print(top_char);
        //bottom line dissolves from left to right
        //invert counter
        const int bottom_index {(mLCD_Width - 1) - mDissolveCountdown}; //= range: (0 - 19)
        const char bottom_char {mMessageBottomLine.charAt(bottom_index)};
        liquidcrystali2c.setCursor(bottom_index, bottom_line_row);
        liquidcrystali2c.print(bottom_char);        
    }
}

void MyLCD::dissolveThis(String top_line, String bottom_line)
{
    const String top    {centerText(top_line)};
    const String bottom {centerText(bottom_line)};
    mMessageTopLine    = top;
    mMessageBottomLine = bottom;
    mDissolveCountdown = mLCD_Width; //this triggers the dissolve
    //Serial diagnostics
    Serial.println("MyLCD::dissolveThis");
    Serial.println(mMessageTopLine);
    Serial.println(mMessageBottomLine);
}

void MyLCD::printDateSuffix(byte day_of_month)
{
    const char *suffix {calendar.getDaySuffix(day_of_month)};
    liquidcrystali2c.print(suffix);
}

void MyLCD::printImportantDate(const Calendar::ImportantDate* importantdate)
{
    //format top line
    String top_line            (importantdate->text); //Paul
    switch (importantdate->event_type)
    {
        case Calendar::EVENTTYPE_ANNIVERSARY:
        case Calendar::EVENTTYPE_BIRTHDAY:
            {
                top_line += "'s "; //Paul & Mena's
                byte anniversary (year() - importantdate->year);
                //fix end of year wrap around
                if (month() == 12 && importantdate->month != 12)
                {
                    ++anniversary; 
                }
                const String anniversary_str {anniversary};
                top_line += anniversary_str; //Paul & Mena's 27
                const String anniversary_date_suffix {calendar.getDaySuffix(anniversary)};
                top_line += anniversary_date_suffix; //Paul & Mena's 27th 
            }
            break;
        case Calendar::EVENTTYPE_APPOINTMENT: //Paul dentist
        case Calendar::EVENTTYPE_HOLIDAY: //Christmas
        case Calendar::MAX_EVENTTYPE:        
        default:
            break;
    }
    //format bottom line
    String bottom_line ("");
    if (importantdate->day == day())
    {
        bottom_line = "today";
    }
    else if (importantdate->day == day() + 1)
    {
        bottom_line = "tomorrow";
    }
    else
    {
        tmElements_t event {};
        event.Day   = importantdate->day;
        event.Month = importantdate->month;
        event.Year  = year();
        time_t event_unix {makeTime(event)};
        const byte day_of_week (weekday(event_unix));
        String day_str {dayShortStr(day_of_week)};
        bottom_line = day_str + ", "; //Wed,
        String month_str {calendar.getMonthShortName(event.Month)};
        bottom_line += month_str; //Wed, Sep 
        bottom_line += ' ';
        String date_str {event.Day};  
        bottom_line += date_str; //Wed, Sep 3
        String date_suffix {calendar.getDaySuffix(event.Day)};
        bottom_line += date_suffix; //Wed, Sep 3rd
    }
    dissolveThis(top_line, bottom_line);
}

//MyLCD private methods start here

String MyLCD::centerText(const String text)
{
    //center the text in a 20 char (mLCD_Width) string by
    //padding the front and rear with spaces
    const byte string_length (text.length());
    if (string_length > mLCD_Width)
    {
        //string is too long
        Serial.println("MyLCD::centerText  String is larger than screen width");
        const String end {text.substring(mLCD_Width)};
        const String front {text.substring(0, mLCD_Width - 1)};
        Serial.print("MyLCD::centerText  Removed \"");
        Serial.print(end);
        Serial.println("\"");
        return front;
    }
    else if (string_length <= 0)
    {
        //string empty
        Serial.println("MyLCD::centerText  no string error");
        //      01234567890123456789
        return "---missing string---";
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
            front += ' '; //add a space
        }
        String rear {""};
        for (int i = 0; i < extra_spaces_in_rear; i++)
        {
            rear += ' '; //add a space
        }
        return front + text + rear;
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

void MyLCD::printClock(const TimeElements time,
                       const Coordinant coordinant,
                       const bool right_justify)
{
    const String clock_string {(calendar.getClockString(time, right_justify))};
    liquidcrystali2c.setCursor(coordinant.x, coordinant.y);
    liquidcrystali2c.print(clock_string);
}

void MyLCD::printDate(const Coordinant coordinant)
{
    liquidcrystali2c.setCursor(coordinant.x, coordinant.y); 
    liquidcrystali2c.print(dayShortStr(weekday()));
    liquidcrystali2c.print (", ");
    liquidcrystali2c.print (calendar.getMonthShortName(gRTC_reading.Month));    
    liquidcrystali2c.print(" ");
    liquidcrystali2c.print((gRTC_reading.Day));
    liquidcrystali2c.print(" ");  //this space to clear last digit when month rolls over (31 to 1) 
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
            liquidcrystali2c.print(" "); // clear the first space if a single digit
        }
        liquidcrystali2c.print(mCounter);                 // print the counter
        mCounter--;                          // counter decrements here
    }
    else
    {
        liquidcrystali2c.print("  ");             // no counter, so clear the board
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
    State mState           {STATE_INIT_BALANCED};
    int   mInverterRunTime {0};
public:  //methods
    void  main                ();
    State getState            ();
    int   getInverterRunTime  ();
    void  resetInverterRunTime();
    void  setState            (const State);
private: //methods
    void initSleepStatefunction();
    void sleepStatefunction();
    void initWakeStatefunction();
    void wakeStatefunction();
    void initBalancedStatefunction();
    void balancedStatefunction();
    void initWarmUpInverterStatefunction();
    void warmUpInverterStatefunction();
    void initStageOneInverterStatefunction();
    void stageOneInverterStatefunction();
    void initStageTwoInverterStatefunction();
    void stageTwoInverterStatefunction();
    void initDaytimeChargingfunction();
    void daytimeChargingfunction();
    void initInverterCooldownfunction();
    void inverterCooldownfunction();
    
    
}mystatemachine;

void  MyStateMachine::main()
{
    switch (getState())
    {
    case STATE_INIT_SLEEP:
        initSleepStatefunction();
        break;
    case STATE_SLEEP:
        sleepStatefunction();
        break;
    case STATE_INIT_WAKE:
        initWakeStatefunction();
        break;
    case STATE_WAKE:
        wakeStatefunction();
        break;
    case STATE_INIT_BALANCED:
        initBalancedStatefunction();
        break;
    case STATE_BALANCED:
        balancedStatefunction();
        break;
    case STATE_INIT_INVERTER_WARM_UP:
        initWarmUpInverterStatefunction();
        break;
    case STATE_INVERTER_WARM_UP:
        warmUpInverterStatefunction();
        break;
    case STATE_INIT_INVERTER_STAGE_ONE:
        initStageOneInverterStatefunction();
        break;
    case STATE_INVERTER_STAGE_ONE:
        stageOneInverterStatefunction();
        break;
    case STATE_INIT_INVERTER_STAGE_TWO:
        initStageTwoInverterStatefunction();
        break;
    case STATE_INVERTER_STAGE_TWO:
        stageTwoInverterStatefunction();
        break;
    case STATE_INIT_DAY_CHARGE:
        initDaytimeChargingfunction();
        break;
    case STATE_DAY_CHARGE:
        daytimeChargingfunction();
        break;
    case STATE_INIT_INVERTER_COOL_DOWN:
        initInverterCooldownfunction();
        break;
    case STATE_INVERTER_COOL_DOWN:
        inverterCooldownfunction();
        break;
    case MAX_STATE:
    default:
        break;
    }
}

MyStateMachine::State MyStateMachine::getState()
{
    return mState;
}

int MyStateMachine::getInverterRunTime()
{
    return mInverterRunTime;
}

void MyStateMachine::resetInverterRunTime()
{
    mInverterRunTime = 0;
}

void MyStateMachine::setState(State state)
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

void MyStateMachine::initSleepStatefunction()
{
    digitalWrite (Pin::battery_charger, HIGH);            //battery charger on
    digitalWrite (Pin::inverter, LOW);                    //inverter off
    digitalWrite (Pin::stage_one_inverter_relay, LOW);    // relay one off
    digitalWrite (Pin::stage_two_inverter_relay, LOW);    // relay two off
    setState(STATE_SLEEP);
}


void MyStateMachine::sleepStatefunction()
{
    //switch to initiate wake state if light
    if (calendar.isDaylight() == true)
    {        
        setState(STATE_INIT_WAKE);
    }
}

void MyStateMachine::initWakeStatefunction()
{
    digitalWrite (Pin::battery_charger, LOW);            // battery charger off
    digitalWrite (Pin::inverter, LOW);                   // inverter off
    digitalWrite (Pin::stage_one_inverter_relay, LOW);   // relay one off
    digitalWrite (Pin::stage_two_inverter_relay, LOW);   // relay two off
    setState(STATE_WAKE);
}

void MyStateMachine::wakeStatefunction()
{
    //check to see if it is time to go back to sleep
    if (calendar.isDaylight() == false)
    {
        setState(STATE_INIT_SLEEP); //initiate sleep state
    }
    //check to see how long it is past dawn.
    //if it is delta_t, or 15 minutes, past dawn switch to state 4 (init balanced)
    //The wake up delay ensures that the the inverter does not burn off the fresh battery charger energy
    else if (calendar.isWakeUpComplete() == true)
    {
        setState(STATE_INIT_BALANCED);  //initiate balanced
    }
}

void MyStateMachine::initBalancedStatefunction()
{
    digitalWrite (Pin::battery_charger, LOW);          //battery charger off
    digitalWrite (Pin::inverter, LOW);                 //inverter off
    digitalWrite (Pin::stage_one_inverter_relay, LOW); // relay one off
    digitalWrite (Pin::stage_two_inverter_relay, LOW); // relay two off
    setState(STATE_BALANCED);                          // balanced initialization complete
}

void MyStateMachine::balancedStatefunction()
{
    const float voltage_to_start_inverter = 13.80;
    const float voltage_to_start_daytime_charging = 12.60;
    //switch to initiate sleep mode if dark
    if (calendar.isDaylight() == false)
    {
        setState(STATE_INIT_SLEEP);
    }
    else if (voltmeter.getVoltage() >= voltage_to_start_inverter)
    {
        setState(STATE_INIT_INVERTER_WARM_UP); 
    }
    else if (voltmeter.getVoltage() <= voltage_to_start_daytime_charging)
    {
        setState(STATE_INIT_DAY_CHARGE);
    }
}

void MyStateMachine::initWarmUpInverterStatefunction()
{
    const int seconds_to_warm_up {4};
    digitalWrite (Pin::battery_charger, LOW);  //battery charger off
    digitalWrite (Pin::inverter, HIGH);         //inverter on
    digitalWrite(Pin::stage_one_inverter_relay, LOW);    // relay one off
    digitalWrite(Pin::stage_two_inverter_relay, LOW);    // relay two off
    coundowntimer.set(seconds_to_warm_up);
    setState(STATE_INVERTER_WARM_UP);
}

void MyStateMachine::warmUpInverterStatefunction()
{
    if (coundowntimer.getCounter())
    {    
        //warming up during countdown
        return;
    }
    else
    {
        setState(STATE_INIT_INVERTER_STAGE_ONE);
    }
}

void MyStateMachine::initStageOneInverterStatefunction()
{
    const int stage_two_switching_delay {15};
    digitalWrite (Pin::battery_charger, LOW);           //battery charger off
    digitalWrite (Pin::inverter, HIGH);                 //inverter on
    digitalWrite (Pin::stage_one_inverter_relay, HIGH); // relay one on
    digitalWrite (Pin::stage_two_inverter_relay, LOW);  // relay two off
    coundowntimer.set(stage_two_switching_delay);
    setState(STATE_INVERTER_STAGE_ONE);
}

void MyStateMachine::stageOneInverterStatefunction()
{
    const float voltage_to_turn_inverter_off   {12.55};
    const float voltage_to_switch_to_stage_two {13.80};
    mInverterRunTime++;
    //switch to initiate sleep mode if dark (unlikely)
    if (calendar.isDaylight() == false)
    {        
        setState(STATE_INIT_SLEEP);
    }
    else if (voltmeter.getVoltage() >= voltage_to_switch_to_stage_two &&
             !coundowntimer.getCounter())
    {
        setState(STATE_INIT_INVERTER_STAGE_TWO);  
    }
    else if (voltmeter.getVoltage() <= voltage_to_turn_inverter_off)
    {
        setState(STATE_INIT_INVERTER_COOL_DOWN);
    }

}

void MyStateMachine::initStageTwoInverterStatefunction()
{
    digitalWrite(Pin::battery_charger, LOW);           //battery charger off
    digitalWrite(Pin::inverter, HIGH);                 //inverter on
    digitalWrite(Pin::stage_one_inverter_relay, HIGH); // relay one on
    digitalWrite(Pin::stage_two_inverter_relay, HIGH); // relay two on
    setState(STATE_INIT_INVERTER_STAGE_TWO);
}


void MyStateMachine::stageTwoInverterStatefunction()
{
    const float voltage_to_drop_back_to_stage_one {12.70};
    //switch to initiate sleep mode if dark
    if (calendar.isDaylight() == false)
    {        
        setState(STATE_INIT_SLEEP);
    }
    else if (voltmeter.getVoltage() <= voltage_to_drop_back_to_stage_one)
    {
        setState(STATE_INIT_INVERTER_STAGE_ONE);
    }
    mInverterRunTime++;
}

void MyStateMachine::initDaytimeChargingfunction()
{
    digitalWrite(Pin::battery_charger, HIGH);         //battery charger on
    digitalWrite(Pin::inverter, LOW);                 //inverter off
    digitalWrite(Pin::stage_one_inverter_relay, LOW); //relay one off
    digitalWrite(Pin::stage_two_inverter_relay, LOW); //relay two off
    setState(STATE_DAY_CHARGE);
}

void MyStateMachine::daytimeChargingfunction()
{
    const float voltage_to_switch_off_charger {13.40};
    if (voltmeter.getVoltage() >= voltage_to_switch_off_charger)
    {
        setState(STATE_INIT_BALANCED);
    }
}

void MyStateMachine::initInverterCooldownfunction()
{
    const int inverter_cooldown_time {30};
    digitalWrite(Pin::battery_charger, LOW);          //battery charger off
    digitalWrite(Pin::inverter, LOW);                 //inverter off
    digitalWrite(Pin::stage_one_inverter_relay, LOW); //relay one off
    digitalWrite(Pin::stage_two_inverter_relay, LOW); //relay two off
    coundowntimer.set(inverter_cooldown_time);
    setState(STATE_INVERTER_COOL_DOWN);
}

void MyStateMachine::inverterCooldownfunction()
{
    if (!coundowntimer.getCounter())
    {
        setState(STATE_INIT_BALANCED);
    }
}
//==end of MyStateMachine====================================================================

class MessageManager
{
private: //variables
    byte         mCurrentMessageIndex   {0};
    time_t       mNextMessageTimestamp  {0};
    const time_t mMessageDuration       {5000}; //milliseconds
    const byte   mQtySystemMessages     {5};        
public:  //methods
    void main();
    void messageInverterRunTime();
    void sunriseSunsetMessage();
    void voltageExtremesMessage();
}messagemanager;

void MessageManager::main()
{
    if (mNextMessageTimestamp <= millis())
    { 
        //set up delay for next message
        mNextMessageTimestamp += mMessageDuration;
        const byte calendar_messages {calendar.getQtyImportantDates()};
        const byte system_messages   {mQtySystemMessages};
        const byte total_messages    (system_messages + calendar_messages);
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
            const int sytem_message_index {mCurrentMessageIndex - calendar_messages};
            switch (sytem_message_index)
            {
                case 0: //voltage record high and low
                    voltageExtremesMessage();
                    break;
                case 1:
                    sunriseSunsetMessage();
                    break;
                case 2:
                    Serial.println(sytem_message_index);
                    break;
                case 3:
                    Serial.println(sytem_message_index);
                    break;
                case 4:
                    Serial.println(sytem_message_index);
                    break;
                //case 5:
                    //Serial.println(sytem_message_index);
                    //break;
                default:
                    break;
            }            
        }
        //adjust index
        mCurrentMessageIndex++;
        if (mCurrentMessageIndex == total_messages)
        {
            mCurrentMessageIndex = 0;
        }
    }
}

void MessageManager::messageInverterRunTime()
{
    const int inverter_run_time {mystatemachine.getInverterRunTime()};
    String top_line    {"Inverter harvested"};
    String bottom_line {""};
    const int one_minute     {60};
    const int one_hour       {one_minute * 60};
    if (inverter_run_time == 0)
    {
        top_line    = "Inverter waiting";
        bottom_line = "for surplus energy";
    }
    //seconds
    else if (inverter_run_time < one_minute)
    {
        const String seconds     {inverter_run_time};
        bottom_line = seconds + " second";
        if (inverter_run_time > 1)
        {
            bottom_line += 's';
        }
         
    }
    else if (inverter_run_time < one_hour)
    {
        const int minutes        {inverter_run_time / one_minute};
        const String minutes_str {minutes};
        bottom_line = minutes_str + " minute";
        if (minutes > 1)
        {
            bottom_line += 's';
        }
        
    }
    else if (inverter_run_time >= one_hour)
    {
        const int  run_minutes        {inverter_run_time / one_minute};
        const int  minutes_in_an_hour {60};
        const int  run_hours          {run_minutes / minutes_in_an_hour};
        const int  remainder_minutes  {run_minutes - (run_hours * minutes_in_an_hour)};
        const String run_hours_string {run_hours};
        bottom_line = run_hours_string + " hour"; 
        if (run_hours > 1)
        {
            bottom_line += 's';
        }
        bottom_line += ' ';
        const String run_minutes_string {remainder_minutes};
        bottom_line += run_minutes_string;
        bottom_line += " minute";
        if (remainder_minutes > 1)
        {
            bottom_line += 's';
        }
    }    
    mylcd.dissolveThis(top_line, bottom_line);
}


void MessageManager::sunriseSunsetMessage()
{
    const String sunrise_clock_string {calendar.getSunriseClockString()};
    const String top_line {"Sunrise " + sunrise_clock_string};
    const String sunset_clock_string {calendar.getSunsetClockString()};
    const String bottom_line {"Sunset  " + sunset_clock_string};
    mylcd.dissolveThis(top_line, bottom_line);
}

void MessageManager::voltageExtremesMessage()
{
    const int qty_voltrecord {2};
    const Voltmeter::VoltRecord voltrecord[qty_voltrecord] {voltmeter.getMin(),
                                                            voltmeter.getMax()};
    String message[qty_voltrecord] {""};
    Serial.println("Voltage extremes:");                                                            
    for (int i = 0; i < qty_voltrecord; i++)
    {
        const String voltage_string  {voltrecord[i].voltage};
        const String clock_string    {calendar.getClockString(voltrecord[i].timestamp)};
        message[i] = voltage_string + "v @ " + clock_string;
    }
    mylcd.dissolveThis(message[0], message[1]);
}

//==end of MessageManager====================================================================


byte          inverter_warm_up_timer = 0; //seconds
int           balance_falling_countdown = 0;
int           balance_rising_countdown = 0;


boolean       LDR_data = false;
boolean       LDR2_data = false;
byte dimmer_reference_number = 0;

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
    Serial.println();
    Wire.begin();                      // start the Wire library
    liquidcrystali2c.begin(20, 4);     // start the LiquidCrystal_I2C library
    myserial.setClock();
    voltmeter.readVoltage();
    voltmeter.initDailyStatistics();
    calendar.init();
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
    //This code runs every gDissolveInterval 
    const time_t gDissolveInterval {50}; //milliseconds 
    if (millis() - gDissolveInterval  >= mDissolveTimestamp)
    {
        mDissolveTimestamp = millis(); //set up delay for next loop
        mylcd.dissolveEffect();
    }
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
        mystatemachine.main();
        
        // This code runs at 2:00am 
        if (gRTC_reading.Hour   == 2 &&
            gRTC_reading.Minute == 0 &&
            gRTC_reading.Second == 0)
        {
            mystatemachine.resetInverterRunTime();
            voltmeter.initDailyStatistics();            
            calendar.init();
        }
    Serial.println(); //end serial report, new line
    }
}


//}
////*******************************
//void myMessageSunrisefunction() {
////*******************************
  ////myClearMessageBoardfunction();
  ////liquidcrystali2c.setCursor (0,1);
  //////           "12345678901234567890"
  ////liquidcrystali2c.print (F("   Sunrise "));
  ////liquidcrystali2c.print (today_sunrise_hour);
  ////liquidcrystali2c.print (F(":"));
  ////if (today_sunrise_minute <= 9){
   ////liquidcrystali2c.print(F("0"));
  ////}
  ////liquidcrystali2c.print (today_sunrise_minute);
  ////liquidcrystali2c.print (F("am"));
  ////liquidcrystali2c.setCursor (0,2);
  //////           "12345678901234567890"
  ////liquidcrystali2c.print (F("   Sunset  ")); 
  ////liquidcrystali2c.print (today_sunset_hour - 12);
  ////liquidcrystali2c.print (F(":"));
  ////if (today_sunset_minute <= 9){
   ////liquidcrystali2c.print(F("0"));
  ////}
  
  ////liquidcrystali2c.print (sunset_minute[calendar.getWeekNumber(gRTC_reading)]);
  ////liquidcrystali2c.print (F("pm"));
//}


//void myMessageWeekNumberfunction() {
////----------------------------------

   //myClearMessageBoardfunction();
   //liquidcrystali2c.setCursor (0,1);
   ////         "12345678901234567890"
   //liquidcrystali2c.print (F("  Solar Week"));
   //liquidcrystali2c.setCursor (0,2);
   //liquidcrystali2c.print (F("        Number "));
   //liquidcrystali2c.print (calendar.getWeekNumber(gRTC_reading));   
//}

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
  }
