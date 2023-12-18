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

// DNPPeer.cpp : Defines the entry point for the application.
//

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
#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpsim.h"
#include "tmwscl/dnp/sdnpo032.h"
#include "tmwscl/dnp/sdnputil.h"

#include "tmwtargio.h"
}

/* The USE_POLLED_MODE constant is used here to demonstrate how the library
 * can be used to configure the target layer to support polled mode vs.
 * event driven. The Linux and Windows TCP channels shipped with the SCL
 * support both. POLLED_MODE FALSE is recommended for Linux.
 * If writing your own target layer, only one configuration is required.
 */
#define USE_POLLED_MODE TMWDEFS_FALSE

#define USE_IPV6        TMWDEFS_FALSE
#define USE_TLS         TMWDEFS_FALSE

 /* To use TLS TMWCNFG_USE_OPENSSL must also be defined as TMWDEFS_TRUE */
#if USE_TLS && !TMWCNFG_USE_OPENSSL
#pragma message("To use TLS TMWCNFG_USE_OPENSSL must be defined")
#endif

#if USE_TLS && !TMWCNFG_USE_OPENSSL
#pragma message("To use TLS TMWCNFG_USE_OPENSSL must be defined")
#endif

/* It is possible to start an SAV5 session with no users and have them added by
 * the Authority/Master using a remote key change mehthod for user number 1 "Common".
 */
TMWTYPES_BOOL     noUserAtStartup = TMWDEFS_FALSE;

/* set this to TMWDEFS_TRUE to use SAv2 implementation */
TMWTYPES_BOOL myUseSAv2 = TMWDEFS_FALSE;

#if !TMWCNFG_MULTIPLE_TIMER_QS

typedef struct mySessionInfo
{
  TMWTYPES_MILLISECONDS  integrityInterval;
  TMWTYPES_MILLISECONDS  lastIntegrityPoll;
  TMWTYPES_MILLISECONDS  eventInterval;
  TMWTYPES_MILLISECONDS  lastEventPoll;
  TMWTYPES_MILLISECONDS  binCmdInterval;
  TMWTYPES_MILLISECONDS  lastBinCmd;

  TMWTYPES_MILLISECONDS  actAsAuthorityInterval; 
  TMWTYPES_MILLISECONDS  lastActAsAuthority;
} MY_SESSION_INFO;


TMWAPPL *pApplContext;

TMWTARGIO_CONFIG IOCnfg;
TMWPHYS_CONFIG physConfig;
TMWTARG_CONFIG targConfig;

DNPCHNL_CONFIG chnlConfig;
DNPTPRT_CONFIG prtConfig;
DNPLINK_CONFIG lnkConfig;

/* Master structures  */
TMWCHNL *master_pChannel;
MDNPSESN_CONFIG master_sesnConfig;
TMWSESN *master_pSession;
MDNPBRM_REQ_DESC request;
MDNPBRM_CROB_INFO CROBInfo;

/* Slave structures */
TMWCHNL *slave_pChannel;
SDNPSESN_CONFIG slave_sesnConfig;
TMWSESN *slave_pSession;

/*
 * The following code defines application - specific functions
 * that are used in this example.
 *
 * These functions begin with "my" to distinguish them from
 * Source Code Library functions, and are defined at the end
 * of the example in order to emphasize the typcial
 * interface with the Source Code Library.
 */
MY_SESSION_INFO * myInitMySession(void);
TMWTYPES_BOOL myTimeForIntegrity(MY_SESSION_INFO *pMySession);
TMWTYPES_BOOL myTimeForEvent(MY_SESSION_INFO *pMySession);
TMWTYPES_BOOL myTimeForBinCmd(MY_SESSION_INFO *pMySession);
void          my_brmCallbackFcn(void *pUserCallbackParam, DNPCHNL_RESPONSE_INFO *pResponse);
void          myPutDiagString(const TMWDIAG_ANLZ_ID *pAnlzId, const TMWTYPES_CHAR *pString);

#if MDNPDATA_SUPPORT_OBJ120
#include "tmwscl/dnp/mdnpsim.h"

TMWTYPES_BOOL     myNowSendUpdateKey = false;
void *            myUserNameDbHandle;

void              myInitMasterSecureAuthentication(MDNPSESN_CONFIG *pSesnConfig);
TMWTYPES_BOOL     myAddMasterAuthUsers(MDNPSESN *pSMNPSession, TMWTYPES_BOOL operateInV2Mode);

void              myInitSlaveSecureAuthentication(SDNPSESN_CONFIG *pSesnConfig);
TMWTYPES_BOOL     myAddSlaveAuthUsers(SDNPSESN *pSDNPSession, TMWTYPES_BOOL operateInV2Mode);

TMWTYPES_BOOL     myTimeToActAsAuthority(MY_SESSION_INFO *pMySession);

void              myActAsAuthority(MDNPBRM_REQ_DESC *pBRMRequest);
void              mySendUpdateKey(MDNPBRM_REQ_DESC *pBRMRequest);

void my_brmSACallbackFcn(void *pUserCallbackParam, DNPCHNL_RESPONSE_INFO *pResponse);

MDNPBRM_REQ_DESC  SARequest;


/* These are the default user keys the test harness uses for testing */
static TMWTYPES_UCHAR defaultUserKey1[] = {
  0x49, 0xC8, 0x7D, 0x5D, 0x90, 0x21, 0x7A, 0xAF, 
  0xEC, 0x80, 0x74, 0xeb, 0x71, 0x52, 0xfd, 0xb5
};

/* It is now recommended that SAV5 should be used with only a single user per Association
 * This would be the default key that the Test Harness uses for other users.
static TMWTYPES_UCHAR defaultUserKeyOther[] = {
  0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 
  0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
}; */

/* This one is used by the Authority and the Outstation, but
 * since this master is acting as the Authority it needs it.
 */
static TMWTYPES_UCHAR  authoritySymCertKey[] = {
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 
  0x09, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 
  0x09, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06
};
#endif


/*
 * Begin the main loop
 */
