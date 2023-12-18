/*****************************************************************************/
/* Triangle MicroWorks, Inc.                         Copyright (c) 1997-2022 */
/*****************************************************************************/
/*                                                                           */
/* This file is the property of:                                             */
/*                                                                           */
/*                       Triangle MicroWorks, Inc.                           */
/*                      Raleigh, North Carolina USA                          */
/*                       www.TriangleMicroWorks.com                          */
/*                          (919) 870-6615                                   */
/*                                                                           */
/* This Source Code and the associated Documentation contain proprietary     */
/* information of Triangle MicroWorks, Inc. and may not be copied or         */
/* distributed in any form without the written permission of Triangle        */
/* MicroWorks, Inc.  Copies of the source code may be made only for backup   */
/* purposes.                                                                 */
/*                                                                           */
/* Your License agreement may limit the installation of this source code to  */
/* specific products.  Before installing this source code on a new           */
/* application, check your license agreement to ensure it allows use on the  */
/* product in question.  Contact Triangle MicroWorks for information about   */
/* extending the number of products that may use this source code library or */
/* obtaining the newest revision.                                            */
/*                                                                           */
/*****************************************************************************/

/* file: tmwdtime.c
 * description: Date time support
 */
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwdtime.h"

static void TMWDEFS_GLOBAL _decrementMinute(
  TMWDTIME *pDateTime);

static void TMWDEFS_GLOBAL _incrementMinute(
  TMWDTIME *pDateTime);

static void TMWDEFS_GLOBAL _decrementMinute(
  TMWDTIME *pDateTime)
{
  if(pDateTime->minutes > 0)
  {
    pDateTime->minutes--;
  }
  else
  {
    pDateTime->minutes = 59;
    tmwdtime_decrementHour(pDateTime);
  }
}

static void TMWDEFS_GLOBAL _incrementMinute(
  TMWDTIME *pDateTime)
{
  pDateTime->minutes += 1;
  if(pDateTime->minutes >= 60)
  {
    pDateTime->minutes -= 60;
    tmwdtime_incrementHour(pDateTime);
  }
}

/*
 * FUNCTION:  tmwdtime_computeDayOfYear() - returns 1-365 or 366
 *
 * This function first calculates the total number of days in the months
 * preceding the specified month, and then adds in the dayOfMonth.
 * Therefore, it can handle (and is relied upon to handle by other functions
 * in this module) a value for dayOfMonth that is larger than possible for
 * that month.  It returns a result as if the date stretched into the
 * subsequent month by the amount of which the dayOfMonth is too large. For
 * example February 31 returns 62, (which is equivalent to March 3 in a
 * non-leap year).
 *
 * CAVEATS:
 *   This function assumes every fourth year is a leap year.  While this is
 *   valid from 1901 through 2099, it is not valid in century years that are
 *   not multiples of 400; i.e., 1600 was a leap year; 1700, 1800, and 1900
 *   were not leap years; 2000 is a leap year; 2100, 2200, and 2300 will not
 *   be leap years.
 */
TMWTYPES_USHORT TMWDEFS_GLOBAL tmwdtime_computeDayOfYear(const TMWDTIME *pDateTime)
{
  /* This expression is broken up using several temporary variables because  */
  /* at least one compiler could not generate code for the entire expression */
  /* if it were combined in one line.                                        */
  TMWTYPES_USHORT tempValue1 = (TMWTYPES_USHORT)(((pDateTime->month + 9) / 12));
  TMWTYPES_USHORT tempValue2 = (TMWTYPES_USHORT)(1 + (((pDateTime->year%4) + 2) / 3));
  TMWTYPES_USHORT tempValue3 = (TMWTYPES_USHORT)((pDateTime->month*275) / 9);

  return((TMWTYPES_USHORT)((tempValue3 - (tempValue1 * tempValue2)) + pDateTime->dayOfMonth - 30));
}

