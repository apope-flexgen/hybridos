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
/* file: Win232Channel.cpp
* description: Implementation of Windows serial I/O Target interface. 
*/
#include "StdAfx.h"
#pragma warning(disable: 4100)

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif


#include "WinIoTarg/include/WinIoTarg.h"
#include "WinIoTarg/include/WinIoTargDefs.h"
#include "WinIoTarg/WinIoInterface.h"
#include "WinIoTarg/Win232Channel.h"


// Constructor
Win232Channel::Win232Channel(const TMWTARG232_CONFIG &config, TMWTARG_CONFIG *pTmwTargConfig)
{
  m_comHandle = INVALID_HANDLE_VALUE;
  m_dwStoredFlags = EEventNone;

  m_readerStopEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
  m_readerThreadHandle = INVALID_HANDLE_VALUE;
  m_readerThreadIsRunning = false;
  m_iReadBufLen = 0;

  m_writerStopEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
  m_writerDoWriteEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
  m_writerThreadHandle = INVALID_HANDLE_VALUE;
  m_writerThreadIsRunning = false;
  m_iWriteBufLen = 0;

  m_charTransmitTime = 0;

  // Copy configuration data structure
  m_config = config;

  m_iNumCharTimesBetweenFrames = pTmwTargConfig->numCharTimesBetweenFrames;
  m_interCharTimeout  = pTmwTargConfig->interCharTimeout;

  strncpy(m_config.portName, config.portName, WINIOTARG_STR_LEN);
  strncpy(m_config.baudRate, config.baudRate, WINIOTARG_STR_LEN);

  strncpy(m_config.chnlName, config.chnlName, WINIOTARG_STR_LEN);

  // Set channel name
  m_name = (m_config.chnlName) ? m_config.chnlName : m_config.portName;

}

// Destructor
Win232Channel::~Win232Channel(void)
{ 
  // Close channel if not already closed
  close();

  CloseHandle(m_readerStopEvent);
  CloseHandle(m_writerStopEvent);
  CloseHandle(m_writerDoWriteEvent);
  CloseHandle(m_readerThreadHandle);
  CloseHandle(m_writerThreadHandle);
}

// Return channel name
const char   *Win232Channel::getName(void)
{
  return(m_name);
}

// Return channel info
const char   *Win232Channel::getInfo(void)
{
  sprintf(m_info,"baud %s, data bits %d, stop bits %d",
    m_config.baudRate, m_config.numDataBits, m_config.numStopBits);

  switch(m_config.parity)
  {
  case WIN232_PARITY_NONE:
    sprintf(m_info + strlen(m_info),", parity none");
    break;

  case WIN232_PARITY_EVEN:
    sprintf(m_info + strlen(m_info),", parity even");
    break;

  case WIN232_PARITY_ODD:
    sprintf(m_info + strlen(m_info),", parity odd");
    break;
  }

  return(m_info);
}


/**********************************************************************************\
Function :			Win232Channel::StopWriterThread
Description : [none]	
Return :			void	-	
Parameters :
Note : [none]
\**********************************************************************************/
void Win232Channel::StopWriterThread()
{
  if (m_writerThreadIsRunning == true)
  { 
    SetEvent(m_writerStopEvent);
    if (WaitForSingleObject(m_writerThreadHandle,5000) == WAIT_TIMEOUT)
    {
      // force it to shutdown (should not happen)
      WINIO_ASSERT(FALSE);
      WinIoTarg_endThread(m_writerThreadHandle);
    }
  }
}
/**********************************************************************************\
Function :			Win232Channel::StartWriterThread
Description : [none]	
Return :			bool	-	
Parameters :
void	-	
Note : [none]
\**********************************************************************************/
bool Win232Channel::StartWriterThread(void)
{
  // Start thread writing on RS232 port

  if (m_writerThreadIsRunning == false)
  {
    if (m_comHandle != INVALID_HANDLE_VALUE)
    {
      m_writerThreadHandle = (HANDLE)WinIoTarg_startThread(writerThread,this,&m_writerThreadID,THREAD_PRIORITY_ABOVE_NORMAL);
      // give up reset of time slice to allow writer thread to run
      Sleep(0);
    }
    else
    {
      return false;
    }
  }
  return true;
}

/**********************************************************************************\
Function :			Win232Channel::StopReaderThread
Description : [none]	
Return :			void	-	
Parameters :
Note : [none]
\**********************************************************************************/
void Win232Channel::StopReaderThread()
{
  if (m_readerThreadIsRunning == true)
  {  
    SetEvent(m_readerStopEvent);
    if (WaitForSingleObject(m_readerThreadHandle,5000) == WAIT_TIMEOUT)
    {
      // force it to shutdown (should not happen)
      WINIO_ASSERT(FALSE);
      WinIoTarg_endThread(m_readerThreadHandle);
    }
  }
}
/**********************************************************************************\
Function :			Win232Channel::StartReaderThread
Description : [none]	
Return :			bool	-	
Parameters :
void	-	
Note : [none]
\**********************************************************************************/
bool Win232Channel::StartReaderThread(void)
{
  // Start thread reading on RS232 port
  if (m_readerThreadIsRunning == false)
  {
    if (m_comHandle != INVALID_HANDLE_VALUE)
    {
      m_readerThreadHandle = (HANDLE)WinIoTarg_startThread(readerThread,this,&m_readerThreadID,THREAD_PRIORITY_ABOVE_NORMAL);
      // give up reset of time slice to allow reder thread to run
      Sleep(0);
    }
    else
    {
      return false;
    }
  }
  return true;
}