int main(int argc, char* argv[])
{
  mySessionInfo * pMySession;
  bool run = true;
  bool useSerial = false;
  int addEventCtr = 0;
  int anlgInPointNum = 0;

  TMWTARG_UNUSED_PARAM(argc);
  TMWTARG_UNUSED_PARAM(argv);

#if TMWCNFG_SUPPORT_DIAG 
  /* Register function to display diagnostic strings to console 
   * This is only necessary if using the WinIOTarg target layer.
   * If implementing a new target layer, tmwtarg_putDiagString()
   * should be modified if diagnostic output is desired.
   */
  tmwtargp_registerPutDiagStringFunc(myPutDiagString);
#endif
  

  /*
  * Initialize the source code library.
  */
  tmwappl_initSCL();

  /* Initialize DNP SCL. This includes:
   *  - initialize polled timer
   *  - Initialize async database
   *  - initialize application context
   */
  tmwtimer_initialize();
  tmwdb_init(10000);
  pApplContext = tmwappl_initApplication();

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

  /* Initialize the common target IO configuration structure.
  * Call tmwtargio_initConfig to initialize default values, then overwrite
  * specific values as needed.
  */
  tmwtargio_initConfig(&IOCnfg);
  
  if(useSerial)
  {
    IOCnfg.type = TMWTARGIO_TYPE_232;

    /* name displayed in analyzer window */
    strcpy(IOCnfg.targ232.chnlName, "DNP Slave");

    strcpy(IOCnfg.targ232.baudRate, "9600");
    IOCnfg.targ232.numDataBits = TMWTARG232_DATA_BITS_8;
    IOCnfg.targ232.numStopBits = TMWTARG232_STOP_BITS_1;
    IOCnfg.targ232.parity      = TMWTARG232_PARITY_NONE;
    IOCnfg.targ232.portMode    = TMWTARG232_MODE_NONE;
    IOCnfg.targ232.polledMode  = USE_POLLED_MODE;

#if defined(TMW_WTK_TARGET)
    /* COM port to open */
    strcpy(IOCnfg.targ232.portName, "COM2");
#endif

#if defined(TMW_LINUX_TARGET) 
    /* port to open */
    strcpy(IOCnfg.targ232.portName, "/dev/ttyS1");
#endif
  }
  else
  {
    IOCnfg.type = TMWTARGIO_TYPE_TCP;

    /* name displayed in analyzer window */
    strcpy(IOCnfg.targTCP.chnlName, "DNP Slave");

    /* *.*.*.* allows any client to connect*/
    strcpy(IOCnfg.targTCP.ipAddress, "*.*.*.*");
    //strcpy(IOCnfg.targTCP.ipAddress, "127.0.0.1");

    /* port to listen on */
    IOCnfg.targTCP.ipPort = 20000;
    IOCnfg.targTCP.polledMode = USE_POLLED_MODE;
    IOCnfg.targTCP.mode = TMWTARGTCP_MODE_SERVER;
    IOCnfg.targTCP.role = TMWTARGTCP_ROLE_OUTSTATION;
    IOCnfg.targTCP.disconnectOnNewSyn = TMWDEFS_FALSE;
    IOCnfg.targTCP.localUDPPort = 20000;

#if USE_IPV6
    /* If you want to use IPV6 instead */
    IOCnfg.targTCP.ipVersion = TMWTARG_IPV6;

    /* *.*.*.* allows any client to connect */
    strcpy(IOCnfg.targTCP.ipAddress, "::");
    /* You can still use *.*.*.* to allow any even though not IPV6) */
    //strcpy(IOCnfg.targTCP.ipAddress, "*.*.*.*");

    /* Allow IPV6 Loopback address to connect */
    //strcpy(IOCnfg.targTCP.ipAddress, "::1");

    /* Allow only client from this IPV6 Address to connect */
    //strcpy(IOCnfg.targTCP.ipAddress, "2600:1700:850:8690:28f3:d72b:f609:c41a");

    /* You can also specify a NIC and/or local IPV6 Address */
    //strcpy(IOCnfg.targTCP.nicName, "enp0s8");
    //strcpy(IOCnfg.targTCP.localIpAddress, "2600:1700:850:8690:3035:bc19:378f:f1bd");
    strcpy(IOCnfg.targTCP.localIpAddress, "::");
#endif

    /* TLS Configuration */
    IOCnfg.targTCP.useTLS = USE_TLS;
    /* You may want to change the TLS configuration from the following default values:
     *  IOCnfg.targTCP.caCrlFileName "../TMWCertificates/ca_public/tmw_sample_ca_certificate_revocation_list.pem"
     *  IOCnfg.targTCP.caFileName "../TMWCertificates/ca_public/tmw_sample_ca_rsa_public_certificate.pem"
     *  IOCnfg.targTCP.dhFileName ""
     *  IOCnfg.targTCP.tlsCommonName "TLS"
     *  IOCnfg.targTCP.tlsRsaCertificateId "../TMWCertificates/client_user/tmw_sample_tls_rsa_public_cert.pem"
     *  IOCnfg.targTCP.tlsRsaPrivateKeyFile "../TMWCertificates/client_user/tmw_sample_tls_rsa_private_key.pem"
     *  IOCnfg.targTCP.tlsRsaPrivateKeyPassPhrase"triangle"
     */
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
  }

 /*
  * Open the slave Channel
  */
  slave_pChannel = dnpchnl_openChannel(pApplContext, &chnlConfig, &prtConfig,
    &lnkConfig, &physConfig, &IOCnfg, &targConfig);

  if(slave_pChannel == TMWDEFS_NULL)
  {
    /* Failed to open */
    printf("Failed to open slave channel, exiting program \n");
    
    /* Sleep for 10 seconds before exiting */
    tmwtarg_sleep(10000);
    return (1);
  }


  /*
   * Initialize and open the slave Session
   */
  sdnpsesn_initConfig(&slave_sesnConfig);
  
#if SDNPDATA_SUPPORT_OBJ120
  /* Set this to true to enable DNP3 Secure Authentication support */
  bool myUseAuthentication = true; 
  
  /* If Secure Authentication is to be enabled */
  if(myUseAuthentication)
    myInitSlaveSecureAuthentication(&slave_sesnConfig);
#endif

  void *s_userHandle = (void*)1;
  slave_pSession = sdnpsesn_openSession(slave_pChannel, &slave_sesnConfig, s_userHandle);
  if(slave_pSession == TMWDEFS_NULL)
  {
    /* Failed to open session */
    printf("Failed to open slave session, exiting program \n");
    
    /* Sleep for 10 seconds before exiting */
    tmwtarg_sleep(10000);
    return (1);
  }

#if SDNPDATA_SUPPORT_OBJ120
  if (myUseAuthentication)
  {
    if (!myAddSlaveAuthUsers((SDNPSESN*)slave_pSession, slave_sesnConfig.authConfig.operateInV2Mode))
    {
      /* Sleep for 10 seconds before exiting */
      tmwtarg_sleep(10000);
      return (1);
    }
  }
#endif  
   
  /* Reinitialize the Channel config Structure for master
   */
  dnpchnl_initConfig(&chnlConfig, &prtConfig, &lnkConfig, &physConfig);

  /* ReInitialize IO Config Structure */
  /* Call tmwtargio_initConfig to initialize default values, then overwrite
   * values as needed.
   */
  tmwtargio_initConfig(&IOCnfg);
  
  if(useSerial)
  {
    IOCnfg.type = TMWTARGIO_TYPE_232;

    strcpy(IOCnfg.targ232.chnlName, "DNP Master");

    strcpy(IOCnfg.targ232.baudRate, "9600");
    IOCnfg.targ232.numDataBits = TMWTARG232_DATA_BITS_8;
    IOCnfg.targ232.numStopBits = TMWTARG232_STOP_BITS_1;
    IOCnfg.targ232.parity      = TMWTARG232_PARITY_NONE;
    IOCnfg.targ232.portMode    = TMWTARG232_MODE_NONE;
    IOCnfg.targ232.polledMode  = USE_POLLED_MODE;

#if defined(TMW_WTK_TARGET)
    /* COM port to open */
    strcpy(IOCnfg.targ232.portName, "COM1");
#endif

#if defined(TMW_LINUX_TARGET) 
    /* port to open */
    strcpy(IOCnfg.targ232.portName, "/dev/ttyS0");
#endif
  }
  else
  {
    IOCnfg.type = TMWTARGIO_TYPE_TCP;
    strcpy(IOCnfg.targTCP.chnlName, "DNP Master");

    /* TCP/IP address of remote device  
     * 127.0.0.1 is the loopback address and will attempt to connect to an outstation listener on this computer
     */
    strcpy(IOCnfg.targTCP.ipAddress, "172.17.0.2");

    /* Port on outstation to connect to */
    IOCnfg.targTCP.ipPort = 20000;
    IOCnfg.targTCP.polledMode = USE_POLLED_MODE;
    IOCnfg.targTCP.mode = TMWTARGTCP_MODE_CLIENT;
    IOCnfg.targTCP.role = TMWTARGTCP_ROLE_MASTER;

    /* TLS Configuration */
    IOCnfg.targTCP.useTLS = USE_TLS;
    /* You may want to change the TLS configuration from the following default values:
     *  IOCnfg.targTCP.caCrlFileName "../TMWCertificates/ca_public/tmw_sample_ca_certificate_revocation_list.pem"
     *  IOCnfg.targTCP.caFileName "../TMWCertificates/ca_public/tmw_sample_ca_rsa_public_certificate.pem"
     *  IOCnfg.targTCP.dhFileName ""
     *  IOCnfg.targTCP.tlsCommonName "TLS"
     *  IOCnfg.targTCP.tlsRsaCertificateId "../TMWCertificates/client_user/tmw_sample_tls_rsa_public_cert.pem"
     *  IOCnfg.targTCP.tlsRsaPrivateKeyFile "../TMWCertificates/client_user/tmw_sample_tls_rsa_private_key.pem"
     *  IOCnfg.targTCP.tlsRsaPrivateKeyPassPhrase"triangle"
     */

    /* Only one channel can bind to the local UDP port */
    IOCnfg.targTCP.localUDPPort = 20001;

    /* When looping back the ip address is 127.0.0.1, but the source UDP address is the real IP address */
    IOCnfg.targTCP.validateUDPAddress = TMWDEFS_FALSE;

    /* Broadcast address to use for sending broadcast requests over UDP */
    strcpy(IOCnfg.targTCP.udpBroadcastAddress, "192.168.1.255");

#if USE_IPV6
    /* If you want to use IPV6 instead */
    IOCnfg.targTCP.ipVersion = TMWTARG_IPV6;

    /* IPV6 Loopback address */
    strcpy(IOCnfg.targTCP.ipAddress, "::1");

    /* IPV6 specific address */
    //strcpy(IOCnfg.targTCP.ipAddress, "2600:1700:850:8690:dcb:c4b7:b101:9a48");

    /* IPV6 broadcast address */
    strcpy(IOCnfg.targTCP.udpBroadcastAddress, "FF02::1");

    /* You can also specify a NIC and/or local IP Address */
    //strcpy(IOCnfg.targTCP.nicName, "enp0s8");
    strcpy(IOCnfg.targTCP.localIpAddress, "::");
    //strcpy(IOCnfg.targTCP.localIpAddress, "2600:1700:850:8690:64b5:ad53:9dbd:28ff");
#endif
  }

 /*
  * Open the Master Channel
  */
  master_pChannel = dnpchnl_openChannel(pApplContext, &chnlConfig, &prtConfig,
    &lnkConfig, &physConfig, &IOCnfg, &targConfig);
  
  if(master_pChannel == TMWDEFS_NULL)
  {
    /* Failed to open */
    printf("Failed to open master channel, exiting program \n");
    
    /* Sleep for 10 seconds before exiting */
    tmwtarg_sleep(10000);
    return (1);
  }

  /*
   * Initialize and open the Master Session
   */
  mdnpsesn_initConfig(&master_sesnConfig);

#if MDNPDATA_SUPPORT_OBJ120
  /* Set myUseAuthentication to true to above to enable DNP3 Secure Authentication support for both master and slave */
  /* If Secure Authentication is to be enabled */
  if(myUseAuthentication)
    myInitMasterSecureAuthentication(&master_sesnConfig);
#endif

  void *m_userHandle = (void*)2;
  master_pSession = mdnpsesn_openSession(master_pChannel, &master_sesnConfig, m_userHandle);
  if(master_pSession == TMWDEFS_NULL)
  {
    /* Failed to open session */
    printf("Failed to open master session, exiting program \n");
    
    /* Sleep for 10 seconds before exiting */
    tmwtarg_sleep(10000);
    return (1);
  }

#if DNPCNFG_SUPPORT_AUTHENTICATION
  if (myUseAuthentication)
  {
    if (!myAddMasterAuthUsers((MDNPSESN*)master_pSession, master_sesnConfig.authConfig.operateInV2Mode))
    {
      /* Sleep for 10 seconds before exiting */
      tmwtarg_sleep(10000);
      return (1);
    }
  }
#endif
    
  /*
   * Initialize Request Descriptor
   */
  mdnpbrm_initReqDesc(&request, master_pSession);
  request.pUserCallback = my_brmCallbackFcn;
  request.pUserCallbackParam = (void *)"Master";
    
#if MDNPCNFG_SUPPORT_SA_VERSION5 && DNPCNFG_SUPPORT_AUTHKEYUPDATE
  mdnpbrm_initReqDesc(&SARequest, master_pSession);
  SARequest.pUserCallback = my_brmSACallbackFcn;
  SARequest.pUserCallbackParam = (void *)master_pSession;
#endif

  /* Now that everything is set up, start a "main loop"
   * that sends and processes request.
   * Note that this simple example only processes one
   * session on one channel.
   */
  pMySession = myInitMySession();

  while (run == TMWDEFS_TRUE)
  {

  /* See if it's time to send a request.
   * This simple demo only issues the following request:
   *  - Integrity Poll
   *  - Event Poll
   */
    if (myTimeForIntegrity(pMySession))
    {
      printf("Integrity Poll for Session \n");
      mdnpbrm_integrityPoll(&request);
    }

    if (myTimeForEvent(pMySession))
    {
      printf("Event Poll for Session \n");
      mdnpbrm_eventPoll(&request);
    }

    if (myTimeForBinCmd(pMySession))
    {
      printf("Issue Binary Output Command for Session \n");

      CROBInfo.pointNumber = 0;
      CROBInfo.control = (TMWTYPES_UCHAR)(CROBInfo.control != DNPDEFS_CROB_CTRL_LATCH_ON 
          ? DNPDEFS_CROB_CTRL_LATCH_ON : DNPDEFS_CROB_CTRL_LATCH_OFF);
      CROBInfo.onTime = 0;
      CROBInfo.offTime = 0;

#if MDNPDATA_SUPPORT_OBJ120
        // If secure authentication is enabled, you can choose to send this aggressive mode
        // to avoid the challenge/reply exchange
        if(master_sesnConfig.authenticationEnabled)
        {
          request.authAggressiveMode = true;
          // You can either use the default user (1) or choose another one that is configured 
          request.authUserNumber = DNPAUTH_DEFAULT_USERNUMBER;

          /* When using SAv5 use the user number that was sent in response to user being added
           * dynamically over DNP. This SHOULD be the default user since only 1 per association
           * is recommended.
           */
          if(!master_sesnConfig.authConfig.operateInV2Mode)
          {
            MDNPSESN *pMDNPSession = (MDNPSESN*)request.pSession; 
            TMWTYPES_USHORT userNumber = mdnpsim_authGetUserNumber(pMDNPSession->pDbHandle, myUserNameDbHandle);
            if(userNumber != 0)
              request.authUserNumber = userNumber;
          }
        }
#endif

     mdnpbrm_binaryCommand(&request, TMWDEFS_NULL, DNPDEFS_FC_SELECT,
          MDNPBRM_AUTO_MODE_OPERATE, 0, DNPDEFS_QUAL_16BIT_INDEX, 1, &CROBInfo);
    }
    

#if MDNPCNFG_SUPPORT_SA_VERSION5 && DNPCNFG_SUPPORT_AUTHKEYUPDATE
    if (!myUseSAv2)
    {
      if (myTimeToActAsAuthority(pMySession))
      {
        myActAsAuthority(&SARequest);
      }
      /* For symmetric update key change method, the encrypted Update Key data
       * from the Authority must be sent to complete the process
       */
      else if (myNowSendUpdateKey)
      {
        mySendUpdateKey(&SARequest);
      }
    }
#endif


   /* Process any data received .
    * Note that mdnpappl_checkForInput will be called
    * many times in between calls to mdnpbrm_xxx()
    */
    /* If the target layer is configured to be event driven, there
     * is no need for the application to check for received data.
     */
    if (USE_POLLED_MODE)
    {
      tmwappl_checkForInput(pApplContext);
    }

   /* Store any data on the data base queue.
    * Typically, this call might be put on a separate
    * thread, or the Async data base would be disabled
    * (i.e., set TMWCNFG_SUPPORT_ASYNCH_DB to TMWDEFS_FALSE).
    */
    while (tmwdb_storeEntry(TMWDEFS_TRUE))
      ;

   /*
    * Check timers
    */
    tmwpltmr_checkTimer();

    tmwtarg_sleep(100);

    /* Begin addEvent example.
     *  This code demonstrates how to use the xxx_addEvent function to
     *  inform the library that a data change has occurred instead of
     *  relying on SCL scanning. 
     *  The period (once every 100 iterations) and the point count (10)
     *  are aribtrary, but demonstrate how the 1st 10 analog values are
     *  updated periodially and the slave will send unsolicited updates
     *  to the master if enabled to do so.
     * NOTE: Setting analogInputScanPeriod to a non-zero value will result
     *       in this value being updated by the SCL scan.
     */
    addEventCtr++;
    if ((addEventCtr % 100) == 0)
    {
      TMWTYPES_ANALOG_VALUE analogValue;
      TMWDTIME timeStamp;
      sdnputil_getDateTime(slave_pSession, &timeStamp);
      analogValue.value.dval = rand();
      analogValue.type = TMWTYPES_ANALOG_TYPE_DOUBLE;

      sdnpo032_addEvent(slave_pSession, anlgInPointNum,
        &analogValue, DNPDEFS_DBAS_FLAG_ON_LINE,
        &timeStamp);
      anlgInPointNum++;
      if (anlgInPointNum == 10)
      {
        anlgInPointNum = 0;
      }
    }
    /* End addEvent example. */
  }

 /* Of course, we'll never get to here, since we're in an
  * infinite loop above, but just for show, go ahead and
  * close all open sessions and channels
  */
  mdnpsesn_closeSession(master_pSession);
  dnpchnl_closeChannel(master_pChannel);

  sdnpsesn_closeSession(slave_pSession);
  dnpchnl_closeChannel(slave_pChannel);

  delete pMySession;
  tmwappl_closeApplication(pApplContext, TMWDEFS_FALSE);

  return (0);
}

