/*****************************************************************************/
/* Triangle MicroWorks, Inc.                         Copyright (c) 1997-2006 */
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

// DNPMultithreadPeer.cpp :  Sample multithreaded DNP Master and Outstation console application.
//
/* DNPMultithreadPeer.cpp : Sample multithreaded DNP console application. 
 * This example shows Master and Outstation sessions on separate channels.
 * The SCL also supports a master and a outstation session on the same channel.
 * This sample will use a thread per channel each on a separate application context.
 * The define TMWCNFG_MULTIPLE_TIMER_QS will compile in the timer queue per channel
 * code in the SCL so that timers on each channel can be called from a separate context.
 * This will allow requests and received data for each channel to be processed on a separate
 * thread as well as timers for each channel to run on separate threads.
 * WinIoTarg and LinIoTarg contains code to use separate system timers for each
 * channel.If you are using another Operating System may want to implement
 * tmwtarg_initMultiTimer, tmwtarg_setMultiTimer, and tmwtarg_deleteMultiTimer
 * in addition to the other tmwtarg_xxx functionality.
 */

#if defined(TMW_WTK_TARGET)
#include "StdAfx.h"
#endif

extern "C" {
#include "tmwscl/utils/tmwappl.h"
#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwphys.h"
#include "tmwscl/utils/tmwpltmr.h"
#include "tmwscl/utils/tmwtarg.h"

#include "tmwscl/dnp/dnpchnl.h"
#include "tmwscl/dnp/dnplink.h"
#include "tmwscl/dnp/mdnpbrm.h"
#include "tmwscl/dnp/mdnpsesn.h"
#include "tmwscl/dnp/dnpdiag.h"

#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnputil.h"
#include "tmwscl/dnp/sdnpo002.h"
#include "tmwscl/dnp/sdnpo022.h"

#include "tmwtargio.h"
}

/* The USE_POLLED_MODE constant is used here to demonstrate how the library
 * can be used to configure the target layer to support polled mode vs.
 * event driven. The Linux and Windows TCP channels shipped with the SCL
 * support both. If porting the library, only one configuration is required.
 */
#define USE_POLLED_MODE TMWDEFS_FALSE
#define USE_TLS         TMWDEFS_FALSE

/* If Multiple Timer Queues is configured, use this file */
#if TMWCNFG_MULTIPLE_TIMER_QS

/* Number of channels and therefore channel threads */
/* Separate channel for master and slave == 2 channels */
#define NUMBER_CHANNELS 6  /* Make this an even number */
/* Number of sessions per channel */
#define NUMBER_SESSIONS 1

/* Sample application configuration */
#define DEFAULT_RESPONSE_TIMEOUT     9000
#define INTEGRITY_INTERVAL          20000
#define EVENT_POLL_INTERVAL         10000   
#define BINCMD_INTERVAL             30000  

#define EVENT_INTERVAL               5000 

/* Simple application structure for controlling generation of requests */
typedef struct
{
  TMWTYPES_MILLISECONDS  integrityInterval;
  TMWTYPES_MILLISECONDS  lastIntegrity;

  TMWTYPES_MILLISECONDS  eventInterval;
  TMWTYPES_MILLISECONDS  lastEventPoll;    

  TMWTYPES_MILLISECONDS  binCmdInterval;
  TMWTYPES_MILLISECONDS  lastBinCmd;     
} MY_MASTER_REQUEST_INFO;


/* Simple application structure for controlling generation of events */
typedef struct
{
  TMWTYPES_MILLISECONDS  eventInterval;
  TMWTYPES_MILLISECONDS  lastEvent;    
} MY_SLAVE_REQUEST_INFO;

/* Statistics */
int successCount;
int intermediateCount;
int timeoutCount;
int otherFailureCount;
int totalCount;
int unsolCount;

MY_MASTER_REQUEST_INFO *myInitMasterRequests(void);
MY_SLAVE_REQUEST_INFO  *myInitSlaveRequests(void);
static void             myStartThread(TMWTYPES_BOOL isMaster, int i);
static TMWTYPES_BOOL    myTimeToSendRequest(TMWTYPES_MILLISECONDS *lastTime, TMWTYPES_MILLISECONDS interval); 
void myBrmCallbackFcn(void *errorStatus, DNPCHNL_RESPONSE_INFO *pResponse);
void myUnsolCallbackFcn(void *param, MDNPSESN_UNSOL_RESP_INFO *pResponse);

