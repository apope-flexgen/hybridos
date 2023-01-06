#pragma once
#ifdef __cplusplus
extern "C" {
#endif

void* FIMS_NewFims();

void FIMS_DestroyFims(void* foo);

int FIMS_ConnectFims(void *p_fims, char *process_name);

void FIMS_Free(void * cs);

int  FIMS_SendFims(void *p_fims, const char* method, const char* uri, const char* replyto, const char* body);

int  FIMS_SendRawFims(void *p_fims, char* body, int slen);

int  FIMS_ReceiveRawFims(void *p_fims, char* body);

void FIMS_Close(void *p_fims);

#ifdef __cplusplus
}  // extern "C"
#endif
