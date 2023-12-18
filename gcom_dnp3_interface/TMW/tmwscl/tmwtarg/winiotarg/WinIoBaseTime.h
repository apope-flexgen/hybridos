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
 * FileName : WinIoBaseTime.h
 * Author   : Fred VanEijk
 * Purpose  : Base class for target time interface
 *            
 */

#ifndef WinIoBaseTime_DEFINED
#define WinIoBaseTime_DEFINED

#include "tmwscl/utils/tmwtarg.h"
#include "WinIoTarg/include/WinIoTargDefs.h"

class WinIoBaseTime
{
public:

  /* function: convertLocalTime */
  void convertLocalTime(TMWDTIME *pDateTime, time_t timeIn);

  /* function: getDateTime
   * purpose: return the current date and time
   * arguments:
   *  pDateTime - storage to return date and time into
   * returns:
   *  void
   */
  virtual void getDateTime(TMWDTIME *pDateTime) = 0;

  /* Set clock to specified time. Future calls to
   * getDateTime will return a clock based on this time.
   */
  virtual TMWTYPES_BOOL setDateTime(const TMWDTIME *pNewDateTime) = 0;

  /* function: setWinIoTimeMode
   * purpose: set the current time mode
   * arguments:
   *  mode - the time mode 
   * returns:
   *  void
   */
  void setWinIoTimeMode(WINIO_TIME_MODE mode)
  {
    m_timeMode = mode;
  }

protected:
  WINIO_TIME_MODE m_timeMode;
};

#endif // WinIoBaseTime_DEFINED