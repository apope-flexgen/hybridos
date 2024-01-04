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

/* file: sdnpdata.h
 * description: This file defines the interface between the Triangle
 *  MicroWorks, Inc. slave DNP source code library and the target database.
 *  The default implementation calls methods in the DNP Slave Database
 *  simulator. These need to be repaced with code to interface with the
 *  device's database.
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwtarg.h"

#include "tmwscl/dnp/sdnpdata.h"
#if TMWCNFG_USE_SIMULATED_DB
#include "tmwscl/dnp/sdnpsim.h"
#include "tmwscl/dnp/sdnpfsim.h"
#endif
#include "tmwscl/dnp/sdnpo120.h"

#if TMWCNFG_USE_MANAGED_SCL
#undef TMWCNFG_USE_SIMULATED_DB
#define TMWCNFG_USE_SIMULATED_DB TMWDEFS_FALSE
#endif

#if TMWCNFG_USE_MANAGED_SCL
#include "tmwscl/.NET/TMW.SCL/SDNPDataBaseWrapper.h"
#include "tmwscl/.NET/TMW.SCL/TMWCryptoWrapper.h"
#endif 

#if TMWTARG_SUPPORT_DNPFILEIO
#include "tmwscl/dnp/sdnptarg.h"
#endif

/* function: sdnpdata_init */
void * TMWDEFS_GLOBAL sdnpdata_init(
  TMWSESN *pSession,
  void *pUserHandle)
{ 
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pUserHandle);
  return(sdnpsim_init(pSession));
#elif TMWCNFG_USE_MANAGED_SCL
  return (SDNPDatabaseWrapper_Init(pSession, pUserHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pUserHandle);
  TMWTARG_UNUSED_PARAM(pSession);
  TMWDIAG_ERROR("sdnpdata_init has not been implemented\n");
  return(TMWDEFS_NULL);
#endif
}

/* function: sdnpdata_close */
void TMWDEFS_GLOBAL sdnpdata_close(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pHandle);
  sdnpsim_close(pHandle);
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_Close(pHandle);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
#endif
}

/* function: sdnpdata_getIIN */
void TMWDEFS_GLOBAL sdnpdata_getIIN(
  TMWSESN *pSession,
  TMWTYPES_USHORT *pIIN)
{
#if TMWCNFG_USE_SIMULATED_DB
  *pIIN = sdnpsim_getIIN(pSession);
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_GetIIN(pSession, pIIN);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pSession);
  TMWTARG_UNUSED_PARAM(pIIN);
#endif
}
 
/* function: sdnpdata_IINQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_IINQuantity(
  void *pHandle)
{ 
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_IINQuantity(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_IINQuantity(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(16);
#endif
}

/* function: sdnpdata_IINRead */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_IINRead(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{ 
#if TMWCNFG_USE_SIMULATED_DB
 return(sdnpsim_IINRead(pHandle, pointNumber));
#elif TMWCNFG_USE_MANAGED_SCL
 return(SDNPDatabaseWrapper_IINRead(pHandle, pointNumber));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  return(TMWDEFS_FALSE);
#endif
}

/* function: sdnpdata_coldRestart */
void TMWDEFS_GLOBAL sdnpdata_coldRestart(
  TMWSESN *pSession)
{
#if TMWCNFG_USE_SIMULATED_DB
 TMWTARG_UNUSED_PARAM(pSession);
#elif TMWCNFG_USE_MANAGED_SCL
 SDNPDatabaseWrapper_ColdRestart(pSession);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pSession);
#endif
}

/* function: sdnpdata_warmRestart */
void TMWDEFS_GLOBAL sdnpdata_warmRestart(
  TMWSESN *pSession)
{
#if TMWCNFG_USE_SIMULATED_DB
 TMWTARG_UNUSED_PARAM(pSession);
#elif TMWCNFG_USE_MANAGED_SCL
 SDNPDatabaseWrapper_WarmRestart(pSession);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pSession);
#endif
}

/* function: sdnpdata_setTime */
void sdnpdata_setTime(
  void *pHandle,
  TMWDTIME *pNewTime)
{
  /* Default behavior is to set the clock. Target implementations
   * may only want 1 sector/session on a device to set the time. 
   * Otherwise multiple masters might set the time differently.
   */
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pHandle);
  /* Set the current date and time */
  tmwtarg_setDateTime(pNewTime);
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_SetTime(pHandle, pNewTime);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  /* Set the current date and time */
  tmwtarg_setDateTime(pNewTime);
#endif
}

/* function: sdnpdata_unsolEventMask */
void TMWDEFS_GLOBAL sdnpdata_unsolEventMask(
  void *pHandle, 
  TMWDEFS_CLASS_MASK unsolEventMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(unsolEventMask);
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_UnsolEventMask(pHandle, unsolEventMask);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(unsolEventMask);
#endif
} 

/* function: sdnpdata_eventAndStaticRead */
void TMWDEFS_GLOBAL sdnpdata_eventAndStaticRead(
  void *pHandle,
  TMWTYPES_BOOL inProgress)
{ 
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(inProgress);
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_EventAndStaticRead(pHandle, inProgress);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(inProgress);
#endif
}

/* function: sdnpdata_funcCode */
void TMWDEFS_GLOBAL sdnpdata_funcCode(
  void *pHandle,
  char functionCode,
  TMWTYPES_BOOL inProgress)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(functionCode);
  TMWTARG_UNUSED_PARAM(inProgress);
}

#if SDNPDATA_SUPPORT_XML || SDNPDATA_SUPPORT_XML2
/* function: sdnpdata_binInGetDescription */
TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_binInGetDescription(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(tmwsim_getDescription((TMWSIM_POINT *)pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinInGetDescription((TMWSIM_POINT *)pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_NULL);
#endif
}
#endif

/* function: sdnpdata_binInQuantity */
TMWTYPES_USHORT TMWDEFS_CALLBACK sdnpdata_binInQuantity(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binInQuantity(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinInQuantity(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(0);
#endif
}

/* function: sdnpdata_binInGetPoint */
void * TMWDEFS_CALLBACK sdnpdata_binInGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binInGetEnabledPoint(pHandle, pointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinInGetPoint(pHandle, pointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNum);
  return(TMWDEFS_NULL);
#endif
}

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
/* function: sdnpdata_binInDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_binInDefVariation(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binInDefVariation(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinInDefVariation(pPoint));
#else
  TMWTARG_UNUSED_PARAM(pPoint);
  return(1);
#endif
} 
#endif

/* function: sdnpdata_binInEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_binInEventClass(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventClass(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinInEventClass(pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_CLASS_MASK_NONE);
#endif
} 

#if SDNPDATA_SUPPORT_CLASS0_POINT
/* function: sdnpdata_binInIsClass0 */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binInIsClass0(
  void *pPoint)
{  
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_inClass0(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_IsInClass0(pPoint, TMWDEFS_FALSE));
#else  
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_TRUE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
/* function: sdnpdata_binInEventDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_binInEventDefVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binInEventDefVariation(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinInEventDefVariation(pPoint, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(3);
#endif
} 
#endif

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
/* function: sdnpdata_binInEventMode */
TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_binInEventMode(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventMode(pPoint));  
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_EventMode(pPoint, TMWDEFS_FALSE));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_EVENT_MODE_SOE);
#endif
} 
#endif

#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpdata_binInAssignClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binInAssignClass(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_setEventClass(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinInAssignClass(pPoint, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(TMWDEFS_FALSE);
#endif
}
#endif

/* function: sdnpdata_binInRead */
void TMWDEFS_GLOBAL sdnpdata_binInRead(
  void *pPoint,
  TMWTYPES_UCHAR *pFlags)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTYPES_BOOL value;
  sdnpsim_binInRead(pPoint, &value, pFlags);
  *pFlags &= ~DNPDEFS_DBAS_FLAG_BINARY_ON;
  if(value) *pFlags |= DNPDEFS_DBAS_FLAG_BINARY_ON;
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_BinInRead(pPoint, pFlags);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pFlags);
#endif

  return;
}

#if SDNPDATA_SUPPORT_EVENT_SCAN
/* function: sdnpdata_binInChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binInChanged(
  void *pPoint,
  TMWTYPES_UCHAR *pFlags)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTYPES_BOOL value;
  if(sdnpsim_binInChanged(pPoint, &value, pFlags))
  {
    *pFlags &= ~DNPDEFS_DBAS_FLAG_BINARY_ON;
    if(value) *pFlags |= DNPDEFS_DBAS_FLAG_BINARY_ON;
    return(TMWDEFS_TRUE);
  }
  return(TMWDEFS_FALSE);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_BinInChanged(pPoint, pFlags);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pFlags);
  return(TMWDEFS_FALSE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_XML || SDNPDATA_SUPPORT_XML2
/* function: sdnpdata_binOutGetDescription */
TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_binOutGetDescription(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(tmwsim_getDescription((TMWSIM_POINT *)pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinOutGetDescription((TMWSIM_POINT *)pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_NULL);
#endif
}
#endif

/* function: sdnpdata_binOutQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_binOutQuantity(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binOutQuantity(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinOutQuantity(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(0);
#endif
}

/* function: sdnpdata_binOutGetPoint */
void * TMWDEFS_GLOBAL sdnpdata_binOutGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binOutGetEnabledPoint(pHandle, pointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinOutGetPoint(pHandle, pointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNum);
  return(TMWDEFS_NULL);
#endif
}

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
/* function: sdnpdata_binOutDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_binOutDefVariation(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binOutDefVariation(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinOutDefVariation(pPoint));
#else
  TMWTARG_UNUSED_PARAM(pPoint);
  return(2);
#endif
}
#endif

#if SDNPDATA_SUPPORT_CLASS0_POINT
/* function: sdnpdata_binOutIsClass0 */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binOutIsClass0(
  void *pPoint)
{  
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_inClass0(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_IsInClass0(pPoint, TMWDEFS_TRUE));
#else  
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_TRUE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_OBJ11
#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
/* function: sdnpdata_binOutEventDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_binOutEventDefVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binOutEventDefVariation(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinOutEventDefVariation(pPoint, classMask));
#else
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(3);
#endif
} 
#endif

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
/* function: sdnpdata_binOutEventMode */
TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_binOutEventMode(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventMode(pPoint));  
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_EventMode(pPoint, TMWDEFS_TRUE));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_EVENT_MODE_SOE);
#endif
} 
#endif

/* function: sdnpdata_binOutEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_binOutEventClass(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventClass(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinOutEventClass(pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_CLASS_MASK_NONE);
#endif
}

#if SDNPDATA_SUPPORT_EVENT_SCAN
/* function: sdnpdata_binOutChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binOutChanged(
  void *pPoint,
  TMWTYPES_UCHAR *pFlags)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTYPES_BOOL value;
  if (sdnpsim_binOutChanged(pPoint, &value, pFlags))
  {
    *pFlags &= ~DNPDEFS_DBAS_FLAG_BINARY_ON;
    if (value) *pFlags |= DNPDEFS_DBAS_FLAG_BINARY_ON;
    return(TMWDEFS_TRUE);
  }
  return(TMWDEFS_FALSE);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_BinOutChanged(pPoint, pFlags);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pFlags);
  return(TMWDEFS_FALSE);
#endif
}
#endif
#endif

#if SDNPDATA_SUPPORT_OBJ13
/* function: sdnpdata_binOutCmdEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_binOutCmdEventClass(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventClass(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinOutCmdEventClass(pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_CLASS_MASK_NONE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpdata_binOutAssignClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binOutAssignClass(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_setEventClass(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinOutAssignClass(pPoint, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(TMWDEFS_FALSE);
#endif
}
#endif

/* function: sdnpdata_binOutRead */
void TMWDEFS_GLOBAL sdnpdata_binOutRead(
  void *pPoint,
  TMWTYPES_UCHAR *pFlags)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTYPES_BOOL value;
  sdnpsim_binOutRead(pPoint, &value, pFlags);
  *pFlags &= ~DNPDEFS_DBAS_FLAG_BINARY_ON;
  if(value) *pFlags |= DNPDEFS_DBAS_FLAG_BINARY_ON;
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_BinOutRead(pPoint, pFlags);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pFlags);
#endif
  return;
}

/* function: sdnpdata_binOutWrite */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binOutWrite(
  void *pPoint,
  TMWTYPES_UCHAR value)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binOutWrite(pPoint, (TMWTYPES_BOOL)(value != 0 ? TMWDEFS_TRUE : TMWDEFS_FALSE)));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinOutWrite(pPoint, (TMWTYPES_BOOL)(value != 0 ? TMWDEFS_TRUE : TMWDEFS_FALSE)));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(value);
  return(TMWDEFS_FALSE);
#endif
}

/* function: sdnpdata_binOutGetControlMask */
SDNPDATA_CROB_CTRL TMWDEFS_GLOBAL sdnpdata_binOutGetControlMask(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return((SDNPDATA_CROB_CTRL)sdnpsim_binOutGetControlMask(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinOutGetControlMask(pPoint));
#else
  TMWTARG_UNUSED_PARAM(pPoint);
  return((SDNPDATA_CROB_CTRL)0);
#endif
}

/* function: sdnpdata_binOutSelect */
DNPDEFS_CROB_ST TMWDEFS_GLOBAL sdnpdata_binOutSelect(
  void *pPoint,
  TMWTYPES_UCHAR controlCode,
  TMWTYPES_UCHAR count,
  TMWTYPES_ULONG onTime,
  TMWTYPES_ULONG offTime)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binOutSelect(pPoint, controlCode, count, onTime, offTime));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinOutSelect(pPoint, controlCode, count, onTime, offTime));
#else
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(controlCode);
  TMWTARG_UNUSED_PARAM(count);
  TMWTARG_UNUSED_PARAM(onTime);
  TMWTARG_UNUSED_PARAM(offTime);
  return(DNPDEFS_CROB_ST_NOT_SUPPORTED);
#endif
}

/* function: sdnpdata_binOutOperate */
DNPDEFS_CROB_ST TMWDEFS_GLOBAL sdnpdata_binOutOperate(
  void *pPoint,
  TMWTYPES_UCHAR controlCode,
  TMWTYPES_UCHAR count,
  TMWTYPES_ULONG onTime,
  TMWTYPES_ULONG offTime)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binOutOperate(pPoint, controlCode, count, onTime, offTime));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinOutOperate(pPoint, controlCode, count, onTime, offTime));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(controlCode);
  TMWTARG_UNUSED_PARAM(count);
  TMWTARG_UNUSED_PARAM(onTime);
  TMWTARG_UNUSED_PARAM(offTime);
  return(DNPDEFS_CROB_ST_NOT_SUPPORTED);
#endif
}

#if SDNPDATA_SUPPORT_SELECT_CANCEL
/* function: sdnpdata_binOutCancelSelect */
void TMWDEFS_GLOBAL sdnpdata_binOutCancelSelect(
  void *pPoint)
{
#if TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinOutCancelSelect(pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
#endif
}
#endif

