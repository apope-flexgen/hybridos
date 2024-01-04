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

/* file: sdnpo086.c
 * description: DNP Slave functionality for Object 86 Data Set Descriptor Object
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"

#include "tmwscl/dnp/sdnpo086.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnputil.h"
#include "tmwscl/dnp/dnputil.h"
#include "tmwscl/dnp/sdnpdiag.h"
 
#if SDNPDATA_SUPPORT_OBJ86


/* function: _isSupportedVariation() */
static TMWTYPES_BOOL TMWDEFS_LOCAL _isSupportedVariation(
  TMWTYPES_UCHAR variation)
{
  switch(variation)
  {
#if SDNPDATA_SUPPORT_OBJ86_V1
  case 1:
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ86_V2
  case 2:
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ86_V3
  case 3:
    break;
#endif
  default:
    return(TMWDEFS_FALSE);
  }

  return(TMWDEFS_TRUE);
}
  
#if SDNPDATA_SUPPORT_OBJ86_V1
   
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
  TMWTYPES_UCHAR  numberElems;
  TMWTYPES_UCHAR  length;
  TMWTYPES_USHORT sizePrefixIndex;
  TMWTYPES_USHORT dataLength;
  DNPDATA_DATASET_DESCR_ELEM *pDescrElem;

  TMWTARG_UNUSED_PARAM(messageIndex);
  TMWTARG_UNUSED_PARAM(pDesc);

  /* Returns pointer to Data set Descriptor in ROM or RAM and number of elements in descriptor */
  pDescrElem = sdnpdata_datasetDescrReadCont(pPoint, &numberElems);
  if(pDescrElem == TMWDEFS_NULL)
    return(SDNPSESN_READ_COMPLETE);

  /* save pointer for size prefix */
  sizePrefixIndex = pResponse->msgLength;
  pResponse->msgLength += 2; 
 
  /* Mandatory first element is data set id or point number*/
  length = dnputil_lengthRequired(pointNum);
  pResponse->pMsgBuf[pResponse->msgLength++] = length+3;
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_DATASET_DESCR_ID;
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_DATASET_TYPE_NONE;
  pResponse->pMsgBuf[pResponse->msgLength++] = 0; /* Max length */
  dnputil_putIntInMessage(&pResponse->pMsgBuf[pResponse->msgLength], pointNum, length);
  pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + length);

  for(i=0; i< numberElems; i++)
  { 
    if((pResponse->msgLength + pDescrElem->ancillaryValue.length +4) > pResponse->maxLength)
    {
      /* Not enough space left in message */
      sdnpdata_datasetDescrRelease(pPoint);
      pResponse->msgLength = sizePrefixIndex;
      return(SDNPSESN_READ_MORE_DATA);
    }

    pResponse->pMsgBuf[pResponse->msgLength++] = pDescrElem->ancillaryValue.length +3;
    pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)pDescrElem->descrElemType;
    pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)pDescrElem->dataTypeCode;
    pResponse->pMsgBuf[pResponse->msgLength++] = pDescrElem->maxDataLength;

    dnputil_putValueInMessage(pResponse->pMsgBuf, &pResponse->msgLength, &pDescrElem->ancillaryValue);

    /* Diagnostics */
    DNPDIAG_SHOW_DATASET_DSCR_CONT(pSession, pointNum, (TMWTYPES_UCHAR)i, pDescrElem, 0);

    pDescrElem++;
  }

  /* Store 16 bit length of data after qualifier 0x5b */
  dataLength = pResponse->msgLength - (sizePrefixIndex +2);
  tmwtarg_store16(&dataLength, &pResponse->pMsgBuf[sizePrefixIndex]);

  sdnpdata_datasetDescrRelease(pPoint);
  return(SDNPSESN_READ_COMPLETE);
}
#endif


