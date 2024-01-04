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

/* file: mdnpo086.h
 * description: This file is intended for internal SCL use only.
 *   DNP Master functionality for Object 86 Data Set Descriptor Objects.
 */
#ifndef MDNPO086_DEFINED
#define MDNPO086_DEFINED

#include "tmwscl/dnp/dnputil.h"
#include "tmwscl/dnp/mdnpdata.h"
#include "tmwscl/dnp/mdnpsesn.h"
#include "tmwscl/dnp/mdnpbrm.h"

#ifdef __cplusplus
extern "C" {
#endif

  /* function: mdnpo086_readObj86v1
   * purpose: Process read response for object 86 variation 1
   * arguments:
   *  pSession - identifies session
   *  pRxFragment - received fragment
   *  pObjHeader - object header for current data object
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo086_readObj86v1(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    DNPUTIL_OBJECT_HEADER *pObjHeader);

  /* function: mdnpo086_readObj86v2
   * purpose: Process read response for object 86 variation 2
   * arguments:
   *  pSession - identifies session
   *  pRxFragment - received fragment
   *  pObjHeader - object header for current data object
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo086_readObj86v2(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    DNPUTIL_OBJECT_HEADER *pObjHeader);

  /* function: mdnpo086_readObj86v3
   * purpose: Process read response for object 86 variation 3
   * arguments:
   *  pSession - identifies session
   *  pRxFragment - received fragment
   *  pObjHeader - object header for current data object
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo086_readObj86v3(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    DNPUTIL_OBJECT_HEADER *pObjHeader);
 
  /* function: mdnpo086_writeV1
   * purpose: Issue a write data set descriptor request using Object 86 v1 
   * arguments:
   *  pTxData - pointer to transmit data structure
   *  pReqDesc - request descriptor containing generic parameters for this request
   *  pUserTxData - not TMWDEFS_NULL if this request is being added to an existing write
   *   request.
   *  numObjects - number of descriptors to write
   *  pPointNumbers - if TMWDEFS_NULL, start with first descriptor defined on master
   *    and send numObjects descriptors. If not TMWDEFS_NULL this points to an array
   *    of point numbers for descriptors to be written.
   *  NOTE: contents of descriptor will be retrieved from database 
   * returns:
   *  TMWDEFS_TRUE if write request was built successfully 
   *  TMWDEFS_FALSE if failure
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo086_writeV1( 
    TMWSESN_TX_DATA *pTxData,
    MDNPBRM_REQ_DESC *pReqDesc,
    TMWSESN_TX_DATA  *pUserTxData,
    TMWTYPES_UCHAR    numObjects,
    TMWTYPES_USHORT  *pPointNumbers);
  
  /* function: mdnpo086_writeV3
   * purpose: Issue a write data set descriptor request using Object 86 v3
   * arguments:
   *  pTxData - pointer to transmit data structure
   *  pReqDesc - request descriptor containing generic parameters for this request
   *  pUserTxData - not TMWDEFS_NULL if this request is being added to an existing write
   *   request.
   *  numObjects - number of descriptors to write
   *  pPointNumbers - if TMWDEFS_NULL, start with first descriptor defined on master
   *    and send numObjects descriptors. If not TMWDEFS_NULL this points to an array
   *    of point numbers for descriptors to be written.
   *  NOTE: contents of descriptor will be retrieved from database 
   * returns:
   *  TMWDEFS_TRUE if write request was built successfully 
   *  TMWDEFS_FALSE if failure
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo086_writeV3( 
    TMWSESN_TX_DATA *pTxData,
    MDNPBRM_REQ_DESC *pReqDesc,
    TMWSESN_TX_DATA  *pUserTxData,
    TMWTYPES_UCHAR    numObjects,
    TMWTYPES_USHORT  *pPointNumbers);
 
#ifdef __cplusplus
}
#endif
#endif /* MDNPO086_DEFINED */
