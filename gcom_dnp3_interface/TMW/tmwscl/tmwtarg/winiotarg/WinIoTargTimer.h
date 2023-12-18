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

/* file: WinIoTargTimer.h
 * description: Interface to windows timers
 */

#ifndef __WinIoTargTimer_H__
#define __WinIoTargTimer_H__

#include "WinIoTarg/include/WinIoTargDefs.h"
#include "WinIoTarg/WinThreading.h"

#pragma warning(disable:4786)
#pragma warning(disable:4251)

#include <deque>
#include <map>

//using namespace std;

//Timer Types
typedef enum WINIOTARG_TIMER_TYPE_ENUM 
{ 
  TYPE_NONE=0, 
  TYPE_PROC, // timer with callback from timer thread
  TYPE_WIN   // timer that fires in same thread as creator
} WINIOTARG_TIMER_TYPE;

//Timer Mode
typedef enum WINIOTARG_TIMER_MODE_ENUM
{ 
  MODE_NONE=0, 
  MODE_PERIODIC, 
  MODE_TIMEOUT 
} WINIOTARG_TIMER_MODE;

// Timer signals
typedef enum WINIOTARGTIMER_EVENT_ENUM
{
    EVENT_POWER_DOWN=0,
    EVENT_TIMEOUT,
    NUM_WINIOTARGTIMER_EVENTS      // number of events in the thread
} WINIOTARGTIMER_EVENT;

//Windows Timer Class
class WINIOTARG_API WinIoTargTimer
{
public:
  typedef void (*PTIMERPROC)(void*);

  //CONSTRUCTOR
  WinIoTargTimer();
  //DESTRUCTOR
  virtual ~WinIoTargTimer();
  //Creation Functions
  int Create(HWND const& rhWnd, UINT uiID, UINT uiPeriod);
  TMWTYPES_BOOL Create(PTIMERPROC pTimerProc, LPVOID pArg, UINT uiPeriod);
  //Starting the timer thread
  void StartThread();
  //Starting
  TMWTYPES_BOOL StartPeriodic();
  TMWTYPES_BOOL StartTimeOut();
  //Stopping
  TMWTYPES_BOOL Stop();
  //Delete the Timers of the given Window
  static TMWTYPES_BOOL Delete(HWND const& rhWnd);
  
  //Check if is active
  TMWTYPES_BOOL IsRunning();
  //ID Getter
  UINT GetID();
  //Period Setter
  void SetPeriodMs(UINT uiPeriod);
  void SetPeriodSecs(UINT uiPeriod);
  //Period Getter
  UINT GetPeriod();
  //Thread Function is a friend
  friend UINT TimerThreadProc(LPVOID pParam);
  //Timer Message
  static const UINT UWM_TIMER;

protected:
  // Stop the timer thread
  void StopThread(void);

private:
  //Disallow copy
  WinIoTargTimer(const WinIoTargTimer&); 
  WinIoTargTimer& operator=(const WinIoTargTimer&);
  //Critical Section for protecting the access to the static members
  static AutoCriticalSectionWinTarg sm_CS;
  static bool sm_bIniCS;
  //Data Map
  static std::multimap<HWND, WinIoTargTimer*> sm_oMultiMap;
  //Data Queue
  static std::deque<WinIoTargTimer*> sm_oQueue;
  //Thread Handle
  static HANDLE  sm_hThread;
  //Event Handles
  static HANDLE m_hTimerEvents[NUM_WINIOTARGTIMER_EVENTS];
  static HANDLE sm_hEvent;
  static HANDLE sm_hShutdownEvent;
  //Thread Run Flag
  static bool sm_bThreadRunning;
  WINIOTARG_TIMER_TYPE m_Type;
  WINIOTARG_TIMER_MODE m_Mode;
  //Timer ID
  UINT m_uiID;
  //Timer Period
  UINT m_uiPeriod;
  //Timer Procedure
  //If this Timer Procedure is called from other threads too, the accessed data should
  //be protected using appropriate synchronization objects
  PTIMERPROC m_pTimerProc;
  //The Argument to Timer Procedure
  LPVOID m_pArg;
  //Scheduled Ticks's Tick Count
  DWORD m_uiTickNext;
  //Creation Flag
  bool m_bCreated;
  //Run Flag
  bool m_bRunning;
};

//ID Getter
inline UINT WinIoTargTimer::GetID()
{
  return m_uiID;
}

//Check if is active
inline TMWTYPES_BOOL WinIoTargTimer::IsRunning()
{
  return m_bRunning;
}

//Period Getter
inline UINT WinIoTargTimer::GetPeriod()
{
  return m_uiPeriod;
}

#endif // __WinIoTargTimer_H__
