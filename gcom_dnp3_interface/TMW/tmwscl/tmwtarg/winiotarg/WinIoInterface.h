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

/* file: WinIoInterface.h
* description: Interface to the target I/O routines. This abstract base class provides
* a generic interface to the various low level target interfaces 
* (see the pure virtual functions below). Currently this includes the Modbus Plus Interface, 
* RS232 interface and the TCP/IP interface.
*
*/

#ifndef WinIoInterface_DEFINED
#define WinIoInterface_DEFINED

#include "tmwscl/utils/tmwtarg.h"
#include "WinIoTarg/include/WinIoTargDefs.h"
#include "WinIoTarg/WinThreading.h"

extern "C" void WinIoAssertion(const char   *expr, const char   *file, int line);

#if defined(WINIOTARG_INCLUDE_ASSERTS)
#define WINIO_ASSERT(expr) ((expr) ? ((void) 0) : WinIoAssertion(#expr, __FILE__, __LINE__))
#else
#define WINIO_ASSERT(expr) ((void) 0)
#endif /* WINIOTARG_INCLUDE_ASSERTS */

#define WINIOTARG_UNUSED_PARAM(x) x

// Bit Mask that defines the operations that the channel thread should peform.
typedef enum
{
  CT_CHECK_NONE    = 0x00,
  CT_CHECK_CONNECT = 0x01,
  CT_CHECK_INPUT   = 0x02,
  CT_NOT_ACTIVE    = 0x04
}
CT_CHECK_MASK;

// This class is exported from the WinIoTarg.dll
class WinIoInterface
{
public:
  // constructor/destructor
  WinIoInterface();
  virtual ~WinIoInterface();


  WINIO_CONTEXT *pWinIoContext;

  AutoCriticalSectionWinTarg &getLogLock()
  {
    return m_logLock;
  }
  static void ProtoAnaLog(TMWDIAG_ID sourceID, const char   *format, ...);
  virtual void LogMessage(TMWDIAG_ID sourceID, const char   *format, ...)
  {
    WINIOTARG_UNUSED_PARAM(sourceID);    
    WINIOTARG_UNUSED_PARAM(format);    

    // When closing channel and deleting WinTCPChannel object WinTCPChannel::connect 
    // can fail and LogMessage get called. Since WinTCPChannel is partially deleted
    // this LogMessage gets called instead, causing ASSERT. 
    // removed this ASSERT.
    //WINIO_ASSERT(FALSE);
    return;
  }
  
  // Open/Close callback specified in open, will be called whenever we
  // detect an open or close
  void *m_pCallbackParam;
  TMWTARG_CHANNEL_CALLBACK_FUNC m_pChannelCallback;

public:
  // Interface (pure virtual)

  virtual void           deleteWinIoChannel(void) = 0;
  virtual bool           modifyWinIoChannel(const void *pUserConfig) = 0;
  virtual bool           isChannelOpen(void) = 0;
  virtual bool           openWinIoChannel(
  TMWTARG_CHANNEL_RECEIVE_CBK_FUNC pCallbackFunc, 
  TMWTARG_CHECK_ADDRESS_FUNC pCheckAddrCallbackFunc, 
  void *pCallbackParam,
  WINIO_OPEN_MODE_ENUM openMode) = 0;
  virtual void                   closeWinIoChannel(void) = 0;
  virtual bool                   resetWinIoChannel(void)
  {  // implemented here because it's optional
    return false;
  }
  virtual const char              *getChannelName(void) = 0;
  virtual const char              *getChannelInfo(void) = 0;
  virtual const char              *getChannelStatus(void) = 0;
  virtual TMWTYPES_MILLISECONDS  getTransmitReady(void) = 0;

  virtual TMWTYPES_BOOL waitForInput(void) = 0;

  virtual TMWTYPES_USHORT receiveOnChannel( 
    TMWTYPES_UCHAR *pBuff, 
    TMWTYPES_USHORT maxBytes, 
    TMWTYPES_MILLISECONDS maxTimeout,
    bool  *timeoutOccured) = 0;

  virtual bool transmitOnChannel( 
    TMWTYPES_UCHAR *pBuff, 
    TMWTYPES_USHORT numBytes) = 0;

