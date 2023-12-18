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

/* file: sdnpo070.c
 * description: DNP3 Slave support for Object 70 File Transfer Objects
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/sdnpdiag.h"
#include "tmwscl/dnp/dnpcnfg.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwdtime.h"

#include "tmwscl/dnp/sdnpo070.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpmem.h"
#include "tmwscl/dnp/sdnpevnt.h"
#include "tmwscl/dnp/dnpdtime.h"

#if SDNPDATA_SUPPORT_OBJ70


/* forward declarations */
static void TMWDEFS_CALLBACK _fileCmdRetryTimeout(
  void *pCallbackParam); 

static DNPDEFS_FILE_TFER_STAT TMWDEFS_LOCAL _readDirectory(
  TMWSESN *pSession);

static void TMWDEFS_LOCAL _buildReadResponse(
  SDNPSESN *pSDNPSession,
  TMWSESN_TX_DATA *pResponse);

static void TMWDEFS_LOCAL _buildReadDirectoryResponse(
  SDNPSESN *pSDNPSession,
  TMWSESN_TX_DATA *pResponse);


/* function: _closeFile
 * purpose: Close either the xml "file" or the database file
 * arguments:
 * returns:
 *  void
 */
static DNPDEFS_FILE_CMD_STAT TMWDEFS_LOCAL _closeFile(
  TMWSESN *pSession,
  TMWTYPES_ULONG handle)
{
#if SDNPDATA_SUPPORT_XML2 && SDNPDATA_SUPPORT_READ_XML
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = 
    (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;
  if(pObj70FileCtrl->xml2DeviceProfileOpen)
    return(sdnpxml2_closeDeviceProfile(pSession, handle));
  else
#endif
    return(sdnpdata_closeFile(pSession, handle));
}

/* function: _allocateFileCtrl
 * purpose: 
 * arguments:
 * returns:
 *  void
 */
static SDNPO070_FILE_CONTROL_BLOCK * TMWDEFS_LOCAL _allocateFileCtrl( 
  SDNPSESN *pSDNPSession)
{
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl;

  /* Check to see if a file is already open, since we only allow one at a time */
  if(pSDNPSession->pObj70FileCtrl != TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }
    
  pObj70FileCtrl = (SDNPO070_FILE_CONTROL_BLOCK *)sdnpmem_alloc(SDNPMEM_OBJECT70_CTRL_TYPE);
  if(pObj70FileCtrl != TMWDEFS_NULL)
  {
    pSDNPSession->pObj70FileCtrl = pObj70FileCtrl;

    tmwtimer_init(&pObj70FileCtrl->fileTransferTimer);
    tmwtimer_init(&pObj70FileCtrl->retryTimer);
    pObj70FileCtrl->filenameLength  = 0;
    pObj70FileCtrl->respState       = SDNPO070_FILE_RESP_IDLE;
    pObj70FileCtrl->retryState      = SDNPO070_FILE_RETRY_IDLE;
    pObj70FileCtrl->eventState      = SDNPO070_EVENT_STAT_NOTREADY;
    pObj70FileCtrl->classMask       = sdnpdata_fileEventClass(pSDNPSession->pDbHandle);
    pObj70FileCtrl->handle          = 0;
    pObj70FileCtrl->rspHandle       = 0;
    pObj70FileCtrl->blockNumber     = 0;
    pObj70FileCtrl->lastBlockWritten= 0xffff;
    pObj70FileCtrl->lastBlockRead   = 0xffff;
    pObj70FileCtrl->size            = 0;
    pObj70FileCtrl->maxBlockSize    = 0;
    pObj70FileCtrl->errorClose      = TMWDEFS_FALSE;
    pObj70FileCtrl->readResponseWaitConfirm    = TMWDEFS_FALSE;
    pObj70FileCtrl->xferComplete    = TMWDEFS_FALSE;
    pObj70FileCtrl->last            = TMWDEFS_FALSE;

#if SDNPDATA_SUPPORT_XML2 
    pObj70FileCtrl->xml2DeviceProfileOpen = TMWDEFS_FALSE;
#endif
  }

  return(pObj70FileCtrl);
}

static void TMWDEFS_LOCAL _deleteFileControl(
  SDNPSESN *pSDNPSession)
{
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = 
    (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

  if(pObj70FileCtrl->retryState != SDNPO070_FILE_RETRY_CLOSE)
  {
    tmwtimer_cancel(&pObj70FileCtrl->retryTimer);
    tmwtimer_cancel(&pObj70FileCtrl->fileTransferTimer);
    sdnpmem_free(pSDNPSession->pObj70FileCtrl);
    pSDNPSession->pObj70FileCtrl = TMWDEFS_NULL;
  }
}


/* function: _internalAddEvent 
 * purpose: 
 * arguments:
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _internalAddEvent(
  SDNPSESN *pSDNPSession)
{
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = 
    (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

  pObj70FileCtrl->eventState = SDNPO070_EVENT_STAT_READY;
  sdnpevnt_updateEvents((TMWSESN *)pSDNPSession, pObj70FileCtrl->classMask);
}

/* function: _errorCloseFile 
 * purpose: 
 * arguments:
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _errorCloseFile(
  TMWSESN *pSession,
  TMWTYPES_ULONG handle)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  DNPDEFS_FILE_CMD_STAT status = _closeFile(pSession, handle); 
  if(status == DNPDEFS_FILE_CMD_STAT_ASYNC)
  {
    if(pSDNPSession->pObj70FileCtrl != TMWDEFS_NULL)
    {
      SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;
      /* Retry database close later, 
       * Don't send File Command Status object.
       * Transport Status will be sent instead.
       */
      pObj70FileCtrl->retryState = SDNPO070_FILE_RETRY_CLOSE;
      pObj70FileCtrl->errorClose = TMWDEFS_TRUE;
      tmwtimer_start(&pObj70FileCtrl->retryTimer, 
        SDNPCNFG_OBJ70_RETRY_TIME, (pSession)->pChannel, _fileCmdRetryTimeout, pSession);
    }
  }
}

/* function: _closeFileRequest
 * purpose: 
 * arguments:
 * returns:
 *  void
 */
static DNPDEFS_FILE_CMD_STAT TMWDEFS_LOCAL _closeFileRequest(
  SDNPSESN *pSDNPSession,
  TMWTYPES_ULONG handle)
{
  DNPDEFS_FILE_CMD_STAT status = _closeFile((TMWSESN *)pSDNPSession, handle); 
  if(status == DNPDEFS_FILE_CMD_STAT_ASYNC)
  {
    TMWSESN *pSession = (TMWSESN *)pSDNPSession;
    SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

    /* Retry database close later */
    pObj70FileCtrl->retryState = SDNPO070_FILE_RETRY_CLOSE;
    tmwtimer_start(&pObj70FileCtrl->retryTimer, 
      SDNPCNFG_OBJ70_RETRY_TIME, pSession->pChannel, _fileCmdRetryTimeout, pSession);
  } 
  return(status);
}

/* function: _fileTimeout
 * purpose: 
 * arguments:
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _fileTimeout(
  void *pCallbackParam)
{
  SDNPSESN                    *pSDNPSession = (SDNPSESN *)pCallbackParam;
  TMWSESN                     *pSession = (TMWSESN*)pSDNPSession;
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = 
    (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

  SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_FILE_TO);

  /* This should exist whenever a timeout occurs */
  if(pObj70FileCtrl != TMWDEFS_NULL)
  {
    tmwtimer_cancel(&pObj70FileCtrl->retryTimer);

    _errorCloseFile(pSession, pObj70FileCtrl->handle);

    /* Application Layer doc 4.17.1.1 says outstation must send a
     *  File Transport status object group 70 var 6 status code 17
     */
    pObj70FileCtrl->rspHandle    = pObj70FileCtrl->handle;
    pObj70FileCtrl->respState    = SDNPO070_FILE_RESP_TRANSPORT_STAT;
    pObj70FileCtrl->tferStatus   = DNPDEFS_FILE_TFER_STAT_HANDLE_EXP;
    pObj70FileCtrl->xferComplete = TMWDEFS_TRUE;

    /* add the event, so this will be sent either polled or unsolicited */
    _internalAddEvent(pSDNPSession); 
  }
}

/* function: _openFileRetryCmd 
 * purpose: Call sdnpdata function to see if database open function
 *  is complete.
 * arguments:
 *  pSession - pointer to session
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _openFileRetryCmd(
  TMWSESN *pSession)
{
  DNPDEFS_FILE_CMD_STAT status;
  TMWDTIME timeOfCreation; 
  DNPDEFS_FILE_TYPE type;
  TMWTYPES_ULONG handle;
  TMWTYPES_ULONG size;
  TMWTYPES_USHORT mode;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = 
    (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

  /* Open file */
  mode = pObj70FileCtrl->data.open.mode;
  status = sdnpdata_openFile(pSession, pObj70FileCtrl->filename, pObj70FileCtrl->authKey, 
    mode, &pObj70FileCtrl->maxBlockSize, &pObj70FileCtrl->data.open.permissions, &timeOfCreation, 
    &handle, &size, &type);
    
  if(status == DNPDEFS_FILE_CMD_STAT_SUCCESS)
  {
    pObj70FileCtrl->retryState = SDNPO070_FILE_RETRY_IDLE;
    pObj70FileCtrl->type = type;
    pObj70FileCtrl->handle = handle;
    pObj70FileCtrl->rspHandle = handle;
    tmwtimer_start(&pObj70FileCtrl->fileTransferTimer, 
        pSDNPSession->fileTransferTimeout, pSession->pChannel, _fileTimeout, pSession);
      
    /* If opened for writing returned size should be 0 */
    if((mode == DNPDEFS_FILE_MODE_WRITE)
      || (mode == DNPDEFS_FILE_MODE_APPEND))
    {
      size = 0;
    }
    pObj70FileCtrl->size = size;
  }
  else if (status == DNPDEFS_FILE_CMD_STAT_ASYNC)
  {
    /* The information has already been saved, just restart the retry timer */
    tmwtimer_start(&pObj70FileCtrl->retryTimer, 
      SDNPCNFG_OBJ70_RETRY_TIME, pSession->pChannel, _fileCmdRetryTimeout, pSession);
    return;
  }
  else
  { 
    /* Failure */
    handle = 0;
    pObj70FileCtrl->xferComplete = TMWDEFS_TRUE;
  }
    
  pObj70FileCtrl->respState = SDNPO070_FILE_RESP_CMD;
  pObj70FileCtrl->cmdStatus = status;

  /* add the event, so this will be sent either polled or unsolicited */
  _internalAddEvent(pSDNPSession); 
}

/* function: _getInfoRetryCmd 
 * purpose: Call sdnpdata function to see if database getInfo function
 *  is complete.
 * arguments:
 *  pSession - pointer to session
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _getInfoRetryCmd(
  TMWSESN *pSession)
{
  DNPDEFS_FILE_TFER_STAT       status;
  SDNPSESN                    *pSDNPSession = (SDNPSESN *)pSession;
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = 
    (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

  SDNPO070_FILE_DESCRIPTOR *pFileDesc = &pObj70FileCtrl->data.getFileInfo.fileDesc;  

  /* Get File Info */
  status = sdnpdata_getFileInfo(pSession, pObj70FileCtrl->filename, &pFileDesc->type, &pFileDesc->size, 
    &pFileDesc->fileCreationTime, &pFileDesc->permissions);
  if(status == DNPDEFS_FILE_TFER_STAT_ASYNC)
  {
    tmwtimer_start(&pObj70FileCtrl->retryTimer, 
      SDNPCNFG_OBJ70_RETRY_TIME, pSession->pChannel, _fileCmdRetryTimeout, pSession);
    return;
  }

  /* Save this information in case application confirm is not received and
   * this must be resent
   */
  pObj70FileCtrl->respState    = SDNPO070_FILE_RESP_GETINFO;
  pObj70FileCtrl->cmdStatus    = status;
  pObj70FileCtrl->xferComplete = TMWDEFS_TRUE;

  _internalAddEvent(pSDNPSession);
}

