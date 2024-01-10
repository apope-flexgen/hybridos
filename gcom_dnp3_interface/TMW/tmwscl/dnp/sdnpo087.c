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

/* file: sdnpo087.c
 * description: DNP Slave functionality for Object 87 Data Set Object
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"

#include "tmwscl/dnp/sdnpo087.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnputil.h"
#include "tmwscl/dnp/dnputil.h"
#include "tmwscl/dnp/dnpdtime.h"
#include "tmwscl/dnp/sdnpdiag.h"
 
#if SDNPDATA_SUPPORT_OBJ87

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
  DNPDATA_DATASET_VALUE *pElem;
  TMWTYPES_USHORT sizePrefixIndex;
  TMWTYPES_USHORT msgLen;
  TMWTYPES_UCHAR numberElems;
  TMWTYPES_UCHAR length;
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWDTIME timeStamp;

  TMWTARG_UNUSED_PARAM(pSession);
  TMWTARG_UNUSED_PARAM(pDesc);
  TMWTARG_UNUSED_PARAM(messageIndex);

  /* Returns pointer to Data Set in ROM or RAM and number of elements in Data Set */
  pElem = sdnpdata_datasetRead(pPoint, &numberElems, &timeStamp);
  if(pElem == TMWDEFS_NULL)
    return(SDNPSESN_READ_COMPLETE);

  /* save pointer for size prefix */
  sizePrefixIndex = pResponse->msgLength;
  pResponse->msgLength += 2; 

  /* Mandatory Data Set Identifier */
  length = dnputil_lengthRequired(pointNum);
  pResponse->pMsgBuf[pResponse->msgLength++] = length;
  dnputil_putIntInMessage(&pResponse->pMsgBuf[pResponse->msgLength], pointNum, length);
  pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + length);

  pResponse->pMsgBuf[pResponse->msgLength++] = 6;

  dnpdtime_dateTimeToMSSince70(&msSince70, &timeStamp);
  dnpdtime_writeMsSince70(&pResponse->pMsgBuf[pResponse->msgLength], &msSince70);
  pResponse->msgLength += 6;

  DNPDIAG_SHOW_DATASET_TIME(pSession, pointNum, &timeStamp, TMWDEFS_FALSE, 0);

  /* Data set elements starting after mandatory time stamp element */
  for(i=0; i< numberElems; i++)
  {
    if((pResponse->msgLength + pElem->length +1) > pResponse->maxLength)
    {
      /* Not enough space left in fragment */
      sdnpdata_datasetRelease(pPoint);
      pResponse->msgLength = sizePrefixIndex;
      return(SDNPSESN_READ_MORE_DATA);
    }

    pResponse->pMsgBuf[pResponse->msgLength++] = pElem->length;

    dnputil_putValueInMessage(pResponse->pMsgBuf, &pResponse->msgLength, pElem);
   
    /* Diagnostics */
    DNPDIAG_SHOW_DATASET(pSession, (TMWTYPES_UCHAR)i, pElem, DNPDEFS_FC_READ, TMWDEFS_FALSE, TMWDEFS_FALSE, 0);

    pElem++;
  }

  /* Store 16 bit length of data after qualifier 0x5b */
  msgLen = pResponse->msgLength - (sizePrefixIndex +2);
  tmwtarg_store16(&msgLen, &pResponse->pMsgBuf[sizePrefixIndex]);

  sdnpdata_datasetRelease(pPoint);
  return(SDNPSESN_READ_COMPLETE);
}

static void *TMWDEFS_CALLBACK _getDatasetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  void *pPoint = sdnpdata_datasetDescrGetPoint(pHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWTYPES_UCHAR characteristics;

    /* Read attributes to see if this is readable */
    sdnpdata_datasetDescrReadChars(pPoint, &characteristics);

    /* If characteristics says it is not a readable static data set, return null. */
    if((characteristics & (DNPDEFS_DATASET_CHAR_RD|DNPDEFS_DATASET_CHAR_ST)) == (DNPDEFS_DATASET_CHAR_RD|DNPDEFS_DATASET_CHAR_ST))
    {
      return(sdnpdata_datasetGetPoint(pHandle, pointNum));
    }
  }
  return TMWDEFS_NULL;
}