/*-----------------------------------------------------------------------------

FUNCTION: StoreInReadBuffer(char *, DWORD)

PURPOSE: Send a rec'd buffer to the approprate location

PARAMETERS:
lpBuf    - address of data buffer
dwBufLen - size of data buffer

COMMENTS: If buffer is 0 length, then do nothing.

-----------------------------------------------------------------------------*/
void Win232Channel::StoreInReadBuffer(TMWTYPES_UCHAR *lpBuf, DWORD dwBufLen)
{
  if (dwBufLen == 0) 
  {
    return;
  }

  CriticalSectionLockWinTarg lock(getReadLock());
  if ((m_iReadBufLen + dwBufLen) < MAX_READ_BUFFER)
  {
    memcpy(ReadBuffer+m_iReadBufLen, lpBuf, dwBufLen);
    m_iReadBufLen += dwBufLen;
    if (!m_config.polledMode)
    {
      m_pRecvDataFunc(m_pChanContextCBData);
    }
  }
  else
  {
    // buffer overflow
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "Read Buffer overflow in StoreInReadBuffer");
  }
  return;
}
/*-----------------------------------------------------------------------------

FUNCTION: StoreInWriteBuffer(char *, DWORD)

PURPOSE: Place a buffer in the transmit buffer

PARAMETERS:
lpBuf    - address of data buffer
dwBufLen - size of data buffer

COMMENTS: If buffer is 0 length, then do nothing.

-----------------------------------------------------------------------------*/
DWORD Win232Channel::StoreInWriteBuffer(const TMWTYPES_UCHAR *lpBuf, DWORD dwBufLen)
{
  if (dwBufLen == 0) 
  {
    return 0;
  }

  CriticalSectionLockWinTarg lock(getWriteLock());
  if ((m_iWriteBufLen + dwBufLen) < MAX_WRITE_BUFFER)
  {
    memcpy(WriteBuffer+m_iWriteBufLen, lpBuf, dwBufLen);
    m_iWriteBufLen += dwBufLen;
  }
  else
  {
    // buffer overflow
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "Write Buffer overflow in StoreInWriteBuffer");
    return 0;
  }
  return dwBufLen;
}

/**********************************************************************************\
Function :			Win232Channel::writerThread
Description : [none]	
Return :			unsigned int __stdcall	-	
Parameters :
void *pParam	-	
Note : [none]
\**********************************************************************************/
unsigned int __stdcall Win232Channel::writerThread(void *pParam)
{
  Win232Channel *pWin232Channel = (Win232Channel *)pParam;

  {
    CriticalSectionLockWinTarg lock(pWin232Channel->getWriteLock());
    pWin232Channel->m_writerThreadIsRunning = true;
  }

  OVERLAPPED osWriter = { 0 };  // overlapped structure for write operations
  HANDLE     hArray[NUM_WRITESTAT_HANDLES];
  HANDLE     h2Array[NUM_WRITESTAT_HANDLES];
  DWORD 	   dwWrite;          // bytes actually written
  DWORD      dwRes;           // result from WaitForMultipleObjects
  bool       fWaitingOnWrite = false;
  bool       fThreadDone = false;
  TMWTYPES_UCHAR     lpBuf[AMOUNT_TO_WRITE];
  char   errorBuffer[128];

  //
  // create two overlapped structures, one for read events
  // and another for status events
  //
  osWriter.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (osWriter.hEvent == NULL)
  {
    pWin232Channel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "CreateEvent failed (Writer Event)");
  }

  //
  // We want to detect the following events:
  //   Read events (from WriteFile)
  //   Thread exit evetns (from our shutdown functions)
  //
  hArray[0] = pWin232Channel->getWriterStopEvent();
  hArray[1] = osWriter.hEvent;

  h2Array[0] = pWin232Channel->getWriterStopEvent();
  h2Array[1] = pWin232Channel->m_writerDoWriteEvent;

  DWORD timeout = INFINITE;
  {
    CriticalSectionLockWinTarg lock(pWin232Channel->getWriteLock());
    float ratio = 2600.0F / (float)atoi(pWin232Channel->m_config.baudRate);
    timeout = (DWORD)(ratio * 1500.0);
  }

  while (!fThreadDone)
  {
    // if no write is outstanding, then issue another one
    if (!fWaitingOnWrite)
    {
      // copy from big buffer to xmit buffer
      DWORD maxBytes = 0;
      {
        CriticalSectionLockWinTarg lock(pWin232Channel->getWriteLock());
        maxBytes = min(AMOUNT_TO_WRITE, pWin232Channel->m_iWriteBufLen);
        memcpy(lpBuf, pWin232Channel->WriteBuffer, maxBytes);
        pWin232Channel->m_iWriteBufLen -= maxBytes;
        memmove(pWin232Channel->WriteBuffer, pWin232Channel->WriteBuffer + maxBytes, pWin232Channel->m_iWriteBufLen);
      }

      if (maxBytes != 0)
      {
        pWin232Channel->LogMessage(TMWDIAG_ID_TARGET, "Issue overlapped WriteFile");
        if (!WriteFile(pWin232Channel->m_comHandle, lpBuf, (DWORD)maxBytes, &dwWrite, &osWriter))
        {
          DWORD writeResult = GetLastError();
          if (writeResult == ERROR_IO_PENDING)
          {
            pWin232Channel->LogMessage(TMWDIAG_ID_TARGET, "WriteFile pending");
            fWaitingOnWrite = true;

            // Allow time for message to be transmitted. Waiting an extra 30 seems to help
            if (pWin232Channel->m_config.bModbusRTU)
              Sleep(pWin232Channel->m_charTransmitTime * (maxBytes + pWin232Channel->m_iNumCharTimesBetweenFrames) + 30);
          }
          else
          {
            WinIoTarg_formatErrorMsg(errorBuffer, writeResult);
            pWin232Channel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "WriteFile failed\n%19s%s", " ", errorBuffer);
          }
        }
        else
        {    // write completed immediately
          if (dwWrite)
          {
            pWin232Channel->LogMessage(TMWDIAG_ID_TARGET, "Write complete immediately wrote %d bytes", dwWrite);
          }
        }
      }
      else
      {
        DWORD dwRes2 = WaitForMultipleObjects(NUM_WRITESTAT_HANDLES, h2Array, FALSE, INFINITE);
        switch (dwRes2)
        {
        case WAIT_OBJECT_0:
          pWin232Channel->LogMessage(TMWDIAG_ID_TARGET, "STOP received exit writer thread event");
          fThreadDone = true;
          break;
        case WAIT_OBJECT_0 + 1:
          pWin232Channel->LogMessage(TMWDIAG_ID_TARGET, "Do Write event received");
          break;
        default:
        {
          WinIoTarg_formatErrorMsg(errorBuffer, GetLastError());
          pWin232Channel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "WaitForMultipleObjects wait failed\n%19s%s", " ", errorBuffer);
        }
        break;
        }
      }
    }
    else
    {
      // try again in 50 ms
      Sleep(50);
    }

    // wait for pending operations to complete
    if (fWaitingOnWrite)
    {
      dwRes = WaitForMultipleObjects(NUM_WRITESTAT_HANDLES, hArray, FALSE, timeout);
      switch (dwRes)
      {
        // thread exit event
      case WAIT_OBJECT_0:
        pWin232Channel->LogMessage(TMWDIAG_ID_TARGET, "exit writer thread event");
        fThreadDone = true;
        break;

        // Write completed
      case WAIT_OBJECT_0 + 1:
        if (GetOverlappedResult(pWin232Channel->m_comHandle, &osWriter, &dwWrite, FALSE) == 0)
        {
          HRESULT err = GetLastError();
          if (err == ERROR_OPERATION_ABORTED)
            pWin232Channel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "write aborted");
          else
          {
            WinIoTarg_formatErrorMsg(errorBuffer, err);
            pWin232Channel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "Error Write GetOverlappedResult\n%19s%s", " ", errorBuffer);
          }
        }
        else
        {
          // write completed successfully
          pWin232Channel->LogMessage(TMWDIAG_ID_TARGET, "write completed successfully wrote %d bytes", dwWrite);
        }

        fWaitingOnWrite = false;
        break;

      case WAIT_TIMEOUT:
        // This means WaitForMultipleObjects timed out not the write itself 
        pWin232Channel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "write timeout");
        break;

      default:
        pWin232Channel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "WaitForMultipleObjects(Writer & Status handles");
        break;
      }
    }
  }

  // close event handles
  if (osWriter.hEvent != NULL)
    CloseHandle(osWriter.hEvent);

  {
    CriticalSectionLockWinTarg lock(pWin232Channel->getWriteLock());
    pWin232Channel->m_writerThreadIsRunning = false;
  }

  return 1;
}