/* function: _deleteFileRetryCmd 
 * purpose: Call sdnpdata function to see if database delete
 *  function is complete
 * arguments:
 *  pSession - pointer to session
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _deleteFileRetryCmd(
  TMWSESN *pSession)
{
  DNPDEFS_FILE_CMD_STAT status;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = 
    (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

  /* Try to delete file */
  status = sdnpdata_deleteFile(pSession, pObj70FileCtrl->filename, pObj70FileCtrl->authKey);
 
  if (status == DNPDEFS_FILE_CMD_STAT_ASYNC)
  {
    /* The information has already been saved, just restart the retry timer */
    tmwtimer_start(&pObj70FileCtrl->retryTimer, 
      SDNPCNFG_OBJ70_RETRY_TIME, pSession->pChannel, _fileCmdRetryTimeout, pSession);
    return;
  }

  pObj70FileCtrl->respState    = SDNPO070_FILE_RESP_CMD;
  pObj70FileCtrl->xferComplete = TMWDEFS_TRUE;
  pObj70FileCtrl->cmdStatus    = status;

  /* add the event, so this will be sent either polled or unsolicited */
  _internalAddEvent(pSDNPSession); 
}

/* function: _closeFileRetryCmd 
 * purpose: Call sdnpdata function to see if database close
 *  function is complete
 * arguments:
 *  pSession - pointer to session
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _closeFileRetryCmd(
  TMWSESN *pSession)
{
  DNPDEFS_FILE_CMD_STAT status;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = 
    (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

  /* Close file transfer */
  status = _closeFile((TMWSESN *)pSDNPSession, pObj70FileCtrl->handle); 
  if(status == DNPDEFS_FILE_CMD_STAT_ASYNC)
  {
    /* The information has already been saved, just restart the retry timer */
    tmwtimer_start(&pObj70FileCtrl->retryTimer, 
      SDNPCNFG_OBJ70_RETRY_TIME, pSession->pChannel, _fileCmdRetryTimeout, pSession);
    return;
  }
  
  /* If this close retry was because of an error, don't send File Command Status
   * File Transport Status was already sent instead
   */
  if(!pObj70FileCtrl->errorClose)
  {
    pObj70FileCtrl->respState    = SDNPO070_FILE_RESP_CMD;
    pObj70FileCtrl->cmdStatus    = status;
    pObj70FileCtrl->rspHandle    = pObj70FileCtrl->handle;
    pObj70FileCtrl->retryState   = SDNPO070_FILE_RETRY_IDLE;
    pObj70FileCtrl->xferComplete = TMWDEFS_TRUE;

    /* add the event, so this will be sent either polled or unsolicited */
    _internalAddEvent(pSDNPSession); 
  }
  else
  {
     pObj70FileCtrl->retryState = SDNPO070_FILE_RETRY_IDLE;
     _deleteFileControl(pSDNPSession);
  }
}

/* function: _readFileRetryCmd 
 * purpose: Call sdnpdata function to see if database read
 *  function is complete
 * arguments:
 *  pSession - pointer to session
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _readFileRetryCmd(
  TMWSESN *pSession)
{
  DNPDEFS_FILE_TFER_STAT status;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = 
    (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

#if SDNPDATA_SUPPORT_XML2 && SDNPDATA_SUPPORT_READ_XML
  if(pObj70FileCtrl->xml2DeviceProfileOpen)
  {
    status = sdnpxml2_readDeviceProfile(pSession, pObj70FileCtrl->handle, &pObj70FileCtrl->last,
      &pObj70FileCtrl->data.read.bytesRead, pObj70FileCtrl->data.read.buffer);
  }
  else
#endif  
  /* Simple file read data */
  status = sdnpdata_readFile(pSession, pObj70FileCtrl->handle, &pObj70FileCtrl->last,
      &pObj70FileCtrl->data.read.bytesRead, pObj70FileCtrl->data.read.buffer);

  /* Check return status */
  if (status == DNPDEFS_FILE_CMD_STAT_ASYNC)
  {
    /* Restart the retry timer */
    tmwtimer_start(&pObj70FileCtrl->retryTimer, 
      SDNPCNFG_OBJ70_RETRY_TIME, pSession->pChannel, _fileCmdRetryTimeout, pSession);
    return;
  }
  else if (status == DNPDEFS_FILE_CMD_STAT_SUCCESS)
  {
    pObj70FileCtrl->lastBlockRead = pObj70FileCtrl->blockNumber;
    pObj70FileCtrl->readResponseWaitConfirm = TMWDEFS_TRUE;
    pObj70FileCtrl->respState = SDNPO070_FILE_RESP_READ;
  }
  else
  {
    pObj70FileCtrl->respState = SDNPO070_FILE_RESP_TRANSPORT_STAT;
    pObj70FileCtrl->tferStatus = status;
    pObj70FileCtrl->rspHandle  = pObj70FileCtrl->handle;
    pObj70FileCtrl->xferComplete = TMWDEFS_TRUE;
    /* close file on error reading */
    _errorCloseFile(pSession, pObj70FileCtrl->handle);
  }

  /* add the event, so this will be sent either polled or unsolicited */
  _internalAddEvent(pSDNPSession); 
}

/* function: _readDirectoryRetryCmd 
 * purpose: Call sdnpdata function to see if database read directory
 *  function is complete
 * arguments:
 *  pSession - pointer to session
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _readDirectoryRetryCmd(
  TMWSESN *pSession)
{
  SDNPSESN                    *pSDNPSession;
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl;
  DNPDEFS_FILE_TFER_STAT       status;

  pSDNPSession = (SDNPSESN *)pSession;

  if(pSDNPSession->pObj70FileCtrl == TMWDEFS_NULL)
    return;

  pObj70FileCtrl = (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

  status = _readDirectory(pSession);
  if(status == DNPDEFS_FILE_TFER_STAT_ASYNC)
  {
    tmwtimer_start(&pObj70FileCtrl->retryTimer, 
      SDNPCNFG_OBJ70_DIR_RETRY_TIME, pSession->pChannel, _fileCmdRetryTimeout, pSession);
    return;
  }
  else if(status == DNPDEFS_FILE_CMD_STAT_SUCCESS)
  {
    pObj70FileCtrl->respState = SDNPO070_FILE_RESP_READ_DIR;
    pObj70FileCtrl->lastBlockRead = pObj70FileCtrl->blockNumber;
  }
  else
  {
    pObj70FileCtrl->respState = SDNPO070_FILE_RESP_TRANSPORT_STAT;
    pObj70FileCtrl->tferStatus = status;
    pObj70FileCtrl->rspHandle  = pObj70FileCtrl->handle;
    pObj70FileCtrl->xferComplete = TMWDEFS_TRUE;
    /* close file on error reading */
    _errorCloseFile(pSession, pObj70FileCtrl->handle);
  }

  _internalAddEvent(pSDNPSession);
}


/* function: _writeFileRetryCmd 
 * purpose: Call sdnpdata function to see if database write function
 *  is complete.
 * arguments:
 *  pSession - pointer to session
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _writeFileRetryCmd(
  TMWSESN *pSession)
{
  DNPDEFS_FILE_TFER_STAT status;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = 
    (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

  /* Write Data */
  status = sdnpdata_writeFile(pSession, pObj70FileCtrl->handle, pObj70FileCtrl->last, 
    pObj70FileCtrl->data.write.numBytes, pObj70FileCtrl->data.write.buffer);
 
  if (status == DNPDEFS_FILE_CMD_STAT_ASYNC)
  {
    /* The information has already been saved, just restart the retry timer */
    tmwtimer_start(&pObj70FileCtrl->retryTimer, 
      SDNPCNFG_OBJ70_RETRY_TIME, pSession->pChannel, _fileCmdRetryTimeout, pSession);
    return;
  }
  else if(status != DNPDEFS_FILE_TFER_STAT_SUCCESS)
  {
    pObj70FileCtrl->xferComplete = TMWDEFS_TRUE;
    /* 4.17.1.3 Application Layer Doc says close file on error */
    _errorCloseFile(pSession, pObj70FileCtrl->handle);
  }
  else
  {
    pObj70FileCtrl->lastBlockWritten = pObj70FileCtrl->blockNumber;
  }
 
  /* For either success or failure create event now */
  pObj70FileCtrl->rspHandle  = pObj70FileCtrl->handle;
  pObj70FileCtrl->respState  = SDNPO070_FILE_RESP_TRANSPORT_STAT;
  pObj70FileCtrl->tferStatus = status;
  
  /* add the event, so this will be sent either polled or unsolicited */
  _internalAddEvent(pSDNPSession); 
}

