/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
#include <napi.h>
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include "libfims.h"

class napi_fims : public Napi::ObjectWrap<napi_fims>
{
public:
    static Napi::Object Init (Napi::Env env, Napi::Object exports);
    napi_fims(const Napi::CallbackInfo& info);

private:
    static Napi::FunctionReference constructor;
    Napi::Value IsConnected(const Napi::CallbackInfo& info);
    Napi::Value Connect(const Napi::CallbackInfo& info);
    Napi::Value Subscribe(const Napi::CallbackInfo& info);
    void Close(const Napi::CallbackInfo& info);
    Napi::Value Send(const Napi::CallbackInfo& info);
    Napi::Value Receive(const Napi::CallbackInfo& info);
    Napi::Value ReceiveTimeout(const Napi::CallbackInfo& info);
    fims* p_fims;
};


class napi_fims_message : public Napi::ObjectWrap<napi_fims_message>
{
public:
    static Napi::Object Init (Napi::Env env, Napi::Object exports);
    napi_fims_message(const Napi::CallbackInfo& info);
    void set_values(const char* method, const char* uri, const char* replyto, const char* body, const char* username);

private:
    static Napi::FunctionReference constructor;
    Napi::Value get_method(const Napi::CallbackInfo& info);
    Napi::Value get_uri(const Napi::CallbackInfo& info);
    Napi::Value get_replyto(const Napi::CallbackInfo& info);
    Napi::Value get_body(const Napi::CallbackInfo& info);
    Napi::Value get_username(const Napi::CallbackInfo& info);

    std::string m_method;
    bool m_method_null;
    std::string m_uri;
    bool m_uri_null;
    std::string m_replyto;
    bool m_replyto_null;
    std::string m_body;
    bool m_body_null;
    std::string m_username;
    bool m_username_null;
};
