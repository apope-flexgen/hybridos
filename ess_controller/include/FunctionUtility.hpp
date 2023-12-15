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


    const std::unordered_map<std::string, assetVar::ATypes> paramToDataTypeDict = {
        // /ess/controls/X
        {"checkCmdHoldTimeout", assetVar::ATypes::AINT},
        {"checkCmdTimeout", assetVar::ATypes::AINT},
        {"cmdSent", assetVar::ATypes::ABOOL},
        {"cmdVar", assetVar::ATypes::ASTRING},
        {"currCheckCmdHoldTime", assetVar::ATypes::AINT},
        {"currCheckCmdTime", assetVar::ATypes::AINT},
        {"currCmdTries", assetVar::ATypes::AINT},
        {"currSendCmdHoldTime", assetVar::ATypes::AINT},
        {"currSendCmdTime", assetVar::ATypes::AINT},
        {"enableAlert", assetVar::ATypes::ABOOL},
        {"expression", assetVar::ATypes::ASTRING},
        {"includeCurrVal", assetVar::ATypes::ABOOL},
        {"lastCmdVal", assetVar::ATypes::AINT},
        {"maxCmdTries", assetVar::ATypes::AINT},
        {"sendCmdHoldTimeout", assetVar::ATypes::AINT},
        {"sendCmdTimeout", assetVar::ATypes::AINT},
        {"tLast", assetVar::ATypes::AFLOAT},
        {"triggerCmd", assetVar::ATypes::ABOOL},
        {"useExpr", assetVar::ATypes::ABOOL},

        // /assets/X/summary/
        {"after", assetVar::ATypes::AINT},
        {"aname", assetVar::ATypes::ASTRING},
        {"debug", assetVar::ATypes::AINT},
        {"enabled", assetVar::ATypes::ABOOL},
        {"every", assetVar::ATypes::AFLOAT},
        {"for", assetVar::ATypes::AINT},
        {"name", assetVar::ATypes::ASTRING},
        {"offset", assetVar::ATypes::AINT},
        {"scaler", assetVar::ATypes::AINT},
        {"schedEvery", assetVar::ATypes::AFLOAT},
        {"type", assetVar::ATypes::ASTRING},
        {"ui_type", assetVar::ATypes::ASTRING},
        {"unit", assetVar::ATypes::ASTRING},
        {"uri", assetVar::ATypes::ASTRING},

        // /ess/sched/X
        {"active", assetVar::ATypes::ABOOL},
        {"amap", assetVar::ATypes::ABOOL},
        // {"debug", assetVar::ATypes::AINT},
        // {"enabled", assetVar::ATypes::ABOOL},
        {"endTime", assetVar::ATypes::AFLOAT},
        // {"every", assetVar::ATypes::AFLOAT},
        {"fcn", assetVar::ATypes::ASTRING},
        {"reftime", assetVar::ATypes::AFLOAT},
        {"runCnt", assetVar::ATypes::AINT},
        {"runEnd", assetVar::ATypes::AINT},
        {"runTime", assetVar::ATypes::AFLOAT},
        {"targ", assetVar::ATypes::ASTRING},
        // {"uri", assetVar::ATypes::ASTRING}


        //actions
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


    varmap& PopulateAmapWithAv(varsmap& vmap, varmap& amap, VarMapUtils* vm, const AssetVarInfo* info);

    varmap& PopulateAmapWithManyAvs(varsmap& vmap, varmap& amap, VarMapUtils* vm, std::vector<AssetVarInfo>& assetVarVector);

    varmap& SharedAmapReset(varmap& amap, VarMapUtils* vm, const char* uiUriKey, const char* controlsUriKey, const char* verifyControlsUriKey);

    FunctionReturnObj SharedHandleCmdProcess(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, std::string controlString);

    FunctionReturnObj SharedIndividualHandleCmdLogic(varmap& amap, const char* aname, fims* p_fims, assetVar* aV, std::string controlString);

    void PrintAssetVar(assetVar* aV, assetVar::ATypes assetVarValType);

}



#endif
