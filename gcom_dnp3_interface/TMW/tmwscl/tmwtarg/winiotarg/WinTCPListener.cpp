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
 * description: This class implements a standard 
 *  TCP/IP listener that waits for connections on a specific port on the 
 *  local machine. This class is only used for servers. An instance of
 *  this class is created for each port on a server.
 */
#include "StdAfx.h"

#include <vector>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "WinIoTarg/include/WinIoTarg.h"
#include "WinIoTarg/WinTCPChannel.h"
#include "WinIoTarg/WinThreading.h"


/**********************************************************************************\
	Function :			WinTCPListener::WinTCPListener
	Description : [none]	
	Return :			constructor	-	
	Parameters :
			TMWTYPES_USHORT port	-	
	Note : [none]
\**********************************************************************************/
WinTCPListener::WinTCPListener(TMWTYPES_USHORT port) :
 m_port(port),
 m_threadID(NULL),
 m_listenThreadHandle(INVALID_HANDLE_VALUE),
 m_threadIsRunning(false)
{
  m_pChannelList = new std::vector<WinTCPChannel *>;

  // Add this listener to the list of listeners
   //WinTCPChannel::getWinTCPListenerList()->appendMembers(this);
  CriticalSectionLockWinTarg lock(WinTCPChannel::getListenerListLock());
  WinTCPChannel::getWinTCPListenerList()->push_back(this);
}

/**********************************************************************************\
	Function :			WinTCPListener::~WinTCPListener
	Description : [none]	
	Return :			destructor	-	
	Parameters :
	Note : [none]
\**********************************************************************************/
 WinTCPListener::~WinTCPListener()
{
  CriticalSectionLockWinTarg lock(WinTCPChannel::getListenerListLock());
#if 1
  using namespace std;

  vector <WinTCPListener *> *v1 = WinTCPChannel::getWinTCPListenerList();
  vector <WinTCPListener *>::iterator Iter;
  for ( Iter = v1->begin( ) ; Iter != v1->end( ) ; Iter++ )
  {
    WinTCPListener *p = *Iter;
    if(p == this)
    {
      WinTCPChannel::getWinTCPListenerList()->erase(Iter);
      break;
    }
  }

#else
  // Remove this listener from the list
  for(unsigned int i = 0; i < WinTCPChannel::getWinTCPListenerList()->size(); i++)
  {
    WinTCPListener *pListen = WinTCPChannel::getWinTCPListenerList()->at(i);
    if(pListen == this)
      WinTCPChannel::getWinTCPListenerList()->erase(i);
  }
#endif

  if (m_listenThreadHandle != INVALID_HANDLE_VALUE)
  {
    CloseHandle(m_listenThreadHandle);
  }

  delete m_pChannelList;
}

/**********************************************************************************\
	Function :			WinTCPListener::isSamePort
	Description : function: WinTCPListener::isSamePort
	Return :			bool	-	
	Parameters :
			TMWTYPES_USHORT port	-	
	Note : [none]
\**********************************************************************************/
bool WinTCPListener::isSamePort(TMWTYPES_USHORT port)
{
  return(m_port == port);
}

