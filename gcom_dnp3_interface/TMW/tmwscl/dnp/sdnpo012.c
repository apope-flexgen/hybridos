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

/* file: sdnpo012.c
 * description: DNP Slave functionality for Object 12 Binary Outputs
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/sdnpdiag.h"
#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/sdnpo012.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnputil.h"

#include "tmwscl/utils/tmwtarg.h"

#if SDNPDATA_SUPPORT_OBJ12
#if SDNPDATA_SUPPORT_OBJ12_V1
/* function: sdnpo012_selObj12v1 */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo012_selObj12v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWTYPES_UCHAR numberOfPoints,
  TMWTYPES_UCHAR status)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_BOOL retStatus = TMWDEFS_TRUE;
  TMWTYPES_USHORT index;

  /* Validate qualifier */
  if(!sdnputil_validateSelOpQualifier(pSession, pResponse, pObjHeader))    
  {
    /* Invalid qualifier, set offset to end to stop parsing request */
    pRxFragment->offset = pRxFragment->msgLength;
    return(TMWDEFS_FALSE);
  }

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    TMWTYPES_UCHAR localStatus = status;
    TMWTYPES_UCHAR controlCode;
    TMWTYPES_UCHAR rxStatus;
    TMWTYPES_UCHAR length;
    TMWTYPES_ULONG offTime;
    TMWTYPES_ULONG onTime;
    TMWTYPES_UCHAR count;
    TMWTYPES_USHORT point;
    void *pPoint;

    if(pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      point = pRxFragment->pMsgBuf[pRxFragment->offset++];
      pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)point;
      length = 12;
    }
    else
    {
      tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &point);
      pRxFragment->offset += 2;

      tmwtarg_store16(&point, &pResponse->pMsgBuf[pResponse->msgLength]);
      pResponse->msgLength += 2;
      length = 13;
    }

    /* Control Code */
    controlCode = pRxFragment->pMsgBuf[pRxFragment->offset++];
    pResponse->pMsgBuf[pResponse->msgLength++] = controlCode;

    /* Count */
    count = pRxFragment->pMsgBuf[pRxFragment->offset++];
    pResponse->pMsgBuf[pResponse->msgLength++] = count;

    /* On Time */
    tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], &onTime);
    tmwtarg_store32(&onTime, &pResponse->pMsgBuf[pResponse->msgLength]);
    pResponse->msgLength += 4;
    pRxFragment->offset += 4;

    /* Off Time */
    tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], &offTime);
    tmwtarg_store32(&offTime, &pResponse->pMsgBuf[pResponse->msgLength]);
    pResponse->msgLength += 4;
    pRxFragment->offset += 4;

    /* Read Status (should always be 0) */
    rxStatus = pRxFragment->pMsgBuf[pRxFragment->offset++];
    if(rxStatus != 0)
    {
      /* Log error and don't execute */
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_CROB_STATUS);

      pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_CROB_ST_FORMAT_ERROR;
      pSDNPSession->iin |= DNPDEFS_IIN_PARAM_ERROR;
      pRxFragment->offset = pRxFragment->msgLength;
      return(TMWDEFS_FALSE);
    }

    /* Diagnostics */
    DNPDIAG_SHOW_CROB(pSession, point, controlCode, count, onTime, offTime, 0, TMWDIAG_ID_RX);
   
    /* Make sure quantity does not exceed max allowed */
    if(localStatus == DNPDEFS_CROB_ST_SUCCESS)
    {
      if(((!pSDNPSession->allowMultiCROBRequests) && ((numberOfPoints + index) > 0))
        || ((numberOfPoints + index) >= pSDNPSession->maxControlRequests))
      {
        localStatus = DNPDEFS_CROB_ST_TOO_MANY_OPS;
      }
    }

    /* Make sure point number is valid */
    if(localStatus == DNPDEFS_CROB_ST_SUCCESS)
    {
      if(point >= sdnpdata_binOutQuantity(pSDNPSession->pDbHandle))
        localStatus = DNPDEFS_CROB_ST_NOT_SUPPORTED;
    }

    /* Get Point */
    if(localStatus == DNPDEFS_CROB_ST_SUCCESS)
    {
      pPoint = sdnpdata_binOutGetPoint(pSDNPSession->pDbHandle, point);
      if(pPoint != TMWDEFS_NULL)
      {
        /* Select Point */ 
        localStatus = sdnpdata_binOutSelect(pPoint, controlCode, count, onTime, offTime);
      }
      else
      {
        localStatus = DNPDEFS_CROB_ST_NOT_SUPPORTED;
      }
    }

    if (localStatus != DNPDEFS_CTLSTAT_SUCCESS)
    {
#ifdef TMW_SUPPORT_MONITOR
      /* In monitor mode, don't show a response. */
      if (pSession->pChannel->pPhysContext->monitorMode)
      {
        return(TMWDEFS_TRUE);
      }
#endif
      DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, DNPDEFS_OBJ_12_BIN_OUT_CTRLS, 1, pObjHeader->qualifier);
      DNPDIAG_SHOW_CROB(pSession, point, controlCode, count, onTime, offTime, localStatus, 0);
    }

    /* Store status into response */
    pResponse->pMsgBuf[pResponse->msgLength++] = localStatus;
  
    sdnputil_updateIINAfterSelOp(pSession, localStatus);

    if(localStatus != DNPDEFS_CROB_ST_SUCCESS)
      retStatus = TMWDEFS_FALSE;

    /* Make sure there is enough room for the response */
    if((pResponse->msgLength + length) > pResponse->maxLength)
    {
      pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
      pRxFragment->offset = pRxFragment->msgLength;
      return(TMWDEFS_FALSE);
    }
  }

  return(retStatus);
}

