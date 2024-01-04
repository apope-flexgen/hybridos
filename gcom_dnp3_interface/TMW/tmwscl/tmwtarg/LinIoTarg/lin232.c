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


/* file: lin232.cpp
 * description: Implementation of RS232 target routines for Linux
 */

#include "tmwtargio.h"
#include "lin232.h"
#include "liniodiag.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwpltmr.h"

#if TMWTARG_SUPPORT_232

#if LINIOTARG_SUPPORT_LEGACY_CONFIG
static TMWTYPES_CHAR * _getBaudString(LIN232_BAUD_RATE baudRate)
{
  switch(baudRate)
  {
  case LIN232_BAUD_110:
    return("110");
  case LIN232_BAUD_300:
    return("300");
  case LIN232_BAUD_600:
    return("600");
  case LIN232_BAUD_1200:
    return("1200");
  case LIN232_BAUD_2400:
    return("2400");
  case LIN232_BAUD_4800:
    return("4800");
  case LIN232_BAUD_9600:
    return("9600");
  case LIN232_BAUD_19200:
    return("19200");
  case LIN232_BAUD_38400:
    return("38400");
  case LIN232_BAUD_57600:
    return("57600");
  case LIN232_BAUD_115200:
    return("115200");
  case LIN232_BAUD_230400:
    return("230400");
  case LIN232_BAUD_576000:
    return("576000");
  case LIN232_BAUD_921600:
    return("921600");
  case LIN232_BAUD_1152000:
    return("1152000");
  default:
    return("unknown");
  }
}
#else
static TMWTYPES_BOOL _setPortSpeed(SERIAL_IO_CHANNEL *pSerialChannel)
{
  if (strcmp(pSerialChannel->chnlConfig.baudRate, "110") == 0)
  {
    pSerialChannel->portSpeed = B110;
    return TMWDEFS_TRUE;
  }
  if (strcmp(pSerialChannel->chnlConfig.baudRate, "300") == 0)
  {
    pSerialChannel->portSpeed = B300;
    return TMWDEFS_TRUE;
  }
  if (strcmp(pSerialChannel->chnlConfig.baudRate, "600") == 0)
  {
    pSerialChannel->portSpeed = B600;
    return TMWDEFS_TRUE;
  }
  if (strcmp(pSerialChannel->chnlConfig.baudRate, "1200") == 0)
  {
    pSerialChannel->portSpeed = B1200;
    return TMWDEFS_TRUE;
  }
  if (strcmp(pSerialChannel->chnlConfig.baudRate, "2400") == 0)
  {
    pSerialChannel->portSpeed = B2400;
    return TMWDEFS_TRUE;
  }
  if (strcmp(pSerialChannel->chnlConfig.baudRate, "4800") == 0)
  {
    pSerialChannel->portSpeed = B4800;
    return TMWDEFS_TRUE;
  }
  if (strcmp(pSerialChannel->chnlConfig.baudRate, "9600") == 0)
  {
    pSerialChannel->portSpeed = B9600;
    return TMWDEFS_TRUE;
  }
  if (strcmp(pSerialChannel->chnlConfig.baudRate, "19200") == 0)
  {
    pSerialChannel->portSpeed = B19200;
    return TMWDEFS_TRUE;
  }
  if (strcmp(pSerialChannel->chnlConfig.baudRate, "38400") == 0)
  {
    pSerialChannel->portSpeed = B38400;
    return TMWDEFS_TRUE;
  }
  if (strcmp(pSerialChannel->chnlConfig.baudRate, "57600") == 0)
  {
    pSerialChannel->portSpeed = B57600;
    return TMWDEFS_TRUE;
  }
  if (strcmp(pSerialChannel->chnlConfig.baudRate, "115200") == 0)
  {
    pSerialChannel->portSpeed = B115200;
    return TMWDEFS_TRUE;
  }
  if (strcmp(pSerialChannel->chnlConfig.baudRate, "230400") == 0)
  {
    pSerialChannel->portSpeed = B230400;
    return TMWDEFS_TRUE;
  }
  if (strcmp(pSerialChannel->chnlConfig.baudRate, "576000") == 0)
  {
    pSerialChannel->portSpeed = B576000;
    return TMWDEFS_TRUE;
  }
  if (strcmp(pSerialChannel->chnlConfig.baudRate, "921600") == 0)
  {
    pSerialChannel->portSpeed = B921600;
    return TMWDEFS_TRUE;
  }
  if (strcmp(pSerialChannel->chnlConfig.baudRate, "1152000") == 0)
  {
    pSerialChannel->portSpeed = B1152000;
    return TMWDEFS_TRUE;
  }
  LINIODIAG_ERRORMSG("Lin232 Invalid baud rate (%s) configured on %s",  pSerialChannel->chnlConfig.baudRate, pSerialChannel->chnlConfig.portName);
  return TMWDEFS_FALSE;
}
#endif

