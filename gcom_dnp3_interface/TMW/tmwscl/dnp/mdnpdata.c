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

/* file: mdnpdata.c
 * description: This file defines the interface between the Triangle
 *  MicroWorks, Inc. DNP master source code library and the target database.
 *  The default implementation calls methods in the DNP Master Database
 *  simulator. These need to be repaced with code to interface with the
 *  device's database.
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/dnp/mdnpdata.h"
#if TMWCNFG_USE_SIMULATED_DB
#include "tmwscl/dnp/mdnpsim.h"
#include "tmwscl/dnp/mdnpfsim.h"
#endif
#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwcrypto.h"
  
#if TMWCNFG_USE_MANAGED_SCL
#undef TMWCNFG_USE_SIMULATED_DB
#define TMWCNFG_USE_SIMULATED_DB TMWDEFS_FALSE
#endif

#if TMWCNFG_USE_MANAGED_SCL
#include "tmwscl/.NET/TMW.SCL/MDNPDataBaseWrapper.h"
#if MDNPCNFG_SUPPORT_SA_VERSION5 
#include "tmwscl/.NET/TMW.SCL/TMWCryptoWrapper.h"
#endif
#endif

#if TMWTARG_SUPPORT_DNPFILEIO
#include "tmwscl/dnp/mdnptarg.h"

/* If the target layer DNP file transfer is supported will need to store the
 * file context for each MDNP session. Using a global variable like this
 * for more than one session will NOT work.
 */
FILE  *pCurrentLocalFileHandle;
#endif

/* function: mdnpdata_processIIN */
void TMWDEFS_GLOBAL mdnpdata_processIIN(
  TMWSESN *pSession,
  TMWTYPES_USHORT *pIIN)
{
  //printf("%s  #1\n", __func__);
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pSession);
  TMWTARG_UNUSED_PARAM(pIIN);
  //printf("%s  #1  .. 1\n", __func__);
#elif TMWCNFG_USE_MANAGED_SCL
  printf("%s  #1  .. 2\n", __func__);
  MDNPDatabaseWrapper_ProcessIIN(pSession, pIIN);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pSession);
  TMWTARG_UNUSED_PARAM(pIIN);
  //printf("%s  #1  .. 3\n", __func__);
#endif
}

/* function: mdnpdata_storeIIN */
void TMWDEFS_GLOBAL mdnpdata_storeIIN(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_BOOL value)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(value);
#elif TMWCNFG_USE_MANAGED_SCL
  MDNPDatabaseWrapper_StoreIIN(pHandle, pointNumber, value);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(value);
#endif
}

/* function: mdnpdata_init */
void * TMWDEFS_GLOBAL mdnpdata_init(
  TMWSESN *pSession, 
  void *pUserHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pUserHandle);
  return(mdnpsim_init(pSession));
#elif TMWCNFG_USE_MANAGED_SCL
  return (MDNPDatabaseWrapper_Init(pSession, pUserHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pUserHandle);
  TMWTARG_UNUSED_PARAM(pSession);
#if TMWTARG_SUPPORT_DNPFILEIO
  pCurrentLocalFileHandle = TMWDEFS_NULL;
#endif
  TMWDIAG_ERROR("mdnpdata_init has not been implemented\n");
  return(TMWDEFS_NULL);
#endif
}

/* function: mdnpdata_close */
void TMWDEFS_GLOBAL mdnpdata_close(void *pHandle)
{
#if TMWCNFG_SUPPORT_ASYNCH_DB
  tmwdb_closeDatabase(pHandle);
#endif

#if TMWCNFG_USE_SIMULATED_DB
  mdnpsim_close(pHandle);
#elif TMWCNFG_USE_MANAGED_SCL
  MDNPDatabaseWrapper_Close(pHandle);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
#endif
}

/* function: mdnpdata_storeReadTime */
void TMWDEFS_GLOBAL mdnpdata_storeReadTime(
  void *pHandle, 
  TMWDTIME *pTimeStamp)
{
#if TMWCNFG_USE_SIMULATED_DB 
  mdnpsim_storeReadTime(pHandle, pTimeStamp);
#elif TMWCNFG_USE_MANAGED_SCL
  MDNPDatabaseWrapper_StoreReadTime(pHandle, pTimeStamp);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pTimeStamp);
#endif
}

/* function: mdnpdata_storeBinaryInput */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeBinaryInput(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR flags,
  TMWTYPES_BOOL isEvent,
  TMWDTIME *pTimeStamp)
{
#if TMWCNFG_USE_SIMULATED_DB
  /* Extract boolean value from DNP flags */
  TMWTYPES_BOOL value;
  TMWTARG_UNUSED_PARAM(isEvent);
  value = (TMWTYPES_BOOL)((flags & DNPDEFS_DBAS_FLAG_BINARY_ON) ? TMWDEFS_TRUE : TMWDEFS_FALSE);
  return(mdnpsim_storeBinaryInput(pHandle, pointNumber, value, flags, pTimeStamp));
#elif TMWCNFG_USE_MANAGED_SCL
  /* Extract boolean value from DNP flags */
  TMWTYPES_BOOL value;
  value = (TMWTYPES_BOOL)((flags & DNPDEFS_DBAS_FLAG_BINARY_ON) ? TMWDEFS_TRUE : TMWDEFS_FALSE);
  return (MDNPDatabaseWrapper_StoreBinaryInput(pHandle, pointNumber, value, flags, isEvent, pTimeStamp));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(flags);
  TMWTARG_UNUSED_PARAM(isEvent);
  TMWTARG_UNUSED_PARAM(pTimeStamp);
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_storeDoubleInput */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeDoubleInput(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR flags,
  TMWTYPES_BOOL isEvent,
  TMWDTIME *pTimeStamp)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(isEvent);
  return(mdnpsim_storeDoubleInput(pHandle, pointNumber, flags, pTimeStamp));
#elif TMWCNFG_USE_MANAGED_SCL
  return (MDNPDatabaseWrapper_StoreDoubleInput(pHandle, pointNumber, flags, isEvent, pTimeStamp));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(flags);
  TMWTARG_UNUSED_PARAM(isEvent);
  TMWTARG_UNUSED_PARAM(pTimeStamp);
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_storeBinaryOutput */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeBinaryOutput(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR flags,
  TMWTYPES_BOOL isEvent,
  TMWDTIME *pTimeStamp)
{
#if TMWCNFG_USE_SIMULATED_DB
  /* Extract boolean value from DNP flags */
  TMWTYPES_BOOL value = (TMWTYPES_BOOL)((flags & DNPDEFS_DBAS_FLAG_BINARY_ON) ? TMWDEFS_TRUE : TMWDEFS_FALSE);
  TMWTARG_UNUSED_PARAM(isEvent);
  return(mdnpsim_storeBinaryOutput(pHandle, pointNumber, value, flags, pTimeStamp));
#elif TMWCNFG_USE_MANAGED_SCL
  /* Extract boolean value from DNP flags */
  TMWTYPES_BOOL value = (TMWTYPES_BOOL)((flags & DNPDEFS_DBAS_FLAG_BINARY_ON) ? TMWDEFS_TRUE : TMWDEFS_FALSE);
  return(MDNPDatabaseWrapper_StoreBinaryOutput(pHandle, pointNumber, value, flags, isEvent, pTimeStamp));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(isEvent);
  TMWTARG_UNUSED_PARAM(pTimeStamp);
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(flags);
  return(TMWDEFS_FALSE);
#endif
} 

/* function: mdnpdata_storeBinaryOutput */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeBinaryCmdStatus(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR status,
  TMWTYPES_BOOL isEvent,
  TMWDTIME *pTimeStamp)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(isEvent);
  TMWTARG_UNUSED_PARAM(pTimeStamp);
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(status);
  return(TMWDEFS_FALSE);
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_StoreBinaryCmdStatus(pHandle, pointNumber, status, isEvent, pTimeStamp));         
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(isEvent);
  TMWTARG_UNUSED_PARAM(pTimeStamp);
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(status);
  return(TMWDEFS_FALSE);