/**********************************************************************************\
	Function :			WinTCPListener::addChannelToListener
	Description : Add another channel to listen for
	Return :			bool	-	
	Parameters :
			WinTCPChannel *pChannel	-	
	Note : [none]
\**********************************************************************************/
bool WinTCPListener::addChannelToListener(WinTCPChannel *pChannel)
{
  bool add = true;

  
  { // scope the critical section lock m_ChannelListLock
    CriticalSectionLockWinTarg lock(m_ChannelListLock);

    // See if channel is on list 
    using namespace std;
    vector <WinTCPChannel *> *v1 = m_pChannelList;
    vector <WinTCPChannel *>::iterator Iter;
    for ( Iter = v1->begin( ) ; Iter != v1->end( ) ; Iter++ )
    {
      if(*Iter == pChannel)
      {
        add = false;
        break; 
      }
    }

    // If not already on list add requested channel
    if (add) 
    {
      m_pChannelList->push_back(pChannel);
      pChannel->LogMessage(TMWDIAG_ID_TARGET, "TCP Add Channel to Listener, added channel to listener on port %d", m_port);
    }
    else
    {
      pChannel->LogMessage(TMWDIAG_ID_TARGET, "TCP Add Channel to Listener, did not add channel to listener, already added on port %d", m_port);
    }
  }

  // If thread is not running initialize and start it
  if (!m_threadIsRunning)
  {
    pChannel->LogMessage(TMWDIAG_ID_TARGET, "TCP Add Channel to Listener, starting listening thread on port: %d", m_port);

    // Open listen socket
    m_socket = socket(pChannel->m_afInet, SOCK_STREAM, IPPROTO_IP);
    if(m_socket == INVALID_SOCKET)
    {
      WinIoTarg_formatErrorMsg(m_errorBuffer, WSAGetLastError());   
      pChannel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TCP Add Channel to Listener: socket failed\n%19s%s", " ",m_errorBuffer);
      return(false);
    }

    // Bind to listen socket
    SOCKADDR_IN6 sockAddr;
    memset(&sockAddr,0,sizeof(sockAddr));

    sockAddr.sin6_family = pChannel->m_afInet;
    sockAddr.sin6_port = htons(m_port);
    sockAddr.sin6_addr = in6addr_any;

    if(bind(m_socket, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) != 0)
    { 
      WinIoTarg_formatErrorMsg(m_errorBuffer, WSAGetLastError());  
      pChannel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TCP Add Channel to Listener: bind failed\n%19s%s", " ",m_errorBuffer);
      closesocket(m_socket);
      return(false);
    }

    // Enable non blocking mode
    unsigned long blockmode = 1;
    if(ioctlsocket(m_socket, FIONBIO, &blockmode) == SOCKET_ERROR)
    { 
      WinIoTarg_formatErrorMsg(m_errorBuffer, WSAGetLastError());  
      pChannel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TCP Add Channel to Listener: ioctlsocket failed\n%19s%s", " ",m_errorBuffer);
      closesocket(m_socket);
      return(false);
    }

    // Listen
    if(listen(m_socket, 1) != 0)
    {    
      WinIoTarg_formatErrorMsg(m_errorBuffer, WSAGetLastError());     
      pChannel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TCP Add Channel to Listener: listen failed\n%19s%s", " ",m_errorBuffer);

      closesocket(m_socket);
      return(false);
    }

    // Start thread waiting for incoming connections
    // Set this here, thread may take a while to get started and we don't want to start another thread
    // by mistake. Setting this after call to startThread may be a problem, if thread starts and exits
    // quickly, we don't want to set this to threadIsRunning after exiting thread...
    this->m_threadIsRunning = true;
    m_listenThreadHandle = (HANDLE)WinIoTarg_startThread(listenThread,this,&m_threadID, THREAD_PRIORITY_BELOW_NORMAL);
    if(m_listenThreadHandle == INVALID_HANDLE_VALUE)
    {
      // if thread failed to start, clear this flag and cleanup.
      this->m_threadIsRunning = false;
      closesocket(m_socket);
      removeChannelFromListener(pChannel);
      pChannel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TCP Add Channel to Listener, startThread failed");
      return(false);
    }  
  }
  else
  {
    pChannel->LogMessage(TMWDIAG_ID_TARGET, "TCP Add Channel to Listener, did not restart listening thread on port: %d, was still running", m_port);
  }

  return(true);
}


