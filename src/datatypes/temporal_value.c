#include "temporal_value.h"
#include <limits.h>
#include "../util/rmalloc.h"
#include <stdbool.h>

const char utcFormat[] = "YYYY-MM-DDTHH:MM:SS.NNNNNNNNNz";
const int dateMask = DATE | DATE_TIME | LOCAL_DATE_TIME;
const int timeMask = TIME | LOCAL_TIME | DATE_TIME | LOCAL_DATE_TIME;

const int normalYearQuarterDays[4] = {0, 90, 181, 273};
const int leapYearQuarterDays[4] = {0, 91, 182, 274};

RG_TemporalValue RG_TemporalValue_New_FromTimeSpec(RG_TemporalType temporalType, struct timespec ts) {
    RG_TemporalValue temporalValue;
    temporalValue.seconds = ts.tv_sec;
    temporalValue.nano = ts.tv_nsec;
    temporalValue.type = temporalType;
    return temporalValue;
}

RG_TemporalValue RG_TemporalValue_New(RG_TemporalType temporalType) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return RG_TemporalValue_New_FromTimeSpec(temporalType, ts);
    
}

RG_TemporalValue RG_Date_New() {
    return RG_TemporalValue_New(DATE);
}

RG_TemporalValue RG_Time_New() {
    return RG_TemporalValue_New(TIME);
}

RG_TemporalValue RG_LocalTime_New() {
    return RG_TemporalValue_New(LOCAL_TIME);
}

RG_TemporalValue RG_DateTime_New() {
    return RG_TemporalValue_New(DATE_TIME);
}

RG_TemporalValue RG_LocalDateTime_New() {
    return RG_TemporalValue_New(LOCAL_DATE_TIME);
}

RG_TemporalValue RG_Date_New_FromTimeSpec(struct timespec ts) {
    return RG_TemporalValue_New_FromTimeSpec(DATE, ts);
}

RG_TemporalValue RG_Time_New_FromTimeSpec(struct timespec ts) {
    return RG_TemporalValue_New_FromTimeSpec(TIME, ts);
}

RG_TemporalValue RG_LocalTime_New_FromTimeSpec(struct timespec ts) {
     return RG_TemporalValue_New_FromTimeSpec(LOCAL_TIME, ts);
}

RG_TemporalValue RG_DateTime_New_FromTimeSpec(struct timespec ts) {
     return RG_TemporalValue_New_FromTimeSpec(DATE_TIME, ts);
}

RG_TemporalValue RG_LocalDateTime_New_FromTimeSpec(struct timespec ts) {
     return RG_TemporalValue_New_FromTimeSpec(LOCAL_DATE_TIME, ts);
}

// SIValue RG_TemporalValue_ToSIValue(RG_TemporalValue timeStamp) {
//     return (struct SIValue)timeStamp;
  
// }
// RG_TemporalValue SIValue_ToRG_TemporalValue(SIValue value) {
//     return (RG_TemporalValue)value;
// }

struct tm* _getTimeDescriptorFromTemporalValue(RG_TemporalValue temporalValue) {
    struct timespec ts;
    ts.tv_sec = temporalValue.seconds;
    ts.tv_nsec = temporalValue.nano;
    return gmtime(&ts.tv_sec);
}

struct timespec _createTimespecFromTm(struct tm t) {
  time_t epoch = timegm(&t);
  struct timespec ts;
  ts.tv_sec = epoch;
  ts.tv_nsec = 0;
  return ts;
}

