/*****************************************************************************/
/* Triangle MicroWorks, Inc.                         Copyright (c) 1997-2022 */
/*****************************************************************************/
/*                                                                           */
/* This file is the property of:                                             */
/*                                                                           */
/*                       Triangle MicroWorks, Inc.                           */
/*                      Raleigh, North Carolina USA                          */
/*                       www.TriangleMicroWorks.com                          */
/*                          (919) 880-6615                                   */
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

/* file: sdnpo088.h
 * description: This file is intended for internal SCL use only.
 *   DNP Slave functionality for Object 88 Data Set Snapshot
 */
#ifndef SDNPO088_DEFINED
#define SDNPO088_DEFINED

#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpevnt.h"

#if SDNPDATA_SUPPORT_OBJ88
#ifdef __cplusplus
extern "C" {
#endif

  /* function: sdnpo088_addEvent
   * purpose: Add a data set snapshot event to event queue
   * arguments:
   *  pSession - pointer to session
   *  point - point number of point that generated the event
   *  pDataSet - pointer to array of data set value structures containing 
   *    the entire data set to be sent. The timeStamp is not included in this array.
   *  numberElems - number of data elements in the pDataSet array. This should
   *    be the same number that would be returned by sdnpdata_datasetDescrReadCont()
   *  pTimeStamp - pointer to time of event.
   *   This should be gotten by calling sdnputil_getDateTime(). This will
   *   set the invalid field of the time structure properly.  
   *    Time is required for data set support. This SHALL not be TMWDEFS_NULL.
   * returns:
   *  TMWDEFS_TRUE if event was added successfully
   *  TMWDEFS_FALSE if event was not added
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo088_addEvent(
    TMWSESN *pSession,
    TMWTYPES_USHORT point,
    DNPDATA_DATASET_VALUE *pDataSet,
    TMWTYPES_UCHAR numberElems,
    TMWDTIME *pTimeStamp);

  /* function: sdnpo088_init
  * purpose: Initialize  
  * arguments:
  *  pSession - pointer to session
  * returns:
  *  void
  */
  void TMWDEFS_GLOBAL sdnpo088_init(
    TMWSESN *pSession); 

  /* function: sdnpo088_close
  * purpose: Close  
  * arguments:
  *  pSession - pointer to session
  * returns:
  *  void
  */
  void TMWDEFS_GLOBAL sdnpo088_close(
    TMWSESN *pSession);

  /* function: sdnpo088_countEvents
  * purpose: Count the number of binary input change events in
  *  queue
  * arguments:
  *  pSession - pointer to session
  *  classMask - count events in this class or classes
  * returns:
  *  void
  */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpo088_countEvents(
    TMWSESN *pSession, 
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_BOOL countAll,
    TMWTYPES_USHORT threshold);

  /* function: sdnpo088_cleanupEvents
  * purpose: Either remove events that have been sent or clear the
  *  sent flags if message was not acknowledged.
  * arguments:
  *  pSession - pointer to session
  *  deleteEvents - remove events if true, else clear sent flag
  * returns:
  *  TMWDEFS_TRUE if specified queue is full
  *  TMWDEFS_FALSE if queue is not full
  */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo088_cleanupEvents(
    TMWSESN *pSession, 
    TMWTYPES_BOOL deleteEvents);

  /* function: sdnpo088_readObj088
   * purpose: read data into response
   * arguments:
   *  pSession - session we are responding to
   *  pResponse - pointer to response message info
   *  pObjHeader - object header we are processing
   * returns:
   *  SDNPSESN_READ_COMPLETE or SDNPSESN_READ_MORE_DATA
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo088_readObj88(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    SDNPSESN_QUAL qualifier);


  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo088_readObj88v0ByClass(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse,
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWDEFS_CLASS_MASK classMask);

  TMWTYPES_BOOL sdnpo088_storeEventData(
    TMWTYPES_UCHAR  numberElems,
    DNPDATA_DATASET_VALUE *pData, 
    SDNPEVNT_O088_EVENT *p088Event);

  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo088_readV1(
    TMWSESN *pSession,
    TMWSESN_TX_DATA *pResponse,
    SDNPEVNT *pEvent);

#ifdef __cplusplus
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ088 */
#endif /* SDNPO088_DEFINED */
