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
 
    struct AssetVarInfo {
        const char* uri = nullptr;
        const char* varName = nullptr;
        const char* assetManagerId = nullptr;
        assetVar::ATypes type = assetVar::ATypes::ABOOL;

        AssetVarInfo() = default;

        AssetVarInfo(const char* u, const char* vn, assetVar::ATypes t) 
            : uri(u), varName(vn), type(t) {}

        AssetVarInfo(const char* u, const char* vn, const char* amId, assetVar::ATypes t) 
            : uri(u), varName(vn), assetManagerId(amId), type(t) {}


    };

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
        const char* inputHandlerFuncName,
        std::string outputHandlerFuncName,
        const char* updatedAssetVarName,
        const char* infoMessage
    ); 

    varmap& PopulateAmapWithAv(varsmap& vmap, varmap& amap, VarMapUtils* vm, const AssetVarInfo* info);

    varmap& PopulateAmapWithManyAvs(varsmap& vmap, varmap& amap, VarMapUtils* vm, std::vector<AssetVarInfo>& assetVarVector);

    varmap& SharedAmapReset(varmap& amap, VarMapUtils* vm, const char* uiUriKey, const char* controlsUriKey, const char* verifyControlsUriKey);

    FunctionReturnObj SharedHandleCmdProcess(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, std::string controlString);

    FunctionReturnObj SharedIndividualHandleCmdLogic(varmap& amap, const char* aname, fims* p_fims, assetVar* aV, std::string controlString, std::string phase);

    
}



#endif
