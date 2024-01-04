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
 * FileName : WinIoBaseTime.cpp
 * Author   : Fred VanEijk
 * Purpose  : Impelentation of Base time class
 *            
 */

#include "StdAfx.h"
#include "WinIoTarg/WinIoBaseTime.h"
#include "WinIoTarg/include/WinIoTarg.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

/* function: convertLocalTime */
void WinIoBaseTime::convertLocalTime(TMWDTIME *pDateTime, time_t timeIn)
{
  struct tm *pSystemTime = localtime(&timeIn);

  pDateTime->mSecsAndSecs = (TMWTYPES_USHORT)(pSystemTime->tm_sec * 1000); /* 0 to 59,999      */
  pDateTime->minutes      = (TMWTYPES_UCHAR)pSystemTime->tm_min;        /* 0 to 59          */
  pDateTime->hour         = (TMWTYPES_UCHAR)pSystemTime->tm_hour;       /* 0 to 23          */
  pDateTime->dayOfMonth   = (TMWTYPES_UCHAR)pSystemTime->tm_mday;       /* 1 to 28,29,30,31 */
  pDateTime->month        = (TMWTYPES_UCHAR)(pSystemTime->tm_mon+1);      /* 1 to 12          */

  /* convert from day of week 0 to 6, to 1 to 7 with Monday being 1 */
  pDateTime->dayOfWeek = (TMWTYPES_UCHAR)pSystemTime->tm_wday;  
  if (pDateTime->dayOfWeek == 0)
  {
    pDateTime->dayOfWeek = 7;  /* change Sunday from 0 to 7               */
  }

  /* convert from years 0 through 199 to 1900 - 2099 */
  pDateTime->year = (TMWTYPES_USHORT)(pSystemTime->tm_year+1900); /* 1901 to 2099 */

  pDateTime->dstInEffect = (pSystemTime->tm_isdst > 0);

  pDateTime->invalid = false; /* set valid */
  pDateTime->tis     = false; /* set no tis */
  pDateTime->qualifier = TMWDTIME_UNKNOWN;

  pDateTime->energyTariff     = 0;
  pDateTime->powerTariff      = 0;
}
