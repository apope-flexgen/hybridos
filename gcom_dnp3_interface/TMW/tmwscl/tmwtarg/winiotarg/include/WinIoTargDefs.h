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

/** \file WinIoTargDefs.h
 * Provides the definition of various macros and types for the WinIoTarg DLL. 
 */

#ifndef WinIoTargDefs_DEFINED
#define WinIoTargDefs_DEFINED

#include <tchar.h>
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwtypes.h"
#include "tmwscl/utils/tmwdiag.h"
#include "tmwscl/utils/tmwtarg.h"
#include "WinIoTargEnums.h"
 
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the WINIOTARG_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// WINIOTARG_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef WINIOTARG_EXPORTS
#define WINIOTARG_API __declspec(dllexport)
#else
#define WINIOTARG_API __declspec(dllimport)
#endif

#define WINIOTARG_NULL (0)

#define WINIOTARG_STR_LEN TMWTARG_STR_LEN
 

/* Largest received UDP Packet length supported.
 * This also should be <= WINTCP_BUFFER_SIZE 
 */ 
#define WINIOTARG_MAX_UDP_RCVLEN 3000

// use these sparingly leave off by default
#define TRACE_MASK_NONE          0x00000000
#define TRACE_MASK_CUSTOM        0x00000100
#define TRACE_MASK_TARGET        0x00000200

/**
A template for a Protocol Analyzer logging function
The function is expected to be implemented outside of the DLL.
It will be called when the DLL calls proto ana log
*/
typedef void (*WinIoTargProtoAnaLogFunType)(void *pChannel, unsigned long trace_mask, TMWDIAG_ID sourceId, const char *chanID, const char *format, va_list ap);

/**
  generic WinIoTarg callback
 */
typedef TMWTYPES_BOOL   (*WINIO_CALLBACK)(
  void *pContext, 
  void *pCallbackParam,
  TMWTYPES_USHORT numBytes,
  TMWTYPES_USHORT maxBytes,
  TMWTYPES_USHORT *pReturnedNumBytes,
  TMWTYPES_UCHAR *pBuf);

/**
  status WinIoTarg callback
 */
typedef void (*WINIO_STATUS_CALLBACK)(
  void *pCallbackParam,
  WINIO_STATUS eStatus,
  TMWTYPES_ULONG iStatusCode);

/**
  Data type used to configure a Modem Pool channel. 
 */
typedef struct WinModemPoolChannelConfigStruct {
  char   chnlName[WINIOTARG_STR_LEN];                      /*!<  User specified channel name */
  char   poolName[WINIOTARG_STR_LEN];                      /*!<  the pool to use */
  char   phoneNumber[WINIOTARG_STR_LEN];                   /*!<  number to dial out */
  TMWTYPES_ULONG answerTime;             /*!<  time to wait after dial for answer */
  TMWTYPES_ULONG redialLimit;            /*!<  number of times to attempt to dial */
  TMWTYPES_ULONG idleTime;               /*!<  time to wait after idle to hang up modem */
  TMWTYPES_BOOL   bModbusRTU;
  TMWTYPES_BOOL   bDialOut;
} WINMODEM_POOL_CHANNEL_CONFIG;

/**
  Data type used to configure a Modem. 
 */
typedef struct WinModemConfigStruct {
  char   chnlName[WINIOTARG_STR_LEN];                      /*!<  User specified channel name */
  char   poolName[WINIOTARG_STR_LEN];                      /*!<  The pool this modem is a member of */

  /* com port stuff */
  char   *portName;                      /*!<  "COM1", "COM2", etc. */
  WIN232_PORT_MODE  portMode;            /*!<  hardware, software, windows */
  char   *baudRate;                      /*!<  in string form; example: "9600" */
  WIN232_PARITY parity;                  /*!<  parity */
  WIN232_DATA_BITS numDataBits;          /*!<  7 or 8 */
  WIN232_STOP_BITS numStopBits;          /*!<  1 or 2 */
  /* modem stuff */
  char   *initString;
  char   *hangupString;
  WINMODEM_DIALING_MODE dialingMode;
  TMWTYPES_USHORT readCommandTimeout;
  TMWTYPES_USHORT writeCommandTimeout;
  WINMODEM_RESP_CHAR respTerminatorChar;
  TMWTYPES_BOOL   bNoDialOut;
  TMWTYPES_BOOL   bEnable;
  void *pWinModemPool;
  WINIO_STATUS_CALLBACK statusCB;
  void *statusCBdata;
} WINMODEM_CONFIG;

/**
  Data type used to configure a Modem Pool. 
 */
typedef struct WinModemPoolConfigStruct {
  char   poolName[WINIOTARG_STR_LEN];                /*!<  User specified pool name */
} WINMODEM_POOL_CONFIG;


// The following defines may be used when configuring UDP ports. 

// Don't open a socket for UDP
#define WINTCP_UDP_PORT_NONE TMWTARG_UDP_PORT_NONE

