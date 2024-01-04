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

/* file: sdnputil.c
 * description: Miscellaneous DNP Slave utilities
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/sdnpdiag.h" 
#include "tmwscl/utils/tmwtarg.h"

#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/sdnputil.h"
#include "tmwscl/dnp/dnpdtime.h"
#include "tmwscl/dnp/sdnpo051.h"
#include "tmwscl/dnp/dnpstat.h"
#include "tmwscl/dnp/sdnpo002.h"
#include "tmwscl/dnp/sdnpo004.h"

/* function: _cleanupObjectInResponse 
 * purpose: Cleanup the current response object we are building.
 *  Either finish it off, 
 *  remove the last point 
 *  or remove the header if there was no object data yet.
 * arguments: 
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _cleanupObjectInResponse(
  TMWSESN_TX_DATA *pResponse, 
  TMWTYPES_USHORT objectHeaderOffset,
  TMWTYPES_UCHAR qualifier, 
  TMWTYPES_BOOL readBits, 
  TMWTYPES_USHORT messageIndex, 
  TMWTYPES_USHORT lastPointNumber, 
  TMWTYPES_USHORT numPointsInObject,
  TMWTYPES_UCHAR numberPointsInByte)
{

  if(numPointsInObject == 0)
  {
    /* Need to remove the header, since there are no points we
     * could put in using this variation.
     */
    pResponse->msgLength = objectHeaderOffset;
  }
  else 
  {
    /* The current object has data in it */
    /* Advance to new byte if this byte is being used */
    if(readBits && ((messageIndex % numberPointsInByte) != 0))
      pResponse->msgLength++;

    /* Update object header to close out this object */
    sdnputil_updateObjectHeader(pResponse, 
      objectHeaderOffset, lastPointNumber, numPointsInObject);
  
    /* Back up to remove point from message if necessary
     * this is where next object header will go.
     */
    if(qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      pResponse->msgLength--; 
    }
    else if((qualifier == DNPDEFS_QUAL_16BIT_INDEX)
      ||(qualifier == DNPDEFS_QUAL_16BIT_INDEX_8BITQ))
    {
      pResponse->msgLength -= 2;
    }
  }
}

/* function: sdnputil_advanceToNextObjHeader 
 * purpose:  Skip past any object data to next object header in received request.
 * arguments: 
 * returns:
 *  SDNPSESN_READ_COMPLETE if successful
 *  SDNPSESN_READ_FAILED if lengths don't make sense
 */
 SDNPSESN_READ_STATUS TMWDEFS_GLOBAL sdnputil_advanceToNextObjHeader(    
  DNPUTIL_RX_MSG *pRequest, 
  DNPUTIL_OBJECT_HEADER *pObjHdr)
{
  TMWTYPES_USHORT quantity = pObjHdr->numberOfPoints;

  /* If indexed, advance past points in message */
  if(pObjHdr->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
  {
    if((pRequest->offset + quantity) > pRequest->msgLength)
      return(SDNPSESN_READ_FAILED);

     pRequest->offset = pRequest->offset + quantity;
  }
  else if((pObjHdr->qualifier == DNPDEFS_QUAL_16BIT_INDEX)
    || (pObjHdr->qualifier == DNPDEFS_QUAL_16BIT_INDEX_8BITQ))
  {
    if(pRequest->offset + (quantity * 2) > pRequest->msgLength)
      return(SDNPSESN_READ_FAILED);
 
    pRequest->offset = pRequest->offset + (quantity * 2);
  } 

  return(SDNPSESN_READ_COMPLETE);
}

/* function: sdnputil_nextSequenceNumber */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnputil_nextSequenceNumber(
  TMWTYPES_UCHAR sequenceNumber)
{
  return((TMWTYPES_UCHAR)((sequenceNumber + 1) & 0x0f));
}

/* function: sdnputil_validateQualifier */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnputil_validateQualifier(
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWTYPES_USHORT numPoints)
{
  /* ApplicationLayer Doc 2.2.6.12 indicates that if range exceeds quantity or
   * if points specified by index do not exist then IIN2.2 should be set.
   * But for limited quantity or all IIN2.2 should not be set.
   */

  switch(pObjHeader->qualifier)
  {
  case DNPDEFS_QUAL_ALL_POINTS:
    /* Set number of points and last point based on number of data points */
    pObjHeader->numberOfPoints = numPoints;
    pObjHeader->lastPointNumber = (TMWTYPES_USHORT)(numPoints - 1);
    break;

  case DNPDEFS_QUAL_8BIT_LIMITED_QTY:
  case DNPDEFS_QUAL_16BIT_LIMITED_QTY:
    /* Validate quantity */
    pObjHeader->firstPointNumber = 0;

    if(pObjHeader->numberOfPoints > numPoints)
      pObjHeader->numberOfPoints = numPoints;

    pObjHeader->lastPointNumber = (TMWTYPES_USHORT)(pObjHeader->numberOfPoints - 1);
    break;

  case DNPDEFS_QUAL_8BIT_START_STOP:
  case DNPDEFS_QUAL_16BIT_START_STOP:
    /* Validate start and stop indices */
    if(pObjHeader->firstPointNumber > pObjHeader->lastPointNumber)
      return(TMWDEFS_FALSE);

    if(pObjHeader->lastPointNumber >= numPoints)
      return(TMWDEFS_FALSE);

    break;

  case DNPDEFS_QUAL_8BIT_INDEX:
  case DNPDEFS_QUAL_16BIT_INDEX:
  case DNPDEFS_QUAL_16BIT_INDEX_8BITQ:
    break;

  default:
    /* Unsupported qualifier for read request */
    return(TMWDEFS_FALSE);
  }

  return(TMWDEFS_TRUE);
}

/* function: sdnputil_initReadStaticResponse */
SDNPSESN_READ_STATUS TMWDEFS_GLOBAL sdnputil_initReadStaticResponse(
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader, 
  TMWTYPES_UCHAR group, 
  TMWTYPES_UCHAR variation,
  TMWTYPES_USHORT startPoint,
  TMWTYPES_USHORT dataSizeInBytes)
{

#if SDNPDATA_SUPPORT_DATASETS
  /* If this is a read of data set object groups,
   * read responses for data sets have qualifier 0x5b 
   * except for objGrp86v2
   */
  if((group >= DNPDEFS_OBJ_85_DATASET_PROTO) && (group <= DNPDEFS_OBJ_88_DATASET_EVENTS))
  {
    if(!((group == DNPDEFS_OBJ_86_DATASET_DESCR) && (variation == 2)))
    { /* Store object group, variation, and qualifier */
      pResponse->pMsgBuf[pResponse->msgLength++] = group;
      pResponse->pMsgBuf[pResponse->msgLength++] = variation;
      pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_QUAL_16BIT_FREE_FORMAT;
      pResponse->pMsgBuf[pResponse->msgLength++] = 1;
      DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, group, variation, DNPDEFS_QUAL_16BIT_FREE_FORMAT); 
      return(SDNPSESN_READ_COMPLETE);
    }
  }
#endif

#if SDNPDATA_SUPPORT_OBJ0
  if((group == DNPDEFS_OBJ_0_DEVICE_ATTRIBUTES) && (variation == 254))
  {
    /* Object 0 variation 254 will put the header in. */
    /* Put a qualifier other than indexed in to make dnputil_writePointNumber() happy
     * Don't increment msgLength, so this will get overwritten by Object 0 
     * variation 254 response code 
     */
    pResponse->pMsgBuf[pResponse->msgLength+2] = DNPDEFS_QUAL_8BIT_START_STOP;
    DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, group, variation, DNPDEFS_QUAL_8BIT_START_STOP); 
    return(SDNPSESN_READ_COMPLETE);
  }
