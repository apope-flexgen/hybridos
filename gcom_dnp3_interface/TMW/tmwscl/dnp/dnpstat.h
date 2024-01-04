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

/* file: dnpstat.h
 * description: This file is intended for internal SCL use only.
 *  DNP Statistics
 */
#ifndef DNPSTAT_DEFINED
#define DNPSTAT_DEFINED

#include "tmwscl/dnp/dnpcnfg.h"
#include "tmwscl/utils/tmwsesn.h"
#include "tmwscl/utils/tmwchnl.h"

#if DNPCNFG_SUPPORT_SSTAT_TIMEOUT
  #define DNPSTAT_SESN_REQUEST_TIMEOUT(pSession) \
  dnpstat_sesnRequestTimeout(pSession)
#else
  #define DNPSTAT_SESN_REQUEST_TIMEOUT(pSession)
#endif

#if DNPCNFG_SUPPORT_SSTAT_FAILED
#define DNPSTAT_SESN_CHECK_REQUEST_FAILED(pSession, status) \
  dnpstat_sesnCheckRequestFailed(pSession, status)
#else
#define DNPSTAT_SESN_CHECK_REQUEST_FAILED(pSession, status)
#endif

#if DNPCNFG_SUPPORT_SSTAT_ASDUS
#define DNPSTAT_SESN_ASDU_SENT(pSession) \
  dnpstat_sesnAsduSent(pSession)
#define DNPSTAT_SESN_ASDU_RECEIVED(pSession) \
  dnpstat_sesnAsduReceived(pSession)
#else
  #define DNPSTAT_SESN_ASDU_SENT(pSession) 
  #define DNPSTAT_SESN_ASDU_RECEIVED(pSession) 
#endif

#if DNPCNFG_SUPPORT_SSTAT_EVENT
  #define DNPSTAT_SESN_EVENT_CONFIRM(pSession, group, point) \
    dnpstat_sesnEventConfirm(pSession, group, point)
  #define DNPSTAT_SESN_EVENT_SENT(pSession, group, point) \
    dnpstat_sesnEventSent(pSession, group, point) 
#else
  #define DNPSTAT_SESN_EVENT_CONFIRM(pSession, group, point)
  #define DNPSTAT_SESN_EVENT_SENT(pSession, group, point)
#endif

#if DNPCNFG_SUPPORT_SSTAT_UNSOL_TIMER 
  #define DNPSTAT_SESN_UNSOL_TIMER_START(pSession, eventClass, delay) \
    dnpstat_sesnUnsolTimerStart(pSession, eventClass, delay)
#else
  #define DNPSTAT_SESN_UNSOL_TIMER_START(pSession, eventClass, delay) 
#endif

#if DNPCNFG_SUPPORT_SSTAT_OVERFLOW
  #define DNPSTAT_SESN_EVENT_OVERFLOW(pSession, group) \
    dnpstat_sesnEventOverflow(pSession, group)
#else
  #define DNPSTAT_SESN_EVENT_OVERFLOW(pSession, group)
#endif

#if DNPCNFG_SUPPORT_SSTAT_AUTH
  #define DNPSTAT_AUTH_SENT(pSession, variation) \
    dnpstat_authSent(pSession, variation)
 
  #define DNPSTAT_AUTH_RCVD(pSession, variation) \
    dnpstat_authRcvd(pSession, variation)

  #define DNPSTAT_AUTH_RESP_TIMEOUT(pSession) \
    dnpstat_authRespTimeout(pSession)


  #define DNPSTAT_AUTH_KEY_CHANGE(pSession, userNumber) \
    dnpstat_authKeyChange(pSession, userNumber)
#else
  #define DNPSTAT_AUTH_SENT(pSession, variation)
 
  #define DNPSTAT_AUTH_RCVD(pSession, variation)

  #define DNPSTAT_AUTH_RESP_TIMEOUT(pSession)

  #define DNPSTAT_AUTH_KEY_CHANGE(pSession, userNumber)
#endif


#if DNPCNFG_SUPPORT_CSTAT_ERRORS
  #define DNPSTAT_CHNL_ERROR(pChannel, pSession, errorCode) \
    dnpstat_chnlError(pChannel, pSession, errorCode)
#else
  #define DNPSTAT_CHNL_ERROR(pChannel, pSession, errorCode)
#endif

#if DNPCNFG_SUPPORT_CSTAT_FRAMES
  #define DNPSTAT_CHNL_FRAME_SENT(pChannel) \
    dnpstat_chnlFrameSent(pChannel)
