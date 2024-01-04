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

/* file: sdnpo060.c
 * description: DNP Slave functionality for Object 60 Class Data
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpo060.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpo002.h"
#include "tmwscl/dnp/sdnpo004.h"
#include "tmwscl/dnp/sdnpo010.h"
#include "tmwscl/dnp/sdnpo011.h"
#include "tmwscl/dnp/sdnpo013.h"
#include "tmwscl/dnp/sdnpo020.h"
#include "tmwscl/dnp/sdnpo021.h"
#include "tmwscl/dnp/sdnpo022.h"
#include "tmwscl/dnp/sdnpo023.h"
#include "tmwscl/dnp/sdnpo030.h"
#include "tmwscl/dnp/sdnpo032.h"
#include "tmwscl/dnp/sdnpo033.h"
#include "tmwscl/dnp/sdnpo040.h"
#include "tmwscl/dnp/sdnpo042.h"
#include "tmwscl/dnp/sdnpo043.h"
#include "tmwscl/dnp/sdnpo070.h"
#include "tmwscl/dnp/sdnpo088.h"
#include "tmwscl/dnp/sdnpo111.h"
#include "tmwscl/dnp/sdnpo113.h"
#include "tmwscl/dnp/sdnpo115.h"
#include "tmwscl/dnp/sdnpo120.h"
#if SDNPCNFG_SUPPORT_SA_VERSION5
#include "tmwscl/dnp/sdnpo122.h"
#endif
#include "tmwscl/dnp/sdnprbe.h"
#include "tmwscl/dnp/sdnputil.h"
 
/* This table specifies which objects should be read for a class 1,2,
 * and/or 3 event data poll.
 */
static const SDNPSESN_READ_EVENTS_FUNC _eventGroups[] = {
#if SDNPDATA_SUPPORT_OBJ2
  sdnpo002_readObj2v0ByClass,                       /* Binary Input Events */
#endif
#if SDNPDATA_SUPPORT_OBJ4
  sdnpo004_readObj4v0ByClass,                       /* Double Bit Input Events */
#endif
#if SDNPDATA_SUPPORT_OBJ11
  sdnpo011_readObj11v0ByClass,                      /* Binary Output Events */
#endif
#if SDNPDATA_SUPPORT_OBJ13
  sdnpo013_readObj13v0ByClass,                      /* Binary Output Command Events */
#endif
#if SDNPDATA_SUPPORT_OBJ22
  sdnpo022_readObj22v0ByClass,                      /* Binary Counter Events */
#endif
#if SDNPDATA_SUPPORT_OBJ23
  sdnpo023_readObj23v0ByClass,                      /* Frozen Counter Events */
#endif
#if SDNPDATA_SUPPORT_OBJ32
  sdnpo032_readObj32v0ByClass,                      /* Analog Input Events */
#endif
#if SDNPDATA_SUPPORT_OBJ33
  sdnpo033_readObj33v0ByClass,                      /* Frozen Analog Input Events */
#endif
#if SDNPDATA_SUPPORT_OBJ42
  sdnpo042_readObj42v0ByClass,                      /* Analog Output Events */
#endif
#if SDNPDATA_SUPPORT_OBJ43
  sdnpo043_readObj43v0ByClass,                      /* Analog Output Command Events */
#endif
#if SDNPDATA_SUPPORT_OBJ70
  sdnpo070_readObj70v0ByClass,                      /* Sequential File Transfer Events */
#endif
#if SDNPDATA_SUPPORT_OBJ88
  sdnpo088_readObj88v0ByClass,                      /* Data Set Snapshot Events */
#endif
#if SDNPDATA_SUPPORT_OBJ111
  sdnpo111_readObj111ByClass,                       /* Strings */
#endif
#if SDNPDATA_SUPPORT_OBJ113
  sdnpo113_readObj113ByClass,                       /* Virtual Terminal Events */
#endif 
#if SDNPDATA_SUPPORT_OBJ115
  sdnpo115_readObj115ByClass,                       /* Extended String Events */
#endif 
#if SDNPDATA_SUPPORT_OBJ120 
  sdnpo120_readObj120ByClass,                       /* Secure Authentication Error Events */
#if SDNPCNFG_SUPPORT_SA_VERSION5
  sdnpo122_readobj122v0ByClass,                     /* Secure Authentication Statistics Events */
#endif
#endif
  TMWDEFS_NULL
};

/* function: _updateEventObjectHeader
 * purpose: Keep track of the most restrictive event class object
 *  header. Hence, if class 1 specifies all points, but class 3
 *  specifies a limited quantity, then we will return the limited
 *  quantity.
 * arguments:
 * returns:
 *  FAILURE if qualifier unsupported, else SUCCESS
 */
