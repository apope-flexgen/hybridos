/*****************************************************************************/
/* Triangle MicroWorks, Inc.                        Copyright (c) 1997-2022  */
/*****************************************************************************/
/*                                                                           */
/* This file is the property of:                                             */
/*                                                                           */
/*                       Triangle MicroWorks, Inc.                           */
/*                      Raleigh, North Carolina USA                          */
/*                       www.TriangleMicroWorks.com                          */
/* (919) 870 - 6615                                                          */
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

// WinIoTarg.cpp : Defines the entry point for the DLL application.
//
/* file: WinIoTarg.cpp
* description: Implementation of generic Windows I/O Target interface. This
*  file dispatches the I/O request to one of the supported low level I/O
*  implementations.
*
*  The interface from the source code libraries is through a set of
*  functions that hide the class interface. These functions are defined 
*  in the WinIoTarg.h header file and implemented in this file.
*/

#include "StdAfx.h"
#include "tmwscl/utils/tmwcnfg.h"

#ifdef _DEBUG
//#ifdef TMW_PRIVATE_VLD
//#pragma comment(lib, "../thirdPartyCode/Visual Leak Detector/lib/Win32/vld.lib")
//#include "../thirdpartycode/Visual Leak Detector/include/vld.h"
//#endif
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "WinIoTarg/include/WinIoTarg.h"
#include "WinIoTarg/include/WinIoTargDefs.h"

#include "WinIoTarg/WinIoBaseTime.h"
#include "WinIoTarg/WinIoSystemTime.h"
#include "WinIoTarg/WinIoSimulatedTime.h"

#include "WinIoTarg/WinTCPChannel.h"

#if TMWTARG_SUPPORT_TLS && TMWCNFG_USE_OPENSSL
#include "WinIoTarg/WinTLSChannel.h"
#endif

#include "WinIoTarg/Win232Channel.h"
#if TMW_SUPPORT_MONITOR
#include "WinIoTarg/WinMonChannel.h"
#endif

#if WIN_MBPLUS_SUPPORT
#include "WinIoTarg/WinMBPChannel.h"
#endif

#if WIN_MODEM_SUPPORT
#include "WinIoTarg/WinModemPoolChannel.h"
#include "WinIoTarg/WinModemPoolManager.h"
#include "WinIoTarg/WinModemPool.h"
#include "WinIoTarg/WinModem.h"
#endif
#include "tmwscl/utils/tmwtarg.h"

static TMWTYPES_BOOL m_bWinSockInitialized = false;

// global data
WinIoTargProtoAnaLogFunType WinIoTargProtoAnaLogFun = WINIOTARG_NULL;
static bool WinIoTargProtoAnaLogEnable = true;

// the application wide shared time object
static WinIoBaseTime *g_pWinIoTimeObject = WINIOTARG_NULL;
static AutoCriticalSectionWinTarg timeObjectLock;

// has winpcap been initialized
static TMWTYPES_BOOL pcapNotInitialized = true;

TMWTYPES_BOOL isWinSockInitialized(void)
{
  return m_bWinSockInitialized == true ? TRUE : FALSE;
}

/*!
*   global  DllMain
*   <TODO: insert function description here>
* 
*   @param  hModule HANDLE     <TODO: insert parameter description here>
*   @param  ul_reason_for_call TMWTYPES_ULONG     <TODO: insert parameter description here>
*   @param  lpReserved LPVOID     <TODO: insert parameter description here>
* 
*   @return TMWTYPES_BOOL <TODO: insert return value description here>
* 
*   @remarks <TODO: insert remarks here>
*/
TMWTYPES_BOOL APIENTRY DllMain(HANDLE hModule, 
  TMWTYPES_ULONG  ul_reason_for_call, 
  LPVOID lpReserved
  )
{
  hModule=hModule;
  lpReserved=lpReserved;

  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH:
    {
      ::OutputDebugString(_T("WinIoTarg.DLL Initializing!\n"));

      struct WSAData wsaData;


      // Initialize winsock 2.0
      if(WSAStartup(MAKEWORD(2,0), &wsaData ) == 0)
      {
        m_bWinSockInitialized = true;
      }
      // Make sure version 2.0 is supported
      if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 0)
      {
        WSACleanup();
        m_bWinSockInitialized = false;
      }
    }
    break;
  case DLL_PROCESS_DETACH:
    {
    ::OutputDebugString(_T("WinIoTarg.DLL Terminating!\n"));

#if WIN_MODEM_SUPPORT
      WinModemPoolManager_destroy();
#endif
      WSACleanup();
    }
    break;

  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
    break;
  }
  return TRUE;
}

