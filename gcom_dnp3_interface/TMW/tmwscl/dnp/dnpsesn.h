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

/* file: dnpsesn.h
 * description: This file is intended for internal SCL use only.
 *   Definition of a generic DNP session.
 */
#ifndef DNPSESN_DEFINED
#define DNPSESN_DEFINED

#include "tmwscl/utils/tmwsesn.h"
#include "tmwscl/utils/tmwdlist.h"
#include "tmwscl/utils/tmwtimer.h"
#include "tmwscl/utils/tmwchnl.h"
#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/dnpchnl.h"

/* DNP Context */
typedef void (*DNPSESN_TX_CALLBACK_FUNC)(TMWSESN_TX_DATA *pTxData);
typedef void (*DNPSESN_PREPARE_MSG_FUNC)(TMWSESN *pSession, TMWSESN_TX_DATA *pTxData);
typedef void (*DNPSESN_ABORT_MSG_FUNC)(TMWSESN *pSession, TMWSESN_TX_DATA *pTxData, DNPCHNL_RESP_STATUS status);
typedef void (*DNPSESN_NEXT_MSG_FUNC)(TMWSESN *pSession);
typedef void (*DNPSESN_PROCESS_INFO_FUNC)(TMWSESN *pSession, TMWSCL_INFO sesnInfo);
typedef TMWTYPES_BOOL (*DNPSESN_PROCESS_FRAGMENT_FUNC)(TMWSESN *pSession, TMWSESN_RX_DATA *pRxData);
typedef TMWTYPES_BOOL (*DNPSESN_IDLE_FUNC)(TMWSESN *pSession);
typedef TMWTYPES_BOOL (*DNPSESN_CHECKDATA_FUNC)(TMWSESN *pSession);

typedef void(*DNPSESN_SAVE_UNSOL_FUNC)(TMWSESN *pSession, TMWSESN_TX_DATA *pTxData);

typedef struct DNPSessionStruct {

  /* Generic TMW session, must be first field */
  TMWSESN tmw;

  /* Configuration */
  TMWTYPES_MILLISECONDS linkStatusPeriod;
  TMWTYPES_BOOL         linkStatusTimeoutDisconnect;
  TMWTYPES_BOOL         validateSourceAddress;
  TMWTYPES_BOOL         enableSelfAddress;

  /* suppress duplicate request error message, used when automatic sending of requests is enabled */
  TMWTYPES_BOOL         suppressError;
  

  /* Number of read timeous allowed before the session is marked offline */
  TMWTYPES_UCHAR readTimeoutsAllowed;
  TMWTYPES_UCHAR readFailedCount; 

  /* Function to process received fragment */
  DNPSESN_TX_CALLBACK_FUNC      pBeforeTxCallback;
  DNPSESN_TX_CALLBACK_FUNC      pAfterTxCallback;
  DNPSESN_TX_CALLBACK_FUNC      pFailedTxCallback;
  DNPSESN_PREPARE_MSG_FUNC      pPrepareMessage;
  DNPSESN_ABORT_MSG_FUNC        pAbortMessage;
  DNPSESN_NEXT_MSG_FUNC         pNextMessage;
  DNPSESN_PROCESS_INFO_FUNC     pProcessInfoFunc;
  DNPSESN_PROCESS_FRAGMENT_FUNC pProcessFragmentFunc;
  DNPSESN_IDLE_FUNC             pIdleFunc;
  DNPSESN_CHECKDATA_FUNC        pCheckData; 


  /* User Data passed into opensession */
  void *pUserHandle;
  void *pCryptoHandle;

  /* Current request being sent */
  TMWSESN_TX_DATA *pCurrentMessage;

#if DNPCNFG_SUPPORT_AUTHENTICATION
  /* Secure Authentication Version 2 */
  TMWTYPES_BOOL             operateInV2Mode;

  /* Required by application spec, needed for authentication */
  TMWTYPES_BOOL   saveLastTxFragment;

#if DNPCNFG_MULTI_SESSION_REQUESTS
  /* Pointer to session function to save last Unsol needed in SDNP only. 
   * Memory is not in this shared base structure
   */
  DNPSESN_SAVE_UNSOL_FUNC   pSaveLastUnsolSentFunc;

  /* If allowing simultaneous requests on multiple sessions on a channel */
  TMWTYPES_USHORT lastTxFragmentLength;
  TMWTYPES_UCHAR  lastTxFragment[DNPCNFG_MAX_TX_FRAGMENT_LENGTH];
#endif
#endif

} DNPSESN;

#ifdef __cplusplus
extern "C" {
#endif

  /* function: dnpsesn_openSession
   * purpose: Open common part of a dnp3 master or slave session.
   * arguments:
   *  pChannel - channel to open session on
   *  enableSelfAddress - TMWDEFS_TRUE means respond to 0xfffc as though
   *    it were your own address. Only makes sense for slave. Master session
   *    can send self address even if this is TMWDEFS_FALSE 
   * returns:
   *  Pointer to new session or TMWDEFS_NULL.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL dnpsesn_openSession(
    TMWCHNL *pChannel,
    TMWSESN *pSession,
    TMWSESN_STAT_CALLBACK pCallback,
    void *pCallbackParam,
    TMWTYPES_SESSION_TYPE type,
    TMWTYPES_MILLISECONDS linkStatusPeriod,
    TMWTYPES_BOOL validateSourceAddress,
    TMWTYPES_BOOL enableSelfAddress);

  void TMWDEFS_GLOBAL dnpsesn_closeSession(
    TMWSESN *pSession);

#ifdef __cplusplus
}
#endif

#endif /* DNPSESN_DEFINED */
