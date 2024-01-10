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

/* file: tmwphysd.c
 * description: Implementation independent physical layer diagnostics
 */
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwphysd.h"

#if TMWCNFG_SUPPORT_DIAG

/* Maximum number of bytes to display on a single row */
#define MAX_ROW_LENGTH 16

/* function: tmwphysd_channelOpened */
void TMWDEFS_GLOBAL tmwphysd_channelOpened(
  TMWCHNL *pChannel)
{
  TMWDIAG_ANLZ_ID id;
  TMWTYPES_CHAR buf[256];

  if (tmwdiag_initId(&id, pChannel, TMWDEFS_NULL, TMWDEFS_NULL, TMWDIAG_ID_PHYS) == TMWDEFS_FALSE)
  {
    return;
  }
  

  tmwtarg_snprintf(buf, sizeof(buf), "%s: opened %s\n", tmwchnl_getChannelName(pChannel), 
    tmwtarg_getChannelInfo(pChannel->pPhysContext->pIOContext));

  tmwdiag_skipLine(&id);
  tmwdiag_putLine(&id, buf);
}

static char * TMWDEFS_LOCAL _reasonToText(
  TMWDEFS_TARG_OC_REASON reason)
{
  switch(reason)
  {  
  case TMWDEFS_TARG_OC_SUCCESS:
    return("opened successfully");

  case TMWDEFS_TARG_OC_FAILURE:  
  case TMWDEFS_TARG_OC_REMOTE_CLOSED:
    return("remote close or communications failure");

  case TMWDEFS_TARG_OC_NEW_CONNECTION:   
    return("new connection received");

  case TMWDEFS_TARG_OC_LOCAL_CLOSED:
    return("local close");    

  case TMWDEFS_TARG_OC_RESET:
    return("local reset");

  } 
  return("unknown reason");
}

/* function: tmwphysd_channelClosed */
void TMWDEFS_GLOBAL tmwphysd_channelClosed(
  TMWCHNL *pChannel,
  TMWDEFS_TARG_OC_REASON reason)
{
  TMWDIAG_ANLZ_ID id;
  TMWTYPES_CHAR buf[256];
  if (tmwdiag_initId(&id, pChannel, TMWDEFS_NULL, TMWDEFS_NULL, TMWDIAG_ID_PHYS) == TMWDEFS_FALSE)
  {
    return;
  }

#ifdef TMW_SUPPORT_MONITOR
  if(!pChannel->pPhysContext->monitorMode)
  {
    tmwtarg_snprintf(buf, sizeof(buf), "%s: closed due to %s\n", tmwchnl_getChannelName(pChannel), 
      _reasonToText(reason));
  }
  else
  {
    if (tmwdiag_initId(&id, pChannel, TMWDEFS_NULL, TMWDEFS_NULL, TMWDIAG_ID_ERROR) == TMWDEFS_FALSE)
    {
      return;
    }
    tmwtarg_snprintf(buf, sizeof(buf), "%s: closed Monitor Channel due to error\n", tmwchnl_getChannelName(pChannel));
  }
#else
  tmwtarg_snprintf(buf, sizeof(buf), "%s: closed due to %s\n", tmwchnl_getChannelName(pChannel), 
    _reasonToText(reason));
#endif

  tmwdiag_skipLine(&id);
  tmwdiag_putLine(&id, buf);
}