/*
 * FUNCTION:  tmwdtime_addOffset() - add offset to date/time structure
 *
 * This function adds the passed offset to the date/time value and resets
 * the dayOfMonth, dayOfWeek, month, and year if necessary, handling non
 * 31-day months, leap days, and leap years along the way.
 *
 * CAVEATS:
 *   This function assumes every fourth year is a leap year.  While this is
 *   valid from 1901 through 2099, it is not valid in century years that are
 *   not multiples of 400; i.e., 1600 was a leap year; 1700, 1800, and 1900
 *   were not leap years; 2000 is a leap year; 2100, 2200, and 2300 will not
 *   be leap years.
 *
 *   When the dayOfWeek is not being used, it is zero, and should not change
 */
void TMWDEFS_GLOBAL tmwdtime_addOffset(
  TMWDTIME *pDateTime, 
  TMWTYPES_ULONG mSecsAndSecsOffset)
{
  /* Avoid unnecessary processing */
  if(mSecsAndSecsOffset != 0UL)
  {
    /* Add date time seconds and ms into offset */
    mSecsAndSecsOffset += (TMWTYPES_ULONG)pDateTime->mSecsAndSecs;

    /* Add minutes one at a time, decrementing offset as we go, until 
     * we have less than one minute remaining
     */
    while(mSecsAndSecsOffset >=60000UL)
    {
      _incrementMinute(pDateTime);
      mSecsAndSecsOffset -= 60000UL;
    }

    /* Remaining seconds and ms are date time seconds and ms */
    pDateTime->mSecsAndSecs = (TMWTYPES_USHORT)mSecsAndSecsOffset;
  }
}

/*****************************************************************************/
/*                                                                           */
/* FUNCTION:  tmwdtime_subtractOffset() - subtract offset from date/time     */
/*                                                                           */
/* This function subtracts an offset from the date/time structure, and, if   */
/* necessary resets the dayOfWeek, dayOfMonth, month, and year, handling non */
/* 31-day months, leap days, and leap years along the way.                   */
/*                                                                           */
/* CAVEATS:                                                                  */
/*   This function calls tmwdtime_decrementHour(), which assumes every       */
/*   fourth year is a leap year.  While this is valid from 1901 through      */
/*   2099, it is not valid in century years that are not multiples of 400;   */
/*   i.e., 1600 was a leap year; 1700, 1800, and 1900 were not leap years;   */
/*   2000 is a leap year; 2100, 2200, and 2300 will not be leap years.       */
/*****************************************************************************/

void TMWDEFS_GLOBAL tmwdtime_subtractOffset(
  TMWDTIME *pDateTime,
  TMWTYPES_ULONG mSecsAndSecsOffset)
{
  /* Avoid unnecessary processing */
  if(mSecsAndSecsOffset != 0UL)
  {
    if(mSecsAndSecsOffset > (TMWTYPES_ULONG)pDateTime->mSecsAndSecs)
    {
      mSecsAndSecsOffset -= (TMWTYPES_ULONG)pDateTime->mSecsAndSecs;
      _decrementMinute(pDateTime);

      while(mSecsAndSecsOffset >=60000UL)
      {
        _decrementMinute(pDateTime);
        mSecsAndSecsOffset -= 60000UL;
      } 

      pDateTime->mSecsAndSecs = (TMWTYPES_USHORT)(60000UL - mSecsAndSecsOffset);
    }
    else
    {
      pDateTime->mSecsAndSecs = (TMWTYPES_USHORT)
        (pDateTime->mSecsAndSecs - (TMWTYPES_USHORT)mSecsAndSecsOffset);
    }
  }
}

/*****************************************************************************/
/*                                                                           */
/* FUNCTION:  tmwdtime_addMinutes() - add minutes to date/time               */
/*                                                                           */
/*****************************************************************************/

void TMWDEFS_GLOBAL tmwdtime_addMinutes(  
  TMWDTIME *pDateTime,       /* source and destination           */
  TMWTYPES_UCHAR minutes)    /* 59 or less */
{
  pDateTime->minutes = (TMWTYPES_UCHAR)(pDateTime->minutes + minutes);
  if(pDateTime->minutes > 59)
  {
    pDateTime->minutes -=60;
    tmwdtime_incrementHour(pDateTime);
  }  

} /* end tmwdtime_addMinutes() */

