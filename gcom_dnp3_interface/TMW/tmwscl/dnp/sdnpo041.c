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

/* file: sdnpo041.c
 * description: DNP Slave functionality for Object 41 Analog Outputs
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/sdnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"

#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/sdnpo041.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnputil.h"

#if SDNPDATA_SUPPORT_OBJ41


#if SDNPDATA_SUPPORT_OBJ41_V1
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo041_selObj41v1(
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
    void *pPoint;
    TMWTYPES_LONG value;
    TMWTYPES_USHORT point;
    TMWTYPES_UCHAR rxStatus;
    TMWTYPES_UCHAR length;
    TMWTYPES_UCHAR localStatus = status;
    TMWTYPES_ANALOG_VALUE anlgValue;

    if(pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      point = pRxFragment->pMsgBuf[pRxFragment->offset++];
      pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)point;
      length = 6;
    }
    else
    {
      tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &point);
      pRxFragment->offset += 2;

      tmwtarg_store16(&point, &pResponse->pMsgBuf[pResponse->msgLength]);
      pResponse->msgLength += 2;
      length = 7;
    }

    /* Value */
    tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], (TMWTYPES_ULONG *)&value);
    tmwtarg_store32((TMWTYPES_ULONG *)&value, &pResponse->pMsgBuf[pResponse->msgLength]);
    pResponse->msgLength += 4;
    pRxFragment->offset += 4;

    /* Read Status (should always be 0) */
    rxStatus = pRxFragment->pMsgBuf[pRxFragment->offset++];
    if(rxStatus != 0)
    {
      /* Log error and don't execute */
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ANLG_STATUS); 

      pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_CROB_ST_FORMAT_ERROR;
      pSDNPSession->iin |= DNPDEFS_IIN_PARAM_ERROR;
      pRxFragment->offset = pRxFragment->msgLength;
      return(TMWDEFS_FALSE);
    }

    /* Make sure quantity does not exceed max allowed */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      if((numberOfPoints + index) >= pSDNPSession->maxControlRequests)
      {
        localStatus = DNPDEFS_CTLSTAT_TOO_MANY_OPS;
      }
    }

    /* Make sure point number is valid */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      if(point >= sdnpdata_anlgOutQuantity(pSDNPSession->pDbHandle))
        localStatus = DNPDEFS_CTLSTAT_NOT_SUPPORTED;
    }

    anlgValue.type = TMWTYPES_ANALOG_TYPE_LONG;
    anlgValue.value.lval = value;

    /* Diagnostics */
    DNPDIAG_SHOW_ANALOG_CONTROL(pSession, point, &anlgValue, 0, TMWDEFS_FALSE, TMWDIAG_ID_RX, TMWDEFS_NULL);

    /* Get Point */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      pPoint = sdnpdata_anlgOutGetPoint(pSDNPSession->pDbHandle, point);
      if (pPoint != TMWDEFS_NULL)
      {
        /* Select Point */
        localStatus = sdnpdata_anlgOutSelect(pPoint, &anlgValue);
      }
      else
      {
        localStatus = DNPDEFS_CTLSTAT_NOT_SUPPORTED;
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
      DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, DNPDEFS_OBJ_41_ANA_OUT_CTRLS, 1, pObjHeader->qualifier);
      DNPDIAG_SHOW_ANALOG_CONTROL(pSession, point, &anlgValue, localStatus, TMWDEFS_FALSE, 0, TMWDEFS_NULL);
    }

    /* Store status into response */
    pResponse->pMsgBuf[pResponse->msgLength++] = localStatus;

    sdnputil_updateIINAfterSelOp(pSession, localStatus);

    if(localStatus != DNPDEFS_CTLSTAT_SUCCESS)
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

TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo041_opObj41v1(  
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
    void *pPoint;
    TMWTYPES_LONG value;
    TMWTYPES_USHORT point;
    TMWTYPES_UCHAR rxStatus;
    TMWTYPES_UCHAR length;
    TMWTYPES_UCHAR localStatus = status;
    TMWTYPES_ANALOG_VALUE anlgValue;

    if(pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      point = pRxFragment->pMsgBuf[pRxFragment->offset++];
      pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)point;
      length = 6;
    }
    else
    {
      tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &point);
      pRxFragment->offset += 2;

      tmwtarg_store16(&point, &pResponse->pMsgBuf[pResponse->msgLength]);
      pResponse->msgLength += 2;

      length = 7;
    }

    /* Value */
    tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], (TMWTYPES_ULONG *)&value);
    tmwtarg_store32((TMWTYPES_ULONG *)&value, &pResponse->pMsgBuf[pResponse->msgLength]);
    pResponse->msgLength += 4;
    pRxFragment->offset += 4;

    /* Read Status (should always be 0) */
    rxStatus = pRxFragment->pMsgBuf[pRxFragment->offset++];
    if(rxStatus != 0)
    {
      /* Log error and don't execute */
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ANLG_STATUS); 

      pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_CROB_ST_FORMAT_ERROR;
      pSDNPSession->iin |= DNPDEFS_IIN_PARAM_ERROR;
      pRxFragment->offset = pRxFragment->msgLength;
      return(TMWDEFS_FALSE);
    }

    /* Make sure quantity does not exceed max allowed */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      if((numberOfPoints + index) >= pSDNPSession->maxControlRequests)
      {
        localStatus = DNPDEFS_CTLSTAT_TOO_MANY_OPS;
      }
    }

    /* Make sure point number is valid */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      if(point >= sdnpdata_anlgOutQuantity(pSDNPSession->pDbHandle))
        localStatus = DNPDEFS_CTLSTAT_NOT_SUPPORTED;
    }
    
    anlgValue.type = TMWTYPES_ANALOG_TYPE_LONG;
    anlgValue.value.lval = value;

    /* Diagnostics */
    DNPDIAG_SHOW_ANALOG_CONTROL(pSession, point, &anlgValue, 0, TMWDEFS_FALSE, TMWDIAG_ID_RX, TMWDEFS_NULL);

    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      /* Get Point */
      pPoint = sdnpdata_anlgOutGetPoint(pSDNPSession->pDbHandle, point);
      if (pPoint != TMWDEFS_NULL)
      {
        /* Operate Point */
        localStatus = sdnpdata_anlgOutOperate(pPoint, &anlgValue);
      }
      else
      {
        localStatus = DNPDEFS_CTLSTAT_NOT_SUPPORTED;
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
      DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, DNPDEFS_OBJ_41_ANA_OUT_CTRLS, 1, pObjHeader->qualifier);
      DNPDIAG_SHOW_ANALOG_CONTROL(pSession, point, &anlgValue, localStatus, TMWDEFS_FALSE, 0, TMWDEFS_NULL);
    }

    /* Store status into response */
    pResponse->pMsgBuf[pResponse->msgLength++] = localStatus;

    sdnputil_updateIINAfterSelOp(pSession, localStatus);

    if(localStatus != DNPDEFS_CTLSTAT_SUCCESS)
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
#endif /* SDNPDATA_SUPPORT_OBJ41_V1 */