/**********************************************************************************\
Function :			WinIoTarg_initConfig
Description : [none]	
Return :			void	-	
Parameters :
WINIO_CONFIG *pConfig	-	
Note : [none]
\**********************************************************************************/
void tmwtargio_initConfig(WINIO_CONFIG *pConfig)
{
  //WinIoTarg_initConfig_start_mark
  //WinIoTarg_DebugPrintf("WinIoTarg_initConfig called\n");  // an example of TMWDebugPrintf usage

  pConfig->type = WINIO_TYPE_232;

  CriticalSectionLockWinTarg lock(timeObjectLock);
  if (g_pWinIoTimeObject == WINIOTARG_NULL)
  {
    WinIoTarg_setTimeMode(WINIO_TIME_MODE_SIMULATED);
  }

  memset(&pConfig->targ232.chnlName[0], 0, WINIOTARG_STR_LEN);
  strcpy(pConfig->targ232.baudRate, "9600");
  pConfig->targ232.numDataBits = WIN232_DATA_BITS_8;
  pConfig->targ232.numStopBits = WIN232_STOP_BITS_1;
  pConfig->targ232.parity = TMWTARG232_PARITY_NONE;
  strcpy(pConfig->targ232.portName, "COM1");
  pConfig->targ232.portMode = TMWTARG232_MODE_NONE;

  // the next 2 are only used in WIN232_MODE_HARDWARE
  pConfig->targ232.dtrMode = TMWTARG232_DTR_ENABLE;
  pConfig->targ232.rtsMode = TMWTARG232_RTS_DISABLE;

  pConfig->targ232.bModbusRTU = false;
  pConfig->targ232.disabled = false;

  // Polled or Event Driven Receive data mode
  pConfig->targ232.polledMode = TMWDEFS_TRUE;

  memset(&pConfig->targTCP.chnlName[0],0,WINIOTARG_STR_LEN);
  pConfig->targTCP.mode = TMWTARGTCP_MODE_CLIENT;
  pConfig->targTCP.ipVersion = TMWTARG_IPV4;
  strcpy(pConfig->targTCP.ipAddress, TMWTARG_ADDR_IPV4_LOOPBACK);
  strcpy(pConfig->targTCP.localIpAddress, TMWTARG_ADDR_IPV4_ANY);
  strcpy(pConfig->targTCP.udpBroadcastAddress, TMWTARG_ADDR_IPV4_UDP_BCAST);
  pConfig->targTCP.ipPort = 2404;
  pConfig->targTCP.ipConnectTimeout = 1000;
  pConfig->targTCP.disconnectOnNewSyn = true;

  // To support DNP3 Specification IP Networking 
  // local UDP port should default to 20000 for DNP, we will
  // initialize it to NONE, so IEC and Modbus protocols don't
  // open UDP port.
  pConfig->targTCP.localUDPPort       = WINTCP_UDP_PORT_NONE; 
  pConfig->targTCP.role               = TMWTARGTCP_ROLE_MASTER;
  pConfig->targTCP.validateUDPAddress = TMWDEFS_TRUE;
  pConfig->targTCP.dualEndPointIpPort = 20000;
  pConfig->targTCP.destUDPPort        = 20000;
  pConfig->targTCP.initUnsolUDPPort   = 20000;
  
  // TLS support 
  pConfig->targTCP.useTLS             = TMWDEFS_FALSE;

  // Polled or Event Driven Receive TCP/UDP data mode
  pConfig->targTCP.polledMode         = TMWDEFS_TRUE;

#if TMWTARG_SUPPORT_TLS && TMWCNFG_USE_OPENSSL
  pConfig->targTCP.nCaVerifyDepth             = 1;
  pConfig->targTCP.nTlsRenegotiationSeconds   = 0;
  pConfig->targTCP.nTlsRenegotiationCount     = 0;
  pConfig->targTCP.nTlsRenegotiationMsTimeout = 0;
  pConfig->targTCP.tlsHandshakeMsTimeout      = 2000; /* 2 seconds */

  strncpy(pConfig->targTCP.caCrlFileName, "../TMWCertificates/ca_public/tmw_sample_ca_certificate_revocation_list.pem", TMWTARG_CRYPTO_ID_LEN-1);
  strncpy(pConfig->targTCP.caFileName, "../TMWCertificates/ca_public/tmw_sample_ca_rsa_public_certificate.pem", TMWTARG_CRYPTO_ID_LEN-1);
  strncpy(pConfig->targTCP.dhFileName, "", 1);
  strncpy(pConfig->targTCP.tlsCommonName, "TLS", TMWTARG_CRYPTO_TLS_NAME_LEN-1);

  strncpy(pConfig->targTCP.tlsRsaCertificateId, "../TMWCertificates/client_user/tmw_sample_tls_rsa_public_cert.pem", TMWTARG_CRYPTO_ID_LEN-1);
  strncpy(pConfig->targTCP.tlsRsaPrivateKeyFile, "../TMWCertificates/client_user/tmw_sample_tls_rsa_private_key.pem", TMWTARG_CRYPTO_ID_LEN-1);
  strncpy(pConfig->targTCP.tlsRsaPrivateKeyPassPhrase, "triangle", TMWTARG_CRYPTO_PASSPHRASE_LEN -1);

  strncpy(pConfig->targTCP.tlsDsaPrivateKeyFile, "", 1);
  strncpy(pConfig->targTCP.tlsDsaPrivateKeyPassPhrase, "", 1);
  strncpy(pConfig->targTCP.tlsDsaCertificateId, "", 1);
#endif

  memset(pConfig->winMBP.chnlName, 0, WINIOTARG_STR_LEN);
  pConfig->winMBP.mode = WINMBP_MODE_CLIENT;
  pConfig->winMBP.leaveMasterPortOpen = true;
  strcpy(pConfig->winMBP.routePath, "1.0.0.0.0");
  pConfig->winMBP.slavePath = 1;
  pConfig->winMBP.cardNum = 0;

  pConfig->winMON.monitorMode = TMWDEFS_FALSE;
  pConfig->winMON.interfaceNum = 0;
  pConfig->winMON.interfaceDescr[0] = 0;
  strcpy(pConfig->winMON.interfaceDescr, "N/A");
  pConfig->winMON.ipV6 = TMWDEFS_FALSE;

  pConfig->connectDelay = 0;
  pConfig->connectRetry = TMWDEFS_SECONDS(1);
  pConfig->forceDisconnected  = TMWDEFS_FALSE;  

  //Set this to true since that was the default behavior before this config option was added.
  //pConfig->useConnectorThread = TMWDEFS_TRUE;
  //WinIoTarg_initConfig_end_mark
}

void WinIoTarg_getIdleCallBack(void *pContext, 
  TMWCHNL_IDLE_CALLBACK *pCallBackFun, void **pCallBackParam)
{
#if WIN_MODEM_SUPPORT
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  if (pIOContext->type == WINIO_TYPE_MODEM_POOL_CHANNEL)
  {
    *pCallBackFun = WinModemPoolChannel::IdleCallback;
    *pCallBackParam = pIOContext->pWinIoInterface;
  }
  else
#else
  TMWTARG_UNUSED_PARAM(pContext);
#endif
  {
    *pCallBackFun   = WINIOTARG_NULL;
    *pCallBackParam = WINIOTARG_NULL;
  }
}


/**********************************************************************************\
Function :			WinIoTarg_initBinFileTargValues
Description : initializes a	TMWTARG_BINFILE_VALS struct.  Call before sending to 
              dnpbinconfig_ReadBinaryConfigFile()
Return :			true on success	-	
Parameters :
TMWTARG_BINFILE_VALS *pBinFileTargValues	-	
Note : [none]
\**********************************************************************************/
  TMWTYPES_BOOL WinIoTarg_initBinFileTargValues(TMWTARG_BINFILE_VALS *pBinFileTargValues)
  {
    strcpy(pBinFileTargValues->allowedConnIpList, "*.*.*.*");

    strcpy(pBinFileTargValues->baudRate, "9600");

    pBinFileTargValues->destUdpPort = 20000;

    pBinFileTargValues->destUdpPortForResponses = 20000;

    pBinFileTargValues->destUdpPortForUnsol = 20000;

    strcpy(pBinFileTargValues->gatewayIp, "*.*.*.*");

    strcpy(pBinFileTargValues->ipAddress, "127.0.0.1");

    strcpy(pBinFileTargValues->ipPortName, "");

    pBinFileTargValues->localUdpPort = 20000;

    strcpy(pBinFileTargValues->serialPortName, "COM1");

    strcpy(pBinFileTargValues->subnetMask, "255.255.255.1");

    pBinFileTargValues->supportsSerialConn = TMWDEFS_FALSE;

    pBinFileTargValues->supportsTCPConn = TMWDEFS_TRUE;

    pBinFileTargValues->tcpListenPort = 2404;

    pBinFileTargValues->tcpListenPortOfRemote = 20000;

    return TMWDEFS_TRUE;
  }