#if SDNPDATA_SUPPORT_OBJ12_V3
/* function: sdnpdata_binOutSelPatternMask */
DNPDEFS_CROB_ST TMWDEFS_GLOBAL sdnpdata_binOutSelPatternMask(
  void *pHandle,
  TMWTYPES_UCHAR control,
  TMWTYPES_UCHAR count,
  TMWTYPES_ULONG activationPeriod,
  TMWTYPES_ULONG deactivationPeriod,
  TMWTYPES_USHORT firstPointNumber,
  TMWTYPES_USHORT lastPointNumber,
  TMWTYPES_UCHAR *pMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binOutSelPatternMask(pHandle, control, count,
    activationPeriod, deactivationPeriod, firstPointNumber, lastPointNumber, pMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinOutSelPatternMask(pHandle, control, count,
    activationPeriod, deactivationPeriod, firstPointNumber, lastPointNumber, pMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(control);
  TMWTARG_UNUSED_PARAM(count);
  TMWTARG_UNUSED_PARAM(activationPeriod);
  TMWTARG_UNUSED_PARAM(deactivationPeriod);
  TMWTARG_UNUSED_PARAM(firstPointNumber);
  TMWTARG_UNUSED_PARAM(lastPointNumber);
  TMWTARG_UNUSED_PARAM(pMask);
  return(DNPDEFS_CROB_ST_NOT_SUPPORTED);
#endif
}

/* function: sdnpdata_binOutOpPatternMask */
DNPDEFS_CROB_ST TMWDEFS_GLOBAL sdnpdata_binOutOpPatternMask(
  void *pHandle,
  TMWTYPES_UCHAR control,
  TMWTYPES_UCHAR count,
  TMWTYPES_ULONG activationPeriod,
  TMWTYPES_ULONG deactivationPeriod,
  TMWTYPES_USHORT firstPointNumber,
  TMWTYPES_USHORT lastPointNumber,
  TMWTYPES_UCHAR *pMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binOutOpPatternMask(pHandle, control, count,
    activationPeriod, deactivationPeriod, firstPointNumber, lastPointNumber, pMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinOutOpPatternMask(pHandle, control, count,
    activationPeriod, deactivationPeriod, firstPointNumber, lastPointNumber, pMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(control);
  TMWTARG_UNUSED_PARAM(count);
  TMWTARG_UNUSED_PARAM(activationPeriod);
  TMWTARG_UNUSED_PARAM(deactivationPeriod);
  TMWTARG_UNUSED_PARAM(firstPointNumber);
  TMWTARG_UNUSED_PARAM(lastPointNumber);
  TMWTARG_UNUSED_PARAM(pMask);
  return(DNPDEFS_CROB_ST_NOT_SUPPORTED);
#endif
}

#if SDNPDATA_SUPPORT_SELECT_CANCEL
/* function: sdnpdata_binOutCancelPatMask */
void TMWDEFS_GLOBAL sdnpdata_binOutCancelPatMask(
  void *pHandle,
  TMWTYPES_USHORT firstPointNumber,
  TMWTYPES_USHORT lastPointNumber,
  TMWTYPES_UCHAR *pMask)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(firstPointNumber);
  TMWTARG_UNUSED_PARAM(lastPointNumber);
  TMWTARG_UNUSED_PARAM(pMask);
}
#endif
#endif

#if SDNPDATA_SUPPORT_OBJ13
#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
/* function: sdnpdata_binOutEventCmdDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_binOutCmdEventDefVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binOutEventDefVariation(pPoint, classMask));  
#elif TMWCNFG_USE_MANAGED_SCL && SDNPDATA_SUPPORT_OBJ11
  return(SDNPDatabaseWrapper_BinOutCmdEventDefVariation(pPoint, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(1);
#endif
} 
#endif

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_binOutCmdEventMode(
  void *pPoint)
{
  #if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventMode(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_EventMode(pPoint, TMWDEFS_TRUE));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_EVENT_MODE_SOE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_EVENT_SCAN
/* function: sdnpdata_binOutCmdChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binOutCmdChanged(
  void *pPoint,
  TMWTYPES_UCHAR *pStatus)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pStatus);
  return(TMWDEFS_FALSE);
#elif TMWCNFG_USE_MANAGED_SCL && SDNPDATA_SUPPORT_OBJ11
  return(SDNPDatabaseWrapper_BinOutCmdChanged(pPoint, pStatus));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pStatus);
  return(TMWDEFS_FALSE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpdata_binOutCmdAssignClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binOutCmdAssignClass(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_setCmdEventClass(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinOutCmdAssignClass(pPoint, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(TMWDEFS_FALSE);
#endif
}
#endif
#endif

#if SDNPDATA_SUPPORT_OBJ20
#if SDNPDATA_SUPPORT_XML || SDNPDATA_SUPPORT_XML2
/* function: sdnpdata_binCntrGetDescription */
TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_binCntrGetDescription(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(tmwsim_getDescription((TMWSIM_POINT *)pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinCntrGetDescription((TMWSIM_POINT *)pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_NULL);
#endif
}
#endif

/* function: sdnpdata_binCntrQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_binCntrQuantity(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binCntrQuantity(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinCntrQuantity(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(0);
#endif
}

/* function: sdnpdata_binCntrGetPoint */
void * TMWDEFS_GLOBAL sdnpdata_binCntrGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binCntrGetEnabledPoint(pHandle, pointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinCntrGetPoint(pHandle, pointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNum);
  return(TMWDEFS_NULL);
#endif
}

/* function: sdnpdata_binCntrGetFrzPoint */
void * TMWDEFS_GLOBAL sdnpdata_binCntrGetFrzPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binCntrGetEnabledPoint(pHandle, pointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinCntrGetFrzPoint(pHandle, pointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNum);
  return(TMWDEFS_NULL);
#endif
} 

#if SDNPDATA_SUPPORT_CLASS0_POINT
/* function: sdnpdata_binCntrIsClass0 */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binCntrIsClass0(
  void *pPoint)
{  
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_inClass0(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_IsInClass0(pPoint, TMWDEFS_FALSE));
#else  
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_TRUE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_OBJ22
#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
/* function: sdnpdata_binCntrEventDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_binCntrEventDefVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binCntrEventDefVariation(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinCntrEventDefVariation(pPoint, classMask));
#else
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(1);
#endif
}
#endif

/* function: sdnpdata_binCntrEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_binCntrEventClass(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventClass(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinCntrEventClass(pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_CLASS_MASK_NONE);
#endif
}

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
/* function: sdnpdata_binCntrEventMode */
TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_binCntrEventMode(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventMode(pPoint));  
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_EventMode(pPoint, TMWDEFS_FALSE));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_EVENT_MODE_MOST_RECENT);
#endif
} 
#endif

#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpdata_binCntrAssignClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binCntrAssignClass(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_setEventClass(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinCntrAssignClass(pPoint, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(TMWDEFS_FALSE);
#endif
} 
#endif

#if SDNPDATA_SUPPORT_EVENT_SCAN
/* function: sdnpdata_binCntrChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binCntrChanged(
  void *pPoint,
  TMWTYPES_ULONG *pValue,
  TMWTYPES_UCHAR *pFlags)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binCntrChanged(pPoint, pValue, pFlags));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinCntrChanged(pPoint, pValue, pFlags));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pValue);
  TMWTARG_UNUSED_PARAM(pFlags);
  return(TMWDEFS_FALSE);
#endif
}
#endif
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
/* function: sdnpdata_binCntrDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_binCntrDefVariation(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binCntrDefVariation(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinCntrDefVariation(pPoint));
#else
  TMWTARG_UNUSED_PARAM(pPoint);
  return(5);
#endif
}
#endif

/* function: sdnpdata_binCntrRead */
void TMWDEFS_GLOBAL sdnpdata_binCntrRead(
  void *pPoint,
  TMWTYPES_ULONG *pValue,
  TMWTYPES_UCHAR *pFlags)
{
#if TMWCNFG_USE_SIMULATED_DB
  sdnpsim_binCntrRead(pPoint, pValue, pFlags);
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_BinCntrRead(pPoint, pValue, pFlags);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pValue);
  TMWTARG_UNUSED_PARAM(pFlags);
#endif
  return;
}

#if SDNPDATA_SUPPORT_OBJ21
/* function: sdnpdata_binCntrFreeze */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binCntrFreeze(
  void *pPoint,
  TMWTYPES_BOOL clearAfterFreeze)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binCntrFreeze(pPoint, clearAfterFreeze));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinCntrFreeze(pPoint, clearAfterFreeze));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(clearAfterFreeze);
  return(TMWDEFS_FALSE);
#endif
}

/* function: sdnpdata_binCntrFreezeAtTime */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binCntrFreezeAtTime(
  void *pPoint,
  DNPDATA_FREEZE_TIME_DATE_FIELD timeDateEnum,
  TMWDTIME *pFreezeTime,
  TMWTYPES_ULONG freezeInterval)
{
  /* According to Table 4.11, if the Time-date field and the Interval field are both
   * zero, then Freeze once immediately.
   */
  if ((timeDateEnum == DNPDATA_FREEZE_TIME_DATE_FIELD_ZERO) && (freezeInterval == 0))
  {
    return (sdnpdata_binCntrFreeze(pPoint, TMWDEFS_FALSE));
  }
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_binCntrFreezeAtTime(pPoint, timeDateEnum, pFreezeTime, freezeInterval));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_BinCntrFreezeAtTime(pPoint, timeDateEnum, pFreezeTime, freezeInterval));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(timeDateEnum);
  TMWTARG_UNUSED_PARAM(pFreezeTime);
  TMWTARG_UNUSED_PARAM(freezeInterval);
  return(TMWDEFS_FALSE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_XML || SDNPDATA_SUPPORT_XML2
/* function: sdnpdata_frznCntrGetDescription */
TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_frznCntrGetDescription(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(tmwsim_getDescription((TMWSIM_POINT *)pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_FrznCntrGetDescription((TMWSIM_POINT *)pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_NULL);
#endif
}
#endif

/* function: sdnpdata_frznCntrQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_frznCntrQuantity(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_frznCntrQuantity(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_FrznCntrQuantity(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(0);
#endif
}

/* function: sdnpdata_frznCntrGetPoint */
void * TMWDEFS_GLOBAL sdnpdata_frznCntrGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_frznCntrGetEnabledPoint(pHandle, pointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_FrznCntrGetPoint(pHandle, pointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNum);
  return(TMWDEFS_NULL);
#endif
}

#if SDNPDATA_SUPPORT_CLASS0_POINT
/* function: sdnpdata_FrznCntrIsClass0 */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_frznCntrIsClass0(
  void *pPoint)
{  
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_frznCntrInClass0(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_FrznCntrIsInClass0(pPoint));
#else  
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_TRUE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpdata_frznCntrAssignClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_frznCntrAssignClass(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_frznCntrSetEventClass(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_FrznCntrAssignClass(pPoint, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(TMWDEFS_FALSE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
/* function: sdnpdata_frznCntrDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_frznCntrDefVariation(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_frznCntrDefVariation(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_FrznCntrDefVariation(pPoint));
#else
  TMWTARG_UNUSED_PARAM(pPoint);
  return(9);
#endif
}
#endif

/* function: sdnpdata_frznCntrRead */
void TMWDEFS_GLOBAL sdnpdata_frznCntrRead(
  void *pPoint,
  TMWTYPES_ULONG *pValue,
  TMWTYPES_UCHAR *pFlags,
  TMWDTIME *pTimeOfFreeze)
{
#if TMWCNFG_USE_SIMULATED_DB
  sdnpsim_frznCntrRead(pPoint, pValue, pFlags, pTimeOfFreeze);
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_FrznCntrRead(pPoint, pValue, pFlags, pTimeOfFreeze);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pValue);
  TMWTARG_UNUSED_PARAM(pFlags);
  TMWTARG_UNUSED_PARAM(pTimeOfFreeze);
#endif
  return;
}

#if SDNPDATA_SUPPORT_OBJ23
/* function: sdnpdata_frznCntrEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_frznCntrEventClass(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_frznCntrGetEventClass(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_FrznCntrEventClass(pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_CLASS_MASK_NONE);
#endif
}

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
/* function: sdnpdata_frznCntrEventDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_frznCntrEventDefVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_frznCntrEventDefVariation(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_FrznCntrEventDefVariation(pPoint, classMask));
#else
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(1);
#endif
}
#endif

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_frznCntrEventMode(
  void *pPoint)
{
  #if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventMode(pPoint));  
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_EventMode(pPoint, TMWDEFS_FALSE));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_EVENT_MODE_SOE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_EVENT_SCAN
/* function: sdnpdata_frznCntrChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_frznCntrChanged(
  void *pPoint,
  TMWTYPES_ULONG *pValue,
  TMWTYPES_UCHAR *pFlags)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_frznCntrChanged(pPoint, pValue, pFlags));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_FrznCntrChanged(pPoint, pValue, pFlags));
#else

  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pValue);
  TMWTARG_UNUSED_PARAM(pFlags);
  return(TMWDEFS_FALSE);
#endif
}
#endif
#endif
#endif

#if SDNPDATA_SUPPORT_OBJ30
#if SDNPDATA_SUPPORT_XML || SDNPDATA_SUPPORT_XML2
/* function: sdnpdata_anlgInGetDescription */
TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_anlgInGetDescription(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(tmwsim_getDescription((TMWSIM_POINT *)pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgInGetDescription((TMWSIM_POINT *)pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_NULL);
#endif
}
#endif

/* function: sdnpdata_anlgInQuantity */
TMWTYPES_USHORT TMWDEFS_CALLBACK sdnpdata_anlgInQuantity(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_anlgInQuantity(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgInQuantity(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(0);
#endif
}

/* function: sdnpdata_anlgInGetPoint */
void * TMWDEFS_CALLBACK sdnpdata_anlgInGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_anlgInGetEnabledPoint(pHandle, pointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgInGetPoint(pHandle, pointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNum);
  return(TMWDEFS_NULL);
#endif
}  
 
/* function: sdnpdata_anlgInGetFreezePoint */
void * TMWDEFS_CALLBACK sdnpdata_anlgInGetFreezePoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_anlgInGetPoint(pHandle, pointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgInGetFreezePoint(pHandle, pointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNum);
  return(TMWDEFS_NULL);
#endif
}

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
/* function: sdnpdata_anlgInDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_anlgInDefVariation(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_anlgInDefVariation(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgInDefVariation(pPoint));
#else
  TMWTARG_UNUSED_PARAM(pPoint);
  return(3);
#endif
}
#endif

#if SDNPDATA_SUPPORT_OBJ32
#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
/* function: sdnpdata_anlgInEventDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_anlgInEventDefVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_anlgInEventDefVariation(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgInEventDefVariation(pPoint, classMask));
#else
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(1);
#endif
}
#endif

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_anlgInEventMode(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventMode(pPoint));  
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_EventMode(pPoint, TMWDEFS_FALSE));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_EVENT_MODE_SOE);
#endif
}
#endif

/* function: sdnpdata_anlgInEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_anlgInEventClass(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventClass(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgInEventClass(pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_CLASS_MASK_NONE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpdata_anlgInAssignClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgInAssignClass(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_setEventClass(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgInAssignClass(pPoint, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(TMWDEFS_FALSE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_EVENT_SCAN
/* function: sdnpdata_anlgInChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgInChanged(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR *pFlags)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_anlgInChanged(pPoint, pValue, pFlags));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgInChanged(pPoint, pValue, pFlags));
#else
  /* Put target code here */
  /* See NOTE in sdnpdata_anlgInRead() above */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pValue);
  TMWTARG_UNUSED_PARAM(pFlags);
  return(TMWDEFS_FALSE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_CLASS0_POINT
/* function: sdnpdata_anlgInIsClass0 */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgInIsClass0(
  void *pPoint)
{  
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_inClass0(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_IsInClass0(pPoint, TMWDEFS_FALSE));
#else  
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_TRUE);
#endif
}
#endif

/* function: sdnpdata_anlgInRead */
void TMWDEFS_GLOBAL sdnpdata_anlgInRead(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR *pFlags)
{
#if TMWCNFG_USE_SIMULATED_DB
  sdnpsim_anlgInRead(pPoint, pValue, pFlags);
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_AnlgInRead(pPoint, pValue, pFlags);
#else
  /* Put target code here */
  /* NOTE:  
   *  If TMWCNFG_SUPPORT_DOUBLE == TRUE returning the value as
   *  pValue->type = TMWTYPES_ANALOG_TYPE_DOUBLE always allows proper
   *  data precision.
   *
   *  If TMWCNFG_SUPPORT_DOUBLE == FALSE and 
   *     TMWCNFG_SUPPORT_SFLOAT == TRUE  
   * To provide for best precision and OVER_RANGE flag setting:
   * return the value as TMWTYPES_ANALOG_TYPE_LONG if the value is between
   * TMWDEFS_LONG_MIN and TMWDEFS_LONG_MAX. 
   * return the value as TMWTYPES_ANALOG_TYPE_SFLOAT if the value is outside
   * of that range. While each type is 32 bits in size the long allows 
   * for 31 bits of precision. Returning a float instead allows greater values,
   * but only 23 bits of precision. 
   * If both TMWCNFG_SUPPORT_DOUBLE and TMWCNFG_SUPPORT_SFLOAT == FALSE and
   * the long value is over or under range, set DNPDEFS_DBAS_FLAG_OVER_RANGE 
   * bit in pFlags and set lval to TMWDEFS_LONG_MIN or TMWDEFS_LONG_MAX.
   */  
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pValue);
  TMWTARG_UNUSED_PARAM(pFlags);
#endif  
  return;
}
#endif

#if SDNPDATA_SUPPORT_OBJ31
/* function: sdnpdata_anlgInFreeze */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgInFreeze(
  void *pPoint,
  TMWTYPES_BOOL clearAfterFreeze)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_anlgInFreeze(pPoint, clearAfterFreeze));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgInFreeze(pPoint, clearAfterFreeze));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(clearAfterFreeze);
  return(TMWDEFS_FALSE);
#endif
}

/* function: sdnpdata_anlgInFreezeAtTime */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgInFreezeAtTime(
  void *pPoint,
  DNPDATA_FREEZE_TIME_DATE_FIELD timeDateEnum,
  TMWDTIME *pFreezeTime,
  TMWTYPES_ULONG freezeInterval)
{
  /* According to Table 4.11, if the Time-date field and the Interval field are both
   * zero, then Freeze once immediately.
   */
  if ((timeDateEnum == DNPDATA_FREEZE_TIME_DATE_FIELD_ZERO) && (freezeInterval == 0))
  {
    return (sdnpdata_anlgInFreeze(pPoint, TMWDEFS_FALSE));
  }
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_anlgInFreezeAtTime(pPoint, timeDateEnum, pFreezeTime, freezeInterval));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgInFreezeAtTime(pPoint, timeDateEnum, pFreezeTime, freezeInterval));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(timeDateEnum);
  TMWTARG_UNUSED_PARAM(pFreezeTime);
  TMWTARG_UNUSED_PARAM(freezeInterval);
  return(TMWDEFS_FALSE);
#endif
}

#if SDNPDATA_SUPPORT_XML || SDNPDATA_SUPPORT_XML2
/* function: sdnpdata_frznAnlgInGetDescription */
TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_frznAnlgInGetDescription(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(tmwsim_getDescription((TMWSIM_POINT *)pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_FrznAnlgInGetDescription((TMWSIM_POINT *)pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_NULL);
#endif
}
#endif

/* function: sdnpdata_frznAnlgInQuantity */
TMWTYPES_USHORT TMWDEFS_CALLBACK sdnpdata_frznAnlgInQuantity(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_frznAnlgInQuantity(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_FrznAnlgInQuantity(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(0);
#endif
}

/* function: sdnpdata_frznAnlgInGetPoint */
void * TMWDEFS_CALLBACK sdnpdata_frznAnlgInGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_frznAnlgInGetPoint(pHandle, pointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_FrznAnlgInGetPoint(pHandle, pointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNum);
  return(TMWDEFS_NULL);
#endif
}  

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
/* function: sdnpdata_frznAnlgInDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_frznAnlgInDefVariation(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_frznAnlgInDefVariation(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_FrznAnlgInDefVariation(pPoint));
#else
  TMWTARG_UNUSED_PARAM(pPoint);
  return(3);
#endif
}
#endif

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
/* function: sdnpdata_frznAnlgInEventDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_frznAnlgInEventDefVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_frznAnlgInEventDefVariation(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_FrznAnlgInEventDefVariation(pPoint, classMask));
#else
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(1);
#endif
}
#endif

#if SDNPDATA_SUPPORT_CLASS0_POINT
/* function: sdnpdata_frznAnlgInIsClass0 */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_frznAnlgInIsClass0(
  void *pPoint)
{  
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_inClass0(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_IsInClass0(pPoint, TMWDEFS_FALSE));
#else  
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_TRUE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpdata_frznAnlgInAssignClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_frznAnlgInAssignClass(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_setEventClass(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_FrznAnlgInAssignClass(pPoint, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(TMWDEFS_FALSE);
#endif
}
#endif

/* function: sdnpdata_frznAnlgInRead */
void TMWDEFS_GLOBAL sdnpdata_frznAnlgInRead(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR *pFlags,
  TMWDTIME *pTimeOfFreeze)
{
#if TMWCNFG_USE_SIMULATED_DB
  sdnpsim_frznAnlgInRead(pPoint, pValue, pFlags, pTimeOfFreeze);
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_FrznAnlgInRead(pPoint, pValue, pFlags, pTimeOfFreeze);
#else
  /* Put target code here */
  /* NOTE:  
   *  If TMWCNFG_SUPPORT_DOUBLE == TRUE returning the value as
   *  pValue->type = TMWTYPES_ANALOG_TYPE_DOUBLE always allows proper
   *  data precision.
   *
   *  If TMWCNFG_SUPPORT_DOUBLE == FALSE and 
   *     TMWCNFG_SUPPORT_SFLOAT == TRUE  
   * To provide for best precision and OVER_RANGE flag setting:
   * return the value as TMWTYPES_ANALOG_TYPE_LONG if the value is between
   * TMWDEFS_LONG_MIN and TMWDEFS_LONG_MAX. 
   * return the value as TMWTYPES_ANALOG_TYPE_SFLOAT if the value is outside
   * of that range. While each type is 32 bits in size the long allows 
   * for 31 bits of precision. Returning a float instead allows greater values,
   * but only 23 bits of precision. 
   * If both TMWCNFG_SUPPORT_DOUBLE and TMWCNFG_SUPPORT_SFLOAT == FALSE and
   * the long value is over or under range, set DNPDEFS_DBAS_FLAG_OVER_RANGE 
   * bit in pFlags and set lval to TMWDEFS_LONG_MIN or TMWDEFS_LONG_MAX.
   */  
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pValue);
  TMWTARG_UNUSED_PARAM(pFlags);
  TMWTARG_UNUSED_PARAM(pTimeOfFreeze);
#endif  
  return;
}

#if SDNPDATA_SUPPORT_OBJ33
#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_frznAnlgInEventMode(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventMode(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_EventMode(pPoint, TMWDEFS_FALSE));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_EVENT_MODE_SOE);
#endif
}
#endif

/* function: sdnpdata_frznAnlgInEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_frznAnlgInEventClass(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventClass(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_FrznAnlgInEventClass(pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_CLASS_MASK_NONE);
#endif
}

#if SDNPDATA_SUPPORT_EVENT_SCAN
/* function: sdnpdata_frznAnlgInChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_frznAnlgInChanged(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR *pFlags)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_frznAnlgInChanged(pPoint, pValue, pFlags));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_FrznAnlgInChanged(pPoint, pValue, pFlags));
#else
  /* Put target code here */
  /* See NOTE in sdnpdata_frznAnlgInRead() above */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pValue);
  TMWTARG_UNUSED_PARAM(pFlags);
  return(TMWDEFS_FALSE);
#endif
}
#endif
#endif
#endif

#if SDNPDATA_SUPPORT_OBJ34
#if SDNPDATA_SUPPORT_XML || SDNPDATA_SUPPORT_XML2
/* function: sdnpdata_anlgInDBandGetDescription */
TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_anlgInDBandGetDescription(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(tmwsim_getDescription((TMWSIM_POINT *)pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgInDBandGetDescription((TMWSIM_POINT *)pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_NULL);
#endif
}
#endif

/* function: sdnpdata_anlgInDBandQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_anlgInDBandQuantity(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_anlgInQuantity(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgInDBandQuantity(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(0);
#endif
}

/* function: sdnpdata_anlgInDBandGetPoint */
void * TMWDEFS_GLOBAL sdnpdata_anlgInDBandGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_anlgInGetEnabledPoint(pHandle, pointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgInDBandGetPoint(pHandle, pointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNum);
  return(TMWDEFS_NULL);
#endif
}

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
/* function: sdnpdata_anlgInDbandDefVar */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_anlgInDbandDefVar(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_anlgInDbandDefVar(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgInDbandDefVar(pPoint));
#else
  TMWTARG_UNUSED_PARAM(pPoint);
  return(2);
#endif
} 
#endif

/* function: sdnpdata_anlgInDBandRead */
void TMWDEFS_GLOBAL sdnpdata_anlgInDBandRead(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pValue)
{ 
  /* Get the value for this object and set the pValue->type accordingly. 
   * NOTE:  
   *  If TMWCNFG_SUPPORT_DOUBLE == TRUE returning the value as
   *  pValue->type = TMWTYPES_ANALOG_TYPE_DOUBLE always allows proper
   *  data precision.
   *
   *  If TMWCNFG_SUPPORT_DOUBLE == FALSE and 
   *     TMWCNFG_SUPPORT_SFLOAT == TRUE  
   * To provide for best precision:
   * return the value as TMWTYPES_ANALOG_TYPE_ULONG if the value is between
   * 0 and TMWDEFS_ULONG_MAX. 
   * return the value as TMWTYPES_ANALOG_TYPE_SFLOAT if the value is outside
   * of that range. While each is 32 bits in size the ulong allows 
   * for 32 bits of precision. Returning a float instead allows greater values,
   * but only 23 bits of precision.
   * If both TMWCNFG_SUPPORT_DOUBLE and TMWCNFG_SUPPORT_SFLOAT == FALSE and
   * the long value exceeds TMWDEFS_ULONG_MAX, return TMWDEFS_ULONG_MAX.
   */
#if TMWCNFG_USE_SIMULATED_DB
  sdnpsim_anlgInGetDBand(pPoint, pValue);
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_AnlgInDBandRead(pPoint, pValue);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pValue);
#endif  
  return;
}

/* function: sdnpdata_anlgInDBandWrite */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgInDBandWrite(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pValue)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_anlgInSetDBand(pPoint, pValue));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgInDBandWrite(pPoint, pValue));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pValue);
  return(TMWDEFS_FALSE);
#endif
}
#endif

#if (SDNPDATA_SUPPORT_OBJ40 || SDNPDATA_SUPPORT_OBJ41)
#if SDNPDATA_SUPPORT_XML || SDNPDATA_SUPPORT_XML2
/* function: sdnpdata_anlgOutGetDescription */
TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_anlgOutGetDescription(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(tmwsim_getDescription((TMWSIM_POINT *)pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgOutGetDescription((TMWSIM_POINT *)pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_NULL);
#endif
}
#endif

/* function: sdnpdata_anlgOutQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_anlgOutQuantity(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_anlgOutQuantity(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgOutQuantity(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(0);
#endif
}

/* function: sdnpdata_anlgOutGetPoint */
void * TMWDEFS_GLOBAL sdnpdata_anlgOutGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_anlgOutGetEnabledPoint(pHandle, pointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgOutGetPoint(pHandle, pointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNum);
  return(TMWDEFS_NULL);
#endif
}

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
/* function: sdnpdata_anlgOutDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_anlgOutDefVariation(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_anlgOutDefVariation(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgOutDefVariation(pPoint));
#else
  TMWTARG_UNUSED_PARAM(pPoint);
  return(2);
#endif
}
#endif

#if SDNPDATA_SUPPORT_OBJ42
#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
/* function: sdnpdata_anlgOutEventDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_anlgOutEventDefVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_anlgOutEventDefVariation(pPoint, classMask);
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgOutEventDefVariation(pPoint, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(2);
#endif
}
#endif

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_anlgOutEventMode(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventMode(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_EventMode(pPoint, TMWDEFS_TRUE));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_EVENT_MODE_SOE);
#endif
}
#endif

/* function: sdnpdata_anlgOutEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_anlgOutEventClass(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventClass(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgOutEventClass(pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_CLASS_MASK_NONE);
#endif
}

#if SDNPDATA_SUPPORT_EVENT_SCAN
/* function: sdnpdata_anlgOutChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgOutChanged(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR *pFlags)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_anlgOutChanged(pPoint, pValue, pFlags);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_AnlgOutChanged(pPoint, pValue, pFlags);
#else
  /* Put target code here */
  /* See NOTE in sdnpdata.h for sdnpdata_anlgOutRead() above */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pValue);
  TMWTARG_UNUSED_PARAM(pFlags);
  return(TMWDEFS_FALSE);
#endif
}
#endif
#endif

#if SDNPDATA_SUPPORT_CLASS0_POINT
/* function: sdnpdata_anlgOutIsClass0 */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgOutIsClass0(
  void *pPoint)
{  
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_inClass0(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_IsInClass0(pPoint, TMWDEFS_TRUE));
#else  
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_TRUE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpdata_anlgOutAssignClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgOutAssignClass(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_setEventClass(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgOutAssignClass(pPoint, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(TMWDEFS_FALSE);
#endif
}
#endif

/* function: sdnpdata_anlgOutRead */
void TMWDEFS_GLOBAL sdnpdata_anlgOutRead(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR *pFlags)
{
#if TMWCNFG_USE_SIMULATED_DB
  sdnpsim_anlgOutRead(pPoint, pValue, pFlags);
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_AnlgOutRead(pPoint, pValue, pFlags);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pValue);
  TMWTARG_UNUSED_PARAM(pFlags);
#endif
  return;
}

/* function: sdnpdata_anlgOutSelect */
DNPDEFS_CTLSTAT TMWDEFS_GLOBAL sdnpdata_anlgOutSelect(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pValue)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_anlgOutSelect(pPoint, pValue));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgOutSelect(pPoint, pValue));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pValue);
  return(DNPDEFS_CTLSTAT_NOT_SUPPORTED);
#endif
}

/* function: sdnpdata_anlgOutOperate */
DNPDEFS_CTLSTAT TMWDEFS_GLOBAL sdnpdata_anlgOutOperate(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pValue)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_anlgOutOperate(pPoint, pValue));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgOutOperate(pPoint, pValue));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pValue);
  return(DNPDEFS_CTLSTAT_NOT_SUPPORTED);
