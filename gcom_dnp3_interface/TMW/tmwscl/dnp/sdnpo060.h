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

/* file: sdnpo060.h
 * description: This file is intended for internal SCL use only.
 *   DNP SLAVE functionality for Object 60 Class Data
 */
#ifndef SDNPO060_DEFINED
#define SDNPO060_DEFINED

#include "tmwscl/dnp/sdnpsesn.h"

#ifdef __cplusplus
extern "C" {
#endif

  /* function: sdnpo060_readObj60v1
   * purpose: read data into response using variation 1
   * arguments:
   *  pSession - session we are responding to
   *  pResponse - pointer to response message info
   *  pObjHeader - object header we are processing
   * returns:
   *  SDNPSESN_READ_COMPLETE or SDNPSESN_READ_MORE_DATA
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo060_readObj60v1(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    SDNPSESN_QUAL qualifier);

  /* function: sdnpo060_readObj60v2
   * purpose: read data into response using variation 2
   * arguments:
   *  pSession - session we are responding to
   *  pResponse - pointer to response message info
   *  pObjHeader - object header we are processing
   * returns:
   *  SDNPSESN_READ_COMPLETE or SDNPSESN_READ_MORE_DATA
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo060_readObj60v2(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    SDNPSESN_QUAL qualifier);

  /* function: sdnpo060_readObj60v3
   * purpose: read data into response using variation 3
   * arguments:
   *  pSession - session we are responding to
   *  pResponse - pointer to response message info
   *  pObjHeader - object header we are processing
   * returns:
   *  SDNPSESN_READ_COMPLETE or SDNPSESN_READ_MORE_DATA
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo060_readObj60v3(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    SDNPSESN_QUAL qualifier);

  /* function: sdnpo060_readObj60v4
   * purpose: read data into response using variation 4
   * arguments:
   *  pSession - session we are responding to
   *  pResponse - pointer to response message info
   *  pObjHeader - object header we are processing
   * returns:
   *  SDNPSESN_READ_COMPLETE or SDNPSESN_READ_MORE_DATA
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo060_readObj60v4(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    SDNPSESN_QUAL qualifier);

  /* function: sdnpo60_readStatics 
   * purpose: Read static data into response
   * description: Using the configured staticGroups table, loop through all
   *  static data groups reading the points.
   * arguments:
   *  pSession - pointer to session structure
   *  pRequest - pointer to request message structure
   *  pResponse - response message buffer to read into
   *  pObjHeader - object header from request
   * returns:  
   *  SDNPSESN_READ_COMPLETE if all static data has been read
   *  SDNPSESN_READ_MORE_DATA if more static data remains to be read
   *  SDNPSESN_READ_FAILED if there was a read failure
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo060_readStatics(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse,
    DNPUTIL_OBJECT_HEADER *pObjHeader);
 
  /* function: sdnpo060_readEvents
   * purpose: Read events from the specified class into response
   * description: Using the _eventGroups table, loop through all event
   *  groups determining whether they generate events in this class, if
   *  so read all existing events into the message buffer.
   * arguments:
   *  classMask - class mask to read
   *  pResponse - response message buffer to read into
   *  pObjHeader - object header from request
   * returns: TMWDEFS_TRUE if any events were added to the message 
   *  else TMWDEFS_FALSE
   */
  SDNPSESN_READ_STATUS TMWDEFS_GLOBAL sdnpo060_readEvents(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader);

#ifdef __cplusplus
}
#endif
#endif /* SDNPO060_DEFINED */
