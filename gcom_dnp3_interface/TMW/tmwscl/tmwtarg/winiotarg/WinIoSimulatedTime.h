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
 * FileName : WinIoSimulatedTime.h
 * Author   : Fred VanEijk
 * Purpose  : Implementation of simulated time (i.e. uses the PC Tick counter)
 *            
 */

#ifndef WinIoSimulatedTime_DEFINED
#define WinIoSimulatedTime_DEFINED

#include "tmwscl/utils/tmwtarg.h"
#include "WinIoTarg/WinIoBaseTime.h"

class WinIoSimulatedTime : public WinIoBaseTime
{
public:
  WinIoSimulatedTime(WINIO_TIME_MODE timeMode)
  {
    m_timeMode = timeMode;
    /* At startup read the system clock once */
      struct _timeb timeBuffer;
      _ftime(&timeBuffer);

      convertLocalTime(&m_simulatedClock, timeBuffer.time);
      m_simulatedClock.mSecsAndSecs = (TMWTYPES_USHORT)(m_simulatedClock.mSecsAndSecs + timeBuffer.millitm);
    m_mSecsWhenLastCalled = GetTickCount();
  }

  /* function: getDateTime
   * purpose: return the current date and time
   * arguments:
   *  pDateTime - storage to return date and time into
   * returns:
   *  void
   */
  virtual void getDateTime(TMWDTIME *pDateTime);

  /* Set clock to specified time. Future calls to
   * getDateTime will return a clock based on this time.
   */
  virtual TMWTYPES_BOOL setDateTime(const TMWDTIME *pNewDateTime);

private:
  TMWDTIME              m_simulatedClock;
  TMWTYPES_MILLISECONDS m_mSecsWhenLastCalled;

};

#endif // WinIoSimulatedTime_DEFINED