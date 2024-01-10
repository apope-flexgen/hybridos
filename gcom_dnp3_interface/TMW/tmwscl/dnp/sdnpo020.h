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

/* file: sdnpo020.h
 * description: This file is intended for internal SCL use only.
 *   DNP SLAVE functionality for Object 20 Binary Counters
 *  
 */
#ifndef SDNPO020_DEFINED
#define SDNPO020_DEFINED

#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpdata.h"

#if SDNPDATA_SUPPORT_OBJ20

#ifdef __cplusplus
extern "C" {
#endif

  /* function: sdnpo020_readObj20
   * purpose: read data into response using specified variation
   * arguments:
   *  pSession - session we are responding to
   *  pResponse - pointer to response message info
   *  pObjHeader - object header we are processing
   * returns:
   *  SDNPSESN_READ_COMPLETE or SDNPSESN_READ_MORE_DATA
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo020_readObj20(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    SDNPSESN_QUAL qualifier);

#if SDNPDATA_SUPPORT_OBJ21
  /* function: sdnp020_freeze
   * purpose: Freeze counters
   * arguments:
   *  pSession - session we are responding to
   *  functionCode - function code of message causing this freeze
   *  pObjHeader - object header we are processing
   *  pFreezeTime - time to perform freeze
   *  freezeInterval - time interval to perfrom periodic freezes (milliseconds)
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnp020_freeze(
    TMWSESN *pSession,
    TMWTYPES_UCHAR functionCode, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    DNPDATA_FREEZE_TIME_DATE_FIELD timeDateEnum,
    TMWDTIME *pFreezeTime,
    TMWTYPES_ULONG freezeInterval);
#endif

#if SDNPDATA_SUPPORT_OBJ22
  /* function: sdnpo020_assignClass
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
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo020_assignClass(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRxFragment,
    DNPUTIL_OBJECT_HEADER *pObjHeader, 
    TMWDEFS_CLASS_MASK classMask);
#endif

#ifdef __cplusplus
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ20 */
#endif /* SDNPO020_DEFINED */
