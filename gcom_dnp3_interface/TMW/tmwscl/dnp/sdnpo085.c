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

/* file: sdnpo085.c
 * description: DNP Slave functionality for Object 85 Data Set Prototype Object
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"

#include "tmwscl/dnp/sdnpo085.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnputil.h"
#include "tmwscl/dnp/dnputil.h"
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/sdnpdiag.h"

#if SDNPDATA_SUPPORT_OBJ85

/* function: _readv1 */
static SDNPSESN_READ_STATUS TMWDEFS_CALLBACK _readv1(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT messageIndex,
  TMWTYPES_USHORT pointNum,
  void *pPoint,
  SDNPUTIL_STATIC_DESC *pDesc)
{ 
  int i;
  TMWTYPES_USHORT sizePrefixIndex;
  TMWTYPES_USHORT dataLength;
  TMWTYPES_UCHAR  numberElems;
  TMWTYPES_UCHAR  length;
  TMWTYPES_UCHAR  uuid[16];
  DNPDATA_DATASET_DESCR_ELEM *pElem;
  int diagIndex = 1;

  TMWTARG_UNUSED_PARAM(messageIndex);
  TMWTARG_UNUSED_PARAM(pDesc);

  if((pResponse->msgLength +30) > pResponse->maxLength)
  {
    /* Not enough space left in fragment */
    return(SDNPSESN_READ_MORE_DATA);
  }

  /* Returns pointer to Data Set Prototype array in ROM or RAM and 
   * number of elements in prototype array
   */
  pElem = sdnpdata_datasetProtoRead(pPoint, &numberElems, uuid);
  if(pElem == TMWDEFS_NULL)
    return(SDNPSESN_READ_COMPLETE);

  /* save pointer for size prefix */
  sizePrefixIndex = pResponse->msgLength;
  pResponse->msgLength += 2; 
 
  /* Mandatory first element is Data Set prototype ID */
  length = dnputil_lengthRequired(pointNum);
  pResponse->pMsgBuf[pResponse->msgLength++] = length+3;
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_DATASET_DESCR_ID;
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_DATASET_TYPE_NONE;
  pResponse->pMsgBuf[pResponse->msgLength++] = 0; /* Max length */
  dnputil_putIntInMessage(&pResponse->pMsgBuf[pResponse->msgLength], pointNum, length);
  pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + length);

  /* Mandatory second element is UUID */
  pResponse->pMsgBuf[pResponse->msgLength++] = 19; /* Length of UUID + 3 */
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_DATASET_DESCR_UUID;
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_DATASET_TYPE_NONE;
  pResponse->pMsgBuf[pResponse->msgLength++] = 0; /* Max length */

  /* Copy UUID into response */
  memcpy(&pResponse->pMsgBuf[pResponse->msgLength], uuid, 16); 
  pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + 16);
  
#if TMWCNFG_SUPPORT_DIAG
  { /* Diagnostics */
    DNPDATA_DATASET_DESCR_ELEM  elem;
    elem.descrElemType = DNPDEFS_DATASET_DESCR_UUID;
    elem.dataTypeCode  = DNPDEFS_DATASET_TYPE_NONE;
    elem.maxDataLength = 0; 
    elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
    elem.ancillaryValue.length = 16;
    elem.ancillaryValue.value.pStrValue = uuid;
    DNPDIAG_SHOW_DATASET_PROTO(pSession, pointNum, 0, &elem, 0);
  }
#endif

  for(i=0; i< numberElems; i++)
  {
    if((pResponse->msgLength + pElem->ancillaryValue.length +4) > pResponse->maxLength)
    {
      /* Not enough space left in fragment */
      sdnpdata_datasetProtoRelease(pPoint);
      pResponse->msgLength = sizePrefixIndex;
      return(SDNPSESN_READ_MORE_DATA);
    }

    pResponse->pMsgBuf[pResponse->msgLength++] = pElem->ancillaryValue.length +3;
    pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)pElem->descrElemType;
    pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)pElem->dataTypeCode;
    pResponse->pMsgBuf[pResponse->msgLength++] = pElem->maxDataLength;

    dnputil_putValueInMessage(pResponse->pMsgBuf, &pResponse->msgLength, &pElem->ancillaryValue);

    /* Diagnostics */
    DNPDIAG_SHOW_DATASET_PROTO(pSession, pointNum, (TMWTYPES_UCHAR)diagIndex, pElem, 0);
    diagIndex++;
    pElem++;
  }

  /* Store 16 bit length of data after qualifier 0x5b */
  dataLength = pResponse->msgLength - (sizePrefixIndex +2);
  tmwtarg_store16(&dataLength, &pResponse->pMsgBuf[sizePrefixIndex]);

  sdnpdata_datasetProtoRelease(pPoint);
  return(SDNPSESN_READ_COMPLETE);
}

/* function: sdnpo085_readObj85 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo085_readObj85(
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
    /* Only variation supported is 1 */
    variation = 1; 
  }
  if(variation != 1)
  {
    pSDNPSession->iin |= DNPDEFS_IIN_OBJECT_UNKNOWN;
    return (SDNPSESN_READ_FAILED);
  }

  desc.group = DNPDEFS_OBJ_85_DATASET_PROTO;
  desc.readVariation = variation;
  desc.pQuantityFunc = sdnpdata_datasetProtoQuantity;
  desc.pGetPointFunc = sdnpdata_datasetProtoGetPoint;
