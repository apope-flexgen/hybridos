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

/* file: WinMBPChannel.cpp
 * description: Implementation of Windows Modbus plus Target interface. 
 */

#include "StdAfx.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif


#if WIN_MBPLUS_SUPPORT
#include "WinIoTarg/include/WinIoTarg.h"
#include "WinIoTarg/include/WinIoTargDefs.h"

#include "WinIoTarg/WinMBPChannel.h"

// Constructor
WinMBPChannel::WinMBPChannel(const WINMBP_CONFIG &config, TMWTARG_CONFIG *pTmwTargConfig) : 
  m_name(WINIOTARG_NULL)
{
  pTmwTargConfig = pTmwTargConfig;
  static char   sDefaultName[] = "Un-named MB+ Card";
  // Copy configuration data structure
  m_config = config;

  strncpy(m_config.chnlName, config.chnlName, WINIOTARG_STR_LEN);

  // Set channel name
  m_name = (m_config.chnlName[0] != 0) ? m_config.chnlName : sDefaultName;
 
  m_pNCB = NULL;
  m_bBusy = false; 
  m_bLeaveMasterPortOpen = config.leaveMasterPortOpen != 0 ? true : false; 
  m_cardNum = config.cardNum;
  m_config.mode = config.mode;
  m_bChannelOpen = false;
  
  if (m_config.mode == WINMBP_MODE_CLIENT)
  {
    strcpy(m_path, "DM.");
    strcat(m_path, config.routePath);
  }
  else // Serever
    sprintf( m_path, "DS.%d.00.00.00.00", config.slavePath); // path string
}

 // Destructor
WinMBPChannel::~WinMBPChannel(void)
{
  // Close channel if not already closed
  close();

}

// Return channel name
const char   *WinMBPChannel::getName(void)
{
  return(m_name);
}

// Return channel info
const char   *WinMBPChannel::getInfo(void)
{
  if (m_config.mode == WINMBP_MODE_CLIENT)
    sprintf(m_info,"%s, for %s as Client", m_name, m_config.routePath);
  else
    sprintf(m_info,"%s on Slave Path %d as Server", m_name, m_config.slavePath);
  return(m_info);
}

// Open channel
bool WinMBPChannel::open()
{ 
  if (m_config.mode == WINMBP_MODE_SERVER)
  {
    char *pPath = &m_path[0];
		if( ( m_pNCB = ncb_open( pPath, m_cardNum ) ) == NULL ) 
    {
      WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR,  "MBP: Unable to open DATA SLAVE path.\n" );
      return false;
		}
    m_pNCB->NCB_LENGTH = 0;
    m_bBusy = false;
    m_bChannelOpen = true;
  }
  WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET,  "MBP: Server opened.\n" );
  return(true);
}

// Close channel
void WinMBPChannel::close(void)
{
  if (m_pNCB != NULL)
  {
    ncb_close (m_pNCB);
    m_pNCB = NULL;
  }
  m_bChannelOpen = false;
  WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET,  "MBP: closed.\n" );
}

TMWTYPES_BOOL WinMBPChannel::waitForInput()
{
	return false;
}

// receive bytes
TMWTYPES_USHORT WinMBPChannel::receive(TMWTYPES_UCHAR  *buf, TMWTYPES_USHORT maxBytes)
{
  TMWTYPES_USHORT numRcv = 0;
  
  // Make sure that we don't poll slave path for new messages
  // before replying
  if ((m_config.mode == WINMBP_MODE_SERVER) && m_bBusy)
    return 0;

  if ((m_config.mode == WINMBP_MODE_CLIENT) && !m_bBusy)
    return 0;

  // if NCB is not defined then simply return 0 bytes received
  if (m_pNCB == WINIOTARG_NULL)
    return 0;

  int ret_val = ncb_receive( m_pNCB, (char *) buf);
	if( ret_val != 0 ) 
  {
    WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR,  "MBP: Receive error: %d occurred.\n", m_pNCB->NCB_RETCODE );
    // note that Err_busy or Err_ttimeout means that response has not yet 
    // been received
    if ((ret_val != ERR_busy) && (ret_val != ERR_timeout))
    {
      // if mode == client then close path on error
      if (m_config.mode == WINMBP_MODE_CLIENT)
      {
        if (m_config.leaveMasterPortOpen == false)
        {
          ret_val = ncb_close( m_pNCB );
          m_pNCB = NULL;
        }
        m_bBusy = false;
      }
    }
    return 0;
  }
  
  numRcv = (TMWTYPES_USHORT)(m_pNCB->NCB_LENGTH + 1); //????
  if (numRcv > maxBytes)
  {
    // Log error
  }

  // if mode == client then close path on success
	if (m_config.mode == WINMBP_MODE_CLIENT)
  {
    if (m_config.leaveMasterPortOpen == false)
    {
      ret_val = ncb_close( m_pNCB );
      m_pNCB = NULL;
    }
    // Reset busy flag to indicate channel is free
    // to send new requests
    m_bBusy = false;
  }
  else
  { 
    // Server... set busy flag so we know 
    // not to receive again until processed
    m_bBusy = true;
  }

  return numRcv;
}

