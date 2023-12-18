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

/* file: sdnpo110.c
 * description: DNP Slave functionality for Object 110 Octet String Object
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/sdnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"

#include "tmwscl/dnp/sdnpo110.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnputil.h"

#if SDNPDATA_SUPPORT_OBJ110
/* function: sdnpo110_readObj110 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo110_readObj110(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  SDNPSESN_QUAL qualifier)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_USHORT numPointsInResponse;
  TMWTYPES_USHORT objectHeaderOffset;
  TMWTYPES_USHORT numPointsInObject;
  TMWTYPES_USHORT lastPointNumber;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT pointIndex;
  TMWTYPES_USHORT numPoints;
  TMWTYPES_UCHAR variation;
  TMWTYPES_UCHAR maxLength;
  TMWTYPES_UCHAR buf[DNPDEFS_MAX_STRING_LENGTH];
  TMWTYPES_UCHAR length;
  void *pPoint;

  /* Find out how many data points there are to read */
  numPoints = sdnpdata_strQuantity(pSDNPSession->pDbHandle);
  if(numPoints != 0 && (pSDNPSession->readPointIndex >= numPoints))
    return(SDNPSESN_READ_COMPLETE);

  /* Validate qualifier and fill in object header */
  if(!sdnputil_validateQualifier(pObjHeader, numPoints))
    return(SDNPSESN_READ_FAILED);

  /* For multifragment responses, numberOfPoints needs to be corrected 
   * for points sent in a previous fragment response 
   */
  if((pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_START_STOP)
    ||(pObjHeader->qualifier == DNPDEFS_QUAL_16BIT_START_STOP))
  {
    if(pSDNPSession->readPointIndex > 0)
      pObjHeader->numberOfPoints = (TMWTYPES_USHORT)((pObjHeader->lastPointNumber - (pSDNPSession->readPointIndex + pObjHeader->firstPointNumber)) +1);
  }
  else if((pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_LIMITED_QTY)
    ||(pObjHeader->qualifier == DNPDEFS_QUAL_16BIT_LIMITED_QTY))
  {
    if(pSDNPSession->readMultiFragNumberOfPoints > 0)
      pObjHeader->numberOfPoints = pSDNPSession->readMultiFragNumberOfPoints;
  }
  else
  {
    pObjHeader->numberOfPoints = (TMWTYPES_USHORT)(pObjHeader->numberOfPoints - pSDNPSession->readPointIndex);
  } 

  /* Maximum response length is specified by the request varation */
  maxLength = pObjHeader->variation;
  if(maxLength == 0) maxLength = 255;

  /* Loop through points, storing into response */
  length = 0;
  variation = 0;
  pointNumber = 0;
  lastPointNumber = 0;
  numPointsInObject = 0;
  numPointsInResponse = 0;
  objectHeaderOffset = pResponse->msgLength;
  pointIndex = pSDNPSession->readPointIndex;

  if((pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    || (pObjHeader->qualifier == DNPDEFS_QUAL_16BIT_INDEX)
    || (pObjHeader->qualifier == DNPDEFS_QUAL_16BIT_INDEX_8BITQ))
  {
    if(pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      pRequest->offset = pRequest->offset + pointIndex;
    }
    else
    {
      pRequest->offset += (pointIndex * 2);
    }
  }

  /* Code to implement TB2016-001 */
  if (numPoints == 0 && !pSDNPSession->readStaticObjectsRequested)
  {
    /* Not a class 0 poll */
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_NO_POINTS);
    return(SDNPSESN_READ_FAILED);
  }

  while(numPointsInResponse < pObjHeader->numberOfPoints)
  {
    /* Get point number from point index */
    if(!dnputil_getPointNumber(pRequest, pObjHeader, pointIndex, &pointNumber))
    {
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_READ_POINT);
      return(SDNPSESN_READ_FAILED);
    }

    /* If all we are doing is parsing the request we can stop now */
    if(qualifier == SDNPSESN_QUAL_PARSE_ONLY)
    {
      numPointsInResponse += 1;
      continue;
    }

    /* Get data point */
    pPoint = sdnpdata_strGetPoint(pSDNPSession->pDbHandle, pointNumber);

#if SDNPDATA_SUPPORT_CLASS0_POINT
    /* If this is a class 0 read, check to see if this point should be included in response */
    if((pSDNPSession->readStaticObjectsRequested) && (pPoint != TMWDEFS_NULL))
    {
      if(!sdnpdata_strIsClass0(pPoint))
        pPoint = TMWDEFS_NULL;
    }