#define ActAsAuthorityInterval 120000
MY_SESSION_INFO * myInitMySession(void)
{
  mySessionInfo *p;
  p = new mySessionInfo;
  p->integrityInterval = 3600000;   /* Integrity poll once per hour */
  p->eventInterval = 5000;          /* Event poll once every 5 seconds */
  p->binCmdInterval = 20000;        /* Binary output command once every 20 seconds */

  p->lastEventPoll = tmwtarg_getMSTime();
  p->lastIntegrityPoll = tmwtarg_getMSTime();
  p->lastBinCmd = tmwtarg_getMSTime();

  if (!noUserAtStartup)
    p->actAsAuthorityInterval = ActAsAuthorityInterval; /* If SAv5, add or edit a user over DNP every xxx minutes */ /* Use the #define as this will be modified if there are no users configured */
  else
    p->actAsAuthorityInterval = 5000;                   /* If SAv5, add first user over DNP in 5 seconds  */
  p->lastActAsAuthority = tmwtarg_getMSTime();

  return p;
}

#if MDNPDATA_SUPPORT_OBJ120
void myInitMasterSecureAuthentication(MDNPSESN_CONFIG *pSesnConfig)
{

#if !MDNPCNFG_SUPPORT_SA_VERSION5
  /* If SAv5 is not supported, use SAv2 */
  myUseSAv2 = TMWDEFS_TRUE;
#endif

  /* Enable DNP3 Secure Authentication support */
  pSesnConfig->authenticationEnabled = TMWDEFS_TRUE;

  /* NOTE: Secure Authentication Version 2 (SAv2) will not function properly without implementing 
   * the functions mdnpdata_authxxx() in mdnpdata.c
   * Optional SAv5 also requires utils/tmwcrypto which uses OpenSSL by default.
   */

  /* Example configuration, some of these may be the default values,
   * but are shown here as an example of what can be set.
   */
  pSesnConfig->authConfig.extraDiags = TMWDEFS_TRUE;
  pSesnConfig->authConfig.aggressiveModeSupport = TMWDEFS_TRUE;

  pSesnConfig->authConfig.maxKeyChangeCount = 1000;

  // 60 seconds is very short for demonstration purposes only.
  pSesnConfig->authConfig.keyChangeInterval = 60000;  

  /* For SAv2 configure the same user number and update key for default user 
   * number on both master and outstation devices 
   * SAv5 allows User Update Keys to be sent to the outstation over DNP.
   */

  if(myUseSAv2)
  {
    /* Use Secure Authentication Version 2 */
#if MDNPCNFG_SUPPORT_SA_VERSION2 
    pSesnConfig->authConfig.operateInV2Mode = TMWDEFS_TRUE;
    pSesnConfig->authConfig.maxErrorCount = 2;

    /* Configure user numbers */
    /* Spec says default user number (1) provides a user number for the device or "any" user */
    pSesnConfig->authConfig.authUsers[0].userNumber = DNPAUTH_DEFAULT_USERNUMBER;

    /* DNP3 Technical Bulletin TB2019-001 indicates that the next version of SA
     * will remove support for support multiple users per Master-Outstation Association
     * and it is recommended that SAV5 should be used with only a single user per Association.
     * While the MDNP SAv2 and SAv5 implementations support multiple users this example
     * will not show that as it would violate that recommendation and not be allowed in the future.
     */
#endif
  } 
}
 

