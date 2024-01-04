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

/* file: WinTCPChannel.cpp
 * description: Implementation of Windows TCP I/O Target interface. This
 *  implementation uses two classes to provice a standard TCP/IP client
 *  server interface. The first class implements a single 'channel' where
 *  a channel consists of a specific IP address:port to IP address:port
 *  connection. An instance of this class is created for each channel
 *  in the source code library.
 *
 */
#include "StdAfx.h"
#pragma warning(disable: 4100)

#include <vector>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#pragma comment(lib, "Ws2_32.lib")

#include "WinIoTarg/include/WinIoTarg.h"
#include "WinIoTarg/include/WinIoTargDefs.h"
#include "WinIoTarg/WinIoInterface.h"
#include "WinIoTarg/WinTCPChannel.h" 

// List of TCP Channels listening for connections
std::vector<WinTCPListener *> *WinTCPChannel::m_pWinTCPListenerList = WINIOTARG_NULL;
AutoCriticalSectionWinTarg WinTCPChannel::m_listenerListCrit;

WinTCPIpAddressList::WinTCPIpAddressList()
{
  m_ipAddressList = new std::vector<char *>;
}

// Delete the list of addresses
WinTCPIpAddressList::~WinTCPIpAddressList()
{   
  IPAddressList::iterator Iter;
  while(m_ipAddressList->size() > 0)
  {
    Iter = m_ipAddressList->begin();
    char *ptr = *Iter;
    m_ipAddressList->erase(Iter);
    delete [] ptr;
  }

  delete m_ipAddressList;
}

bool WinTCPChannel::ConvertNameToIpAddress(char *pName, int addrFamily, void* addrPtr)
{
  //If this is a valid IP address then return the converted address.
  int result;
  result = inet_pton(addrFamily, pName, addrPtr);
  if (result == 1)
  {
    return true;
  }

  // If this is not a valid IP address then try to look it up with DNS
  struct addrinfo hints, *pResult;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = addrFamily;

  result = getaddrinfo(pName, NULL, &hints, &pResult);
  if(result == 0)
  {
    if (pResult->ai_family == AF_INET6)
    {
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)pResult->ai_addr;
      memcpy(addrPtr, &ipv6->sin6_addr, sizeof(in6_addr));
      return true;
    }
    else
    {
      struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *)pResult->ai_addr;
      memcpy(addrPtr, &sockaddr_ipv4->sin_addr, sizeof(in_addr));
      return true;
    }
  }
  else
  { 
    WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TCP: GetAddrInfo failed for host named %s",pName);
  }
  return false;
}

#define MAX_HOST_LEN 128
// Add this address to the list of allowed addresses.
bool WinTCPIpAddressList::Add(const char *pAddresses)
{
  int index; 
  char  *entryPtr =  new char[MAX_HOST_LEN];
  m_ipAddressList->push_back(entryPtr);
   
  index = 0;
  while( *pAddresses != 0x0 )
  {
    if(*pAddresses != ';' && *pAddresses != ',')
    {
      if(*pAddresses != ' ')
        entryPtr[index++] = *pAddresses;
    }
    else
    {
      // Null terminate completed address or name
      entryPtr[index++] = 0;

      entryPtr =  new char[MAX_HOST_LEN];
      m_ipAddressList->push_back(entryPtr);
      index = 0;
    }

    pAddresses++;

    if(index == (MAX_HOST_LEN-1))
      // Too long
      break;
  }

  entryPtr[index++] = 0;

  return true;
}

// Is this address in the list of allowed addresses? (*.*.*.* allows all addresses)
bool WinTCPIpAddressList::IsAddrConfigured(ADDRESS_FAMILY afInet, const char *pAddress)
{  
  //IPAddressList *v1 = m_ipAddressList;
  IPAddressList::iterator Iter;
  TMWTYPES_CHAR tempBuffer[64];

  for ( Iter = m_ipAddressList->begin( ) ; Iter != m_ipAddressList->end( ) ; Iter++ )
  {
    char *ptr = (char *)*Iter;

    if (strcmp(ptr, "::") == 0)
    {
      // Allow application to configure either :: or *.*.*.* to be mean "any" to make it easier.
      ptr = "*.*.*.*";
    }

    if(strcmp(ptr, "*.*.*.*") != 0)
    {
      IN6_ADDR in6Address;
      if (WinTCPChannel::ConvertNameToIpAddress(ptr, afInet, &in6Address))
      {
        ptr = (char *)inet_ntop(afInet, &in6Address, tempBuffer, sizeof(tempBuffer));
      }
    } 

    if(strcmp(ptr, pAddress) == 0)
      return true;
  }
  return false;
}

char *WinTCPIpAddressList::GetFirstAddress()
{ 
  if(m_ipAddressList->size() > 0)
  {
    IPAddressList::iterator Iter; 
    Iter = m_ipAddressList->begin();
    char *ptr = (char *)*Iter;
    return (ptr);
  }
  else 
    return TMWDEFS_NULL;
}

// Constructor
WinTCPChannel::WinTCPChannel(const WINTCP_CONFIG &config, TMWTARG_CONFIG *pTmwTargConfig) : 
  m_commSocket(INVALID_SOCKET), m_clientSocket(INVALID_SOCKET), m_serverSocket(INVALID_SOCKET)
{
  m_bNewConnection = false;
  m_bForceReset = false;
  m_pChannel = WINIOTARG_NULL;

  // Copy configuration data structure
  m_config = config;
  strncpy(m_config.ipAddress, config.ipAddress, TMWTARG_IP_ADDR_LENGTH);
  strncpy(m_config.chnlName, config.chnlName, TMWTARG_STR_LEN);

  ipAddressList.Add(config.ipAddress);
  m_afInet = AF_INET;
  m_ipAddrLen = sizeof(IN_ADDR);
  if (config.ipVersion == TMWTARG_IPV6)
  {
    m_afInet = AF_INET6;
    m_ipAddrLen = sizeof(IN6_ADDR);
  }

  // Set channel name for logging
  m_name = (m_config.chnlName) ? m_config.chnlName : m_config.ipAddress;

  // Set port number for logging
  if (m_config.mode != WINTCP_MODE_UDP)
  {
    m_portNumber = this->m_config.ipPort;
    LogMessage(TMWDIAG_ID_TARGET, "Created new TCP Channel");
  }
  else
  {
    m_portNumber = this->m_config.localUDPPort;
    LogMessage(TMWDIAG_ID_TARGET, "Created new UDP Only Channel");
  }

  // UDP related 
  m_UDPthreadIsRunning    = false;
  m_UDPSocket             = INVALID_SOCKET;
  m_udpReaderThreadHandle = INVALID_HANDLE_VALUE;
  m_srcUDPPort            = WINTCP_UDP_PORT_NONE;
  memset(&m_validUDPAddress, 0, sizeof(m_validUDPAddress));
  m_UDPbufferReadIndex    = 0;
  m_UDPbufferWriteIndex   = 0;
  m_UPDMulticastJoin      = false;

}

// Destructor
WinTCPChannel::~WinTCPChannel(void) 
{
  LogMessage(TMWDIAG_ID_TARGET, "Deleting TCP Channel");

  // Close channel if not already closed
  if (isChannelOpen())
    close();

  if(m_udpReaderThreadHandle == INVALID_HANDLE_VALUE)
    CloseHandle(m_udpReaderThreadHandle);
}