/**********************************************************************************\
	Function :			WinTCPListener::removeChannelFromListener
	Description : Remove a channel from list of channels we are listening for
	Return :			void	-	
	Parameters :
			WinTCPChannel *pRemovingWinsktrg	-	
	Note : [none]
\**********************************************************************************/
void WinTCPListener::removeChannelFromListener(WinTCPChannel *pRemoveChannel)
{
  CriticalSectionLockWinTarg lock(m_ChannelListLock);
  using namespace std;
  vector <WinTCPChannel *> *v1 = m_pChannelList;
  vector <WinTCPChannel *>::iterator Iter;
  for ( Iter = v1->begin( ) ; Iter != v1->end( ) ; Iter++ )
  {
    if(*Iter == pRemoveChannel)
    {
      m_pChannelList->erase(Iter);
      break;
    }
  }
}

/**********************************************************************************\
	Function :			WinTCPListener::listenThread
	Description : Entry point for listening thread
	Return :			unsigned int	-	
	Parameters :
			void *pParam	-	
	Note : [none]
\**********************************************************************************/
unsigned int __stdcall WinTCPListener::listenThread(void *pParam)
{
  WinTCPListener *pWinTCPListener = (WinTCPListener *) pParam;
  pWinTCPListener->m_threadIsRunning = true;
  char   errorBuffer[128];

  WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET, "TCP LISTENER enter listenThread");

  while(pWinTCPListener->m_threadIsRunning)
  {
    fd_set readfds;
    FD_ZERO(&readfds);
#pragma warning(disable:4127)
    FD_SET(pWinTCPListener->m_socket, &readfds);
#pragma warning(default:4127)
    struct timeval timeout = {0, 100 * 1000};

    // This may block for up to the timeout value
    int result = select(0, &readfds, WINIOTARG_NULL, WINIOTARG_NULL, &timeout);

    {
      CriticalSectionLockWinTarg lock(pWinTCPListener->m_ChannelListLock);
      if(pWinTCPListener->m_pChannelList->size() == 0)
      {
        // All the listening channels must have
        // closed while the select was blocking.
        // we will exit the thread
        WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET, "TCP LISTENER: no more listening channels, exiting listenThread");
        break;
      }
    }

    if(result == SOCKET_ERROR)
    {      
      WinIoTarg_formatErrorMsg(errorBuffer, WSAGetLastError());      
      WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR,  "TCP LISTENER: select failed\n%19s%s", " ",errorBuffer);
      break;
    }

    if(FD_ISSET(pWinTCPListener->m_socket, &readfds))
    {
      SOCKADDR_IN6 remoteAddr;
      memset(&remoteAddr,0,sizeof(remoteAddr));
      int addrLength = sizeof(remoteAddr);

      WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET, "TCP LISTENER: accept incoming connection");

      // This function should not block
      // since ioctlsocket() has been called
      SOCKET acceptSocket = accept(pWinTCPListener->m_socket, (struct sockaddr *)&remoteAddr, &addrLength);

      if(acceptSocket == INVALID_SOCKET)
      {
        WinIoTarg_formatErrorMsg(errorBuffer, WSAGetLastError());  
        WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TCP LISTENER: accept failed\n%19s%s", " ",errorBuffer);
      }
      else
      {
        int loopCount;

        SOCKADDR_IN6 myAddr;
        int mySize = sizeof(myAddr);
        getsockname(acceptSocket, (struct sockaddr *)&myAddr, &mySize);

        char myIPAddress[64];
        if (myAddr.sin6_family == AF_INET6)
        {
          inet_ntop(myAddr.sin6_family, &myAddr.sin6_addr, myIPAddress, sizeof(myIPAddress));
        }
        else
        {
          struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *)&myAddr;
          inet_ntop(myAddr.sin6_family, &sockaddr_ipv4->sin_addr.s_addr, myIPAddress, sizeof(myIPAddress));
        }

        char connectIPAddress[64];
        if (remoteAddr.sin6_family == AF_INET6)
        {
          if (inet_ntop(remoteAddr.sin6_family, &remoteAddr.sin6_addr, connectIPAddress, sizeof(connectIPAddress)) == WINIOTARG_NULL)
          {
            strncpy(connectIPAddress, "???.???.???.???", sizeof(connectIPAddress));
          }
        }
        else
        {
          struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *)&remoteAddr;
          if (inet_ntop(remoteAddr.sin6_family, &sockaddr_ipv4->sin_addr.s_addr, connectIPAddress, sizeof(connectIPAddress)) == WINIOTARG_NULL)
          {
            strncpy(connectIPAddress, "???.???.???.???", sizeof(connectIPAddress));
          }
        }

        bool bConnectionAccepted = false;
        bool bAlreadyReconnected = false;
        WinTCPChannel *pChannel = WINIOTARG_NULL;
        {
          CriticalSectionLockWinTarg lock(pWinTCPListener->m_ChannelListLock);

          int connectionListSize = (int)pWinTCPListener->m_pChannelList->size();
          int i;

          // First look at channels that specify an exact match, then loop again looking at channels that allow ANY IP Address
          for(loopCount = 0; loopCount < 2; loopCount++)
          {
            for(i = 0; i < connectionListSize; i++)
            {
              pChannel = pWinTCPListener->m_pChannelList->at(i);

              // Is the channel is currently connected (open)?
              bool bChannelIsOpen  = pChannel->isChannelOpen();

              // First look at channels that specify an exact match, then loop again looking at channels that allow ANY IP Address
              const char *pAddressToCheck;
              if(loopCount == 0)
                pAddressToCheck = connectIPAddress;
              else
                pAddressToCheck = "*.*.*.*";

              // Check local and remote addresses against channel configuration
              if(pChannel->IsLocalAddrConfigured(myIPAddress) && pChannel->IsAddrConfigured(pAddressToCheck))
              {
                if (!bChannelIsOpen)
                {
                  // Disable the Nagle algorithm to make sure packets are sent in a timely fashion.
                  BOOL flag = true;
                  BOOL TLSAcceptSuccess = pChannel->TLSListen(acceptSocket);
                  if(!TLSAcceptSuccess)
                  {
                    loopCount = 40;
                  }
                  else 
                  {
                    pChannel->LogMessage(TMWDIAG_ID_TARGET, "TCP LISTENER: connection accepted\n");
                    if(setsockopt(acceptSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(BOOL)) != 0)
                    {   
                      WinIoTarg_formatErrorMsg(errorBuffer, WSAGetLastError());   
                      pChannel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TCP LISTENER: setsockopt failed\n%19s%s", " ",errorBuffer);
                      break;
                    }
                    // connection accepted so exit 'for loops'
                    bConnectionAccepted = true;
                    loopCount = 2;
                  }
                  break;
                }
                else // Already connected
                {
                  // if configured, mark this channel for disconnect. This will allow
                  // new connection to come in next time. This handles not receiving notification
                  // of disconnect from remote end, but remote end trying to reconnect.

                  // The next receive() will see that m_bNewConnection is true, 
                  // calling channel callback to inform the SCL the connection has closed with reason TMWDEFS_TARG_OC_NEW_CONNECTION
                  // The SCL will call tmwtarg_closeChannel() which will result in old connection being closed.
                  // Then the next SYN will connect and result in a channel callback indicating the new connection.
                  // If new connection comes in before next receive(), the old connection has not closed yet so same thing happens again
                  // until receive() sees bNewConnection is true.

                  // If you want to allow multiple simultaneous connections from any IP address
                  // to this particular port, this parameter should be set to FALSE
                  if(pChannel->getDisconnectMode())
                  {    
                    // we were previously connected and a new connection has come in, this might mean connection has failed, and master is reconnecting.
                    pChannel->LogMessage(TMWDIAG_ID_TARGET, "TCP LISTENER: listenThread, previously connected and a new connection has come in, disconnect old connection");
                    pChannel->setNewConnection(true);
                    loopCount = 20;
                    break;
                  }

                  // DNP3 Spec IP networking code
                  // getDisconnectMode should always return false for proper DNP networking!
                  // If dual end point master receives a connect indication and it
                  //  already has a connection, it should reject this incoming one
                  // If dual end point outstation receives a connect indication and it
                  //  already has a connection originated by master, it should reject
                  //  new connection, 
                  //  (SPM, may want to send a link status request
                  //  to see if old connection is still OK).
                  // If the old connection was originated by this outstation, disconnect
                  //  it and use the new one from the master.
                  else if((pChannel->isDualEndPoint())
                    && (pChannel->m_config.role == WINTCP_ROLE_OUTSTATION)
                    && (pChannel->m_clientSocket != INVALID_SOCKET))
                  { 

                    // Close existing connection and accept the new connection.
                    closesocket(pChannel->m_clientSocket);
                    pChannel->m_clientSocket = INVALID_SOCKET;
                    pChannel->m_commSocket = INVALID_SOCKET;
                    pChannel->m_serverSocket = acceptSocket;
                    pChannel->m_commSocket = acceptSocket;

                    // Disable the Nagle algorithm to make sure packets are sent in a timely fashion.
                    BOOL flag = TRUE;
                    if(setsockopt(acceptSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(BOOL)) != 0)
                    {   
                      WinIoTarg_formatErrorMsg(errorBuffer, WSAGetLastError());  
                      pChannel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR,  "TCP LISTENER: setsockopt 2 failed\n%19s%s", " ",errorBuffer);
                      break;
                    }
                    // connection accepted so exit 'for loops'
                    // Since we have already reconnected, replacing the existing connection with a new 
                    // one, set this boolean to avoid the callback to the SCL 
                    bAlreadyReconnected = true;
                    bConnectionAccepted = true;
                    loopCount = 2;
                    break;
                  }
                }
              }
            }
          }
        }

        if(bConnectionAccepted)
        {
          WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET, "TCP LISTENER: listenThread connection accepted from %s", connectIPAddress);
          // If configured for it, save the address from the master to
          // be used for comparing with src address in UDP requests.
          if(pChannel->m_config.validateUDPAddress)
          {
            if (remoteAddr.sin6_family == AF_INET6)
            {
              pChannel->m_validUDPAddress = remoteAddr.sin6_addr;
            }
            else
            {
              void *pAddr;
              struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *)&remoteAddr;
              pAddr = &sockaddr_ipv4->sin_addr.s_addr;
              memcpy(&pChannel->m_validUDPAddress, pAddr, pChannel->m_ipAddrLen);
            }
          }

          if(!bAlreadyReconnected)
          {
            // Tell the channel we have a new connection 
            WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET, "TCP LISTENER: listenThread Tell the channel we have a new connection from %s", connectIPAddress);
            pChannel->setSocket(acceptSocket, false);
          }
        }
        else
        {
          // No one interested so close the socket
          if(loopCount==21)
          {
            WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET, "TCP LISTENER: listenThread Disconnected old connection so it can receive next connect from %s", connectIPAddress);
          }
          else if(loopCount==41)
          {
            WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET, "TCP LISTENER: listenThread, Disconnected because SSL Accept failed from %s", connectIPAddress);
          }
          else
          {
            WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET, "TCP LISTENER: listenThread No channel waiting for connection from %s, so close the socket", connectIPAddress);
          }
          closesocket(acceptSocket);
        }
      }
    }
  }

  // do not listen for any more connections
  closesocket(pWinTCPListener->m_socket);
  pWinTCPListener->m_threadIsRunning = false;

  WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET, "TCP LISTENER: listenThread complete, do not listen for any more connections on port: %d", pWinTCPListener->getPort());
  // don't delete this WinTCPListener object, let the main thread do this

  if (pWinTCPListener->m_listenThreadHandle != INVALID_HANDLE_VALUE)
  {
    CloseHandle(pWinTCPListener->m_listenThreadHandle);
    pWinTCPListener->m_listenThreadHandle = INVALID_HANDLE_VALUE;
  }
  return 0;
}
