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
 * description: This file is intended for internal SCL use only.
 *  Slave DNP Diagnostics
 */
#ifndef SDNPDIAG_DEFINED
#define SDNPDIAG_DEFINED

#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpevnt.h"

/* Define error numbers used by Slave DNP */
typedef enum {
  SDNPDIAG_OPER_OBJVAR,
  SDNPDIAG_FREEZE_OBJVAR,
  SDNPDIAG_WRITE_OBJVAR,
  SDNPDIAG_SELECT_OBJVAR,
  SDNPDIAG_DIRECT_OBJVAR,
  SDNPDIAG_ASSIGN_OBJVAR,
  SDNPDIAG_REQ_PENDING,
  SDNPDIAG_CANCEL_FRAGMENT,  
  SDNPDIAG_OPER_SELECT,     
  SDNPDIAG_CHNL_BUSY,  
  SDNPDIAG_APPL_NO_RESP,
  SDNPDIAG_ALLOC_EVENT, 
  SDNPDIAG_ADD_EVENT, 
  SDNPDIAG_SELECT,
  SDNPDIAG_OPERATE,
  SDNPDIAG_DIR_OPERATE,
  SDNPDIAG_FREEZE,
  SDNPDIAG_ASSIGN_CLASS,
  SDNPDIAG_WRITE,
  SDNPDIAG_ENABLE_UNSOL, 
  SDNPDIAG_ENABLE_UNSOL_VAR, 
  SDNPDIAG_ENABLE_UNSOL_NA,
  SDNPDIAG_RESTART_COLD,
  SDNPDIAG_RESTART_WARM,
  SDNPDIAG_UNSOL_CONF_SEQ,
  SDNPDIAG_UNSOL_CONF_UNEXP,
  SDNPDIAG_RESPONSE_SEQ, 
  SDNPDIAG_RESPONSE_UNEXP, 
  SDNPDIAG_APPL_TX, 
  SDNPDIAG_APPL_TX_UNSOL, 
  SDNPDIAG_TIME_NO_OBJECT, 
  SDNPDIAG_UNSOL_TO,
  SDNPDIAG_READ_POINT,
  SDNPDIAG_REQUESTED_POINT,
  SDNPDIAG_NO_POINTS,
  SDNPDIAG_PATTERN_QUAN,
  SDNPDIAG_INV_QUAL_CTRL,
  SDNPDIAG_INV_QUAL_PATTERN,
  SDNPDIAG_INV_QUAL_FILE,
  SDNPDIAG_INDICES_PATTERN,
  SDNPDIAG_PATTERN_8_16,  
  SDNPDIAG_CROB_STATUS,   
  SDNPDIAG_ANLG_STATUS,
  SDNPDIAG_VTERM_NOTFOUND,
  SDNPDIAG_VTERM_WRITE,
  SDNPDIAG_STR_NOTENABLED,
  SDNPDIAG_STR_WRITE,
  SDNPDIAG_PATTERN_STATUS,
  SDNPDIAG_PATTERN_NOTRCVD,
  SDNPDIAG_FILE_TO,
  SDNPDIAG_FILE_VAR,
  SDNPDIAG_FILE_AUTO_CLOSE,
  SDNPDIAG_FILE_AUTH,
  SDNPDIAG_FILE_NAME,
  SDNPDIAG_FILE_DISABLED,
  SDNPDIAG_50_QUAL,
  SDNPDIAG_50_QUANT,     
  SDNPDIAG_50_NORECORD,   
  SDNPDIAG_FREEZE_QUAL,
  SDNPDIAG_BAD_QUAL,
  SDNPDIAG_SELECT_TO_MANY,
  SDNPDIAG_PARSE_ERROR,
  SDNPDIAG_XML_SIZE_ERROR,
  SDNPDIAG_AUTH_BROAD_DISC,
  SDNPDIAG_AUTH_NOT_AGGR,
  SDNPDIAG_AUTH_BADUPDMETHOD,
  SDNPDIAG_AUTH_BADUPDSCS,
  SDNPDIAG_AUTH_AUTHORFAIL,
  SDNPDIAG_AUTH_USEREXPIRED,
  SDNPDIAG_AUTH_BADSIGNATURE,
  SDNPDIAG_INDEXEDTIME_NOTENABLED,
  SDNPDIAG_INDEXEDTIME_WRITE,

  /* This must be last entry */
  SDNPDIAG_ERROR_ENUM_MAX
} SDNPDIAG_ERROR_ENUM; 


#if !TMWCNFG_SUPPORT_DIAG

#define SDNPDIAG_ERROR(pChannel, pSession, errorNumber) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(errorNumber);

#define SDNPDIAG_ERROR_MSG(pChannel, pSession, errorNumber, pMsg) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(errorNumber);\
  TMWTARG_UNUSED_PARAM(pMsg);

#define SDNPDIAG_SHOW_DATASET_EVENT(pSession, pResponse) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(pResponse);

#define SDNPDIAG_AUTHEVENT(pSession, state, event) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(state); TMWTARG_UNUSED_PARAM(event);

