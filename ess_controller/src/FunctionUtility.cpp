#ifndef FUNCTIONUTILITY_CPP
#define FUNCTIONUTILITY_CPP

#include "asset.h"
#include "formatters.hpp"
#include <iostream>
#include "FunctionUtility.hpp"


namespace FunctionUtility
{


    int FunctionSharedResultFinder
    (
        int (*function)(varsmap&, varmap&, 
        const char*, fims*, assetVar*), 
        varsmap& vmap, 
        varmap& amap, 
        const char* aname, 
        fims* p_fims, 
        assetVar* aV

    ) 
    {

        //Generic call for OutputHandler -> CloseContactors, OpenContactors, StartPCS, StopPCS, StandbyPCS
        int returnValue = function(vmap, amap, aname, p_fims, aV);
        return returnValue;
    }
    

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
            //Success
            case 0:
                status = "SUCCESS";
                event = true;
                break;
            //In Progress
            case 1:
                status = "IN PROGRESS";
                break;
            //Error
            case -1:
                status = "ERROR";
                event = true;
                break;
        }

        std::string msg = fmt::format("[{}]-[{}]-[{}]-[{}]-[{}]", status, scheduledFuncName, inputHandlerFuncName, outputHandlerFuncName, infoMessage);


        if(1)FPS_PRINT_INFO("[{}]-[{}]-[{}]-[{}]-[{}]", status, scheduledFuncName, inputHandlerFuncName, outputHandlerFuncName, infoMessage);


        if(event) {
 
            if(0)FPS_PRINT_INFO("aname [{}]", cstr{aname});
            if(0)FPS_PRINT_INFO("val [{}]", aV->getdVal());
            if(0)FPS_PRINT_INFO("assetVarName [{}] ", aV->getName());
            if(0)FPS_PRINT_INFO("assetVar [{}] ", aV->getfName());
            if(0)FPS_PRINT_INFO("schedUri [{}] ", aV->comp.c_str());
            if(0)FPS_PRINT_INFO("scheduledFuncUri [{}] ", aV->getName());

            aV->sendEvent(aname, p_fims, Severity::Info, msg.c_str());

            const char* schedUri = aV->comp.c_str();
            const char* scheduledFuncUri = aV->getName();

            // /usr/local/bin/fims_send -m set -r /$$ -u /ess/sched/bms/LocalStartBMS_bms@endTime 1
            // AssetVarInfo assetVarInfo(schedUri, scheduledFuncUri, scheduledFuncUri, AssetVarValType::Double);
            auto assetVarInfo = AssetVarInfo(schedUri, scheduledFuncUri, assetVar::ATypes::AFLOAT);


            amap = PopulateAmapWithAv(vmap, amap, vm, assetVarInfo);
            amap[scheduledFuncUri]->setParam("endTime", 1);

            if (!amap[updatedAssetVarName]) return;

            amap[updatedAssetVarName]->setParam("uri", aV->getfName());
            amap[updatedAssetVarName]->setParam("every", 0.1);

        }

    }


    varmap& PopulateAmapWithAv(varsmap& vmap, varmap& amap, VarMapUtils* vm, const AssetVarInfo& info)
    {

        int ival= 0;
        double dval= 0.0;
        bool bval = false;
        char* cval = nullptr;

        char* amId = nullptr;

        if (const AssetVarInfoWithManagerId* avInfo = dynamic_cast<const AssetVarInfoWithManagerId*>(&info))  {
            amId = const_cast<char*>(avInfo->assetManagerId);
        } else {
            amId = const_cast<char*>(info.varName);
        }

        assetVar* av = vm->getVar(vmap, info.uri, info.varName);
        if (!av) {
            switch (info.type){
                case assetVar::ATypes::AINT:
                    av = vm->makeVar(vmap, info.uri, info.varName, ival);
                    break;
                case assetVar::ATypes::AFLOAT:
                    av = vm->makeVar(vmap, info.uri, info.varName, dval);
                    break;
                case assetVar::ATypes::ABOOL:
                    av = vm->makeVar(vmap, info.uri, info.varName, bval);
                    break;
                case assetVar::ATypes::ASTRING:
                    av = vm->makeVar(vmap, info.uri, info.varName, cval);
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
            amap = FunctionUtility::PopulateAmapWithAv(vmap, amap, vm, assetVarInfo);
        }
        return amap;
    } 

}



#endif
