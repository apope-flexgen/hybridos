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

/* file: sdnpo080.h
 * description: This file is intended for internal SCL use only.
 *   DNP Slave functionality for Object 80 IIN bits
 */
#ifndef SDNPO080_DEFINED
#define SDNPO080_DEFINED
 
#include "tmwscl/dnp/sdnpsesn.h"

#ifdef __cplusplus
extern "C" {
#endif

  /* function: sdnpo080_readObj80v1
   * purpose: read data into response using variation 1
   * arguments:
   *  pSession - session we are responding to
   *  pResponse - pointer to response message info
   *  pObjHeader - object header we are processing
   * returns:
   *  SDNPSESN_READ_COMPLETE or SDNPSESN_READ_MORE_DATA
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo080_readObj80v1(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    SDNPSESN_QUAL qualifier);

  /* function: sdnpo080_writeObj80v1
   * purpose: write data from request using variation 1
   * arguments:
   *  pSession - session we received request from
   *  pRxFragment - received request fragment
   *  pResponse - pointer to response message info
   *  pObjHeader - object header we are processing
   *  pOffset - offset into received request fragment
   * returns:
   *  SDNPSESN_READ_COMPLETE or SDNPSESN_READ_MORE_DATA
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo080_writeObj80v1(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRxFragment,
    TMWSESN_TX_DATA *pResponse,
    DNPUTIL_OBJECT_HEADER *pObjHeader);

#ifdef __cplusplus
}
#endif

#endif /* SDNPO080_DEFINED */
