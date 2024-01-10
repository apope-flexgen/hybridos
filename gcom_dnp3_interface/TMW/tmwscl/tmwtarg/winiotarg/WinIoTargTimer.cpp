/*****************************************************************************/
/* Triangle MicroWorks, Inc.                         Copyright (c) 1997-2022 */
/*****************************************************************************/
/*                                                                           */
/* This file is the property of:                                             */
/*                                                                           */
/*                       Triangle MicroWorks, Inc.                           */
/*                      Raleigh, North Carolina USA                          */
/*                       www.TriangleMicroWorks.com                          */
/* (919) 870 - 6615                                   */
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

/* file: WinIoTargTimer.cpp
 * description: Interface to windows timers
 */

#include "StdAfx.h"
#pragma warning(disable: 4100)

#include "WinIoTarg/include/WinIoTarg.h"
#include "WinIoTarg/WinIoInterface.h"
#include "WinIoTarg/WinIoTargTimer.h"
#include "tmwscl/utils/tmwtarg.h"

using namespace std;

#define DEBUG_EVENTS

#define UWM_TIMER_MSG "UWM_TIMER_MSG-{F57C90D5_EC7C_158A_AD22_015F293A5FC0}"

// User Defined Timer Message
const UINT WinIoTargTimer::UWM_TIMER = RegisterWindowMessage(_T(UWM_TIMER_MSG));

// Critical Section for protecting the access to the static members
AutoCriticalSectionWinTarg WinIoTargTimer::sm_CS;
bool WinIoTargTimer::sm_bIniCS = false;

// Data Map
multimap<HWND, WinIoTargTimer*> WinIoTargTimer::sm_oMultiMap;

// Data Queue
deque<WinIoTargTimer*> WinIoTargTimer::sm_oQueue;

// Thread Handle
HANDLE WinIoTargTimer::sm_hThread = NULL;

// Event Handle
HANDLE WinIoTargTimer::sm_hEvent = NULL;
HANDLE WinIoTargTimer::sm_hShutdownEvent = NULL;
HANDLE WinIoTargTimer::m_hTimerEvents[2];

// Thread Run Flag
bool WinIoTargTimer::sm_bThreadRunning = false;

// CONSTRUCTOR
WinIoTargTimer::WinIoTargTimer() : 
  m_bCreated(false), 
  m_bRunning(false), 
  m_pTimerProc(NULL), 
  m_pArg(NULL), 
  m_uiID(0xffffffff), 
  m_Type(TYPE_NONE),
  m_Mode(MODE_NONE)
{
  if (false == sm_bIniCS)
  {
    // First Object
    // The Critical section needs Initialization
    sm_bIniCS = true;
  }
}

// DESTRUCTOR
WinIoTargTimer::~WinIoTargTimer()
{
  if (false == m_bCreated)
    // Nothing to Destroy
    return;

  {
    CriticalSectionLockWinTarg lock(sm_CS);

    //=======================================================================
    WINIO_ASSERT(sm_hThread != NULL);
    if (TYPE_WIN == m_Type)
    {
      multimap<HWND, WinIoTargTimer*>::iterator it = sm_oMultiMap.begin(), itEnd = sm_oMultiMap.end();
      while (it != itEnd)
      {
        if (it->second == this)
        {
          sm_oMultiMap.erase(it);
          break;
        }
        it++;
      }
    }
    else // TYPE_PROC == m_iType
    {
      deque<WinIoTargTimer*>::iterator it1 = sm_oQueue.begin(), itEnd1 = sm_oQueue.end();
      while (it1 != itEnd1)
      {
        if (*it1 == this)
        {
          sm_oQueue.erase(it1);
          break;
        }
        it1++;
      }
    }
  }

  //=======================================================================
  if (true == sm_oMultiMap.empty() && true == sm_oQueue.empty())
  {
    // Last Object
    WinIoTargTimer::StopThread();
    sm_bIniCS = false;
  }
}