/* function: _fileCmdRetryTimeout
 * purpose: Call the sdnpdata function again to see if it has completed
 * arguments: 
 *  pCallbackParam - pointer to session structure
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _fileCmdRetryTimeout(
  void *pCallbackParam)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pCallbackParam;
  TMWSESN *pSession = (TMWSESN*)pSDNPSession;

  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = 
    (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;
  
  if(pObj70FileCtrl != TMWDEFS_NULL) 
  {
    switch(pObj70FileCtrl->retryState)
    {
    case SDNPO070_FILE_RETRY_OPEN:
      _openFileRetryCmd(pSession); 
      break;
   
    case SDNPO070_FILE_RETRY_READ:
      _readFileRetryCmd(pSession);
      break;

    case SDNPO070_FILE_RETRY_READ_DIR:
      _readDirectoryRetryCmd(pSession);
      break;
      
    case SDNPO070_FILE_RETRY_WRITE:
      _writeFileRetryCmd(pSession);
      break;

    case SDNPO070_FILE_RETRY_GETINFO:
      _getInfoRetryCmd(pSession);
      break;

    case SDNPO070_FILE_RETRY_DELETE:
      _deleteFileRetryCmd(pSession);
      break;

    case SDNPO070_FILE_RETRY_CLOSE:
      _closeFileRetryCmd(pSession);
      break;

    default:
      break;
    }
  }
}

/* function: _buildGetFileInfoResponseNoHdr 
 * purpose: 
 * arguments:
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _buildGetFileInfoResponseNoHdr(
  SDNPSESN *pSDNPSession,
  TMWSESN_TX_DATA *pResponse)
{
  TMWTYPES_USHORT tmpShort;
  TMWTYPES_MS_SINCE_70 msSince70;
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = 
    (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;
  SDNPO070_FILE_DESCRIPTOR *pFileDesc = &pObj70FileCtrl->data.getFileInfo.fileDesc;  
  TMWTYPES_USHORT filenameLength = pFileDesc->nameLen;

  /* Bytes In Object */
  tmpShort = (TMWTYPES_USHORT)(20 + filenameLength);
  tmwtarg_store16(&tmpShort, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 2;

  /* File Name Offset */
  tmpShort = 20;
  tmwtarg_store16(&tmpShort, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 2;

  /* File Name Size */
  tmwtarg_store16(&filenameLength, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 2;

  /* File Type */
  tmwtarg_store16(&pFileDesc->type, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 2;

  /* File Size */
  tmwtarg_store32(&pFileDesc->size, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 4;

  /* Time Of Creation */
  dnpdtime_dateTimeToMSSince70(&msSince70, &pFileDesc->fileCreationTime);
  dnpdtime_writeMsSince70(pResponse->pMsgBuf + pResponse->msgLength, &msSince70);
  pResponse->msgLength += 6;

  /* Permissions */
  tmwtarg_store16(&pFileDesc->permissions, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 2;

  /* Request Id */
  tmwtarg_store16(&pObj70FileCtrl->requestId, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 2;

  /* Filename */
  memcpy(pResponse->pMsgBuf + pResponse->msgLength, pObj70FileCtrl->filename, filenameLength);
  pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + filenameLength);
}

/* function: _buildCmdResponse 
 * purpose: 
 * arguments:
 *  NOTE
 *  Pass these values in as arguments instead of using pObj70FileCtrl
 *  so that a response can be sent even if we cannot allocate control
 *  structure.
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _buildCmdResponse(
  TMWSESN_TX_DATA *pResponse, 
  TMWTYPES_ULONG handle, 
  TMWTYPES_ULONG size, 
  TMWTYPES_USHORT maxBlockSize,
  TMWTYPES_USHORT requestId,
  DNPDEFS_FILE_CMD_STAT status,
  TMWTYPES_BOOL requestConfirm)
{
  TMWTYPES_USHORT tmpShort;

  /* Generate response */
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_OBJ_70_FILE_EVENTS;
  pResponse->pMsgBuf[pResponse->msgLength++] = 4;
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_QUAL_16BIT_FREE_FORMAT;
  pResponse->pMsgBuf[pResponse->msgLength++] = 1;

  DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, DNPDEFS_OBJ_70_FILE_EVENTS, 4, DNPDEFS_QUAL_16BIT_FREE_FORMAT); 

  /* Bytes In Object */
  tmpShort = 13;
  tmwtarg_store16(&tmpShort, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 2;

  /* File Handle */
  tmwtarg_store32(&handle, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 4;

  /* File Size */
  tmwtarg_store32(&size, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 4;

  /* Maximum Block Size */
  tmwtarg_store16(&maxBlockSize, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 2;

  /* Request Id */
  tmwtarg_store16(&requestId, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 2;

  /* Status */
  pResponse->pMsgBuf[pResponse->msgLength++] = status;

  DNPDIAG_SHOW_FILE_STATUS(
    pResponse->pSession,
    handle,
    size,
    maxBlockSize,
    requestId,
    status,
    0,
    TMWDEFS_NULL);

  if(requestConfirm)  
  { 
    /* Delayed File transfer status objects are reported as events */
    pResponse->txFlags |= TMWSESN_TXFLAGS_CONTAINS_EVENTS;
  }
}

/* function: _buildReadResponse 
 * purpose: 
 * arguments:
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _buildReadResponse(
  SDNPSESN *pSDNPSession,
  TMWSESN_TX_DATA *pResponse)
{
  TMWTYPES_USHORT bytesInObject;
  TMWTYPES_USHORT bytesRead;
  TMWTYPES_ULONG block;

  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = 
    (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

  /* Read successful, return File Transfer Object with data */
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_OBJ_70_FILE_EVENTS;
  pResponse->pMsgBuf[pResponse->msgLength++] = 5;
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_QUAL_16BIT_FREE_FORMAT;
  pResponse->pMsgBuf[pResponse->msgLength++] = 1;

  DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, DNPDEFS_OBJ_70_FILE_EVENTS, 5, DNPDEFS_QUAL_16BIT_FREE_FORMAT); 

  /* Number of bytes in object */
  bytesRead = pObj70FileCtrl->data.read.bytesRead;
  bytesInObject = (TMWTYPES_USHORT)(8 + bytesRead);
  tmwtarg_store16(&bytesInObject, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 2;

  /* Handle */
  tmwtarg_store32(&pObj70FileCtrl->handle, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 4;

  /* Block */
  block = pObj70FileCtrl->blockNumber;
  if(pObj70FileCtrl->last)
    block |= 0x80000000L;
  tmwtarg_store32(&block, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 4;

  /* Data */
  memcpy((pResponse->pMsgBuf + pResponse->msgLength), pObj70FileCtrl->data.read.buffer, bytesRead);
  pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + bytesRead);

  DNPDIAG_SHOW_FILE_DATA(pResponse->pSession, pObj70FileCtrl->handle, pObj70FileCtrl->blockNumber, bytesRead, pObj70FileCtrl->last, TMWDEFS_TRUE, TMWDIAG_ID_TX);

  pObj70FileCtrl->blockNumber++;  
}

/* function: _buildReadDirectoryResponse 
 * purpose: 
 * arguments:
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _buildReadDirectoryResponse(
  SDNPSESN *pSDNPSession,
  TMWSESN_TX_DATA *pResponse)
{
  TMWTYPES_USHORT      bytesInObjectOffset;
  TMWTYPES_USHORT      bytesInObject;
  int                  i;
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = 
    (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

  /* Send back an obj 70 variation 5, containing directory information */
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_OBJ_70_FILE_EVENTS;
  pResponse->pMsgBuf[pResponse->msgLength++] = 5;
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_QUAL_16BIT_FREE_FORMAT;
  pResponse->pMsgBuf[pResponse->msgLength++] = 1;

  DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, DNPDEFS_OBJ_70_FILE_EVENTS, 5, DNPDEFS_QUAL_16BIT_FREE_FORMAT); 

  bytesInObjectOffset = pResponse->msgLength;
  
  /* Number of bytes in object, assume directory is empty */
  bytesInObject = 8;
  tmwtarg_store16(&bytesInObject, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 2;

  /* Handle */
  tmwtarg_store32(&pObj70FileCtrl->handle, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 4;

  /* Block, assume last block */
  pObj70FileCtrl->blockNumber |= 0x80000000L;
  tmwtarg_store32(&pObj70FileCtrl->blockNumber, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 4;

  /* Put 1 or more object 70 variation 7 in as the data portion of this response 
   * until there are no more entries, or there is no more room in message
   */
  for(i = 0; i < pObj70FileCtrl->data.readDir.fileDescIndex; i++) 
  {
    TMWTYPES_USHORT tmpShort;
    TMWTYPES_MS_SINCE_70 msSince70;
    SDNPO070_FILE_DESCRIPTOR *pFileDesc = &pObj70FileCtrl->data.readDir.fileDesc[i];

    /* File Name Offset */
    tmpShort = 20;
    tmwtarg_store16(&tmpShort, pResponse->pMsgBuf + pResponse->msgLength);
    pResponse->msgLength += 2;

    /* File Name Size */
    tmwtarg_store16(&pFileDesc->nameLen, pResponse->pMsgBuf + pResponse->msgLength);
    pResponse->msgLength += 2;

    /* File Type */
    tmwtarg_store16(&pFileDesc->type, pResponse->pMsgBuf + pResponse->msgLength);
    pResponse->msgLength += 2;

    /* File Size */
    tmwtarg_store32(&pFileDesc->size, pResponse->pMsgBuf + pResponse->msgLength);
    pResponse->msgLength += 4;

    /* Time Of Creation */
    dnpdtime_dateTimeToMSSince70(&msSince70, &pFileDesc->fileCreationTime);
    dnpdtime_writeMsSince70(pResponse->pMsgBuf + pResponse->msgLength, &msSince70);
    pResponse->msgLength += 6;

    /* Permissions */
    tmwtarg_store16(&pFileDesc->permissions, pResponse->pMsgBuf + pResponse->msgLength);
    pResponse->msgLength += 2;

    /* Request Id */
    tmwtarg_store16(&pObj70FileCtrl->requestId, pResponse->pMsgBuf + pResponse->msgLength);
    pResponse->msgLength += 2;

    /* Filename */
    memcpy(pResponse->pMsgBuf + pResponse->msgLength, pFileDesc->pName, pFileDesc->nameLen);
    pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + pFileDesc->nameLen);

    DNPDIAG_SHOW_FILE_INFO((TMWSESN*)pSDNPSession, tmpShort, pFileDesc->nameLen, pFileDesc->type,
      pFileDesc->size, &pFileDesc->fileCreationTime, pFileDesc->permissions, pFileDesc->pName);
  }

  /* Number of bytes in object */
  bytesInObject = (TMWTYPES_USHORT)(pResponse->msgLength - bytesInObjectOffset - 2);
  tmwtarg_store16(&bytesInObject, pResponse->pMsgBuf + bytesInObjectOffset);
    
  /* If this is not the last entry, clear the last bit */
  if(!pObj70FileCtrl->last)
  { 
    /* (2 bytes for bytesInObject and 4 bytes for handle) == 6 */
    pObj70FileCtrl->blockNumber &=  ~0x80000000L;
    tmwtarg_store32(&pObj70FileCtrl->blockNumber, pResponse->pMsgBuf + bytesInObjectOffset + 6);
  } 

  pObj70FileCtrl->blockNumber++;
}

/* function: _buildXferStatusResponse 
 * purpose: 
 * arguments:
 *  NOTE
 *  Pass these values in as arguments instead of using pObj70FileCtrl
 *  so that a response can be sent even if we cannot allocate control
 *  structure.
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _buildXferStatusResponse(
  SDNPSESN *pSDNPSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_ULONG handle,
  TMWTYPES_ULONG block,
  DNPDEFS_FILE_TFER_STAT status,
  TMWTYPES_BOOL requestConfirm)
{
  TMWTYPES_USHORT bytesInObject;
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = 
    (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

  /* Error during read, return File Transfer Status Object */
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_OBJ_70_FILE_EVENTS;
  pResponse->pMsgBuf[pResponse->msgLength++] = 6;
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_QUAL_16BIT_FREE_FORMAT;
  pResponse->pMsgBuf[pResponse->msgLength++] = 1;

  DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, DNPDEFS_OBJ_70_FILE_EVENTS, 6, DNPDEFS_QUAL_16BIT_FREE_FORMAT); 

  /* Number of bytes in object */
  bytesInObject = 9;
  tmwtarg_store16(&bytesInObject, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 2;

  /* Handle */
  tmwtarg_store32(&handle, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 4;
  
  /* If this was a read or write response we need to increment block number  
   * otherwise this is sent on error and block number doesn't matter 
   * after this xfer status was sent. 
   */
  if(pObj70FileCtrl != TMWDEFS_NULL)
    pObj70FileCtrl->blockNumber++;

  /* Block */
  tmwtarg_store32(&block, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 4;

  /* Status */
  pResponse->pMsgBuf[pResponse->msgLength++] = status;
 
  DNPDIAG_SHOW_FILE_DATA_STATUS(
    pResponse->pSession,
    handle, 
    (block & 0x7fffffff),
    (TMWTYPES_BOOL)((block & 0x80000000) != 0),
    status,
    0,
    TMWDEFS_NULL);
  
  /* Need to request an application confirmation, this is not normally
   * done for response to write requests, but is done for delayed response to
   * file write requests, so set the confirm bit here.
   */
  if(requestConfirm)
  { 
    /* Delayed responses are considered events and should request appl confirm */
    /* Do not add SA prechallenge here, it will be done if needed because of EVENTS */
    pResponse->txFlags |= TMWSESN_TXFLAGS_CONTAINS_EVENTS;
    pResponse->pMsgBuf[0] |= DNPDEFS_AC_CONFIRM;
    pResponse->txFlags &= ~TMWSESN_TXFLAGS_NO_RESPONSE;
  }
}

/* function: _processOpenRequest
 * purpose: 
 * arguments:
 * returns:
 *  TMWDEFS_TRUE if processed
 *  TMWDEFS_FALSE if variation was invalid
 */
static TMWTYPES_BOOL TMWDEFS_LOCAL _processOpenRequest(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = TMWDEFS_NULL;
  DNPDEFS_FILE_PERMISSIONS permissions;
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWTYPES_USHORT filenameOffset;
  TMWTYPES_USHORT filenameLength;
  TMWTYPES_USHORT bytesInObject;
  TMWTYPES_USHORT requestId;
  TMWDTIME timeOfCreation;
  DNPDEFS_FILE_TYPE type;
  TMWTYPES_ULONG authKey;
  TMWTYPES_USHORT mode;
  TMWTYPES_CHAR filename[DNPCNFG_MAX_FILENAME+1];
  DNPDEFS_FILE_CMD_STAT status;

  TMWTYPES_ULONG size = 0;
  TMWTYPES_ULONG handle = 0;
  TMWTYPES_USHORT maxBlockSize = 0;
  
  if(pObjHeader->variation != 3)
  {
    /* Don't try to process rest of message. Return false so Object Unknown error IIN bit is set */
    pRxFragment->offset = pRxFragment->msgLength;
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_FILE_VAR);
    return(TMWDEFS_FALSE);
  }
  
  /* Length of at least 28 bytes in request was verified in sdnpo070_processRequest */ 
  
  /* Number of bytes in object */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &bytesInObject);
  pRxFragment->offset += 2;

  /* Filename Offset */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &filenameOffset);
  pRxFragment->offset += 2;

  /* Filename Size */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &filenameLength);
  pRxFragment->offset += 2;

  /* Time of Creation */
  dnpdtime_readMsSince70(&msSince70, pRxFragment->pMsgBuf + pRxFragment->offset);
  dnpdtime_msSince70ToDateTime(&timeOfCreation, &msSince70);
  pRxFragment->offset += 6;

  /* Permissions */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &permissions);
  pRxFragment->offset += 2;

  /* Authentication Key */
  tmwtarg_get32(pRxFragment->pMsgBuf + pRxFragment->offset, &authKey);
  pRxFragment->offset += 4;

  /* File Size */
  tmwtarg_get32(pRxFragment->pMsgBuf + pRxFragment->offset, &size);
  pRxFragment->offset += 4;

  /* Mode */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &mode);
  pRxFragment->offset += 2;

  /* Maximum Block Size */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &maxBlockSize);
  pRxFragment->offset += 2;

  /* Request ID */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &requestId);
  pRxFragment->offset += 2;
  
  /* Protect against badly formatted message */
  if((pRxFragment->offset + filenameLength) > pRxFragment->msgLength)
  {
    /* Don't try to process rest of message, set IIN and return true so another error IIN is not set */
    pRxFragment->offset = pRxFragment->msgLength;
    ((SDNPSESN*)pSession)->iin |= DNPDEFS_IIN_PARAM_ERROR;
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
    return(TMWDEFS_TRUE);
  }

  /* Make sure file name fits in array */
  if(filenameLength > DNPCNFG_MAX_FILENAME)
  {  
    /* Don't try to process rest of message, set IIN and return true so another error IIN is not set */
    pRxFragment->offset = pRxFragment->msgLength;
    ((SDNPSESN*)pSession)->iin |= DNPDEFS_IIN_PARAM_ERROR;
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_FILE_NAME);
    return(TMWDEFS_TRUE);
  }

  /* Copy filename and make sure it is null terminated */
  memcpy(filename, pRxFragment->pMsgBuf + pRxFragment->offset, filenameLength);
  pRxFragment->offset = pRxFragment->offset + filenameLength;
  filename[filenameLength] = '\0';

  pObj70FileCtrl = _allocateFileCtrl(pSDNPSession);
  
  if(pObj70FileCtrl == TMWDEFS_NULL)
  { 
    /* If this is a reopen of same file allow it. (Master SHOULD have sent an abort request, but clearly he is done.) */
    SDNPO070_FILE_CONTROL_BLOCK *pOldFileCtrl = (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;
    if(pOldFileCtrl != TMWDEFS_NULL)
    { 
      if((pOldFileCtrl->filenameLength == filenameLength)
        &&(!memcmp(filename, pOldFileCtrl->filename, filenameLength)))
      { 
        /* Close the file, so it can be reopened below */
        _closeFile(pSession, pOldFileCtrl->handle);

        /* Clean up the existing timers etc. and allocate a new structure */
        _deleteFileControl(pSDNPSession);
        pObj70FileCtrl = _allocateFileCtrl(pSDNPSession);

        SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_FILE_AUTO_CLOSE);
      } 
    }
  } 

  if(pObj70FileCtrl == TMWDEFS_NULL)
  {
    status = DNPDEFS_FILE_CMD_STAT_TOO_MANY;
  }
  else
  { 
    DNPDIAG_SHOW_FILE_OPEN(pSession, filename, filenameLength, size, &msSince70, permissions, authKey, mode, maxBlockSize, requestId);

    if(mode == DNPDEFS_FILE_MODE_READ)
    {
      /* Leave 8 bytes for header and 10 bytes for rest of message */
      if(maxBlockSize > (pResponse->maxLength - 18))
      {
        maxBlockSize = (TMWTYPES_USHORT)(pResponse->maxLength - 18);
      }
    }
    else
    {
      DNPCHNL *pDNPChannel = (DNPCHNL*)pSession->pChannel;
      if(maxBlockSize > (pDNPChannel->rxFragmentSize - 16))
      {
        maxBlockSize = (TMWTYPES_USHORT)(pDNPChannel->rxFragmentSize - 16);
      }
    } 
   
    if(maxBlockSize > SDNPCNFG_MAX_BLOCK_SIZE)
    {
      maxBlockSize = SDNPCNFG_MAX_BLOCK_SIZE;
    }

#if SDNPDATA_SUPPORT_XML2 && SDNPDATA_SUPPORT_READ_XML
    /* See if this is an open of the xml device profile. */
    status = sdnpxml2_openDeviceProfile(pSession, filename, authKey, mode,
      &timeOfCreation, &handle, &size, &type);
 
    if(status != DNPDEFS_FILE_CMD_STAT_SUCCESS) 
    {
      if(status != DNPDEFS_FILE_CMD_STAT_NOT_FOUND) 
      {
        /* It WAS a device profile, but it FAILED to open, clean up and send error back. */
        sdnpmem_free(pSDNPSession->pObj70FileCtrl);
        pSDNPSession->pObj70FileCtrl = TMWDEFS_NULL;
        handle = 0;
        _buildCmdResponse(pResponse, handle, size, maxBlockSize, requestId, status, TMWDEFS_FALSE);
        return(TMWDEFS_TRUE);
      }
    }

    /* If it was not the device profile, call database to try to open a "normal" file */
    if(status != DNPDEFS_FILE_CMD_STAT_SUCCESS)
#endif

    /* Open file */
    status = sdnpdata_openFile(pSession, filename, authKey, mode,
      &maxBlockSize, &permissions, &timeOfCreation, &handle, &size, &type);

    memcpy(pObj70FileCtrl->filename, filename, filenameLength+1);
    pObj70FileCtrl->filenameLength = filenameLength;

    if(status == DNPDEFS_FILE_CMD_STAT_SUCCESS)
    {
      pObj70FileCtrl->type = type;
      pObj70FileCtrl->handle = handle;
      pObj70FileCtrl->maxBlockSize = maxBlockSize;

      tmwtimer_start(&pObj70FileCtrl->fileTransferTimer, 
        pSDNPSession->fileTransferTimeout, pSession->pChannel, _fileTimeout, pSession);

      /* If opened for writing returned size should be 0 */
      if(mode == DNPDEFS_FILE_MODE_WRITE || mode == DNPDEFS_FILE_MODE_APPEND)
      {
        size = 0;
      }
      pObj70FileCtrl->rspHandle    = handle;
      pObj70FileCtrl->size         = size;
      pObj70FileCtrl->maxBlockSize = maxBlockSize;
      pObj70FileCtrl->requestId    = requestId;
    }
    else if (status == DNPDEFS_FILE_CMD_STAT_ASYNC)
    {
      /* SCL should send a NULL response now and later send the
       * full response as an event 
       */
      tmwtimer_start(&pObj70FileCtrl->fileTransferTimer, 
        pSDNPSession->fileTransferTimeout, pSession->pChannel, _fileTimeout, pSession);
        
      pObj70FileCtrl->authKey = authKey;
      pObj70FileCtrl->data.open.mode = mode;
      pObj70FileCtrl->requestId = requestId;
      pObj70FileCtrl->maxBlockSize = maxBlockSize;
      pObj70FileCtrl->data.open.mode = mode;
      pObj70FileCtrl->data.open.permissions = permissions;
      pObj70FileCtrl->retryState = SDNPO070_FILE_RETRY_OPEN;

      tmwtimer_start(&pObj70FileCtrl->retryTimer, 
        SDNPCNFG_OBJ70_RETRY_TIME, pSession->pChannel, _fileCmdRetryTimeout, pSession);
        
      /* This will send a NULL response now and respond later */
      return(TMWDEFS_TRUE);
    }
    else
    {  
      /* Failed, clean up. */
      sdnpmem_free(pSDNPSession->pObj70FileCtrl);
      pSDNPSession->pObj70FileCtrl = TMWDEFS_NULL;
      handle = 0;
    }
  }
 
  _buildCmdResponse(pResponse, handle, size, maxBlockSize, requestId, status, TMWDEFS_FALSE);
  return(TMWDEFS_TRUE);
}