#endif
}

#if SDNPDATA_SUPPORT_SELECT_CANCEL
/* function: sdnpdata_anlgOutCancelSelect */
void TMWDEFS_GLOBAL sdnpdata_anlgOutCancelSelect(
  void *pPoint)
{
#if TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgOutCancelSelect(pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
#endif
}
#endif

#if SDNPDATA_SUPPORT_OBJ43
#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
/* function: sdnpdata_anlgOutCmdEventDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_anlgOutCmdEventDefVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_anlgOutEventDefVariation(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgOutCmdEventDefVariation(pPoint, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(2);
#endif
}
#endif

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_anlgOutCmdEventMode(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventMode(pPoint));  
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_EventMode(pPoint, TMWDEFS_TRUE));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_EVENT_MODE_SOE);
#endif
}
#endif

/* function: sdnpdata_anlgOutCmdEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_anlgOutCmdEventClass(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventClass(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgOutCmdEventClass(pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_CLASS_MASK_NONE);
#endif
}

#if SDNPDATA_SUPPORT_EVENT_SCAN
/* function: sdnpdata_anlgOutCmdChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgOutCmdChanged(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR *pStatus)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pValue);
  TMWTARG_UNUSED_PARAM(pStatus);
  return(TMWDEFS_FALSE);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_AnlgOutCmdChanged(pPoint, pValue, pStatus);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pValue);
  TMWTARG_UNUSED_PARAM(pStatus);
  return(TMWDEFS_FALSE);
#endif
}
#endif
#endif

#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpdata_anlgOutCmdAssignClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgOutCmdAssignClass(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_setCmdEventClass(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AnlgOutCmdAssignClass(pPoint, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(TMWDEFS_FALSE);
#endif
}
#endif
#endif

#if SDNPDATA_SUPPORT_OBJ3
/* Double Bit Inputs */

#if SDNPDATA_SUPPORT_XML || SDNPDATA_SUPPORT_XML2
/* function: sdnpdata_dblInGetDescription */
TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_dblInGetDescription(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(tmwsim_getDescription((TMWSIM_POINT *)pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DblInGetDescription((TMWSIM_POINT *)pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_NULL);
#endif
}
#endif

/* function: sdnpdata_dblInQuantity */
TMWTYPES_USHORT TMWDEFS_CALLBACK sdnpdata_dblInQuantity(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_dblInQuantity(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DblInQuantity(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(0);
#endif
}

/* function: sdnpdata_dblInGetPoint */
void * TMWDEFS_CALLBACK sdnpdata_dblInGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_dblInGetEnabledPoint(pHandle, pointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DblInGetPoint(pHandle, pointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNum);
  return(TMWDEFS_NULL);
#endif
}

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
/* function: sdnpdata_dblInDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_dblInDefVariation(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_dblInDefVariation(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DblInDefVariation(pPoint));
#else
  TMWTARG_UNUSED_PARAM(pPoint);
  return(1);
#endif
}  
#endif

#if SDNPDATA_SUPPORT_OBJ4
#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_dblInEventMode(
  void *pPoint)
{
  #if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventMode(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_EventMode(pPoint, TMWDEFS_FALSE));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_EVENT_MODE_SOE);
#endif
}
#endif

/* function: sdnpdata_dblInEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_dblInEventClass(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventClass(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DblInEventClass(pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_CLASS_MASK_NONE);
#endif
}

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
/* function: sdnpdata_dblInEventDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_dblInEventDefVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_dblInEventDefVariation(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DblInEventDefVariation(pPoint, classMask));
#else
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(3);
#endif
}
#endif
#endif

#if SDNPDATA_SUPPORT_CLASS0_POINT
/* function: sdnpdata_dblInIsClass0 */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_dblInIsClass0(
  void *pPoint)
{  
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_inClass0(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_IsInClass0(pPoint, TMWDEFS_FALSE));
#else  
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_TRUE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpdata_dblInAssignClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_dblInAssignClass(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_setEventClass(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DblInAssignClass(pPoint, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(TMWDEFS_FALSE);
#endif
}
#endif

/* function: sdnpdata_dblInRead */
void TMWDEFS_GLOBAL sdnpdata_dblInRead(
  void *pPoint,
  TMWTYPES_UCHAR *pFlags)
{
#if TMWCNFG_USE_SIMULATED_DB
  sdnpsim_dblInRead(pPoint, pFlags);
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_DblInRead(pPoint, pFlags);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pFlags);
#endif

  return;
}

#if SDNPDATA_SUPPORT_EVENT_SCAN
/* function: sdnpdata_dblInChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_dblInChanged(
  void *pPoint,
  TMWTYPES_UCHAR *pFlags)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_dblInChanged(pPoint, pFlags));
 
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DblInChanged(pPoint, pFlags));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pFlags);
  return(TMWDEFS_FALSE);
#endif
}
#endif
#endif

#if SDNPDATA_SUPPORT_OBJ50_V4
/* Time and date - indexed abs time and long interval */
/* function sdnpdata_indexedTimeGetDescription */
TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_indexedTimeGetDescription(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(tmwsim_getDescription((TMWSIM_POINT *)pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_IndexedTimeGetDescription((TMWSIM_POINT *)pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_NULL);
#endif
}

/* function sdnpdata_indexedTimeQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_indexedTimeQuantity(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_indexedTimeQuantity(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_IndexedTimeQuantity(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(0);
#endif
}
/* function sdnpdata_indexedTimeGetPoint */
void * TMWDEFS_GLOBAL sdnpdata_indexedTimeGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_indexedTimeGetEnabledPoint(pHandle, pointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_IndexedTimeGetPoint(pHandle, pointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNum);
  return(TMWDEFS_NULL);
#endif
}

/* function sdnpdata_indexedTimeIsClass0 */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_indexedTimeIsClass0(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_inClass0(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_IsInClass0(pPoint, TMWDEFS_TRUE));
#else  
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_TRUE);
#endif
}

/* function sdnpdata_indexedTimeRead */
TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpdata_indexedTimeRead(
  void *pPoint,
  TMWDTIME *pTimeStamp,
  TMWTYPES_ULONG *pIntervalCount,
  TMWTYPES_BYTE  *pIntervalUnit)
{
#if TMWCNFG_USE_SIMULATED_DB
  sdnpsim_indexedTimeRead(pPoint, pTimeStamp, pIntervalCount, pIntervalUnit);
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_IndexedTimeRead(pPoint, pTimeStamp, pIntervalCount, pIntervalUnit);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pTimeStamp);
  TMWTARG_UNUSED_PARAM(pIntervalCount);
  TMWTARG_UNUSED_PARAM(pIntervalUnit);
  return(TMWDEFS_FALSE);
#endif
}
/* function sdnpdata_indexedTimeWrite */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_indexedTimeWrite(
  void *pPoint,
  TMWDTIME pTimeStamp,
  TMWTYPES_ULONG pIntervalCount,
  TMWTYPES_BYTE  pIntervalUnit)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_indexedTimeWrite(pPoint, pTimeStamp, pIntervalCount, pIntervalUnit);
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_IndexedTimeWrite(pPoint, &pTimeStamp, &pIntervalCount, &pIntervalUnit));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pTimeStamp);
  TMWTARG_UNUSED_PARAM(pIntervalCount);
  TMWTARG_UNUSED_PARAM(pIntervalUnit);
  return(TMWDEFS_FALSE);
#endif
}

#endif
#if SDNPDATA_SUPPORT_OBJ110
#if SDNPDATA_SUPPORT_XML || SDNPDATA_SUPPORT_XML2
/* Strings */
/* function: sdnpdata_strDescription */
TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_strGetDescription(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(tmwsim_getDescription((TMWSIM_POINT *)pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_StrGetDescription((TMWSIM_POINT *)pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_NULL);
#endif
}
#endif

/* function: sdnpdata_strQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_strQuantity(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_strQuantity(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_StrQuantity(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(0);
#endif
}

/* function: sdnpdata_strGetPoint */
void * TMWDEFS_GLOBAL sdnpdata_strGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_strGetEnabledPoint(pHandle, pointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_StrGetPoint(pHandle, pointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNum);
  return(TMWDEFS_NULL);
#endif
}

#if SDNPDATA_SUPPORT_OBJ111
#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
/* function: sdnpdata_strEventMode */
TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_strEventMode(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventMode(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_EventMode(pPoint, TMWDEFS_FALSE));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_EVENT_MODE_SOE);
#endif
}
#endif

/* function: sdnpdata_strEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_strEventClass(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventClass(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_StrEventClass(pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_CLASS_MASK_NONE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpdata_strAssignClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_strAssignClass(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_setEventClass(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_StrAssignClass(pPoint, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(TMWDEFS_FALSE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_CLASS0_POINT
/* function: sdnpdata_strIsClass0 */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_strIsClass0(
  void *pPoint)
{  
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_inClass0(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_IsInClass0(pPoint, TMWDEFS_FALSE));
#else  
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_TRUE);
#endif
}
#endif

/* function: sdnpdata_strRead */
void TMWDEFS_GLOBAL sdnpdata_strRead(
  void *pPoint,
  TMWTYPES_UCHAR maxLength,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR *pLength)
{
#if TMWCNFG_USE_SIMULATED_DB
  sdnpsim_strRead(pPoint, maxLength, pBuf, pLength);
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_StrRead(pPoint, maxLength, pBuf, pLength);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(maxLength);
  TMWTARG_UNUSED_PARAM(pBuf);
  TMWTARG_UNUSED_PARAM(pLength);
#endif 
  return;
}

/* function: sdnpdata_strWrite */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_strWrite(
  void *pPoint,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR bufLength)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_strWrite(pPoint, pBuf, bufLength));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_StrWrite(pPoint, pBuf, bufLength));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pBuf);
  TMWTARG_UNUSED_PARAM(bufLength);
  return(TMWDEFS_FALSE);
