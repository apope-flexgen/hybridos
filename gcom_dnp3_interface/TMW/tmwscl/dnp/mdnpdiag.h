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
 * description: This file is intended for internal SCL use only.
 *   Master DNP Diagnostics.
 */
#ifndef MDNPDIAG_DEFINED
#define MDNPDIAG_DEFINED

#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/dnp/mdnpsesn.h"

/* Define error numbers used by Master DNP */
typedef enum {
  MDNPDIAG_INV_LENGTH,
  MDNPDIAG_OBJ_LENGTH,
  MDNPDIAG_FC,
  MDNPDIAG_INV_STARTSTOP,
  MDNPDIAG_INV_QUALIFIER,
  MDNPDIAG_INV_QUALIFIER_POINT,
  MDNPDIAG_INV_VARIATION,
  MDNPDIAG_INV_VARIATION_VALUE,
  MDNPDIAG_INV_QUANTITY,
  MDNPDIAG_BAD_APPL_SEQ,
  MDNPDIAG_OBJVAR,
  MDNPDIAG_ECHO,
  MDNPDIAG_INV_8BIT_POINT,
  MDNPDIAG_FRZN_FEEDBACK,
  MDNPDIAG_OPER_FEEDBACK,
  MDNPDIAG_OPER_MEMORY,
  MDNPDIAG_INV_PROTOCOL,
  MDNPDIAG_STATUS_NOT_ZERO,
  MDNPDIAG_FILE_DIR_READ,
  MDNPDIAG_INV_BROADCAST,
  MDNPDIAG_UNSOL_STARTUP,
  MDNPDIAG_NULL_RESTART,
  MDNPDIAG_NO_CTO,
  MDNPDIAG_AUTH_FAIL,
  MDNPDIAG_AUTH_RESTKEYSEXC,

  /* This must be last entry */
  MDNPDIAG_ERROR_ENUM_MAX
} MDNPDIAG_ERROR_ENUM; 

#if !TMWCNFG_SUPPORT_DIAG

#define MDNPDIAG_SHOW_IIN(pSession, iin) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(iin);

#define MDNPDIAG_PROCESS_RESPONSE(pSession, description) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(description);

#define MDNPDIAG_SHOW_DELAY(pSession, delay, propagationDelay) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(delay); TMWTARG_UNUSED_PARAM(propagationDelay);

#define MDNPDIAG_SHOW_MSG_DELAY(pSession, delay, inMilliseconds) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(delay); TMWTARG_UNUSED_PARAM(inMilliseconds);

#define MDNPDIAG_ERROR(pChannel, pSession, errorNumber) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(errorNumber);

#define MDNPDIAG_ERROR_MSG(pChannel, pSession, errorNumber, pMsg) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(errorNumber); \
  TMWTARG_UNUSED_PARAM(pMsg);

#define MDNPDIAG_SHOW_FILE_AUTH(pSession, authKey) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(authKey);

#define MDNPDIAG_AUTHEVENT(pSession, state, event, userNumber) \
 TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(state); TMWTARG_UNUSED_PARAM(event); \
 TMWTARG_UNUSED_PARAM(event); TMWTARG_UNUSED_PARAM(userNumber);

#else

#define MDNPDIAG_SHOW_IIN(pSession, iin) \
  mdnpdiag_showIIN(pSession, iin)

#define MDNPDIAG_PROCESS_RESPONSE(pSession, description) \
  mdnpdiag_processResponse(pSession, description)

#define MDNPDIAG_SHOW_DELAY(pSession, delay, propagationDelay) \
  mdnpdiag_showDelay(pSession, delay, propagationDelay);

#define MDNPDIAG_SHOW_MSG_DELAY(pSession, delay, inMilliseconds) \
  mdnpdiag_showMsgDelay(pSession, delay, inMilliseconds)

#define MDNPDIAG_ERROR(pChannel, pSession, errorNumber) \
  mdnpdiag_errorMsg(pChannel, pSession, errorNumber, TMWDEFS_NULL)

