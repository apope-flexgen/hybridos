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

/* file: mdnpo091.h
 * description: This file is intended for internal SCL use only.
 *   DNP Activate Configuration Response Object 91
 */
#ifndef MDNPO091_DEFINED
#define MDNPO091_DEFINED

#include "tmwscl/dnp/dnputil.h"
#include "tmwscl/dnp/mdnpsesn.h"

#ifdef __cplusplus
extern "C" {
#endif

  /* function: mdnpo091_respObj91v1
   * purpose: Process Activate Configuration response object 91 variation 1
   * arguments:
   *  pSession - identifies session
   *  pRxFragment - received fragment
   *  pObjHeader - object header for current data object
   *  pOffset - current offset into received fragment
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo091_respObj91v1(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    DNPUTIL_OBJECT_HEADER *pObjHeader);

#ifdef __cplusplus
}
#endif
#endif /* MDNPO091_DEFINED */
