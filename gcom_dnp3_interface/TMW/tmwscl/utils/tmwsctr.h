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

/* file: tmwsctr.h
 * description: Definition of a generic sector
 */
#ifndef TMWSCTR_DEFINED
#define TMWSCTR_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwdlist.h"
#include "tmwscl/utils/tmwsesn.h"

typedef enum TMWSCTR_STAT_EVENT_ENUM
{
  /* Indicates spontaneous event has been added to the event queue,
   * pEventData is a pointer to a TMWSCTR_STAT_EVENT_STRUCT indicating typeId 
   * and Information Object Address
   */
  TMWSCTR_STAT_EVENT_ADDED,
  
  /* Indicates event has been removed from the event queue, when event mode is
   * only most recent or queue has overflowed. pEventData is a pointer to a 
   * TMWSCTR_STAT_EVENT_STRUCT indicating typeId and Information Object Address
   */
  TMWSCTR_STAT_EVENT_REMOVED,

  /* Indicates spontaneous event queue has overflowed, event was lost 
   * pEventData is a pointer to a TMWTYPES_UCHAR indicating typeId 
   */
  TMWSCTR_STAT_EVENT_OVERFLOW, 
 
  /* 
   * Indicates an event is being sent.
   * pEventData is a pointer to a TMWSCTR_STAT_EVENT_STRUCT indicating typeId 
   * and Information Object Address of event that is being sent.
   */
  TMWSCTR_STAT_EVENT_SENT,

  /* 
   * Indicates an event has been confirmed by master
   * In some protocol configurations (for example 101 unconfirmed data) this 
   * could just mean removed from the queue because we think the master rcvd it.
   * pEventData is a pointer to a TMWSCTR_STAT_EVENT_STRUCT indicating typeId 
   * and Information Object Address of event that was acked. 
   */  
   TMWSCTR_STAT_EVENT_CONFIRM,

  /* Indicates spontaneous events have been read from queue, so queue is
   * no longer full, only sent after TMWSCTR_STAT_EVENT_OVERFLOW.
   * pEventData is a pointer to a TMWTYPES_UCHAR indicating typeId 
   */
  TMWSCTR_STAT_EVENT_NOTFULL
} TMWSCTR_STAT_EVENT;


typedef struct { 
  /* Data type id for event */
  TMWTYPES_UCHAR typeId;

  /* Information Object Address of point in event */
  TMWTYPES_ULONG ioa;
} TMWSCTR_STAT_EVENT_STRUCT;

typedef void (*TMWSCTR_STAT_CALLBACK)(
  /* User provided parameter pStatCallbackParam, provided in config structure
   * when sector was opened 
   */
  void *pCallbackParam, 

  /* Indicates what statistic event has occurred */
  TMWSCTR_STAT_EVENT eventType, 

  /* Pointer to additional information specific to the
   * statistic event that occurred. 
   * For both TMWSCTR_STAT_EVENT_OVERFLOW and TMWSCTR_STAT_EVENT_NOTFULL
   * this is a pointer to an TMWTYPES_UCHAR indicating what event queue
   * overflowed. ie I14DEF_TYPE_MSPNA1 for 101/104 single point.
   * For TMWSCTR_STAT_EVENT_SENT and TMWSCTR_STAT_EVENT_CONFIRM this is a pointer
   * to a TMWSCTR_STAT_EVENT_STRUCT structure.
   */
  void *pEventData);

/* TMW Sector */
typedef struct TMWSectorStruct {
  /* List Member, must be first entry */
  TMWDLIST_MEMBER listMember;

  /* Pointer to session that this sector belongs to */
  TMWSESN *pSession;
  
  /* Statistics Callback */
  void *pStatCallbackParam;
  TMWSCTR_STAT_CALLBACK pStatCallbackFunc;

  /* User Data*/
  void *pUserData;

  /* Diagnostic mask */
  TMWTYPES_ULONG sctrDiagMask;

} TMWSCTR;


#if TMWCNFG_SUPPORT_STATS
#define TMWSCTR_STAT_CALLBACK_FUNC(pSector, eventType, pEventData) \
  tmwsctr_callStatCallback(pSector, eventType, pEventData)
 #else

#define TMWSCTR_STAT_CALLBACK_FUNC(pSector, eventType, pEventData) \
  TMWTARG_UNUSED_PARAM(pSector); TMWTARG_UNUSED_PARAM(eventType); TMWTARG_UNUSED_PARAM(pEventData);
#endif

#ifdef __cplusplus
extern "C" {
#endif

  /* function: tmwsctr_openSector
   * purpose:
   * arguments:
   * returns:
   *  TMWDEFS_TRUE if open was successful
   *  TMWDEFS_FALSE otherwise
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsctr_openSector(
    TMWSCTR *pSector, 
    TMWSESN *pSession,
    TMWSCTR_STAT_CALLBACK pCallback,
    void *pCallbackParam);

  /* function: tmwsctr_setStatCallback
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsctr_setStatCallback(
    TMWSCTR *pSector,
    TMWSCTR_STAT_CALLBACK pCallback,
    void *pCallbackParam);
    
  /* function: tmwsctr_callStatCallback
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsctr_callStatCallback(
    TMWSCTR *pSector,
    TMWSCTR_STAT_EVENT eventType,
    void *pEventData);

  /* function: tmwsctr_closeSector
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsctr_closeSector(
    TMWSCTR *pSector);

  /* function: tmwsctr_getSectorName
   * purpose: returns the name of this sector
   * arguments:
   *  pSector - pointer to Sector structure
   * returns:
   *  pointer to a string which contains the name of this
   *  sector.
   */
  TMWDEFS_SCL_API const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwsctr_getSectorName(
    TMWSCTR *pSector);

  /* function: tmwsctr_getChannelName 
   * purpose: returns the name of the channel this sector is on
   * arguments:
   *  pSector - pointer to Sector structure
   * returns:
   *  pointer to a string which contains the name of the
   *  channel.
   */
  TMWDEFS_SCL_API const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwsctr_getChannelName(
    TMWSCTR *pSector);
  
  /* function: tmwsctr_setUserDataPtr
   * purpose: Tell the sector to store this pointer to user data
   *   This can be used by the target application code for easier lookup
   *   into data related to the SCL sector
   * arguments:
   *  pSector - sector to set
   *  pUserData - pointer to user data. Will not be used by SCL
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsctr_setUserDataPtr(
    TMWSCTR *pSector, 
    void *pUserData);

  /* function: tmwsctr_getUserDataPtr
   * purpose: Ask the sector for the pointer that was stored by tmwsctr_setUserDataPtr
   * arguments:
   *  pSector - sector to get data from 
   * returns:
   *  pUserData - pointer to user data.
   */
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL tmwsctr_getUserDataPtr(
    TMWSCTR *pSector);

#ifdef __cplusplus
}
#endif
#endif /* TMWSCTR_DEFINED */