#if SDNPDATA_SUPPORT_OBJ86_V2
/* function: _readv2 */
static SDNPSESN_READ_STATUS TMWDEFS_CALLBACK _readv2(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT messageIndex,
  TMWTYPES_USHORT pointNum,
  void *pPoint,
  SDNPUTIL_STATIC_DESC *pDesc)
{
  TMWTYPES_UCHAR characteristics;

  TMWTARG_UNUSED_PARAM(messageIndex);
  TMWTARG_UNUSED_PARAM(pDesc);
 
  /* Read attributes */
  sdnpdata_datasetDescrReadChars(pPoint, &characteristics);

  /* Diagnostics */
  DNPDIAG_SHOW_DATASET_DSCR_CHRS(pSession, pointNum, characteristics, 0);

  /* Store into response */
  pResponse->pMsgBuf[pResponse->msgLength++] = characteristics;

  return(SDNPSESN_READ_COMPLETE);
}
#endif

#if SDNPDATA_SUPPORT_OBJ86_V3
/* function: _readv3 */
static SDNPSESN_READ_STATUS TMWDEFS_CALLBACK _readv3(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT messageIndex,
  TMWTYPES_USHORT pointNum,
  void *pPoint,
  SDNPUTIL_STATIC_DESC *pDesc)
{ 
  int i;
  DNPDATA_DATASET_DESCR_INDEX *pDescrElem;
  TMWTYPES_USHORT sizePrefixIndex;
  TMWTYPES_USHORT dataLength;
  TMWTYPES_UCHAR  numberElems;
  TMWTYPES_UCHAR  length;

  TMWTARG_UNUSED_PARAM(messageIndex);
  TMWTARG_UNUSED_PARAM(pDesc);

  /* Returns pointer to Data set Descriptor in ROM or RAM and number of elements in descriptor */
  /* PointNum is Data Set Identifier */
  /* should I get pPoint first? */
  pDescrElem = sdnpdata_datasetDescrReadIndex(pPoint, &numberElems);
  if(pDescrElem == TMWDEFS_NULL)
    return(SDNPSESN_READ_COMPLETE);

  /* save pointer for size prefix */
  sizePrefixIndex = pResponse->msgLength;
  pResponse->msgLength += 2; 

  length = dnputil_lengthRequired(pointNum);
  pResponse->pMsgBuf[pResponse->msgLength++] = length;
  dnputil_putIntInMessage(&pResponse->pMsgBuf[pResponse->msgLength], pointNum, length);
  pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + length);
 
  /* These Point Type/Point Index pairs are in same order as DAEL elements
   * in Data Set Descriptor 
   */
  for(i=0; i< numberElems; i++)
  {
    length = dnputil_lengthRequired(pDescrElem->pointIndex);

    if((pResponse->msgLength + length+2) > pResponse->maxLength)
    {
      /* Not enough space left in message */
      sdnpdata_datasetDescrRelease(pPoint);
      pResponse->msgLength = sizePrefixIndex;
      return(SDNPSESN_READ_MORE_DATA);
    }

    pResponse->pMsgBuf[pResponse->msgLength++] = length +1;
    pResponse->pMsgBuf[pResponse->msgLength++] = pDescrElem->pointType;

    dnputil_putIntInMessage(&pResponse->pMsgBuf[pResponse->msgLength], pDescrElem->pointIndex, length);
    pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + length);

    /* Diagnostics */
    DNPDIAG_SHOW_DATASET_DSCR_INDX(pSession, pointNum, (TMWTYPES_UCHAR)i, pDescrElem, 0);
    pDescrElem++;
  }

  /* Store 16 bit length of data after qualifier 0x5b */
  dataLength = pResponse->msgLength - (sizePrefixIndex +2);
  tmwtarg_store16(&dataLength, &pResponse->pMsgBuf[sizePrefixIndex]);

  sdnpdata_datasetDescrRelease(pPoint);
  return(SDNPSESN_READ_COMPLETE);
}
#endif

