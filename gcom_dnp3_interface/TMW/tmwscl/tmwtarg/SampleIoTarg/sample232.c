/*****************************************************************************/
/* Triangle MicroWorks, Inc.                         Copyright (c) 2008-2019 */
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


/* file: sample232.cpp
 * description: Sample implementation of RS232 target routines
 */

#include "tmwtargio.h"
#include "sample232.h"
#include "sampleiodiag.h"

#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwpltmr.h"

#if TMWTARG_SUPPORT_232

static TMWTYPES_CHAR * _getParityString(TMWTARG232_PARITY parity)
{
  if(parity == TMWTARG232_PARITY_NONE)
    return("none");
  else if(parity == TMWTARG232_PARITY_EVEN)
    return("even");
  else
    return("odd");
}

/* function: sample232_initChannel
 * purpose: Initialize an RS232 communications channel. This routine creates
 *  a communications channel as specified in the pConfig argument. The
 *  channel does not need to be opened as this will be accomplished in
 *  the sampleTCP_openChannel function described below. This routine
 *  returns a user defined context which is passed to all successive
 *  calls for this channel.
 * arguments:
 *  pUserConfig - Pointer to configuration data passed to the TMW
 *   physical layer code. This data is not used by the TMW code
 *   and should be used by the target routines to identify and
 *   configure the communications channel.
 *  pTmwConfig - TMW target configuration data structure
 *  pTargIoChannel - TMW target IO channel pointer.
 *
 * returns:
 *  void * channel context
 *   The channel context is a target-defined context that
 *   will be passed to all of the remaining channel target functions.
 *   The source code library does not change or manipulate this
 *   pointer in any way. The pointer cannot be NULL since this
 *   is interpreted as a failure.
 */
void * TMWDEFS_GLOBAL sample232_initChannel(
  const void *pUserConfig,
  TMWTARG_CONFIG *pTmwConfig,
  TMWTARG_IO_CHANNEL *pTargIoChannel)
{
  TMWTARGIO_CONFIG *pIOConfig = (TMWTARGIO_CONFIG *) pUserConfig;
  SERIAL_IO_CHANNEL *pSerialChannel;

  pSerialChannel = (SERIAL_IO_CHANNEL *) malloc(sizeof(SERIAL_IO_CHANNEL));
  if (pSerialChannel == NULL)
    return TMWDEFS_NULL;

  memset(pSerialChannel, 0, sizeof(SERIAL_IO_CHANNEL));

  /* Store the channel configuration */
  pSerialChannel->chnlConfig = pIOConfig->targ232;

  pTargIoChannel->pChannelCallback = pTmwConfig->pChannelCallback;
  pTargIoChannel->pChannelCallbackParam = pTmwConfig->pCallbackParam;
  pTargIoChannel->pChannelReadyCallback = pTmwConfig->pChannelReadyCallback;
  pTargIoChannel->pChannelReadyCbkParam = pTmwConfig->pChannelReadyCbkParam;
  pSerialChannel->numCharTimesBetweenFrames = pTmwConfig->numCharTimesBetweenFrames; 
  pSerialChannel->interCharTimeout = pTmwConfig->interCharTimeout;

  /* Set up the Channel Operation Functions */
  pTargIoChannel->pOpenFunction       = sample232_openChannel;
  pTargIoChannel->pXmitReadyFunction  = sample232_getTransmitReady;
  pTargIoChannel->pXmitFunction       = sample232_transmit;
  pTargIoChannel->pRecvFunction       = sample232_receive;
  pTargIoChannel->pCheckInputFunction = sample232_checkForInputFunction;
  pTargIoChannel->pCloseFunction      = sample232_closeChannel;
  pTargIoChannel->pDeleteFunction     = sample232_deleteChannel;

  pTargIoChannel->polledMode = pSerialChannel->chnlConfig.polledMode;

  sprintf(pTargIoChannel->chanInfoBuf, "baud %s, data bits %d, stop bits %d parity %s",
    pSerialChannel->chnlConfig.baudRate,
    pSerialChannel->chnlConfig.numDataBits + 7,
    pSerialChannel->chnlConfig.numStopBits + 1,
    _getParityString(pSerialChannel->chnlConfig.parity));
  pTargIoChannel->pChannelName = pSerialChannel->chnlConfig.chnlName;

  return pSerialChannel;
}

/* function: sample232_deleteChannel
 * purpose: Deletes an RS232 communications channel. This routine should
 *  delete an RS232 communications channel and free all associated memory
 *  and resources.
 * arguments:
 *  pTargIoChannel - TMW target IO channel pointer.
 * returns:
 *  void
 */
void TMWDEFS_GLOBAL sample232_deleteChannel(
  TMWTARG_IO_CHANNEL *pTargIoChannel)
{
  SERIAL_IO_CHANNEL *pSerialChannel = (SERIAL_IO_CHANNEL *)pTargIoChannel->pChannelInfo;
  /* Free the channel */
  free(pSerialChannel);
}

/* function: sample232_openChannel
 * purpose: Open an RS232 communications channel.
 * arguments:
 *  pTargIoChannel - TMW target IO channel pointer.
 * returns:
 *  TMWDEFS_TRUE if the channel is opened, else TMWDEFS_FALSE
 */
