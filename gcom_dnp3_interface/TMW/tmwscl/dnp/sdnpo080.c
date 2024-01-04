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

/* file: sdnpo080.c
 * description: DNP Slave functionality for Object 80 Internal Indication
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/dnp/sdnpo080.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnputil.h"
#include "tmwscl/dnp/dnpdiag.h"

#if SDNPDATA_SUPPORT_OBJ80_READ
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo080_readObj80v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  SDNPSESN_QUAL qualifier)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPSESN_READ_STATUS initStatus;
  int numPointsInObject;
  int iinNumber;
  TMWTYPES_BOOL iinValue;
  TMWTYPES_USHORT objectHeaderOffset;
  TMWTYPES_USHORT iinBits = 0; 

  TMWTARG_UNUSED_PARAM(pRequest);

  if((pObjHeader->qualifier != DNPDEFS_QUAL_8BIT_START_STOP)
    &&(pObjHeader->qualifier != DNPDEFS_QUAL_16BIT_START_STOP))
  {
    pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
    return (SDNPSESN_READ_FAILED);
  }

  /* If all we are doing is parsing the request we can stop now */
  if(qualifier == SDNPSESN_QUAL_PARSE_ONLY)
  {
    return(SDNPSESN_READ_COMPLETE); 
  }

  /* Determine how many IIN bits including private ones are supported */
  if(pObjHeader->lastPointNumber >= sdnpdata_IINQuantity(pSDNPSession->pDbHandle))
  { 
    pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
    return (SDNPSESN_READ_FAILED);
  }

  /* If the range includes the standard 0-15, get those bits also */
  if(pObjHeader->firstPointNumber < 16)
  {
    /* Get user specified IIN bits */
    sdnpdata_getIIN(pSession, &iinBits);

    /* Add in the SCL bits */
    iinBits |= pSDNPSession->iin;
  }

  /* Store new object header */
  objectHeaderOffset = pResponse->msgLength;
  if((initStatus = sdnputil_initReadStaticResponse(pResponse, pObjHeader, 
    80, 1, pObjHeader->firstPointNumber, 1)) != SDNPSESN_READ_COMPLETE)
  {
    /* Unable to write header, return status */
    return(initStatus);
  }

  numPointsInObject = 0;
  pResponse->pMsgBuf[pResponse->msgLength] = 0;

  for(iinNumber = pObjHeader->firstPointNumber; iinNumber <= pObjHeader->lastPointNumber; iinNumber++)
  {
    if(iinNumber < 16)
    {
      int bitPosition;
      if(iinNumber < 8)
        bitPosition = iinNumber+8;
      else
        bitPosition = iinNumber-8;

      if(iinBits & (0x01 << bitPosition))
      {
        iinValue = TMWDEFS_TRUE;
      }
      else
      {
        iinValue = TMWDEFS_FALSE;
      }
    }
    else
    {
      /* Get private IIN bit value from database */
      iinValue = sdnpdata_IINRead(pSDNPSession->pDbHandle, (TMWTYPES_USHORT)iinNumber);
    }

    DNPDIAG_SHOW_IIN_VALUE(pSession, (TMWTYPES_USHORT)iinNumber, iinValue);

    /* 1 bit each */
    if(iinValue)
    {
      pResponse->pMsgBuf[pResponse->msgLength] |= (0x01 << (numPointsInObject%8));  
    }

    numPointsInObject++;
    if(((numPointsInObject %8) == 0)
      &&(iinNumber < pObjHeader->lastPointNumber))
    {
      pResponse->msgLength++;
      pResponse->pMsgBuf[pResponse->msgLength] = 0;
    }
  }

  pResponse->msgLength++;

  /* Update object header */
  sdnputil_updateObjectHeader(pResponse, 
    objectHeaderOffset, pObjHeader->lastPointNumber, (TMWTYPES_USHORT)numPointsInObject);

  return(SDNPSESN_READ_COMPLETE);
}
#endif

TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo080_writeObj80v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTARG_UNUSED_PARAM(pResponse);
  TMWTARG_UNUSED_PARAM(pRxFragment);

  /* Length of request was verified in sdnpsesn _validateMessageSize */ 

  pRxFragment->offset += 1;
  /* Only Need time and restart IIN bits are written to at this time */
  if(pObjHeader->numberOfPoints == 1)
  { 
    /* clear IIN NEED TIME */
    if(pObjHeader->firstPointNumber == DNPDEFS_IIN_NEEDTIME_INDEX) 
    {
      pSDNPSession->iin &= ~DNPDEFS_IIN_NEED_TIME;
      return(TMWDEFS_TRUE);
    }

    /* clear IIN RESTART */
    else if(pObjHeader->firstPointNumber == DNPDEFS_IIN_RESTART_INDEX) 
    {
      pSDNPSession->iin &= ~DNPDEFS_IIN_RESTART;
      return(TMWDEFS_TRUE);
    }
  }
  return(TMWDEFS_FALSE);
}