static SDNPSESN_READ_STATUS TMWDEFS_LOCAL _updateEventObjectHeader(
  TMWSESN *pSession,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  
#if SDNPDATA_SUPPORT_OBJ2 && SDNPDATA_SUPPORT_OBJ4 
  pSDNPSession->readObj2AndObj4 = SDNPSESN_OBJ2ANDOBJ4READ;
  pSDNPSession->obj2Variation = pSDNPSession->obj02DefaultVariation;
  pSDNPSession->obj4Variation = pSDNPSession->obj04DefaultVariation;
#endif

  /* See if the new qualifier is not all points */
  if(pObjHeader->qualifier != DNPDEFS_QUAL_ALL_POINTS)
  {
    /* First check for 16 bit limited quantity */
    if(pObjHeader->qualifier == DNPDEFS_QUAL_16BIT_LIMITED_QTY)
    {
      pSDNPSession->eventObjectHeader.qualifier = DNPDEFS_QUAL_16BIT_LIMITED_QTY;

      /* Set the most restrictive number of points */
      if((pSDNPSession->eventObjectHeader.numberOfPoints == 0)
        || (pSDNPSession->eventObjectHeader.numberOfPoints > pObjHeader->numberOfPoints))
      {
        pSDNPSession->eventObjectHeader.numberOfPoints = pObjHeader->numberOfPoints;
      }
    }
    else if(pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_LIMITED_QTY)
    {
      /* 8 Bit limited quantity overrides all points but not 16 bit limited */
      if(pSDNPSession->eventObjectHeader.qualifier == DNPDEFS_QUAL_ALL_POINTS)
        pSDNPSession->eventObjectHeader.qualifier = DNPDEFS_QUAL_8BIT_LIMITED_QTY;

      /* Set the most restrictive number of points */
      if((pSDNPSession->eventObjectHeader.numberOfPoints == 0)
        || (pSDNPSession->eventObjectHeader.numberOfPoints > pObjHeader->numberOfPoints))
      {
        pSDNPSession->eventObjectHeader.numberOfPoints = pObjHeader->numberOfPoints;
      }
    }
    else
      return(SDNPSESN_READ_FAILED);
  }

  return(SDNPSESN_READ_COMPLETE);
}

/* function: _checkForStaticRead
 * purpose:  determine if request contains both an event and static data read  
 *  if so call database function to allow them to lock the database to
 *  prevent modifying static values until the read is complete 
 * arguments:
 *  pSDNPSession -
 *  pRequest -
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _checkForStaticRead(
  SDNPSESN *pSDNPSession, 
  DNPUTIL_RX_MSG *pRequest)
{
  /* If database is already locked, don't bother to call  
   * this will be called more than once on multifragment responses 
   */
  if(!pSDNPSession->databaseLocked)
  {
    TMWTYPES_ULONG offset;
    DNPUTIL_OBJECT_HEADER header;

    /* Save offset to current object header */
    offset = pRequest->offset;

    /* Parse rest of object headers from message */
    while(pRequest->offset < pRequest->msgLength)
    {
      if(dnputil_parseObjectHeader(pRequest, 0, &header))
      {
        if(header.group == 60 && header.variation == 1)
        {
          sdnpdata_eventAndStaticRead(pSDNPSession->pDbHandle, TMWDEFS_TRUE);
          pSDNPSession->databaseLocked = TMWDEFS_TRUE;
          break;
        }
        /* Skip any object data */
        if(SDNPSESN_READ_FAILED == sdnputil_advanceToNextObjHeader(pRequest, &header))
          break;
      }
      else
      {
        break;
      }
    }
    pRequest->offset = offset;
  }
}

/* function: sdnpo60_readObj60v1 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo060_readObj60v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  SDNPSESN_QUAL qualifier)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  
  TMWTARG_UNUSED_PARAM(pRequest);
  TMWTARG_UNUSED_PARAM(pResponse);

  if(qualifier == SDNPSESN_QUAL_PARSE_ONLY)
    return(SDNPSESN_READ_COMPLETE);
 
  pSDNPSession->staticObjectHeader.numberOfPoints = pObjHeader->numberOfPoints; 
  pSDNPSession->staticObjectHeader.qualifier = pObjHeader->qualifier; 

  /* Validate qualifier */
  switch(pObjHeader->qualifier)
  {
  case DNPDEFS_QUAL_ALL_POINTS:
  case DNPDEFS_QUAL_8BIT_LIMITED_QTY:
  case DNPDEFS_QUAL_16BIT_LIMITED_QTY:
    pSDNPSession->readStaticObjectsRequested = TMWDEFS_TRUE;
    return(SDNPSESN_READ_COMPLETE);
  } 

  return(SDNPSESN_READ_FAILED);
}