/* function: _processCloseRequest
 * purpose: 
 * arguments:
 * returns:
 *  TMWDEFS_TRUE if processed
 *  TMWDEFS_FALSE if variation was invalid
 */
static TMWTYPES_BOOL TMWDEFS_LOCAL _processCloseRequest(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_USHORT bytesInObject;
  TMWTYPES_USHORT maxBlockSize;
  TMWTYPES_USHORT requestId;
  TMWTYPES_ULONG handle;
  TMWTYPES_ULONG size;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;
  DNPDEFS_FILE_CMD_STAT status = DNPDEFS_FILE_CMD_STAT_SUCCESS;

  if(pObjHeader->variation != 4)
  {
    /* Don't try to process rest of message. Return false so Object Unknown error IIN bit is set */
    pRxFragment->offset = pRxFragment->msgLength;
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_FILE_VAR);
    return(TMWDEFS_FALSE);
  }

  /* Length of at least 15 bytes in request was verified in sdnpo070_processRequest */ 

  /* Bytes In Object */
  tmwtarg_get16(pRxFragment->pMsgBuf + pResponse->msgLength, &bytesInObject);
  pRxFragment->offset += 2;

  /* File Handle */
  tmwtarg_get32(pRxFragment->pMsgBuf + pRxFragment->offset, &handle);
  pRxFragment->offset += 4;

  /* File Size */
  tmwtarg_get32(pRxFragment->pMsgBuf + pRxFragment->offset, &size);
  pRxFragment->offset += 4;

  /* Maximum Block Size */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &maxBlockSize);
  pRxFragment->offset += 2;

  /* Request Id */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &requestId);
  pRxFragment->offset += 2;

  /* Status */
  status = pRxFragment->pMsgBuf[pRxFragment->offset++];
  if(status != 0)
  {
    /* Log error but continue */
  }

  DNPDIAG_SHOW_FILE_CLOSE(pSession, handle, requestId);

  /* If there is not a transfer in progress */
  if(pObj70FileCtrl == TMWDEFS_NULL)
  {
    status = DNPDEFS_FILE_CMD_STAT_INV_HANDLE;
  }
  else
  {
    pObj70FileCtrl->size          = size;
    pObj70FileCtrl->maxBlockSize  = maxBlockSize;
    if(status == DNPDEFS_FILE_CMD_STAT_SUCCESS)
    {
      pObj70FileCtrl->requestId = requestId;
      if(handle == pObj70FileCtrl->handle)
      {
        
        if(pObj70FileCtrl->readResponseWaitConfirm == TMWDEFS_TRUE)
        {
          sdnpdata_confirmFileRead((TMWSESN *)pSDNPSession, pObj70FileCtrl->handle);
          pObj70FileCtrl->readResponseWaitConfirm = TMWDEFS_FALSE;
        }

        /* Close file */
        status = _closeFileRequest(pSDNPSession, handle);  
        if(status == DNPDEFS_FILE_CMD_STAT_ASYNC)
        { 
          /* This will send a NULL response, and retry close later */
          return(TMWDEFS_TRUE);
        }

        /* If the sdnpdata_closeFile FAILED, delete the control object */
        if (status != DNPDEFS_FILE_CMD_STAT_SUCCESS)
        {
          _deleteFileControl(pSDNPSession);
        }
      }
      else
      {
        status = DNPDEFS_FILE_CMD_STAT_INV_HANDLE;
      }  
    } 

    /* Failures, such as bad handle should not delete the control object.
     * master could send more reads, writes, or close
     * Failure from sdnpdata_closeFile() already deleted the file control above.
     */
    if(status == DNPDEFS_FILE_CMD_STAT_SUCCESS) 
    {
      _deleteFileControl(pSDNPSession);
    }
  }

  _buildCmdResponse(pResponse, handle, size, maxBlockSize, requestId, status, TMWDEFS_FALSE);

  return(TMWDEFS_TRUE);
}

/* function: _processAbortRequest
 * purpose: 
 * arguments:
 * returns:
 *  TMWDEFS_TRUE if processed
 *  TMWDEFS_FALSE if variation was invalid
 */