#endif
}

#if SDNPDATA_SUPPORT_EVENT_SCAN
/* function: sdnpdata_strChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_strChanged(
  void *pPoint,
  TMWTYPES_UCHAR maxLength,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR *pLength)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_strChanged(pPoint, maxLength, pBuf, pLength));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_StrChanged(pPoint, maxLength, pBuf, pLength));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(maxLength);
  TMWTARG_UNUSED_PARAM(pBuf);
  TMWTARG_UNUSED_PARAM(pLength);
  return(TMWDEFS_FALSE);
#endif
}
#endif
#endif

#if SDNPDATA_SUPPORT_OBJ112
#if SDNPDATA_SUPPORT_XML || SDNPDATA_SUPPORT_XML2
/* Virtual Terminal */
/* function: sdnpdata_vtermGetDescription */
TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_vtermGetDescription(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(tmwsim_getDescription((TMWSIM_POINT *)pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_VtermGetDescription((TMWSIM_POINT *)pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_NULL);
#endif
}
#endif

TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_vtermQuantity(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_vtermQuantity(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_VtermQuantity(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(0);
#endif
}

void * TMWDEFS_GLOBAL sdnpdata_vtermGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_vtermGetEnabledPoint(pHandle, pointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_VtermGetPoint(pHandle, pointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNum);
  return(TMWDEFS_NULL);
#endif
}

#if SDNPDATA_SUPPORT_OBJ113
#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
/* function: sdnpdata_vtermEventMode */
TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_vtermEventMode(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventMode(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_EventMode(pPoint, TMWDEFS_FALSE));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_EVENT_MODE_SOE);
#endif
} 
#endif

TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_vtermEventClass(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventClass(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_VtermEventClass(pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_CLASS_MASK_NONE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_ASSIGN
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_vtermAssignClass(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_setEventClass(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_VtermAssignClass(pPoint, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(TMWDEFS_FALSE);
#endif
}
#endif

void TMWDEFS_GLOBAL sdnpdata_vtermRead(
  void *pPoint,
  TMWTYPES_UCHAR maxLength,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR *pLength)
{
#if TMWCNFG_USE_SIMULATED_DB
  sdnpsim_vtermRead(pPoint, maxLength, pBuf, pLength);
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_VtermRead(pPoint, maxLength, pBuf, pLength);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(maxLength);
  TMWTARG_UNUSED_PARAM(pBuf);
  TMWTARG_UNUSED_PARAM(pLength);
#endif
  return;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_vtermWrite(
  void *pPoint,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR bufLength)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_vtermWrite(pPoint, pBuf, bufLength));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_VtermWrite(pPoint, pBuf, bufLength));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pBuf);
  TMWTARG_UNUSED_PARAM(bufLength);
  return(TMWDEFS_FALSE);
#endif
}

#if SDNPDATA_SUPPORT_EVENT_SCAN
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_vtermChanged(
  void *pPoint,
  TMWTYPES_UCHAR maxLength,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR *pLength)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_vtermChanged(pPoint, maxLength, pBuf, pLength));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_VtermChanged(pPoint, maxLength, pBuf, pLength));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(maxLength);
  TMWTARG_UNUSED_PARAM(pBuf);
  TMWTARG_UNUSED_PARAM(pLength);
  return(TMWDEFS_FALSE);
#endif
}
#endif
#endif

#if SDNPDATA_SUPPORT_OBJ114
#if SDNPDATA_SUPPORT_XML || SDNPDATA_SUPPORT_XML2
/* Extended Strings */
/* function: sdnpdata_extStrDescription */
TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_extStrGetDescription(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(tmwsim_getDescription((TMWSIM_POINT *)pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_ExtStrGetDescription((TMWSIM_POINT *)pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_NULL);
#endif
}
#endif

/* function: sdnpdata_extStrQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_extStrQuantity(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_extStrQuantity(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_ExtStrQuantity(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(0);
#endif
}

/* function: sdnpdata_extStrGetPoint */
void * TMWDEFS_GLOBAL sdnpdata_extStrGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_extStrGetEnabledPoint(pHandle, pointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_ExtStrGetPoint(pHandle, pointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNum);
  return(TMWDEFS_NULL);
#endif
}

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
/* function: sdnpdata_extStrDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_extStrDefVariation(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_extStrDefVariation(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_ExtStrDefVariation(pPoint));
#else
  TMWTARG_UNUSED_PARAM(pPoint);
  return(1);
#endif
}
#endif

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
/* function: sdnpdata_extStrEventDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_extStrEventDefVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getDefEventVariation(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_ExtStrEventDefVariation(pPoint, classMask));
#else
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(3);
#endif
}
#endif

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
/* function: sdnpdata_extStrEventMode */
TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_extStrEventMode(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventMode(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_EventMode(pPoint, TMWDEFS_FALSE));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_EVENT_MODE_SOE);
#endif
} 
#endif

/* function: sdnpdata_extStrEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_extStrEventClass(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventClass(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_ExtStrEventClass(pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_CLASS_MASK_NONE);
#endif
}

#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpdata_extStrAssignClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_extStrAssignClass(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_setEventClass(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_ExtStrAssignClass(pPoint, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(TMWDEFS_FALSE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_CLASS0_POINT
/* function: sdnpdata_extStrIsClass0 */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_extStrIsClass0(
  void *pPoint)
{  
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_inClass0(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_IsInClass0(pPoint, TMWDEFS_FALSE));
#else  
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_TRUE);
#endif
}
#endif


#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
/* function: sdnpdata_extStrGetPtr */
TMWTYPES_UCHAR * TMWDEFS_GLOBAL sdnpdata_extStrGetPtr(
  void *pPoint,
  TMWTYPES_USHORT *pLength,
  TMWTYPES_UCHAR *pFlags)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_extStrGetPtr(pPoint, pLength, pFlags));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_ExtStrGetPointer(pPoint, pLength, pFlags));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pLength);
  return(TMWDEFS_NULL);
#endif 
}

/* function: sdnpdata_extStrRelease */
void TMWDEFS_GLOBAL sdnpdata_extStrRelease(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pPoint); 
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_ExtStrRelease(pPoint);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint); 
#endif
}

#else
/* function: sdnpdata_extStrRead */
void TMWDEFS_GLOBAL sdnpdata_extStrRead(
  void *pPoint,
  TMWTYPES_USHORT maxLength,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT *pLength,
  TMWTYPES_UCHAR *pFlags)
{
#if TMWCNFG_USE_SIMULATED_DB
  sdnpsim_extStrRead(pPoint, maxLength, pBuf, pLength, pFlags);
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_ExtStrRead(pPoint, maxLength, pBuf, pLength, pFlags);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(maxLength);
  TMWTARG_UNUSED_PARAM(pBuf);
  TMWTARG_UNUSED_PARAM(pLength);
#endif 
  return;
}
#endif

/* function: sdnpdata_extStrWrite */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_extStrWrite(
  void *pPoint,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT bufLength)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_extStrWrite(pPoint, pBuf, bufLength));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_ExtStrWrite(pPoint, pBuf, bufLength));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pBuf);
  TMWTARG_UNUSED_PARAM(bufLength);
  return(TMWDEFS_FALSE);
#endif
}

#if SDNPDATA_SUPPORT_EVENT_SCAN
/* function: sdnpdata_extStrChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_extStrChanged(
  void *pPoint,
  TMWTYPES_USHORT maxLength,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT *pLength,
  TMWTYPES_UCHAR *pFlags)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_extStrChanged(pPoint, maxLength, pBuf, pLength, pFlags));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_ExtStrChanged(pPoint, maxLength, pBuf, pLength, pFlags));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(maxLength);
  TMWTARG_UNUSED_PARAM(pBuf);
  TMWTARG_UNUSED_PARAM(pLength);
  return(TMWDEFS_FALSE);
#endif
}
#endif
#endif

#if SDNPDATA_SUPPORT_OBJ70
/* File Transfer Support */

/* function: sdnpdata_fileEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_fileEventClass(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  SDNPSIM_DATABASE *pDb = (SDNPSIM_DATABASE *)pHandle;
  SDNPFSIM_DATABASE *pFDBHandle = (SDNPFSIM_DATABASE *)&(pDb->sdnpFsim);
  return(sdnpfsim_fileEventClass(pFDBHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_FileEventClass(pHandle);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(TMWDEFS_CLASS_MASK_NONE);
#endif
}

#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpdata_fileAssignClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_fileAssignClass(
  void *pHandle,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  SDNPSIM_DATABASE *pDb = (SDNPSIM_DATABASE *)pHandle;
  SDNPFSIM_DATABASE *pFDBHandle = (SDNPFSIM_DATABASE *)&(pDb->sdnpFsim);
  return(sdnpfsim_setFileEventClass(pFDBHandle, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_FileAssignClass(pHandle, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(classMask);
  return(TMWDEFS_FALSE);
#endif
}
#endif

/* function: sdnpdata_getFileInfo */
DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnpdata_getFileInfo(
  TMWSESN *pSession,
  TMWTYPES_CHAR *pFilename,
  DNPDEFS_FILE_TYPE *pType,
  TMWTYPES_ULONG *pSize,
  TMWDTIME *pTimeOfCreation,
  DNPDEFS_FILE_PERMISSIONS *pPermissions)
{
#if 0
  /* Sample code for restricting file access when file transfer
   * is compiled in and enabled.
   */

   /* convert to lower case for comparison */
  strlwr(pFilename);
   
  if ((strncmp(pFilename, "c:/temp", 7) != 0)
    && (strncmp(pFilename, "c:/users/public", 15) != 0))
    return DNPDEFS_FILE_CMD_STAT_DENIED;
#endif

#if TMWCNFG_USE_SIMULATED_DB
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPSIM_DATABASE *pDb = (SDNPSIM_DATABASE *)pSDNPSession->pDbHandle;
  SDNPFSIM_DATABASE *pFDBHandle = (SDNPFSIM_DATABASE *)&(pDb->sdnpFsim);
  return(sdnpfsim_getFileInfo(pFDBHandle, pFilename,
    pType, pSize, pTimeOfCreation, pPermissions));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_GetFileInfo(pSession, pFilename, pType, pSize, pTimeOfCreation, pPermissions));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pSession);
#if TMWTARG_SUPPORT_DNPFILEIO
  return sdnptarg_getFileInfo(pFilename, pType, pSize, pTimeOfCreation, pPermissions);
#else
  TMWTARG_UNUSED_PARAM(pFilename);
  TMWTARG_UNUSED_PARAM(pType);
  TMWTARG_UNUSED_PARAM(pSize);
  TMWTARG_UNUSED_PARAM(pTimeOfCreation);
  TMWTARG_UNUSED_PARAM(pPermissions);
  return(DNPDEFS_FILE_CMD_STAT_DENIED);
#endif
#endif
}

DNPDEFS_FILE_TFER_STAT TMWDEFS_GLOBAL sdnpdata_readFileInfo(
  TMWSESN *pSession,
  TMWTYPES_ULONG fileHandle,
  TMWTYPES_USHORT maxNameSize,
  TMWTYPES_CHAR *pName,
  TMWTYPES_BOOL *pLast,
  DNPDEFS_FILE_TYPE *pType,
  TMWTYPES_ULONG *pSize,
  TMWDTIME *pTimeOfCreation,
  DNPDEFS_FILE_PERMISSIONS *pPermissions)
{
#if TMWCNFG_USE_SIMULATED_DB
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPSIM_DATABASE *pDb = (SDNPSIM_DATABASE *)pSDNPSession->pDbHandle;
  SDNPFSIM_DATABASE *pFDBHandle = (SDNPFSIM_DATABASE *)&(pDb->sdnpFsim);
  return(sdnpfsim_readFileInfo(pSession, pFDBHandle, fileHandle, maxNameSize, pName, pLast, pType, pSize, pTimeOfCreation, pPermissions));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_ReadFileInfo(pSession, fileHandle, maxNameSize, pName, pLast, pType, pSize, pTimeOfCreation, pPermissions));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pSession);
#if TMWTARG_SUPPORT_DNPFILEIO
  return sdnptarg_readFileInfo(fileHandle, maxNameSize, pName, pLast, pType,
                              pSize, pTimeOfCreation, pPermissions);
#else
  TMWTARG_UNUSED_PARAM(fileHandle);
  TMWTARG_UNUSED_PARAM(maxNameSize);
  TMWTARG_UNUSED_PARAM(pName);
  TMWTARG_UNUSED_PARAM(pLast);
  TMWTARG_UNUSED_PARAM(pType);
  TMWTARG_UNUSED_PARAM(pSize);
  TMWTARG_UNUSED_PARAM(pTimeOfCreation);
  TMWTARG_UNUSED_PARAM(pPermissions);
  return(DNPDEFS_FILE_TFER_STAT_NOT_OPEN);
#endif
#endif
}

/* function: sdnpdata_getAuthentication */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_getAuthentication(
  TMWSESN *pSession,
  TMWTYPES_CHAR *pUsername,
  TMWTYPES_CHAR *pPassword,
  TMWTYPES_ULONG *pAuthKey)
{
#if TMWCNFG_USE_SIMULATED_DB
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPSIM_DATABASE *pDb = (SDNPSIM_DATABASE *)pSDNPSession->pDbHandle;
  SDNPFSIM_DATABASE *pFDBHandle = (SDNPFSIM_DATABASE *)&(pDb->sdnpFsim);
  return(sdnpfsim_getAuthentication(pFDBHandle, pUsername, pPassword, pAuthKey));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_GetAuthentication(pSession, pUsername, pPassword, pAuthKey));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pSession);
  TMWTARG_UNUSED_PARAM(pUsername);
  TMWTARG_UNUSED_PARAM(pPassword);
  TMWTARG_UNUSED_PARAM(pAuthKey);
  return(TMWDEFS_FALSE);
#endif
}

/* function: sdnpdata_deleteFile */
DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnpdata_deleteFile(
  TMWSESN *pSession,
  TMWTYPES_CHAR *pFilename,
  TMWTYPES_ULONG authKey)
{
#if 0
  /* Sample code for restricting file access when file transfer
   * is compiled in and enabled.
   */

   /* convert to lower case for comparison */
  strlwr(pFilename);

  if ((strncmp(pFilename, "c:/temp", 7) != 0)
    && (strncmp(pFilename, "c:/users/public", 15) != 0))
    return DNPDEFS_FILE_CMD_STAT_DENIED;
#endif
#if TMWCNFG_USE_SIMULATED_DB 
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPSIM_DATABASE *pDb = (SDNPSIM_DATABASE *)pSDNPSession->pDbHandle;
  SDNPFSIM_DATABASE *pFDBHandle = (SDNPFSIM_DATABASE *)&(pDb->sdnpFsim);
  return(sdnpfsim_deleteFile(pFDBHandle, pFilename, authKey));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DeleteFile(pSession, pFilename, authKey));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pSession);
  TMWTARG_UNUSED_PARAM(authKey);
#if TMWTARG_SUPPORT_DNPFILEIO
  return sdnptarg_deleteFile(pFilename);
#else
  TMWTARG_UNUSED_PARAM(pFilename);
  return(DNPDEFS_FILE_CMD_STAT_DENIED);
#endif
#endif
}

/* function: sdnpdata_readFile */
DNPDEFS_FILE_TFER_STAT TMWDEFS_GLOBAL sdnpdata_readFile(
  TMWSESN *pSession,
  TMWTYPES_ULONG handle,
  TMWTYPES_BOOL *pLast,
  TMWTYPES_USHORT *pBytesRead,
  TMWTYPES_UCHAR *pBuf)
{
#if TMWCNFG_USE_SIMULATED_DB
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPSIM_DATABASE *pDb = (SDNPSIM_DATABASE *)pSDNPSession->pDbHandle;
  SDNPFSIM_DATABASE *pFDBHandle = (SDNPFSIM_DATABASE *)&(pDb->sdnpFsim);
  return(sdnpfsim_readFile(pSession, pFDBHandle, handle, pLast, pBytesRead, pBuf));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_ReadFile(pSession, handle, pLast, pBytesRead, pBuf));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pSession);
#if TMWTARG_SUPPORT_DNPFILEIO
  return sdnptarg_readFile(handle, pLast, pBytesRead, pBuf);
#else
  TMWTARG_UNUSED_PARAM(handle);
  TMWTARG_UNUSED_PARAM(pLast);
  TMWTARG_UNUSED_PARAM(pBytesRead);
  TMWTARG_UNUSED_PARAM(pBuf);
  return(DNPDEFS_FILE_TFER_STAT_NOT_OPEN);
#endif
#endif
}

/* function: sdnpdata_confirmFileRead */
void TMWDEFS_GLOBAL sdnpdata_confirmFileRead(
  TMWSESN *pSession,
  TMWTYPES_ULONG handle)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pSession);
  TMWTARG_UNUSED_PARAM(handle);
}

#endif

#if SDNPDATA_SUPPORT_OBJ70 || SDNPDATA_SUPPORT_XML2
/* The following are also used for generating the device profile */
/* function: sdnpdata_openFile */
DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnpdata_openFile(
  TMWSESN *pSession,
  TMWTYPES_CHAR *pFilename,
  TMWTYPES_ULONG authKey,
  DNPDEFS_FILE_MODE mode,
  TMWTYPES_USHORT *pMaxBlockSize,
  DNPDEFS_FILE_PERMISSIONS *pPermissions,
  TMWDTIME *pTimeOfCreation,
  TMWTYPES_ULONG *pHandle,
  TMWTYPES_ULONG *pSize,
  DNPDEFS_FILE_TYPE *pType)
{
#if 0
  /* Sample code for restricting file open access when file transfer
   * is compiled in and enabled.
   * This may also be done in sdndata_deleteFile and sdnpdata_getFileInfo
   */

  /* convert to lower case for comparison */
  strlwr(pFilename);

  if (mode == DNPDEFS_FILE_MODE_WRITE
    || mode == DNPDEFS_FILE_MODE_APPEND)
  {
    if ((strncmp(pFilename, "c:/temp", 7) != 0)
      && (strncmp(pFilename, "c:/users/public", 15) != 0))
      return DNPDEFS_FILE_CMD_STAT_DENIED;
  }
  else /* mode == DNPDEFS_FILE_MODE_READ */
  {
    if ((strncmp(pFilename, "v:", 2) != 0)
      && (strncmp(pFilename, "c:/temp", 7) != 0)
      && (strncmp(pFilename, "c:/users/public", 15) != 0))
      return DNPDEFS_FILE_CMD_STAT_DENIED;
  } 
#endif

#if TMWCNFG_USE_SIMULATED_DB
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPSIM_DATABASE *pDb = (SDNPSIM_DATABASE *)pSDNPSession->pDbHandle;
  SDNPFSIM_DATABASE *pFDBHandle = (SDNPFSIM_DATABASE *)&(pDb->sdnpFsim);
  return(sdnpfsim_openFile(pSession, pFDBHandle, pFilename, authKey, mode,
    pMaxBlockSize, pPermissions, pTimeOfCreation, pHandle, pSize, pType));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_OpenFile(pSession, pFilename, authKey, mode,
    pMaxBlockSize, pPermissions, pTimeOfCreation, pHandle, pSize, pType));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pSession);
  TMWTARG_UNUSED_PARAM(authKey);
