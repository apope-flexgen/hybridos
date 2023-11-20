#ifndef FUNCTIONUTILITY_CPP
#define FUNCTIONUTILITY_CPP

#include "asset.h"
#include "formatters.hpp"
#include <iostream>
#include "FunctionUtility.hpp"
#include "InfoMessageUtility.hpp"

 extern "C++" {
    int HandleCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}

namespace FunctionUtility
{

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
    ) 
    {

        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;

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
 
            if(0)FPS_PRINT_INFO("aname [{}]", cstr{aname});
            if(0)FPS_PRINT_INFO("val [{}]", aV->getdVal());
            if(0)FPS_PRINT_INFO("assetVarName [{}] ", aV->getName());
            if(0)FPS_PRINT_INFO("assetVar [{}] ", aV->getfName());
            if(0)FPS_PRINT_INFO("schedUri [{}] ", aV->comp.c_str());
            if(0)FPS_PRINT_INFO("scheduledFuncUri [{}] ", aV->getName());

            std::string msg = InfoMessageUtility::handlerEventMessage(status, scheduledFuncName, infoMessage);
            aV->sendEvent(aname, p_fims, Severity::Info, msg.c_str());

            const char* schedUri = aV->comp.c_str();
            const char* scheduledFuncUri = aV->getName();

            // /usr/local/bin/fims_send -m set -r /$$ -u /ess/sched/bms/LocalStartBMS_bms@endTime 1
            // AssetVarInfo assetVarInfo(schedUri, scheduledFuncUri, scheduledFuncUri, AssetVarValType::Double);
            auto assetVarInfo = AssetVarInfo(schedUri, scheduledFuncUri, assetVar::ATypes::AFLOAT);


            amap = PopulateAmapWithAv(vmap, amap, vm, &assetVarInfo);


            amap[scheduledFuncUri]->setParam("endTime", 1);

            if (!amap[updatedAssetVarName]) return;

            //Setting up to reset the every to whatever the user wants or the default
            //TODO make another part of the aV called something like "schedEvery"
            double every = 0.1;
            if(amap[updatedAssetVarName]->gotParam("schedEvery")) {
                every = amap[updatedAssetVarName]->getdParam("schedEvery");
            } 
            amap[updatedAssetVarName]->setParam("uri", aV->getfName());
            amap[updatedAssetVarName]->setParam("every", every);

        }

    }


    varmap& PopulateAmapWithAv(varsmap& vmap, varmap& amap, VarMapUtils* vm, const AssetVarInfo* info)
    {

        int ival= 0;
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

    varmap& PopulateAmapWithManyAvs(varsmap& vmap, varmap& amap, VarMapUtils* vm, std::vector<AssetVarInfo>& assetVarVector)
    {
        for (const AssetVarInfo& assetVarInfo : assetVarVector) {
            amap = FunctionUtility::PopulateAmapWithAv(vmap, amap, vm, &assetVarInfo);
        }
        return amap;
    } 

    varmap& SharedAmapReset(varmap& amap, VarMapUtils* vm, const char* uiUriKey, const char* controlsUriKey, const char* verifyControlsUriKey) {
        
        //Default every for scheduled functions - TODO discuss what this number should be
        double every = 0.1;

        if(!amap[controlsUriKey]->getbParam("triggerCmd") && !amap[controlsUriKey]->getbParam("cmdSent")) {
            amap[controlsUriKey]->setParam("triggerCmd", true);
        }

        if(amap[uiUriKey]->gotParam("schedEvery")) {
            every = amap[uiUriKey]->getdParam("schedEvery");
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

    FunctionReturnObj SharedHandleCmdProcess(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, std::string controlString) {

        FunctionReturnObj returnObject; 
        int reload = 0;
        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;


        if(0)FPS_PRINT_INFO("aname [{}]", aname);


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

        std::string assetName = "bms";


        auto relname = fmt::format("{}_{}_{}", __func__, control, "bms").c_str();
        assetVar* hcAV = amap[relname];
        if (!hcAV || (reload = hcAV->getiVal()) == 0)
        {
            reload = 0;
        }


        // Phase 0 (Start)
        if(reload == 0) {
            linkVals(*vm, vmap, amap, "bms", "/reload", reload, relname);
            hcAV = amap[relname];
            hcAV->setVal(1);
            reload = 1;
            if(1)FPS_PRINT_INFO("Completed Phase 0 (Start)");
        }

        // Phase 1 (Control)
        if(reload == 1) {
            returnObject = SharedIndividualHandleCmdLogic(amap, aname, p_fims, aV, control, "1 (Control)");
            switch(returnObject.statusIndicator) {
                case SUCCESS:
                    // Successfully did the initial control but is still in progress because needs to do the VerifyControl
                    hcAV->setVal(2);
                    reload = 2;
                    if(1)FPS_PRINT_INFO("Completed Phase 1 (Control)");
                    break;
                case RESET:
                    hcAV->setVal(0);
                    if(1)FPS_PRINT_INFO("Resetting From Phase 1");
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
            if(1)FPS_PRINT_INFO("Completed Phase 2 (Trigger VerifyControl)");
        }

        // Phase 3 (VerifyControl)
        if(reload == 3) {
            returnObject = SharedIndividualHandleCmdLogic(amap, aname, p_fims, aV, verifyControl, "3 (VerifyControl)");
            switch(returnObject.statusIndicator) {
                case RESET:
                    hcAV->setVal(0);
                    if(1)FPS_PRINT_INFO("Resetting From Phase 3");
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

    FunctionReturnObj SharedIndividualHandleCmdLogic(varmap& amap, const char* aname, fims* p_fims, assetVar* aV, std::string controlString, std::string phase) {

        FunctionReturnObj returnObject; 


        asset_manager * am = aV->am;

        std::string assetName = "bms";

        std::string control = controlString;
        std::string controlSuccess = controlString + SUCCESS_STRING;
        std::string controlAlarm = controlString + ALARM_STRING;

        

        HandleCmd(*am->vmap, am->amap, "bms", p_fims, amap[control.c_str()]);
        if(!amap[control.c_str()]->getbParam("triggerCmd") && !amap[control.c_str()]->getbParam("cmdSent")) {
            if(amap[controlSuccess.c_str()]) {
                if(!(amap[controlSuccess.c_str()]->getbVal())){
                    amap[controlAlarm.c_str()]->setVal((InfoMessageUtility::controlAlarmMessage(control, assetName)).c_str());
                    returnObject.statusIndicator = FAILURE;
                    returnObject.message = InfoMessageUtility::handleCmdTimedOutFail(control, controlSuccess);
                } else {
                    returnObject.statusIndicator = SUCCESS;
                    returnObject.message = InfoMessageUtility::controlSuccessMessage(control, assetName);
                }
            } else {
                // ControlSuccess doesn't exist
            }
        } else {
            returnObject.statusIndicator = IN_PROGRESS;
            returnObject.message = InfoMessageUtility::handleCmdInProgress(control, phase);
        }

        return returnObject;
    }

}



#endif