static TMWTYPES_BOOL TMWDEFS_LOCAL _processAbortRequest(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_USHORT bytesInObject;
  TMWTYPES_USHORT maxBlockSize;
  TMWTYPES_USHORT requestId;
  TMWTYPES_ULONG handle;
  TMWTYPES_ULONG size;
  DNPDEFS_FILE_CMD_STAT status;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

  if(pObjHeader->variation != 4)
  {
    /* Don't try to process rest of message. Return false so Object Unknown error IIN bit is set */
    pRxFragment->offset = pRxFragment->msgLength;
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_FILE_VAR);
    return(TMWDEFS_FALSE);
  }

  /* Length of at least 15 bytes in request was verified in sdnpo070_processRequest */ 

  /* Bytes In Object */
  tmwtarg_get16(pRxFragment->pMsgBuf + pResponse->msgLength, &bytesInObject);
  pRxFragment->offset += 2;

  /* File Handle */
  tmwtarg_get32(pRxFragment->pMsgBuf + pRxFragment->offset, &handle);
  pRxFragment->offset += 4;

  /* File Size */
  tmwtarg_get32(pRxFragment->pMsgBuf + pRxFragment->offset, &size);
  pRxFragment->offset += 4;

  /* Maximum Block Size */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &maxBlockSize);
  pRxFragment->offset += 2;

  /* Request Id */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &requestId);
  pRxFragment->offset += 2;

  /* Status */
  status = pRxFragment->pMsgBuf[pRxFragment->offset++];
  if(status != 0)
  {
    /* Log error but continue */
  }

  if(pObj70FileCtrl == TMWDEFS_NULL)
  {
    status = DNPDEFS_FILE_CMD_STAT_INV_HANDLE;
  }
  else
  {
    pObj70FileCtrl->xferComplete = TMWDEFS_TRUE;
    if((handle != 0) && (handle == pObj70FileCtrl->handle))
    {
      /* Abort file transfer */
      status = _closeFileRequest(pSDNPSession, handle);   
      if(status == DNPDEFS_FILE_CMD_STAT_ASYNC)
      { 
        /* This will send a NULL response, and retry close later */
        return(TMWDEFS_TRUE);
      }

      /* If the sdnpdata_closeFile FAILED, delete the control object */
      if (status != DNPDEFS_FILE_CMD_STAT_SUCCESS)
      {
        _deleteFileControl(pSDNPSession);
      }
    }
    else
    {
      status = DNPDEFS_FILE_CMD_STAT_INV_HANDLE; 
    }

    /* Failures, such as bad handle should not delete the control object. 
     * master might send more reads, writes, or close.
     * Failure from sdnpdata_closeFile() already deleted the file control above.
     */
    if(status == DNPDEFS_FILE_CMD_STAT_SUCCESS) 
    {
      _deleteFileControl(pSDNPSession);
    }
  }

  _buildCmdResponse(pResponse, handle, size, maxBlockSize, requestId, status, TMWDEFS_FALSE);

  return(TMWDEFS_TRUE);
}

/* function: _processDeleteRequest
 * purpose: process request to delete a file from the master
 * arguments:
 * returns:
 *  TMWDEFS_TRUE if processed
 *  TMWDEFS_FALSE if variation was invalid
 */
static TMWTYPES_BOOL TMWDEFS_LOCAL _processDeleteRequest(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl;
  TMWTYPES_CHAR filename[DNPCNFG_MAX_FILENAME+1];
  DNPDEFS_FILE_PERMISSIONS permissions;
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWTYPES_USHORT filenameOffset;
  TMWTYPES_USHORT filenameLength;
  TMWTYPES_USHORT bytesInObject;
  TMWTYPES_USHORT maxBlockSize;
  DNPDEFS_FILE_CMD_STAT status;
  TMWTYPES_USHORT requestId;
  TMWDTIME timeOfCreation;
  TMWTYPES_ULONG authKey;
  TMWTYPES_USHORT mode;
  TMWTYPES_ULONG size;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  if(pObjHeader->variation != 3)
  {
    /* Don't try to process rest of message. Return false so Object Unknown error IIN bit is set */
    pRxFragment->offset = pRxFragment->msgLength;
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_FILE_VAR);
    return(TMWDEFS_FALSE);
  }

  /* Length of at least 28 bytes in request was verified in sdnpo070_processRequest */ 

  /* Number of bytes in object */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &bytesInObject);
  pRxFragment->offset += 2;

  /* Filename Offset */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &filenameOffset);
  pRxFragment->offset += 2;

  /* Filename Size */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &filenameLength);
  pRxFragment->offset += 2;

  /* Time of Creation */
  dnpdtime_readMsSince70(&msSince70, pRxFragment->pMsgBuf + pRxFragment->offset);
  dnpdtime_msSince70ToDateTime(&timeOfCreation, &msSince70);
  pRxFragment->offset += 6;

  /* Permissions */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &permissions);
  pRxFragment->offset += 2;

  /* Authentication Key */
  tmwtarg_get32(pRxFragment->pMsgBuf + pRxFragment->offset, &authKey);
  pRxFragment->offset += 4;

  /* File Size */
  tmwtarg_get32(pRxFragment->pMsgBuf + pRxFragment->offset, &size);
  pRxFragment->offset += 4;

  /* Mode */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &mode);
  pRxFragment->offset += 2;

  /* Maximum Block Size */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &maxBlockSize);
  pRxFragment->offset += 2;

  /* Request ID */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &requestId);
  pRxFragment->offset += 2;

  /* Protect against badly formatted message */
  if((pRxFragment->offset + filenameLength) > pRxFragment->msgLength)
  {
    /* Don't try to process rest of message, set IIN and return true so another error IIN is not set */
    pRxFragment->offset = pRxFragment->msgLength;
    ((SDNPSESN*)pSession)->iin |= DNPDEFS_IIN_PARAM_ERROR;
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
    return(TMWDEFS_TRUE);
  }

  /* Make sure file name fits in array */
  if(filenameLength > DNPCNFG_MAX_FILENAME)
  { 
    /* Don't try to process rest of message, set IIN and return true so another error IIN is not set */
    pRxFragment->offset = pRxFragment->msgLength;
    ((SDNPSESN*)pSession)->iin |= DNPDEFS_IIN_PARAM_ERROR;
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_FILE_NAME);
    return(TMWDEFS_TRUE);
  }

  /* Copy filename and make sure it is null terminated */
  memcpy(filename, pRxFragment->pMsgBuf + pRxFragment->offset, filenameLength);
  pRxFragment->offset = pRxFragment->offset + filenameLength;
  filename[filenameLength] = '\0';

  pObj70FileCtrl = TMWDEFS_NULL;

  /* If the file control block is in use, just say no */
  /* When multiple file control blocks are supported this would change */
  if(pSDNPSession->pObj70FileCtrl != TMWDEFS_NULL)
  {
    status = DNPDEFS_FILE_CMD_STAT_TOO_MANY;
  }
  else
  {
    /* Try to delete file */
    status = sdnpdata_deleteFile(pSession, filename, authKey);
    if (status == DNPDEFS_FILE_CMD_STAT_SUCCESS)
    {
      _buildCmdResponse(pResponse, 0, 0, 0, requestId, status, TMWDEFS_FALSE);
      return(TMWDEFS_TRUE);
    }
    else if (status == DNPDEFS_FILE_CMD_STAT_ASYNC)
    {
      pObj70FileCtrl = _allocateFileCtrl((SDNPSESN *)pSession);
      if(pObj70FileCtrl != TMWDEFS_NULL)
      {
        memcpy(pObj70FileCtrl->filename, filename, filenameLength+1); 
        pObj70FileCtrl->authKey = authKey;

        /* SCL should send a NULL response now and later send the
         * full response as an event 
         */
        tmwtimer_start(&pObj70FileCtrl->fileTransferTimer, 
          pSDNPSession->fileTransferTimeout, pSession->pChannel, _fileTimeout, pSession);
            
        pObj70FileCtrl->requestId = requestId;
        memcpy(pObj70FileCtrl->filename, filename, filenameLength+1);
        pObj70FileCtrl->authKey = authKey;

        pObj70FileCtrl->retryState = SDNPO070_FILE_RETRY_DELETE;

        tmwtimer_start(&pObj70FileCtrl->retryTimer, 
          SDNPCNFG_OBJ70_RETRY_TIME, pSession->pChannel, _fileCmdRetryTimeout, pSession);
            
        /* This will send a NULL response */
        return(TMWDEFS_TRUE);
      }
    }  
  }

  _buildCmdResponse(pResponse, 0, 0, 0, requestId, status, TMWDEFS_FALSE);

  if(pObj70FileCtrl != TMWDEFS_NULL)
    _deleteFileControl((SDNPSESN *)pSession);

  return(TMWDEFS_TRUE);
}

#define MAX_PASSWORD_LEN 64
/* function: _processAuthenticationRequest
 * purpose: process request for authentication key from master
 *  This must respond immediately, response cannot be sent as event.
 * arguments:
 * returns:
 *  TMWDEFS_TRUE if processed
 *  TMWDEFS_FALSE if variation was invalid
 */