/* function: _setVariationInfo() */
static void TMWDEFS_LOCAL _setVariationInfo(
  SDNPUTIL_STATIC_DESC *pDesc,
  TMWTYPES_UCHAR variation)
{
  switch(variation)
  {
  default:
    /* Fall through to first one that is supported */ 
#if SDNPDATA_SUPPORT_OBJ86_V1
  case 1:
    pDesc->variation = 1;
    pDesc->pReadIntoRespFunc = _readv1;
    pDesc->readBits = TMWDEFS_FALSE;
    pDesc->sizeInBytes = 18; 
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ86_V2
  case 2:
    pDesc->variation = 2;
    pDesc->pReadIntoRespFunc = _readv2;
    pDesc->readBits = TMWDEFS_FALSE;
    pDesc->sizeInBytes = 1;
    break;
#endif

#if SDNPDATA_SUPPORT_OBJ86_V3
  case 3:
    pDesc->variation = 3;
    pDesc->pReadIntoRespFunc = _readv3;
    pDesc->readBits = TMWDEFS_FALSE;
    pDesc->sizeInBytes = 14;
    break;
#endif
  }
}

/* function: sdnpo086_readObj86 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo086_readObj86(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  SDNPSESN_QUAL qualifier)
{ 
  SDNPUTIL_STATIC_DESC desc;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_UCHAR variation = pObjHeader->variation;

  if(!_isSupportedVariation(variation))
  {
    pSDNPSession->iin |= DNPDEFS_IIN_OBJECT_UNKNOWN;
    return (SDNPSESN_READ_FAILED);
  }

  desc.group = DNPDEFS_OBJ_86_DATASET_DESCR;
  desc.readVariation = variation;
  desc.pQuantityFunc = sdnpdata_datasetDescrQuantity;
  desc.pGetPointFunc = sdnpdata_datasetDescrGetPoint;
#if SDNPDATA_SUPPORT_CLASS0_POINT
  desc.pIsInClass0Func = TMWDEFS_NULL;
#endif

  _setVariationInfo(&desc, variation);

  return(sdnputil_readStatic(pSession,
    pRequest, pResponse, pObjHeader, qualifier, &desc));
}

#if SDNPDATA_SUPPORT_OBJ86_V1
/* function: sdnpo086_writeObj086v1 */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo086_writeObj086v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_USHORT index;
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
    TMWTYPES_ULONG datasetId;
    TMWTYPES_BOOL invalidLength;
    TMWTYPES_USHORT sizePrefix;
    TMWTYPES_ULONG stopIndex;
    TMWTYPES_UCHAR length;
    DNPDATA_DATASET_DESCR_ELEM descr;
 
    pPoint = TMWDEFS_NULL; 
    elemIndex = 0;
    datasetId = 0xffffffff;
    stopIndex = 0;
 
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

      if(datasetId == 0xffffffff)
      {
        if(descr.descrElemType == DNPDEFS_DATASET_DESCR_ID)
        {
          datasetId = descr.ancillaryValue.value.uint32Value;
          pPoint = sdnpdata_datasetDescrCreatePoint(pSDNPSession->pDbHandle, (TMWTYPES_USHORT)datasetId);
          if(pPoint == TMWDEFS_NULL)
          {
            DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_DATASET_FAIL);  
            pRxFragment->offset = stopIndex;
            status = TMWDEFS_FALSE;
          }
        }
        else
        {
          DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_DATASET_MANDATORY); 
          pRxFragment->offset = stopIndex;
          status = TMWDEFS_FALSE;
        }
      }
      else
      {
        if(!sdnpdata_datasetDescrWriteCont(pPoint, (TMWTYPES_UCHAR)elemIndex, &descr))
        { 
          pRxFragment->offset = stopIndex;
          status = TMWDEFS_FALSE;
        }

        /* Diagnostics */
        DNPDIAG_SHOW_DATASET_DSCR_CONT(pSession, (TMWTYPES_USHORT)datasetId, (TMWTYPES_UCHAR)elemIndex, &descr, TMWDIAG_ID_RX);
        elemIndex++;
      }   
    }
  }
  return(status);
} 
#endif

