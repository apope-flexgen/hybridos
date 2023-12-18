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

/* file: mdnpdiag.h
 * description: Master DNP Diagnostics
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwdefs.h"

#include "tmwscl/dnp/mdnpdiag.h"
#include "tmwscl/dnp/mdnpdata.h"
#if DNPCNFG_SUPPORT_AUTHENTICATION
#include "tmwscl/dnp/mdnpauth.h"
#endif

#if TMWCNFG_SUPPORT_DIAG

static const DNPDIAG_ERROR_ENTRY mdnpDiagErrorMsg[] = {
  {MDNPDIAG_INV_LENGTH         ,  "Invalid message length"}, 
  {MDNPDIAG_OBJ_LENGTH         ,  "Object does not fit into request"},
  {MDNPDIAG_FC                 ,  "Invalid function code for request"},
  {MDNPDIAG_INV_STARTSTOP      ,  "Invalid start or stop"},
  {MDNPDIAG_INV_QUALIFIER      ,  "Invalid qualifier"},
  {MDNPDIAG_INV_QUALIFIER_POINT,  "Invalid qualifier for point number specified, request not sent"},
  {MDNPDIAG_INV_VARIATION      ,  "Invalid variation"},
  {MDNPDIAG_INV_VARIATION_VALUE,  "Invalid variation for value specified, request not sent"},
  {MDNPDIAG_INV_QUANTITY       ,  "Invalid quantity"},
  {MDNPDIAG_BAD_APPL_SEQ       ,  "Error, bad application sequence number"},
  {MDNPDIAG_OBJVAR             ,  "Object/Variation not supported for function code"},
  {MDNPDIAG_ECHO               ,  "Remote device did not echo request"}, 
  {MDNPDIAG_INV_8BIT_POINT     ,  "Invalid point number for 8 bit index"},
  {MDNPDIAG_FRZN_FEEDBACK      ,  "Frozen Counter Feedback Poll canceled due to invalid status"},
  {MDNPDIAG_OPER_FEEDBACK      ,  "Operate/Feedback canceled due to invalid status"},
  {MDNPDIAG_OPER_MEMORY        ,  "Operate/Feedback canceled due to memory allocation failure"},
  {MDNPDIAG_INV_PROTOCOL       ,  "Session protocol is invalid for request"},
  {MDNPDIAG_STATUS_NOT_ZERO    ,  "Analog or Binary Command unsuccessful"},
  {MDNPDIAG_FILE_DIR_READ      ,  "Parse error on file directory read response"},
  {MDNPDIAG_INV_BROADCAST      ,  "Request cannot be sent using broadcast"},
  {MDNPDIAG_UNSOL_STARTUP      ,  "Discard Unsolicited Response in Startup state, waiting for Integrity Poll to complete"},
  {MDNPDIAG_NULL_RESTART       ,  "Discard Unsolicited Response with Restart IIN, Secure Authentication Key Exchange required"},
  {MDNPDIAG_NO_CTO             ,  "Discard Response containing a relative time object with no preceding CTO"},
  {MDNPDIAG_AUTH_FAIL          ,  "Secure Authentication failure, returning to IDLE state"},
  {MDNPDIAG_AUTH_RESTKEYSEXC   ,  "Secure Authentication, Rekeys Due to Restart Exceeded, wait for Key Change Timeout"},
 
  {MDNPDIAG_ERROR_ENUM_MAX     ,  ""}
};

/* Array to determine if specific error messages are disabled.
 * No error messages are disabled by default.
 */
static TMWTYPES_UCHAR _errorMsgDisabled[(MDNPDIAG_ERROR_ENUM_MAX/8)+1] = {0};

/* routine: mdnpdiag_init */
void TMWDEFS_GLOBAL mdnpdiag_init()
{
  /* No error messages are disabled by default. */
  memset(_errorMsgDisabled, 0, (MDNPDIAG_ERROR_ENUM_MAX/8)+1);
}

/* routine: mdnpdiag_validateErrorTable */
TMWTYPES_BOOL mdnpdiag_validateErrorTable(void)
{
  int i;
  for(i=0; i<MDNPDIAG_ERROR_ENUM_MAX;i++)
  {
    if(mdnpDiagErrorMsg[i].errorNumber != i)
      return(TMWDEFS_FALSE);
  }

  return(TMWDEFS_TRUE);
}