/**********************************************************************************\
Function :			Win232Channel::readerThread
Description : [none]	
Return :			unsigned int __stdcall	-	
Parameters :
void *pParam	-	
Note : [none]
\**********************************************************************************/
unsigned int __stdcall Win232Channel::readerThread(void *pParam)
{
  Win232Channel *pWin232Channel = (Win232Channel *)pParam;

  {
    CriticalSectionLockWinTarg lock(pWin232Channel->getReadLock());
    pWin232Channel->m_readerThreadIsRunning = true;
  }

  OVERLAPPED osReader = {0};  // overlapped structure for read operations
  OVERLAPPED osStatus = {0};  // overlapped structure for status operations
  HANDLE     hReadArray[NUM_READSTAT_HANDLES];
  HANDLE     hCommEventArray[NUM_READSTAT_HANDLES];
  DWORD 	   dwRead;          // bytes actually read
  DWORD      dwRes;           // result from WaitForMultipleObjects
  bool       fWaitingOnRead = false;
  bool       fThreadDone = false;
  bool bWaitOK = true;
  TMWTYPES_UCHAR     lpBuf[AMOUNT_TO_READ];
  char   errorBuffer[128];

  //
  // create two overlapped structures, one for read events
  // and another for status events
  //
  osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (osReader.hEvent == NULL)
  {
    pWin232Channel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "CreateEvent failed (Reader Event)");
  }

  osStatus.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (osStatus.hEvent == NULL)
  {
    pWin232Channel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "CreateEvent failed (Status Event)");
  }

  //
  // We want to detect the following events:
  //   Read events (from ReadFile)
  //   Thread exit evetns (from our shutdown functions)
  //
  hReadArray[0] = pWin232Channel->getReaderStopEvent();
  hReadArray[1] = osReader.hEvent;

  hCommEventArray[0] = pWin232Channel->getReaderStopEvent();
  hCommEventArray[1] = osStatus.hEvent;

  while (!fThreadDone) 
  {
    // if no read is outstanding, then issue another one
    if (!fWaitingOnRead)
    {
      if (bWaitOK == true)
      {
        DWORD waitRes = WaitCommEvent(pWin232Channel->m_comHandle,LPDWORD(&pWin232Channel->m_dwStoredFlags),&osStatus);
        if (waitRes == 0)
        {
          DWORD err = GetLastError();
          if (err == ERROR_IO_PENDING)
          {
            pWin232Channel->LogMessage(TMWDIAG_ID_TARGET, "WaitCommEvent pending");
          }
          else
          {
            WinIoTarg_formatErrorMsg(errorBuffer, err);
            pWin232Channel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "WaitCommEvent Failed\n%19s%s", " ",errorBuffer);
          }
        }
      }

      dwRes = WaitForMultipleObjects(NUM_READSTAT_HANDLES, hCommEventArray, FALSE, INFINITE);
      switch (dwRes)
      {
        // thread exit event
      case WAIT_OBJECT_0: 
        pWin232Channel->LogMessage(TMWDIAG_ID_TARGET, "exit reader thread event");
        fThreadDone = true;
        break;

      case WAIT_OBJECT_0 + 1:
        pWin232Channel->LogMessage(TMWDIAG_ID_TARGET,  "WaitCommEvent done flags=0x%08x",pWin232Channel->m_dwStoredFlags);
        // Handle break event
        if (pWin232Channel->m_dwStoredFlags & Win232Channel::EEventBreak)
        {
          pWin232Channel->LogMessage(TMWDIAG_ID_TARGET , "BREAK received");
        }

        // Handle CTS event
        if (pWin232Channel->m_dwStoredFlags & Win232Channel::EEventCTS)
        {
          pWin232Channel->LogMessage(TMWDIAG_ID_TARGET , "Clear to send %s", pWin232Channel->GetCTS()?"on":"off");
        }

        // Handle DSR event
        if (pWin232Channel->m_dwStoredFlags & Win232Channel::EEventDSR)
        {
          pWin232Channel->LogMessage(TMWDIAG_ID_TARGET , "Data set ready %s", pWin232Channel->GetDSR()?"on":"off");
        }

        // Handle error event
        if (pWin232Channel->m_dwStoredFlags & Win232Channel::EEventError)
        {
          char errString[512];
          int curPos = 0;
          DWORD error = pWin232Channel->GetError();
          curPos = sprintf(errString+curPos, "ERROR: ");
          switch (error)
          {
          case Win232Channel::EErrorBreak:		
            curPos = sprintf(errString+curPos, "Break condition");			
            break;
          case Win232Channel::EErrorFrame:		
            curPos = sprintf(errString+curPos, "Framing error");			
            break;
          case Win232Channel::EErrorIOE:		
            curPos = sprintf(errString+curPos, "IO device error");			
            break;
          case Win232Channel::EErrorMode:		
            curPos = sprintf(errString+curPos, "Unsupported mode");			
            break;
          case Win232Channel::EErrorOverrun:	
            curPos = sprintf(errString+curPos, "Buffer overrun");			
            break;
          case Win232Channel::EErrorRxOver:		
            curPos = sprintf(errString+curPos, "Input buffer overflow");	
            break;
          case Win232Channel::EErrorParity:		
            curPos = sprintf(errString+curPos, "Input parity error");		
            break;
          case Win232Channel::EErrorTxFull:		
            curPos = sprintf(errString+curPos, "Output buffer full");		
            break;
          default:						
            curPos = sprintf(errString+curPos,  "Unknown");					
            break;
          }
          pWin232Channel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR,  errString);
        }

        // Handle ring event
        if (pWin232Channel->m_dwStoredFlags & Win232Channel::EEventRing)
        {
          pWin232Channel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR,  "RING");
        }

        // Handle RLSD/CD event
        if (pWin232Channel->m_dwStoredFlags & Win232Channel::EEventRLSD)
        {
          pWin232Channel->LogMessage(TMWDIAG_ID_TARGET, "RLSD/CD %s\r\n", pWin232Channel->GetRLSD()?"on":"off");
        }

        // Handle data receive event
        if (pWin232Channel->m_dwStoredFlags & Win232Channel::EEventRecv)
        {
          BOOL retStat;
          pWin232Channel->LogMessage(TMWDIAG_ID_TARGET, "Issue overlapped ReadFile");

          // loop to make sure to get all data that has been received.
          retStat = ReadFile(pWin232Channel->m_comHandle, lpBuf, AMOUNT_TO_READ, &dwRead, &osReader);
          while (retStat)
          {    
            // read completed immediately
            if (dwRead)
            {
              pWin232Channel->LogMessage(TMWDIAG_ID_TARGET, "Read complete immediately read %d bytes",dwRead);
              pWin232Channel->StoreInReadBuffer(lpBuf, dwRead);
            }
            retStat = ReadFile(pWin232Channel->m_comHandle, lpBuf, AMOUNT_TO_READ, &dwRead, &osReader); 
            if (dwRead == 0)
              break;
          }
          if(retStat == 0)
          {
            DWORD readResult = GetLastError();
            if (readResult == ERROR_IO_PENDING)
            {
              pWin232Channel->LogMessage(TMWDIAG_ID_TARGET, "ReadFile pending");
              fWaitingOnRead = true;
            }
            else
            {
              WinIoTarg_formatErrorMsg(errorBuffer, readResult);
              pWin232Channel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "ReadFile failed\n%19s%s", " ",errorBuffer);
            }
          } 
        }
        bWaitOK = true;
        break;

      case WAIT_TIMEOUT:
        pWin232Channel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "WaitCommEvent timeout");
        bWaitOK = false;
        continue;
        break;

      default:
        WinIoTarg_formatErrorMsg(errorBuffer, GetLastError()); 
        pWin232Channel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "WaitCommEvent failed\n%19s%s", " ",errorBuffer);
        bWaitOK = false;
        continue;
        break;
      }
    }

    // wait for pending operations to complete
    if (fWaitingOnRead)
    {
      dwRes = WaitForMultipleObjects(NUM_READSTAT_HANDLES, hReadArray, FALSE, INFINITE);
      switch (dwRes)
      {
        // thread exit event
      case WAIT_OBJECT_0: 
        pWin232Channel->LogMessage(TMWDIAG_ID_TARGET,  "exit thread event");
        fThreadDone = true;
        break;

        // read completed
      case WAIT_OBJECT_0 + 1:
        if (GetOverlappedResult(pWin232Channel->m_comHandle, &osReader, &dwRead, FALSE) == 0)
        {
          HRESULT err = GetLastError();
          if (err == ERROR_OPERATION_ABORTED)
            pWin232Channel->LogMessage(TMWDIAG_ID_TARGET, "pending read aborted");
          else
          {
            WinIoTarg_formatErrorMsg(errorBuffer, err);  
            pWin232Channel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "pending read GetOverlappedResult\n%19s%s", " ",errorBuffer);
          }
        }
        else 
        {     
          // read completed successfully
          pWin232Channel->LogMessage(TMWDIAG_ID_TARGET, "pending read completed read %d bytes", dwRead);
          pWin232Channel->StoreInReadBuffer(lpBuf, dwRead);
        }

        fWaitingOnRead = false;
        break;

      case WAIT_TIMEOUT:
        // This means WaitForMultipleObjects timed out not the read itself 
        pWin232Channel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "pending read timeout");
        break;                       

      default:
        pWin232Channel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "WaitForMultipleObjects(Reader & Status handles)");
        break;
      }
    }
  }

  // Cancel any pending IO to prevent a failure after exiting this thread.
  CancelIo(pWin232Channel->m_comHandle);

  // close event handles
  CloseHandle(osReader.hEvent);
  CloseHandle(osStatus.hEvent);

  {
    CriticalSectionLockWinTarg lock(pWin232Channel->getReadLock());
    pWin232Channel->m_readerThreadIsRunning = false;
  }

  return 1;
}

