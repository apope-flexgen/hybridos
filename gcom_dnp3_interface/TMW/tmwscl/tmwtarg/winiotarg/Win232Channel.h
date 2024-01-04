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
/* file: Win232Channel.h
* description: Implementation of Windows serial I/O Target interface. 
*/
#ifndef Win232Channel_DEFINED
#define Win232Channel_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwtypes.h"
#include "tmwscl/utils/tmwtarg.h"
#include "WinIoTarg/WinIoInterface.h"
#include "WinIoTarg/include/WinIoTargDefs.h"
#include "WinIoTarg/WinThreading.h"

#define MAX_READ_BUFFER         8192
#define MAX_WRITE_BUFFER        8192

/* When to tell the SCL to slow down transmitting data. 
 * 550 bytes is larger than maximum SCL transmits at a time
 */
#define WRITE_BUFFER_HIGH_WATER   (MAX_WRITE_BUFFER-550)

// Set this larger than largest modbus RTU message, so when read completes
// we know inter character timeout occurred, either during message 
// or after last byte was received.
// At least 292 for DNP is more efficient.
#define AMOUNT_TO_READ          300
#define AMOUNT_TO_WRITE         300
#define NUM_READSTAT_HANDLES    2
#define NUM_WRITESTAT_HANDLES   2
#define RS232_EVENTFLAGS_DEFAULT      EV_BREAK | EV_CTS | EV_DSR | EV_ERR | EV_RING | EV_RLSD

// Define class used to manage an RS232 channel
class Win232Channel : public WinIoInterface
{
public:
  // Communication event
  typedef enum
  {
    EEventUnknown  	   = -1,			// Unknown event
    EEventNone  	   = 0,				// Event trigged without cause
    EEventBreak 	   = EV_BREAK,		// A break was detected on input
    EEventCTS   	   = EV_CTS,		// The CTS signal changed state
    EEventDSR   	   = EV_DSR,		// The DSR signal changed state
    EEventError 	   = EV_ERR,		// A line-status error occurred
    EEventRing  	   = EV_RING,		// A ring indicator was detected
    EEventRLSD  	   = EV_RLSD,		// The RLSD signal changed state
    EEventRecv  	   = EV_RXCHAR,		// Data is received on input
    EEventRcvEv 	   = EV_RXFLAG,		// Event character was received on input
    EEventSend		   = EV_TXEMPTY,	// Last character on output was sent
    EEventPrinterError = EV_PERR,		// Printer error occured
    EEventRx80Full	   = EV_RX80FULL,	// Receive buffer is 80 percent full
    EEventProviderEvt1 = EV_EVENT1,		// Provider specific event 1
    EEventProviderEvt2 = EV_EVENT2,		// Provider specific event 2
  } 
  EEvent;

  // Communication errors
  typedef enum
  {
    EErrorUnknown = 0,			// Unknown
    EErrorBreak   = CE_BREAK,	// Break condition detected
    EErrorFrame   = CE_FRAME,	// Framing error
    EErrorIOE     = CE_IOE,		// I/O device error
    EErrorMode    = CE_MODE,	// Unsupported mode
    EErrorOverrun = CE_OVERRUN,	// Character buffer overrun, next byte is lost
    EErrorRxOver  = CE_RXOVER,	// Input buffer overflow, byte lost
    EErrorParity  = CE_RXPARITY,// Input parity error
    EErrorTxFull  = CE_TXFULL	// Output buffer full
  }
  EError;


  // Constructor
  Win232Channel(const TMWTARG232_CONFIG &config, TMWTARG_CONFIG *pTmwTargConfig);

  // Destructor
  virtual ~Win232Channel(void);
  
  // Set open/close callbacks
  void setChannelCallback(
    TMWTARG_CHANNEL_CALLBACK_FUNC pChannelCallback, 
    void *pCallbackParam);

  // Return the name for this channel
  const char   *getName(void);

  // Return configuration info for this channel
  const char   *getInfo(void);

  // Get this channel handle
  void *getComHandle() { return m_comHandle; }

  // Open this channel
  bool open();

  // Close this channel
  void close(void);

  // Transmit numBytes bytes from buf to this channel
  bool transmit(const TMWTYPES_UCHAR *buf, TMWTYPES_USHORT numBytes);

  // Receive up to maxBytes bytes on this channel and store in buf
  TMWTYPES_USHORT receive_async(TMWTYPES_UCHAR *buf, TMWTYPES_USHORT maxBytes, bool  *timeoutOccured);

