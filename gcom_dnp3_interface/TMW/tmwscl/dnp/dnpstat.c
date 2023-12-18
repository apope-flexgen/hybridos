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
 * description: DNP Statistics
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/dnpstat.h"

#if DNPCNFG_SUPPORT_SSTAT_TIMEOUT
void TMWDEFS_GLOBAL dnpstat_sesnRequestTimeout(
  TMWSESN *pSession)
{
  TMWSESN_STAT_CALLBACK_FUNC(pSession, TMWSESN_STAT_REQUEST_TIMEOUT, TMWDEFS_NULL);
}
#endif

#if DNPCNFG_SUPPORT_SSTAT_FAILED
void TMWDEFS_GLOBAL dnpstat_sesnCheckRequestFailed(
  TMWSESN *pSession,
  DNPCHNL_RESP_STATUS status)
{   
  if((status == DNPCHNL_RESP_STATUS_FAILURE)
    ||(status == DNPCHNL_RESP_STATUS_MISMATCH)
    || (status == DNPCHNL_RESP_STATUS_STATUSCODE)
    || (status == DNPCHNL_RESP_STATUS_IIN))
    TMWSESN_STAT_CALLBACK_FUNC(pSession, TMWSESN_STAT_REQUEST_FAILED, TMWDEFS_NULL);
}
#endif

#if DNPCNFG_SUPPORT_SSTAT_ASDUS
void TMWDEFS_GLOBAL dnpstat_sesnAsduSent(
  TMWSESN *pSession)
{
  TMWSESN_STAT_CALLBACK_FUNC(pSession, TMWSESN_STAT_ASDU_SENT, TMWDEFS_NULL);
}
void TMWDEFS_GLOBAL dnpstat_sesnAsduReceived(
  TMWSESN *pSession)
{
  TMWSESN_STAT_CALLBACK_FUNC(pSession, TMWSESN_STAT_ASDU_RECEIVED, TMWDEFS_NULL);
}
#endif

#if DNPCNFG_SUPPORT_SSTAT_EVENT
void TMWDEFS_GLOBAL dnpstat_sesnEventConfirm(
  TMWSESN *pSession,
  TMWTYPES_UCHAR group,
  TMWTYPES_ULONG point)
{
  TMWSESN_STAT_DNPEVENT eventInfo; 
  eventInfo.group = group; 
  eventInfo.point = point; 
  TMWSESN_STAT_CALLBACK_FUNC(pSession, TMWSESN_STAT_DNPEVENT_CONFIRM, &eventInfo); 
}
void TMWDEFS_GLOBAL dnpstat_sesnEventSent(
  TMWSESN *pSession,
  TMWTYPES_UCHAR group,
  TMWTYPES_ULONG point)
{
  TMWSESN_STAT_DNPEVENT eventInfo; 
  eventInfo.group = group; 
  eventInfo.point = point; 
  TMWSESN_STAT_CALLBACK_FUNC(pSession, TMWSESN_STAT_DNPEVENT_SENT, &eventInfo); 
}
#endif 

#if DNPCNFG_SUPPORT_SSTAT_UNSOL_TIMER 
void TMWDEFS_GLOBAL dnpstat_sesnUnsolTimerStart(
  TMWSESN *pSession,
  TMWDEFS_CLASS_MASK eventClass,
  TMWTYPES_MILLISECONDS delay)
{
  TMWSESN_STAT_DNPUNSOLTIMER eventInfo; 
  eventInfo.eventClass = eventClass; 
  eventInfo.delay = delay; 
  TMWSESN_STAT_CALLBACK_FUNC(pSession, TMWSESN_STAT_DNPUNSOL_TIMER_START, &eventInfo); 
}
#endif

#if DNPCNFG_SUPPORT_SSTAT_OVERFLOW
void TMWDEFS_GLOBAL dnpstat_sesnEventOverflow(
  TMWSESN *pSession,
  TMWTYPES_UCHAR group)
{
  TMWSESN_STAT_CALLBACK_FUNC(pSession, TMWSESN_STAT_EVENT_OVERFLOW, &group); 
}
#endif

/* Secure Authentication Statistics */
#if DNPCNFG_SUPPORT_SSTAT_AUTH

void TMWDEFS_GLOBAL dnpstat_authSent(
  TMWSESN *pSession,
  TMWTYPES_UCHAR variation)
{
  TMWSESN_STAT_CALLBACK_FUNC(pSession, TMWSESN_STAT_AUTH_SENT, &variation);
}
void TMWDEFS_GLOBAL dnpstat_authRcvd(
  TMWSESN *pSession,
  TMWTYPES_UCHAR variation)
{
  TMWSESN_STAT_CALLBACK_FUNC(pSession, TMWSESN_STAT_AUTH_RCVD, &variation);
}
void TMWDEFS_GLOBAL dnpstat_authRespTimeout(
  TMWSESN *pSession)
{
  TMWSESN_STAT_CALLBACK_FUNC(pSession, TMWSESN_STAT_AUTH_RESPTIMEOUT, TMWDEFS_NULL);
} 

void TMWDEFS_GLOBAL dnpstat_authKeyChange(
  TMWSESN *pSession,
  TMWTYPES_USHORT userNumber)
{
  TMWTARG_UNUSED_PARAM(userNumber);
  /*SPM user number */
  TMWSESN_STAT_CALLBACK_FUNC(pSession, TMWSESN_STAT_AUTH_KEYCHANGE, TMWDEFS_NULL);
} 
#endif


/* Channel Statistics */
#if DNPCNFG_SUPPORT_CSTAT_ERRORS
void TMWDEFS_GLOBAL dnpstat_chnlError(
  TMWCHNL *pChannel,
  TMWSESN *pSession,
  TMWCHNL_ERROR_CODE errorCode)
{
  TMWCHNL_STAT_ERROR_TYPE errorInfo;
  errorInfo.errorCode = errorCode;
  errorInfo.pSession = pSession;
  TMWCHNL_STAT_CALLBACK_FUNC(pChannel, TMWCHNL_STAT_ERROR, &errorInfo);
}
#endif

#if DNPCNFG_SUPPORT_CSTAT_FRAMES
void TMWDEFS_GLOBAL dnpstat_chnlFrameSent(
  TMWCHNL *pChannel)
{
  TMWCHNL_STAT_CALLBACK_FUNC(pChannel, TMWCHNL_STAT_FRAME_SENT, TMWDEFS_NULL);
}
void TMWDEFS_GLOBAL dnpstat_chnlFrameRcvd(
  TMWCHNL *pChannel)
{
  TMWCHNL_STAT_CALLBACK_FUNC(pChannel, TMWCHNL_STAT_FRAME_RECEIVED, TMWDEFS_NULL);
}
#endif

#if DNPCNFG_SUPPORT_CSTAT_FRAGS
void TMWDEFS_GLOBAL dnpstat_chnlFragmentSent(
  TMWCHNL *pChannel)
{
  TMWCHNL_STAT_CALLBACK_FUNC(pChannel, TMWCHNL_STAT_FRAGMENT_SENT, TMWDEFS_NULL);
}
void TMWDEFS_GLOBAL dnpstat_chnlFragmentRcvd(
  TMWCHNL *pChannel)
{
  TMWCHNL_STAT_CALLBACK_FUNC(pChannel, TMWCHNL_STAT_FRAGMENT_RECEIVED, TMWDEFS_NULL);
}
#endif
