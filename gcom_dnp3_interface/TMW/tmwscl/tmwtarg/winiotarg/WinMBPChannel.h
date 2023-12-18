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

/* file: WinMBPChannel.h
 * description: Implementation of Windows Modbus Plus Target interface. 
 */
#ifndef WINMBP_DEFINED
#define WINMBP_DEFINED

#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwphys.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwtypes.h"
#include "WinIoTarg/WinIoInterface.h"
#include "WinIoTarg/include/WinIoTargDefs.h"

#pragma comment(lib, "netlib.lib")
#pragma comment(lib, "Delayimp.lib")

#include "Mbpsupp/netbios.h"
#include "Mbpsupp/netlib.h"

// Define class used to manage a modbus plus channel
class WinMBPChannel : public WinIoInterface
{
public:
  // Constructor
  WinMBPChannel(const WINMBP_CONFIG &config, TMWTARG_CONFIG *pTmwTargConfig);
  
  // Destructor
  virtual ~WinMBPChannel(void);
  
  // Return the name for this channel
  const char   *getName(void);
  
  // Return configuration info for this channel
  const char   *getInfo(void);
  
  // Open this channel
  bool open();
  
  // Close this channel
  void close(void);
  
  // Receive up to maxBytes bytes on this channel and store in buf
  TMWTYPES_USHORT receive(TMWTYPES_UCHAR *buf, TMWTYPES_USHORT maxBytes);
  
  // Transmit numBytes bytes from buf to this channel
  bool transmit(const TMWTYPES_UCHAR *buf, TMWTYPES_USHORT numBytes);
  
  // Return true if we can transmit on this channel
  TMWTYPES_MILLISECONDS isTransmitReady(void);
  
  // Get channel status as a string
  const char   *getStatus(void);
  
private:
  char   *m_name;
  char   m_path[20];
  TMWTYPES_USHORT m_cardNum;
  char   m_info[128];
  char   m_status[256];
  WINMBP_CONFIG m_config;
  NCB *m_pNCB;  /* netbios control block ptr */
  bool m_bChannelOpen;
  
  bool m_bBusy; 
  bool m_bLeaveMasterPortOpen;
  
public:  // Interface
  static WinMBPChannel *Create(
    const void *pConfig, 
    TMWTARG_CONFIG *pTmwTargConfig);
  
  virtual void deleteWinIoChannel();
  
  virtual bool modifyWinIoChannel(const void *pUserConfig);
  
  virtual bool isChannelOpen();
  
  virtual bool openWinIoChannel(TMWTARG_CHANNEL_RECEIVE_CBK_FUNC pCallbackFunc, 
    TMWTARG_CHECK_ADDRESS_FUNC pCheckAddrCallbackFunc, void *pCallbackParam, WINIO_OPEN_MODE_ENUM openMode);
  
  virtual void closeWinIoChannel();
  
  virtual const char   *getChannelName();
  
  virtual const char   *getChannelInfo();
  virtual const char   *getChannelStatus();
  
  virtual TMWTYPES_MILLISECONDS getTransmitReady();

	virtual TMWTYPES_BOOL waitForInput();

  virtual TMWTYPES_USHORT receiveOnChannel( 
    TMWTYPES_UCHAR *pBuff, 
    TMWTYPES_USHORT maxBytes, 
    TMWTYPES_MILLISECONDS maxTimeout,
    bool  *timeoutOccured);
  
  virtual bool transmitOnChannel(
    TMWTYPES_UCHAR *pBuff, 
    TMWTYPES_USHORT numBytes);
  
};

#endif /* WINMBP_DEFINED */

