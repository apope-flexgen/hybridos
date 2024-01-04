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

/* file: dnputil.c
 * description: Miscellaneous DNP utilities
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/dnpcnfg.h"
#include "tmwscl/dnp/dnputil.h"
#include "tmwscl/dnp/dnpdtime.h"
#include "tmwscl/utils/tmwtarg.h"


 TMWSESN_TX_DATA * TMWDEFS_GLOBAL dnputil_getCurrentMessage(TMWSESN *pSession)
{
#if DNPCNFG_MULTI_SESSION_REQUESTS 
  return ((DNPSESN *)pSession)->pCurrentMessage;
#else
   return pSession->pChannel->pCurrentMessage;
#endif
}

 void TMWDEFS_GLOBAL dnputil_setCurrentMessage(TMWSESN *pSession, TMWSESN_TX_DATA *pMsg)
 {
#if DNPCNFG_MULTI_SESSION_REQUESTS 
  ((DNPSESN *)pSession)->pCurrentMessage = pMsg;
#else
  pSession->pChannel->pCurrentMessage = pMsg;
#endif
 }


/* function: dnputil_linkFrameSizeToTprt */
TMWTYPES_USHORT TMWDEFS_GLOBAL dnputil_linkFrameSizeToTprt(
  TMWTYPES_USHORT linkFrameSize)
{
  TMWTYPES_USHORT tprtFrameSize;

  /* Subtract off the link header */
  linkFrameSize = (TMWTYPES_USHORT)(linkFrameSize - 10);

  /* Figure out how many complete blocks and multiply by 16 */
  tprtFrameSize = (TMWTYPES_USHORT)((linkFrameSize / 18) * 16);

  /* Figure out if we have an incomplete block at the end */
  if((linkFrameSize % 18) != 0)
  {
    if((linkFrameSize % 18) < 2)
    {
      DNPDIAG_ERROR(TMWDEFS_NULL, TMWDEFS_NULL, DNPDIAG_FRAME_SIZE);
      return(tprtFrameSize);
    }

    tprtFrameSize = (TMWTYPES_USHORT)(tprtFrameSize + (linkFrameSize % 18) - 2);
  }

  return(tprtFrameSize);
}

/* function: dnputil_validateMsgLen */
/* Validate received message length */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnputil_validateMsgLen(
  TMWSESN *pSession, 
  DNPUTIL_RX_MSG *pRxFragment, 
  TMWTYPES_USHORT length)
{
  if((pRxFragment->offset+length) > pRxFragment->msgLength)
  {
    pRxFragment->offset = pRxFragment->msgLength;
    if(pSession != TMWDEFS_NULL)
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
    }
    else
    {
      DNPDIAG_ERROR(TMWDEFS_NULL, TMWDEFS_NULL, DNPDIAG_INVALID_SIZE);
    }

    return TMWDEFS_FALSE;
  }
  return TMWDEFS_TRUE;
}

/* function: dnputil_parseApplHeader */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnputil_parseApplHeader(
  TMWSESN_RX_DATA *pRxFragment,
  DNPUTIL_RX_MSG *pMsg,
  TMWTYPES_BOOL copyMsgBuf)
{
  /* Initialize message pointers */
  if(copyMsgBuf)
    memcpy(pMsg->pMsgBuf, pRxFragment->pMsgBuf, pRxFragment->msgLength);
  else
    pMsg->pMsgBuf = pRxFragment->pMsgBuf;

  pMsg->msgLength = pRxFragment->msgLength;
  pMsg->offset = 0;

  /* Copy message data */
  pMsg->firstByteTime = pRxFragment->firstByteTime;
  pMsg->lastByteTime = pRxFragment->lastByteTime;

  /* Copy broadcast info to received message structure */
  pMsg->isBroadcast = pRxFragment->isBroadcast;
  pMsg->rxAddress = pRxFragment->rxAddress;

  /* Read application control byte */
  pMsg->ac = pMsg->pMsgBuf[pMsg->offset++];

  /* Read function code */
  pMsg->fc = pMsg->pMsgBuf[pMsg->offset++];

  /* If this is a response or an unsolicited response assume
   * we are a master and parse the IIN bits 
   */
  if((pMsg->fc == DNPDEFS_FC_RESPONSE)
    || (pMsg->fc == DNPDEFS_FC_UNSOLICITED))
  {
    /* Protect against bad message */
    if(pMsg->offset+2 <= pRxFragment->msgLength)
    {
      tmwtarg_get16(pMsg->pMsgBuf + pMsg->offset, &pMsg->iin);
      pMsg->offset += 2;
    }
    else
    {
      /* Message was too short */
      pMsg->offset = pMsg->msgLength; 
      DNPDIAG_ERROR(TMWDEFS_NULL, TMWDEFS_NULL, DNPDIAG_PARSE_APPHEADER); 
      return(TMWDEFS_FALSE);
    }
  }

  return(TMWDEFS_TRUE);
}