#endif

  /* If we have points to send write object header */
  if(pObjHeader->numberOfPoints > 0)
  {
    TMWTYPES_UCHAR responseQual;

    /* Make sure object header and at least one data point fits,
     * Use 7 because this is worst case (16bit start stop)
     * could check for 4, 5 and 7 depending on qualifier
     */
    if((pResponse->msgLength + 7 + dataSizeInBytes) >= pResponse->maxLength)
      return(SDNPSESN_READ_MORE_DATA);

    if(pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      /* Store object group, variation, and qualifier */
      pResponse->pMsgBuf[pResponse->msgLength++] = group;
      pResponse->pMsgBuf[pResponse->msgLength++] = variation;
      pResponse->pMsgBuf[pResponse->msgLength++] = responseQual = DNPDEFS_QUAL_8BIT_INDEX;

      /* Store quantity, may be updated later */
      pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_CHAR)pObjHeader->numberOfPoints;
    }
    else if((pObjHeader->qualifier == DNPDEFS_QUAL_16BIT_INDEX)
      || (pObjHeader->qualifier == DNPDEFS_QUAL_16BIT_INDEX_8BITQ))
    {      
      /* Store object group, variation, and qualifier */
      pResponse->pMsgBuf[pResponse->msgLength++] = group;
      pResponse->pMsgBuf[pResponse->msgLength++] = variation;
      pResponse->pMsgBuf[pResponse->msgLength++] = responseQual = DNPDEFS_QUAL_16BIT_INDEX;

      /* Store quantity, may be updated later */
      tmwtarg_store16(&pObjHeader->numberOfPoints, pResponse->pMsgBuf + pResponse->msgLength);
      pResponse->msgLength += 2;
    }
    else
    {
      /* If max point number fits in 8 bits use DNPDEFS_QUAL_8BIT_START_STOP 
       * else use DNPDEFS_QUAL_16_BIT_START_STOP
       */
      responseQual = (TMWTYPES_UCHAR)((pObjHeader->lastPointNumber < 256) 
         ? DNPDEFS_QUAL_8BIT_START_STOP : DNPDEFS_QUAL_16BIT_START_STOP);

      /* Store range based on qualifier */
      if(responseQual == DNPDEFS_QUAL_8BIT_START_STOP)
      {
        /* Store object group, variation, and qualifier */
        pResponse->pMsgBuf[pResponse->msgLength++] = group;
        pResponse->pMsgBuf[pResponse->msgLength++] = variation;
        pResponse->pMsgBuf[pResponse->msgLength++] = responseQual;

        /* Start start and end point */
        pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)startPoint;
        pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)startPoint;
      }
      else
      {
        /* Store object group, variation, and qualifier */
        pResponse->pMsgBuf[pResponse->msgLength++] = group;
        pResponse->pMsgBuf[pResponse->msgLength++] = variation;
        pResponse->pMsgBuf[pResponse->msgLength++] = responseQual;

        /* Start start and end point */
        tmwtarg_store16(&startPoint, &pResponse->pMsgBuf[pResponse->msgLength]);
        pResponse->msgLength += 2;

        tmwtarg_store16(&startPoint, &pResponse->pMsgBuf[pResponse->msgLength]);
        pResponse->msgLength += 2;
      }
    }
    DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, group, variation, responseQual); 
  }
   
  return(SDNPSESN_READ_COMPLETE);
}