#endif
  
}

/* function: mdnpdata_storeBinaryCounter */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeBinaryCounter(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_ULONG value,
  TMWTYPES_UCHAR flags,
  TMWTYPES_BOOL isEvent,
  TMWDTIME *pTimeStamp)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(isEvent);
  return(mdnpsim_storeBinaryCounter(pHandle, pointNumber, value, flags, pTimeStamp));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_StoreBinaryCounter(pHandle, pointNumber, value, flags, isEvent, pTimeStamp));         
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(value);
  TMWTARG_UNUSED_PARAM(flags);
  TMWTARG_UNUSED_PARAM(isEvent);
  TMWTARG_UNUSED_PARAM(pTimeStamp);
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_storeFrozenCounter */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeFrozenCounter(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_ULONG value,
  TMWTYPES_UCHAR flags,
  TMWTYPES_BOOL isEvent,
  TMWDTIME *pTimeStamp)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(isEvent);
  return(mdnpsim_storeFrozenCounter(pHandle, pointNumber, value, flags, pTimeStamp));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_StoreFrozenCounter(pHandle, pointNumber, value, flags, isEvent, pTimeStamp));         
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(value);
  TMWTARG_UNUSED_PARAM(flags);
  TMWTARG_UNUSED_PARAM(isEvent);
  TMWTARG_UNUSED_PARAM(pTimeStamp);
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_storeAnalogInput */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeAnalogInput(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_ANALOG_VALUE *pValue, 
  TMWTYPES_UCHAR flags,
  TMWTYPES_BOOL isEvent,
  TMWDTIME *pTimeStamp)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(isEvent);
  return(mdnpsim_storeAnalogInput(pHandle, pointNumber, pValue, flags, pTimeStamp));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_StoreAnalogInput(pHandle, pointNumber, pValue, flags, isEvent, pTimeStamp));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(pValue);
  TMWTARG_UNUSED_PARAM(flags);
  TMWTARG_UNUSED_PARAM(isEvent);
  TMWTARG_UNUSED_PARAM(pTimeStamp);
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_storeFrozenAnalogInput */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeFrozenAnalogInput(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_ANALOG_VALUE *pValue, 
  TMWTYPES_UCHAR flags,
  TMWTYPES_BOOL isEvent,
  TMWDTIME *pTimeStamp)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(isEvent);
  return(mdnpsim_storeFrozenAnalogInput(pHandle, pointNumber, pValue, flags, pTimeStamp));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_StoreFrozenAnalogInput(pHandle, pointNumber, pValue, flags, isEvent, pTimeStamp));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(pValue);
  TMWTARG_UNUSED_PARAM(flags);
  TMWTARG_UNUSED_PARAM(isEvent);
  TMWTARG_UNUSED_PARAM(pTimeStamp);
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_storeAnalogInputDeadband */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeAnalogInputDeadband(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_ANALOG_VALUE *pValue)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpsim_storeAnalogInputDeadband(pHandle, pointNumber, pValue));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_StoreAnalogInputDeadband(pHandle, pointNumber, pValue));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(pValue);
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_storeAnalogOutput */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeAnalogOutput(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR flags,
  TMWTYPES_BOOL isEvent,
  TMWDTIME *pTimeStamp)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(isEvent);
  return(mdnpsim_storeAnalogOutput(pHandle, pointNumber, pValue, flags, pTimeStamp));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_StoreAnalogOutput(pHandle, pointNumber, pValue, flags, isEvent, pTimeStamp));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(pValue);
  TMWTARG_UNUSED_PARAM(flags);
  TMWTARG_UNUSED_PARAM(isEvent);
  TMWTARG_UNUSED_PARAM(pTimeStamp);
  return(TMWDEFS_FALSE);
#endif
}   

TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeAnalogCmdStatus(
  void *pHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_ANALOG_VALUE *pValue, 
  TMWTYPES_UCHAR status,
  TMWTYPES_BOOL isEvent,
  TMWDTIME *pTimeStamp)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(pValue);
  TMWTARG_UNUSED_PARAM(status);
  TMWTARG_UNUSED_PARAM(isEvent);
  TMWTARG_UNUSED_PARAM(pTimeStamp);
  return(TMWDEFS_FALSE);
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_StoreAnalogCmdStatus(pHandle, pointNumber, pValue, status, isEvent, pTimeStamp));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(pValue);
  TMWTARG_UNUSED_PARAM(status);
  TMWTARG_UNUSED_PARAM(isEvent);
  TMWTARG_UNUSED_PARAM(pTimeStamp);
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_storeRestartTime */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeRestartTime(
  void *pHandle,
  TMWTYPES_ULONG time)
{
#if TMWCNFG_USE_SIMULATED_DB
  mdnpsim_storeRestartTime(pHandle, time);
  return(TMWDEFS_FALSE);
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_StoreRestartTime(pHandle, time));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(time);
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_storeIndexedTime */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeIndexedTime(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWDTIME indexedTime,
  TMWTYPES_ULONG intervalCount,
  TMWTYPES_BYTE intervalUnit)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpsim_storeIndexedTime(pHandle, pointNumber, indexedTime, intervalCount, intervalUnit));
#elif TMWCNFG_USE_MANAGED_SCL 
  return(MDNPDatabaseWrapper_StoreIndexedTime(pHandle, pointNumber, indexedTime, intervalCount, intervalUnit));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(indexedTime);
  TMWTARG_UNUSED_PARAM(intervalCount);
  TMWTARG_UNUSED_PARAM(intervalUnit);
  return(TMWDEFS_FALSE);
#endif
}
/* function: mdnpdata_storeString */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeString(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR *pStrBuf,
  TMWTYPES_UCHAR strLen)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpsim_storeString(pHandle, pointNumber, pStrBuf, strLen));
#elif TMWCNFG_USE_MANAGED_SCL 
  return(MDNPDatabaseWrapper_StoreString(pHandle, pointNumber, pStrBuf, strLen));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(pStrBuf);
  TMWTARG_UNUSED_PARAM(strLen);
  return(TMWDEFS_FALSE);
#endif
}

#if MDNPDATA_SUPPORT_OBJ114
/* function: mdnpdata_storeExtString */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeExtString(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR *pStrBuf,
  TMWTYPES_USHORT strLen,
  TMWTYPES_UCHAR flags,
  TMWTYPES_BOOL isEvent,
  TMWDTIME *pTimeStamp)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpsim_storeExtString(pHandle, pointNumber, pStrBuf, strLen, flags, pTimeStamp));
#elif TMWCNFG_USE_MANAGED_SCL 
  return(MDNPDatabaseWrapper_StoreExtString(pHandle, pointNumber, pStrBuf, strLen, flags, isEvent, pTimeStamp));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(pStrBuf);
  TMWTARG_UNUSED_PARAM(strLen);
  return(TMWDEFS_FALSE);
#endif
} 
#endif

/* function: mdnpdata_storeActiveConfig */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeActiveConfig(
  void *pHandle, 
  TMWTYPES_UCHAR index,
  TMWTYPES_ULONG timeDelay,
  TMWTYPES_UCHAR statusCode,
  TMWTYPES_UCHAR *pStrBuf,
  TMWTYPES_UCHAR strLen)
{ 
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(index);
  TMWTARG_UNUSED_PARAM(timeDelay);
  TMWTARG_UNUSED_PARAM(statusCode);
  TMWTARG_UNUSED_PARAM(pStrBuf);
  TMWTARG_UNUSED_PARAM(strLen);
  return(TMWDEFS_FALSE);
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_StoreActiveConfig(pHandle, index, timeDelay, statusCode, pStrBuf, strLen));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(index);
  TMWTARG_UNUSED_PARAM(timeDelay);
  TMWTARG_UNUSED_PARAM(statusCode);
  TMWTARG_UNUSED_PARAM(pStrBuf);
  TMWTARG_UNUSED_PARAM(strLen);
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_storeVirtualTerminal */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeVirtualTerminal(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR *pVtermBuf,
  TMWTYPES_UCHAR vtermLen)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpsim_storeVirtualTerminal(pHandle, pointNumber, pVtermBuf, vtermLen));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_StoreVirtualTerminal(pHandle, pointNumber, pVtermBuf, vtermLen));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(pVtermBuf);
  TMWTARG_UNUSED_PARAM(vtermLen);
  return(TMWDEFS_FALSE);