#define DNPSTAT_CHNL_FRAME_RCVD(pChannel) \
    dnpstat_chnlFrameRcvd(pChannel)
#else
  #define DNPSTAT_CHNL_FRAME_SENT(pChannel) 
  #define DNPSTAT_CHNL_FRAME_RCVD(pChannel) 
#endif


#if DNPCNFG_SUPPORT_CSTAT_FRAGS
  #define DNPSTAT_CHNL_FRAGMENT_SENT(pChannel) \
    dnpstat_chnlFragmentSent(pChannel)
  #define DNPSTAT_CHNL_FRAGMENT_RCVD(pChannel) \
    dnpstat_chnlFragmentRcvd(pChannel)
#else
  #define DNPSTAT_CHNL_FRAGMENT_SENT(pChannel) 
  #define DNPSTAT_CHNL_FRAGMENT_RCVD(pChannel) 
#endif

#ifdef __cplusplus
extern "C" {
#endif

  
#if TMWCNFG_SUPPORT_STATS
void TMWDEFS_GLOBAL dnpstat_chnlError(
  TMWCHNL *pChannel,
  TMWSESN *pSession,
  TMWCHNL_ERROR_CODE errorCode);
#endif

#if DNPCNFG_SUPPORT_SSTAT_TIMEOUT
void TMWDEFS_GLOBAL dnpstat_sesnRequestTimeout(
  TMWSESN *pSession);
#endif

#if DNPCNFG_SUPPORT_SSTAT_FAILED
void TMWDEFS_GLOBAL dnpstat_sesnCheckRequestFailed(
  TMWSESN *pSession,
  DNPCHNL_RESP_STATUS status);
#endif

#if DNPCNFG_SUPPORT_SSTAT_ASDUS
void TMWDEFS_GLOBAL dnpstat_sesnAsduSent(
  TMWSESN *pSession);

void TMWDEFS_GLOBAL dnpstat_sesnAsduReceived(
  TMWSESN *pSession);
#endif

#if DNPCNFG_SUPPORT_SSTAT_EVENT
void TMWDEFS_GLOBAL dnpstat_sesnEventConfirm(
  TMWSESN *pSession,
  TMWTYPES_UCHAR group,
  TMWTYPES_ULONG point);

void TMWDEFS_GLOBAL dnpstat_sesnEventSent(
  TMWSESN *pSession,
  TMWTYPES_UCHAR group,
  TMWTYPES_ULONG point);
#endif

#if DNPCNFG_SUPPORT_SSTAT_UNSOL_TIMER
void TMWDEFS_GLOBAL dnpstat_sesnUnsolTimerStart(
  TMWSESN *pSession,
  TMWDEFS_CLASS_MASK eventClass,
  TMWTYPES_MILLISECONDS delay);
#endif

#if DNPCNFG_SUPPORT_SSTAT_OVERFLOW
void TMWDEFS_GLOBAL dnpstat_sesnEventOverflow(
  TMWSESN *pSession,
  TMWTYPES_UCHAR group);
#endif

/* Secure Authentication Statistics */
#if DNPCNFG_SUPPORT_SSTAT_AUTH

void TMWDEFS_GLOBAL dnpstat_authSent(
  TMWSESN *pSession,
  TMWTYPES_UCHAR variation);

void TMWDEFS_GLOBAL dnpstat_authRcvd(
  TMWSESN *pSession,
  TMWTYPES_UCHAR variation);

void TMWDEFS_GLOBAL dnpstat_authRespTimeout(
  TMWSESN *pSession);

void TMWDEFS_GLOBAL dnpstat_authKeyChange(
  TMWSESN *pSession,
  TMWTYPES_USHORT userNumber);
 
#endif
 

#if DNPCNFG_SUPPORT_CSTAT_FRAMES
void TMWDEFS_GLOBAL dnpstat_chnlFrameSent(
  TMWCHNL *pChannel);

void TMWDEFS_GLOBAL dnpstat_chnlFrameRcvd(
  TMWCHNL *pChannel);
#endif

#if DNPCNFG_SUPPORT_CSTAT_FRAGS
void TMWDEFS_GLOBAL dnpstat_chnlFragmentSent(
  TMWCHNL *pChannel);

void TMWDEFS_GLOBAL dnpstat_chnlFragmentRcvd(
  TMWCHNL *pChannel);
#endif

#ifdef __cplusplus
}
#endif

#endif