/* function: sdnpo60_readStatics */ 
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo060_readStatics(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_USHORT remainingPoints;

  if(!pSDNPSession->readStaticObjectsRequested)
  {
    /* Not a class 0 poll */
    return(SDNPSESN_READ_COMPLETE);
  }
 
  /* Are we reading static objects for the first time */
  if(pSDNPSession->readStatus == SDNPSESN_READ_COMPLETE)
  {
    /* Yes, initialize point index to first point */
    pSDNPSession->readPointIndex = 0;
  }

  /* Save the number of points requested by the master so it can be
   * kept track of and restored for each static group. 
   * numberOfPoints will be changed by subroutines depending on the 
   * qualifier and actual number of points in each group. 
   */
  remainingPoints = pObjHeader->numberOfPoints;
  while(pSDNPSession->readGroupIndex < SDNPCNFG_MAX_NUMBER_STATIC_GROUPS)
  {
    SDNPSESN_READ_FUNC pReadFunc;
    SDNPSESN_READ_STATUS status;

    /* Skip this entry if object group is 0, which indicates it is empty */
    if(pSDNPSession->staticGroups[pSDNPSession->readGroupIndex] != 0)
    {
      pReadFunc = sdnpsesn_getStaticReadFunc(
        pSDNPSession->staticGroups[pSDNPSession->readGroupIndex], 0);

      if(pReadFunc != TMWDEFS_NULL)
      {
        /* Restore the remaining number of points specified by master */
        pObjHeader->numberOfPoints = remainingPoints;
 
        pObjHeader->group = pSDNPSession->staticGroups[pSDNPSession->readGroupIndex];
        pObjHeader->variation = 0;

        status = pReadFunc(pSession, pRequest, pResponse, pObjHeader, SDNPSESN_QUAL_BUILD_RESPONSE);

        if(status != SDNPSESN_READ_COMPLETE)
          return(status);

        /* Subtract the number of points that have been read  
         * remainingPoints could be zero meaning a quantity was 
         * not specified, in this case leave it unchanged.
         */
        if(remainingPoints != 0)
         remainingPoints = (TMWTYPES_USHORT)(remainingPoints - pObjHeader->numberOfPoints);
      }
    }
    pSDNPSession->readPointIndex = 0;
    pSDNPSession->readGroupIndex += 1;
  }

  return(SDNPSESN_READ_COMPLETE);
}
/* function: sdnpo60_readObj60v2 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo060_readObj60v2(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  SDNPSESN_QUAL qualifier)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  TMWTARG_UNUSED_PARAM(pRequest);
  TMWTARG_UNUSED_PARAM(pResponse);

  if(qualifier != SDNPSESN_QUAL_BUILD_RESPONSE)
    return(SDNPSESN_READ_COMPLETE);

  pSDNPSession->readEventClassesRequested |= TMWDEFS_CLASS_MASK_ONE;
  return(_updateEventObjectHeader(pSession, pObjHeader));
}

/* function: sdnpo60_readObj60v3 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo060_readObj60v3(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  SDNPSESN_QUAL qualifier)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  TMWTARG_UNUSED_PARAM(pRequest);
  TMWTARG_UNUSED_PARAM(pResponse);

  if(qualifier != SDNPSESN_QUAL_BUILD_RESPONSE)
    return(SDNPSESN_READ_COMPLETE);

  pSDNPSession->readEventClassesRequested |= TMWDEFS_CLASS_MASK_TWO;
  return(_updateEventObjectHeader(pSession, pObjHeader));
}

/* function: sdnpo60_readObj60v4 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo060_readObj60v4(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  SDNPSESN_QUAL qualifier)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  TMWTARG_UNUSED_PARAM(pRequest);
  TMWTARG_UNUSED_PARAM(pResponse);

  if(qualifier != SDNPSESN_QUAL_BUILD_RESPONSE)
    return(SDNPSESN_READ_COMPLETE);

  pSDNPSession->readEventClassesRequested |= TMWDEFS_CLASS_MASK_THREE;
  return(_updateEventObjectHeader(pSession, pObjHeader));
}

/* function: sdnpo060_readEvents */
SDNPSESN_READ_STATUS TMWDEFS_GLOBAL sdnpo060_readEvents(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_USHORT remainingPoints;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_UCHAR reqVariation;
  TMWTYPES_USHORT i = 0;

  if(pSDNPSession->readEventClassesRequested == 0)
  {
    return(SDNPSESN_READ_COMPLETE);
  }

  /* see if call to lock database is necessary */
  _checkForStaticRead(pSDNPSession, pRequest);

  /* Change object header to default variation */
  reqVariation = pObjHeader->variation;
  pObjHeader->variation = 0;

  /* Save the number of points requested by the master so it can be
   * kept track of and restored for each event group. 
   * numberOfPoints will be changed by subroutines depending on the 
   * qualifier and actual number of events in each group. 
   */
  remainingPoints = pObjHeader->numberOfPoints;

  /* Loop through event groups */
  while(_eventGroups[i])
  {
    SDNPSESN_READ_STATUS status;
    
    /* Restore the remaining number of points specified by master */
    pObjHeader->numberOfPoints = remainingPoints;

    /* Read events into response */
    status = _eventGroups[i](pSession, pRequest, pResponse,
      pObjHeader, pSDNPSession->readEventClassesRequested);

    if(status != SDNPSESN_READ_COMPLETE)
    {
      /* Restore original object variation and return */
      pObjHeader->variation = reqVariation;
      return(status);
    }

    /* Subtract the number of points that have been read  
     * remainingPoints could be zero meaning a quantity was 
     * not specified, in this case leave it unchanged.
     */
    if(remainingPoints != 0)
      remainingPoints = (TMWTYPES_USHORT)(remainingPoints - pObjHeader->numberOfPoints);

    i += 1;
  }

  /* Restore original object variation and return */
  pObjHeader->variation = reqVariation;
  return(SDNPSESN_READ_COMPLETE);
}