static TMWTYPES_CHAR * _getParityString(TMWTARG232_PARITY parity)
{
  if(parity == TMWTARG232_PARITY_NONE)
    return("none");
  else if(parity == TMWTARG232_PARITY_EVEN)
    return("even");
  else
    return("odd");
}

/* function: lin232_initChannel */
void * TMWDEFS_GLOBAL lin232_initChannel(
  const void *pUserConfig,
  TMWTARG_CONFIG *pTmwConfig,
  TMWTARG_IO_CHANNEL *pTargIoChannel)
{
  TMWTARGIO_CONFIG *pIOConfig = (TMWTARGIO_CONFIG *) pUserConfig;
  SERIAL_IO_CHANNEL *pSerialChannel;

  pSerialChannel = (SERIAL_IO_CHANNEL *) malloc(sizeof(SERIAL_IO_CHANNEL));
  if (pSerialChannel == NULL)
    return TMWDEFS_NULL;

  memset(pSerialChannel, 0, sizeof(SERIAL_IO_CHANNEL));

  /* Store the channel configuration */
  pSerialChannel->chnlConfig = pIOConfig->targ232;

#if LINIOTARG_SUPPORT_LEGACY_CONFIG
  pSerialChannel->portSpeed = pSerialChannel->chnlConfig.baudRate;
#else
  if (!_setPortSpeed(pSerialChannel))
  {
    free(pSerialChannel);
    return TMWDEFS_NULL;
  }
#endif

  pTargIoChannel->pChannelCallback = pTmwConfig->pChannelCallback;
  pTargIoChannel->pChannelCallbackParam = pTmwConfig->pCallbackParam;
  pTargIoChannel->pChannelReadyCallback = pTmwConfig->pChannelReadyCallback;
  pTargIoChannel->pChannelReadyCbkParam = pTmwConfig->pChannelReadyCbkParam;
  pSerialChannel->numCharTimesBetweenFrames = pTmwConfig->numCharTimesBetweenFrames; 
  pSerialChannel->interCharTimeout = pTmwConfig->interCharTimeout;

  /* Set up the Channel Operation Functions */
  pTargIoChannel->pOpenFunction       = lin232_openChannel;
  pTargIoChannel->pXmitReadyFunction  = lin232_getTransmitReady;
  pTargIoChannel->pXmitFunction       = lin232_transmit;
  pTargIoChannel->pRecvFunction       = lin232_receive;
  pTargIoChannel->pCheckInputFunction = lin232_checkForInputFunction;
  pTargIoChannel->pCloseFunction      = lin232_closeChannel;
  pTargIoChannel->pDeleteFunction     = lin232_deleteChannel;

  pTargIoChannel->polledMode = pSerialChannel->chnlConfig.polledMode;

  pSerialChannel->ttyfd = -1;

  sprintf(pTargIoChannel->chanInfoBuf, "baud %s, data bits %d, stop bits %d parity %s",
#if LINIOTARG_SUPPORT_LEGACY_CONFIG
    _getBaudString(pSerialChannel->chnlConfig.baudRate),
#else
    pSerialChannel->chnlConfig.baudRate,
#endif
    pSerialChannel->chnlConfig.numDataBits + 7,
    pSerialChannel->chnlConfig.numStopBits + 1,
    _getParityString(pSerialChannel->chnlConfig.parity));
  pTargIoChannel->pChannelName = pSerialChannel->chnlConfig.chnlName;

  return pSerialChannel;
}

/* function: lin232_deleteChannel */
void TMWDEFS_GLOBAL lin232_deleteChannel(
  TMWTARG_IO_CHANNEL *pTargIoChannel)
{
  SERIAL_IO_CHANNEL *pSerialChannel = (SERIAL_IO_CHANNEL *)pTargIoChannel->pChannelInfo;
  /* Free the channel */
  free(pSerialChannel);
}