/* Diagnostics */
static void             myLogOutput(char *pBuf);
static void             myPutDiagString(const TMWDIAG_ANLZ_ID *pAnlzId, const TMWTYPES_CHAR *pString);

static TMW_ThreadDecl   myChannelThreadMDNP(void *pParam);
static TMW_ThreadDecl   myChannelThreadSDNP(void *pParam);


/*
 * Begin the main loop
 */
int main(int argc, char* argv[])
{
  TMWTYPES_USHORT i;
  char logBuf[256];
  TMWTARG_UNUSED_PARAM(argc);
  TMWTARG_UNUSED_PARAM(argv);

#if TMWCNFG_SUPPORT_DIAG 
  /* Register function to display diagnostic strings to console 
   * This is only necessary if using the Windows or Linux target layer.
   * If implementing a new target layer, tmwtarg_putDiagString()
   * should be modified if diagnostic output is desired.
   */
  tmwtargp_registerPutDiagStringFunc(myPutDiagString);
#endif

  /*
  * Initialize the source code library.
  */
  tmwappl_initSCL();

  /*
   * Initialize async database used by master.
   */
  tmwdb_init(10000);

  /* Start all of the channel threads */
  for(i=0; i<NUMBER_CHANNELS; i++)
  {
    sprintf(logBuf, "starting master DNP thread %d\n", i);
    myLogOutput(logBuf);
     
    myStartThread(TMWDEFS_TRUE, i);

    i++;
    sprintf(logBuf, "starting slave DNP thread %d\n", i);
    myLogOutput(logBuf);
    
    myStartThread(TMWDEFS_FALSE, i);
  }

  /* Loop forever checking async database queue
   */
  while(1)
  {
    /* Store any data on the data base queue.*/
     while (tmwdb_storeEntry(TMWDEFS_TRUE))
       ;

    /* sleep for 50 milliseconds */
    tmwtarg_sleep(50);
  }

  return(0);
}