/* function: sdnputil_updateObjectHeader */
void TMWDEFS_GLOBAL sdnputil_updateObjectHeader(
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT objectHeaderOffset,
  TMWTYPES_USHORT lastPointNumber,
  TMWTYPES_USHORT numberOfPoints)
{
  TMWTYPES_UCHAR qualifier = pResponse->pMsgBuf[objectHeaderOffset + 2];
  if(qualifier == DNPDEFS_QUAL_8BIT_START_STOP)
  {
    pResponse->pMsgBuf[objectHeaderOffset + 4] = (TMWTYPES_UCHAR)lastPointNumber;
  }
  else if(qualifier == DNPDEFS_QUAL_16BIT_START_STOP)
  {
    tmwtarg_store16(&lastPointNumber, &pResponse->pMsgBuf[objectHeaderOffset + 5]);
  } 
  else if(qualifier == DNPDEFS_QUAL_8BIT_INDEX)
  {
    pResponse->pMsgBuf[objectHeaderOffset + 3] = (TMWTYPES_UCHAR)numberOfPoints;
  }
  else if(qualifier == DNPDEFS_QUAL_16BIT_INDEX)
  {
    tmwtarg_store16(&numberOfPoints, &pResponse->pMsgBuf[objectHeaderOffset + 3]);
  }
  else if(qualifier == DNPDEFS_QUAL_16BIT_FREE_FORMAT)
  {
    pResponse->pMsgBuf[objectHeaderOffset + 3] = (TMWTYPES_UCHAR)numberOfPoints;
  }
}

/* function: sdnputil_readStatic */
SDNPSESN_READ_STATUS TMWDEFS_GLOBAL sdnputil_readStatic(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  SDNPSESN_QUAL qualifier,
  SDNPUTIL_STATIC_DESC *pDesc)
{
  SDNPSESN       *pSDNPSession = (SDNPSESN *)pSession;
  void           *pPoint;
  TMWTYPES_USHORT numPointsInResponse;
  TMWTYPES_USHORT objectHeaderOffset;
  TMWTYPES_USHORT numPointsInObject;
  TMWTYPES_USHORT lastPointNumber;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT pointIndex;
  TMWTYPES_USHORT numPoints;
  SDNPSESN_READ_STATUS status;
  TMWTYPES_UCHAR  numberPointsInByte;
  TMWTYPES_BOOL   readBits;
  TMWTYPES_BOOL   indexingUsed;

  /* Find out how many data points there are to read */
  numPoints = pDesc->pQuantityFunc(pSDNPSession->pDbHandle);
  
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

  /* If all we are doing is parsing the request we can stop now */
  if(qualifier == SDNPSESN_QUAL_PARSE_ONLY)
  {
    return(sdnputil_advanceToNextObjHeader(pRequest, pObjHeader)); 
  }

 
  /* Double Bit Input has 2 bits, 
   * while Binary Input and Binary Input Status has only 1 
   * This variable is not used for any other object groups.
   */
  if(pDesc->group == 3)
  {
    numberPointsInByte = 4;
  }
  else
  {
    numberPointsInByte = 8;
  }

  /* Loop through points, storing into response */
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
    indexingUsed = TMWDEFS_TRUE;
    if(pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      pRequest->offset = pRequest->offset + pointIndex;
      pDesc->sizeInBytes +=1;
    }
    else
    {
      pRequest->offset += (pointIndex * 2);
      pDesc->sizeInBytes += 2;
    }
  }
  else
  {
    indexingUsed = TMWDEFS_FALSE;
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
    TMWTYPES_USHORT messageIndex = 0;
    status = SDNPSESN_READ_FAILED;
 
    /* Get point number from point index */
    if(!dnputil_getPointNumber(pRequest, pObjHeader, pointIndex, &pointNumber))
    {
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_READ_POINT);
      return(SDNPSESN_READ_FAILED);
    }
 
    /* Get data point, and make sure it is enabled */
    pPoint = pDesc->pGetPointFunc(pSDNPSession->pDbHandle, pointNumber);

#if SDNPDATA_SUPPORT_CLASS0_POINT
    /* If this is a class 0 read, check to see if this point should be included in response */
    if((pSDNPSession->readStaticObjectsRequested) && (pPoint != TMWDEFS_NULL))
    {
      if((pDesc->pIsInClass0Func != TMWDEFS_NULL) && (!pDesc->pIsInClass0Func(pPoint)))
        pPoint = TMWDEFS_NULL;
    }