/* function: dnputil_parseObjectHeader */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnputil_parseObjectHeader(
  DNPUTIL_RX_MSG *pMsg,
  TMWTYPES_USHORT objectIndex, 
  DNPUTIL_OBJECT_HEADER *pHdr)
{
  TMWTYPES_ULONG offset = pMsg->offset;

  TMWTARG_UNUSED_PARAM(objectIndex);

  if((offset + 3) > pMsg->msgLength)
  {
    DNPDIAG_ERROR(TMWDEFS_NULL, TMWDEFS_NULL, DNPDIAG_PARSE_HEADER);
    return(TMWDEFS_FALSE);
  }

  /* Initialize pointer to object header */
  pHdr->pObjectHeader = pMsg->pMsgBuf + offset;

  pHdr->group = pMsg->pMsgBuf[offset++];
  pHdr->variation = pMsg->pMsgBuf[offset++];
  pHdr->qualifier = pMsg->pMsgBuf[offset++];

  switch(pHdr->qualifier)
  {
  case DNPDEFS_QUAL_ALL_POINTS:
    pHdr->firstPointNumber = 0;
    pHdr->lastPointNumber = 0;
    pHdr->numberOfPoints = 0;
    break;

  case DNPDEFS_QUAL_8BIT_START_STOP:
    if(offset + 2 > pMsg->msgLength)
      return(TMWDEFS_FALSE);

    pHdr->firstPointNumber = pMsg->pMsgBuf[offset++];
    pHdr->lastPointNumber = pMsg->pMsgBuf[offset++];

    if(pHdr->firstPointNumber > pHdr->lastPointNumber)
    {
      DNPDIAG_ERROR(TMWDEFS_NULL, TMWDEFS_NULL, DNPDIAG_FIRST_LAST);
      return(TMWDEFS_FALSE);
    }

    pHdr->numberOfPoints = (TMWTYPES_USHORT)((pHdr->lastPointNumber - pHdr->firstPointNumber) + 1);
    break;

  case DNPDEFS_QUAL_16BIT_START_STOP:
    if(offset + 4 > pMsg->msgLength)
      return(TMWDEFS_FALSE);

    tmwtarg_get16(&pMsg->pMsgBuf[offset], &pHdr->firstPointNumber);
    offset += 2;

    tmwtarg_get16(&pMsg->pMsgBuf[offset], &pHdr->lastPointNumber);
    offset += 2;

    if(pHdr->firstPointNumber > pHdr->lastPointNumber)
    {
      DNPDIAG_ERROR(TMWDEFS_NULL, TMWDEFS_NULL, DNPDIAG_FIRST_LAST);
      return(TMWDEFS_FALSE);
    }

    pHdr->numberOfPoints = (TMWTYPES_USHORT)((pHdr->lastPointNumber - pHdr->firstPointNumber) + 1);

    /* Maximum number of points the library supports is 0xffff 
     * start and stop of 0-0xffff would be 0x10000 points.
     */
    if((pHdr->numberOfPoints == 0) && (pHdr->lastPointNumber == 0xffff))
    {
      pHdr->numberOfPoints = 0xffff;
    }
    break;

  case DNPDEFS_QUAL_8BIT_LIMITED_QTY:
    if(offset + 1 > pMsg->msgLength)
      return(TMWDEFS_FALSE);

    pHdr->numberOfPoints = pMsg->pMsgBuf[offset++];

    if(pHdr->numberOfPoints == 0)
    {
      DNPDIAG_ERROR(TMWDEFS_NULL, TMWDEFS_NULL, DNPDIAG_QUANTITY_0);
      return(TMWDEFS_FALSE);
    }

    pHdr->firstPointNumber = 0;
    pHdr->lastPointNumber = 0;
    break;

  case DNPDEFS_QUAL_16BIT_LIMITED_QTY:
    if(offset + 2 > pMsg->msgLength)
      return(TMWDEFS_FALSE);

    tmwtarg_get16(&pMsg->pMsgBuf[offset], &pHdr->numberOfPoints);
    offset += 2;

    if(pHdr->numberOfPoints == 0)
    {
      DNPDIAG_ERROR(TMWDEFS_NULL, TMWDEFS_NULL, DNPDIAG_QUANTITY_0);
      return(TMWDEFS_FALSE);
    }

    pHdr->firstPointNumber = 0;
    pHdr->lastPointNumber = 0;
    break;

  case DNPDEFS_QUAL_8BIT_INDEX:
  case DNPDEFS_QUAL_16BIT_INDEX_8BITQ: 
    if(offset + 1 > pMsg->msgLength)
      return(TMWDEFS_FALSE);

    pHdr->numberOfPoints = pMsg->pMsgBuf[offset++];

    if(pHdr->numberOfPoints == 0)
    {
      DNPDIAG_ERROR(TMWDEFS_NULL, TMWDEFS_NULL, DNPDIAG_QUANTITY_0);
      return(TMWDEFS_FALSE);
    }

    pHdr->firstPointNumber = 0;
    pHdr->lastPointNumber = 0;
    break;

  case DNPDEFS_QUAL_16BIT_INDEX: 
    if(offset + 2 > pMsg->msgLength)
      return(TMWDEFS_FALSE);

    tmwtarg_get16(&pMsg->pMsgBuf[offset], &pHdr->numberOfPoints);
    offset += 2;

    if(pHdr->numberOfPoints == 0)
    {
      DNPDIAG_ERROR(TMWDEFS_NULL, TMWDEFS_NULL, DNPDIAG_QUANTITY_0);
      return(TMWDEFS_FALSE);
    }

    pHdr->firstPointNumber = 0;
    pHdr->lastPointNumber = 0;
    break;

  case DNPDEFS_QUAL_8BIT_FREE_FORMAT:
  case DNPDEFS_QUAL_16BIT_FREE_FORMAT:
    if(offset + 1 > pMsg->msgLength)
      return(TMWDEFS_FALSE);

    pHdr->firstPointNumber = 0;
    pHdr->lastPointNumber = 0;
    pHdr->numberOfPoints = pMsg->pMsgBuf[offset++];
    break;

  default:
    pMsg->offset = offset;
    DNPDIAG_ERROR(TMWDEFS_NULL, TMWDEFS_NULL, DNPDIAG_QUALIFIER);
    return(TMWDEFS_FALSE);
  }

  /* Initialize pointer to object data */
  pHdr->pObjectData = pMsg->pMsgBuf + offset;
  pMsg->offset = offset;

  return(TMWDEFS_TRUE);
}

