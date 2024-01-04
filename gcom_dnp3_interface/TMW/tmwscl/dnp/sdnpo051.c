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

/* file: sdnpo051.c
 * description: DNP slave functionality for Object 51 Common Time of 
 *  Occurrence
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/dnp/dnpdtime.h"
#include "tmwscl/dnp/sdnpo051.h"
#include "tmwscl/dnp/sdnpdata.h"

#if SDNPDATA_SUPPORT_OBJ2_V3 || SDNPDATA_SUPPORT_OBJ4_V3
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo051_storeCTO(
  TMWSESN *pSession, 
  TMWSESN_TX_DATA *pResponse, 
  TMWDTIME *pCTOTime)
{
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWTYPES_UCHAR variation;

  TMWTARG_UNUSED_PARAM(pSession);

  if((pResponse->msgLength + 10) >= pResponse->maxLength)
    return(TMWDEFS_FALSE);

  /* Group */
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_OBJ_51_TIME_DATE_CTO;

  /* Variation */
  if(pCTOTime->invalid)
  {
    variation = 2;
    pResponse->pMsgBuf[pResponse->msgLength++] = 2;
  }
  else
  {
    variation = 1;
    pResponse->pMsgBuf[pResponse->msgLength++] = 1;
  }

  /* Qualifier */
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_QUAL_8BIT_LIMITED_QTY;

  /* Quantity */
  pResponse->pMsgBuf[pResponse->msgLength++] = 1;

  /* Time */
  dnpdtime_dateTimeToMSSince70(&msSince70, pCTOTime);
  dnpdtime_writeMsSince70(pResponse->pMsgBuf + pResponse->msgLength, &msSince70);
  pResponse->msgLength += 6;
  
  DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, DNPDEFS_OBJ_51_TIME_DATE_CTO, variation, DNPDEFS_QUAL_8BIT_LIMITED_QTY); 

  return(TMWDEFS_TRUE);
}
#endif
