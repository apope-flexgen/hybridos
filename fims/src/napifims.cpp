/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include "napifims.h"

Napi::FunctionReference napi_fims::constructor;
Napi::FunctionReference napi_fims_message::constructor;

Napi::Object napi_fims_message::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "napi_fims_message", {
      InstanceMethod("getMethod", &napi_fims_message::get_method),
      InstanceMethod("getUri", &napi_fims_message::get_uri),
      InstanceMethod("getReplyTo", &napi_fims_message::get_replyto),
      InstanceMethod("getBody", &napi_fims_message::get_body),
      InstanceMethod("getUsername", &napi_fims_message::get_username)
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("napi_fims_message", func);
    return exports;
}

napi_fims_message::napi_fims_message(const Napi::CallbackInfo& info) : Napi::ObjectWrap<napi_fims_message>(info) // @suppress("Class members should be properly initialized")
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    m_method_null = m_uri_null = m_replyto_null = m_body_null = m_username_null = true;
}


Napi::Value napi_fims_message::get_method(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    if(m_method_null)
        return Napi::Value::From(env, NULL);
    return Napi::String::New(env, m_method);
}

Napi::Value napi_fims_message::get_uri(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    if(m_uri_null)
        return Napi::Value::From(env, NULL);
    return Napi::String::New(env, m_uri);
}

Napi::Value napi_fims_message::get_replyto(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    if(m_replyto_null)
        return Napi::Value::From(env, NULL);
    return Napi::String::New(env, m_replyto);
}

Napi::Value napi_fims_message::get_body(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    if(m_body_null)
        return Napi::Value::From(env, NULL);
    return Napi::String::New(env, m_body);
}

Napi::Value napi_fims_message::get_username(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    if(m_username_null)
        return Napi::Value::From(env, NULL);
    return Napi::String::New(env, m_username);
}

void napi_fims_message::set_values(const char* method, const char* uri, const char* replyto, const char* body, const char* username)
{
    if(method != NULL)
        m_method = method;
    m_method_null = (method == NULL);

    if(uri != NULL)
        m_uri = uri;
    m_uri_null = (uri == NULL);

    if(replyto != NULL)
        m_replyto = replyto;
    m_replyto_null = (replyto == NULL);

    if(body != NULL)
        m_body = body;
    m_body_null = (body == NULL);

    if(username != NULL)
        m_username = username;
    m_username_null = (username == NULL);
}

Napi::Object napi_fims::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "napi_fims", {
      InstanceMethod("isConnected", &napi_fims::IsConnected),
      InstanceMethod("connect", &napi_fims::Connect),
      InstanceMethod("subscribe", &napi_fims::Subscribe),
      InstanceMethod("close", &napi_fims::Close),
      InstanceMethod("send", &napi_fims::Send),
      InstanceMethod("receive", &napi_fims::Receive),
      InstanceMethod("receiveTimeout", &napi_fims::ReceiveTimeout)
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("napi_fims", func);
    return exports;
}

napi_fims::napi_fims(const Napi::CallbackInfo& info) : Napi::ObjectWrap<napi_fims>(info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    p_fims = new fims();
}

Napi::Value napi_fims::IsConnected(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    return Napi::Boolean::New(env, p_fims->Connected());
}

Napi::Value napi_fims::Connect(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);


    std::string process_name = "Unknown Node Module";
    if(info[0].IsString() && info.Length() > 0)
    {
        process_name = info[0].As<Napi::String>().Utf8Value();
    }

    return Napi::Boolean::New(env, p_fims->Connect((char *)process_name.c_str()));
}

void napi_fims::Close(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    p_fims->Close();
}

Napi::Value napi_fims::Send(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if(info.Length() != 5 || !info[0].IsString() || !info[1].IsString() ||
      !(info[2].IsString() || info[2].IsNull()) || !(info[3].IsString() || info[3].IsNull()) || !(info[4].IsString() || info[4].IsNull()))
    {
        Napi::TypeError::New(env, "Invalid Input").ThrowAsJavaScriptException();
    }

    std::string method = info[0].As<Napi::String>().Utf8Value();
    std::string uri = info[1].As<Napi::String>().Utf8Value();

    std::string replyto;
    if(!info[2].IsNull())
        replyto = info[2].As<Napi::String>().Utf8Value();

    std::string body;
    if(!info[3].IsNull())
        body = info[3].As<Napi::String>().Utf8Value();

    std::string username;
    if(!info[4].IsNull())
        username = info[4].As<Napi::String>().Utf8Value();

    bool rtn_val = p_fims->Send(method.c_str(), uri.c_str(),
                                info[2].IsNull() ? NULL : replyto.c_str(),
                                info[3].IsNull() ? NULL : body.c_str(),
                                info[4].IsNull() ? NULL : username.c_str());
    return Napi::Boolean::New(env, rtn_val);
}

Napi::Value napi_fims::Receive(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if(info.Length() != 1 || !info[0].IsObject())
    {
        Napi::TypeError::New(env, "Need to pass fims_message").ThrowAsJavaScriptException();
    }

    napi_fims_message *nmsg = Napi::ObjectWrap<napi_fims_message>::Unwrap(info[0].As<Napi::Object>());

    fims_message* msg = p_fims->Receive();

    if(msg != NULL)
    {
        nmsg->set_values(msg->method, msg->uri, msg->replyto, msg->body, msg->username);
        p_fims->free_message(msg);
    }
    else
        nmsg->set_values(NULL, NULL, NULL, NULL, NULL);
    return Napi::Boolean::New(env, true);
}

Napi::Value napi_fims::ReceiveTimeout(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if(info.Length() != 2 || !info[0].IsObject() || !info[1].IsNumber())
    {
        Napi::TypeError::New(env, "Invalid input").ThrowAsJavaScriptException();
    }

    napi_fims_message *nmsg = Napi::ObjectWrap<napi_fims_message>::Unwrap(info[0].As<Napi::Object>());

    fims_message* msg = p_fims->Receive_Timeout((int)info[1].ToNumber().Int32Value());

    if(msg != NULL)
    {
        nmsg->set_values(msg->method, msg->uri, msg->replyto, msg->body, msg->username);
        p_fims->free_message(msg);
    }
    else
        nmsg->set_values(NULL, NULL, NULL, NULL, NULL);
    return Napi::Boolean::New(env, true);
}

Napi::Value napi_fims::Subscribe(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if(info.Length() <= 0)
    {
        Napi::TypeError::New(env, "Invalid input").ThrowAsJavaScriptException();
    }
    int count = info.Length();
    char** uris = new char*[count];
    for(int i = 0; i < count; i++)
    {
        if(!info[i].IsString())
        {
            for(int j = 0; j < i; j++)
                free(uris[j]);
            Napi::TypeError::New(env, "Invalid input").ThrowAsJavaScriptException();
        }
        uris[i] = strdup(info[i].As<Napi::String>().Utf8Value().c_str());
    }

    Napi::Boolean returnValue = Napi::Boolean::New( env, p_fims->Subscribe((const char **)uris, count));
    for(int i = 0; i < count; i++)
        free(uris[i]);
    return returnValue;
}

Napi::Object InitAll (Napi::Env env, Napi::Object exports)
{
    napi_fims::Init(env, exports);
    return napi_fims_message::Init(env, exports);
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitAll)