/*****************************************************************************/
/*                                                                           */
/* FUNCTION:  tmwdtime_decrementHour() - subtracts minutes from date/time    */ 
/*****************************************************************************/

void TMWDEFS_GLOBAL tmwdtime_subtractMinutes(  
  TMWDTIME *pDateTime,       /* source and destination           */
  TMWTYPES_UCHAR minutes)    /* 59 or less */
{
  if(pDateTime->minutes < minutes)
  {
    pDateTime->minutes += 60;
    tmwdtime_decrementHour(pDateTime);
  }
  pDateTime->minutes = (TMWTYPES_UCHAR)(pDateTime->minutes - minutes);
} /* end tmwdtime_subtractMinutes() */

/*****************************************************************************/
/*                                                                           */
/* FUNCTION:  tmwdtime_adjustMinutes() - add or subtract minutes to date/time*/
/*                                                                           */
/*****************************************************************************/

void TMWDEFS_GLOBAL tmwdtime_adjustMinutes(  
  TMWDTIME *pDateTime,       /* source and destination           */
  TMWTYPES_SHORT bias)       /* -1380 < minutes < 1380 */
{
  if (bias > 0)
  {
    int i;
    TMWTYPES_UCHAR minutesBias = bias % 60;
    int hoursBias = bias / 60;
    tmwdtime_addMinutes(pDateTime,minutesBias);
    for (i=0;i<hoursBias;i++)
    {
      tmwdtime_incrementHour(pDateTime);
    }
  } else if (bias < 0)
  {
    int i;
    TMWTYPES_UCHAR minutesBias;
    int hoursBias;
    minutesBias = -bias % 60;
    hoursBias = -bias / 60;
    tmwdtime_subtractMinutes(pDateTime,minutesBias);
    for (i=0;i<hoursBias;i++)
    {
      tmwdtime_decrementHour(pDateTime);
    }
  }

} /* end tmwdtime_adjustMinutes() */

/*****************************************************************************/
/*                                                                           */
/* FUNCTION:  tmwdtime_incrementHour() - increment an hour from date/time    */
/*                                                                           */
/* This function increments the date/time value by one hour, and resets the  */
/* dayOfMonth, month, and year if necessary, handling non 31-day months,     */
/* leap days, and leap years along the way.                                  */
/*                                                                           */
/* CAVEATS:                                                                  */
/*   This function assumes every fourth year is a leap year.  While this is  */
/*   valid from 1901 through 2099, it is not valid in century years that are */
/*   not multiples of 400; i.e., 1600 was a leap year; 1700, 1800, and 1900  */
/*   were not leap years; 2000 is a leap year; 2100, 2200, and 2300 will not */
/*   be leap years.                                                          */
/*                                                                           */
/*   When the dayOfWeek is not being used, it is zero, and should not change */
/*****************************************************************************/

void TMWDEFS_GLOBAL tmwdtime_incrementHour( /* increment an hour from date/time */
  TMWDTIME *pDateTime)       /* source and destination           */
{
  if(pDateTime->hour < 23)   /* assumes hour is 0-23 */
  {
    pDateTime->hour++;
  }
  else
  {
    pDateTime->hour = 0;
    tmwdtime_incrementDay(pDateTime);
  }
} /* end tmwdtime_incrementHour() */

/*****************************************************************************/
/*                                                                           */
/* FUNCTION:  tmwdtime_decrementHour() - decrement an hour from date/time    */
/*                                                                           */
/* This function decrements the date/time value by one hour, and resets the  */
/* dayOfMonth, month, and year if necessary, handling non 31-day months,     */
/* leap days, and leap years along the way.                                  */
/*                                                                           */
/* CAVEATS:                                                                  */
/*   This function calls tmwdtime_computeDayOfYear(), which assumes every    */
/*   fourth year is a leap year.  While this is valid from 1901 through      */
/*   2099, it is not valid in century years that are not multiples of 400;   */
/*   i.e., 1600 was a leap year; 1700, 1800, and 1900 were not leap years;   */
/*   2000 is a leap year; 2100, 2200, and 2300 will not be leap years.       */
/*                                                                           */
/*   When the dayOfWeek is not being used, it is zero, and should not change */
/*****************************************************************************/

