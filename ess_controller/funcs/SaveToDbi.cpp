#ifndef SAVETODBI_CPP
#define SAVETODBI_CPP

#include "asset.h"
#include "formatters.hpp"
#include "ess_utils.hpp"

extern "C++"
{
    int SaveToDbi(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*av);
}

/**
 * @brief Initializes the parameters to be used for running system command
 * 
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param p_fims the fims gateway used to send message out
 * @param aV the assetVar to send value out
 */
int SaveToDbi(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av)
{
    // msg bufs should be ok, if they overflow we can check stuff to be sure.
    // if that happens then just switch to fmt::format (with std::string)
    fmt::basic_memory_buffer<char, 255> uri_buf;
    fmt::basic_memory_buffer<char, 300> msg_buf;

    uri_buf.clear();

    fmt::format_to(std::back_inserter(uri_buf), "/dbi/ess_controller/saved_registers{}/{}", Av->comp,Av->name);
    uri_buf.push_back('\0'); 
    fmt::format_to(std::back_inserter(msg_buf), "{:c}", *Av);
    msg_buf.push_back('\0'); 

    if(!p_fims->Send("set", uri_buf.data(), nullptr, msg_buf.data()))
    {
        return -1; // couldn't send fims_msg
    }
    return 0; // no error
}

#endif