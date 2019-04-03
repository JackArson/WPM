//Copyright (C) 2019 Paul R Bailey  aka  'Jack Arson'
//Written on a Raspberry Pi (Linux) using Arduino-1.8.9
//also runs fine on Windows with Arduino-1.8.8

/* Workshop Power Manager controls my 12 volt solar power system in my workshop.
 * It monitors system voltage and can activate a battery charger or an inverter.
 * It also controls 2, 120 volt circuits and can switch them away from the grid
 * to the inverter as needed.
 *     The hardware is an Arduino Mega, (although an Uno would have been enough.)
 * Attached to that is a battery backed up RTC clock, and a 4 x 20 LCD screen.  These
 * items communicate to the Mega through the i2c protocol.  I also built a 9 volt power
 * supply (drops to 5 volts after the Mega's regulator.)  I also built a voltage
 * divider.  The divider allows my 5 volt Mega to measure up to 20 volts.
 *     In addition to it's primary functions, it runs a small message system, a
 * a track lighting system, and it reads two LDR's (light-dependent resistors.)
 * The message system display's system statistics and reminds me of important dates.
   I converted the track lighting to 12 volts DC and bought 12 volt DC LED light bulbs
   for it.  The controller keeps the LED lights below 12 volts using pulse width modulation.
 * It also reads a potentiometer I am using as a dimmer switch.  Finally, the LDR display
 * serves to alert me to any switching problems with my opto-coupler equipped circuit
 * swapping relays.
*/

#include <Time.h>
#include <TimeLib.h>
#include <LiquidCrystal_I2C.h>
#include <DS1307RTC.h> //object pre-initialized as 'RTC' in header file 
#include <Wire.h>