void TMWDEFS_GLOBAL tmwdtime_decrementHour( /* decrement an hour from date/time */
  TMWDTIME *pDateTime)       /* source and destination           */
{
  if(pDateTime->hour > 0)
  {
    pDateTime->hour--;
  }
  else
  {
    pDateTime->hour = 23;
    tmwdtime_decrementDay(pDateTime);
  }
} /* end tmwdtime_decrementHour() */
/*****************************************************************************/
/*                                                                           */
/* FUNCTION:  tmwdtime_incrementDay() - increment a day from date/time       */
/*                                                                           */
/* This function increments the date/time value by one day, and resets the   */
/* month, and year if necessary, handling non 31-day months, leap days, and  */
/* leap years along the way.                                                 */
/*                                                                           */
/* CAVEATS:                                                                  */
/*   This function assumes every fourth year is a leap year.  While this is  */
/*   valid from 1901 through 2099, it is not valid in century years that are */
/*   not multiples of 400; i.e., 1600 was a leap year; 1700, 1800, and 1900  */
/*   were not leap years; 2000 is a leap year; 2100, 2200, and 2300 will not */
/*   be leap years.                                                          */
/*                                                                           */
/*   When the dayOfWeek is not being used, it is zero, and should not change */
/*****************************************************************************/

void TMWDEFS_GLOBAL tmwdtime_incrementDay( /* increment a day from date/time                */
  TMWDTIME *pDateTime)      /* source and destination                        */
{
  if(pDateTime->dayOfWeek != 0)
  {
    pDateTime->dayOfWeek++;
    if(pDateTime->dayOfWeek == 8)
    {
      pDateTime->dayOfWeek = 1;
    }
  }

  pDateTime->dayOfMonth++;

  /*******************************************************************/
  /* Test for rollover into the next month by testing the day of the */
  /* month with respect to the month and leap year. if its February  */
  /* 29th and not a leap year, or if its February 30th (the previous */
  /* day must have been a leap day), or if ist April 31st, June      */
  /* 31st, September 31st, or November 31st, or if it is the 32nd of */
  /* any month, then we must have rolled over.                       */
  /*******************************************************************/

  if(((pDateTime->dayOfMonth == 29) && (pDateTime->month == 2) &&
                                        ((pDateTime->year % 4) != 0)) ||
      ((pDateTime->dayOfMonth == 30) && (pDateTime->month == 2)) ||
      ((pDateTime->dayOfMonth == 31) && ((pDateTime->month == 4) ||
                                         (pDateTime->month == 6) ||
                                         (pDateTime->month == 9) ||
                                         (pDateTime->month == 11))) ||
      (pDateTime->dayOfMonth == 32))
  {
    pDateTime->dayOfMonth = 1; /* assumes day of month is 1-28,29,30,31 */
    pDateTime->month++;

    if(pDateTime->month > 12)   /* assumes month is 1-12 */
    {
      pDateTime->month = 1;
      pDateTime->year++;
    }
  }
} /* end tmwdtime_incrementDay() */

/*****************************************************************************/
/*                                                                           */
/* FUNCTION:  tmwdtime_decrementDay() - decrement a day from date/time       */
/*                                                                           */
/* This function decrements the date/time value by one day, and resets the   */
/* month, and year if necessary, handling non 31-day months, leap days, and  */
/* leap years along the way.                                                 */
/*                                                                           */
/* CAVEATS:                                                                  */
/*   This function calls tmwdtime_computeDayOfYear(), which assumes every    */
/*   fourth year is a leap year.  While this is valid from 1901 through      */
/*   2099, it is not valid in century years that are not multiples of 400;   */
/*   i.e., 1600 was a leap year; 1700, 1800, and 1900 were not leap years;   */
/*   2000 is a leap year; 2100, 2200, and 2300 will not be leap years.       */
/*                                                                           */
/*   When the dayOfWeek is not being used, it is zero, and should not change */
/*****************************************************************************/