/* function: sdnpo087_readObj87 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo087_readObj87(
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
    variation = 1; 
  }
  
  if(variation != 1)
  {
    pSDNPSession->iin |= DNPDEFS_IIN_OBJECT_UNKNOWN;
    return (SDNPSESN_READ_FAILED);
  }

  desc.group = DNPDEFS_OBJ_87_DATASET_VALUE;
  desc.readVariation = variation;
  desc.pQuantityFunc = sdnpdata_datasetQuantity;
  desc.pGetPointFunc = _getDatasetPoint;
  desc.variation = 1;
  desc.pReadIntoRespFunc = _readv1;
  desc.readBits = TMWDEFS_FALSE;
#if SDNPDATA_SUPPORT_CLASS0_POINT
  desc.pIsInClass0Func = TMWDEFS_NULL;
#endif

  /* At least room for mandatory elements */
  desc.sizeInBytes = 14; 

  return(sdnputil_readStatic(pSession,
    pRequest, pResponse, pObjHeader, qualifier, &desc));
}

 TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnp087_datasetDescrRead(
  TMWSESN *pSession, 
  TMWTYPES_USHORT datasetId,
  TMWTYPES_UCHAR  maxNumber,
  DNPDEFS_DATASET_TYPE_CODE *pDataTypes,
  DNPDEFS_DATASET_DESCR_CODE *pElemTypes)
{
  TMWTYPES_UCHAR elemIndex;
  void *pPoint;
  DNPDATA_DATASET_DESCR_ELEM *pDescrElem;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession; 

  elemIndex = 0;
  pPoint = sdnpdata_datasetDescrGetPoint(pSDNPSession->pDbHandle, (TMWTYPES_USHORT)datasetId);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWTYPES_UCHAR numberDescrElems;
    TMWTYPES_UCHAR descrIndex;

    pDescrElem = sdnpdata_datasetDescrReadCont(pPoint, &numberDescrElems);
    for(descrIndex=0; descrIndex < numberDescrElems; descrIndex++)
    { 
      if(elemIndex >= maxNumber)
        break;

      pElemTypes[elemIndex] = pDescrElem->descrElemType;
      if((pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_DAEL)
        ||(pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_CTLS)
        ||(pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_CTLV))
      {
        pDataTypes[elemIndex++] = pDescrElem->dataTypeCode;
      }
      else if(pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_PTYP)
      {
        /* get prototype information */
        TMWTYPES_USHORT pointNum;
        void *pProtoPoint;
        TMWTYPES_UCHAR *pStrValue;

        if(pDescrElem->ancillaryValue.type == DNPDATA_VALUE_STRPTR)
          pStrValue = pDescrElem->ancillaryValue.value.pStrValue;
        else if(pDescrElem->ancillaryValue.type == DNPDATA_VALUE_STRARRAY)
          pStrValue = pDescrElem->ancillaryValue.value.strValue;
        else
          break;

        if(sdnpdata_datasetProtoGetID(pSDNPSession->pDbHandle, pStrValue, &pointNum)) 
        {
          pProtoPoint = sdnpdata_datasetProtoGetPoint(pSDNPSession->pDbHandle, pointNum);
          if(pProtoPoint != TMWDEFS_NULL)
          {
            int i;
            DNPDATA_DATASET_DESCR_ELEM *pProtoElem;
            TMWTYPES_UCHAR temp[16];
            TMWTYPES_UCHAR numberElems;

            pProtoElem = sdnpdata_datasetProtoRead(pProtoPoint, &numberElems, temp);
            if(pProtoElem != TMWDEFS_NULL) 
            {
              for(i=0; i<numberElems; i++)
              { 
                /* Don't write past end of array */
                if(elemIndex >= maxNumber)
                  break;

                pElemTypes[elemIndex] = pProtoElem->descrElemType;
                if((pProtoElem->descrElemType == DNPDEFS_DATASET_DESCR_DAEL)
                  ||(pProtoElem->descrElemType == DNPDEFS_DATASET_DESCR_CTLS)
                  ||(pProtoElem->descrElemType == DNPDEFS_DATASET_DESCR_CTLV))
                {
                  pDataTypes[elemIndex++] = pProtoElem->dataTypeCode;
                }
                pProtoElem++;
              }
              sdnpdata_datasetProtoRelease(pProtoPoint);
            }
            else
            {
              break;
            }
          }
          else
          {  
            DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_DATASET_PROTO);
            break;
          }
        }
        else
        {
          DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_DATASET_PROTO);
          break;
        }
      }
      pDescrElem++;
    }

    sdnpdata_datasetDescrRelease(pPoint);
  }

  return(elemIndex);
}