#endif
}

#if MDNPDATA_SUPPORT_OBJ70
/* function: mdnpdata_storeAuthKey */
TMWTYPES_BOOL mdnpdata_storeFileAuthKey(
  void *pHandle,
  TMWTYPES_ULONG authKey)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpfsim_storeFileAuthKey(pHandle, authKey));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_StoreFileAuthKey(pHandle, authKey));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(authKey);
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_storeFileStatus */
TMWTYPES_BOOL mdnpdata_storeFileStatus(
  void *pHandle,
  TMWTYPES_ULONG handle,
  TMWTYPES_ULONG fileSize,
  TMWTYPES_USHORT maxBlockSize,
  TMWTYPES_USHORT requestId,
  DNPDEFS_FILE_CMD_STAT status,
  TMWTYPES_USHORT nOptionalChars,
  const TMWTYPES_CHAR *pOptionalChars)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpfsim_storeFileStatus(pHandle, handle, fileSize, maxBlockSize, requestId, 
    status, nOptionalChars, pOptionalChars));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_StoreFileStatus(pHandle, handle, fileSize, maxBlockSize, requestId, 
    status, nOptionalChars, pOptionalChars));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(handle);
  TMWTARG_UNUSED_PARAM(fileSize);
  TMWTARG_UNUSED_PARAM(maxBlockSize);
  TMWTARG_UNUSED_PARAM(requestId);
  TMWTARG_UNUSED_PARAM(status);
  TMWTARG_UNUSED_PARAM(nOptionalChars);
  TMWTARG_UNUSED_PARAM(pOptionalChars);
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_storeFileData */
TMWTYPES_BOOL mdnpdata_storeFileData(
  void *pHandle,
  TMWTYPES_ULONG handle,
  TMWTYPES_ULONG blockNumber,
  TMWTYPES_BOOL lastBlockFlag,
  TMWTYPES_USHORT nBytesInBlockData,
  const TMWTYPES_UCHAR *pBlockData)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpfsim_storeFileData(pHandle, handle, blockNumber, lastBlockFlag,
    nBytesInBlockData, pBlockData));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_StoreFileData(pHandle, handle, blockNumber, lastBlockFlag,
    nBytesInBlockData, pBlockData));
#elif TMWTARG_SUPPORT_DNPFILEIO
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(handle);
  TMWTARG_UNUSED_PARAM(blockNumber);
  TMWTARG_UNUSED_PARAM(lastBlockFlag); 
  return (mdnptarg_storeFileData(pCurrentLocalFileHandle, nBytesInBlockData, pBlockData));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(handle);
  TMWTARG_UNUSED_PARAM(blockNumber);
  TMWTARG_UNUSED_PARAM(lastBlockFlag);
  TMWTARG_UNUSED_PARAM(nBytesInBlockData);
  TMWTARG_UNUSED_PARAM(pBlockData);
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_storeFileDataStatus */
TMWTYPES_BOOL mdnpdata_storeFileDataStatus(
  void *pHandle,
  TMWTYPES_ULONG handle,
  TMWTYPES_ULONG blockNumber,
  TMWTYPES_BOOL lastBlockFlag,
  DNPDEFS_FILE_TFER_STAT status,
  TMWTYPES_USHORT nOptionalChars,
  const TMWTYPES_CHAR *pOptionalChars)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpfsim_storeFileDataStatus(pHandle, handle, blockNumber, lastBlockFlag, 
    status, nOptionalChars, pOptionalChars));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_StoreFileDataStatus(pHandle, handle, blockNumber, lastBlockFlag, 
    status, nOptionalChars, pOptionalChars));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(handle);
  TMWTARG_UNUSED_PARAM(blockNumber);
  TMWTARG_UNUSED_PARAM(lastBlockFlag);
  TMWTARG_UNUSED_PARAM(status);
  TMWTARG_UNUSED_PARAM(nOptionalChars);
  TMWTARG_UNUSED_PARAM(pOptionalChars);
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_storeFileInfo */
TMWTYPES_BOOL mdnpdata_storeFileInfo(
  void *pHandle,
  TMWTYPES_USHORT fileNameOffset,
  TMWTYPES_USHORT fileNameSize,
  DNPDEFS_FILE_TYPE fileType,
  TMWTYPES_ULONG fileSize,
  TMWDTIME *pFileCreationTime,
  DNPDEFS_FILE_PERMISSIONS permissions,
  const TMWTYPES_CHAR *pFileName)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(fileNameOffset);
  TMWTARG_UNUSED_PARAM(fileNameSize);
  return(mdnpfsim_storeFileInfo(pHandle, fileType, 
    fileSize, pFileCreationTime, permissions, pFileName));
#elif TMWCNFG_USE_MANAGED_SCL
  TMWTARG_UNUSED_PARAM(fileNameOffset);
  return(MDNPDatabaseWrapper_StoreFileInfo(pHandle, fileType, 
    fileSize, pFileCreationTime, permissions, pFileName, fileNameSize));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(fileNameOffset);
  TMWTARG_UNUSED_PARAM(fileNameSize);
  TMWTARG_UNUSED_PARAM(fileType);
  TMWTARG_UNUSED_PARAM(fileSize);
  TMWTARG_UNUSED_PARAM(pFileCreationTime);
  TMWTARG_UNUSED_PARAM(permissions);
  TMWTARG_UNUSED_PARAM(pFileName);
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_getFileAuthKey */
TMWTYPES_ULONG mdnpdata_getFileAuthKey(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpfsim_getFileAuthKey(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_GetFileAuthKey(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_openLocalFile */
TMWTYPES_BOOL mdnpdata_openLocalFile(
  void *pHandle,
  const TMWTYPES_CHAR *pLocalFileName, 
  DNPDEFS_FILE_MODE fileMode)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpfsim_openLocalFile(pHandle, pLocalFileName, fileMode));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_OpenLocalFile(pHandle, pLocalFileName, fileMode));
#elif TMWTARG_SUPPORT_DNPFILEIO
  TMWTARG_UNUSED_PARAM(pHandle);
  pCurrentLocalFileHandle =  mdnptarg_openLocalFile(pLocalFileName, fileMode);
  return (pCurrentLocalFileHandle != NULL);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pLocalFileName);
  TMWTARG_UNUSED_PARAM(fileMode);
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_closeLocalFile */
TMWTYPES_BOOL mdnpdata_closeLocalFile(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpfsim_closeLocalFile(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_CloseLocalFile(pHandle));
#elif TMWTARG_SUPPORT_DNPFILEIO
  TMWTARG_UNUSED_PARAM(pHandle);
  mdnptarg_closeLocalFile(pCurrentLocalFileHandle);
  pCurrentLocalFileHandle = TMWDEFS_NULL;
  return(TMWDEFS_TRUE);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(TMWDEFS_FALSE);
#endif
} 
  
TMWTYPES_BOOL mdnpdata_getLocalFileInfo(
  void *pHandle,
  const TMWTYPES_CHAR *pLocalFileName, 
  TMWTYPES_ULONG *pFileSize,
  TMWDTIME *pDateTime)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpfsim_getLocalFileInfo(pHandle, pLocalFileName, pFileSize, pDateTime));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_GetLocalFileInfo(pHandle, pLocalFileName, pFileSize, pDateTime));
#elif TMWTARG_SUPPORT_DNPFILEIO
  TMWTARG_UNUSED_PARAM(pHandle);
  return (mdnptarg_getLocalFileInfo(pLocalFileName, pFileSize, pDateTime));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pLocalFileName);
  TMWTARG_UNUSED_PARAM(pFileSize);
  TMWTARG_UNUSED_PARAM(pDateTime);
  return TMWDEFS_FALSE;
#endif
}