//create liquidcrystali2c object
LiquidCrystal_I2C  liquidcrystali2c(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  

namespace Pin
{                              
    //digital pins
    const byte circuit_sensor_one       {2};  //brown   wire lower cable              
    const byte circuit_sensor_two       {3};  //orange wire lower cable
    const byte battery_charger          {11}; //green wire upper cable  
    const byte inverter                 {5};  //blue wire upper cable
    const byte workbench_lighting       {10}; //purple wire upper cable
    const byte stage_one_inverter_relay {8};  //circuit 'A'
    const byte stage_two_inverter_relay {9};  //circuit 'S'
    //analog pins
    const byte voltage_divider          {A0}; //green  wire lower cable
    const byte dimmer_potentiometer     {A1}; //yellow wire
}
//==end of namespace Pin======================================================================

class TimeNow
{
private: //variables
    tmElements_t mRTC_Reading;             //the current time
public:  //methods
    void         changeYear            (int year);
    void         changeMonth           (int month);
    void         changeDay             (int day);
    void         changeHour            (int hour);
    void         changeMinute          (int minute);
    void         changeSecond          (int second);
    tmElements_t getElements           ();
    int          getYear               ();
    int          getMonth              ();
    int          getDay                ();
    int          getHour               ();
    int          getMinute             ();
    int          getSecond             ();
    void         syncTimeWithRTC_Clock ();
}timenow;

void TimeNow::changeYear(int year)
{
    //make a copy of current reading
    tmElements_t current_RTC_reading {mRTC_Reading};
    //change second
    current_RTC_reading.Year = year;
    //convert to time_t
    time_t new_RTC_reading {makeTime(current_RTC_reading)};
    RTC.set(new_RTC_reading);
    syncTimeWithRTC_Clock();
}

void TimeNow::changeMonth(int month)
{
    constrain(month, 1, 12);
    //make a copy of current reading
    tmElements_t current_RTC_reading {mRTC_Reading};
    //change second
    current_RTC_reading.Month = month;
    //convert to time_t
    time_t new_RTC_reading {makeTime(current_RTC_reading)};
    RTC.set(new_RTC_reading);
    syncTimeWithRTC_Clock();
}


void TimeNow::changeDay(int day)
{
    constrain(day, 0, 31);
    //make a copy of current reading
    tmElements_t current_RTC_reading {mRTC_Reading};
    //change second
    current_RTC_reading.Day = day;
    //convert to time_t
    time_t new_RTC_reading {makeTime(current_RTC_reading)};
    RTC.set(new_RTC_reading);
    syncTimeWithRTC_Clock();
}

void TimeNow::changeHour(int hour)
{
    constrain(hour, 0, 23);
    //make a copy of current reading
    tmElements_t current_RTC_reading {mRTC_Reading};
    //change hour
    current_RTC_reading.Hour = hour;
    //convert to time_t
    time_t new_RTC_reading {makeTime(current_RTC_reading)};
    RTC.set(new_RTC_reading);
    syncTimeWithRTC_Clock();
}

void TimeNow::changeMinute(int minute)
{
    constrain(minute, 0, 59);
    //make a copy of current reading
    tmElements_t current_RTC_reading {mRTC_Reading};
    //change minute
    current_RTC_reading.Minute = minute;
    //convert to time_t
    time_t new_RTC_reading {makeTime(current_RTC_reading)};
    RTC.set(new_RTC_reading);
    syncTimeWithRTC_Clock();
}

void TimeNow::changeSecond(int second)
{
    constrain(second, 0, 59);
    //make a copy of current reading
    tmElements_t current_RTC_reading {mRTC_Reading};
    //change second
    current_RTC_reading.Second = second;
    //convert to time_t
    time_t new_RTC_reading {makeTime(current_RTC_reading)};
    RTC.set(new_RTC_reading);
    syncTimeWithRTC_Clock();
}

tmElements_t TimeNow::getElements()
{
    return mRTC_Reading;
}

int TimeNow::getYear()
{
    return mRTC_Reading.Year;
}

int TimeNow::getMonth()
{
    return mRTC_Reading.Month;
}

int TimeNow::getDay()
{
    return mRTC_Reading.Day;
}

int TimeNow::getHour()
{
    return mRTC_Reading.Hour;
}

int TimeNow::getMinute()
{
    return mRTC_Reading.Minute;
}

int TimeNow::getSecond()
{
    return mRTC_Reading.Second;
}


void TimeNow::syncTimeWithRTC_Clock()
{
    if (RTC.read(mRTC_Reading))
    {
        setTime(RTC.get());        
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
//==end of TimeNow=============================================================================

//QTY_IMPORTANT_DATES is a COMPILE TIME CONSTANT 
const byte QTY_IMPORTANT_DATES = 23;  //this must be initialized in global space instead
                                      //of inside Calendar.  
class Calendar
{
public:
    enum DST_Action
    {
        DST_ACTION_NONE,
        DST_ACTION_CLOCK_FELL_BACK,
        DST_ACTION_CLOCK_SPRUNG_FORWARD,
        MAX_DST_ACTION
    };
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
    //const char* mMonthShortName[13] =    {"", "Jan", "Feb", "Mar", "Apr",
                                              //"May", "Jun", "Jul", "Aug",
                                              //"Sep", "Oct", "Nov", "Dec"};
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
        {"Paul's Coronation", 4, 3, 2019, EVENTTYPE_APPOINTMENT} //23
    };
    const char* mDaySuffix[4] = {"th", "st", "nd", "rd"};
    //Sometimes there are more than 52 weeks in a year.
    //These tables are close enough to use year after year
    
    //Why is there a 4:58am in this list when know the sun has never risen at 4:58am?
    //Because this list has not had the Daylight Savings Time + 1 (spring forward)
    //applied yet.  Near summer solstice it does rise at 5:58am!

    //This table only valid in southeast Ohio, USA
    const byte mTableSunriseHours[53]   = { 7,  7,  7,  7,  7,  7,  7,  7,  7,  6,
                                            6,  6,  6,  6,  5,  5,  5,  5,  5,  5,
                                            5,  5,  5,  4,  4,  5,  5,  5,  5,  5,
                                            5,  5,  5,  5,  5,  5,  6,  6,  6,  6,
                                            6,  6,  6,  6,  6,  7,  7,  7,  7,  7,
                                            7,  7,  7}; 
    const byte mTableSunriseMinutes[53] = {47, 48, 46, 42, 37, 31, 23, 14,  4, 54,
                                           43, 32, 21, 10, 59, 48, 38, 29, 20, 13,
                                           7,  3,  0, 58, 58,  0,  3,  7, 12, 18,
                                           24, 30, 37, 43, 50, 56,  3,  9, 16, 23,
                                           29, 36, 44, 51, 59,  7, 15, 23, 30, 36,
                                           41, 45, 47};
    const byte mTableSunsetHours[53]  =   {17, 17, 17, 17, 17, 17, 17, 18, 18, 18,
                                           18, 18, 18, 18, 18, 19, 19, 19, 19, 19,
                                           19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
                                           19, 19 ,19, 19, 19, 18, 18, 18, 18, 18,
                                           17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
                                           17, 17, 17}; 
    const byte mTableSunsetMinutes[53]  = {13, 19, 26, 34, 43, 51, 59,  7, 15, 23,
                                           30, 37, 44, 51, 58,  5, 12, 19, 26, 33,
                                           39, 45, 50, 54, 57, 58, 58, 56, 53, 48,
                                           42, 34, 26, 16, 06, 55, 44, 32, 21,  9,
                                           58, 48, 38, 29, 21, 14,  8,  4,  2,  2,
                                           4,  7, 13};

private: //variables continued
    byte mQtyImportantDatesToReport              {};
    ImportantDate const * mDatesToReportList[QTY_IMPORTANT_DATES] {};
    //mDatesToReportList array is large enough to hold pointers to every event if needed.
    byte mTodaySunriseHour   {};
    byte mTodaySunriseMinute {};
    byte mTodaySunsetHour    {};
    byte mTodaySunsetMinute  {};
    bool mDaylightSavingsTime {true};
public:  //methods
    void           init                    ();
    DST_Action     daylightSavingCheck     ();
    String         getClockString          (const tmElements_t time,
                                            const bool right_justify = false);
    const char*    getDaySuffix            (byte day_number);
    time_t         getDSTSpringForward     ();
    const ImportantDate* getImportantDate  (const byte index);     
    const char*    getMonthShortName       (const byte month_number);
    String         getSunriseClockString   ();
    String         getSunsetClockString    ();
    byte           getWeekNumber           (tmElements_t date);
    byte           getQtyImportantDates    ();
    bool           isAM                    (const tmElements_t time);
    bool           isDaylight              ();
    bool           isDaylightSavingsTime   (tmElements_t input_date);
    bool           isWakeUpComplete        ();
    void           loadImportantDates      ();
    void           setSunriseSunset        ();

}calendar;

void Calendar::init()
{
    mQtyImportantDatesToReport = 0;
    mDaylightSavingsTime = isDaylightSavingsTime(timenow.getElements());
    loadImportantDates();
    setSunriseSunset();
}

Calendar::DST_Action Calendar::daylightSavingCheck()
{
    bool last_DST_reading {mDaylightSavingsTime};
    bool this_DST_reading {isDaylightSavingsTime(timenow.getElements())};
    if      (last_DST_reading == true && this_DST_reading == true)
    {
        return Calendar::DST_Action::DST_ACTION_NONE;
    }
    else if (last_DST_reading == false && this_DST_reading == false)
    {
        return Calendar::DST_Action::DST_ACTION_NONE;
    }
    else if (last_DST_reading == true && this_DST_reading == false)
    {
        Serial.println(F("Calendar::daylightSavingCheck  Fall back occured"));
        timenow.changeHour(1); //set clock to 1AM 
        return Calendar::DST_Action::DST_ACTION_CLOCK_FELL_BACK;
    }
    else
    {
        Serial.println(F("Calendar::daylightSavingCheck  Spring forward occured"));
        timenow.changeHour(3); //set clock to 3AM
        return Calendar::DST_Action::DST_ACTION_CLOCK_SPRUNG_FORWARD;
    }
}

const char* Calendar::getDaySuffix(byte day_number)
{
    //Isolate the teens and be sure they get 'th' suffix (11th 12th 13th)
    //The non-teen 1, 2, 3,s get their special suffix (2nd, 22nd, 31st....52nd) 
    //Therefore, any number over 20 should be reduced by tens until it is 10 or less
    //Any number under 20 must be left alone
    if (day_number >= 20)
    {
        while (day_number > 10)
        {
            day_number -= 10;
        }
    }
    //day_of_month should now be between (1 and 20)
    if (day_number < 4)
    {
        return mDaySuffix[day_number]; // 0th, 1st, 2nd, 3rd
    }
    else
    {
        return mDaySuffix[0]; //th
    }
}

String Calendar::getClockString(const tmElements_t time,
                                const bool right_justify)
{
    int  format {};
    String clock_string ("");
    if ((time.Hour) >= 12)
    {      
        format = 12;
    }
    if ((time.Hour) == 12 || (time.Hour) == 0)
    {
        clock_string += F("12");
    }
    else
    {
        //if a single digit AND right_justify add a leading space
        if (time.Hour - format < 10 && right_justify == true)
        {             
            clock_string = " " + clock_string;
        }
        const String hour_string (time.Hour - format); 
        clock_string += hour_string;
    }   
    clock_string += F(":");
    //if a single digit, add a leading zero
    if (time.Minute < 10)
    {  
        clock_string += F("0");
    }
    const String minute_string {time.Minute};   
    clock_string += minute_string;
    if (isAM(time)      == true)
    {
        clock_string += F("am");
    }
    else if (isAM(time) == false)
    {                          
        clock_string += F("pm");
    }
    //add a plus symbol to the end of the string to indicate DST
    if (mDaylightSavingsTime)
    {
        clock_string += F("+");
    }
    return clock_string;    
}

bool Calendar::isWakeUpComplete()
{
    const int wake_up_delay = 15;  //minutes 
    //The wake up delay ensures that the the inverter does not burn off the fresh
    //battery charger energy.

    const int sunrise_minutes {(mTodaySunriseHour * 60) + mTodaySunriseMinute};
    const int now_minutes     {(hour() * 60) + minute()};  
    if (now_minutes >= sunrise_minutes + wake_up_delay)
    {
        return true; 
    }
    else
    {
        return false;
    }
}

const Calendar::ImportantDate* Calendar::getImportantDate(const byte index)
{
    return mDatesToReportList[index];
}

const char* Calendar::getMonthShortName(const byte month_number)
{
    //monthShortStr is defined in the time library
    return monthShortStr(month_number);
}


String Calendar::getSunriseClockString()
{
    tmElements_t sunrise {};
    sunrise.Hour   = mTodaySunriseHour;
    sunrise.Minute = mTodaySunriseMinute;
    return getClockString(sunrise);
}

String Calendar::getSunsetClockString()
{
    tmElements_t sunset {};
    sunset.Hour   = mTodaySunsetHour;
    sunset.Minute = mTodaySunsetMinute;
    return getClockString(sunset);
}
    
byte Calendar::getWeekNumber(tmElements_t date)  //needed to read sunrise sunset table
{
    //convert date to the first moment of the year by zeroing all but year (and wday)
    date.Hour   = 0;
    date.Minute = 0;
    date.Second = 0;
    date.Day    = 0;
    date.Month  = 0;
    //convert first moment of the year to unix time
    const time_t year_start {makeTime(date)};
    //how many seconds have elapsed since the year began?
    const time_t seconds_since_start_of_year {now() - year_start};
    //divide by SECS_PER_WEEK (Time Library) to compute week number 
    const time_t week_number {seconds_since_start_of_year / SECS_PER_WEEK};
    return(week_number);
}

byte Calendar::getQtyImportantDates()
{
    return mQtyImportantDatesToReport;
}

bool Calendar::isAM(const tmElements_t time)
{
    //Time Library has isAM functions, but not with a tmElements_t parameter.
    //The moment after the clock hits 12 noon the time is 'post merīdiem.'
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
    const int todays_sunrise_minutes {(mTodaySunriseHour * 60) + mTodaySunriseMinute};
    const int todays_sunset_minutes  {(mTodaySunsetHour  * 60) + mTodaySunsetMinute};
    const int now_minutes     {(hour()            * 60) + minute()};
    if (now_minutes >= todays_sunrise_minutes && now_minutes < todays_sunset_minutes)
    {
        return true; 
    }
    else
    {
        return false;
    }
}

bool Calendar::isDaylightSavingsTime(tmElements_t input_date)
{  
    //Rules for my area (Ohio, USA) and most of United States
    //DST begins on the second Sunday of March    at 2AM and
    //       ends on the first Sunday of November at 2AM.
    
    //Find the FIRST Sunday of March
    //Create a blank tmElements_t object, it's a seven part time and date.   
    tmElements_t march_start {}; //sets all elements to 0
    //March is the 3rd march of the year
    const uint8_t march {3}; 
    //Set the month
    march_start.Month = march;
    //Set the year to the year from the input_date parameter 
    march_start.Year  = input_date.Year;
    //time_t numbers are the number of seconds that have elapsed since January 1st 1970.
    //Use the time library makeTime function to convert the tmElements_t object
    //into a time_t so some time library math can be done.  
    const time_t start_march {makeTime(march_start)};
    //The time library has a 'nextSunday' function
    //Since the march_start.Day variable was set to the 0th instead of the 1st,
    //Sunday's that fall on the 1st of the march will 
    //qualify as 'nextSunday' as well as any Sunday that falls on the 2nd through 7th.
    //Use the nextSunday function to find the first Sunday of the month.
    const time_t first_sunday_of_march {nextSunday(start_march)};
    //Add one weeks worth of seconds to the time_t first_sunday_of_march
    //to get the 2nd Sunday of March.  SECS_PER_WEEK is defined in the time library.
    const time_t second_sunday_of_march {first_sunday_of_march + SECS_PER_WEEK};
    //Add two hours worth of seconds to change the time from midnight to 2AM.
    //It is important to change the time to 2AM AFTER the nextSunday function has
    //been called, and not before. SECS_PER_HOUR is also defined in the time library.
    const time_t second_sunday_of_march_2AM {second_sunday_of_march + SECS_PER_HOUR * 2};

    //Find the first Sunday of November in a similar manner.
    tmElements_t november_start{};
    //November is the 11th march of the year
    const uint8_t november {11};
    //Set the month
    november_start.Month = november;
    //Set the year to the year from the input_date parameter 
    november_start.Year  = input_date.Year;
    //Convert the tmElements_t to a time_t
    const time_t start_november {makeTime(november_start)};   
    //Use the nextSunday function to find the first Sunday of the month
    const time_t first_sunday_of_november {nextSunday(start_november)};
    //Add two hours worth of seconds to change the time from midnight to 2AM.
    const time_t first_sunday_of_november_2AM  {first_sunday_of_november + SECS_PER_HOUR * 2};
    //Convert the input parameter to a time_t number for a comparison
    const time_t date_input {makeTime(input_date)};
    //Compare results and return the answer
    if (date_input >= second_sunday_of_march_2AM &&
        date_input <  first_sunday_of_november_2AM)
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
    mQtyImportantDatesToReport = 0;  //reset this variable
    //load all events in the search_window_days
    const time_t search_window_days {14};
    //const time_t seconds_in_a_day   {86400};
    const time_t search_window_secs {search_window_days * SECS_PER_DAY}; 
    for (int i = 0; i < QTY_IMPORTANT_DATES; i++)
    {
        //build anniversary date of event
        tmElements_t event_anniversary {};
        //clear Hours, Minutes, OnceEverySecond
        event_anniversary.Hour   = 0;
        event_anniversary.Minute = 0;
        event_anniversary.Second = 0;
        //align year
        event_anniversary.Year   = timenow.getYear();
        //get day and month
        event_anniversary.Day    = mImportantDateList[i].day;
        event_anniversary.Month  = mImportantDateList[i].month;
        //convert tmElements_t to time_t
        time_t event = makeTime(event_anniversary);
        //see if date is in search window
        time_t today_start {previousMidnight(now())};
        if (event >= today_start && event <= now() + search_window_secs)
        {
            //Fill mDatesToReportList with pointers to the important date table 
            const ImportantDate *pointer {&mImportantDateList[i]};
            mDatesToReportList[mQtyImportantDatesToReport] = pointer;
            ++mQtyImportantDatesToReport;            
        }
    }
}

void Calendar::setSunriseSunset()
{
    const int week_number {getWeekNumber(timenow.getElements())};
    mTodaySunriseHour   = mTableSunriseHours[week_number] + mDaylightSavingsTime;
    mTodaySunriseMinute = mTableSunriseMinutes[week_number];
    mTodaySunsetHour    = mTableSunsetHours[week_number] + mDaylightSavingsTime;
    mTodaySunsetMinute  = mTableSunsetMinutes[week_number]; 
}
//==end of Calendar==========================================================================

class MySerial
{
private: //variables
    bool mUseLaptopOperatingVoltage{true};    
public:
    bool checkForUserInput();
    void printFullDateTime(const tmElements_t timestamp);
    void printFullDateTime(const time_t timestamp);
    void printState(char const *text);
    void printTimestamp();
    bool usingLaptopOperatingVoltage();
private: //methods
    int  getIntegerInput();
    //void print
}myserial;

bool MySerial::checkForUserInput()
{
    //A simple interface that accepts single character commands,
    //or single character commands with a numeric argument from the serial
    //monitor input box

    //example commands: (case insensitive)
    //'c'     toggle voltage 'c'orrection
    //'m6'    change the 'm'inute to 6
    //'h14'   change the 'h'our to 2PM (24 hour format)
    //'s28'   change the 's'econds to 28
    //'s'     change the 's'econds to 0
    //'d4'    change the 'd'ay to the 4th
    //'n3'    change the mo'n'th to March, because 'm' was already taken
    //'y56'   change the year to 2056
    //'y2020' change the year to 2020

    //check if the user wants to toggle 'c'orrected voltage    
    if (Serial.available())
    {
        const char input  {static_cast<char>(Serial.read())};
        const int  value  {getIntegerInput()};
        String time_command_feedback {""};
        switch (input)
        {
            case 'c':
            case 'C':
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
                break;
            case 'm':
            case 'M':
                timenow.changeMinute(value);
                time_command_feedback = "Minute";
                break;
            case 'h':
            case 'H':
                timenow.changeHour(value);
                time_command_feedback = "Hour";
                break;
            case 's':
            case 'S':
                timenow.changeSecond(value);
                time_command_feedback = "Second";
                break;
            case 'd':
            case 'D':
                timenow.changeDay(value);
                time_command_feedback = "Day";
                calendar.init();
                break;
            case 'n':
            case 'N':
                timenow.changeMonth(value);
                time_command_feedback = "Month";
                calendar.init();
                break;
            case 'y':
            case 'Y':
                timenow.changeYear(value);
                time_command_feedback = "Year";
                calendar.init();
                break;
            
            default:
                break;
            
        }
        if (time_command_feedback != "")
        {
            time_command_feedback += " changed to ";
            time_command_feedback += value;
            Serial.println(time_command_feedback);
            Serial.print("Now: ");
            printFullDateTime(now());
            calendar.init();
        }
    }
}

void MySerial::printFullDateTime (const tmElements_t timestamp)
{
    //Tuesday
    String fullDateTime {dayStr(timestamp.Wday)};
    //Tuesday,
    fullDateTime += ", ";
    //Tuesday, May
    fullDateTime += monthStr(timestamp.Month);
    fullDateTime += " ";
    //Tuesday, May 3
    fullDateTime += timestamp.Day;
    //Tuesday, May 3rd
    fullDateTime += calendar.getDaySuffix(timestamp.Day);
    fullDateTime += " ";
    //Tuesday, May 3rd 2019
    fullDateTime += 1970 + timestamp.Year;
    fullDateTime += " ";
    //Tuesday, May 3rd 2019 01:23:07
    Serial.print(fullDateTime);
    printTimestamp();
    Serial.println();
}

void MySerial::printFullDateTime (const time_t timestamp)
{
    tmElements_t timestamp_elements {};
    breakTime(timestamp, timestamp_elements);
    printFullDateTime(timestamp_elements);
}


void MySerial::printTimestamp()
{    
    if (timenow.getHour() <= 9)
    {
        Serial.print(F("0"));
    }
    Serial.print(timenow.getHour());
    Serial.print(F(":"));
    if (timenow.getMinute() <= 9)
    {
        Serial.print(F("0"));
    }
    Serial.print(timenow.getMinute());
    Serial.print(F(":"));
    if (timenow.getSecond() <= 9)
    {
        Serial.print(F("0"));
   }
   Serial.print(timenow.getSecond());
   Serial.print(F("  "));
}

bool MySerial::usingLaptopOperatingVoltage()
{
    return mUseLaptopOperatingVoltage;
}

//private MySerial methods
int MySerial::getIntegerInput()
{
    String numeral   {Serial.readString()};    
    long int integer {numeral.toInt()};
    return integer;
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
    byte   mDissolveCountdown {};
public:
    void   drawDisplay        ();
    void   dissolveEffect     ();
    void   dissolveThis       (String top_line, String bottom_line);
    void   printClock         (const TimeElements time,
                               const Coordinant   coordinant,
                               const bool         right_justify);
    void   printDate          (const Coordinant   coordinant);
    void   printDateSuffix    (const byte numeral);
    void   printImportantDate (const Calendar::ImportantDate* importantdate);
    void   printLDRresults    ();
    void   printStateChangeDelayCounter (const int timer_value);
    //void   updateBacklight    ();
    String centerText         (const String text);
}mylcd;

void MyLCD::drawDisplay()
{
    //updateBacklight();
    Coordinant coordinant {12, 3};
    const bool right_justify    {true};
    printClock(timenow.getElements(), coordinant, right_justify);
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
    //send a copy of the message to the serial monitor 
    Serial.print(mMessageTopLine);
    Serial.println(mMessageBottomLine);
}

void MyLCD::printDateSuffix(byte numeral)
{
    const char *suffix {calendar.getDaySuffix(numeral)};
    liquidcrystali2c.print(suffix);
}

void MyLCD::printImportantDate(const Calendar::ImportantDate* importantdate)
{
    //format top line
    String top_line            (importantdate->text); //Paul & Mena
    switch (importantdate->event_type)
    {
        case Calendar::EVENTTYPE_ANNIVERSARY:
        case Calendar::EVENTTYPE_BIRTHDAY:
            {
                top_line += F("'s "); //Paul & Mena's
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
    String bottom_line (F(""));
    //build a time_t of event for a tommorrow comparison
    tmElements_t event {};
    event.Year  = timenow.getYear();
    event.Month = importantdate->month;
    event.Day   = importantdate->day;
    time_t event_time_t {makeTime(event)};
    time_t right_now {now()};
    time_t tommorrow_begins {nextMidnight(right_now)};
    time_t tommorrow_ends   {nextMidnight(right_now) + SECS_PER_DAY};
    if (importantdate->day == timenow.getDay())
    {        
        bottom_line = F("today");
    }
    else if (event_time_t >= tommorrow_begins &&
             event_time_t <  tommorrow_ends)
    {
        bottom_line = F("tomorrow");
    }
    else
    {
        tmElements_t event {};
        //event.Hour  = 0;
        event.Day   = importantdate->day;
        event.Month = importantdate->month;
        event.Year  = timenow.getYear();
        time_t event_unix {makeTime(event)};
        myserial.printFullDateTime(event_unix);
        const byte day_of_week (weekday(event_unix));
        String day_str {dayShortStr(day_of_week)};
        bottom_line = day_str + F(", "); //Wed,
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
        Serial.println(F("MyLCD::centerText  String was larger than screen width"));
        const String end {text.substring(mLCD_Width)};
        const String front {text.substring(0, mLCD_Width - 1)};
        Serial.print(F("MyLCD::centerText  Removed \""));
        Serial.print(end);
        Serial.println(F("\""));
        return front;
    }
    else if (string_length <= 0)
    {
        //string empty
        Serial.println(F("MyLCD::centerText  no string error"));
        //      01234567890123456789
        return F("---missing string---");
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
    if (digitalRead(Pin::circuit_sensor_one))
    { 
        liquidcrystali2c.print(F("A"));
    }
    else
    {
        liquidcrystali2c.print(F(" "));
    }

    if (digitalRead(Pin::circuit_sensor_two))
    {
        liquidcrystali2c.print(F("S"));
    }
    else
    {
        liquidcrystali2c.print(F(" "));
    }
}

void MyLCD::printStateChangeDelayCounter(const int timer_value)
{
    liquidcrystali2c.setCursor(8, 0);
    if (timer_value == 0)
    {
        liquidcrystali2c.print(F("  "));// no counter, so clear the number
        return;
    }
    else if (timer_value <= 9)
    {
        liquidcrystali2c.print(F(" ")); // clear the first space if a single digit
    }
    liquidcrystali2c.print(timer_value); // print the counter
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
    liquidcrystali2c.print(F(", "));
    liquidcrystali2c.print(calendar.getMonthShortName(timenow.getMonth()));    
    liquidcrystali2c.print(F(" "));
    liquidcrystali2c.print((timenow.getDay()));
    liquidcrystali2c.print(F(" "));  //this space to clear last digit when month rolls over (31 to 1) 
}
//==end of MyLCD=============================================================================

class Timing
{
private: //variables
    //countdown timers
    byte         mStateChangeDelayCounter {};
    int          m2AM_LockoutCounter      {};
    //timestamps
    time_t       mDissolveTimestamp       {};
    time_t       mOneSecondTimestamp      {}; //to control 1s loop
    
public:  //methods
    byte getStateChangeDelayCounter           ();
    bool hasOneSecondPassed   ();
    bool isIt2AM              (); 
    bool isDissolveReady      ();
    void lockout2AM           (int lockout_seconds);    
    void setCountdownTimer    (byte x);
    void updateCounters       ();
private: //methods
    


}timing;

byte Timing::getStateChangeDelayCounter()
{
    return mStateChangeDelayCounter;
}

bool Timing::isIt2AM()
{
    if (m2AM_LockoutCounter)
    {
        Serial.print("L");
    }    
    if (timenow.getHour()   == 2 &&
        timenow.getMinute() == 0 &&
        timenow.getSecond() == 0 &&
        m2AM_LockoutCounter == false)
    {
        //set m2AM_LockoutCounter for one second so this won't check again until
        //2:00:01AM.  This ensures that true is returned just once per day;
        m2AM_LockoutCounter = 1;
        Serial.println("Timing::isIt2AM() = true!"); 
        return true;
    }
    else
    {
        return false;
    }
    
}

bool Timing::isDissolveReady()
{
    const time_t   dissolve_interval {50}; //milliseconds 
    if (millis() - dissolve_interval  >= mDissolveTimestamp)
    {
        mDissolveTimestamp = millis(); //set up delay for next loop
        return true;
    }
    else
    {
        return false;
    }    
}

bool Timing::hasOneSecondPassed()
{
    if (mOneSecondTimestamp != now()) 
    {
        //set up 1000ms delay for the next loop
        mOneSecondTimestamp = now();
        return true;
    }
    else
    {
        return false;
    }
}

void Timing::lockout2AM(int lockout_seconds)
{
    m2AM_LockoutCounter = lockout_seconds;
    Serial.print("Timing::lockout2AM_Check  mLockoutExpires in ");
    Serial.print(lockout_seconds);
    Serial.print(" seconds");
}

void Timing::setCountdownTimer(byte x)
{
    mStateChangeDelayCounter = x;
}

void Timing::updateCounters()
{
    //this runs once every second
    mylcd.printStateChangeDelayCounter(mStateChangeDelayCounter);
    //decrement counters
    if (mStateChangeDelayCounter > 0)
    {
        --mStateChangeDelayCounter;
    }
    if (m2AM_LockoutCounter > 0)
    {
        --m2AM_LockoutCounter;
    }
}
//==end of Timing============================================================================

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
        mMax.timestamp = timenow.getElements();
        Serial.print(" New daily high just set.");
    }
    if (mVoltage < mMin.voltage)
    {
        mMin.voltage   = mVoltage; 
        mMin.timestamp = timenow.getElements();
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
        mMin.timestamp = timenow.getElements();
        mMax.voltage = mVoltage;
        mMax.timestamp = timenow.getElements();
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
    int   mInverterRunTime {};
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
    const char *start_string {" State changed to "};
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
    timing.setCountdownTimer(seconds_to_warm_up);
    setState(STATE_INVERTER_WARM_UP);
}

void MyStateMachine::warmUpInverterStatefunction()
{
    if (timing.getStateChangeDelayCounter())
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
    timing.setCountdownTimer(stage_two_switching_delay);
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
             !timing.getStateChangeDelayCounter())
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
    timing.setCountdownTimer(inverter_cooldown_time);
    setState(STATE_INVERTER_COOL_DOWN);
}

void MyStateMachine::inverterCooldownfunction()
{
    if (!timing.getStateChangeDelayCounter())
    {
        setState(STATE_INIT_BALANCED);
    }
}
//==end of MyStateMachine====================================================================

class MessageManager
{
private: //variables
    byte         mCurrentMessageIndex   {};
    time_t       mNextMessageTimestamp  {};
    const time_t mMessageDuration       {5000}; //milliseconds
    const byte   mQtySystemMessages     {3};        
public:  //methods
    void init();
    void main();
    void messageInverterRunTime();
    void messageSunriseSunset();
    void messageVoltageExtremes();
}messagemanager;

void MessageManager::init()
{
    mCurrentMessageIndex = 0;
    //mNextMessageTimestamp = 0; 
}

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
        }
        else //system messages
        {
            const int sytem_message_index {mCurrentMessageIndex - calendar_messages};
            switch (sytem_message_index)
            {
            case 0: //voltage record high and low
                messageVoltageExtremes();
                break;
            case 1:
                messageSunriseSunset();
                break;
            case 2:
                messageInverterRunTime();
                break;
            default:
                Serial.println(F("MessageManager::main  Message index was too high"));
                mCurrentMessageIndex = 0;
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
    String top_line    {F("Inverter harvested")};
    String bottom_line {""};
    if (inverter_run_time == 0)
    {
        top_line    = F("Inverter waiting");
        bottom_line = F("no harvest yet");
    }
    //report run time in seconds
    else if (inverter_run_time < SECS_PER_MIN)
    {
        const String seconds     {inverter_run_time};
        bottom_line = seconds + F(" second");
        if (inverter_run_time > 1)
        {
            bottom_line += F("s");
        }         
    }
    //report run time in minutes
    else if (inverter_run_time < SECS_PER_HOUR)
    {
        const int minutes        (inverter_run_time / SECS_PER_MIN);
        const String minutes_str {minutes};
        bottom_line = minutes_str + F(" minute");
        if (minutes > 1)
        {
            bottom_line += F("s");
        }
    }
    //report run time in hours and minutes
    else if (inverter_run_time >= SECS_PER_HOUR)
    {
        const int     run_minutes        (inverter_run_time / SECS_PER_MIN);
        const int     minutes_in_an_hour {60};
        const int     run_hours          {run_minutes / minutes_in_an_hour};
        const int     remainder_minutes  {run_minutes - (run_hours * minutes_in_an_hour)};
        const String  run_hours_string   {run_hours};
        bottom_line = run_hours_string + F(" hour"); 
        if (run_hours > 1)
        {
            bottom_line += F("s");
        }
        if (remainder_minutes)
        {
            bottom_line += F(" ");
            const String run_minutes_string {remainder_minutes};
            bottom_line += run_minutes_string;
            bottom_line += F(" minute");
            if (remainder_minutes > 1)
            {
                bottom_line += F("s");
            }
        }
    }
    //formatting complete, send it to lcd    
    mylcd.dissolveThis(top_line, bottom_line);
}

void MessageManager::messageSunriseSunset()
{
    const String sunrise {F("Sunrise ")};
    const String sunrise_clock_string {calendar.getSunriseClockString()};
    const String top_line {sunrise + sunrise_clock_string};
    const String sunset {F("Sunset ")};
    const String sunset_clock_string {calendar.getSunsetClockString()};
    const String bottom_line {sunset + sunset_clock_string};
    mylcd.dissolveThis(top_line, bottom_line);
}

void MessageManager::messageVoltageExtremes()
{
    const int qty_voltrecord {2};
    const Voltmeter::VoltRecord voltrecord[qty_voltrecord] {voltmeter.getMin(),
                                                            voltmeter.getMax()};
    String message[qty_voltrecord] {""};
    for (int i = 0; i < qty_voltrecord; i++)
    {
        const String voltage_string  {voltrecord[i].voltage};
        const String clock_string    {calendar.getClockString(voltrecord[i].timestamp)};
        message[i] = voltage_string + "v @ " + clock_string;
    }
    mylcd.dissolveThis(message[0], message[1]);
}
//==end of MessageManager====================================================================

class TrackLight
{
private: //enums
    enum TrackLightState
    {
        TRACKLIGHTSTATE_READING_NEW_INPUT,
        TRACKLIGHTSTATE_IGNORING_SMALL_INPUT_FLUCTUATIONS,
        MAX_TRACKLIGHTSTATE
    };
    
private: //variables
    TrackLightState mTrackLightState {TRACKLIGHTSTATE_READING_NEW_INPUT};
    //to prevent flicker drift, freeze light level if this many milliseconds 
    //have passed since the last mLargeAdjustment
    time_t      mAdjustmentWindowTimestamp {millis()};
    //Hold the input at mInputAnchorPoint unless there is a mLargeAdjustment
    //Why? My poor quality potentiometer does not always produce a steady reading.
    int         mInputAnchorPoint               {};
    
public:  //methods
    void readDimmerSwitch         ();
    void setLightLevel            ();
private: //methods
    bool largeAdjustmentDetected  (const int dimmer_reading);
    int  regulateVoltage          (const int input_level);
    

}tracklight;

void TrackLight::readDimmerSwitch()
{
    //the dimmer_reading range is integer values between 0 and 1023
    int dimmer_reading {analogRead(Pin::dimmer_potentiometer)};
    //invert the value because I hooked my potentiometer up backwards
    dimmer_reading = 1023 - dimmer_reading;
    if (mTrackLightState == TRACKLIGHTSTATE_IGNORING_SMALL_INPUT_FLUCTUATIONS)
    {
        //check for a mLargeAdjustment in the dimmer switch
        if(largeAdjustmentDetected(dimmer_reading))
        {
            //change state
            mTrackLightState = TRACKLIGHTSTATE_READING_NEW_INPUT;
            //set timer
            mAdjustmentWindowTimestamp = millis();
        }
    }
    else if (mTrackLightState == TRACKLIGHTSTATE_READING_NEW_INPUT)
    {
        //allow the reading to change
        mInputAnchorPoint = dimmer_reading;
        //check for large adjustments and extend adjustment window if detected.
        //The adjustment_window is the amount of time that can pass without a
        //large adjustment.  The reading anchors itself after that
        const int adjustment_window {1000}; //milliseconds
        if (largeAdjustmentDetected(dimmer_reading))
        {
            //extend time window
            mAdjustmentWindowTimestamp = millis();
        }
        else if (millis() - adjustment_window >= mAdjustmentWindowTimestamp)
        {
            //out of time
            mTrackLightState = TRACKLIGHTSTATE_IGNORING_SMALL_INPUT_FLUCTUATIONS;
        }
    }
}

void TrackLight::setLightLevel()
{
    const int new_setting {regulateVoltage(mInputAnchorPoint)};
    //the setting is a one byte value (0 - 255)
    //this is one fourth the value of the input reading
    int value_to_write {new_setting / 4};
    //enforce range min/maximum
    constrain(value_to_write, 0, 255);
    analogWrite(Pin::workbench_lighting, value_to_write);
}

//private methods

bool TrackLight::largeAdjustmentDetected(const int dimmer_reading)
{
    const int large_adjustment {100}; //out of 1024
    int difference {abs(mInputAnchorPoint - dimmer_reading)};
    if (difference > large_adjustment)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int TrackLight::regulateVoltage(const int input_level)
{
    //Keeps the light output consistent with varying input voltages.
    //The voltage at the bulb (post MOSFET) always measures -0.40 volts less than
    //the system voltage
    const float max_led_voltage      {12.50};
    const float voltage_loss         { 0.40};
    const float max_voltage          {max_led_voltage + voltage_loss};
    float system_voltage             {voltmeter.getVoltage()};
    if (system_voltage == 0.0) //avoid a division by zero error if voltage can't be read
    {
        //send an error, but try to keep the light working
        //assume a strong voltage is present to protect LEDs
        Serial.println(F("TrackLight::regulateVoltage  No voltage error."));
        const float system_normal_high {14.5};
        system_voltage = system_normal_high;
    }
    const float voltage_regulation_ratio {max_voltage / system_voltage};
    return input_level * voltage_regulation_ratio;
}
//==end of TrackLight========================================================================

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
    //start the Wire library
    Wire.begin();
    //start the LiquidCrystal_I2C library
    const int lcd_columns {20};
    const int lcd_rows    { 4};
    liquidcrystali2c.begin(lcd_columns, lcd_rows);
    timenow.syncTimeWithRTC_Clock();
    voltmeter.readVoltage();
    voltmeter.initDailyStatistics();
    calendar.init();
    //set pins  
    pinMode (Pin::inverter, OUTPUT);
    pinMode (Pin::battery_charger, OUTPUT);
    pinMode (Pin::voltage_divider, INPUT);
    pinMode (Pin::circuit_sensor_one, INPUT);
    pinMode (Pin::circuit_sensor_two, INPUT);
    pinMode (Pin::workbench_lighting, OUTPUT);
    pinMode (Pin::stage_one_inverter_relay, OUTPUT);
    pinMode (Pin::stage_two_inverter_relay, OUTPUT);
}
               
void loop()
{
    timenow.syncTimeWithRTC_Clock();  
    voltmeter.readVoltage();
    myserial.checkForUserInput();
    tracklight.readDimmerSwitch();
    tracklight.setLightLevel();
    if (timing.isDissolveReady())
    {
        mylcd.dissolveEffect();
    }
    //This code runs every second (1000ms)
    if (timing.hasOneSecondPassed())   
    {
        //timenow.syncTimeWithRTC_Clock();
        timing.updateCounters();    
        mylcd.drawDisplay();
        //print messages (if any are ready)
        messagemanager.main();
        myserial.printTimestamp(); //print timestamp
        voltmeter.main();          //print voltage
        mystatemachine.main();     //print state changes
        Serial.println();
    }
    if (timing.isIt2AM())
    {
        mystatemachine.resetInverterRunTime();
        voltmeter.initDailyStatistics();  //resets daily voltage highs and lows           
        if (calendar.daylightSavingCheck() == Calendar::DST_ACTION_CLOCK_FELL_BACK)
        {
            //When time falls back from 2AM to 1AM, 2AM will repeat itself in 1 hour.
            //The check for 2AM must therefore be skipped for over an hour so the
            //program does not get caught in a repetitive loop.
            //SECS_PER_HOUR is defined in the time library.
            const int for_over_an_hour (SECS_PER_HOUR + 1); //one hour one second
            timing.lockout2AM(for_over_an_hour);
        }
        messagemanager.init();  
        calendar.init();
    }
}

//Serial.print("Year: ");
//Serial.println(test_elements.Year);
