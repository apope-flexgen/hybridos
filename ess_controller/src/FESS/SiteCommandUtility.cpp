#ifndef SITECOMMANDUTILITY_CPP
#define SITECOMMANDUTILITY_CPP

#include "asset.h"
#include "formatters.hpp"
#include <iostream>
#include "FESS/FunctionUtility.hpp"
#include "FESS/InfoMessageUtility.hpp"
#include "FESS/OutputHandler.hpp"

 extern "C++" {
    int HandleCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}

namespace SiteCommandUtility
{

// ==================== Remote Functions ====================

    /**
    * @brief Internal function called by SiteRunCmd to Start a BESS block
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    * @param scheduledFuncName the name of the function that RemoteStart was called by (currently only SiteRunCmd)
    */
    int RemoteStart(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, const char* scheduledFuncName) 
    {
       
        if(0)FPS_PRINT_INFO("{}", __func__);


        int reload = 0;
        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;

        std::string siteUri = "site_status_control_command";

        auto relname = fmt::format("{}_{}", __func__, "ess").c_str() ;
        assetVar* hpAv = amap[relname];
        if (!hpAv || (reload = hpAv->getiVal()) == 0)
        {
            reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
        }


        int returnValue = IN_PROGRESS; 

        if(reload == 0){
            linkVals(*vm, vmap, amap, aname, "/reload", reload, relname);
            hpAv = amap[relname];
            reload = 1;
            if(0)FPS_PRINT_INFO("Completed Phase 0", nullptr);
        }

        if(reload == 1) {
            returnValue = FunctionUtility::SharedInputHandlerRemoteFunction(vmap, amap, aname, p_fims, aV, siteUri, "SiteRunCmd", "CloseContactors");
            if(returnValue == SUCCESS) {
                aV->setParam("endTime", 0);
                reload = 2;
                returnValue = IN_PROGRESS;
            }
        }

        if(reload == 2){
            returnValue = FunctionUtility::SharedInputHandlerRemoteFunction(vmap, amap, aname, p_fims, aV, siteUri, "SiteRunCmd", "Start");
            if(returnValue == SUCCESS) {
                reload = 1;
            }
        }


        hpAv->setVal(reload);
        return returnValue;
        

    }

    /**
    * @brief Internal function called by SiteRunCmd to Stop a BESS block
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    * @param scheduledFuncName the name of the function that RemoteStop was called by (currently only SiteRunCmd)
    */
    int RemoteStop(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, const char* scheduledFuncName) 
    {
       
        if(0)FPS_PRINT_INFO("{}", __func__);


        int reload = 0;
        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;

        std::string siteUri = "site_status_control_command";


        auto relname = fmt::format("{}_{}", __func__, "ess").c_str() ;
        assetVar* hpAv = amap[relname];
        if (!hpAv || (reload = hpAv->getiVal()) == 0)
        {
            reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
        }


        int returnValue = IN_PROGRESS; 

        if(reload == 0){
            linkVals(*vm, vmap, amap, aname, "/reload", reload, relname);
            hpAv = amap[relname];
            reload = 1;
        }

        if(reload == 1) {
            returnValue = FunctionUtility::SharedInputHandlerRemoteFunction(vmap, amap, aname, p_fims, aV, siteUri, "SiteRunCmd", "Stop");
            if(returnValue == SUCCESS) {
                aV->setParam("endTime", 0);
                reload = 2;
                returnValue = IN_PROGRESS;
            }
        }

        if(reload == 2){
            returnValue = FunctionUtility::SharedInputHandlerRemoteFunction(vmap, amap, aname, p_fims, aV, siteUri, "SiteRunCmd", "OpenContactors");
            if(returnValue == SUCCESS) {
                reload = 1;
            }
        }

        hpAv->setVal(reload);
        return returnValue;

    }

    /**
    * @brief Internal function called by SiteRunCmd to Standby a BESS block
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    * @param scheduledFuncName the name of the function that RemoteStandby was called by (currently only SiteRunCmd)
    */
    int RemoteStandby(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, const char* scheduledFuncName)
    {
       
        if(0)FPS_PRINT_INFO("{}", __func__);

        int reload = 0;
        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;
        essPerf ePerf(am, aname, __func__);

        std::string siteUri = "site_status_control_command";


        const char *essch = (const char*)"ess";
        if (aV->gotParam("ess"))
        {
            essch = aV->getcParam("ess");
        }

        auto relname = fmt::format("{}_{}", __func__, essch).c_str();
        assetVar* cAv = amap[relname];
        if (!cAv || (reload = cAv->getiVal()) == 0)
        {
            reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
        }

        if (reload == 0) {

            linkVals(*vm, vmap, amap, essch, "/reload", reload, relname);
            cAv = amap[relname];

            std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
                // /status/bms/DCClosed
                FunctionUtility::AssetVarInfo("/status/bms", "DCClosed", assetVar::ATypes::ABOOL),
                // /status/pcs/SystemState
                FunctionUtility::AssetVarInfo("/status/pcs", "SystemState", assetVar::ATypes::ASTRING)
            };
            amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            reload = 1;
        }


        if(reload == 1) {
            bool dcClosed = amap["DCClosed"]->getbVal();
            char* systemStateStatus = amap["SystemState"]->getcVal();
            bool systemState = false;

            if(!(systemStateStatus == nullptr)){
                std::string compareString = systemStateStatus;
                systemState = (compareString == "On");
            }

            if(systemState){
                reload = 3;
            } else if(!dcClosed) {
                reload = 2;
            } else {
                reload = 3;
            }

        }


        int returnValue = IN_PROGRESS;


        if(reload == 2){
            returnValue = FunctionUtility::SharedInputHandlerRemoteFunction(vmap, amap, aname, p_fims, aV, siteUri, "SiteRunCmd", "CloseContactors");
            if(returnValue == SUCCESS) {
                aV->setParam("endTime", 0);
                reload = 1;
                returnValue = IN_PROGRESS;
            }
        }

        if(reload == 3) {
            returnValue = FunctionUtility::SharedInputHandlerRemoteFunction(vmap, amap, aname, p_fims, aV, siteUri, "SiteRunCmd", "Standby");
            if(returnValue == SUCCESS) {
                reload = 1;
            }
        }
        

        cAv->setVal(reload);

        return returnValue;

        
    }




}


#endif
