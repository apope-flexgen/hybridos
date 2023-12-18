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

/* file: sdnpo000.c
 * description: DNP3 Slave support for Object 0 Device Attributes
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/sdnpo000.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnputil.h"
#include "tmwscl/dnp/dnputil.h"
#include "tmwscl/dnp/sdnpdiag.h"

#include "tmwscl/utils/tmwtarg.h"

#if SDNPDATA_SUPPORT_OBJ0
  
static SDNPSESN_READ_STATUS _deviceAttributeRead(
  TMWSESN *pSession, 
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT pointIndex,
  void *pPoint,
  TMWTYPES_UCHAR readVariation,
  TMWTYPES_UCHAR variation)
{ 
  DNPDATA_ATTRIBUTE_VALUE data;
  void *pAttribute = sdnpdata_deviceAttrGetVar(pPoint, variation);

  if((pAttribute == TMWDEFS_NULL) ||
    !sdnpdata_deviceAttrRead(pAttribute, &data))
  {
    return(SDNPSESN_READ_FAILED);
  } 

  /* Make sure data will fit, then store it in response */
  if((pResponse->msgLength + 7 + data.length) > pResponse->maxLength)
    return(SDNPSESN_READ_MORE_DATA);

  /* Diagnostics */
  DNPDIAG_SHOW_DEVICE_ATTRIBUTE(pSession, pointIndex, variation, &data, 0);

  if(readVariation == 254)
  {
    /* Store object group, variation, and qualifier in message */
    pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_OBJ_0_DEVICE_ATTRIBUTES;
    pResponse->pMsgBuf[pResponse->msgLength++] = variation;
    pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_QUAL_8BIT_START_STOP;
    pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)pointIndex;
    pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)pointIndex;
  }

  /* Store data in message */
  dnputil_putAttrValueInMessage(pResponse, &data); 
  return (SDNPSESN_READ_COMPLETE);
} 

static SDNPSESN_READ_STATUS _readVx(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT messageIndex,
  TMWTYPES_USHORT pointNum,
  void *pPoint,
  SDNPUTIL_STATIC_DESC *pDesc)
{
  TMWTYPES_UCHAR nextVariation;
  TMWTYPES_UCHAR property; 
  SDNPSESN_READ_STATUS readStatus = SDNPSESN_READ_FAILED;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_UCHAR variation = pDesc->variation;

  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(messageIndex);

  if(variation == 255)
  {
#if !TEST_OLD_FORMAT 
    TMWTYPES_USHORT dataLength;
    TMWTYPES_USHORT typeIndex;

    readStatus = SDNPSESN_READ_COMPLETE;

    /* variation is stored in readGroupIndex when reading attributes 
     * This supports the case that the response required multiple fragments
     */
    variation = (TMWTYPES_UCHAR)pSDNPSession->readGroupIndex;

    /* Skip type and length fields, but save index to type field */
    typeIndex = pResponse->msgLength;
    pResponse->msgLength+=2;
    
    /* return list of variations supported */
    while(sdnpdata_deviceAttrNext(pPoint, variation, &nextVariation, &property))
    {
      /* Make sure data will fit, then store it in response */
      if((pResponse->msgLength + 2) > pResponse->maxLength)
      {
        pSDNPSession->readPointIndex = pointNum;
        pSDNPSession->readGroupIndex = variation;
        readStatus = SDNPSESN_READ_MORE_DATA;
        break;
      }
      variation = nextVariation;

      DNPDIAG_SHOW_DEVICE_ATTR_PROP(pSession, pointNum, variation, property);
      pResponse->pMsgBuf[pResponse->msgLength++] = variation;
      pResponse->pMsgBuf[pResponse->msgLength++] = property;
    }

    if(readStatus != SDNPSESN_READ_MORE_DATA)
      pSDNPSession->readGroupIndex = 0;

    dataLength = pResponse->msgLength - (typeIndex + 2);
    if(dataLength < 256)
    {
      if(dataLength == 0)
      {
        pResponse->msgLength = typeIndex;
        return(SDNPSESN_READ_FAILED);
      }
      pResponse->pMsgBuf[typeIndex++] = DNPDEFS_ATTRIBUTE_TYPE_LIST;
      pResponse->pMsgBuf[typeIndex] = (TMWTYPES_UCHAR)dataLength;
    }
    else
    {
      pResponse->pMsgBuf[typeIndex++] = DNPDEFS_ATTRIBUTE_TYPE_EXLIST;
      pResponse->pMsgBuf[typeIndex] = (TMWTYPES_UCHAR)(dataLength - 256);
    }
#else
/* ONLY FOR TESTING old format of object group 0 variation 255 response */ 
    /* DO NOT USE */
    TMWTYPES_USHORT dataLength;
    TMWTYPES_USHORT sizePrefixIndex;

    /* Old format only worked for index 0 */
    if(pointNum != 0)
    {
      return(SDNPSESN_READ_FAILED);
    }

    readStatus = SDNPSESN_READ_COMPLETE;
    variation = 0;

    /* This simple code assumes this is first object in response */
    pResponse->msgLength = 4;

    /* Store object group, variation, and qualifier in message */
    pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_OBJ_0_DEVICE_ATTRIBUTES;
    pResponse->pMsgBuf[pResponse->msgLength++] = 255;
    pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_QUAL_16BIT_FREE_FORMAT;
    pResponse->pMsgBuf[pResponse->msgLength++] = 1;
    sizePrefixIndex = pResponse->msgLength;
    pResponse->msgLength +=2;
    
    /* return list of variations supported */
    while(sdnpdata_deviceAttrNext(pPoint, variation, &nextVariation, &property))
    {
      /* Make sure data will fit, then store it in response */
      if((pResponse->msgLength + 2) > pResponse->maxLength)
      {
        /* Just end it here */
        return(SDNPSESN_READ_MORE_DATA);
      }
      variation = nextVariation;

      DNPDIAG_SHOW_DEVICE_ATTR_PROP(pSession, pointNum, variation, property);
      pResponse->pMsgBuf[pResponse->msgLength++] = variation;
      pResponse->pMsgBuf[pResponse->msgLength++] = property; 
    }
    
    /* Store 16 bit length of data after qualifier 0x5b */
    dataLength = pResponse->msgLength - (sizePrefixIndex +2);
    tmwtarg_store16(&dataLength, &pResponse->pMsgBuf[sizePrefixIndex]); 
#endif
  }
  else if(variation == 254)
  {
    /* return all of the variations supported */
    /* variation is stored in readGroupIndex when reading attributes */
    variation = (TMWTYPES_UCHAR)pSDNPSession->readGroupIndex;
    while(sdnpdata_deviceAttrNext(pPoint, variation, &nextVariation, &property))
    {
      readStatus =_deviceAttributeRead(pSession, pResponse, pointNum, pPoint, 254, nextVariation);
      if(readStatus == SDNPSESN_READ_MORE_DATA)
      {
        pSDNPSession->readPointIndex = pointNum;
        pSDNPSession->readGroupIndex = variation;
        break;
      }
      variation = nextVariation; 
    }
    if(readStatus == SDNPSESN_READ_COMPLETE)
    {
      pSDNPSession->readGroupIndex = 0;
    }
  }
  else
  {
    readStatus = _deviceAttributeRead(pSession, pResponse, pointNum, pPoint, variation, variation);
  }

  return (readStatus);
}

