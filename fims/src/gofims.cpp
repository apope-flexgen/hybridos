#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "libfims_internal.hpp"

#include "gofims.h"
//#include "library.h"

#define GOFIMS_DEBUG if(0)

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

int  FIMS_ConnectFims(void* p_fims, const char* process_name)
{
  GOFIMS_DEBUG 
     std::cout << "[c++ bridge] FIMS_ConnectFims(" << process_name << ")" << std::endl;
  int rc = AsFims(p_fims)->Connect(process_name);
  free((void*) process_name);
  return rc;
}

// This function will create and send a message to FIMS server
int  FIMS_SendFims(void *p_fims, const char* method, const char* uri, const char* replyto, const char* body, const char* username)
{
  GOFIMS_DEBUG 
     std::cout << "[c++ bridge] FIMS_SendFims("<< uri << ")" << std::endl;
  
  int rc = AsFims(p_fims)->Send(method, uri, replyto, body, username);
  free((void*)method);
  free((void*)uri);
  free((void*)replyto);
  free((void*)body);
  free((void*)username);

  return rc;
}

void FIMS_Close(void *p_fims)
{
  AsFims(p_fims)->Close();
  GOFIMS_DEBUG 
     std::cout << "[c++ bridge] FIMS_Close" << std::endl;
  return;
}