#if SDNPDATA_SUPPORT_OBJ41_V2
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo041_selObj41v2(  
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
    void *pPoint;
    TMWTYPES_SHORT value;
    TMWTYPES_USHORT point;
    TMWTYPES_UCHAR rxStatus;
    TMWTYPES_UCHAR length;
    TMWTYPES_UCHAR localStatus = status;
    TMWTYPES_ANALOG_VALUE anlgValue;

    if(pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      point = pRxFragment->pMsgBuf[pRxFragment->offset++];
      pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)point;
      length = 4;
    }
    else
    {
      tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &point);
      pRxFragment->offset += 2;

      tmwtarg_store16(&point, &pResponse->pMsgBuf[pResponse->msgLength]);
      pResponse->msgLength += 2;
      length = 5;
    }

    /* Value */
    tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], (TMWTYPES_USHORT *)&value);
    tmwtarg_store16((TMWTYPES_USHORT *)&value, &pResponse->pMsgBuf[pResponse->msgLength]);
    pResponse->msgLength += 2;
    pRxFragment->offset += 2;

    /* Read Status (should always be 0) */
    rxStatus = pRxFragment->pMsgBuf[pRxFragment->offset++];
    if(rxStatus != 0)
    {
      /* Log error and don't execute */
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ANLG_STATUS); 

      pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_CROB_ST_FORMAT_ERROR;
      pSDNPSession->iin |= DNPDEFS_IIN_PARAM_ERROR;
      pRxFragment->offset = pRxFragment->msgLength;
      return(TMWDEFS_FALSE);
    }

    /* Make sure quantity does not exceed max allowed */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      if((numberOfPoints + index) >= pSDNPSession->maxControlRequests)
      {
        localStatus = DNPDEFS_CTLSTAT_TOO_MANY_OPS;
      }
    }

    /* Make sure point number is valid */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      if(point >= sdnpdata_anlgOutQuantity(pSDNPSession->pDbHandle))
        localStatus = DNPDEFS_CTLSTAT_NOT_SUPPORTED;
    }

    anlgValue.type = TMWTYPES_ANALOG_TYPE_SHORT;
    anlgValue.value.sval = value;

    /* Diagnostics */
    DNPDIAG_SHOW_ANALOG_CONTROL(pSession, point, &anlgValue, 0, TMWDEFS_FALSE, TMWDIAG_ID_RX, TMWDEFS_NULL);

    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      /* Get Point */
      pPoint = sdnpdata_anlgOutGetPoint(pSDNPSession->pDbHandle, point);
      if (pPoint != TMWDEFS_NULL)
      {
        /* Select Point */
        localStatus = sdnpdata_anlgOutSelect(pPoint, &anlgValue);
      }
      else
      {
        localStatus = DNPDEFS_CTLSTAT_NOT_SUPPORTED;
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
      DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, DNPDEFS_OBJ_41_ANA_OUT_CTRLS, 2, pObjHeader->qualifier);
      DNPDIAG_SHOW_ANALOG_CONTROL(pSession, point, &anlgValue, localStatus, TMWDEFS_FALSE, 0, TMWDEFS_NULL);
    }

    /* Store status into response */
    pResponse->pMsgBuf[pResponse->msgLength++] = localStatus;

    sdnputil_updateIINAfterSelOp(pSession, localStatus);

    if(localStatus != DNPDEFS_CTLSTAT_SUCCESS)
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

TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo041_opObj41v2(
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
    void *pPoint;
    TMWTYPES_SHORT value;
    TMWTYPES_USHORT point;
    TMWTYPES_UCHAR rxStatus;
    TMWTYPES_UCHAR length;
    TMWTYPES_UCHAR localStatus = status;
    TMWTYPES_ANALOG_VALUE anlgValue;

    if(pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      point = pRxFragment->pMsgBuf[pRxFragment->offset++];
      pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)point;
      length = 4;
    }
    else
    {
      tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &point);
      pRxFragment->offset += 2;

      tmwtarg_store16(&point, &pResponse->pMsgBuf[pResponse->msgLength]);
      pResponse->msgLength += 2;
      length = 5;
    }

    /* Value */
    tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], (TMWTYPES_USHORT *)&value);
    tmwtarg_store16((TMWTYPES_USHORT *)&value, &pResponse->pMsgBuf[pResponse->msgLength]);
    pResponse->msgLength += 2;
    pRxFragment->offset += 2;

    /* Read Status (should always be 0) */
    rxStatus = pRxFragment->pMsgBuf[pRxFragment->offset++];
    if(rxStatus != 0)
    {
      /* Log error and don't execute */
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ANLG_STATUS); 

      pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_CROB_ST_FORMAT_ERROR;
      pSDNPSession->iin |= DNPDEFS_IIN_PARAM_ERROR;
      pRxFragment->offset = pRxFragment->msgLength;
      return(TMWDEFS_FALSE);
    }

    /* Make sure quantity does not exceed max allowed */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      if((numberOfPoints + index) >= pSDNPSession->maxControlRequests)
      {
        localStatus = DNPDEFS_CTLSTAT_TOO_MANY_OPS;
      }
    }

    /* Make sure point number is valid */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      if(point >= sdnpdata_anlgOutQuantity(pSDNPSession->pDbHandle))
        localStatus = DNPDEFS_CTLSTAT_NOT_SUPPORTED;
    }

    anlgValue.type = TMWTYPES_ANALOG_TYPE_SHORT;
    anlgValue.value.sval = value;

    /* Diagnostics */
    DNPDIAG_SHOW_ANALOG_CONTROL(pSession, point, &anlgValue, 0, TMWDEFS_FALSE, TMWDIAG_ID_RX, TMWDEFS_NULL);

    /* Get Point */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      pPoint = sdnpdata_anlgOutGetPoint(pSDNPSession->pDbHandle, point);
      if (pPoint != TMWDEFS_NULL)
      {
        /* Operate Point */
        localStatus = sdnpdata_anlgOutOperate(pPoint, &anlgValue);
      }
      else
      {
        localStatus = DNPDEFS_CTLSTAT_NOT_SUPPORTED;
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
      DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, DNPDEFS_OBJ_41_ANA_OUT_CTRLS, 2, pObjHeader->qualifier);
      DNPDIAG_SHOW_ANALOG_CONTROL(pSession, point, &anlgValue, localStatus, TMWDEFS_FALSE, 0, TMWDEFS_NULL);
    }

    /* Store status into response */
    pResponse->pMsgBuf[pResponse->msgLength++] = localStatus;

    sdnputil_updateIINAfterSelOp(pSession, localStatus);

    if(localStatus != DNPDEFS_CTLSTAT_SUCCESS)
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
#endif /* SDNPDATA_SUPPORT_OBJ41_V2 */