#if SDNPDATA_SUPPORT_OBJ86_V3
/* function: sdnpo086_writeObj086v3 */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo086_writeObj086v3(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{ 
  TMWTYPES_USHORT index;
  DNPDATA_DATASET_DESCR_INDEX descr;
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
    TMWTYPES_ULONG datasetId;
    TMWTYPES_ULONG stopIndex;
    TMWTYPES_BOOL invalidLength;
    TMWTYPES_USHORT sizePrefix;
    TMWTYPES_UCHAR length;

    datasetId = 0;
    elemIndex = 0;
    stopIndex = 0;

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

    /* Get length of data set id 1, 2 or 4*/
    length = pRxFragment->pMsgBuf[pRxFragment->offset++]; 

    /* Protect against badly formatted message */
    if((pRxFragment->offset+length) <= stopIndex)
    {
      datasetId = dnputil_getUintFromMessage(&pRxFragment->pMsgBuf[pRxFragment->offset], length);
      pRxFragment->offset = pRxFragment->offset + length;

      pPoint = sdnpdata_datasetDescrGetPoint(pSDNPSession->pDbHandle, (TMWTYPES_USHORT)datasetId);
      if(pPoint == TMWDEFS_NULL) 
      {
        pRxFragment->offset = stopIndex;
        status = TMWDEFS_FALSE;
      }
     
      /* Make sure at least length byte is there */
      invalidLength = TMWDEFS_TRUE;
      while(pRxFragment->offset < stopIndex)
      {
        length = pRxFragment->pMsgBuf[pRxFragment->offset++]; 

        /* Protect against badly formatted message */
        if((pRxFragment->offset+length) <= stopIndex)
        {
          descr.pointType  = pRxFragment->pMsgBuf[pRxFragment->offset++];  

          length--;
          descr.pointIndex = dnputil_getUintFromMessage(&pRxFragment->pMsgBuf[pRxFragment->offset], length);
          pRxFragment->offset = pRxFragment->offset + length;

          /* Diagnostics */
          DNPDIAG_SHOW_DATASET_DSCR_INDX(pSession, (TMWTYPES_USHORT)datasetId, (TMWTYPES_UCHAR)elemIndex, &descr, TMWDIAG_ID_RX);

          if(!sdnpdata_datasetDescrWriteIndex(pPoint, (TMWTYPES_UCHAR)elemIndex, &descr))
          {
            pRxFragment->offset = stopIndex;
            status = TMWDEFS_FALSE;
          }
          
          invalidLength = TMWDEFS_FALSE;
          elemIndex++;
        }
        else
        {
          pRxFragment->offset = stopIndex;
        }
      }
    }
    if(invalidLength)
    { 
      pRxFragment->offset = pRxFragment->msgLength;
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
      return(TMWDEFS_FALSE);
    }
  }

  return(status);
}
#endif

#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpo086_assignClass */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo086_assignClass(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWDEFS_CLASS_MASK classMask)
{
  SDNPSESN *pSDNPSession;
  TMWTYPES_BOOL status;
  TMWTYPES_USHORT index;

  pSDNPSession = (SDNPSESN *)pSession;

  /* limit requested number of points to quantity in database 
   * if status == TMWDEFS_FALSE IN2.2 will be set in response.
   */
  status = sdnputil_validateAssignQualifier(pObjHeader, sdnpdata_datasetDescrQuantity(pSDNPSession->pDbHandle));

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    void *pPoint;
    TMWTYPES_USHORT pointNumber;

    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    pPoint = sdnpdata_datasetDescrGetPoint(pSDNPSession->pDbHandle, pointNumber);
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

    if(!sdnpdata_datasetDescrAssignClass(pPoint, classMask))
      status = TMWDEFS_FALSE;
  }

  return(status);
}
#endif
 
#endif /* SDNPDATA_SUPPORT_OBJ086 */