  virtual bool transmitUDP( 
    TMWTYPES_UCHAR UDPPort,
    TMWTYPES_UCHAR *pBuff, 
    TMWTYPES_USHORT numBytes)
  {
    WINIOTARG_UNUSED_PARAM(UDPPort);
    WINIOTARG_UNUSED_PARAM(pBuff);
    WINIOTARG_UNUSED_PARAM(numBytes);
    return(false);
  };

  //This method is called by the channel thread to determine
  //if receive data is available.
  virtual bool checkReceive(TMWTYPES_MILLISECONDS timeout)
  {
    WaitForSingleObject(m_channelThreadEvent, timeout);
    return false;
  }

  bool ChannelThreadStart();
  bool ChannelThreadStop();

  //Return true if this channel is configured to have the channel thread initiate the
  //connection instead of performing in the context of the library via tmwtarg_openChannel
  bool ChannelThreadConnecting()
  {
    return ((m_chanThreadCheckMask & CT_CHECK_CONNECT) == CT_CHECK_CONNECT);
  }
  void closeChannel()
  {
    m_chanState = TMWTARG_CHANNEL_CLOSED;
  }
  void openChannel()
  {
    m_chanState = TMWTARG_CHANNEL_OPENED;
    m_chanThreadCheckMask &= ~CT_NOT_ACTIVE;
    SetEvent(m_channelThreadEvent);
  }

  TMWTARG_CHANNEL_RECEIVE_CBK_FUNC getRecvCallbackFunc()
  {
    return m_pRecvDataFunc;
  }

  TMWTARG_CHECK_ADDRESS_FUNC getCheckAddrCallbackFunc()
  {
    return m_pCheckAddrFunc;
  }

  void *getChanContextCBData()
  {
    return m_pChanContextCBData;
  }

  
  void setDataCallbacks(TMWTARG_CHANNEL_RECEIVE_CBK_FUNC pCallbackFunc, 
                                    TMWTARG_CHECK_ADDRESS_FUNC pCheckAddrCallbackFunc, 
                                    void *pCallbackParam)
  {
    m_pRecvDataFunc = pCallbackFunc;
    m_pCheckAddrFunc = pCheckAddrCallbackFunc;
    m_pChanContextCBData = pCallbackParam;
  }

  void setStatusCB(WINIO_STATUS_CALLBACK pCallback, void *pCallbackParam)
  {
    m_pStatusCallback = pCallback;
    m_pStatusCallbackParam = pCallbackParam;
  }

  void setChanEnabled(bool bState)
  {
    m_bEnabled = bState;
  }
  bool isChanEnabled(void)
  {
    return m_bEnabled;
  }

  void setForceDisconnected(bool bState)
  {
    m_forceDisconnected = bState;
  }
  bool isForceDisconnected(void)
  {
    return m_forceDisconnected;
  }

protected:
  TMWTARG_CHANNEL_RECEIVE_CBK_FUNC m_pRecvDataFunc;
  TMWTARG_CHECK_ADDRESS_FUNC m_pCheckAddrFunc;
  void *m_pChanContextCBData;

  WINIO_STATUS_CALLBACK m_pStatusCallback;
  void *m_pStatusCallbackParam;

  void *m_pChannel;

  TMWTARG_CHANNEL_STATE      m_chanState;
  TMWTARG_THREAD_STATE       m_chanThreadState;
  AutoCriticalSectionWinTarg m_chanThreadStateLock;
  unsigned char              m_chanThreadCheckMask;
  HANDLE                     m_channelThreadEvent;   // handle to signal channel thread
  SOCKET                     m_selectPendingSocket;

  bool callStatusCB(WINIO_STATUS statusType, TMWTYPES_ULONG statusData)
  {
    if (m_pStatusCallback)
    {
      m_pStatusCallback(m_pStatusCallbackParam, statusType, statusData);
      return true;
    }
    return false;
  }

private:
  AutoCriticalSectionWinTarg m_logLock;
  bool m_bEnabled;
  bool m_forceDisconnected;
  
  unsigned int  m_channelThreadID;
  HANDLE        m_channelThreadHandle;

  // Channel thread entry point
  static unsigned int __stdcall channelThread(WinIoInterface *pParam);
};

#endif // WinIoInterface_DEFINED