#if TMWTARG_SUPPORT_DNPFILEIO
  return sdnptarg_openFile(pFilename, mode, pMaxBlockSize, pPermissions,
                          pTimeOfCreation, pHandle, pSize, pType);
#else
  TMWTARG_UNUSED_PARAM(pFilename);
  TMWTARG_UNUSED_PARAM(mode);
  TMWTARG_UNUSED_PARAM(pMaxBlockSize);
  TMWTARG_UNUSED_PARAM(pPermissions);
  TMWTARG_UNUSED_PARAM(pTimeOfCreation);
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pSize);
  TMWTARG_UNUSED_PARAM(pType);
  return(DNPDEFS_FILE_CMD_STAT_DENIED);
#endif
#endif
}

/* function: sdnpdata_closeFile */
DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnpdata_closeFile(
  TMWSESN *pSession,
  TMWTYPES_ULONG handle)
{
#if TMWCNFG_USE_SIMULATED_DB
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPSIM_DATABASE *pDb = (SDNPSIM_DATABASE *)pSDNPSession->pDbHandle;
  SDNPFSIM_DATABASE *pFDBHandle = (SDNPFSIM_DATABASE *)&(pDb->sdnpFsim);
  return(sdnpfsim_closeFile(pSession, pFDBHandle, handle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_CloseFile(pSession, handle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pSession);
#if TMWTARG_SUPPORT_DNPFILEIO
  return sdnptarg_closeFile(handle);
#else
  TMWTARG_UNUSED_PARAM(handle);
  return(DNPDEFS_FILE_CMD_STAT_DENIED);
#endif
#endif
}

/* function: sdnpdata_writeFile */
DNPDEFS_FILE_TFER_STAT TMWDEFS_GLOBAL sdnpdata_writeFile(
  TMWSESN *pSession,
  TMWTYPES_ULONG handle,
  TMWTYPES_BOOL last,
  TMWTYPES_USHORT numBytes,
  TMWTYPES_UCHAR *pBuf)
{
#if TMWCNFG_USE_SIMULATED_DB
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPSIM_DATABASE *pDb = (SDNPSIM_DATABASE *)pSDNPSession->pDbHandle;
  SDNPFSIM_DATABASE *pFDBHandle = (SDNPFSIM_DATABASE *)&(pDb->sdnpFsim);
  return(sdnpfsim_writeFile(pFDBHandle, handle, last, numBytes, pBuf));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_WriteFile(pSession, handle, last, numBytes, pBuf));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pSession);
#if TMWTARG_SUPPORT_DNPFILEIO
  return sdnptarg_writeFile(handle, last, numBytes, pBuf);
#else
  TMWTARG_UNUSED_PARAM(handle);
  TMWTARG_UNUSED_PARAM(last);
  TMWTARG_UNUSED_PARAM(numBytes);
  TMWTARG_UNUSED_PARAM(pBuf);
  return(DNPDEFS_FILE_TFER_STAT_NOT_OPEN);
#endif
#endif
}
#endif  /* SDNPDATA_SUPPORT_OBJ70 */

#if SDNPCNFG_USER_MANAGED_EVENTS
/* The following code is necessary only if User Managed Event
 * queues are implemented, instead of letting the SCL manage events.
 * This might be useful if events needed to be saved in nonVolatile
 * memory, or particular queuing algorithms are desired.
 * #define SDNPCNFG_USER_MANAGED_EVENTS TMWDEFS_TRUE when compiling
 * and set userManagedEvents = TMWDEFS_TRUE in SDNPSESN_CONFIG when the
 * session is opened.
 */

/* function: sdnpdata_umEventAdd */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_umEventAdd(
  void *pHandle,
  TMWTYPES_UCHAR group,
  TMWTYPES_USHORT point,
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_UCHAR defaultVariation,
  TMWTYPES_UCHAR flags,
  SDNPDATA_ADD_EVENT_VALUE *pValue,
  TMWDTIME *pTimeStamp)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_umEventAdd(pHandle, group, point, classMask, defaultVariation, flags, pValue, pTimeStamp));
#elif TMWCNFG_USE_MANAGED_SCL
  return(sdnpsim_umEventAdd(pHandle, group, point, classMask, defaultVariation, flags, pValue, pTimeStamp));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(group);
  TMWTARG_UNUSED_PARAM(point);
  TMWTARG_UNUSED_PARAM(flags);
  TMWTARG_UNUSED_PARAM(classMask);
  TMWTARG_UNUSED_PARAM(defaultVariation);
  TMWTARG_UNUSED_PARAM(pTimeStamp);
  TMWTARG_UNUSED_PARAM(pValue);
  return(TMWDEFS_FALSE);
#endif
}

/* function: sdnpdata_umEventCount */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_umEventNotSentCount(
  void *pHandle,
  TMWTYPES_UCHAR group,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_umEventNotSentCount(pHandle, group, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(sdnpsim_umEventNotSentCount(pHandle, group, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(group);
  TMWTARG_UNUSED_PARAM(classMask);
  return(0);
#endif
}

/* function: sdnpdata_umEventGet */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_umEventGet(
  void *pHandle,
  TMWTYPES_UCHAR group,
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_BOOL firstEvent,
  SDNPDATA_GET_EVENT *pEvent)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_umEventGet(pHandle, group, classMask, firstEvent, pEvent));
#elif TMWCNFG_USE_MANAGED_SCL
  return(sdnpsim_umEventGet(pHandle, group, classMask, firstEvent, pEvent));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(group);
  TMWTARG_UNUSED_PARAM(classMask);
  TMWTARG_UNUSED_PARAM(firstEvent);
  TMWTARG_UNUSED_PARAM(pEvent);
  return(TMWDEFS_FALSE);
#endif
}

/* function: sdnpdata_umEventSent */
void TMWDEFS_GLOBAL sdnpdata_umEventSent(
  void *pHandle,
  TMWTYPES_UCHAR group,
  TMWTYPES_USHORT point)
{
#if TMWCNFG_USE_SIMULATED_DB
  sdnpsim_umEventSent(pHandle, group, point);
#elif TMWCNFG_USE_MANAGED_SCL
  sdnpsim_umEventSent(pHandle, group, point);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(group);
  TMWTARG_UNUSED_PARAM(point);
#endif
} 

/* function: sdnpdata_umEventNotSent */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_umEventNotSent(
  void *pHandle,
  TMWTYPES_UCHAR group)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_umEventNotSent(pHandle, group));
#elif TMWCNFG_USE_MANAGED_SCL
  return(sdnpsim_umEventNotSent(pHandle, group));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(group);
  return(TMWDEFS_FALSE);
#endif
}

/* function: sdnpdata_umEventRemove */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_umEventRemove(
  void *pHandle,
  TMWTYPES_UCHAR group)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_umEventRemove(pHandle, group));
#elif TMWCNFG_USE_MANAGED_SCL
  return(sdnpsim_umEventRemove(pHandle, group));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(group);
  return(TMWDEFS_FALSE);
#endif
}

#endif

#if SDNPDATA_SUPPORT_OBJ0
/* function: sdnpdata_deviceAttrQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_deviceAttrQuantity(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_deviceAttrQuantity(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DeviceAttrQuantity(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(0);
#endif
}

void * TMWDEFS_GLOBAL sdnpdata_deviceAttrGetPoint( 
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_deviceAttrGetPoint(pHandle, pointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DeviceAttrGetPoint(pHandle, pointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNum);
  return(TMWDEFS_NULL);
#endif
}
 
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_deviceAttrNext(
  void *pPoint,
  TMWTYPES_UCHAR variation,
  TMWTYPES_UCHAR *pNextVariation,
  TMWTYPES_UCHAR *pProperty)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_deviceAttrNext(pPoint, variation, pNextVariation, pProperty));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DeviceAttrNext(pPoint, variation, pNextVariation, pProperty));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);  
  TMWTARG_UNUSED_PARAM(variation);  
  TMWTARG_UNUSED_PARAM(pNextVariation);  
  TMWTARG_UNUSED_PARAM(pProperty);  
  return(TMWDEFS_FALSE);
#endif
}

void * TMWDEFS_GLOBAL sdnpdata_deviceAttrGetVar( 
  void *pPoint,
  TMWTYPES_UCHAR variation)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_deviceAttrGetVar(pPoint, variation));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DeviceAttrGetVar(pPoint, variation));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(variation);
  return(TMWDEFS_NULL);
#endif
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_deviceAttrWrite(
  void *pAttribute, 
  DNPDATA_ATTRIBUTE_VALUE *pData)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_deviceAttrWrite(pAttribute, pData));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DeviceAttrWrite(pAttribute, pData));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pAttribute);  
  TMWTARG_UNUSED_PARAM(pData);  
  return(TMWDEFS_FALSE);
#endif 
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_deviceAttrRead(
  void *pAttribute,
  DNPDATA_ATTRIBUTE_VALUE *pData)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_deviceAttrRead(pAttribute, pData));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DeviceAttrRead(pAttribute, pData));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pAttribute);  
  TMWTARG_UNUSED_PARAM(pData); 
  return(TMWDEFS_FALSE);
#endif
}
#endif


#if SDNPDATA_SUPPORT_DATASETS
#if SDNPDATA_SUPPORT_XML || SDNPDATA_SUPPORT_XML2
/* function: sdnpdata_datasetGetDescription */
TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_datasetGetDescription(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(tmwsim_getDescription((TMWSIM_POINT *)pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetGetDescription((TMWSIM_POINT *)pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_NULL);
#endif
}
#endif
#endif

#if SDNPDATA_SUPPORT_OBJ85
/* function: sdnpdata_datasetProtoQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_datasetProtoQuantity(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_datasetProtoQuantity(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetProtoQuantity(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  return(0);
#endif
}

/* function: sdnpdata_datasetProtoGetID */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpdata_datasetProtoGetID(
  void *pHandle,
  TMWTYPES_UCHAR *pUUID,
  TMWTYPES_USHORT *pPointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_datasetProtoGetID(pHandle, pUUID, pPointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetProtoGetID(pHandle, pUUID, pPointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pUUID); 
  TMWTARG_UNUSED_PARAM(pPointNum); 
  return(TMWDEFS_FALSE);
#endif
}

/* function: sdnpdata_datasetProtoGetPoint */
void * TMWDEFS_CALLBACK sdnpdata_datasetProtoGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_datasetProtoGetPoint(pHandle, pointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetProtoGetPoint(pHandle, pointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pointNum); 
  return(TMWDEFS_NULL);
#endif
}

/* function: sdnpdata_datasetProtoRead */
DNPDATA_DATASET_DESCR_ELEM * TMWDEFS_GLOBAL sdnpdata_datasetProtoRead(
  void *pPoint,
  TMWTYPES_UCHAR *pNumberElems,
  TMWTYPES_UCHAR *pUUID)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_datasetProtoRead(pPoint, pNumberElems, pUUID));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetProtoRead(pPoint, pNumberElems, pUUID));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint); 
  TMWTARG_UNUSED_PARAM(pUUID); 
  TMWTARG_UNUSED_PARAM(pNumberElems); 
  return(TMWDEFS_NULL);
#endif
} 

/* function: sdnpdata_datasetProtoRelease */
void TMWDEFS_GLOBAL sdnpdata_datasetProtoRelease(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pPoint); 
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_DatasetProtoRelease(pPoint);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint); 
#endif
}

/* function: sdnpdata_datasetProtoCreatePoint */
void * TMWDEFS_GLOBAL sdnpdata_datasetProtoCreatePoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum,
  TMWTYPES_UCHAR *pUUID) 
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_datasetProtoCreatePoint(pHandle, pointNum, pUUID, TMWDEFS_TRUE));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetProtoCreatePoint(pHandle, pointNum, pUUID));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNum);  
  TMWTARG_UNUSED_PARAM(pUUID); 
  return(TMWDEFS_NULL);
#endif
}

/* function: sdnpdata_datasetProtoWrite */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_datasetProtoWrite(
  void *pPoint,
  TMWTYPES_UCHAR index,
  DNPDATA_DATASET_DESCR_ELEM *pElem)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_datasetProtoWrite(pPoint, index, pElem));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetProtoWrite(pPoint, index, pElem));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint); 
  TMWTARG_UNUSED_PARAM(index); 
  TMWTARG_UNUSED_PARAM(pElem); 
  return(TMWDEFS_FALSE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_OBJ86

/* function: sdnpdata_datasetDescrQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_datasetDescrQuantity(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_datasetDescrQuantity(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetDescrQuantity(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  return(0);
#endif
}

/* function: sdnpdata_datasetDescrCreatePoint */
void * TMWDEFS_GLOBAL sdnpdata_datasetDescrCreatePoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum) 
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_datasetDescrCreatePoint(pHandle, pointNum, TMWDEFS_TRUE));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetDescrCreatePoint(pHandle, pointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pointNum); 
  return(TMWDEFS_NULL);
#endif
}

/* function: sdnpdata_datasetDescrGetPoint */
void * TMWDEFS_CALLBACK sdnpdata_datasetDescrGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_datasetDescrGetPoint(pHandle, pointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetDescrGetPoint(pHandle, pointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pointNum); 
  return(TMWDEFS_NULL);
#endif
}

/* function: sdnpdata_datasetDescrReadCont */
DNPDATA_DATASET_DESCR_ELEM * TMWDEFS_GLOBAL sdnpdata_datasetDescrReadCont(
  void *pPoint,
  TMWTYPES_UCHAR *pNumberElems)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_datasetDescrReadCont(pPoint, pNumberElems));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetDescrReadCont(pPoint, pNumberElems));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint); 
  TMWTARG_UNUSED_PARAM(pNumberElems); 
  return(TMWDEFS_NULL);
#endif
}

/* function: sdnpdata_datasetDescrReadIndex */
DNPDATA_DATASET_DESCR_INDEX * TMWDEFS_GLOBAL sdnpdata_datasetDescrReadIndex(
  void *pPoint,
  TMWTYPES_UCHAR *pNumberElems)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_datasetDescrReadIndex(pPoint, pNumberElems));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetDescrReadIndex(pPoint, pNumberElems));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint); 
  TMWTARG_UNUSED_PARAM(pNumberElems); 
  return(TMWDEFS_NULL);
#endif
}

/* function: sdnpdata_datasetDescrReadChars */
void TMWDEFS_GLOBAL sdnpdata_datasetDescrReadChars(
  void *pPoint,
  TMWTYPES_UCHAR *pValue)
{
#if TMWCNFG_USE_SIMULATED_DB
  sdnpsim_datasetDescrReadChars(pPoint, pValue);
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_DatasetDescrReadChars(pPoint, pValue);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint); 
  TMWTARG_UNUSED_PARAM(pValue); 
#endif
}

/* function: sdnpdata_datasetDescrRelease */
void TMWDEFS_GLOBAL sdnpdata_datasetDescrRelease(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pPoint); 
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_DatasetDescrRelease(pPoint);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint); 
#endif
}

/* function: sdnpdata_datasetDescrWriteCont */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_datasetDescrWriteCont(
  void *pPoint,
  TMWTYPES_UCHAR index,
  DNPDATA_DATASET_DESCR_ELEM *pElem)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_datasetDescrWriteCont(pPoint, index, pElem));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetDescrWriteCont(pPoint, index, pElem));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint); 
  TMWTARG_UNUSED_PARAM(index); 
  TMWTARG_UNUSED_PARAM(pElem); 
  return(TMWDEFS_FALSE);
#endif
}

/* function: sdnpdata_datasetDescrWriteIndex */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_datasetDescrWriteIndex(
  void *pPoint,
  TMWTYPES_UCHAR index,
  DNPDATA_DATASET_DESCR_INDEX *pElem)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_datasetDescrWriteIndex(pPoint, index, pElem));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetDescrWriteIndex(pPoint, index, pElem));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint); 
  TMWTARG_UNUSED_PARAM(index); 
  TMWTARG_UNUSED_PARAM(pElem); 
  return(TMWDEFS_FALSE);
#endif
}

/* function: sdnpdata_datasetDescrEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_datasetDescrEventClass(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_datasetDescrEventClass(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetDescrEventClass(pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_CLASS_MASK_NONE);
#endif
}

#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpdata_datasetDescrAssignClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_datasetDescrAssignClass(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_datasetDescrAssignClass(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetDescrAssignClass(pPoint, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(TMWDEFS_FALSE);
#endif
}
#endif
#endif

#if SDNPDATA_SUPPORT_OBJ87
/* function: sdnpdata_datasetQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_datasetQuantity(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_datasetQuantity(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetQuantity(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  return(0);
#endif
}

/* function: sdnpdata_datasetGetPoint */
void * TMWDEFS_CALLBACK sdnpdata_datasetGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_datasetGetPoint(pHandle, pointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetGetPoint(pHandle, pointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pointNum); 
  return(TMWDEFS_NULL);
#endif
}

/* function: sdnpdata_datasetRead */
DNPDATA_DATASET_VALUE * TMWDEFS_GLOBAL sdnpdata_datasetRead(
  void *pPoint,
  TMWTYPES_UCHAR *pNumberElems,
  TMWDTIME *pTimeStamp)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_datasetRead(pPoint, pNumberElems, pTimeStamp));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetRead(pPoint, pNumberElems, pTimeStamp));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint); 
  TMWTARG_UNUSED_PARAM(pNumberElems); 
  TMWTARG_UNUSED_PARAM(pTimeStamp); 
  return(TMWDEFS_NULL);
#endif
}

