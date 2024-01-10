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

/* file: sdnpdiag.h
 * description: Slave DNP Diagnostics
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwdefs.h"

#include "tmwscl/dnp/sdnpdiag.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnputil.h"
#include "tmwscl/dnp/sdnpo087.h"
#if DNPCNFG_SUPPORT_AUTHENTICATION
#include "tmwscl/dnp/sdnpauth.h"
#endif

#if TMWCNFG_SUPPORT_DIAG

static const DNPDIAG_ERROR_ENTRY sdnpDiagErrorMsg[] = {
   
  {SDNPDIAG_OPER_OBJVAR           ,  "Object/variation not supported for operate request"}, 
  {SDNPDIAG_FREEZE_OBJVAR         ,  "Object/variation not supported for freeze request"}, 
  {SDNPDIAG_WRITE_OBJVAR          ,  "Object/variation not supported for write request"},
  {SDNPDIAG_SELECT_OBJVAR         ,  "Object/variation not supported for select request"},
  {SDNPDIAG_DIRECT_OBJVAR         ,  "Object/variation not supported for direct operate request"}, 
  {SDNPDIAG_ASSIGN_OBJVAR         ,  "Object/variation not supported for assign class"}, 
  {SDNPDIAG_REQ_PENDING           ,  "Request received with previous request still pending"},
  {SDNPDIAG_CANCEL_FRAGMENT       ,  "Canceled current response fragment"},  
  {SDNPDIAG_OPER_SELECT           ,  "Operate did not follow select"}, 
  {SDNPDIAG_CHNL_BUSY             ,  "Response deferred due to channel busy"},
  {SDNPDIAG_APPL_NO_RESP          ,  "Application error with no outstanding response"},
  {SDNPDIAG_ALLOC_EVENT           ,  "Error allocating space for event"},
  {SDNPDIAG_ADD_EVENT             ,  "Error adding event"},
  {SDNPDIAG_SELECT                ,  "Error processing select request"},
  {SDNPDIAG_OPERATE               ,  "Error processing operate request"},
  {SDNPDIAG_DIR_OPERATE           ,  "Error processing direct operate request"},
  {SDNPDIAG_FREEZE                ,  "Error processing freeze request"},
  {SDNPDIAG_ASSIGN_CLASS          ,  "Error processing assign class request"}, 
  {SDNPDIAG_WRITE                 ,  "Error processing write request"}, 
  {SDNPDIAG_ENABLE_UNSOL          ,  "Enable/Disable unsolicited requests only support object 60"}, 
  {SDNPDIAG_ENABLE_UNSOL_VAR      ,  "Invalid variation for Enable/Disable unsolicited request"}, 
  {SDNPDIAG_ENABLE_UNSOL_NA       ,  "Enable/Disable unsolicited request not allowed"},
  {SDNPDIAG_RESTART_COLD          ,  "Performing Cold Restart"},
  {SDNPDIAG_RESTART_WARM          ,  "Performing Warm Restart"},
  {SDNPDIAG_UNSOL_CONF_SEQ        ,  "Invalid sequence number in unsolicited confirmation"},
  {SDNPDIAG_UNSOL_CONF_UNEXP      ,  "Unsolicited confirmation received when not expected"},
  {SDNPDIAG_RESPONSE_SEQ          ,  "Invalid sequence number in response confirmation"},
  {SDNPDIAG_RESPONSE_UNEXP        ,  "Response confirmation received when not expected"},
  {SDNPDIAG_APPL_TX               ,  "Application layer transmission failed"},
  {SDNPDIAG_APPL_TX_UNSOL         ,  "Unsolicited Response could not be sent, no connection"},
  {SDNPDIAG_TIME_NO_OBJECT        ,  "Record Current Time Request has no object data"},
  {SDNPDIAG_UNSOL_TO              ,  "Unsolicited confirmation timed out"}, 
  {SDNPDIAG_READ_POINT            ,  "Error reading point number from request"}, 
  {SDNPDIAG_REQUESTED_POINT       ,  "Requested point is not available"},
  {SDNPDIAG_NO_POINTS             ,  "No points for the requested object group are available"},
  {SDNPDIAG_PATTERN_QUAN          ,  "Invalid quantity for Pattern Control Block"},
  {SDNPDIAG_INV_QUAL_CTRL         ,  "Invalid qualifier for control"},
  {SDNPDIAG_INV_QUAL_PATTERN      ,  "Invalid qualifier for Pattern Control Block"},
  {SDNPDIAG_INV_QUAL_FILE         ,  "Invalid qualifier for file request"},
  {SDNPDIAG_INDICES_PATTERN       ,  "Invalid indices for pattern mask"},
  {SDNPDIAG_PATTERN_8_16          ,  "Pattern Mask qualifier must be 8 or 16 bit start stop"},
  {SDNPDIAG_CROB_STATUS           ,  "Status in CROB request not 0"},
  {SDNPDIAG_ANLG_STATUS           ,  "Analog control status not equal 0"},
  {SDNPDIAG_VTERM_NOTFOUND        ,  "Virtual Terminal point not found or enabled for write"},
  {SDNPDIAG_VTERM_WRITE           ,  "Error writing to virtual terminal"},
  {SDNPDIAG_STR_NOTENABLED        ,  "String not enabled for write or group (string vs extended string) is incorrect"},
  {SDNPDIAG_STR_WRITE             ,  "Error writing to string"},
  {SDNPDIAG_PATTERN_STATUS        ,  "Status must be 0 in Pattern Control Block request"},
  {SDNPDIAG_PATTERN_NOTRCVD       ,  "Pattern Control Block not received prior to Pattern Mask"},
  {SDNPDIAG_FILE_TO               ,  "File transfer timed out"},
  {SDNPDIAG_FILE_VAR              ,  "Invalid variation for file request"},
  {SDNPDIAG_FILE_AUTO_CLOSE       ,  "File automatically closed because of reopen of same filename"},
  {SDNPDIAG_FILE_AUTH             ,  "Error returned from authentication routine"},
  {SDNPDIAG_FILE_NAME             ,  "File name exceeds maximum supported"},
  {SDNPDIAG_FILE_DISABLED         ,  "File transfer not enabled"},
  {SDNPDIAG_50_QUAL               ,  "Invalid qualifier for write Object 50 Variation 1"},
  {SDNPDIAG_50_QUANT              ,  "Invalid quantity for write Object 50 Variation 1"},
  {SDNPDIAG_50_NORECORD           ,  "Record Current Time request was not received"},
  {SDNPDIAG_FREEZE_QUAL           ,  "Unsupported qualifier or invalid point indices in freeze request"},
  {SDNPDIAG_BAD_QUAL              ,  "Unsupported qualifier"},
  {SDNPDIAG_SELECT_TO_MANY        ,  "Too many control objects in select"},
  {SDNPDIAG_PARSE_ERROR           ,  "Error parsing message"},
  {SDNPDIAG_XML_SIZE_ERROR        ,  "Object won't fit in XML buffer"},
  {SDNPDIAG_AUTH_BROAD_DISC       ,  "Critical broadcast message discarded"},
  {SDNPDIAG_AUTH_NOT_AGGR         ,  "Application confirmation was not sent Aggressive Mode as required"},
  {SDNPDIAG_AUTH_BADUPDMETHOD     ,  "Secure Authentication, unsupported update key change method"},
  {SDNPDIAG_AUTH_BADUPDSCS        ,  "Secure Authentication, Status Change Sequence Number must increment"},
  {SDNPDIAG_AUTH_AUTHORFAIL       ,  "Secure Authentication, Authorization Failed"},
  {SDNPDIAG_AUTH_USEREXPIRED      ,  "Secure Authentication, User Role has expired" },
  {SDNPDIAG_AUTH_BADSIGNATURE     ,  "Secure Authentication, Signature not verified" },
  {SDNPDIAG_INDEXEDTIME_NOTENABLED,  "Indexed absolute time and long interval not enabled for write"},
  {SDNPDIAG_INDEXEDTIME_WRITE     ,  "Error writing to indexed absolute time and long interval"},

  {SDNPDIAG_ERROR_ENUM_MAX    ,  ""}
};

/* Array to determine if specific error messages are disabled.
 */
