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
#ifndef TMWDTIME_DEFINED
#define TMWDTIME_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwtypes.h"
#include "tmwscl/utils/tmwcnfg.h"

/* Define enumerated type used to indicate how the time stamp was
 * generated when storing a binary input change event.
 */
typedef enum TMWDTIME_QUAL_ENUM
{
  TMWDTIME_UNKNOWN,      /* Time qualifier is not known                      */
  TMWDTIME_ABSOLUTE,     /* Absolute time was returned in message            */
  TMWDTIME_SYNC,         /* Time was calculated based on relative time       */
                         /* and a synchronized base time. This will          */
                         /* occur in DNP with a binary input with            */
                         /* relative time and a synchronized common          */
                         /* time of occurrence or in IEC 65870-5-101/104     */
                         /* when a 24 bit time is received after a           */
                         /* time sync event.                                 */
  TMWDTIME_UNSYNC        /* Time was calculated from a relative time         */
                         /* and an unsynchronized base time. For             */
                         /* example, this will occur in DNP with             */
                         /* a binary input with relative time and an         */
                         /* unsynchronized common time of occurrence or      */
                         /* in IEC 60870-5-101/104 when a 24 bit time        */
                         /* is received with out receiving a time sync       */
                         /* event.                                           */
} TMWDTIME_QUAL;

typedef struct TMWDateTime
{
  TMWTYPES_USHORT mSecsAndSecs; /* 0 to 59,999                              */
  TMWTYPES_UCHAR  minutes;      /* 0 to 59                                  */
  TMWTYPES_UCHAR  hour;         /* 0 to 23                                  */
  TMWTYPES_UCHAR  dayOfWeek;    /* 1 to 7 (0 = not used, 1= Monday)         */
  TMWTYPES_UCHAR  dayOfMonth;   /* 1 to 28,29,30,31                         */
  TMWTYPES_UCHAR  month;        /* 1 to 12                                  */
  TMWTYPES_USHORT year;         /* 1901 to 2099, the years over which the   */
                                /* calculations in this module are valid    */
  TMWTYPES_BOOL   dstInEffect;  /* TMWDEFS_TRUE when daylight savings time  */
                                /* in effect, otherwise TMWDEFS_FALSE       */
  TMWTYPES_BOOL   invalid;      /* set to TMWDEFS_TRUE when the date/time   */
                                /* is deemed inaccurate.                    */
  TMWTYPES_BOOL   genuineTime;  /* set to TMWDEFS_TRUE if time was acquired */
                                /* from RTU, TMWDEFS_FALSE if time was      */
                                /* substituted by intermediate equipment    */
                                /* (used only by IEC 60870-5-101/104 when   */
                                /* sending time in the monitor direction)   */
                                /* As defined in 101 edition 2 2003 7.2.6.18*/
  TMWTYPES_BOOL   tis;          /* TMWDEFS_TRUE if tariff info switch is on */
                                /* (used only by IEC 60870-5-102)           */
  TMWTYPES_UCHAR  energyTariff; /* Energy tariff info                       */
                                /* (used only by IEC 60870-5-102)           */
  TMWTYPES_UCHAR  powerTariff;  /* Power  tariff info                       */
                                /* (used only by IEC 60870-5-102)           */
  TMWDTIME_QUAL   qualifier;    /* Specifies how this time was generated    */    

  void           *pSession;     /* pointer to TMWSESN structure. This allows */
                                /* target layer to return time on per session*/
                                /* basis. This will be valid or set to       */
                                /* TMWDEFS_NULL before tmwtarg_getDateTime() */
                                /* is called                                 */
} TMWDTIME;

#ifdef __cplusplus
  extern "C"
  {
#endif

  /* Adds or subtracts a second/millisecond offset from the specified time */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdtime_addOffset(
    TMWDTIME *pDateTime, 
    TMWTYPES_ULONG mSecsAndSecsOffset);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdtime_subtractOffset(
    TMWDTIME *pDateTime, 
    TMWTYPES_ULONG mSecsAndSecsOffset);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdtime_adjustMinutes(
    TMWDTIME *pDateTime,
    TMWTYPES_SHORT bias);

  void TMWDEFS_GLOBAL tmwdtime_addMinutes(  
    TMWDTIME *pDateTime,
    TMWTYPES_UCHAR minutes);
 
  void TMWDEFS_GLOBAL tmwdtime_subtractMinutes(  
    TMWDTIME *pDateTime,  
    TMWTYPES_UCHAR minutes);
 
  /* Manipulate date time by hour or day */
  void TMWDEFS_GLOBAL tmwdtime_incrementHour(
    TMWDTIME *pDateTime);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdtime_decrementHour(
    TMWDTIME *pDateTime);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdtime_incrementDay(
    TMWDTIME *pDateTime);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdtime_decrementDay(
    TMWDTIME *pDateTime);

  /* Figure out what day of year/week the specified date is */
  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL tmwdtime_computeDayOfYear(
    const TMWDTIME *pDateTime);

  TMWDEFS_SCL_API TMWTYPES_UCHAR TMWDEFS_GLOBAL tmwdtime_computeDayOfWeek(
    const TMWDTIME *pDateTime);

  /* Convert to/from 24/12 hour clock */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL tmwdtime_hoursAmPmTo24(
    TMWTYPES_UCHAR amPmHour, 
    TMWTYPES_BOOL isPm);

  TMWTYPES_UCHAR TMWDEFS_GLOBAL tmwdtime_hours24ToAmPm(
    TMWTYPES_UCHAR hours24, 
    TMWTYPES_BOOL *isPm);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwdtime_checkTimeOrder(
    TMWDTIME *time1, 
    TMWDTIME *time2);

  /* function: tmwdtime_compareTime
   * purpose: Compares two TMWDTIME times
   *  handle will be invalid.
   * arguments:
   *  time1 - 1st time 
   *  time2 - 2nd time 
   * returns:
   *   1 if time1 is is greater than time2
   *   0 if time1 is is equal to time2
   *  -1 if time1 is is less than time2
   */
  TMWDEFS_SCL_API TMWTYPES_INT TMWDEFS_GLOBAL tmwdtime_compareTime(
    TMWDTIME *time1, 
    TMWDTIME *time2);

  /* function: tmwdtime_getDateTime 
   * purpose: Get current time. Session is specified so that session specific 
   *  time may be returned.
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdtime_getDateTime(
    void *pSession,
    TMWDTIME *pDateTime);

#ifdef __cplusplus
  }
#endif
#endif /* TMWDTIME_DEFINED */
