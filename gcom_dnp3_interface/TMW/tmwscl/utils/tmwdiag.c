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

/* file: tmwdiag.c
 * description: General Diagnostics Support
 */
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwdiag.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwmem.h"


const TMWTYPES_CHAR *tmwdiag_monthNames[] =
{
  "Jan", 
  "Feb", 
  "Mar", 
  "Apr", 
  "May", 
  "Jun", 
  "Jul", 
  "Aug", 
  "Sep", 
  "Oct", 
  "Nov", 
  "Dec", 
  "???"
};

#if TMWCNFG_SUPPORT_DIAG
const TMWTYPES_CHAR *tmwdiag_dayNames[] =
{
  "Mon",
  "Tue",
  "Wed",
  "Thu",
  "Fri",
  "Sat",
  "Sun"
};

const TMWTYPES_CHAR *tmwdiag_ampmNames[] =
{
  "am",
  "pm",
  "  "
};


/* routine to call when diagnostic logging occurs to test if we should log the diagnostic */
TMWDIAG_CHK_FILTER_CALLBACK pDiagChkFilterCallback = TMWDEFS_NULL;

/* user callback parameter to pass to pDiagChkFilterCallback */
void *pDiagChkFilterCallbackParam = TMWDEFS_NULL;

/* function: tmwdiag_setChkFilterCallback */
void TMWDEFS_GLOBAL tmwdiag_setChkFilterCallback(
  TMWDIAG_CHK_FILTER_CALLBACK pCallback,
  void *pCallbackParam)
{
  pDiagChkFilterCallback = pCallback;
  pDiagChkFilterCallbackParam = pCallbackParam;
}

/* function: tmwdiag_checkFilter */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwdiag_checkFilter(
  TMWCHNL *pChannel, 
  TMWSESN *pSession, 
  TMWSCTR *pSector, 
  TMWDIAG_ID sourceId)
{
  if (pDiagChkFilterCallback != TMWDEFS_NULL)
  {
    return pDiagChkFilterCallback(pDiagChkFilterCallbackParam, pChannel, pSession, pSector, sourceId);
  }
  if (pSector)
  {
    return ((sourceId & pSector->sctrDiagMask) != 0);
  }
  if (pSession)
  {
    return ((sourceId & pSession->sesnDiagMask) != 0);
  }
  if (pChannel)
  {
    return ((sourceId & pChannel->chnlDiagMask) != 0);
  }
  return TMWDEFS_TRUE;
}

/* function: tmwdiag_initId */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwdiag_initId(
  TMWDIAG_ANLZ_ID *pAnlzId,
  TMWCHNL *pChannel,
  TMWSESN *pSession,
  TMWSCTR *pSector,
  TMWDIAG_ID sourceId)
{
  if (tmwdiag_checkFilter(pChannel, pSession, pSector, sourceId) == TMWDEFS_FALSE)
  {
    return TMWDEFS_FALSE;
  }

  memset(pAnlzId, 0, sizeof(TMWDIAG_ANLZ_ID));

  pAnlzId->sourceId = sourceId;

  pAnlzId->pSector = pSector;
  if(pSector != TMWDEFS_NULL)
  {
    pAnlzId->pSession = pSector->pSession;
  }
  else
  {
    pAnlzId->pSession = pSession;
  }

  if(pAnlzId->pSession != TMWDEFS_NULL)
  {
    pAnlzId->pChannel = pAnlzId->pSession->pChannel;
  }
  else
  {
    pAnlzId->pChannel = pChannel;
  }
  return TMWDEFS_TRUE;
}

