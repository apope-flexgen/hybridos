#ifndef FUNCTIONUTILITY_CPP
#define FUNCTIONUTILITY_CPP

#include "asset.h"
#include "formatters.hpp"
#include <iostream>
#include "FunctionUtility.hpp"
#include "InfoMessageUtility.hpp"
#include "OutputHandler.hpp"

 extern "C++" {
    int HandleCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}

namespace FunctionUtility
{

    /**
    * @brief Shared function that handles the result of all InputHandler->OutputHandler function processes
    * 
    * @param returnValue value that signifies a result 1->in progress, 0->success, -1->failure
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    * @param scheduledFuncName function on the scheduler that is the origin of the run process ex: LocalStartBMS, LocalStopPCS, etc
    * @param updatedAssetVarName the string on the /assets/X/summary page that got updated and caused the scheduled function to get scheduled ex: /assets/bms/summary/[close_contactors], /assets/pcs/summary/[start], etc
    * @param infoMessage a string message that could give any helpful information on the status of the run process
    */
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
    ) 
    {
        if(0)FPS_PRINT_INFO("{}", __func__);


        std::string status = "";
        bool event = false;

        switch(returnValue) {
            case SUCCESS:
                status = "SUCCESS";
                event = true;
                break;
            case IN_PROGRESS:
                status = "IN PROGRESS";
                if(1)FPS_PRINT_INFO("{}", (InfoMessageUtility::handlerEventMessage(status, scheduledFuncName, infoMessage)).c_str());
                break;
            case FAILURE:
                status = "FAILURE";
                event = true;
                break;
        }


        if(event) {

            //If it is an event which is either a Success or Failure -> I want it to be pulled of the scheduler immediately
 
            if(0)FPS_PRINT_INFO("aname [{}]", cstr{aname});
            if(0)FPS_PRINT_INFO("val [{}]", aV->getdVal());
            if(0)FPS_PRINT_INFO("assetVarName [{}] ", aV->getName());
            if(0)FPS_PRINT_INFO("\nassetVar [{}] \n", aV->getfName());
            if(0)FPS_PRINT_INFO("schedUri [{}] ", aV->comp.c_str());
            if(0)FPS_PRINT_INFO("scheduledFuncUri [{}] ", aV->getName());



            std::string msg = InfoMessageUtility::handlerEventMessage(status, scheduledFuncName, infoMessage);
            aV->sendEvent(aname, p_fims, Severity::Info, msg.c_str());

            PullOffScheduler(amap, aV, updatedAssetVarName);

        }

    }

    void PullOffScheduler(varmap& amap, assetVar* aV, const char* updatedAssetVarName) 
    {

        if(0)FunctionUtility::PrintAssetVar(aV, assetVar::ATypes::AFLOAT);
        aV->setParam("endTime", 1);

        if (!amap[updatedAssetVarName]) return;

        amap[updatedAssetVarName]->setParam("uri", aV->getfName());
        amap[updatedAssetVarName]->setParam("every", aV->getdParam("every"));
    }


    void SharedInputHandlerLocalFunction(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, const char* inputHandlerFuncName) {
        
        if(0)FPS_PRINT_INFO("{}", __func__);

        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;

        std::string assetName = "";
        std::string uiUriName = "";
        std::function<FunctionUtility::FunctionReturnObj(varsmap&, varmap&, const char*, fims*, assetVar*, const char*)> outputHandlerFunction;

        std::string scheduledFuncName = inputHandlerFuncName;


        if (scheduledFuncName == "LocalStartBMS") {
            assetName += "bms";
            uiUriName += "close_contactors";
            outputHandlerFunction = OutputHandler::CloseContactors;
        } else if (scheduledFuncName == "LocalStopBMS") {
            assetName += "bms";
            uiUriName += "open_contactors";
            outputHandlerFunction = OutputHandler::OpenContactors;
        } else if (scheduledFuncName == "LocalStartPCS") {
            assetName += "pcs";
            uiUriName += "start";
            outputHandlerFunction = OutputHandler::StartPCS;
        } else if (scheduledFuncName == "LocalStopPCS") {
            assetName += "pcs";
            uiUriName += "stop";
            outputHandlerFunction = OutputHandler::StopPCS;
        } else if (scheduledFuncName == "LocalStandbyPCS") {
            assetName += "pcs";
            uiUriName += "standby";
            outputHandlerFunction = OutputHandler::StandbyPCS;
        } else {
            // TODO
        }

        std::string uri = fmt::format("/assets/{}/summary", assetName);

        std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
            FunctionUtility::AssetVarInfo(uri.c_str(), uiUriName.c_str(), assetVar::ATypes::ABOOL)
        };
        amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

        //Function has been called but the assets uri hasn't been updated so it shouldn't have been called
        if(!amap[uiUriName.c_str()]->getbVal()) return;

        // "every" in /ess/sched/bms/LocalStartPCS -> "every" in /assets/pcs/summary/start
        amap[uiUriName.c_str()]->setParam("every", aV->getdParam("every"));


        FunctionUtility::FunctionReturnObj returnObject = outputHandlerFunction(vmap, amap, aname, p_fims, aV, uiUriName.c_str());
        int returnValue = returnObject.statusIndicator;
        std::string message = returnObject.message;

        FunctionUtility::FunctionResultHandler(returnValue, vmap, amap, aname, p_fims, aV, scheduledFuncName.c_str(), uiUriName.c_str(), message.c_str());

    
    }

    int SharedInputHandlerRemoteFunction(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, std::string siteUri, std::string scheduledFuncName, std::string outputHandlerFuncName) {
        
        if(0)FPS_PRINT_INFO("{}", __func__);

        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;

        std::function<FunctionUtility::FunctionReturnObj(varsmap&, varmap&, const char*, fims*, assetVar*, const char*)> outputHandlerFunction;


        if (outputHandlerFuncName == "CloseContactors") {
            outputHandlerFunction = OutputHandler::CloseContactors;
        } else if (outputHandlerFuncName == "OpenContactors") {
            outputHandlerFunction = OutputHandler::OpenContactors;
        } else if (outputHandlerFuncName == "Start") {
            outputHandlerFunction = OutputHandler::StartPCS;
        } else if (outputHandlerFuncName == "Stop") {
            outputHandlerFunction = OutputHandler::StopPCS;
        } else if (outputHandlerFuncName == "Standby") {
            outputHandlerFunction = OutputHandler::StandbyPCS;
        } else {
            // TODO
        }

    
        std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
            FunctionUtility::AssetVarInfo("/site/ess", siteUri.c_str(), assetVar::ATypes::AINT)
        };
        amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);


        // "every" in /ess/sched/bms/LocalStartPCS -> "every" in /assets/pcs/summary/start
        amap[siteUri.c_str()]->setParam("every", aV->getdParam("every"));



        FunctionUtility::FunctionReturnObj returnObject = outputHandlerFunction(vmap, amap, aname, p_fims, aV, siteUri.c_str());
        int returnValue = returnObject.statusIndicator;
        std::string message = returnObject.message;

        FunctionUtility::FunctionResultHandler(returnValue, vmap, amap, aname, p_fims, aV, scheduledFuncName.c_str(), siteUri.c_str(), message.c_str());

        return returnValue;

    }

    /**
    * @brief gives the ability to populate the amap with a singular given assetVar based on the information given in the "info" variable
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param vm  VarMapUtils pointer
    * @param info The necessary info of an assetVar for the purpose of populating it within the amap
    */
    varmap& PopulateAmapWithAv(varsmap& vmap, varmap& amap, VarMapUtils* vm, const AssetVarInfo* info)
    {

        int ival= -1;
        double dval= 0.0;
        bool bval = false;
        char* cval = nullptr;

        char* amId = nullptr;

        // if(info->uri) FPS_PRINT_INFO("info->uri [{}]", info->uri);
        // if(info->varName) FPS_PRINT_INFO("info->varName [{}]", info->varName);
        // if(info->assetManagerId) FPS_PRINT_INFO("info->assetManagerId [{}]", info->assetManagerId);
        // if(info->type) FPS_PRINT_INFO("info->type [{}]", info->type);
        

        if (info->assetManagerId)  {
            amId = const_cast<char*>(info->assetManagerId);
        } else {
            amId = const_cast<char*>(info->varName);
        }

        assetVar* av = vm->getVar(vmap, info->uri, info->varName);
        if (!av) {
            switch (info->type){
                case assetVar::ATypes::AINT:
                    av = vm->makeVar(vmap, info->uri, info->varName, ival);
                    break;
                case assetVar::ATypes::AFLOAT:
                    av = vm->makeVar(vmap, info->uri, info->varName, dval);
                    break;
                case assetVar::ATypes::ABOOL:
                    av = vm->makeVar(vmap, info->uri, info->varName, bval);
                    break;
                case assetVar::ATypes::ASTRING:
                    av = vm->makeVar(vmap, info->uri, info->varName, cval);
                    break;
                default:
                    FPS_PRINT_ERROR("Error in Switch Statement of PopulateAmapWithAv - No known type given for AV", nullptr);

            }
        }
        amap[amId] = av;

        return amap;
    } 

    /**
    * @brief allows to populate multiple assetVars within the amap at once by calling with a vector list of the desired assetVars, this will then call the PopulateAmapWithAv for each one
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param vm  VarMapUtils pointer
    * @param assetVarVector a vector list of all the assetVars that will be populated individually in the amap
    */
    varmap& PopulateAmapWithManyAvs(varsmap& vmap, varmap& amap, VarMapUtils* vm, std::vector<AssetVarInfo>& assetVarVector)
    {
        for (const AssetVarInfo& assetVarInfo : assetVarVector) {
            amap = FunctionUtility::PopulateAmapWithAv(vmap, amap, vm, &assetVarInfo);
        }
        return amap;
    } 

    /**
    * @brief Shared function for all of the OutputHandler functions for resetting certain parameters that are necessary to make the process work properly
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param uiUriKey the string for the uri from /assets/X/summary    ex: close_contactors, open_contactors, start, stop, standby
    * @param controlsUriKey the string for the uri in /ess/controls/X    ex: CloseContactors, OpenContactors, Start, etc.
    * @param verifyControlsUriKey the string for the uri in /ess/controls/X that verifies the actual control    ex: VerifyCloseContactors, VerifyStart, etc.
    */
    varmap& SharedAmapReset(varmap& amap, VarMapUtils* vm, const char* uiUriKey, const char* controlsUriKey, const char* verifyControlsUriKey) {
        
        if(0)FPS_PRINT_INFO("{}", __func__);

        //Default every for scheduled functions - TODO discuss what this number should be
        double every = 0.1;

        if((!amap[controlsUriKey]->getbParam("triggerCmd") && !amap[controlsUriKey]->getbParam("cmdSent")) || !amap[controlsUriKey]->gotParam("triggerCmd")) {
            amap[controlsUriKey]->setParam("triggerCmd", true);
        }

        if(amap[uiUriKey]->gotParam("every")) {
            every = amap[uiUriKey]->getdParam("every");
        } 
        double tNow = vm->get_time_dbl();
        double tDiff = tNow - amap[controlsUriKey]->getdParam("tLast");
        if (tDiff > (1.5 * every)) {
            amap[controlsUriKey]->setParam("tLast", vm->get_time_dbl());
        }

        tDiff = tNow - amap[verifyControlsUriKey]->getdParam("tLast");
        if (tDiff > (1.5 * every)) {
            amap[verifyControlsUriKey]->setParam("tLast", vm->get_time_dbl());
        }

        return amap;
    }

    /**
    * @brief Shared function for all OutputHandler functions which walks through 4 phases: Start, Control, Trigger VerifyControl, VerifyControl. There are two calls to HandleCmd for the control and the verify control ex: CloseContactors and VerifyCloseContactors
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    * @param controlString the string for the /ess/controls/X/[] uri    ex: CloseContactors, OpenContactors, Start, etc.
    */
    FunctionReturnObj SharedHandleCmdProcess(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, std::string controlString) {

        FunctionReturnObj returnObject; 
        int reload = 0;
        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;


        if(0)FPS_PRINT_INFO("aname [{}]", aname);

        if(0)FPS_PRINT_INFO("{}", __func__);


        // **Control** -> CloseContactors
        std::string control = controlString;
        if(0)FPS_PRINT_INFO("control [{}]", control);
        // **Control**Success -> CloseContactorsSuccess
        std::string controlSuccess = controlString + SUCCESS_STRING;
        if(0)FPS_PRINT_INFO("controlSuccess [{}]", controlSuccess);
        // Verify**Control** -> VerifyCloseContactors
        std::string verifyControl = VERIFY_STRING + controlString;
        if(0)FPS_PRINT_INFO("verifyControl [{}]", verifyControl);
        // Verify**Control**Success -> VerifyCloseContactorsSuccess
        std::string verifyControlSuccess = VERIFY_STRING + controlString + SUCCESS_STRING;
        if(0)FPS_PRINT_INFO("verifyControlSuccess [{}]", verifyControlSuccess);

        std::string controlAlarm = controlString + ALARM_STRING;
        if(0)FPS_PRINT_INFO("controlAlarm [{}]", controlAlarm);

        std::string verifyControlAlarm = VERIFY_STRING + controlString + ALARM_STRING;
        if(0)FPS_PRINT_INFO("verifyControlAlarm [{}]", verifyControlAlarm);

        //TODO make "bms" below dynamic to any aname


        auto relname = fmt::format("{}_{}_{}", __func__, control, aname).c_str();
        assetVar* hcAV = amap[relname];
        if (!hcAV || (reload = hcAV->getiVal()) == 0)
        {
            reload = 0;
        }

 
        // Phase 0 (Start)
        if(reload == 0) {
            linkVals(*vm, vmap, amap, aname, "/reload", reload, relname);
            hcAV = amap[relname];
            hcAV->setVal(1);
            reload = 1;
            if(0)FPS_PRINT_INFO("Completed Phase 0 (Start)", nullptr);
        }

        // Phase 1 (Control)
        if(reload == 1) {
            returnObject = SharedIndividualHandleCmdLogic(amap, aname, p_fims, aV, control);
            switch(returnObject.statusIndicator) {
                case SUCCESS:
                    // Successfully did the initial control but is still in progress because needs to do the VerifyControl
                    hcAV->setVal(2);
                    reload = 2;
                    if(0)FPS_PRINT_INFO("Completed Phase 1 (Control)", nullptr);
                    break;
                case RESET:
                    hcAV->setVal(0);
                    if(0)FPS_PRINT_INFO("Resetting From Phase 1", nullptr);
                default:
                    break;

            }
        }

        // Phase 2 (Trigger VerifyControl)
        if(reload == 2) {
            amap[verifyControl.c_str()]->setParam("triggerCmd", true);
            amap[verifyControl.c_str()]->setVal(0);
            hcAV->setVal(3);
            reload = 3;
            if(0)FPS_PRINT_INFO("Completed Phase 2 (Trigger VerifyControl)", nullptr);
        }

        // Phase 3 (VerifyControl)
        if(reload == 3) {

            returnObject = SharedIndividualHandleCmdLogic(amap, aname, p_fims, aV, verifyControl);
            switch(returnObject.statusIndicator) {
                case RESET:
                    hcAV->setVal(0);
                    if(0)FPS_PRINT_INFO("Resetting From Phase 3", nullptr);
                default:
                    break;

            }
        }

        int statusIndicator = returnObject.statusIndicator;
        if(statusIndicator == FAILURE || statusIndicator == SUCCESS){
            // Full reset because it's getting pulled off the scheduler
            hcAV->setVal(0);
        }

        return returnObject;
    }

    /**
    * @brief 
    * 
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    * @param controlString the string for the /ess/controls/X/[] uri **Can be either control or verifyControl**    ex: CloseContactors, VerifyCloseContactors, Start, VerifyStart, etc.
    */
    FunctionReturnObj SharedIndividualHandleCmdLogic(varmap& amap, const char* aname, fims* p_fims, assetVar* aV, std::string controlString) {

        if(0)FPS_PRINT_INFO("{}", __func__);

        FunctionReturnObj returnObject; 


        asset_manager * am = aV->am;

        std::string assetName = aname;

        std::string control = controlString;
        std::string controlSuccess = controlString + SUCCESS_STRING;
        std::string controlAlarm = controlString + ALARM_STRING;



        if(0)FPS_PRINT_INFO("control [{}]", control);
        if(0)FPS_PRINT_INFO("controlSuccess [{}]", controlSuccess);
        if(0)FPS_PRINT_INFO("controlAlarm [{}]", controlAlarm);

        
        if(!amap[control.c_str()]){
            FPS_PRINT_INFO("NO CONTROL STRING");
            returnObject.statusIndicator = FAILURE;
            return returnObject;
        }

        amap[control.c_str()]->setParam("enableAlert", true);

        // FPS_PRINT_INFO("HandleCmd");
        // FunctionUtility::PrintAssetVar(amap[control.c_str()], assetVar::ATypes::AINT);
        HandleCmd(*am->vmap, am->amap, aname, p_fims, amap[control.c_str()]);
        if(!amap[control.c_str()]->getbParam("triggerCmd") && !amap[control.c_str()]->getbParam("cmdSent")) {
            if(amap[controlSuccess.c_str()]) {
                if(!(amap[controlSuccess.c_str()]->getbVal())){
                    amap[controlAlarm.c_str()]->setVal((InfoMessageUtility::controlAlarmMessage(control, assetName)).c_str());
                    returnObject.statusIndicator = FAILURE;
                    returnObject.message = InfoMessageUtility::handleCmdTimedOutFail(control, controlSuccess);
                    returnObject.message += fmt::format(
                        "  -> Condition(s): [{}:{}] == true",
                        amap[controlSuccess.c_str()]->getfName(), 
                        amap[controlSuccess.c_str()]->getbVal()
                    );

            
                } else {
                    returnObject.statusIndicator = SUCCESS;
                    returnObject.message = InfoMessageUtility::controlSuccessMessage(control, assetName);
                }
            } else {
                // ControlSuccess doesn't exist
                returnObject.statusIndicator = IN_PROGRESS;
                returnObject.message = fmt::format(
                    "{} doesn't exist and therefore the control, {}, cannot be validated.",
                    controlSuccess,
                    control
                );
            }
        } else {
            returnObject.statusIndicator = IN_PROGRESS;
            returnObject.message = InfoMessageUtility::handleCmdInProgress(control);
        }

        return returnObject;
    }


 
    void PrintAssetVar(assetVar* aV, assetVar::ATypes assetVarValType) {
        std::string assetVarString = "";
        assetVarString += "\n{\n";
        // assetVarString += "  \"" + aV->getfName() + "\": {\n";
        if(0)FPS_PRINT_INFO("Start of PrintAssetVar");
        assetVarString += fmt::format("  \"{}\": {{\n", aV->getfName());

        switch (assetVarValType){
            case assetVar::ATypes::AINT:
                if(0)FPS_PRINT_INFO("Before iParam");
                assetVarString += fmt::format("    \"value\": {},\n", aV->getiVal());
                break;
            case assetVar::ATypes::AFLOAT:
                if(0)FPS_PRINT_INFO("Before dVal");
                assetVarString += fmt::format("    \"value\": {},\n", aV->getdVal());
                break;
            case assetVar::ATypes::ABOOL:
                if(0)FPS_PRINT_INFO("Before bVal");
                assetVarString += fmt::format("    \"value\": {},\n", aV->getbVal());
                break;
            case assetVar::ATypes::ASTRING:
                if(0)FPS_PRINT_INFO("Before cVal");
                assetVarString += fmt::format("    \"value\": \"{}\",\n", aV->getcVal());
                break;
            default:
                break;
        }

        // for (const auto& keyValuePair : paramToDataTypeDict) {
        for (auto it = paramToDataTypeDict.begin(); it != paramToDataTypeDict.end(); ++it) {

            std::string paramString = it->first;
            assetVar::ATypes paramType = it->second;

            if(0)FPS_PRINT_INFO("paramString {}", paramString);

            if(aV->gotParam(paramString.c_str())) {
                if(0)FPS_PRINT_INFO("Param was found");
                switch (paramType){
                    case assetVar::ATypes::AINT:
                        if(0)FPS_PRINT_INFO("iParam {}", aV->getiParam(paramString.c_str()));
                        assetVarString += fmt::format("    \"{}\": {}", paramString, aV->getiParam(paramString.c_str()));
                        break;
                    case assetVar::ATypes::AFLOAT:
                        if(0)FPS_PRINT_INFO("dParam {}", aV->getdParam(paramString.c_str()));
                        assetVarString += fmt::format("    \"{}\": {}", paramString, aV->getdParam(paramString.c_str()));
                        break;
                    case assetVar::ATypes::ABOOL:
                        if(0)FPS_PRINT_INFO("bParam {}", aV->getbParam(paramString.c_str()));
                        assetVarString += fmt::format("    \"{}\": {}", paramString, aV->getbParam(paramString.c_str()));
                        break;
                    case assetVar::ATypes::ASTRING:
                        if(0)FPS_PRINT_INFO("cParam {}", aV->getcParam(paramString.c_str()));
                        assetVarString += fmt::format("    \"{}\": \"{}\"", paramString, aV->getcParam(paramString.c_str()));
                        break;
                    default:
                        break;
                }

                if (std::next(it) != paramToDataTypeDict.end()) {
                    //not the last element
                    assetVarString += ",\n";
                } else {
                    //the last element
                    assetVarString += "\n";
                }
            }

        }

        assetVarString += fmt::format("  }}\n");
        assetVarString += fmt::format("}}\n\n");

        FPS_PRINT_INFO("{}", assetVarString);
    }

    std::unordered_map<std::string, std::string> controlsEnabledLogicMap = {
        {"close_contactors", ""},
        {"open_contactors", ""},
        {"start", ""},
        {"stop", ""},
        {"standby", ""}

    };

    std::string GetEnabledLogicMessage(std::string controlUri){
        return controlsEnabledLogicMap[controlUri];
    }

    void UpdateEnabledLogicMessage(std::string controlUri, std::string logicMessage){
        controlsEnabledLogicMap[controlUri] = logicMessage;
    }


}


#endif
