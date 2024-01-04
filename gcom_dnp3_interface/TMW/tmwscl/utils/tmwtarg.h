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

/* file: tmwtarg.h
 * description: This file defines the interface between all Triangle
 *   MicroWorks, Inc. (TMW) source code libraries and the target hardware
 *   and software. This file contains a number of function declarations
 *   (which are implemented in tmwtarg.c) that provide access to required
 *   system resources. The first step in porting a TMW source code library
 *   to your device is to implement each of these functions for your target
 *   platform
 *
 *  It should not be necessary for a target implementor to change anything 
 *  in this file. Changes should be made to tmwtarg.c
 */
#ifndef TMWTARG_DEFINED
#define TMWTARG_DEFINED

/* Include target specific header files as required */
#if !defined(_lint)
#if defined(_MSC_VER)
#ifndef _BIND_TO_CURRENT_VCLIBS_VERSION
#define _BIND_TO_CURRENT_VCLIBS_VERSION 1
#endif
#include <stdio.h>
#include <string.h>
#include <memory.h>
#else
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif
#endif

/* Triangle MicroWorks, Inc. Header Files */
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwtypes.h"
#include "tmwscl/utils/tmwdiag.h"
#include "tmwscl/utils/tmwdtime.h"
#include "tmwscl/utils/tmwtargp.h"
#include "tmwtargcnfg.h"
#include "tmwtargos.h"

/* Used to avoid 'unused parameter' warnings
 */
#ifdef DONT_USE_TMWTARG_UNUSED_PARAM
  #define TMWTARG_UNUSED_PARAM(x)
#else
  #define TMWTARG_UNUSED_PARAM(x) TMWCNFG_UNUSED_PARAM(x)
#endif

/* Indicate what remote UDP port to send the datagram to
 * NOTE: UDP is only supported for DNP
 */
/* Don't use UDP */
#define TMWTARG_UDP_NONE        0

/* Send to the remote port to be used for requests or responses */
#define TMWTARG_UDP_SEND        1

/* Send to the remote port to be used for unsolicited responses
 * Once the outstation has received a request from master this will
 * use the same dest port to be used for all responses.
 */
#define TMWTARG_UDP_SEND_UNSOL  2

/* Send to broadcast address when UDP ONLY is configured */
#define TMWTARG_UDPONLY_BROADCAST 3


/* The following defines may be used when configuring UDP ports.
 *  (These may be redefined to any three ports that are not going
 *  to be used as real UDP port numbers).
 */

/* Don't open a socket for UDP */
#define TMWTARG_UDP_PORT_NONE 0

/* Let the UDP/IP stack determine what port number to use (master) */
#define TMWTARG_UDP_PORT_ANY  1

/* When sending responses use the source port number from the request (slave) */
#define TMWTARG_UDP_PORT_SRC  2

/* Length of strings contained in structures  */
#define TMWTARG_STR_LEN     256

typedef enum TmwTargIpVersionEnum {
  TMWTARG_IPV4,
  TMWTARG_IPV6
} TMWTARG_IP_VERSION;

#define TMWTARG_ADDR_IPV4_LOOPBACK  "127.0.0.1"
#define TMWTARG_ADDR_IPV4_ANY       "0.0.0.0"
#define TMWTARG_ADDR_IPV4_UDP_BCAST "192.168.1.255"

#define TMWTARG_ADDR_IPV6_LOOPBACK  "::1"
#define TMWTARG_ADDR_IPV6_ANY       "::"
#define TMWTARG_ADDR_IPV6_UDP_BCAST "FF02::1"

/* Maximum length for the name of a network interface */
#define TMWTARG_IF_NAME_LENGTH  32

/* Maximum length of common name to expect on incoming TLS certs */
#define TMWTARG_CRYPTO_TLS_NAME_LEN  128

#if !WINIOTARG_SUPPORT_LEGACY_CONFIG
/** parity for RS232 serial communications channel */
typedef enum TmwTarg232Parity
{
  TMWTARG232_PARITY_NONE,   /* no parity */
  TMWTARG232_PARITY_EVEN,   /* even  parity*/
  TMWTARG232_PARITY_ODD     /* odd  parity*/
} TMWTARG232_PARITY;

/** flow control for serial communications */
typedef enum TmwTarg232PortMode
{
  TMWTARG232_MODE_NONE,       /* no flow control */
  TMWTARG232_MODE_HARDWARE,   /* hardware flow control */
  TMWTARG232_MODE_WINDOWS     /* windows flow control */
} TMWTARG232_PORT_MODE;

/** DTR mode for serial communications port when in TMWTARG232_MODE_WINDOWS */
typedef enum TmwTarg232DtrMode
{
  TMWTARG232_DTR_DISABLE = 0,     /* Disables the DTR line when the device is opened
                                   * and leaves it disabled. */
  TMWTARG232_DTR_ENABLE = 1,      /* Enables the DTR line when the device is opened 
                                   * and leaves it on. */
  TMWTARG232_DTR_HANDSHAKE = 2    /* Enables DTR handshaking. If handshaking is 
                                   * enabled, it is an error for the application to 
                                   * adjust the line by using the EscapeCommFunction
                                   * function. */
} TMWTARG232_DTR_MODE;

/** RTS mode for serial communications port when in TMWTARG232_MODE_WINDOWS */
typedef enum TmwTarg232RtsMode
{
  TMWTARG232_RTS_DISABLE = 0,     /* Disables the RTS line when the device is opened
                                   * and leaves it disabled. */
  TMWTARG232_RTS_ENABLE = 1,      /* Enables the RTS line when the device is opened
                                   * and leaves it on. */
  TMWTARG232_RTS_HANDSHAKE = 2,   /* Enables RTS handshaking. The driver raises the 
                                   * RTS line when the "type-ahead" (input) buffer 
                                   * is less than one-half full and lowers the RTS line
                                   * when the buffer is more than three-quarters full. 
                                   * If handshaking is enabled, it is an error for the 
                                   * application to adjust the line by using the 
                                   * EscapeCommFunction function. */
  TMWTARG232_RTS_TOGGLE = 3       /* Specifies that the RTS line will be high if bytes 
                                   * are available for transmission. After all buffered
                                   * bytes have been sent, the RTS line will be low. */
} TMWTARG232_RTS_MODE;
#endif


/* Maximum length of passphrase for cryptography */
#define TMWTARG_CRYPTO_PASSPHRASE_LEN    128

#ifdef _WIN32
/* Maximum length of IP address strings (IPV6 addresses can be up to 45 bytes) */
#define TMWTARG_IP_ADDR_LENGTH   128

/* Maximum length of path and filenames for cryptography */
#define TMWTARG_CRYPTO_ID_LEN    512

#else

/* Maximum length of IP address strings (IPV6 addresses can be up to 45 bytes) */
#define TMWTARG_IP_ADDR_LENGTH   64

/* Maximum length of path and filenames for cryptography */
#define TMWTARG_CRYPTO_ID_LEN    256  /* Reduce channel memory requirements for 
                                       * non-Windows applications. */

typedef enum TmwTargTCPRoleEnum {
  /* Master, this only matters for DNP Dual End Point */
  TMWTARGTCP_ROLE_MASTER,

  /* Outstation, this only matters for DNP Dual End Point */
  TMWTARGTCP_ROLE_OUTSTATION
} TMWTARGTCP_ROLE;

/* Ideally there would be a single enumerated type for all target layers.
* The Windows specific mapping is done to ensure backward compatibility.
* New target layers should use the non-windows mapping.
*/

 /* Define data types used to interface to the TCP target library. */
typedef enum TmwTargTCPModeEnum {
  /* listen for connection */
  TMWTARGTCP_MODE_SERVER,

  /* attempt to make a connection */
  TMWTARGTCP_MODE_CLIENT,

  /* UDP only, no TCP connection */
  TMWTARGTCP_MODE_UDP,

  /* both client and server functionality */
  TMWTARGTCP_MODE_DUAL_ENDPOINT,

} TMWTARGTCP_MODE;