/* function: sdnpo012_opObj12v1 */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo012_opObj12v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWTYPES_UCHAR numberOfPoints,
  TMWTYPES_UCHAR status)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_BOOL retStatus = TMWDEFS_TRUE;
  TMWTYPES_USHORT index;

  /* Validate qualifier */       
  if(!sdnputil_validateSelOpQualifier(pSession, pResponse, pObjHeader))
  {
    /* Invalid qualifier, set offset to end to stop parsing request */
    pRxFragment->offset = pRxFragment->msgLength;
    return(TMWDEFS_FALSE);
  }

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    TMWTYPES_UCHAR localStatus = status;
    TMWTYPES_UCHAR controlCode;
    TMWTYPES_UCHAR rxStatus;
    TMWTYPES_UCHAR length;
    TMWTYPES_ULONG offTime;
    TMWTYPES_ULONG onTime;
    TMWTYPES_UCHAR count;
    TMWTYPES_USHORT point;
    void *pPoint;

    if(pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      point = pRxFragment->pMsgBuf[pRxFragment->offset++];
      pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)point;
      length = 12;
    }
    else
    {
      tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &point);
      pRxFragment->offset += 2;

      tmwtarg_store16(&point, &pResponse->pMsgBuf[pResponse->msgLength]);
      pResponse->msgLength += 2;
      length = 13;
    }

    /* Control Code */
    controlCode = pRxFragment->pMsgBuf[pRxFragment->offset++];
    pResponse->pMsgBuf[pResponse->msgLength++] = controlCode;

    /* Count */
    count = pRxFragment->pMsgBuf[pRxFragment->offset++];
    pResponse->pMsgBuf[pResponse->msgLength++] = count;

    /* On Time */
    tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], &onTime);
    tmwtarg_store32(&onTime, &pResponse->pMsgBuf[pResponse->msgLength]);
    pResponse->msgLength += 4;
    pRxFragment->offset += 4;

    /* Off Time */
    tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], &offTime);
    tmwtarg_store32(&offTime, &pResponse->pMsgBuf[pResponse->msgLength]);
    pResponse->msgLength += 4;
    pRxFragment->offset += 4;

    /* Read Status (should always be 0) */
    rxStatus = pRxFragment->pMsgBuf[pRxFragment->offset++];
    if(rxStatus != 0)
    {
      /* Log error and don't execute */
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_CROB_STATUS); 

      pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_CROB_ST_FORMAT_ERROR;
      pSDNPSession->iin |= DNPDEFS_IIN_PARAM_ERROR;
      pRxFragment->offset = pRxFragment->msgLength;
      return(TMWDEFS_FALSE);
    }

    /* Diagnostics */
    DNPDIAG_SHOW_CROB(pSession, point, controlCode, count, onTime, offTime, 0, TMWDIAG_ID_RX);

    /* Make sure quantity does not exceed max allowed */
    if(localStatus == DNPDEFS_CROB_ST_SUCCESS)
    {
      if(((!pSDNPSession->allowMultiCROBRequests) && ((numberOfPoints + index) > 0))
        || ((numberOfPoints + index) >= pSDNPSession->maxControlRequests))
      {
        localStatus = DNPDEFS_CROB_ST_TOO_MANY_OPS;
      }
    }

    /* Make sure point number is valid */
    if(localStatus == DNPDEFS_CROB_ST_SUCCESS)
    {
      if(point >= sdnpdata_binOutQuantity(pSDNPSession->pDbHandle))
        localStatus = DNPDEFS_CROB_ST_NOT_SUPPORTED;
    }

    /* Get Point */
    if(localStatus == DNPDEFS_CROB_ST_SUCCESS)
    {
      pPoint = sdnpdata_binOutGetPoint(pSDNPSession->pDbHandle, point);
      if(pPoint != TMWDEFS_NULL)
      { 
        /* Operate Point */
        localStatus = sdnpdata_binOutOperate(pPoint, controlCode, count, onTime, offTime);
      }
      else
      {
        localStatus = DNPDEFS_CROB_ST_NOT_SUPPORTED;
      }
    }

    if (localStatus != DNPDEFS_CTLSTAT_SUCCESS)
    {
#ifdef TMW_SUPPORT_MONITOR
      /* In monitor mode, don't show a response. */
      if (pSession->pChannel->pPhysContext->monitorMode)
      {
        return(TMWDEFS_TRUE);
      }
#endif
      DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, DNPDEFS_OBJ_12_BIN_OUT_CTRLS, 1, pObjHeader->qualifier);
      DNPDIAG_SHOW_CROB(pSession, point, controlCode, count, onTime, offTime, localStatus, 0);
    }

    /* Store status into response */
    pResponse->pMsgBuf[pResponse->msgLength++] = localStatus;

    /* Update IIN Bits */
    sdnputil_updateIINAfterSelOp(pSession, localStatus);

    if(localStatus != DNPDEFS_CROB_ST_SUCCESS)
      retStatus = TMWDEFS_FALSE;

    /* Make sure there is enough room for the response */
    if((pResponse->msgLength + length) > pResponse->maxLength)
    {
      pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
      pRxFragment->offset = pRxFragment->msgLength;
      return(TMWDEFS_FALSE);
    }
  }

  return(retStatus);
}

