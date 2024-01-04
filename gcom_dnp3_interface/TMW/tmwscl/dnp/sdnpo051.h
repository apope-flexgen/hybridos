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

/* file: sdnpo051.h
 * description: This file is intended for internal SCL use only.
 *  DNP slave functionality for Object 51 Common Time of Occurrence
 */
#ifndef SDNPO051_DEFINED
#define SDNPO051_DEFINED

#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpdata.h"

#if SDNPDATA_SUPPORT_OBJ2_V3 || SDNPDATA_SUPPORT_OBJ4_V3

#ifdef __cplusplus
extern "C" {
#endif

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo051_storeCTO(
    TMWSESN *pSession, 
    TMWSESN_TX_DATA *pResponse, 
    TMWDTIME *pCTOTime);

#ifdef __cplusplus
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ2_V3 */
#endif /* SDNPO051_DEFINED */