// Creation Functions
int WinIoTargTimer::Create(HWND const& rhWnd, UINT uiID, UINT uiPeriod)
{
  if (true == m_bCreated)
    // Already Created
    return m_uiID;
  // Check the Window
  if ((NULL == rhWnd) || (FALSE==IsWindow(rhWnd)))
    // Error
    return -1;

  CriticalSectionLockWinTarg lock(sm_CS);

  m_Type = TYPE_WIN;
  m_uiPeriod = uiPeriod;
  // Check if the ID is already taken
  multimap<HWND, WinIoTargTimer*>::iterator it = sm_oMultiMap.find(rhWnd);
  if (it != sm_oMultiMap.end())
  {
    UINT uiMax = 0;
    bool bFound = false;
    while (it != sm_oMultiMap.upper_bound(rhWnd))
    {
      if (false == bFound)
      {
        if (it->second->m_uiID == uiID)
          bFound = true;
      }
      if (it->second->m_uiID > uiMax)
        uiMax = it->second->m_uiID;
      it++;
    }
    if (true == bFound)
      m_uiID = uiMax + 1;
    else
      m_uiID = uiID;
  }
  else
    m_uiID = uiID;
  // Save in Map
  sm_oMultiMap.insert(pair<HWND, WinIoTargTimer*>(rhWnd, this));
  if (NULL == sm_hThread)
  {
    // First object is creating the Thread in Suspended State
	  StartThread();
  }
  m_bCreated = true;
  return m_uiID;
}

TMWTYPES_BOOL WinIoTargTimer::Create(PTIMERPROC pTimerProc, LPVOID pArg, UINT uiPeriod)
{
  if (true == m_bCreated)
    // Already Created
    return false;
  CriticalSectionLockWinTarg lock(sm_CS);

  m_Type = TYPE_PROC;
  m_uiPeriod = uiPeriod;
  m_pTimerProc = pTimerProc;
  m_pArg = pArg;
  // Save in Queue
  sm_oQueue.push_back(this);
  if (NULL == sm_hThread)
  {
	  StartThread();
    // First object is creating the Thread in Suspended State
  }
  m_bCreated = true;
  return true;
}

void WinIoTargTimer::StopThread()
{
  SetEvent(WinIoTargTimer::sm_hShutdownEvent);
  DWORD status = WaitForSingleObject(WinIoTargTimer::sm_hThread,2000);
  if (status == WAIT_TIMEOUT)
  {
    TerminateThread(WinIoTargTimer::sm_hThread,0);
  }

  sm_hThread = NULL;
}

void WinIoTargTimer::StartThread()
{
	DWORD dwThreadID;
	sm_hThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL, (DWORD)0,
	  (LPTHREAD_START_ROUTINE)TimerThreadProc, this, (DWORD)CREATE_SUSPENDED, (LPDWORD)&dwThreadID);
	WINIO_ASSERT(sm_hThread != NULL);
	
  // Create the Event (the system closes the handle automatically when the process terminates)
	sm_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	WINIO_ASSERT(sm_hEvent != NULL);
	sm_hShutdownEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	WINIO_ASSERT(sm_hShutdownEvent != NULL);

  m_hTimerEvents[EVENT_TIMEOUT]    = sm_hEvent;
  m_hTimerEvents[EVENT_POWER_DOWN] = sm_hShutdownEvent;

	// Initialized to Above Normal Priority to ensure higher priority than the User Interface threads
	SetThreadPriority(sm_hThread, THREAD_PRIORITY_ABOVE_NORMAL);
	// Start the Thread
	ResumeThread(sm_hThread);
}

// Starting
TMWTYPES_BOOL WinIoTargTimer::StartPeriodic()
{
  if (m_bCreated == false)
    // Not Created
    return false;

  CriticalSectionLockWinTarg lock(sm_CS);
  if (m_bRunning == false)
  {
    m_Mode = MODE_PERIODIC;
    // Next Scheduled Tick
    m_bRunning = true;
    m_uiTickNext = GetTickCount() + m_uiPeriod;
    // First Tick
    if (TYPE_WIN == m_Type)
    {
      multimap<HWND, WinIoTargTimer*>::iterator it = sm_oMultiMap.begin(), itEnd = sm_oMultiMap.end();
      while (it != itEnd)
      {
        if (it->second == this)
        {
          PostMessage(it->first, UWM_TIMER, m_uiID, GetTickCount());
          break;
        }
        it++;
      }
    }
    else // TYPE_PROC == m_iType
      m_pTimerProc(m_pArg);
    if (sm_bThreadRunning == false)
    {
      // Resume the thread
      sm_bThreadRunning = true;
      SetEvent(sm_hEvent);
      return true;
    }
  }
  return true;
}

// Starting
TMWTYPES_BOOL WinIoTargTimer::StartTimeOut()
{
  if (m_bCreated == false)
    // Not Created
    return false;

  CriticalSectionLockWinTarg lock(sm_CS);
  if (m_bRunning == false)
  {
    m_Mode = MODE_TIMEOUT;
    // Next Scheduled Tick
    m_bRunning = true;
    m_uiTickNext = GetTickCount() + m_uiPeriod;
    if (sm_bThreadRunning == false)
    {
      // Resume the thread
      sm_bThreadRunning = true;
      SetEvent(sm_hEvent);
      return true;
    }
  }
  return true;
}