#if SDNPDATA_SUPPORT_SELECT_CANCEL
/* function: sdnpo012_cancelSelectV1 */
void TMWDEFS_CALLBACK sdnpo012_cancelSelectV1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    TMWTYPES_USHORT point;
    void *pPoint;

    if(pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      point = pRxFragment->pMsgBuf[pRxFragment->offset++];
    }
    else
    {
      tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &point);
      pRxFragment->offset += 2;
    }

    /* Skip the following fields */
    /* Control Code 1 */ 
    /* Count 1 */ 
    /* On Time 4 */
    /* Off Time 4 */
    /* Read Status 1 */
    pRxFragment->offset += 11;
    
    /* Get Point */
    pPoint = sdnpdata_binOutGetPoint(pSDNPSession->pDbHandle, point);
    if(pPoint != TMWDEFS_NULL)
    { 
      /* Cancel Select for this Point */
      sdnpdata_binOutCancelSelect(pPoint);
    }
  }
}
#endif

#endif /* SDNPDATA_SUPPORT_OBJ12_V1 */

#if SDNPDATA_SUPPORT_OBJ12_V2
/* function: _parsePCB */
static TMWTYPES_BOOL _parsePCB(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWTYPES_UCHAR *pStatus)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_BOOL sendError = TMWDEFS_FALSE;
  TMWTYPES_UCHAR pcbStat;

  TMWTARG_UNUSED_PARAM(pStatus);

  /* Validate qualifier */
  if(pObjHeader->qualifier != DNPDEFS_QUAL_8BIT_LIMITED_QTY)
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_INV_QUAL_PATTERN);
    sendError = TMWDEFS_TRUE; 
  }

  /* Validate quantity */
  if(pObjHeader->numberOfPoints != 1)
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_PATTERN_QUAN);
    sendError = TMWDEFS_TRUE; 
  }

  /* 11 octets required for this object */
  if(pRxFragment->offset + 11 > pRxFragment->msgLength)
  {
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE); 
    sendError = TMWDEFS_TRUE; 
  }
  
  /* Make sure there is enough room for the response */
  if((pResponse->msgLength + 15) > pResponse->maxLength)
  {
    sendError = TMWDEFS_TRUE; 
  }

  if(sendError)
  {
    /* set offset to end to stop parsing request */
    pRxFragment->offset = pRxFragment->msgLength;
    pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
    return(TMWDEFS_FALSE);
  }

  /* Read control */
  pSDNPSession->pcbData.control = pRxFragment->pMsgBuf[pRxFragment->offset++];

  /* Read count */
  pSDNPSession->pcbData.count = pRxFragment->pMsgBuf[pRxFragment->offset++];

  /* Read activation period */
  tmwtarg_get32(pRxFragment->pMsgBuf + pRxFragment->offset, &pSDNPSession->pcbData.activation);
  pRxFragment->offset += 4;

  /* Read deactivation period */
  tmwtarg_get32(pRxFragment->pMsgBuf + pRxFragment->offset, &pSDNPSession->pcbData.deactivation);
  pRxFragment->offset += 4;

  /* Read status */
  pcbStat = pRxFragment->pMsgBuf[pRxFragment->offset++];

  /* Fill in response if required */
  pResponse->pMsgBuf[pResponse->msgLength++] = pObjHeader->group;
  pResponse->pMsgBuf[pResponse->msgLength++] = pObjHeader->variation;
  pResponse->pMsgBuf[pResponse->msgLength++] = pObjHeader->qualifier;
  pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)pObjHeader->numberOfPoints;

  pResponse->pMsgBuf[pResponse->msgLength++] = pSDNPSession->pcbData.control;
  pResponse->pMsgBuf[pResponse->msgLength++] = pSDNPSession->pcbData.count;

  tmwtarg_store32(&pSDNPSession->pcbData.activation, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 4;

  tmwtarg_store32(&pSDNPSession->pcbData.deactivation, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 4;

  pSDNPSession->pcbData.pcbStatusOffset = pResponse->msgLength;
  
  if(pcbStat != 0)
  { 
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_PATTERN_STATUS);
    pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_CROB_ST_FORMAT_ERROR;
    pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
    return(TMWDEFS_FALSE);
  }
  else
  {
    pResponse->pMsgBuf[pResponse->msgLength++] = pcbStat;
  }

  return(TMWDEFS_TRUE);
}