/* function: mdnpdata_readLocalFile */
TMWTYPES_USHORT mdnpdata_readLocalFile(
  void *pHandle,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT bufSize,
  TMWTYPES_BOOL *pLastBlock)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpfsim_readLocalFile(pHandle, pBuf, bufSize, pLastBlock));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_ReadLocalFile(pHandle, pBuf, bufSize, pLastBlock));
#elif TMWTARG_SUPPORT_DNPFILEIO
  TMWTARG_UNUSED_PARAM(pHandle);
  return (mdnptarg_readLocalFile(pCurrentLocalFileHandle, pBuf, bufSize, pLastBlock));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pBuf);
  TMWTARG_UNUSED_PARAM(bufSize);
  TMWTARG_UNUSED_PARAM(pLastBlock);
  return(0);
#endif
}
#endif
#if MDNPDATA_SUPPORT_OBJ0
void TMWDEFS_GLOBAL mdnpdata_storeDeviceAttribute(
  void *pHandle,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR variation,
  DNPDATA_ATTRIBUTE_VALUE *pData)
{
#if TMWCNFG_USE_SIMULATED_DB
  mdnpsim_storeDeviceAttribute(pHandle, point, variation, pData);
#elif TMWCNFG_USE_MANAGED_SCL
  MDNPDatabaseWrapper_StoreDeviceAttribute(pHandle, point, variation, pData);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(point);
  TMWTARG_UNUSED_PARAM(variation);
  TMWTARG_UNUSED_PARAM(pData);
#endif
}

void TMWDEFS_GLOBAL mdnpdata_storeDeviceAttrProperty(
  void *pHandle, 
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR variation,
  TMWTYPES_UCHAR property)
{
#if TMWCNFG_USE_SIMULATED_DB
  mdnpsim_storeDeviceAttrProperty(pHandle, point, variation, property);
#elif TMWCNFG_USE_MANAGED_SCL
  MDNPDatabaseWrapper_StoreDeviceAttrProperty(pHandle, point, variation, property);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(point);
  TMWTARG_UNUSED_PARAM(variation);
  TMWTARG_UNUSED_PARAM(property);
#endif
}
#endif

#if MDNPDATA_SUPPORT_OBJ85
/* function: mdnpdata_datasetProtoQuantity */
TMWTYPES_USHORT mdnpdata_datasetProtoQuantity(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpsim_datasetProtoQuantity(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_DatasetProtoQuantity(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(0);
#endif
}

/* function: mdnpdata_datasetProtoSlaveQty */
TMWTYPES_USHORT mdnpdata_datasetProtoSlaveQty(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpsim_datasetProtoSlaveQty(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_DatasetProtoSlaveQty(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(0);
#endif
}

/* function: mdnpdata_storeDatasetProtoUUID */
void TMWDEFS_GLOBAL mdnpdata_storeDatasetProtoUUID(
  void *pHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR *pUUID)
{ 
#if TMWCNFG_USE_SIMULATED_DB
  mdnpsim_storeDatasetProtoUUID(pHandle, pointNumber, pUUID, TMWDEFS_TRUE);
#elif TMWCNFG_USE_MANAGED_SCL
  MDNPDatabaseWrapper_StoreDatasetProtoUUID(pHandle, pointNumber, pUUID);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pointNumber); 
  TMWTARG_UNUSED_PARAM(pUUID); 
#endif
}

/* function: mdnpdata_storeDatasetProto */
void TMWDEFS_GLOBAL mdnpdata_storeDatasetProto(
  void *pHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR  elemIndex,
  DNPDATA_DATASET_DESCR_ELEM *pDescr)
{ 
#if TMWCNFG_USE_SIMULATED_DB
  mdnpsim_storeDatasetProto(pHandle, pointNumber, elemIndex, pDescr);
#elif TMWCNFG_USE_MANAGED_SCL
  MDNPDatabaseWrapper_StoreDatasetProto(pHandle, pointNumber, elemIndex, pDescr);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pointNumber); 
  TMWTARG_UNUSED_PARAM(elemIndex); 
  TMWTARG_UNUSED_PARAM(pDescr); 
#endif
}

/* function: mdnpdata_datasetProtoGetID */
TMWTYPES_BOOL TMWDEFS_CALLBACK mdnpdata_datasetProtoGetID(
  void *pHandle,
  TMWTYPES_UCHAR *pUUID,
  TMWTYPES_USHORT *pPointNum)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpsim_datasetProtoGetID(pHandle, pUUID, pPointNum));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_DatasetProtoGetID(pHandle, pUUID, pPointNum));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pUUID); 
  TMWTARG_UNUSED_PARAM(pPointNum); 
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_datasetProtoGet */
DNPDATA_DATASET_DESCR_ELEM * TMWDEFS_GLOBAL mdnpdata_datasetProtoGet(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR *pNumberElems,
  TMWTYPES_UCHAR *pUUID)
{ 
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpsim_datasetProtoGet(pHandle, pointNumber, pNumberElems, pUUID));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_DatasetProtoGet(pHandle, pointNumber, pNumberElems, pUUID));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pointNumber); 
  TMWTARG_UNUSED_PARAM(pNumberElems); 
  TMWTARG_UNUSED_PARAM(pUUID); 
  return(TMWDEFS_NULL);
#endif
}

/* function: mdnpdata_datasetProtoRelease */
void TMWDEFS_GLOBAL mdnpdata_datasetProtoRelease(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pHandle); 
#elif TMWCNFG_USE_MANAGED_SCL
  MDNPDatabaseWrapper_DatasetProtoRelease(pHandle);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
#endif
}
#endif

#if MDNPDATA_SUPPORT_OBJ86

/* function: mdnpdata_datasetDescrQuantity */
TMWTYPES_USHORT mdnpdata_datasetDescrQuantity(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpsim_datasetDescrQuantity(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_DatasetDescrQuantity(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(0);
#endif
}

/* function: mdnpdata_datasetDescrSlaveQty */
TMWTYPES_USHORT mdnpdata_datasetDescrSlaveQty(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpsim_datasetDescrSlaveQty(pHandle));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_DatasetDescrSlaveQty(pHandle));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  return(0);
#endif
}

/* function: mdnpdata_storeDatasetDescrCont */
void TMWDEFS_GLOBAL mdnpdata_storeDatasetDescrCont(
  void *pHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR  elemIndex,
  DNPDATA_DATASET_DESCR_ELEM *pDescr)
{
#if TMWCNFG_USE_SIMULATED_DB
  mdnpsim_storeDatasetDescrCont(pHandle, pointNumber, elemIndex, pDescr, TMWDEFS_TRUE);
#elif TMWCNFG_USE_MANAGED_SCL
  MDNPDatabaseWrapper_StoreDatasetDescrCont(pHandle, pointNumber, elemIndex, pDescr);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(pDescr);
  TMWTARG_UNUSED_PARAM(elemIndex);
#endif
}

/* function: mdnpdata_storeDatasetDescrChars */
void TMWDEFS_GLOBAL mdnpdata_storeDatasetDescrChars(
  void *pHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR  value)
{
#if TMWCNFG_USE_SIMULATED_DB
  mdnpsim_storeDatasetDescrChars(pHandle, pointNumber, value);
#elif TMWCNFG_USE_MANAGED_SCL
  MDNPDatabaseWrapper_StoreDatasetDescrChars(pHandle, pointNumber, value);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(value);
#endif
}

/* function: mdnpdata_storeDatasetDescrIndex */
void TMWDEFS_GLOBAL mdnpdata_storeDatasetDescrIndex(
  void *pHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR  elemIndex,
  DNPDATA_DATASET_DESCR_INDEX *pDescr)
{
#if TMWCNFG_USE_SIMULATED_DB
  mdnpsim_storeDatasetDescrIndex(pHandle, pointNumber, elemIndex, pDescr, TMWDEFS_TRUE);
#elif TMWCNFG_USE_MANAGED_SCL
  MDNPDatabaseWrapper_StoreDatasetDescrIndex(pHandle, pointNumber, elemIndex, pDescr);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(elemIndex);
  TMWTARG_UNUSED_PARAM(pDescr);
#endif
}