// Is this address in the list of allowed addresses?
bool WinTCPChannel::IsLocalAddrConfigured(const char *pAddress)
{
  if(m_config.localIpAddress == NULL)
    return true;

  if(strlen(m_config.localIpAddress) == 0)
    return true;

	if(strcmp(m_config.localIpAddress, TMWTARG_ADDR_IPV4_ANY) == 0)
		return true;

  if(strcmp(m_config.localIpAddress, TMWTARG_ADDR_IPV6_ANY) == 0)
		return true;

	if(strcmp(m_config.localIpAddress, pAddress) == 0)
		return true;

  /* the local address has been entered with a scope ID, remove it for the address comparison. */
  if (m_config.ipVersion == TMWTARG_IPV6)
  {
    char * scopeId = strchr(m_config.localIpAddress, '%');
    if (scopeId)
    {
      size_t addrLen = (size_t)(scopeId - &m_config.localIpAddress[0]);
      if (strncmp(m_config.localIpAddress, pAddress, addrLen) == 0)
        return true;
    }

  }

  return false;
}

// Is this address in the list of allowed addresses?
bool WinTCPChannel::IsAddrConfigured(const char *pConnectAddress)
{
  return (ipAddressList.IsAddrConfigured(m_afInet, pConnectAddress));
}

/**********************************************************************************\
	Function :			WinTCPChannel::setSocket
	Description : set the class members socket	
	Return :			void	-	
	Parameters :
			TMWTYPES_ULONG commSocket	-	
	Note : [none]
\**********************************************************************************/
void WinTCPChannel::setSocket(SOCKET acceptSocket, bool isClient)
{
  if(isClient)
    m_clientSocket = acceptSocket;
  else
    m_serverSocket = acceptSocket;

  m_commSocket = acceptSocket;

 #if  TMWTARG_SUPPORT_CHANNEL_THREAD
  SetEvent(m_channelThreadEvent); //Signal the channel thread to start processing data

  if(m_pChannelCallback != WINIOTARG_NULL)
    m_pChannelCallback(m_pCallbackParam, true, TMWDEFS_TARG_OC_SUCCESS);
#endif
}


// Return this channel's name
const char *WinTCPChannel::getName(void)
{
  return m_name;
}

// Return info about this channel
const char *WinTCPChannel::getInfo(void)
{ 
  sprintf(m_info, "Port: %s:%d", m_config.ipAddress, m_config.ipPort);
  return m_info;
}

// Get current channel status
const char *WinTCPChannel::getStatus(void)
{
  if(isChannelOpen() == false)
  {
    sprintf(m_status, "Closed");
  }
  else
  {
    sprintf(m_status, "Open");
  }

  return(m_status);
}

/**********************************************************************************\
	Function :			WinTCPChannel::connect
	Description : Attempt to connect to a remote server
	Return :			bool	-	
	Parameters :
			void	-	
	Note : [none]
\**********************************************************************************/
bool WinTCPChannel::connect(void)
{
  SOCKET tempCommSocket;

  LogMessage(TMWDIAG_ID_TARGET, "TCP Opening connection");

  // if already connected, just return true;
  if(m_commSocket != INVALID_SOCKET)
  {
    LogMessage(TMWDIAG_ID_TARGET, "TCP Connection is in progress"); 
    return(true);
  }

  tempCommSocket = socket(m_afInet, SOCK_STREAM, IPPROTO_IP);
  if(tempCommSocket == INVALID_SOCKET)
  {   
    WinIoTarg_formatErrorMsg(m_errorBuffer, WSAGetLastError());  
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TCP connect socket failed\n%19s%s", " ",m_errorBuffer);
    return(false);
  }

  /* Disable the Nagle algorithm to make sure packets are sent in a timely fashion */
  BOOL flag = true;
  if(setsockopt(tempCommSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(BOOL)) != 0)
  { 
    WinIoTarg_formatErrorMsg(m_errorBuffer, WSAGetLastError());   
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TCP Connect setsockopt failed\n%19s%s", " ",m_errorBuffer);
    return(false);
  }
 
  SOCKADDR_IN6 sockAddr;
  memset(&sockAddr, 0, sizeof(sockAddr));
  sockAddr.sin6_family = m_afInet;
  void *pSockAddrIpAddr = &((SOCKADDR_IN *)&sockAddr)->sin_addr.s_addr;
  if (m_afInet == AF_INET6)
  {
    pSockAddrIpAddr = &sockAddr.sin6_addr;
  }

  char *ipAddress = ipAddressList.GetFirstAddress();
  if (!ConvertNameToIpAddress(ipAddress, m_afInet, pSockAddrIpAddr))
  {
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TCP Connect ConvertNameToIpAddress failed");
    closesocket(tempCommSocket);
    return(false);
  }

  
  if(isClient())
  {
    sockAddr.sin6_port = htons(m_config.ipPort);
  }

  // For DNP Networking support dual end point.
  // ipPort would be used for listening on, not connecting to.
  else if(isDualEndPoint())
  {
    sockAddr.sin6_port = htons(m_config.dualEndPointIpPort);
  }

  unsigned long blockmode = 1;  // enable non-blocking
  if(ioctlsocket(tempCommSocket, FIONBIO, &blockmode) == SOCKET_ERROR)
  {
    WinIoTarg_formatErrorMsg(m_errorBuffer, WSAGetLastError());    
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TCP Connect NONBLOCKING ioctl failed\n%19s%s", " ",m_errorBuffer);
    closesocket(tempCommSocket);
    return(false);
  }
   
  /* This code will force the client to use a particular IP Address and/or port for
     the outgoing connection (1700 commented out in this example) */
  SOCKADDR_IN6 sockAddr1;
  memset(&sockAddr1,0,sizeof(sockAddr1));
  sockAddr1.sin6_family = m_afInet;
  sockAddr1.sin6_port = htons(0); /*htons(1700);*/
  void *pSockAddr1IpAddr = &((SOCKADDR_IN *)&sockAddr1)->sin_addr.s_addr;
  if (m_afInet == AF_INET6)
  {
    pSockAddr1IpAddr = &sockAddr1.sin6_addr;
  }
  inet_pton(sockAddr1.sin6_family, m_config.localIpAddress, pSockAddr1IpAddr);

  if(bind(tempCommSocket, (struct sockaddr *) &sockAddr1, sizeof(sockAddr1)) != 0)
  { 
    WinIoTarg_formatErrorMsg(m_errorBuffer, WSAGetLastError());     
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TCP Bind failed on local IP Address\n%19s%s", " ",m_errorBuffer);
    /* Don't closesocket and return, just let it use whatever address it can */
  } 

  /* This function should not block since ioctlsocket() has been called */
  int result = ::connect(tempCommSocket, (struct sockaddr *)&sockAddr, sizeof(sockAddr));

  /* A non blocking connect should always return SOCKET_ERROR */
  if(result != SOCKET_ERROR)
  { 
    WinIoTarg_formatErrorMsg(m_errorBuffer, WSAGetLastError());     
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TCP Connect failed\n%19s%s", " ",m_errorBuffer);
    closesocket(tempCommSocket);
    return(false);
  }

  /* Error code should be would block */
  DWORD last_error = WSAGetLastError();
  if(last_error != WSAEWOULDBLOCK)
  {
    WinIoTarg_formatErrorMsg(m_errorBuffer, last_error);      
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR,  "TCP Connect failed\n%19s%s", " ",m_errorBuffer);
    closesocket(tempCommSocket);
    return(false);
  }

  /* Use select to wait for the connection to be accepted */
  fd_set writefds;
  FD_ZERO(&writefds);
#pragma warning(disable:4127)
  FD_SET(tempCommSocket, &writefds);
