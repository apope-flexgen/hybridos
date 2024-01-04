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

/* file: dnpdtime.h
 * description: DNP3 data time support
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/dnpdtime.h"
#include "tmwscl/utils/tmwtarg.h"

/* function: dnpdtime_dateTimeToMSSince70 */
void TMWDEFS_GLOBAL dnpdtime_dateTimeToMSSince70(
  TMWTYPES_MS_SINCE_70 *pMsSince70, 
  TMWDTIME *pDateTime)
{
  TMWTYPES_ULONG mostSignificant;
  TMWTYPES_ULONG leastSignificant;

  /* Year is not allowed to predate 1970 */
  if (pDateTime->year < 1970)
    pDateTime->year = 1970;

  /* first calculate the number of days in the years from 1/1/70 to the year
   * preceeding the current one, assuming every fourth year was a leap year 
   * (valid from 1970-2099)                                                  
   */
  mostSignificant = 
    ((pDateTime->year - 1970) * 365) 
    + ((pDateTime->year - 1969)/4);

  /* now add in the days in the current year up to, but not including the
   * current day.                                                            
   */
  mostSignificant += tmwdtime_computeDayOfYear(pDateTime) - 1;

  /* now multiply by 24 to put this in units of hours and add in the number
   * of hours in the current day up to, but not including the current hour.  
   */
  mostSignificant = (mostSignificant * 24) + (TMWTYPES_ULONG)pDateTime->hour;

  /* now multiply by 60 to put this in units of minutes and add in the
   * number of minutes in the current hour up to, but not including the
   * current minute.                                                         
   */
  mostSignificant = (mostSignificant * 60) + (TMWTYPES_ULONG)pDateTime->minutes;

  /* mostSignificant now holds the number of minutes since 1/1/70.  We will
   * copy the seconds and milliseconds into a temp variable.  Since its
   * maximum value is 60,000 which is close to 64K, the math is easier to
   * convert to the 6 byte DNP format.                                   
   */
  leastSignificant = pDateTime->mSecsAndSecs;
  leastSignificant += (mostSignificant&0xffff)*60000U;

  mostSignificant = ((mostSignificant>>16)*60000U) + (leastSignificant>>16);

  /* Now store into DNP 6 byte format */
  pMsSince70->leastSignificant = (TMWTYPES_USHORT)leastSignificant;
  pMsSince70->mostSignificant  = mostSignificant;
}

/* function: dnpdtime_msSince70ToDateTime */
void TMWDEFS_GLOBAL dnpdtime_msSince70ToDateTime(
  TMWDTIME *pDateTime, 
  const TMWTYPES_MS_SINCE_70 *pMsSince70)
{
  TMWTYPES_ULONG tmp1, tmp2;

  /* Convert 6 byte time into two numbers we can process, specifically one 
   * 32 bit number that holds the number of minutes since 1/1/70 and another
   * that holds the number of seconds and milliseconds in the current minute
   */
  tmp1 = (pMsSince70->mostSignificant % 60000U) << 16;
  tmp2 = ((pMsSince70->mostSignificant / 60000U) << 16) 
    + ((tmp1 + pMsSince70->leastSignificant) / 60000U);

  /* Store seconds and milliseconds */
  pDateTime->mSecsAndSecs = (TMWTYPES_USHORT)
    ((tmp1 + pMsSince70->leastSignificant) % 60000UL);

  /* Store minutes */
  pDateTime->minutes = (TMWTYPES_UCHAR)(tmp2 % 60);

  /* Convert to hours */
  tmp2 = tmp2  / 60;
  pDateTime->hour = (TMWTYPES_UCHAR)(tmp2 % 24);

  /* Convert to days */
  tmp2 = tmp2  / 24;

  /* Guess the year, ignoring leap years */
  pDateTime->year = (TMWTYPES_USHORT)((tmp2 / 365) + 1970);

  /* Calculate the number of days since Jan 1, 1970:  Add the number
   * of days in the current year to the number of years (since 1970)
   * 365, plus the number of leap days in preceeding years.
   */
  tmp1 = 
    ((((TMWTYPES_ULONG)pDateTime->year) - 1970) * 365) + 
    ((((TMWTYPES_ULONG)pDateTime->year) - 1969) / 4);

  /* Now adjust the year, compensating for leap years. This algoritm will
   * break in the year 2100 since it is not a leap year.
   */
  while(tmp1 > tmp2)
  {
    /* If this computed number of days based on the year guess is greater
     * than the actual number of days (in tmp2), then our guess
     * was wrong, decrement the year and try again.
     */
    pDateTime->year--;

    /* Calculate the number of days since Jan 1, 1970:  Add the number
     * of days in the current year to the number of years (since 1970)
     * 365, plus the number of leap days in preceeding years.
     */
    tmp1 = 
      ((((TMWTYPES_ULONG)pDateTime->year) - 1970) * 365) + 
      ((((TMWTYPES_ULONG)pDateTime->year) - 1969) / 4);
  }

  /* Now subtract all the days up until the computed year, leaving the
   * number of days in the computed year. Add one to the count of days
   * within the year because tmwdtime_computeDayOfYear() will compute Jan 1
   * as day 1, not day 0.                                                    
   */
  tmp2 = tmp2 - tmp1 + 1; /* now contains days since Jan 1. */

  /* As we did for years, take a guess at the month and day of month, and
   * then correct it by subtracting days until the computed day of year is
   * the same as the actual day of the year.                                 
   */
  pDateTime->month = (TMWTYPES_UCHAR)(tmp2/28 + 1);
  pDateTime->dayOfMonth = (TMWTYPES_UCHAR)(tmp2%28 + 1);

  while(tmwdtime_computeDayOfYear(pDateTime) != tmp2)
  {
    pDateTime->dayOfMonth--;
    if(pDateTime->dayOfMonth == 0)
    {
      pDateTime->month--;
      pDateTime->dayOfMonth = 31;
    }
  }

  pDateTime->dstInEffect = TMWDEFS_FALSE; /* daylight savings time is unknown */
  pDateTime->invalid = TMWDEFS_FALSE; /* the conversion is valid */
  pDateTime->dayOfWeek = (TMWTYPES_UCHAR)tmwdtime_computeDayOfWeek(pDateTime);

  pDateTime->qualifier = TMWDTIME_ABSOLUTE;
}

