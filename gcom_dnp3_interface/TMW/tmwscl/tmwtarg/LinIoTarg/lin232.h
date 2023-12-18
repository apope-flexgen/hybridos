/*****************************************************************************/
/* Triangle MicroWorks, Inc.                         Copyright (c) 2008-2011 */
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

/* file: lin232.h
 * description: Definition of RS232 target routines for Linux
 */
#ifndef lin232_DEFINED
#define lin232_DEFINED

#if TMWTARG_SUPPORT_232

#include <termios.h>
#include "tmwscl/utils/tmwtypes.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwtarg.h"
#if TMWTARG_SUPPORT_POLL
#include <poll.h>
#endif

/* Define serial port channel */
typedef struct SerialIOChannel {

  TMWTARG232_CONFIG      chnlConfig;
  /* 
   * Specifies the amount of time (in character times) to use to 
   * determine that a frame has been completed.  For modbus RTU this 
   * value is 3.5 (i.e. 4 will be used)
   */
  TMWTYPES_USHORT numCharTimesBetweenFrames;

  /*
   * Specifies the amount of time to use to 
   * determine that an inter character timeout has occured.  
   * For modbus RTU this value is 1.5 character times (i.e. 2 would be used)
   */
  TMWTYPES_USHORT interCharTimeout;
  
  /* Linux tty fd must be carried around */
  TMWTYPES_INT       ttyfd;
  speed_t            portSpeed;

#if TMWTARG_SUPPORT_POLL
  struct pollfd   pollFd;
#endif

} SERIAL_IO_CHANNEL;


/* function: lin232_initChannel */
void * TMWDEFS_GLOBAL lin232_initChannel(
  const void *pUserConfig,
  TMWTARG_CONFIG *pTmwConfig,
  TMWTARG_IO_CHANNEL *pTargIoChannel);

/* function: lin232_deleteChannel */
void TMWDEFS_GLOBAL lin232_deleteChannel(
  TMWTARG_IO_CHANNEL *pTargIoChannel);

/* function: lin232_openChannel */
TMWTYPES_BOOL TMWDEFS_GLOBAL lin232_openChannel(
  TMWTARG_IO_CHANNEL *pTargIoChannel);

/* function: lin232_closeChannel */
void TMWDEFS_GLOBAL lin232_closeChannel(
  TMWTARG_IO_CHANNEL *pTargIoChannel);

/* function: lin232_getTransmitReady */
TMWTYPES_MILLISECONDS TMWDEFS_GLOBAL lin232_getTransmitReady(
  TMWTARG_IO_CHANNEL *pTargIoChannel);

/* function: lin232_receive */
TMWTYPES_USHORT TMWDEFS_GLOBAL lin232_receive(
  TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT maxBytes,
  TMWTYPES_MILLISECONDS maxTimeout,
  TMWTYPES_BOOL *pInterCharTimeoutOccurred);

/* function: lin232_transmit */
TMWTYPES_BOOL TMWDEFS_GLOBAL lin232_transmit(
  TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT numBytes);

void TMWDEFS_GLOBAL lin232_checkForInputFunction(TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_MILLISECONDS timeout);

#endif /* #if TMWTARG_SUPPORT_232 */

#endif /* lin232_DEFINED */