TMWTYPES_BOOL myAddMasterAuthUsers(MDNPSESN *pMDNPSession, TMWTYPES_BOOL operateInV2Mode)
{
#if MDNPCNFG_SUPPORT_SA_VERSION2 && TMWCNFG_SUPPORT_CRYPTO
  /* If SAv2 and using optional TMWCRYPTO interface set the crypto database*/
  if (operateInV2Mode)
  {
    TMWTYPES_USHORT userNumber = 1;

    /* If using simulated database in tmwcrypto, add the User Update Key for the default user (1) to it.
     * This key should really should be in YOUR crypto database not the simulated one in tmwcrypto.c.
     * You would not normally need to call tmwcrypto_configSimKey to add it to your own database.
     */
    if (!tmwcrypto_configSimKey(pMDNPSession->pDbHandle, TMWCRYPTO_USER_UPDATE_KEY, userNumber, defaultUserKey1, 16, TMWDEFS_NULL, 0))
    {
      /* Failed to add key */
      printf("Failed to add key, exiting program \n");
      return (TMWDEFS_FALSE);
    }

    /* DNP3 Technical Bulletin TB2019-001 indicates that the next version of SA
     * will remove support for support multiple users per Master-Outstation Association
     * and it is recommended that SAV5 should be used with only a single user per Association.
     * While the MDNP SAv2 and SAv5 implementations support multiple users this example
     * will not show that as it would violate that recommendation and not be allowed in the future.
     */

    return TMWDEFS_TRUE;
  }
#endif 
#if MDNPCNFG_SUPPORT_SA_VERSION5
  /* In SAv5 we no longer use an array of user configuration, instead you can add one user at a time.
   * The Authority can also tell the master to add a user using a globally unique user name
   * and instruct the master to send the update key and role (permissions) for that user over DNP to the outstation.
   *
   *    See mdnpbrm_authUserStatusChange and mdnpbrm_authUserUpdateKeyChange
   */
  if (!operateInV2Mode)
  {
    TMWTYPES_USHORT userNumber = 1;

    if (!noUserAtStartup)
    {
      /* You are allowed to start with NO user configured on the master and outstation and add the user at runtime.
       * In that case you don't have to configure a user here
       */

       /* If using simulated database in tmwcrypto, add the User Update Key for the default user (1) to it.
        * This key should really should be in YOUR crypto database not the simulated one in tmwcrypto.c.
        * You would not normally call tmwcrypto_configSimKey to add it to your own database.
        */
      if (!tmwcrypto_configSimKey(pMDNPSession->dnp.pCryptoHandle, TMWCRYPTO_USER_UPDATE_KEY, userNumber, defaultUserKey1, 16, TMWDEFS_NULL, 0))
      {
        /* Failed to add key */
        printf("Failed to add key, exiting program \n");
        return (TMWDEFS_FALSE);
      }

      /* For SAv5 add the user to the mdnp library */
      mdnpsesn_addAuthUser((TMWSESN*)pMDNPSession, userNumber);

      /* DNP3 Technical Bulletin TB2019-001 indicates that the next version of SA
       * will remove support for support multiple users per Master-Outstation Association
       * and it is recommended that SAV5 should be used with only a single user per Association.
       * While the MDNP SAv2 and SAv5 implementations support multiple users this example
       * will not show that as it would violate that recommendation and not be allowed in the future.
       */
    }

    /* Configure other values in YOUR crypto database to allow remote user key and role update from Master.*/

    /* This sample uses the same values that the Test Harness Outstation uses by default */

    /* Outstation name must be configured in both Master and Outstation.
     * This is already set in the mdnpsim database by default.
     * OutstationName = "SDNP Outstation";
     */

     /* If using the simulated database in tmwcrypto, add the Authority Certification Symmetric Key to it.
      * This key really should be in YOUR crypto database not the simulated one in tmwcrypto.c.
      * You would not normally call tmwcrypto_configSimKey to add it to your own database.
      * This key is used by the Central Authority, not the master, but this sample is acting as the Authority.
      */
    if (!tmwcrypto_configSimKey(pMDNPSession->dnp.pCryptoHandle, TMWCRYPTO_AUTH_CERT_SYM_KEY, 0, (TMWTYPES_UCHAR *)&authoritySymCertKey, 32, TMWDEFS_NULL, 0))
    {
      /* Failed to add key */
      printf("Failed to add key, exiting program \n");
      return (TMWDEFS_FALSE);
    }

    /* If using simulated database in tmwcrypto, configre Outstation Public Key when Asymmetric Key Update is supported
     * This really should be in YOUR crypto database not the simulated one.
     * You would not normally call tmwcrypto_configSimKey to add it to your own database.
     */
    const TMWTYPES_CHAR *pPublicKey = "TMWTestOSRsa2048PubKey.pem";
    TMWTYPES_USHORT keyLength = (TMWTYPES_USHORT)strlen(pPublicKey);
    if (!tmwcrypto_configSimKey(pMDNPSession->dnp.pCryptoHandle, TMWCRYPTO_OS_ASYM_PUB_KEY, 0, (TMWTYPES_UCHAR *)pPublicKey, keyLength, TMWDEFS_NULL, 0))
    {
      /* Failed to add key */
      printf("Failed to add key, exiting program \n");
      return (TMWDEFS_FALSE);
    }

    return TMWDEFS_TRUE;
  }
#endif
  return TMWDEFS_FALSE;
}
#endif
#if MDNPCNFG_SUPPORT_SA_VERSION5 && DNPCNFG_SUPPORT_AUTHKEYUPDATE
#define BUFFER_LENGTH 2048
/* In SAv5 a trusted third party, known as the Authority will command the 
 * master to add a user with a globally unique name and to send requests over the DNP 
 * protocol to add that user and his role on the outstation also.
 *
 * This function will act as that authority to demonstrate how your application should
 * request the master SCl to take action.
 */
