
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


/* file: sampleTCP.cpp
* description: Implementation of TCP/IP target routines for Sample
*/

#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwpltmr.h"
#include "tmwtargio.h"

#if TMWTARG_SUPPORT_TCP
#include "sampletcp.h"
#include "sampleiodiag.h"


/* function: sampleTCP_initChannel
 * purpose: Initialize a TCP communications channel. This routine creates
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
void * TMWDEFS_GLOBAL sampleTCP_initChannel(
  const void *pUserConfig,
  TMWTARG_CONFIG *pTmwConfig,
  TMWTARG_IO_CHANNEL *pTargIoChannel)
{
  TMWTARGIO_CONFIG *pIOConfig = (TMWTARGIO_CONFIG *) pUserConfig;
  TCP_IO_CHANNEL *pTcpChannel;

  pTcpChannel = (TCP_IO_CHANNEL *) malloc(sizeof(TCP_IO_CHANNEL));
  if (pTcpChannel == NULL)
    return TMWDEFS_NULL;

  memset(pTcpChannel, 0, sizeof(TCP_IO_CHANNEL));

  /* Store the channel configuration */
  pTcpChannel->chnlConfig = pIOConfig->targTCP;

  /* Set up the Channel Callback Functions */
  pTargIoChannel->pChannelCallback      = pTmwConfig->pChannelCallback;
  pTargIoChannel->pChannelCallbackParam = pTmwConfig->pCallbackParam;
  pTargIoChannel->pChannelReadyCallback = pTmwConfig->pChannelReadyCallback;
  pTargIoChannel->pChannelReadyCbkParam = pTmwConfig->pChannelReadyCbkParam;

  /* Set up the Channel Operation Functions */
  pTargIoChannel->pOpenFunction       = sampleTCP_openChannel;
  pTargIoChannel->pXmitReadyFunction  = sampleTCP_getTransmitReady;
  pTargIoChannel->pXmitFunction       = sampleTCP_transmit;
  pTargIoChannel->pRecvFunction       = sampleTCP_receive;
  pTargIoChannel->pCheckInputFunction = sampleTCP_checkForInputFunction;
  pTargIoChannel->pCloseFunction      = sampleTCP_closeChannel;
  pTargIoChannel->pDeleteFunction     = sampleTCP_deleteChannel;
  
  pTargIoChannel->polledMode = pTcpChannel->chnlConfig.polledMode;

  sprintf(pTargIoChannel->chanInfoBuf,"Nic: %s Port: %s:%d",
    pTcpChannel->chnlConfig.nicName,
    pTcpChannel->chnlConfig.ipAddress,
    pTcpChannel->chnlConfig.ipPort);
  pTargIoChannel->pChannelName = pTcpChannel->chnlConfig.chnlName;

  TMWTARG_LOCK_INIT(&pTcpChannel->tcpChannelLock);

#if TMWTARG_SUPPORT_UDP
  pTargIoChannel->pXmitUdpFunction   = sampleTCP_transmitUDP;
  /*TMWTARG_LOCK_INIT(&pTcpChannel->udpBufferLock); */
#endif

  return pTcpChannel;
}

/* function: sampleTCP_deleteChannel
 * purpose: Deletes a TCP communications channel. This routine should
 *  delete a TCP communications channel and free all associated memory
 *  and resources.
 * arguments:
 *  pTargIoChannel - TMW target IO channel pointer.
 * returns:
 *  void
 */
void TMWDEFS_GLOBAL sampleTCP_deleteChannel(TMWTARG_IO_CHANNEL *pTargIoChannel)
{
  TCP_IO_CHANNEL *pTcpChannel = (TCP_IO_CHANNEL *) pTargIoChannel->pChannelInfo;
  sampleTCP_closeChannel(pTargIoChannel);

  TMWTARG_LOCK_DELETE(&pTcpChannel->tcpChannelLock);

#if TMWTARG_SUPPORT_UDP
  /*TMWTARG_LOCK_DELETE(&pTcpChannel->udpBufferLock); */
#endif

  free(pTcpChannel);
}

/* function: sampleTCP_openChannel
 * purpose: Open a TCP communications channel.
 *  This function should attempt to listen or make the connection.
 * arguments:
 *  pTargIoChannel - TMW target IO channel pointer.
 * returns:
 *  TMWDEFS_TRUE if the connection is established, else TMWDEFS_FALSE
 */
TMWTYPES_BOOL sampleTCP_openChannel(TMWTARG_IO_CHANNEL *pTargIoChannel)
{
  TCP_IO_CHANNEL *pTcpChannel = (TCP_IO_CHANNEL *) pTargIoChannel->pChannelInfo;
  TMWTYPES_BOOL   status = TMWDEFS_FALSE;

  /* Put your code here */
  SAMPLEIODIAG_ERRORMSG("TCP(%s), Error opening sample TCP channel",  pTcpChannel->chnlConfig.chnlName);
  TMWTARG_UNUSED_PARAM(pTcpChannel);

  return(status);
}