// return 1 on failure, 0 on success
int _timeDescriptor_YearWeek(const struct tm *timeDescriptor, int *year, int *week) {
    // work with local copy
    struct tm tm = *timeDescriptor;
    // fully populate the yday and wday fields.
    if (mktime(&tm) == -1) {
    return 1;
    }

    // Find day-of-the-week: 0 to 6.
    // Week starts on Monday per ISO 8601
    // 0 <= DayOfTheWeek <= 6, Monday, Tuesday ... Sunday
    int DayOfTheWeek = (tm.tm_wday + (7 - 1)) % 7;

    // Offset the month day to the Monday of the week.
    tm.tm_mday -= DayOfTheWeek;
    // Offset the month day to the mid-week (Thursday) of the week, 3 days later.
    tm.tm_mday += 3;
    // Re-evaluate tm_year and tm_yday  (local time)
    if (mktime(&tm) == -1) {
    return 1;
    }

    *year = tm.tm_year + 1900;
    // Convert yday to week of the year, stating with 1.
    *week = tm.tm_yday / 7 + 1;
    return 0;
}

bool _isLeapYear(int year) {
    if (year % 400 == 0)
        return true;
    if (year % 100 == 0)
        return false;
    if (year % 4 == 0)
        return true;
    return false;
}

int RG_TemporalValue_GetYear(RG_TemporalValue temporalValue) {
    if (!(temporalValue.type & dateMask)) {
        // not a date type. should return error/execption
        return INT32_MIN;
    }
    struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);
    return timeDescriptor->tm_year + 1900;
}

int RG_TemporalValue_GetQuarter(RG_TemporalValue temporalValue) {
    if (!(temporalValue.type & dateMask)) {
        // not a date type. should return error/execption
        return INT32_MIN;
    }
    struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);
    int month = timeDescriptor->tm_mon;
    return month / 4 +1;
}

int RG_TemporalValue_GetMonth(RG_TemporalValue temporalValue) {
    if (!(temporalValue.type & dateMask)) {
        // not a date type. should return error/execption
        return INT32_MIN;
    }
    struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);
    return timeDescriptor->tm_mon + 1;
}
/*
* https://en.wikipedia.org/wiki/ISO_week_date
*
* first week:
*
* The ISO 8601 definition for week 01 is the week with the Gregorian year's first Thursday in it. 
* The following definitions based on properties of this week are mutually equivalent, 
* since the ISO week starts with Monday:
*
*   It is the first week with a majority (4 or more) of its days in January.
*   Its first day is the Monday nearest to 1 January.
*   It has 4 January in it. Hence the earliest possible first week extends 
*   from Monday 29 December (previous Gregorian year) to Sunday 4 January, 
*   the latest possible first week extends from Monday 4 January to Sunday 10 January.
*   It has the year's first working day in it, if Saturdays, Sundays and 1 January are not working days.
*
* If 1 January is on a Monday, Tuesday, Wednesday or Thursday, it is in week 01. 
* If 1 January is on a Friday, it is part of week 53 of the previous year. 
* If it is on a Saturday, it is part of the last week of the previous year which is numbered 52 
* in a common year and 53 in a leap year. 
* If it is on a Sunday, it is part of week 52 of the previous year. 
* 
* last week
* The last week of the ISO week-numbering year, i.e. the 52nd or 53rd one, 
* is the week before week 01. This week’s properties are:
*
*   It has the year's last Thursday in it.
*   It is the last week with a majority (4 or more) of its days in December.
*   Its middle day, Thursday, falls in the ending year.
*   Its last day is the Sunday nearest to 31 December.
*   It has 28 December in it. Hence the earliest possible last week extends from Monday 22 December 
*   to Sunday 28 December, the latest possible last week extends from Monday 28 December 
*   to Sunday 3 January.
*
* If 31 December is on a Monday, Tuesday or Wednesday, it is in week 01 of the next year.
* If it is on a Thursday, it is in week 53 of the year just ending;
* if on a Friday it is in week 52 in common years and week 53 in leap years;
* if on a Saturday or Sunday, it is in week 52 of the year just ending.
*/
int RG_TemporalValue_GetWeek(RG_TemporalValue temporalValue) {
    if (!(temporalValue.type & dateMask)) {
        // not a date type. should return error/execption
        return INT32_MIN;
    }
    struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);

    int y = 0, w = 0;
    int err = _timeDescriptor_YearWeek(timeDescriptor, &y, &w);
    return w;
}