/* function: mdnpdiag_showIIN */
void TMWDEFS_GLOBAL mdnpdiag_showIIN(
  TMWSESN *pSession, 
  TMWTYPES_USHORT iin)
{
  if(iin != 0)
  {
    TMWDIAG_ANLZ_ID id;
    char buf[128];

    if(tmwdiag_initId(&id, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_APPL | TMWDIAG_ID_RX) == TMWDEFS_FALSE)
    {
      return;
    }
    tmwtarg_snprintf(buf, sizeof(buf), "===> %-10s IIN Bits:\n", tmwsesn_getSessionName(pSession));

    tmwdiag_skipLine(&id);
    tmwdiag_putLine(&id, buf);

    if(iin & DNPDEFS_IIN_RESTART)
    {
      tmwtarg_snprintf(buf, sizeof(buf), "%16sIIN1.7 Device Restart\n", " ");
      tmwdiag_putLine(&id, buf);
    }

    if(iin & DNPDEFS_IIN_TROUBLE)
    {
      tmwtarg_snprintf(buf, sizeof(buf), "%16sIIN1.6 Device Trouble\n", " ");
      tmwdiag_putLine(&id, buf);
    }

    if(iin & DNPDEFS_IIN_LOCAL)
    {
      tmwtarg_snprintf(buf, sizeof(buf), "%16sIIN1.5 Some Output Points Are In Local Mode\n", " ");
      tmwdiag_putLine(&id, buf);
    }

    if(iin & DNPDEFS_IIN_NEED_TIME)
    {
      tmwtarg_snprintf(buf, sizeof(buf), "%16sIIN1.4 Time Synchronization Required\n", " ");
      tmwdiag_putLine(&id, buf);
    }

    if(iin & DNPDEFS_IIN_CLASS_3)
    {
      tmwtarg_snprintf(buf, sizeof(buf), "%16sIIN1.3 Class 3 Event Data Is Available\n", " ");
      tmwdiag_putLine(&id, buf);
    }

    if(iin & DNPDEFS_IIN_CLASS_2)
    {
      tmwtarg_snprintf(buf, sizeof(buf), "%16sIIN1.2 Class 2 Event Data Is Available\n", " ");
      tmwdiag_putLine(&id, buf);
    }

    if(iin & DNPDEFS_IIN_CLASS_1)
    {
      tmwtarg_snprintf(buf, sizeof(buf), "%16sIIN1.1 Class 1 Event Data Is Available\n", " ");
      tmwdiag_putLine(&id, buf);
    }

    if(iin & DNPDEFS_IIN_ALL_STATIONS)
    {
      tmwtarg_snprintf(buf, sizeof(buf), "%16sIIN1.0 All-Stations Message Received\n", " ");
      tmwdiag_putLine(&id, buf);
    }

    if(iin & DNPDEFS_IIN_BAD_CONFIG)
    {
      tmwtarg_snprintf(buf, sizeof(buf), "%16sIIN2.5 Configuration Corrupt\n", " ");
      tmwdiag_putLine(&id, buf);
    }

    if(iin & DNPDEFS_IIN_ALREADY_EXECUTING)
    {
      tmwtarg_snprintf(buf, sizeof(buf), "%16sIIN2.4 Operation Is Already Executing\n", " ");
      tmwdiag_putLine(&id, buf);
    }

    if(iin & DNPDEFS_IIN_BUFFER_OVFL)
    {
      tmwtarg_snprintf(buf, sizeof(buf), "%16sIIN2.3 Event Buffer Overflow\n", " ");
      tmwdiag_putLine(&id, buf);
    }

    if(iin & DNPDEFS_IIN_OUT_OF_RANGE)
    {
      tmwtarg_snprintf(buf, sizeof(buf), "%16sIIN2.2 Parameter Error\n", " ");
      tmwdiag_putLine(&id, buf);
    }

    if(iin & DNPDEFS_IIN_OBJECT_UNKNOWN)
    {
      tmwtarg_snprintf(buf, sizeof(buf), "%16sIIN2.1 Object Unknown\n", " ");
      tmwdiag_putLine(&id, buf);
    }

    if(iin & DNPDEFS_IIN_BAD_FUNCTION)
    {
      tmwtarg_snprintf(buf, sizeof(buf), "%16sIIN2.0 Function Code Not Implemented\n", " ");
      tmwdiag_putLine(&id, buf);
    }
  }
}

/* function: mdnpdiag_processResponse */
void TMWDEFS_GLOBAL mdnpdiag_processResponse(
  TMWSESN *pSession,
  const char *description)
{
  TMWDIAG_ANLZ_ID id;
  char buf[256];

  if(tmwdiag_initId(&id, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_USER | TMWDIAG_ID_RX) == TMWDEFS_FALSE)
  {
    return;
  }

  tmwtarg_snprintf(buf, sizeof(buf), "+++> %-10s Process response to request: %s\n", 
    tmwsesn_getSessionName(pSession), description);

  tmwdiag_skipLine(&id);
  tmwdiag_putLine(&id, buf);
}