/**********************************************************************************\
	Function :			Win232Channel::setChannelCallback
	Description : [none]	
	Return :			void	-	
	Parameters :
			  TMWTARG_CHANNEL_CALLBACK_FUNC pChannelCallback	-	
			  void *pCallbackParam	-	
	Note : [none]
\**********************************************************************************/
void Win232Channel::setChannelCallback(
  TMWTARG_CHANNEL_CALLBACK_FUNC pChannelCallback, 
  void *pCallbackParam)
{
  m_pChannelCallback = pChannelCallback;
  m_pCallbackParam = pCallbackParam;
}

// Open channel
/**********************************************************************************\
Function :			Win232Channel::open
Description : [none]	
Return :			bool	-	
Parameters :
Note : [none]
\**********************************************************************************/
bool Win232Channel::open()
{ 
  if (m_comHandle == INVALID_HANDLE_VALUE)
  {
    const TCHAR   *prefix = _T("\\\\.\\");
    TCHAR  deviceName[256];
    char   buffer[128];
    
    USES_CONVERSION;

    if (_tcsnccmp(A2T(m_config.portName), prefix, _tcslen(prefix)) != 0)
    {
      _tcscpy(deviceName, prefix);
      _tcscat(deviceName, A2T(m_config.portName));
    }
    else
    {
      _tcscpy(deviceName, A2T(m_config.portName));
    }

    m_comHandle = CreateFile(
      deviceName,                       // file name
      GENERIC_READ | GENERIC_WRITE,     // access mode
      0,                                // share mode
      NULL,                             // SECURITY ATTRIBUTES 
      OPEN_EXISTING,                    // how to create
      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, // file attributes
      NULL);                            // handle to template file

    if(m_comHandle == INVALID_HANDLE_VALUE)
    { 
      WinIoTarg_formatErrorMsg(buffer, GetLastError());
      LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "open: Port Create failed\n%19s%s", " ",buffer);
      Sleep(1000);
    }
    else
    {
      if (!SetCommMask(m_comHandle, 
        Win232Channel::EEventBreak |
        Win232Channel::EEventCTS   |
        Win232Channel::EEventDSR   |
        Win232Channel::EEventError |
        Win232Channel::EEventRing  |
        Win232Channel::EEventRLSD  |
        Win232Channel::EEventRecv  
        ))
      {
        WinIoTarg_formatErrorMsg(buffer, GetLastError());
        LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "open: SetCommMask failed\n %19s%s", " ", buffer);
        Sleep(1000);
      }
    }
  }

  if(m_comHandle == INVALID_HANDLE_VALUE)
  { 
    WinIoTarg_formatErrorMsg(m_errorBuffer, GetLastError()); 
    return false;
  }

  const char   *parity_string;
  char   wholeChannel[256];
  bool result;
  DCB dcb;

  result = TRUE;
  switch(m_config.portMode)
  {
  case WIN232_MODE_NONE:
    {
      if(result)
      {
        result = SetupComm(m_comHandle, 3072, 3072) != 0 ? true : false;
      }

      switch (m_config.parity)
      {
      case(WIN232_PARITY_EVEN): 			 
        parity_string ="E"; 
        break;

      case(WIN232_PARITY_ODD):				 
        parity_string ="O"; 
        break;

      default:
        parity_string ="N"; 
        break;
      }

      sprintf(wholeChannel, "baud=%s parity=%s data=%d stop=%d",
        m_config.baudRate, parity_string, m_config.numDataBits, m_config.numStopBits);

      ZeroMemory(&dcb, sizeof(dcb));
      if(result)
      {
        USES_CONVERSION;
        dcb.DCBlength = sizeof(dcb);
        result = BuildCommDCB(A2T(wholeChannel), &dcb) != 0 ? true : false;
      }

      dcb.fBinary 				= true;
      dcb.fOutxCtsFlow		= false;
      dcb.fOutxDsrFlow		= false;
      dcb.fDtrControl 		= DTR_CONTROL_ENABLE; // DTR_CONTROL_ENABLE
      dcb.fDsrSensitivity = false;
      dcb.fOutX 					= false;
      dcb.fInX						= false;
      dcb.fErrorChar			= false;
      dcb.fNull 					= false;
      dcb.fRtsControl 		= RTS_CONTROL_DISABLE; // RTS_CONTROL_DISABLE

      if(result)
      {
        result = SetCommState(m_comHandle, &dcb) != 0 ? true : false;
      }

      /*  For setting SetCommTimeouts
      Total Interval Behavior 
      0     0        Returns when the buffer is completely filled. Time-outs are not used.  

      T     0        Returns when the buffer is completely filled or when T milliseconds 
      have elapsed since the beginning of the operation.  

      0     Y        Returns when the buffer is completely filled or when Y milliseconds 
      have elapsed between the receipt of any two characters. Timing does not 
      begin until the first character is received.  

      T     Y        Returns when the buffer is completely filled or when either 
      type of time-out occurs.  
      */

      if (result && m_config.bModbusRTU == true)
      {
        // with modbus RTU, a message must be contiguous, with no more than 3 empty frames.
        // For now, we will use this fact to extablish the end of a message.  Another aproach
        // is to calculate the CRC on the Fly, and when the sum of the CRC + the data == 0, then
        // the message is with a very high degree of probability, complete.  Yet another way, would 
        // be to have the requesting routine supply the number of bytes expected (not great, from 
        // a layering approach).

        // Set up the timeouts such that ReadFile returns after the first group of characters has been received
        // (the group (i.e. frame is returned when a time period == to ReadIntervalTimeout occurs between characters.

        COMMTIMEOUTS commTimeouts = {0, 0, 0, 0, 0};
        DWORD bitsPerChar = 1 + m_config.numDataBits + (dcb.Parity != NOPARITY ? 1 : 0) + m_config.numStopBits; // 1 start bit, 8 data bits, 1 parity bit + 2 stop bits
        WORD timeBetweenFrames = WORD((m_iNumCharTimesBetweenFrames*bitsPerChar*1000) / dcb.BaudRate) + 1;  // time in mS
        m_charTransmitTime = WORD((bitsPerChar * 1300) / dcb.BaudRate)+1;

        GetCommTimeouts(m_comHandle,&commTimeouts);
        commTimeouts.ReadIntervalTimeout = timeBetweenFrames;
        commTimeouts.ReadTotalTimeoutMultiplier = 0;
        result = SetCommTimeouts(m_comHandle,&commTimeouts) != 0 ? true : false;
        if (!result)
        {
          WinIoTarg_formatErrorMsg(m_errorBuffer, GetLastError());  
          LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "failed to SetCommTimeouts for modbus:\n%19s%s", " ",m_errorBuffer);
        }
      }

      if (result && m_config.bModbusRTU == false)
      { // Must set some timeouts (for now hard coded)
        COMMTIMEOUTS commTimeouts = {0, 0, 0, 0, 0};
        GetCommTimeouts(m_comHandle, &commTimeouts);
        commTimeouts.ReadIntervalTimeout = 100;
        commTimeouts.ReadTotalTimeoutConstant = 500;
        commTimeouts.ReadTotalTimeoutMultiplier = 0;      // #chars to read does not add to timeout amount
        commTimeouts.WriteTotalTimeoutConstant = 2000;
        commTimeouts.WriteTotalTimeoutMultiplier = 60;    // 60ms per char sent

        result = SetCommTimeouts(m_comHandle, &commTimeouts) != 0 ? true : false;
        if (!result)
        {
          WinIoTarg_formatErrorMsg(m_errorBuffer, GetLastError());  
          LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "failed to SetCommTimeouts:\n%19s%s", " ",m_errorBuffer); 
        }
      }

      break;
    }

  case WIN232_MODE_HARDWARE:
    {
      if(result)
      {
        result = SetupComm(m_comHandle, 3072, 3072) != 0 ? true : false;
        if (!result)
        {
          WinIoTarg_formatErrorMsg(m_errorBuffer, GetLastError());  
          LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "failed to SetupComm:\n%19s%s", " ",m_errorBuffer); 
        }
      }

      switch(m_config.parity)
      {
      case(WIN232_PARITY_EVEN): 			 
        parity_string = "E"; 
        break;

      case(WIN232_PARITY_ODD):				 
        parity_string = "O"; 
        break;

      default:
        parity_string = "N";
        break;
      }

      sprintf(wholeChannel,"baud=%s parity=%s data=%d stop=%d", m_config.baudRate, parity_string, m_config.numDataBits, m_config.numStopBits);

      if(result)
      {
        ZeroMemory(&dcb, sizeof(dcb));
        dcb.DCBlength = sizeof(dcb);
        USES_CONVERSION;
        result = BuildCommDCB(A2T(wholeChannel), &dcb) != 0 ? true : false;
        if (!result)
        {
          WinIoTarg_formatErrorMsg(m_errorBuffer, GetLastError());   
          LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "failed to BuildCommDCB:\n%19s%s", " ",m_errorBuffer); 
        }
      }

      dcb.fBinary 				= true;
      dcb.fOutxCtsFlow		= true;
      dcb.fOutxDsrFlow		= true;
      dcb.fDtrControl 		= m_config.dtrMode; // DTR_CONTROL_HANDSHAKE
      dcb.fDsrSensitivity = true;
      dcb.fOutX 					= false;
      dcb.fInX						= false;
      dcb.fErrorChar			= false;
      dcb.fNull 					= false;
      dcb.fRtsControl 		= m_config.rtsMode; // RTS_CONTROL_HANDSHAKE

      if(result)
      {
        result = SetCommState(m_comHandle, &dcb) != 0 ? true : false;
        if (!result)
        {
          WinIoTarg_formatErrorMsg(m_errorBuffer, GetLastError());   
          LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "failed to SetCommState:\n%19s%s", " ",m_errorBuffer);  
        }
      }

      if(result)
      {
        COMMTIMEOUTS commTimeouts = {MAXDWORD, 0, 0, 0, 0};
        result = SetCommTimeouts(m_comHandle, &commTimeouts) != 0 ? true : false;
        if (!result)
        {
          WinIoTarg_formatErrorMsg(m_errorBuffer, GetLastError());      
          LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "failed to SetCommTimeouts:\n%19s%s", " ",m_errorBuffer);  
        }
      }

      break;
    }

  case WIN232_MODE_WINDOWS:
    {
      sprintf(wholeChannel,"baud=%s data=%d stop=%d", m_config.baudRate, m_config.numDataBits, m_config.numStopBits);

      GetCommState(m_comHandle,&dcb);
      if(result)
      {
        result = SetCommState(m_comHandle, &dcb) != 0 ? true : false;
        if (!result)
        {
          WinIoTarg_formatErrorMsg(m_errorBuffer, GetLastError());       
          LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "failed to SetCommState:\n%19s%s", " ",m_errorBuffer);
        }
      }

      if(result)
      {
        result = SetupComm(m_comHandle, 3072, 3072) != 0 ? true : false;
        if (!result)
        {
          WinIoTarg_formatErrorMsg(m_errorBuffer, GetLastError());       
          LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "failed to SetupComm:\n%19s%s", " ",m_errorBuffer);
        }
      }

      if(result)
      {
        COMMTIMEOUTS commTimeouts = {MAXDWORD, 0, 0, 0, 0};
        result = SetCommTimeouts(m_comHandle, &commTimeouts) != 0 ? true : false;
        if (!result)
        {
          WinIoTarg_formatErrorMsg(m_errorBuffer, GetLastError());        
          LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "failed to SetCommTimeouts:\n%19s%s", " ",m_errorBuffer);
        }
      }

      break;
    }
  }

  // if the handle was allocated, but one of the other
  // setup functions failed, then close the handle
  if((!result) && (m_comHandle != INVALID_HANDLE_VALUE))
  {
    CloseHandle(m_comHandle);
    m_comHandle = INVALID_HANDLE_VALUE; 
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "failed to open port");
  }

  if(m_config.disabled)
  {
    return(TMWDEFS_FALSE);
  }

  if((result) && (m_comHandle != INVALID_HANDLE_VALUE))
  {
    LogMessage(TMWDIAG_ID_TARGET, "COM port now open at %s", wholeChannel);
    if(m_pChannelCallback != WINIOTARG_NULL)
    {
      Sleep(100);
      m_pChannelCallback(m_pCallbackParam, true, TMWDEFS_TARG_OC_SUCCESS);
    }
  }

  return(result);
}