#pragma warning(default:4127)

  /* Store the select FD in the interface object so it can be closed if the channel is closed. */
  {
#if TMWTARG_SUPPORT_CHANNEL_THREAD
    CriticalSectionLockWinTarg lock(m_chanThreadStateLock);
    if (m_chanThreadState != TMWTARG_THREAD_RUNNING)
    {
      closesocket(tempCommSocket);
      return(false);
    }
#endif
    m_selectPendingSocket = tempCommSocket;
  }
  
  struct timeval timeout = { 0, (long)m_config.ipConnectTimeout * 1000 };
  result = select(0, WINIOTARG_NULL, &writefds, WINIOTARG_NULL, &timeout);

  m_selectPendingSocket = INVALID_SOCKET;

  /* If error we did not connect */
  if(result == SOCKET_ERROR)
  { 
    WinIoTarg_formatErrorMsg(m_errorBuffer, WSAGetLastError());      
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TCP Connect select failed\n%19s%s", " ",m_errorBuffer);
    closesocket(tempCommSocket);
    return(false);
  }

  /* If no error, see if activity on the correct file descriptor */
  if(!FD_ISSET(tempCommSocket, &writefds))
  {
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TCP Connect failed, no activity on socket");
    closesocket(tempCommSocket);
    return(false);
  }

  /* If channel was closed while waiting for select */
  if(m_chanState != TMWTARG_CHANNEL_OPENED)
  {
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TCP Connect failed, channel closed");
    closesocket(tempCommSocket);
    return(false);
  }

  // For DNP Networking, save the address 
  if (m_config.validateUDPAddress)
  {
    memcpy(&m_validUDPAddress, pSockAddrIpAddr, m_ipAddrLen);
  }

  // This is a virtual method that is only implemented if this is doing TLS over TCP.
  if(!TLSConnect(tempCommSocket))
  {
    closesocket(tempCommSocket);
    return(false);
  }
 
  // Tell the channel we have a new socket (connection)
  setSocket(tempCommSocket, true);

  /* Return success */
  LogMessage(TMWDIAG_ID_TARGET, "TCP Connect success");
  return(true);
}

/**********************************************************************************\
	Function :			WinTCPChannel::listen
	Description : start waiting for incoming connection
	Return :			bool	-	
	Parameters :
			void	-	
	Note : [none]
\**********************************************************************************/
bool WinTCPChannel::listen(void)
{
  CriticalSectionLockWinTarg lock(getListenerListLock());
  bool status = false;
  // create the listen list if it does not exist
  if (m_pWinTCPListenerList == WINIOTARG_NULL)
  {
    m_pWinTCPListenerList = new std::vector<WinTCPListener *>;
  }

  LogMessage(TMWDIAG_ID_TARGET, "TCP listen for a connection on port:", m_config.ipPort);

  /* search listening threads for a thread that is listening 
   * for a connection on this same port  
   */
  WinTCPListener *pWinTCPListener = WINIOTARG_NULL;
  bool foundListener = false;
  for(unsigned int i = 0; i < getWinTCPListenerList()->size(); i++)
  {
    pWinTCPListener = getWinTCPListenerList()->at(i);
    if(pWinTCPListener->isSamePort(m_config.ipPort))
    {
      LogMessage(TMWDIAG_ID_TARGET, "TCP listen, found an existing Listener to use on port: %d", m_config.ipPort);
      foundListener = true;
      break;
    }
  }

  if(!foundListener)
  {
    LogMessage(TMWDIAG_ID_TARGET,  "TCP listen, no existing Listener found, creating one on port:", m_config.ipPort);
    pWinTCPListener = new WinTCPListener(m_config.ipPort);
  }

  if(pWinTCPListener != WINIOTARG_NULL)
  {
    LogMessage(TMWDIAG_ID_TARGET, "TCP Listen, add this channel to the listener on port:", m_config.ipPort);
    status = pWinTCPListener->addChannelToListener(this);
  }

  if(status)
    LogMessage(TMWDIAG_ID_TARGET, "TCP Listen, successfully listening on port:", m_config.ipPort);
  else
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR,  "TCP Listen, failed on port:", m_config.ipPort);

  return(status);
}

/**********************************************************************************\
	Function :			WinTCPChannel::setChannelCallback
	Description : [none]	
	Return :			void	-	
	Parameters :
			  TMWTARG_CHANNEL_CALLBACK_FUNC pChannelCallback	-	
			  void *pCallbackParam	-	
	Note : [none]
\**********************************************************************************/
void WinTCPChannel::setChannelCallback(
  TMWTARG_CHANNEL_CALLBACK_FUNC pChannelCallback, 
  void *pCallbackParam)
{
  m_pChannelCallback = pChannelCallback;
  m_pCallbackParam = pCallbackParam;
}


/**********************************************************************************\
	Function :			WinTCPChannel::open
	Description : [none]	
	Return :			bool	-	
	Parameters :
			void	-	
	Note : [none]
\**********************************************************************************/
bool WinTCPChannel::open(void)
{
  bool status = false;

  if (m_config.mode != WINTCP_MODE_UDP)
    LogMessage(TMWDIAG_ID_TARGET, "TCP open");
  else
    LogMessage(TMWDIAG_ID_TARGET, "UDP Only Open");

  if(!TLSOpen())
   return false;

  // Special logic to handle Dual End Point and UDP Only for DNP is required.
   
  // If configured to use TLS, don't allow the use of UDP 
  if(!m_config.useTLS)
  {
    // For DNP normally, both master and outstation should always support UDP
    // But for DNP and especially the other protocols check config to see if 
    // user wants it.
    if(m_config.localUDPPort != WINTCP_UDP_PORT_NONE)
    {
      // If UDP has not been opened yet
      if(m_UDPSocket == INVALID_SOCKET)
      {
        // If configured for UDP only, don't use broadcast address,
        // Return success or failure
        if(m_config.mode == WINTCP_MODE_UDP)
        {
          char *ipAddress = ipAddressList.GetFirstAddress();
          IN6_ADDR validUDPAddress;
          ConvertNameToIpAddress(ipAddress, m_afInet, &validUDPAddress);
          status = udpEndPoint(ipAddress);
          if(m_config.validateUDPAddress)
          {
            m_validUDPAddress = validUDPAddress;
          }
          return(status);
        }
        else
        {
          // This is tolerant of failure, if TCP AND UDP. may want to catch this
          // The problem is both master and slave use 20000 by default so it often fails
          // when testing in loopback.
          status = udpEndPoint(m_config.udpBroadcastAddress);
        }
      }
    }
  }


  if(isServer())
  {
    // Setup listener if not already listening
    status = listen();

    // If listener is running see if socket is valid
    if(status)
    {
      if(m_serverSocket == INVALID_SOCKET)
      {
        status = false;
      }
    }
  }
  else if(isClient())
  {
    status = connect();
  }
  else if(isDualEndPoint())
  {
    if (m_chanThreadCheckMask & CT_CHECK_CONNECT)
    {
      status = connect();
      if (status)
      {
        /* Connection was successful, clear to connect flag so new   */
        /* connections are not attempted until data needs to be sent.*/
        m_chanThreadCheckMask &= ~CT_CHECK_CONNECT;
      }
    }
    else
    {
      // Setup listener if not already listening
      // Tell library it is connected,
      // so that SCL will attempt to send data when needed
      // which will cause a TCP connect request to be attempted.
      status = listen();
    }
  }

  return(status);
}