/* function: sdnpo087_writeObj087v1 */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo087_writeObj087v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_USHORT index;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession; 
  TMWTYPES_BOOL  status = TMWDEFS_TRUE;

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
    int  elemIndex;
    TMWTYPES_BOOL invalidLength;
    TMWTYPES_ULONG  datasetId;
    TMWTYPES_USHORT sizePrefix;
    TMWTYPES_ULONG stopIndex;
    TMWTYPES_UCHAR  numberDescrElems;
    TMWTYPES_UCHAR characteristics;
    DNPDATA_DATASET_VALUE elem;
    DNPDEFS_DATASET_DESCR_CODE elemTypes[DNPCNFG_MAX_DATASET_DESCR_ELEMS];
    DNPDEFS_DATASET_TYPE_CODE  dataTypes[DNPCNFG_MAX_DATASET_DESCR_ELEMS];

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

    /* Get Data Set Id from message */
    if(!dnputil_getValueFromMessage(pRxFragment->pMsgBuf, &pRxFragment->offset, stopIndex, DNPDEFS_DATASET_TYPE_UINT, &elem))
    {
      return(TMWDEFS_FALSE);
    }

    datasetId = elem.value.uint32Value;

    /* Read attributes to see if this is writable */
    pPoint = sdnpdata_datasetDescrGetPoint(pSDNPSession->pDbHandle, (TMWTYPES_USHORT)datasetId);
    if(pPoint == TMWDEFS_NULL) 
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_DATASET_DESCR);
      pRxFragment->offset = stopIndex;

      /* This will set IIN2.2 PARAMETER ERROR or OUT OF RANGE */
      return(TMWDEFS_FALSE);
    }

    sdnpdata_datasetDescrReadChars(pPoint, &characteristics);

    /* If characteristics says it is not a writable data set, skip it. */
    if((characteristics & DNPDEFS_DATASET_CHAR_WR) == 0)
    {
      pRxFragment->offset = stopIndex;
      status = TMWDEFS_FALSE;
      continue;
    }

    /* Read the Data Set descriptor information, including any prototypes required and
     * store the type information on the stack.
     */
    numberDescrElems = sdnp087_datasetDescrRead(pSession, (TMWTYPES_USHORT)datasetId, DNPCNFG_MAX_DATASET_DESCR_ELEMS, dataTypes, elemTypes);

    /* If no descriptor elements, its because there is no descriptor in the database 
     * if there are some, but not enough then thats a different problem.
     */
    if(numberDescrElems == 0)
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_DATASET_DESCR);
      pRxFragment->offset = stopIndex;

      /* This will set IIN2.2 PARAMETER ERROR or OUT OF RANGE */
      return(TMWDEFS_FALSE);
    }
 
    /* Get Time from message */ 
    if(!dnputil_getValueFromMessage(pRxFragment->pMsgBuf, &pRxFragment->offset, stopIndex, DNPDEFS_DATASET_TYPE_TIME, &elem))
    {
      pRxFragment->offset = stopIndex;
      return(TMWDEFS_FALSE);
    }

    if(elem.type != DNPDATA_VALUE_TIME)
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_DATASET_MANDATORY);
      pRxFragment->offset = stopIndex;
      status = TMWDEFS_FALSE;
      continue;
    }
    
    /* Create point for this data set id and store time 
     * Specification shows data set id as first element, and time as second element
     * but since data set id is also the index, storing it as a separate data set 
     * element would be redundant also, since both the data set id and time are 
     * mandatory, splitting them out makes it clear.
     */ 
    pPoint = sdnpdata_datasetCreatePoint(pSDNPSession->pDbHandle, (TMWTYPES_USHORT)datasetId, SDNPDATA_DATASET_MODE_WRITE, &elem.value.timeValue);
    if(pPoint == TMWDEFS_NULL)
    {
      pRxFragment->offset = stopIndex;
      status = TMWDEFS_FALSE;
      continue;
    }
    
    DNPDIAG_SHOW_DATASET_TIME(pSession, (TMWTYPES_USHORT)datasetId, &elem.value.timeValue, TMWDEFS_FALSE, TMWDIAG_ID_RX);

    elemIndex = 0;
    while(pRxFragment->offset < stopIndex)
    {
      DNPDEFS_DATASET_TYPE_CODE typeCode;
      if(elemIndex < numberDescrElems)
      {
        typeCode = dataTypes[elemIndex];
      }
      else
      { 
        /* setting to NONE and not returning would allow parsing of 
         * dataset leaving data as octet string 
         *   typeCode = DNPDEFS_DATASET_TYPE_NONE;
         * However, TB2004-004e says outstation must reject request.
         */

        /* There are some descriptor elements, but not enough */
        DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_DATASET_ELEMS);

        pRxFragment->offset = stopIndex;

        /* This will set IIN2.2 PARAMETER ERROR or OUT OF RANGE */
        return(TMWDEFS_FALSE);
      }

      if(!dnputil_getValueFromMessage(pRxFragment->pMsgBuf, &pRxFragment->offset, stopIndex, typeCode, &elem))
      {
        pRxFragment->offset = stopIndex;
        return(TMWDEFS_FALSE);
      }

      if((elemTypes[elemIndex] != DNPDEFS_DATASET_DESCR_CTLS)
        &&(elemTypes[elemIndex] != DNPDEFS_DATASET_DESCR_CTLV))
      {
        if(!sdnpdata_datasetWrite(pPoint, (TMWTYPES_UCHAR)elemIndex, &elem))
        {
          pRxFragment->offset = stopIndex;
          status = TMWDEFS_FALSE;
        }
        DNPDIAG_SHOW_DATASET(pSession, (TMWTYPES_UCHAR)elemIndex, &elem, DNPDEFS_FC_WRITE, TMWDEFS_FALSE, TMWDEFS_FALSE, TMWDIAG_ID_RX); 
      }
      else
      {
        DNPDIAG_SHOW_DATASET(pSession, (TMWTYPES_UCHAR)elemIndex, &elem, DNPDEFS_FC_WRITE, TMWDEFS_TRUE, TMWDEFS_FALSE, TMWDIAG_ID_RX); 
      }
      elemIndex++;
    }
  }
  return(status);
} 