/* function: tmwdiag_error */
void TMWDEFS_GLOBAL tmwdiag_error(
  TMWCHNL *pChannel,
  TMWSESN *pSession,
  TMWSCTR *pSector,
  const TMWTYPES_CHAR *errorString)
{
  const TMWTYPES_CHAR *name = TMWDEFS_NULL;
  TMWDIAG_ANLZ_ID id;
  TMWTYPES_CHAR buf[256];

  if (tmwdiag_initId(&id, pChannel, pSession, pSector, TMWDIAG_ID_ERROR) == TMWDEFS_FALSE)
  {
    return;
  }
  

  if(pSector)
  {
    name = tmwsctr_getSectorName(pSector);
  }
  else if(pSession)
  {
    name = tmwsesn_getSessionName(pSession);
  }
  else if(pChannel)
  {
    name = tmwchnl_getChannelName(pChannel);
  }
  else
  {
    name = TMWDEFS_NULL;
  }
 
  if(name)
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%s: **** Error: %s **** \n", name, errorString);
  }
  else
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), " **** Error: %s **** \n", errorString);
  }

  tmwdiag_putLine(&id, "\n");
  tmwdiag_putLine(&id, buf);
  tmwdiag_putLine(&id, "\n");
}

/* function: tmwdiag_message */
void TMWDEFS_GLOBAL tmwdiag_message(
  TMWCHNL *pChannel,
  TMWSESN *pSession,
  TMWSCTR *pSector,
  TMWDIAG_ID sourceId,
  const TMWTYPES_CHAR *messageString)
{
  const TMWTYPES_CHAR *name = TMWDEFS_NULL;
  TMWDIAG_ANLZ_ID id;
  TMWTYPES_CHAR buf[4096];

  if (tmwdiag_initId(&id, pChannel, pSession, pSector, sourceId) == TMWDEFS_FALSE)
  {
    return;
  }
  

  if(pSector)
  {
    name = tmwsctr_getSectorName(pSector);
  }
  else if(pSession)
  {
    name = tmwsesn_getSessionName(pSession);
  }
  else if(pChannel)
  {
    name = tmwchnl_getChannelName(pChannel);
  }
  else
  {
    name = TMWDEFS_NULL;
  }

  if(name)
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%s: **** %s **** \n", name, messageString);
  }
  else
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), " **** %s **** \n", messageString);
  }

  tmwdiag_putLine(&id, "\n");
  tmwdiag_putLine(&id, buf);
  tmwdiag_putLine(&id, "\n");
}

/* function: tmwdiag_target */
void TMWDEFS_GLOBAL tmwdiag_target(
  void *pChannel,
  TMWDIAG_ID sourceId,
  const TMWTYPES_CHAR *messageString)
{
  TMWDIAG_ANLZ_ID id;
  TMWTYPES_CHAR buf[256];

  if (tmwdiag_initId(&id, (TMWCHNL*)pChannel, TMWDEFS_NULL, TMWDEFS_NULL, sourceId) == TMWDEFS_FALSE)
  {
    return;
  }

  tmwtarg_snprintf(buf, sizeof(buf), " ### %s\n", messageString);

  tmwdiag_skipLine(&id);
  tmwdiag_putLine(&id, buf);
}

/* function: tmwdiag_message_name */
void TMWDEFS_GLOBAL tmwdiag_message_name(
  const TMWTYPES_CHAR *name,
  const TMWTYPES_CHAR *messageString,
  TMWDIAG_ID sourceId)
{
  TMWDIAG_ANLZ_ID id;
  TMWTYPES_CHAR buf[1024];

  if (tmwdiag_initId(&id, TMWDEFS_NULL, TMWDEFS_NULL, TMWDEFS_NULL, sourceId) == TMWDEFS_FALSE)
  {
    return;
  }

  if(name)
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%s  %s  \n", name, messageString);
  }
  else
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "  %s  \n", messageString);
  }

  tmwdiag_putLine(&id, "\n");
  tmwdiag_putLine(&id, buf);
  tmwdiag_putLine(&id, "\n");
}

/* function: tmwdiag_skipLine */
void TMWDEFS_GLOBAL tmwdiag_skipLine(
  TMWDIAG_ANLZ_ID *pAnlzId)
{
  pAnlzId->time.invalid = TMWDEFS_TRUE;
  tmwtarg_putDiagString(pAnlzId, "\n");
}