/* function: mdnpdata_datasetDescrGetCont */
DNPDATA_DATASET_DESCR_ELEM * TMWDEFS_GLOBAL mdnpdata_datasetDescrGetCont(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR *pNumberElems)
{ 
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpsim_datasetDescrGetCont(pHandle, pointNumber, pNumberElems));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_StoreDatasetDescrGetCont(pHandle, pointNumber, pNumberElems));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pointNumber); 
  TMWTARG_UNUSED_PARAM(pNumberElems); 
  return(TMWDEFS_NULL);
#endif
}

/* function: mdnpdata_datasetDescrGetIndex */
DNPDATA_DATASET_DESCR_INDEX * TMWDEFS_GLOBAL mdnpdata_datasetDescrGetIndex(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR *pNumberElems)
{ 
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpsim_datasetDescrGetIndex(pHandle, pointNumber, pNumberElems));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_StoreDatasetDescrGetIndex(pHandle, pointNumber, pNumberElems));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pointNumber); 
  TMWTARG_UNUSED_PARAM(pNumberElems); 
  return(TMWDEFS_NULL);
#endif
}

/* function: mdnpdata_datasetDescrRelease */
void TMWDEFS_GLOBAL mdnpdata_datasetDescrRelease(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pHandle); 
#elif TMWCNFG_USE_MANAGED_SCL
  MDNPDatabaseWrapper_DatasetDescrRelease(pHandle);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
#endif
}
#endif

#if (MDNPDATA_SUPPORT_OBJ87 || MDNPDATA_SUPPORT_OBJ88)

/* function: mdnpdata_storeDatasetTime */
void TMWDEFS_GLOBAL mdnpdata_storeDatasetTime(
  void *pHandle, 
  TMWTYPES_USHORT pointNumber,
  TMWDTIME *pTimeStamp,
  TMWTYPES_BOOL isEvent)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(isEvent);
  mdnpsim_storeDatasetTime(pHandle, pointNumber, pTimeStamp);
#elif TMWCNFG_USE_MANAGED_SCL
  TMWTARG_UNUSED_PARAM(isEvent);
  MDNPDatabaseWrapper_StoreDatasetTime(pHandle, pointNumber, pTimeStamp, isEvent);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(pTimeStamp);
  TMWTARG_UNUSED_PARAM(isEvent);
#endif
}

/* function: mdnpdata_storeDataset */
void TMWDEFS_GLOBAL mdnpdata_storeDataset(
  void *pHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR  elemIndex,
  DNPDATA_DATASET_VALUE *pElem,
  TMWTYPES_BOOL isEvent)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(isEvent);
  mdnpsim_storeDataset(pHandle, pointNumber, elemIndex, pElem);
#elif TMWCNFG_USE_MANAGED_SCL
  TMWTARG_UNUSED_PARAM(isEvent);
  MDNPDatabaseWrapper_StoreDataset(pHandle, pointNumber, elemIndex, pElem, isEvent);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  TMWTARG_UNUSED_PARAM(elemIndex);
  TMWTARG_UNUSED_PARAM(pElem);
  TMWTARG_UNUSED_PARAM(isEvent);
#endif
}

/* function: mdnpdata_datasetGet */
DNPDATA_DATASET_VALUE * TMWDEFS_GLOBAL mdnpdata_datasetGet(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR *pNumberElems,
  TMWDTIME *pTimeStamp)
{ 
#if TMWCNFG_USE_SIMULATED_DB
  return(mdnpsim_datasetGet(pHandle, pointNumber, pNumberElems, pTimeStamp));
#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_DatasetGet(pHandle, pointNumber, pNumberElems, pTimeStamp));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pointNumber); 
  TMWTARG_UNUSED_PARAM(pNumberElems); 
  TMWTARG_UNUSED_PARAM(pTimeStamp); 
  return(TMWDEFS_NULL);
#endif
}

/* function: mdnpdata_datasetRelease */
void TMWDEFS_GLOBAL mdnpdata_datasetRelease(
  void *pHandle)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pHandle); 
#elif TMWCNFG_USE_MANAGED_SCL
  MDNPDatabaseWrapper_DatasetRelease(pHandle);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
#endif
}
#endif

#if MDNPDATA_SUPPORT_OBJ120 
/* function: mdnpdata_authIsCriticalReq */
TMWTYPES_BOOL mdnpdata_authIsCriticalReq(
  void            *pHandle, 
  TMWTYPES_UCHAR  *pRxMsg,
  TMWTYPES_USHORT  msgLength,
  TMWTYPES_USHORT *pUserNumber)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pRxMsg); 
  TMWTARG_UNUSED_PARAM(msgLength);  
  TMWTARG_UNUSED_PARAM(pUserNumber);  
  return(TMWDEFS_FALSE);
#elif TMWCNFG_USE_MANAGED_SCL
  /* Spec says use Default User number 1 */
  *pUserNumber = 1;
  return MDNPDatabaseWrapper_AuthIsCriticalReq(pHandle, pRxMsg, msgLength);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pRxMsg); 
  TMWTARG_UNUSED_PARAM(msgLength); 

  /* Spec says use Default User number 1 */
  *pUserNumber = 1;
  return(TMWDEFS_FALSE);
#endif
}
 

#if MDNPCNFG_SUPPORT_SA_VERSION5
#if TMWCNFG_USE_MANAGED_SCL
/* function: _toCryptoHandle */
static TMWDEFS_LOCAL void *_toCryptoHandle(void *pHandle)
{
  MDNPSIM_DATABASE *pSim = (MDNPSIM_DATABASE *)pHandle;
  return pSim->managedDBhandle; 
}
#endif
#endif /* MDNPCNFG_SUPPORT_SA_VERSION5 || TMWCNFG_SUPPORT_CRYPTO */

#if MDNPCNFG_SUPPORT_SA_VERSION5 

/* TMWCNFG_SUPPORT_CRYPTO_AESGMAC
 * There is no need for StoreKSQ and GetKSQ when using AES-GMAC on master, 
 * since master always gets KSQ from outstation before using it.
 */

