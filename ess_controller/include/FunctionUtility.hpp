#ifndef FUNCTIONUTILITY_HPP
#define FUNCTIONUTILITY_HPP

#include "asset.h"
#include "assetVar.h"
#include "formatters.hpp"
#include <iostream>

#define FAILURE -1
#define IN_PROGRESS 1
#define SUCCESS 0
#define RESET 2

#define SUCCESS_STRING "Success"
#define VERIFY_STRING "Verify"
#define ALARM_STRING "_ALARM"

namespace FunctionUtility
{

    struct FunctionReturnObj {
        int statusIndicator = 1;
        std::string message = "";
        
        FunctionReturnObj() = default;

        FunctionReturnObj(int status, std::string infoMessage) 
            : statusIndicator(status), message(infoMessage) {}
    };


    void FunctionResultHandler
    (
        int returnValue, 
        varsmap& vmap, 
        varmap& amap, 
        const char* aname, 
        fims* p_fims, 
        assetVar* aV,
        const char* scheduledFuncName,
        const char* updatedAssetVarName,
        const char* infoMessage
    ); 

    void PullOffScheduler(varmap& amap, assetVar* aV, const char* updatedAssetVarName);

    void SharedInputHandlerLocalFunction(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, const char* scheduledFuncName);

    int SharedInputHandlerRemoteFunction(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, std::string siteUri, std::string scheduledFuncName, std::string outputHandlerFuncName);

    varmap& SharedAmapReset(varmap& amap, VarMapUtils* vm, const char* uiUriKey, const char* controlsUriKey, const char* verifyControlsUriKey);

    FunctionReturnObj SharedHandleCmdProcess(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, std::string controlString);

    FunctionReturnObj SharedIndividualHandleCmdLogic(varmap& amap, const char* aname, fims* p_fims, assetVar* aV, std::string controlString);

}



#endif