/**********************************************************************************\
Function :			WinIoTarg_applyBinFileTargValues
Description : copies values from a TMWTARG_BINFILE_VALS struct to the target layer
Return :			true on success	
Parameters :
WINIO_CONFIG *pConfig	-	target layer values
TMWTARG_BINFILE_VALS *pBinFileTargValues - target related values from a binary config file
Note : [none]
\**********************************************************************************/
  TMWTYPES_BOOL WinIoTarg_applyBinFileTargValues(WINIO_CONFIG *pIoConfig, 
    TMWTARG_BINFILE_VALS *pBinFileTargValues,
    TMWTYPES_BOOL *pIsChannelSerial)
  {

    TMWTYPES_BOOL outstation = TMWDEFS_FALSE;
    TMWTYPES_BOOL master = TMWDEFS_FALSE;
    TMWTYPES_BOOL masterForOutstation = TMWDEFS_FALSE;


    /*determine what is being configured*/
    if(pBinFileTargValues->sessionIsOutstation) /*session is an outstation*/
    {
      if(pBinFileTargValues->binFileIsOutstation) /*target values are from an outstation bin config file*/
        outstation = TMWDEFS_TRUE;
      else /*target values are from a master bin config file*/
        return TMWDEFS_FALSE;  /*cannot configure an outstation with a master binary config file*/
    }
    else  /*session is a master*/
      if(pBinFileTargValues->binFileIsOutstation) /*target values are from an outstation bin config file*/
        masterForOutstation = TMWDEFS_TRUE;  /*configure the master to talk to the outstation*/
      else
        master = TMWDEFS_TRUE;  /*configure the master*/


    /*these values are only valid for an outstation*/
    if(outstation)
    {
      if(pBinFileTargValues->useTcpListenPort1_3_8)
        pIoConfig->targTCP.ipPort = pBinFileTargValues->tcpListenPort;

      if(pBinFileTargValues->useDestUdpPortForUnsol1_3_13)
        pIoConfig->targTCP.initUnsolUDPPort = pBinFileTargValues->destUdpPortForUnsol;

      if(pBinFileTargValues->useDestUdpPortForResponses1_3_14)
      {
        if(pBinFileTargValues->useSourcePortNumberForResponses)
          pIoConfig->targTCP.destUDPPort = TMWTARG_UDP_PORT_SRC;
        else
          pIoConfig->targTCP.destUDPPort = pBinFileTargValues->destUdpPortForResponses;
      }
    }

    /*these values are only valid for a master*/
    if(master)
    {
      if(pBinFileTargValues->useDestUpdPort1_3_12)
        pIoConfig->targTCP.destUDPPort = pBinFileTargValues->destUdpPort;

      if((pBinFileTargValues->useTcpListenPortOfRemote1_3_9) 
        && (!pBinFileTargValues->endpointIsTcpDual))
        pIoConfig->targTCP.ipPort = pBinFileTargValues->tcpListenPortOfRemote;
    }


    /*these items are valid for both a master and an outstation*/
    if(master || outstation)
    {

      if(pBinFileTargValues->useIpPortName1_3_1)
        if(strlen(pBinFileTargValues->ipPortName) < WINIOTARG_STR_LEN)
          strcpy(pIoConfig->targTCP.chnlName, pBinFileTargValues->ipPortName);

      if(pBinFileTargValues->useIpAddress1_3_3)
        strcpy(pIoConfig->targTCP.localIpAddress, pBinFileTargValues->ipAddress);

      if(pBinFileTargValues->useAllowedConnIpList1_3_7)
        strcpy(pIoConfig->targTCP.ipAddress, pBinFileTargValues->allowedConnIpList);

      if(pBinFileTargValues->useSupportedComm1_1_13)
      {
        if(pBinFileTargValues->supportsSerialConn == TMWDEFS_TRUE) 
        {
          pIoConfig->type = WINIO_TYPE_232;
          pIoConfig->targ232.disabled = TMWDEFS_FALSE;
          strcpy(pIoConfig->targ232.baudRate, pBinFileTargValues->baudRate);
          strcpy(pIoConfig->targ232.portName, pBinFileTargValues->serialPortName);
        }
       
        if(pBinFileTargValues->supportsTCPConn == TMWDEFS_TRUE) 
        {
          pIoConfig->type = WINIO_TYPE_UDP_TCP;
        }
      }

      if(pBinFileTargValues->useEndpoint1_3_2)
      {
        if(pBinFileTargValues->endpointIsTcpInitiating)
          pIoConfig->targTCP.mode = TMWTARGTCP_MODE_CLIENT;

        else if(pBinFileTargValues->endpointIsTcpListening)
          pIoConfig->targTCP.mode = TMWTARGTCP_MODE_SERVER;

        else if(pBinFileTargValues->endpointIsTcpDual)
          pIoConfig->targTCP.mode = TMWTARGTCP_MODE_DUAL_ENDPOINT;

        /* If this is UDP ONLY */
        else if(pBinFileTargValues->endpointIsUDPDatagram)
          pIoConfig->targTCP.mode = TMWTARGTCP_MODE_UDP;
      }
      
      if (pBinFileTargValues->endpointIsTcpDual)
      {
        if(pBinFileTargValues->useTcpListenPort1_3_8)
          pIoConfig->targTCP.ipPort = pBinFileTargValues->tcpListenPort;
        
        if(pBinFileTargValues->useTcpListenPortOfRemote1_3_9)
          pIoConfig->targTCP.dualEndPointIpPort = pBinFileTargValues->tcpListenPortOfRemote;
      }

      if(pBinFileTargValues->useLocalUdpPort1_3_11)
        pIoConfig->targTCP.localUDPPort = pBinFileTargValues->localUdpPort;
    }

    /* these items are valid when configuring a master for a specific outstation */
    else if(masterForOutstation)
    {
      if(pBinFileTargValues->useSupportedComm1_1_13)
      {
        if(pBinFileTargValues->supportsSerialConn == TMWDEFS_TRUE) 
        {
          pIoConfig->type = WINIO_TYPE_232;
          pIoConfig->targ232.disabled = TMWDEFS_FALSE;
          strcpy(pIoConfig->targ232.baudRate, pBinFileTargValues->baudRate);
          *pIsChannelSerial = TMWDEFS_TRUE;
        }
       
        if(pBinFileTargValues->supportsTCPConn == TMWDEFS_TRUE) 
        {
          pIoConfig->type = WINIO_TYPE_UDP_TCP;
          *pIsChannelSerial = TMWDEFS_FALSE;
        }
      }

      if(pBinFileTargValues->useEndpoint1_3_2)
      {
        /* if outstation is listening, master is the client */
        if(pBinFileTargValues->endpointIsTcpListening)
        {
          pIoConfig->targTCP.mode = TMWTARGTCP_MODE_CLIENT;
          
          /* master would connect to outstation listen port */
          if(pBinFileTargValues->useTcpListenPort1_3_8)
            pIoConfig->targTCP.ipPort = pBinFileTargValues->tcpListenPort;
        }
        else if(pBinFileTargValues->endpointIsTcpDual)
        {
          pIoConfig->targTCP.mode = TMWTARGTCP_MODE_DUAL_ENDPOINT;

          /* master connects to outstation listen port */
          if(pBinFileTargValues->useTcpListenPort1_3_8)
            pIoConfig->targTCP.dualEndPointIpPort = pBinFileTargValues->tcpListenPort;
        
          /* master listens on outstations listenPortOfRemote */
          if(pBinFileTargValues->useTcpListenPortOfRemote1_3_9)
            pIoConfig->targTCP.ipPort = pBinFileTargValues->tcpListenPortOfRemote; 
        }

        /* If this is UDP ONLY */
        else if(pBinFileTargValues->endpointIsUDPDatagram)
          pIoConfig->targTCP.mode = TMWTARGTCP_MODE_UDP;
      }

      /* local address of outstation, would be destination ipAddress. */
      if(pBinFileTargValues->useIpAddress1_3_3)
        strcpy(pIoConfig->targTCP.ipAddress, pBinFileTargValues->ipAddress);
      
      /* outstation local UDP port would be master destination port */
      if(pBinFileTargValues->useLocalUdpPort1_3_11)
        pIoConfig->targTCP.destUDPPort = pBinFileTargValues->localUdpPort; 

      /* master local UDP port would be outstation dest UDP port for unsol */
      if(pBinFileTargValues->useDestUdpPortForUnsol1_3_13)
        pIoConfig->targTCP.localUDPPort = pBinFileTargValues->destUdpPortForUnsol;
      
      /* master local UDP port would be outstation dest UDP port, if it does not say to use source port. */
      if((pBinFileTargValues->useDestUdpPortForResponses1_3_14)
        && (pBinFileTargValues->destUdpPortForResponses != TMWTARG_UDP_PORT_SRC))
      {
        pIoConfig->targTCP.localUDPPort = pBinFileTargValues->destUdpPortForResponses;
      }
    }

    return TMWDEFS_TRUE;
  }