TMWTYPES_BOOL     firstKeyUpdate = TMWDEFS_TRUE;
TMWTYPES_ULONG    globalStatusChangeSequenceNumber = 0;
 void myActAsAuthority(MDNPBRM_REQ_DESC *pSARequest)
 {
   MDNPSESN         *pMDNPSession;
   const char       *pUserPublicKeyFile;
   const char       *pUserPrivateKeyFile;
   const char       *pAuthorityPrivateKeyFile;
   const char       *AsymPrvKeyPassword;
   const char       *pUserName;
   TMWTYPES_UCHAR    algorithm;
   TMWTYPES_ULONG    sequence;
   TMWTYPES_USHORT   plainTextLength;
   TMWTYPES_USHORT   userNameLength;
   TMWTYPES_USHORT   certDataLength;
   TMWTYPES_UCHAR    operation;
   TMWTYPES_BOOL     sendUpdateKey;
   TMWTYPES_UCHAR    keyChangeMethod; 
   TMWTYPES_USHORT   userRole;
   TMWTYPES_USHORT   userRoleExpiryInterval;
   TMWTYPES_UCHAR    plainTextArray[1024]; 
   TMWTYPES_UCHAR    certData[1024];

   pMDNPSession = (MDNPSESN*)pSARequest->pSession;
   sequence = ++globalStatusChangeSequenceNumber;

   /* As an example, add and modify user Common periodically */
   /* Multiple users per association is now being discouraged...*/
   pUserName = "Common";

   if(firstKeyUpdate)
   {
     operation = DNPAUTH_USER_STATUS_ADD;
     firstKeyUpdate = TMWDEFS_FALSE;
   }
   else
   {
     operation = DNPAUTH_USER_STATUS_CHANGE;
   }
   
   /* Certification data should be provided by the Central Authority, 
    * but for this example act as the authority. 
    */
   sendUpdateKey = TMWDEFS_TRUE;


   keyChangeMethod = DNPAUTH_KEYCH_SYMAES256_SHA256;

   /* To use asymmetric key change method you need to configure the correct type and size asymmetric keys
    * incuding the Authority, Outstation and User Keys.
    * Outstation must also be configured properly for this key change method.
    *
    * keyChangeMethod = DNPAUTH_KEYCH_ASYM_RSA2048_SHA256;
    */

   userRole = DNPAUTH_USER_ROLE_SINGLEUSER;
   userRoleExpiryInterval = 100;

   plainTextArray[0] = operation & 0xff; 

   plainTextArray[1] = sequence & 0xff;
   plainTextArray[2] = (sequence>>8) & 0xff;
   plainTextArray[3] = (sequence>>16) & 0xff;
   plainTextArray[4] = (sequence>>24) & 0xff; 

   plainTextLength = 5;

   plainTextArray[plainTextLength++] = userRole & 0xff;
   plainTextArray[plainTextLength++] = (userRole>>8) & 0xff; 

   plainTextArray[plainTextLength++] = userRoleExpiryInterval & 0xff;
   plainTextArray[plainTextLength++] = (userRoleExpiryInterval>>8) & 0xff;

   userNameLength = (TMWTYPES_USHORT)strlen((const char *)pUserName);
   plainTextArray[plainTextLength++] = userNameLength & 0xff;
   plainTextArray[plainTextLength++] = (userNameLength>>8) & 0xff; 

   TMWCRYPTO_KEY userPubKey;
   if (keyChangeMethod < 64)
   {
     /* For symmetric methods the user name is next.  */
     memcpy(&plainTextArray[plainTextLength], pUserName, userNameLength);
     plainTextLength += userNameLength;
   }
   else
   {
     /* Asymmetric method */
     /* For asymmetric methods it would be User Public Key Length, User Name, and User Public Key, */

     /* If using simulated database in tmwcrypto, configure the User Asymmetric keys for this method
      * These keys should really should be in YOUR crypto database not the simulated one in tmwcrypto.c.
      * You would not normally need to call tmwcrypto_configSimKey to add it to your own database.
      */
     switch (keyChangeMethod)
     {
     case DNPAUTH_KEYCH_ASYM_RSA1024_SHA1:
       pAuthorityPrivateKeyFile = "TMWTestAuthorityDsa1024PrvKey.pem";
       pUserPublicKeyFile = "TMWTestUserDsa1024PubKey.pem";
       pUserPrivateKeyFile = "TMWTestUserDsa1024PrvKey.pem";
       AsymPrvKeyPassword = "triangle";
       algorithm = TMWCRYPTO_ALG_SIGN_DSA_1024_SHA1;
       break;

     case DNPAUTH_KEYCH_ASYM_RSA2048_SHA256:
       pAuthorityPrivateKeyFile = "TMWTestAuthorityDsa2048PrvKey.pem";
       pUserPublicKeyFile = "TMWTestUserDsa2048PubKey.pem";
       pUserPrivateKeyFile = "TMWTestUserDsa2048PrvKey.pem";
       AsymPrvKeyPassword = "triangle";
       algorithm = TMWCRYPTO_ALG_SIGN_DSA_2048_SHA256;
       break;

     case DNPAUTH_KEYCH_ASYM_RSA3072_SHA256:
       pAuthorityPrivateKeyFile = "TMWTestAuthorityDsa3072PrvKey.pem";
       pUserPublicKeyFile = "TMWTestUserDsa3072PubKey.pem";
       pUserPrivateKeyFile = "TMWTestUserDsa3072PrvKey.pem";
       AsymPrvKeyPassword = "triangle";
       algorithm = TMWCRYPTO_ALG_SIGN_DSA_3072_SHA256;
       break;

       // New key change methods from TB2016-002
     case DNPAUTH_KEYCH_ASYM_RSA1024_RSA_SHA1:
       pAuthorityPrivateKeyFile = "TMWTestAuthorityRsa1024PrvKey.pem";
       pUserPublicKeyFile = "TMWTestUserRsa1024PubKey.pem";
       pUserPrivateKeyFile = "TMWTestUserRsa1024PrvKey.pem";
       AsymPrvKeyPassword = "triangle";
       algorithm = TMWCRYPTO_ALG_SIGN_RSA_1024_SHA1;
       break;

     case DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256:
       pAuthorityPrivateKeyFile = "TMWTestAuthorityRsa2048PrvKey.pem";
       pUserPublicKeyFile = "TMWTestUserRsa2048PubKey.pem";
       pUserPrivateKeyFile = "TMWTestUserRsa2048PrvKey.pem";
       AsymPrvKeyPassword = "triangle";
       algorithm = TMWCRYPTO_ALG_SIGN_RSA_2048_SHA256;
       break;

     case DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256:
       pAuthorityPrivateKeyFile = "TMWTestAuthorityRsa3072PrvKey.pem";
       pUserPublicKeyFile = "TMWTestUserRsa3072PubKey.pem";
       pUserPrivateKeyFile = "TMWTestUserRsa3072PrvKey.pem";
       AsymPrvKeyPassword = "triangle";
       algorithm = TMWCRYPTO_ALG_SIGN_RSA_3072_SHA256;
       break;

     default:
       printf("FAILED, keys not configured for this key change method\n");
       return;
     }

     /* If using simulated database in tmwcrypto, configure Authority Private Key known only by the Authority
      * This key is used by the Central Authority, not the master, but this sample is acting as the Authority.
      * Since this master is simulating the central authority it needs to know this key.
      * This really should be in YOUR crypto database not the simulated one.
      * You would not normally need to call tmwcrypto_configSimKey to add it to your own database.
      */
     if (!tmwcrypto_configSimKey(pMDNPSession->dnp.pCryptoHandle, TMWCRYPTO_AUTH_ASYM_PRV_KEY, 0, (TMWTYPES_UCHAR *)pAuthorityPrivateKeyFile, (TMWTYPES_USHORT)strlen(pAuthorityPrivateKeyFile), (TMWTYPES_UCHAR *)"triangle", 8))
     {
       /* Failed to add key */
       printf("Failed to add key, exiting program \n");
       return;
     }

     /* Configure some user information including user public keys
      * This should really be updating your real database, not the simulated and simulated crypto databases.
      */
     myUserNameDbHandle = mdnpsim_authConfigUser(pMDNPSession->pDbHandle, (TMWTYPES_CHAR*)pUserName, userNameLength,
       sequence, keyChangeMethod, operation, userRole, userRoleExpiryInterval, (TMWTYPES_UCHAR*)"No Update Key Yet", 1, (TMWTYPES_UCHAR*)"No Certification Data Yet", 1);

     tmwcrypto_configSimKey(pMDNPSession->dnp.pCryptoHandle, TMWCRYPTO_USER_ASYM_PUB_KEY, 1, (TMWTYPES_UCHAR *)pUserPublicKeyFile, (TMWTYPES_USHORT)strlen(pUserPublicKeyFile), TMWDEFS_NULL, 0);

     tmwcrypto_configSimKey(pMDNPSession->dnp.pCryptoHandle, TMWCRYPTO_USER_ASYM_PRV_KEY, 1, (TMWTYPES_UCHAR *)pUserPrivateKeyFile, (TMWTYPES_USHORT)strlen(pUserPrivateKeyFile), (TMWTYPES_UCHAR *)AsymPrvKeyPassword, 8);

     /* Get user public key from key file.
      * From Jan 6 2011 DNP3 Tech SA Teleconference TC11-01-06-SA Minutes.
      * The public key should be an octet by octet copy of the
      * SubjectPublicKeyInfo field from the X509 certificate (RFC 5280).
      *  SubjectPublicKeyInfo  ::=  SEQUENCE  {
      *    algorithm            AlgorithmIdentifier,
      *    subjectPublicKey     BIT STRING  }
      * There is an example in rfc5280 page 141
      * Get the encoded key data
     */
     TMWTYPES_USHORT userNumber = 1;
     if (tmwcrypto_getKey(pMDNPSession->dnp.pCryptoHandle, TMWCRYPTO_USER_ASYM_PUB_KEY, (void *)userNumber, &userPubKey))
     {
       /* get index and leave 2 bytes for length of pub key data */
       TMWTYPES_USHORT lengthIndex = plainTextLength;
       plainTextLength += 2;

       // User name now
       memcpy(&plainTextArray[plainTextLength], pUserName, userNameLength);
       plainTextLength += userNameLength;

       /* Indicate how much room there is left in buffer before calling getKeyData */
       TMWTYPES_USHORT userPublicKeyLength = BUFFER_LENGTH - plainTextLength;
       if (!tmwcrypto_getKeyData(pMDNPSession->dnp.pCryptoHandle, &userPubKey, (void *)userNumber,
         &plainTextArray[plainTextLength], &userPublicKeyLength))
       {
         printf("FAILED, get user public key data \n");
         return;
       }
       plainTextLength += userPublicKeyLength;

       plainTextArray[lengthIndex++] = userPublicKeyLength & 0xff;
       plainTextArray[lengthIndex] = (userPublicKeyLength >> 8) & 0xff;
     }
   }

   if (keyChangeMethod < 64)
   {
     /* Method is symmetric, so the the pre-shared (between AUTHORITY and outstation), the
      * Authority Certification Key will be used to generate the MAC for this data.
      * Normally, master would NOT know this key.
      */
     certDataLength = 0;
     TMWCRYPTO_KEY certKey;
     tmwcrypto_getKey(pMDNPSession->dnp.pCryptoHandle, TMWCRYPTO_AUTH_CERT_SYM_KEY, 0, &certKey);
     if (!tmwcrypto_MACValue(pMDNPSession->dnp.pCryptoHandle, TMWCRYPTO_ALG_MAC_SHA256, &certKey, 32, plainTextArray, plainTextLength, certData, &certDataLength))
     {
       printf("FAILED, get Authority Cert Key \n");
       return;
     }
   }
   else
   {
     /* asymmetric */
     TMWCRYPTO_KEY authPrvKey;

     // Sign this with authority private key.
     tmwcrypto_getKey(pMDNPSession->dnp.pCryptoHandle, TMWCRYPTO_AUTH_ASYM_PRV_KEY, 0, &authPrvKey);
     memcpy(authPrvKey.password, "triangle", strlen("triangle"));
     authPrvKey.passwordLength = (TMWTYPES_USHORT)strlen("triangle");
     if (!tmwcrypto_genDigitalSignature(pMDNPSession->dnp.pCryptoHandle, algorithm, &authPrvKey, plainTextArray, plainTextLength, certData, &certDataLength))
     {
       printf("FAILED, Generate Digital Signature\n");
       return;
     }
   }
   
   /*
    * Call mdnpbrm_authUserStatusChange function to create or update an existing user on the outstation.
    *
    * Your application should not use the simulated database. We will use it here so this sample can run with the SCL as shipped.
    *
    * You should put the user data in Your database, and when mdnpdata_authGetUserName(),
    * mdnpdata_authGetChangeUserData() etc are called you should use the userNameDbHandle
    * you passed to mdnpbrm_authUserStatusChange to look up the user information.
    * When mdnpdata_authStoreUpdKeyChangeReply() is called the new user number returned from the outstation
    * should be added to your database.
    *
    * If the keyChangeMethod is symmetric, the g120v12 will return the user number. The master should then ask the
    * DNP Authority to provide the encrypted User Update Key data to be sent in a g120v13/v15 request. This can then be sent
    * by calling mdnpbrm_authUserSendSymUpdateKey()
    * If the keyChangeMethod is asymmetric, the master does not have to ask the DNP Authority for the data and the MDNP
    * library will authomatically send the g120v13/v14 request to send the User Update Key
    */
   TMWCRYPTO_KEY key;
   TMWTYPES_USHORT requestedKeyLength = 32;

   if ((keyChangeMethod == DNPAUTH_KEYCH_SYMAES128_SHA1) || (keyChangeMethod == DNPAUTH_KEYCH_ASYM_RSA1024_SHA1))
     requestedKeyLength = 16;

   tmwcrypto_generateNewKey(pMDNPSession->dnp.pCryptoHandle, TMWCRYPTO_USER_UPDATE_KEY, requestedKeyLength, &key);

   /* Add the User Update Key and Certification data to the database.
    * This should really be updating your real database, not the simulated one
    */
   myUserNameDbHandle = mdnpsim_authConfigUser(pMDNPSession->pDbHandle, (TMWTYPES_CHAR *)pUserName, userNameLength,
     sequence, keyChangeMethod, operation, userRole, userRoleExpiryInterval, key.value, key.length, certData, certDataLength);

   if (keyChangeMethod < 67)
   {
     /* If Key Change Method is symmetric, register a callback function to complete the remote update process */
     pSARequest->pUserCallback = my_brmSACallbackFcn;
   }


   if (mdnpbrm_authUserStatusChange(pSARequest, myUserNameDbHandle, operation, sendUpdateKey) == TMWDEFS_NULL)
   {
     printf("FAILED, mdnpbrm_authUserStatusChange() returned failure \n");
     return;
   }
 }

 /* This function will have the MDNP library send the encrypted User Update Key
  * that was received from the Authority and put in the database. In this sample 
  * the MDNP library will call mdnpdata_authGetUpdateKeyData() to get this encrypted data
  * but if it has not been implemented the MDNP library will simulate the Authority 
  * and encrypt the data using a configured Authority Certification (Symmetric) Key
  */
 void mySendUpdateKey(MDNPBRM_REQ_DESC *pSARequest)
 {
    pSARequest->pUserCallback = TMWDEFS_NULL;
    mdnpbrm_authUserSendSymUpdateKey(pSARequest, myUserNameDbHandle);
    myNowSendUpdateKey = false;
 }