/* function: dnputil_getPointNumber */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnputil_getPointNumber(
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pHdr, 
  TMWTYPES_USHORT index, 
  TMWTYPES_USHORT *pPointNumber)
{
  if(pHdr->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
  {
    if(pRxFragment->offset + 1 > pRxFragment->msgLength)
    {
      pRxFragment->offset = pRxFragment->msgLength;
      return(TMWDEFS_FALSE);
    }
    *pPointNumber = pRxFragment->pMsgBuf[pRxFragment->offset++];
  }
  else if((pHdr->qualifier == DNPDEFS_QUAL_16BIT_INDEX)
    ||(pHdr->qualifier == DNPDEFS_QUAL_16BIT_INDEX_8BITQ))
  {
    if(pRxFragment->offset + 2 > pRxFragment->msgLength)
    {
      pRxFragment->offset = pRxFragment->msgLength;
      return(TMWDEFS_FALSE);
    }

    tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], pPointNumber);
    pRxFragment->offset += 2;
  }
  else
  {
    *pPointNumber = (TMWTYPES_USHORT)(pHdr->firstPointNumber + index);
  }

  return(TMWDEFS_TRUE);
}

/* function: dnputil_writePointNumber */
void TMWDEFS_GLOBAL dnputil_writePointNumber(
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT objectHeaderOffset,
  TMWTYPES_USHORT pointNumber)
{
  TMWTYPES_UCHAR qualifier = pResponse->pMsgBuf[objectHeaderOffset + 2];
  if(qualifier == DNPDEFS_QUAL_8BIT_INDEX)
  {
    pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)pointNumber;
  }
  else if(qualifier == DNPDEFS_QUAL_16BIT_INDEX)
  {
    tmwtarg_store16(&pointNumber, pResponse->pMsgBuf + pResponse->msgLength);
    pResponse->msgLength += 2;
  }
}

/* function: dnputil_getAnalogValueShort */
TMWTYPES_SHORT dnputil_getAnalogValueShort(
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR *pFlags)
{
  switch(pValue->type)
  {
  case TMWTYPES_ANALOG_TYPE_SHORT: 
    {
      return(pValue->value.sval);
    }

  case TMWTYPES_ANALOG_TYPE_LONG: 
    if(pValue->value.lval < TMWDEFS_SHORT_MIN) 
    {
      *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
      return(TMWDEFS_SHORT_MIN);
    }
    else if(pValue->value.lval > TMWDEFS_SHORT_MAX)
    {
      *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
      return(TMWDEFS_SHORT_MAX);
    }
    else
    {
      return((TMWTYPES_SHORT)pValue->value.lval);
    }

#if TMWCNFG_SUPPORT_FLOAT
  case TMWTYPES_ANALOG_TYPE_SFLOAT: 
    if(pValue->value.fval < TMWDEFS_SHORT_MIN) 
    {
      *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
      return(TMWDEFS_SHORT_MIN);
    }
    else if(pValue->value.fval > TMWDEFS_SHORT_MAX)
    {
      *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
      return(TMWDEFS_SHORT_MAX);
    }
    else
    {
      return((TMWTYPES_SHORT)pValue->value.fval);
    }
       
  case TMWTYPES_ANALOG_TYPE_SCALED: 
   if(pValue->value.scaled.lval < TMWDEFS_SHORT_MIN) 
    {
      *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
      return(TMWDEFS_SHORT_MIN);
    }
    else if(pValue->value.scaled.lval > TMWDEFS_SHORT_MAX)
    {
      *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
      return(TMWDEFS_SHORT_MAX);
    }
    else
    {
      return((TMWTYPES_SHORT)pValue->value.scaled.lval);
    }
#endif

#if TMWCNFG_SUPPORT_DOUBLE
  case TMWTYPES_ANALOG_TYPE_DOUBLE: 
    if(pValue->value.dval < TMWDEFS_SHORT_MIN) 
    {
      *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
      return(TMWDEFS_SHORT_MIN);
    }
    else if(pValue->value.dval > TMWDEFS_SHORT_MAX)
    {
      *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
      return(TMWDEFS_SHORT_MAX);
    }
    else
    {
      return((TMWTYPES_SHORT)pValue->value.dval);
    }

  case TMWTYPES_ANALOG_TYPE_DSCALED: 
   if(pValue->value.dscaled.lval < TMWDEFS_SHORT_MIN) 
    {
      *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
      return(TMWDEFS_SHORT_MIN);
    }
    else if(pValue->value.dscaled.lval > TMWDEFS_SHORT_MAX)
    {
      *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
      return(TMWDEFS_SHORT_MAX);
    }
    else
    {
      return((TMWTYPES_SHORT)pValue->value.dscaled.lval);
    }
#endif

  default:
    ASSERT(0);  /* other types are not implemented */
    break;
  }

  return(0);
}

