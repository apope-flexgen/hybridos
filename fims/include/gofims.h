#pragma once
#ifdef __cplusplus
extern "C" {
#endif

void* FIMS_NewFims();

void FIMS_DestroyFims(void* foo);

int FIMS_ConnectFims(void *p_fims, char *process_name);

void FIMS_Free(void * cs);

int  FIMS_SendFims(void *p_fims, const char* method, const char* uri, const char* replyto, const char* body, const char* username);

int  FIMS_SendFims(void *p_fims, const char* method, std::size_t method_size, const char* uri, std::size_t uri_size, const char* replyto, std::size_t replyto_size, const char* body, std::size_t body_size);

void FIMS_Close(void *p_fims);

#ifdef __cplusplus
}  // extern "C"
#endif