// Close channel
void Win232Channel::close(void)
{
  StopWriterThread();
  StopReaderThread();

  if(m_comHandle != INVALID_HANDLE_VALUE)
  {
    CloseHandle(m_comHandle);
    m_comHandle = INVALID_HANDLE_VALUE;
    LogMessage(TMWDIAG_ID_TARGET, "COM port now closed");
  }
}

// receive bytes async
TMWTYPES_USHORT Win232Channel::receive_async(TMWTYPES_UCHAR  *buf, TMWTYPES_USHORT maxBytes, bool  *timeoutOccured)
{
  CriticalSectionLockWinTarg lock(getReadLock());
  DWORD returnBufLen = 0;

  if((m_comHandle != INVALID_HANDLE_VALUE) && m_iReadBufLen > 0)
  {
    if (m_iReadBufLen >= maxBytes)
    {
      memcpy(buf,ReadBuffer,maxBytes);
      m_iReadBufLen -= maxBytes;
      memmove(ReadBuffer,ReadBuffer+maxBytes,m_iReadBufLen);

      returnBufLen = maxBytes;

      // For now just say we did not have inter character timeout 
      // Since we can't tell timeout in middle of message from timeout at end.
      // CRC will fail in link layer if entire message has not been received.
      *timeoutOccured = false;
    }
    else
    {
      memcpy(buf,ReadBuffer,m_iReadBufLen);
      returnBufLen = m_iReadBufLen;
      m_iReadBufLen = 0;

      // For now just say we did not have inter character timeout 
      // Since we can't tell timeout in middle of message from timeout at end.
      // CRC will fail in link layer if entire message has not been received.
      *timeoutOccured = false;
    }
  }

  return((TMWTYPES_USHORT)returnBufLen);
}