/**********************************************************************************\
	Function :			WinTCPChannel::close
	Description : [none]	
	Return :			void	-	
	Parameters :
			void	-	
	Note : [none]
\**********************************************************************************/
void WinTCPChannel::close(void) 
{
  {
    CriticalSectionLockWinTarg lock(getListenerListLock());

    /* If we're a server, quit listening for connections */
    if(isServer() ||isDualEndPoint())
    {
      LogMessage(TMWDIAG_ID_TARGET, "TCP close");

      /* Dual end point should not try to reconnect. And server never tries */
      m_chanThreadCheckMask &= ~CT_CHECK_CONNECT;

      if (m_pWinTCPListenerList)
      {
        for(unsigned int i = 0; i < getWinTCPListenerList()->size(); i++)
        {
          WinTCPListener *pWinTCPListener = getWinTCPListenerList()->at(i);
          if(pWinTCPListener->isSamePort(m_config.ipPort))
          {
            pWinTCPListener->removeChannelFromListener(this);
            // If this is the listener thread running, don't try to terminate it 
            DWORD threadId = GetCurrentThreadId();
            if (threadId != pWinTCPListener->getThreadId())
            {
              // call method to see if channel list is empty.
              if (pWinTCPListener->getChannelListSize() == 0)
              {
                // if so wait for thread to end
                DWORD status = WaitForSingleObject(pWinTCPListener->m_listenThreadHandle, 5000);
                if (status == WAIT_TIMEOUT)
                {
                  TerminateThread(pWinTCPListener->m_listenThreadHandle, 0);
                  pWinTCPListener->forceClose();

                  LogMessage(TMWDIAG_ID_TARGET, "WinTCPChannel::close terminateThread\r\n");
                }
                // destructor removes listener from listener list.
                delete pWinTCPListener;
              }
            }
            break;
          }
        }
      }
    }

    if (m_pWinTCPListenerList)
    {
      if (getWinTCPListenerList()->size() == 0)
      {
        LogMessage(TMWDIAG_ID_TARGET, "TCP close, no more channels listening for this port, delete Listener");
        delete m_pWinTCPListenerList;
        m_pWinTCPListenerList = WINIOTARG_NULL;
      }
    }
  }

  TLSClose();

  StopUDPReaderThread();

  m_chanThreadCheckMask |= CT_NOT_ACTIVE;

  if(m_UDPSocket != INVALID_SOCKET)
  {
    LogMessage(TMWDIAG_ID_TARGET, "TCP close UDP socket");
    if (m_UPDMulticastJoin)
    {
      struct ipv6_mreq mreq;
      mreq.ipv6mr_interface = 0;
      inet_pton(m_afInet, m_config.udpBroadcastAddress, &mreq.ipv6mr_multiaddr);
      setsockopt(m_UDPSocket, IPPROTO_IPV6, IPV6_LEAVE_GROUP, (const char *)&mreq, sizeof(mreq));
      m_UPDMulticastJoin = false;
    }
    closesocket(m_UDPSocket); 
    m_UDPSocket = INVALID_SOCKET;
  }

  if(m_clientSocket != INVALID_SOCKET)
  {
    LogMessage(TMWDIAG_ID_TARGET, "TCP close Client socket");
    closesocket(m_clientSocket);
    m_clientSocket = INVALID_SOCKET;
  }

  if(m_serverSocket != INVALID_SOCKET)
  {
    LogMessage(TMWDIAG_ID_TARGET, "TCP close Server socket");
    closesocket(m_serverSocket); 
    m_serverSocket = INVALID_SOCKET;
  }
  m_commSocket = INVALID_SOCKET;

  memset(&m_validUDPAddress, 0, sizeof(m_validUDPAddress));
}

/**********************************************************************************\
	Function :			WinTCPChannel::lowReceive
	Description : Low level receive method	
	Return :			TMWTYPES_USHORT	-	
	Parameters :
			  TMWTYPES_UCHAR *pBuff	-	
			  TMWTYPES_ULONG maxNumChars	-	
			  bool peekOnly	-	
	Note : [none]
\**********************************************************************************/
TMWTYPES_USHORT WinTCPChannel::lowReceive(
  TMWTYPES_UCHAR *pBuff, 
  TMWTYPES_ULONG maxNumChars, 
  bool peekOnly) 
{
  TMWTYPES_USHORT numReceived = 0;
  fd_set readfds;

  if(m_commSocket == INVALID_SOCKET)
  {
    /* For Dual End Point, the library thinks there is a connection so it will try
     * to send causing a connect attempt. Don't tell the library it is closed.
     */
    if (!isDualEndPoint())
    {
      if (m_pChannelCallback != WINIOTARG_NULL)
        m_pChannelCallback(m_pCallbackParam, false, TMWDEFS_TARG_OC_FAILURE);
    }
    return(0);
  }

  if(m_bForceReset == true)
  {
    m_bForceReset = false;
    if(m_pChannelCallback != WINIOTARG_NULL)
      m_pChannelCallback(m_pCallbackParam, false, TMWDEFS_TARG_OC_RESET);
  } 
  
  if(m_config.useTLS)
  {
    return (TMWTYPES_USHORT)TLSLowReceive(pBuff, maxNumChars);
  }

  FD_ZERO(&readfds);
#pragma warning(disable:4127)
  FD_SET(m_commSocket,&readfds);
#pragma warning(default:4127)
  struct timeval timeout = {0,0};

  // Call select with timeout of 0 to see if data is available
  if(select(0, &readfds, WINIOTARG_NULL, WINIOTARG_NULL, &timeout) == SOCKET_ERROR)
  {
    // Socket Error
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "In lowReceive select returned socket error");
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "Last error = %d", WSAGetLastError());

    if(m_pChannelCallback != WINIOTARG_NULL)
      m_pChannelCallback(m_pCallbackParam, false,TMWDEFS_TARG_OC_FAILURE); 
    else
      close();
  }
  else if(FD_ISSET(m_commSocket, &readfds))
  {
    int returnValue = recv(m_commSocket, (char*)pBuff, maxNumChars, peekOnly ? MSG_PEEK:0); 
    if(returnValue == SOCKET_ERROR)
    {
      // Socket Error
      LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "In lowReceive recv returned socket error");
      LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "Last error = %d", WSAGetLastError());

      if(m_pChannelCallback != WINIOTARG_NULL)
        m_pChannelCallback(m_pCallbackParam, false, TMWDEFS_TARG_OC_FAILURE);
      else
        close();
    }
    else if(returnValue == 0) 
    {
      // Disconnect
      LogMessage(TMWDIAG_ID_TARGET, "In lowReceive recv returned 0");

      if(m_pChannelCallback != WINIOTARG_NULL)
        m_pChannelCallback(m_pCallbackParam, false, TMWDEFS_TARG_OC_FAILURE);
      else
        close();
    }
    else
    {
      // Read some bytes
      numReceived = (TMWTYPES_USHORT)returnValue;
    }
  }

  return(numReceived);
}

/**********************************************************************************\
	Function :			WinTCPChannel::receive
	Description : Receive bytes from channel, called from SCL
	Return :			TMWTYPES_USHORT	-	
	Parameters :
			  TMWTYPES_UCHAR  *pBuff	-	
			  TMWTYPES_ULONG  maxNumChars	-	
	Note : [none]
\**********************************************************************************/
TMWTYPES_USHORT WinTCPChannel::receive(
  TMWTYPES_UCHAR  *pBuff, 
  TMWTYPES_ULONG  maxNumChars)
{
  TMWTYPES_USHORT numReceived = 0;
 
  // first see if there are any bytes received on the UDP socket 
  // Currently only for DNP Networking.
  if(m_config.localUDPPort != WINTCP_UDP_PORT_NONE)
  {
    numReceived = UDPReceive(pBuff, maxNumChars);
    if(numReceived > 0)
    {
      return(numReceived);
    }
  }

  // Now check TCP socket 
  if (m_bNewConnection == true)
  {
    m_bNewConnection = false;

    if(m_pChannelCallback != WINIOTARG_NULL)
      m_pChannelCallback(m_pCallbackParam, false, TMWDEFS_TARG_OC_NEW_CONNECTION);
    else
      close();

    numReceived = 0;
  }
  else
  {
    if(m_config.mode != WINTCP_MODE_UDP)
      numReceived = lowReceive(pBuff, maxNumChars, false);
  }

  return(numReceived);
}