#if DNPCNFG_SUPPORT_AUTHKEYUPDATE  
/* function: mdnpdata_authGetOSName */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_authGetOSName(
  void            *pHandle, 
  TMWTYPES_UCHAR  *pOSName,
  TMWTYPES_USHORT *pOSNameLength)
{
#if TMWCNFG_USE_SIMULATED_DB
  return mdnpsim_authGetOSName(pHandle, pOSName, pOSNameLength);
#elif TMWCNFG_USE_MANAGED_SCL
  return TMWCryptoWrapper_getOSName(_toCryptoHandle(pHandle), 
    (TMWTYPES_CHAR*)pOSName, pOSNameLength);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pOSName); 
  TMWTARG_UNUSED_PARAM(pOSNameLength); 
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_authGetUserName */ 
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_authGetUserName(
  void            *pHandle, 
  void            *userNameDbHandle,
  TMWTYPES_UCHAR  *pUserName,
  TMWTYPES_USHORT *pUserNameLength)
{
#if TMWCNFG_USE_SIMULATED_DB
  return mdnpsim_authGetUserName(pHandle, userNameDbHandle, pUserName, pUserNameLength);
#elif TMWCNFG_USE_MANAGED_SCL   
  return TMWCryptoWrapper_getUserNameByHandle(_toCryptoHandle(pHandle), 
    userNameDbHandle, (TMWTYPES_CHAR*)pUserName, pUserNameLength);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(userNameDbHandle); 
  TMWTARG_UNUSED_PARAM(pUserName); 
  TMWTARG_UNUSED_PARAM(pUserNameLength); 
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_authGetKeyChangeMethod */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_authGetKeyChangeMethod(
  void            *pHandle, 
  void            *userNameDbHandle,
  TMWTYPES_UCHAR  *pKeyChangeMethod)
{ 
#if TMWCNFG_USE_SIMULATED_DB 
  /* Don't care about these three */
  TMWTYPES_ULONG  statusChangeSequenceNumber;
  TMWTYPES_USHORT userRole;
  TMWTYPES_USHORT userRoleExpiryInterval;
  return mdnpsim_authGetChangeUserData(pHandle, userNameDbHandle, &statusChangeSequenceNumber,
    pKeyChangeMethod, &userRole, &userRoleExpiryInterval);
#elif TMWCNFG_USE_MANAGED_SCL  
  /* Don't care about these three */
  TMWTYPES_ULONG  statusChangeSequenceNumber;
  TMWTYPES_USHORT userRole;
  TMWTYPES_USHORT userRoleExpiryInterval;
  return TMWCryptoWrapper_getChangeUserData(_toCryptoHandle(pHandle), userNameDbHandle,
    &statusChangeSequenceNumber,
    pKeyChangeMethod, &userRole, &userRoleExpiryInterval);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(userNameDbHandle);
  TMWTARG_UNUSED_PARAM(pKeyChangeMethod);
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_authGetChangeUserData */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_authGetChangeUserData(
  void            *pHandle, 
  void            *userNameDbHandle,
  TMWTYPES_ULONG  *pStatusChangeSequenceNumber,
  TMWTYPES_UCHAR  *pKeyChangeMethod,
  TMWTYPES_USHORT *pUserRole,
  TMWTYPES_USHORT *pUserRoleExpiryInterval)
{
#if TMWCNFG_USE_SIMULATED_DB 
  return mdnpsim_authGetChangeUserData(pHandle, userNameDbHandle, pStatusChangeSequenceNumber,
    pKeyChangeMethod, pUserRole, pUserRoleExpiryInterval);
#elif TMWCNFG_USE_MANAGED_SCL  
  return TMWCryptoWrapper_getChangeUserData(_toCryptoHandle(pHandle), userNameDbHandle,
    pStatusChangeSequenceNumber,
    pKeyChangeMethod, pUserRole, pUserRoleExpiryInterval);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(userNameDbHandle); 
  TMWTARG_UNUSED_PARAM(pStatusChangeSequenceNumber);  
  TMWTARG_UNUSED_PARAM(pKeyChangeMethod);  
  TMWTARG_UNUSED_PARAM(pUserRole);  
  TMWTARG_UNUSED_PARAM(pUserRoleExpiryInterval);  
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_authGetCertData */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_authGetCertData(
  void            *pHandle, 
  void            *userNameDbHandle,
  TMWTYPES_UCHAR  *pCertData,
  TMWTYPES_USHORT *pCertDataLength)
{
#if TMWCNFG_USE_SIMULATED_DB 
  return mdnpsim_authGetCertData(pHandle, userNameDbHandle, pCertData, pCertDataLength);
#elif TMWCNFG_USE_MANAGED_SCL 
  return(TMWCryptoWrapper_getCertData(_toCryptoHandle(pHandle), userNameDbHandle, 
    pCertData, pCertDataLength));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(userNameDbHandle); 
  TMWTARG_UNUSED_PARAM(pCertData); 
  TMWTARG_UNUSED_PARAM(pCertDataLength);  
  return(TMWDEFS_FALSE);
#endif
}

TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_authGetUpdateKeyData(
  void            *pHandle,
  void            *userNameDbHandle,
  TMWTYPES_UCHAR  *pOSData,
  TMWTYPES_USHORT *pOSDataLength)
{
#if TMWCNFG_USE_SIMULATED_DB 
  return mdnpsim_authGetUpdateKeyData(pHandle, userNameDbHandle, pOSData, pOSDataLength);
#elif TMWCNFG_USE_MANAGED_SCL 
  return(TMWCryptoWrapper_getUpdateKeyData(_toCryptoHandle(pHandle), userNameDbHandle, 
    pOSData, pOSDataLength));
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(userNameDbHandle); 
  TMWTARG_UNUSED_PARAM(pOSData); 
  TMWTARG_UNUSED_PARAM(pOSDataLength);  
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_authStoreUpdKeyChangeReply */
void TMWDEFS_GLOBAL mdnpdata_authStoreUpdKeyChangeReply(
  void            *pHandle, 
  void            *userNameDbHandle,
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_UCHAR  *pOSData,
  TMWTYPES_USHORT  OSDataLength)
{
#if TMWCNFG_USE_SIMULATED_DB 
  TMWTARG_UNUSED_PARAM(pOSData); 
  TMWTARG_UNUSED_PARAM(OSDataLength);
  mdnpsim_authStoreUpdKeyChangeReply(pHandle, userNameDbHandle, userNumber);
#elif TMWCNFG_USE_MANAGED_SCL
  TMWCryptoWrapper_storeUpdKeyChangeReply(_toCryptoHandle(pHandle), userNameDbHandle, userNumber, pOSData, OSDataLength);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(userNameDbHandle); 
  TMWTARG_UNUSED_PARAM(userNumber); 
  TMWTARG_UNUSED_PARAM(pOSData);
  TMWTARG_UNUSED_PARAM(OSDataLength);
#endif
} 

TMWTYPES_USHORT TMWDEFS_GLOBAL mdnpdata_authDeleteUser(
  void            *pHandle, 
  void            *userNameDbHandle)
{
#if TMWCNFG_USE_SIMULATED_DB 
  return mdnpsim_authDeleteUser(pHandle, userNameDbHandle); 
#elif TMWCNFG_USE_MANAGED_SCL
  return TMWCryptoWrapper_authDeleteUser(_toCryptoHandle(pHandle), userNameDbHandle);
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(userNameDbHandle); 
  return 0;
#endif
} 
#endif /* DNPCNFG_SUPPORT_AUTHKEYUPDATE */ 

/* function: mdnpdata_authStoreSecStat */
void TMWDEFS_GLOBAL mdnpdata_authStoreSecStat(
  void *pHandle,
  TMWTYPES_USHORT index,
  TMWTYPES_ULONG value)
{
#if TMWCNFG_USE_SIMULATED_DB  
  mdnpsim_storeAuthSecStat(pHandle, TMWDEFS_FALSE, 0, index, value, 0, TMWDEFS_NULL);
#elif TMWCNFG_USE_MANAGED_SCL
  MDNPDatabaseWrapper_StoreAuthSecStat(pHandle, TMWDEFS_FALSE, 0, index, value, 0, TMWDEFS_FALSE, TMWDEFS_NULL);         
#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(index);
  TMWTARG_UNUSED_PARAM(value);  
#endif
}

/* function: mdnpdata_authStoreOSSecStat */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_authStoreOSSecStat(
  void *pHandle,
  TMWTYPES_USHORT assocId,
  TMWTYPES_USHORT pointIndex,
  TMWTYPES_ULONG value,
  TMWTYPES_UCHAR flags,
  TMWTYPES_BOOL isEvent,
  TMWDTIME *pTimeStamp)
{
#if TMWCNFG_USE_SIMULATED_DB
  TMWTARG_UNUSED_PARAM(isEvent);
  TMWTARG_UNUSED_PARAM(pTimeStamp);
  return(mdnpsim_storeAuthSecStat(pHandle, TMWDEFS_TRUE, assocId, pointIndex, value, flags, pTimeStamp));

#elif TMWCNFG_USE_MANAGED_SCL
  return(MDNPDatabaseWrapper_StoreAuthSecStat(pHandle, TMWDEFS_TRUE, assocId, pointIndex, value, flags, isEvent, pTimeStamp));      

#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(assocId);
  TMWTARG_UNUSED_PARAM(pointIndex);
  TMWTARG_UNUSED_PARAM(value);
  TMWTARG_UNUSED_PARAM(flags);
  TMWTARG_UNUSED_PARAM(isEvent);
  TMWTARG_UNUSED_PARAM(pTimeStamp);
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_authLogMaxRekeyTCPClose  */ 
void TMWDEFS_GLOBAL mdnpdata_authLogMaxRekeyTCPClose(
  void            *pHandle)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
}

/* function: mdnpdata_authLogUnexpectedMsg  */ 
void TMWDEFS_GLOBAL mdnpdata_authLogUnexpectedMsg(
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

/* function: mdnpdata_authLogFailedUpdateKey  */ 
void TMWDEFS_GLOBAL mdnpdata_authLogFailedUpdateKey(
  void            *pHandle, 
  TMWTYPES_UCHAR   state, 
  TMWTYPES_ULONG   event)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(state); 
  TMWTARG_UNUSED_PARAM(event); 
}
#endif

/* function: mdnpdata_authLogTx */ 
void TMWDEFS_GLOBAL mdnpdata_authLogTx(
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

/* function: mdnpdata_authLogRx */ 
void TMWDEFS_GLOBAL mdnpdata_authLogRx(
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

/* function: mdnpdata_authLogErrorTx */ 
void TMWDEFS_GLOBAL mdnpdata_authLogErrorTx(
  void           *pHandle, 
  TMWTYPES_USHORT userNumber,
  TMWTYPES_USHORT assocId,
  TMWTYPES_ULONG  sequenceNumber,
  TMWTYPES_UCHAR  errorCode,
  TMWDTIME       *pTimeStamp,
  TMWTYPES_CHAR  *pMsgBuf,
  TMWTYPES_USHORT msgLength,
  TMWTYPES_BOOL   msgSent)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(userNumber);
  TMWTARG_UNUSED_PARAM(assocId);
  TMWTARG_UNUSED_PARAM(sequenceNumber);
  TMWTARG_UNUSED_PARAM(errorCode); 
  TMWTARG_UNUSED_PARAM(pTimeStamp); 
  TMWTARG_UNUSED_PARAM(pMsgBuf); 
  TMWTARG_UNUSED_PARAM(msgLength); 
  TMWTARG_UNUSED_PARAM(msgSent); 
} 
    
/* function: mdnpdata_authLogErrorRx  */ 
void TMWDEFS_GLOBAL mdnpdata_authLogErrorRx(
  void            *pHandle, 
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_USHORT  assocId,
  TMWTYPES_ULONG   sequenceNumber,
  TMWTYPES_UCHAR   errorCode,
  TMWDTIME        *pTimeStamp,
  TMWTYPES_UCHAR  *pMsgBuf,
  TMWTYPES_USHORT  msgLength)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(userNumber); 
  TMWTARG_UNUSED_PARAM(assocId);
  TMWTARG_UNUSED_PARAM(sequenceNumber);
  TMWTARG_UNUSED_PARAM(errorCode);
  TMWTARG_UNUSED_PARAM(pTimeStamp);
  TMWTARG_UNUSED_PARAM(pMsgBuf);
  TMWTARG_UNUSED_PARAM(msgLength);
}

/* function: mdnpdata_authEvent  */ 
void TMWDEFS_GLOBAL mdnpdata_authEvent(
  void                  *pHandle, 
  TMWTYPES_USHORT       userNumber,
  MDNPAUTH_EVENT_ENUM   event)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(userNumber); 
  TMWTARG_UNUSED_PARAM(event);
}


#if MDNPCNFG_SUPPORT_SA_VERSION2
/* function: mdnpdata_authLogKeyStatRqTx */ 
void TMWDEFS_GLOBAL mdnpdata_authLogKeyStatRqTx(
  void            *pHandle, 
  TMWTYPES_USHORT  userNumber)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(userNumber);
}

/* function: mdnpdata_authLogKeyChangeTx */ 
void TMWDEFS_GLOBAL mdnpdata_authLogKeyChangeTx(
  void            *pHandle, 
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_ULONG   sequenceNumber)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(userNumber);
  TMWTARG_UNUSED_PARAM(sequenceNumber);
}

/* function: mdnpdata_authLogAggrTx */ 
void TMWDEFS_GLOBAL mdnpdata_authLogAggrTx(
  void            *pHandle, 
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_ULONG   sequenceNumber)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(userNumber);
  TMWTARG_UNUSED_PARAM(sequenceNumber);
}
 