/* function: sdnpo012_initObj12v2 */
void TMWDEFS_CALLBACK sdnpo012_initObj12v2(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  pSDNPSession->pcbData.pcbStatusOffset = 0;
  pSDNPSession->pcbData.control = 0;
  pSDNPSession->pcbData.count = 0;
  pSDNPSession->pcbData.activation = 0;
  pSDNPSession->pcbData.deactivation = 0;
}

/* function: sdnpo012_selObj12v2 */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo012_selObj12v2(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWTYPES_UCHAR numberOfPoints,
  TMWTYPES_UCHAR status)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTARG_UNUSED_PARAM(numberOfPoints);

  if(!_parsePCB(pSession, pRxFragment, pResponse, pObjHeader, &status))
  {
    return(TMWDEFS_FALSE);
  }

  /* If status is not 0, update status in response */
  if(status != DNPDEFS_CROB_ST_SUCCESS)
  { 
    sdnputil_updateIINAfterSelOp(pSession, status);

    pResponse->pMsgBuf[pSDNPSession->pcbData.pcbStatusOffset] = status;
    return(TMWDEFS_FALSE);
  }

  return(TMWDEFS_TRUE);
}

/* function: sdnpo012_opObj12v2 */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo012_opObj12v2(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWTYPES_UCHAR numberOfPoints,
  TMWTYPES_UCHAR status)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTARG_UNUSED_PARAM(numberOfPoints);

  if(!_parsePCB(pSession, pRxFragment, pResponse, pObjHeader, &status))
  {
    return(TMWDEFS_FALSE);
  }

  /* If status is not 0, update status in response */
  if(status != DNPDEFS_CROB_ST_SUCCESS)
  {
    sdnputil_updateIINAfterSelOp(pSession, status);
    pResponse->pMsgBuf[pSDNPSession->pcbData.pcbStatusOffset] = status;
    return(TMWDEFS_FALSE);
  }

  return(TMWDEFS_TRUE);
}