/* function: _selopObj087v1 */
static TMWTYPES_BOOL TMWDEFS_LOCAL _selopObj087v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse, 
  TMWTYPES_UCHAR numberOfPoints,
  TMWTYPES_UCHAR status,
  TMWTYPES_BOOL isOperate)
{
  TMWTYPES_USHORT index;
  TMWTYPES_BOOL retStatus = TMWDEFS_TRUE;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
   
  /* Put object header in response */
  pResponse->pMsgBuf[pResponse->msgLength++] = 87;
  pResponse->pMsgBuf[pResponse->msgLength++] = 1;
  pResponse->pMsgBuf[pResponse->msgLength++] = 0x5b;
  pResponse->pMsgBuf[pResponse->msgLength++] = numberOfPoints;

  /* Loop through the points in the request */
  for(index = 0; index < numberOfPoints; index++)
  { 
    void *pPoint;
    int  elemIndex;
    int  valueIndex;
    TMWTYPES_BOOL invalidLength;
    TMWTYPES_ULONG  datasetId;
    TMWTYPES_USHORT sizePrefix;
    TMWTYPES_ULONG stopIndex;
    TMWTYPES_ULONG msgOffset;
    TMWTYPES_ULONG ctrlStatusOffset;
    TMWTYPES_UCHAR  numberDescrElems;
    SDNPDATA_DATASET_MODE mode;
    DNPDATA_DATASET_CTRL_VALUE elem[DNPCNFG_MAX_DATASET_CTRLS];
    DNPDEFS_DATASET_DESCR_CODE elemTypes[DNPCNFG_MAX_DATASET_DESCR_ELEMS];
    DNPDEFS_DATASET_TYPE_CODE  dataTypes[DNPCNFG_MAX_DATASET_DESCR_ELEMS];
    
    sizePrefix = 0;
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

    /* Make sure response can fit in buffer, request is 2 bytes smaller than echoed response because of IIN bits */
    if((pResponse->msgLength + sizePrefix+2) > pResponse->maxLength)
    { 
      /* Don't parse rest of request message */
      pRxFragment->offset = pRxFragment->msgLength;
      pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
      return(TMWDEFS_FALSE);
    }

    /* Copy request into response buffer, set control statuses later */
    memcpy(&pResponse->pMsgBuf[pResponse->msgLength], &pRxFragment->pMsgBuf[pRxFragment->offset-2], sizePrefix+2);
    pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + sizePrefix +2);

    /* Get Data Set Id from message */
    if(!dnputil_getValueFromMessage(pRxFragment->pMsgBuf, &pRxFragment->offset, stopIndex, DNPDEFS_DATASET_TYPE_INT, &elem[0].data))
    { 
      /* Don't parse rest of request message */
      pRxFragment->offset = pRxFragment->msgLength;
      return(TMWDEFS_FALSE);
    }

    datasetId = elem[0].data.value.uint32Value;

    /* Read the Data Set descriptor information, including any prototypes required and
     * store the type information on the stack.
     */
    numberDescrElems = sdnp087_datasetDescrRead(pSession, (TMWTYPES_USHORT)datasetId, DNPCNFG_MAX_DATASET_DESCR_ELEMS, dataTypes, elemTypes);

    /* Get Time from message */ 
    if(!dnputil_getValueFromMessage(pRxFragment->pMsgBuf, &pRxFragment->offset, stopIndex, DNPDEFS_DATASET_TYPE_TIME, &elem[0].data))
    {
      /* Don't parse rest of request message */
      pRxFragment->offset = pRxFragment->msgLength;
      return(TMWDEFS_FALSE);
    }

    DNPDIAG_SHOW_DATASET_TIME(pSession, (TMWTYPES_USHORT)datasetId, &elem[0].data.value.timeValue, TMWDEFS_FALSE, TMWDIAG_ID_RX);
   
    /* Create or get handle for point for this data set id */
    if(isOperate)
      mode = SDNPDATA_DATASET_MODE_OPERATE;
    else
      mode = SDNPDATA_DATASET_MODE_SELECT;

    pPoint = sdnpdata_datasetCreatePoint(pSDNPSession->pDbHandle, (TMWTYPES_USHORT)datasetId, mode, &elem[0].data.value.timeValue);
    if(pPoint == TMWDEFS_NULL)
    {
      status = DNPDEFS_DATASET_ST_NOT_SUPPORTED;
    }

    /* loop through all of the dataset elements in request message */
    ctrlStatusOffset = 0;
    valueIndex = 0;
    elemIndex = 0;
    while(pRxFragment->offset < stopIndex)
    {
      DNPDEFS_DATASET_TYPE_CODE typeCode;

      if(elemIndex < numberDescrElems)
      {
        typeCode = dataTypes[elemIndex];
      }
      else
      { 
        pRxFragment->offset = pRxFragment->msgLength;
        pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
        DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_DATASET_ELEMS);
        return(TMWDEFS_FALSE);
      }

      /* Save offset in case this is control status, which might need to be overwritten */
      msgOffset = pRxFragment->offset;
      if(!dnputil_getValueFromMessage(pRxFragment->pMsgBuf, &pRxFragment->offset, stopIndex, typeCode, &elem[valueIndex].data))
      {
        pRxFragment->offset = pRxFragment->msgLength;
        return(TMWDEFS_FALSE);
      }

      /* If status == failure was passed in, or set in this function, just skip this code */
      if(status == DNPDEFS_DATASET_ST_SUCCESS)
      {
#if TMWCNFG_SUPPORT_DIAG
        /* Diagnostics */
        if((elemTypes[elemIndex] == DNPDEFS_DATASET_DESCR_CTLV) 
          ||(elemTypes[elemIndex] == DNPDEFS_DATASET_DESCR_CTLS))
        {
          DNPDIAG_SHOW_DATASET(pSession, (TMWTYPES_UCHAR)elemIndex, &elem[valueIndex].data, DNPDEFS_FC_OPERATE, TMWDEFS_FALSE, TMWDEFS_FALSE, TMWDIAG_ID_RX); 
        }
        else
        {
          /* indicate this element is ignored since it is not a control. */
          DNPDIAG_SHOW_DATASET(pSession, (TMWTYPES_UCHAR)elemIndex, &elem[valueIndex].data, DNPDEFS_FC_OPERATE, TMWDEFS_TRUE, TMWDEFS_FALSE, TMWDIAG_ID_RX); 
        }
#endif
        /* Only control status and control value are acted on by outstation in a control operation.
         * Data elements and others are ignored.
         * Control status indicates the start of a control group containing control values
         */
        if(ctrlStatusOffset != 0)
        {
          if(elemTypes[elemIndex] == DNPDEFS_DATASET_DESCR_CTLV)
          {
            /* add this to list of values to be sent to target layer */
            elem[valueIndex].elemIndex = (TMWTYPES_UCHAR)elemIndex;
           
            valueIndex++;
            elemIndex++;

            if(valueIndex >= DNPCNFG_MAX_DATASET_CTRLS)
            {
              pRxFragment->offset = pRxFragment->msgLength;
              pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
              DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_DATASET_CTRLS);
              return(TMWDEFS_FALSE);
            }

            /* If more elements in request, continue */
            if(elemIndex < numberDescrElems)
              continue;
          }

          /* If this is another control status, or the last element in request, 
           * send outstanding control to target layer 
           */
          if((elemTypes[elemIndex] == DNPDEFS_DATASET_DESCR_CTLS)
            ||((elemIndex+1) >= numberDescrElems))
          {
            
            if(pPoint != TMWDEFS_NULL)
            {
              /* send control to database */
              if(isOperate)
              {
                status = sdnpdata_datasetOperate(pPoint, elem, (TMWTYPES_UCHAR)valueIndex);
              }
              else
              {
                status = sdnpdata_datasetSelect(pPoint, elem, (TMWTYPES_UCHAR)valueIndex);
              }
            }

            if(status != DNPDEFS_DATASET_ST_SUCCESS)
            {
              /* response byte for byte matches request after first 2 bytes */
              pResponse->pMsgBuf[ctrlStatusOffset++] = 1;
              pResponse->pMsgBuf[ctrlStatusOffset] = status;

              sdnputil_updateIINAfterSelOp(pSession, status);

              /* Not required to parse rest of request message */
              pRxFragment->offset = pRxFragment->msgLength;
              return(TMWDEFS_FALSE);
            }
          }
        }
      }
 
      if((elemTypes[elemIndex] == DNPDEFS_DATASET_DESCR_CTLS) 
        && (elem[valueIndex].data.value.uint32Value == 0))
      {
        valueIndex = 0;
        ctrlStatusOffset = msgOffset + 2;

        if(status != DNPDEFS_DATASET_ST_SUCCESS)
        {
          /* set status and IIN bits appropriately */
          pResponse->pMsgBuf[ctrlStatusOffset++] = 1;
          pResponse->pMsgBuf[ctrlStatusOffset] = status;

          sdnputil_updateIINAfterSelOp(pSession, status);

          /* Not required to parse rest of request message */
          pRxFragment->offset = pRxFragment->msgLength;
          return(TMWDEFS_FALSE);
        }
      }

      elemIndex++;
    }
  }
  
  return(retStatus); 
} 
  
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo087_selObj087v1(  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWTYPES_UCHAR numberOfPoints,
  TMWTYPES_UCHAR status)
{
  TMWTARG_UNUSED_PARAM(numberOfPoints);

  /* Validate qualifier */
  if (pObjHeader->qualifier != DNPDEFS_QUAL_16BIT_FREE_FORMAT)
  {
    /* Don't try to process rest of message */
    pRxFragment->offset = pRxFragment->msgLength;
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_BAD_QUAL);
    return(TMWDEFS_FALSE);
  }

  return(_selopObj087v1(pSession, pRxFragment, pResponse, (TMWTYPES_UCHAR)pObjHeader->numberOfPoints, status, TMWDEFS_FALSE));
}

TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo087_opObj087v1(  
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWTYPES_UCHAR numberOfPoints,
  TMWTYPES_UCHAR status)
{
  TMWTARG_UNUSED_PARAM(numberOfPoints);

  /* Validate qualifier */
  if (pObjHeader->qualifier != DNPDEFS_QUAL_16BIT_FREE_FORMAT)
  {
    /* Don't try to process rest of message */
    pRxFragment->offset = pRxFragment->msgLength;
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_BAD_QUAL);
    return(TMWDEFS_FALSE);
  }

  return(_selopObj087v1(pSession, pRxFragment, pResponse, (TMWTYPES_UCHAR)pObjHeader->numberOfPoints, status, TMWDEFS_TRUE));
}

#if SDNPDATA_SUPPORT_SELECT_CANCEL
void TMWDEFS_CALLBACK sdnpo087_cancelSelect(  
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_USHORT index;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  /* Loop through the points in the request */
  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  { 
    void *pPoint;  
    TMWTYPES_ULONG  datasetId;
    TMWTYPES_ULONG nextPointOffset;
    TMWTYPES_USHORT sizePrefix;
    DNPDATA_DATASET_CTRL_VALUE elem[1]; 

    /* Get sizePrefix, 
     * this message was verified for correct length when the operate was parsed. 
     */
    tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &sizePrefix);
    pRxFragment->offset += 2;

    nextPointOffset = pRxFragment->offset + sizePrefix;

    /* Get Data Set Id from message */
    if(!dnputil_getValueFromMessage(pRxFragment->pMsgBuf, &pRxFragment->offset, nextPointOffset, DNPDEFS_DATASET_TYPE_INT, &elem[0].data))
    {
      pRxFragment->offset = pRxFragment->msgLength; 
      return;
    }
    datasetId = elem[0].data.value.uint32Value;

    /* Skip Time */ 
    pRxFragment->offset+=6;

    pPoint = sdnpdata_datasetCreatePoint(pSDNPSession->pDbHandle, (TMWTYPES_USHORT)datasetId, SDNPDATA_DATASET_MODE_SELECT, TMWDEFS_NULL);
    if(pPoint != TMWDEFS_NULL)
    { 
       sdnpdata_datasetCancelSelect(pPoint);
    }
    pRxFragment->offset = nextPointOffset; 
  }
} 
#endif
#endif /* SDNPDATA_SUPPORT_OBJ087 */