#endif

    if(pPoint == TMWDEFS_NULL)
    { 
      /* Get point returned null or first time through */
      if(numPointsInObject > 0)
      {
        /* Increment message buffer if required */
        if(pDesc->readBits && !indexingUsed
           && ((numPointsInObject % numberPointsInByte) != 0))
        {
          pResponse->msgLength += 1;
        }

        /* Don't update device attributes 0 variation 254, this is done in sdnp000.c */
        if((pDesc->group != DNPDEFS_OBJ_0_DEVICE_ATTRIBUTES) 
          ||(pDesc->variation != 254))
        {
          /* Update object header since we will have a gap */
          sdnputil_updateObjectHeader(pResponse, 
            objectHeaderOffset, lastPointNumber, numPointsInObject);
        }

        numPointsInObject = 0;
      }

      /* If point was explicitly asked for, bail */
      if(indexingUsed)
      {
        SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_REQUESTED_POINT);
        return(SDNPSESN_READ_FAILED);
      }
    }
    else if(pPoint != TMWDEFS_NULL)
    {
      /* This loop allows us to read the same point again if the variation
       * needed has changed because the flags were not nominal.
       */
      status = SDNPSESN_READ_NEW_VARIATION;
      while(status == SDNPSESN_READ_NEW_VARIATION)
      {
        TMWTYPES_UCHAR responseQualifier;

        /* We have a point, write out object header if required */
        if(numPointsInObject == 0)
        {
          SDNPSESN_READ_STATUS initStatus;

          /* Store new object header */
          objectHeaderOffset = pResponse->msgLength;
          pSDNPSession->readPointIndex = pointIndex;
          if((initStatus = sdnputil_initReadStaticResponse(pResponse, pObjHeader, 
            pDesc->group, pDesc->variation, pointNumber, pDesc->sizeInBytes)) != SDNPSESN_READ_COMPLETE)
          {
            /* Unable to write header, return status */
            return(initStatus);
          }
        }

        responseQualifier = pResponse->pMsgBuf[objectHeaderOffset + 2];

        /* Write out point number if required */
        dnputil_writePointNumber(pResponse, objectHeaderOffset, pointNumber);

        if(pDesc->readBits)
        {
          /* If we are replying using 8 or 16 bit indexing
           * force the message index to 0 so that bit or bits(double bits) 
           * will always be put in the least significant bits of the mask
           */
          messageIndex = numPointsInObject;
          if(indexingUsed)
          {
            messageIndex = 0;
          }
        }

        /* Save this in case it gets changed in pReadIntoRespFunc() */
        readBits = pDesc->readBits;

        /* Store data into response */
        status = pDesc->pReadIntoRespFunc(pSession, 
          pResponse, messageIndex, pointNumber, pPoint, pDesc);
        if(status == SDNPSESN_READ_NEW_VARIATION)
        { 
          /* This indicates flags are not nominal, so we need to switch to a variation that sends flags 
           * Finish up this object and start the new object variation 
           */
          _cleanupObjectInResponse(pResponse,  objectHeaderOffset, responseQualifier, readBits, 
            messageIndex, lastPointNumber, numPointsInObject, numberPointsInByte);
         
          /* start next object for new variation, */
          numPointsInObject = 0;

          /* Allow room for index */
          if(indexingUsed)
            pDesc->sizeInBytes+=2;

        }
        else if(status == SDNPSESN_READ_MORE_DATA)
        {
          if ((pDesc->group == DNPDEFS_OBJ_0_DEVICE_ATTRIBUTES)
            && pDesc->variation == 255)
          {
            if (numPointsInObject > 0)
            {
              _cleanupObjectInResponse(pResponse, objectHeaderOffset, responseQualifier, readBits,
                messageIndex, pointNumber, numPointsInObject, numberPointsInByte);
            }
          }
          else if((pDesc->group != DNPDEFS_OBJ_0_DEVICE_ATTRIBUTES) 
            ||pDesc->variation != 254)
          {
            _cleanupObjectInResponse(pResponse,  objectHeaderOffset, responseQualifier, readBits, 
            messageIndex, lastPointNumber, numPointsInObject, numberPointsInByte);
            
            if (pResponse->msgLength == 4)
            {
              /* If there was not enough room for even 1 point, return failure */
              status = SDNPSESN_READ_FAILED;
            }

#if SDNPDATA_SUPPORT_OBJ120
            /* Check to see if just a g120v3 aggressive mode start object in the response */
            if ((pResponse->txFlags &= TMWSESN_TXFLAGS_DNP_AUTH_AGGR)
              && (pResponse->msgLength == 14))
            {
              /* If there was not enough room for even 1 point, return failure */
              status = SDNPSESN_READ_FAILED;
            }
#endif
          }   
          /* Update read point index for multifragment responses */
          pSDNPSession->readPointIndex = pointIndex;

          return(status);
        }
        else if(status == SDNPSESN_READ_FAILED)
        {
          if ((pDesc->group != DNPDEFS_OBJ_0_DEVICE_ATTRIBUTES)
            || pDesc->variation != 254)
          {
            _cleanupObjectInResponse(pResponse, objectHeaderOffset, responseQualifier, readBits,
              messageIndex, lastPointNumber, numPointsInObject, numberPointsInByte);
          }
          numPointsInObject = 0;
        }
      }
    }

    if(status == SDNPSESN_READ_FAILED)
    {
      /* If point was explicitly asked for bail */
      if(indexingUsed)
      {
        SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_REQUESTED_POINT);
        return(SDNPSESN_READ_FAILED);
      }
      