// line up with _WINIO_TYPE_ENUM
typedef enum TmwTargIOTypeEnum {
  TMWTARGIO_TYPE_232 = 0,
  TMWTARGIO_TYPE_TCP = 4,
  TMWTARGIO_TYPE_UDP_TCP = 5,
  TMWTARGIO_TYPE_NONE = 8
} TMWTARGIO_TYPE_ENUM;

typedef enum TmwTarg232StopBitsEnum {
  TMWTARG232_STOP_BITS_1,
  TMWTARG232_STOP_BITS_2
} TMWTARG232_STOP_BITS;

typedef enum TmwTarg232DataBitsEnum {
  TMWTARG232_DATA_BITS_7,
  TMWTARG232_DATA_BITS_8
} TMWTARG232_DATA_BITS;

#endif

typedef enum TmwTargChannelState
{
  TMWTARG_CHANNEL_INITIALIZED = 1,
  TMWTARG_CHANNEL_CLOSED,
  TMWTARG_CHANNEL_OPENED,
} TMWTARG_CHANNEL_STATE;

/**
  Data type used to configure the the RS232 interface.
 */
typedef struct TmwTarg232ConfigStruct {
  char                 chnlName[TMWTARG_STR_LEN];  /* User specified channel name */
  char                 portName[TMWTARG_STR_LEN];  /* "COM1", "COM2", etc. */
  TMWTARG232_PORT_MODE portMode;                   /* hardware, software, windows */
  TMWTARG_TYPE_BAUDRATE baudRate;                  /* in string form; example: "9600" */
  TMWTARG232_PARITY    parity;                     /* parity */
  TMWTARG232_DATA_BITS numDataBits;                /* 7 or 8 */
  TMWTARG232_STOP_BITS numStopBits;                /* 1 or 2 */
  TMWTYPES_BOOL        bModbusRTU;
  TMWTARG232_DTR_MODE  dtrMode;
  TMWTARG232_RTS_MODE  rtsMode;
  TMWTYPES_BOOL        disabled;
  TMWTYPES_BOOL        polledMode;                 /* Polled or Event Driven Receive data mode */
} TMWTARG232_CONFIG;

/* NOTE: Common TCP config structure, which is convenient for test applications */
typedef struct TmwTargTCPConfigStruct { 
  /* User specified channel name */
  TMWTYPES_CHAR   chnlName[TMWTARG_STR_LEN];    

  /* On client -
   *      this is the IP address to set up TCP connection to
   * On server and Dual End Point Device -
   *      this is the IP address to accept TCP connection from
   *      May be *.*.*.* indicating accept connection from any client
   */
  TMWTYPES_CHAR   ipAddress[TMWTARG_IP_ADDR_LENGTH];

  /* Allows binding to a specific address   */
  TMWTYPES_CHAR   localIpAddress[TMWTARG_IP_ADDR_LENGTH]; 

  /* On client -
  *      this is the port to connect to
  * On server and Dual End Point Device -
  *      this is the port to listen on
  */
  TMWTYPES_USHORT ipPort;

  /* Number of milliseconds to wait for TCP connect to succeed or fail */
  TMWTYPES_ULONG  ipConnectTimeout;

  /* Indicate CLIENT, SERVER, DUAL END POINT, or UDP only
  *  (DUAL END POINT provides both CLIENT and SERVER functionality but
  *  with only one connection at a time)
  */
  TMWTARGTCP_MODE mode;

  /* If TRUE, when a new connect indication comes in and this channel is
  * already connected, it will be marked for disconnect. This will allow a
  * new connection to come in next time. This handles not receiving notification
  * of disconnect from the remote end, but remote end trying to reconnect.
  * If you want to allow multiple simultaneous connections to multiple channels
  * from any IP address to a particular port number, this parameter should be
  * set to FALSE.
  *
  * For DNP this should be set to TMWDEFS_FALSE according to DNP3 Specification
  *  IP Networking. Keep alive will detect that original connection has
  * failed, which would then allow a new connection to be rcvd.
  */
  TMWTYPES_BOOL   disconnectOnNewSyn;

  /* NOTE: The following configuration parameters are required to support
  *   DNP3 Specification IP Networking. These are not required for
  *   the IEC or Modbus protocols.

  *  Indicate master or outstation (slave) role in dnp networking
  *  as specified by DNP3 Specification IP Networking
  */
  TMWTARGTCP_ROLE role;

  /* If Dual End Point is supported a listen will be done on the above ipPort
  *  and a connection request will be sent to this port number when needed.
  *  This should match ipPort on remote device.
  *  Normal state is listen, connection will be made when there is data to send.
  */
  TMWTYPES_USHORT dualEndPointIpPort;
#if TMWTARG_SUPPORT_UDP
  /* Destination IP address for UDP broadcast requests.
  * This is only used by a DNP Master when TCP and UDP are supported.
  * If UDP ONLY is configured, ipAddress will be used as destination 
  * for all requests.
  */
  TMWTYPES_CHAR     udpBroadcastAddress[TMWTARG_IP_ADDR_LENGTH];

  /* Local port for sending and receiving UDP datagrams on.
  * If this is set to TMWTARG_UDP_PORT_NONE, UDP will not be enabled.
  * For DNP networking UDP should be supported.
  * It is not needed for any of the current IEC or modbus protocols.
  * On Master - If this is set to TMWTARG_UDP_PORT_ANY, an unspecified available
  *             port will be used.
  * On Slave  - This should be chosen to match the UDP port that the master uses
  *             to send Datagram messages to.
  *             This must not be TMWTARGP_UDP_PORT_ANY or TMWTARG_UDP_PORT_SRC.
  */
  TMWTYPES_USHORT localUDPPort;

  /* On Master - if TCP and UDP is configured this specifies the destination UDP/IP
  *              port to send broadcast requests in UDP datagrams to.
  *             if UDP ONLY is configured this specifies the destination UDP/IP
  *              port to send all requests in UDP datagrams to.
  *             This must match the "localUDPPort" on the slave.
  * On Slave  - if TCP and UDP this is not used.
  *             if UDP ONLY is configured this specifies the destination UDP/IP
  *              port to send responses to.
  *              Can be TMWTARG_UDP_PORT_SRC indicating use the src port from a
  *              UDP request received from master.
  */
  TMWTYPES_USHORT destUDPPort;

  /* On master - Not used.
  * On Slave  - if TCP and UDP not used.
  *             if UDP ONLY is configured this specifies the destination UDP/IP
  *              port to send the initial Unsolicited Null response to.
  *              After receiving a UDP request from master, destUDPPort (which)
  *              may indicate use src port) will be used for all responses.
  *              This must not be TMWTARG_UDP_PORT_NONE, TMWTARG_UDP_PORT_ANY, or
  *              TMWTARG_UDP_PORT_SRC for a slave that supports UDP.
  */
  TMWTYPES_USHORT initUnsolUDPPort;

  /* Whether or not to validate source address of received UDP datagram. */
  TMWTYPES_BOOL   validateUDPAddress;
#endif

  /* Use TLS Transport Layer Security for this channel */
  TMWTYPES_BOOL   useTLS;

  /* Polled or Event Driven Receive data mode */
  TMWTYPES_BOOL   polledMode;

  /* TLS configuration */
#if TMWTARG_SUPPORT_TLS
  /* File containing the private key for RSA TLS ciphers */
  char tlsRsaPrivateKeyFile[TMWTARG_CRYPTO_ID_LEN]; 

  /* PassPhrase for decrypting the private key for RSA TLS ciphers */
  char tlsRsaPrivateKeyPassPhrase[TMWTARG_CRYPTO_PASSPHRASE_LEN]; 

  /* File containing the certificate for key for RSA TLS ciphers */
  char tlsRsaCertificateId[TMWTARG_CRYPTO_ID_LEN];      


  /* File containing the private key for DSA TLS ciphers */
  char tlsDsaPrivateKeyFile[TMWTARG_CRYPTO_ID_LEN];  

  /* PassPhrase for decrypting the private key for DSA TLS ciphers */
  char tlsDsaPrivateKeyPassPhrase[TMWTARG_CRYPTO_PASSPHRASE_LEN];

  /* File containing the certificate for key for DSA TLS ciphers */
  char tlsDsaCertificateId[TMWTARG_CRYPTO_ID_LEN];    


  /* Common name to expect on incoming TLS certs (empty string disables) */
  char tlsCommonName[TMWTARG_CRYPTO_TLS_NAME_LEN];        

  /* File containing Certificate Authority Certificates */
  char caFileName[TMWTARG_CRYPTO_ID_LEN];     

  /* Path to Directory of Certificate Authority Certificates (instead of caFileName) */
  char caPathName[TMWTARG_CRYPTO_ID_LEN];         

  /* File containing Certificate Revocation List */
  char caCrlFileName[TMWTARG_CRYPTO_ID_LEN];         

  /* Depth of certificate chaining verification */
  TMWTYPES_UCHAR  nCaVerifyDepth;                         

  /* Max time (seconds) before forcing cipher renegotiation */
  int  nTlsRenegotiationSeconds; 

  /* Max PDUs before forcing cipher renegotiation */
  int  nTlsRenegotiationCount; 

  /* Max time to wait for client to respond to renegotiation request
   * Not currently used.
   */
  int  nTlsRenegotiationMsTimeout;                       

  /* Max time in milliseconds to wait for TLS connect handshake to complete */
  int  tlsHandshakeMsTimeout;                             

  /* File containing DH parameters for TLS cipher suites */
  char dhFileName[TMWTARG_CRYPTO_ID_LEN];                 
#endif

  /* Allows binding to a specific interface */
  TMWTYPES_CHAR   nicName[TMWTARG_IF_NAME_LENGTH];
  TMWTARG_IP_VERSION ipVersion;
} TMWTARGTCP_CONFIG;

