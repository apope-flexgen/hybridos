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

/* file: mdnpo080.c
 * description: DNP Master functionality for Object 80 Internal Indication (IIN) bits
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpo080.h"
#include "tmwscl/dnp/mdnpdata.h"

#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"

#if MDNPDATA_SUPPORT_OBJ80

/* function: mdnpo080_readObj80v1 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo080_readObj80v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT index;
  TMWTYPES_UCHAR shift;
  TMWTYPES_BOOL value;

  index = 0;
  shift = 0;
  while(index < pObjHeader->numberOfPoints)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    /* Protect against badly formatted message */
    if(pRxFragment->offset >= pRxFragment->msgLength)
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
      return(TMWDEFS_FALSE);
    }

    if((pRxFragment->pMsgBuf[pRxFragment->offset] & (0x01 << shift)) != 0)
      value = TMWDEFS_TRUE;
    else
      value = TMWDEFS_FALSE;

    DNPDIAG_SHOW_IIN_VALUE(pSession, pointNumber, value);

    mdnpdata_storeIIN(pMDNPSession->pDbHandle, pointNumber, value);

    index = (TMWTYPES_USHORT)(index + 1);
    shift = (TMWTYPES_UCHAR)(shift + 1);
    if(shift > 7)
    {
      shift = 0;
      pRxFragment->offset += 1;
    }
  }

  if(shift != 0)
    pRxFragment->offset += 1;

  return(TMWDEFS_TRUE);
}

#endif /* MDNPDATA_SUPPORT_OBJ80 */