#if SDNPDATA_SUPPORT_OBJ0
      /* If a device attribute single point and specific attribute was requested
       */
      if((pDesc->group == DNPDEFS_OBJ_0_DEVICE_ATTRIBUTES)
         &&(pDesc->variation <254)
         &&(pObjHeader->numberOfPoints == 1))
      {
        SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_REQUESTED_POINT);
        return(SDNPSESN_READ_FAILED);
      }
#endif

      /* Decrement the number of points in the response */
      pObjHeader->numberOfPoints -= 1;
      pointIndex++;
      continue;
    }

    if(pDesc->readBits)
    {     
      /* Increment bitmask if we filled a complete byte */
      if(((messageIndex % numberPointsInByte) == (numberPointsInByte-1)) 
        || (indexingUsed))
      {
        pResponse->msgLength += 1;
      }
    }

    /* Get next point */
    lastPointNumber = pointNumber;
    numPointsInResponse += 1;
    numPointsInObject += 1;
    pointIndex += 1;

    /* Make sure another point will fit */
    if((pResponse->msgLength + pDesc->sizeInBytes) > pResponse->maxLength)
      break;
  }

  /* Increment message buffer if required */
  if(pDesc->readBits && !indexingUsed
    && ((numPointsInObject % numberPointsInByte) != 0))
  {
    pResponse->msgLength += 1;
  }

  /* Update read point index for multifragment responses */
  pSDNPSession->readPointIndex = pointIndex;
  
  /* Don't update device attributes 0 variation 254, this is done in sdnp000.c */
  if((pDesc->group != DNPDEFS_OBJ_0_DEVICE_ATTRIBUTES) 
    ||(pDesc->variation != 254))
  {
    if(numPointsInObject > 0)
    {
      /* Update object header */
      sdnputil_updateObjectHeader(pResponse, 
        objectHeaderOffset, lastPointNumber, numPointsInObject);
    }
  }

  /* See if we are done */
  if(numPointsInResponse >= pObjHeader->numberOfPoints)
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

 
#if SDNPDATA_SUPPORT_OBJ2 && SDNPDATA_SUPPORT_OBJ4

/* function: sdnputil_getObj2Or4Event */
void TMWDEFS_CALLBACK sdnputil_getObj2Or4Event(
  SDNPEVNT_DESC *pDesc,
  SDNPEVNT **pEventPtr, 
  TMWDEFS_CLASS_MASK classMask)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pDesc->pSession;
  SDNPEVNT *pEvent = *pEventPtr;

#if SDNPCNFG_USER_MANAGED_EVENTS
  SDNPEVNT_DATA event;
#endif
 
  if (pEvent == TMWDEFS_NULL)
    return;

  if (pSDNPSession->readObj2AndObj4 != SDNPSESN_OBJ2ANDOBJ4READ)
  {
    if (pEvent->group == pDesc->group)
      return;

    while (pEvent != TMWDEFS_NULL && pEvent->group != pDesc->group)
    {
#if SDNPCNFG_USER_MANAGED_EVENTS
      if (pSDNPSession->userManagedEvents)
        pEvent = sdnpevent_getNextEvent(pDesc, classMask, pEvent, &event);
      else
#endif
      pEvent = (SDNPEVNT *)tmwdlist_getNext((TMWDLIST_MEMBER *)pEvent);
    }
  }
  else
  {
    /* Reading both group 2 and group 4 together */
    while ((pEvent != TMWDEFS_NULL )
      && ((pEvent->classMask & classMask) == 0))
    {
#if SDNPCNFG_USER_MANAGED_EVENTS
      if (pSDNPSession->userManagedEvents)
        pEvent = sdnpevent_getNextEvent(pDesc, classMask, pEvent, &event);
      else
#endif
      pEvent = (SDNPEVNT *)tmwdlist_getNext((TMWDLIST_MEMBER *)pEvent);
    }

    if ((pEvent != TMWDEFS_NULL)
      && (pEvent->group != pDesc->group))
    {
      if (pEvent->group == 2)
        sdnpo002_setDesc(pDesc);
      else
        sdnpo004_setDesc(pDesc);
    }
  }

  *pEventPtr = pEvent;
}
 
/* function: sdnputil_checkForObjRead */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnputil_checkForObjRead(
  DNPUTIL_RX_MSG *pRequest,
  DNPDEFS_OBJ_GROUP_ID objectGroup,
  DNPUTIL_OBJECT_HEADER *pHeader)
{ 
  TMWTYPES_ULONG offset; 

  /* Save offset to current object header */
  offset = pRequest->offset;

  /* Parse rest of object headers from message */
  while(pRequest->offset < pRequest->msgLength)
  {
    if(dnputil_parseObjectHeader(pRequest, 0, pHeader))
    {
      if(pHeader->group == objectGroup)
      { 
        pRequest->offset = offset;
        return TMWDEFS_TRUE;
      }
      /* Skip any object data */
      if(SDNPSESN_READ_FAILED == sdnputil_advanceToNextObjHeader(pRequest, pHeader))
        break;
    }
    else
    {
      break;
    }
  }
  pRequest->offset = offset;
  return TMWDEFS_FALSE;
}

#endif