/* The following macros will use the safe string functions
 * available when compiling for windows
 * Modify these as necessary for your target
 */
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#define STRCPY(dest, size, source) \
  strcpy_s(dest,size,source)

#define STRNCPY(dest, size, source, length) \
  strncpy_s(dest, size, source, length)

#define STRCAT(dest, size, source) \
    strcat_s(dest, size, source);

#else
/* If not using windows safe functions */
#define STRCPY(dest, size, source) \
  strncpy(dest, source, size)

#define STRNCPY(dest, size, source, length) \
  strncpy(dest, source, length)

#define STRCAT(dest, size, source) \
    strcat(dest, source);
#endif



/* Define a callback used by the target layer to tell the
 * source code library (SCL) that the channel connection has been
 * asynchronously opened or closed.
 *  NOTE: This function should only be called after the SCL calls
 *   tmwtarg_openChannel and before it calls tmwtarg_closeChannel.
 *
 * arguments:
 *  pCallbackParam - parameter passed by the SCL to tmwtarg_initChannel
 *   in the target configuration data structure (TMWTARG_CONFIG).
 *  openOrClose -
 *   TMWDEFS_TRUE indicates that the connection has asynchronously opened
 *    - ie the connection has opened after the previous call to
 *    tmwtarg_openChannel returned TMWDEFS_FALSE
 *   TMWDEFS_FALSE indicates that the connection has asynchronously closed
 *    - ie the connection has closed after the previous call to
 *    tmwtarg_openChannel returned TMWDEFS_TRUE
 *  reason - used to determine diagnostic message when openOrClose==TMWDEFS_FALSE
 */
typedef void (*TMWTARG_CHANNEL_CALLBACK_FUNC)(
  void *pCallbackParam,
  TMWTYPES_BOOL openOrClose,
  TMWDEFS_TARG_OC_REASON reason);

/* Define a callback used by the target channel I/O to tell the
 * source code library that the channel is ready to transmit data.
 * This should be called by the target layer after the call to
 * tmwtarg_getTransmitReady returned a non zero value to the SCL
 */
typedef void (*TMWTARG_CHANNEL_READY_CBK_FUNC)(
  void *pCallbackParam);

/* Define a callback used by the target channel I/O to tell the
 * source code library that the channel has received data
 */
typedef void (*TMWTARG_CHANNEL_RECEIVE_CBK_FUNC)(
  void *pCallbackParam);

/* Channel Operation Function Delcarations */
struct TmwtargIOChannel;
typedef struct TmwtargIOChannel TMWTARG_IO_CHANNEL;

typedef TMWTYPES_BOOL(*TMWTARG_CHANNEL_OPEN_FUNC)(
  TMWTARG_IO_CHANNEL *pTargIoChannel);

typedef TMWTYPES_MILLISECONDS(*TMWTARG_CHANNEL_XMIT_READY_FUNC)(
  TMWTARG_IO_CHANNEL *pTargIoChannel);

typedef TMWTYPES_BOOL(*TMWTARG_CHANNEL_XMIT_FUNC)(
  TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT numBytes);

typedef TMWTYPES_BOOL(*TMWTARG_CHANNEL_XMIT_UDP_FUNC)(
  TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_UCHAR UDPPort,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT numBytes);

typedef TMWTYPES_USHORT(*TMWTARG_CHANNEL_RECV_FUNC)(
  TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT maxBytes,
  TMWTYPES_MILLISECONDS maxTimeout,
  TMWTYPES_BOOL *pInterCharTimeoutOccurred);

typedef void(*TMWTARG_CHANNEL_CHECK_INPUT_FUNC)(
  TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_MILLISECONDS timeout);

typedef void(*TMWTARG_CHANNEL_CLOSE_FUNC)(
  TMWTARG_IO_CHANNEL *pTargIoChannel);

/*
 * The incoming message is for this channel
 *  TMWPHYS_ADDRESS_MATCH_SUCCESS=0,
 *
 * The incoming message may be for this channel,
 * so far the bytes match this protocol,
 * but more bytes are needed to tell if the address matches.
 *  TMWPHYS_ADDRESS_MATCH_MAYBE,
 *
 * The incoming message is not for this channel
 *  TMWPHYS_ADDRESS_MATCH_FAILED
*/
typedef TMWPHYS_ADDRESS_MATCH_TYPE TMWTARG_ADDRESS_MATCH_TYPE;

/* Define a callback used by the target channel I/O to ask if
 * a received message is meant for this channel. This can be used
 * by modem pools to determine which channel to connect an incoming
 * call to.
 */
typedef TMWTARG_ADDRESS_MATCH_TYPE (*TMWTARG_CHECK_ADDRESS_FUNC)(
  void *pCallbackParam,
  TMWTYPES_UCHAR *buf,
  TMWTYPES_USHORT numBytes,
  TMWTYPES_MILLISECONDS firstByteTime);

