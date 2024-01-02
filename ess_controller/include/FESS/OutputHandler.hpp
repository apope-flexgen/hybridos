#ifndef OUTPUTHANDLER_HPP
#define OUTPUTHANDLER_HPP

#include "asset.h"
#include "formatters.hpp"
#include "FunctionUtility.hpp"


namespace OutputHandler {

    FunctionUtility::FunctionReturnObj OpenContactors(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
    FunctionUtility::FunctionReturnObj CloseContactors(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
    FunctionUtility::FunctionReturnObj StartPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
    FunctionUtility::FunctionReturnObj StopPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
    FunctionUtility::FunctionReturnObj StandbyPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
}

#endif