// transmit bytes (store in xmit buffer)
bool Win232Channel::transmit(const TMWTYPES_UCHAR *buf, TMWTYPES_USHORT numBytes)
{
  DWORD bytesSent = 0;

  if((m_comHandle != INVALID_HANDLE_VALUE) && (numBytes > 0))
  {
    bytesSent = StoreInWriteBuffer(buf, (DWORD)numBytes);
    SetEvent(m_writerDoWriteEvent);
  }

  return(numBytes == (TMWTYPES_USHORT)bytesSent);
}


// Can we transmit now ?
TMWTYPES_MILLISECONDS Win232Channel::isTransmitReady(void)
{
  /* if the previous frame has not been transmitted, dont buffer these.
   * They will be concatenated and there is no length in RTU frames.
   */
  if (m_config.bModbusRTU && m_iWriteBufLen > 2)
    return 50;

  if (m_config.disabled)
    return 1000;

  if (m_comHandle != INVALID_HANDLE_VALUE)
  {
    /* first check to see if write buffer is filling up */
    if (m_iWriteBufLen > WRITE_BUFFER_HIGH_WATER)
    {
      return 100;
    }

    bool canTransmit = false;

    DCB testDCB;
    GetCommState(m_comHandle, &testDCB);

    if ((testDCB.fOutxCtsFlow == TRUE) && (testDCB.fOutxDsrFlow == TRUE))
    {
      // We must be in hardware mode
      DWORD ModemStat;
      GetCommModemStatus(m_comHandle, &ModemStat);
      if ((ModemStat & MS_CTS_ON) && (ModemStat & MS_DSR_ON))
      {
        // CTS and DSR must be high to send data if in Hardware mode
        canTransmit = true;
      }
    }
    else
    {
      // We must be in non hardware mode
      canTransmit = true;
    } 

    // read port status
    DWORD commError;
    COMSTAT lpStat;
    ClearCommError(m_comHandle, &commError, &lpStat);

    return((lpStat.cbOutQue == 0 && canTransmit) ? 0 : 100);

  }

  return(0);
}