TMW_ThreadDecl myChannelThreadMDNP(void *pParam)
{
  int i;
  TMWTYPES_BOOL run;
  int numberIntegritiesSent;
  TMWAPPL *pThreadApplContext;
  MY_MASTER_REQUEST_INFO *pMyRequests;
  TMWCHNL *pSclChannel;  
  MDNPSESN_CONFIG sesnConfig;
  TMWSESN *pSessions[NUMBER_SESSIONS];
  MDNPBRM_REQ_DESC requests[NUMBER_SESSIONS];
  MDNPBRM_CROB_INFO CROBInfo[NUMBER_SESSIONS];

  /* Configuration */
  TMWTARGIO_CONFIG    IOCnfg;
  DNPCHNL_CONFIG chnlConfig;
  DNPTPRT_CONFIG prtConfig;
  DNPLINK_CONFIG lnkConfig;
  TMWPHYS_CONFIG physConfig;
  TMWTARG_CONFIG targConfig;
  char buf[256];
  TMWTYPES_ULONG pollInterval = 1000;

  long channelNumber = (long)pParam;

  /* Create separate application context per channel thread 
   * This is not required, all channels can exist on a single application context.
   */
  pThreadApplContext = tmwappl_initApplication();

  pMyRequests = myInitMasterRequests();

  run = TMWDEFS_TRUE;

  numberIntegritiesSent = 0;

  /* Initialize the Target config Structure
   * Call tmwtarg_initConfig to initialize with default values,
   * then overwrite values as needed.
   */
  tmwtarg_initConfig(&targConfig);
  
  /* Initialize the Channel config Structure
   * Call dnpchnl_initConfig to initialize with default values,
   * then overwrite values as needed.
   */
  dnpchnl_initConfig(&chnlConfig, &prtConfig, &lnkConfig, &physConfig);
  lnkConfig.networkType = DNPLINK_NETWORK_TCP_UDP;

  /* Initialize the common target IO configuration structure.
  * Call tmwtargio_initConfig to initialize default values, then overwrite
  * specific values as needed.
  */
  tmwtargio_initConfig(&IOCnfg);
  IOCnfg.type = TMWTARGIO_TYPE_TCP;
  
  /* name displayed in analyzer window */ 
  sprintf(IOCnfg.targTCP.chnlName, "DNPmaster%lu", channelNumber);

  /* TCP/IP address of remote device  
   * 127.0.0.1 is the loopback address and will attempt to connect to an outstation listener on this computer
   */
  strcpy(IOCnfg.targTCP.ipAddress, "127.0.0.1");

  /* Port on outstation to connect to */
  IOCnfg.targTCP.ipPort = (TMWTYPES_USHORT)(20000 + channelNumber);
  IOCnfg.targTCP.polledMode = USE_POLLED_MODE;

  IOCnfg.targTCP.mode = TMWTARGTCP_MODE_CLIENT;
  IOCnfg.targTCP.role = TMWTARGTCP_ROLE_MASTER;

  /* TLS Configuration */
  IOCnfg.targTCP.useTLS = USE_TLS;

  /* Only one channel can bind to the local UDP port */
  IOCnfg.targTCP.localUDPPort = (TMWTYPES_USHORT)(20000 + NUMBER_CHANNELS + channelNumber); /* TMWTARG_UDP_PORT_NONE */

  /* Broadcast address to use for sending broadcast requests over UDP */
  strcpy(IOCnfg.targTCP.udpBroadcastAddress, "192.168.1.255");

  /*
   * Open the Channel
   */
  sprintf(buf, "opening DNP channel %lu\n", channelNumber);
  myLogOutput(buf);
  
  pSclChannel = dnpchnl_openChannel(pThreadApplContext, &chnlConfig, &prtConfig, 
    &lnkConfig, &physConfig, &IOCnfg, &targConfig);

  if(pSclChannel == TMWDEFS_NULL)
  {
    /* Failed to open */
    printf("Failed to open master channel, exiting program \n");
    
    /* Sleep for 10 seconds before exiting */
    tmwtarg_sleep(10000);
    return (0);
  }

  /*
   * Initialize and open the Sessions
   */
  mdnpsesn_initConfig(&sesnConfig);
  sesnConfig.autoRequestMask &= ~MDNPSESN_AUTO_EVENT_POLL;
  sesnConfig.autoRequestMask |= MDNPSESN_AUTO_ENABLE_UNSOL;
  for (i = 0; i < NUMBER_SESSIONS; i++)
  {
    sesnConfig.destination = (unsigned short)(4 + i);
    sesnConfig.source = (unsigned short)(3 + i);
    sesnConfig.defaultResponseTimeout = DEFAULT_RESPONSE_TIMEOUT;
    sprintf(buf, "opening DNP session %d\n", i);
    myLogOutput(buf);
    
    pSessions[i] = mdnpsesn_openSession(pSclChannel, &sesnConfig, TMWDEFS_NULL);
    if(pSessions[i] == TMWDEFS_NULL)
    {
      /* Failed to open */
      printf("Failed to open master session, exiting program \n");
    
      /* Sleep for 10 seconds before exiting */
      tmwtarg_sleep(10000);
      return (0);
    }
    
    mdnpsesn_setUnsolUserCallback(pSessions[i], myUnsolCallbackFcn, TMWDEFS_NULL);
  }

  /*
   * Initialize Request Descriptors
   */
  for (i = 0; i < NUMBER_SESSIONS; i++)
  {
    mdnpbrm_initReqDesc(&requests[i], pSessions[i]);
    requests[i].pUserCallback = myBrmCallbackFcn;
    requests[i].pUserCallbackParam = TMWDEFS_NULL;
  }

  if (USE_POLLED_MODE)
  {
    /* If running in polled mode, tmwappl_checkForInput needs to be called every 50ms. */
    pollInterval = 50;
  }

  /* Now that everything is set up, start a "main loop"
   * that sends and processes requests.
   */

  while (run) 
  {
    /*
     * Process any data returned by the outstation for the channel on this application context.
     */
    /* If the target layer is configured to be event driven, there
     * is no need for the application to check for received data.
     */
    if (USE_POLLED_MODE)
    {
      tmwappl_checkForInput(pThreadApplContext);
    }

    /* See if it's time to send a request.
     * This simple demo only issues the following requests:
     *  - Integrity Poll
     *  - Event Poll
     */
    if(myTimeToSendRequest(&pMyRequests->lastIntegrity, pMyRequests->integrityInterval))
    {
      for (int i = 0; i < NUMBER_SESSIONS; i++)
      {
        sprintf(buf, "Integrity Poll for Channel %lu Session %d\n", channelNumber, i);
        myLogOutput(buf);
 
        mdnpbrm_integrityPoll(&requests[i]);
        numberIntegritiesSent++;
      }
    }
  
    if(myTimeToSendRequest(&pMyRequests->lastEventPoll, pMyRequests->eventInterval))
    {
      for (int i = 0; i < NUMBER_SESSIONS; i++)
      {
        sprintf(buf, "Event Poll for Channel %lu Session %d\n", channelNumber, i);
        myLogOutput(buf);
 
        mdnpbrm_eventPoll(&requests[i]);
      }
    }
    if(myTimeToSendRequest(&pMyRequests->lastBinCmd, pMyRequests->binCmdInterval))
    {
      for (int i = 0; i < NUMBER_SESSIONS; i++)
      {
        sprintf(buf, "Issue Binary Output Command Channel %lu Session %d\n", channelNumber, i);
        myLogOutput(buf);
      
        CROBInfo[i].pointNumber = 0;
        CROBInfo[i].control = (TMWTYPES_UCHAR) (CROBInfo[i].control != 
          DNPDEFS_CROB_CTRL_LATCH_ON ? DNPDEFS_CROB_CTRL_LATCH_ON : DNPDEFS_CROB_CTRL_LATCH_OFF);
        CROBInfo[i].onTime = 0;
        CROBInfo[i].offTime = 0;

    	  mdnpbrm_binaryCommand(&requests[i], TMWDEFS_NULL, DNPDEFS_FC_SELECT,
            MDNPBRM_AUTO_MODE_OPERATE, 0, DNPDEFS_QUAL_16BIT_INDEX, 1, &CROBInfo[0]);
      }
    }
    tmwtarg_sleep(pollInterval);
  }

  for (i = 0; i < NUMBER_SESSIONS; i++)
  {
    sprintf(buf, "closing DNP session %d\n", i);
    myLogOutput(buf);
    
    mdnpsesn_closeSession(pSessions[i]);
  }

  sprintf(buf, "closing DNP channel %lu\n", channelNumber);
  myLogOutput(buf);
  
  dnpchnl_closeChannel(pSclChannel);
  delete pMyRequests;
  tmwappl_closeApplication(pThreadApplContext, TMWDEFS_FALSE);

  return (0);
}

