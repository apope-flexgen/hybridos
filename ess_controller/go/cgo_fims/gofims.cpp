#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "libfims.h"

#include "gofims.h"
//#include "library.h"

#define GOFIMS_DEBUG if(1)

void* FIMS_NewFims(void) 
{
  GOFIMS_DEBUG 
     std::cout << "[c++ bridge] FIMS_NewFims()\n";
  fims* p_fims = new fims();
  GOFIMS_DEBUG 
     std::cout << "[c++ bridge] FIMS_NewFims() will return pointer "
            << p_fims << std::endl;
  return p_fims;
}

// Utility function local to the bridge's implementation
fims* AsFims(void* p_fims) { return reinterpret_cast<fims*>(p_fims); }

void FIMS_Free(void * cs)
{
  free(cs);
}

void FIMS_DestroyFims(void* p_fims) {
  GOFIMS_DEBUG 
     std::cout << "[c++ bridge] FIMS_DestroyFims(" << p_fims << ")" << std::endl;
  AsFims(p_fims)->~fims();
}

//bool fims::Connect(char *process_name)
int  FIMS_ConnectFims(void* p_fims, char* process_name)
{
  GOFIMS_DEBUG 
     std::cout << "[c++ bridge] FIMS_ConnectFims(" << process_name << ")" << std::endl;
  int rc = AsFims(p_fims)->Connect(process_name);
  free((void*) process_name);
  return rc;
}
// This function will create and send a message to FIMS server
//
//bool fims::Send(const char* method, const char* uri, const char* replyto, const char* body)
int  FIMS_SendFims(void *p_fims, const char* method, const char* uri, const char* replyto, const char* body)
{
  GOFIMS_DEBUG 
     std::cout << "[c++ bridge] FIMS_SendFims("<< uri << ")" << std::endl;
  
  int rc = AsFims(p_fims)->Send(method, uri, replyto, body);
  free((void*)method);
  free((void*)uri);
  free((void*)replyto);
  free((void*)body);

  return rc;
}
//ssize_t aesSend(int sock, void* mstr, uint32_t slen) 
//bool fims::Send(const char* method, const char* uri, const char* replyto, const char* body)
int  FIMS_SendRawFims(void *p_fims, char* body, int slen)
{
  GOFIMS_DEBUG 
     std::cout << "[c++ bridge aes] FIMS_SendRawFims("<< body << ")" << std::endl;
  //int rc = 0;
  if(slen == 0)
     slen = strlen(body)+1;
  int sock = AsFims(p_fims)->get_socket();
  size_t rc = aesSend(sock, body, slen);

  //we dont need to free this
  //free((void*)body);

  return (int)rc;
}

//int  FIMS_SendRawFims(void *p_fims, std::string &body)
//{
//  GOFIMS_DEBUG 
//     std::cout << "[c++ bridge aes] FIMS_SendRawFims string("<< body << ")" << std::endl;
//  return FIMS_SendRawFims(p_fims, (char *)body.c_str());
//}

//ssize_t aesRecv(int connection, void* buffer, size_t len, int flags) 
//bool fims::Send(const char* method, const char* uri, const char* replyto, const char* body)
int  FIMS_ReceiveRawFims(void *p_fims, char* body)
{
  int slen = MAX_MESSAGE_SIZE;
  int sock = AsFims(p_fims)->get_socket();
  size_t rc = aesRecv(sock, (void *)body, slen -1, 0);
  if (rc > 0)
  {
    body[slen] = 0;  
    GOFIMS_DEBUG 
      std::cout << "[c++ bridge aes] FIMS_ReceiveRawFims("<< body << ") rc :"<< rc <<" slen :"<< slen<< std::endl;
  }

  return (int)rc;
}

void FIMS_Close(void *p_fims)
{
  AsFims(p_fims)->Close();
  GOFIMS_DEBUG 
     std::cout << "[c++ bridge] FIMS_Close" << std::endl;
  return;
}