/* function: dnputil_getAnalogValueLong */
TMWTYPES_LONG dnputil_getAnalogValueLong(
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR *pFlags)
{
  switch(pValue->type)
  {
  case TMWTYPES_ANALOG_TYPE_SHORT: 
    return(pValue->value.sval);

  case TMWTYPES_ANALOG_TYPE_LONG: 
    return(pValue->value.lval);

#if TMWCNFG_SUPPORT_FLOAT
  case TMWTYPES_ANALOG_TYPE_SFLOAT: 
    /* If a value was too big to fit in a LONG, it would be
     * put in SFLOAT, however, 2147483648 put into SFLOAT 
     * becomes 2.1474836E+9 which is less than 2147483648
     * Use >=LONG_MAX and <=LONG_MIN converted to SFLOAT for 
     * comparisons
     */
    if(pValue->value.fval <= (TMWTYPES_SFLOAT)TMWDEFS_LONG_MIN) 
    {
      *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
      return(TMWDEFS_LONG_MIN);
    }
    else if(pValue->value.fval >= (TMWTYPES_SFLOAT)TMWDEFS_LONG_MAX)
    {
      *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
      return(TMWDEFS_LONG_MAX);
    }
    else
    {
      return((TMWTYPES_LONG)pValue->value.fval);
    }
    
  case TMWTYPES_ANALOG_TYPE_SCALED: 
    return((TMWTYPES_LONG)pValue->value.scaled.lval);

#endif

#if TMWCNFG_SUPPORT_DOUBLE
  case TMWTYPES_ANALOG_TYPE_DOUBLE: 
    if(pValue->value.dval < TMWDEFS_LONG_MIN) 
    {
      *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
      return(TMWDEFS_LONG_MIN);
    }
    else if(pValue->value.dval > TMWDEFS_LONG_MAX)
    {
      *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
      return(TMWDEFS_LONG_MAX);
    }
    else
    {
      return((TMWTYPES_LONG)pValue->value.dval);
    }
    
  case TMWTYPES_ANALOG_TYPE_DSCALED: 
    return((TMWTYPES_LONG)pValue->value.dscaled.lval);

#endif

  default:
    ASSERT(0);  /* other types are not implemented */
    break;
  }

  return(0);
}

#if TMWCNFG_SUPPORT_FLOAT
/* function: dnputil_getAnalogValueFloat */
TMWTYPES_SFLOAT dnputil_getAnalogValueFloat(
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR *pFlags)
{
  switch(pValue->type)
  {
  case TMWTYPES_ANALOG_TYPE_SHORT: 
    return((TMWTYPES_SFLOAT)pValue->value.sval);
 
  case TMWTYPES_ANALOG_TYPE_LONG: 
    return((TMWTYPES_SFLOAT)pValue->value.lval);

  case TMWTYPES_ANALOG_TYPE_SFLOAT: 
    return(pValue->value.fval);
    
  case TMWTYPES_ANALOG_TYPE_SCALED: 
    return(pValue->value.scaled.fval);

#if TMWCNFG_SUPPORT_DOUBLE
  case TMWTYPES_ANALOG_TYPE_DOUBLE: 
    if(pValue->value.dval < TMWDEFS_SFLOAT_MIN) 
    {
      *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
      return(TMWDEFS_SFLOAT_MIN);
    }
    else if(pValue->value.dval > TMWDEFS_SFLOAT_MAX) 
    {
      *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
      return(TMWDEFS_SFLOAT_MAX);
    }
    else
    {
      return((TMWTYPES_SFLOAT)pValue->value.dval);
    }

  case TMWTYPES_ANALOG_TYPE_DSCALED:  
    if(pValue->value.dscaled.dval < TMWDEFS_SFLOAT_MIN) 
    {
      *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
      return(TMWDEFS_SFLOAT_MIN);
    }
    else if(pValue->value.dscaled.dval > TMWDEFS_SFLOAT_MAX) 
    {
      *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
      return(TMWDEFS_SFLOAT_MAX);
    }
    else
    {
      return((TMWTYPES_SFLOAT)pValue->value.dscaled.dval);
    }
#endif
  
  default:
    ASSERT(0);  /* other types are not implemented */
    break;
  }

  return(0);
}
#endif

#if TMWCNFG_SUPPORT_DOUBLE
/* function: dnputil_getAnalogValueDouble */
TMWTYPES_DOUBLE dnputil_getAnalogValueDouble(
  TMWTYPES_ANALOG_VALUE *pValue)
{
  switch(pValue->type)
  {
  case TMWTYPES_ANALOG_TYPE_SHORT: 
    return(pValue->value.sval);

  case TMWTYPES_ANALOG_TYPE_LONG: 
    return(pValue->value.lval);

#if TMWCNFG_SUPPORT_FLOAT
  case TMWTYPES_ANALOG_TYPE_SFLOAT: 
    return(pValue->value.fval);

  case TMWTYPES_ANALOG_TYPE_SCALED: 
    return(pValue->value.scaled.fval);
#endif

#if TMWCNFG_SUPPORT_DOUBLE
  case TMWTYPES_ANALOG_TYPE_DOUBLE: 
    return(pValue->value.dval);
    
  case TMWTYPES_ANALOG_TYPE_DSCALED: 
    return(pValue->value.dscaled.dval);
#endif

  default:
    ASSERT(0);  /* other types are not implemented */
    break;
  }

  return(0);
}
#endif

 /* function: dnputil_getAnlgDBandValueUShort */
TMWTYPES_USHORT dnputil_getAnlgDBandValueUShort(
  TMWTYPES_ANALOG_VALUE *pValue)
{
  switch(pValue->type)
  {
  case TMWTYPES_ANALOG_TYPE_SHORT: 
    return((TMWTYPES_USHORT)pValue->value.sval);

  case TMWTYPES_ANALOG_TYPE_USHORT: 
    return(pValue->value.usval);

  case TMWTYPES_ANALOG_TYPE_LONG: 
    if(pValue->value.lval > TMWDEFS_USHORT_MAX)
      return(TMWDEFS_USHORT_MAX);
    else
      return((TMWTYPES_USHORT)pValue->value.lval);

  case TMWTYPES_ANALOG_TYPE_ULONG: 
    if(pValue->value.ulval > TMWDEFS_USHORT_MAX)
      return(TMWDEFS_USHORT_MAX);
    else
      return((TMWTYPES_USHORT)pValue->value.ulval);

#if TMWCNFG_SUPPORT_FLOAT
  case TMWTYPES_ANALOG_TYPE_SFLOAT:
    if(pValue->value.fval > TMWDEFS_USHORT_MAX)
      return(TMWDEFS_USHORT_MAX);
    else
      return((TMWTYPES_USHORT)pValue->value.fval);

  case TMWTYPES_ANALOG_TYPE_SCALED:
    if(pValue->value.scaled.lval > TMWDEFS_USHORT_MAX)
      return(TMWDEFS_USHORT_MAX);
    else
      return((TMWTYPES_USHORT)pValue->value.scaled.lval);
#endif

#if TMWCNFG_SUPPORT_DOUBLE
  case TMWTYPES_ANALOG_TYPE_DOUBLE: 
    if(pValue->value.dval > TMWDEFS_USHORT_MAX)
      return(TMWDEFS_USHORT_MAX);
    else
      return((TMWTYPES_USHORT)pValue->value.dval);

  case TMWTYPES_ANALOG_TYPE_DSCALED:
    if(pValue->value.dscaled.lval > TMWDEFS_USHORT_MAX)
      return(TMWDEFS_USHORT_MAX);
    else
      return((TMWTYPES_USHORT)pValue->value.dscaled.lval);
#endif

  default:
    ASSERT(0);  /* other types are not implemented */
    break;
  }

  return(0);
}