typedef struct TMWTargConfigStruct {
  /*
   * Specifies the amount of time (in character times) to use to
   * determine that a frame has been completed.  For modbus RTU this
   * value is 3.5 (i.e. 4 will be used)
   */
  TMWTYPES_USHORT numCharTimesBetweenFrames;

  /*
   * Specifies the amount of time to use to
   * determine that an inter character timeout has occurred.
   * For modbus RTU this value is 1.5 character times (i.e. 2 would be used)
   */
  TMWTYPES_USHORT interCharTimeout;

  /* The following 4 callback parameters are set by the SCL to allow the
   *  target layer to callback to the SCL. These should NOT be set by
   *  the user.
   */

  /*
   *  pChannelCallback - function that should be called if the channel
   *   is asynchronously opened or closed (from outside the source code
   *   library). Support for this parameter is optional for most protocols
   *   but recommended for target devices that support asynchronous notification.
   *   For IEC 60870-5-104 this support for this function is required so the SCL
   *   can maintain proper sequence numbers.
   *   The callback can also be called if a low level read or write fails
   *   in the target code as a result of a port being indirectly closed.
   *   This will force the SCL to close the channel and immediately start
   *   trying to reopen it.
   */
  TMWTARG_CHANNEL_CALLBACK_FUNC pChannelCallback;
  /*
   *  pCallbackParam - parameter to be passed to channel callback
   */
  void *pCallbackParam;

  /*
   *  pChannelReadyCallback - function that may be called to tell the
   *   source code library that the channel is ready to transmit data.
   *   This callback function may be called by the target layer after the call
   *   to tmwtarg_getTransmitReady returns a non zero value to the SCL.
   *   If the target layer does not call this callback function the SCL
   *   will retry after the amount of time indicated by tmwtarg_getTransmitReady
   */
  TMWTARG_CHANNEL_READY_CBK_FUNC pChannelReadyCallback;
  /*
   * pChannelReadyCbkParam - parameter to be passed to channel ready callback.
   */
  void *pChannelReadyCbkParam;

  /* This is a pointer to the source code library channel */
  TMWCHNL *pChannel;

  /* Number of Milliseconds to wait before retrying to establish the connection.*/
  TMWTYPES_MILLISECONDS connectRetry;
} TMWTARG_CONFIG;


/*
 * Holds values related to the target that have been read from a binary 
 * configuration file. (Only used with DNP3).
*/
typedef struct TMWBinFileTargValuesStruct {

  TMWTYPES_BOOL sessionIsOutstation;
  TMWTYPES_BOOL binFileIsOutstation;

  /*1.1.13*/
  TMWTYPES_BOOL useSupportedComm1_1_13;
  TMWTYPES_BOOL supportsSerialConn;
  TMWTYPES_BOOL supportsTCPConn;

  /*1.2.1*/
  TMWTYPES_BOOL useSerialPortName1_2_1;
  TMWTYPES_CHAR serialPortName[TMWTARG_STR_LEN];

  /*1.2.3*/
  TMWTYPES_BOOL useBuadRate1_2_3;
  TMWTYPES_CHAR baudRate[TMWTARG_STR_LEN];

  /*1.2.4 - flow control*/


  /*1.2.6*/
  /*TMWTYPES_BOOL supportsCollisionAvoidance; */
  /*TMWTYPES_MILLISECONDS minBackOffTime; */
  /*TMWTYPES_MILLISECONDS maxRandBackOffTime; */

  /*1.2.7*/
  /*TMWTYPES_BOOL checksRxInterCharGap; */
  /*TMWTYPES_BOOL interRxCharGapAllowed; */
  /*TMWTYPES_MILLISECONDS receiverInterCharTimeout; */

  /*1.2.8*/
  /*TMWTYPES_BOOL txInterCharGaps; */

  /*1.3.1*/
  TMWTYPES_BOOL useIpPortName1_3_1;
  TMWTYPES_CHAR ipPortName[TMWTARG_STR_LEN];

  /*1.3.2*/
  TMWTYPES_BOOL useEndpoint1_3_2;
  TMWTYPES_BOOL endpointIsTcpInitiating;
  TMWTYPES_BOOL endpointIsTcpListening;
  TMWTYPES_BOOL endpointIsTcpDual;
  TMWTYPES_BOOL endpointIsUDPDatagram;

  /*1.3.3*/
  /* IP Address of this device */
  TMWTYPES_BOOL useIpAddress1_3_3;
  TMWTYPES_CHAR ipAddress[TMWTARG_STR_LEN];

  /*1.3.4*/
  TMWTYPES_BOOL useSubnetMask1_3_4;
  TMWTYPES_CHAR subnetMask[TMWTARG_STR_LEN];

  /*1.3.5*/
  TMWTYPES_BOOL useGateWayIp1_3_5;
  TMWTYPES_CHAR gatewayIp[TMWTARG_STR_LEN];

  /*1.3.7*/
  /* Accepts TCP Connections or UDP Datagrams from IPs in the list
   * IPs separated by ";".  Could be *.*.*.*
   * Schema calls this IP Address of Remote Device
   * For master or dual end point this is the address to connect to.
   */
  TMWTYPES_BOOL useAllowedConnIpList1_3_7;
  TMWTYPES_CHAR allowedConnIpList[TMWTARG_STR_LEN];

  /*1.3.8*/
  TMWTYPES_BOOL useTcpListenPort1_3_8;
  TMWTYPES_USHORT tcpListenPort;

  /*1.3.9*/
  TMWTYPES_BOOL useTcpListenPortOfRemote1_3_9;
  TMWTYPES_USHORT tcpListenPortOfRemote;

  /*1.3.11*/
  TMWTYPES_BOOL useLocalUdpPort1_3_11;
  TMWTYPES_USHORT localUdpPort;

  /*1.3.12*/
  /* used by masters only*/
  TMWTYPES_BOOL useDestUpdPort1_3_12;
  TMWTYPES_USHORT destUdpPort;

  /*1.3.13*/
  /* used by outstations only*/
  TMWTYPES_BOOL useDestUdpPortForUnsol1_3_13;
  TMWTYPES_USHORT destUdpPortForUnsol;

  /*1.3.14*/
  /* used by outstations only*/
  TMWTYPES_BOOL useDestUdpPortForResponses1_3_14;
  TMWTYPES_USHORT destUdpPortForResponses;
  TMWTYPES_BOOL useSourcePortNumberForResponses;

} TMWTARG_BINFILE_VALS;

typedef enum TmwTargThreadState
{
  TMWTARG_THREAD_IDLE,
  TMWTARG_THREAD_EXITED,
  TMWTARG_THREAD_RUNNING,
  TMWTARG_THREAD_EXITING,
} TMWTARG_THREAD_STATE;