static TMWTYPES_UCHAR _errorMsgDisabled[(SDNPDIAG_ERROR_ENUM_MAX/8)+1];

/* routine: sdnpdiag_init */
void TMWDEFS_GLOBAL sdnpdiag_init()
{
  /* No error messages are disabled by default. */
  memset(_errorMsgDisabled, 0, (SDNPDIAG_ERROR_ENUM_MAX/8)+1);
}

/* routine: sdnpdiag_validateErrorTable */
TMWTYPES_BOOL sdnpdiag_validateErrorTable(void)
{
  int i;
  for(i=0; i<SDNPDIAG_ERROR_ENUM_MAX;i++)
  {
    if(sdnpDiagErrorMsg[i].errorNumber != i)
      return(TMWDEFS_FALSE);
  }

  return(TMWDEFS_TRUE);
}

/* routine: sdnpdiag_errorMsgEnable */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdiag_errorMsgEnable(
  SDNPDIAG_ERROR_ENUM errorNumber,
  TMWTYPES_BOOL value)
{
  if ((errorNumber > SDNPDIAG_ERROR_ENUM_MAX)
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
 
/* routine: _errorMsgDisabledFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _errorMsgDisabledFunc(
   SDNPDIAG_ERROR_ENUM errorNumber)
{
  int value;

  if ((errorNumber > SDNPDIAG_ERROR_ENUM_MAX)
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

/* routine: sdnpdiag_errorMsg */
void TMWDEFS_GLOBAL sdnpdiag_errorMsg(
  TMWCHNL *pChannel,
  TMWSESN *pSession, 
  SDNPDIAG_ERROR_ENUM errorNumber,
  TMWTYPES_CHAR *pExtraTextMsg)
{
  const char *pName;
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
 
  pName = tmwsesn_getSessionName(pSession);

  if(pName == (const char *)0)
    pName = " ";

  if(pExtraTextMsg == TMWDEFS_NULL)
    (void)tmwtarg_snprintf(buf, sizeof(buf), "**** %s   %s ****\n", pName, sdnpDiagErrorMsg[errorNumber].pErrorMsg);
  else
    (void)tmwtarg_snprintf(buf, sizeof(buf), "**** %s   %s  %s****\n", pName, sdnpDiagErrorMsg[errorNumber].pErrorMsg, pExtraTextMsg);

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, buf);

}

#if SDNPDATA_SUPPORT_OBJ87 && SDNPDATA_SUPPORT_OBJ88
void TMWDEFS_GLOBAL sdnpdiag_showDatasetEvent(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse)
{
  int  elemIndex;
  TMWTYPES_ULONG  stopIndex;
  TMWTYPES_ULONG  datasetId;
  TMWTYPES_ULONG  offset;
  TMWTYPES_USHORT sizePrefix;
  TMWTYPES_UCHAR  numberDescrElems;
  DNPDATA_DATASET_VALUE elem;
  DNPDEFS_DATASET_DESCR_CODE elemTypes[DNPCNFG_MAX_DATASET_DESCR_ELEMS];
  DNPDEFS_DATASET_TYPE_CODE  dataTypes[DNPCNFG_MAX_DATASET_DESCR_ELEMS];

  offset = pResponse->msgLength;

  /* Get sizePrefix */
  tmwtarg_get16(&pResponse->pMsgBuf[offset], &sizePrefix);
  offset += 2;

  stopIndex = offset + sizePrefix;

  /* Get Data Set Id from message */
  if(!dnputil_getValueFromMessage(pResponse->pMsgBuf, &offset, stopIndex, DNPDEFS_DATASET_TYPE_INT, &elem))
    return;

  datasetId = elem.value.uint32Value;

  /* Read the Data Set descriptor information, including any prototypes required and
  * store the type information on the stack.
  */
  numberDescrElems = sdnp087_datasetDescrRead(pSession, (TMWTYPES_USHORT)datasetId, DNPCNFG_MAX_DATASET_DESCR_ELEMS, dataTypes, elemTypes);

  /* Get Time from message */ 
  if(!dnputil_getValueFromMessage(pResponse->pMsgBuf, &offset, stopIndex, DNPDEFS_DATASET_TYPE_TIME, &elem))
    return;

  if(elem.type != DNPDATA_VALUE_TIME)
    return;

  DNPDIAG_SHOW_DATASET_TIME(pSession, (TMWTYPES_USHORT)datasetId, &elem.value.timeValue, TMWDEFS_FALSE, TMWDIAG_ID_RX);

  elemIndex = 0;
  while(offset < stopIndex)
  {
    DNPDEFS_DATASET_TYPE_CODE typeCode;
    if(elemIndex < numberDescrElems)
    {
      typeCode = dataTypes[elemIndex];
    }
    else
    { 
      return;
    }

    if(!dnputil_getValueFromMessage(pResponse->pMsgBuf, &offset, stopIndex, typeCode, &elem))
      return;

    /* Diagnostics */
    DNPDIAG_SHOW_DATASET(pSession, (TMWTYPES_UCHAR)elemIndex, &elem, DNPDEFS_FC_READ, TMWDEFS_FALSE, TMWDEFS_FALSE, 0); 
    elemIndex++;
  }
}
#endif


#if DNPCNFG_SUPPORT_AUTHENTICATION
static char *_toState(TMWTYPES_UCHAR state)
{
  switch(state)
  { 
    case SDNPAUTH_STATE_INIT:
      return("INIT"); 
    case SDNPAUTH_STATE_IDLE:
      return("IDLE");
    case SDNPAUTH_STATE_WAITFORREPLY:
      return("WAITFORREPLY"); 
    default:
      return("default");
  }
}

static char *_toEvent(TMWTYPES_ULONG event)
{   
  switch(event)
  {    
    case SDNPAUTH_EVT_CHALLENGE:
      return("CHALLENGE");
    case SDNPAUTH_EVT_CHALLENGE_REPLY:
      return("CHALLENGE_REPLY");
    case SDNPAUTH_EVT_KEYSTATUSREQ:
      return("KEY_STATUS_REQUEST");
    case SDNPAUTH_EVT_KEYCHANGEREQ:
      return("KEY_CHANGE_REQUEST");
    case SDNPAUTH_EVT_ERRORREQ:
      return("ERROR_REQUEST"); 
    case SDNPAUTH_EVT_AGGRESSIVE_MODE:
      return("AGGRESSIVE_MODE");
    case SDNPAUTH_EVT_CRITICAL_RCVD:
      return("CRITICAL_RCVD");
    case SDNPAUTH_EVT_NONCRITICAL_RCVD:
      return("NONCRITICAL_RCVD");
    case SDNPAUTH_EVT_EXPECTEDKEYTIMEOUT:
      return("EXPECTEDKEYCHANGETIMEOUT");
    case SDNPAUTH_EVT_REPLYTIMEOUT:
      return("REPLYTIMEOUT"); 
    case SDNPAUTH_EVT_COMMFAILUREDETECTED:
      return("COMMFAILUREDETECTED");
      
    case SDNPAUTH_EVT_MAXAUTHENTFAILURES:
      return("MAXAUTHENTFAILURES");
      
    default:
      return("default");
  }
}

/* function: SDNPDIAG_AUTHEVENT */
void TMWDEFS_GLOBAL sdnpdiag_authEvent(
  TMWSESN *pSession,
  TMWTYPES_UCHAR state,
  TMWTYPES_ULONG event)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[260]; 
  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_DATA|TMWDIAG_ID_RX) == TMWDEFS_FALSE)
  {
    return;
  }

  tmwtarg_snprintf(buf, sizeof(buf), "     %-10s Outstation Authentication Event, state=%s  event=%s\n",  
    tmwsesn_getSessionName(pSession), _toState(state), _toEvent(event));

  tmwdiag_putLine(&anlzId, buf);
} 

