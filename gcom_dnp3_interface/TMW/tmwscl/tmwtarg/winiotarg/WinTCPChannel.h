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

/* file: WinTCPChannel.h
 * description: Implementation of Windows TCP I/O Target interface. 
 * TCP/IP Channel implementation. This class is used for clients and
 * servers. A channel is a specific IP:Port to IP:Port connection. A
 * client will simply try to connect to the specified server. A server
 * will instance a WinTCPListener class to listen for connections from
 * one or more clients.
 */
#ifndef WinTCPChannel_DEFINED
#define WinTCPChannel_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwtypes.h"
#include "tmwscl/utils/tmwphys.h"
#include "tmwscl/utils/tmwtarg.h"
#include "WinIoTarg/include/WinIoTargDefs.h"
#include "WinIoTarg/WinIoInterface.h"
#include "WinIoTarg/WinTCPListener.h"
#include "WinIoTarg/WinThreading.h"


#if TMWCNFG_USE_OPENSSL
#include "openssl/ssl.h" 
#include <openssl/err.h>  
#include <openssl/conf.h>
#include <openssl/engine.h>
#endif

/* This indicates max number of received UDP Octets that can be buffered on a channel.
 * This must be >= WINIOTARG_MAX_UDP_RCVLEN */
#define WINTCP_BUFFER_SIZE         5000

// Number of TCP channels to support on a single CFI listener thread.
#define NumberOfChannelsPerCFIThread 16

typedef std::vector<char *> IPAddressList;

class WinTCPIpAddressList{ 

  // List of source ipAddresses allowed  
  IPAddressList *m_ipAddressList;

public:

  WinTCPIpAddressList();
  ~WinTCPIpAddressList();

  bool Add(const char *pAddress);

  bool IsAddrConfigured(ADDRESS_FAMILY afInet, const char *pAddress);

  char *GetFirstAddress();
};
  
class WinTCPChannel : public WinIoInterface
{
friend class WinTCPListener;

public:
  // Constructor
  WinTCPChannel() {};
  WinTCPChannel(const WINTCP_CONFIG &config, TMWTARG_CONFIG *pTmwTargConfig);
  
  // Desctructor
  ~WinTCPChannel(void);
  
  // See if local ip addess is configured for this channel
  bool IsLocalAddrConfigured(const char *pAddress);
  
  // See if this ip addess is configured for this channel
  bool IsAddrConfigured(const char *pConnectAddress);
  
  // Set close callbacks
  void setChannelCallback(
    TMWTARG_CHANNEL_CALLBACK_FUNC pChannelCallback, 
    void *pCallbackParam);

  // Return name of this channel
  const char   *getName(void);
  
  // Return info about this channel
  const char   *getInfo(void);

  // Get channel status as a string
  const char   *getStatus(void);

  // Return IP address for this channel
  const char   *getIPAddress(void)
  { 
    return(m_config.ipAddress); 
  }

  const char *getLocalIPAddress(void)
  {
	return(m_config.localIpAddress);
  }

  // Return whether or not to disconnect because of new connect indication 
  bool   getDisconnectMode(void)
  { 
    return(m_config.disconnectOnNewSyn != 0 ? true : false); 
  }

  void setSocket(SOCKET commSocket, bool isClient);
  
  SOCKET getCommSocket()
  {
    return m_commSocket;
  }

  // Open this channel
  bool open();
  
  // Close this channel
  void close(void);
  
  // Receive up to maxBytes bytes from this channel
  TMWTYPES_USHORT receive(
    TMWTYPES_UCHAR *buf, 
    TMWTYPES_ULONG maxBytes);
  
  // Transmit numBytes bytes on this channel
  bool transmit(
    const TMWTYPES_UCHAR *buf, 
    TMWTYPES_USHORT numBytes);
  
  // Returns 0 if transmit is ready, else number of milliseconds to wait
  // before asking again.
  TMWTYPES_MILLISECONDS isTransmitReady(void);

  void setNewConnection(bool b)
  {
    m_bNewConnection = b;
  }

  // Store bytes received from UDP port in circular buffer 
  // for reading by the SCL
  void inputData(
    TMWTYPES_UCHAR  *buf, 
    TMWTYPES_USHORT length);
 
  // Get received UDP bytes out of circular buffer
  TMWTYPES_USHORT UDPReceive(
    TMWTYPES_UCHAR  *pBuff, 
    TMWTYPES_ULONG  maxBytes); 
  
  // Determine what destination port to use based
  // on UDPPort and configuration and depending on
  // configuration whether a request 
  // has been received (if this is outstation)
  void setUDPDestPort(
    TMWTYPES_UCHAR UDPPort);

  void StopUDPReaderThread(void);
  bool StartUDPReaderThread(void);

  // Should reader thread run
  bool m_bUdpReaderThreadRun;

  virtual void LogMessage(TMWDIAG_ID sourceID, const char   *format, ...);
  static bool ConvertNameToIpAddress(char *pName, int addrFamily, void* addrPtr);

  void UDPBroadcastLog(TMWDIAG_ID sourceID, const char   *format, ...);
  void UDPTransmitLog(TMWDIAG_ID sourceID, const char   *format, ...);
  void logFunction(TMWDIAG_ID sourceID, char *data, const char *format, va_list va);

protected:
  // Current configuration
  WINTCP_CONFIG m_config;

  bool m_bNewConnection;
  bool m_bForceReset;

  char   m_errorBuffer[128];

  // This will be equal to either m_clientSocket, or m_serverSocket depending
  // on which connection is open. Dual End Point systems contain both client
  // and server, but only one connection is allowed at a time.
  SOCKET m_commSocket;  