/* function: mdnpdiag_showDelay */
void TMWDEFS_GLOBAL mdnpdiag_showDelay(
  TMWSESN *pSession,
  TMWTYPES_MILLISECONDS delay,
  TMWTYPES_USHORT propagationDelay)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[128];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | TMWDIAG_ID_RX) == TMWDEFS_FALSE)
  {
    return;
  }
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Elapsed time measured on Master  = %06d\n", " ", delay);
  tmwdiag_putLine(&anlzId, buf);
 
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s New One Way Propagation Delay used for time sync = %06d\n", " ", propagationDelay);
  tmwdiag_putLine(&anlzId, buf);
}

/* function: mdnpdiag_showMsgDelay */
void TMWDEFS_GLOBAL mdnpdiag_showMsgDelay(
  TMWSESN *pSession,
  TMWTYPES_USHORT delay,
  TMWTYPES_BOOL inMilliseconds)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[128];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | TMWDIAG_ID_RX) == TMWDEFS_FALSE)
  {
    return;
  }
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Elapsed time returned from Slave = %06d %s\n", " ", delay, (inMilliseconds) ? "milliseconds" : "seconds");
  tmwdiag_putLine(&anlzId, buf);
}

/* function: mdnpdiag_errorMsgEnable */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdiag_errorMsgEnable(
  MDNPDIAG_ERROR_ENUM errorNumber,
  TMWTYPES_BOOL value)
{
  if ((errorNumber > MDNPDIAG_ERROR_ENUM_MAX)
    || (errorNumber < 0))
  {
    return(TMWDEFS_FALSE);
  }

  if(value)
    _errorMsgDisabled[errorNumber/8] &=  ~(1 <<(errorNumber%8));
  else
    _errorMsgDisabled[errorNumber/8] |=  (1 <<(errorNumber%8));

  return(TMWDEFS_TRUE);
}

/* function: _errorMsgDisabled */
static TMWTYPES_BOOL TMWDEFS_LOCAL _errorMsgDisabledFunc(
   MDNPDIAG_ERROR_ENUM errorNumber)
{
  int value;

  if ((errorNumber > MDNPDIAG_ERROR_ENUM_MAX)
    || (errorNumber < 0))
  {
    return(TMWDEFS_TRUE);
  }

  value = _errorMsgDisabled[errorNumber/8] & (1 <<(errorNumber%8));
  if(value == 0)
    return(TMWDEFS_FALSE);
  else
    return(TMWDEFS_TRUE);
}

/* function: mdnpdiag_errorMsg */
void TMWDEFS_GLOBAL mdnpdiag_errorMsg(
  TMWCHNL *pChannel,
  TMWSESN *pSession, 
  MDNPDIAG_ERROR_ENUM errorNumber,
  TMWTYPES_CHAR *pMsg)

{
  const char *pName = TMWDEFS_NULL;
  TMWDIAG_ANLZ_ID anlzId;
  char buf[256];

#ifdef TMW_SUPPORT_MONITOR
  /* If in analyzer or listen only mode, do not display this error */
  if(pChannel != TMWDEFS_NULL && pChannel->pPhysContext->monitorMode)
#endif
  if(_errorMsgDisabledFunc(errorNumber))
    return;

  if(tmwdiag_initId(&anlzId, pChannel, pSession, TMWDEFS_NULL, TMWDIAG_ID_APPL | TMWDIAG_ID_ERROR) == TMWDEFS_FALSE)
  {
    return;
  }
 
  if(pSession != TMWDEFS_NULL)
    pName = tmwsesn_getSessionName(pSession);

  if(pName == (const char *)TMWDEFS_NULL)
    pName = " ";

  if(pMsg == TMWDEFS_NULL)
    (void)tmwtarg_snprintf(buf, sizeof(buf), "**** %s   %s ****\n", pName, mdnpDiagErrorMsg[errorNumber].pErrorMsg);
  else
    (void)tmwtarg_snprintf(buf, sizeof(buf), "**** %s   %s  %s****\n", pName, mdnpDiagErrorMsg[errorNumber].pErrorMsg, pMsg);

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, buf);
}

/* function: mdnpdiag_showAuthKey */
void mdnpdiag_showFileAuthKey(
  TMWSESN *pSession, 
  TMWTYPES_ULONG authKey)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[128];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | TMWDIAG_ID_RX) == TMWDEFS_FALSE)
  {
    return;
  }
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s File authentication key returned from Outstation = %d\n", " ", authKey);
  tmwdiag_putLine(&anlzId, buf);
}

