#ifndef INPUTHANDLER_HPP
#define INPUTHANDLER_HPP

#include "asset.h"
#include "formatters.hpp"


namespace InputHandler
{
    void LocalStartBMS(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*aV);
    void LocalStopBMS(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*aV);
    void LocalStartPCS(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*aV);
    void LocalStopPCS(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*aV);
    void LocalStandbyPCS(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*aV);
    void SiteRunCmd(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*aV);
    void SiteBMSContactorControl(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
    void SitePCSStatusControl(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
    void BatteryRackBalanceCoarse(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);

}
 
#endif