/* myTimeToActAsAuthority
 * See if it is time to act as the Authority to tell the master to add a user
 * to the master and outstation. If so, return TMWDEFS_TRUE and update the 
 * "time of last poll" field to be the current time.
 */
TMWTYPES_BOOL myTimeToActAsAuthority(MY_SESSION_INFO *pMySession) 
{
  TMWTYPES_MILLISECONDS myTime;
  TMWTYPES_BOOL returnVal;
  
  myTime = tmwtarg_getMSTime();
  if (myTime >=(pMySession->lastActAsAuthority + pMySession->actAsAuthorityInterval))
  {
    pMySession->lastActAsAuthority = myTime;
    returnVal = TMWDEFS_TRUE;
  }
  else 
  {
    returnVal = TMWDEFS_FALSE;
  }
  return (returnVal);
}
#endif


#if SDNPDATA_SUPPORT_OBJ120
void myInitSlaveSecureAuthentication(SDNPSESN_CONFIG *pSesnConfig)
{

#if !SDNPCNFG_SUPPORT_SA_VERSION5
  /* If SAv5 is not supported, use SAv2 */
  myUseSAv2 = TMWDEFS_TRUE;
#endif

  /* Enable DNP3 Secure Authentication support */
  pSesnConfig->authenticationEnabled = TMWDEFS_TRUE;
  
  /* NOTE: Secure Authentication Version 2 (SAv2) will not function properly without implementing 
   * the functions sdnpdata_authxxx() in sdnpdata.c
   * SAv5 also requires utils/tmwcrypto which uses OpenSSL by default.
   */

  /* For SAv2 configure the same user number and update key for default user 
   * number on both master and outstation devices 
   * SAv5 allows User Update Keys to be sent to the outstation over DNP.
   */ 

  /* Use Secure Authentication Version 2 */
  if(myUseSAv2)
  {
#if SDNPCNFG_SUPPORT_SA_VERSION2 
    pSesnConfig->authConfig.operateInV2Mode = TMWDEFS_TRUE;
    pSesnConfig->authConfig.maxErrorCount = 2;

    /* Configure user numbers */
    /* Spec says user number 1 provides a user number for the device or "any" user */
    pSesnConfig->authConfig.authUsers[0].userNumber = DNPAUTH_DEFAULT_USERNUMBER;

    /* DNP3 Technical Bulletin TB2019-001 indicates that the next version of SA
     * will remove support for support multiple users per Master-Outstation Association
     * and it is recommended that SAV5 should be used with only a single user per Association.
     * While the MDNP SAv2 and SAv5 implementations support multiple users this example
     * will not show that as it would violate that recommendation and not be allowed in the future.
     */
#endif
  }

  /* Example configuration, some of these may be the default values,
   * but are shown here as an example of what can be set.
   */
  pSesnConfig->authConfig.extraDiags = TMWDEFS_TRUE;
  pSesnConfig->authConfig.aggressiveModeSupport = TMWDEFS_TRUE;
  pSesnConfig->authConfig.maxKeyChangeCount = 1000; 
  // 120 seconds is very short for demonstration purposes only.
  pSesnConfig->authConfig.keyChangeInterval = 120000;  
  pSesnConfig->authConfig.assocId = 0;
}