#endif

    /* If point enabled get current value */
    if(pPoint != TMWDEFS_NULL)
    {
      sdnpdata_strRead(pPoint, sizeof(buf), buf, &length);

      /* Diagnostics */
      DNPDIAG_SHOW_STRING_DATA(pSession, pointNumber, buf, length, 0);

      /* Not allowed to send length 0 (variation 0) to master */
      if(length == 0)
      {
        pPoint = TMWDEFS_NULL;
      }
      else
      {
        /* See if string will be truncated */
        if(length > maxLength)
        {
          if(pSDNPSession->truncateStrings)
          {
            length = maxLength;
          }
          else
          {
            pPoint = TMWDEFS_NULL;
          }
        }
      }  
    }

    /* Make sure point is enabled and string length matches current object */
    if((pPoint == TMWDEFS_NULL) || (length != variation))
    {
      /* Point is disabled, update current object header */
      if(numPointsInObject > 0)
      {
        /* Update object header since we will have a gap */
        sdnputil_updateObjectHeader(pResponse, 
          objectHeaderOffset, lastPointNumber, numPointsInObject);

        numPointsInObject = 0;
      }  
    }
    else
    {
      /* make sure this string will fit in response */
      if((pResponse->msgLength + variation) > pResponse->maxLength)
      {
        break;
      }
    }

    if(pPoint != TMWDEFS_NULL) 
    {
      /* We have a point, write out object header if required */
      if(numPointsInObject == 0)
      {
        /* This function will check to make sure string fits in response */
        SDNPSESN_READ_STATUS status;
        variation = length;
        objectHeaderOffset = pResponse->msgLength;
        pSDNPSession->readPointIndex = pointIndex;
        if((status = sdnputil_initReadStaticResponse(pResponse, pObjHeader, 
          DNPDEFS_OBJ_110_STRING_DATA, variation, pointNumber, variation)) != SDNPSESN_READ_COMPLETE)
        {
          /* Unable to write header, return status */
          return(status);
        }
      }

      /* Write out point number if required */
      dnputil_writePointNumber(pResponse, objectHeaderOffset, pointNumber);
    
      /* Store data into response */
      memcpy(pResponse->pMsgBuf + pResponse->msgLength, buf, variation);
      pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + variation);
  
      numPointsInObject += 1;
      numPointsInResponse += 1;
      lastPointNumber = pointNumber;
    }
    else
    { 
      /* If point was explicitly asked for, bail */
      if((pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
        || (pObjHeader->qualifier == DNPDEFS_QUAL_16BIT_INDEX))
      {
        SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_REQUESTED_POINT);
        return(SDNPSESN_READ_FAILED);
      }
      pObjHeader->numberOfPoints -= 1;
    }

    /* Get next point */
    pointIndex += 1;

    /* We used to make sure another string would fit assuming it was the same size, 
     * but that not always true, so let the code above decide if it fits 
     */
  }

  /* Does current object have any data in it */
  if(numPointsInObject > 0)
  {
    /* Update object header */
    sdnputil_updateObjectHeader(pResponse, 
      objectHeaderOffset, lastPointNumber, numPointsInObject);
  }

  /* Update read point index for multifragment responses */
  pSDNPSession->readPointIndex = pointIndex;

  /* See if we are done */
  if (numPointsInResponse >= pObjHeader->numberOfPoints)
  {
    /* Code to implement TB2016-001 */
    if (numPointsInResponse == 0)
    {
      /* When reading class 0 this is not an error
       * When getting the second or more fragment of a specific read this is not an error
       */
      if (!pSDNPSession->readStaticObjectsRequested && (pSDNPSession->readStatus != SDNPSESN_READ_MORE_DATA))
      {
        SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_NO_POINTS);
        return(SDNPSESN_READ_FAILED);
      }
    }
    return(SDNPSESN_READ_COMPLETE);
  }
  else
  {
    /* If read of limited quantity, update this so multifrag response will use correct quantity */
    if((pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_LIMITED_QTY)
      ||(pObjHeader->qualifier == DNPDEFS_QUAL_16BIT_LIMITED_QTY))
    {
       pSDNPSession->readMultiFragNumberOfPoints = pObjHeader->numberOfPoints - numPointsInResponse;
    }
    return(SDNPSESN_READ_MORE_DATA);
  }
}

/* function: sdnpo110_writeObj110 */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo110_writeObj110(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_UCHAR strLength;
  TMWTYPES_USHORT point;
  TMWTYPES_USHORT index;
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
  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    /* Get next point number from request */
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &point);
    
    /* Verify length of request */
    if((pRxFragment->offset + strLength) > pRxFragment->msgLength)
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
      pRxFragment->offset = pRxFragment->msgLength;
      return TMWDEFS_FALSE;
    }

    /* Diagnostics */
    DNPDIAG_SHOW_STRING_DATA(pSession, point, (pRxFragment->pMsgBuf + pRxFragment->offset), strLength, TMWDIAG_ID_RX);

    /* Get point */
    pPoint = sdnpdata_strGetPoint(pSDNPSession->pDbHandle, point);
    if(pPoint == TMWDEFS_NULL)
    {
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_STR_NOTENABLED);
      pRxFragment->offset = pRxFragment->msgLength;
      return(TMWDEFS_FALSE);
    }

    /* Write the string */
    if(!sdnpdata_strWrite(pPoint, pRxFragment->pMsgBuf + pRxFragment->offset, strLength))
    {
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_STR_WRITE);
      return(TMWDEFS_FALSE);
    }

    /* Increment to next string */
    pRxFragment->offset = pRxFragment->offset + strLength;
  }

  return(TMWDEFS_TRUE);
}

#if SDNPDATA_SUPPORT_OBJ111
#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpo110_assignClass */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo110_assignClass(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWDEFS_CLASS_MASK classMask)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_BOOL status = TMWDEFS_TRUE;
  TMWTYPES_USHORT index;

  status = sdnputil_validateAssignQualifier(pObjHeader, sdnpdata_strQuantity(pSDNPSession->pDbHandle));

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    TMWTYPES_USHORT pointNumber;
    void *pPoint;

    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    pPoint = sdnpdata_strGetPoint(pSDNPSession->pDbHandle, pointNumber);
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

    if(!sdnpdata_strAssignClass(pPoint, classMask))
      status = TMWDEFS_FALSE;
  }

  return(status);
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ111 */
#endif /* SDNPDATA_SUPPORT_OBJ110 */