/* function: dnputil_getAnlgDBandValueULong */
TMWTYPES_ULONG dnputil_getAnlgDBandValueULong(
  TMWTYPES_ANALOG_VALUE *pValue)
{
  switch(pValue->type)
  {
  case TMWTYPES_ANALOG_TYPE_SHORT: 
    return((TMWTYPES_ULONG)pValue->value.sval);

  case TMWTYPES_ANALOG_TYPE_USHORT: 
    return((TMWTYPES_ULONG)pValue->value.usval);

  case TMWTYPES_ANALOG_TYPE_LONG: 
    return((TMWTYPES_ULONG)pValue->value.lval);

  case TMWTYPES_ANALOG_TYPE_ULONG: 
    return(pValue->value.ulval);

#if TMWCNFG_SUPPORT_FLOAT
  case TMWTYPES_ANALOG_TYPE_SFLOAT:
    if(pValue->value.fval > (TMWTYPES_SFLOAT)TMWDEFS_ULONG_MAX)
      return(TMWDEFS_ULONG_MAX);
    else
      return((TMWTYPES_ULONG)pValue->value.fval);

  case TMWTYPES_ANALOG_TYPE_SCALED:
    return((TMWTYPES_ULONG)pValue->value.scaled.lval);
#endif

#if TMWCNFG_SUPPORT_DOUBLE
  case TMWTYPES_ANALOG_TYPE_DOUBLE: 
    if(pValue->value.dval > TMWDEFS_ULONG_MAX)
      return(TMWDEFS_ULONG_MAX);
    else
      return((TMWTYPES_ULONG)pValue->value.dval);

  case TMWTYPES_ANALOG_TYPE_DSCALED:
    return((TMWTYPES_ULONG)pValue->value.dscaled.lval);
#endif

  default:
    ASSERT(0);  /* other types are not implemented */
    break;
  }

  return(0);
}

#if TMWCNFG_SUPPORT_FLOAT
/* function: dnputil_getAnlgDBandValueFloat */
TMWTYPES_SFLOAT dnputil_getAnlgDBandValueFloat(
  TMWTYPES_ANALOG_VALUE *pValue)
{
  switch(pValue->type)
  {
  case TMWTYPES_ANALOG_TYPE_SHORT: 
    return((TMWTYPES_SFLOAT)pValue->value.sval);
  
  case TMWTYPES_ANALOG_TYPE_USHORT: 
    return((TMWTYPES_SFLOAT)pValue->value.usval);

  case TMWTYPES_ANALOG_TYPE_LONG: 
    return((TMWTYPES_SFLOAT)pValue->value.lval);
  
  case TMWTYPES_ANALOG_TYPE_ULONG: 
    return((TMWTYPES_SFLOAT)pValue->value.ulval);

  case TMWTYPES_ANALOG_TYPE_SFLOAT: 
    return(pValue->value.fval);

  case TMWTYPES_ANALOG_TYPE_SCALED:
    return(pValue->value.scaled.fval);

#if TMWCNFG_SUPPORT_DOUBLE
  case TMWTYPES_ANALOG_TYPE_DOUBLE: 
    if(pValue->value.dval > TMWDEFS_SFLOAT_MAX)
      return(TMWDEFS_SFLOAT_MAX);
    else
      return((TMWTYPES_SFLOAT)pValue->value.dval);

  case TMWTYPES_ANALOG_TYPE_DSCALED: 
    if(pValue->value.dscaled.dval > TMWDEFS_SFLOAT_MAX)
      return(TMWDEFS_SFLOAT_MAX);
    else
      return((TMWTYPES_SFLOAT)pValue->value.dscaled.dval);
#endif
  
  default:
    ASSERT(0);  /* other types are not implemented */
    break;
  }

  return(0);
}
#endif

#if TMWCNFG_SUPPORT_DOUBLE
/* function: dnputil_getAnlgDBandValueDouble */
/* This function is not called by the SCL since no deadband variation uses 
 * double precision. It is used by the database functions to convert
 * values to DOUBLE for storage to avoid the problem with the max value
 * that can be stored in a LONG, versus the precision of an SFLOAT. 
 */
TMWTYPES_DOUBLE dnputil_getAnlgDBandValueDouble(
  TMWTYPES_ANALOG_VALUE *pValue)
{
  switch(pValue->type)
  {
  case TMWTYPES_ANALOG_TYPE_SHORT: 
    return((TMWTYPES_DOUBLE)pValue->value.sval);
 
  case TMWTYPES_ANALOG_TYPE_USHORT: 
    return((TMWTYPES_DOUBLE)pValue->value.usval);

  case TMWTYPES_ANALOG_TYPE_LONG: 
    return((TMWTYPES_DOUBLE)pValue->value.lval);

  case TMWTYPES_ANALOG_TYPE_ULONG: 
    return((TMWTYPES_DOUBLE)pValue->value.ulval);

#if TMWCNFG_SUPPORT_FLOAT
  case TMWTYPES_ANALOG_TYPE_SFLOAT: 
    return((TMWTYPES_DOUBLE)pValue->value.fval);

  case TMWTYPES_ANALOG_TYPE_SCALED:
    return((TMWTYPES_DOUBLE)pValue->value.scaled.fval);
#endif

  case TMWTYPES_ANALOG_TYPE_DOUBLE: 
    return(pValue->value.dval);

  case TMWTYPES_ANALOG_TYPE_DSCALED:
    return((TMWTYPES_DOUBLE)pValue->value.dscaled.dval);

  default:
    ASSERT(0);  /* other types are not implemented */
    break;
  }

  return(0);
}
#endif