int RG_TemporalValue_GetWeekYear(RG_TemporalValue temporalValue) {
    if (!(temporalValue.type & dateMask)) {
        // not a date type. should return error/execption
        return INT32_MIN;
    }
    struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);

    int y = 0, w = 0;
    int err = _timeDescriptor_YearWeek(timeDescriptor, &y, &w);
    return y;
}

int RG_TemporalValue_GetDayOfQuarter(RG_TemporalValue temporalValue) {
    if (!(temporalValue.type & dateMask)) {
        // not a date type. should return error/execption
        return INT32_MIN;
    }
    struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);

    int q = timeDescriptor->tm_mon /4;
    if (_isLeapYear(timeDescriptor->tm_year)) 
        return timeDescriptor->tm_yday - leapYearQuarterDays[q] + 1;
    else 
        return timeDescriptor->tm_yday - normalYearQuarterDays[q] + 1;
}

int RG_TemporalValue_GetDay(RG_TemporalValue temporalValue) {
    if (!(temporalValue.type & dateMask)) {
        // not a date type. should return error/execption
        return INT32_MIN;
    }
    struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);

    return timeDescriptor->tm_mday + 1; 
}

int RG_TemporalValue_GetOrdinalDay(RG_TemporalValue temporalValue) {
    if (!(temporalValue.type & dateMask)) {
        // not a date type. should return error/execption
        return INT32_MIN;
    }
    struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);

    return timeDescriptor->tm_yday + 1; 
}

int RG_TemporalValue_GetDayOfWeek(RG_TemporalValue temporalValue) {
    if (!(temporalValue.type & dateMask)) {
        // not a date type. should return error/execption
        return INT32_MIN;
    }
    struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);

    return timeDescriptor->tm_wday;
}

int RG_TemporalValue_GetHour(RG_TemporalValue temporalValue) {
    if (!(temporalValue.type & timeMask)) {
        // not a date type. should return error/execption
        return INT32_MIN;
    }
    struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);

    return timeDescriptor->tm_hour;
}

int RG_TemporalValue_GetMinute(RG_TemporalValue temporalValue) {
    if (!(temporalValue.type & timeMask)) {
        // not a date type. should return error/execption
        return INT32_MIN;
    }
    struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);

    return timeDescriptor->tm_min;
}

int RG_TemporalValue_GetSecond(RG_TemporalValue temporalValue) {
    if (!(temporalValue.type & timeMask)) {
        // not a date type. should return error/execption
        return INT32_MIN;
    }
    struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);

    return timeDescriptor->tm_sec;
}

int RG_TemporalValue_GetMillisecond(RG_TemporalValue temporalValue) {
    if (!(temporalValue.type & timeMask)) {
        // not a date type. should return error/execption
        return INT32_MIN;
    }

    return temporalValue.nano * 1000000;
}

int RG_TemporalValue_GetMicrosecond(RG_TemporalValue temporalValue) {
    if (!(temporalValue.type & timeMask)) {
        // not a date type. should return error/execption
        return INT32_MIN;
    }

    return temporalValue.nano * 1000;
}

int RG_TemporalValue_GetNaosecond(RG_TemporalValue temporalValue) {
    if (!(temporalValue.type & timeMask)) {
        // not a date type. should return error/execption
        return INT32_MIN;
    }

    return temporalValue.nano;
}


const char *RG_TemporalValue_ToString(RG_TemporalValue timestamp) {
    struct tm *time= gmtime(&timestamp.seconds);
    size_t len = strlen(utcFormat)+1;
    char *buffer = rm_malloc(len);
    len -= strftime(buffer, len, "%Y-%m-%dT%H:%M:%S.", time);
    snprintf(&buffer[strlen(buffer)], len, ".%09uiz", timestamp.nano);
    return buffer;
}

int RG_TemporalValue_Compare(RG_TemporalValue a, RG_TemporalValue b) {
    return a.seconds == b.seconds ? a.nano - b.nano : a.seconds - b.seconds;
}