/* function: mdnpdata_authLogChallRplyTx */ 
void TMWDEFS_GLOBAL mdnpdata_authLogChallRplyTx(
  void            *pHandle, 
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_ULONG   sequenceNumber)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(userNumber);
  TMWTARG_UNUSED_PARAM(sequenceNumber);
}

/* function: mdnpdata_authLogKeyStatusRx  */ 
void TMWDEFS_GLOBAL mdnpdata_authLogKeyStatusRx(
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

/* function: mdnpdata_authLogChallRx  */ 
void TMWDEFS_GLOBAL mdnpdata_authLogChallRx(
  void            *pHandle, 
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_ULONG   sequenceNumber,
  TMWTYPES_UCHAR   macAlgorithm,
  TMWTYPES_UCHAR   reasonForChallenge)
{
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(userNumber); 
  TMWTARG_UNUSED_PARAM(sequenceNumber);
  TMWTARG_UNUSED_PARAM(macAlgorithm); 
  TMWTARG_UNUSED_PARAM(reasonForChallenge);
}

/* function: mdnpdata_authLogChallRplyRx  */ 
void TMWDEFS_GLOBAL mdnpdata_authLogChallRplyRx(
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

/* function: mdnpdata_authLogAggr  */ 
 void TMWDEFS_GLOBAL mdnpdata_authLogAggrRx(
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
    
/* function: mdnpdata_authStoreErrorRx  */ 
void TMWDEFS_GLOBAL mdnpdata_authStoreErrorRx(
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

TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_authGetNewSessionKeys(
  void             *pHandle, 
  DNPDATA_AUTH_KEY *pControlSessionKey,
  DNPDATA_AUTH_KEY *pMonitorSessionKey)
{
#if TMWCNFG_USE_SIMULATED_DB && !TMWCNFG_SUPPORT_CRYPTO
  /* Use simulated only if crypto is not available */
  return(mdnpsim_authGetNewSessionKeys(pHandle, pControlSessionKey, pMonitorSessionKey));
  
#elif TMWCNFG_USE_MANAGED_SCL
  /* Prefer managed over crypto, because managed uses windows crypto library */
  return MDNPDatabaseWrapper_AuthGetNewSessionKeys(pHandle, pControlSessionKey, pMonitorSessionKey);

#elif TMWCNFG_SUPPORT_CRYPTO  
  /*  If TMWCRYPTO interface is supported (it is required for SAv5) use it for SAv2. */
  MDNPSIM_DATABASE *pDb = (MDNPSIM_DATABASE *)pHandle;
  void *pCryptoHandle = pDb->pMDNPSession->dnp.pCryptoHandle;
  TMWCRYPTO_KEY cryptoKey;
  if(tmwcrypto_generateNewKey(pCryptoHandle, TMWCRYPTO_USER_CONTROL_SESSION_KEY, 16, &cryptoKey))
  {
    memcpy(pControlSessionKey->value, cryptoKey.value, 16);
    pControlSessionKey->length = 16; 
  
    if(tmwcrypto_generateNewKey(pCryptoHandle, TMWCRYPTO_USER_MONITOR_SESSION_KEY, 16, &cryptoKey))
    {
      memcpy(pMonitorSessionKey->value, cryptoKey.value, 16);
      pMonitorSessionKey->length = 16;
      return TMWDEFS_TRUE;
    }
  }

  return TMWDEFS_FALSE;

#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(pControlSessionKey); 
  TMWTARG_UNUSED_PARAM(pMonitorSessionKey); 
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_authKeyWrapSupport */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_authKeyWrapSupport(
  TMWTYPES_UCHAR keyWrapAlgorithm)
{
#if TMWCNFG_USE_SIMULATED_DB && !TMWCNFG_SUPPORT_CRYPTO
  /* Use simulated only if crypto is not available */
  return mdnpsim_authKeyWrapSupport(keyWrapAlgorithm);
  
#elif TMWCNFG_USE_MANAGED_SCL
  /* Prefer managed over crypto, because managed uses windows crypto library */
  return MDNPDatabaseWrapper_AuthKeyWrapSupport(keyWrapAlgorithm);

#elif TMWCNFG_SUPPORT_CRYPTO  
  /*  If TMWCRYPTO interface is supported (it is required for SAv5) use it for SAv2. */
  return tmwcrypto_algorithmSupport(TMWDEFS_NULL, dnpauth_keyWraptoTMWCryptoAlgo(keyWrapAlgorithm));

#else
  /* Put target code here */
  TMWTARG_UNUSED_PARAM(keyWrapAlgorithm); 
  return(TMWDEFS_FALSE);
#endif
} 

typedef struct {
  TMWTYPES_UCHAR bytes[8];
} MDNPDATA_R_TYPE;

/* RFC 3394 says A6A6A6A6A6A6A6A6 is the default initial value (also called initialization vector) */
static TMWTYPES_UCHAR m_IV[8] = {0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6};   

/* function: mdnpdata_authEncryptKeyWrapData */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_authEncryptKeyWrapData(
  void            *pHandle, 
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_UCHAR   keyWrapAlgorithm,
  TMWTYPES_UCHAR  *pPlainData, 
  TMWTYPES_USHORT  plainDataLength, 
  TMWTYPES_UCHAR  *pEncryptedData,
  TMWTYPES_USHORT *pEncryptedLength)
{ 
#if TMWCNFG_USE_SIMULATED_DB && !TMWCNFG_SUPPORT_CRYPTO
  /* Use simulated only if crypto is not available */
  return mdnpsim_authEncryptKeyWrapData(pHandle, userNumber, keyWrapAlgorithm, 
    pPlainData, plainDataLength, pEncryptedData, pEncryptedLength);

#elif TMWCNFG_USE_MANAGED_SCL
  /* Prefer managed over crypto, because managed uses windows crypto library */
  if(keyWrapAlgorithm == DNPAUTH_KEYWRAP_AES128)
  {
    /* This code implements the AES Key Wrap Algorithm specified in RFC3394,
     * It calls a function that provides the AES encryption algorithm specified 
     * in FIPS 197 multiple times based on the length of the plain data.
     */ 
    int n;
    int j;
    MDNPDATA_R_TYPE R[20]; 
    TMWTYPES_UCHAR input[16];
    TMWTYPES_UCHAR A[16];
    TMWTYPES_UCHAR B[16];
    
    /* plainText must be multiple of 8bytes (64bits)*/
    n = plainDataLength/8;
    if(plainDataLength%8 != 0)
    { 
      n++; 
    }

    /* make sure there is enough room in the array */
    if(n > 19)
      return TMWDEFS_FALSE;
   
    /* Section 2.2.1 alternative description of the key wrap algorithm from RFC3394 
     * Set A0 = IV, an initial value (see 2.2.3)
     */
    memcpy(A, m_IV, 8);
    
    /* For i=1 to n, R[i]=Pi] */
    memcpy(&R[1], pPlainData, plainDataLength);
    
    for(j=0; j<=5; j++)
    {  
      int i;
      for(i=1; i<=n; i++)
      {           
        TMWTYPES_USHORT bLength = 16;

        /* B = AES(K, A | R[i])     */  
        memcpy(input, A, 8);
        memcpy(&input[8], &R[i], 8); 

        /* This calls the AES Encryption function specified in FIPS 197 */
        MDNPDatabaseWrapper_AuthAESEncrypt(pHandle, userNumber, input, 16, B, &bLength);
        /* The returned encrypted data would also be 16 bytes long */

        /* A = MSB(64, B) ^ t where t = (n*j)+i */
        memcpy(A, B, 8);
        A[7] ^= (n*j) +i;

        /* R[i] = LSB(64, B) */ 
        memcpy(&R[i], &B[8], 8);
      }
    }

    /* Set C[0] = A    */
    /* For i = 1 to n  */
    /*    C[i] = R[i]  */
    memcpy(pEncryptedData, A, 16);
    for(int i=1; i<=n; i++) 
      memcpy(&pEncryptedData[i*8], &R[i], 8);
   
    *pEncryptedLength = (TMWTYPES_USHORT)(n*8)+8;
    return TMWDEFS_TRUE;
  }
  return TMWDEFS_FALSE;

#elif TMWCNFG_SUPPORT_CRYPTO 
  /*  If TMWCRYPTO interface is supported (it is required for SAv5) use it for SAv2. */
  MDNPSIM_DATABASE *pDb = (MDNPSIM_DATABASE *)pHandle;
  void *pCryptoHandle = pDb->pMDNPSession->dnp.pCryptoHandle;
  TMWCRYPTO_KEY updateKey;  
  TMWTYPES_BOOL status = tmwcrypto_getKey(pCryptoHandle, TMWCRYPTO_USER_UPDATE_KEY, (void*) userNumber, &updateKey);
  if(status)
  {
    return tmwcrypto_encryptData(pCryptoHandle, dnpauth_keyWraptoTMWCryptoAlgo(keyWrapAlgorithm),
       &updateKey, pPlainData, plainDataLength, pEncryptedData, pEncryptedLength);
  }
  else
  {
    return TMWDEFS_FALSE;
  }

#else
  /* Put target code here */
  /* You may want to start with the C code above, that implements the AES Key Wrap algorithm and 
   * replace the call to MDNPDatabaseWrapper_AuthAESEncrypt(...) with your own encrypt function. 
   * Or you can choose to implement your own key wrap function as described in RFC3394 
   */
  TMWTARG_UNUSED_PARAM(pHandle); 
  TMWTARG_UNUSED_PARAM(userNumber); 
  TMWTARG_UNUSED_PARAM(keyWrapAlgorithm); 
  TMWTARG_UNUSED_PARAM(pPlainData); 
  TMWTARG_UNUSED_PARAM(plainDataLength); 
  TMWTARG_UNUSED_PARAM(pEncryptedData); 
  TMWTARG_UNUSED_PARAM(pEncryptedLength); 
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpdata_authHMACSupport */
TMWTYPES_CHAR mdnpdata_authHMACSupport(
  TMWTYPES_UCHAR HMACAlgorithm)
{
#if TMWCNFG_USE_SIMULATED_DB && !TMWCNFG_SUPPORT_CRYPTO
  /* Use simulated only if crypto is not available */
  return mdnpsim_authHMACSupport(HMACAlgorithm);
  
#elif TMWCNFG_USE_MANAGED_SCL
  /* Prefer managed over crypto, because managed uses windows crypto library */
  return MDNPDatabaseWrapper_AuthHMACSupport(HMACAlgorithm);

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

/* function: mdnpdata_authHMACValue */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_authHMACValue(
  TMWTYPES_UCHAR    algorithm,
  DNPDATA_AUTH_KEY *pKey,
  TMWTYPES_UCHAR   *pData,
  TMWTYPES_ULONG    dataLength,
  TMWTYPES_UCHAR   *pHMACValue,
  TMWTYPES_USHORT  *pHMACValueLength)
{ 
#if TMWCNFG_USE_SIMULATED_DB && !TMWCNFG_SUPPORT_CRYPTO
  /* Use simulated only if crypto is not available */
  return mdnpsim_authHMACValue(algorithm, pKey, pData, dataLength,
                                pHMACValue, pHMACValueLength);
           
#elif TMWCNFG_USE_MANAGED_SCL
  /* Prefer managed over crypto, because managed uses windows crypto library */
  return MDNPDatabaseWrapper_AuthHMACValue(algorithm, pKey, pData, dataLength,
                                           pHMACValue, pHMACValueLength);

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
 
/* function: mdnpdata_authRandomChallengeData */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_authRandomChallengeData(
  TMWTYPES_UCHAR  *pBuf,
  TMWTYPES_USHORT  minLength,
  TMWTYPES_USHORT *pLength)
{
#if TMWCNFG_USE_SIMULATED_DB && !TMWCNFG_SUPPORT_CRYPTO
  /* Use simulated only if crypto is not available */
  return mdnpsim_authRandomChallengeData(pBuf, minLength, pLength);
  
#elif TMWCNFG_USE_MANAGED_SCL
  /* Prefer managed over crypto, because managed uses windows crypto library */
  return MDNPDatabaseWrapper_AuthRandomChallengeData(pBuf, minLength, pLength);

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