// Set Ready To Send
bool Win232Channel::SetRTS(bool value)
{
  // Reset error state
  m_lLastError = ERROR_SUCCESS;

  // Check if the device is open
  if (m_comHandle == INVALID_HANDLE_VALUE)
  {
    // Set the internal error code
    m_lLastError = ERROR_INVALID_HANDLE;

    // Issue an error and quit
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "GetError - Device is not opened 0x%08x",m_lLastError);
    return false;
  }

  if (!::EscapeCommFunction(m_comHandle, (value ? SETRTS : CLRRTS)))
  {
    // Display a warning
    WinIoTarg_formatErrorMsg(m_errorBuffer, GetLastError());
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "SetRTS - Unable to set ready to send\n%19s%s", " ",m_errorBuffer);
    return false;
  }
  return true;
}

// Set Data Terminal Ready
bool Win232Channel::SetDTR(bool value)
{
  // Reset error state
  m_lLastError = ERROR_SUCCESS;

  // Check if the device is open
  if (m_comHandle == INVALID_HANDLE_VALUE)
  {
    // Set the internal error code
    m_lLastError = ERROR_INVALID_HANDLE;

    // Issue an error and quit
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "GetError - Device is not opened 0x%08x", m_lLastError);
    return false;
  }

  if (!::EscapeCommFunction(m_comHandle, (value ? SETDTR : CLRDTR)))
  {
    // Display a warning
    WinIoTarg_formatErrorMsg(m_errorBuffer, GetLastError());
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "SetDTR - Unable to set data terminal ready\n%19s%s", " ", m_errorBuffer);
    return false;
  }
  return true;
}

Win232Channel::EError Win232Channel::GetError (void)
{
  // Reset error state
  m_lLastError = ERROR_SUCCESS;

  // Check if the device is open
  if (m_comHandle == INVALID_HANDLE_VALUE)
  {
    // Set the internal error code
    m_lLastError = ERROR_INVALID_HANDLE;

    // Issue an error and quit
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "GetError - Device is not opened, Handle is not valid");
    return EErrorUnknown;
  }

  // Obtain COM status
  DWORD dwErrors = 0;
  if (!::ClearCommError(m_comHandle,&dwErrors,0))
  {
    // Set the internal error code
    m_lLastError = GetLastError();        

    // Issue an error and quit
    WinIoTarg_formatErrorMsg(m_errorBuffer, m_lLastError);  
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "GetError - Unable to obtain COM status\n%19s%s", " ",m_errorBuffer);
    return EErrorUnknown;
  }

  // Return the error
  return EError(dwErrors);
}

// Get Clear To Send
bool Win232Channel::GetCTS (void)
{
  // Reset error state
  m_lLastError = ERROR_SUCCESS;

  // Obtain the modem status
  DWORD dwModemStat = 0;
  if (!::GetCommModemStatus(m_comHandle,&dwModemStat))
  {
    // Obtain the error code
    m_lLastError = GetLastError();

    // Display a warning
    WinIoTarg_formatErrorMsg(m_errorBuffer, m_lLastError);   
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "GetCTS - Unable to obtain the modem status\n%19s%s", " ",m_errorBuffer);
    return false;
  }

  // Determine if CTS is on
  return (dwModemStat & MS_CTS_ON) != 0;
}

bool Win232Channel::GetDSR (void)
{
  // Reset error state
  m_lLastError = ERROR_SUCCESS;

  // Obtain the modem status
  DWORD dwModemStat = 0;
  if (!::GetCommModemStatus(m_comHandle,&dwModemStat))
  {
    // Obtain the error code
    m_lLastError = GetLastError();

    // Display a warning
    WinIoTarg_formatErrorMsg(m_errorBuffer, m_lLastError); 
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "GetDSR - Unable to obtain the modem status\n%19s%s", " ",m_errorBuffer);
    return false;
  }

  // Determine if DSR is on
  return (dwModemStat & MS_DSR_ON) != 0;
}

