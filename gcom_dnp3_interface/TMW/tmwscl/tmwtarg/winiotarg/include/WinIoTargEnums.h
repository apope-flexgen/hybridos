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

/** \file WinIoTargEnums.h
 * Provides the definition of various enumerations for the WinIoTarg DLL. 
 */
#pragma once

/** used to specify the type of target to create */
typedef enum _WINIO_TYPE_ENUM 
{
  WINIO_TYPE_232,                   /*!< RS232 channel configuration */
  WINIO_TYPE_MODEM_POOL_CHANNEL,    /*!< Modem pool channel configuration */
  WINIO_TYPE_MODEM_POOL,            /*!< Moden pool configuration */
  WINIO_TYPE_MODEM,                 /*!< Modem configuration */
  WINIO_TYPE_TCP,                   /*!< TCP/IP channel configuration */
  WINIO_TYPE_UDP_TCP,               /*!< UDP/TCP channel configuration */
  WINIO_TYPE_MBP,                   /*!< Modbus plus channel configuration */
  WINIO_TYPE_MON,                   /*!< Monitor channel configuration */
  WINIO_TYPE_NONE                   /*!< No channel configuration */
} WINIO_TYPE_ENUM;

/** used to specify the type of open to do */
typedef enum _WINIO_OPEN_MODE_ENUM
{
  WINIO_OPEN_MODE_NONE,             /*!< nothing special */
  WINIO_OPEN_MODE_DIAL,             /*!< Dial the modem */
  WINIO_OPEN_MODE_DONT_DIAL         /*!< Don't dial the modem */
} WINIO_OPEN_MODE_ENUM;

/** current modem status/state */
typedef enum _WINIO_STATUS
  {
    MODEM_STATUS_REDIAL_ATTEMPTS_OVER_LIMIT,    /*!< redial failed */
    MODEM_STATUS_AVAILABLE,                     /*!< modem is available */
    MODEM_STATUS_CONNECTED,                     /*!< modem is connected */
    MODEM_STATUS_CARRIER_DETECT,                /*!< carrier detected */
    MODEM_STATUS_POOL_ADDED,                    /*!< modem pool added */
    MODEM_STATUS_POOL_REMOVED                   /*!< modem pool removed */
  } WINIO_STATUS;

/** parity for RS232 serial communications channel */
typedef enum _WIN232_PARITY
{
  WIN232_PARITY_NONE,   /*!< no parity */
  WIN232_PARITY_EVEN,   /*!< even  parity*/
  WIN232_PARITY_ODD     /*!< odd  parity*/
} WIN232_PARITY;

/** number of data bits to use for serial communications */
typedef enum _WIN232_DATA_BITS
{
  WIN232_DATA_BITS_7 = 7,   /*!< 7 data bits */
  WIN232_DATA_BITS_8        /*!< 8 data bits */
} WIN232_DATA_BITS;

/** number of stop bits to use for serial communications */
typedef enum _WIN232_STOP_BITS
{
  WIN232_STOP_BITS_1 = 1,   /*!< 1 stop bit */
  WIN232_STOP_BITS_2        /*!< 2 stop bits */
} WIN232_STOP_BITS;

/** flow control for serial communications */
typedef enum _WIN232_PORT_MODE
{
  WIN232_MODE_NONE,       /*!< no flow control */
  WIN232_MODE_HARDWARE,   /*!< hardware flow control */
  WIN232_MODE_WINDOWS     /*!< windows flow control */
} WIN232_PORT_MODE;

/* Note: these come from winbase.h */
////
//// DTR Control Flow Values.
////
//#define DTR_CONTROL_DISABLE    0x00
//#define DTR_CONTROL_ENABLE     0x01
//#define DTR_CONTROL_HANDSHAKE  0x02
//
////
//// RTS Control Flow Values
////
//#define RTS_CONTROL_DISABLE    0x00
//#define RTS_CONTROL_ENABLE     0x01
//#define RTS_CONTROL_HANDSHAKE  0x02
//#define RTS_CONTROL_TOGGLE     0x03