#if DNPCNFG_SUPPORT_AUTHENTICATION
static char *_toState(TMWTYPES_UCHAR state)
{
  switch(state)
  {
    case MDNPAUTH_STATE_INIT:
      return("INIT");
    case MDNPAUTH_STATE_WAITFORKEYSTATUS:
      return("WAITFORKEYSTATUS");
    case MDNPAUTH_STATE_WAITFORKEYCHANGECONF:
      return("WAITFORKEYCHANGECONF");
    case MDNPAUTH_STATE_IDLE:
      return("IDLE");
    case MDNPAUTH_STATE_WAITFORREPLY:
      return("WAITFORREPLY"); 
    case MDNPAUTH_STATE_WAITUSERCHANGERESP:
      return("WAITUSERCHANGERESP"); 
    case MDNPAUTH_STATE_WAITUPDKEYREPLY:
      return("WAITUPDKEYREPLY"); 
    case MDNPAUTH_STATE_WAITUPDKEYCONF:
      return("WAITUPDKEYCONF"); 
    default:
      return("default");
  }
}

static char *_toEvent(TMWTYPES_ULONG event)
{   
  switch(event)
  { 
    case MDNPAUTH_EVT_CHALLENGE:
      return("CHALLENGE");
    case MDNPAUTH_EVT_CHALLENGE_REPLY:
      return("CHALLENGE_REPLY");
    case MDNPAUTH_EVT_KEY_STATUS:
      return("KEY_STATUS");
    case MDNPAUTH_EVT_ERROR_RESPONSE:
      return("ERROR_RESPONSE"); 
    case MDNPAUTH_EVT_AGGRESSIVE_MODE:
      return("AGGRESSIVE_MODE");
    case MDNPAUTH_EVT_CRITICAL_RCVD:
      return("CRITICAL_RCVD");
    case MDNPAUTH_EVT_NONCRITICAL_RCVD:
      return("NONCRITICAL_RCVD");
    case MDNPAUTH_EVT_KEYCHANGETIMEOUT:
      return("KEYCHANGETIMEOUT");
    case MDNPAUTH_EVT_REPLYTIMEOUT:
      return("REPLYTIMEOUT"); 
    case MDNPAUTH_EVT_MAXINVALIDREPLIES:
      return("MAXINVALIDREPLIES");
    case MDNPAUTH_EVT_COMMFAILUREDETECTED:
      return("COMMFAILUREDETECTED");

    case MDNPAUTH_EVT_CHANGE_USERSTATUS:
      return("CHANGEUSERSTATUS");
    case MDNPAUTH_EVT_USER_CERT:
      return("USERCERT");
    case MDNPAUTH_EVT_NULLAUTH_RCVD:
      return("NULLAUTHRCVD");
    case MDNPAUTH_EVT_TX_UPDATE_KEY:
      return("TXUPDATEKEY");
    case MDNPAUTH_EVT_KEYCHANGERPLY:
      return("KEYCHANGEREPLY");
    case MDNPAUTH_EVT_KEYUPDATE_SYM:
      return("KEYUPDATESIM");
    case MDNPAUTH_EVT_TX_UPDATE_KEY_CNFM:
      return("TXUPDATEKEYCNFM");
    case MDNPAUTH_EVT_UPDKEYCHANGECONF:
      return("UPDKEYCHANGECONF"); 

#if MDNPCNFG_SUPPORT_SA_VERSION5
    case MDNPAUTH_EVT_RESTARTIIN:
      return("RESTARTIIN"); 
#endif

    default:
      return("default");
  }
}
 
/* function: mdnpdiag_authEvent */
void mdnpdiag_authEvent(
  TMWSESN *pSession,
  TMWTYPES_UCHAR state,
  TMWTYPES_ULONG event,
  TMWTYPES_ULONG userNumber)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[260];
 
  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_DATA|TMWDIAG_ID_RX) == TMWDEFS_FALSE)
  {
    return;
  }

  tmwtarg_snprintf(buf, sizeof(buf), "     %-10s Master Authentication Event, user=%u state=%s  event=%s\n", 
    tmwsesn_getSessionName(pSession), userNumber, _toState(state), _toEvent(event));

  tmwdiag_putLine(&anlzId, buf);
}
#endif /* DNPCNFG_SUPPORT_AUTHENTICATION */
#endif /* TMWCNFG_SUPPORT_DIAG */