static TMWTYPES_BOOL TMWDEFS_LOCAL _processAuthenticationRequest(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_CHAR username[DNPCNFG_MAX_FILENAME + 1];
  TMWTYPES_CHAR password[MAX_PASSWORD_LEN + 1];
  TMWTYPES_USHORT usernameOffset;
  TMWTYPES_USHORT usernameLength;
  TMWTYPES_USHORT passwordOffset;
  TMWTYPES_USHORT passwordLength;
  TMWTYPES_USHORT bytesInObject;
  TMWTYPES_ULONG authKey;

  if (pObjHeader->variation != 2)
  {
    /* Don't try to process rest of message. Return false so Object Unknown error IIN bit is set */
    pRxFragment->offset = pRxFragment->msgLength;
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_FILE_VAR);
    return(TMWDEFS_FALSE);
  }

  /* Length of at least 14 bytes in request was verified in sdnpo070_processRequest */

  /* Number of bytes in object */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &bytesInObject);
  pRxFragment->offset += 2;

  /* Username Offset */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &usernameOffset);
  pRxFragment->offset += 2;

  /* Username Size */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &usernameLength);
  pRxFragment->offset += 2;

  /* Password Offset */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &passwordOffset);
  pRxFragment->offset += 2;

  /* Password Size */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &passwordLength);
  pRxFragment->offset += 2;

  /* Authentication Key */
  tmwtarg_get32(pRxFragment->pMsgBuf + pRxFragment->offset, &authKey);
  pRxFragment->offset += 4;

  /* Protect against badly formatted message */
  if ((pRxFragment->offset + usernameLength + passwordLength) > pRxFragment->msgLength)
  {
    /* Don't try to process rest of message, set IIN and return true so another error IIN is not set */
    pRxFragment->offset = pRxFragment->msgLength;
    ((SDNPSESN*)pSession)->iin |= DNPDEFS_IIN_PARAM_ERROR;
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
    return(TMWDEFS_TRUE);
  }

  /* Make sure these fit in arrays */
  if ((usernameLength > DNPCNFG_MAX_FILENAME)
    ||(passwordLength > MAX_PASSWORD_LEN))
  { 
    /* Don't try to process rest of message, set IIN and return true so another error IIN is not set */
    pRxFragment->offset = pRxFragment->msgLength;
    ((SDNPSESN*)pSession)->iin |= DNPDEFS_IIN_PARAM_ERROR;
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_FILE_NAME);
    return(TMWDEFS_TRUE);
  }

  /* Copy username and make sure it is null terminated */
  memcpy(username, pRxFragment->pMsgBuf + pRxFragment->offset, usernameLength);
  pRxFragment->offset = pRxFragment->offset + usernameLength;
  username[usernameLength] = '\0';

  /* Copy password and make sure it is null terminated */
  memcpy(password, pRxFragment->pMsgBuf + pRxFragment->offset, passwordLength);
  pRxFragment->offset = pRxFragment->offset + passwordLength;
  password[passwordLength] = '\0';

  /* Get authentication */
  if(!sdnpdata_getAuthentication(pSession, username, password, &authKey))
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_FILE_AUTH);
    authKey = 0;
  }

  /* Generate response */
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_OBJ_70_FILE_EVENTS;
  pResponse->pMsgBuf[pResponse->msgLength++] = 2;
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_QUAL_16BIT_FREE_FORMAT;
  pResponse->pMsgBuf[pResponse->msgLength++] = 1;
  
  DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, DNPDEFS_OBJ_70_FILE_EVENTS, 2, DNPDEFS_QUAL_16BIT_FREE_FORMAT); 

  /* Number of bytes in object */
  bytesInObject = 12;
  tmwtarg_store16(&bytesInObject, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 2;

  /* Application Layer doc says user name offset, user name size, password offset
   * and password size fields contain zeros and there are no user name octets or 
   * password octets included in the response for security reasons. 
   */

  /* Username Offset */
  usernameOffset = 0;
  tmwtarg_store16(&usernameOffset, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 2;

  /* Username Size */
  usernameLength = 0;
  tmwtarg_store16(&usernameLength, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 2;

  /* Password Offset */
  passwordOffset = 0;
  tmwtarg_store16(&passwordOffset, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 2;

  /* Password Size */
  passwordLength = 0;
  tmwtarg_store16(&passwordLength, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 2;

  /* Authentication Key */
  tmwtarg_store32(&authKey, pResponse->pMsgBuf + pResponse->msgLength);
  pResponse->msgLength += 4;

  return(TMWDEFS_TRUE);
}

/* function: _processGetInfoRequest
 * purpose: Process Get File Info Function Code from master
 *  This is for a single file and does not require an open request
 * arguments:
 * returns:
 *  TMWDEFS_TRUE if processed
 *  TMWDEFS_FALSE if variation was invalid
 */
static TMWTYPES_BOOL TMWDEFS_LOCAL _processGetInfoRequest(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_CHAR filename[DNPCNFG_MAX_FILENAME+1];
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl;
  DNPDEFS_FILE_PERMISSIONS permissions;
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWTYPES_USHORT filenameOffset;
  TMWTYPES_USHORT filenameLength;
  TMWTYPES_USHORT bytesInObject;
  DNPDEFS_FILE_CMD_STAT status;
  TMWTYPES_USHORT requestId;
  DNPDEFS_FILE_TYPE type;
  TMWTYPES_ULONG size;

  if(pObjHeader->variation != 7)
  {
    /* Don't try to process rest of message. Return false so Object Unknown error IIN bit is set */
    pRxFragment->offset = pRxFragment->msgLength;
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_FILE_VAR);
    return(TMWDEFS_FALSE);
  }

  /* Length of at least 22 bytes in request was verified in sdnpo070_processRequest */ 

  /* Number of bytes in object */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &bytesInObject);
  pRxFragment->offset += 2;

  /* Filename Offset */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &filenameOffset);
  pRxFragment->offset += 2;

  /* Filename Size */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &filenameLength);
  pRxFragment->offset += 2;

  /* Ignore type, size, timeOfCreation and permissions */
  /* File Type */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &type);
  pRxFragment->offset += 2;

  /* File Size */
  tmwtarg_get32(pRxFragment->pMsgBuf + pRxFragment->offset, &size);
  pRxFragment->offset += 4;

  /* Time of Creation */
  dnpdtime_readMsSince70(&msSince70, pRxFragment->pMsgBuf + pRxFragment->offset);
  pRxFragment->offset += 6;

  /* Permissions */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &permissions);
  pRxFragment->offset += 2;

  /* Request ID */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &requestId);
  pRxFragment->offset += 2;

  /* Protect against badly formatted message */
  if((pRxFragment->offset + filenameLength) > pRxFragment->msgLength)
  {
    /* Don't try to process rest of message, set IIN and return true so another error IIN is not set */
    pRxFragment->offset = pRxFragment->msgLength;
    ((SDNPSESN*)pSession)->iin |= DNPDEFS_IIN_PARAM_ERROR;
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
    return(TMWDEFS_TRUE);
  }

  /* Make sure file name fits in array */
  if(filenameLength > DNPCNFG_MAX_FILENAME)
  {
    /* Don't try to process rest of message, set IIN and return true so another error IIN is not set */
    pRxFragment->offset = pRxFragment->msgLength;
    ((SDNPSESN*)pSession)->iin |= DNPDEFS_IIN_PARAM_ERROR;
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_FILE_NAME);
    return(TMWDEFS_TRUE);
  }

  /* Copy filename and make sure it is null terminated */
  memcpy(filename, pRxFragment->pMsgBuf + pRxFragment->offset, filenameLength);
  pRxFragment->offset = pRxFragment->offset + filenameLength;
  filename[filenameLength] = '\0';

  pObj70FileCtrl = _allocateFileCtrl((SDNPSESN *)pSession);
  if(pObj70FileCtrl != TMWDEFS_NULL)
  {
    SDNPO070_FILE_DESCRIPTOR *pFileDesc = &pObj70FileCtrl->data.getFileInfo.fileDesc;  
    pFileDesc->nameLen = filenameLength;
    memcpy(pObj70FileCtrl->filename, filename, filenameLength +1);
 
    pObj70FileCtrl->requestId = requestId;

    /* Get File Info */
    status = sdnpdata_getFileInfo(pSession, filename, &pFileDesc->type, &pFileDesc->size, 
      &pFileDesc->fileCreationTime, &pFileDesc->permissions);

    if(status == DNPDEFS_FILE_CMD_STAT_ASYNC)
    {
      /* SCL should send a NULL response now and later send the
       * full response as an event 
       */
      tmwtimer_start(&pObj70FileCtrl->fileTransferTimer, 
        ((SDNPSESN *)pSession)->fileTransferTimeout, pSession->pChannel, _fileTimeout, pSession);
        
      pObj70FileCtrl->retryState = SDNPO070_FILE_RETRY_GETINFO;

      tmwtimer_start(&pObj70FileCtrl->retryTimer, 
        SDNPCNFG_OBJ70_RETRY_TIME, pSession->pChannel, _fileCmdRetryTimeout, pSession);
        
      /* This will send a NULL response now and respond later */
      return(TMWDEFS_TRUE);
    }
    /* if success generate obj70v7 response */
    else if(status == DNPDEFS_FILE_CMD_STAT_SUCCESS)
    {      
      pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_OBJ_70_FILE_EVENTS;
      pResponse->pMsgBuf[pResponse->msgLength++] = 7;
      pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_QUAL_16BIT_FREE_FORMAT;
      pResponse->pMsgBuf[pResponse->msgLength++] = 1;

      DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, DNPDEFS_OBJ_70_FILE_EVENTS, 7, DNPDEFS_QUAL_16BIT_FREE_FORMAT); 

      _buildGetFileInfoResponseNoHdr((SDNPSESN *)pSession, pResponse); 
      _deleteFileControl((SDNPSESN *)pSession);
      return(TMWDEFS_TRUE);
    }
  } 
  else
  {
    /* No file control block to save information in */
    status = DNPDEFS_FILE_CMD_STAT_TOO_MANY;
  }
  
  /* pResponse, handle, size, maxBlockSize, requestId, status */
  _buildCmdResponse(pResponse, 0, 0, 0, requestId, status, TMWDEFS_FALSE);
 
  if(pObj70FileCtrl != TMWDEFS_NULL)
    _deleteFileControl((SDNPSESN *)pSession);

  return(TMWDEFS_TRUE);
}

typedef TMWTYPES_BOOL (*SDNPO070_PROCESS_FUNC)(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader);

/* function: sdnpo070_processRequest */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo070_processRequest(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  SDNPO070_PROCESS_FUNC pProcessFunc; 
  SDNPSESN *pSDNPSession = (SDNPSESN*)pSession;
  TMWTYPES_UCHAR fc = pRxFragment->pMsgBuf[1];
  TMWTYPES_USHORT expectedSize;

  if(!pSDNPSession->fileTransferEnabled)
  {
    /* Don't try to process rest of message, set IIN and return true so another error IIN is not set */
    pRxFragment->offset = pRxFragment->msgLength;
    pSDNPSession->iin |= DNPDEFS_IIN_BAD_FUNCTION;
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_FILE_DISABLED);
    return(TMWDEFS_TRUE);
  }

  switch(fc)
  {
  case DNPDEFS_FC_OPEN_FILE:
    expectedSize = 28;
    pProcessFunc = _processOpenRequest;
    break;

  case DNPDEFS_FC_CLOSE_FILE:
    expectedSize = 15;
    pProcessFunc = _processCloseRequest;
    break;

  case DNPDEFS_FC_DELETE_FILE:
    expectedSize = 28;
    pProcessFunc = _processDeleteRequest;
    break;

  case DNPDEFS_FC_GET_FILE_INFO:
    expectedSize = 22;
    pProcessFunc = _processGetInfoRequest;
    break;

  case DNPDEFS_FC_AUTHENTICATE:
    expectedSize = 14;
    pProcessFunc = _processAuthenticationRequest;
    break;

  case DNPDEFS_FC_ABORT:
    expectedSize = 15;
    pProcessFunc = _processAbortRequest;
    break;

  default:
    return(TMWDEFS_FALSE);
  }

  /* Protect against badly formatted message */
  if((pRxFragment->offset + expectedSize) > pRxFragment->msgLength)
  {
    /* Don't try to process rest of message, set IIN and return true so another error IIN is not set */
    pRxFragment->offset = pRxFragment->msgLength;
    ((SDNPSESN*)pSession)->iin |= DNPDEFS_IIN_PARAM_ERROR;
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
    return(TMWDEFS_TRUE);
  }

  return(pProcessFunc(pSession, pRxFragment, pResponse, pObjHeader));
}
 
/* function: sdnpo070_readObj70v0ByClass */
/* This is a read file event request */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo070_readObj70v0ByClass(
  TMWSESN *pSession, 
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWDEFS_CLASS_MASK classMask)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession; 
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = 
    (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

  TMWTARG_UNUSED_PARAM(pRequest);
  TMWTARG_UNUSED_PARAM(pObjHeader); 

  /* If there is a control object that has an event ready to send 
   * and the classMask bit matches or this is a read of object 70
   * not an event class read continue with read.
   */
  if((pObj70FileCtrl == TMWDEFS_NULL)
    ||(pObj70FileCtrl->eventState != SDNPO070_EVENT_STAT_READY)
    ||!((pObj70FileCtrl->classMask & classMask) || (classMask == 0xff)))
  {
    /* Set this to zero, so caller knows no events were found */
    pObjHeader->numberOfPoints = 0;
    return(SDNPSESN_READ_COMPLETE);
  }

  /* Assume we will send something below */
  pObj70FileCtrl->eventState = SDNPO070_EVENT_STAT_SENT;
  switch(pObj70FileCtrl->respState)
  {
    case SDNPO070_FILE_RESP_CMD:
      /* This could be in response to Open, Close, Abort, Delete, or to generate 
       * an error response on failure
       */
      _buildCmdResponse(pResponse, pObj70FileCtrl->rspHandle, pObj70FileCtrl->size, 
       pObj70FileCtrl->maxBlockSize, pObj70FileCtrl->requestId, pObj70FileCtrl->cmdStatus, TMWDEFS_TRUE);
      break;

    case SDNPO070_FILE_RESP_READ:   
      _buildReadResponse(pSDNPSession, pResponse);
      /* Since this response was delayed and sent as an event, set the CON bit requesting an application confirm */
      pResponse->txFlags |= TMWSESN_TXFLAGS_CONTAINS_EVENTS;
      pObj70FileCtrl->readResponseWaitConfirm = TMWDEFS_TRUE;
      break;  

    case SDNPO070_FILE_RESP_READ_DIR:   
      _buildReadDirectoryResponse(pSDNPSession, pResponse);
      /* Since this response was delayed and sent as an event, set the CON bit requesting an application confirm */
      pResponse->txFlags |= TMWSESN_TXFLAGS_CONTAINS_EVENTS;
      pObj70FileCtrl->readResponseWaitConfirm = TMWDEFS_TRUE;
      break;

    case SDNPO070_FILE_RESP_GETINFO:
      pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_OBJ_70_FILE_EVENTS;
      pResponse->pMsgBuf[pResponse->msgLength++] = 7;
      pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_QUAL_16BIT_FREE_FORMAT;
      pResponse->pMsgBuf[pResponse->msgLength++] = 1;

      DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, DNPDEFS_OBJ_70_FILE_EVENTS, 7, DNPDEFS_QUAL_16BIT_FREE_FORMAT); 

      _buildGetFileInfoResponseNoHdr((SDNPSESN *)pSession, pResponse);

      /* Delayed responses are considered events */
      pResponse->txFlags |= TMWSESN_TXFLAGS_CONTAINS_EVENTS;
      break;

    case SDNPO070_FILE_RESP_TRANSPORT_STAT:
      _buildXferStatusResponse(pSDNPSession, pResponse, pObj70FileCtrl->rspHandle,
        pObj70FileCtrl->blockNumber, pObj70FileCtrl->tferStatus, TMWDEFS_TRUE);
      break;

    default:
      /* This won't happen
       * Set this back to Not Ready since there was no event to send 
       */
      pObj70FileCtrl->eventState = SDNPO070_EVENT_STAT_NOTREADY;
  } 

  return(SDNPSESN_READ_COMPLETE);
}