/** DTR mode for serial communications port when in WIN232_MODE_HARDWARE */
typedef enum _WIN232_DTR_MODE
{
  WIN232_DTR_DISABLE = 0,     /* Disables the DTR line when the device is opened and leaves it disabled. */
  WIN232_DTR_ENABLE = 1,      /* Enables the DTR line when the device is opened and leaves it on. */
  WIN232_DTR_HANDSHAKE = 2    /* Enables DTR handshaking. If handshaking is enabled, it is an error for the application to adjust the line by using the EscapeCommFunction function. */
} WIN232_DTR_MODE;

/** RTS mode for serial communications port when in WIN232_MODE_HARDWARE */
typedef enum _WIN232_RTS_MODE
{
  WIN232_RTS_DISABLE = 0,       /* Disables the RTS line when the device is opened and leaves it disabled. */
  WIN232_RTS_ENABLE = 1,         /* Enables the RTS line when the device is opened and leaves it on. */
  WIN232_RTS_HANDSHAKE = 2,   /* Enables RTS handshaking. The driver raises the RTS line when the "type-ahead" (input) buffer is less than one-half full and lowers the RTS line when the buffer is more than three-quarters full. If handshaking is enabled, it is an error for the application to adjust the line by using the EscapeCommFunction function. */
  WIN232_RTS_TOGGLE = 3          /* Specifies that the RTS line will be high if bytes are available for transmission. After all buffered bytes have been sent, the RTS line will be low. */
} WIN232_RTS_MODE;

/** pulse or tone dialing for a modem */
typedef enum _WINMODEM_DIALING_MODE
{
  WINMODEM_DIALING_MODE_TONE,   /*!< tone dialing */
  WINMODEM_DIALING_MODE_PULSE   /*!< pulse dialing */
} WINMODEM_DIALING_MODE;

/** character that terminates a modem command */
typedef enum _WINMODEM_RESP_CHAR
{
  WINMODEM_RESP_CHAR_NONE,   /*!< none */
  WINMODEM_RESP_CHAR_CR,     /*!< carriage return */
  WINMODEM_RESP_CHAR_LF      /*!< line feed */
} WINMODEM_RESP_CHAR;

/// <summary>TCP/IP client or server </summary>
typedef enum _WINTCP_MODE
{
  WINTCP_MODE_SERVER,           /// <summary>socket will listen for connection </summary>
  WINTCP_MODE_CLIENT,           /// <summary>socket will attempt to make a connection </summary>
  WINTCP_MODE_UDP,              /// <summary>UDP only, no TCP connection </summary>
  WINTCP_MODE_DUAL_ENDPOINT     /// <summary>both client and server functionality </summary>
} WINTCP_MODE;

/** DNP Networking master or outstation role. Dual End Point behavior concerning new connections when already connected differs */
typedef enum _WINTCP_ROLE
{
  WINTCP_ROLE_MASTER,        /*!< master, has priority on new connections */
  WINTCP_ROLE_OUTSTATION     /*!< outstation (slave) */
} WINTCP_ROLE;

/** Modbus Plus client or server */
typedef enum _WINMBP_MODE
{
  WINMBP_MODE_SERVER,   /*!< server */
  WINMBP_MODE_CLIENT    /*!< client */
} WINMBP_MODE;

/** provide system or simulated time */
typedef enum _WINIO_TIME_MODE
{
  WINIO_TIME_MODE_SYSTEM,               /*!< time is system time */
  WINIO_TIME_MODE_SYSTEM_NO_SETTIME,    /*!< do not allow set time */
  WINIO_TIME_MODE_SIMULATED,            /*!< time is simulated time */
  WINIO_TIME_MODE_SYSTEM_TIME_ZONE,      /*!< time is system time zone time */
  WINIO_TIME_MODE_SYSTEM_TIME_ZONE_NO_SETTIME      /*!< time is system time zone time with no set time supported*/
} WINIO_TIME_MODE;

