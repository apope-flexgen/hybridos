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

/* file: sdnpo112.c
 * description: DNP Slave functionality for Object 112 Virtual Terminal
 *  Output Block.
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/sdnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"

#include "tmwscl/dnp/sdnpo112.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnputil.h"

#if SDNPDATA_SUPPORT_OBJ112
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo112_writeObj112(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_BOOL status = TMWDEFS_TRUE;
  TMWTYPES_UCHAR strLength;
  TMWTYPES_USHORT point;
  TMWTYPES_USHORT i;
  void *pPoint;

  /* Response to a write request is null response so we don't need to
   *  put anything into it here.
   */
  TMWTARG_UNUSED_PARAM(pResponse);

  /* zero length strings not allowed for write request */
  if(pObjHeader->variation == 0)
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_WRITE_OBJVAR);
    return(TMWDEFS_FALSE);
  }

  /* Object variation specifies string length */
  strLength = pObjHeader->variation;

  /* Loop through the points in the request */
  for(i = 0; i < pObjHeader->numberOfPoints; i++)
  {
    TMWTYPES_ULONG stringOffset;

    /* Get next point number from request */
    dnputil_getPointNumber(pRxFragment, pObjHeader, i, &point);

    /* Verify length of request */
    if((pRxFragment->offset + strLength) > pRxFragment->msgLength)
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
      pRxFragment->offset = pRxFragment->msgLength;
      return TMWDEFS_FALSE;
    }

    /* Diagnostics */
    DNPDIAG_SHOW_VTERM_OUTPUT(pSession, point, (pRxFragment->pMsgBuf + pRxFragment->offset), strLength);
 
    /* Save offset to start of string, and then increment to next point */
    stringOffset = pRxFragment->offset;
    pRxFragment->offset = pRxFragment->offset + strLength;

    /* Get point */
    pPoint = sdnpdata_vtermGetPoint(pSDNPSession->pDbHandle, point);
    if(pPoint == TMWDEFS_NULL)
    {
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_VTERM_NOTFOUND);
      status = TMWDEFS_FALSE;
      continue;
    }

    /* Write the string */
    if(!sdnpdata_vtermWrite(pPoint, pRxFragment->pMsgBuf + stringOffset, strLength))
    {
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_VTERM_WRITE);
      status = TMWDEFS_FALSE;
    }
  }

  return(status);
}

#if SDNPDATA_SUPPORT_OBJ113
#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpo112_assignClass */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo112_assignClass(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWDEFS_CLASS_MASK classMask)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_BOOL status = TMWDEFS_TRUE;
  TMWTYPES_USHORT index;

  status = sdnputil_validateAssignQualifier(pObjHeader, sdnpdata_vtermQuantity(pSDNPSession->pDbHandle));

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    TMWTYPES_USHORT pointNumber;
    void *pPoint;

    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    pPoint = sdnpdata_vtermGetPoint(pSDNPSession->pDbHandle, pointNumber);
    if(pPoint == TMWDEFS_NULL)
    {
      /* Failure of any point for startstop or indexed qualifiers should set IIN2.2 in response */
      if((pObjHeader->qualifier < DNPDEFS_QUAL_ALL_POINTS)
        ||(pObjHeader->qualifier > DNPDEFS_QUAL_16BIT_LIMITED_QTY))
      {
        status = TMWDEFS_FALSE;
      }
      continue;
    }

    if(!sdnpdata_vtermAssignClass(pPoint, classMask))
      status = TMWDEFS_FALSE;
  }

  return(status);
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ113 */
#endif /* SDNPDATA_SUPPORT_OBJ112 */
