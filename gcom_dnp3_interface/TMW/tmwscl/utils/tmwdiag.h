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

/* file: tmwdiag.h
 * description: General diagnostics
 */
#ifndef TMWDIAG_DEFINED
#define TMWDIAG_DEFINED
#ifndef __midl
#include "tmwscl/utils/tmwmem.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwchnl.h"
#include "tmwscl/utils/tmwsesn.h"
#include "tmwscl/utils/tmwsctr.h"
#include "tmwscl/utils/tmwdtime.h"
#endif

/* The following codes identify the source of diagnostic messages.  These
 * messages may be used for protocol analyzer displays.  Because these
 * definitions are bit masks, messages from specific layers (for example)
 * may easily be excluded from the display.
 */
typedef TMWTYPES_ULONG TMWDIAG_ID;

#define TMWDIAG_ID_PHYS           0x00000001UL
#define TMWDIAG_ID_LINK           0x00000002UL
#define TMWDIAG_ID_TPRT           0x00000004UL
#define TMWDIAG_ID_APPL           0x00000008UL
#define TMWDIAG_ID_USER           0x00000010UL
#define TMWDIAG_ID_MMI            0x00000020UL
#define TMWDIAG_ID_OPC            0x00000040UL
#define TMWDIAG_ID_OPC_DEEP       0x00800000UL
#define TMWDIAG_ID_OPC_UA         0x00000040UL /* same as TMWDIAG_ID_OPC for now */

#define TMWDIAG_ID_STATIC_DATA    0x00000080UL
#define TMWDIAG_ID_STATIC_HDRS    0x00000100UL
#define TMWDIAG_ID_EVENT_DATA     0x00000200UL
#define TMWDIAG_ID_EVENT_HDRS     0x00000400UL
#define TMWDIAG_ID_CYCLIC_DATA    0x00000800UL
#define TMWDIAG_ID_CYCLIC_HDRS    0x00001000UL
#define TMWDIAG_ID_SECURITY_DATA  0x00002000UL
#define TMWDIAG_ID_SECURITY_HDRS  0x00004000UL

#define TMWDIAG_ID_TX             0x00008000UL
#define TMWDIAG_ID_RX             0x00010000UL
#define TMWDIAG_ID_TIMESTAMP      0x00020000UL
#define TMWDIAG_ID_ERROR          0x00040000UL
#define TMWDIAG_ID_TARGET         0x00080000UL
#define TMWDIAG_ID_61850          0x00100000UL
#define TMWDIAG_ID_TASE2          0x00200000UL
#define TMWDIAG_ID_ODBC           0x00400000UL

#define TMWDIAG_ID_DEF_MASK      (TMWDIAG_ID_PHYS | TMWDIAG_ID_LINK | TMWDIAG_ID_TPRT | TMWDIAG_ID_APPL |      \
                                  TMWDIAG_ID_USER | TMWDIAG_ID_MMI  | TMWDIAG_ID_STATIC_DATA |                 \
                                  TMWDIAG_ID_STATIC_HDRS | TMWDIAG_ID_EVENT_DATA | TMWDIAG_ID_EVENT_HDRS |     \
                                  TMWDIAG_ID_CYCLIC_DATA | TMWDIAG_ID_CYCLIC_HDRS | TMWDIAG_ID_SECURITY_DATA | \
                                  TMWDIAG_ID_SECURITY_HDRS | TMWDIAG_ID_TX | TMWDIAG_ID_RX |                   \
                                  TMWDIAG_ID_TIMESTAMP | TMWDIAG_ID_ERROR | TMWDIAG_ID_TARGET)

typedef TMWTYPES_BOOL (*TMWDIAG_CHK_FILTER_CALLBACK)(
  void *pCallbackParam, 
  TMWCHNL *pChannel, 
  TMWSESN *pSession, 
  TMWSCTR *pSector, 
  TMWDIAG_ID sourceId);

#ifndef __midl
/* protocol analyzer diagnostic display identification */
typedef struct TMWDiagAnalyzerId
{
#ifdef __cplusplus
  TMWDiagAnalyzerId() : sourceId(0), pChannel(TMWDEFS_NULL), pSession(TMWDEFS_NULL), pSector(TMWDEFS_NULL) {}
#endif

  TMWDIAG_ID sourceId;
  TMWCHNL *pChannel;
  TMWSESN *pSession;
  TMWSCTR *pSector;
  TMWDTIME time;
} TMWDIAG_ANLZ_ID;