/* function: sdnpdata_datasetRelease */
void TMWDEFS_GLOBAL sdnpdata_datasetRelease(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pPoint); 
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetRelease(pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint); 
#endif
}

/* function: sdnpdata_datasetCreatePoint */
void * TMWDEFS_GLOBAL sdnpdata_datasetCreatePoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum,
  SDNPDATA_DATASET_MODE mode,
  TMWDTIME *pTimeStamp)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(mode); 
  return(sdnpsim_datasetCreatePoint(pHandle, pointNum, pTimeStamp));
#elif TMWCNFG_USE_MANAGED_SCL
  TMWTARG_UNUSED_PARAM(mode); 
  return(SDNPDatabaseWrapper_DatasetCreatePoint(pHandle, pointNum, mode, pTimeStamp));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pointNum); 
  TMWTARG_UNUSED_PARAM(mode); 
  TMWTARG_UNUSED_PARAM(pTimeStamp); 
  return(TMWDEFS_NULL);
#endif
}

/* function: sdnpdata_datasetWrite */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_datasetWrite(
  void *pPoint,
  TMWTYPES_UCHAR index,
  DNPDATA_DATASET_VALUE *pElem)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_datasetWrite(pPoint, index, pElem));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetWrite(pPoint, index, pElem));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint); 
  TMWTARG_UNUSED_PARAM(index); 
  TMWTARG_UNUSED_PARAM(pElem); 
  return(TMWDEFS_FALSE);
#endif
}

/* function: sdnpdata_datasetSelect */
DNPDEFS_DATASET_ST TMWDEFS_GLOBAL sdnpdata_datasetSelect(
  void *pPoint,
  DNPDATA_DATASET_CTRL_VALUE *pCtrlValues,
  TMWTYPES_UCHAR numberCtrlValues)
{
#if TMWCNFG_USE_SIMULATED_DB
    return(sdnpsim_datasetSelect(pPoint, pCtrlValues, numberCtrlValues));
#elif TMWCNFG_USE_MANAGED_SCL
    return(SDNPDatabaseWrapper_DatasetSelect(pPoint, pCtrlValues, numberCtrlValues));
#else
    /* Put target code here */
    TMWTARG_UNUSED_PARAM(pPoint);
    TMWTARG_UNUSED_PARAM(pCtrlValues);
    TMWTARG_UNUSED_PARAM(numberCtrlValues);  
    return(DNPDEFS_DATASET_ST_NOT_SUPPORTED);
#endif
}

/* function: sdnpdata_datasetOperate */
DNPDEFS_DATASET_ST TMWDEFS_GLOBAL sdnpdata_datasetOperate(
  void *pPoint,
  DNPDATA_DATASET_CTRL_VALUE *pCtrlValues,
  TMWTYPES_UCHAR numberCtrlValues)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_datasetOperate(pPoint, pCtrlValues, numberCtrlValues));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DatasetOperate(pPoint, pCtrlValues, numberCtrlValues));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint); 
  TMWTARG_UNUSED_PARAM(pCtrlValues); 
  TMWTARG_UNUSED_PARAM(numberCtrlValues); 
  return(DNPDEFS_DATASET_ST_NOT_SUPPORTED);
#endif
}

#if SDNPDATA_SUPPORT_SELECT_CANCEL
/* function: sdnpdata_datasetCancelSelect */
void TMWDEFS_GLOBAL sdnpdata_datasetCancelSelect(
  void *pPoint)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint); 
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ87 */

#if SDNPDATA_SUPPORT_OBJ91
/* function: sdnpdata_activateConfig */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_activateConfig(
 void           *pHandle,
 TMWTYPES_USHORT pointNum,
 TMWTYPES_BOOL   lastString,
 TMWTYPES_UCHAR *pString,
 TMWTYPES_USHORT stringLength,
 TMWTYPES_ULONG *pDelay,
 TMWTYPES_UCHAR *pErrorText,
 TMWTYPES_UCHAR *pErrorLength) 
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pointNum); 
  TMWTARG_UNUSED_PARAM(lastString); 
  TMWTARG_UNUSED_PARAM(pString); 
  TMWTARG_UNUSED_PARAM(stringLength); 
  TMWTARG_UNUSED_PARAM(pErrorText); 
  *pDelay = 0x1000; 
  *pErrorLength = 0;
  return 0;
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_ActivateConfig(pHandle, pointNum, lastString, pString, stringLength, pDelay, pErrorText, pErrorLength);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pointNum); 
  TMWTARG_UNUSED_PARAM(lastString); 
  TMWTARG_UNUSED_PARAM(pString); 
  TMWTARG_UNUSED_PARAM(stringLength); 
  TMWTARG_UNUSED_PARAM(pDelay); 
  TMWTARG_UNUSED_PARAM(pErrorText); 
  TMWTARG_UNUSED_PARAM(pErrorLength); 
  *pErrorLength = 0;
  return 0;
#endif
} 
#endif

#if SDNPDATA_SUPPORT_OBJ120

#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpdata_authAssignClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authAssignClass(
  void *pHandle,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_authAssignClass(pHandle, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AuthAssignClass(pHandle, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(classMask);
  return(TMWDEFS_FALSE);
#endif
}
#endif

/* function: sdnpdata_authErrorEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_authErrorEventClass(
  void *pHandle)
{
  /* Replace this with target code */
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_authErrorEventClass(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AuthErrorEventClass(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return TMWDEFS_CLASS_MASK_NONE; 
#endif
}

/* function: sdnpdata_authIsCriticalAuthReq */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authIsCriticalReq(
  void          *pHandle,
  TMWTYPES_UCHAR fc,
  TMWTYPES_UCHAR *pRxMsg,
  TMWTYPES_USHORT msgLength)
 {
#if TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_AuthIsCriticalReq(
      pHandle,
      fc,
      pRxMsg, 
      msgLength);
#else
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pRxMsg); 
  TMWTARG_UNUSED_PARAM(msgLength); 
  /* This is the list of mandatory critical from the secure authentication spec 
   * You can add function codes to this list, or write your own code to look in the
   *  message for a particular object.  
   *  Making Application Confirm messages and other "noAck" requests critical 
   *  can be a problem on a half duplex on multidrop line, as the master does 
   *  not expect a "response" and the line may not be available.
   */
  switch(fc)
  {
    case DNPDEFS_FC_WRITE:
    case DNPDEFS_FC_SELECT:
    case DNPDEFS_FC_OPERATE:
    case DNPDEFS_FC_DIRECT_OP:
    case DNPDEFS_FC_DIRECT_OP_NOACK:
    case DNPDEFS_FC_COLD_RESTART:
    case DNPDEFS_FC_WARM_RESTART:
    case DNPDEFS_FC_INIT_APPLICATION:
    case DNPDEFS_FC_START_APPLICATION:
    case DNPDEFS_FC_STOP_APPLICATION:
    case DNPDEFS_FC_ENABLE_UNSOL:
    case DNPDEFS_FC_DISABLE_UNSOL:
    case DNPDEFS_FC_RECORD_CURRENT_TIME:
    case DNPDEFS_FC_OPEN_FILE:
    case DNPDEFS_FC_CLOSE_FILE:
    case DNPDEFS_FC_DELETE_FILE:
    case DNPDEFS_FC_GET_FILE_INFO:
    case DNPDEFS_FC_AUTHENTICATE: 
    case DNPDEFS_FC_ABORT: 
    case DNPDEFS_FC_ACTIVATE_CONFIG:
      return TMWDEFS_TRUE;
      break;
    default:
      break;  
  }
  return TMWDEFS_FALSE;  
#endif
}

#if SDNPCNFG_SUPPORT_SA_VERSION5
#if TMWCNFG_USE_MANAGED_SCL
/* function: _toCryptoHandle */
static TMWDEFS_LOCAL void *_toCryptoHandle(void *pHandle)
{
  SDNPSIM_DATABASE *pSim = (SDNPSIM_DATABASE *)pHandle;
  return pSim->managedDBhandle; 
}
#endif
#endif
 
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_authGetUserRole(
  void           *pHandle,
  TMWTYPES_USHORT userNumber)
{
#if TMWCNFG_USE_MANAGED_SCL 
  return TMWCryptoWrapper_getUserRole(_toCryptoHandle(pHandle), userNumber);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(userNumber);
  return DNPAUTH_USER_ROLE_SINGLEUSER;
#endif
}

/* function: sdnpdata_authRequestAllowed */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authRequestAllowed(
  void           *pHandle,
  TMWTYPES_USHORT userNumber,
  TMWTYPES_UCHAR  fc,
  TMWTYPES_UCHAR *pRxMsg,
  TMWTYPES_USHORT msgLength)
{ 
  TMWTYPES_USHORT role;
  /* Put target code here */ 
  TMWTARG_UNUSED_PARAM(pRxMsg);
  TMWTARG_UNUSED_PARAM(msgLength);

  /*  
   * Your implementation should enforce the role (permissions) for a user.
   * This role may be sent by the Master and stored by sdnpdata_authUserStatusChgRcvd()
   * or could be configured on this outstation for each user.
   */

  /* The following SAMPLE CODE implements one interpretation of the roles specified 
   * in the secure authentication spec table 7-12. 
   * You should evaluate this and replace this with your own implementation as desired.
   * For example, do you want an Engineer to be able to issue a Cold Restart command or not.
   */
  role = sdnpdata_authGetUserRole(pHandle, userNumber);
  if(role == DNPAUTH_USER_ROLE_SINGLEUSER)
    return TMWDEFS_TRUE;

  /* User Not found or Expired role */
  if(role == DNPAUTH_USER_ROLE_EXPIRED)
    return TMWDEFS_FALSE;

  switch(fc)
  {
    case DNPDEFS_FC_WRITE: 
      /* Allow Binary output write requests by Operator if desired.
       * if(pRxMsg[2] == DNPDEFS_OBJ_10_BIN_OUTS) 
       * {
       *  if(role == DNPAUTH_USER_ROLE_OPERATOR)
       *  {
       *    return TMWDEFS_TRUE;
       *  }
       *  else
       *  {
       *    return TMWDEFS_FALSE;
       *  }
       * }
       */

      /* Allow other write requests by Engineer and Installer */
      if((role != DNPAUTH_USER_ROLE_ENGINEER)
        &&(role != DNPAUTH_USER_ROLE_INSTALLER))
      {
        return TMWDEFS_FALSE;
      }
   
      break;

    case DNPDEFS_FC_SELECT:
      /* fall through */
    case DNPDEFS_FC_OPERATE:
      /* fall through */
    case DNPDEFS_FC_DIRECT_OP:
      /* fall through */
    case DNPDEFS_FC_DIRECT_OP_NOACK:
      if(role != DNPAUTH_USER_ROLE_OPERATOR)
        return TMWDEFS_FALSE;
      break;

    case DNPDEFS_FC_COLD_RESTART:
      /* fall through */
    case DNPDEFS_FC_WARM_RESTART:
      /* fall through */
    case DNPDEFS_FC_INIT_APPLICATION:
      /* fall through */
    case DNPDEFS_FC_START_APPLICATION:
      /* fall through */
    case DNPDEFS_FC_STOP_APPLICATION: 
      /* fall through */
    case DNPDEFS_FC_ENABLE_UNSOL:
      /* fall through */
    case DNPDEFS_FC_DISABLE_UNSOL:
      /* fall through */
    case DNPDEFS_FC_RECORD_CURRENT_TIME:
      /* fall through */
    case DNPDEFS_FC_ACTIVATE_CONFIG:
      if( (role != DNPAUTH_USER_ROLE_ENGINEER)
        &&(role != DNPAUTH_USER_ROLE_INSTALLER))
        return TMWDEFS_FALSE;
      break;

    case DNPDEFS_FC_OPEN_FILE:
      /* fall through */
    case DNPDEFS_FC_CLOSE_FILE:
      /* fall through */
    case DNPDEFS_FC_AUTHENTICATE: 
      if( (role != DNPAUTH_USER_ROLE_ENGINEER)
        &&(role != DNPAUTH_USER_ROLE_INSTALLER)
        && (role != DNPAUTH_USER_ROLE_SECAUD)) 
        return TMWDEFS_FALSE;
      break;

    case DNPDEFS_FC_DELETE_FILE:
      if( (role != DNPAUTH_USER_ROLE_ENGINEER)
        &&(role != DNPAUTH_USER_ROLE_RBACMNT)) 
        return TMWDEFS_FALSE;
      break;
 
    default: 
      break;  
  }
  return TMWDEFS_TRUE;  
}

/* function: sdnpdata_authLogErrorRx */
void TMWDEFS_GLOBAL sdnpdata_authLogErrorRx(
    void            *pHandle,
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_USHORT  assocId,
    TMWTYPES_ULONG   sequenceNumber,
    TMWTYPES_UCHAR   errorCode,
    TMWDTIME        *pTimeStamp,
    TMWTYPES_CHAR   *pErrorText,
    TMWTYPES_USHORT  errorTextLength)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(userNumber);
  TMWTARG_UNUSED_PARAM(assocId);
  TMWTARG_UNUSED_PARAM(sequenceNumber); 
  TMWTARG_UNUSED_PARAM(errorCode);
  TMWTARG_UNUSED_PARAM(pTimeStamp); 
  TMWTARG_UNUSED_PARAM(pErrorText);
  TMWTARG_UNUSED_PARAM(errorTextLength); 
}

/* function: sdnpdata_authLogErrorTx */
void TMWDEFS_GLOBAL sdnpdata_authLogErrorTx(
    void            *pHandle,
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_USHORT  assocId,
    TMWTYPES_ULONG   sequenceNumber,
    TMWTYPES_UCHAR   errorCode,
    TMWDTIME        *pTimeStamp,
    TMWTYPES_CHAR   *pErrorText,
    TMWTYPES_USHORT  errorTextLength,
    TMWTYPES_BOOL    msgSent)
{
#if TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_AuthLogErrorTx(
    pHandle, 
    userNumber, 
    assocId, 
    sequenceNumber, 
    errorCode,
    pTimeStamp, 
    pErrorText, 
    errorTextLength,
    msgSent);
  TMWTARG_UNUSED_PARAM(msgSent);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(userNumber);
  TMWTARG_UNUSED_PARAM(assocId);
  TMWTARG_UNUSED_PARAM(sequenceNumber);
  TMWTARG_UNUSED_PARAM(errorCode);
  TMWTARG_UNUSED_PARAM(pTimeStamp);
  TMWTARG_UNUSED_PARAM(pErrorText);
  TMWTARG_UNUSED_PARAM(errorTextLength);
  TMWTARG_UNUSED_PARAM(msgSent);
#endif
}

#if SDNPCNFG_SUPPORT_SA_VERSION5

#if TMWCNFG_SUPPORT_CRYPTO_AESGMAC

/* According to SAv5 4.2.1.1 if AES-GMAC is supported, Persist KSQ */
void TMWDEFS_GLOBAL sdnpdata_authStoreKSQ(
  void            *pHandle,
  TMWTYPES_ULONG   KSQ)
{
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(KSQ);
}
    
/* According to SAv5 4.2.1.1 if AES-GMAC is supported, Persist KSQ 
  * This will only be called when the sdnp session is started up.
  * If AES-GMAC is not used this should return 0.
  */
void TMWDEFS_GLOBAL sdnpdata_authGetKSQ(
  void            *pHandle,
  TMWTYPES_ULONG  *pKSQ)
{
  TMWTARG_UNUSED_PARAM(pHandle);
  *pKSQ = 0;
}
#endif


#if DNPCNFG_SUPPORT_AUTHKEYUPDATE  

/* function: sdnpdata_authKeyChgMethodSupport */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authKeyChgMethodSupport(
  void           *pHandle,
  TMWTYPES_UCHAR  keyChangeMethod)
 {
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pHandle);  
  TMWTARG_UNUSED_PARAM(keyChangeMethod);  
  return TMWDEFS_TRUE;
#elif TMWCNFG_USE_MANAGED_SCL 
  return TMWCryptoWrapper_keyChgMethodSupport(_toCryptoHandle(pHandle), 
    keyChangeMethod);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(keyChangeMethod); 
  return(TMWDEFS_FALSE);
#endif
} 
 
/* function: sdnpdata_authGetOSName */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authGetOSName(
  void            *pHandle, 
  TMWTYPES_CHAR   *pOSName,
  TMWTYPES_USHORT *pOSNameLength)
{ 
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_authGetOSName(
      pHandle,
      pOSName,
      pOSNameLength);
#elif TMWCNFG_USE_MANAGED_SCL
  return TMWCryptoWrapper_getOSName(_toCryptoHandle(pHandle), 
    pOSName, pOSNameLength);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pOSName); 
  TMWTARG_UNUSED_PARAM(pOSNameLength); 
  return(TMWDEFS_FALSE);
#endif
} 

/* function: sdnpdata_authGetUserName */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authGetUserName(
  void            *pHandle, 
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_CHAR   *pUserName,
  TMWTYPES_USHORT *pUserNameLength)
{ 
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_authGetUserName(pHandle, userNumber, pUserName, pUserNameLength);
#elif TMWCNFG_USE_MANAGED_SCL 
  return TMWCryptoWrapper_getUserName(_toCryptoHandle(pHandle), userNumber, pUserName, pUserNameLength);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(userNumber); 
  TMWTARG_UNUSED_PARAM(pUserName); 
  TMWTARG_UNUSED_PARAM(pUserNameLength); 
  return(TMWDEFS_FALSE);
#endif
}

#if SDNPDATA_SUPPORT_OBJ120_V8
/* This only needs to be implemented if optional IEC 62351 Certificate sent
 * in g120v8 for asymmetric key update is supported
 */