void TMWDEFS_GLOBAL tmwdtime_decrementDay( /* decrement a day from date/time */
  TMWDTIME *pDateTime)      /* source and destination                        */
{
  if(pDateTime->dayOfWeek != 0)
  {
    pDateTime->dayOfWeek--;
    if(pDateTime->dayOfWeek == 0)
    {
      pDateTime->dayOfWeek = 7;
    }
  }

  if(pDateTime->dayOfMonth > 1)
  {
    pDateTime->dayOfMonth--;
  }
  else
  {
    unsigned short day_of_year = tmwdtime_computeDayOfYear(pDateTime);

    if(day_of_year == 1)
    {
      if(pDateTime->year > 0)
      {
        pDateTime->year--;
      }

      pDateTime->month = 12;
      pDateTime->dayOfMonth = 31;
    }
    else
    {
      /*********************************************************************/
      /* Set the day of the month to an invalid number, and then keep      */
      /* backing it up until the computed day of the year is correct.      */
      /* This assumes that the day of year calculation correctly handles   */
      /* values for the day of a month which is greater than possible for  */
      /* that particular month, which it does.                             */
      /*********************************************************************/

      pDateTime->dayOfMonth = 32;
      pDateTime->month--;
      day_of_year--;

      do
      {
        pDateTime->dayOfMonth--;
      }
      while(day_of_year != tmwdtime_computeDayOfYear(pDateTime));
    }
  }
} /* end tmwdtime_decrementDay() */

/*****************************************************************************/
/*                                                                           */
/* FUNCTION: tmwdtime_computeDayOfWeek() - returns day of week 1-7 (Mon-Sun) */
/*                                                                           */
/* For the given month, dayOfMonth, and year, this function computes and     */
/* returns the correct day of week.                                          */
/*                                                                           */
/* RETURNS:                                                                  */
/*   TMWTYPES_USHORT dayOfWeek from 1 to 7 where 1=Monday, 7=Sunday          */
/*                                                                           */
/* CAVEATS:                                                                  */
/*   This function assumes every fourth year is a leap year (and it calls    */
/*   tmwdtime_computeDayOfYear(), which also makes the same assumption).     */
/*   While this is valid from 1901 through 2099, it is not valid in century  */
/*   years that are not multiples of 400; i.e., 1600 was a leap year; 1700,  */
/*   1800, and 1900 were not leap years; 2000 is a leap year; 2100, 2200,    */
/*   and 2300 will not be leap years.                                        */
/*****************************************************************************/

TMWTYPES_UCHAR TMWDEFS_GLOBAL tmwdtime_computeDayOfWeek( /* returns day of week 1-7          */
                                         /* (Mon-Sun)                        */
  const TMWDTIME *pDateTime)             /* the source date/time             */
{
  TMWTYPES_USHORT result;

  /* Next, compute day of week of first day of year (using day of week from  */
  /* 0 to 6).  This is valid for years from 1901 to 2099; i.e., it assumes   */
  /* every fourth year is a leap year.  This computation is based on the     */
  /* fact the first day of the year increments by one each year after        */
  /* non-leap years, and by two after a leap year. Therefore, the day of the */
  /* week of the first day of a year is the day of the week of the first day */
  /* of 1901 (Tuesday or day 1 where 0=Monday), plus the number of years     */
  /* since 1901, plus the number of leap days since Jan 1, 1901. (Note that  */
  /* on Jan 1 of a leap year, a leap day hasn't occurred yet for that year;  */
  /* e.g., from Jan 1, 1905 to Jan 1, 1908, there was only one leap day.)    */

  result = (TMWTYPES_USHORT) (1 + (pDateTime->year - 1901) + ((pDateTime->year - 1901)/4));

  /* Next, add to this the number of days of the year.  Since the result of  */
  /* that calculation starts at one (i.e., January 1 is day 1), we have to   */
  /* subtract 1 from this value.                                             */

  result = (TMWTYPES_USHORT) (result + tmwdtime_computeDayOfYear(pDateTime) - 1);

  /* Finally, modulo the result by 7 and add one to have result represent    */
  /* days of the week from 1 to 7 (1=Monday).                                */

  return((TMWTYPES_UCHAR) ((result % 7) + 1));
}