#if !TMWCNFG_SUPPORT_DIAG
#define TMWDIAG_CHECK_FILTER(pChannel, pSession, pSector, sourceId) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(pSector); TMWTARG_UNUSED_PARAM(sourceId);

#define TMWDIAG_SCTR_ERROR(pSector, errorString) \
  TMWTARG_UNUSED_PARAM(pSector); TMWTARG_UNUSED_PARAM(errorString);

#define TMWDIAG_SESN_ERROR(pSession, errorString) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(errorString);

#define TMWDIAG_CHNL_ERROR(pChannel, errorString) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(errorString);

#define TMWDIAG_SCTR_MESSAGE(pSector, errorString) \
  TMWTARG_UNUSED_PARAM(pSector); TMWTARG_UNUSED_PARAM(errorString);

#define TMWDIAG_SESN_MESSAGE(pSession, errorString) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(errorString);

#define TMWDIAG_CHNL_MESSAGE(pChannel, errorString) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(errorString);

#define TMWDIAG_ERROR(errorString) \
  TMWTARG_UNUSED_PARAM(errorString);  

#define TMWDIAG_MESSAGE(messageString, sourceId) \
  TMWTARG_UNUSED_PARAM(messageString); TMWTARG_UNUSED_PARAM(sourceId);

#define TMWDIAG_MESSAGE_NAME(name, messageString, sourceId) \
  TMWTARG_UNUSED_PARAM(name); TMWTARG_UNUSED_PARAM(messageString); TMWTARG_UNUSED_PARAM(sourceId);

#define TMWDIAG_TARGET(pChannel, sourceId, messageString) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(sourceId); TMWTARG_UNUSED_PARAM(messageString);

#else
#define TMWDIAG_CHECK_FILTER(pChannel, pSession, pSector, sourceId) \
  tmwdiag_checkFilter(pChannel, pSession, pSector, sourceId);

#define TMWDIAG_SCTR_ERROR(pSector, errorString) \
  tmwdiag_error(TMWDEFS_NULL, TMWDEFS_NULL, pSector, errorString);

#define TMWDIAG_SESN_ERROR(pSession, errorString) \
  tmwdiag_error(TMWDEFS_NULL, pSession, TMWDEFS_NULL, errorString);

#define TMWDIAG_CHNL_ERROR(pChannel, errorString) \
  tmwdiag_error(pChannel, TMWDEFS_NULL, TMWDEFS_NULL, errorString);

#define TMWDIAG_SCTR_MESSAGE(pSector, errorString) \
  tmwdiag_message(TMWDEFS_NULL, TMWDEFS_NULL, pSector, TMWDIAG_ID_MMI, errorString);

#define TMWDIAG_SESN_MESSAGE(pSession, errorString) \
  tmwdiag_message(TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_MMI, errorString);

#define TMWDIAG_CHNL_MESSAGE(pChannel, errorString) \
  tmwdiag_message(pChannel, TMWDEFS_NULL, TMWDEFS_NULL, TMWDIAG_ID_MMI, errorString);

#define TMWDIAG_ERROR(errorString) \
  tmwdiag_error(TMWDEFS_NULL, TMWDEFS_NULL, TMWDEFS_NULL, errorString)

#define TMWDIAG_MESSAGE(messageString,sourceId) \
  tmwdiag_message(TMWDEFS_NULL, TMWDEFS_NULL, TMWDEFS_NULL, sourceId, messageString)

#define TMWDIAG_MESSAGE_NAME(name, messageString, sourceId) \
  tmwdiag_message_name(name, messageString, sourceId)

#define TMWDIAG_TARGET(pChannel, sourceId, messageString) \
  tmwdiag_target(pChannel, sourceId, messageString)

#endif