/* function: tmwdiag_putLine */
void TMWDEFS_GLOBAL tmwdiag_putLine(
  TMWDIAG_ANLZ_ID *pAnlzId,
  const TMWTYPES_CHAR *buf)
{
// #if !TMWDIAG_TIMESTAMP_STRUCT
//   /* This will convert the timestamp to ascii and put it in the log message */
//   TMWTYPES_CHAR timebuf[32];
//   TMWDTIME time;
//   TMWTYPES_UINT len;

//   /* Output time */
//   tmwdtime_getDateTime(TMWDEFS_NULL, &time);
//   len = tmwtarg_snprintf(timebuf, sizeof(timebuf), "%02u:%02u:%02u.%03u", 
//     time.hour, time.minutes, time.mSecsAndSecs/1000, time.mSecsAndSecs%1000);

//   (void)tmwtarg_snprintf(timebuf + len, sizeof(timebuf) - len, "%s", ": ");

//   pAnlzId->sourceId |= TMWDIAG_ID_TIMESTAMP;
//   tmwtarg_putDiagString(pAnlzId, timebuf);
//   pAnlzId->sourceId &= ~TMWDIAG_ID_TIMESTAMP;
// #else
  /* Output buf */
  tmwdtime_getDateTime(TMWDEFS_NULL, &pAnlzId->time); 
// #endif
  tmwtarg_putDiagString(pAnlzId, buf);
}

/* function: tmwdiag_showMemoryUsage */
void TMWDEFS_GLOBAL tmwdiag_showMemoryUsage(
  TMWMEM_GET_USAGE_FUNC pFunc)
{
  TMWDIAG_ANLZ_ID id;
  TMWTYPES_CHAR buf[256];
  TMWMEM_POOL_STRUCT allocInfo;
  const TMWTYPES_CHAR *pName;
  TMWTYPES_UCHAR   i=0;

  if (tmwdiag_initId(&id, TMWDEFS_NULL, TMWDEFS_NULL, TMWDEFS_NULL, TMWDIAG_ID_MMI) == TMWDEFS_FALSE)
  {
    return;
  }

  (void)tmwtarg_snprintf(buf, sizeof(buf), "      structure                     allocated       free          max        size\n");

  tmwdiag_putLine(&id, "\n");
  tmwdiag_putLine(&id, buf);
  
  while(pFunc(i, &pName, &allocInfo))
  {
    i++;

#if TMWCNFG_USE_DYNAMIC_MEMORY && !TMWCNFG_ALLOC_ONLY_AT_STARTUP 
    if(allocInfo.max > 0)
      (void)tmwtarg_snprintf(buf, sizeof(buf), "%30s  %10d   %10d   %10d  %10d\n", pName, allocInfo.allocated, (allocInfo.max-allocInfo.allocated), allocInfo.max, allocInfo.size);
    else
      (void)tmwtarg_snprintf(buf, sizeof(buf), "%30s  %10d   %10d   %10d  %10d\n", pName, allocInfo.allocated, 0, 0, allocInfo.size);
    tmwdiag_putLine(&id, buf);
#else
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%30s  %10d   %10d   %10d  %10d\n", pName, tmwdlist_size(&allocInfo.allocatedBuffers), tmwdlist_size(&allocInfo.freeBuffers), allocInfo.max, allocInfo.size);
    tmwdiag_putLine(&id, buf);
#endif
  }
}
#endif /* TMWCNFG_SUPPORT_DIAG */

