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

/* file: mdnpo085.h
 * description: This file is intended for internal SCL use only.
 *   DNP Master functionality for Object 85 Data Set Prototype Objects.
 */
#ifndef MDNPO085_DEFINED
#define MDNPO085_DEFINED

#include "tmwscl/dnp/dnputil.h"
#include "tmwscl/dnp/mdnpdata.h"
#include "tmwscl/dnp/mdnpsesn.h"
#include "tmwscl/dnp/mdnpbrm.h"

#ifdef __cplusplus
extern "C" {
#endif

  /* function: mdnpo085_readObj85v1
   * purpose: Process read response for object 85 variation 1
   * arguments:
   *  pSession - identifies session
   *  pRxFragment - received fragment
   *  pObjHeader - object header for current data object
   *  pOffset - current offset into received fragment
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo085_readObj85v1(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    DNPUTIL_OBJECT_HEADER *pObjHeader);

  /* function: mdnpo085_writeV1
   * purpose: Issue a data set prototype write request using Object 85 variation 1
   * arguments:
   *  pTxData - pointer to transmit data structure
   *  pReqDesc - request descriptor containing generic parameters for this request
   *  pUserTxData - not TMWDEFS_NULL if this request is being added to an existing write
   *   request.
   *  numObjects - number of prototypes to write
   *  pPointNumbers - if TMWDEFS_NULL, start with first prototype defined on master
   *    and send numObjects prototypes. If not TMWDEFS_NULL this points to an array
   *    of point numbers for prototypes to be written.
   *  NOTE: contents of prototype will be retrieved from database 
   * returns:
   *  TMWDEFS_TRUE if write request was built successfully 
   *  TMWDEFS_FALSE if failure
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo085_writeV1( 
    TMWSESN_TX_DATA  *pTxData,
    MDNPBRM_REQ_DESC *pReqDesc,
    TMWSESN_TX_DATA  *pUserTxData,
    TMWTYPES_UCHAR    numObjects,
    TMWTYPES_USHORT  *pPointNumbers);

#ifdef __cplusplus
}
#endif
#endif /* MDNPO085_DEFINED */