/* function: SDNPDIAG_AUTHDISCARD */
void TMWDEFS_GLOBAL sdnpdiag_authDiscard(
  TMWSESN *pSession,
  TMWTYPES_UCHAR state,
  TMWTYPES_ULONG event)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[260];  
  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_ERROR|TMWDIAG_ID_SECURITY_DATA|TMWDIAG_ID_RX) == TMWDEFS_FALSE)
  {
    return;
  }
  
  tmwtarg_snprintf(buf, sizeof(buf), "     %-10s Received message discarded, Authentication state=%s  event=%s\n",  
    tmwsesn_getSessionName(pSession), _toState(state), _toEvent(event));

  tmwdiag_putLine(&anlzId, buf);
} 

/* function: SDNPDIAG_AUTHDISCARDPENDING */
void TMWDEFS_GLOBAL sdnpdiag_authDiscardPending(
  TMWSESN *pSession,
  TMWTYPES_ULONG event)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[260]; 

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_ERROR|TMWDIAG_ID_SECURITY_DATA|TMWDIAG_ID_RX) == TMWDEFS_FALSE)
  {
    return;
  }
  
  tmwtarg_snprintf(buf, sizeof(buf), "     %-10s Pending critical message discarded, event=%s\n",  
    tmwsesn_getSessionName(pSession), _toEvent(event));

  tmwdiag_putLine(&anlzId, buf);
} 