/**********************************************************************************\
Function :			WinIoTarg_Create
Description : [none]	
Return :			void *	-	
Parameters :
const void *pUserConfig	-	
TMWTARG_CONFIG *pTmwTargConfig	-	
Note : [none]
\**********************************************************************************/
void *WinIoTarg_Create(const void *pUserConfig,TMWTARG_CONFIG *pTmwTargConfig)
{

  if (pUserConfig == NULL)
  {
    return NULL;
  }

  WINIO_CONFIG *pIOConfig = (WINIO_CONFIG *)pUserConfig;
  WINIO_CONTEXT *pContext = new WINIO_CONTEXT();

  pContext->type = pIOConfig->type;
  pContext->connectDelay = pIOConfig->connectDelay;
  pContext->connectRetry = pIOConfig->connectRetry;

  /* Ensure that this is not set to zero to avoid a busy loop */
  if (pContext->connectRetry == 0)
    pContext->connectRetry = 500;

  switch (pIOConfig->type)
  {
#if WIN_MODEM_SUPPORT
  case WINIO_TYPE_MODEM:
    pContext->pWinIoInterface = (void*)WinModem::Create(&pIOConfig->winModem, pTmwTargConfig);
    break;

  case WINIO_TYPE_MODEM_POOL:
    pContext->pWinIoInterface = (void*)WinModemPool::Create(&pIOConfig->winModemPool);
    break;

  case WINIO_TYPE_MODEM_POOL_CHANNEL:
    pContext->pWinIoInterface = (void*)WinModemPoolChannel::Create(&pIOConfig->winModemPoolChannel, pTmwTargConfig);
    break;
#endif

  case WINIO_TYPE_232:
    pContext->pWinIoInterface = (void*)Win232Channel::Create(&pIOConfig->targ232, pTmwTargConfig);
    break;

  case WINIO_TYPE_TCP:
  case WINIO_TYPE_UDP_TCP:
    if (isWinSockInitialized() != TRUE)
    {
      pContext->pWinIoInterface = NULL;
    }
    else
    {
#if TMW_SUPPORT_MONITOR
      if(pIOConfig->winMON.monitorMode == TMWDEFS_TRUE)
      {
        pContext->pWinIoInterface = (void*)WinMonChannel::Create(&pIOConfig->winMON, pTmwTargConfig);
        break;
      }
#endif

#if TMWTARG_SUPPORT_TLS && TMWCNFG_USE_OPENSSL
      if(pIOConfig->targTCP.useTLS == TMWDEFS_TRUE)
      {
        pContext->pWinIoInterface = (void*)WinTLSChannel::Create(&pIOConfig->targTCP, pTmwTargConfig);
        break;
      }
#endif

      pContext->pWinIoInterface = (void*)WinTCPChannel::Create(&pIOConfig->targTCP, pTmwTargConfig);
    }
    break;

#if WIN_MBPLUS_SUPPORT
  case WINIO_TYPE_MBP:
    pContext->pWinIoInterface = (void*)WinMBPChannel::Create(&pIOConfig->winMBP, pTmwTargConfig);
    break;
#endif

#if TMW_SUPPORT_MONITOR
  case WINIO_TYPE_MON:
    pContext->pWinIoInterface = (void*)WinMonChannel::Create(&pIOConfig->winMON, pTmwTargConfig);
    break;
#endif
  default:
    return WINIOTARG_NULL;
  }

  if (pContext->pWinIoInterface == WINIOTARG_NULL)
  {
    delete pContext;
    return WINIOTARG_NULL;
  }

  ((WinIoInterface *)pContext->pWinIoInterface)->pWinIoContext = pContext;

  ((WinIoInterface *)pContext->pWinIoInterface)->setForceDisconnected(pIOConfig->forceDisconnected);

  pContext->pTxCallback = WINIOTARG_NULL;
  pContext->pTxCallbackParam = WINIOTARG_NULL;

  pContext->pRxCallback = WINIOTARG_NULL;
  pContext->pRxCallbackParam = WINIOTARG_NULL;

  return pContext;
}

/**********************************************************************************\
Function :			WinIoTarg_Modify
Description : [none]	
Return :			void *	-	
Parameters :
void *pContext	-	
const void *pUserConfig	-	
Note : [none]
\**********************************************************************************/
TMWTYPES_BOOL WinIoTarg_Modify(void *pContext, const void *pUserConfig)
{
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  WinIoInterface *pWinIoInterface = (WinIoInterface*)(pIOContext->pWinIoInterface);
  return pWinIoInterface->modifyWinIoChannel(pUserConfig);
}

/**********************************************************************************\
Function :			WinIoTarg_Destroy
Description : [none]	
Return :			void	-	
Parameters :
void *pContext	-	
Note : [none]
\**********************************************************************************/
void WinIoTarg_Destroy(void *pContext)
{
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  WinIoInterface *pWinIoInterface = (WinIoInterface*)(pIOContext->pWinIoInterface);

  delete pWinIoInterface;
  delete pIOContext;
}

/**********************************************************************************\
Function :			WinIoTarg_Exit
Description : [none]	
Return :			void	-	
Parameters [none]
Note : [none]
\**********************************************************************************/
WINIOTARG_API void WinIoTarg_Exit()
{
#if TMWTARG_SUPPORT_TLS && TMWCNFG_USE_OPENSSL
  // Clean up OpenSSL to reduce memory leak diagnostics.
  WinTLSChannel::TLSTerminateSslLibrary();
#endif
}

/**********************************************************************************\
Function :			WinIoTarg_isChannelOpen
Description : [none]	
Return :			TMWTYPES_BOOL	-	
Parameters :
void *pContext	-	
Note : [none]
\**********************************************************************************/
TMWTYPES_BOOL WinIoTarg_isChannelOpen(void *pContext)
{
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  WinIoInterface *pWinIoInterface = (WinIoInterface*)(pIOContext->pWinIoInterface);

  return pWinIoInterface->isChannelOpen();
}

/**********************************************************************************\
Function :			WinIoTarg_openChannel
Description : [none]	
Return :			TMWTYPES_BOOL	-	
Parameters :
void *pContext	-	
Note : [none]
\**********************************************************************************/
TMWTYPES_BOOL   WinIoTarg_openChannel(void *pContext,
  TMWTARG_CHANNEL_RECEIVE_CBK_FUNC pReceiveCallbackFunc,
  TMWTARG_CHECK_ADDRESS_FUNC pCheckAddrCallbackFunc,
  void *pCallbackParam)
{
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  WinIoInterface *pWinIoInterface = (WinIoInterface*)(pIOContext->pWinIoInterface);

  if(pWinIoInterface->isForceDisconnected())
  {
    return false;
  }

  // See if already made connection 
  if (pWinIoInterface->isChannelOpen() == true)
  { 
    return true;
  }

  pWinIoInterface->setDataCallbacks(pReceiveCallbackFunc, pCheckAddrCallbackFunc, pCallbackParam);
  pWinIoInterface->openChannel();
  if (pWinIoInterface->ChannelThreadConnecting())
  {
    return false;
  }
  else
  { 
    // Servers do not use connector thread for example
    return(pWinIoInterface->openWinIoChannel(pReceiveCallbackFunc, pCheckAddrCallbackFunc, pCallbackParam, WINIO_OPEN_MODE_NONE));
  }
}

/**********************************************************************************\
Function :			WinIoTarg_closeChannel
Description : [none]	
Return :			void	-	
Parameters :
void *pContext	-	
Note : [none]
\**********************************************************************************/
void WinIoTarg_closeChannel(void *pContext)
{
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  WinIoInterface *pWinIoInterface = (WinIoInterface*)(pIOContext->pWinIoInterface);
  pWinIoInterface->closeWinIoChannel();
}


