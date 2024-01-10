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

/* file: sdnpo112.h
 * description: This file is intended for internal SCL use only.
 *   DNP Slave functionality for Object 112 Virtual Terminal Output Block.
 */
#ifndef SDNPO112_DEFINED
#define SDNPO112_DEFINED

#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpdata.h"

#if SDNPDATA_SUPPORT_OBJ112

#ifdef __cplusplus
extern "C" {
#endif

  /* function: sdnpo112_writeObj112
   * purpose: write data from request
   * arguments:
   *  pSession - session we received request from
   *  pRxFragment - received request fragment
   *  pResponse - pointer to response message info
   *  pObjHeader - object header we are processing
   *  pOffset - offset into received request fragment
   * returns:
   *  TMWDEFS_TRUE if write was successful
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo112_writeObj112(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRxFragment,
    TMWSESN_TX_DATA *pResponse,
    DNPUTIL_OBJECT_HEADER *pObjHeader);
  
 /* function: sdnpo112_assignClass 
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
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo112_assignClass(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWDEFS_CLASS_MASK classMask);

#ifdef __cplusplus
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ112 */
#endif /* SDNPO112_DEFINED */
