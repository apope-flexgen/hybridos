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
/*
 * FileName : WinIoSimulatedTime.cpp
 * Author   : Fred VanEijk
 * Purpose  : Implement the simulated time
 *            
 */

#include "StdAfx.h"
#include "WinIoTarg/WinIoSimulatedTime.h"
#include "WinIoTarg/include/WinIoTarg.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif


/*****************************************************************************/
/*                                                                           */
/* FUNCTION:  _incrementDay() - increment a day from date/time               */
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

static void TMWDEFS_LOCAL _incrementDay( /* increment a day from date/time   */
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
  /* month with respect to the month and leap year. ifits February  */
  /* 29th and not a leap year, or ifits February 30th (the previous */
  /* day must have been a leap day), or ifist April 31st, June      */
  /* 31st, September 31st, or November 31st, or ifit is the 32nd of */
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
/* FUNCTION:  _incrementHour() - increment an hour from date/time            */
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

static void TMWDEFS_GLOBAL _incrementHour( /* increment an hour from date/time */
  TMWDTIME *pDateTime)       /* source and destination           */
{
  if(pDateTime->hour < 23)   /* assumes hour is 0-23 */
  {
    pDateTime->hour++;
  }
  else
  {
    pDateTime->hour = 0;
    _incrementDay(pDateTime);
  }
} /* end _incrementHour() */

static void TMWDEFS_GLOBAL _incrementMinute(
  TMWDTIME *pDateTime)
{
  pDateTime->minutes += 1;
  if(pDateTime->minutes >= 60)
  {
    pDateTime->minutes -= 60;
    _incrementHour(pDateTime);
  }
}

/*
 * FUNCTION:  _addOffset() - add offset to date/time structure
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
static void TMWDEFS_LOCAL  _addOffset(
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

/* function: getDateTime */
void WinIoSimulatedTime::getDateTime(TMWDTIME *pDateTime)
{
  void *pSession = pDateTime->pSession;
  TMWTYPES_MILLISECONDS freeRunMilliseconds = GetTickCount();

  /* the following code is used to simulate a clock on the target device */
  _addOffset(&m_simulatedClock, freeRunMilliseconds - m_mSecsWhenLastCalled);

  /* Update last called */
  m_mSecsWhenLastCalled = freeRunMilliseconds;

  *pDateTime = m_simulatedClock;
  pDateTime->pSession = pSession;
}

/* function: setDateTime */
TMWTYPES_BOOL WinIoSimulatedTime::setDateTime(const TMWDTIME *pNewDateTime)
{
  m_simulatedClock = *pNewDateTime;
  m_mSecsWhenLastCalled = GetTickCount();
  return true;
}