/* function: sdnpdata_authUserCertRcvd */ 
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_authUserCertRcvd(
  void            *pHandle, 
  TMWTYPES_UCHAR   keyChangeMethod, 
  TMWTYPES_UCHAR   certType, 
  TMWTYPES_UCHAR  *pCertData, 
  TMWTYPES_USHORT  certDataLength,
  TMWTYPES_ULONG  *pStatusChangeSequence,
  TMWTYPES_UCHAR  *pOperation,
  TMWTYPES_UCHAR  *pError)
{
#if !(TMWCNFG_USE_OPENSSL_1_0_2) 
  SDNPSIM_DATABASE *pDb;
  TMWTYPES_LONG   userRole;
  TMWTYPES_USHORT userRoleExpiryInterval;
  TMWTYPES_CHAR   areaOfResponsibility[64];
  TMWTYPES_CHAR   userName[DNPCNFG_AUTH_MAX_USERNAME_LENGTH];
  
  TMWTARG_UNUSED_PARAM(certType);

  pDb = (SDNPSIM_DATABASE *)pHandle; 

  if (tmwcrypto_verifyUserCertificate(
    pDb->pSDNPSession->dnp.pCryptoHandle,
    pCertData,
    certDataLength,
    pStatusChangeSequence,
    pOperation,
    userName,
    sizeof(userName),
    &userRole,
    &userRoleExpiryInterval,
    areaOfResponsibility,
    (TMWTYPES_USHORT)sizeof(areaOfResponsibility)))
  {
    /* Validate areaOfResponsibility */
    TMWTYPES_USHORT userNumber = sdnpdata_authUserStatusChgRcvd(
      pHandle,
      userName,
      (TMWTYPES_USHORT)strlen(userName),
      keyChangeMethod,
      *pOperation,
      (TMWTYPES_USHORT)userRole,
      userRoleExpiryInterval);
    if (!userNumber)
    {
      *pError = DNPAUTH_ERROR_UNKNOWN_USER;
    }
    return (userNumber);
  }
  *pError = DNPAUTH_ERROR_INVALIDCERTDATA;
  return (0);
#else
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_authUserCertRcvd(
    pHandle,
    keyChangeMethod, 
    certType, 
    pCertData, 
    certDataLength,
    pStatusChangeSequence,
    pOperation,
    pError);
#elif TMWCNFG_USE_MANAGED_SCL
  return TMWCryptoWrapper_userCertRcvd(
    _toCryptoHandle(pHandle), 
    keyChangeMethod, 
    certType, 
    pCertData, 
    certDataLength,
    pStatusChangeSequence,
    pOperation,
    pError);
#else
  /* Put target code here */ 
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(keyChangeMethod);  
  TMWTARG_UNUSED_PARAM(certType); 
  TMWTARG_UNUSED_PARAM(pCertData); 
  TMWTARG_UNUSED_PARAM(certDataLength); 
  TMWTARG_UNUSED_PARAM(pStatusChangeSequence); 
  TMWTARG_UNUSED_PARAM(pOperation); 
  TMWTARG_UNUSED_PARAM(pError); 
  return(0);
#endif
#endif
}
#endif

/* function: sdnpdata_authUserStatusChngRcvd */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_authUserStatusChgRcvd(
  void            *pHandle,
  TMWTYPES_CHAR   *pUserName,
  TMWTYPES_USHORT  userNameLength,
  TMWTYPES_UCHAR   keyChangeMethod, 
  TMWTYPES_UCHAR   operation, 
  TMWTYPES_USHORT  userRole, 
  TMWTYPES_USHORT  userRoleExpiryInterval)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_authUserStatusChg(pHandle,
    pUserName,
    userNameLength,
    keyChangeMethod, 
    operation, 
    userRole, 
    userRoleExpiryInterval);
#elif TMWCNFG_USE_MANAGED_SCL
  return TMWCryptoWrapper_userStatusChgRcvd(_toCryptoHandle(pHandle), 
    pUserName,
    userNameLength,
    keyChangeMethod, 
    operation, 
    userRole, 
    userRoleExpiryInterval);
#else
  /* Put target code here */ 
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pUserName); 
  TMWTARG_UNUSED_PARAM(userNameLength); 
  TMWTARG_UNUSED_PARAM(keyChangeMethod); 
  TMWTARG_UNUSED_PARAM(operation); 
  TMWTARG_UNUSED_PARAM(userRole); 
  TMWTARG_UNUSED_PARAM(userRoleExpiryInterval); 
  return(TMWDEFS_FALSE);
#endif
}

/* function: sdnpdata_authUpdateKeyChangeReq */ 
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_authUpdateKeyChgReq( 
  void            *pHandle,
  TMWTYPES_UCHAR   keyChangeMethod,
  TMWTYPES_CHAR   *pUserName,
  TMWTYPES_USHORT  userNameLength)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(keyChangeMethod); 
  return sdnpsim_authUpdateKeyChgReqRcvd(
    pHandle, 
    pUserName,
    userNameLength);
#elif TMWCNFG_USE_MANAGED_SCL
  return TMWCryptoWrapper_updateKeyChgReqRcvd(
    _toCryptoHandle(pHandle), 
    keyChangeMethod,
    pUserName,
    userNameLength);
#else
  /* Put target code here */ 
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(keyChangeMethod); 
  TMWTARG_UNUSED_PARAM(pUserName); 
  TMWTARG_UNUSED_PARAM(userNameLength); 
  return(0);
#endif
} 
#endif  /* DNPCNFG_SUPPORT_AUTHKEYUPDATE */
 
/* function: sdnpdata_authSecStatQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_authSecStatQuantity(
  void *pHandle)
{
 #if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_authSecStatQuantity(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AuthSecStatQuantity(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(0);
#endif
}
 
/* function: sdnpdata_authSecStatGetPoint */
void * TMWDEFS_GLOBAL sdnpdata_authSecStatGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_authSecStatGetPoint(pHandle, pointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AuthSecStatGetPoint(pHandle, pointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNum);
  return(TMWDEFS_NULL);
#endif
}

/* function: sdnpdata_authSecStatEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_authSecStatEventClass(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventClass(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AuthSecStatEventClass(pPoint));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_CLASS_MASK_NONE);
#endif
} 

/* function: sdnpdata_authSecStatIsClass0 */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authSecStatIsClass0(
  void *pPoint)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return TMWDEFS_TRUE;
}

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
/* function: sdnpdata_authSecStatEventDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_authSecStatEventDefVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_authSecStatEventDefVariation(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AuthSecStatEventDefVariation(pPoint, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(3);
#endif
} 
#endif

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
/* function: sdnpdata_authSecStatEventMode */
TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_authSecStatEventMode(
  void *pPoint)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_getEventMode(pPoint));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_EventMode(pPoint, TMWDEFS_FALSE));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  return(TMWDEFS_EVENT_MODE_SOE);
#endif
} 
#endif

#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpdata_authSecStatAssignClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authSecStatAssignClass(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(sdnpsim_setEventClass(pPoint, classMask));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_AuthSecStatAssignClass(pPoint, classMask));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(classMask);
  return(TMWDEFS_FALSE);
#endif
}
#endif

/* function: sdnpdata_authSecStatRead */
void TMWDEFS_GLOBAL sdnpdata_authSecStatRead(
  void *pPoint,
  TMWTYPES_ULONG *pValue, 
  TMWTYPES_UCHAR *pFlags)
{
#if TMWCNFG_USE_SIMULATED_DB
  sdnpsim_authSecStatRead(pPoint, pValue, pFlags);
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_AuthSecStatRead(pPoint, pValue, pFlags);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pValue);
  TMWTARG_UNUSED_PARAM(pFlags);
#endif
}

/* function: sdnpdata_authSecStatThreshold */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_authSecStatThreshold(
  void            *pHandle,
  TMWTYPES_USHORT  index) 
{ 
#if TMWCNFG_USE_SIMULATED_DB  
  return sdnpsim_getAuthSecStatThreshold(pHandle, index);
#elif TMWCNFG_USE_MANAGED_SCL  
  return SDNPDatabaseWrapper_GetAuthSecStatThreshold(pHandle, index);
#else
  /* Replace this with your target code, allowing configuration of these thresholds */
  TMWTARG_UNUSED_PARAM(pHandle); 
  switch(index)
  {
    case  DNPAUTH_UNEXPECTED_MSG_INDEX:       
      return DNPAUTH_UNEXPECTED_MSG_THRESHOLD;
      break;
    case  DNPAUTH_AUTHOR_FAIL_INDEX:       
      return DNPAUTH_AUTHOR_FAIL_THRESHOLD;
      break;
    case  DNPAUTH_AUTHENT_FAIL_INDEX:        
      return DNPAUTH_AUTHENT_FAIL_THRESHOLD;
     break;
    case  DNPAUTH_REPLY_TIMEOUT_INDEX:        
      return DNPAUTH_REPLY_TIMEOUT_THRESHOLD;
     break;
    case  DNPAUTH_REKEY_DUETOFAIL_INDEX:        
      return DNPAUTH_REKEY_THRESHOLD;   
     break;     
    case  DNPAUTH_TOTAL_MSG_SENT_INDEX:        
      return DNPAUTH_TOTAL_MSG_SENT_THRESHOLD;  
     break;     
    case  DNPAUTH_TOTAL_MSG_RCVD_INDEX:        
      return DNPAUTH_TOTAL_MSG_RCVD_THRESHOLD;   
     break;     
    case  DNPAUTH_CRIT_MSG_SENT_INDEX:        
      return DNPAUTH_CRIT_MSG_SENT_THRESHOLD;    
     break;      
    case  DNPAUTH_CRIT_MSG_RCVD_INDEX:        
      return DNPAUTH_CRIT_MSG_RCVD_THRESHOLD;    
     break;     
    case  DNPAUTH_DISCARDED_MSG_INDEX:        
      return DNPAUTH_DISCARDED_MSG_THRESHOLD;  
     break;     
    case  DNPAUTH_ERROR_MSG_SENT_INDEX:        
      return DNPAUTH_ERROR_MSG_SENT_THRESHOLD;   
     break;     
    case  DNPAUTH_ERROR_MSG_RCVD_INDEX:        
      return DNPAUTH_ERROR_MSG_RCVD_THRESHOLD;  
     break;     
    case  DNPAUTH_SUCCESS_AUTHENT_INDEX:      
      return DNPAUTH_SUCCESS_AUTHENT_THRESHOLD;   
     break;     
    case  DNPAUTH_SESKEY_CHANGE_INDEX:        
      return DNPAUTH_SESKEY_CHANGE_THRESHOLD;   
     break;     
    case  DNPAUTH_FAILSESKEY_CHANGE_INDEX:        
      return DNPAUTH_FAILSESKEY_CHANGE_THRESHOLD;    
     break;     
    case  DNPAUTH_UPDKEY_CHANGE_INDEX:        
      return DNPAUTH_UPDKEY_CHANGE_THRESHOLD;  
     break;     
    case  DNPAUTH_FAILUPDKEY_CHANGE_INDEX:        
      return DNPAUTH_FAILUPDKEY_CHANGE_THRESHOLD;    
     break;    
    case  DNPAUTH_REKEY_DUE_RESTART_INDEX:        
      return DNPAUTH_REKEY_DUE_RESTART_THRESHOLD;  
     break;     
   }
   return 0; 
#endif
}

/* function: sdnpdata_authSecStatSet */
void TMWDEFS_GLOBAL sdnpdata_authSecStatSet(
  void            *pPoint,
  TMWTYPES_ULONG   value)
{ 
#if TMWCNFG_USE_SIMULATED_DB
  sdnpsim_authSecStatSet(pPoint, value);
#elif TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_AuthSecStatSet(pPoint, value);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(value); 
#endif
}
 
/* function: sdnpdata_authSesnKeyStatCount */
void TMWDEFS_GLOBAL sdnpdata_authSesnKeyStatCount(
  void            *pHandle,   
  TMWTYPES_USHORT  assocId,
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_ULONG   sequenceNumber)
{
  /* Number of session key status requests has exceeded configured maximum allowed
   * Send an error message on the other associations using the following function.
   * sdnpo120_sendErrorResponseV7(pSessionForOtherAssociation, rxAddress, userNumber, sequenceNumber, 
   * DNPAUTH_ERROR_MAX_STATUS_REQ, "optionalErrorText", (TMWTYPES_UCHAR)strlen("optionalErrorText")); 
   */ 

#if TMWCNFG_USE_MANAGED_SCL
  SDNPDatabaseWrapper_AuthSesnKeyStatCount(pHandle, assocId, userNumber, sequenceNumber);
#else
   /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(assocId);
  TMWTARG_UNUSED_PARAM(userNumber);
  TMWTARG_UNUSED_PARAM(sequenceNumber);
#endif

}

/* function: sdnpdata_authLogTx */
void TMWDEFS_GLOBAL sdnpdata_authLogTx(
  void            *pHandle,
  TMWTYPES_UCHAR   variation,
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_ULONG   sequenceNumber,
  TMWTYPES_UCHAR  *pMsgBuf,
  TMWTYPES_USHORT  msgLength)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(variation);
  TMWTARG_UNUSED_PARAM(userNumber);
  TMWTARG_UNUSED_PARAM(sequenceNumber);
  TMWTARG_UNUSED_PARAM(pMsgBuf);
  TMWTARG_UNUSED_PARAM(msgLength);
}

/* function: sdnpdata_authLogRx */
void TMWDEFS_GLOBAL sdnpdata_authLogRx(
  void            *pHandle, 
  TMWTYPES_UCHAR   variation,
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_ULONG   sequenceNumber,
  TMWTYPES_UCHAR  *pMsgBuf,
  TMWTYPES_USHORT  msgLength)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(variation);
  TMWTARG_UNUSED_PARAM(userNumber);
  TMWTARG_UNUSED_PARAM(sequenceNumber);
  TMWTARG_UNUSED_PARAM(pMsgBuf);
  TMWTARG_UNUSED_PARAM(msgLength);
}


/* function: sdnpdata_authLogUnexpectedMsg */
void TMWDEFS_GLOBAL sdnpdata_authLogUnexpectedMsg(
  void            *pHandle, 
  TMWTYPES_UCHAR   state, 
  TMWTYPES_ULONG   event, 
  TMWSESN_RX_DATA *pRxFragment)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(state);
  TMWTARG_UNUSED_PARAM(event);
  TMWTARG_UNUSED_PARAM(pRxFragment);
}

void TMWDEFS_GLOBAL sdnpdata_authLogMaxRekeyTCPClose(
  void            *pHandle)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
}

#endif /* SDNPCNFG_SUPPORT_SA_VERSION5 */

#if SDNPCNFG_SUPPORT_SA_VERSION2

/* function: sdnpdata_authLogChallTx */
void TMWDEFS_GLOBAL sdnpdata_authLogChallTx(
  void            *pHandle,
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_ULONG   sequenceNumber,  
  TMWTYPES_UCHAR   macAlgorithm,
  TMWTYPES_UCHAR   reason)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(userNumber);
  TMWTARG_UNUSED_PARAM(sequenceNumber);
  TMWTARG_UNUSED_PARAM(macAlgorithm);
  TMWTARG_UNUSED_PARAM(reason);
}

/* function: sdnpdata_authLogChallRplyTx */
void TMWDEFS_GLOBAL sdnpdata_authLogChallRplyTx(
  void            *pHandle,
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_ULONG   sequenceNumber)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(userNumber);
  TMWTARG_UNUSED_PARAM(sequenceNumber);
}

/* function: sdnpdata_authLogKeyStatusTx */
void sdnpdata_authLogKeyStatusTx(
  void            *pHandle, 
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_ULONG   sequenceNumber,
  TMWTYPES_UCHAR   keyWrapAlgorithm,
  TMWTYPES_UCHAR   keyStatus,
  TMWTYPES_UCHAR   macAlgorithm)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(userNumber);
  TMWTARG_UNUSED_PARAM(sequenceNumber);
  TMWTARG_UNUSED_PARAM(keyWrapAlgorithm);
  TMWTARG_UNUSED_PARAM(keyStatus);
  TMWTARG_UNUSED_PARAM(macAlgorithm);
}

/* function: sdnpdata_authLogAggrTx */
void TMWDEFS_GLOBAL sdnpdata_authLogAggrTx(
  void            *pHandle,
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_ULONG   sequenceNumber)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(userNumber);
  TMWTARG_UNUSED_PARAM(sequenceNumber); 
}

/* function: sdnpdata_authLogChallRx */
void TMWDEFS_GLOBAL sdnpdata_authLogChallRx(
  void            *pHandle,
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_ULONG   sequenceNumber,  
  TMWTYPES_UCHAR   macAlgorithm,
  TMWTYPES_UCHAR   reason)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(userNumber);
  TMWTARG_UNUSED_PARAM(sequenceNumber);
  TMWTARG_UNUSED_PARAM(macAlgorithm);
  TMWTARG_UNUSED_PARAM(reason); 
}

/* function: sdnpdata_authLogChallRplyRx */
void TMWDEFS_GLOBAL sdnpdata_authLogChallRplyRx(
  void            *pHandle,
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_ULONG   sequenceNumber,  
  TMWTYPES_BOOL    status)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(userNumber);
  TMWTARG_UNUSED_PARAM(sequenceNumber);
  TMWTARG_UNUSED_PARAM(status); 
} 

/* function: sdnpdata_authLogAggrRx */
void TMWDEFS_GLOBAL sdnpdata_authLogAggrRx(
  void            *pHandle,
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_ULONG   sequenceNumber,  
  TMWTYPES_BOOL    status)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(userNumber);
  TMWTARG_UNUSED_PARAM(sequenceNumber);
  TMWTARG_UNUSED_PARAM(status); 
}
 
/* function: sdnpdata_authLogKeyStatRqRx */
void TMWDEFS_GLOBAL sdnpdata_authLogKeyStatRqRx(
  void            *pHandle,
  TMWTYPES_USHORT  userNumber)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(userNumber);
}

/* function: sdnpdata_authLogKeyChangeRx */
void TMWDEFS_GLOBAL sdnpdata_authLogKeyChangeRx(
  void            *pHandle,
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_ULONG   sequenceNumber,  
  TMWTYPES_BOOL    status)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(userNumber);
  TMWTARG_UNUSED_PARAM(sequenceNumber);
  TMWTARG_UNUSED_PARAM(status);
}
   
typedef struct {
  TMWTYPES_UCHAR bytes[8];
} SDNPDATA_R_TYPE;