#define MDNPDIAG_ERROR_MSG(pChannel, pSession, errorNumber, pMsg) \
  mdnpdiag_errorMsg(pChannel, pSession, errorNumber, pMsg)

#define MDNPDIAG_SHOW_FILE_AUTH(pSession, authKey) \
  mdnpdiag_showFileAuthKey(pSession, authKey)

#if DNPCNFG_SUPPORT_AUTHENTICATION
#define MDNPDIAG_AUTHEVENT(pSession, state, event, userNumber) \
  mdnpdiag_authEvent(pSession, state, event, userNumber)
#endif

#ifdef __cplusplus
extern "C" {
#endif
 
  /* routine: mdnpdiag_init
   * purpose: internal diagnostic init function
   * arguments:
   *  void
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdiag_init(void);
 
  /* routine: mdnpdiag_validateErrorTable
   * purpose: Called only to verify if error message table is correct.
   *  This is intended for test purposes only.
   * arguments:
   *  void
   * returns:
   *  TMWDEFS_TRUE if formatted correctly
   *  TMWDEFS_FALSE if there is an error in the table.
   */
  TMWTYPES_BOOL mdnpdiag_validateErrorTable(void);

  /* function: mdnpdiag_showIIN
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdiag_showIIN(
    TMWSESN *pSession, 
    TMWTYPES_USHORT iin);

  /* function: mdnpdiag_processResponse
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdiag_processResponse(
    TMWSESN *pSession, 
    const char *description);

  /* function: mdnpdiag_showDelay
   * purpose: display total time and calculated one way propagation delay
   * arguments:
   *  pSession - pointer to session
   *  delay - total time from first byte of request sent by master till 
   *   first byte of response received back on master
   *  propagationDelay - calculated one way propagation delay
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdiag_showDelay(
    TMWSESN *pSession,
    TMWTYPES_MILLISECONDS delay,
    TMWTYPES_USHORT propagationDelay);
 
  /* function: mdnpdiag_showMsgDelay
   * purpose: display delay on slave reported in msg
   * arguments:
   *  pSession - pointer to session
   *  delay - delay on slave as reported in response
   *  inMilliseconds - TRUE if delay is in milliseconds
   *                   FALSE if delay is in seconds
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdiag_showMsgDelay(
    TMWSESN *pSession,
    TMWTYPES_USHORT delay,
    TMWTYPES_BOOL inMilliseconds);
 
  /* function: mdnpdiag_error
   * purpose: Display error message
   * arguments:
   *  pChannel - channel from which this message originated
   *  pSession - session from which this message originated
   *  errorNumber - error message to display
   *  pExtraTextMsg - pointer to additional text to display with error msg
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdiag_errorMsg(
    TMWCHNL *pChannel,
    TMWSESN *pSession, 
    MDNPDIAG_ERROR_ENUM errorNumber,
    TMWTYPES_CHAR *pMsg);
 
  /* function: mdnpdiag_errorMsgEnable
   * purpose: Enable/Disable specific error message output
   * arguments:
   *  errorNumber - error message to display
   *  enabled - TMWDEFS_TRUE if error message should be enabl
   ed
   *            TMWDEFS_FALSE if error message should be disabled
   * returns:
   *  void
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdiag_errorMsgEnable(
    MDNPDIAG_ERROR_ENUM errorNumber,
    TMWTYPES_BOOL enabled);

  /* function: mdnpdiag_showAuthKey
   * purpose: Display file authentication key
   * arguments:
   *  authKey - authentication key returned from slave
   * returns:
   *  void
   */
  void mdnpdiag_showFileAuthKey(
    TMWSESN *pSession, 
    TMWTYPES_ULONG authKey);

  /* function: mdnpdiag_authEvent
   * purpose: Display authentication state and event
   * arguments:
   */
  void mdnpdiag_authEvent(
    TMWSESN *pSession,
    TMWTYPES_UCHAR state,
    TMWTYPES_ULONG event,
    TMWTYPES_ULONG userNumber);

#ifdef __cplusplus
}
#endif
#endif /* TMWCNFG_SUPPORT_DIAG */
#endif /* MDNPDIAG_DEFINED */
