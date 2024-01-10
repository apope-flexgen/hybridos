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

/* file: twmtargos.h
 * description: Common definitions used by the SCL that are OS dependent.
 */
#ifndef tmwtargos_DEFINED
#define tmwtargos_DEFINED

#include "tmwscl/tmwtarg/WinioTarg/include/WinIoTargEnums.h"

/* Type for the handle that the OS passes back for the created thread */
#define TMW_ThreadId            unsigned int

/* Declaration for the entry point of a thread */
#define TMW_ThreadDecl          unsigned int __stdcall

/* The type definition of the entry point of a thread */
/* In most cases it will be the same as TMW_ThreadDecl */
#define TMW_ThreadPtr           unsigned int

/* The type defintion the OS expects to pass as an arguement when starting a thread */
#define TMW_ThreadArg           void *

/* A Macro is used here instead of a function call because of Windows __stdcall calling convention */
#define TMW_ThreadCreate(pThreadId, pFunc, funcArg, priority, stackSize)                       \
  HANDLE threadHandle;                                                                         \
  threadHandle = (HANDLE)_beginthreadex(NULL, 0, pFunc, funcArg, CREATE_SUSPENDED, pThreadId); \
  SetThreadPriority(threadHandle, THREAD_PRIORITY_NORMAL);                                     \
  ResumeThread(threadHandle); 

/* Nothing needs to be done in Windows, but is required in Linux */
#define TMW_ThreadDetach(pThreadId) TMWTARG_UNUSED_PARAM(pThreadId)

/* Ideally there would be a single enumerated type for all target layers.
* The Windows specific mapping is done to ensure backward compatibility and with .NET.
*/
typedef WINTCP_ROLE TMWTARGTCP_ROLE;
  #define  TMWTARGTCP_ROLE_MASTER       WINTCP_ROLE_MASTER
  #define  TMWTARGTCP_ROLE_OUTSTATION   WINTCP_ROLE_OUTSTATION

typedef  WINTCP_MODE  TMWTARGTCP_MODE;
  #define TMWTARGTCP_MODE_SERVER        WINTCP_MODE_SERVER
  #define TMWTARGTCP_MODE_CLIENT        WINTCP_MODE_CLIENT
  #define TMWTARGTCP_MODE_UDP           WINTCP_MODE_UDP
  #define TMWTARGTCP_MODE_DUAL_ENDPOINT WINTCP_MODE_DUAL_ENDPOINT

/** Ensure compatibility with WINIO_TYPE_ENUM */
typedef WINIO_TYPE_ENUM TMWTARGIO_TYPE_ENUM;
  #define TMWTARGIO_TYPE_232                WINIO_TYPE_232 
  #define TMWTARGIO_TYPE_MODEM_POOL_CHANNEL WINIO_TYPE_MODEM_POOL_CHANNEL
  #define TMWTARGIO_TYPE_MODEM_POOL         WINIO_TYPE_MODEM_POOL
  #define TMWTARGIO_TYPE_MODEM              WINIO_TYPE_MODEM
  #define TMWTARGIO_TYPE_TCP                WINIO_TYPE_TCP
  #define TMWTARGIO_TYPE_UDP_TCP            WINIO_TYPE_UDP_TCP
  #define TMWTARGIO_TYPE_MBP                WINIO_TYPE_MBP
  #define TMWTARGIO_TYPE_MON                WINIO_TYPE_MON
  #define TMWTARGIO_TYPE_NONE               WINIO_TYPE_NONE

/** number of data bits to use for serial communications */
typedef WIN232_DATA_BITS TMWTARG232_DATA_BITS;
  #define TMWTARG232_DATA_BITS_7  WIN232_DATA_BITS_7
  #define TMWTARG232_DATA_BITS_8  WIN232_DATA_BITS_8

/** number of stop bits to use for serial communications */
typedef WIN232_STOP_BITS TMWTARG232_STOP_BITS;
  #define TMWTARG232_STOP_BITS_1  WIN232_STOP_BITS_1
  #define TMWTARG232_STOP_BITS_2  WIN232_STOP_BITS_2

typedef TMWTYPES_CHAR TMWTARG_TYPE_BAUDRATE[256]; /* WINIOTARG_STR_LEN */

#define WINIOTARG_SUPPORT_LEGACY_CONFIG TMWDEFS_FALSE
#if WINIOTARG_SUPPORT_LEGACY_CONFIG
/* These definitions have been depricated in favor of OS independent values. TMW recommends updating
* applications to use the new definitions. However the depricated definitions can optionally be
* enabled here by defining WINIOTARG_SUPPORT_LEGACY_CONFIG.
*/
#define WinIoTarg_initConfig tmwtargio_initConfig
#define winTCP targTCP
#define win232 targ232
#define tmwtargp_Sleep tmwtarg_sleep

/** parity for RS232 serial communications channel */
typedef WIN232_PARITY TMWTARG232_PARITY;
  #define TMWTARG232_PARITY_NONE   WIN232_PARITY_NONE
  #define TMWTARG232_PARITY_EVEN   WIN232_PARITY_EVEN
  #define TMWTARG232_PARITY_ODD    WIN232_PARITY_ODD

/** flow control for serial communications */
typedef WIN232_PORT_MODE TMWTARG232_PORT_MODE;
  #define TMWTARG232_MODE_NONE     WIN232_MODE_NONE
  #define TMWTARG232_MODE_HARDWARE WIN232_MODE_HARDWARE
  #define TMWTARG232_MODE_WINDOWS  WIN232_MODE_WINDOWS

/** DTR mode for serial communications port when in TMWTARG232_MODE_WINDOWS */
typedef WIN232_DTR_MODE TMWTARG232_DTR_MODE;
  #define TMWTARG232_DTR_DISABLE   WIN232_DTR_DISABLE
  #define TMWTARG232_DTR_ENABLE    WIN232_DTR_ENABLE
  #define TMWTARG232_DTR_HANDSHAKE WIN232_DTR_HANDSHAKE

/** RTS mode for serial communications port when in TMWTARG232_MODE_WINDOWS */
typedef WIN232_RTS_MODE TMWTARG232_RTS_MODE;
  #define TMWTARG232_RTS_DISABLE   WIN232_RTS_DISABLE
  #define TMWTARG232_RTS_ENABLE    WIN232_RTS_ENABLE
  #define TMWTARG232_RTS_HANDSHAKE WIN232_RTS_HANDSHAKE 
  #define TMWTARG232_RTS_TOGGLE    WIN232_RTS_TOGGLE

#endif

#endif // tmwtargos_DEFINED
