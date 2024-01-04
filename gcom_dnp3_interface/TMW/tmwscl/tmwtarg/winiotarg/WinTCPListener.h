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

/* file: WinTCPListener.h
 * description: This class implements a standard 
 *  TCP/IP listener that waits for connections on a specific port on the 
 *  local machine. This class is only used for servers. An instance of
 *  this class is created for each port on a server.
 *
 */
#ifndef WinTCPListener_DEFINED
#define WinTCPListener_DEFINED

#include <vector>

class WinTCPChannel;

// Define listener class used on servers to listen for new connections.
// One of these classes will be instanced for each port. This class can
// listen for connections from multiple clients
class WinTCPListener 
{
public:
  // Constructor
  WinTCPListener(TMWTYPES_USHORT port);

  // Destructor
  virtual ~WinTCPListener(void);
  
  // Return true if this listener is listening on the
  // specified port
  bool isSamePort(TMWTYPES_USHORT port);

  TMWTYPES_USHORT getPort() { return m_port; }
  
  // Add a channel from this listener
  // allowing multiple channels to receive 
  // notification that a connection has been accepted
  // This allows connections from multiple masters to a wellknown
  // listener port. 
  bool addChannelToListener(WinTCPChannel *pChannel);
  
  // Remove a channel from this listener
  void removeChannelFromListener(WinTCPChannel *pChannel);
 
  DWORD getThreadId()
  {
    return m_threadID;
  }

  int getChannelListSize(void)
  {
    return((int)m_pChannelList->size());
  }

  void forceClose(void)
  {
    closesocket(m_socket);
  }

  HANDLE m_listenThreadHandle;

  
protected:
  // Is thread currently running
  bool m_threadIsRunning;

  // Socket we are listening on
  SOCKET m_socket;
  
private:
  // Port we are listening on
  TMWTYPES_USHORT m_port;
  unsigned int m_threadID;
  
  // List of valid channels on this port
  typedef std::vector<WinTCPChannel *> WinTCPChannels;
  WinTCPChannels *m_pChannelList;

  AutoCriticalSectionWinTarg m_ChannelListLock;
  
  char   m_errorBuffer[128];
  
  // Listen thread entry point
  static unsigned int __stdcall listenThread(void *pParam);
  
};

#endif // WinTCPListener_DEFINED