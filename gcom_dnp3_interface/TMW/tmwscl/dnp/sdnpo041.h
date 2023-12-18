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

/* file: sdnpo041.h
 * description: This file is intended for internal SCL use only.
 *   DNP SLAVE functionality for Object 41 Analog Outputs
 */
#ifndef SDNPO041_DEFINED
#define SDNPO041_DEFINED

#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpdata.h"

#if SDNPDATA_SUPPORT_OBJ41

#ifdef __cplusplus
extern "C" {
#endif

#if SDNPDATA_SUPPORT_OBJ41_V1
  /* function: sdnpo041_selObj41v1
   * purpose:
   * arguments:
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo041_selObj41v1(  
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWTYPES_UCHAR numberOfPoints,
    TMWTYPES_UCHAR status);

  /* function: sdnpo041_opObj41v1
   * purpose:
   * arguments:
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo041_opObj41v1(  
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWTYPES_UCHAR numberOfPoints,
    TMWTYPES_UCHAR status);
#endif

#if SDNPDATA_SUPPORT_OBJ41_V2
  /* function: sdnpo041_selObj41v2
   * purpose:
   * arguments:
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo041_selObj41v2(  
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWTYPES_UCHAR numberOfPoints,
    TMWTYPES_UCHAR status);

  /* function: sdnpo041_opObj41v2
   * purpose:
   * arguments:
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo041_opObj41v2(  
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWTYPES_UCHAR numberOfPoints,
    TMWTYPES_UCHAR status);
#endif

#if SDNPDATA_SUPPORT_OBJ41_V3
  /* function: sdnpo041_selObj41v3
   * purpose:
   * arguments:
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo041_selObj41v3(  
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWTYPES_UCHAR numberOfPoints,
    TMWTYPES_UCHAR status);

  /* function: sdnpo041_opObj41v3
   * purpose:
   * arguments:
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo041_opObj41v3(  
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWTYPES_UCHAR numberOfPoints,
    TMWTYPES_UCHAR status);
#endif

#if SDNPDATA_SUPPORT_OBJ41_V4
  /* function: sdnpo041_selObj41v4
   * purpose:
   * arguments:
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo041_selObj41v4(  
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWTYPES_UCHAR numberOfPoints,
    TMWTYPES_UCHAR status);

  /* function: sdnpo041_opObj41v4
   * purpose:
   * arguments:
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo041_opObj41v4(  
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWTYPES_UCHAR numberOfPoints,
    TMWTYPES_UCHAR status);
#endif
  
#if SDNPDATA_SUPPORT_SELECT_CANCEL
  /* function: sdnpo041_cancelSelect
   * purpose: cancel outstanding select request.
   * arguments: 
   *  pSession - session we are responding to
   *  pRxFragment - received request fragment
   *  pObjHeader - object header we are processing
   * returns: 
   *  void
   */
  void TMWDEFS_CALLBACK sdnpo041_cancelSelect(  
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    DNPUTIL_OBJECT_HEADER *pObjHeader);
#endif

#if SDNPDATA_SUPPORT_OBJ43
  /* function: sdnpo041_assignClass
   * purpose: process assign class request
   * arguments:
   *  pSession - session we are responding to
   *  pRxFragment - received request fragment
   *  pObjHeader - object header we are processing
   *  classMask - new class mask to assign
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo041_assignClass(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRxFragment,
    DNPUTIL_OBJECT_HEADER *pObjHeader, 
    TMWDEFS_CLASS_MASK classMask);
#endif

#ifdef __cplusplus
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ41 */
#endif /* SDNPO041_DEFINED */