/**********************************************************************************\
Function :			WinIoTarg_resetChannel
Description : [none]	
Return :			TMWTYPES_BOOL	-	
Parameters :
void *pContext	-	
Note : [none]
\**********************************************************************************/
TMWTYPES_BOOL WinIoTarg_resetChannel(void *pContext)
{
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  WinIoInterface *pWinIoInterface = (WinIoInterface*)(pIOContext->pWinIoInterface);

  return pWinIoInterface->resetWinIoChannel();
}
/**********************************************************************************\
Function :			WinIoTarg_getChannelName
Description : [none]	
Return :			const char *	-	
Parameters :
void *pContext	-	
Note : [none]
\**********************************************************************************/
const char *WinIoTarg_getChannelName(void *pContext)
{
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  WinIoInterface *pWinIoInterface = (WinIoInterface*)(pIOContext->pWinIoInterface);

  if(pWinIoInterface == WINIOTARG_NULL)
    return("No interface");

  return pWinIoInterface->getChannelName();
}

/**********************************************************************************\
Function :			WinIoTarg_getChannelStatus
Description : [none]	
Return :			const char *	-	
Parameters :
void *pContext	-	
Note : [none]
\**********************************************************************************/
const char *WinIoTarg_getChannelStatus(void *pContext)
{
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  WinIoInterface *pWinIoInterface = (WinIoInterface*)(pIOContext->pWinIoInterface);

  return pWinIoInterface->getChannelStatus();
}

/**********************************************************************************\
Function :			WinIoTarg_getChannelInfo
Description : [none]	
Return :			const char *	-	
Parameters :
void *pContext	-	
Note : [none]
\**********************************************************************************/
const char *WinIoTarg_getChannelInfo(void *pContext)
{
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  WinIoInterface *pWinIoInterface = (WinIoInterface*)(pIOContext->pWinIoInterface);

  return pWinIoInterface->getChannelInfo();
}

/**********************************************************************************\
Function :			WinIoTarg_getTransmitReady
Description : [none]	
Return :			TMWTYPES_MILLISECONDS	-	
Parameters :
void *pContext	-	
Note : [none]
\**********************************************************************************/
TMWTYPES_MILLISECONDS WinIoTarg_getTransmitReady(void *pContext)
{
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  WinIoInterface *pWinIoInterface = (WinIoInterface*)(pIOContext->pWinIoInterface);

  return pWinIoInterface->getTransmitReady();
}

/* WinIoTarg_waitForInput
 *   Unsupported function for internal use.
 */ 
/**********************************************************************************\
Function :			WinIoTarg_waitForInput
Description : Unsupported function for internal use.	
Return :	[none]	   	-	
Parameters :
void *pContext	-	
Note : [none]
\**********************************************************************************/
TMWTYPES_BOOL WinIoTarg_waitForInput(void *pContext)
{
	WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
	WinIoInterface *pWinIoInterface = (WinIoInterface*)(pIOContext->pWinIoInterface);
	return pWinIoInterface->waitForInput();
}
 
/**********************************************************************************\
Function :			WinIoTarg_StopThreads
Description : Stop Check For Input and Connector processing so that channel can be safely deleted.
Return :			  - void	
Parameters :
void *pContext	-	
Note : [none]
\**********************************************************************************/
void WinIoTarg_StopThreads(void *pContext)
{
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  WinIoInterface *pWinIoInterface = (WinIoInterface*)(pIOContext->pWinIoInterface);

  pWinIoInterface->closeChannel();

#if TMWTARG_SUPPORT_CHANNEL_THREAD
  pWinIoInterface->ChannelThreadStop();
#endif

  WinIoTarg_closeChannel(pIOContext);
}

/**********************************************************************************\
Function :			WinIoTarg_receive
Description : [none]	
Return :			TMWTYPES_USHORT	-	
Parameters :
void *pContext	-	
TMWTYPES_UCHAR *pBuff	-	
TMWTYPES_USHORT maxBytes	-	
TMWTYPES_MILLISECONDS interCharacterTimeout	-	
TMWTYPES_BOOL *timeoutOccured	-	
Note : [none]
\**********************************************************************************/
TMWTYPES_USHORT WinIoTarg_receive(void *pContext, 
  TMWTYPES_UCHAR *pBuff, 
  TMWTYPES_USHORT maxBytes, 
  TMWTYPES_MILLISECONDS maxTimeout, 
  TMWTYPES_BOOL  *pTimeoutOccured)
{
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  WinIoInterface *pWinIoInterface = (WinIoInterface*)(pIOContext->pWinIoInterface);
  TMWTYPES_USHORT numBytes = 0;

  TMWTYPES_BOOL bTimeoutOccured = *pTimeoutOccured != 0 ? true : false;
  numBytes = pWinIoInterface->receiveOnChannel(pBuff, maxBytes, maxTimeout, &bTimeoutOccured);
  *pTimeoutOccured = bTimeoutOccured  == true ? true : false;

  if ((numBytes > 0) && (pIOContext->pRxCallback != WINIOTARG_NULL))
  {
    TMWTYPES_USHORT newNumBytes = numBytes;

    if (!pIOContext->pRxCallback(pContext, pIOContext->pRxCallbackParam, numBytes, maxBytes, &newNumBytes, pBuff))
      return false;

    numBytes = newNumBytes;
  }

  return numBytes;
}


/**********************************************************************************\
Function :			WinIoTarg_transmit
Description : [none]	
Return :			TMWTYPES_BOOL	-	
Parameters :
void *pContext	-	
TMWTYPES_UCHAR *pBuff	-	
TMWTYPES_USHORT numBytes	-	
Note : [none]
\**********************************************************************************/
TMWTYPES_BOOL WinIoTarg_transmit(void *pContext, TMWTYPES_UCHAR *pBuff, TMWTYPES_USHORT numBytes)
{
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  WinIoInterface *pWinIoInterface = (WinIoInterface*)(pIOContext->pWinIoInterface);

  if (pIOContext->pTxCallback != WINIOTARG_NULL)
  {
    TMWTYPES_USHORT newNumBytes = numBytes;

    if (!pIOContext->pTxCallback(pContext, pIOContext->pTxCallbackParam, numBytes, numBytes, &newNumBytes, pBuff))
      return true;

    if(newNumBytes == 0)
      return true;

    numBytes = newNumBytes;
  }

  return pWinIoInterface->transmitOnChannel(pBuff, numBytes);
}

/**********************************************************************************\
Function :			WinIoTarg_transmitUDP
Description : [none]	
Return :			TMWTYPES_BOOL	-	
Parameters :
void *pContext	-	
TMWTYPES_UCHAR UDPPort	-	 A define that indicates the remote UDP port to
transmit to. 
TMWTARG_UDP_SEND       - Send to the remote port to be used for 
requests or responses
TMWTARG_UDP_SEND_UNSOL - Send to the remote port to be used for   
unsolicited responses
TMWTARG_UDPONLY_BROADCAST - Send to the broadcast address when UDP ONLY 
is configured.
TMWTYPES_UCHAR *pBuff	-	
TMWTYPES_USHORT numBytes	-	
Note : [none]
\**********************************************************************************/
TMWTYPES_BOOL WinIoTarg_transmitUDP(void *pContext, 
  TMWTYPES_UCHAR UDPPort,
  TMWTYPES_UCHAR *pBuff, 
  TMWTYPES_USHORT numBytes)
{
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  WinIoInterface *pWinIoInterface = (WinIoInterface*)(pIOContext->pWinIoInterface);

  if (pIOContext->pTxCallback != WINIOTARG_NULL)
  {
    TMWTYPES_USHORT newNumBytes = numBytes;

    if (!pIOContext->pTxCallback(pContext, pIOContext->pTxCallbackParam, numBytes, 
      numBytes, &newNumBytes, pBuff))
    {
      return true;
    }
    numBytes = newNumBytes;
  }

  return pWinIoInterface->transmitUDP(UDPPort, pBuff, numBytes);
}

