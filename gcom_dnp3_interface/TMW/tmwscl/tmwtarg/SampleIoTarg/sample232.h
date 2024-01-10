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

/* file: sample232.h
 * description: Definition of sample RS232 target routines
 */
#ifndef sample232_DEFINED
#define sample232_DEFINED

#if TMWTARG_SUPPORT_232

#include "tmwscl/utils/tmwtypes.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwtarg.h"

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
  
} SERIAL_IO_CHANNEL;


/* function: sample232_initChannel */
void * TMWDEFS_GLOBAL sample232_initChannel(
  const void *pUserConfig,
  TMWTARG_CONFIG *pTmwConfig,
  TMWTARG_IO_CHANNEL *pTargIoChannel);

/* function: sample232_deleteChannel */
void TMWDEFS_GLOBAL sample232_deleteChannel(
  TMWTARG_IO_CHANNEL *pTargIoChannel);

/* function: sample232_openChannel */
TMWTYPES_BOOL TMWDEFS_GLOBAL sample232_openChannel(
  TMWTARG_IO_CHANNEL *pTargIoChannel);

/* function: sample232_closeChannel */
void TMWDEFS_GLOBAL sample232_closeChannel(
  TMWTARG_IO_CHANNEL *pTargIoChannel);

/* function: sample232_getTransmitReady */
TMWTYPES_MILLISECONDS TMWDEFS_GLOBAL sample232_getTransmitReady(
  TMWTARG_IO_CHANNEL *pTargIoChannel);

/* function: sample232_receive */
TMWTYPES_USHORT TMWDEFS_GLOBAL sample232_receive(
  TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT maxBytes,
  TMWTYPES_MILLISECONDS maxTimeout,
  TMWTYPES_BOOL *pInterCharTimeoutOccurred);

/* function: sample232_transmit */
TMWTYPES_BOOL TMWDEFS_GLOBAL sample232_transmit(
  TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT numBytes);

/* function: sample232_checkForInputFunction */
void TMWDEFS_GLOBAL sample232_checkForInputFunction(TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_MILLISECONDS timeout);

#endif /* #if TMWTARG_SUPPORT_232 */

#endif /* sample232_DEFINED */
