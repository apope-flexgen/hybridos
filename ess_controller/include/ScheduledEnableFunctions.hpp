#ifndef SCHEDULEDENABLEFUNCTIONS_HPP
#define SCHEDULEDENABLEFUNCTIONS_HPP

#include "asset.h"
#include "formatters.hpp"

namespace ScheduledEnableFunctions
{
void CloseContactorsEnable(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
void OpenContactorsEnable(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
void StartEnable(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
void StopEnable(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
void StandbyEnable(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
}

#endif