  // Socket, will be equal to INVALID_SOCKET on TCP client if connection 
  // was not originated from this end
  SOCKET m_clientSocket;

  // Socket, will be equal to INVALID_SOCKET on TCP server if connection 
  // has not been originated from remote end. Even though listening this will 
  // still be INVALID_SOCKET
  SOCKET m_serverSocket;

  // Close callback specified in open, will be called whenever we
  // detect an open or close from the remote connection.
  void *m_pCallbackParam;
  TMWTARG_CHANNEL_CALLBACK_FUNC m_pChannelCallback;


private:

  typedef std::vector<WinTCPListener *> WinTCPListeners;
  static WinTCPListeners *m_pWinTCPListenerList;
  static AutoCriticalSectionWinTarg m_listenerListCrit;

  static std::vector<WinTCPListener *> *getWinTCPListenerList() 
  {
    WINIO_ASSERT(m_pWinTCPListenerList != WINIOTARG_NULL);
    return m_pWinTCPListenerList; 
  }

  static AutoCriticalSectionWinTarg &getListenerListLock()
  {
    return m_listenerListCrit;
  }

  // List of source ipAddresses allowed 
  WinTCPIpAddressList ipAddressList;
  ADDRESS_FAMILY m_afInet;
  int m_ipAddrLen;

  // Destination address for UDP. Could be specific or broadcast depending on configuration
  SOCKADDR_IN6 m_destUDPSockAddr;

  // Destination broadcast address for UDP only DNP Master
  SOCKADDR_IN6 m_UDPOnlyBrdcastSockAddr;

  // Debug/diagnostics info
  char   *m_name;
  TMWTYPES_USHORT m_portNumber;
  char   m_info[64];
  char   m_status[64];
  
  // Attempt to connect to remote server
  bool connect(void);

  
  // Listen for connections from clients
  bool listen(void);
  
  // Receive bytes
  TMWTYPES_USHORT lowReceive(
    TMWTYPES_UCHAR *buff, 
    TMWTYPES_ULONG maxBytes, 
    bool peekOnly);
  
  // Return true if this connection is a server
  bool isServer(void) 
  { 
    return(m_config.mode == WINTCP_MODE_SERVER); 
  }

  // Return true if this connection is a client
  bool isClient(void) 
  { 
    return(m_config.mode == WINTCP_MODE_CLIENT); 
  }

  // Return true if this device supports dual end points
  // as defined in DNP3 Specification IP Networking
  bool isDualEndPoint(void) 
  { 
    return(m_config.mode == WINTCP_MODE_DUAL_ENDPOINT); 
  }


  // UDP related

  // Attempt to attach to socket for UDP send and receive
  bool udpEndPoint(char *pIpAddressString);

  // Is UDP thread currently running
  bool m_UDPthreadIsRunning;
  unsigned int m_threadID;
  HANDLE m_udpReaderThreadHandle;

  // Buffer to hold received UDP octets
  TMWTYPES_UCHAR m_UDPbuffer[WINTCP_BUFFER_SIZE];
  int m_UDPbufferWriteIndex;
  int m_UDPbufferReadIndex;
  
  SOCKET m_UDPSocket;
  TMWTYPES_USHORT m_srcUDPPort;  
  IN6_ADDR m_validUDPAddress;
  bool m_UPDMulticastJoin;

  // Reader thread entry point
  static unsigned int __stdcall UDPReaderThread(void *pParam);

public: // Interface
  static WinTCPChannel *Create(
    const void *pConfig, 
    TMWTARG_CONFIG *pTmwTargConfig);

  virtual void deleteWinIoChannel();

  virtual bool modifyWinIoChannel(const void *pUserConfig);
  
  virtual bool openWinIoChannel(TMWTARG_CHANNEL_RECEIVE_CBK_FUNC pCallbackFunc, 
    TMWTARG_CHECK_ADDRESS_FUNC pCheckAddrCallbackFunc, void *pCallbackParam, WINIO_OPEN_MODE_ENUM openMode);

  virtual bool isChannelOpen();
  
  virtual void closeWinIoChannel();
  virtual bool resetWinIoChannel();

  virtual const char   *getChannelName();

  virtual const char   *getChannelInfo();

  virtual const char   *getChannelStatus();

  virtual TMWTYPES_MILLISECONDS getTransmitReady();

	virtual TMWTYPES_BOOL waitForInput();

  virtual TMWTYPES_USHORT receiveOnChannel(
    TMWTYPES_UCHAR *pBuff, 
    TMWTYPES_USHORT maxBytes, 
    TMWTYPES_MILLISECONDS maxTimeout,
    bool  *timeoutOccured);

  /* Transmit over TCP connection */
  virtual bool transmitOnChannel(
    TMWTYPES_UCHAR *buf, 
    TMWTYPES_USHORT numBytes);
  
  /* Transmit using UDP Datagram */
  virtual bool transmitUDP(
    TMWTYPES_UCHAR UDPort,
    TMWTYPES_UCHAR *buf, 
    TMWTYPES_USHORT numBytes);

  virtual bool checkReceive(TMWTYPES_MILLISECONDS timeout);

  // These are virtual methods that are overridden by WinTLSChannel if using TLS
  virtual bool TLSConnect(SOCKET tempCommSocket);
  virtual bool TLSListen(SOCKET acceptSocket);
  virtual bool TLSOpen();
  virtual void TLSClose(void);
  virtual int TLSTransmit(const TMWTYPES_UCHAR *buf, TMWTYPES_USHORT numBytes);
  virtual int TLSLowReceive(TMWTYPES_UCHAR *buff, TMWTYPES_ULONG maxBytes);

};

#endif // WinTCPChannel_DEFINED 
