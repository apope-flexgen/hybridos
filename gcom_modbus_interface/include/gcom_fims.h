#pragma once
#include "gcom_iothread.h"
#include "gcom_config.h"
#include "shared_utils.h"

std::shared_ptr<IO_Fims> make_fims(struct cfg& myCfg);
bool fims_connect(struct cfg &myCfg, bool debug);
bool gcom_recv_raw_message(fims &fims_gateway, Meta_Data_Info &meta_data, void *data_buf, uint32_t data_buf_len);
bool test_fims_connect(struct cfg &myCfg, bool debug);
bool test_fims_send_uri(struct cfg &myCfg, const char* uri, const char* method, const char*body,  bool debug);
bool send_pub(fims& fims_gateway, std::string_view uri, std::string_view body) noexcept;
bool send_set(fims& fims_gateway, std::string_view uri, std::string_view body) noexcept;
bool send_post(fims& fims_gateway, std::string_view uri, std::string_view body) noexcept;
bool start_fims(std::vector<std::string>&subs, struct cfg& myCfg);
bool stop_fims(struct cfg& myCfg);
bool start_process(struct cfg& myCfg);
bool stop_process(struct cfg& myCfg);
bool parseHeader(struct cfg& myCfg, std::shared_ptr<IO_Fims> io_fims);
void emit_event(fims* pFims, const char* source, const char* message, int severity);