TMWTYPES_BOOL myAddSlaveAuthUsers(SDNPSESN *pSDNPSession, TMWTYPES_BOOL operateInV2Mode)
{
  /* If SAv2 and using optional TMWCRYPTO interface set the crypto database*/
  if (operateInV2Mode)
  {
    TMWTYPES_USHORT userNumber = 1;

    /* If using simulated database in tmwcrypto, add the User Update Key for the default user (1) to it.
     * This key should really should be in YOUR crypto database not the simulated one in tmwcrypto.c.
     * You would not normally need to call tmwcrypto_configSimKey to add it to your own database.
     * For SAv2 the handle used to look up keys in the sim database is the SDNP DB handle.
     */

    if (!tmwcrypto_configSimKey(pSDNPSession->pDbHandle, TMWCRYPTO_USER_UPDATE_KEY, userNumber, defaultUserKey1, 16, TMWDEFS_NULL, 0))
    {
      /* Failed to add key */
      printf("Failed to add key, exiting program \n");
      return (TMWDEFS_FALSE);
    }

    /* DNP3 Technical Bulletin TB2019-001 indicates that the next version of SA
     * will remove support for support multiple users per Master-Outstation Association
     * and it is recommended that SAV5 should be used with only a single user per Association.
     * While the MDNP SAv2 and SAv5 implementations support multiple users this example
     * will not show that as it would violate that recommendation and not be allowed in the future.
     */
  }
#if SDNPCNFG_SUPPORT_SA_VERSION5
  /* In SAv5 we no longer use an array of user configuration, instead we add one user at a time.
   * The Authority can also tell the master to add a user using a globally unique user name
   * and instruct the master to send the update key and role (permissions) for that user over DNP to the outstation.
   *
   *    See mdnpbrm_authUserStatusChange and mdnpbrm_authUserUpdateKeyChange
   */
  if (!operateInV2Mode)
  {
    const TMWTYPES_CHAR *pKey;
    TMWTYPES_USHORT userNumber = 1;
    SDNPSESN *pSDNPSession = (SDNPSESN *)slave_pSession;

    /* It is possible to start an SAV5 session with no users and have them added from the Authority/Master using a remote key change mehthod for user number 1 "Common" */
    if (!noUserAtStartup)
    {
      /* If using simulated database in tmwcrypto, add the User Update Key for the default user (1) to it.
       * This key should really should be in YOUR crypto database not the simulated one in tmwcrypto.c.
       * You would not normally need to call tmwcrypto_configSimKey to add it to your own database.
       */
      if (!tmwcrypto_configSimKey(pSDNPSession->dnp.pCryptoHandle, TMWCRYPTO_USER_UPDATE_KEY, userNumber, defaultUserKey1, 16, TMWDEFS_NULL, 0))
      {
        /* Failed to add key */
        printf("Failed to add key, exiting program \n");
        return (TMWDEFS_FALSE);
      }

      /* For SAv5 add the user to the sdnp library */
      sdnpsesn_addAuthUser((TMWSESN*)pSDNPSession, userNumber);

      /* Also add this to the SDNP simulated database to support optional remote key change methods */
      sdnpsim_authConfigUser(pSDNPSession->pDbHandle, (TMWTYPES_CHAR*)"Common", 6, 1, DNPAUTH_USER_ROLE_SINGLEUSER, 100);


      /* DNP3 Technical Bulletin TB2019-001 indicates that the next version of SA
       * will remove support for support multiple users per Master-Outstation Association
       * and it is recommended that SAV5 should be used with only a single user per Association.
       * While the MDNP SAv2 and SAv5 implementations support multiple users this example
       * will not show that as it would violate that recommendation and not be allowed in the future.
       */
    }

    /* Configure other values in YOUR crypto database to allow remote user key and role update from Master.*/

    /* This sample uses the same values that the Test Harness Outstation uses by default */

    /* Outstation name must be configured in both Master and Outstation.
     * This is already set in the mdnpsim database
     * OutstationName = "SDNP Outstation";
     */

     /* If using simulated database in tmwcrypto, configure the Authority Certification Symmetric Key to it.
      * This key really should be in YOUR crypto database not the simulated one in tmwcrypto.c.
      * You would not normally need to call tmwcrypto_configSimKey to add it to your own database.
      * This key is used by the Central Authority and configured on the Outstation.
      */
    if (!tmwcrypto_configSimKey(pSDNPSession->dnp.pCryptoHandle, TMWCRYPTO_AUTH_CERT_SYM_KEY, 0, (TMWTYPES_UCHAR *)&authoritySymCertKey, 32, TMWDEFS_NULL, 0))
    {
      /* Failed to add key */
      printf("Failed to add key, exiting program \n");
      return (TMWDEFS_FALSE);
    }

    /* If using simulated database in tmwcrypto, configure Outstation Private Key when Asymmetric Key Update is supported.
     * This really should be in YOUR crypto database not the simulated one.
     * You would not normally need to call tmwcrypto_configSimKey to add it to your own database.
     */
    pKey = "TMWTestOSRsa2048PrvKey.pem";
    TMWTYPES_USHORT keyLength = (TMWTYPES_USHORT)strlen(pKey);
    if (!tmwcrypto_configSimKey(pSDNPSession->dnp.pCryptoHandle, TMWCRYPTO_OS_ASYM_PRV_KEY, 0, (TMWTYPES_UCHAR *)pKey, keyLength, (TMWTYPES_UCHAR *)"triangle", 8))
    {
      /* Failed to add key */
      printf("Failed to add key, exiting program \n");
      return (TMWDEFS_FALSE);
    }

    /* If using simulated database in tmwcrypto, configure Authority Public Key when Asymmetric Key Update is supported
     * This really should be in YOUR crypto database not the simulated one.
     * You would not normally need to call tmwcrypto_configSimKey to add it to your own database.
     */
    TMWTYPES_UCHAR keyChangeMethod = DNPAUTH_KEYCH_ASYM_RSA2048_SHA256;
    switch (keyChangeMethod)
    {
    case DNPAUTH_KEYCH_ASYM_RSA1024_SHA1:
      pKey = "TMWTestAuthorityDsa1024PubKey.pem";
      break;

    case DNPAUTH_KEYCH_ASYM_RSA2048_SHA256:
      pKey = "TMWTestAuthorityDsa2048PubKey.pem";
      break;

    case DNPAUTH_KEYCH_ASYM_RSA3072_SHA256:
      pKey = "TMWTestAuthorityDsa3072PubKey.pem";
      break;

      // New key change methods from TB2016-002
    case DNPAUTH_KEYCH_ASYM_RSA1024_RSA_SHA1:
      pKey = "TMWTestAuthorityRsa1024PubKey.pem";
      break;

    case DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256:
      pKey = "TMWTestAuthorityRsa2048PubKey.pem";
      break;

    case DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256:
      pKey = "TMWTestAuthorityRsa3072PubKey.pem";
      break;

    default:
      printf("FAILED, keys not configured for this key change method\n");
      return (TMWDEFS_FALSE);
    }

    keyLength = (TMWTYPES_USHORT)strlen(pKey);
    if (!tmwcrypto_configSimKey(pSDNPSession->dnp.pCryptoHandle, TMWCRYPTO_AUTH_ASYM_PUB_KEY, 0, (TMWTYPES_UCHAR *)pKey, keyLength, TMWDEFS_NULL, 0))
    {
      /* Failed to add key */
      printf("Failed to add key, exiting program \n");
      return (TMWDEFS_FALSE);
    }

    /* add security statistics points to database */
    for (int i = 0; i < DNPAUTH_NUMBER_STATISTICS; i++)
      sdnpsim_addAuthSecStat(pSDNPSession->pDbHandle, i, TMWDEFS_CLASS_MASK_THREE, 0x01, 0);  /* spec says it SHALL be in an event class */

  }
#endif
  return (TMWDEFS_TRUE);
}