#if SDNPDATA_SUPPORT_SELECT_CANCEL
/* function: sdnpo012_cancelSelectV2 */
void TMWDEFS_CALLBACK sdnpo012_cancelSelectV2(
  DNPUTIL_RX_MSG *pRxFragment)
{ 
  /* Skip the following fields */
  /* Control Code 1 */ 
  /* Count 1 */ 
  /* On Time 4 */
  /* Off Time 4 */
  /* Read Status 1 */
  pRxFragment->offset += 11; 
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ12_V2 */

#if SDNPDATA_SUPPORT_OBJ12_V3
/* function: _parsePatternMask */
/* This should return TMWDEFS_FALSE if qualifier was bad and
 * message cannot be parsed 
 * It should return TMWDEFS_TRUE even if the status was not success
 */
static TMWTYPES_BOOL _parsePatternMask(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWTYPES_UCHAR **ppMask,
  TMWTYPES_UCHAR *pStatus)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_USHORT numPoints;
  TMWTYPES_USHORT maskSize;
  TMWTYPES_USHORT lengthNeeded;

  /* Validate qualifier */
  if((pObjHeader->qualifier != DNPDEFS_QUAL_8BIT_START_STOP)
    && (pObjHeader->qualifier != DNPDEFS_QUAL_16BIT_START_STOP))
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_PATTERN_8_16);
    
    /* set offset to end to stop parsing request */
    pRxFragment->offset = pRxFragment->msgLength;
    pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
    return(TMWDEFS_FALSE);
  }

  /* checking firstPointNumber > lastPointNumber was done in dnputil_parseObjectHeader */

  if(pObjHeader->lastPointNumber >= sdnpdata_binOutQuantity(pSDNPSession->pDbHandle))
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_INDICES_PATTERN);
    *pStatus = DNPDEFS_CROB_ST_NOT_SUPPORTED;
    /* Don't return, so that we can build response with status codes */
  }

  /* Figure out how many bytes should be in mask */
  numPoints = (TMWTYPES_USHORT)(pObjHeader->lastPointNumber - pObjHeader->firstPointNumber + 1);
  maskSize = (TMWTYPES_USHORT)(numPoints / 8);
  if(numPoints%8)
    maskSize = (TMWTYPES_USHORT)(maskSize + 1);

  /* Make sure mask size is correct, and to be safe
   * make sure there is enough room for the response
   */
  if(pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_START_STOP)
  {
    lengthNeeded = 5;
  }
  else
  {
    lengthNeeded = 7;
  }
  if(((pRxFragment->offset + maskSize) > pRxFragment->msgLength)
    || ((pResponse->msgLength + lengthNeeded + maskSize) > pResponse->maxLength))
  {
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE); 

    /* set offset to end to stop parsing request */
    pRxFragment->offset = pRxFragment->msgLength;
    pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
    return(TMWDEFS_FALSE);
  }

  /* Get pointer to mask and skip over in receive fragment */
  *ppMask = pRxFragment->pMsgBuf + pRxFragment->offset;
  pRxFragment->offset = pRxFragment->offset + maskSize;

  /* Fill in response if required */
  pResponse->pMsgBuf[pResponse->msgLength++] = pObjHeader->group;
  pResponse->pMsgBuf[pResponse->msgLength++] = pObjHeader->variation;
  pResponse->pMsgBuf[pResponse->msgLength++] = pObjHeader->qualifier;
  if(pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_START_STOP)
  {
    pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)pObjHeader->firstPointNumber;
    pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)pObjHeader->lastPointNumber;
  }
  else
  {
    tmwtarg_store16(&pObjHeader->firstPointNumber, pResponse->pMsgBuf + pResponse->msgLength);
    pResponse->msgLength += 2;

    tmwtarg_store16(&pObjHeader->lastPointNumber, pResponse->pMsgBuf + pResponse->msgLength);
    pResponse->msgLength += 2;
  }

  memcpy(pResponse->pMsgBuf + pResponse->msgLength, *ppMask, maskSize);
  pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + maskSize);

  return(TMWDEFS_TRUE);
}

/* function: sdnpo012_selObj12v3 */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo012_selObj12v3(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWTYPES_UCHAR numberOfPoints,
  TMWTYPES_UCHAR status)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  PCB_DATA *pPCBData = &pSDNPSession->pcbData;
  TMWTYPES_UCHAR *pMask;
  TMWTARG_UNUSED_PARAM(numberOfPoints);

  if(!_parsePatternMask(pSession, pRxFragment, pResponse, pObjHeader, &pMask, &status))
  {
    return(TMWDEFS_FALSE);
  }

  /* Try to select pattern mask */
  if(status == DNPDEFS_CROB_ST_SUCCESS)
  {
    status = sdnpdata_binOutSelPatternMask(pSDNPSession->pDbHandle, 
      pPCBData->control, pPCBData->count, pPCBData->activation, pPCBData->deactivation, 
      pObjHeader->firstPointNumber, pObjHeader->lastPointNumber, pMask);
  }

  /* If status is not success, update status in response */
  if(status != DNPDEFS_CROB_ST_SUCCESS)
  {
    sdnputil_updateIINAfterSelOp(pSession, status);
    pResponse->pMsgBuf[pSDNPSession->pcbData.pcbStatusOffset] = status;
    return(TMWDEFS_FALSE);
  }
  return(TMWDEFS_TRUE);
}