TMW_ThreadDecl myChannelThreadSDNP(void *pParam)
{
  int  i;
  TMWAPPL *pThreadApplContext;
  MY_SLAVE_REQUEST_INFO *pMyRequests;
  TMWCHNL *pSclChannel;
  TMWSESN *pSclSession[NUMBER_SESSIONS];

  /* Configuration */
  TMWTARGIO_CONFIG    IOCnfg;
  TMWPHYS_CONFIG  physConfig;
  TMWTARG_CONFIG  targConfig;
  DNPCHNL_CONFIG  DNPConfig;
  DNPLINK_CONFIG  linkConfig;
  DNPTPRT_CONFIG  tprtConfig;
  SDNPSESN_CONFIG sesnConfig;
  char logBuf[256];
  TMWTYPES_ULONG pollInterval = 1000;

  long channelNumber = (long)pParam;

  /* Create separate application context per channel thread 
   * This is not required, all channels can exist on a single application context.
   */
  pThreadApplContext = tmwappl_initApplication();

  pMyRequests = myInitSlaveRequests();

  /*
   * Initialize all configuration structures to defaults
   */
  tmwtarg_initConfig(&targConfig);
  
  dnpchnl_initConfig(&DNPConfig, &tprtConfig, &linkConfig, &physConfig);
  linkConfig.networkType = DNPLINK_NETWORK_TCP_UDP;

  /* Initialize the common target IO configuration structure.
  * Call tmwtargio_initConfig to initialize default values, then overwrite
  * specific values as needed.
  */
  tmwtargio_initConfig(&IOCnfg);
  IOCnfg.type = TMWTARGIO_TYPE_TCP;

  /* name displayed in analyzer window */
  sprintf(IOCnfg.targTCP.chnlName, "DNPslave%lu", channelNumber);

  /* IP address to accept connections from */
  /* *.*.*.* allows any client to connect */
  strcpy(IOCnfg.targTCP.ipAddress, "*.*.*.*");

  /* Channel number is one more than master channel, so subtract 1*/
  IOCnfg.targTCP.ipPort = (TMWTYPES_USHORT)(20000 + channelNumber -1);
  IOCnfg.targTCP.polledMode = USE_POLLED_MODE;
  IOCnfg.targTCP.mode = TMWTARGTCP_MODE_SERVER;
  IOCnfg.targTCP.role = TMWTARGTCP_ROLE_OUTSTATION;

  /* TLS Configuration */
  IOCnfg.targTCP.useTLS = USE_TLS;
#if TMWTARG_SUPPORT_TLS
    if (USE_TLS)
    {
      /* The default TMW configuration sets up the client's certificate files.
       * Because the file names are the same length but the values for
       * Linux and Windows differ, this sample application simply updates
       * the path to the server's certificates.
       */
      char *clientPtr;
      char serverString[] = "server";
      clientPtr = strstr(IOCnfg.targTCP.tlsRsaCertificateId, "client");
      if (clientPtr)
      {
        /* Copy without NULL terminator */
        memcpy(clientPtr, serverString, sizeof(serverString)-1);
      }
      clientPtr = strstr(IOCnfg.targTCP.tlsRsaPrivateKeyFile, "client");
      if (clientPtr)
      {
        /* Copy without NULL terminator */
        memcpy(clientPtr, serverString, sizeof(serverString)-1);
      }
    }
#endif

  IOCnfg.targTCP.localUDPPort = (TMWTYPES_USHORT)(20000 + channelNumber -1);

  /* When looping back the ip address is 127.0.0.1, but the source UDP address is the real IP address */
  IOCnfg.targTCP.validateUDPAddress = TMWDEFS_FALSE;

  IOCnfg.targTCP.disconnectOnNewSyn = TMWDEFS_FALSE;

  sdnpsesn_initConfig(&sesnConfig);

  /*
   * Open the Channel, Sessions, and Sectors
   */
  sprintf(logBuf, "opening DNP channel %lu\n", channelNumber);
  myLogOutput(logBuf);
  
  pSclChannel = dnpchnl_openChannel(pThreadApplContext, &DNPConfig, &tprtConfig, 
    &linkConfig, &physConfig, &IOCnfg, &targConfig);
  
  if(pSclChannel == TMWDEFS_NULL)
  {
    /* Failed to open */
    printf("Failed to open slave channel, exiting program \n");
    
    /* Sleep for 10 seconds before exiting */
    tmwtarg_sleep(10000);
    return (0);
  }

  for (i = 0; i < NUMBER_SESSIONS; i++)
  {
    sprintf(logBuf, "opening DNP session %d\n", i);
    myLogOutput(logBuf);
    
    sesnConfig.destination = (unsigned short)(3 + i);
    sesnConfig.source = (unsigned short)(4 + i);
    pSclSession[i] = (TMWSESN *)sdnpsesn_openSession(pSclChannel, &sesnConfig, TMWDEFS_NULL);
    if(pSclSession[i] == TMWDEFS_NULL)
    {
      /* Failed to open */
      printf("Failed to open slave session, exiting program \n");
    
      /* Sleep for 10 seconds before exiting */
      tmwtarg_sleep(10000);
      return (0);
    }
  }

  if (USE_POLLED_MODE)
  {
    /* If running in polled mode, tmwappl_checkForInput needs to be called every 50ms. */
    pollInterval = 50;
  }

  /*
   * Now that everything is set up, start a "main loop"
   * that sends events.
   */

  while (1)
  {
    /*
     * Process any data sent by the master for the channel on this application context.
     */
    /* If the target layer is configured to be event driven, there
     * is no need for the application to check for received data.
     */
    if (USE_POLLED_MODE)
    {
      tmwappl_checkForInput(pThreadApplContext);
    }
   
    if(myTimeToSendRequest(&pMyRequests->lastEvent, pMyRequests->eventInterval))
    {
      for (int i = 0; i < NUMBER_SESSIONS; i++)
      {
        TMWDTIME timeStamp;
        sprintf(logBuf, "Sending event for Channel %lu, Session %d\n", channelNumber, i);;
        myLogOutput(logBuf);

        sdnputil_getDateTime(pSclSession[i], &timeStamp);
        sdnpo002_addEvent(pSclSession[i], 4, 0x02, &timeStamp);
        sdnpo022_addEvent(pSclSession[i], 4, 100, 0x02, &timeStamp);
      }
    }
    tmwtarg_sleep(pollInterval);
  }

  /* Close sessions and channel */
  for (i = 0; i < NUMBER_SESSIONS; i++)
  {
    sprintf(logBuf, "closing DNP session %d\n", i);
    myLogOutput(logBuf);
    sdnpsesn_closeSession(pSclSession[i]);
  }

  sprintf(logBuf, "closing DNP channel %lu\n", channelNumber);
  myLogOutput(logBuf);
  dnpchnl_closeChannel(pSclChannel);
  delete pMyRequests;
  tmwappl_closeApplication(pThreadApplContext, TMWDEFS_FALSE);

  return (0);
}