#endif

/* myTimeForIntegrity
 * See if it is time to conduct an Integrity Poll. If so, return
 * TMWDEFS_TRUE and update the "time of last poll" field to
 * be the current time.
 *
 * Note that this simplified example does not take into account
 * the fact that the lastIntegrityPoll field could roll over.
 */
TMWTYPES_BOOL myTimeForIntegrity(MY_SESSION_INFO *pMySession)
{
  TMWTYPES_MILLISECONDS myTime;
  TMWTYPES_BOOL returnVal;

  myTime = tmwtarg_getMSTime();
  if (myTime >=(pMySession->lastIntegrityPoll + pMySession->integrityInterval))
  {
    pMySession->lastIntegrityPoll = myTime;
    returnVal = TMWDEFS_TRUE;
  }
  else
  {
    returnVal = TMWDEFS_FALSE;
  }
  return (returnVal);
}

/* myTimeForEvent
 * See if it is time to conduct an Event Poll. If so, return
 * TMWDEFS_TRUE and update the "time of last poll" field to
 * be the current time.
 *
 * Note that this simplified example does not take into account
 * the fact that the lastEventPoll field could roll over.
 */
TMWTYPES_BOOL myTimeForEvent(MY_SESSION_INFO *pMySession)
{
  TMWTYPES_MILLISECONDS myTime;
  TMWTYPES_BOOL returnVal;

  myTime = tmwtarg_getMSTime();
  if (myTime > (pMySession->lastEventPoll + pMySession->eventInterval))
  {
    pMySession->lastEventPoll = myTime;
    returnVal = TMWDEFS_TRUE;
  }
  else
  {
    returnVal = TMWDEFS_FALSE;
  }
  return (returnVal);
}

/* myTimeForBinCmd
 * See if it is time to do a binary output. If so, return
 * TMWDEFS_TRUE and update the "time of last poll" field to
 * be the current time.
 *
 * Note that this simplified example does not take into account
 * the fact that the lastBinCmd field could roll over.
 */
TMWTYPES_BOOL myTimeForBinCmd(MY_SESSION_INFO *pMySession)
{
  TMWTYPES_MILLISECONDS myTime;
  TMWTYPES_BOOL returnVal;

  myTime = tmwtarg_getMSTime();
  if (myTime > (pMySession->lastBinCmd + pMySession->binCmdInterval))
  {
    pMySession->lastBinCmd = myTime;
    returnVal = TMWDEFS_TRUE;
  }
  else
  {
    returnVal = TMWDEFS_FALSE;
  }
  return (returnVal);
}

/* The following functions are called by the SCL */

/* my_brmCallbackFcn
 * Example user request callback function
 * This function will be called when request completes. 
 * pResponse->status will indicate if request was successful, timed out or 
 * otherwise failed.
 */
void my_brmCallbackFcn(void *pUserCallbackParam, DNPCHNL_RESPONSE_INFO *pResponse)
{
  printf("in BRM Callback Function, Sector %s; status is %d\n", (char *)pUserCallbackParam, pResponse->status);
}

#if MDNPCNFG_SUPPORT_SA_VERSION5 && DNPCNFG_SUPPORT_AUTHKEYUPDATE
/* my_brmSACallback
 * Example SA Auth User Status request callback function
 * This function will be called when the Auth User Status Change receives responses
 * including the g120v12 from the Outstation
 */
void my_brmSACallbackFcn(void *pUserCallbackParam, DNPCHNL_RESPONSE_INFO *pResponse)
{
  TMWTARG_UNUSED_PARAM(pUserCallbackParam);
  printf("in my_brmSACallback Callback Function, status is %d\n", pResponse->status);
  if(pResponse->last)
    myNowSendUpdateKey = true;
}
#endif


#if TMWCNFG_SUPPORT_DIAG
/* Simple diagnostic output function, registered with the Source Code Library */
void myPutDiagString(const TMWDIAG_ANLZ_ID *pAnlzId,const TMWTYPES_CHAR *pString)
{
  TMWDIAG_ID id = pAnlzId->sourceId;
 
  if((TMWDIAG_ID_ERROR & id) 
    ||(TMWDIAG_ID_APPL & id) 
    ||(TMWDIAG_ID_USER & id)
    ||(TMWDIAG_ID_SECURITY_DATA & id)
    ||(TMWDIAG_ID_SECURITY_HDRS & id)
   ) 
  {
    printf((char *)pString);  
    return;
  }

  /* Comment this out to turn off verbose diagnostics */
  /* For now print everything */
  /* printf((char *)pString); */
}
#endif
#endif
