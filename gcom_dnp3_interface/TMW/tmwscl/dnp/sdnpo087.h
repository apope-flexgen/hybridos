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

/* file: sdnpo087.h
 * description: This file is intended for internal SCL use only.
 *   DNP Slave functionality for Object 87 Data Set Present Value Object
 */
#ifndef SDNPO087_DEFINED
#define SDNPO087_DEFINED

#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpdata.h"

#if SDNPDATA_SUPPORT_OBJ87
#ifdef __cplusplus
extern "C" {
#endif
 

  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnp087_datasetDescrRead(
    TMWSESN *pSession, 
    TMWTYPES_USHORT datasetId,
    TMWTYPES_UCHAR  maxNumber,
    DNPDEFS_DATASET_TYPE_CODE *pDataTypes,
    DNPDEFS_DATASET_DESCR_CODE *pElemTypes);

  /* function: sdnpo087_readObj087
   * purpose: read data into response
   * arguments:
   *  pSession - session we are responding to
   *  pResponse - pointer to response message info
   *  pObjHeader - object header we are processing
   * returns:
   *  SDNPSESN_READ_COMPLETE or SDNPSESN_READ_MORE_DATA
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo087_readObj87(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    SDNPSESN_QUAL qualifier);

  /* function: sdnpo087_writeObj087v1
   * purpose: write data from request
   * arguments:
   *  pSession - session we received request from
   *  pRxFragment - received request fragment
   *  pResponse - pointer to response message info
   *  pObjHeader - object header we are processing
   * returns:
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo087_writeObj087v1(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    TMWSESN_TX_DATA *pResponse,
    DNPUTIL_OBJECT_HEADER *pObjHeader);

  /* function: sdnpo087_selObj087v1
   * purpose: issue control select containing data from request
   * arguments:
   *  pSession - session we received request from
   *  pRxFragment - received request fragment
   *  pResponse - pointer to response message info
   *  pObjHeader - object header we are processing
   *  numberOfPoints
   *  status - 
   * returns:
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo087_selObj087v1(  
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWTYPES_UCHAR numberOfPoints,
    TMWTYPES_UCHAR status);

  /* function: sdnpo087_opObj087v1
   * purpose: issue control operate containing data from request
   * arguments:
   *  pSession - session we received request from
   *  pRxFragment - received request fragment
   *  pResponse - pointer to response message info
   *  pObjHeader - object header we are processing
   *  numberOfPoints
   *  status - 
   * returns:
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo087_opObj087v1(  
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWTYPES_UCHAR numberOfPoints,
    TMWTYPES_UCHAR status);

#if SDNPDATA_SUPPORT_SELECT_CANCEL
  /* function: sdnpo087_cancelSelect
   * purpose: cancel outstanding select request
   * arguments:
   *  pSession - session we received request from
   *  pRxFragment - received request fragment
   *  pObjHeader - object header we are processing
   * returns:
   *  void
   */
  void TMWDEFS_CALLBACK sdnpo087_cancelSelect(  
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    DNPUTIL_OBJECT_HEADER *pObjHeader);
#endif

#ifdef __cplusplus
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ087 */
#endif /* SDNPO087_DEFINED */