#define SDNPDIAG_AUTHDISCARD(pSession, state, event) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(state); TMWTARG_UNUSED_PARAM(event);

#define SDNPDIAG_AUTHDISCARDPENDING(pSession, event) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(event);

#define SDNPDIAG_AUTHSESDECRYPTFAIL(pSession, algorithm) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(algorithm); 

#define SDNPDIAG_AUTHSESKEYFAIL(pSession, length, expectedLength) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(length); TMWTARG_UNUSED_PARAM(expectedLength); 

#else

#define SDNPDIAG_ERROR(pChannel, pSession, errorNumber) \
  sdnpdiag_errorMsg(pChannel, pSession, errorNumber, TMWDEFS_NULL)

#define SDNPDIAG_ERROR_MSG(pChannel, pSession, errorNumber, pMsg) \
  sdnpdiag_errorMsg(pChannel, pSession, errorNumber, pMsg)


#define SDNPDIAG_SHOW_DATASET_EVENT(pEvent, pResponse) \
  sdnpdiag_showDatasetEvent(pSession, pResponse)

#define SDNPDIAG_AUTHEVENT(pSession, state, event) \
  sdnpdiag_authEvent(pSession, state, event)

#define SDNPDIAG_AUTHDISCARD(pSession, state, event) \
  sdnpdiag_authDiscard(pSession, state, event)  

#define SDNPDIAG_AUTHDISCARDPENDING(pSession, event) \
  sdnpdiag_authDiscardPending(pSession, event)  

#define SDNPDIAG_AUTHSESDECRYPTFAIL(pSession, algorithm) \
  sdnpdiag_authSesDecryptFail(pSession, algorithm)
 
#define SDNPDIAG_AUTHSESKEYFAIL(pSession, length, expectedLength) \
  sdnpdiag_authSesKeyFail(pSession, length, expectedLength) 

#endif


#if TMWCNFG_SUPPORT_DIAG
#ifdef __cplusplus
extern "C" {
#endif

  /* routine: sdnpdiag_init
   * purpose: internal diagnostic init function
   * arguments:
   *  void
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdiag_init(void);

  /* routine: sdnpdiag_validateErrorTable
   * purpose: Called only to verify if error message table is correct.
   *  This is intended for test purposes only.
   * arguments:
   *  void
   * returns:
   *  TMWDEFS_TRUE if formatted correctly
   *  TMWDEFS_FALSE if there is an error in the table.
   */
  TMWTYPES_BOOL sdnpdiag_validateErrorTable(void);

  /* function: sdnpdiag_error
   * purpose: Display error message
   * arguments:
   *  pChannel - channel from which this message originated
   *  pSession - session from which this message originated
   *  errorNumber - enum indicating what error message to display
   *  pExtraTextMsg - pointer to additional text to display with error msg
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdiag_errorMsg(
    TMWCHNL *pChannel,
    TMWSESN *pSession, 
    SDNPDIAG_ERROR_ENUM errorNumber,
    TMWTYPES_CHAR *pExtraTextMsg);

  /* function: sdnpdiag_errorMsgEnable
   * purpose: Enable/Disable specific error message output
   * arguments:
   *  errorNumber - enum indicating what error message 
   *  enabled - TMWDEFS_TRUE if error message should be enabled
   *            TMWDEFS_FALSE if error message should be disabled
   * returns:
   *  void
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdiag_errorMsgEnable(
    SDNPDIAG_ERROR_ENUM errorNumber,
    TMWTYPES_BOOL enabled);

  void TMWDEFS_GLOBAL sdnpdiag_showDatasetEvent(
    TMWSESN *pSession,
    TMWSESN_TX_DATA *pResponse);

  /* function: sdnpdiag_authEvent
   * purpose: Display authentication state and event
   * arguments:
   */
  void sdnpdiag_authEvent(
    TMWSESN *pSession,
    TMWTYPES_UCHAR state,
    TMWTYPES_ULONG event);

  /* function: sdnpdiag_authDiscard
   * purpose: Display message indicating message was discarded.
   * arguments:
   */
  void sdnpdiag_authDiscard(
    TMWSESN *pSession,
    TMWTYPES_UCHAR state,
    TMWTYPES_ULONG event);

  /* function: sdnpdiag_authDiscardPending
   * purpose: Display message indicating pending critical message was discarded.
   * arguments:
   */
  void sdnpdiag_authDiscardPending(
    TMWSESN *pSession,
    TMWTYPES_ULONG event);

  /* function: sdnpdiag_authDecryptFail
   * purpose: Display authentication 
   * arguments:
   */
  void sdnpdiag_authSesDecryptFail(
    TMWSESN *pSession,
    TMWTYPES_UCHAR algorithm);

  /* function: sdnpdiag_authSesKeyFail */
  void TMWDEFS_GLOBAL sdnpdiag_authSesKeyFail(
    TMWSESN *pSession,
    TMWTYPES_USHORT length,
    TMWTYPES_USHORT expectedLength); 
     

#ifdef __cplusplus
}
#endif
#endif /* TMWCNFG_SUPPORT_DIAG */
#endif /* SDNPDIAG_DEFINED */
