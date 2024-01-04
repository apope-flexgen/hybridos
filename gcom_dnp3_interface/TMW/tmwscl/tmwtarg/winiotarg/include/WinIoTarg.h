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

/** \file WinIoTarg.h
* Provides A windows Interface to the TMW target I/O functions. 
* These functions provide a generic interface to the various low level target interfaces.
* Currently this includes the Modbus Plus, RS232 and the TCP/IP interface.
*/

#ifndef WinIoTarg_DEFINED
#define WinIoTarg_DEFINED

#include <time.h>
#include "WinIoTarg/include/WinIoTargDefs.h"
#include "tmwscl/utils/tmwtarg.h"

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the WINIOTARG_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// WINIOTARG_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef WINIOTARG_EXPORTS
#define WINIOTARG_API __declspec(dllexport)
#else
#pragma comment(lib, "WinIoTarg.lib")
#define WINIOTARG_API __declspec(dllimport)
#ifdef MANAGEDSCL_BUILD
#undef WINIOTARG_API 
#define WINIOTARG_API 
#endif
#endif

extern WinIoTargProtoAnaLogFunType WinIoTargProtoAnaLogFun;

#ifdef __cplusplus
extern "C" {
#endif

  /* WinIoTarg_initConfig */
  /**
  *   Initialize the WINIO_CONFIG structure to a set of reasonable
  *   default values.
  * 
  *  \dontinclude WinIoTarg.cpp
  *   The default values are implemented as follows:
  *  \skip //WinIoTarg_initConfig_start_mark
  *  \until //WinIoTarg_initConfig_end_mark
  * \n
  *   @param  pConfig - A pointer to the Windows I/O target configuration structure
  * 
  *   @return void 
  * 
  *   @remarks WINIO_CONFIG should be completely initialized prior to calling WinIoTarg_Create().
  *  
  */
  WINIOTARG_API void tmwtargio_initConfig(WINIO_CONFIG *pConfig);

  /* WinIoTarg_Create */
  /**
  *  Initialize a communications channel. This function creates 
  *  a communications channel as specified in the pConfig argument. The
  *  channel does not need to be opened as this will be accomplised in
  *  the tmwtarg_openChannel function described below. This function 
  *  returns a user defined context which is passed to all successive 
  *  calls for this channel. The contents of the context are not used 
  *  by the TMW SCL and are defined as required by the target interface.
  * 
  *   @param  pUserConfig - Pointer to configuration data passed to the TMW
  physical layer code. This data is not used by the TMW code
  and should be used by the target functions to identify and
  configure the communications channel.
  *   @param  pTmwTargConfig - TMW target configuration data structure.
  * 
  *   @return channel context as described above.
  * 
  *   @remarks none
  */
  WINIOTARG_API void *WinIoTarg_Create(
    const void *pUserConfig, 
    TMWTARG_CONFIG *pTmwTargConfig);

   /* WinIoTarg_applyBinFileTargValues */
   WINIOTARG_API TMWTYPES_BOOL WinIoTarg_applyBinFileTargValues(WINIO_CONFIG *pIoConfig, 
     TMWTARG_BINFILE_VALS *pBinTargFileValues,
     TMWTYPES_BOOL *pIsChannelSerial);

   /* WinIoTarg_initBinFileTargValues */
   WINIOTARG_API TMWTYPES_BOOL WinIoTarg_initBinFileTargValues(TMWTARG_BINFILE_VALS *pBinFileTargValues);

  /* WinIoTarg_Modify */
  /**
  *    Modify a communications channel. This function modifies 
  *    a communications channel as specified in the pConfig argument.
  * 
  *   @param pContext - Context returned from call to WinIoTarg_Create().
  *   @param pUserConfig - Pointer to configuration data passed to the TMW
  *    physical layer code. This data is not used by the TMW code
  *    and should be used by the target functions to identify and
  *    configure the communications channel.
  * 
  *   @return  TMWDEFS_TRUE if success.
  * 
  *   @remarks This may not modifiy all parameters at runtime, a re-start of the channel may be required.
  */
  WINIOTARG_API TMWTYPES_BOOL WinIoTarg_Modify(
    void *pContext,
    const void *pUserConfig);

  /* WinIoTarg_Destroy */
  /**
  *   Delete a communications channel. This function should
  *  delete a communications channel and free all associated memory
  *  and resources.
  * 
  *   @param pContext Context returned from call to WinIoTarg_Create()
  * 
  *   @remarks WinIoTarg_closeChannel() will be called
  */
  WINIOTARG_API void WinIoTarg_Destroy(void *pContext);

  /* WinIoTarg_Exit */
  /**
  *   Application is exiting, free up any resources if desired.
  *  
  */
  WINIOTARG_API void WinIoTarg_Exit();

  /* WinIoTarg_isChannelOpen */
  /**
  *   Return the current Open/Close state of the channel
  * 
  *   @param pContext Context returned from call to WinIoTarg_Create()
  * 
  *   @return  true if channel is open, else false
  * 
  *   @remarks none
  */
  WINIOTARG_API TMWTYPES_BOOL WinIoTarg_isChannelOpen(void *pContext);

  /* WinIoTarg_openChannel */
  /**
  *   Open a communications channel.
  * 
  *   @param pContext Context returned from call to WinIoTarg_Create()
  *   @param pReceiveCallbackFunc function to call when data is available
  *   @param pCheckAddrCallbackFunc callback to test address of message
  *   @param pCallbackParam parameter passed to callback function for data
  * 
  *   @return  true if successful, else false
  * 
  *   @remarks none
  */
  WINIOTARG_API TMWTYPES_BOOL WinIoTarg_openChannel(void *pContext,
    TMWTARG_CHANNEL_RECEIVE_CBK_FUNC pReceiveCallbackFunc,
    TMWTARG_CHECK_ADDRESS_FUNC pCheckAddrCallbackFunc,
    void *pCallbackParam);

  /* WinIoTarg_closeChannel */
  /**
  *   Close a communications channel
  * 
  *   @param pContext Context returned from call to WinIoTarg_Create()
  * 
  *   @remarks none
  */
  WINIOTARG_API void WinIoTarg_closeChannel(void *pContext);

  /* WinIoTarg_resetChannel */
  /**
  *   Reset a communications channel (this indirectly causes a target close and open)
  * 
  *   @param pContext Context returned from call to WinIoTarg_Create()
  * 
  *   @return  true if supported
  * 
  *   @remarks none
  */
  WINIOTARG_API TMWTYPES_BOOL WinIoTarg_resetChannel(void *pContext);

  /* WinIoTarg_getChannelName */
  /**
  *   Returns the name for this channel.  This method allows the target to return an appropriate
  *  name for this channel. 
  * 
  *   @param pContext Context returned from call to WinIoTarg_Create()
  * 
  *   @return  pointer to a null terminated string which contains the channel name
  * 
  *   @remarks Typically this would be something out of the
  *  configuration information passed to the WinIoTarg_openChannel() function.
  */
  WINIOTARG_API const char *WinIoTarg_getChannelName(void *pContext);

  /* WinIoTarg_getChannelStatus */
  /**
  *   Return status information for this channel
  * description: This method allows the target to return user defined
  *  status string to be displayed when the channel status is needed.
  * 
  *   @param pContext Context returned from call to WinIoTarg_Create()
  * 
  *   @return  Pointer to a null terminated string which contains the status.
  * 
  *   @remarks Typically this would contain formatted information about the 
  *  channel status.
  */
  WINIOTARG_API const char *WinIoTarg_getChannelStatus(void *pContext);

  /* WinIoTarg_getChannelInfo */
  /**
  *   Return configuration information for this channel
  * description: This method allows the target to return a user defined
  *  information string to be displayed when the channel is opened.
  * 
  *   @param pContext Context returned from call to WinIoTarg_Create()
  * 
  *   @return  Pointer to a null terminated string which contains the info.
  * 
  *   @remarks Typically this would contain formatted information about the 
  *  channel configuration and/or status.
  */
  WINIOTARG_API const char *WinIoTarg_getChannelInfo(void *pContext);

  /* WinIoTarg_getTransmitReady */
  /**
  *   Determine whether a channel is ready to transmit or not.
  * 
  *   @param pContext Context returned from call to WinIoTarg_Create()
  * 
  *   @return  number of milliseconds to wait before checking to see if channel
  *  is ready again.
  * 
  *   @remarks This function can be used to delay transmission until various 
  *  target related dependencies have been satisfied. A common
  *  example is modem setup time.
  */
  WINIOTARG_API TMWTYPES_MILLISECONDS WinIoTarg_getTransmitReady(void *pContext);

  /* WinIoTarg_waitForInput
   *   Unsupported function for internal use.
   */
  WINIOTARG_API TMWTYPES_BOOL WinIoTarg_waitForInput(void *pContext);

  /* WinIoTarg_StopThreads 
   *   Stop Check For Input and Connector processing so that channel can be safely deleted.
   */
  WINIOTARG_API void WinIoTarg_StopThreads(void *pContext);

  /* WinIoTarg_receive */
  /**
  *   Receive bytes from the specified channel
  * 
  *   @param pContext Context returned from call to WinIoTarg_Create()
  *   @param pBuff Buffer into which to store received bytes
  *   @param maxBytes The maximum number of bytes to read
  *   @param maxTimeout  maximum time to wait in milliseconds for input
  *   from this channel.
  *   @param pTimeoutOccured true if an intercharacter 
  *   timeout occured while receiving bytes. This is an optional
  *   timeout that can be implemented in the target to terminate
  *   a frame if to long a time passes between receipt of bytes
  *   in a frame.
  * 
  *   @return  The number of bytes actually read.
  * 
  *   @remarks none
  */
  WINIOTARG_API TMWTYPES_USHORT WinIoTarg_receive(
    void *pContext, 
    TMWTYPES_UCHAR *pBuff, 
    TMWTYPES_USHORT maxBytes, 
    TMWTYPES_MILLISECONDS maxTimeout, 
    TMWTYPES_BOOL *pTimeoutOccured);

  /* WinIoTarg_transmit */
  /**
  *   Transmit bytes on the specified channel
  * 
  *   @param pContext Context returned from call to WinIoTarg_Create()
  *   @param pBuff Array of bytes to transmit
  *   @param numBytes Number of bytes to transmit
  * 
  *   @return  true if all the bytes were successfully transmitted,
  *  else false.
  * 
  *   @remarks none
  */
  WINIOTARG_API TMWTYPES_BOOL WinIoTarg_transmit(
    void *pContext, 
    TMWTYPES_UCHAR *pBuff, 
    TMWTYPES_USHORT numBytes);

  /* WinIoTarg_transmitUDP */
  /**
  *   Transmit bytes on the specified channel
  *    NOTE: This function is only used to support DNP3 Specification 
  *     IP Networking.
  * 
  *   @param pContext Context returned from call to WinIoTarg_Create()
  *   @paramUDPPort - This is a define that indicates the remote UDP port to
  *   transmit to. 
  *    TMWTARG_UDP_SEND       - Send to the remote port to be used for 
  *                             requests or responses
  *    TMWTARG_UDP_SEND_UNSOL - Send to the remote port to be used for
  *                             unsolicited responses. Once outstation has
  *                             received a request from master this would be
  *                             same port as all responses.
  *    TMWTARG_UDPONLY_BROADCAST Send to the broadcast address when UDP ONLY
  *                             is configured.
  *   @param pBuff Array of bytes to transmit
  *   @param numBytes Number of bytes to transmit
  *
  *   @return  true if all the bytes were successfully transmitted,
  *  else false.
  *
  *   @remarks none
  */
  WINIOTARG_API TMWTYPES_BOOL WinIoTarg_transmitUDP(
    void *pContext, 
    TMWTYPES_UCHAR UDPPort,
    TMWTYPES_UCHAR *pBuff, 
    TMWTYPES_USHORT numBytes);

  /* WinIoTarg_setRTS */
  /**
  *   set or clear RTS
  * 
  */ 
  WINIOTARG_API TMWTYPES_BOOL WinIoTarg_setRTS(
    void *pContext, 
    TMWTYPES_BOOL rts);

  /* WinIoTarg_setRTS */
  /**
  *   set or clear RTS
  *
  */
  WINIOTARG_API TMWTYPES_BOOL WinIoTarg_setDTR(
    void *pContext, 
    TMWTYPES_BOOL dtr);

  /* WinIoTarg_setSyncTransmit */
  /**
  *   set synchronous transmit mode
  * 
  *   @param flag true to set sync mode 
  * 
  *   @remarks   WinIoTarg_getTransmitReady() will return 1 if 
  *    Sync Transmit is false and a transmit callback is defined.
  */
  WINIOTARG_API void WinIoTarg_setSyncTransmit(
    TMWTYPES_BOOL flag);

  /* WinIoTarg_setTxCallback */
  /**
  *   set a call back function to process data just prior to
  * transmitting
  * 
  *   @param pContext Context returned from call to WinIoTarg_Create()
  *   @param pCallback function to call when data is transmitted.
  *   @param pCallbackParam user callback parameter to pass to pCallback
  * 
  *   @remarks see WinIoTarg_setSyncTransmit()
  */
  WINIOTARG_API void WinIoTarg_setTxCallback(
    void *pContext,
    WINIO_CALLBACK pCallback,
    void *pCallbackParam);

  /* WinIoTarg_setRxCallback */
  /**
  *   set a call back function to process data just after it is
  * received
  * 
  *   @param pContext Context returned from call to WinIoTarg_Create()
  *   @param pCallback function to call when data ie received
  *   @param pCallbackParam user callback parameter to pass to pCallback
  * 
  *   @remarks none
  */
  WINIOTARG_API void WinIoTarg_setRxCallback(
    void *pContext,
    WINIO_CALLBACK pCallback,
    void *pCallbackParam);

  /* WinIoTarg_setStatusCallback */
  /**
  *   set a status callback function
  * 
  *   @param pContext Context returned from call to WinIoTarg_Create()
  *   @param pCallback function to call when status changes
  *   @param pCallbackParam user callback parameter to pass to pCallback
  * 
  *   @remarks none
  */
  WINIOTARG_API void WinIoTarg_setStatusCallback(    
    void *pContext,
    WINIO_STATUS_CALLBACK pCallback,
    void *pCallbackParam);

#if WIN_MODEM_SUPPORT
  /* WinModem_setStatusCallback */
  /**
  *   set a status callback function
  * 
  *   @param pContext Context returned from call to WinIoTarg_Create()
  *   @param pCallback function to call when status changes
  *   @param pCallbackParam user callback parameter to pass to pCallback
  * 
  *   @remarks none
  */
  WINIOTARG_API void WinModem_setStatusCallback(    
    void *pContext,
    WINIO_STATUS_CALLBACK pCallback,
    void *pCallbackParam);

  /* WinModemPool_setStatusCallback */
  /**
  *   set a status callback function for a modem pool
  * 
  *   @param pContext Context returned from call to WinIoTarg_Create()
  *   @param pCallback function to call when status changes
  *   @param pCallbackParam user callback parameter to pass to pCallback
  * 
  *   @remarks none
  */
  WINIOTARG_API void WinModemPool_setStatusCallback(    
    void *pContext,
    WINIO_STATUS_CALLBACK pCallback,
    void *pCallbackParam);

  /* WinModemPoolManager_setStatusCallback */
  /**
  *   set a status callback function for the modem pool manager
  * 
  *   @param pContext Context returned from call to WinIoTarg_Create()
  *   @param pCallback function to call when status changes
  *   @param pCallbackParam user callback parameter to pass to pCallback
  * 
  *   @remarks none
  */
  WINIOTARG_API void WinModemPoolManager_setStatusCallback(    
    void *pContext,
    WINIO_STATUS_CALLBACK pCallback,
    void *pCallbackParam);

  /* WinModemPoolManager_getManager */
  /**
  *   gets the modem pool manager object
  * 
  *   @return  the modem pool manager
  * 
  *   @remarks none
  */
  WINIOTARG_API void *WinModemPoolManager_getManager(void); 

  /* WinModemPoolManager_destroy */
  /**
  *   destroys the modem pool manager object
  * 
  *   @return  void
  * 
  *   @remarks none
  */
  WINIOTARG_API void WinModemPoolManager_destroy(void); 

#endif

  /* WinIoTarg_setEnabled */
  /**
  *   enable/disable a channel.
  * 
  *   @param pContext Context returned from call to WinIoTarg_Create()
  *   @param bStatus true to enable, false to disable channel
  * 
  *   @remarks will cause WinIoTarg_openChannel() to allways fail
  */
  WINIOTARG_API void WinIoTarg_setEnabled(    
    void *pContext,
    TMWTYPES_BOOL bStatus);

  /* WinIoTarg_startThread */
  /**
  *   start a WinIoTarg thread
  * 
  *   @param threadFun the function to run as a thread
  *   @param pParam a parameter to pass to the thread function
  *   @param threadID the ID of the thread returned to the caller
  *   @param nPriority the priority of the thread
  * 
  *   @return  the thread handle
  * 
  *   @remarks See Windows documentation for details on thread priority.  
  *   The thread handle is used to later end the thread.
  */
  WINIOTARG_API void *WinIoTarg_startThread(
    WINIO_THREAD_FUN threadFun, 
    void *pParam, 
    unsigned int *threadID,
    int nPriority);

  /* WinIoTarg_endThread */
  /**
  *   end a WinIoTarg thread
  * 
  *   @param threadHandle the thread handle
  * 
  *   @remarks See Windows documentation for details on threads.
  */
  WINIOTARG_API void WinIoTarg_endThread(
    void *threadHandle);

  /* WinIoTarg_IsProtoAnaLogEnabled */
  /**
  *   is protocol logging from target enabled
  */
  WINIOTARG_API TMWTYPES_BOOL WinIoTarg_IsProtoAnaLogEnabled(void);

  /* WinIoTarg_setProtoAnaLogEnable */
  /**
  *   enable/disable protocol logging from target
  */
  WINIOTARG_API void WinIoTarg_setProtoAnaLogEnable(TMWTYPES_BOOL bEnable);

  /* WinIoTarg_setProtoAnaLogFun */
  /**
  *   set a 'printf' like function to allow protocol analyzer logging from WinIoTarg
  * 
  *   @param pFun the function that will be called when logging
  * 
  *   @remarks none
  */
  WINIOTARG_API void WinIoTarg_setProtoAnaLogFun(WinIoTargProtoAnaLogFunType pFun);

  /* WinIoTarg_getIdleCallBack */
  /**
  *   get the idle callback for the current context
  * 
  *   @param pContext Context returned from call to WinIoTarg_Create()
  *   @param pCallBackFun void fn(void*)
  *   @param pCallBackParam a parameter passed to the callback function
  * 
  *   @remarks Currently used by Modem Pools
  */
  WINIOTARG_API void WinIoTarg_getIdleCallBack(void *pContext, 
    TMWCHNL_IDLE_CALLBACK *pCallBackFun, void **pCallBackParam);

  /* WinIoTarg_getDateTime */
  /**
  *   return the current date and time
  * 
  *   @param pDateTime storage to return date and time into
  * 
  *   @remarks Depends on how WinIoTarg_setTimeMode was called
  */
  WINIOTARG_API void WinIoTarg_getDateTime(TMWDTIME *pDateTime);

  /* WinIoTarg_getUTCDateTime */
  /**
  *   return the current Coordinated Universal Time (UTC) date and time
  * 
  *   @param pDateTime storage to return date and time into
  */
  WINIOTARG_API void WinIoTarg_getUTCDateTime(TMWDTIME *pDateTime);

  /* WinIoTarg_setDateTime */
  /**
  *   Set clock to specified time. Future calls to
  *          WinIoTarg_getDateTime() will return a clock based on this time.
  * 
  *   @param pNewDateTime date time to set
  * 
  *   @return  true if success
  * 
  *   @remarks none
  */
  WINIOTARG_API TMWTYPES_BOOL WinIoTarg_setDateTime(const TMWDTIME *pNewDateTime);
 
  /* WinIoTarg_setUTCDateTime */
  /**
  *   Set clock to specified time. Future calls to
  *          WinIoTarg_getUTCDateTime() will return a clock based on this time.
  * 
  *   @param pNewDateTime date time to set
  * 
  *   @return  true if success
  * 
  *   @remarks none
  */
  WINIOTARG_API TMWTYPES_BOOL WinIoTarg_setUTCDateTime(const TMWDTIME *pNewDateTime);

  /* WinIoTarg_convertLocalTime */
  /**
  *   convert a local time to a TMWDTIME
  * 
  *   @param pDateTime date time to set
  *   @param timeIn the local time
  * 
  *   @remarks none
  */
  WINIOTARG_API void WinIoTarg_convertLocalTime(
    TMWDTIME *pDateTime, 
    time_t timeIn);

  /* WinIoTarg_setTimeMode */
  /**
  *   specify time mode 
  * 
  *   @param timeMode the mode to set
  * 
  *   @remarks currently simulated or system time are supported.
  */
  WINIOTARG_API void WinIoTarg_setTimeMode(
    WINIO_TIME_MODE timeMode);

  /* WinIoTarg_setTimeZone */
  /**
  *   specify time zone and DST usage
  * 
  *   @param timeZone the zone to set
  * 
  *   @param ignoreDST true to never use day light savings time
  * 
  */
  WINIOTARG_API void WinIoTarg_setTimeZone(
    int timeZone, 
    TMWTYPES_BOOL ignoreDST);

  /* WinIoTarg_getCurrentTimeZone */
  /**
  *   get the current time zone configured on this computer
  * 
  *   @return  
  */
  WINIOTARG_API TMWTYPES_BOOL WinIoTarg_getCurrentTimeZone(char *curTimeZone);

  /* WinIoTarg_getTimeZone */
  /**
  *   get the time zone name for the supplied time zone index
  * 
  *   @return  
  */
  WINIOTARG_API TMWTYPES_BOOL WinIoTarg_getTimeZone(int index, char *returnedTimeZone);

  /* WinIoTarg_getTimeZones */
  /**
  *   get the available time zones configured on this computer
  * 
  *   @return  time zone index
  */
  WINIOTARG_API void *WinIoTarg_getTimeZones(void);

  /* WinIoTarg_createTimeZones */
  /**
  *   create time zones
  *
  *   @return  void
  */
  WINIOTARG_API void WinIoTarg_createTimeZones(void);

  /* WinIoTarg_cleanTimeZones */
  /**
  *   destroy time ones
  *
  *   @return  void
  */
  WINIOTARG_API void WinIoTarg_cleanTimeZones(void);

  /* WinIoTarg_getSpecificTimeFromUtcTime */
  /**
  *   get the specific time from a utc time based on time zone index
  */
  WINIOTARG_API void WinIoTarg_getSpecificTimeFromUtcTime(TMWDTIME *utcTime, TMWDTIME *specificTime, int timeZoneIndex, TMWTYPES_BOOL ignoreDST);

  /* WinIoTarg_getUTCTimeZone */
  /**
  *   get the utc time zone index
  * 
  *   @return  time zone index
  */
  WINIOTARG_API int WinIoTarg_getUTCTimeZone(const char *utcTZName);

  /* WinIoTarg_getTimeZoneIndex */
  /**
  *   get the time zone index for a time zone name
  * 
  *   @return  time zone index
  */
  WINIOTARG_API int WinIoTarg_getTimeZoneIndex(const char *tzName);

  /* WinIoTarg_strptime */
  /**
  *   get the time zone index for a time zone name
  * 
  *   @return  time zone index
  */
  WINIOTARG_API char *WinIoTarg_strptime(const char *s, const char *format, struct tm *tm, int *millisecs);

#if TMW_SUPPORT_MONITOR

  /* WinIoTarg_inputData */
  /**
  *   initialize winPcap
  * 
  *   @remarks Only used if monitoring TCP
  */
  WINIOTARG_API TMWTYPES_BOOL WinIoTarg_initTCPMonitor(void);

  /* WinIoTarg_CheckHostName */
  /**
  *   Check to see if host name is valid
  * 
  *   @remarks Only used if monitoring TCP
  */
  WINIOTARG_API TMWTYPES_BOOL WinIoTarg_CheckHostName(char *pHost);


  /* WinIoTarg_GetTCPInterfaces */
  /**
  *   Get all of the TCP interfaces
  * 
  *   @remarks Only used if monitoring TCP
  */
  WINIOTARG_API void WinIoTarg_GetTCPInterfaces(char *pInterface[], int *pCount);

  /* WinIoTarg_inputData */
  /**
  *   provide raw input data to a monitor channel
  * 
  *   @param pContext Context returned from call to WinIoTarg_Create()
  *   @param pBuf pointer to the input data
  *   @param length length of the data provided
  * 
  *   @remarks Only used for a monitor channel
  */
  WINIOTARG_API void WinIoTarg_inputData(
    void *pContext, 
    TMWTYPES_UCHAR *pBuf, 
    TMWTYPES_USHORT length);
#endif

  /* WinIoTarg_GetPrivateProfileInt */
  /**
  *   retrieves an integer associated with a key in the specified 
  *   section of an initialization file.
  * 
  *   @param lpAppName the name of the section in the initialization file.
  *   @param lpKeyName the name of the key whose value is to be retrieved. This value is 
  *    in the form of a string; the function converts the string into an integer and 
  *    returns the integer. 
  *   @param nDefault Default value to return if the key name cannot be found.
  *   @param lpFileName the name of the initialization file. If this parameter does not 
  *    contain a full path to the file, the system searches for the file in the Windows directory.
  * 
  *   @return  The return value is the integer equivalent of the string following the 
  *    specified key name in the specified initialization file. If the key is not found, 
  *    the return value is the specified default value.
  * 
  *   @remarks The function searches the file for a 
  *    key that matches the name specified by the lpKeyName 
  *    parameter under the section name specified by the 
  *    lpAppName parameter. A section in the initialization 
  *    file must have the following form:
  *\n
  *           [section]\n
  *           key=value\n
  *              .\n
  *              .\n
  *              .\n
  *\n
  */
  WINIOTARG_API TMWTYPES_LONG WinIoTarg_GetPrivateProfileInt(
    const char *lpAppName,  // section name
    const char *lpKeyName,  // key name
    int nDefault,       // return value if key name not found
    const char *lpFileName  // initialization file name
    );

  WINIOTARG_API TMWTYPES_BOOL WinIoTarg_SetPrivateProfileInt(
    const char *lpAppName,  // section name
    const char *lpKeyName,  // key name
    int value,          // value to set
    const char *lpFileName  // initialization file name
    );

  /* WinIoTarg_GetPrivateProfileString */
  /**
  *   The GetPrivateProfileString function retrieves a 
  *   string from the specified section in an initialization file.
  * 
  *   @param lpAppName specifies the name of the section containing the key name.
  *   @param lpKeyName the name of the key whose associated string is to be retrieved.
  *   @param lpDefault If the lpKeyName key cannot be found in the initialization file, 
  *    copies the default string to the lpReturnedString buffer.
  *   @param lpReturnedString Pointer to the buffer that receives the retrieved string. 
  *   @param nSize Size of the buffer pointed to by the lpReturnedString parameter, in TCHARs. 
  *   @param lpFileName the name of the initialization file. If this parameter does not 
  *    contain a full path to the file, the system searches for the file in the Windows directory.
  * 
  *   @return  the number of characters copied to the buffer, not including the terminating null character.
  * 
  *   @remarks The GetPrivateProfileString function searches the specified 
  *   initialization file for a key that matches the name specified by the 
  *   lpKeyName parameter under the section heading specified by the 
  *   lpAppName parameter. If it finds the key, the function copies the 
  *   corresponding string to the buffer. If the key does not exist, the 
  *   function copies the default character string specified by the 
  *   lpDefault parameter. A section in the initialization file must 
  *   have the following form:
  *   \n
  *   [section]\n
  *   key=string\n
  *         .\n
  *         .\n
  *         .\n
  *   \n
  */
  WINIOTARG_API TMWTYPES_ULONG WinIoTarg_GetPrivateProfileString(
    const char *lpAppName,        // section name
    const char *lpKeyName,        // key name
    const char *lpDefault,        // default string
    char *lpReturnedString,       // destination buffer
    TMWTYPES_ULONG nSize,                  // size of destination buffer
    const char *lpFileName        // initialization file name
    );

  WINIOTARG_API TMWTYPES_BOOL WinIoTarg_SetPrivateProfileString(
    const char *lpAppName,        // section name
    const char *lpKeyName,        // key name
    const char *value,            // value to set
    const char *lpFileName        // initialization file name
    );

  /*  */
  /* WinIoTarg_CreateSessionTimeObject */
  /**
  *   create date time object function used by .NET SCL
  *  
  *  
  */ 
  WINIOTARG_API void *WinIoTarg_CreateSessionTimeObject(const TMWDTIME *pTime);

  /*  */
  /* WinIoTarg_CreateSessionTimeObject */
  /**
  *   get date time function used by .NET SCL
  *  
  *  
  */ 
  WINIOTARG_API void  WinIoTarg_SetSessionDateTime(void *pHandle, TMWDTIME *pDateTime);

  /*  */
  /* WinIoTarg_GetSessionDateTime */
  /**
  *   set date time function used by .NET SCL
  *  
  *  
  */ 
  WINIOTARG_API void  WinIoTarg_GetSessionDateTime(void *pHandle, TMWDTIME *pDateTime);

  /*  */
  /* WinIoTarg_DeleteSessionTimeObject */
  /**
  *   delete date time function used by .NET SCL
  *  
  *  
  */ 
  WINIOTARG_API void  WinIoTarg_DeleteSessionTimeObject(void *pHandle);


  WINIOTARG_API void WinIoTarg_lockInit(TMWDEFS_RESOURCE_LOCK *pLock);
  WINIOTARG_API void WinIoTarg_lockSection(TMWDEFS_RESOURCE_LOCK *pLock);
  WINIOTARG_API void WinIoTarg_unlockSection(TMWDEFS_RESOURCE_LOCK *pLock);
  WINIOTARG_API void WinIoTarg_lockDelete(TMWDEFS_RESOURCE_LOCK *pLock);

  /* WinIoTarg_formatErrorMsg, 
   *   NOTE: this is only for use within winiotarg, it is not exported from dll. 
   */
  void WinIoTarg_formatErrorMsg(char   *pBuffer, TMWTYPES_ULONG error);

  /* WinIoTarg_DebugPrintf */
  /**
  *   Provides a mechanism to send debug statements to a file or console window based on settings in an INI file.
  *     NOTE: this is only for use within winiotarg, it is not exported from dll. 
  * 
  *   @param format format control (like printf() )
  *   @param  ... Optional arguments for the format.
  * 
  *   @return  Number of characters output.
  * 
  *   @remarks See documentation for printf().\n
  *\n
  *   The following INI file (TMWDebug.ini in the Windows directory) settings are supported:\n
  *     [debug]\n
  *     log=value         ; 1 to enable 0 to disable\n
  *     file=value	      ; 1 to log to file 0 to log to console\n
  *     file_name=string  ; the file name if logging to a file\n
  *\n
  */
  int WinIoTarg_DebugPrintf(char   *format,...);

#ifdef __cplusplus
};
#endif


// not exported functions
TMWTYPES_BOOL isWinSockInitialized(void);

#endif // WinIoTarg_DEFINED
