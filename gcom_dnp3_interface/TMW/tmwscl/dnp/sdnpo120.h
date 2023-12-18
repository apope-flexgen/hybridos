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

/* file: sdnpo120.h
 * description:  
 *   DNP Slave functionality for Object 120 Secure Authentication
 */
#ifndef SDNPO120_DEFINED
#define SDNPO120_DEFINED

#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpdata.h"

#if SDNPDATA_SUPPORT_OBJ120
#include "tmwscl/dnp/sdnpauth.h"
#include "tmwscl/dnp/sdnpevnt.h"


#ifdef __cplusplus
extern "C" {
#endif 


  /* function: sdnpo120_sendErrorResponseV7
   * purpose:
   * arguments:
   *  pSession - session we are sending this error message on.
   * returns:
   */
  void TMWDEFS_GLOBAL sdnpo120_sendErrorResponseV7(
    TMWSESN         *pSession,   
    TMWTYPES_USHORT  rxAddress,
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_ULONG   sequenceNumber,
    TMWTYPES_UCHAR   errorCode,
    TMWTYPES_CHAR   *pErrorText,
    TMWTYPES_USHORT  errorTextLength);

  /* function: sdnpo120_addEvent 
   * purpose: add a Secure Authentication Error event. The Secure Authentication 
   *  Specification recommends that errors that occur on one association should be
   *  reported to other masters if more than one association with a master exists.
   *  The data for this event will be provided by sdnpdata_authLogErrorTx().
   * arguments:
   *  pSession - pointer to session 
   *  userNumber - Secure Authentication User number.
   *  assocId - Association Id indicating where the error occurred.
   *  sequenceNumber - either Key Change Sequence Number or Challenge Sequence Number
   *   depending on the error
   *  errorCode - error code to be sent in event message           
   *    The following error codes are defined by the Secure Authentication Spec
   *     DNPAUTH_ERROR_NONE  
   *     DNPAUTH_ERROR_AUTHENTFAILED   
   *     DNPAUTH_ERROR_UNEXPECTEDREPLY  
   *     DNPAUTH_ERROR_NOREPLY       
   *     DNPAUTH_ERROR_AGGRESSNOTPERMIT  
   *     DNPAUTH_ERROR_MACNOTPERMIT 
   *     DNPAUTH_ERROR_KEYWRAPNOTPERMIT 
   *     DNPAUTH_ERROR_AUTHORFAILED   
   *  pTimeStamp - pointer to time of event
   *   This should be gotten by calling sdnputil_getDateTime(). This will
   *   set the invalid field of the time structure properly.  
   *    Even if no time is available this SHALL not be TMWDEFS_NULL. You
   *    may choose to set the DNP3 time epoch ie Jan1 1970.
   *  pErrorText - pointer to optional error text
   *  errorTextLength - length of optional error text
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpo120_addEvent(
    TMWSESN         *pSession, 
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_USHORT  assocId,
    TMWTYPES_ULONG   sequenceNumber,
    TMWTYPES_UCHAR   errorCode,
    TMWDTIME        *pTimeStamp,
    TMWTYPES_CHAR   *pErrorText,
    TMWTYPES_USHORT  errorTextLength);

     
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo120_readObj120Var7(
    SDNPEVNT *pEvent,
    TMWSESN_TX_DATA *pResponse,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpo120_readObj120v8
   * purpose: read data into response
   * arguments:
   *  pSession - session we are responding to
   *  pResponse - pointer to response message info
   *  pObjHeader - object header we are processing
   * returns:
   *  SDNPSESN_READ_COMPLETE or SDNPSESN_READ_MORE_DATA
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo120_readObj120v8(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse,
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    SDNPSESN_QUAL qualifier);

  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo120_readObj120ByClass(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWDEFS_CLASS_MASK classMask);

/* function: sdnpo120_init 
   * purpose: initialize Secure Authentication Error events
   * arguments:
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo120_init(
    TMWSESN *pSession);

  /* function: sdnpo120_close
   * purpose: 
   * arguments:
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo120_close(
    TMWSESN *pSession);

  /* function: sdnpo120_countEvents 
   * purpose: count number of Secure Authentication Error events 
   *  queued that match the requested class
   * arguments:
   * returns:
   *  void
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpo120_countEvents(
    TMWSESN *pSession, 
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_BOOL countAll,
    TMWTYPES_USHORT threshold);

  /* function: sdnpo120_cleanupEvents 
   * purpose: remove Secure Authentication Error events that have been
   *  successfully transmitted from queue.
   * arguments:
   * returns:
   *  void
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo120_cleanupEvents(
    TMWSESN *pSession, 
    TMWTYPES_BOOL deleteEvents);
     
  /* function: sdnpo120_assignClass 
  * purpose: process assign class request
  * arguments:
  *  pSession - session we are responding to
  *  pObjHeader - object header we are processing
  *  pRxFragment - received request fragment
  *  pOffset - offset into received fragment
  *  classMask - new class mask to assign
  * returns:
  *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
  */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo120_assignClass(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWDEFS_CLASS_MASK classMask);

#ifdef __cplusplus
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ120 */
#endif /* SDNPO120_DEFINED */