// Stopping
TMWTYPES_BOOL WinIoTargTimer::Stop()
{
  if (false == m_bCreated)
    // Not Created
    return false;
  //CriticalSectionLock lock(sm_CS);
  //=======================================================================
  m_bRunning = false;
  //=======================================================================
  return true;
}

// Period Setter
void WinIoTargTimer::SetPeriodMs(UINT uiPeriod)
{
  m_uiPeriod = uiPeriod;
  CriticalSectionLockWinTarg lock(sm_CS);
  //=======================================================================
  UINT uiTickNext = GetTickCount() + m_uiPeriod;
  if (uiTickNext < m_uiTickNext)
    m_uiTickNext = uiTickNext;
  //=======================================================================
}

void WinIoTargTimer::SetPeriodSecs(UINT uiPeriod)
{
  m_uiPeriod = uiPeriod*1000;
  CriticalSectionLockWinTarg lock(sm_CS);
  //=======================================================================
  UINT uiTickNext = GetTickCount() + m_uiPeriod;
  if (uiTickNext < m_uiTickNext)
    m_uiTickNext = uiTickNext;
  //=======================================================================
}

// Delete the Timers of the given Window
TMWTYPES_BOOL WinIoTargTimer::Delete(HWND const& rhWnd)
{
  // Check if the Window is valid
  if ((rhWnd != NULL) && IsWindow(rhWnd))
  {
    multimap<HWND, WinIoTargTimer*>::iterator it, itEnd;
    CriticalSectionLockWinTarg lock(sm_CS);
    //=======================================================================
    it = WinIoTargTimer::sm_oMultiMap.find(rhWnd);
    if (it != sm_oMultiMap.end())
    {
      itEnd = WinIoTargTimer::sm_oMultiMap.upper_bound(it->first);
      while (it != itEnd)
        it = WinIoTargTimer::sm_oMultiMap.erase(it);
      // OK, deleted
      return true;
    }
    //=======================================================================
    // Cannot Find Window
    return false;
  }
  // Invalid Window
  return false;
}

// Thread Function
// The Timer's resolution is imposed to about 10 ms. The Timer's Period should be > 20ms, but for
// good results is recommended >= 100 ms.

#define MAX_TICK  200
#define MIN_TICK  20
#define MIN_TICK2 10