#ifdef __cplusplus
extern "C" {
#endif

#if TMWCNFG_SUPPORT_THREADS

  /* If multiple threads executing the SCL is to be supported, support for
   * locking critical resources must be provided. This lock function will be
   * called by the SCL before accessing a common resource. The SCL requires
   * the ability to prevent other threads from acquiring the same lock, but
   * may make nested calls to lock the same resource. If a binary semaphore
   * is the only native mechanism available this may have to be enhanced to
   * provide a counting semaphore.
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg__lockInit(TMWDEFS_RESOURCE_LOCK *pLock);
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg__lockSection(TMWDEFS_RESOURCE_LOCK *pLock);
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg__unlockSection(TMWDEFS_RESOURCE_LOCK *pLock);
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg__lockDelete(TMWDEFS_RESOURCE_LOCK *pLock);
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg__lockShare(TMWDEFS_RESOURCE_LOCK *pLock,
                                                         TMWDEFS_RESOURCE_LOCK *pLock1);

  #define TMWTARG_LOCK_INIT(lock)         tmwtarg__lockInit(lock)
  #define TMWTARG_LOCK_SECTION(lock)      tmwtarg__lockSection(lock)
  #define TMWTARG_UNLOCK_SECTION(lock)    tmwtarg__unlockSection(lock)
  #define TMWTARG_LOCK_DELETE(lock)       tmwtarg__lockDelete(lock)

  /* This function is only required for 104 redundancy with a multi-threaded
   * architecture. It will allow the use of a single lock for the redundancy
   * group as well as the redundant connection channels.
   * NOTE: It does not need to be implemented if 104 redundancy with a
   * multi-threaded architecture is not being used.
   */
  #define TMWTARG_LOCK_SHARE(lock, lock1) tmwtarg__lockShare(lock, lock1)

#else

  #define TMWTARG_LOCK_INIT(lock)           ((void) 0)
  #define TMWTARG_LOCK_SECTION(lock)        ((void) 0)
  #define TMWTARG_UNLOCK_SECTION(lock)      ((void) 0)
  #define TMWTARG_LOCK_DELETE(lock)         ((void) 0)

  /* This function is only required for 104 redundancy with a multi-threaded
   * architecture. It will allow the use of a single lock for the redundancy
   * group as well as the redundant connection channels.
   * Copy the information for the lock into lock1 so that the same lock is used
   * for both structures. This cannot be the lock itself. It would typically
   * be a pointer or an index or a reference to the lock.
   * NOTE: This does not need to be implemented if 104 redundancy with a
   * multi-threaded architecture is not being used.
   */
  #define TMWTARG_LOCK_SHARE(lock, lock1)   ((void) 0)
#endif


typedef struct TmwtargIOChannel {
  TMWTARGIO_TYPE_ENUM               type;
  TMWTARG_CHANNEL_STATE             chanState;
  TMWCHNL                          *pChannel;
  void                             *pChannelInfo;
  TMWTYPES_CHAR                     chanInfoBuf[64];
  TMWTYPES_CHAR                    *pChannelName;

  /* Callback function for this channel */
  TMWTARG_CHANNEL_CALLBACK_FUNC     pChannelCallback;      /* From TMWTARG_CONFIG */
  void                             *pChannelCallbackParam; /* From TMWTARG_CONFIG */
  TMWTARG_CHANNEL_READY_CBK_FUNC    pChannelReadyCallback; /* From TMWTARG_CONFIG */
  void                             *pChannelReadyCbkParam; /* From TMWTARG_CONFIG */
  TMWTARG_CHANNEL_RECEIVE_CBK_FUNC  pReceiveCallbackFunc;  /* From openChannel    */
  TMWTARG_CHECK_ADDRESS_FUNC        pCheckAddrCallbackFunc;/* From openChannel    */
  void                             *pCallbackParam;        /* From openChannel, 
                                                            * used by both above  */

  /* Channel Operation Functions */
  TMWTARG_CHANNEL_OPEN_FUNC         pOpenFunction;
  TMWTARG_CHANNEL_XMIT_READY_FUNC   pXmitReadyFunction;
  TMWTARG_CHANNEL_XMIT_FUNC         pXmitFunction;
  TMWTARG_CHANNEL_XMIT_UDP_FUNC     pXmitUdpFunction;
  TMWTARG_CHANNEL_RECV_FUNC         pRecvFunction;
  TMWTARG_CHANNEL_CHECK_INPUT_FUNC  pCheckInputFunction;
  TMWTARG_CHANNEL_CLOSE_FUNC        pCloseFunction;
  TMWTARG_CHANNEL_CLOSE_FUNC        pDeleteFunction;

  TMWTYPES_BOOL                     polledMode;
  TMWTARG_THREAD_STATE              chanThreadState;

#if TMWCNFG_SUPPORT_THREADS
  TMW_ThreadId                      chanThreadHandle;
#endif
  TMWTYPES_MILLISECONDS time_connect;
} TMWTARG_IO_CHANNEL;

  /* function: tmwtarg_alloc
   * purpose:  Allocate memory. This function will only be called if
   * TMWCNFG_USE_DYNAMIC_MEMORY is TMWDEFS_TRUE.
   * arguments:
   *  numBytes - number of bytes requested
   * returns:
   *  pointer to allocated memory if successful
   *  TMWDEFS_NULL if unsuccessful
   */
  TMWDEFS_SCL_API void *TMWDEFS_GLOBAL tmwtarg_alloc(TMWTYPES_UINT numBytes);

  /* function: tmwtarg_calloc
   * purpose:  Allocates storage space for an array of num elements, each of
   *  length size bytes. Each element is initialized to 0. This function will
   *  only be called if TMWCNFG_USE_DYNAMIC_MEMORY is TMWDEFS_TRUE.
   * arguments:
   *  num - number of items to alloc
   *  size - size of each item
   * returns:
   *  pointer to allocated memory if successful
   *  TMWDEFS_NULL if unsuccessful
   */
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL tmwtarg_calloc(TMWTYPES_UINT num,
                                                       TMWTYPES_UINT size);

  /* function: tmwtarg_free
   * purpose:  Free memory allocated by tmwtarg_alloc()
   * arguments:
   *  pBuf - pointer to buffer to be freed
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg_free(void *pBuf);

  /* function: tmwtarg_snprintf
   * purpose: Write formatted data to a string.
   * arguments:
   *  buf - Storage location for output
   *  count - Maximum number of characters that can be stored in buf
   *  format - Format-control string
   *  ... - Optional arguments
   * returns:
   *  TMWTYPES_INT - returns the number of bytes stored in buffer,
   *  not counting the terminating null character
   */
  TMWDEFS_SCL_API TMWTYPES_INT TMWDEFS_GLOBAL tmwtarg_snprintf(
    TMWTYPES_CHAR *buf,
    TMWTYPES_UINT count,
    const TMWTYPES_CHAR *format,
    ...);

#if TMWCNFG_SUPPORT_DIAG
  /* function: tmwtarg_putDiagString
   * purpose: Display a string of characters. This routine is used
   *  to display diagnostic information from the source code library
   *  if desired.
   * arguments:
   *  pAnlzId - pointer to structure containing information about where
   *   and why this message originated.
   *  pString - pointer to null terminated character string to display
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg_putDiagString(
    const TMWDIAG_ANLZ_ID *pAnlzId,
    const TMWTYPES_CHAR *pString);
#endif

  /* function: tmwtarg_getMSTime
   * purpose: Return the current value of a continuously running
   *  millisecond timer.
   * arguments:
   *  none
   * returns:
   *  Current value of millisecond clock.
   */
  TMWDEFS_SCL_API TMWTYPES_MILLISECONDS TMWDEFS_GLOBAL tmwtarg_getMSTime(void);

  /* function: tmwtarg_getDateTime
   * purpose: Return the current date and time. Some protocols (ie DNP3)
   *  are required to use UTC time or if devices span multiple time zones
   *  it may be recommended to use UTC time. This function should either return
   *  local or UTC time as desired.
   * arguments:
   *  pDateTime - structure into which to store the current date and time
   *  pDateTime->pSession will point to a TMWSESN structure or TMWDEFS_NULL
   *    this allows target layer to return time on a per session basis.
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg_getDateTime(
    TMWDTIME *pDateTime);

  /* function: tmwtarg_setDateTime
   * purpose: Set the current date and time. This function will only be
   *  called from a slave session as a result of a clock synchronization
   *  request.
   * arguments:
   *  pDateTime - pointer to structure containing new time
   * returns:
   *  TMWTYPES_BOOL - true if success
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtarg_setDateTime(
    TMWDTIME *pDateTime);

#if !TMWCNFG_MULTIPLE_TIMER_QS
  /* function: tmwtarg_startTimer()
   * purpose: Start a timer that will call the specified  callback function
   *  in 'timeout' milliseconds. Only a single event timer is required by the
   *  source code library.
   * arguments:
   *  timeout - number of milliseconds to wait
   *    This value can be zero. In that case the timer should call the callback
   *    function as soon as possible.
   *    NOTE: If this value is too large for the timer implementation, a timer
   *    with the largest supported value should be started. When the callback
   *    is called for the lesser value, the SCL will start another timer with
   *    the remaining time.
   *  pCallbackFunc - function to call when timer expires
   *  pCallbackParam - user specified callback parameter
   * returns:
   *  void
   *   NOTE: since it is not possible for this function to return failure it is
   *    important that a timer is started. If the timer cannot be started you
   *    should log this in some way or generate an exception since the SCL timers
   *    may not function after this failure. Calling the callback function sooner
   *    than asked for will cause the SCL to call this function again with the
   *    remaining time.
   */
  void TMWDEFS_GLOBAL tmwtarg_startTimer(
    TMWTYPES_MILLISECONDS timeout,
    TMWTYPES_CALLBACK_FUNC pCallbackFunc,
    void *pCallbackParam);

  /* function: tmwtarg_cancelTimer
   * purpose: Cancel current timer
   * arguments:
   *  none
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL tmwtarg_cancelTimer(void);

#else
  /* if TMWCNFG_MULTIPLE_TIMER_QS are supported */

  /* function: tmwtarg_initMultiTimer()
   * purpose: Create a timer for this channel. This will be a periodic
   *  timer that will be used to call the specified callback function
   *  in 'timeout' milliseconds. One timer will be required per channel.
   *  This is used when multiple timer queues (one per thread) are supported.
   * arguments:
   *  pChannel - pointer to channel to initialize the MultiTimer.
   * returns:
   *  TMWTYPES_INT - 0 if successful, error code on failure.
   */
  TMWTYPES_INT TMWDEFS_GLOBAL tmwtarg_initMultiTimer(
    TMWCHNL                *pChannel);

  /* function: tmwtarg_setMultiTimer()
   * purpose: Set the timeout period for this channel.
   * arguments:
   *  pChannel - pointer to channel to start the MultiTimer.
   *  timeout - number of milliseconds to wait.
   *    A value of zero will cancel the timer.
   *    NOTE: If this value is too large for the timer implementation, a timer
   *    with the largest supported value should be started. When the callback is
   *    called for the lesser value, the SCL will start another timer with the
   *    remaining time.
   * returns:
   *  TMWTYPES_INT - 0 if successful, error code on failure.
   *   NOTE: If the timer cannot be started you should log this in some way or
   *    generate an exception since the SCL timers may not function after this
   *    failure. Calling the callback function sooner than asked for will cause
   *    the SCL to call this function again with the remaining time.
   */
  TMWTYPES_INT TMWDEFS_GLOBAL tmwtarg_setMultiTimer(
    TMWCHNL                *pChannel,
    TMWTYPES_MILLISECONDS   timeout);

  /* function: tmwtarg_deleteMultiTimer
   * purpose: Delete the timer for this channel.
   * arguments:
   *  pChannel - pointer to channel to delete the MultiTimer
   * returns:
   *  TMWTYPES_INT - 0 if successful, error code on failure.
   */
  TMWTYPES_INT TMWDEFS_GLOBAL tmwtarg_deleteMultiTimer(
    TMWCHNL *pChannel);
#endif

  /* function: tmwtarg_exit
   * purpose: Application is notifying the target layer that it is exiting.
   *  Target layer MAY choose to deallocate memory or other resources.
   * arguments:
   *  none
   * returns:
   *  void
   */
  void tmwtarg_exit(void);

  /* function: tmwtarg_initChannel
   * purpose: Initialize a communications channel. This routine creates
   *  a communications channel as specified in the pConfig argument. The
   *  channel does not need to be opened as this will be accomplished in
   *  the tmwtarg_openChannel function described below. This routine
   *  returns a user defined context which is passed to all successive
   *  calls for this channel. The contents of the context are not used
   *  by the TMW SCL and are defined as required by the target interface.
   * arguments:
   *  pUserConfig - Pointer to configuration data passed to the TMW
   *   physical layer code. This data is not used by the TMW code
   *   and should be used by the target routines to identify and
   *   configure the communications channel.
   *  pTmwConfig - TMW target configuration data structure
   *  pChannel - pointer to channel

   * returns:
   *  void * channel context
   *   The channel context is a target-defined context that
   *   will be passed to all of the remaining channel target functions.
   *   The source code library does not change or manipulate this
   *   pointer in any way. The pointer cannot be NULL since this
   *   is interpreted as a failure.
   */
  void * TMWDEFS_GLOBAL tmwtarg_initChannel(
    const void *pUserConfig,
    TMWTARG_CONFIG *pTmwConfig,
    TMWCHNL *pChannel);

  /* function: tmwtarg_stopThreads
   * purpose: Stop any threads running on this communications channel in
   *  anticipation of tmwtarg_deleteChannel being called. This allows any
   *  threads that might be looking for connections or received data from
   *  calling back into the library and contending for a channel critical
   *  section lock.
   * arguments:
   *  pContext - Context returned from call to tmwtarg_initChannel
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg_stopThreads(
      void *pContext);

  /* function: tmwtarg_deleteChannel
   * purpose: Delete a communications channel. This routine should
   *  delete a communications channel and free all associated memory
   *  and resources.
   * arguments:
   *  pContext - Context returned from call to tmwtarg_initChannel
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL tmwtarg_deleteChannel(
    void *pContext);

  /* function: tmwtarg_getChannelName
   * purpose: Returns the name for this channel
   *  For Diagnostic Purposes only.
   * description: This method allows the target to return an appropriate
   *  name for this channel. Typically this would be something out of the
   *  configuration information passed to the tmwtarg_initChannel routine.
   * arguments:
   *  pContext - Channel context returned from call to tmwtarg_initChannel
   * returns: pointer to a null terminated string which contains the
   *  name.
   */
  TMWDEFS_SCL_API const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwtarg_getChannelName(
    void *pContext);

  /* function: tmwtarg_getChannelInfo
   * purpose: Return configuration information for this channel
   * description: This method allows the target to return a user defined
   *  information string to be displayed when the channel is opened.
   *  typically this would contain formatted information about the
   *  channel configuration and/or status.
   * arguments:
   *  pContext - Context returned from call to tmwtarg_initChannel
   * returns:
   *  Pointer to a null terminated string which contains the name.
   */
  const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwtarg_getChannelInfo(
    void *pContext);

  /* function: tmwtarg_openChannel
   * purpose: Open a communications channel.
   *  If this was over TCP/IP this function would attempt to listen
   *  or make the connection. This function should return TMWDEFS_TRUE when
   *  the connection was successfully set up. With an RS232 port this is
   *  probably when the port is opened. With a TCP Server where a listen would
   *  be performed, or a client where a connect request is sent out, this
   *  function should return TMWDEFS_FALSE until the connection is complete.
   *  If this function returns TMWDEFS_FALSE this function will be called
   *  periodically to try to connect. You can also call the pChannelCallback
   *  function, that was passed into tmwtarg_initChannel in the TMWTARG_CONFIG
   *  structure, to indicate the connection has been completed. This will cause
   *  the library to call tmwtarg_openChannel again to allow you to return
   *  TMWDEFS_TRUE. Until tmwtarg_openChannel gets TMWDEFS_TRUE as a return value
   *  the library will not consider the channel is open/connected.
   * arguments:
   *  pContext - Context returned from call to tmwtarg_initChannel
   *  pReceiveCallbackFunc - Function to be called when data is available
   *   to be read if using event driven rather than polled mode. Most
   *   implementations will not need to call this function. This is used for
   *   event driven I/O. NOTE: This callback should not be called until
   *   tmwtarg_openChannel has returned TMWDEFS_TRUE.
   *  pCheckAddrCallbackFunc - Function to be called to determine if this
   *   received data is intended for this channel. This is only supported for
   *   DNP and 101/103. It is intended to provide support for modem pool
   *   implementations for incoming unsolicited messages. Most implementations
   *   will not need to call this function. Note: this callback should not be
   *   called until tmwtarg_openChannel has returned TMWDEFS_TRUE.
   *  pCallbackParam - parameter to be passed to both the pReceivedCallbackFunc
   *   and pCheckAddrCallbackFunc.
   * returns:
   *  TMWDEFS_TRUE if connected (read description in purpose:),
   *  else TMWDEFS_FALSE if connection is not yet completed.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtarg_openChannel(
    void *pContext,
    TMWTARG_CHANNEL_RECEIVE_CBK_FUNC pReceiveCallbackFunc,
    TMWTARG_CHECK_ADDRESS_FUNC pCheckAddrCallbackFunc,
    void *pCallbackParam);

  /* function: tmwtarg_closeChannel
   * purpose: Close a communications channel
   * arguments:
   *  pContext - Context returned from call to tmwtarg_initChannel
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL tmwtarg_closeChannel(
    void *pContext);

  /* function: tmwtarg_getSessionName
   * purpose: Returns the name for this session
   *  For Diagnostic Purposes only.  Registration function also provided.
   * description: This method allows the target to return an appropriate
   *  name for this session. This function could just return the name for the
   *  pSession->pChannel if names are not maintained per session.
   * arguments:
   *  pSession - pointer to session returned by xxxsesn_openSession()
   * returns: pointer to a null terminated string which contains the
   *  name.
   */
  TMWDEFS_SCL_API const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwtarg_getSessionName(
    TMWSESN *pSession);

  /* function: tmwtarg_getSectorName
   * purpose: Returns the name for this sector
   *  For Diagnostic Purposes only.  Registration function also provided.
   * description: This method allows the target to return an appropriate
   *  name for this sector. This function could just return the name for the
   *  pSector->pSession or pSector->pChannel if names are not maintained per sector.
   * arguments:
   *  pSession - pointer to session returned by xxxsesn_openSector()
   * returns: pointer to a null terminated string which contains the
   *  name.
   */
  TMWDEFS_SCL_API const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwtarg_getSectorName(
    TMWSCTR *pSector);


  /* function: tmwtarg_isChannelOpen
   * purpose: Determine whether a channel is open/connected
   * arguments:
   *  pContext - Context returned from call to tmwtarg_initChannel
   * returns:
   *  true if channel is open/connected
   */
  TMWTYPES_BOOL tmwtarg_isChannelOpen(void *pContext);

  /* function: tmwtarg_getTransmitReady
   * purpose: Determine whether a channel is ready to transmit or not.
   *  This routine can be used to delay transmission until various
   *  target related dependencies have been satisfied. A common
   *  example is modem setup time.
   * arguments:
   *  pContext - Context returned from call to tmwtarg_initChannel
   * returns:
   *  0 if channel is ready to transmit,
   *  non-zero, if channel is not OK to transmit. This value will indicate
   *  the number of milliseconds the SCL should wait before calling this
   *  function again for this channel. If the SCL has registered a
   *  TMWTARG_CHANNEL_READY_CBK_FUNC callback function the target layer may
   *  call this callback function if the channel is ready sooner than
   *  this return value would indicate. If the callback function is not
   *  called the SCL will retry this channel in the number of milliseconds
   *  returned by this function.
   */
  TMWTYPES_MILLISECONDS TMWDEFS_GLOBAL tmwtarg_getTransmitReady(
    void *pContext);

  /* function: tmwtarg_receive
   * purpose: Receive bytes from the specified channel
   * arguments:
   *  pContext - Context returned from call to tmwtarg_initChannel
   *  pBuff - Buffer into which to store received bytes
   *  maxBytes - The maximum number of bytes to read
   *  maxTimeout - maximum time to wait in milliseconds for input
   *   from this channel.
   *  pInterCharTimeoutOccurred - TMWDEFS_TRUE if an intercharacter
   *   timeout occurred while receiving bytes. This is an optional
   *   timeout that can be implemented in the target to terminate
   *   a frame if too much time passes between receipt of bytes
   *   in a frame.
   *  pFirstByteTime - pointer to variable to be filled in indicating
   *   the time the first byte of message was received. If this is left
   *   unchanged the SCL will determine what time it saw the first byte.
   *   This can be set by calling tmwtarg_getMSTime() or its equivalent.
   * returns:
   *  The number of bytes actually read.
   * NOTES:
   *  - The Source Code Library will usually use a timeout value of 0;
   *    This indicates the call to tmwtarg_receive should be nonblocking
   *    (i.e., return 0 if no bytes are available.)
   *  - For Modbus RTU this function should not return any bytes
   *    until either the entire frame was received or an inter Character Timeout
   *    occurred. If you are implementing multiple protocols, one of which is
   *    Modbus RTU, then the pContext structure should include a flag that
   *    indicates whether full frames are required. The target implementation
   *    of tmwtarg_receive can use this indicator to ensure that it returns
   *    the entire frame for Modbus RTU. Other protocols can use this
   *    indicator to allow them to return any number of bytes actually
   *    read.
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL tmwtarg_receive(
    void *pContext,
    TMWTYPES_UCHAR *pBuff,
    TMWTYPES_USHORT maxBytes,
    TMWTYPES_MILLISECONDS maxTimeout,
    TMWTYPES_BOOL *pInterCharTimeoutOccurred,
    TMWTYPES_MILLISECONDS *pFirstByteTime);

  /* function: tmwtarg_transmit
   * purpose: Transmit bytes on the specified channel
   * arguments:
   *  pContext - Context returned from call to tmwtarg_initChannel
   *  pBuff - Array of bytes to transmit
   *  numBytes - Number of bytes to transmit
   * returns:
   *  TMWDEFS_TRUE if all the bytes were successfully transmitted,
   *  else TMWDEFS_FALSE.
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtarg_transmit(
    void *pContext,
    TMWTYPES_UCHAR *pBuff,
    TMWTYPES_USHORT numBytes);

  /* function: tmwtarg_transmitUDP
   * purpose: Transmit bytes using UDP on the specified channel
   * arguments:
   *  pContext - Context returned from call to tmwtarg_initChannel
   *  UDPPort - This is a define that indicates the remote UDP port to
   *   transmit to.
   *    TMWTARG_UDP_SEND       - Send to the remote port to be used for
   *                             requests or responses
   *    TMWTARG_UDP_SEND_UNSOL - Send to the remote port to be used for
   *                             unsolicited responses.  Once outstation has
   *                             received a request from master this would be
   *                             same port as all responses.
   *    TMWTARG_UDPONLY_BROADCAST Send to the broadcast address when UDP ONLY
   *                             is configured.
   *  pBuff - Array of bytes to transmit
   *  numBytes - Number of bytes to transmit
   * returns:
   *  TMWDEFS_TRUE if all the bytes were successfully transmitted,
   *  else TMWDEFS_FALSE.
   * NOTE: This only needs to be implemented for DNP to support
   *  the DNP3 Specification IP Networking. It is not required
   *  for IEC or modbus and will not be called by those protocols.
   *  If DNP3 UDP is not required, this function can simply return TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtarg_transmitUDP(
    void *pContext,
    TMWTYPES_UCHAR UDPPort,
    TMWTYPES_UCHAR *pBuff,
    TMWTYPES_USHORT numBytes);

  /* Big Endian vs Little Endian
   * For all protocols currently supported by the Triangle MicroWorks
   * source code libraries the message byte order is least significant
   * byte first(LSB). The following get/store routines were rewritten
   * to allow them to work on either a LSB first (little-endian) or Most
   * Significant Byte first(MSB) processors. However, because of differences
   * in the way 64 bit floating point values are stored in memory, it may
   * be necessary to modify tmwtarg_get64 and tmwtarg_put64. (These functions
   * are currently only used by DNP for 64 bit floating point TMWTYPES_DOUBLE
   * and not by the IEC 60870-5 and modbus protocols).
   */

  /* function: tmwtarg_get8
   * purpose: retrieve a 8 bit value from a message
   * arguments:
   *  pSource - pointer to location in message buffer to copy bytes from
   *  pDest - pointer to location in memory to copy bytes to.
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg_get8(
    const TMWTYPES_UCHAR *pSource,
    TMWTYPES_UCHAR *pDest);

  /* function: tmwtarg_store8
   * purpose: store a 8 bit value into a message
   * arguments:
   *  pSource - pointer to location in message buffer to copy bytes from
   *  pDest - pointer to location in memory to copy bytes to.
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg_store8(
    const TMWTYPES_UCHAR *pSource,
    TMWTYPES_UCHAR *pDest);

  /* function: tmwtarg_get16
   * purpose: retrieve a 16 bit value from a message compensating
   *  for byte order.
   * arguments:
   *  pSource - pointer to location in message buffer to copy bytes from
   *  pDest - pointer to location in memory to copy bytes to.
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg_get16(
    const TMWTYPES_UCHAR *pSource,
    TMWTYPES_USHORT *pDest);

  /* function: tmwtarg_store16
   * purpose: store a 16 bit value into a message compensating
   *  for byte order.
   * arguments:
   *  pSource - pointer to location in memory to copy bytes from
   *  pDest - pointer to location in message buffer to copy bytes to
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg_store16(
    const TMWTYPES_USHORT *pSource,
    TMWTYPES_UCHAR *pDest);

  /* function: tmwtarg_get24
   * purpose: retrieve a 24 bit value from a message compensating
   *  for byte order.
   * arguments:
   *  pSource - pointer to location in message buffer to copy bytes from
   *  pDest - pointer to location in memory to copy bytes to
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg_get24(
    const TMWTYPES_UCHAR *pSource,
    TMWTYPES_ULONG *pDest);

  /* function: tmwtarg_store24
   * purpose: store a 24 bit value into a message compensating
   *  for byte order.
   * arguments:
   *  pSource - pointer to location in memory to copy bytes from
   *  pDest - pointer to location in message buffer to copy bytes to
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg_store24(
    const TMWTYPES_ULONG *pSource,
    TMWTYPES_UCHAR *pDest);

  /* function: tmwtarg_get32
   * purpose: retrieve a 32 bit value from a message compensating
   *  for byte order.
   * arguments:
   *  pSource - pointer to location in message buffer to copy bytes from
   *  pDest - pointer to location in memory to copy bytes to
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg_get32(
    const TMWTYPES_UCHAR *pSource,
    TMWTYPES_ULONG *pDest);

  /* function: tmwtarg_store32
   * purpose: store a 32 bit value into a message compensating
   *  for byte order.
   * arguments:
   *  pSource - pointer to location in memory to copy bytes from
   *  pDest - pointer to location in message buffer to copy bytes to
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg_store32(
    const TMWTYPES_ULONG *pSource,
    TMWTYPES_UCHAR *pDest);

  /* function: tmwtarg_get64
   * purpose: retrieve a 64 bit value from a message compensating
   *  for byte order.
   * arguments:
   *  pSource - pointer to location in message buffer to copy bytes from
   *  pDest - pointer to location in memory to copy bytes to
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg_get64(
    const TMWTYPES_UCHAR *pSource,
    TMWTYPES_DOUBLE *pDest);

  /* function: tmwtarg_store64
   * purpose: store a 64 bit value into a message compensating
   *  for byte order.
   * arguments:
   *  pSource - pointer to location in memory to copy bytes from
   *  pDest - pointer to location in message buffer to copy bytes to
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg_store64(
    const TMWTYPES_DOUBLE *pSource,
    TMWTYPES_UCHAR *pDest);

  /* function: tmwtarg_getSFloat
   * purpose: retrieve a 32 bit single precision floating point value from a
   *  message compensating for byte order (and floating point format if native
   *  format is not IEEE-754 format as required by DNP).
   * arguments:
   *  pSource - pointer to location in message buffer to copy bytes from
   *  pDest - pointer to location in memory to copy bytes to
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg_getSFloat(
    const TMWTYPES_UCHAR *pSource,
    TMWTYPES_SFLOAT *pDest);

  /* function: tmwtarg_storeSFloat
   * purpose: store a 32 bit single precision floating point value into a
   *  message compensating for byte order (and floating point format if native
   *  format is not IEEE-754 format as required by DNP).
   * arguments:
   *  pSource - pointer to location in memory to copy bytes from
   *  pDest - pointer to location in message buffer to copy bytes to
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg_storeSFloat(
    const TMWTYPES_SFLOAT *pSource,
    TMWTYPES_UCHAR *pDest);

  /* function: tmwtarg_appendString
   * purpose: Append two string allocating new memory and freeing original
   *  string. This method is currently only required to support the generation
   *  of an XML document from the target database.
   * arguments:
   *  pStr1 - String to append to, call tmwtarg_free when done
   *  pStr2 - String to append to pStr1
   * returns:
   *  new string which contains str2 appended to str1
   */
  TMWTYPES_CHAR *tmwtarg_appendString(
    TMWTYPES_CHAR *pStr1,
    TMWTYPES_CHAR *pStr2);

  /* function: tmwtarg_initConfig
   * purpose: Initialize the TMW target layer.
   *  This routine should be called to initialize all the members of the
   *  data structure to the default values. The caller should then modify
   *  individual data fields as desired. The resulting structure will be
   *  passed as an argument to tmwtarg_initChannel
   * arguments:
   *  pConfig - pointer to target layer configuration data structure to
   *   be initialized
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg_initConfig(
    TMWTARG_CONFIG *pConfig);

  /* The following two functions are only required for if
   * DNPCNFG_SUPPORT_BINCONFIG is defined as TMWDEFS_TRUE
   */

  /* function: tmwtarg_initBinFileValues
   * purpose: initialize a struct of target values
   * arguments:
   *  pBinTargFileValues - pointer to struct that will hold target values 
   *   read from binary file
   * returns
   *  TRUE if successful
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtarg_initBinFileValues(
    TMWTARG_BINFILE_VALS *pBinTargFileValues);


  /* function: tmwtarg_applyBinFileTargValues
   * purpose: copy target values from a struct of values read from a 
   *  binary file to the target layer
   * arguments:
   *  pBinTargFileValues - pointer to struct holding target values read from binary file
   * returns
   *  TRUE if successful
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtarg_applyBinFileTargValues(
    void *pIoConfig,
    TMWTARG_BINFILE_VALS *pBinTargFileValues,
    TMWTYPES_BOOL *pIsChannelSerial);

  /* function: tmwtarg_sleep
   * purpose: suspends execution of the calling thread for specified number 
   *  of milliseconds
   * arguments:
   *  milliseconds - specifies number of milliseconds to suspend the calling thread
   * returns
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg_sleep(
    TMWTYPES_MILLISECONDS milliseconds);

  /* macro: ASSERT( booleanExpression )
   * purpose: Evaluates its argument. If the result is 0, the macro
   *          prints a diagnostic message and aborts the program.
   *          If the condition is nonzero, it does nothing.  The
   *          diagnostic message has the form
   *          'assertion failed in file <name> in line <num>'
   *          where name is the name of the source file, and num
   *          is the line number of the assertion that failed in
   *          the source file.
   * Note:    This functionality is available only if
   *          TMWCNFG_INCLUDE_ASSERTS is defined and we are
   *          compiling on the microsoft compiler.
   * arguments:
   *   booleanExpression - Specifies an expression (including pointer values) that
   *   evaluates to nonzero or 0.
   *
   * returns:
   *  void
   */

#if defined(TMW_PRIVATE) && defined(TMWCNFG_INCLUDE_ASSERTS) && defined(_MSC_VER)
  TMWDEFS_SCL_API void TMWAssertion(const char *expr, const char *file, int line);
  #ifdef ASSERT
    #undef ASSERT
  #endif
  #define ASSERT(expr) ((expr) ? ((void) 0) : TMWAssertion(#expr, __FILE__, __LINE__))
#else
  #ifdef ASSERT
    #undef ASSERT
  #endif
  #define ASSERT(expr) ((void) 0)
#endif /* TMW_PRIVATE && TMWCNFG_INCLUDE_ASSERTS */

#if defined(_MSC_VER)
/* macro __LOC__
 * purpose: used in #pragma message to place file name and line number
 *          in message.
 * arguments:
 *   none
 * example:
 *   #pragma message(__LOC__ "Is this a reasonable test?");
 * returns:
 *  void
 */
#ifndef __LOC__
#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "(" __STR1__(__LINE__) ") : Note: "
#endif

#endif


#ifdef __cplusplus
};
#endif

#endif /* TMWTARG_DEFINED */