/*****************************************************************************/
/* Triangle MicroWorks, Inc.                         Copyright (c) 1997-2022 */
/*****************************************************************************/
/*                                                                           */
/* FUNCTION: tmwdtime_hoursAmPmTo24 - converts am/pm hours to 24 hour format */
/*                                                                           */
/*****************************************************************************/

TMWTYPES_UCHAR TMWDEFS_GLOBAL tmwdtime_hoursAmPmTo24( /* converts am/pm hours to 24 hour     */
                                      /* format                              */
  TMWTYPES_UCHAR amPmHour,            /* the hours in am/pm format           */
  TMWTYPES_BOOL  isPm)                /* true if pm                          */
{
  if(isPm)
  {
    /* 12pm is the 12th hour in 24 hour format; all other pm hours are hours */
    /* 13 through 23                                                         */
    if(amPmHour != 12) 
    {
      amPmHour += 12;
    }
  }
  else
  {
    /* 12am is the 0th hour in 24 hour format */
    if(amPmHour == 12) 
    {
      amPmHour =  0;
    }
  }

  return(amPmHour);
} /* end tmwdtime_hoursAmPmTo24() */

/*****************************************************************************/
/* Triangle MicroWorks, Inc.                         Copyright (c) 1997-2022 */
/*****************************************************************************/
/*                                                                           */
/* FUNCTION: tmwdtime_hours24ToAmPm - converts 24 hour format to am/pm hours */
/*                                    format                                 */
/*                                                                           */
/*****************************************************************************/

TMWTYPES_UCHAR TMWDEFS_GLOBAL tmwdtime_hours24ToAmPm( /* converts 24 hour format to am/pm  */
                                      /* hours format                        */
  TMWTYPES_UCHAR  hours24,            /* the hours in 24 hour format         */
  TMWTYPES_BOOL  *isPm)               /* set to true if pm                   */
{
  if(hours24 == 0)
  {
    /* the 0th hour is 12am */
    *isPm   = TMWDEFS_FALSE;
    hours24 = 12;
  }
  else if(hours24 == 12)
  {
    /* the 12th hour is 12pm */
    *isPm = TMWDEFS_TRUE;
  }
  else if(hours24 > 12)
  {
    /* hours over 12 are pm hours 1 to 11 */
    *isPm = TMWDEFS_TRUE;
    hours24 -= 12;
  }
  else
  {
    /* all other hours are am hours 1 to 11 */
    *isPm = TMWDEFS_FALSE;
  }

  return(hours24);
} /* end tmwdtime_hours24ToAmPm() */

/*****************************************************************************/
/* Triangle MicroWorks, Inc.                         Copyright (c) 1997-2022 */
/*****************************************************************************/
/*                                                                           */
/* FUNCTION: tmwdtime_checkTimeOrder - compares two times in TMWDTIME        */
/*                                     format.  if time1 is greater than     */
/*                                     time2, the function returns           */
/*                                     TMWDEFS_FALSE.  if time2 is greater   */
/*                                     than or equal to time1, the function  */
/*                                     returns TMWDEFS_TRUE.  The invalid    */
/*                                     bit from the TMWDTIME structure is    */
/*                                     ignored.  The day of week member of   */
/*                                     the TMWDTIME structure is not tested, */
/*                                     since the day of month is tested.     */
/*****************************************************************************/

