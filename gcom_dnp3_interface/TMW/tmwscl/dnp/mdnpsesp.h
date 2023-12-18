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

/* file: mdnpsesp.h
 * description: This file is intended for internal SCL use only.
 *   Private master DNP session support
 */
#ifndef MDNPSESP_DEFINED
#define MDNPSESP_DEFINED

#include "tmwscl/dnp/mdnpdata.h"
#include "tmwscl/dnp/dnpsesn.h" 
#include "tmwscl/dnp/mdnpsesn.h"
#include "tmwscl/dnp/dnpbncfg.h"
#if DNPCNFG_SUPPORT_AUTHENTICATION
#include "tmwscl/dnp/mdnpauth.h"
#endif
#include "tmwscl/dnp/mdnpfile.h"
#include "tmwscl/utils/tmwchnl.h"

#define MDNPSESN_CLASS1_AUTO_ENABLE 0x1
#define MDNPSESN_CLASS2_AUTO_ENABLE 0x2
#define MDNPSESN_CLASS3_AUTO_ENABLE 0x4 


/* Define the callback method to be called when a delayed response 
 * is received from the slave. Currently this is only used for sequential
 * file transfer Object Group 70 when a response is received after an
 * immediate Null response. This could be used in general when needed.
 */
typedef void (*MDNPSESN_DELAYED_CALLBACK_FUNC)(
  TMWSESN *pSession,
  DNPCHNL_RESPONSE_INFO *pResponse);

#define MDNPSESN_UNSOL_STARTUP 0      /* Disable Unsolicited has been sent */ 
#define MDNPSESN_UNSOL_FIRSTUR 1      /* Integrity poll has completed, waiting for first unsolicited response */
#define MDNPSESN_UNSOL_IDLE    2      /* Initial Unsolicited response has been received */

/* Define MDNP Session Context */
typedef struct MDNPSessionStruct {

  /* Generic dnp Session info, must be first entry */
  DNPSESN dnp;

  /* Configuration */
  MDNPSESN_AUTO_REQ autoRequestMask;
  TMWTYPES_UCHAR autoRequestBits;

  /* Database handle */
  void *pDbHandle;

  /* Configuration */
  TMWTYPES_MILLISECONDS defaultResponseTimeout;

  /* Miscellaneous state */
  TMWTYPES_USHORT currentIIN;
  TMWTYPES_USHORT previousIIN;
  TMWTYPES_UCHAR reqSequenceNumber;

  /* as described in section 1.5 of Volume 2 Application Layer Spec Part 3 12/15/2007 */
  TMWTYPES_UCHAR unsolRespState;
  
  /* Flag to prevent sending integrity request while in context of openSession */
  TMWTYPES_BOOL openInProgress;

  /* User function to call when unsolicited message received */
  void *pUnsolUserCallbackParam;
  MDNPSESN_UNSOL_CALLBACK_FUNC pUnsolUserCallback;

#if MDNPDATA_SUPPORT_OBJ2_V3
  TMWDTIME lastCTOReceived;
#endif

#if MDNPDATA_SUPPORT_OBJ50_V1
  TMWTYPES_USHORT propagationDelay;
  TMWTYPES_MILLISECONDS delayMeasurementTxTime;
#endif

#if MDNPDATA_SUPPORT_OBJ50_V3
  /* Time last byte of last message was transmitted */
  TMWDTIME lastByteTime;
#endif

#if MDNPDATA_SUPPORT_OBJ70
  /* Indicates whether simple file or directory was opened */
  DNPDEFS_FILE_TYPE              fileXferType;

  /* Configured maximum block size for file transfer */
  TMWTYPES_USHORT                maxFileBlockSize;

  /* Pointer to file transfer context structure. Currently only
   * a single file transfer is allowed at a time
   */
  MDNPFILE_XFER_CONTEXT         *pFileXferContext;

  /* Function to be called later after immediate NULL response
   * was received.
   */
  MDNPSESN_DELAYED_CALLBACK_FUNC pInternalCallback;
#endif
  
#if MDNPDATA_SUPPORT_OBJ91
  /* combine obj70vxx to use one object header in activate config request */
  TMWTYPES_BOOL                  combineActConfigData; 
#endif

#if MDNPDATA_SUPPORT_OBJ120
  /* User function to call when Secure Authentication message is received */
  void *pSAUserCallbackParam;
  MDNPSESN_SA_CALLBACK_FUNC pSAUserCallback;

  MDNPSESN_AUTO_REQ autoRequestAggrModeMask;
  TMWTYPES_BOOL sendAggrModeConfirm;
  TMWTYPES_BOOL authenticationEnabled;
  MDNPAUTH_INFO *pAuthenticationInfo;
#endif

  /* Timer to delay sending other requests after select response
   * but before operate command is sent.
   */
  TMWTIMER selOpDelayTimer;
  TMWTYPES_MILLISECONDS selOpDelayTime;

} MDNPSESN;

#ifdef __cplusplus
extern "C" {
#endif

  /* function: mdnpsesn_processFragment */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsesn_processFragment(
    TMWSESN *pSession,
    TMWSESN_RX_DATA *pRxFragment);
  
  /* function: mdnpsesn_autoIntegrityCallback */
  void TMWDEFS_GLOBAL mdnpsesn_autoIntegrityCallback(
    void *pCallbackParam,
    DNPCHNL_RESPONSE_INFO *pResponse);

  /* function: mdnpsesn_getBinFileSessionValues
   * purpose: Read session values from a struct holding values
   * read from a binary dnp config file.
   * arguments:
   *  pSesnConfig - pointer to session config which is updated
   *  pBinFileValues - struct holding values from binary config file
   * returns: 
   *  true if successfully copied values.
   */
   TMWTYPES_BOOL mdnpsesn_getBinFileSessionValues(
     MDNPSESN_CONFIG *pSesnConfig,
     DNPBNCFG_FILEVALUES *pBinFileValues,
     TMWTYPES_BOOL isChannelSerial);

#ifdef __cplusplus
}
#endif
#endif /* MDNPSESP_DEFINED */