/**********************************************************************************\
	Function :			WinTCPChannel::isTransmitReady
	Description : Are we ready to transmit?
	Return :			 
	Parameters :
			 
	Note : [none]
\**********************************************************************************/
TMWTYPES_MILLISECONDS WinTCPChannel::isTransmitReady(void)
{
  // We are ready as long as the remote end has not closed
  // so peek to see if socket is still open

  // There is a problem calling lowReceive. The select may say
  // there was something to read, but the recv would return 0
  // This would cause the callback function to be called, which
  // would recurse through here until the stack overflowed.
  // unsigned char buffer;
  // lowReceive(&buffer, sizeof(buffer), true);

  // If connection is not open and this is a dual end point system
  // set the CT_CHECK_CONNECT flag so the channel thread will make
  // the TCP connection. 
  // In any case if connection is not open return some delay.
  // If connected return no delay.
  if(!isChannelOpen())
  {
    if(isDualEndPoint())
    {
#if TMWTARG_SUPPORT_CHANNEL_THREAD
      m_chanThreadCheckMask |= CT_CHECK_CONNECT;
      return(1000);
#else 
      if (connect())
        return(100);
      else
        return(1000);
#endif
    }
    return(500);
  }
  return(0);
}

// Transmit bytes to this TCP connection 
bool WinTCPChannel::transmit(
  const TMWTYPES_UCHAR *pBufferToSend, 
  TMWTYPES_USHORT numCharsToSend)
{
  bool success = false;

  // synchronize with connection thread so it doesn't
  // connect or disconnect while we try to receive data
  if(isChannelOpen())
  {
    LogMessage(TMWDIAG_ID_TARGET, "TCP transmit %d bytes", numCharsToSend);

    int returnValue;

    if(!m_config.useTLS)
      returnValue = send(m_commSocket, (char*)pBufferToSend, numCharsToSend, 0);
    else
      returnValue = TLSTransmit(pBufferToSend, numCharsToSend);
     
    if(returnValue != SOCKET_ERROR)
    {
      success = true;
    }
  }

  return(success);
}

/* function: Create */
WinTCPChannel *WinTCPChannel::Create(
  const void *pConfig, 
  TMWTARG_CONFIG *pTmwTargConfig)
{
  WINTCP_CONFIG *pTCPConfig = (WINTCP_CONFIG *)pConfig;
  WinTCPChannel *pChannel = new WinTCPChannel(*pTCPConfig, pTmwTargConfig);
  pChannel->setChannelCallback(pTmwTargConfig->pChannelCallback, pTmwTargConfig->pCallbackParam);
  pChannel->m_pChannel = pTmwTargConfig->pChannel;

#if TMWTARG_SUPPORT_CHANNEL_THREAD
  if ((pTCPConfig->mode == WINTCP_MODE_CLIENT) || (pTCPConfig->mode == WINTCP_MODE_DUAL_ENDPOINT) || !pChannel->m_config.polledMode)
  {
    if (pTCPConfig->mode == WINTCP_MODE_CLIENT)
    {
      pChannel->m_chanThreadCheckMask = CT_CHECK_CONNECT;
    }
    if (!pChannel->m_config.polledMode)
    {
      pChannel->m_chanThreadCheckMask |= CT_CHECK_INPUT;
    }
    pChannel->ChannelThreadStart();
  }
#endif

  return pChannel;
}

/* function: wintcp_deleteChannel */
void WinTCPChannel::deleteWinIoChannel()
{
  closeWinIoChannel();
}

bool WinTCPChannel::modifyWinIoChannel(const void *pUserConfig)
{
  WINIO_CONFIG *tcpUserConfig = (WINIO_CONFIG *)pUserConfig;
  if (tcpUserConfig->targTCP.chnlName != NULL)
  {
    strncpy(m_config.chnlName, tcpUserConfig->targTCP.chnlName, WINIOTARG_STR_LEN);
  }
  return false;
}

/* function: wintcp_openChannel */
bool WinTCPChannel::openWinIoChannel(
   TMWTARG_CHANNEL_RECEIVE_CBK_FUNC pCallbackFunc, 
   TMWTARG_CHECK_ADDRESS_FUNC pCheckAddrCallbackFunc, 
   void *pCallbackParam, WINIO_OPEN_MODE_ENUM openMode)
{
  m_pRecvDataFunc = pCallbackFunc;
  m_pCheckAddrFunc = pCheckAddrCallbackFunc;
  m_pChanContextCBData = pCallbackParam;
  return(open());
}

/* function: wintcp_closeChannel */
void WinTCPChannel::closeWinIoChannel()
{
  close();
}

/* function: wintcp_closeChannel */
bool WinTCPChannel::resetWinIoChannel()
{
  m_bForceReset = true;
  return true;
}

/* function: wintcp_getChannelName */
const char *WinTCPChannel::getChannelName()
{
  return getName();
}

/* function: wintcp_getChannelInfo */
const char *WinTCPChannel::getChannelInfo()
{
  return getInfo();
}

// Returns true if this channel is currently open
// isChannelOpen will not call lowreceive because isTransmitReady calls it.
// lowReceive should not be called by the listenThread or it may
// call back to main thread.
bool WinTCPChannel::isChannelOpen(void)
{  
  if(m_config.mode == WINTCP_MODE_UDP)
    return(m_UDPSocket != INVALID_SOCKET); 

  return(m_commSocket != INVALID_SOCKET); 
}

/* function: wintcp_getChannelStatus */
const char *WinTCPChannel::getChannelStatus()
{
  return getStatus();
}

/* function: wintcp_getTransmitReady */
TMWTYPES_MILLISECONDS WinTCPChannel::getTransmitReady()
{
  return isTransmitReady();
}

TMWTYPES_BOOL WinTCPChannel::waitForInput()
{
	if (isChannelOpen())
	{
		fd_set readfds;

		FD_ZERO(&readfds);
#pragma warning(disable:4127)
		FD_SET(m_commSocket, &readfds);
#pragma warning(default:4127)
		struct timeval timeout = { 0, 100000 };

		int err = select(0, &readfds, WINIOTARG_NULL, WINIOTARG_NULL, &timeout);
		if (err > 0)
			return true;
	}
	else
	{
		Sleep(100);
	}

	return false;
}

/* function: wintcp_receive */
TMWTYPES_USHORT WinTCPChannel::receiveOnChannel(
  TMWTYPES_UCHAR *pBuff, 
  TMWTYPES_USHORT maxBytes, 
  TMWTYPES_MILLISECONDS maxTimeout,
  bool  *timeoutOccured)
{
  *timeoutOccured = false;
  return receive(pBuff, maxBytes);
}

/* function: wintcp_transmit */
bool WinTCPChannel::transmitOnChannel(
  TMWTYPES_UCHAR *buf, 
  TMWTYPES_USHORT numBytes)
{
  return transmit(buf, numBytes);
}


/* The rest of this file was added to support DNP3 Spec IP Networking */

