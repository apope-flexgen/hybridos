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

/* file: sdnpo070.h
 * description: This file is intended for internal SCL use only.
 *   DNP3 Slave support for Object 70 File Transfer Objects
 */
#ifndef SDNPO070_DEFINED
#define SDNPO070_DEFINED

#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/dnputil.h"

#if SDNPDATA_SUPPORT_OBJ70
 
/* Indicates what response to send */
typedef enum {
  SDNPO070_FILE_RESP_IDLE,
  SDNPO070_FILE_RESP_CMD,            /* open, close, delete or abort complete */
  SDNPO070_FILE_RESP_READ,           /* Reading a simple file */
  SDNPO070_FILE_RESP_READ_DIR,       /* Reading a directory */
  SDNPO070_FILE_RESP_GETINFO,        /* Get Info response */
  SDNPO070_FILE_RESP_TRANSPORT_STAT  /* Send Transport Status Object Obj70 V6 */
} SDNPO070_FILE_RESP;

/* Indicates what command to retry */
typedef enum {
  SDNPO070_FILE_RETRY_IDLE,    
  SDNPO070_FILE_RETRY_OPEN,     
  SDNPO070_FILE_RETRY_READ,     
  SDNPO070_FILE_RETRY_READ_DIR,   
  SDNPO070_FILE_RETRY_WRITE,    
  SDNPO070_FILE_RETRY_CLOSE,    
  SDNPO070_FILE_RETRY_GETINFO,   
  SDNPO070_FILE_RETRY_DELETE
} SDNPO070_FILE_RETRY;

/* Indicates state of event processing */
typedef enum {
  SDNPO070_EVENT_STAT_NOTREADY,
  SDNPO070_EVENT_STAT_READY,
  SDNPO070_EVENT_STAT_SENT
} SDNPO070_EVENT_STAT;

/* Structure used to store file descriptor information */
typedef struct Object70FileDescrStruct { 

  DNPDEFS_FILE_TYPE        type;
  TMWDTIME                 fileCreationTime;
  TMWTYPES_ULONG           size;
  DNPDEFS_FILE_PERMISSIONS permissions;
  TMWTYPES_CHAR           *pName;
  TMWTYPES_USHORT          nameLen;
} SDNPO070_FILE_DESCRIPTOR;


/* Structure used to store file events */
typedef struct Object70FileCtrlStruct { 

  SDNPO070_FILE_RESP      respState;
  SDNPO070_EVENT_STAT     eventState;
  SDNPO070_FILE_RETRY     retryState;

  DNPDEFS_FILE_CMD_STAT   cmdStatus;
  DNPDEFS_FILE_TFER_STAT  tferStatus;
  TMWTYPES_BOOL           readResponseWaitConfirm; 
  TMWTYPES_BOOL           errorClose;

  /* When this event is deleted we can deallocate the file control block */
  TMWTYPES_BOOL           xferComplete;

  /* General information */
  
  /* handle of open file */
  TMWTYPES_ULONG          handle;

  /* handle to use in cmd response, could be incorrect handle */
  TMWTYPES_ULONG          rspHandle;

  TMWTYPES_USHORT         requestId;
  TMWTYPES_USHORT         maxBlockSize;
  TMWTYPES_ULONG          authKey;
  TMWTYPES_ULONG          size;
  TMWTYPES_ULONG          blockNumber;
  TMWTYPES_ULONG          lastBlockWritten;
  TMWTYPES_ULONG          lastBlockRead;

  DNPDEFS_FILE_TYPE       type;
  TMWTYPES_BOOL           last;

  TMWTYPES_USHORT         filenameLength;
  TMWTYPES_CHAR           filename[DNPCNFG_MAX_FILENAME+1];
  
  TMWTIMER                fileTransferTimer;

  /* Event portion */
  TMWTIMER                retryTimer;
  TMWDEFS_CLASS_MASK      classMask;

#if SDNPDATA_SUPPORT_XML2
  TMWTYPES_BOOL           xml2DeviceProfileOpen; 
#endif

  union {

    /* OPEN FILE */
    struct openStruct{
      TMWTYPES_USHORT          mode;
      DNPDEFS_FILE_PERMISSIONS permissions;
    } open;
 
    /* READ FILE */
    struct readStruct{
      TMWTYPES_USHORT          bytesRead;
      TMWTYPES_UCHAR           buffer[SDNPCNFG_MAX_BLOCK_SIZE]; 
    } read;
  
    /* GET INFO */
    struct getInfo{
      SDNPO070_FILE_DESCRIPTOR fileDesc;
    } getFileInfo;

    /* READ DIRECTORY */
    struct readDirStruct{
      TMWTYPES_UCHAR           fileDescIndex;
      TMWTYPES_USHORT          bytesLeft;
      TMWTYPES_ULONG           size;
      SDNPO070_FILE_DESCRIPTOR fileDesc[SDNPCNFG_OBJ70_DIR_SIZE];

      /* Buffer to hold all of the names */
      TMWTYPES_CHAR            fileNameBuffer[SDNPCNFG_MAX_BLOCK_SIZE]; 
    } readDir;

    /* WRITE FILE */
    struct writeStruct{
      TMWTYPES_USHORT          numBytes;
      TMWTYPES_UCHAR           buffer[SDNPCNFG_MAX_BLOCK_SIZE]; 
    } write;

  } data;
} SDNPO070_FILE_CONTROL_BLOCK; 