MY_MASTER_REQUEST_INFO *myInitMasterRequests(void)
{
  MY_MASTER_REQUEST_INFO *p;
  TMWTYPES_MILLISECONDS currentTime ; 

  p = new MY_MASTER_REQUEST_INFO;

  p->integrityInterval = INTEGRITY_INTERVAL;
  p->eventInterval     = EVENT_POLL_INTERVAL;
  p->binCmdInterval    = BINCMD_INTERVAL;

  currentTime = tmwtarg_getMSTime();
  p->lastIntegrity = currentTime;
  p->lastEventPoll = currentTime;
  p->lastBinCmd    = currentTime;

  return p;
}

MY_SLAVE_REQUEST_INFO * myInitSlaveRequests(void)
{
  MY_SLAVE_REQUEST_INFO *p;
  p = new MY_SLAVE_REQUEST_INFO;
  p->eventInterval = EVENT_INTERVAL; 
  p->lastEvent = tmwtarg_getMSTime();

  return p;
}

/*
 * myTimeToSendRequest
 * See if it is time to send a request
 *
 * Note that this simplified example does not take into account
 * the fact that the lastIntegrity field could roll over.
 */
TMWTYPES_BOOL myTimeToSendRequest(TMWTYPES_MILLISECONDS *pLastTime, TMWTYPES_MILLISECONDS interval) 
{
  TMWTYPES_MILLISECONDS currentTime;
  TMWTYPES_BOOL returnVal;

  currentTime = tmwtarg_getMSTime();
  if (currentTime >= (*pLastTime + interval))
  {
    *pLastTime = currentTime;
    returnVal = TMWDEFS_TRUE;
  }
  else
  {
    returnVal = TMWDEFS_FALSE;
  }
  return (returnVal);
}

