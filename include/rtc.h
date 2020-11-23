#include <Arduino.h>
#include <DS3231.h>
#include <Wire.h>
#include <time.h>       /* time_t, struct tm, time, mktime */


DS3231 Clock;

void rtc_init() {
    Wire.begin();
    Serial.println("RTC inited.");
}

static inline String getDate() {
    bool centery;
    String dateout = String(Clock.getDate()) + "/" + String(Clock.getMonth(centery)) + "/" + String(Clock.getYear());
    return dateout;
}
static inline String getTime() {
    
    bool twelve;
    bool pm_time; 
    String timeout = String(Clock.getHour(twelve, pm_time)) + ":" + String(Clock.getMinute()) + ":" + String(Clock.getSecond());
    return timeout;
}
  
static inline time_t getEpochTime() {
    bool centery;
    bool twelve;
    bool pm_time; 

    time_t epoch;
    struct tm t;    
    t.tm_year   = Clock.getYear()-1900+2000;
    t.tm_mon    = Clock.getMonth(centery)-1;           // Month, 0 - jan
    t.tm_mday   = Clock.getDate();                     // Day of the month
    t.tm_hour   = Clock.getHour(twelve, pm_time);
    t.tm_min    = Clock.getMinute();
    t.tm_sec    = Clock.getSecond();
    epoch       = mktime(&t);
    return epoch;
}


/* ----- Convert epoch to datetime www.epochconverter.com/programming/c ----- */

static inline tm getDateTimeFromEpoch(time_t rawtime) {
    struct tm  ts;
    ts = *localtime(&rawtime);
    return ts;
}

