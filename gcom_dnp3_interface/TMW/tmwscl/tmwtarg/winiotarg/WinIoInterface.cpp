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

/* file: WinIoInterface.cpp
* description: Interface to the target I/O routines. This abstract base class provides
* a generic interface to the various low level target interfaces 
* (see the pure virtual functions below). Currently this includes the Modbus Plus Interface, 
* RS232 interface and the TCP/IP interface.
*
*/

#include "StdAfx.h"
#include "WinIoTarg/WinIoInterface.h"
#include "WinIoTarg/include/WinIoTarg.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

/*****************************************************************************/
extern "C" void WinIoAssertion(const char   *expr, const char   *fileName, int lineNum)
{
  TCHAR   buf[1024]; 
  _stprintf_s
    (
    buf, 1024,
    __T("%hs\nFile: %hs, Line: %d"),
    expr,
    fileName,
    (int) lineNum
    );
  
  ::MessageBox
    (
    NULL,
    buf,
   _T("WinIO Assertion Failed. Breaking!"),
    MB_OK | MB_ICONSTOP
    );

  ::DebugBreak();

  return;
}


void WinIoInterface::ProtoAnaLog(TMWDIAG_ID sourceID, const char   *format, ...)
{
  if (WinIoTarg_IsProtoAnaLogEnabled() == false)
  {
    return;
  }
  va_list va;
  va_start(va, format);
  if (WinIoTargProtoAnaLogFun != WINIOTARG_NULL)
  {
    WinIoTargProtoAnaLogFun(TMWDEFS_NULL, TRACE_MASK_NONE, sourceID, "", format, va);
  }
  va_end(va);
}

/**********************************************************************************\
Function :			WinIoInterface::WinIoInterface
Description : [none]	
Return :			constructor	-	
Parameters :
Note : [none]
\**********************************************************************************/
WinIoInterface::WinIoInterface()
{
  m_chanState = TMWTARG_CHANNEL_INITIALIZED;
  m_chanThreadState = TMWTARG_THREAD_IDLE;
  m_channelThreadHandle = INVALID_HANDLE_VALUE;
  m_channelThreadEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
  m_chanThreadCheckMask = CT_CHECK_NONE;
 
  pWinIoContext = WINIOTARG_NULL;

  m_pStatusCallback = WINIOTARG_NULL;
  m_pStatusCallbackParam = WINIOTARG_NULL;

  m_pRecvDataFunc = WINIOTARG_NULL;
  m_pCheckAddrFunc = WINIOTARG_NULL;
  m_pChanContextCBData = WINIOTARG_NULL; 
  m_pChannel = WINIOTARG_NULL;
  m_forceDisconnected  = false;
  setChanEnabled(true); // initialy on-line redial limit reached will take it off line
  m_selectPendingSocket = INVALID_SOCKET;
}

/**********************************************************************************\
Function :			WinIoInterface::~WinIoInterface
Description : [none]	
Return :			destructor	-	
Parameters :
Note : [none]
\**********************************************************************************/
WinIoInterface::~WinIoInterface()
{
  CloseHandle(m_channelThreadEvent);
}

/**********************************************************************************\
	Function :			WinIoInterface::ChannelThreadStart
	Description : [none]	
	Return :			bool	-	
	Parameters :
			WinIoInterface *pWinIoInterface	-	
	Note : [none]
\**********************************************************************************/
bool WinIoInterface::ChannelThreadStart(void)
{
  m_chanThreadState = TMWTARG_THREAD_RUNNING;

  m_channelThreadHandle = (HANDLE)WinIoTarg_startThread((WINIO_THREAD_FUN)channelThread,this,&m_channelThreadID, THREAD_PRIORITY_BELOW_NORMAL);
  if(m_channelThreadHandle == INVALID_HANDLE_VALUE)
  {
    m_chanThreadState = TMWTARG_THREAD_IDLE;
    return false;
  }
  
  return true;
}

/**********************************************************************************\
	Function :			WinIoInterface::ChannelThreadStop
	Description : [none]	
	Return :			bool	-	
	Parameters : [none]
	Note : [none]
\**********************************************************************************/
bool WinIoInterface::ChannelThreadStop(void)
{
  if (m_chanThreadState == TMWTARG_THREAD_RUNNING)
  {
    {
      CriticalSectionLockWinTarg lock(m_chanThreadStateLock);
      m_chanThreadState = TMWTARG_THREAD_EXITING;
      SetEvent(m_channelThreadEvent);

      if (m_selectPendingSocket != INVALID_SOCKET)
      {
        /* If the channel thread is waiting for a connection, closing the socket
         * will ensure the select fails immediately instead of waiting for a
         * timeout to occur.
         */
        closesocket(m_selectPendingSocket);
      }
    }

    WaitForSingleObject(m_channelThreadHandle, 100);
    while (m_chanThreadState > TMWTARG_THREAD_EXITED)
    {
      Sleep(100);
    }

    CloseHandle(m_channelThreadHandle);
    m_channelThreadHandle = INVALID_HANDLE_VALUE;
    return true;
  }
  return false;
}

/**********************************************************************************\
	Function :			WinIoInterface::channelThread
	Description : Entry point for channel thread
	Return :			unsigned int	-	
	Parameters :
			WinIoInterface *pWinIoInterface	-	
	Note : [none]
\**********************************************************************************/
unsigned int __stdcall WinIoInterface::channelThread(WinIoInterface *pWinIoInterface)
{
  /* Wait for the libray to call WinIoTarg_openChannel to open the channel */
  while ((pWinIoInterface->m_chanThreadState == TMWTARG_THREAD_RUNNING) &&
    (pWinIoInterface->m_chanState == TMWTARG_CHANNEL_INITIALIZED))
  {
    WaitForSingleObject(pWinIoInterface->m_channelThreadEvent, 1000);
  }

  //perform the initial connection delay
  Sleep(pWinIoInterface->pWinIoContext->connectDelay);
  
  while (pWinIoInterface->m_chanThreadState == TMWTARG_THREAD_RUNNING)
  {
    if ((pWinIoInterface->m_chanThreadCheckMask & CT_NOT_ACTIVE) == 0)
    {
      if (pWinIoInterface->isChannelOpen()) //Channel is connected
      {
        //If this channel thread is configured to check for receive data, all the channel's checkReceive function to do so.
        if (pWinIoInterface->m_chanThreadCheckMask & CT_CHECK_INPUT)
        {
          if (pWinIoInterface->checkReceive(500))
          {
            pWinIoInterface->m_pRecvDataFunc(pWinIoInterface->m_pChanContextCBData);
          }
          continue;
        }
      }
      else //Channel is not connected
      {
        if (pWinIoInterface->m_chanThreadCheckMask & CT_CHECK_CONNECT)
        {
          //If this channel thread is configured to check the connection status, call openWinIoChannel to do so.
          if (pWinIoInterface->openWinIoChannel(pWinIoInterface->getRecvCallbackFunc(),
            pWinIoInterface->getCheckAddrCallbackFunc(), pWinIoInterface->getChanContextCBData(), WINIO_OPEN_MODE_NONE))
          {
            continue;
          }
        }
      }
    }
    WaitForSingleObject(pWinIoInterface->m_channelThreadEvent, pWinIoInterface->pWinIoContext->connectRetry);
  }

  pWinIoInterface->m_chanThreadState = TMWTARG_THREAD_EXITED;

  return 0;
} 