#if DNPCNFG_SUPPORT_DATASETS_ATTRS
/* function: dnputil_lengthRequired */
TMWTYPES_UCHAR TMWDEFS_GLOBAL dnputil_lengthRequired(
  TMWTYPES_ULONG value)
{
  if(value <= TMWDEFS_UCHAR_MAX)
  {
    return(1);
  }
  else if(value <= TMWDEFS_USHORT_MAX)
  {
    return(2);
  }
  return(4);
}

/* function: dnputil_putIntInMessage */
void TMWDEFS_GLOBAL dnputil_putIntInMessage(
  TMWTYPES_UCHAR *ptr, 
  TMWTYPES_ULONG value, 
  TMWTYPES_UCHAR length)
{
  if(length == 1)
  {
    *ptr = (TMWTYPES_UCHAR)value;
  }
  else if(length == 2)
  {
    TMWTYPES_USHORT tmp = (TMWTYPES_USHORT)value;
    tmwtarg_store16(&tmp, ptr);
  }
  else
  {
    tmwtarg_store32(&value, ptr);
  }
}

/* function: dnputil_getIntFromMessage */
TMWTYPES_LONG TMWDEFS_GLOBAL dnputil_getIntFromMessage(
  TMWTYPES_UCHAR *ptr,
  TMWTYPES_UCHAR length)
{
  if(length == 1)
  {
    TMWTYPES_CHAR tmp = *ptr;
    return((TMWTYPES_LONG)tmp);  
  }
  else if(length == 2)
  {
    TMWTYPES_USHORT tmp;
    TMWTYPES_SHORT tmp2;
    tmwtarg_get16(ptr, &tmp);
    tmp2 = (TMWTYPES_SHORT)tmp;
    return(tmp2);
  }
  else if(length == 4)
  {
    TMWTYPES_ULONG tmp;
    tmwtarg_get32(ptr, &tmp);
    return(tmp);
  }
  return 0;
}

/* function: dnputil_getUintFromMessage */
TMWTYPES_ULONG TMWDEFS_GLOBAL dnputil_getUintFromMessage(
  TMWTYPES_UCHAR *ptr,
  TMWTYPES_UCHAR length)
{
  if(length == 1)
  {
    return(*ptr);  
  }
  else if(length == 2)
  {
    TMWTYPES_USHORT tmp;
    tmwtarg_get16(ptr, &tmp);
    return(tmp);
  }
  else if(length == 4)
  {
    TMWTYPES_ULONG tmp;
    tmwtarg_get32(ptr, &tmp);
    return(tmp);
  }
  else
    return 0;
}
/* function: dnputil_getAncValueFromMessage */
void TMWDEFS_GLOBAL dnputil_getAncValueFromMessage(
  DNPUTIL_RX_MSG            *pRxFragment,
  DNPDEFS_DATASET_DESCR_CODE code,
  TMWTYPES_UCHAR             length,
  DNPDATA_DATASET_VALUE     *pValue)
{
  switch(code)
  {
  case DNPDEFS_DATASET_DESCR_ID: 
    pValue->type = DNPDATA_VALUE_UINT32;
    pValue->value.int32Value = dnputil_getUintFromMessage(&pRxFragment->pMsgBuf[pRxFragment->offset], length);
    break;
  case DNPDEFS_DATASET_DESCR_UUID: 
  case DNPDEFS_DATASET_DESCR_PTYP: 
  case DNPDEFS_DATASET_DESCR_NSPC: 
  case DNPDEFS_DATASET_DESCR_NAME: 
  case DNPDEFS_DATASET_DESCR_DAEL: 
  case DNPDEFS_DATASET_DESCR_CTLV: 
  case DNPDEFS_DATASET_DESCR_CTLS: 
    pValue->type = DNPDATA_VALUE_STRPTR;
    pValue->value.pStrValue = &pRxFragment->pMsgBuf[pRxFragment->offset];
    break; 
  default:
    DNPDIAG_ERROR(TMWDEFS_NULL, TMWDEFS_NULL, DNPDIAG_DATASET_VALUE);
    break;
  }

  pValue->length = length;
  pRxFragment->offset = pRxFragment->offset + length;
} 
    