  // Transmit numBytes bytes from buf to this channel async
  bool transmit_async(const TMWTYPES_UCHAR *buf, TMWTYPES_USHORT numBytes);

  // Set/clear Ready To Send hardware control
  bool SetRTS(bool value);

  // Set/clear Data Terminal Ready hardware control
  bool SetDTR(bool value);

  // Return true if we can transmit on this channel
  TMWTYPES_MILLISECONDS isTransmitReady(void);

  // Get channel status as a string
  const char   *getStatus(void);

  //////////////
  HANDLE getReaderStopEvent()
  {
    return m_readerStopEvent;
  }

  AutoCriticalSectionWinTarg &getReadLock()
  {
    return m_readLock;
  }
  void StopReaderThread();
  bool StartReaderThread(void);

  //////////////
  HANDLE getWriterStopEvent()
  {
    return m_writerStopEvent;
  }

  AutoCriticalSectionWinTarg &getWriteLock()
  {
    return m_writeLock;
  }
  void StopWriterThread();
  bool StartWriterThread(void);

  virtual void LogMessage(TMWDIAG_ID sourceID, const char   *format, ...);

  // Obtain CTS/DSR/RING/RLSD settings
  bool GetCTS (void);
  bool GetDSR (void);
  bool GetRing (void);
  bool GetRLSD (void);

  // Obtain the error
  EError GetError (void);

protected:
  LONG	m_lLastError;		// Last serial error
  void StoreInReadBuffer(TMWTYPES_UCHAR * lpBuf, DWORD dwBufLen);
  DWORD StoreInWriteBuffer(const TMWTYPES_UCHAR * lpBuf, DWORD dwBufLen);

private:
  EEvent m_dwStoredFlags;
  char   *m_name;
  char   m_info[128];
  char   m_status[128];
  char   m_errorBuffer[128];
  HANDLE m_comHandle;  

  TMWTARG232_CONFIG m_config;
  TMWTYPES_ULONG m_iNumCharTimesBetweenFrames;
  TMWTYPES_ULONG m_interCharTimeout;
  TMWTYPES_ULONG m_charTransmitTime;
  
  // Open/Close callback specified in open, will be called whenever we
  // detect an open or close
  void *m_pCallbackParam;
  TMWTARG_CHANNEL_CALLBACK_FUNC m_pChannelCallback;

  // 
  bool m_readerThreadIsRunning;
  unsigned int m_readerThreadID;
  HANDLE m_readerThreadHandle;
  HANDLE m_readerStopEvent;   // handle to stop event
  AutoCriticalSectionWinTarg m_readLock;

  TMWTYPES_UCHAR  ReadBuffer[MAX_READ_BUFFER];
  DWORD m_iReadBufLen;

  // 
  bool m_writerThreadIsRunning;
  unsigned int m_writerThreadID;
  HANDLE m_writerThreadHandle;
  HANDLE m_writerStopEvent;   // handle to stop event
  AutoCriticalSectionWinTarg m_writeLock;
  TMWTYPES_UCHAR  WriteBuffer[MAX_WRITE_BUFFER];
  DWORD m_iWriteBufLen;
  HANDLE m_writerDoWriteEvent;   // handle to tell thread to write buffer

  //
  static unsigned int __stdcall readerThread(void *pParam);
  static unsigned int __stdcall writerThread(void *pParam);


public:  // Interface
  static Win232Channel *Create(const void *pConfig, TMWTARG_CONFIG *pTmwTargConfig);

  virtual void deleteWinIoChannel();

  virtual bool modifyWinIoChannel(const void *pUserConfig);

  virtual bool isChannelOpen();

  virtual bool openWinIoChannel(TMWTARG_CHANNEL_RECEIVE_CBK_FUNC pCallbackFunc, TMWTARG_CHECK_ADDRESS_FUNC pCheckAddrCallbackFunc, void *pCallbackParam, WINIO_OPEN_MODE_ENUM openMode);

  virtual void closeWinIoChannel();

  virtual const char   *getChannelName();

  virtual const char   *getChannelInfo();
  virtual const char   *getChannelStatus();

  virtual TMWTYPES_MILLISECONDS getTransmitReady();

	virtual TMWTYPES_BOOL waitForInput();

  virtual TMWTYPES_USHORT receiveOnChannel(TMWTYPES_UCHAR *pBuff, TMWTYPES_USHORT maxBytes, TMWTYPES_MILLISECONDS maxTimeout, bool *timeoutOccured);

  virtual bool transmitOnChannel(TMWTYPES_UCHAR *pBuff, TMWTYPES_USHORT numBytes);

};

#endif /* Win232Channel_DEFINED */