/* function: lin232_openChannel */
TMWTYPES_BOOL TMWDEFS_GLOBAL lin232_openChannel(
  TMWTARG_IO_CHANNEL *pTargIoChannel)
{
  SERIAL_IO_CHANNEL *pSerialChannel = (SERIAL_IO_CHANNEL *)pTargIoChannel->pChannelInfo;
  int ttyfd;
  struct termios settings;

  /* zero out the structure */
  memset(&settings, 0, sizeof(settings));

  if ((ttyfd = open(pSerialChannel->chnlConfig.portName, O_RDWR | O_NOCTTY)) < 0)
  {
    LINIODIAG_ERRORMSG("Lin232 Error opening %s, %s",  pSerialChannel->chnlConfig.portName, strerror(errno));
    return TMWDEFS_FALSE;
  }

  LINIODIAG_MSG("Lin232 opened %s",  pSerialChannel->chnlConfig.portName);

  /* read your man page for the meaning of all this. # man termios */
  /* Its a bit to involved to comment here                         */
  cfsetispeed(&settings, pSerialChannel->portSpeed);
  cfsetospeed(&settings, pSerialChannel->portSpeed);

  settings.c_line = 0;
  settings.c_iflag |= IGNBRK;
  settings.c_iflag &=~ IGNPAR;  /*SPM changed this to not ignore framing and parity errors */
  settings.c_iflag &=~ PARMRK;
  settings.c_iflag |=  INPCK; /* SPM changed to enable input parity checking */
  settings.c_iflag &=~ ISTRIP;
  settings.c_iflag &=~ INLCR;
  settings.c_iflag &=~ IGNCR;
  settings.c_iflag &=~ ICRNL;
  settings.c_iflag &=~ IUCLC;
  settings.c_iflag &=~ IXON;
  settings.c_iflag |= IXANY;
  settings.c_iflag &=~ IXOFF;
  settings.c_iflag &=~ IMAXBEL;

  settings.c_oflag |= OPOST;
  settings.c_oflag &=~ OLCUC;
  settings.c_oflag &=~ ONLCR;
  settings.c_oflag &=~ OCRNL;
  settings.c_oflag &=~ ONOCR;   /*SPM changed to allow output cr at column zero*/
  settings.c_oflag &=~ ONLRET;
  settings.c_oflag &=~ OFILL;
  settings.c_oflag &=~ OFDEL;


  settings.c_cflag &=~ CSIZE;
  if(pSerialChannel->chnlConfig.numDataBits == TMWTARG232_DATA_BITS_8)
    settings.c_cflag |= CS8;
  else
    settings.c_cflag |= CS7;

  if(pSerialChannel->chnlConfig.numStopBits == TMWTARG232_STOP_BITS_1)
    settings.c_cflag &=~ CSTOPB;
  else
    settings.c_cflag |= CSTOPB;

  settings.c_cflag |= CREAD;

  if(pSerialChannel->chnlConfig.parity == TMWTARG232_PARITY_NONE)
  {
    settings.c_cflag &=~ PARENB;
    settings.c_cflag &=~ PARODD;
  }
  else if(pSerialChannel->chnlConfig.parity == TMWTARG232_PARITY_EVEN)
  {
    settings.c_cflag |= PARENB;
    settings.c_cflag &=~ PARODD;
  }
  else
  {
    settings.c_cflag |= PARENB;
    settings.c_cflag |= PARODD;
  }

  settings.c_cflag &=~ HUPCL;  /* don't lower modem control lines after last process closed the device */
  settings.c_cflag |= CLOCAL;  /* ignore modem control lines */

  if(pSerialChannel->chnlConfig.portMode == TMWTARG232_MODE_NONE)
    settings.c_cflag &=~ CRTSCTS;
  else
    settings.c_cflag |= CRTSCTS;

  settings.c_lflag &=~ ISIG;
  settings.c_lflag &=~ ICANON;
  settings.c_lflag &=~ ECHO;
  settings.c_lflag |= IEXTEN;


  if(pSerialChannel->chnlConfig.bModbusRTU == TMWDEFS_TRUE)
  { 
    int bitsPerChar;
    int timeBetweenFrames;

    /* for modbus RTU, a message must be contiguous, with no more than 3 empty frames.
     * For now, we will use this fact to extablish the end of a message. 
     */

    /* 1 start bit, 8 data bits, 1 parity bit + 2 stop bits */
    bitsPerChar = 12;
    
    /* time in deciseconds  */
    timeBetweenFrames = ((pSerialChannel->numCharTimesBetweenFrames*bitsPerChar*10) /9600 /*dcb.BaudRate*/) + 1;

    /* The following two combined mean when there is a gap between chars or 255 chars are    *  received the read will return
     */
    settings.c_cc[VMIN] = 255;
    /* time between frames in deciseconds,  1/10 of a second. */
    settings.c_cc[VTIME] = timeBetweenFrames;
  }
  else
  {
    settings.c_cc[VMIN] = 0;
    settings.c_cc[VTIME] = 0;
  }

  if(tcsetattr(ttyfd, TCSANOW, &settings ) < 0)
  {
    LINIODIAG_ERRORMSG("Lin232 tcsetattr failed %s", strerror(errno));
    return(TMWDEFS_FALSE);
  }

  pSerialChannel->ttyfd = ttyfd;
#if TMWTARG_SUPPORT_POLL
  pSerialChannel->pollFd.fd = ttyfd;
  pSerialChannel->pollFd.events = POLLIN;
#endif
  return(TMWDEFS_TRUE);
}