/* function: sdnpdata_authDecryptKeyWrapData */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authDecryptKeyWrapData(
  void            *pHandle,
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_UCHAR   algorithm,
  TMWTYPES_UCHAR  *pEncryptedData, 
  TMWTYPES_USHORT  encryptedValueLength, 
  TMWTYPES_UCHAR  *pPlainValue,
  TMWTYPES_USHORT *pPlainLength)
{
#if TMWCNFG_USE_SIMULATED_DB && !TMWCNFG_SUPPORT_CRYPTO
  /* Use simulated only if crypto is not available */
  return sdnpsim_authDecryptKeyWrapData(
      pHandle,
      userNumber,
      algorithm,
      pEncryptedData, 
      encryptedValueLength, 
      pPlainValue,
      pPlainLength);

#elif TMWCNFG_USE_MANAGED_SCL
  /* Prefer managed over crypto, because managed uses windows crypto library */
  if(algorithm == DNPAUTH_KEYWRAP_AES128)
  {
    /* This code implements the AES Key UnWrap Algorithm specified in RFC3394,
     * It calls a function that provides the AES decryption algorithm specified 
     * in FIPS 197 multiple times based on the length of the encrypted data.
     */ 
    int i;
    int j;
    int n;
    SDNPDATA_R_TYPE R[20]; 
    TMWTYPES_UCHAR input[16];
    TMWTYPES_UCHAR A[16];
    TMWTYPES_UCHAR B[16];
    
    n = (encryptedValueLength/8)-1;
    /* make sure R array is large enough */
    if(n > 19)
      return TMWDEFS_FALSE;

    /* Set A= C[0]  */
    memcpy(A, pEncryptedData, 8); 
    
    /* For i=1 to n, R[i]=C[i] */ 
    memcpy(&R[1], pEncryptedData+8, encryptedValueLength-8);

    for(j=5; j>=0; j--)
    {  
      for(i=n; i>0; i--)
      { 
        TMWTYPES_USHORT bLength = 16;

        /* B = AES-1(K, (A ^ t) | R[i]) where t = n*j+i */
        A[7] ^= (n*j) +i;
        memcpy(&input[0], A, 8);
        memcpy(&input[8], &R[i], 8); 

        SDNPDatabaseWrapper_AuthAESDecrypt(
          pHandle,
          userNumber, 
          input, 
          16, 
          B,
          &bLength);
        /* The returned decrypted data would also be 16 bytes long */
     
        /* A = MSB(64, B) */
        memcpy(A, B, 8); 
          
        /* R[i] = LSB(64, B) */
        memcpy(&R[i], &B[8], 8); 
      }
    }

    for (int k = 0; k < 8; k++)
    {
      if (A[k] != 0xA6)
        return TMWDEFS_FALSE;
    }

    /* For i = 1 to n, P[i] = R[i] */
    *pPlainLength = encryptedValueLength-8;
    memcpy(pPlainValue, &R[1], encryptedValueLength-8);
    return TMWDEFS_TRUE;
  }
  return TMWDEFS_FALSE;

#elif TMWCNFG_SUPPORT_CRYPTO 
  /*  If TMWCRYPTO interface is supported (it is required for SAv5) use it for SAv2. */
  SDNPSIM_DATABASE *pDb = (SDNPSIM_DATABASE *)pHandle;
  void *pCryptoHandle = pDb->pSDNPSession->dnp.pCryptoHandle;

  TMWCRYPTO_KEY updateKey;  
  TMWTYPES_BOOL status = tmwcrypto_getKey(pCryptoHandle, TMWCRYPTO_USER_UPDATE_KEY, (void*) userNumber, &updateKey);
  if(status)
  {
    return tmwcrypto_decryptData(pCryptoHandle, dnpauth_keyWraptoTMWCryptoAlgo(algorithm),
       &updateKey, pEncryptedData, encryptedValueLength, pPlainValue, pPlainLength);
  }
  else
    return TMWDEFS_FALSE;

#else
  /* Put target code here */
  /* You may want to start with the C code above, that implements the AES Key UnWrap algorithm and 
   * replace the call to SDNPDatabaseWrapper_AuthAESDecrypt(...) with your own decrypt function. 
   * Or you can choose to implement your own key unwrap function as described in RFC3394 
   */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(userNumber);
  TMWTARG_UNUSED_PARAM(algorithm);
  TMWTARG_UNUSED_PARAM(pEncryptedData);
  TMWTARG_UNUSED_PARAM(encryptedValueLength);
  TMWTARG_UNUSED_PARAM(pPlainValue);
  TMWTARG_UNUSED_PARAM(pPlainLength);
  return(TMWDEFS_FALSE);
#endif
}

TMWTYPES_CHAR sdnpdata_authHMACSupport(
  TMWTYPES_UCHAR HMACAlgorithm)
{
#if TMWCNFG_USE_SIMULATED_DB && !TMWCNFG_SUPPORT_CRYPTO
  /* Use simulated only if crypto is not available */
  return sdnpsim_authHMACSupport(HMACAlgorithm);
  
#elif TMWCNFG_USE_MANAGED_SCL
  /* Prefer managed over crypto, because managed uses windows crypto library */
  return SDNPDatabaseWrapper_AuthHMACSupport(HMACAlgorithm);

#elif TMWCNFG_SUPPORT_CRYPTO
  /*  If TMWCRYPTO interface is supported (it is required for SAv5) use it for SAv2. */
  if(tmwcrypto_algorithmSupport(TMWDEFS_NULL, dnpauth_MACtoTMWCryptoAlgo(HMACAlgorithm)))
    return dnpauth_MACtoLength(HMACAlgorithm);
  else
    return(0);

#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(HMACAlgorithm);
  return(0);
#endif
}

/* function: sdnpdata_authHMACValue */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authHMACValue(
  TMWTYPES_UCHAR    algorithm,
  DNPDATA_AUTH_KEY *pKey,
  TMWTYPES_UCHAR   *pData,
  TMWTYPES_ULONG   dataLength,
  TMWTYPES_UCHAR   *pHMACValue,
  TMWTYPES_USHORT  *pHMACValueLength)
{ 
#if TMWCNFG_USE_SIMULATED_DB && !TMWCNFG_SUPPORT_CRYPTO
  /* Use simulated only if crypto is not available */
  return sdnpsim_authHMACValue(
      algorithm,
      pKey,
      pData,
      dataLength,
      pHMACValue,
      pHMACValueLength);
  
#elif TMWCNFG_USE_MANAGED_SCL
  /* Prefer managed over crypto, because managed uses windows crypto library */
  return SDNPDatabaseWrapper_AuthHMACValue(
      algorithm,
      pKey,
      pData,
      dataLength,
      pHMACValue,
      pHMACValueLength);

#elif TMWCNFG_SUPPORT_CRYPTO 
  /*  If TMWCRYPTO interface is supported (it is required for SAv5) use it for SAv2. */
  TMWCRYPTO_KEY key;
  memcpy(key.value, pKey->value, pKey->length);
  key.length = pKey->length;
    return tmwcrypto_MACValue(TMWDEFS_NULL, dnpauth_MACtoTMWCryptoAlgo(algorithm), 
      &key, dnpauth_MACtoLength(algorithm), 
      pData, dataLength, 
      pHMACValue, pHMACValueLength);

#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(algorithm);
  TMWTARG_UNUSED_PARAM(pKey);
  TMWTARG_UNUSED_PARAM(pData);
  TMWTARG_UNUSED_PARAM(dataLength);
  TMWTARG_UNUSED_PARAM(pHMACValue);
  TMWTARG_UNUSED_PARAM(pHMACValueLength);
  return(TMWDEFS_FALSE);
#endif
}

/* function: sdnpdata_authRandomChallengeData
 * purpose:  generate pseudo-random data, at least 4 bytes long, using algorithm specified
 *   in FIPS 186-2 Digital Signal Standard 
 * arguments: 
 * returns:
 *  TMWDEFS_TRUE of successful
 *  TMWDEFS_FALSE otherwise
 */ 
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authRandomChallengeData(
  TMWTYPES_UCHAR  *pBuf,
  TMWTYPES_USHORT  minLength,
  TMWTYPES_USHORT *pLength)
{  
#if TMWCNFG_USE_SIMULATED_DB && !TMWCNFG_SUPPORT_CRYPTO
  /* Use simulated only if crypto is not available */
  return sdnpsim_authRandomChallengeData(
      minLength,
      pBuf,
      pLength);
  
#elif TMWCNFG_USE_MANAGED_SCL
  /* Prefer managed over crypto, because managed uses windows crypto library */
  return SDNPDatabaseWrapper_AuthRandomChallengeData(
      pBuf,
      minLength,
      pLength);

#elif TMWCNFG_SUPPORT_CRYPTO 
  /*  If TMWCRYPTO interface is supported (it is required for SAv5) use it for SAv2. */
  return tmwcrypto_getRandomData(TMWDEFS_NULL, minLength, pBuf, pLength); 

#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pBuf);
  TMWTARG_UNUSED_PARAM(minLength);
  TMWTARG_UNUSED_PARAM(pLength);
  return(TMWDEFS_FALSE);
#endif
}
#endif
#endif

#if SDNPDATA_SUPPORT_XML2 && SDNPDATA_SUPPORT_OBJ70
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_deviceProfileFile(
  void *pHandle,
  TMWTYPES_CHAR *pFileName,
  TMWTYPES_ULONG authKey,
  TMWTYPES_BOOL *pAuthKeyOK)
{
#if TMWCNFG_USE_SIMULATED_DB
  SDNPSIM_DATABASE *pDb = (SDNPSIM_DATABASE *)pHandle;
  SDNPFSIM_DATABASE *pFDBHandle = (SDNPFSIM_DATABASE *)&(pDb->sdnpFsim); 
  return(sdnpfsim_deviceProfileFile(pFDBHandle, pFileName, authKey, pAuthKeyOK));
#elif TMWCNFG_USE_MANAGED_SCL
  return(SDNPDatabaseWrapper_DeviceProfileFile(pHandle, pFileName, authKey, pAuthKeyOK));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pFileName);
  TMWTARG_UNUSED_PARAM(authKey);
  TMWTARG_UNUSED_PARAM(pAuthKeyOK);
  return(TMWDEFS_FALSE);
#endif
}
#endif

#if SDNPDATA_SUPPORT_XML2

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetDeviceConfig(
  void *pHandle, 
  SDNPDATA_XML_DEVICECONFIG *pConfig)
{ 
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_XmlGetDeviceConfig(pHandle, pConfig);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_XmlGetDeviceConfig(pHandle, pConfig);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pConfig);
  return TMWDEFS_FALSE;
#endif
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetSerialConfig(
  void *pHandle, 
  SDNPDATA_XML_SERIALCONFIG *pConfig)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_XmlGetSerialConfig(pHandle, pConfig);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_XmlGetSerialConfig(pHandle, pConfig);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pConfig);
  return TMWDEFS_FALSE;
#endif
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetNetworkConfig(
  void *pHandle, 
  SDNPDATA_XML_NETWORKCONFIG *pConfig)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_XmlGetNetworkConfig(pHandle, pConfig);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_XmlGetNetworkConfig(pHandle, pConfig);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pConfig);
  return TMWDEFS_FALSE;
#endif
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetOutstationConfig(
  void *pHandle, 
  SDNPDATA_XML_OUTSTATIONCONFIG *pConfig)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_XmlGetOutstationConfig(pHandle, pConfig);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_XmlGetOutstationConfig(pHandle, pConfig);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pConfig);
  return TMWDEFS_FALSE;
#endif
}   

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetOutstationPerform(
  void *pHandle, 
  SDNPDATA_XML_OUTSTATIONPERFORM *pConfig)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_XmlGetOutstationPerform(pHandle, pConfig);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_XmlGetOutstationPerform(pHandle, pConfig);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pConfig);
  return TMWDEFS_FALSE;
#endif
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetFieldConfig(
  void *pHandle, 
  SDNPDATA_XML_FIELDCONFIG *pConfig)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_XmlGetFieldConfig(pHandle, pConfig);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_XmlGetFieldConfig(pHandle, pConfig);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pConfig);
  return TMWDEFS_FALSE;
#endif
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetSecurityConfig(
  void *pHandle, 
  SDNPDATA_XML_SECURITYCONFIG *pConfig)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_XmlGetSecurityConfig(pHandle, pConfig);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_XmlGetSecurityConfig(pHandle, pConfig);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pConfig);
  return TMWDEFS_FALSE;
#endif
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetBinOutGroupConfig(
  void *pHandle, 
  SDNPDATA_XML_BINOUTGROUPCONFIG *pConfig)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_XmlGetBinOutGroupConfig(pHandle, pConfig);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_XmlGetBinOutGroupConfig(pHandle, pConfig);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pConfig);
  return TMWDEFS_FALSE;
#endif
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetCntrGroupConfig(
  void *pHandle, 
  SDNPDATA_XML_COUNTERGROUPCONFIG *pConfig)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_XmlGetCntrGroupConfig(pHandle, pConfig);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_XmlGetCntrGroupConfig(pHandle, pConfig);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pConfig);
  return TMWDEFS_FALSE;
#endif
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetAnlgInGroupConfig(
  void *pHandle, 
  SDNPDATA_XML_ANLGINGROUPCONFIG *pConfig)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_XmlGetAnlgInGroupConfig(pHandle, pConfig);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_XmlGetAnlgInGroupConfig(pHandle, pConfig);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pConfig);
  return TMWDEFS_FALSE;
#endif
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetFileConfig(
  void *pHandle, 
  SDNPDATA_XML_FILECONFIG *pConfig)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_XmlGetFileConfig(pHandle, pConfig);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_XmlGetFileConfig(pHandle, pConfig);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pConfig);
  return TMWDEFS_FALSE;
#endif
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetPointListDefinition(
  void *pHandle, 
  SDNPDATA_XML_POINTLISTCONFIG *pConfig)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_XmlGetPointListDefinition(pHandle, pConfig);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_XmlGetPointListDefinition(pHandle, pConfig);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pConfig);
  return TMWDEFS_FALSE;
#endif
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetBinInPointConfig(
  void *pPoint, 
  SDNPDATA_XML_BININPOINTCONFIG *pConfig)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_XmlGetBinInPointConfig(pPoint, pConfig);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_XmlGetBinInPointConfig(pPoint, pConfig);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pConfig);
  return TMWDEFS_FALSE;
#endif
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetDblInPointConfig(
  void *pPoint, 
  SDNPDATA_XML_DBLINPOINTCONFIG *pConfig)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_XmlGetDblInPointConfig(pPoint, pConfig);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_XmlGetDblInPointConfig(pPoint, pConfig);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pConfig);
  return TMWDEFS_FALSE;
#endif
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetBinOutPointConfig(
  void *pPoint, 
  SDNPDATA_XML_BINOUTPOINTCONFIG *pConfig)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_XmlGetBinOutPointConfig(pPoint, pConfig);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_XmlGetBinOutPointConfig(pPoint, pConfig);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pConfig);
  return TMWDEFS_FALSE;
#endif
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetCntrPointConfig(
  void *pPoint, 
  SDNPDATA_XML_CNTRPOINTCONFIG *pConfig)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_XmlGetCntrPointConfig(pPoint, pConfig);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_XmlGetCntrPointConfig(pPoint, pConfig);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pConfig);
  return TMWDEFS_FALSE;
#endif
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetAnlgInPointConfig(
  void *pPoint, 
  SDNPDATA_XML_ANLGINPOINTCONFIG *pConfig)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_XmlGetAnlgInPointConfig(pPoint, pConfig);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_XmlGetAnlgInPointConfig(pPoint, pConfig);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pConfig);
  return TMWDEFS_FALSE;
#endif
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetAnlgOutPointConfig(
  void *pPoint, 
  SDNPDATA_XML_ANLGOUTPOINTCONFIG *pConfig)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_XmlGetAnlgOutPointConfig(pPoint, pConfig);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_XmlGetAnlgOutPointConfig(pPoint, pConfig);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pConfig);
  return TMWDEFS_FALSE;
#endif
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetStringPointConfig(
  void *pPoint, 
  SDNPDATA_XML_STRINGPOINTCONFIG *pConfig)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_XmlGetStringPointConfig(pPoint, pConfig);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_XmlGetStringPointConfig(pPoint, pConfig);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pConfig);
  return TMWDEFS_FALSE;
#endif
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetVtermPointConfig(
  void *pPoint, 
  SDNPDATA_XML_VTERMPOINTCONFIG *pConfig)
{
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_XmlGetVtermPointConfig(pPoint, pConfig);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_XmlGetVtermPointConfig(pPoint, pConfig);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pPoint);
  TMWTARG_UNUSED_PARAM(pConfig);
  return TMWDEFS_FALSE;
#endif
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetPerFileConfig(
  void *pHandle, 
  TMWTYPES_ULONG index, 
  SDNPDATA_XML_PERFILECONFIG *pConfig)
{
  TMWTARG_UNUSED_PARAM(pHandle);
#if TMWCNFG_USE_SIMULATED_DB
  return sdnpsim_XmlGetPerFileConfig(index, pConfig);
#elif TMWCNFG_USE_MANAGED_SCL
  return SDNPDatabaseWrapper_XmlGetPerFileConfig(pHandle, index, pConfig);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(index);
  TMWTARG_UNUSED_PARAM(pConfig);
  return TMWDEFS_FALSE;
#endif
}


/* function: sdnpdata_binOutGetSupCtrl */
SDNPDATA_XML_SUP TMWDEFS_GLOBAL sdnpdata_binOutGetSupCtrl(
  void *pPoint)
{
  TMWTARG_UNUSED_PARAM(pPoint);
#if TMWCNFG_USE_SIMULATED_DB
  /* This is the same for all points in the simulated database. */
  return (SDNPDATA_XML_SUP_SELOP
    | SDNPDATA_XML_SUP_DIROP
    | SDNPDATA_XML_SUP_DIRNOACK
    | SDNPDATA_XML_SUP_MORE_ONE
    | SDNPDATA_XML_SUP_SEL_CANCEL); 
#else
  /* Put target code here for each point if it differs */
  return (SDNPDATA_XML_SUP_SELOP
      | SDNPDATA_XML_SUP_DIROP
      | SDNPDATA_XML_SUP_DIRNOACK
      | SDNPDATA_XML_SUP_MORE_ONE
      | SDNPDATA_XML_SUP_SEL_CANCEL);
#endif
}

/* function: sdnpdata_anlgOutGetSupCtrl */
SDNPDATA_XML_SUP TMWDEFS_GLOBAL sdnpdata_anlgOutGetSupCtrl(
  void *pPoint)
{
  TMWTARG_UNUSED_PARAM(pPoint);
#if TMWCNFG_USE_SIMULATED_DB
  /* This is the same for all points in the simulated database. */
  return (SDNPDATA_XML_SUP_SELOP
    | SDNPDATA_XML_SUP_DIROP
    | SDNPDATA_XML_SUP_DIRNOACK);
#else
  /* Put target code here for each point if it differs */
  return (SDNPDATA_XML_SUP_SELOP
    | SDNPDATA_XML_SUP_DIROP
    | SDNPDATA_XML_SUP_DIRNOACK);
#endif
}

#endif /* SDNPDATA_SUPPORT_XML2 */
