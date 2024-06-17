#ifndef SITECOMMANDUTILITY_HPP
#define SITECOMMANDUTILITY_HPP

#include "asset.h"
#include "formatters.hpp"

namespace SiteCommandUtility
{
int RemoteStart(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV,
                const char* scheduledFuncName);
int RemoteStop(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV,
               const char* scheduledFuncName);
int RemoteStandby(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV,
                  const char* scheduledFuncName);
}

#endif