bool Win232Channel::GetRing (void)
{
  // Reset error state
  m_lLastError = ERROR_SUCCESS;

  // Obtain the modem status
  DWORD dwModemStat = 0;
  if (!::GetCommModemStatus(m_comHandle,&dwModemStat))
  {
    // Obtain the error code
    m_lLastError = GetLastError();

    // Display a warning
    WinIoTarg_formatErrorMsg(m_errorBuffer, m_lLastError);
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "GetRing - Unable to obtain the modem status\n%19s%s", " ",m_errorBuffer);
    return false;
  }

  // Determine if Ring is on
  return (dwModemStat & MS_RING_ON) != 0;
}

bool Win232Channel::GetRLSD (void)
{
  // Reset error state
  m_lLastError = ERROR_SUCCESS;

  // Obtain the modem status
  DWORD dwModemStat = 0;
  if (!::GetCommModemStatus(m_comHandle,&dwModemStat))
  {
    // Obtain the error code
    m_lLastError = GetLastError();

    // Display a warning
    WinIoTarg_formatErrorMsg(m_errorBuffer, m_lLastError); 
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "GetRLSD - Unable to obtain the modem status\n%19s%s", " ",m_errorBuffer);
    return false;
  }

  // Determine if RLSD is on
  return (dwModemStat & MS_RLSD_ON) != 0;
}

// Get current channel status
const char   *Win232Channel::getStatus(void)
{
  if(m_comHandle == INVALID_HANDLE_VALUE)
  {
    sprintf(m_status, "Closed");
  }
  else
  {
    COMSTAT lpStat;        /* windows com status structure */
    DWORD comError;

    ClearCommError(m_comHandle, &comError, &lpStat);
    sprintf(m_status, "Open: OutQue = %4u, InQue = %4u", lpStat.cbOutQue, lpStat.cbInQue);
  }

  return(m_status);
}

/* function: Create */
Win232Channel *Win232Channel::Create(
  const void *pConfig, 
  TMWTARG_CONFIG *pTmwTargConfig)
{
  const TMWTARG232_CONFIG *p232Config = (TMWTARG232_CONFIG *)pConfig;
  Win232Channel *pChannel = new Win232Channel(*p232Config, pTmwTargConfig);
  pChannel->setChannelCallback(pTmwTargConfig->pChannelCallback, pTmwTargConfig->pCallbackParam);
  pChannel->m_pChannel = pTmwTargConfig->pChannel;
#if TMWTARG_SUPPORT_CHANNEL_THREAD
  pChannel->m_chanThreadCheckMask = CT_CHECK_CONNECT;
  pChannel->ChannelThreadStart();
#endif
  return pChannel;
}

/* function: win232_deleteChannel */
void Win232Channel::deleteWinIoChannel()
{
  closeWinIoChannel();
}

bool Win232Channel::modifyWinIoChannel(
  const void *pUserConfig)
{
  WINIO_CONFIG  *pIOConfig = (WINIO_CONFIG *)pUserConfig; 
  m_config.disabled = pIOConfig->targ232.disabled;

  if (pIOConfig->targ232.chnlName != NULL)
  {
    strncpy(m_config.chnlName, pIOConfig->targ232.chnlName, WINIOTARG_STR_LEN);
  }

  // SCL will close and reopen for other parameter changes 
  return true;
}

/* function: win232_isChannelOpen */
bool Win232Channel::isChannelOpen()
{
  if(m_config.disabled)
    return(false);
  return(m_comHandle != INVALID_HANDLE_VALUE);
}

/* function: win232_openChannel */
bool Win232Channel::openWinIoChannel(
  TMWTARG_CHANNEL_RECEIVE_CBK_FUNC pCallbackFunc, 
  TMWTARG_CHECK_ADDRESS_FUNC pCheckAddrCallbackFunc, 
  void *pCallbackParam, 
  WINIO_OPEN_MODE_ENUM openMode)
{
  /* If disabled, but already open, just return false.
   * This allows modem commands to be sent without interference by SCL output
   */
  if (m_config.disabled && (m_comHandle != INVALID_HANDLE_VALUE))
    return false;

  m_pRecvDataFunc = pCallbackFunc;
  m_pCheckAddrFunc = pCheckAddrCallbackFunc;
  m_pChanContextCBData = pCallbackParam;
  
  if (open())
  {
    StartReaderThread();
    return true;
  }
  return false;
}

/* function: win232_closeChannel */
void Win232Channel::closeWinIoChannel()
{
  close();
}

/* function: win232_getChannelName */
const char   *Win232Channel::getChannelName()
{
  return getName();
}

const char   *Win232Channel::getChannelInfo()
{
  return getInfo();
}

const char   *Win232Channel::getChannelStatus()
{
  return getStatus();
}

/* function: win232_getTransmitReady */
TMWTYPES_MILLISECONDS Win232Channel::getTransmitReady()
{
  return isTransmitReady();
}

TMWTYPES_BOOL Win232Channel::waitForInput()
{
	return false;
}

/* function: win232_receive */
TMWTYPES_USHORT Win232Channel::receiveOnChannel(
  TMWTYPES_UCHAR *buf, 
  TMWTYPES_USHORT maxBytes, 
  TMWTYPES_MILLISECONDS maxTimeout,
  bool  *timeoutOccured)
{
  return receive_async(buf, maxBytes, timeoutOccured);
}

/* function: win232_transmit */
bool Win232Channel::transmitOnChannel(
  TMWTYPES_UCHAR *buf, 
  TMWTYPES_USHORT numBytes)
{
  bool bStarted = StartWriterThread();
  if (bStarted == false)
  {
    return 0;
  }
  return transmit(buf, numBytes);
}

void Win232Channel::LogMessage(
  TMWDIAG_ID sourceID, 
  const char   *format, 
  ...)
{
  if (WinIoTarg_IsProtoAnaLogEnabled() == false)
  {
    return;
  }

  if (WinIoTargProtoAnaLogFun != WINIOTARG_NULL)
  {
    CriticalSectionLockWinTarg lock(getLogLock());
    char   _chanID[256];
    sprintf(_chanID,"%s - %s - Win232 ",this->m_config.chnlName,this->m_config.portName);
    
    va_list va;
    va_start(va, format);
    WinIoTargProtoAnaLogFun(m_pChannel, TRACE_MASK_NONE, sourceID, _chanID, format, va);
    va_end(va);
  }
}