/* function: dnputil_putValueInMessage */
void TMWDEFS_GLOBAL dnputil_putValueInMessage(
  TMWTYPES_UCHAR          *pMsgData,
  TMWTYPES_USHORT         *pMsgLength,
  DNPDATA_DATASET_VALUE   *pValue)
{
  TMWTYPES_UCHAR length = pValue->length;
  TMWTYPES_USHORT msgLength = *pMsgLength;

  switch(pValue->type)
  {
  case DNPDATA_VALUE_STRPTR:
    memcpy(&pMsgData[msgLength], pValue->value.pStrValue, length);
    break;
  case DNPDATA_VALUE_STRARRAY:
    memcpy(&pMsgData[msgLength], pValue->value.strValue, length);
    break;
  case DNPDATA_VALUE_UINT32:
    dnputil_putIntInMessage(&pMsgData[msgLength], pValue->value.uint32Value, length);
    break;
  case DNPDATA_VALUE_INT32:
    dnputil_putIntInMessage(&pMsgData[msgLength], (TMWTYPES_ULONG)pValue->value.int32Value, length);
    break;
  case DNPDATA_VALUE_SFLT:
    tmwtarg_storeSFloat(&pValue->value.sfltValue, &pMsgData[msgLength]);
    break;
  case DNPDATA_VALUE_DOUBLE:
    {
      /* spec says only 4 and 8 octet floats are allowed */
      if(length == 4)
      {
        TMWTYPES_SFLOAT temp = (TMWTYPES_SFLOAT)pValue->value.doubleValue;
        tmwtarg_storeSFloat(&temp, &pMsgData[msgLength]);
      }
      else
      {
        tmwtarg_store64(&pValue->value.doubleValue, &pMsgData[msgLength]);
      }
    }
    break; 
  case DNPDATA_VALUE_TIME:
    {
      TMWTYPES_MS_SINCE_70 msSince70; 
      /* Convert to milliseconds since 1970 */
      dnpdtime_dateTimeToMSSince70(&msSince70, &pValue->value.timeValue);

      /* Write into response */
      dnpdtime_writeMsSince70(pMsgData + msgLength, &msSince70);
      break;
    }
  default:
    DNPDIAG_ERROR(TMWDEFS_NULL, TMWDEFS_NULL, DNPDIAG_DATASET_TYPE);
    break;
  }

  *pMsgLength = (TMWTYPES_USHORT)(msgLength + length);
} 

/* function: dnputil_getValueFromMessage */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnputil_getValueFromMessage(
  TMWTYPES_UCHAR            *pMsgBuf,
  TMWTYPES_ULONG            *pOffset,
  TMWTYPES_ULONG             maxOffset,
  DNPDEFS_DATASET_TYPE_CODE  type,
  DNPDATA_DATASET_VALUE     *pValue)
{
  TMWTYPES_ULONG offset = *pOffset;
  TMWTYPES_UCHAR length = pMsgBuf[offset++]; 

  if((offset + length) <= maxOffset)
  {
    TMWTYPES_BOOL status = TMWDEFS_TRUE;
    switch(type)
    {
    case DNPDEFS_DATASET_TYPE_OSTR:
    case DNPDEFS_DATASET_TYPE_BSTR:
    case DNPDEFS_DATASET_TYPE_VSTR:
      pValue->type = DNPDATA_VALUE_STRPTR;
      pValue->value.pStrValue = &pMsgBuf[offset];
      break;

    case DNPDEFS_DATASET_TYPE_UINT:
      pValue->type = DNPDATA_VALUE_UINT32;
      pValue->value.uint32Value = dnputil_getUintFromMessage(&pMsgBuf[offset], length);
      break;

    case DNPDEFS_DATASET_TYPE_INT:
      pValue->type = DNPDATA_VALUE_INT32;
      pValue->value.int32Value = dnputil_getIntFromMessage(&pMsgBuf[offset], length);
      break;
    case DNPDEFS_DATASET_TYPE_FLT:
      if(length == 8)
      {
        TMWTYPES_DOUBLE tmp;
        tmwtarg_get64(&pMsgBuf[offset], &tmp);
        pValue->type = DNPDATA_VALUE_DOUBLE;
        pValue->value.doubleValue = tmp;
      }
      else 
      {
        if(length == 4)
        {
          TMWTYPES_SFLOAT tmp;
          tmwtarg_get32(&pMsgBuf[offset], (TMWTYPES_ULONG *)&tmp);
          pValue->value.sfltValue = tmp;
        }
        else
        {
          pValue->value.sfltValue = 0;
          status = TMWDEFS_FALSE; 
        }
        pValue->type = DNPDATA_VALUE_SFLT;
      }
      break;
    case DNPDEFS_DATASET_TYPE_TIME:
    {
      if(length == 6)
      {
        TMWTYPES_MS_SINCE_70 msSince70;
        dnpdtime_readMsSince70(&msSince70, &pMsgBuf[offset]);
        dnpdtime_msSince70ToDateTime(&pValue->value.timeValue, &msSince70);
        pValue->type = DNPDATA_VALUE_TIME;
      }
      else
      {
        status = TMWDEFS_FALSE; 
      }
      break;
    }
    case DNPDEFS_DATASET_TYPE_NONE:
    {
      /* We could not determine what type to convert this to,
       * Just point to the raw bytes in the message
       */
      pValue->type = DNPDATA_VALUE_STRPTR;
      pValue->value.pStrValue = &pMsgBuf[offset];
      break;
    }
    /* case DNPDEFS_DATASET_TYPE_UNCD:*/
    default:
      status = TMWDEFS_FALSE; 
      break;
    }
    if(status)
    {
      pValue->length = length;
      *pOffset = offset + length;
      return(TMWDEFS_TRUE);
    }
  }
  else
  {
    DNPDIAG_ERROR(TMWDEFS_NULL, TMWDEFS_NULL, DNPDIAG_INVALID_SIZE);
  }

  pValue->length = 0;
  DNPDIAG_ERROR(TMWDEFS_NULL, TMWDEFS_NULL, DNPDIAG_DATASET_VALUE);
  return(TMWDEFS_FALSE);
}