TMWTYPES_BOOL TMWDEFS_GLOBAL sample232_openChannel(
  TMWTARG_IO_CHANNEL *pTargIoChannel)
{
  SERIAL_IO_CHANNEL *pSerialChannel = (SERIAL_IO_CHANNEL *)pTargIoChannel->pChannelInfo;
  TMWTYPES_BOOL openStatus = TMWDEFS_FALSE;

  /* Attempt to open this RS232 channel. Return TMWDEFS_TRUE if
   * the channel is successfully opened, else TMWDEFS_FALSE.
   */
  /* Put your code here */
  if (openStatus == TMWDEFS_FALSE)
  {
    SAMPLEIODIAG_ERRORMSG("sample232 Error opening %s",  pSerialChannel->chnlConfig.portName);
    return TMWDEFS_FALSE;
  }

  SAMPLEIODIAG_MSG("sample232 opened %s",  pSerialChannel->chnlConfig.portName);
  return(TMWDEFS_TRUE);
}

/* function: sample232_closeChannel
 * purpose: Close an RS232 communications channel
 * arguments:
 *  pTargIoChannel - TMW target IO channel pointer.
 * returns:
 *  void
 */
void TMWDEFS_GLOBAL sample232_closeChannel(
  TMWTARG_IO_CHANNEL *pTargIoChannel)
{
  SERIAL_IO_CHANNEL *pSerialChannel = (SERIAL_IO_CHANNEL *)pTargIoChannel->pChannelInfo;

  /* Put your code here */
  TMWTARG_UNUSED_PARAM(pSerialChannel);
}

/* function: sample232_getTransmitReady
 * purpose: Determine whether a channel is ready to transmit or not.
 *  This routine can be used to delay transmission until various
 *  target related dependencies have been satisfied. A common
 *  example is modem setup time.
 * arguments:
 *  pTargIoChannel - TMW target IO channel pointer.
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
TMWTYPES_MILLISECONDS TMWDEFS_GLOBAL sample232_getTransmitReady(
  TMWTARG_IO_CHANNEL *pTargIoChannel)
{
  /* Put your code here */
  TMWTARG_UNUSED_PARAM(pTargIoChannel);
  return 0;
}

/* function: sample232_receive
 * purpose: Receive bytes from the specified channel
 * arguments:
 *  pTargIoChannel - TMW target IO channel pointer.
 *  pBuff - Buffer into which to store received bytes
 *  maxBytes - The maximum number of bytes to read
 *  maxTimeout - maximum time to wait in milliseconds for input
 *   from this channel.
 *  pInterCharTimeoutOccurred - TMWDEFS_TRUE if an intercharacter
 *   timeout occured while receiving bytes. This is an optional
 *   timeout that can be implemented in the target to terminate
 *   a frame if too much time passes between receipt of bytes
 *   in a frame.
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
TMWTYPES_USHORT TMWDEFS_GLOBAL sample232_receive(
  TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT maxBytes,
  TMWTYPES_MILLISECONDS maxTimeout,
  TMWTYPES_BOOL *pInterCharTimeoutOccurred)
{
  SERIAL_IO_CHANNEL *pSerialChannel = (SERIAL_IO_CHANNEL *)pTargIoChannel->pChannelInfo;
  TMWTYPES_USHORT bytesReceived = 0;

  /* Put your code here */
  TMWTARG_UNUSED_PARAM(pBuff);
  TMWTARG_UNUSED_PARAM(maxBytes);
  TMWTARG_UNUSED_PARAM(maxTimeout);
  TMWTARG_UNUSED_PARAM(pInterCharTimeoutOccurred);
  TMWTARG_UNUSED_PARAM(pSerialChannel);

  return bytesReceived;
}

/* function: sample232_transmit
 * purpose: Transmit bytes on the specified channel
 * arguments:
 *  pTargIoChannel - TMW target IO channel pointer.
 *  pBuff - Array of bytes to transmit
 *  numBytes - Number of bytes to transmit
 * returns:
 *  TMWDEFS_TRUE if all the bytes were successfully transmitted,
 *  else TMWDEFS_FALSE.
 */
TMWTYPES_BOOL TMWDEFS_GLOBAL sample232_transmit(
  TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT numBytes)
{
  SERIAL_IO_CHANNEL *pSerialChannel = (SERIAL_IO_CHANNEL *)pTargIoChannel->pChannelInfo;
  TMWTYPES_USHORT bytesSent = 0;

  /* Put your code here */
  TMWTARG_UNUSED_PARAM(pBuff);
  TMWTARG_UNUSED_PARAM(numBytes);
  TMWTARG_UNUSED_PARAM(pSerialChannel);

  if(numBytes == (TMWTYPES_USHORT)bytesSent)
    return TMWDEFS_TRUE;
  else
  {
    SAMPLEIODIAG_ERRORMSG("sample232 write failed %s");
    return TMWDEFS_FALSE;
  }
}

/* function: sample232_checkForInputFunction
 * purpose: Determine if this channel has recieved any data. It should post
 *  on the recieve for the number of milliseconds specified. If data is
 *  received, it should call the channel's recieve callback function.
 * arguments:
 *  pTargIoChannel - TMW target IO channel pointer.
 *  timeout - specifies the time (in milliseconds) this functions should wait
 *   for receive data.
 * returns:
 *  void
 */
void TMWDEFS_GLOBAL sample232_checkForInputFunction(TMWTARG_IO_CHANNEL *pTargIoChannel, TMWTYPES_MILLISECONDS timeout)
{
  SERIAL_IO_CHANNEL *pSerialChannel = (SERIAL_IO_CHANNEL *)pTargIoChannel->pChannelInfo;
  TMWTYPES_BOOL dataAvailable = TMWDEFS_FALSE;

  /* Put your code here */
  TMWTARG_UNUSED_PARAM(timeout);
  TMWTARG_UNUSED_PARAM(pSerialChannel);

  if (dataAvailable)
  {
    pTargIoChannel->pReceiveCallbackFunc(pTargIoChannel->pCallbackParam);
  }
}
  
#endif /* #if TMWTARG_SUPPORT_232 */