static DNPDEFS_FILE_TFER_STAT TMWDEFS_LOCAL _readDirectory(
  TMWSESN *pSession)
{  
  TMWTYPES_LONG          maxNameSize;
  TMWTYPES_USHORT        nameLen;
  SDNPSESN              *pSDNPSession  = (SDNPSESN *)pSession;
  DNPDEFS_FILE_TFER_STAT readDirStatus = DNPDEFS_FILE_TFER_STAT_SUCCESS;

  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = 
    (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;
  
  if(pObj70FileCtrl->size == 0)
  {
    /* open request said directory was empty, don't read it and get an error */
    pObj70FileCtrl->last = TMWDEFS_TRUE;
    return(readDirStatus);
  }

  while(readDirStatus == DNPDEFS_FILE_TFER_STAT_SUCCESS)
  {
    SDNPO070_FILE_DESCRIPTOR *pFileDesc;
    TMWTYPES_CHAR *pName;

    /* How much room left in buffer */
    maxNameSize = (TMWTYPES_USHORT)(pObj70FileCtrl->data.readDir.bytesLeft);    
    maxNameSize -=20;  /* each directory entry requires 20 bytes in addition to the name size itself */
    pFileDesc = &pObj70FileCtrl->data.readDir.fileDesc[pObj70FileCtrl->data.readDir.fileDescIndex];

    /* Reading a directory, get info on next file or subdirectory */
    pName = pFileDesc->pName;
    readDirStatus = sdnpdata_readFileInfo(pSession, 
      pObj70FileCtrl->handle,
      (TMWTYPES_USHORT)maxNameSize, 
      pName, 
      &pObj70FileCtrl->last, 
      &pFileDesc->type,
      &pFileDesc->size, 
      &pFileDesc->fileCreationTime, 
      &pFileDesc->permissions);

    pObj70FileCtrl->tferStatus = readDirStatus;
    if(readDirStatus == DNPDEFS_FILE_TFER_STAT_ASYNC)
    { 
      return(DNPDEFS_FILE_TFER_STAT_ASYNC);
    }
    else if(readDirStatus == DNPDEFS_FILE_TFER_STAT_MISC)
    {
      /* Next entry would not fit in remaining space */
      if(pObj70FileCtrl->data.readDir.fileDescIndex == 0)
      {
        /* If the first entry won't fit, return error */
        return(readDirStatus);
      }
      return(DNPDEFS_FILE_TFER_STAT_SUCCESS);
    }
    if(readDirStatus == DNPDEFS_FILE_TFER_STAT_SUCCESS)
    {
      nameLen = (TMWTYPES_USHORT)strlen(pName);
      pFileDesc->nameLen = nameLen;
      pObj70FileCtrl->data.readDir.fileDescIndex++;
      pObj70FileCtrl->data.readDir.bytesLeft = (TMWTYPES_USHORT)(pObj70FileCtrl->data.readDir.bytesLeft - nameLen -20);

      /* If this is the last entry or there is no more room
       * return DONE so that either a response will be sent or an event added
       * to indicate a response could be built.
       */
      if(pObj70FileCtrl->last 
        || (pObj70FileCtrl->data.readDir.bytesLeft < 40)
        || (pObj70FileCtrl->data.readDir.fileDescIndex >= SDNPCNFG_OBJ70_DIR_SIZE))
      {
        return(DNPDEFS_FILE_TFER_STAT_SUCCESS);
      }
        
      pFileDesc = &pObj70FileCtrl->data.readDir.fileDesc[pObj70FileCtrl->data.readDir.fileDescIndex];
      pFileDesc->pName = pName + nameLen;
    }
  }

  return(readDirStatus);
}

/* function: sdnpo070_countEvents */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpo070_countEvents(
  TMWSESN *pSession, 
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_BOOL countAll)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = 
    (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

  TMWTARG_UNUSED_PARAM(countAll);

  if((pObj70FileCtrl != TMWDEFS_NULL)
    && (pObj70FileCtrl->eventState == SDNPO070_EVENT_STAT_READY)
      && (pObj70FileCtrl->classMask & classMask))
  {
    return(1);
  }
  
  return(0);
}

/* function: sdnpo070_cleanupEvents() */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo070_cleanupEvents(
  TMWSESN *pSession, 
  TMWTYPES_BOOL deleteEvents)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  if(pSDNPSession->pObj70FileCtrl != TMWDEFS_NULL)
  {
    SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;
    
    if(pObj70FileCtrl->eventState == SDNPO070_EVENT_STAT_SENT) 
    {
      if(deleteEvents)
      {
        /* tell the database that the master confirmed the last read */
        if(pObj70FileCtrl->readResponseWaitConfirm)
        {
          sdnpdata_confirmFileRead((TMWSESN *)pSDNPSession, pObj70FileCtrl->handle);
          pObj70FileCtrl->readResponseWaitConfirm = TMWDEFS_FALSE;
        }

        pObj70FileCtrl->eventState = SDNPO070_EVENT_STAT_NOTREADY; 

        if(pObj70FileCtrl->xferComplete)
        {
          /* One last try to close... */
          if(pObj70FileCtrl->retryState == SDNPO070_FILE_RETRY_CLOSE)
          {
            _closeFile((TMWSESN *)pSDNPSession, pObj70FileCtrl->handle);
          }
          
          _deleteFileControl(pSDNPSession);

          /* return whether or not the event queue is full */
          return(TMWDEFS_FALSE);
        }
      }
      else
      {
        /* If this was a read or write response, decrement block number so it will 
         * resend correct block number 
         */
        if((pObj70FileCtrl->blockNumber & 0x7fffffff)!= 0)
        {
          if(pObj70FileCtrl->eventState == SDNPO070_EVENT_STAT_SENT)
          {
            pObj70FileCtrl->blockNumber--;
          }
        }

        /* set state so this event will be resent */
        pObj70FileCtrl->eventState = SDNPO070_EVENT_STAT_READY;
      }
    }

    /* return whether or not the event queue is full */
    if(pObj70FileCtrl->eventState == SDNPO070_EVENT_STAT_READY) 
      return(TMWDEFS_TRUE);
  }

  /* return whether or not the event queue is full */
  return(TMWDEFS_FALSE);
}

/* function: sdnpo070_readObj70 
 * This is a read simple file or read directory request 
 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo070_readObj70(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  SDNPSESN_QUAL qualifier)
{
  TMWTYPES_USHORT bytesInObject;
  TMWTYPES_ULONG handle;
  TMWTYPES_ULONG block;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = 
    (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;
  DNPDEFS_FILE_TFER_STAT status = DNPDEFS_FILE_TFER_STAT_SUCCESS;
 
  if(pObjHeader->variation<4 || pObjHeader->variation>7)
  {
    return(SDNPSESN_READ_FAILED);
  }
  if(pObjHeader->variation == 5)
  {
    /* Read of variation 5 should have variation 5 data in it. It is not a "normal" read */ 
    TMWTYPES_BOOL readSameBlock = TMWDEFS_FALSE;

    /* Protect against badly formatted message */
    if((pRxFragment->offset + 10) > pRxFragment->msgLength)
    {
      /* Don't try to process rest of message */
      pRxFragment->offset = pRxFragment->msgLength;
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
      return(SDNPSESN_READ_FAILED);
    }

    /* Number of bytes in object */
    tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &bytesInObject);
    pRxFragment->offset += 2;

    /* Handle */
    tmwtarg_get32(pRxFragment->pMsgBuf + pRxFragment->offset, &handle);
    pRxFragment->offset += 4;

    /* Block */
    tmwtarg_get32(pRxFragment->pMsgBuf + pRxFragment->offset, &block);
    pRxFragment->offset += 4;

    if(qualifier != SDNPSESN_QUAL_BUILD_RESPONSE)
      return(SDNPSESN_READ_COMPLETE);

    /* If there is not a transfer in progress, try to allocate a control block 
     * for sending the response
     */
    if(pObj70FileCtrl == TMWDEFS_NULL)
    { 
      status = DNPDEFS_FILE_TFER_STAT_NOT_OPEN;
    }
    else 
    {
      pObj70FileCtrl->rspHandle = handle;

      /* Make sure handle matches */ 
      if(handle == pObj70FileCtrl->handle)
      {
        /* Restart file transfer timer */
        tmwtimer_start(&pObj70FileCtrl->fileTransferTimer, 
          pSDNPSession->fileTransferTimeout, pSession->pChannel, _fileTimeout, pSession);

        if((block != (pObj70FileCtrl->blockNumber & 0x7fffffff))
          && (block != pObj70FileCtrl->lastBlockRead))
        {  
          status = DNPDEFS_FILE_TFER_STAT_BAD_BLOCK;
        }
        else
        {
          if(block == pObj70FileCtrl->lastBlockRead)
          {
            /* set this so it will send the same data as before without rereading database */
            readSameBlock = TMWDEFS_TRUE;
            pObj70FileCtrl->blockNumber = block;
          }
          else if(pObj70FileCtrl->readResponseWaitConfirm == TMWDEFS_TRUE)
          {
            sdnpdata_confirmFileRead((TMWSESN *)pSDNPSession, pObj70FileCtrl->handle);
            pObj70FileCtrl->readResponseWaitConfirm = TMWDEFS_FALSE;
          }
        }
      }
      else
      {
        pObj70FileCtrl->blockNumber = block;
        status = DNPDEFS_FILE_TFER_STAT_INV_HANDLE;
      }

      /* If handle and block number were correct */
      if(status == DNPDEFS_FILE_TFER_STAT_SUCCESS)
      {
        /* If we are reading a directory */
        if(pObj70FileCtrl->type == DNPDEFS_FILE_TYPE_DIRECTORY)
        {
          pObj70FileCtrl->data.readDir.bytesLeft = pObj70FileCtrl->maxBlockSize; 
          pObj70FileCtrl->data.readDir.fileDescIndex = 0;
          pObj70FileCtrl->data.readDir.fileDesc[0].pName = pObj70FileCtrl->data.readDir.fileNameBuffer;

          status = _readDirectory(pSession);
          if(status == DNPDEFS_FILE_TFER_STAT_SUCCESS)
          {
            /* if success, build the response */  
            _buildReadDirectoryResponse(pSDNPSession, pResponse);
            if(pObj70FileCtrl->eventState == SDNPO070_EVENT_STAT_READY)
            {
              /* Since this response was sent previously as an event, continue to treat it as an event
               * set the CON bit requesting an application confirm 
               */
              pResponse->txFlags |= TMWSESN_TXFLAGS_CONTAINS_EVENTS;
              pObj70FileCtrl->eventState = SDNPO070_EVENT_STAT_SENT;
            }
            pObj70FileCtrl->readResponseWaitConfirm = TMWDEFS_TRUE;
            pObj70FileCtrl->lastBlockRead = block;
            return(SDNPSESN_READ_COMPLETE);
          }
          else if(status == DNPDEFS_FILE_TFER_STAT_ASYNC)
          { 
            /* This will send a NULL response now and later send the 
             * full response as an event
             */
            pObj70FileCtrl->retryState = SDNPO070_FILE_RETRY_READ_DIR;

            tmwtimer_start(&pObj70FileCtrl->retryTimer, 
              SDNPCNFG_OBJ70_RETRY_TIME, pSession->pChannel, _fileCmdRetryTimeout, pSession);
           
            return(SDNPSESN_READ_COMPLETE);
          }
        }
        else
        {
          /* If this is a reread of the same block that was last sent to the master
           * just use the old data 
           */
          if(!readSameBlock)
          {
#if SDNPDATA_SUPPORT_XML2 && SDNPDATA_SUPPORT_READ_XML
            if(pObj70FileCtrl->xml2DeviceProfileOpen)
            {
              status = sdnpxml2_readDeviceProfile(pSession, handle, &pObj70FileCtrl->last,
                &pObj70FileCtrl->data.read.bytesRead, pObj70FileCtrl->data.read.buffer);
            }
            else
#endif
            /* Simple file read data, into file control buffer, since they may have 
             * to be resent if master does not confirm data 
             */
            status = sdnpdata_readFile(pSession, handle, &pObj70FileCtrl->last,
              &pObj70FileCtrl->data.read.bytesRead, pObj70FileCtrl->data.read.buffer);
          }
         
          /* Check return status */
          if(status == DNPDEFS_FILE_TFER_STAT_SUCCESS)
          {
            pObj70FileCtrl->lastBlockRead = block;
            _buildReadResponse(pSDNPSession, pResponse);
            if(pObj70FileCtrl->eventState == SDNPO070_EVENT_STAT_READY)
            {
              /* Since this response was sent previously as an event, continue to treat it as an event
               * set the CON bit requesting an application confirm 
               */
              pResponse->txFlags |= TMWSESN_TXFLAGS_CONTAINS_EVENTS;
              pObj70FileCtrl->eventState = SDNPO070_EVENT_STAT_SENT;
            }

            pObj70FileCtrl->readResponseWaitConfirm = TMWDEFS_TRUE; 
            return(SDNPSESN_READ_COMPLETE);
          }
          else if(status == DNPDEFS_FILE_CMD_STAT_ASYNC)
          { 
            /* SCL should send a NULL response now and later send the
             * full response as an event 
             */
            pObj70FileCtrl->retryState = SDNPO070_FILE_RETRY_READ;

            tmwtimer_start(&pObj70FileCtrl->retryTimer, 
              SDNPCNFG_OBJ70_RETRY_TIME, pSession->pChannel, _fileCmdRetryTimeout, pSession);
           
            /* This will send a NULL response */
            return(SDNPSESN_READ_COMPLETE);
          }
        }
      }
    
       /* 4.17.1.3 Application Layer Doc says close file on error */
      _errorCloseFile(pSession, pObj70FileCtrl->handle);  
    }
    
    _buildXferStatusResponse(pSDNPSession, pResponse, handle, block, status, TMWDEFS_FALSE);
    
    if(pObj70FileCtrl != TMWDEFS_NULL)
    {
      _deleteFileControl(pSDNPSession);
    }

    return(SDNPSESN_READ_COMPLETE);
  }
  else 
  {
    if(qualifier != SDNPSESN_QUAL_BUILD_RESPONSE)
    {
      return(SDNPSESN_READ_COMPLETE);
    }

    /* If there is a control object that has an event ready to send */
    if((pObj70FileCtrl == TMWDEFS_NULL)
      ||(pObj70FileCtrl->eventState != SDNPO070_EVENT_STAT_READY))
    {
      return(SDNPSESN_READ_COMPLETE);
    }
   
    if(pObjHeader->variation == 4)
    { 
      if(pObj70FileCtrl->respState == SDNPO070_FILE_RESP_CMD)
      {
        /* This could be in response to Open, Close, Abort, Delete, or to generate 
         * an error response on failure
         */
        pObj70FileCtrl->eventState = SDNPO070_EVENT_STAT_SENT;
        _buildCmdResponse(pResponse, pObj70FileCtrl->rspHandle, pObj70FileCtrl->size, 
          pObj70FileCtrl->maxBlockSize, pObj70FileCtrl->requestId, pObj70FileCtrl->cmdStatus, TMWDEFS_TRUE);
      }
      return(SDNPSESN_READ_COMPLETE);
    }
    else if(pObjHeader->variation == 6)
    {   
      if(pObj70FileCtrl->respState == SDNPO070_FILE_RESP_TRANSPORT_STAT)
      {
        _buildXferStatusResponse(pSDNPSession, pResponse, pObj70FileCtrl->rspHandle,
          pObj70FileCtrl->blockNumber, pObj70FileCtrl->tferStatus, TMWDEFS_TRUE);
      }
      return(SDNPSESN_READ_COMPLETE);
    } 
    else if(pObjHeader->variation == 7)
    { 
      if(pObj70FileCtrl->respState ==SDNPO070_FILE_RESP_GETINFO)
      {
        pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_OBJ_70_FILE_EVENTS;
        pResponse->pMsgBuf[pResponse->msgLength++] = 7;
        pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_QUAL_16BIT_FREE_FORMAT;
        pResponse->pMsgBuf[pResponse->msgLength++] = 1;

        DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, DNPDEFS_OBJ_70_FILE_EVENTS, 7, DNPDEFS_QUAL_16BIT_FREE_FORMAT); 

        _buildGetFileInfoResponseNoHdr((SDNPSESN *)pSession, pResponse);

        /* Delayed responses are considered events */
        pResponse->txFlags |= TMWSESN_TXFLAGS_CONTAINS_EVENTS;
      }
      return(SDNPSESN_READ_COMPLETE);
    }
  }
  return(SDNPSESN_READ_FAILED);
}