UINT TimerThreadProc(LPVOID pParam)
{
  //HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
  //if (hr != S_OK)
  //{
  //  WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TIMER: CoInitialize failed error: 0x%08x \n", hr);
  //}

  DWORD dwTick;
  TMWTYPES_BOOL bSuspend;
  WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET, "TIMER: Thread: %x started\n",GetCurrentThreadId());
  multimap<HWND, WinIoTargTimer*>::iterator it, itEnd, _itEnd;
  deque<WinIoTargTimer*>::iterator it1, itEnd1;
  for (;;)
  {
    CriticalSectionLockWinTarg lock(WinIoTargTimer::sm_CS);
    //=======================================================================
    it = WinIoTargTimer::sm_oMultiMap.begin();
    itEnd = WinIoTargTimer::sm_oMultiMap.end();
    while (it != itEnd)
    {
      if (true == it->second->m_bRunning)
      {
        dwTick = GetTickCount();
        if (dwTick + MIN_TICK2 >= it->second->m_uiTickNext)
        {
          // Check if the Window is still valid
          if (IsWindow(it->first))
          {
            if (it->second->m_Mode == MODE_PERIODIC)
            {
              // Reschedule Tick
              it->second->m_uiTickNext += it->second->m_uiPeriod;
              // Current Tick
              PostMessage(it->first, WinIoTargTimer::UWM_TIMER, it->second->m_uiID, dwTick);
            }
            else if (it->second->m_Mode == MODE_TIMEOUT)
            {
              // Current Tick
              PostMessage(it->first, WinIoTargTimer::UWM_TIMER, it->second->m_uiID, dwTick);
              it->second->m_bRunning = false;
            }
          }
          else
          {
            // Destroy everything related to that Window from the Map
            _itEnd = WinIoTargTimer::sm_oMultiMap.upper_bound(it->first);
            while (it != _itEnd)
              it = WinIoTargTimer::sm_oMultiMap.erase(it);
            continue;
          }
        }
      }
      it++;
    }
    //
    it1 = WinIoTargTimer::sm_oQueue.begin();
    itEnd1 = WinIoTargTimer::sm_oQueue.end();
    while (it1 != itEnd1)
    {
      if ((*it1)->m_bRunning == true)
      {
        dwTick = GetTickCount();
        if (dwTick + MIN_TICK2 >=(*it1)->m_uiTickNext)
        {
          if ((*it1)->m_Mode == MODE_PERIODIC)
          {
            // Reschedule Tick
            (*it1)->m_uiTickNext +=(*it1)->m_uiPeriod;
            // Current Tick - call the Timer Procedure
            (*it1)->m_pTimerProc((*it1)->m_pArg);
          }
          else if ((*it1)->m_Mode == MODE_TIMEOUT)
          {
            // Current Tick - call the Timer Procedure
            (*it1)->m_bRunning = false;
            (*it1)->m_pTimerProc((*it1)->m_pArg);
          }
        }
      }
      it1++;
    }
    //
    bSuspend = true;
    // Schedule the Next Global Tick
    dwTick = 0xffffffff;
    if (WinIoTargTimer::sm_oMultiMap.size() > 0)
    {	
      for (it = WinIoTargTimer::sm_oMultiMap.begin(); it != WinIoTargTimer::sm_oMultiMap.end(); it++)
      {
        if (true == it->second->m_bRunning)
        {
          bSuspend = false;
          if (it->second->m_uiTickNext < dwTick)
            dwTick = it->second->m_uiTickNext;
        }
      }
    }
    //
    if (WinIoTargTimer::sm_oQueue.size() > 0)
    {	
      for (it1 = WinIoTargTimer::sm_oQueue.begin(); it1 != WinIoTargTimer::sm_oQueue.end(); it1++)
      {
        if (true ==(*it1)->m_bRunning)
        {
          bSuspend = false;
          if ((*it1)->m_uiTickNext < dwTick)
            dwTick = (*it1)->m_uiTickNext;
        }
      }
    }
    if (false == bSuspend)
    {
      if (dwTick > GetTickCount())
      {
        dwTick -= GetTickCount();
        if (dwTick < MIN_TICK)
          dwTick = MIN_TICK;
      }
      else
        dwTick = MIN_TICK;
    }
    else // true == bSuspend
    {
      DWORD          status;
      TMWTYPES_ULONG eventIndex;
      // Suspend the Thread (wait for events)
      WinIoTargTimer::sm_bThreadRunning = false;
      UnGuardWinTarg unlock(lock);
      status = WaitForMultipleObjects(NUM_WINIOTARGTIMER_EVENTS,WinIoTargTimer::m_hTimerEvents, FALSE, INFINITE);
      eventIndex = status - WAIT_OBJECT_0;
      switch (eventIndex)
      {
        case EVENT_TIMEOUT:
          continue;
          break;
        case EVENT_POWER_DOWN:
          // receiving a shutdown down signal. Exit the thread
          WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET, "TIMER:  Thread: %x stopped\n", GetCurrentThreadId());
          //CoUninitialize();
          return 0;
          break;
        default:
          break;
      }
      continue;
    }
    //=======================================================================
    // Check at most every MAX_TICK ms because some Timer Periods could be changed
    if (dwTick > MAX_TICK)
      dwTick = MAX_TICK;
    // Sleep until the Next Event
    Sleep(dwTick);
  }
}

#if 0
/*
For use in a Win32 Console Application:

- include WinIoTargTimer.h, WinIoTargTimer.cpp in a new Win32 console project;

- in stdafx.h be sure to include the headers:
*/

#include <windows.h>
#include <limits.h>
#include <crtdbg.h>

/*
   The following is a simple example of Win32 Console 
   Application using WinIoTargTimer:
*/

#include "StdAfx.h"
#include "WinIoTargTimer.h"
#include <conio.h>
#include <iostream>

using namespace std;

static DWORD g_dwStart;

void Func1(void*)
{
  cout << "Func1 " << GetTickCount() - g_dwStart << endl;
}

void Func2(void*)
{
  cout << "Func2 " << GetTickCount() - g_dwStart << endl;
}

int main(int argc, char* argv[])
{
  WinIoTargTimer oWinIoTargTimer1, oWinIoTargTimer2;
  oWinIoTargTimer1.Create(Func1, NULL, 200);
  oWinIoTargTimer2.Create(Func2, NULL, 400);
  g_dwStart = GetTickCount();
  oWinIoTargTimer1.Start();
  oWinIoTargTimer2.Start();
  while( !_kbhit() )
    ;
  _getch();
  oWinIoTargTimer1.Stop();
  oWinIoTargTimer2.Stop();
  return 0;
}

#endif