/**********************************************************************************\
Function :			WinIoTarg_setRTS
Description : [none]
Return :			TMWTYPES_BOOL	-
Parameters :
void *pContext	-
TMWTYPES_BOOL rts	-
Note : [none]
\**********************************************************************************/
TMWTYPES_BOOL WinIoTarg_setRTS(void *pContext, TMWTYPES_BOOL rts)
{
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  Win232Channel *pWin232Channel = (Win232Channel*)(pIOContext->pWinIoInterface);

  return pWin232Channel->SetRTS(rts);
}

/**********************************************************************************\
Function :			WinIoTarg_setDTR
Description : [none]
Return :			TMWTYPES_BOOL	-
Parameters :
void *pContext	-
TMWTYPES_BOOL dtr -
Note : [none]
\**********************************************************************************/
TMWTYPES_BOOL WinIoTarg_setDTR(void *pContext, TMWTYPES_BOOL dtr)
{
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  Win232Channel *pWin232Channel = (Win232Channel*)(pIOContext->pWinIoInterface);

  return pWin232Channel->SetDTR(dtr);
}

/**********************************************************************************\
Function :			WinIoTarg_setSyncTransmit
Description : [none]	
Return :			void	-	
Parameters :
TMWTYPES_BOOL flag	-	
Note : [none]
\**********************************************************************************/
void WinIoTarg_setSyncTransmit(TMWTYPES_BOOL flag)
{  
  TMWTARG_UNUSED_PARAM(flag);
  /* No longer used */ 
}

/**********************************************************************************\
Function :			WinIoTarg_setTxCallback
Description : [none]	
Return :			void	-	
Parameters :
void *pContext	-	
WINIO_CALLBACK pCallback	-	
void *pCallbackParam	-	
Note : [none]
\**********************************************************************************/
void WinIoTarg_setTxCallback(void *pContext, WINIO_CALLBACK pCallback,  void *pCallbackParam)
{
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  pIOContext->pTxCallback = pCallback;
  pIOContext->pTxCallbackParam = pCallbackParam;
}

/**********************************************************************************\
Function :			WinIoTarg_setRxCallback
Description : [none]	
Return :			void	-	
Parameters :
void *pContext	-	
WINIO_CALLBACK pCallback	-	
void *pCallbackParam	-	
Note : [none]
\**********************************************************************************/
void WinIoTarg_setRxCallback(void *pContext, WINIO_CALLBACK pCallback, void *pCallbackParam)
{
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  pIOContext->pRxCallback = pCallback;
  pIOContext->pRxCallbackParam = pCallbackParam;
}

/**********************************************************************************\
Function :			WinIoTarg_setStatusCallback
Description : [none]	
Return :			void	-	
Parameters :
void *pContext	-	
WINIO_STATUS_CALLBACK pCallback	-	
void *pCallbackParam	-	
Note : [none]
\**********************************************************************************/
void WinIoTarg_setStatusCallback(void *pContext, WINIO_STATUS_CALLBACK pCallback, void *pCallbackParam)
{
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  WinIoInterface *pWinIoInterface = (WinIoInterface*)(pIOContext->pWinIoInterface);
  if (pWinIoInterface)
  {
    pWinIoInterface->setStatusCB(pCallback, pCallbackParam);
  }
}

#if WIN_MODEM_SUPPORT
/**********************************************************************************\
Function :			WinModem_setStatusCallback
Description : [none]	
Return :			void	-	
Parameters :
void *pContext	-	
WINIO_STATUS_CALLBACK pCallback	-	
void *pCallbackParam	-	
Note : [none]
\**********************************************************************************/
void WinModem_setStatusCallback(void *pContext, WINIO_STATUS_CALLBACK pCallback, void *pCallbackParam)
{
  WinModem *pWinModem = (WinModem *)pContext;
  if (pWinModem)
  {
    pWinModem->setStatusCB(pCallback, pCallbackParam);
  }
}

/**********************************************************************************\
Function :			WinModemPool_setStatusCallback
Description : [none]	
Return :			void	-	
Parameters :
void *pContext	-	
WINIO_STATUS_CALLBACK pCallback	-	
void *pCallbackParam	-	
Note : [none]
\**********************************************************************************/
void WinModemPool_setStatusCallback(void *pContext, WINIO_STATUS_CALLBACK pCallback, void *pCallbackParam)
{
  WinModemPool *pWinModemPool = (WinModemPool *)pContext;
  if (pWinModemPool)
  {
    pWinModemPool->setStatusCB(pCallback, pCallbackParam);
  }
}

/**********************************************************************************\
Function :			WinModemPoolManager_setStatusCallback
Description : [none]	
Return :			void	-	
Parameters :
void *pContext	-	
WINIO_STATUS_CALLBACK pCallback	-	
void *pCallbackParam	-	
Note : [none]
\**********************************************************************************/
void WinModemPoolManager_setStatusCallback(void *pContext, WINIO_STATUS_CALLBACK pCallback, void *pCallbackParam)
{
  WinModemPoolManager *pWinModemPoolManager = (WinModemPoolManager *)pContext;
  if (pWinModemPoolManager)
  {
    pWinModemPoolManager->setStatusCB(pCallback, pCallbackParam);
  }
}

/**********************************************************************************\
Function :			WinModemPoolManager_getManager
Description : [none]	
Return :			void *	-	
Parameters :
void	-	
Note : [none]
\**********************************************************************************/
void *WinModemPoolManager_getManager(void)
{
  return GetModemPoolManager();
}

/**********************************************************************************\
Function :			WinModemPoolManager_destroy
Description : [none]	
Return :			void -	
Parameters : none
Note : [none]
\**********************************************************************************/
void WinModemPoolManager_destroy(void)
{
  if (WinModemPoolManager::g_pModemPoolManager)
  {
    delete WinModemPoolManager::g_pModemPoolManager;
    WinModemPoolManager::g_pModemPoolManager = NULL;
  }
}

#endif

/**********************************************************************************\
Function :			WinIoTarg_setEnabled
Description : [none]	
Return :			void	-	
Parameters :
void *pContext	-	
TMWTYPES_BOOL bStatus	-	
Note : [none]
\**********************************************************************************/
void WinIoTarg_setEnabled(    
  void *pContext,
  TMWTYPES_BOOL bStatus)
{
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  WinIoInterface *pWinIoInterface = (WinIoInterface*)(pIOContext->pWinIoInterface);
  if (pWinIoInterface)
  {
    pWinIoInterface->setChanEnabled(bStatus != 0 ? true : false);
  }
}

/**********************************************************************************\
Function :			WinIoTarg_startThread
Description :  Start event handle thread
Return :			void *	-	
Parameters :
WINIO_THREAD_FUN threadFun	-	
void *pParam	-	
unsigned int *threadID	-	
int nPriority	-	
Note : [none]
\**********************************************************************************/
void *WinIoTarg_startThread(WINIO_THREAD_FUN threadFun, void *pParam, unsigned int *threadID, int nPriority)
{
  // Start thread waiting for comm events
  // void *threadHandle = (void*)_beginthread(threadFun, 0, pParam);
  void *threadHandle = (void*)_beginthreadex(NULL, 0, threadFun, pParam, CREATE_SUSPENDED, threadID);
  if (threadHandle == 0)
  {
    return WINIOTARG_NULL;
  }
  SetThreadPriority(threadHandle, nPriority);
  ResumeThread(threadHandle);

  return threadHandle;
}

/**********************************************************************************\
Function :			WinIoTarg_endThread
Description : [none]	
Return :			void	-	
Parameters :
void *threadHandle	-	
Note : [none]
\**********************************************************************************/
void WinIoTarg_endThread(void *threadHandle)
{
  /* threadHandle is not a pointer, it is the handle itself */
  TerminateThread((HANDLE)threadHandle, 1);
}