#if SDNPDATA_SUPPORT_OBJ2_V3 || SDNPDATA_SUPPORT_OBJ4_V3
/* function: sdnputil_readObj2or4Var3 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnputil_readObj2or4Var3(
  SDNPEVNT **pEventPtr,
  TMWTYPES_USHORT *pNumEventsInResponse,
  SDNPEVNT_DESC *pDesc,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWDEFS_CLASS_MASK classMask)
{
  TMWTYPES_USHORT numberOfEvents;
  TMWTYPES_MS_SINCE_70 ctoTime;
#if SDNPCNFG_USER_MANAGED_EVENTS
  SDNPEVNT_DATA eventCopy;
#endif
  TMWTYPES_BOOL ctoTimeIsInvalid = TMWDEFS_FALSE; 
  TMWTYPES_BOOL needCTO = TMWDEFS_TRUE;
  TMWSESN *pSession = pDesc->pSession;
  TMWTYPES_USHORT objHeaderOffset = 0;
  TMWTYPES_USHORT numEventsInObject = 0; 
  SDNPSESN_READ_STATUS status = SDNPSESN_READ_COMPLETE;
  TMWTYPES_UCHAR responseQual = DNPDEFS_QUAL_8BIT_INDEX;
  SDNPEVNT *pEvent = *pEventPtr;
  do
  {
    TMWTYPES_MS_SINCE_70 eventTime;
    TMWTYPES_MS_SINCE_70 deltaTime;

    *pEventPtr = pEvent;

    /* See if this point's class matches request */
    if((pEvent->classMask & classMask) == 0)
      continue;

    /* Convert event time to DNP time */
    dnpdtime_dateTimeToMSSince70(&eventTime, &pEvent->timeStamp);

    /* If we have already written CTO see if this event still fits */
    if(!needCTO)
    {
#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
      /* if the defaultVariation is set to zero, use the variation that was saved when the event was queued
       */
      if(pDesc->readVariation == 0)
      { 
        if (pEvent->defaultVariation == 2 && pEvent->timeStamp.invalid)
        {
          /* Need to switch to variation 3 unsync */
          pEvent->defaultVariation = 3;
        }
        /* If default variation is different for this point write header and return */
        else if(pEvent->defaultVariation != 3)
        {
          sdnpevnt_updateObjectHeader(pResponse, objHeaderOffset, numEventsInObject);
          return(SDNPSESN_READ_COMPLETE);
        }
        pDesc->pSetVariationInfo(pDesc, 3);
      }
#endif
       
      /* If invalid bit is different or time is too far off from CTO 
       * stop this object and send another CTO 
       */
      if(ctoTimeIsInvalid != pEvent->timeStamp.invalid)
      {
        needCTO = TMWDEFS_TRUE;
      }
      else
      {
        dnpdtime_subtractTime(&deltaTime, &eventTime, &ctoTime);
        if(deltaTime.mostSignificant != 0)
        {
          needCTO = TMWDEFS_TRUE;
        }
      }
      if(needCTO)
      {
        sdnpevnt_updateObjectHeader(pResponse, objHeaderOffset, numEventsInObject);
        numEventsInObject = 0;
      }
    }

    if(needCTO)
    {
      /* CTO time is event time */
      ctoTime = eventTime;
      ctoTimeIsInvalid = pEvent->timeStamp.invalid;
      deltaTime.mostSignificant = 0;
      deltaTime.leastSignificant = 0;

      /* Make sure we have room for a CTO, Object Header, and at least
       * one event. We just check the worst case situation.
       */
      if((pResponse->msgLength + 10 + 5 + 2 + 3) > pResponse->maxLength)
        return(SDNPSESN_READ_MORE_DATA);

      /* Store Common Time of Occurance */
      sdnpo051_storeCTO(pSession, pResponse, &pEvent->timeStamp);
      needCTO = TMWDEFS_FALSE;

      /* Save offset to object header */
      objHeaderOffset = pResponse->msgLength;
    }

    /* See if this is the first point in this object */
    if(numEventsInObject == 0)
    {
      /* numberOfEvents is the initial value in the response header, but it gets set to the correct value
       * numberOfEvents is used to determine qualifier
       */
      numberOfEvents = sdnpevnt_countEvents(pSession, classMask, pDesc, TMWDEFS_FALSE, 256);

      /* Write object header */
      if((status = sdnpevnt_initReadEventResponse(pResponse,
        pObjHeader, pDesc->group, pDesc->variation,
        pDesc->quantity-1, numberOfEvents, pDesc->sizeInBytes, &responseQual)) != SDNPSESN_READ_COMPLETE)
      {
        return(status);
      }
    }
     
    /* Diagnostics */
    if(pDesc->group == 2)
    {
      DNPDIAG_SHOW_BINARY_INPUT(pSession,
        pEvent->point, pEvent->flags, TMWDEFS_TRUE, &pEvent->timeStamp);
    }
    else
    { 
      DNPDIAG_SHOW_DOUBLE_INPUT(pSession,
        pEvent->point, pEvent->flags, TMWDEFS_TRUE, &pEvent->timeStamp);
    }

    /* Write point number */
    if(responseQual == DNPDEFS_QUAL_8BIT_INDEX)
    {
      pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)pEvent->point;
    }
    else
    {
      tmwtarg_store16(&pEvent->point, &pResponse->pMsgBuf[pResponse->msgLength]);
      pResponse->msgLength += 2;
    }

    /* Store event data into response */
    pResponse->pMsgBuf[pResponse->msgLength++] = pEvent->flags;

    /* Store relative time */
    tmwtarg_store16(&deltaTime.leastSignificant, pResponse->pMsgBuf + pResponse->msgLength);
    pResponse->msgLength += 2;

    /* Mark event for deletion when acknowledge received */
