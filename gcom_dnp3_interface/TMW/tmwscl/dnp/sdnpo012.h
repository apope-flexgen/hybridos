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

/* file: sdnpo012.h
 * description: This file is intended for internal SCL use only.
 *   DNP SLAVE functionality for Object 12 Binary Outputs
 *  
 */
#ifndef SDNPO012_DEFINED
#define SDNPO012_DEFINED

#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpdata.h"

#if SDNPDATA_SUPPORT_OBJ12

#ifdef __cplusplus
extern "C" {
#endif

#if SDNPDATA_SUPPORT_OBJ12_V1
  /* function: sdnpo012_selObj12v1
   * purpose: Select binary output control
   * arguments:
   * returns:
   *  TMWDEFS_TRUE if successfull, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo012_selObj12v1(  
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWTYPES_UCHAR numberOfPoints,
    TMWTYPES_UCHAR status);

  /* function: sdnpo012_opObj12v1
   * purpose: Operate binary output control
   * arguments:
   * returns:
   *  TMWDEFS_TRUE if successfull, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo012_opObj12v1(  
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWTYPES_UCHAR numberOfPoints,
    TMWTYPES_UCHAR status);
#endif

#if SDNPDATA_SUPPORT_OBJ12_V2
  /* function: sdnpo012_initObj12v2
   * purpose: Initialize pattern control block response
   * arguments:
   * returns:
   *  void
   */
  void TMWDEFS_CALLBACK sdnpo012_initObj12v2(
    TMWSESN *pSession);

  /* function: sdnpo012_selObj12v2
   * purpose: Select Pattern Control Block (PCB)
   * arguments:
   * returns:
   *  TMWDEFS_TRUE if successfull, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo012_selObj12v2(  
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWTYPES_UCHAR numberOfPoints,
    TMWTYPES_UCHAR status);

  /* function: sdnpo012_opObj12v2
   * purpose: Operate Pattern Control Block
   * arguments:
   * returns:
   *  TMWDEFS_TRUE if successfull, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo012_opObj12v2(  
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWTYPES_UCHAR numberOfPoints,
    TMWTYPES_UCHAR status);
#endif

#if SDNPDATA_SUPPORT_OBJ12_V3
  /* function: sdnpo012_selObj12v3
   * purpose: Select Pattern Mask
   * arguments:
   * returns:
   *  TMWDEFS_TRUE if successfull, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo012_selObj12v3(  
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWTYPES_UCHAR numberOfPoints,
    TMWTYPES_UCHAR status);

  /* function: sdnpo012_opObj12v3
   * purpose: Operate Pattern Mask
   * arguments:
   * returns:
   *  TMWDEFS_TRUE if successfull, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo012_opObj12v3(  
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWTYPES_UCHAR numberOfPoints,
    TMWTYPES_UCHAR status);
#endif

#if SDNPDATA_SUPPORT_SELECT_CANCEL 
  /* function: sdnpo012_cancelSelect
   * purpose: cancel outstanding select request
   * arguments:
   *  pSession - session we are responding to
   *  pRxFragment - received request fragment
   *  pObjHeader - object header we are processing
   * returns:
   *  void
   */
  void TMWDEFS_CALLBACK sdnpo012_cancelSelect(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    DNPUTIL_OBJECT_HEADER *pObjHeader);
#endif

#if SDNPDATA_SUPPORT_OBJ13
  /* function: sdnpo012_assignClass
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
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo012_assignClass(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRxFragment,
    DNPUTIL_OBJECT_HEADER *pObjHeader, 
    TMWDEFS_CLASS_MASK classMask);
#endif

#ifdef __cplusplus
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ12 */
#endif /* SDNPO012_DEFINED */