/* function: sdnpo000_readObj0 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo000_readObj0(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  SDNPSESN_QUAL qualifier)
{
  SDNPUTIL_STATIC_DESC desc;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_UCHAR variation = pObjHeader->variation;

  if(variation == 0)
  {
    pSDNPSession->iin |= DNPDEFS_IIN_OBJECT_UNKNOWN;
    return (SDNPSESN_READ_FAILED);
  }

  desc.group = DNPDEFS_OBJ_0_DEVICE_ATTRIBUTES;
  desc.readVariation = variation;
  desc.pQuantityFunc = sdnpdata_deviceAttrQuantity;
  desc.pGetPointFunc = sdnpdata_deviceAttrGetPoint;
  desc.variation = variation;
  desc.pReadIntoRespFunc = _readVx;
  desc.readBits = TMWDEFS_FALSE;
  
#if SDNPDATA_SUPPORT_CLASS0_POINT
  desc.pIsInClass0Func = TMWDEFS_NULL;
#endif
 
  desc.sizeInBytes = 6;

  return(sdnputil_readStatic(pSession,
    pRequest, pResponse, pObjHeader, qualifier, &desc));
}

/* function: sdnpo000_writeObj00 */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo000_writeObj00(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_BOOL status = TMWDEFS_FALSE; 
  void *pPoint;
  void *pAttribute;
  DNPDATA_ATTRIBUTE_VALUE data;

  /* Response to a write request is null response so we don't need to
   *  put anything into it here.
   */
  TMWTARG_UNUSED_PARAM(pResponse);
  
  /* Get data from message */
  if(!dnputil_getAttrValueFromMessage(pRxFragment, &data))
  {
    pRxFragment->offset = pRxFragment->msgLength;
    return(TMWDEFS_FALSE);
  }

  /* Diagnostics */
  DNPDIAG_SHOW_DEVICE_ATTRIBUTE(pSession, pObjHeader->firstPointNumber, pObjHeader->variation, &data, TMWDIAG_ID_RX);
    
  /* Get point */
  pPoint = sdnpdata_deviceAttrGetPoint(pSDNPSession->pDbHandle, pObjHeader->firstPointNumber);
  if(pPoint != TMWDEFS_NULL)
  {
    pAttribute = sdnpdata_deviceAttrGetVar(pPoint, pObjHeader->variation);
    if((pAttribute != TMWDEFS_NULL) && sdnpdata_deviceAttrWrite(pAttribute, &data))
    {
      status = TMWDEFS_TRUE;
    }
  }
  return(status);
}
 
#endif /* SDNPDATA_SUPPORT_OBJ0 */