/* function: sdnpdiag_authSesDecryptFail */
void TMWDEFS_GLOBAL sdnpdiag_authSesDecryptFail(
  TMWSESN *pSession,
  TMWTYPES_UCHAR algorithm)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[256];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_ERROR|TMWDIAG_ID_SECURITY_DATA|TMWDIAG_ID_RX) == TMWDEFS_FALSE)
  {
    return;
  }

  tmwtarg_snprintf(buf, sizeof(buf), "**** %s   Secure Authentication Decryption failure, algorithm %d, Verify update keys on master and outstation match ****\n",  
    tmwsesn_getSessionName(pSession), algorithm);

  tmwdiag_putLine(&anlzId, buf);
} 
 
/* function: sdnpdiag_authSesKeyFail */
void TMWDEFS_GLOBAL sdnpdiag_authSesKeyFail(
  TMWSESN *pSession,
  TMWTYPES_USHORT length,
  TMWTYPES_USHORT expectedLength)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[128];  
 
  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_ERROR|TMWDIAG_ID_SECURITY_DATA|TMWDIAG_ID_RX) == TMWDEFS_FALSE)
  {
    return;
  }

  tmwtarg_snprintf(buf, sizeof(buf), "**** %s   Secure Authentication Session Key failure, length %d expected %d****\n",  
    tmwsesn_getSessionName(pSession), length, expectedLength);

  tmwdiag_putLine(&anlzId, buf);
}  
#endif /* DNPCNFG_SUPPORT_AUTHENTICATION */
#endif /* TMWCNFG_SUPPORT_DIAG */