/* function: lin232_closeChannel */
void TMWDEFS_GLOBAL lin232_closeChannel(
  TMWTARG_IO_CHANNEL *pTargIoChannel)
{
  SERIAL_IO_CHANNEL *pSerialChannel = (SERIAL_IO_CHANNEL *)pTargIoChannel->pChannelInfo;
  close(pSerialChannel->ttyfd);
  pSerialChannel->ttyfd = -1;
}

/* function: lin232_getTransmitReady */
TMWTYPES_MILLISECONDS TMWDEFS_GLOBAL lin232_getTransmitReady(
  TMWTARG_IO_CHANNEL *pTargIoChannel)
{
  TMWTARG_UNUSED_PARAM(pTargIoChannel);
  return 0;
}

/* function: lin232_receive */
TMWTYPES_USHORT TMWDEFS_GLOBAL lin232_receive(
  TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT maxBytes,
  TMWTYPES_MILLISECONDS maxTimeout,
  TMWTYPES_BOOL *pInterCharTimeoutOccurred)
{
  SERIAL_IO_CHANNEL *pSerialChannel = (SERIAL_IO_CHANNEL *)pTargIoChannel->pChannelInfo;
  int bytesReceived;
#if !TMWTARG_SUPPORT_POLL
  fd_set          rfds;
  struct timeval  tv;
#endif
  TMWTARG_UNUSED_PARAM(maxTimeout);
  TMWTARG_UNUSED_PARAM(pInterCharTimeoutOccurred);

  bytesReceived = 0;

#if TMWTARG_SUPPORT_POLL
  if (poll(&pSerialChannel->pollFd, 1, 0) > 0)
  {
#else
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&rfds);
  FD_SET(pSerialChannel->ttyfd, &rfds);

  if(select(pSerialChannel->ttyfd + 1, &rfds, NULL, NULL, &tv) != -1)
  {  
    if(FD_ISSET( pSerialChannel->ttyfd, &rfds))
#endif
    {
      if((bytesReceived = read(pSerialChannel->ttyfd, pBuff, maxBytes)) == -1)
      {
        LINIODIAG_ERRORMSG("Lin232 read failed %s", strerror(errno));
        bytesReceived = 0;
      }
    }
  }
  return bytesReceived;
}

/* function: lin232_transmit */
TMWTYPES_BOOL TMWDEFS_GLOBAL lin232_transmit(
  TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT numBytes)
{
  SERIAL_IO_CHANNEL *pSerialChannel = (SERIAL_IO_CHANNEL *)pTargIoChannel->pChannelInfo;
  int bytesSent = write(pSerialChannel->ttyfd, pBuff, numBytes);
  if(numBytes == (TMWTYPES_USHORT)bytesSent)
    return TMWDEFS_TRUE;
  else
  {
    LINIODIAG_ERRORMSG("Lin232 write failed %s", strerror(errno));
    return TMWDEFS_FALSE;
  }
}

void TMWDEFS_GLOBAL lin232_checkForInputFunction(TMWTARG_IO_CHANNEL *pTargIoChannel, TMWTYPES_MILLISECONDS timeout)
{
  SERIAL_IO_CHANNEL *pSerialChannel = (SERIAL_IO_CHANNEL *)pTargIoChannel->pChannelInfo;

  if (pSerialChannel->ttyfd != -1)
  {
#if TMWTARG_SUPPORT_POLL
    if (poll(&pSerialChannel->pollFd, 1, timeout) > 0)
#else
    fd_set          rfds;
    struct timeval  tv;

    tv.tv_sec = 0;
    tv.tv_usec = timeout * 1000;
    FD_ZERO(&rfds);
    FD_SET(pSerialChannel->ttyfd, &rfds);

    if (select(pSerialChannel->ttyfd + 1, &rfds, NULL, NULL, &tv) > 0)
#endif
    {
      pTargIoChannel->pReceiveCallbackFunc(pTargIoChannel->pCallbackParam);
    }
  }
  else
  {
    tmwtarg_sleep(500);
  }
}
  
#endif /* #if TMWTARG_SUPPORT_232 */