void myUnsolCallbackFcn(void *param, MDNPSESN_UNSOL_RESP_INFO *pResponse)
{
  char buf[256];
  TMWTARG_UNUSED_PARAM(param);
  TMWTARG_UNUSED_PARAM(pResponse);

  if(!(++unsolCount%100))
  {
    sprintf(buf, "DNP Unsolicited count %d\n", unsolCount);
    myLogOutput(buf);
  }
}

void myBrmCallbackFcn(void *errorStatus, DNPCHNL_RESPONSE_INFO *pResponse)
{
  char buf[256];
  
  TMWTARG_UNUSED_PARAM(errorStatus);

  sprintf(buf, "in Callback Function; status is %d\n", pResponse->status);
  myLogOutput(buf);
  
  switch(pResponse->status)
  {
  case DNPCHNL_RESP_STATUS_SUCCESS:
    successCount++;
    break;
  case DNPCHNL_RESP_STATUS_INTERMEDIATE:
    intermediateCount++;
    break;
  case DNPCHNL_RESP_STATUS_TIMEOUT:
    timeoutCount++;
    break;
  default:
    otherFailureCount++;
    break;
  }
  
  if(++totalCount >= 100)
  {
    sprintf(buf, "DNP Successful %d, Intermediate %d, Timeouts %d, Other Failures %d\n", 
      successCount, intermediateCount, timeoutCount, otherFailureCount);
    myLogOutput(buf);
    
    totalCount = 0;
  }
}
  
/* Sample function to start either Windows or Linux thread */
static void myStartThread(TMWTYPES_BOOL isMaster, int i)
{
  TMW_ThreadId threadId;
  if (isMaster)
  {
    TMW_ThreadCreate(&threadId, myChannelThreadMDNP, (TMW_ThreadArg)i, 0, 0);
  }
  else
  {
    TMW_ThreadCreate(&threadId, myChannelThreadSDNP, (TMW_ThreadArg)i, 0, 0);
  }
}

void myLogOutput(char *pBuf)
{
  /* to stdout */
  printf("%s",pBuf);
}

#if TMWCNFG_SUPPORT_DIAG
/* Simple diagnostic output function, registered with the Source Code Library */
void myPutDiagString(const TMWDIAG_ANLZ_ID *pAnlzId,const TMWTYPES_CHAR *pString)
{
  TMWDIAG_ID id = pAnlzId->sourceId;

  if((TMWDIAG_ID_ERROR & id) 
    ||(TMWDIAG_ID_APPL & id)
    ||(TMWDIAG_ID_USER & id))
  {
    myLogOutput((char *)pString);  
    return;
  }

  /* Comment this out to turn off verbose diagnostics */
  /* For now print everything */
  /* myLogOutput((char *)pString); */
}
#endif
#endif