// transmit bytes
bool WinMBPChannel::transmit(const TMWTYPES_UCHAR *buf, TMWTYPES_USHORT numBytes)
{
  int ret_val; // general purpose return value

  if (m_config.mode == WINMBP_MODE_SERVER)
  {
    // if there is a pending unsolicted req try to send response
    if ((m_pNCB != NULL) && m_bBusy)
    {
      if( ncb_send( m_pNCB, numBytes, (char *) buf, 10) != 0 ) 
      {
        // close path on error probably not
        m_bBusy = false;
        return false;
      }
      // reset busy flag so receive routine knows it can 
      // receive new unsolited messages
      m_bBusy = false;
      return true;
    }
  }
  else  // Client
  {
    if (m_bBusy)
      return false;

    if (m_pNCB == NULL)
    {
      char *pPath = &m_path[0];

		  if( ( m_pNCB = ncb_open( pPath, m_cardNum ) ) == NULL ) 
      {
    	  WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR,  "MBP: Unable to open DATA MASTER path.\n" );
			  return false;
		  }
    }
    
    if( ncb_send( m_pNCB, numBytes, (char *) buf, 10) != 0 ) 
    {
      WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR,  "MBP: Send error: %d occurred.\n", m_pNCB->NCB_RETCODE );
      // close path, exit on error
      if (m_config.leaveMasterPortOpen == false)
      {
        ret_val = ncb_close( m_pNCB );
        m_pNCB = NULL;
      }
      return false;
		}
    m_bBusy = true;
  }
  
  return(true);
}

// Can we transmit now ?
TMWTYPES_MILLISECONDS WinMBPChannel::isTransmitReady(void)
{
  if (m_config.mode == WINMBP_MODE_SERVER)
  {
    if (m_bBusy)
      return 0;
    else
      return 10;
  }
  else
  {
    if (!m_bBusy)
      return 0;
    else
      return 10;
  }
}

// Get current channel status
const char   *WinMBPChannel::getStatus(void)
{
  strcpy(m_status, "Unknown");
  return(m_status);
}

/* function: Create */
WinMBPChannel *WinMBPChannel::Create(
  const void *pConfig, 
  TMWTARG_CONFIG *pTmwTargConfig)
{
	// Load the required DLL
	HINSTANCE hinst = LoadLibrary(_T("netlib.dll"));

	if (hinst == NULL)
	{
		// Get the error text from the system.  This is here for debugging
		// purposes only.  At this time, nothing is done with this.
		DWORD errCode = ::GetLastError();
		TCHAR* pErrText = NULL;
		DWORD language = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

		DWORD fmtStat = ::FormatMessage((FORMAT_MESSAGE_ALLOCATE_BUFFER | 
										 FORMAT_MESSAGE_IGNORE_INSERTS | 
										 FORMAT_MESSAGE_FROM_SYSTEM), 
										NULL,
										errCode,
										language,
										(LPTSTR)&pErrText,
										0,
										NULL);
    if (fmtStat == 0)
    {
      //TMWTYPES_ULONG lastError = GetLastError();
      // call the callback and pass up the error code
    }

		::LocalFree(pErrText);		// Free the error text memory

		return NULL;	// We will return NULL to the caller
	}
  const WINMBP_CONFIG *pMBPConfig = (WINMBP_CONFIG *)pConfig;
  WinMBPChannel *channel = new WinMBPChannel(*pMBPConfig, pTmwTargConfig);
#if TMWTARG_SUPPORT_CHANNEL_THREAD
  if (pMBPConfig->mode == WINMBP_MODE_CLIENT)
  {
    channel->m_chanThreadCheckMask = CT_CHECK_CONNECT;
    channel->ChannelThreadStart();
  }
#endif
  return channel;
}

/* function: winmbp_deleteChannel */
void WinMBPChannel::deleteWinIoChannel()
{
  closeWinIoChannel();
}

bool WinMBPChannel::modifyWinIoChannel(const void *pUserConfig)
{
  pUserConfig = pUserConfig;
  return false;
}

/* function: winmbp_isChannelOpen */
bool WinMBPChannel::isChannelOpen()
{
  return m_bChannelOpen;
}

/* function: winmbp_openChannel */
bool WinMBPChannel::openWinIoChannel(TMWTARG_CHANNEL_RECEIVE_CBK_FUNC pCallbackFunc, 
                                         TMWTARG_CHECK_ADDRESS_FUNC pCheckAddrCallbackFunc, 
                                         void *pCallbackParam, WINIO_OPEN_MODE_ENUM openMode)
{
  openMode = openMode;
  m_pRecvDataFunc = pCallbackFunc;
  m_pCheckAddrFunc = pCheckAddrCallbackFunc;
  m_pChanContextCBData = pCallbackParam;
  return(open());
}

/* function: winmbp_closeChannel */
void WinMBPChannel::closeWinIoChannel()
{
  close();
}

/* function: winmbp_getChannelName */
const char   *WinMBPChannel::getChannelName()
{
  return getName();
}

const char   *WinMBPChannel::getChannelInfo()
{
  return getInfo();
}

const char   *WinMBPChannel::getChannelStatus()
{
  return getStatus();
}

/* function: winmbp_getTransmitReady */
TMWTYPES_MILLISECONDS WinMBPChannel::getTransmitReady()
{
  return isTransmitReady();
}

/* function: winmbp_receive */
TMWTYPES_USHORT WinMBPChannel::receiveOnChannel(
  TMWTYPES_UCHAR *buf, 
  TMWTYPES_USHORT maxBytes, 
  TMWTYPES_MILLISECONDS maxTimeout,
  bool  *timeoutOccured)
{
  maxTimeout = maxTimeout;
  *timeoutOccured = false;
  return receive(buf, maxBytes);
}

/* function: winmbp_transmit */
bool WinMBPChannel::transmitOnChannel(
  TMWTYPES_UCHAR *buf, 
  TMWTYPES_USHORT numBytes)
{
  return transmit(buf, numBytes);
}
#endif