#if SDNPDATA_SUPPORT_OBJ41_V3
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo041_selObj41v3(  TMWSESN *pSession,
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
    void *pPoint;
    TMWTYPES_SFLOAT value;
    TMWTYPES_USHORT point;
    TMWTYPES_UCHAR rxStatus;
    TMWTYPES_UCHAR length;
    TMWTYPES_UCHAR localStatus = status;
    TMWTYPES_ANALOG_VALUE anlgValue;

    if(pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      point = pRxFragment->pMsgBuf[pRxFragment->offset++];
      pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)point;
      length = 6;
    }
    else
    {
      tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &point);
      pRxFragment->offset += 2;

      tmwtarg_store16(&point, &pResponse->pMsgBuf[pResponse->msgLength]);
      pResponse->msgLength += 2;
      length = 7;
    }

    /* Value */
    tmwtarg_getSFloat(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    tmwtarg_storeSFloat(&value, &pResponse->pMsgBuf[pResponse->msgLength]);
    pResponse->msgLength += 4;
    pRxFragment->offset += 4;

    /* Read Status (should always be 0) */
    rxStatus = pRxFragment->pMsgBuf[pRxFragment->offset++];
    if(rxStatus != 0)
    {
      /* Log error and don't execute */
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ANLG_STATUS); 

      pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_CROB_ST_FORMAT_ERROR;
      pSDNPSession->iin |= DNPDEFS_IIN_PARAM_ERROR;
      pRxFragment->offset = pRxFragment->msgLength;
      return(TMWDEFS_FALSE);
    }

    /* Make sure quantity does not exceed max allowed */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      if((numberOfPoints + index) >= pSDNPSession->maxControlRequests)
      {
        localStatus = DNPDEFS_CTLSTAT_TOO_MANY_OPS;
      }
    }

    /* Make sure point number is valid */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      if(point >= sdnpdata_anlgOutQuantity(pSDNPSession->pDbHandle))
        localStatus = DNPDEFS_CTLSTAT_NOT_SUPPORTED;
    }

    anlgValue.type = TMWTYPES_ANALOG_TYPE_SFLOAT;
    anlgValue.value.fval = value;

    /* Diagnostics */
    DNPDIAG_SHOW_ANALOG_CONTROL(pSession, point, &anlgValue, 0, TMWDEFS_FALSE, TMWDIAG_ID_RX, TMWDEFS_NULL);

    /* Get Point */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      pPoint = sdnpdata_anlgOutGetPoint(pSDNPSession->pDbHandle, point);
      if (pPoint != TMWDEFS_NULL)
      {
        /* Select Point */
        localStatus = sdnpdata_anlgOutSelect(pPoint, &anlgValue);
      }
      else
      {
        localStatus = DNPDEFS_CTLSTAT_NOT_SUPPORTED;
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
      DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, DNPDEFS_OBJ_41_ANA_OUT_CTRLS, 3, pObjHeader->qualifier);
      DNPDIAG_SHOW_ANALOG_CONTROL(pSession, point, &anlgValue, localStatus, TMWDEFS_FALSE, 0, TMWDEFS_NULL);
    }

    /* Store status into response */
    pResponse->pMsgBuf[pResponse->msgLength++] = localStatus;

    sdnputil_updateIINAfterSelOp(pSession, localStatus);

    if(localStatus != DNPDEFS_CTLSTAT_SUCCESS)
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

TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo041_opObj41v3(  
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
    void *pPoint;
    TMWTYPES_SFLOAT value;
    TMWTYPES_USHORT point;
    TMWTYPES_UCHAR rxStatus;
    TMWTYPES_UCHAR length;
    TMWTYPES_UCHAR localStatus = status;
    TMWTYPES_ANALOG_VALUE anlgValue;

    if(pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      point = pRxFragment->pMsgBuf[pRxFragment->offset++];
      pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)point;
      length = 6;
    }
    else
    {
      tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &point);
      pRxFragment->offset += 2;

      tmwtarg_store16(&point, &pResponse->pMsgBuf[pResponse->msgLength]);
      pResponse->msgLength += 2;
      length = 7;
    }

    /* Value */
    tmwtarg_getSFloat(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    tmwtarg_storeSFloat(&value, &pResponse->pMsgBuf[pResponse->msgLength]);
    pResponse->msgLength += 4;
    pRxFragment->offset += 4;

    /* Read Status (should always be 0) */
    rxStatus = pRxFragment->pMsgBuf[pRxFragment->offset++];
    if(rxStatus != 0)
    {
      /* Log error and don't execute */
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ANLG_STATUS); 

      pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_CROB_ST_FORMAT_ERROR;
      pSDNPSession->iin |= DNPDEFS_IIN_PARAM_ERROR;
      pRxFragment->offset = pRxFragment->msgLength;
      return(TMWDEFS_FALSE);
    }
 
    /* Make sure quantity does not exceed max allowed */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      if((numberOfPoints + index) >= pSDNPSession->maxControlRequests)
      {
        localStatus = DNPDEFS_CTLSTAT_TOO_MANY_OPS;
      }
    }

    /* Make sure point number is valid */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      if(point >= sdnpdata_anlgOutQuantity(pSDNPSession->pDbHandle))
        localStatus = DNPDEFS_CTLSTAT_NOT_SUPPORTED;
    }

    anlgValue.type = TMWTYPES_ANALOG_TYPE_SFLOAT;
    anlgValue.value.fval = value;

    /* Diagnostics */
    DNPDIAG_SHOW_ANALOG_CONTROL(pSession, point, &anlgValue, 0, TMWDEFS_FALSE, TMWDIAG_ID_RX, TMWDEFS_NULL);

    /* Get Point */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      pPoint = sdnpdata_anlgOutGetPoint(pSDNPSession->pDbHandle, point);
      if (pPoint != TMWDEFS_NULL)
      {
        /* Operate Point */
        localStatus = sdnpdata_anlgOutOperate(pPoint, &anlgValue);
      }
      else
      {
        localStatus = DNPDEFS_CTLSTAT_NOT_SUPPORTED;
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
      DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, DNPDEFS_OBJ_41_ANA_OUT_CTRLS, 3, pObjHeader->qualifier);
      DNPDIAG_SHOW_ANALOG_CONTROL(pSession, point, &anlgValue, localStatus, TMWDEFS_FALSE, 0, TMWDEFS_NULL);
    }

    /* Store status into response */
    pResponse->pMsgBuf[pResponse->msgLength++] = localStatus;

    sdnputil_updateIINAfterSelOp(pSession, localStatus);

    if(localStatus != DNPDEFS_CTLSTAT_SUCCESS)
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
#endif /* SDNPDATA_SUPPORT_OBJ41_V3 */