#if SDNPDATA_SUPPORT_CLASS0_POINT
  desc.pIsInClass0Func = TMWDEFS_NULL;
#endif

  desc.variation = 1;
  desc.pReadIntoRespFunc = _readv1;
  desc.readBits = TMWDEFS_FALSE;

  /* Minimum size required. */
  desc.sizeInBytes = 48;

  return(sdnputil_readStatic(pSession,
    pRequest, pResponse, pObjHeader, qualifier, &desc));
}

/* function: sdnpo085_writeObj085v1 */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo085_writeObj085v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_USHORT index;
  DNPDATA_DATASET_DESCR_ELEM descr;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_BOOL status = TMWDEFS_TRUE;

  TMWTARG_UNUSED_PARAM(pResponse);
 
  /* Validate qualifier */
  if (pObjHeader->qualifier != DNPDEFS_QUAL_16BIT_FREE_FORMAT)
  {
    /* Don't try to process rest of message */
    pRxFragment->offset = pRxFragment->msgLength;
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_BAD_QUAL); 
    return(TMWDEFS_FALSE);
  }

  /* Loop through the points in the request */
  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    void *pPoint;
    int elemIndex;
    int diagIndex;
    TMWTYPES_ULONG prototypeId;
    TMWTYPES_ULONG stopIndex;
    TMWTYPES_BOOL invalidLength;
    TMWTYPES_USHORT sizePrefix;
    TMWTYPES_UCHAR length;

    pPoint = TMWDEFS_NULL;
    prototypeId = 0;
    stopIndex = 0;
    elemIndex = 0;
    diagIndex = 1;

    /* Protect against badly formatted message */
    invalidLength = TMWDEFS_TRUE;
    if((pRxFragment->offset+2) <= pRxFragment->msgLength)
    { 
      /* Get sizePrefix */
      tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &sizePrefix);
      pRxFragment->offset += 2;

      stopIndex = pRxFragment->offset + sizePrefix;
      if(stopIndex <= pRxFragment->msgLength)
      {
        invalidLength = TMWDEFS_FALSE;
      }
    }

    /* Protect against badly formatted message */
    if(invalidLength)
    {
      pRxFragment->offset = pRxFragment->msgLength;
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
      return(TMWDEFS_FALSE);
    }

    /* Make sure at least length byte is there */
    while(pRxFragment->offset < stopIndex)
    {
      length = pRxFragment->pMsgBuf[pRxFragment->offset++]; 
      
      /* Protect against badly formatted message */
      if((pRxFragment->offset+length) > stopIndex)
      {
        pRxFragment->offset = pRxFragment->msgLength;
        DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
        return(TMWDEFS_FALSE);
      }

      descr.descrElemType = (DNPDEFS_DATASET_DESCR_CODE)pRxFragment->pMsgBuf[pRxFragment->offset++];  
      descr.dataTypeCode  = (DNPDEFS_DATASET_TYPE_CODE)pRxFragment->pMsgBuf[pRxFragment->offset++]; 
      descr.maxDataLength = pRxFragment->pMsgBuf[pRxFragment->offset++]; 

      length = length - 3;
      dnputil_getAncValueFromMessage(pRxFragment, descr.descrElemType, length, &descr.ancillaryValue);

      if(elemIndex == 0)
      {
        /* First element must be Prototype ID */
        if(descr.descrElemType != DNPDEFS_DATASET_DESCR_ID)
        {
          DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_DATASET_MANDATORY);
          pRxFragment->offset = stopIndex;
          status = TMWDEFS_FALSE;
        }
        prototypeId = descr.ancillaryValue.value.uint32Value;
      }
      else if(elemIndex == 1)
      {
        /* Second element must be UUID */
        if(descr.descrElemType != DNPDEFS_DATASET_DESCR_UUID)
        {
          DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_DATASET_MANDATORY);
          pRxFragment->offset = stopIndex;
          status = TMWDEFS_FALSE;
        }
        else
        {
          pPoint = sdnpdata_datasetProtoCreatePoint(pSDNPSession->pDbHandle, (TMWTYPES_USHORT)prototypeId, descr.ancillaryValue.value.pStrValue);
          if(pPoint == TMWDEFS_NULL)
          {
            DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_DATASET_FAIL);
            pRxFragment->offset = stopIndex;
            status = TMWDEFS_FALSE;
          }
          DNPDIAG_SHOW_DATASET_PROTO(pSession, (TMWTYPES_USHORT)prototypeId, 0, &descr, TMWDIAG_ID_RX);
        } 
      }
      else 
      {
        if(!sdnpdata_datasetProtoWrite(pPoint, (TMWTYPES_UCHAR)(elemIndex -2), &descr))
        {
          pRxFragment->offset = stopIndex;
          status = TMWDEFS_FALSE;
        }

        /* Diagnostics */
        DNPDIAG_SHOW_DATASET_PROTO(pSession, (TMWTYPES_USHORT)prototypeId, (TMWTYPES_UCHAR)diagIndex, &descr, TMWDIAG_ID_RX);
        diagIndex++;
      }
      elemIndex++;  
    }
  }
  return(status);
} 

#endif /* SDNPDATA_SUPPORT_OBJ085 */