/* function: dnputil_putAttrValueInMessage */
void TMWDEFS_GLOBAL dnputil_putAttrValueInMessage(
  TMWSESN_TX_DATA         *pTxData,
  DNPDATA_ATTRIBUTE_VALUE *pValue)
{
  TMWTYPES_UCHAR length = pValue->length;

  pTxData->pMsgBuf[pTxData->msgLength++] = (TMWTYPES_UCHAR)pValue->type; 
  pTxData->pMsgBuf[pTxData->msgLength++] = pValue->length;

  switch(pValue->type)
  {
  case DNPDEFS_ATTRIBUTE_TYPE_VSTR:
  case DNPDEFS_ATTRIBUTE_TYPE_OSTR:
  case DNPDEFS_ATTRIBUTE_TYPE_BSTR: 
    memcpy(&pTxData->pMsgBuf[pTxData->msgLength], pValue->value.pStrValue, length);
    break;
  case DNPDEFS_ATTRIBUTE_TYPE_UINT:
    dnputil_putIntInMessage(&pTxData->pMsgBuf[pTxData->msgLength], pValue->value.uintValue, length);
    break;
  case DNPDEFS_ATTRIBUTE_TYPE_INT:
    dnputil_putIntInMessage(&pTxData->pMsgBuf[pTxData->msgLength], (TMWTYPES_ULONG)pValue->value.intValue, length);
    break;
  case DNPDEFS_ATTRIBUTE_TYPE_FLT:
    if(pValue->length == 4)
      tmwtarg_storeSFloat(&pValue->value.fltValue.sfltValue, &pTxData->pMsgBuf[pTxData->msgLength]);
    else
      tmwtarg_store64(&pValue->value.fltValue.doubleValue, &pTxData->pMsgBuf[pTxData->msgLength]);
    break; 
  case DNPDEFS_ATTRIBUTE_TYPE_DNP3TIME:
    {
      TMWTYPES_MS_SINCE_70 msSince70; 
      /* Convert to milliseconds since 1970 */
      dnpdtime_dateTimeToMSSince70(&msSince70, &pValue->value.timeValue);

      /* Write into message */
      dnpdtime_writeMsSince70(&pTxData->pMsgBuf[pTxData->msgLength], &msSince70);
    }
    break;
  
  default:
    DNPDIAG_ERROR(TMWDEFS_NULL, TMWDEFS_NULL, DNPDIAG_DATASET_TYPE);
    break;
  }

  pTxData->msgLength = (TMWTYPES_USHORT)(pTxData->msgLength + length);
} 

/* function: dnputil_getAttrValueFromMessage */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnputil_getAttrValueFromMessage(
  DNPUTIL_RX_MSG            *pRxFragment,
  DNPDATA_ATTRIBUTE_VALUE   *pValue)
{
  TMWTYPES_BOOL invalidSize;
  TMWTYPES_UCHAR type; 
  TMWTYPES_UCHAR length; 
  invalidSize = TMWDEFS_FALSE;
 
  if((pRxFragment->offset+2) <= pRxFragment->msgLength) 
  { 
    type = pRxFragment->pMsgBuf[pRxFragment->offset++]; 
    length = pRxFragment->pMsgBuf[pRxFragment->offset++]; 

    if((pRxFragment->offset+length) <= pRxFragment->msgLength) 
    { 
      TMWTYPES_BOOL status = TMWDEFS_TRUE;
      pValue->length = length;
      pValue->type = (DNPDEFS_ATTRIBUTE_DATA_TYPE)type;

      switch(type)
      {
      case DNPDEFS_ATTRIBUTE_TYPE_VSTR:
      case DNPDEFS_ATTRIBUTE_TYPE_OSTR:
      case DNPDEFS_ATTRIBUTE_TYPE_BSTR:
        pValue->value.pStrValue = &pRxFragment->pMsgBuf[pRxFragment->offset];
        break;

      case DNPDEFS_ATTRIBUTE_TYPE_UINT:
        pValue->value.uintValue = dnputil_getUintFromMessage(&pRxFragment->pMsgBuf[pRxFragment->offset], length);
        break;

      case DNPDEFS_ATTRIBUTE_TYPE_INT:
        pValue->value.intValue = dnputil_getIntFromMessage(&pRxFragment->pMsgBuf[pRxFragment->offset], length);
        break;
      case DNPDEFS_ATTRIBUTE_TYPE_FLT:
        if(length == 8)
        {
          TMWTYPES_DOUBLE tmp;
          tmwtarg_get64(&pRxFragment->pMsgBuf[pRxFragment->offset], &tmp);
          pValue->value.fltValue.doubleValue = tmp;
        }
        else
        {
          if(length == 4)
          {
            TMWTYPES_SFLOAT tmp;
            tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], (TMWTYPES_ULONG *)&tmp);
            pValue->value.fltValue.sfltValue = tmp;
          }
          else
          {
            /* Invalid length */
            pValue->value.fltValue.sfltValue = 0;
            status = TMWDEFS_FALSE;
          }
        }
        break;
        
      case DNPDEFS_ATTRIBUTE_TYPE_DNP3TIME:
       { 
         if(length == 6)
         {
           TMWTYPES_MS_SINCE_70 msSince70;
           dnpdtime_readMsSince70(&msSince70, &pRxFragment->pMsgBuf[pRxFragment->offset]);
           dnpdtime_msSince70ToDateTime(&pValue->value.timeValue, &msSince70);
         }
         else
         {
           /* Invalid length */
           status = TMWDEFS_FALSE;
         }
       }
       break;
       
      default: 
        status = TMWDEFS_FALSE;
        break;
      }

      if(status)
      {
        pRxFragment->offset = pRxFragment->offset + length;
        return TMWDEFS_TRUE;
      }
    }
    else
    {
      invalidSize = TMWDEFS_TRUE;
    }
  }
  else
  {
    invalidSize = TMWDEFS_TRUE;
  }

  if(invalidSize)
  {
    DNPDIAG_ERROR(TMWDEFS_NULL, TMWDEFS_NULL, DNPDIAG_INVALID_SIZE);
  }

  pValue->length = 0;
  DNPDIAG_ERROR(TMWDEFS_NULL, TMWDEFS_NULL, DNPDIAG_ATTRIBUTE_VALUE);
  return TMWDEFS_FALSE;
}
#endif