/**********************************************************************************\
	Function :			WinTCPChannel::udpEndPoint
	Description : Attempt to open a socket to be used for UDP send/receive 
        To support DNP3 Spec IP Networking
	Return :			bool	-	
	Parameters :
			void	-	
	Note : [none]
\**********************************************************************************/
bool WinTCPChannel::udpEndPoint(char *pIpAddressString)
{
  SOCKET tempCommSocket;
  SOCKADDR_IN6 my_addr;

  LogMessage(TMWDIAG_ID_TARGET, "UDP: Opening UDP End Point");

  // See if this UDP end point is already open 
  if(m_UDPSocket != INVALID_SOCKET)
  {
    LogMessage(TMWDIAG_ID_TARGET, "UDP: End Point already open");
    return(true);
  }
  memset(&m_destUDPSockAddr, 0, sizeof(m_destUDPSockAddr));

  IN6_ADDR destIpAddress;
 
  bool validAddress = ConvertNameToIpAddress(pIpAddressString, m_afInet, &destIpAddress);

  if((m_afInet == AF_INET) && strchr(pIpAddressString, '.') == 0)
  {
    if (validAddress == false)
    {
      LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "UDP: failed to resolve host name: %s", pIpAddressString);
      return(false);
    }
    char addBuf[46];
    LogMessage(TMWDIAG_ID_TARGET , "UDP: Resolved host name to IP Address: %s", inet_ntop(m_afInet, &destIpAddress, addBuf, sizeof(addBuf)));
  }

  if (validAddress == false)
  {
    /* We don't have to specify the ip address of the remote system
     * for receiving from UDP and TCP. 
     * We do have to specify it for sending however, so this does not
     * work for TCP/UDP master or UDP only outstation.
     */
    destIpAddress =  in6addr_any;
  }

  /* Let UDPAddress for validation get set when connection is made */
  if(m_config.validateUDPAddress)
    memset(&m_validUDPAddress, 0xff, sizeof(m_validUDPAddress)); 

  m_destUDPSockAddr.sin6_family = m_afInet;
  m_destUDPSockAddr.sin6_port = htons(m_config.destUDPPort);
  if (m_afInet == AF_INET6)
  {
    m_destUDPSockAddr.sin6_addr = destIpAddress;
  }
  else
  {
    struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *)&m_destUDPSockAddr;
    sockaddr_ipv4->sin_addr.s_addr = *(ULONG *)&destIpAddress;
  }
   
  // Set up broadcast address to support broadcast for UDP only Master
  // This won't be used for Master TCP_UDP or Outstation but is harmless.
  ConvertNameToIpAddress(m_config.udpBroadcastAddress, m_afInet, &destIpAddress);
  memset(&m_UDPOnlyBrdcastSockAddr, 0, sizeof(m_UDPOnlyBrdcastSockAddr));
  m_UDPOnlyBrdcastSockAddr.sin6_family = m_afInet;
  m_UDPOnlyBrdcastSockAddr.sin6_port = htons(m_config.destUDPPort);
  if (m_afInet == AF_INET6)
  {
    m_UDPOnlyBrdcastSockAddr.sin6_addr = destIpAddress;
  }
  else
  {
    struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *)&m_UDPOnlyBrdcastSockAddr;
    sockaddr_ipv4->sin_addr.s_addr = *(ULONG *)&destIpAddress;
  }
   
  /* Now set up UDP socket to send and receive on. */
  tempCommSocket = socket(m_afInet, SOCK_DGRAM, 0);
  if(tempCommSocket < 0)
  {    
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "UDP failed to open socket");
    return(false);
  }

  // success,  bind to configured local UDP port.
  memset(&my_addr,0,sizeof(my_addr));
  my_addr.sin6_family = m_afInet; 
  if (m_afInet == AF_INET6)
  {
    inet_pton(my_addr.sin6_family, m_config.localIpAddress, &my_addr.sin6_addr);
  }
  else
  {
    struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *)&my_addr;
    inet_pton(my_addr.sin6_family, m_config.localIpAddress, &sockaddr_ipv4->sin_addr.s_addr);
  }
  if(m_config.localUDPPort == WINTCP_UDP_PORT_ANY)
  {
    //zero lets bind choose an available port number
    my_addr.sin6_port = 0;
  }
  else
  {
    my_addr.sin6_port = htons(m_config.localUDPPort);
  }

  // bind to assign a port to the socket 
  if (bind(tempCommSocket, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) 
  {
    WinIoTarg_formatErrorMsg(m_errorBuffer, WSAGetLastError()); 
    WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "UDP failed to bind to port %d, %s", htons(my_addr.sin6_port), m_errorBuffer);
    closesocket(tempCommSocket);
    return(false);
  }
   
  /*This is not required for enabling broadcast
   * BOOL flag = TRUE;
   * if(setsockopt(tempCommSocket, SOL_SOCKET, SO_BROADCAST, (char *)&flag, sizeof(BOOL)) != 0)
   * { 
   *  WinIoTarg_formatErrorMsg(m_errorBuffer, WSAGetLastError());   
   *  LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "UDP setsockopt failed\n%19s%s", " ",m_errorBuffer);
   *  return(false);
   * }
   */

  // Store new socket to be used for UDP
  m_UDPSocket = tempCommSocket;
  
  /* if master and not udp only, no udp reader is required */
  if(isClient() && m_config.mode != WINTCP_MODE_UDP)
  {
    LogMessage(TMWDIAG_ID_TARGET, "UDP Open End Point, udp reader not required, returned success");
    return(true);
  }

  if(!StartUDPReaderThread())
  {
    closesocket(m_UDPSocket);
    m_UDPSocket = INVALID_SOCKET;
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "UDP failed to start UDP reader thread");
    return(false);
  }
 
  if ((m_afInet == AF_INET6) && (m_config.role == WINTCP_ROLE_OUTSTATION))
  {
    struct ipv6_mreq mreq;
    mreq.ipv6mr_interface = 0;
    inet_pton(m_afInet, m_config.udpBroadcastAddress, &mreq.ipv6mr_multiaddr);
    if (setsockopt(tempCommSocket, IPPROTO_IPV6, IPV6_JOIN_GROUP, (const char *)&mreq, sizeof(mreq)) == 0)
    {
      m_UPDMulticastJoin = true;
    }
  }

  // Return success 
  LogMessage(TMWDIAG_ID_TARGET, "UDP Open End Point returned success");
  return(true);
}

/**********************************************************************************\
	Function :			WinTCPChannel::inputData
	Description : Put data from datagram into circular buffer, for reading by SCL.
        To support DNP3 Spec IP Networking 
	Return :			void	-	
	Parameters :
	Note : [none]
\**********************************************************************************/
void WinTCPChannel::inputData(TMWTYPES_UCHAR  *buf, TMWTYPES_USHORT length)
{
  int roomLeft = WINTCP_BUFFER_SIZE - m_UDPbufferWriteIndex;

  /* Is there enough room at end of buffer?  */
  if(roomLeft >= length)
  {
    memcpy(&m_UDPbuffer[m_UDPbufferWriteIndex], buf, length);
    m_UDPbufferWriteIndex += length;
    if (m_UDPbufferWriteIndex == WINTCP_BUFFER_SIZE)
    {
      m_UDPbufferWriteIndex = 0;
    }
  }
  else
  {
    memcpy(&m_UDPbuffer[m_UDPbufferWriteIndex], buf, roomLeft);
    length = (TMWTYPES_USHORT)(length - roomLeft);
    memcpy(&m_UDPbuffer[0], &buf[roomLeft], length);
    m_UDPbufferWriteIndex = length;
  }
   
  if (!m_config.polledMode)
  {
    m_pRecvDataFunc(m_pChanContextCBData);
  }
}