/* function: tmwphysd_bytesSent */
void TMWDEFS_GLOBAL tmwphysd_bytesSent(
  TMWCHNL *pChannel,
  const TMWTYPES_UCHAR *pBuff, 
  TMWTYPES_USHORT numBytes)
{
  TMWDIAG_ANLZ_ID id;
  TMWTYPES_INT rowLength;
  TMWTYPES_CHAR buf[256];
  TMWTYPES_INT index;
  TMWTYPES_INT len;
  TMWTYPES_INT i;

  if (tmwdiag_initId(&id, pChannel, TMWDEFS_NULL, TMWDEFS_NULL, TMWDIAG_ID_PHYS) == TMWDEFS_FALSE)
  {
    return;
  }

  index = 0;
  while(index < numBytes)
  {
    if(index == 0)
    {
      tmwdiag_skipLine(&id);
      len = tmwtarg_snprintf(buf, sizeof(buf), "<... %-10s ", tmwchnl_getChannelName(pChannel));
    }
    else
    {
      len = tmwtarg_snprintf(buf, sizeof(buf), "%16s", " ");
    }

    rowLength = numBytes - index;
    if(rowLength > MAX_ROW_LENGTH)
    {
      rowLength = MAX_ROW_LENGTH;
    }

    for(i = 0; i < rowLength; i++)
    {
      len += tmwtarg_snprintf(buf + len, sizeof(buf) - len, "%02x ", pBuff[index++]);
    }

    tmwtarg_snprintf(buf + len, sizeof(buf) - len, "\n");
    tmwdiag_putLine(&id, buf);
  }
}

/* function: tmwphysd_bytesReceived */
void TMWDEFS_GLOBAL tmwphysd_bytesReceived(
  TMWCHNL *pChannel,
  const TMWTYPES_UCHAR *pBuff, 
  TMWTYPES_USHORT numBytes)
{
  TMWDIAG_ANLZ_ID id;
  TMWTYPES_INT rowLength;
  TMWTYPES_CHAR buf[256];
  TMWTYPES_INT index;
  TMWTYPES_INT len;
  TMWTYPES_INT i;

  if (tmwdiag_initId(&id, pChannel, TMWDEFS_NULL, TMWDEFS_NULL, TMWDIAG_ID_PHYS | TMWDIAG_ID_RX) == TMWDEFS_FALSE)
  {
    return;
  }

  index = 0;
  while(index < numBytes)
  {
    if(index == 0)
    {
      tmwdiag_skipLine(&id);
      len = tmwtarg_snprintf(buf, sizeof(buf), "...> %-10s ", tmwchnl_getChannelName(pChannel));
    }
    else
    {
      len = tmwtarg_snprintf(buf, sizeof(buf), "%16s", " ");
    }

    rowLength = numBytes - index;
    if(rowLength > MAX_ROW_LENGTH)
    {
      rowLength = MAX_ROW_LENGTH;
    }

    for(i = 0; i < rowLength; i++)
    {
      len += tmwtarg_snprintf(buf + len, sizeof(buf) - len, "%02x ", pBuff[index++]);
    }

    tmwtarg_snprintf(buf + len, sizeof(buf) - len, "\n");
    tmwdiag_putLine(&id, buf);
  }
}

/* function: tmwphysd_error */
void TMWDEFS_GLOBAL tmwphysd_error(
  TMWCHNL *pChannel,
  const TMWTYPES_CHAR *errorString)
{
  TMWDIAG_ANLZ_ID id;
  TMWTYPES_CHAR buf[256];

  if (tmwdiag_initId(&id, pChannel, TMWDEFS_NULL, TMWDEFS_NULL, TMWDIAG_ID_PHYS | TMWDIAG_ID_ERROR) == TMWDEFS_FALSE)
  {
    return;
  }

  tmwtarg_snprintf(buf, sizeof(buf), "%s: physical layer error: %s\n", tmwchnl_getChannelName(pChannel), errorString);

  tmwdiag_skipLine(&id);
  tmwdiag_putLine(&id, buf);
  tmwdiag_skipLine(&id);
}

/* function: tmwphysd_info */
void TMWDEFS_GLOBAL tmwphysd_info(
  TMWCHNL *pChannel,
  const TMWTYPES_CHAR *infoString)
{
  TMWDIAG_ANLZ_ID id;
  TMWTYPES_CHAR buf[256];

  if (tmwdiag_initId(&id, pChannel, TMWDEFS_NULL, TMWDEFS_NULL, TMWDIAG_ID_PHYS) == TMWDEFS_FALSE)
  {
    return;
  }

  tmwtarg_snprintf(buf, sizeof(buf), "%s: physical layer: %s\n", tmwchnl_getChannelName(pChannel), infoString);

  tmwdiag_skipLine(&id);
  tmwdiag_putLine(&id, buf);
  tmwdiag_skipLine(&id);
}
#endif