#ifdef __cplusplus
extern "C" {
#endif

/* function: sdnpo070_init
   * purpose: Initialize file transfer events
   * arguments:
   *  pSession - pointer to session
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo070_init(
    TMWSESN *pSession);

  /* function: sdnpo070_close
   * purpose: Close file transfer event processing
   * arguments:
   *  pSession - pointer to session
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo070_close(
    TMWSESN *pSession);

  /* function: sdnpo070_countEvents
   * purpose: Count the number file transfer events in
   *  queue
   * arguments:
   *  pSession - pointer to session
   *  classMask - count events in this class or classes
   * returns:
   *  void
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpo070_countEvents(
    TMWSESN *pSession, 
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_BOOL countAll);

  /* function: sdnpo070_cleanupEvents
   * purpose: Either remove events that have been sent or clear the
   *  sent flags if message was not acknowledged.
   * arguments:
   *  pSession - pointer to session
   *  deleteEvents - remove events if true, else clear sent flag
   * returns:
   *  void
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo070_cleanupEvents(
    TMWSESN *pSession, 
    TMWTYPES_BOOL deleteEvents); 

  /* function: sdnpo070_readObj70v0ByClass  
   * purpose: 
   * arguments:
   *  pSession - session we are responding to
   *  pResponse - pointer to response message info
   *  pObjHeader - object header we are processing
   * returns:
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo070_readObj70v0ByClass(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpo070_processRequest
   * purpose: process file transfer requests
   * arguments:
   *  pSession - session we are responding to
   *  pResponse - pointer to response message info
   *  pObjHeader - object header we are processing
   * returns:
   *  TMWDEFS_TRUE if succesful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo070_processRequest(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRxFragment,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader);

  /* function: sdnpo070_readObj70
   * purpose: read file data
   * arguments:
   *  pSession - session we are responding to
   *  pResponse - pointer to response message info
   *  pObjHeader - object header we are processing
   * returns:
   *  SDNPSESN_READ_COMPLETE or SDNPSESN_READ_MORE_DATA
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo070_readObj70(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    SDNPSESN_QUAL qualifier);

  /* function: sdnpo070_writeObj70
   * purpose: write file data
   * arguments:
   *  pSession - session we are responding to
   *  pResponse - pointer to response message info
   *  pObjHeader - object header we are processing
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo070_writeObj70(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRxFragment,
    TMWSESN_TX_DATA *pResponse,
    DNPUTIL_OBJECT_HEADER *pObjHeader);
 
  /* function: sdnpo070_assignClass
  * purpose: process assign class request
   * arguments:
   *  pSession - session we are responding to
   *  pObjHeader - object header we are processing
   *  pRxFragment - received request fragment
   *  pOffset - offset into received fragment
   *  classMask - new class mask to assign
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo070_assignClass(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRxFragment,
    DNPUTIL_OBJECT_HEADER *pObjHeader, 
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpo070_fileCmdComplete
   * purpose: Function user can call when a sdnpdata_filexxx
   *  command that has returned ASYNC has completed. The SCL
   *  will start a timer to recall the sdnpdata_filexxx function
   *  periodically. User can call this function instead to cause
   *  the command to be retried immediately.
   * arguments:
   *  pSession - session request was made on.
   *  handle - handle that was returned when file was opened
   * returns:
   *  void
   */
 void TMWDEFS_GLOBAL sdnpo070_fileCmdComplete(
   TMWSESN *pSession,
   TMWTYPES_ULONG handle);

#ifdef __cplusplus
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ70 */
#endif /* SDNPO070_DEFINED */