/**********************************************************************************\
	Function :			WinTCPChannel::StopUDPReaderThread
	Description : Stop UDP Reader thread
        To support DNP3 Spec IP Networking
	Return :			void	-	
	Parameters :
	Note : [none]
\**********************************************************************************/
void WinTCPChannel::StopUDPReaderThread()
{  
  
  // Stop reader thread 
  if (m_UDPthreadIsRunning == true)
  { 
    m_bUdpReaderThreadRun = false;

    if (WaitForSingleObject(m_udpReaderThreadHandle,5000) == WAIT_TIMEOUT)
    {
      m_UDPthreadIsRunning = false;
      WinIoTarg_endThread(m_udpReaderThreadHandle);
    }
  }
}
/**********************************************************************************\
	Function :			WinTCPChannel::StartUDPReaderThread
	Description : Start UDP Reader thread
        To support DNP3 Spec IP Networking
	Return :			bool	-	
	Parameters :
			void	-	
	Note : [none]
\**********************************************************************************/
bool WinTCPChannel::StartUDPReaderThread(void)
{

  // Start thread reading on UDP port
  if (m_UDPthreadIsRunning == false)
  {
    m_bUdpReaderThreadRun = true;

    m_udpReaderThreadHandle = (HANDLE)WinIoTarg_startThread(UDPReaderThread,this,&m_threadID, THREAD_PRIORITY_BELOW_NORMAL);
    if(m_udpReaderThreadHandle == INVALID_HANDLE_VALUE)
      return false;
  }

  return true;
}

/**********************************************************************************\
	Function :			WinTCPChannel::UDPReaderThread
	Description :  UDP reader thread
        To support DNP3 Spec IP Networking
	Return :			unsigned int __stdcall	-	
	Parameters :
			void *pParam	-	
	Note : [none]
\**********************************************************************************/
unsigned int __stdcall WinTCPChannel::UDPReaderThread(void *pParam)
{
  TMWTYPES_UCHAR rcvBuf[WINIOTARG_MAX_UDP_RCVLEN];
  WinTCPChannel *pTCPChannel = (WinTCPChannel *)pParam;

  pTCPChannel->m_UDPthreadIsRunning = true;
  
	while(pTCPChannel->m_bUdpReaderThreadRun)
  {
    fd_set readfds;
    SOCKET socket = pTCPChannel->m_UDPSocket;

    if(socket == INVALID_SOCKET)
    {
      if (pTCPChannel->m_udpReaderThreadHandle != INVALID_HANDLE_VALUE)
      {
        CloseHandle(pTCPChannel->m_udpReaderThreadHandle);
        pTCPChannel->m_udpReaderThreadHandle = INVALID_HANDLE_VALUE;
      }
      pTCPChannel->m_UDPthreadIsRunning = false;
      return(1);
    }

    FD_ZERO(&readfds);
#pragma warning(disable:4127)
    FD_SET(socket,&readfds);
#pragma warning(default:4127)
    struct timeval timeout = {1,0};

    // Call select with timeout of 0 to see if data is available
    if(select(0, &readfds, WINIOTARG_NULL,WINIOTARG_NULL, &timeout) == SOCKET_ERROR)
    {
      /* Should we exit here? */
    }
    else if(FD_ISSET(socket, &readfds))
    {
      int returnValue;
      SOCKADDR_IN6 from;
      int  fromLen  = sizeof(from);

      returnValue = recvfrom(socket, (char*)rcvBuf, WINIOTARG_MAX_UDP_RCVLEN, 0,
       (sockaddr *)&from, &fromLen);

      if(returnValue < 0) 
      {
        int temp = WSAGetLastError();
        if(temp == WSAEMSGSIZE)  
        {
          WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "UDP Error returned from recvFrom, UDP datagram received was larger than buffer size %d bytes, data discarded", WINIOTARG_MAX_UDP_RCVLEN);
          returnValue = WINIOTARG_MAX_UDP_RCVLEN;
        }
        else
        {
          char m_errorBuffer[128];
          WinIoTarg_formatErrorMsg(m_errorBuffer, temp); 
          WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "UDP Error returned from recvFrom, %s", m_errorBuffer);
        }
      }

      if(returnValue>0)
      {
        // If outstation TCP and UDP, verify the src address with TCP connection end point.
        // If outstation UDP only, verify the src address of master
        // If master UDP only, verify src address of slave.
        // These are done by setting m_validUDPAddress to appropriate address
        //   if validation is enabled.
        void *pAddr = &from.sin6_addr;
        if (from.sin6_family == AF_INET)
        {
          struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *)&from;
          pAddr = &sockaddr_ipv4->sin_addr.s_addr;
        }

        if((memcmp(&pTCPChannel->m_validUDPAddress, &in6addr_any, pTCPChannel->m_ipAddrLen) == 0)
          ||(memcmp(pAddr, &pTCPChannel->m_validUDPAddress, pTCPChannel->m_ipAddrLen) == 0))
        {
          pTCPChannel->m_srcUDPPort = from.sin6_port;
       
          // put the bytes received into a circular buffer for reading by SCL
          pTCPChannel->inputData(rcvBuf, (TMWTYPES_USHORT)returnValue);
        }
        else
        {
          char addBuf[128];
          WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "UDP data discarded from %s", inet_ntop(from.sin6_family, pAddr, addBuf, sizeof(addBuf))); 
        }
      }
    }
	}

  if (pTCPChannel->m_udpReaderThreadHandle != INVALID_HANDLE_VALUE)
  {
    CloseHandle(pTCPChannel->m_udpReaderThreadHandle);
    pTCPChannel->m_udpReaderThreadHandle = INVALID_HANDLE_VALUE;
  }
  pTCPChannel->m_UDPthreadIsRunning = false;
  return 1;
}

/**********************************************************************************\
	Function :			WinTCPChannel::setUDPDestPort
	Description :  Set UDP destination port based on parameter
        and whether or not we have an source port from a previous request
        To support DNP3 Spec IP Networking
	Return :			unsigned int __stdcall	-	
	Parameters :
    TMWTYPES_UCHAR UDPPort -
      TMWTARG_UDP_SEND       - Send to the remote port to be used for 
                               requests or responses.
      TMWTARG_UDP_SEND_UNSOL - Send to the remote port to be used for   
                               unsolicited responses. Once outstation has
                               received a request from master this would be
                               same port as all responses.
      TMWTARG_UDPONLY_BROADCAST - Send to the remote port configured and to the 
                               broadcast address when UDP ONLY is configured.
                               This is in a separate structure and not modified
                               here.
	Note : [none]
\**********************************************************************************/
void WinTCPChannel::setUDPDestPort(
  TMWTYPES_UCHAR UDPPort)
{ 
  // If we don't have a source port yet, use the configured port 
  if(m_srcUDPPort == WINTCP_UDP_PORT_NONE)
  {
    if(UDPPort == TMWTARG_UDP_SEND)
    {
      m_destUDPSockAddr.sin6_port = htons(m_config.destUDPPort); 
    }
    else if(UDPPort == TMWTARG_UDP_SEND_UNSOL)
    {
      m_destUDPSockAddr.sin6_port = htons(m_config.initUnsolUDPPort);
    }
  }
  else /* we have a source port from a previous message */
  {
    // If configuration allows it, use src port from previous request
    // This would only be allowed on an outstation.
    if(m_config.destUDPPort == WINTCP_UDP_PORT_SRC)
    {
      m_destUDPSockAddr.sin6_port = m_srcUDPPort;
    }
    else
    {
      m_destUDPSockAddr.sin6_port = htons(m_config.destUDPPort);
    }
  }
}