/**********************************************************************************\
Function :			WinIoTarg_IsProtoAnaLogEnabled
Description : [none]	
Return :			bool	-	is protocol logging from target enabled
Parameters :
Note : [none]
\**********************************************************************************/
bool WinIoTarg_IsProtoAnaLogEnabled(void)
{
  return WinIoTargProtoAnaLogEnable;
}

/**********************************************************************************\
Function :			WinIoTarg_setProtoAnaLogEnable
Description : [none]	
Return :			void	-	
Parameters :
bool bEnable	-	enable/disable protocol logging from target
Note : [none]
\**********************************************************************************/
void WinIoTarg_setProtoAnaLogEnable(bool bEnable)
{
  WinIoTargProtoAnaLogEnable = bEnable;
}

/**********************************************************************************\
Function :			WinIoTarg_setProtoAnaLogFun
Description : [none]	
Return :			void	-	
Parameters :
WinIoTargProtoAnaLogFunType pFun	-	
Note : [none]
\**********************************************************************************/
void WinIoTarg_setProtoAnaLogFun(WinIoTargProtoAnaLogFunType pFun)
{
  WinIoTargProtoAnaLogFun = pFun;
}

/**********************************************************************************\
Function :			WinIoTarg_getDateTime
Description : [none]	
Return :			void	-	
Parameters :
TMWDTIME *pDateTime	-	
Note : [none]
\**********************************************************************************/
void WinIoTarg_getDateTime(TMWDTIME *pDateTime)
{
  CriticalSectionLockWinTarg lock(timeObjectLock);
  if (g_pWinIoTimeObject == NULL)
  {
    WinIoTarg_setTimeMode(WINIO_TIME_MODE_SIMULATED);
  }
  g_pWinIoTimeObject->getDateTime(pDateTime);
}

/**********************************************************************************\
Function :			WinIoTarg_setDateTime
Description : [none]	
Return :			TMWTYPES_BOOL	-	
Parameters :
const TMWDTIME *pNewDateTime	-	
Note : [none]
\**********************************************************************************/
TMWTYPES_BOOL WinIoTarg_setDateTime(const TMWDTIME *pNewDateTime)
{
  CriticalSectionLockWinTarg lock(timeObjectLock);
  if (g_pWinIoTimeObject == NULL)
  {
    WinIoTarg_setTimeMode(WINIO_TIME_MODE_SIMULATED);
  }
  return(g_pWinIoTimeObject->setDateTime(pNewDateTime));
}

/**********************************************************************************\
Function :			WinIoTarg_getUTCDateTime
Description : [none]	
Return :			void	-	
Parameters :
TMWDTIME *pDateTime	-	
Note : [none]
\**********************************************************************************/
void WinIoTarg_getUTCDateTime(TMWDTIME *pDateTime)
{  
  WinIoSystemTime::getUTCDateTime(pDateTime);
}

/**********************************************************************************\
Function :			WinIoTarg_setUTCDateTime
Description : [none]	
Return :			TMWTYPES_BOOL	-	
Parameters :
const TMWDTIME *pNewDateTime	-	
Note : [none]
\**********************************************************************************/
TMWTYPES_BOOL WinIoTarg_setUTCDateTime(const TMWDTIME *pNewDateTime)
{
  return(WinIoSystemTime::setUTCDateTime(pNewDateTime));
}

/**********************************************************************************\
Function :			WinIoTarg_convertLocalTime
Description : [none]	
Return :			void	-	
Parameters :
TMWDTIME *pDateTime	-	
time_t timeIn	-	
Note : [none]
\**********************************************************************************/
void WinIoTarg_convertLocalTime(TMWDTIME *pDateTime, time_t timeIn)
{
  CriticalSectionLockWinTarg lock(timeObjectLock);
  if (g_pWinIoTimeObject == NULL)
  {
    WinIoTarg_setTimeMode(WINIO_TIME_MODE_SIMULATED);
  }
  g_pWinIoTimeObject->convertLocalTime(pDateTime, timeIn);
}

/**********************************************************************************\
Function :			WinIoTarg_setTimeMode
Description : [none]	
Return :			void	-	
Parameters :
WINIO_TIME_MODE timeMode	-	
Note : [none]
\**********************************************************************************/
void WinIoTarg_setTimeMode(WINIO_TIME_MODE timeMode)
{
  CriticalSectionLockWinTarg lock(timeObjectLock);
  if (g_pWinIoTimeObject)
  {
    delete g_pWinIoTimeObject;
  }
  switch(timeMode)
  {

  case WINIO_TIME_MODE_SYSTEM_NO_SETTIME:
  case WINIO_TIME_MODE_SYSTEM:
  case WINIO_TIME_MODE_SYSTEM_TIME_ZONE:
  case WINIO_TIME_MODE_SYSTEM_TIME_ZONE_NO_SETTIME:
    g_pWinIoTimeObject = new WinIoSystemTime(timeMode);
    break;
  case WINIO_TIME_MODE_SIMULATED:
    g_pWinIoTimeObject = new WinIoSimulatedTime(timeMode);
    break;
  default:
    g_pWinIoTimeObject = new WinIoSystemTime(WINIO_TIME_MODE_SIMULATED);
    break;
  }
}
  
#if TMW_USE_GATEWAY_DEFINES
#if _MSC_VER >= 1400
void WinIoTarg_setTimeZone(int timeZone, bool ignoreDST)
{
  WinIoSystemTime::SetTimeZone(timeZone, ignoreDST);
}

WINIOTARG_API bool WinIoTarg_getCurrentTimeZone(char *curTimeZone)
{
  return WinIoSystemTime::GetCurrentTimeZone(curTimeZone);
}

WINIOTARG_API bool WinIoTarg_getTimeZone(int index, char *returnedTimeZone)
{
  return WinIoSystemTime::GetTimeZone(index, returnedTimeZone);
}

WINIOTARG_API int WinIoTarg_getUTCTimeZone(const char *utcTZName)
{
  return WinIoSystemTime::GetUTCTimeZone(utcTZName);
}

WINIOTARG_API int WinIoTarg_getTimeZoneIndex(const char *tzName)
{
  return WinIoSystemTime::GetTimeZoneIndex(tzName);
}

WINIOTARG_API void *WinIoTarg_getTimeZones(void)
{
  return &WinIoSystemTime::m_arrRegTimeZoneInfo;
}

WINIOTARG_API void WinIoTarg_createTimeZones(void)
{
#if TMW_USE_GATEWAY_DEFINES
#if _MSC_VER >= 1400
  // do this to get the time zone sub system setup
  WinIoSystemTime::GetRegistryManager();
#endif
#endif
}
WINIOTARG_API void WinIoTarg_cleanTimeZones(void)
{
#if TMW_USE_GATEWAY_DEFINES
#if _MSC_VER >= 1400
  WinIoSystemTime::CleanRegManager();
#endif
#endif
  if (g_pWinIoTimeObject)
  {
    delete g_pWinIoTimeObject;
  }
}

WINIOTARG_API void WinIoTarg_getSpecificTimeFromUtcTime(TMWDTIME *utcTime, TMWDTIME *specificTime, int timeZoneIndex, TMWTYPES_BOOL ignoreDST)
{
  WinIoSystemTime::GetSpecificTimeFromUtcTime(utcTime, specificTime, timeZoneIndex, ignoreDST);
}

WINIOTARG_API char *WinIoTarg_strptime(const char *s, const char *format, struct tm *tm, int *millisecs)
{
  return WinIoSystemTime::strptime(s, format, tm, millisecs);
}
#endif
#endif

#if TMW_SUPPORT_MONITOR

