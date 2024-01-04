/*****************************************************************************/
/* Triangle MicroWorks, Inc.                         Copyright (c) 2008-2011 */
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


/* file: tmwtargio.c
* description: Target I/O initialization routine.
*/
#include "tmwtargcnfg.h"
#include "tmwtargio.h"

#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwpltmr.h"


/* function: tmwtargio_initConfig
 * description: This function is called to initialize the I/O channel's configuration
 *              structure. The configuration provided is optimized to work with TMW's
 *              Test Harne and demonstrates how to this initialization can be customized.
 */
void TMWDEFS_GLOBAL tmwtargio_initConfig(TMWTARGIO_CONFIG *pConfig)
{

#if (TMWTARG_SUPPORT_TCP == TMWDEFS_FALSE) && (TMWTARG_SUPPORT_232 == TMWDEFS_FALSE)
  pConfig->type = TMWTARGIO_TYPE_NONE;
#endif

#if (TMWTARG_SUPPORT_TCP == TMWDEFS_FALSE) && (TMWTARG_SUPPORT_232 == TMWDEFS_TRUE)
  pConfig->type = TMWTARGIO_TYPE_232;
#endif

#if (TMWTARG_SUPPORT_TCP == TMWDEFS_TRUE) && (TMWTARG_SUPPORT_232 == TMWDEFS_FALSE)
  pConfig->type = TMWTARGIO_TYPE_TCP;
#endif

#if (TMWTARG_SUPPORT_TCP == TMWDEFS_TRUE) && (TMWTARG_SUPPORT_232 == TMWDEFS_TRUE)
  pConfig->type = TMWTARGIO_TYPE_232;
#endif

#if TMWTARG_SUPPORT_232 
  strncpy(pConfig->targ232.chnlName, "serial0", TMWTARG_STR_LEN);
  strncpy(pConfig->targ232.portName, "/sample/serial-0", TMWTARG_STR_LEN);
  strncpy(pConfig->targ232.baudRate, "9600", sizeof(pConfig->targ232.baudRate));
  pConfig->targ232.numDataBits = TMWTARG232_DATA_BITS_8;
  pConfig->targ232.numStopBits = TMWTARG232_STOP_BITS_1;
  pConfig->targ232.parity      = TMWTARG232_PARITY_NONE;
  pConfig->targ232.portMode    = TMWTARG232_MODE_NONE;
  pConfig->targ232.bModbusRTU  = TMWDEFS_FALSE;
#endif

#if TMWTARG_SUPPORT_TCP
  strncpy(pConfig->targTCP.chnlName, "", TMWTARG_STR_LEN);
  pConfig->targTCP.mode = TMWTARGTCP_MODE_CLIENT;
  pConfig->targTCP.role = TMWTARGTCP_ROLE_MASTER;
  pConfig->targTCP.ipVersion = TMWTARG_IPV4;
  strncpy(pConfig->targTCP.ipAddress, "127.0.0.1", TMWTARG_IP_ADDR_LENGTH);
  strncpy(pConfig->targTCP.localIpAddress, "*.*.*.*", TMWTARG_IP_ADDR_LENGTH);

  pConfig->targTCP.ipPort                 = 20000;
  pConfig->targTCP.dualEndPointIpPort     = 20000;
  pConfig->targTCP.disconnectOnNewSyn     = TMWDEFS_FALSE;
  pConfig->targTCP.ipConnectTimeout       = 1000;
  strncpy(pConfig->targTCP.nicName, "", TMWTARG_IF_NAME_LENGTH);

  /* Polled or Event Driven Receive data mode */
  pConfig->targTCP.polledMode         = TMWDEFS_TRUE;

#if TMWTARG_SUPPORT_UDP
  strncpy(pConfig->targTCP.udpBroadcastAddress, "192.168.1.255", TMWTARG_IP_ADDR_LENGTH);
  pConfig->targTCP.localUDPPort           = TMWTARG_UDP_PORT_NONE;
  pConfig->targTCP.destUDPPort            = 20000;
  pConfig->targTCP.initUnsolUDPPort       = 20000;
  pConfig->targTCP.validateUDPAddress     = TMWDEFS_TRUE;
#endif

  /* TLS support  */
  pConfig->targTCP.useTLS             = TMWDEFS_FALSE;
#if TMWTARG_SUPPORT_TLS
  pConfig->targTCP.nCaVerifyDepth             = 1;
  pConfig->targTCP.nTlsRenegotiationCount     = 0;
  pConfig->targTCP.nTlsRenegotiationMsTimeout = 0;
  pConfig->targTCP.nTlsRenegotiationSeconds   = 0;
  pConfig->targTCP.tlsHandshakeMsTimeout      = 2000; /* 2 seconds */
#endif
#endif
}