/**********************************************************************************\
	Function :			WinTCPChannel::transmitUDP
	Description : [none]	
	Return :			TMWTYPES_BOOL	-	
	Parameters :
			TMWTYPES_UCHAR UDPPort	-	 A define that indicates the remote UDP port to
       transmit to. 
        TMWTARG_UDP_SEND       - Send to the remote port to be used for 
                                 requests or responses
        TMWTARG_UDP_SEND_UNSOL - Send to the remote port to be used for   
                                 unsolicited responses
        TMWTARG_UDPONLY_BROADCAST Send to the broadcast address when UDP ONLY 
                                 is configured.
			TMWTYPES_UCHAR *pBuff	-	
			TMWTYPES_USHORT numBytes	-	
	Note : [none]
\**********************************************************************************/
bool WinTCPChannel::transmitUDP(
  TMWTYPES_UCHAR UDPPort,
  TMWTYPES_UCHAR *buf, 
  TMWTYPES_USHORT numBytes)
{
  setUDPDestPort(UDPPort);
        
  if((m_config.mode == WINTCP_MODE_UDP)
    && (UDPPort != TMWTARG_UDPONLY_BROADCAST))
  {
    UDPTransmitLog(TMWDIAG_ID_TARGET, "UDP transmit %d bytes", numBytes);
  }
  else
  {
    UDPBroadcastLog(TMWDIAG_ID_TARGET, "UDP broadcast %d bytes", numBytes);
  }

  // This can either be a specific or broadcast address depending on configuration
  void *pDestAddress = &m_destUDPSockAddr;
  
  // If this is a broadcast request by UDP only master use that address.
  if (UDPPort == TMWTARG_UDPONLY_BROADCAST)
    pDestAddress = &m_UDPOnlyBrdcastSockAddr;

  if(SOCKET_ERROR == sendto(m_UDPSocket, (char*)buf, numBytes, 0,
    (const sockaddr *)pDestAddress, sizeof(m_destUDPSockAddr)))
  {
    WinIoTarg_formatErrorMsg(m_errorBuffer, WSAGetLastError());  
    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "UDP sendto failed\n%19s%s", " ",m_errorBuffer);
    return(false);
  }
  
  return(true);
}

/**********************************************************************************\
	Function :			WinTCPChannel::UDPReceive
	Description : [none]	
	Return :			TMWTYPES_BOOL	-	
	Parameters :
		
	Note : [none]
\**********************************************************************************/
TMWTYPES_USHORT WinTCPChannel::UDPReceive(
  TMWTYPES_UCHAR  *pBuff, 
  TMWTYPES_ULONG  maxBytes)
{
  TMWTYPES_ULONG bytesRead = 0;
  TMWTYPES_UCHAR *readPtr = WINIOTARG_NULL;

  // See if there are any bytes in the circular buffer
  if(m_UDPbufferReadIndex == m_UDPbufferWriteIndex)
    return(0);

  readPtr = &m_UDPbuffer[m_UDPbufferReadIndex];

  if(m_UDPbufferReadIndex < m_UDPbufferWriteIndex)
  {
    bytesRead = m_UDPbufferWriteIndex - m_UDPbufferReadIndex;
    if(bytesRead > maxBytes)
    {
      bytesRead = maxBytes;
    }
    m_UDPbufferReadIndex += bytesRead;
  }
  else
  {
    bytesRead = WINTCP_BUFFER_SIZE - m_UDPbufferReadIndex;
    if(bytesRead > maxBytes)
    {
      bytesRead = maxBytes;
      m_UDPbufferReadIndex += bytesRead;
    }
    else
    {
      m_UDPbufferReadIndex = 0;
    }
  }

  memcpy(pBuff, readPtr, bytesRead);

  return((TMWTYPES_USHORT)bytesRead);
}

// This would be overridden by WinTLSChannel if TLS is being used.
bool WinTCPChannel::TLSOpen(void)
{
  return true;
}

// This would be overridden by WinTLSChannel if TLS is being used.
void WinTCPChannel::TLSClose(void) 
{
}

// This would be overridden by WinTLSChannel if TLS is being used.
bool WinTCPChannel::TLSConnect(SOCKET tempCommSocket)
{
  return true;
}

// This would be overridden by WinTLSChannel if TLS is being used.
bool WinTCPChannel::TLSListen(SOCKET acceptSocket)
{
  return true;
} 

// This would be overridden by WinTLSChannel if TLS is being used.
int WinTCPChannel::TLSTransmit(
  const TMWTYPES_UCHAR *pBufferToSend, 
  TMWTYPES_USHORT numCharsToSend)
{
  return 0;
} 

// This would be overridden by WinTLSChannel if TLS is being used.
int WinTCPChannel::TLSLowReceive(
  TMWTYPES_UCHAR *pBuff, 
  TMWTYPES_ULONG maxNumChars) 
{
  return 0;
}

// Common function for the three log functions
void WinTCPChannel::logFunction(TMWDIAG_ID sourceID, char *data, const char *format, va_list va)
{
  CriticalSectionLockWinTarg lock(getLogLock());
  WinIoTargProtoAnaLogFun(m_pChannel, TRACE_MASK_NONE, sourceID, data, format, va);
  va_end(va);
}

// Same as LogMessage but specifically for UDP broadcast to get correct address efficiently
void WinTCPChannel::UDPBroadcastLog(TMWDIAG_ID sourceID, const char *format, ...)
{
  if (WinIoTarg_IsProtoAnaLogEnabled())
  {
    if (WinIoTargProtoAnaLogFun != WINIOTARG_NULL)
    {
      char _chanID[256];
      sprintf(_chanID, "%s - %s:%d - ", this->m_config.chnlName, this->m_config.udpBroadcastAddress, this->m_config.destUDPPort);
      va_list va;
      va_start(va, format);
      logFunction(sourceID, _chanID, format, va);
      va_end(va);
    }
  }
}

// Same as LogMessage but specifically for UDP transmit to get correct port efficiently
void WinTCPChannel::UDPTransmitLog(TMWDIAG_ID sourceID, const char *format, ...)
{
  if (WinIoTarg_IsProtoAnaLogEnabled())
  {
    if (WinIoTargProtoAnaLogFun != WINIOTARG_NULL)
    {
      char _chanID[256];
      sprintf(_chanID, "%s - %s:%d - ", this->m_config.chnlName, this->m_config.ipAddress, htons(this->m_destUDPSockAddr.sin6_port));
      va_list va;
      va_start(va, format);
      logFunction(sourceID, _chanID, format, va);
      va_end(va);
    }
  }
}

void WinTCPChannel::LogMessage(TMWDIAG_ID sourceID, const char *format, ...)
{
  if (WinIoTarg_IsProtoAnaLogEnabled())
  {
    if (WinIoTargProtoAnaLogFun != WINIOTARG_NULL)
    {
      char _chanID[2048];
      sprintf(_chanID, "%s - %s:%d - ", this->m_config.chnlName, this->m_config.ipAddress, m_portNumber);
      va_list va;
      va_start(va, format);
      logFunction(sourceID, _chanID, format, va);
      va_end(va);
    }
  }
}

bool WinTCPChannel::checkReceive(TMWTYPES_MILLISECONDS timeout)
{
  fd_set readfds;
  struct timeval selTimeout = { 0, (long)timeout*1000 };
  SOCKET socket = getCommSocket();

  if (!m_config.polledMode && (socket != INVALID_SOCKET))
    {
    FD_ZERO(&readfds);
    FD_SET(m_commSocket, &readfds);

    // Call select to see if data is available
    if (select(0, &readfds, WINIOTARG_NULL, WINIOTARG_NULL, &selTimeout) == 1)
    {
      return true;
        }
      }

  return false;
}