TMWTYPES_BOOL TMWDEFS_GLOBAL tmwdtime_checkTimeOrder(  /* checks time contained in request   */
  TMWDTIME  *time1,                    /* time #1 to compare to time #2      */
  TMWDTIME  *time2)                    /* time #2 to compare to time #1      */
{

  if(time2->year != time1->year)
  {
    if(time2->year > time1->year)
    {
      return(TMWDEFS_TRUE);
    }
    else
    {
      return(TMWDEFS_FALSE);
    }
  }

  /* if here, year is equal so continue to month */
  if(time2->month != time1->month)
  {
    if(time2->month > time1->month)
    {
      return(TMWDEFS_TRUE);
    }
    else
    {
      return(TMWDEFS_FALSE);
    }
  }

  /* if here, month is equal so continue to day of month */
  if(time2->dayOfMonth != time1->dayOfMonth)
  {
    if(time2->dayOfMonth > time1->dayOfMonth)
    {
      return(TMWDEFS_TRUE);
    }
    else
    {
      return(TMWDEFS_FALSE);
    }
  }

  /* if here, day of month is equal. No need to test     */
  /* day of week member of structure so continue to hour */
  if(time2->hour != time1->hour)
  {
    if(time2->hour > time1->hour)
    {
      return(TMWDEFS_TRUE);
    }
    else
    {
      return(TMWDEFS_FALSE);
    }
  }

 /* if here, hour is equal so continue to minutes */
  if(time2->minutes != time1->minutes)
  {
    if(time2->minutes > time1->minutes)
    {
      return(TMWDEFS_TRUE);
    }
    else
    {
      return(TMWDEFS_FALSE);
    }
  }

  /* if here, minutes are equal so continue to */
  /* milliseconds and seconds                  */
  if(time2->mSecsAndSecs >= time1->mSecsAndSecs)
  {
    return(TMWDEFS_TRUE);
  }
  else
  {
    return(TMWDEFS_FALSE);
  }

}

/* function: tmwdtime_compareTime */
TMWTYPES_INT TMWDEFS_GLOBAL tmwdtime_compareTime(  /* checks time contained in request   */
  TMWDTIME  *time1,                    /* time #1 to compare to time #2      */
  TMWDTIME  *time2)                    /* time #2 to compare to time #1      */
{

  if(time1->year != time2->year)
  {
    if(time1->year > time2->year)
    {
      return(1);
    }
    if(time1->year < time2->year)
    {
      return(-1);
    }
  }

  /* if here, year is equal so continue to month */
  if(time1->month != time2->month)
  {
    if(time1->month > time2->month)
    {
      return(1);
    }
    if(time1->month < time2->month)
    {
      return(-1);
    }
  }

  /* if here, month is equal so continue to day of month */
  if(time1->dayOfMonth != time2->dayOfMonth)
  {
    if(time1->dayOfMonth > time2->dayOfMonth)
    {
      return(1);
    }
    if(time1->dayOfMonth < time2->dayOfMonth)
    {
      return(-1);
    }
  }

  /* if here, day of month is equal. No need to test     */
  /* day of week member of structure so continue to hour */
  if(time1->hour != time2->hour)
  {
    if(time1->hour > time2->hour)
    {
      return(1);
    }
    if(time1->hour < time2->hour)
    {
      return(-1);
    }
  }

 /* if here, hour is equal so continue to minutes */
  if(time1->minutes != time2->minutes)
  {
    if(time1->minutes > time2->minutes)
    {
      return(1);
    }
    if(time1->minutes < time2->minutes)
    {
      return(-1);
    }
  }

  /* if here, minutes are equal so continue to */
  /* milliseconds and seconds                  */
  if(time1->mSecsAndSecs > time2->mSecsAndSecs)
  {
    return(1);
  }
  if(time1->mSecsAndSecs < time2->mSecsAndSecs)
  {
    return(-1);
  }

  return(0); /* time1 is equal to time2 */
}

/* function: tmwdtime_getDateTime */
void TMWDEFS_GLOBAL tmwdtime_getDateTime(
  void *pSession,
  TMWDTIME *pDateTime)
{ 
  /* Get system date and time.
   * Specifying session allows target to return session specific time
   */
  pDateTime->pSession = pSession;
  tmwtarg_getDateTime(pDateTime);
}