// Let the UDP/IP stack determine what port number to use (master)
#define WINTCP_UDP_PORT_ANY  TMWTARG_UDP_PORT_ANY

// When sending responses use the source port number from the request (slave)
#define WINTCP_UDP_PORT_SRC  TMWTARG_UDP_PORT_SRC

/* Maximum length of path and filenames for cryptography */
#define WINIO_CRYPTO_ID_LEN            TMWTARG_CRYPTO_ID_LEN

/* Maximum length of passphrase for cryptography */
#define WINIO_CRYPTO_PASSPHRASE_LEN    TMWTARG_CRYPTO_PASSPHRASE_LEN

/* Maximum length of common name to expect on incoming TLS certs */
#define WINIO_CRYPTO_TLS_NAME_LEN      TMWTARG_CRYPTO_TLS_NAME_LEN

/**
  Use the common TMWTARG data Type to configure the TCP/IP interface.
 */
typedef TMWTARGTCP_CONFIG WINTCP_CONFIG;

/**
 * Data type used to configure the modbus plus interface.\n
 *\n
 * Modbus Plus Route configuration\n
 *\n
 * MASTER MODBUS PLUS (i.e. as a client)\n
 * routePath specifies how to get to the slave.\n
 *   NOTE: \n
 *   1. For example if a slave on the local MB+ network is node 63 and is listening on path 1 the routePath is 63.1\n
 *   \n
 * cardNum is the card on which to send the message
 * slavePath does not apply for master
 * \n
 * SLAVE MODBUS PLUS (i.e. as a server)
 * routePath does not apply
 * cardNum is the card to listen for messages on
 * slavePath is the path on the card to listen on\n
 *    NOTE:\n
 *      1. The node id (i.e. 63 in the above example) is set in the MBX driver.\n
 *      2. Diffrent values for slavePath allow reciept of messages by multiple applications on the computer.\n
 * 
 */
typedef struct WinMBPConfigStruct {
  char chnlName[WINIOTARG_STR_LEN];            /*!<  User specified channel name                  */
  char routePath[WINIOTARG_STR_LEN];           /*!<  MBP Routing Path of the form xx.xx.xx.xx.xx  */
  TMWTYPES_USHORT cardNum;       /*!<  MBP Host Adapter number, 0-65535             */
  TMWTYPES_UCHAR  slavePath;     /*!<  MBP Slave Path for Server channels, 1-8      */
  TMWTYPES_ULONG  recvTimeout;   /*!<  Channel Receive Timeout, not currently used  */
  WINMBP_MODE     mode;          /*!<  Clent or Server Mode, see WinMBPModeEnum     */
  TMWTYPES_BOOL   leaveMasterPortOpen;
} WINMBP_CONFIG;

/**
  Data Type used to configure the Monitor Interface.
 */
typedef struct  {
  TMWTYPES_BOOL    monitorMode;
  char             chnlName[WINIOTARG_STR_LEN];       /*!<  User specified channel name */
  char             pFilter[WINIOTARG_STR_LEN];                  
  TMWTYPES_BOOL    commandInput;  
  TMWTYPES_CHAR    interfaceNum;                      /*!< Left for backward compatibility */
  TMWTYPES_CHAR    interfaceDescr[WINIOTARG_STR_LEN]; /*!< Which TCP interface to monitor on */
  char             ipAddress[TMWTARG_IP_ADDR_LENGTH];
  TMWTYPES_USHORT  ipPort;
  TMWTYPES_BOOL    ipV6;
} WINMON_CONFIG;

/**
  The context returned from WinIoTarg_Create()
 */
typedef struct WinIOContext
{
  void *pWinIoInterface;
  WINIO_TYPE_ENUM type;
  TMWTYPES_ULONG  connectDelay;
  TMWTYPES_MILLISECONDS  connectRetry;

  void *pTxCallbackParam;
  WINIO_CALLBACK pTxCallback;

  void *pRxCallbackParam;
  WINIO_CALLBACK pRxCallback;

} WINIO_CONTEXT;

/**
  A Data Type that includes all other configuration data types
 */
typedef struct WinIOConfig {
  WINIO_TYPE_ENUM type;
  WINIO_TIME_MODE timeMode;
  TMWTYPES_ULONG  connectDelay;
  TMWTYPES_ULONG  connectRetry;
  TMWTYPES_BOOL forceDisconnected;
  TMWTARG232_CONFIG targ232;
  WINMODEM_POOL_CHANNEL_CONFIG winModemPoolChannel;
  WINMODEM_POOL_CONFIG winModemPool;
  WINMODEM_CONFIG winModem;
  TMWTARGTCP_CONFIG targTCP;
  WINMBP_CONFIG winMBP;
  WINMON_CONFIG winMON;
} WINIO_CONFIG;

/**
  prototype for a thread function
 */
typedef unsigned( __stdcall  *WINIO_THREAD_FUN )( void * );

#endif // WinIoTargDefs_DEFINED