/* This function is handy even when diagnostics are disabled. */
/* function: tmwdiag_time2string */
void TMWDEFS_GLOBAL tmwdiag_time2string(
  const TMWDTIME *pTimeStamp, 
  TMWDEFS_TIME_FORMAT timeFormat,
  TMWTYPES_CHAR *pTimeString, 
  TMWTYPES_UINT bufferSize, 
  TMWTYPES_BOOL forceZeros)
{
  const TMWDTIME *tmpPtr;
  TMWTYPES_UCHAR month;
  TMWTYPES_CHAR *invalid;
  TMWDTIME tmp;
  
  if (forceZeros)
  {
    tmp.dayOfMonth = 0;
    tmp.dayOfWeek = 0;
    tmp.dstInEffect = 0;
    tmp.energyTariff = 0;
    tmp.hour = 0;
    tmp.invalid = 0;
    tmp.minutes = 0;
    tmp.month = 1;
    tmp.mSecsAndSecs = 0;
    tmp.powerTariff = 0;
    tmp.tis = 0;
    tmp.year = 0;
    tmpPtr = &tmp;
  }
  else
  {
    tmpPtr = pTimeStamp;
  }
  
  invalid = "\0";
  if (tmpPtr->invalid)
  {
    invalid = " (invalid)";
  }
  
  /* Validate month */
  month = tmpPtr->month;
  if ((month < 1) || (month > 12))
  {
    month = 13;
  }
  
  if (timeFormat == TMWDEFS_TIME_FORMAT_NONE)
  {
    pTimeString[0] = '\0';
  }
  else 
  {
    switch (timeFormat)
    {
      case(TMWDEFS_TIME_FORMAT_24):
        {
          /* display date string in format of mm:ss.mmm */
          (void)tmwtarg_snprintf(pTimeString, bufferSize, "%02u:%02u.%03u%s",
            tmpPtr->minutes, tmpPtr->mSecsAndSecs/1000, 
            tmpPtr->mSecsAndSecs%1000, invalid);
          break;
        }
      case(TMWDEFS_TIME_FORMAT_32):
        {
          /* display date string in format of hh:mm:ss.mmm */
          (void)tmwtarg_snprintf(pTimeString, bufferSize, "%02u:%02u:%02u.%03u%s",
            tmpPtr->hour, tmpPtr->minutes, tmpPtr->mSecsAndSecs/1000, 
            tmpPtr->mSecsAndSecs%1000, invalid);
          break;
        }
      case(TMWDEFS_TIME_FORMAT_40):
        {
          /* display date string in format of ddMMMyy hh:mm */
          (void)tmwtarg_snprintf(pTimeString, bufferSize, "%02u%3s%02u %2u:%02u%s",
            tmpPtr->dayOfMonth, tmwdiag_monthNames[month - 1],
            tmpPtr->year % 100, tmpPtr->hour, tmpPtr->minutes, invalid);
          break;
        }
      case(TMWDEFS_TIME_FORMAT_56):
        {
          /* display date string in format of ddMMMyy hh:mm:ss.mmm */
          (void)tmwtarg_snprintf(pTimeString, bufferSize, "%02u%3s%02u %02u:%02u:%02u.%03u%s",
            tmpPtr->dayOfMonth, tmwdiag_monthNames[month - 1],
            tmpPtr->year % 100, tmpPtr->hour, tmpPtr->minutes, 
            tmpPtr->mSecsAndSecs/1000, tmpPtr->mSecsAndSecs%1000, invalid);
        
          break;
        }
      case(TMWDEFS_TIME_FORMAT_XML):
        {
          /* display date string in format of mm/dd/yyyy hh:mm:ss.mmm */
          (void)tmwtarg_snprintf(pTimeString, bufferSize, "%02u/%02u/%04u %02u:%02u:%02u.%03u",
            tmpPtr->month,
            tmpPtr->dayOfMonth, 
            tmpPtr->year, 
            tmpPtr->hour, 
            tmpPtr->minutes, 
            tmpPtr->mSecsAndSecs/1000, 
            tmpPtr->mSecsAndSecs%1000);
        
          break;
        }
      case(TMWDEFS_TIME_FORMAT_LOG):
        {
          /* display date string in format of hh:mm:ss.mmm */
          tmwtarg_snprintf(pTimeString, bufferSize, "%02u:%02u:%02u.%03u: ", 
            tmpPtr->hour, tmpPtr->minutes, tmpPtr->mSecsAndSecs/1000, tmpPtr->mSecsAndSecs%1000);
  
          break;
        }
      default:
        {
          (void)tmwtarg_snprintf(pTimeString, bufferSize, "%21s", "unknown-time-format");
          break;
        }
    }
  }
}