TMWTYPES_BOOL WinIoTarg_initTCPMonitor(void)
{
  if(pcapNotInitialized)
  {
    HINSTANCE hinst = LoadLibrary(_T("wpcap.dll"));
    if (hinst == NULL)
      return(TMWDEFS_FALSE);

    pcapNotInitialized = false;
  }
  return(TMWDEFS_TRUE);
}

TMWTYPES_BOOL WinIoTarg_CheckHostName(
  char  *pHost)
{
  /* If this is not a x.x.x.x ip address, see if the hostname can be found */
  if(strchr(pHost, '.') == 0)
  {
    HOSTENT *h = gethostbyname(pHost);
    if(h == NULL)
    {
      return(false);
    }
  }
  return(true);
}

void WinIoTarg_GetTCPInterfaces(char *pInterface[], int *pCount)
{ 
  WinMonChannel::getAllInterfaces(pInterface, pCount);
}


void WinIoTarg_inputData(
  void *pContext, 
  TMWTYPES_UCHAR *pBuf, 
  TMWTYPES_USHORT length)
{
  WINIO_CONTEXT *pIOContext = (WINIO_CONTEXT *)pContext;
  if(pIOContext->type == WINIO_TYPE_MON)
  {
    WinMonChannel *pInterface = (WinMonChannel*)(pIOContext->pWinIoInterface);
    pInterface->inputData(pBuf, length);
  }
}
#endif

TMWTYPES_LONG WinIoTarg_GetPrivateProfileInt(
  const TCHAR *lpAppName,  // section name
  const TCHAR *lpKeyName,  // key name
  int nDefault,           // return value if key name not found
  const TCHAR *lpFileName  // initialization file name
  )
{
  return GetPrivateProfileInt(lpAppName,lpKeyName,nDefault,lpFileName);
}

TMWTYPES_BOOL WinIoTarg_SetPrivateProfileInt(
  const TCHAR *lpAppName,  // section name
  const TCHAR *lpKeyName,  // key name
  int value,              // value to set
  const TCHAR *lpFileName  // initialization file name
  )
{
  TCHAR str[256] = {0};
  _tprintf(str,_T("%d"),value);
  BOOL bRet = WritePrivateProfileString(lpAppName,lpKeyName,str,lpFileName);
  TMWTYPES_ULONG error = 0;
  if (bRet == 0)
  {
    error = GetLastError();
    return(TMWDEFS_FALSE);
  }
  return TMWDEFS_TRUE;
}

TMWTYPES_ULONG WinIoTarg_GetPrivateProfileString(
  const TCHAR *lpAppName,        // section name
  const TCHAR *lpKeyName,        // key name
  const TCHAR *lpDefault,        // default string
  TCHAR *lpReturnedString,       // destination buffer
  TMWTYPES_ULONG nSize,         // size of destination buffer
  const TCHAR *lpFileName        // initialization file name
  )
{
  return GetPrivateProfileString(lpAppName,lpKeyName,lpDefault,lpReturnedString,nSize,lpFileName);
}

TMWTYPES_BOOL WinIoTarg_SetPrivateProfileString(
  const TCHAR *lpAppName,        // section name
  const TCHAR *lpKeyName,        // key name
  const TCHAR *value,            // default string
  const TCHAR *lpFileName        // initialization file name
  )
{
  if(WritePrivateProfileString(lpAppName,lpKeyName,value,lpFileName))
  {
    return(TMWDEFS_TRUE);
  }
  return(TMWDEFS_FALSE);
}

void * TMWDEFS_GLOBAL WinIoTarg_CreateSessionTimeObject(const TMWDTIME *pDateTime)
{
  WinIoBaseTime *pTimeObject = new WinIoSimulatedTime(WINIO_TIME_MODE_SIMULATED);
  pTimeObject->setDateTime(pDateTime);
  return pTimeObject;
} 

void TMWDEFS_GLOBAL WinIoTarg_SetSessionDateTime(void *pHandle, TMWDTIME *pDateTime)
{
  WinIoBaseTime *pTimeObject = (WinIoBaseTime*)pHandle;
  pTimeObject->setDateTime(pDateTime);
}

void TMWDEFS_GLOBAL WinIoTarg_GetSessionDateTime(void *pHandle, TMWDTIME *pDateTime)
{
  WinIoBaseTime *pTimeObject = (WinIoBaseTime*)pHandle;
  pTimeObject->getDateTime(pDateTime);
}

void TMWDEFS_GLOBAL WinIoTarg_DeleteSessionTimeObject(void *pHandle)
{
  WinIoBaseTime *pTimeObject = (WinIoBaseTime*)pHandle;
  delete pTimeObject;
}

void WinIoTarg_lockInit(TMWDEFS_RESOURCE_LOCK *pLock)
{
  *pLock = (CRITICAL_SECTION *)malloc(sizeof(CRITICAL_SECTION));
  InitializeCriticalSection((CRITICAL_SECTION*)*pLock);
}
void WinIoTarg_lockSection(TMWDEFS_RESOURCE_LOCK *pLock)
{
  EnterCriticalSection((CRITICAL_SECTION*)*pLock);
}

void WinIoTarg_unlockSection(TMWDEFS_RESOURCE_LOCK *pLock)
{
  LeaveCriticalSection((CRITICAL_SECTION*)*pLock);
}

void WinIoTarg_lockDelete(TMWDEFS_RESOURCE_LOCK *pLock)
{
  DeleteCriticalSection((CRITICAL_SECTION*)*pLock);
  free(*pLock);
}

void WinIoTarg_formatErrorMsg(char *pBuffer, TMWTYPES_ULONG error)
{
  TCHAR buffer[128];
  ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error,
    0, buffer, 128, NULL);
  USES_CONVERSION;
  strcpy(pBuffer, T2A(buffer));
}

/*****************************************************************************/
int WinIoTarg_DebugPrintf(char *format,...)
{
  struct tm *newtime;
  time_t aclock;
  TCHAR  file_name[256];
  static HANDLE hStdout = 0;
  static FILE* fp = 0;
  char   Buffer[2048];
  va_list argpointer;
  int Result;
  DWORD BytesWritten;

  if (WinIoTarg_GetPrivateProfileInt(_T("debug"), _T("log"), 0, _T("TMWDebug.ini")) == 0)
    return 0;

  va_start(argpointer, format);
  Result = vsprintf(Buffer, format, argpointer);

  if (WinIoTarg_GetPrivateProfileInt(_T("debug"), _T("file"), 0, _T("TMWDebug.ini")) == 0)
  {
    if (fp != 0)
    {
      fclose(fp);
      fp =0;
    }
    if (hStdout == 0)
    {
      AllocConsole();
      hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    WriteFile(hStdout, Buffer, Result, &BytesWritten, NULL);    
  }
  else
  {
    if (hStdout != 0)
    {
      FreeConsole();
      hStdout = 0;
    }

    if (fp == 0)
    {
      WinIoTarg_GetPrivateProfileString(_T("debug"), _T("file_name"), _T(""), file_name, 255, _T("TMWDebug.ini"));
        USES_CONVERSION;
      fp = fopen(T2A(file_name), "w");
      if (fp == 0)
      {
        ::MessageBox
          (
          NULL,
          _T("Could not open log file.\n Please define 'file_name' in TMWDebug.ini"),
          _T("TMW Debug Log Message"),
          MB_OK | MB_ICONSTOP
          );
        exit(1);
      }
    }

    time(&aclock);                 /* Get time in seconds */
    newtime = localtime(&aclock);  /* Convert time to struct tm form  */
    fprintf(fp, "%s: %s", asctime(newtime), Buffer);
    fflush(fp);
  }
  va_end(argpointer);

  return (Result);
}