/* function: sdnpo012_opObj12v3 */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo012_opObj12v3(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWTYPES_UCHAR numberOfPoints,
  TMWTYPES_UCHAR status)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  PCB_DATA *pPCBData = &pSDNPSession->pcbData;
  TMWTYPES_UCHAR *pMask;
  TMWTARG_UNUSED_PARAM(numberOfPoints);

  if(!_parsePatternMask(pSession, pRxFragment, pResponse, pObjHeader, &pMask, &status))
  {
    return(TMWDEFS_FALSE);
  }

  /* If status is good so far, try to operate pattern mask */
  if(status == DNPDEFS_CROB_ST_SUCCESS)
  {
    status = sdnpdata_binOutOpPatternMask(pSDNPSession->pDbHandle, 
      pPCBData->control, pPCBData->count, pPCBData->activation, pPCBData->deactivation, 
      pObjHeader->firstPointNumber, pObjHeader->lastPointNumber, pMask);
  }

  /* If status is not success, update status in response */
  if(status != DNPDEFS_CROB_ST_SUCCESS)
  {
    sdnputil_updateIINAfterSelOp(pSession, status);
    pResponse->pMsgBuf[pSDNPSession->pcbData.pcbStatusOffset] = status;
    return(TMWDEFS_FALSE);
  }

  return(TMWDEFS_TRUE);
}

#if SDNPDATA_SUPPORT_SELECT_CANCEL
/* function: sdnpo012_cancelSelectV3 */
void TMWDEFS_CALLBACK sdnpo012_cancelSelectV3(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_USHORT numPoints;
  TMWTYPES_USHORT maskSize;
  TMWTYPES_UCHAR *pMask;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  /* Figure out how many bytes in mask */
  numPoints = (TMWTYPES_UCHAR)(pObjHeader->lastPointNumber - pObjHeader->firstPointNumber + 1);
  maskSize = (TMWTYPES_UCHAR)(numPoints / 8);
  if(numPoints%8)
    maskSize = (TMWTYPES_USHORT)(maskSize + 1);

  /* Get pointer to mask and skip over in receive fragment */
  pMask = pRxFragment->pMsgBuf + pRxFragment->offset;
  pRxFragment->offset = pRxFragment->offset + maskSize;

  sdnpdata_binOutCancelPatMask(pSDNPSession->pDbHandle, 
    pObjHeader->firstPointNumber, pObjHeader->lastPointNumber, pMask); 
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ12_V3 */

#if SDNPDATA_SUPPORT_SELECT_CANCEL
void TMWDEFS_CALLBACK sdnpo012_cancelSelect(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  if(pObjHeader->variation == 1)
  {
#if SDNPDATA_SUPPORT_OBJ12_V1
    sdnpo012_cancelSelectV1(pSession, pRxFragment, pObjHeader);
#endif
  }
  else if(pObjHeader->variation == 2)
  {
#if SDNPDATA_SUPPORT_OBJ12_V2
    sdnpo012_cancelSelectV2(pRxFragment);
#endif
  }
  else
  {
#if SDNPDATA_SUPPORT_OBJ12_V3
    sdnpo012_cancelSelectV3(pSession, pRxFragment, pObjHeader);
#endif
  }
}
#endif

#if SDNPDATA_SUPPORT_OBJ13
#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpo012_assignClass */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo012_assignClass(
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
  status = sdnputil_validateAssignQualifier(pObjHeader, sdnpdata_binOutQuantity(pSDNPSession->pDbHandle));
  
  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    void *pPoint;
    TMWTYPES_USHORT pointNumber;
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    pPoint = sdnpdata_binOutGetPoint(pSDNPSession->pDbHandle, pointNumber);
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

    if(!sdnpdata_binOutCmdAssignClass(pPoint, classMask))
      status = TMWDEFS_FALSE;
  }

  return(status);
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ11 */

#endif /* SDNPDATA_SUPPORT_OBJ12 */