/* function: sampleTCP_closeChannel
 * purpose: Close a TCP communications channel
 * arguments:
 *  pTargIoChannel - TMW target IO channel pointer.
 * returns:
 *  void
 */
void TMWDEFS_GLOBAL sampleTCP_closeChannel(TMWTARG_IO_CHANNEL *pTargIoChannel)
{
  TCP_IO_CHANNEL *pTcpChannel = (TCP_IO_CHANNEL *) pTargIoChannel->pChannelInfo;

  /* Put your code here */
  TMWTARG_UNUSED_PARAM(pTcpChannel);
}

/* function: sampleTCP_getTransmitReady
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
TMWTYPES_MILLISECONDS TMWDEFS_GLOBAL sampleTCP_getTransmitReady(TMWTARG_IO_CHANNEL *pTargIoChannel)
{
  TCP_IO_CHANNEL *pTcpChannel = (TCP_IO_CHANNEL *) pTargIoChannel->pChannelInfo;

  /* Put your code here */
  TMWTARG_UNUSED_PARAM(pTcpChannel);

  return(0); 
}

/* function: sampleTCP_receive, this also receives UDP data
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
TMWTYPES_USHORT TMWDEFS_GLOBAL sampleTCP_receive(
  TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT maxBytes,
  TMWTYPES_MILLISECONDS maxTimeout,
  TMWTYPES_BOOL *pInterCharTimeoutOccurred)
{
  TCP_IO_CHANNEL    *pTcpChannel = (TCP_IO_CHANNEL *) pTargIoChannel->pChannelInfo;

  /* Put your code here */
  TMWTARG_UNUSED_PARAM(pBuff);
  TMWTARG_UNUSED_PARAM(maxBytes);
  TMWTARG_UNUSED_PARAM(maxTimeout);
  TMWTARG_UNUSED_PARAM(pInterCharTimeoutOccurred);
  TMWTARG_UNUSED_PARAM(pTcpChannel);
  return((TMWTYPES_USHORT) 0);
}

/* function: sampleTCP_transmit
 * purpose: Transmit bytes on the specified channel
 * arguments:
 *  pTargIoChannel - TMW target IO channel pointer.
 *  pBuff - Array of bytes to transmit
 *  numBytes - Number of bytes to transmit
 * returns:
 *  TMWDEFS_TRUE if all the bytes were successfully transmitted,
 *  else TMWDEFS_FALSE.
 */
TMWTYPES_BOOL TMWDEFS_GLOBAL sampleTCP_transmit(
  TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT numBytes)
{
  TCP_IO_CHANNEL *pTcpChannel = (TCP_IO_CHANNEL *) pTargIoChannel->pChannelInfo;
  TMWTYPES_BOOL result = TMWDEFS_FALSE;

  /* Put your code here */
  TMWTARG_UNUSED_PARAM(pBuff);
  TMWTARG_UNUSED_PARAM(numBytes);
  TMWTARG_UNUSED_PARAM(pTcpChannel);

  return(result);
}

/* function: sampleTCP_checkForInputFunction
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
void TMWDEFS_GLOBAL sampleTCP_checkForInputFunction(TMWTARG_IO_CHANNEL *pTargIoChannel, TMWTYPES_MILLISECONDS timeout)
{
  TCP_IO_CHANNEL *pTcpChannel = (TCP_IO_CHANNEL *)pTargIoChannel->pChannelInfo;

  /* Put your code here */
  TMWTARG_UNUSED_PARAM(timeout);
  TMWTARG_UNUSED_PARAM(pTcpChannel);
}

/* function: sampleTCP_exit */
void TMWDEFS_GLOBAL sampleTCP_exit(void)
{

}

#if TMWTARG_SUPPORT_UDP

/* function: sampleTCP_transmitUDP
 * purpose: Transmit bytes using UDP on the specified channel
 * arguments:
 *  pTargIoChannel - TMW target IO channel pointer.
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
TMWTYPES_BOOL TMWDEFS_GLOBAL sampleTCP_transmitUDP(
  TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_UCHAR UDPPort,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT numBytes)
{
  TCP_IO_CHANNEL      *pTcpChannel = (TCP_IO_CHANNEL *) pTargIoChannel->pChannelInfo;
  TMWTYPES_BOOL       result = TMWDEFS_FALSE;

  /* Put your code here */
  TMWTARG_UNUSED_PARAM(UDPPort);
  TMWTARG_UNUSED_PARAM(pBuff);
  TMWTARG_UNUSED_PARAM(numBytes);
  TMWTARG_UNUSED_PARAM(pTcpChannel);

  return(result);
}
#endif /* #if TMWTARG_SUPPORT_UDP */

#endif /* #if TMWTARG_SUPPORT_TCP */