#if SDNPCNFG_USER_MANAGED_EVENTS
    if(((SDNPSESN *)pSession)->userManagedEvents)
    {
      sdnpdata_umEventSent(((SDNPSESN *)pSession)->pDbHandle, pDesc->group, pEvent->point);
    }
    else
#endif
      pEvent->eventSent = TMWDEFS_TRUE;

    /* Mark response so we know it contains events */
    pResponse->txFlags |= TMWSESN_TXFLAGS_CONTAINS_EVENTS;

    /* Keep track of how many events have been read */
    numEventsInObject += 1;
    *pNumEventsInResponse += 1;

    DNPSTAT_SESN_EVENT_SENT(pDesc->pSession, pDesc->group, pEvent->point); 
    
    /* Have we read all the events the request asked for */
    if(*pNumEventsInResponse >= pObjHeader->numberOfPoints)
      break;

    /* Make sure another event will fit in response,
     * allow 2 bytes for point index, even though it may only be 1 byte
     */
    if(((pResponse->msgLength + pDesc->sizeInBytes +2) >= pResponse->maxLength)
      || ((responseQual == DNPDEFS_QUAL_8BIT_INDEX) && (numEventsInObject == 255)))
      break;
#if SDNPCNFG_USER_MANAGED_EVENTS
    pEvent = (SDNPEVNT *)sdnpevent_getNextEvent(pDesc, classMask, pEvent, &eventCopy);
#else
    pEvent = (SDNPEVNT *)tmwdlist_getNext((TMWDLIST_MEMBER *)pEvent);
#endif
#if SDNPDATA_SUPPORT_OBJ2 && SDNPDATA_SUPPORT_OBJ4
    {
      TMWTYPES_UCHAR group = pDesc->group;
      /* This will update pDesc and pEvent if necessary */
      sdnputil_getObj2Or4Event(pDesc, &pEvent, classMask);
      if (group != pDesc->group)
      {
        /* Update the previous object header, before starting a new object */
        sdnpevnt_updateObjectHeader(pResponse, objHeaderOffset, numEventsInObject);

        /* This function only handles read variation 3 or read variation 0 with default variation of 3 */
        if ((pDesc->readVariation != 0)
          && (pDesc->readVariation != 3))
        {
          *pEventPtr = pEvent;
          return(SDNPSESN_READ_COMPLETE);
        }

        /* Update offset to next object header */
        objHeaderOffset = pResponse->msgLength;
        numEventsInObject = 0;
      }
    }
#endif
  } while(pEvent!= TMWDEFS_NULL);

  /* If we have written any events into response update header with actual
   * number of events written
   */
  if(numEventsInObject > 0)
    sdnpevnt_updateObjectHeader(pResponse, objHeaderOffset, numEventsInObject);

  *pEventPtr = pEvent;
  return(status);
}
#endif

/* function: sdnputil_updateIINEvents */
void TMWDEFS_GLOBAL sdnputil_updateIINEvents(
  TMWSESN *pSession,
  TMWDEFS_CLASS_MASK classMask)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  if(classMask & TMWDEFS_CLASS_MASK_ONE)
    pSDNPSession->iin |= DNPDEFS_IIN_CLASS_1;

  if(classMask & TMWDEFS_CLASS_MASK_TWO)
    pSDNPSession->iin |= DNPDEFS_IIN_CLASS_2;

  if(classMask & TMWDEFS_CLASS_MASK_THREE)
    pSDNPSession->iin |= DNPDEFS_IIN_CLASS_3;
}

/* function: sdnputil_validateSelOpQualifier */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnputil_validateSelOpQualifier(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  pResponse->pMsgBuf[pResponse->msgLength++] = pObjHeader->group;
  pResponse->pMsgBuf[pResponse->msgLength++] = pObjHeader->variation;
  pResponse->pMsgBuf[pResponse->msgLength++] = pObjHeader->qualifier;

  /* Validate qualifier */   
  switch(pObjHeader->qualifier)
  {
  case DNPDEFS_QUAL_8BIT_INDEX:
  case DNPDEFS_QUAL_16BIT_INDEX_8BITQ:
    pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)pObjHeader->numberOfPoints;
    break;

  case DNPDEFS_QUAL_16BIT_INDEX:
    tmwtarg_store16(&pObjHeader->numberOfPoints, &pResponse->pMsgBuf[pResponse->msgLength]);
    pResponse->msgLength += 2;
    break;

  default:
    /* remove first part of the object header from response */
    pResponse->msgLength -=3;
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_INV_QUAL_CTRL);
    pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;

    return(TMWDEFS_FALSE);
  }
  return(TMWDEFS_TRUE);
}

