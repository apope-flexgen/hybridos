#ifndef INPUTHANDLER_HPP
#define INPUTHANDLER_HPP

#include "asset.h"
#include "formatters.hpp"


namespace InputHandler
{
    int RemoteStart(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, const char* scheduledFuncName);
    int RemoteStop(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, const char* scheduledFuncName);
    int RemoteStandby(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, const char* scheduledFuncName);
}
 
#endif