/* function: dnpdtime_writeMsSince70 */
void TMWDEFS_GLOBAL dnpdtime_writeMsSince70(
  TMWTYPES_UCHAR *destination, 
  const TMWTYPES_MS_SINCE_70 *pMsSince70)
{
  tmwtarg_store16(&pMsSince70->leastSignificant, destination);
  tmwtarg_store32(&pMsSince70->mostSignificant, destination + 2);
}

/* function: dnpdtime_readMsSince70 */
void TMWDEFS_GLOBAL dnpdtime_readMsSince70(
  TMWTYPES_MS_SINCE_70 *pMsSince70, 
  const TMWTYPES_UCHAR *source)
{
  tmwtarg_get16(source, &pMsSince70->leastSignificant);
  tmwtarg_get32(source + 2, &pMsSince70->mostSignificant);
}

/* function: dnpdtime_addTime */
void TMWDEFS_GLOBAL dnpdtime_addTime(
  TMWTYPES_MS_SINCE_70 *pDestMsSince70,
  const TMWTYPES_MS_SINCE_70 *pOperMsSince70, 
  TMWTYPES_ULONG milliseconds)
{
  TMWTYPES_ULONG tempLowTime;

  pDestMsSince70->mostSignificant = pOperMsSince70->mostSignificant;

  tempLowTime = 
    (TMWTYPES_ULONG)pOperMsSince70->leastSignificant 
    + (TMWTYPES_ULONG)milliseconds;

  /* Check for overflow */
  if(tempLowTime < milliseconds)
  {
    /* add the overflow in */    
    pDestMsSince70->mostSignificant += 0x00010000UL;
  }

  pDestMsSince70->mostSignificant += (tempLowTime >> 16);
  pDestMsSince70->leastSignificant = (TMWTYPES_USHORT) tempLowTime;
}

/* function: dnpdtime_subtractTime */
void TMWDEFS_GLOBAL dnpdtime_subtractTime(
  TMWTYPES_MS_SINCE_70 *pDestMsSince70,
  const TMWTYPES_MS_SINCE_70 *pOper1MsSince70, 
  const TMWTYPES_MS_SINCE_70 *pOper2MsSince70)
{
  TMWTYPES_ULONG tempLowTime;
  TMWTYPES_ULONG tempHighTime;

  tempHighTime = pOper1MsSince70->mostSignificant;

  tempLowTime = 
   (TMWTYPES_ULONG)pOper1MsSince70->leastSignificant - 
   (TMWTYPES_ULONG)pOper2MsSince70->leastSignificant;

  /* first determine if the difference (above) was negative */
  if(tempLowTime & 0x80000000UL)
  {
    /* borrow 'one' from most significant and add it in to 
     * the least significant to get it positive. 
     */    
    tempHighTime -= 1;
    tempLowTime  += 0x10000UL;
  }

  pDestMsSince70->mostSignificant = 
    tempHighTime - pOper2MsSince70->mostSignificant;

  pDestMsSince70->leastSignificant = (TMWTYPES_USHORT) tempLowTime;
}