/* function: sdnputil_updateIINAfterSelOp */
void TMWDEFS_GLOBAL sdnputil_updateIINAfterSelOp(
  TMWSESN *pSession, 
  TMWTYPES_UCHAR status)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  switch(status)
  { 
    case DNPDEFS_CTLSTAT_NOT_SUPPORTED:
      pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
      break;

    case DNPDEFS_CTLSTAT_LOCAL:
      /* Make sure local IIN bit is set. This should have already been returned
       * by the user in sdnpdata_getIIN but just to be sure...
       */
      pSDNPSession->iin |= DNPDEFS_IIN_LOCAL;
      break;

    case DNPDEFS_CTLSTAT_ALREADY_ACTIVE:
      pSDNPSession->iin |= DNPDEFS_IIN_ALREADY_EXECUTING;
      break;

     /* For the following, set no IIN bits 
      * see 1815-2012 table 4-10
      * DNPDEFS_CTLSTAT_SUCCESS
      * DNPDEFS_CTLSTAT_TIMEOUT 
      * DNPDEFS_CTLSTAT_NO_SELECT 
      * DNPDEFS_CTLSTAT_FORMAT_ERROR
      * DNPDEFS_CTLSTAT_ALREADY_ACTIVE   
      * DNPDEFS_CTLSTAT_HARDWARE_ERROR 
      * DNPDEFS_CTLSTAT_TOO_MANY_OPS
      * DNPDEFS_CTLSTAT_NOT_AUTHORIZED
      * DNPDEFS_CTLSTAT_UNDEFINED   
      * New status codes added by TB2014-002 will be treated this same way.
      * DNPDEFS_CTLSTAT_DOWNSTRM_LOCAL 
      * DNPDEFS_CTLSTAT_ALR_COMPLETE  
      * DNPDEFS_CTLSTAT_BLOCKED
      * DNPDEFS_CTLSTAT_CANCELLED
      * DNPDEFS_CTLSTAT_BLOCKED_OM
      * DNPDEFS_CTLSTAT_DOWNSTRM_FAIL
      */
    default:
      break;
  }
  return;
}

/* function: sdnputil_getDateTime */
void TMWDEFS_GLOBAL sdnputil_getDateTime(
  TMWSESN *pSession,
  TMWDTIME *pDateTime)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  /* Get system date and time */
  tmwdtime_getDateTime(pSession, pDateTime);

  if(pSDNPSession == TMWDEFS_NULL)
    return;

  /* If clock valid timer has expired tag time as invalid */
  if((pSDNPSession->clockValidPeriod != 0)&&(!tmwtimer_isActive(&pSDNPSession->clockValidTimer)))
    pDateTime->invalid = TMWDEFS_TRUE;
}


/* function: sdnputil_parseApplHeader */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnputil_parseApplHeader(
  TMWSESN_RX_DATA *pRxFragment,
  SDNPSESN *pSDNPSession)
{ 
  pSDNPSession->pendingRequest = TMWDEFS_TRUE;

#if SDNPDATA_KEEP_LAST_RESPONSE
  /* Compare to see if received request is a duplicate of last received request
   * if so, resend last response, but do not process this request again
   */
  if(!pSDNPSession->notDuplicateEligible
    &&(pRxFragment->msgLength == pSDNPSession->lastRcvdRequest.msgLength))
  {
    int i;
    for(i=0; i<pRxFragment->msgLength; i++)
    {
      if(pRxFragment->pMsgBuf[i] != pSDNPSession->lastRcvdRequest.pMsgBuf[i])
      {
        break;
      }
    }
    if(i==pRxFragment->msgLength)
    {
      pSDNPSession->duplicateRequestRcvd = TMWDEFS_TRUE;
      return(TMWDEFS_TRUE);
    }
  } 
  pSDNPSession->notDuplicateEligible = TMWDEFS_FALSE;
#endif

  dnputil_parseApplHeader(pRxFragment, &pSDNPSession->lastRcvdRequest, TMWDEFS_TRUE);
 
  return(TMWDEFS_TRUE);
}

#if SDNPDATA_SUPPORT_ASSIGN
TMWTYPES_BOOL sdnputil_validateAssignQualifier(
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWTYPES_USHORT numPoints)
{
  /* ApplicationLayer Doc 2.2.6.12 indicates that if range exceeds quantity or
   * if points specified by index do not exist then IIN2.2 should be set.
   */
  switch(pObjHeader->qualifier)
  {
  case DNPDEFS_QUAL_ALL_POINTS:
    /* Set number of points based on number of data points */
    pObjHeader->numberOfPoints = numPoints;
    break;

  case DNPDEFS_QUAL_8BIT_START_STOP:
  case DNPDEFS_QUAL_16BIT_START_STOP: 
    /* if the master asked too many points, use quantity in database. */
    if(pObjHeader->numberOfPoints > numPoints)
    {
      pObjHeader->numberOfPoints = numPoints;
      /* return false so IIN2.2 will be set in response. */
      return TMWDEFS_FALSE;
    }
    break;

  case DNPDEFS_QUAL_8BIT_INDEX:
  case DNPDEFS_QUAL_16BIT_INDEX:
  case DNPDEFS_QUAL_16BIT_INDEX_8BITQ:
    /* These are not in the parsing table in the spec */
    /* The message will be parsed for the validity of the points */
    break;

  case DNPDEFS_QUAL_8BIT_LIMITED_QTY:
  case DNPDEFS_QUAL_16BIT_LIMITED_QTY:
    /* if the master asked too many points, use quantity in database. */
    if(pObjHeader->numberOfPoints > numPoints)
    {
      pObjHeader->numberOfPoints = numPoints;
    }
    break;

  default:
    /* Unsupported qualifier for assign request */
    pObjHeader->numberOfPoints = 0;
    return(TMWDEFS_FALSE);
  }
  return(TMWDEFS_TRUE);
}
#endif