#ifdef __cplusplus
extern "C" {
#endif

#if TMWCNFG_SUPPORT_DIAG
  extern const TMWTYPES_CHAR *tmwdiag_monthNames[];
  extern const TMWTYPES_CHAR *tmwdiag_dayNames[];
  extern const TMWTYPES_CHAR *tmwdiag_ampmNames[];

  /* function: tmwdiag_setChkFilterCallback 
   * purpose: Set callback function to be called before formatting a diagnostic message.
   *  This allows the application at run time to determine whether a message should be 
   *  created.
   * arguments: The callback function
   * returns: 
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdiag_setChkFilterCallback(
    TMWDIAG_CHK_FILTER_CALLBACK pCallback,
    void *pCallbackParam);

  /* function: tmwdiag_checkFilter
   * purpose:
   * arguments:
   *  pChannel - channel for this diagnostic message.
   *  pSession - session for this diagnostic message.
   *  pSector - sector for this diagnostic message.
   *  sourceId - bit mask indicating source of diagnost message.
   * returns:
   *  TMWTYPES_BOOL
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwdiag_checkFilter(
    TMWCHNL *pChannel, 
    TMWSESN *pSession, 
    TMWSCTR *pSector, 
    TMWDIAG_ID sourceId);

  /* function: tmwdiag_initId
   * purpose: this function is called before a diagnostic message is formatted. It will call
   *  an application callback function if one has been registered. That function can indicate 
   *  whether or not to format this message or to save on processing time and not format this
   *  diagnostic message
   *  pChannel - channel for this diagnostic message.
   *  pSession - session for this diagnostic message.
   *  pSector - sector for this diagnostic message.
   *  sourceId - bit mask indicating source of diagnost message.
   * arguments:
   * returns:
   *  TMWTYPES_BOOL
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwdiag_initId(
    TMWDIAG_ANLZ_ID *pAnlzId,
    TMWCHNL *pChannel,
    TMWSESN *pSession,
    TMWSCTR *pSector,
    TMWDIAG_ID sourceId);

  /* function: tmwdiag_error
   * purpose: Display error message
   * arguments:
   *  errorString - error message to display
   * returns: void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdiag_error(
    TMWCHNL *pChannel,
    TMWSESN *pSession,
    TMWSCTR *pSector,
    const TMWTYPES_CHAR *errorString);

  /* function: tmwdiag_message
   * purpose: Display message
   * arguments:
   *  name - channel/opc name
   *  messageString - message to display
   *  sourceId - message ID
   * returns: void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdiag_message_name(
    const TMWTYPES_CHAR *name,
    const TMWTYPES_CHAR *messageString,
    TMWDIAG_ID sourceId
    );

  /* function: tmwdiag_message
   * purpose: Display message
   * arguments:
   *  pChannel - channel 
   *  pSession - session
   *  pSector - sector
   *  sourceId - message ID
   *  messageString - message to display
   * returns: void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdiag_message(
    TMWCHNL *pChannel,
    TMWSESN *pSession,
    TMWSCTR *pSector,
    TMWDIAG_ID sourceId,
    const TMWTYPES_CHAR *messageString);

  /* function: tmwdiag_target
   * purpose: Display target message
   * arguments:
   *  pChannel - channel 
   *  pSession - session
   *  pSector - sector
   *  sourceId - message ID
   *  messageString - message to display
   * returns: void
   */
  void TMWDEFS_GLOBAL tmwdiag_target(
    void *pChannel,
    TMWDIAG_ID sourceId,
    const TMWTYPES_CHAR *messageString);

  /* function: tmwdiag_skipLine 
   * purpose: Put a blank line to output device
   * arguments:
   *  pAnlzId - identifies source, used for filtering
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdiag_skipLine(
    TMWDIAG_ANLZ_ID *pAnlzId);

  /* function: tmwdiag_putLine 
   * purpose: Put a single line of diagnostic info to output device
   * arguments:
   *  pAnlzId - identifies source, used for filtering
   *  buf - text to display
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdiag_putLine(
    TMWDIAG_ANLZ_ID *pAnlzId,
    const TMWTYPES_CHAR *buf);


  /* function: tmwdiag_showMemoryUsage 
   * purpose: Show the memory usage for each type specific pool  
   *   supported by the usage function passed in.
   * arguments: 
   *   pFunc - pointer to the function to retrieve individual
   *     type specific pool information.
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdiag_showMemoryUsage(
    TMWMEM_GET_USAGE_FUNC pFunc);

#endif /* TMWCNFG_SUPPORT_DIAG */

  /* function: tmwdiag_time2string 
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdiag_time2string(
    const TMWDTIME *pTimeStamp, 
    TMWDEFS_TIME_FORMAT timeFormat,
    TMWTYPES_CHAR *pTimeString, 
    TMWTYPES_UINT bufferSize, 
    TMWTYPES_BOOL forceZeros);

#ifdef __cplusplus
}
#endif

#endif /* __midl */

#endif /* TMWDIAG_DEFINED */