#if SDNPDATA_SUPPORT_OBJ41_V4
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo041_selObj41v4(  TMWSESN *pSession,
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
    void *pPoint;
    TMWTYPES_DOUBLE value;
    TMWTYPES_USHORT point;
    TMWTYPES_UCHAR rxStatus;
    TMWTYPES_UCHAR length;
    TMWTYPES_UCHAR localStatus = status;
    TMWTYPES_ANALOG_VALUE anlgValue;

    if(pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      point = pRxFragment->pMsgBuf[pRxFragment->offset++];
      pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)point;
      length = 10;
    }
    else
    {
      tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &point);
      pRxFragment->offset += 2;

      tmwtarg_store16(&point, &pResponse->pMsgBuf[pResponse->msgLength]);
      pResponse->msgLength += 2;
      length = 11;
    }

    /* Value */
    tmwtarg_get64(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    tmwtarg_store64(&value, &pResponse->pMsgBuf[pResponse->msgLength]);
    pResponse->msgLength += 8;
    pRxFragment->offset += 8;

    /* Read Status (should always be 0) */
    rxStatus = pRxFragment->pMsgBuf[pRxFragment->offset++];
    if(rxStatus != 0)
    {
      /* Log error and don't execute */
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ANLG_STATUS); 

      pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_CROB_ST_FORMAT_ERROR;
      pSDNPSession->iin |= DNPDEFS_IIN_PARAM_ERROR;
      pRxFragment->offset = pRxFragment->msgLength;
      return(TMWDEFS_FALSE);
    } 
    
    /* Make sure quantity does not exceed max allowed */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      if((numberOfPoints + index) >= pSDNPSession->maxControlRequests)
      {
        localStatus = DNPDEFS_CTLSTAT_TOO_MANY_OPS;
      }
    }

    /* Make sure point number is valid */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      if(point >= sdnpdata_anlgOutQuantity(pSDNPSession->pDbHandle))
        localStatus = DNPDEFS_CTLSTAT_NOT_SUPPORTED;
    } 

    anlgValue.type = TMWTYPES_ANALOG_TYPE_DOUBLE;
    anlgValue.value.dval = value;

    /* Diagnostics */
    DNPDIAG_SHOW_ANALOG_CONTROL(pSession, point, &anlgValue, 0, TMWDEFS_FALSE, TMWDIAG_ID_RX, TMWDEFS_NULL);

    /* Get Point */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      pPoint = sdnpdata_anlgOutGetPoint(pSDNPSession->pDbHandle, point);
      if (pPoint != TMWDEFS_NULL)
      {
        /* Select Point */
        localStatus = sdnpdata_anlgOutSelect(pPoint, &anlgValue);
      }
      else
      {
        localStatus = DNPDEFS_CTLSTAT_NOT_SUPPORTED;
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
      DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, DNPDEFS_OBJ_41_ANA_OUT_CTRLS, 4, pObjHeader->qualifier);
      DNPDIAG_SHOW_ANALOG_CONTROL(pSession, point, &anlgValue, localStatus, TMWDEFS_FALSE, 0, TMWDEFS_NULL);
    }

    /* Store status into response */
    pResponse->pMsgBuf[pResponse->msgLength++] = localStatus;

    sdnputil_updateIINAfterSelOp(pSession, localStatus);

    if(localStatus != DNPDEFS_CTLSTAT_SUCCESS)
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

TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo041_opObj41v4(
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
    void *pPoint;
    TMWTYPES_DOUBLE value;
    TMWTYPES_USHORT point;
    TMWTYPES_UCHAR rxStatus;
    TMWTYPES_UCHAR length;
    TMWTYPES_UCHAR localStatus = status;
    TMWTYPES_ANALOG_VALUE anlgValue;

    if(pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      point = pRxFragment->pMsgBuf[pRxFragment->offset++];
      pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)point;
      length = 10;
    }
    else
    {
      tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &point);
      pRxFragment->offset += 2;

      tmwtarg_store16(&point, &pResponse->pMsgBuf[pResponse->msgLength]);
      pResponse->msgLength += 2;
      length = 11;
    }

    /* Value */
    tmwtarg_get64(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    tmwtarg_store64(&value, &pResponse->pMsgBuf[pResponse->msgLength]);
    pResponse->msgLength += 8;
    pRxFragment->offset += 8;

    /* Read Status (should always be 0) */
    rxStatus = pRxFragment->pMsgBuf[pRxFragment->offset++];
    if(rxStatus != 0)
    {
      /* Log error and don't execute */
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ANLG_STATUS); 

      pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_CROB_ST_FORMAT_ERROR;
      pSDNPSession->iin |= DNPDEFS_IIN_PARAM_ERROR;
      pRxFragment->offset = pRxFragment->msgLength;
      return(TMWDEFS_FALSE);
    }
  
    /* Make sure quantity does not exceed max allowed */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      if((numberOfPoints + index) >= pSDNPSession->maxControlRequests)
      {
        SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_SELECT_TO_MANY);
        localStatus = DNPDEFS_CTLSTAT_TOO_MANY_OPS;
      }
    }

    /* Make sure point number is valid */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      if(point >= sdnpdata_anlgOutQuantity(pSDNPSession->pDbHandle))
        localStatus = DNPDEFS_CTLSTAT_NOT_SUPPORTED;
    }

    anlgValue.type = TMWTYPES_ANALOG_TYPE_DOUBLE;
    anlgValue.value.dval = value;

    /* Diagnostics */
    DNPDIAG_SHOW_ANALOG_CONTROL(pSession, point, &anlgValue, 0, TMWDEFS_FALSE, TMWDIAG_ID_RX, TMWDEFS_NULL);

    /* Get Point */
    if(localStatus == DNPDEFS_CTLSTAT_SUCCESS)
    {
      pPoint = sdnpdata_anlgOutGetPoint(pSDNPSession->pDbHandle, point);
      if (pPoint != TMWDEFS_NULL)
      {
        /* Operate Point */
        localStatus = sdnpdata_anlgOutOperate(pPoint, &anlgValue);
      }
      else
      {
        localStatus = DNPDEFS_CTLSTAT_NOT_SUPPORTED;
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
      DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, DNPDEFS_OBJ_41_ANA_OUT_CTRLS, 4, pObjHeader->qualifier);
      DNPDIAG_SHOW_ANALOG_CONTROL(pSession, point, &anlgValue, localStatus, TMWDEFS_FALSE, 0, TMWDEFS_NULL);
    }

    /* Store status into response */
    pResponse->pMsgBuf[pResponse->msgLength++] = localStatus;

    sdnputil_updateIINAfterSelOp(pSession, localStatus);

    if(localStatus != DNPDEFS_CTLSTAT_SUCCESS)
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
#endif /* SDNPDATA_SUPPORT_OBJ41_V4 */

#if SDNPDATA_SUPPORT_SELECT_CANCEL
void TMWDEFS_CALLBACK sdnpo041_cancelSelect(
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
 
    /* skip value field based on variation */
    if(pObjHeader->variation == 4)
      pRxFragment->offset += 9; 
    else if(pObjHeader->variation == 2)
      pRxFragment->offset += 3; 
    else
      pRxFragment->offset += 5; 
        
    pPoint = sdnpdata_anlgOutGetPoint(pSDNPSession->pDbHandle, point);
    if(pPoint != TMWDEFS_NULL)
    {
      /* Cancel Select for this point */ 
      sdnpdata_anlgOutCancelSelect(pPoint);
    }
  }
}
#endif

#if SDNPDATA_SUPPORT_OBJ43
#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpo041_assignClass */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo041_assignClass(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWDEFS_CLASS_MASK classMask)
{
  SDNPSESN *pSDNPSession;
  TMWTYPES_BOOL status;
  TMWTYPES_USHORT i;

  pSDNPSession = (SDNPSESN *)pSession;

  /* limit requested number of points to quantity in database 
   * if status == TMWDEFS_FALSE IN2.2 will be set in response.
   */
  status = sdnputil_validateAssignQualifier(pObjHeader, sdnpdata_anlgOutQuantity(pSDNPSession->pDbHandle));

  for(i = 0; i < pObjHeader->numberOfPoints; i++)
  {
    void *pPoint;
    TMWTYPES_USHORT pointNumber;
    dnputil_getPointNumber(pRxFragment, pObjHeader, i, &pointNumber);

    pPoint = sdnpdata_anlgOutGetPoint(pSDNPSession->pDbHandle, pointNumber);
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

    if(!sdnpdata_anlgOutCmdAssignClass(pPoint, classMask))
      status = TMWDEFS_FALSE;
  }

  return(status);
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ43 */
#endif /* SDNPDATA_SUPPORT_OBJ41 */

