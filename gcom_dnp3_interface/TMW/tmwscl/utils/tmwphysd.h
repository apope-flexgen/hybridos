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

/* file: tmwphysd.h
 * description: Physical Layer Diagnostics
 */
#ifndef TMWPHYSD_DEFINED
#define TMWPHYSD_DEFINED

#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwchnl.h"

#if !TMWCNFG_SUPPORT_DIAG

#define TMWPHYSD_CHANNEL_OPENED(pChannel) \
  TMWTARG_UNUSED_PARAM(pChannel);

#define TMWPHYSD_CHANNEL_CLOSED(pChannel, reason) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(reason);

#define TMWPHYSD_BYTES_SENT(pChannel, pBuff, numBytes) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(pBuff); TMWTARG_UNUSED_PARAM(numBytes);

#define TMWPHYSD_BYTES_RECEIVED(pChannel, pBuff, numBytes) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(pBuff); TMWTARG_UNUSED_PARAM(numBytes);

#define TMWPHYSD_ERROR(pChannel, errorString) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(errorString);

#define TMWPHYSD_INFO(pChannel, infoString) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(infoString);

#else

#define TMWPHYSD_CHANNEL_OPENED(pChannel) \
  tmwphysd_channelOpened(pChannel)

#define TMWPHYSD_CHANNEL_CLOSED(pChannel, reason) \
  tmwphysd_channelClosed(pChannel, reason)

#define TMWPHYSD_BYTES_SENT(pChannel, pBuff, numBytes) \
  tmwphysd_bytesSent(pChannel, pBuff, numBytes)

#define TMWPHYSD_BYTES_RECEIVED(pChannel, pBuff, numBytes) \
  tmwphysd_bytesReceived(pChannel, pBuff, numBytes)

#define TMWPHYSD_ERROR(pChannel, errorString) \
  tmwphysd_error(pChannel, errorString)

#define TMWPHYSD_INFO(pChannel, infoString) \
  tmwphysd_info(pChannel, infoString)

#ifdef __cplusplus
extern "C" {
#endif

  /* function: tmwphysd_channelOpened
   * purpose: Display channel opened diagnostic message
   * arguments:
   *  pChannel - pointer to channel structure for this channel
   * returns: 
   *  void
   */
  void TMWDEFS_GLOBAL tmwphysd_channelOpened(
    TMWCHNL *pChannel);

  /* function: tmwphysd_channelClosed
   * purpose: Display channel closed diagnostic message
   * arguments:
   *  pChannel - pointer to channel structure for this channel
   * returns: 
   *  void
   */
  void TMWDEFS_GLOBAL tmwphysd_channelClosed(
    TMWCHNL *pChannel,
    TMWDEFS_TARG_OC_REASON reason);

  /* function: tmwphysd_bytesSent
   * purpose: Display transmitted bytes
   * arguments:
   *  pChannel - pointer to channel structure for this channel
   *  pBuff - buffer containing bytes to display
   *  numBytes - number of bytes to display
   * returns: 
   *  void
   */
  void TMWDEFS_GLOBAL tmwphysd_bytesSent(
    TMWCHNL *pChannel,
    const TMWTYPES_UCHAR *pBuff, 
    TMWTYPES_USHORT numBytes);

  /* function: tmwphysd_bytesReceived
   * purpose: Display received bytes
   * arguments:
   *  pChannel - pointer to channel structure for this channel
   *  pBuff - buffer containing bytes to display
   *  numBytes - number of bytes to display
   * returns: 
   *  void
   */
  void TMWDEFS_GLOBAL tmwphysd_bytesReceived(
    TMWCHNL *pChannel,
    const TMWTYPES_UCHAR *pBuff, 
    TMWTYPES_USHORT numBytes);

  /* function: tmwphysd_error
   * purpose: Display error message
   * arguments:
   *  pChannel - pointer to channel structure for this channel
   *  errorMsg - error message to display
   * returns: 
   *  void
   */
  void TMWDEFS_GLOBAL tmwphysd_error(
    TMWCHNL *pChannel,
    const TMWTYPES_CHAR *errorString);  
    
  /* function: tmwphysd_info
   * purpose: Display informational message
   * arguments:
   *  pChannel - pointer to channel structure for this channel
   *  infoMsg - informational message to display
   * returns: 
   *  void
   */
  void TMWDEFS_GLOBAL tmwphysd_info(
    TMWCHNL *pChannel,
    const TMWTYPES_CHAR *infoString);

#ifdef __cplusplus
}
#endif
#endif /* TMWCNFG_SUPPORT_DIAG */
#endif /* TMWPHYSD_DEFINED */