/* function: sdnpo070_writeObj70 */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo070_writeObj70(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  DNPDEFS_FILE_TFER_STAT status;
  TMWTYPES_USHORT bytesInObject;
  TMWTYPES_ULONG blockNumber;
  TMWTYPES_USHORT numBytes;
  TMWTYPES_UCHAR *pData;
  TMWTYPES_ULONG handle;
  TMWTYPES_ULONG block;
  TMWTYPES_BOOL last;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = 
    (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

  TMWTARG_UNUSED_PARAM(pObjHeader);

  /* Length of at least 10 bytes in request was verified in sdnpsesn _validateMessageSize */ 

  /* Number of bytes in object */
  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &bytesInObject);
  pRxFragment->offset += 2;

  /* Handle */
  tmwtarg_get32(pRxFragment->pMsgBuf + pRxFragment->offset, &handle);
  pRxFragment->offset += 4;

  /* Block */
  tmwtarg_get32(pRxFragment->pMsgBuf + pRxFragment->offset, &block);
  pRxFragment->offset += 4;

  blockNumber = block & 0x7fffffffL;
  last = (TMWTYPES_BOOL)(((block & 0x80000000L) != 0) ? TMWDEFS_TRUE : TMWDEFS_FALSE);

  /* Data */
  pData = pRxFragment->pMsgBuf + pRxFragment->offset;
  numBytes = (TMWTYPES_USHORT)(bytesInObject - 8);

  status = DNPDEFS_FILE_TFER_STAT_SUCCESS;

  DNPDIAG_SHOW_FILE_DATA(pSession, handle, blockNumber, numBytes, last, TMWDEFS_FALSE, TMWDIAG_ID_RX);
  
  /* make sure the the number of bytes does not exceed received message size */
  if ((pRxFragment->offset + numBytes) <= pRxFragment->msgLength)
  {
    pRxFragment->offset = pRxFragment->offset + numBytes;
  }
  else
  {
    /* Don't try to process rest of message */
    pRxFragment->offset = pRxFragment->msgLength;
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
    status = DNPDEFS_FILE_TFER_STAT_MISC;
  }

  if(pObj70FileCtrl == TMWDEFS_NULL)
  {
    status = DNPDEFS_FILE_TFER_STAT_NOT_OPEN;
    _buildXferStatusResponse(pSDNPSession, pResponse, handle, block, status, TMWDEFS_FALSE); 
    return(TMWDEFS_TRUE);
  }
  else
  {
    pObj70FileCtrl->rspHandle = handle;
   
   /* Make sure handle is the expected handle */ 
    if((handle != 0) && (handle == pObj70FileCtrl->handle))
    {
      /* Normally just allow write to sequential block numbers 
       * However, be tolerant of master retrying last write since it
       * might not have seen the response, or we might not have seen the appl confirm
       */
      if((blockNumber == pObj70FileCtrl->blockNumber)
        ||(blockNumber == pObj70FileCtrl->lastBlockWritten))
      {
        /* Restart file transfer timer */
        tmwtimer_start(&pObj70FileCtrl->fileTransferTimer, 
          pSDNPSession->fileTransferTimeout, pSession->pChannel, _fileTimeout, pSession);
      }
      else
      {
        status = DNPDEFS_FILE_TFER_STAT_BAD_BLOCK;
      }   

      /* Make sure it is not longer than agreed upon */
      if (numBytes > pObj70FileCtrl->maxBlockSize)
      {
        status = DNPDEFS_FILE_TFER_STAT_OVERRUN;
      }
    }
    else
    {
      status = DNPDEFS_FILE_TFER_STAT_INV_HANDLE;
    }

    if(status == DNPDEFS_FILE_TFER_STAT_SUCCESS)
    {
      /* If this is a retry of a block that was already written to Database
       * don't write it again
       */
      if(pObj70FileCtrl->lastBlockWritten != block)
        /* Write Data */
        status = sdnpdata_writeFile(pSession, handle, last, numBytes, pData);

      if(status == DNPDEFS_FILE_TFER_STAT_ASYNC)
      { 
        /* SCL should send a NULL response now and later send the
         * full response as an event 
         */
        pObj70FileCtrl->last = last;
        pObj70FileCtrl->data.write.numBytes = numBytes;
        memcpy(pObj70FileCtrl->data.write.buffer, pData, numBytes);

        pObj70FileCtrl->retryState = SDNPO070_FILE_RETRY_WRITE;

        tmwtimer_start(&pObj70FileCtrl->retryTimer, 
          SDNPCNFG_OBJ70_RETRY_TIME, pSession->pChannel, _fileCmdRetryTimeout, pSession);
      
        /* This will send a NULL response now and respond later */
        return(TMWDEFS_TRUE);
      }
    }

    /* 4.17.1.3 Application Layer Doc says close file on error */
    if(status != DNPDEFS_FILE_TFER_STAT_SUCCESS)
    {
      _errorCloseFile(pSession, pObj70FileCtrl->handle);
 
      /* This is an immediate response, application confirm is not required. */
      _buildXferStatusResponse(pSDNPSession, pResponse, handle, block, status, TMWDEFS_FALSE); 
 
      _deleteFileControl(pSDNPSession);
      return(TMWDEFS_TRUE);
    }
  }
    
  /* This is an immediate response, application confirm is not required. */
  pObj70FileCtrl->lastBlockWritten = block;
  pObj70FileCtrl->blockNumber = block;
  pObj70FileCtrl->eventState = SDNPO070_EVENT_STAT_NOTREADY;
  _buildXferStatusResponse(pSDNPSession, pResponse, handle, block, status, TMWDEFS_FALSE); 

  return(TMWDEFS_TRUE);
}

#if SDNPDATA_SUPPORT_ASSIGN
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo070_assignClass(
  TMWSESN *pSession, 
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader, 
  TMWDEFS_CLASS_MASK classMask)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTARG_UNUSED_PARAM(pRxFragment);
  TMWTARG_UNUSED_PARAM(pObjHeader);

  if(pObjHeader->qualifier != DNPDEFS_QUAL_ALL_POINTS)
    return(TMWDEFS_FALSE);

  return(sdnpdata_fileAssignClass(pSDNPSession->pDbHandle, classMask));   
}
#endif

/* function: sdnpo070_fileCmdComplete */
void TMWDEFS_GLOBAL sdnpo070_fileCmdComplete(
  TMWSESN *pSession,
  TMWTYPES_ULONG handle)
{
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
#if TMWCNFG_SUPPORT_THREADS
  TMWDEFS_RESOURCE_LOCK *pLock = &pSession->pChannel->lock;
#endif

  /* Since only one file transfer per session is currently supported
   * ignore the handle 
   */
  TMWTARG_UNUSED_PARAM(handle);

  TMWTARG_LOCK_SECTION(pLock);
  pObj70FileCtrl = (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

  /* cancel the retry timer, since database says command is complete */
  if(pObj70FileCtrl != TMWDEFS_NULL)
  {
    tmwtimer_cancel((TMWTIMER *)&pObj70FileCtrl->retryTimer);
  }

  /* User has told us the command is complete, call 
   * the same function we call when the retry timer expires.
   */
  _fileCmdRetryTimeout(pSession);

  TMWTARG_UNLOCK_SECTION(pLock);
}

#endif /* SDNPDATA_SUPPORT_OBJ70 */
