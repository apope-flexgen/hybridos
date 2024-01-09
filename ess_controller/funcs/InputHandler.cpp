



#ifndef INPUTHANDLER_CPP
#define INPUTHANDLER_CPP

#include "asset.h"
#include "assetVar.h"
#include "formatters.hpp"
#include "FunctionUtility.hpp"
#include "OutputHandler.hpp"
#include "InfoMessageUtility.hpp"



namespace InputHandler
{
    /**
    * @brief Function that can be put on the scheduler and is the first function in the chain of the process of CloseContactors for the/a BMS
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    */
    void LocalStartBMS(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*aV)
    {
    
        if(0)FPS_PRINT_INFO("{}", __func__);
        FunctionUtility::SharedInputHandlerLocalFunction(vmap, amap, aname, p_fims, aV, __func__);
    }

    /**
    * @brief Function that can be put on the scheduler and is the first function in the chain of the process of OpenContactors for the/a BMS
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    */
    void LocalStopBMS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
    {
        if(0)FPS_PRINT_INFO("{}", __func__);
        FunctionUtility::SharedInputHandlerLocalFunction(vmap, amap, aname, p_fims, aV, __func__);

    }

    /**
    * @brief Function that can be put on the scheduler and is the first function in the chain of the process of Start for the/a PCS
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    */
    void LocalStartPCS(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*aV)
    {
        if(0)FPS_PRINT_INFO("{}", __func__);
        FunctionUtility::SharedInputHandlerLocalFunction(vmap, amap, aname, p_fims, aV, __func__);
    }

    /**
    * @brief Function that can be put on the scheduler and is the first function in the chain of the process of Stop for the/a PCS
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    */
    void LocalStopPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
    {

        if(0)FPS_PRINT_INFO("{}", __func__);
        FunctionUtility::SharedInputHandlerLocalFunction(vmap, amap, aname, p_fims, aV, __func__);

    }

    /**
    * @brief Function that can be put on the scheduler and is the first function in the chain of the process of Standby for the/a PCS
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    */
    void LocalStandbyPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
    {

        if(0)FPS_PRINT_INFO("{}", __func__);
        FunctionUtility::SharedInputHandlerLocalFunction(vmap, amap, aname, p_fims, aV, __func__);
    }

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



    /**
    * @brief Function that can be put on the scheduler and is a command function to alter the state of a BESS block from the site level
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    */
    void SiteRunCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
    {

        if(0)FPS_PRINT_INFO("{}", __func__);
       

        int reload = 0;
        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;

        std::string siteUri = "site_status_control_command";


        int bmsReload = 0;
        int pcsReload = 0;


        auto relname = fmt::format("{}_{}", __func__, "ess").c_str() ;
        assetVar* essAv = amap[relname];
        if (!essAv || (reload = essAv->getiVal()) == 0)
        {
            reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
        }

        auto bmsRelName = fmt::format("{}_{}", __func__, "maintModeBms").c_str() ;
        assetVar* bmsAv = amap[bmsRelName];
        if (!bmsAv || (bmsReload = bmsAv->getiVal()) == 0)
        {
            bmsReload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
        }

        auto pcsRelName = fmt::format("{}_{}", __func__, "maintModePcs").c_str() ;
        assetVar* pcsAv = amap[pcsRelName];
        if (!pcsAv || (pcsReload = pcsAv->getiVal()) == 0)
        {
            pcsReload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
        }


        if (reload == 0)
        {
            linkVals(*vm, vmap, amap, aname, "/reload", reload, relname);
            essAv = amap[relname];

            linkVals(*vm, vmap, amap, aname, "/reload", reload, bmsRelName);
            bmsAv = amap[relname];
            linkVals(*vm, vmap, amap, aname, "/reload", reload, pcsRelName);
            pcsAv = amap[relname];

            std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
                // /site/ess/start_stop
                FunctionUtility::AssetVarInfo("/site/ess", siteUri.c_str(), assetVar::ATypes::AINT),
                // /assets/bms/summary/maint_mode
                FunctionUtility::AssetVarInfo("/assets/bms/summary", "maint_mode", "maint_mode_BMS", assetVar::ATypes::ABOOL),
                // /assets/pcs/summary/maint_mode
                FunctionUtility::AssetVarInfo("/assets/pcs/summary", "maint_mode", "maint_mode_PCS", assetVar::ATypes::ABOOL),
                // /status/bms/IsFaulted
                FunctionUtility::AssetVarInfo("/status/bms", "IsFaulted", assetVar::ATypes::ABOOL)
            };
            amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            reload = 1;
            essAv->setVal(reload);
            return;
        }

        if (reload == 1)
        {

            //Hasn't been updated yet
            if(amap[siteUri.c_str()]->getiVal() == -1) {
                FunctionUtility::PullOffScheduler(amap, aV, siteUri.c_str());
                return;
            }

            //If either the bms or the pcs are in maint_mode these methods can't be accessed
            if(amap["maint_mode_BMS"]->getbVal()) {
                std::string message = fmt::format(
                    "Condition(s): [{}:{}] == false",
                    amap["maint_mode_BMS"]->getfName(), 
                    amap["maint_mode_BMS"]->getbVal()
                );
                FunctionUtility::FunctionResultHandler(-1, vmap, amap, aname, p_fims, aV, __func__, siteUri.c_str(), message.c_str());
                return;
            }
            if(amap["maint_mode_PCS"]->getbVal()) {
                std::string message = fmt::format(
                    "Condition(s): [{}:{}] == false",
                    amap["maint_mode_PCS"]->getfName(), 
                    amap["maint_mode_PCS"]->getbVal()
                );
                FunctionUtility::FunctionResultHandler(-1, vmap, amap, aname, p_fims, aV, __func__, siteUri.c_str(), message.c_str());
                return;
            }


            //disabling the maintenance mode of the bms and pcs while running
            if (amap["maint_mode_BMS"]->gotParam("enabled")){
                if (amap["maint_mode_BMS"]->getbParam("enabled")){
                    amap["maint_mode_BMS"]->setParam("enabled", false);
                    bmsAv->setVal(1);
                } 
            } else {
                amap["maint_mode_BMS"]->setParam("enabled", false);
            }

            if (amap["maint_mode_PCS"]->gotParam("enabled")){
                if (amap["maint_mode_PCS"]->getbParam("enabled")){
                    amap["maint_mode_PCS"]->setParam("enabled", false);
                    pcsAv->setVal(1);
                } 
            } else {
                amap["maint_mode_PCS"]->setParam("enabled", false);
            }


            if(amap["IsFaulted"]->getbVal()) {
                std::string message = fmt::format(
                    "Condition(s): [{}:{}] == false",
                    amap["IsFaulted"]->getfName(), 
                    amap["IsFaulted"]->getbVal()
                );
                FunctionUtility::FunctionResultHandler(-1, vmap, amap, aname, p_fims, aV, __func__, siteUri.c_str(), message.c_str());
                return;
            }


            reload = 2;
            essAv->setVal(reload);
            
        }

        if(reload == 2){
        
            int stCommand = amap[siteUri.c_str()]->getiVal();
            int returnValue = 0;

            switch (stCommand) {
                //Shutdown
                case 0:
                    returnValue = RemoteStop(vmap, amap, aname, p_fims, aV, __func__);
                    break;
                //Startup
                case 1:
                    returnValue = RemoteStart(vmap, amap, aname, p_fims, aV, __func__);
                    break;
                //Standby
                case 2:
                    returnValue = RemoteStandby(vmap, amap, aname, p_fims, aV, __func__);
                    break;
                default:
                    returnValue = FAILURE;
                    break;
            }



            if(returnValue == SUCCESS || returnValue == FAILURE) {
                //Re-enabling maint_modes
                if(bmsAv->getiVal() == 1){
                    amap["maint_mode_BMS"]->setParam("enabled", true);
                    bmsAv->setVal(0);
                }
                if(pcsAv->getiVal() == 1){
                    amap["maint_mode_BMS"]->setParam("enabled", true);
                    pcsAv->setVal(0);
                }
                reload = 1;
                essAv->setVal(reload);
            }
        }
    }

    /**
    * @brief Function that can be put on the scheduler and is a command function to alter the state of a BESS block from the site level
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    */
    void SiteBMSContactorControl(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
    {

        if(0)FPS_PRINT_INFO("{}", __func__);
       

        int reload = 0;
        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;

        //Uri from site controller which is used in many different places
        std::string siteUri = "bms_dc_contactor_control";


        auto relname = fmt::format("{}_{}", __func__, "ess").c_str() ;
        assetVar* essAv = amap[relname];
        if (!essAv || (reload = essAv->getiVal()) == 0)
        {
            reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
        }


        if (reload == 0)
        {
            linkVals(*vm, vmap, amap, aname, "/reload", reload, relname);
            essAv = amap[relname];
            std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
                // /site/ess/start_stop
                FunctionUtility::AssetVarInfo("/site/ess", siteUri.c_str(), assetVar::ATypes::AINT),
                // /status/bms/IsFaulted
                FunctionUtility::AssetVarInfo("/status/bms", "IsFaulted", assetVar::ATypes::ABOOL),

                // /assets/bms/summary/maint_mode
                FunctionUtility::AssetVarInfo("/assets/bms/summary", "maint_mode", assetVar::ATypes::ABOOL),
                // /assets/bms/summary/close_contactors
                FunctionUtility::AssetVarInfo("/assets/bms/summary", "close_contactors", assetVar::ATypes::ABOOL),
                // /status/bms/CloseContactorsEnabled
                FunctionUtility::AssetVarInfo("/status/bms", "CloseContactorsEnabled", assetVar::ATypes::ABOOL),
                // /controls/bms/CloseContactorsEnable
                FunctionUtility::AssetVarInfo("/controls/bms", "CloseContactorsEnable", assetVar::ATypes::ABOOL),

                // /assets/bms/summary/open_contactors
                FunctionUtility::AssetVarInfo("/assets/bms/summary", "open_contactors", assetVar::ATypes::ABOOL),
                // /status/bms/DCClosed
                FunctionUtility::AssetVarInfo("/status/bms", "DCClosed", "DCClosed_BMS", assetVar::ATypes::ABOOL),
                // /status/bms/OpenContactorsEnabled
                FunctionUtility::AssetVarInfo("/status/bms", "OpenContactorsEnabled", assetVar::ATypes::ABOOL),
                // /controls/bms/OpenContactorsEnable
                FunctionUtility::AssetVarInfo("/controls/bms", "OpenContactorsEnable", assetVar::ATypes::ABOOL),
                // /status/pcs/DCClosed
                FunctionUtility::AssetVarInfo("/status/pcs", "DCClosed", "DCClosed_PCS", assetVar::ATypes::ABOOL)
            };
            amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            reload = 1;
            essAv->setVal(reload);
            return;
        }

        if(0)FunctionUtility::PrintAssetVar(amap[siteUri.c_str()], assetVar::ATypes::AINT);

        if (reload == 1)
        {
 
            //Hasn't been updated yet
            if(amap[siteUri.c_str()]->getiVal() == 0) {
                FunctionUtility::PullOffScheduler(amap, aV, siteUri.c_str());
                return;
            }


            if(amap["IsFaulted"]->getbVal()) {
                std::string message = fmt::format(
                    "Condition(s): [{}:{}] == false",
                    amap["IsFaulted"]->getfName(), 
                    amap["IsFaulted"]->getbVal()
                );
                FunctionUtility::FunctionResultHandler(-1, vmap, amap, aname, p_fims, aV, __func__, siteUri.c_str(), message.c_str());
                return;
            }


            reload = 2;
            essAv->setVal(reload);
            
        }

        if(reload == 2){
        
            int stCommand = amap[siteUri.c_str()]->getiVal();
            int returnValue = 0;

            switch (stCommand) {
                //CloseContactors
                case 2:
                    if(!amap["DCClosed_BMS"]->getbVal() && !amap["IsFaulted"]->getbVal()) {
                        returnValue = FunctionUtility::SharedInputHandlerRemoteFunction(vmap, amap, aname, p_fims, aV, siteUri, __func__, "CloseContactors");
                    } else {
                        std::string message = "";
                        message += fmt::format(
                            "Failed Attempt to close bms contactors by a value of [{}] sent to [{}] because",
                            stCommand,
                            amap["bms_dc_contactor_control"]->getfName()
                        );
                        message += fmt::format(
                            " ---> Condition(s): [{}:{}] == false && [{}:{}] == false",
                            amap["DCClosed_BMS"]->getfName(), 
                            amap["DCClosed_BMS"]->getbVal(),
                            amap["IsFaulted"]->getfName(), 
                            amap["IsFaulted"]->getbVal()
                        );
                        aV->sendEvent(aname, p_fims, Severity::Info, message.c_str());
                        returnValue = FAILURE;
                    }
                    break;
                //OpenContactors
                case 3:
                    if (amap["DCClosed_BMS"]->getbVal() && !amap["DCClosed_PCS"]->getbVal()) {
                        returnValue = FunctionUtility::SharedInputHandlerRemoteFunction(vmap, amap, aname, p_fims, aV, siteUri, __func__, "OpenContactors");
                    } else {
                        std::string message = "";
                        message += fmt::format(
                            "Failed Attempt to open bms contactors by a value of [{}] sent to [{}] because",
                            stCommand,
                            amap["bms_dc_contactor_control"]->getfName()
                        );
                        message += fmt::format(
                            " ---> Condition(s): [{}:{}] == true && [{}:{}] == false",
                            amap["DCClosed_BMS"]->getfName(), 
                            amap["DCClosed_BMS"]->getbVal(),
                            amap["DCClosed_PCS"]->getfName(), 
                            amap["DCClosed_PCS"]->getbVal()
                        );
                        aV->sendEvent(aname, p_fims, Severity::Info, message.c_str());
                        returnValue = FAILURE;
                    }
                    break;
                default:
                    returnValue = FAILURE;
                    break;
            }



            if(returnValue == SUCCESS || returnValue == FAILURE) {
                reload = 1;
                essAv->setVal(reload);
                FunctionUtility::PullOffScheduler(amap, aV, siteUri.c_str());
            }
        }
    }

    /**
    * @brief Function that can be put on the scheduler and is a command function to alter the state of a BESS block from the site level
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    */
    void SitePCSStatusControl(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
    {

        if(0)FPS_PRINT_INFO("{}", __func__);
       

        int reload = 0;
        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;

        //Uri from site controller which is used in many different places
        std::string siteUri = "start_stop_standby_command";


        auto relname = fmt::format("{}_{}", __func__, "ess").c_str() ;
        assetVar* essAv = amap[relname];
        if (!essAv || (reload = essAv->getiVal()) == 0)
        {
            reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
        }


        if (reload == 0)
        {
            linkVals(*vm, vmap, amap, aname, "/reload", reload, relname);
            essAv = amap[relname];
            std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
                // /site/ess/start_stop
                FunctionUtility::AssetVarInfo("/site/ess", siteUri.c_str(), assetVar::ATypes::AINT),
                // /status/pcs/IsFaulted
                FunctionUtility::AssetVarInfo("/status/pcs", "IsFaulted", assetVar::ATypes::ABOOL),


                // /assets/pcs/summary/maint_mode
                FunctionUtility::AssetVarInfo("/assets/pcs/summary", "maint_mode", assetVar::ATypes::ABOOL),
                // /assets/pcs/summary/stop
                FunctionUtility::AssetVarInfo("/assets/pcs/summary", "stop", assetVar::ATypes::ABOOL),
                // /status/pcs/SystemState
                FunctionUtility::AssetVarInfo("/status/pcs", "SystemState", assetVar::ATypes::ASTRING),
                // /status/pcs/StopEnabled
                FunctionUtility::AssetVarInfo("/status/pcs", "StopEnabled", assetVar::ATypes::ABOOL),
                // /controls/pcs/StopEnable
                FunctionUtility::AssetVarInfo("/controls/pcs", "StopEnable", assetVar::ATypes::ABOOL),


                // /status/bms/DCClosed
                FunctionUtility::AssetVarInfo("/status/bms", "DCClosed", assetVar::ATypes::ABOOL),
                // /assets/pcs/summary/start
                FunctionUtility::AssetVarInfo("/assets/pcs/summary", "start", assetVar::ATypes::ABOOL),
                // /status/pcs/StartEnabled
                FunctionUtility::AssetVarInfo("/status/pcs", "StartEnabled", assetVar::ATypes::ABOOL),
                // /controls/pcs/StartEnable
                FunctionUtility::AssetVarInfo("/controls/pcs", "StartEnable", assetVar::ATypes::ABOOL),

                // /assets/pcs/summary/standby
                FunctionUtility::AssetVarInfo("/assets/pcs/summary", "standby", assetVar::ATypes::ABOOL),
                // /status/pcs/StandbyEnabled
                FunctionUtility::AssetVarInfo("/status/pcs", "StandbyEnabled", assetVar::ATypes::ABOOL),
                // /controls/pcs/StandbyEnable
                FunctionUtility::AssetVarInfo("/controls/pcs", "StandbyEnable", assetVar::ATypes::ABOOL)
            };
            amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            reload = 1;
            essAv->setVal(reload);
            return;
        }

        if (reload == 1)
        {

            //Hasn't been updated yet
            if(amap[siteUri.c_str()]->getiVal() == -1) {
                FunctionUtility::PullOffScheduler(amap, aV, siteUri.c_str());
                return;
            }


            if(amap["IsFaulted"]->getbVal()) {
                std::string message = fmt::format(
                    "Condition(s): [{}:{}] == false",
                    amap["IsFaulted"]->getfName(), 
                    amap["IsFaulted"]->getbVal()
                );
                FunctionUtility::FunctionResultHandler(-1, vmap, amap, aname, p_fims, aV, __func__, siteUri.c_str(), message.c_str());
                return;
            }


            reload = 2;
            essAv->setVal(reload);
            
        }

        if(reload == 2){
            int stCommand = amap[siteUri.c_str()]->getiVal();
            int returnValue = 0;

            char* systemStateStatus = amap["SystemState"]->getcVal();
            bool systemState = false;

            switch (stCommand) {
                //Stop
                case 0:
                    if(!(systemStateStatus == nullptr)){
                        std::string compareString = systemStateStatus;
                        systemState = (compareString != "Stop");
                    }
                    if (systemState) {
                        returnValue = FunctionUtility::SharedInputHandlerRemoteFunction(vmap, amap, aname, p_fims, aV, siteUri, __func__, "Stop");
                    } else {
                        std::string systemStateVal = "";
                        if(amap["SystemState"]->getcVal() == nullptr) {
                            systemStateVal += "nullptr";
                        } else {
                            systemStateVal += amap["SystemState"]->getcVal();
                        }

                        std::string message = "";
                        message += fmt::format(
                            "Failed Attempt to stop pcs by a value of [{}] sent to [{}] because",
                            stCommand,
                            amap["start_stop_standby_command"]->getfName()
                        );
                        message += fmt::format(
                            " ---> Condition(s): [{}:{}] != Stop",
                            amap["SystemState"]->getfName(), 
                            systemStateVal
                        );
                        aV->sendEvent(aname, p_fims, Severity::Info, message.c_str());
                        returnValue = FAILURE;
                    }
                    break;
                //Start
                case 1:
                    if(!(systemStateStatus == nullptr)){
                        std::string compareString = systemStateStatus;
                        systemState = (compareString == "Stop" || compareString == "Standby");
                    }
                    if (amap["DCClosed"]->getbVal() && (systemState) && !amap["IsFaulted"]->getbVal()) {
                        returnValue = FunctionUtility::SharedInputHandlerRemoteFunction(vmap, amap, aname, p_fims, aV, siteUri, __func__, "Start");
                    } else {
                        std::string systemStateVal = "";
                        if(amap["SystemState"]->getcVal() == nullptr) {
                            systemStateVal += "nullptr";
                        } else {
                            systemStateVal += amap["SystemState"]->getcVal();
                        }
                        std::string message = "";
                        message += fmt::format(
                            "Failed Attempt to start pcs by a value of [{}] sent to [{}] because",
                            stCommand,
                            amap["start_stop_standby_command"]->getfName()
                        );
                        message += fmt::format(
                            " ---> Condition(s): [{}:{}] == true && [{}:{}] == (Stop or Standby) && [{}:{}] == false",
                            amap["DCClosed"]->getfName(), 
                            amap["DCClosed"]->getbVal(),
                            amap["SystemState"]->getfName(), 
                            systemStateVal,
                            amap["IsFaulted"]->getfName(), 
                            amap["IsFaulted"]->getbVal()
                        );
                        aV->sendEvent(aname, p_fims, Severity::Info, message.c_str());
                        returnValue = FAILURE;
                    }
                    break;
                //Standby
                case 2:
                    if(!(systemStateStatus == nullptr)){
                        std::string compareString = systemStateStatus;
                        systemState = (compareString == "Stop" || compareString == "Run");
                    }
                    if (amap["DCClosed"]->getbVal() && (systemState)) {
                        returnValue = FunctionUtility::SharedInputHandlerRemoteFunction(vmap, amap, aname, p_fims, aV, siteUri, __func__, "Standby");
                    } else {
                        std::string systemStateVal = "";
                        if(amap["SystemState"]->getcVal() == nullptr) {
                            systemStateVal += "nullptr";
                        } else {
                            systemStateVal += amap["SystemState"]->getcVal();
                        }
                        std::string message = "";
                        message += fmt::format(
                            "Failed Attempt to standby pcs by a value of [{}] sent to [{}] because",
                            stCommand,
                            amap["start_stop_standby_command"]->getfName()
                        );
                        message += fmt::format(
                            " ---> Condition(s): [{}:{}] == true && [{}:{}] == (Stop or Run)",
                            amap["DCClosed"]->getfName(), 
                            amap["DCClosed"]->getbVal(),
                            amap["SystemState"]->getfName(), 
                            systemStateVal
                        );
                        aV->sendEvent(aname, p_fims, Severity::Info, message.c_str());
                        returnValue = FAILURE;
                    }
                    break;
                default:
                    returnValue = FAILURE;
                    break;
            }



            if(returnValue == SUCCESS || returnValue == FAILURE) {
                reload = 1;
                essAv->setVal(reload);
                FunctionUtility::PullOffScheduler(amap, aV, siteUri.c_str());
            }
        }

    }


    /**
    * @brief Function that can be put on the scheduler and uses logic to enable /ess/controls/bms/CloseContactors
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    */
    void CloseContactorsEnable(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*aV)
    {
        if(0)FPS_PRINT_INFO("{}", __func__);


        // bool debug = false;
        int reload = 0;
        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;
        essPerf ePerf(am, aname, __func__);

        const char *bmsch = (const char*)"bms";
        if (aV->gotParam("bms"))
        {
            bmsch = aV->getcParam("bms");
        }

        auto relname = fmt::format("{}_{}", __func__, bmsch).c_str();
        assetVar* cAv = amap[relname];
        if (!cAv || (reload = cAv->getiVal()) == 0)
        {
            reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
        }

        if(reload == 0){

            linkVals(*vm, vmap, amap, bmsch, "/reload", reload, relname);
            cAv = amap[relname];

            std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
                // /assets/bms/summary/maint_mode
                FunctionUtility::AssetVarInfo("/assets/bms/summary", "maint_mode", assetVar::ATypes::ABOOL),
                // /assets/bms/summary/close_contactors
                FunctionUtility::AssetVarInfo("/assets/bms/summary", "close_contactors", assetVar::ATypes::ABOOL),
                // /status/bms/DCClosed
                FunctionUtility::AssetVarInfo("/status/bms", "DCClosed", assetVar::ATypes::ABOOL),
                // /status/bms/IsFaulted
                FunctionUtility::AssetVarInfo("/status/bms", "IsFaulted", assetVar::ATypes::ABOOL),
                // /status/bms/CloseContactorsEnabled
                FunctionUtility::AssetVarInfo("/status/bms", "CloseContactorsEnabled", assetVar::ATypes::ABOOL),
                // /controls/bms/CloseContactorsEnable
                FunctionUtility::AssetVarInfo("/controls/bms", "CloseContactorsEnable", assetVar::ATypes::ABOOL)
            };

            amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            reload = 1;
            cAv->setVal(reload);
        }

        if(reload == 1){

            std::string message = "";

            if (!amap["DCClosed"]->getbVal() && amap["maint_mode"]->getbVal() && !amap["IsFaulted"]->getbVal()) {
                message += "CloseContactorsEnable TRUE";
                amap["close_contactors"]->setParam("enabled", true);
                amap["CloseContactorsEnabled"]->setVal(true);
                cAv->setVal(reload);
            } else {
                message += fmt::format(
                    " ---> Condition(s): [{}:{}] == false && [{}:{}] == true && [{}:{}] == false",
                    amap["DCClosed"]->getfName(), 
                    amap["DCClosed"]->getbVal(),
                    amap["maint_mode"]->getfName(), 
                    amap["maint_mode"]->getbVal(),
                    amap["IsFaulted"]->getfName(), 
                    amap["IsFaulted"]->getbVal()
                );
                FunctionUtility::UpdateEnabledLogicMessage("close_contactors", message);
                amap["close_contactors"]->setParam("enabled", false);
                amap["CloseContactorsEnabled"]->setVal(false);
                cAv->setVal(reload);
            }

            if(0)FPS_PRINT_INFO("{}", message);

        }

    }

    /**
    * @brief Function that can be put on the scheduler and uses logic to enable /ess/controls/bms/OpenContactors
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    */
    void OpenContactorsEnable(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*aV)
    {

        
        if(0)FPS_PRINT_INFO("{}", __func__);

        // bool debug = false;
        int reload = 0;
        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;
        essPerf ePerf(am, aname, __func__);

        const char *bmsch = (const char*)"bms";
        if (aV->gotParam("bms"))
        {
            bmsch = aV->getcParam("bms");
        }

        auto relname = fmt::format("{}_{}", __func__, bmsch).c_str();
        assetVar* cAv = amap[relname];
        if (!cAv || (reload = cAv->getiVal()) == 0)
        {
            reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
        }

        if(reload == 0){

            linkVals(*vm, vmap, amap, bmsch, "/reload", reload, relname);
            cAv = amap[relname];

            std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
                // /assets/bms/summary/maint_mode
                FunctionUtility::AssetVarInfo("/assets/bms/summary", "maint_mode", assetVar::ATypes::ABOOL),
                // /assets/bms/summary/open_contactors
                FunctionUtility::AssetVarInfo("/assets/bms/summary", "open_contactors", assetVar::ATypes::ABOOL),
                // /status/bms/DCClosed
                FunctionUtility::AssetVarInfo("/status/bms", "DCClosed", "DCClosed_BMS", assetVar::ATypes::ABOOL),
                // /status/bms/OpenContactorsEnabled
                FunctionUtility::AssetVarInfo("/status/bms", "OpenContactorsEnabled", assetVar::ATypes::ABOOL),
                // /controls/bms/OpenContactorsEnable
                FunctionUtility::AssetVarInfo("/controls/bms", "OpenContactorsEnable", assetVar::ATypes::ABOOL),
                // /status/pcs/DCClosed
                FunctionUtility::AssetVarInfo("/status/pcs", "DCClosed", "DCClosed_PCS", assetVar::ATypes::ABOOL)
            };

            amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            reload = 1;
            cAv->setVal(reload);
        }

        if(reload == 1){
            std::string message = "";

            if (amap["DCClosed_BMS"]->getbVal() && !amap["DCClosed_PCS"]->getbVal() && amap["maint_mode"]->getbVal()) {
                message += fmt::format("{} TRUE", __func__);
                amap["open_contactors"]->setParam("enabled", true);
                amap["OpenContactorsEnabled"]->setVal(true);
            } else {
                message += fmt::format(
                    " ---> Condition(s): [{}:{}] == true && [{}:{}] == false && [{}:{}] == true",
                    amap["DCClosed_BMS"]->getfName(), 
                    amap["DCClosed_BMS"]->getbVal(),
                    amap["DCClosed_PCS"]->getfName(), 
                    amap["DCClosed_PCS"]->getbVal(),
                    amap["maint_mode"]->getfName(), 
                    amap["maint_mode"]->getbVal()
                );
                FunctionUtility::UpdateEnabledLogicMessage("open_contactors", message);
                amap["open_contactors"]->setParam("enabled", false);
                amap["OpenContactorsEnabled"]->setVal(false);
            }

            if(0)FPS_PRINT_INFO("{}", message);
        }


    }

    /**
    * @brief Function that can be put on the scheduler and uses logic to enable /ess/controls/pcs/Start
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    */
    void StartEnable(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*aV)
    {
        if(0)FPS_PRINT_INFO("{}", __func__);


        // bool debug = false;
        int reload = 0;
        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;
        essPerf ePerf(am, aname, __func__);

        const char *pcsch = (const char*)"pcs";
        if (aV->gotParam("pcs"))
        {
            pcsch = aV->getcParam("pcs");
        }

        auto relname = fmt::format("{}_{}", __func__, pcsch).c_str();
        assetVar* cAv = amap[relname];
        if (!cAv || (reload = cAv->getiVal()) == 0)
        {
            reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
        }

        if(reload == 0){

            linkVals(*vm, vmap, amap, pcsch, "/reload", reload, relname);
            cAv = amap[relname];

            std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
                // /status/bms/DCClosed
                FunctionUtility::AssetVarInfo("/status/bms", "DCClosed", assetVar::ATypes::ABOOL),
                // /assets/pcs/summary/maint_mode
                FunctionUtility::AssetVarInfo("/assets/pcs/summary", "maint_mode", assetVar::ATypes::ABOOL),
                // /assets/pcs/summary/start
                FunctionUtility::AssetVarInfo("/assets/pcs/summary", "start", assetVar::ATypes::ABOOL),
                // /status/pcs/SystemState
                FunctionUtility::AssetVarInfo("/status/pcs", "SystemState", assetVar::ATypes::ASTRING),
                // /status/pcs/IsFaulted
                FunctionUtility::AssetVarInfo("/status/pcs", "IsFaulted", assetVar::ATypes::ABOOL),
                // /status/pcs/StartEnabled
                FunctionUtility::AssetVarInfo("/status/pcs", "StartEnabled", assetVar::ATypes::ABOOL),
                // /controls/pcs/StartEnable
                FunctionUtility::AssetVarInfo("/controls/pcs", "StartEnable", assetVar::ATypes::ABOOL)
            };


            amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            reload = 1;
            cAv->setVal(reload);
        }

        if(reload == 1){
            char* systemStateStatus = amap["SystemState"]->getcVal();

            bool systemState = false;

            if(!(systemStateStatus == nullptr)){
                std::string compareString = systemStateStatus;
                systemState = (compareString == "Stop" || compareString == "Standby");
            }


            std::string message = "";

            if (amap["DCClosed"]->getbVal() && amap["maint_mode"]->getbVal() && (systemState) && !amap["IsFaulted"]->getbVal()) {
                message += fmt::format("{} TRUE", __func__);
                amap["start"]->setParam("enabled", true);
                amap["StartEnabled"]->setVal(true);
            } else {

                std::string systemStateVal = "";
                if(amap["SystemState"]->getcVal() == nullptr) {
                    systemStateVal += "nullptr";
                } else {
                    systemStateVal += amap["SystemState"]->getcVal();
                }

                message += fmt::format(
                    " ---> Condition(s): [{}:{}] == true && [{}:{}] == true && [{}:{}] == (Stop or Standby) && [{}:{}] == false",
                    amap["DCClosed"]->getfName(), 
                    amap["DCClosed"]->getbVal(),
                    amap["maint_mode"]->getfName(), 
                    amap["maint_mode"]->getbVal(),
                    amap["SystemState"]->getfName(), 
                    systemStateVal,
                    amap["IsFaulted"]->getfName(), 
                    amap["IsFaulted"]->getbVal()
                );
                FunctionUtility::UpdateEnabledLogicMessage("start", message);
                amap["start"]->setParam("enabled", false);
                amap["StartEnabled"]->setVal(false);
            }

            if(0)FPS_PRINT_INFO("{}", message);
        }


    }

    /**
    * @brief Function that can be put on the scheduler and uses logic to enable /ess/controls/pcs/Stop
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    */
    void StopEnable(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*aV)
    {
        if(0)FPS_PRINT_INFO("{}", __func__);


        // bool debug = false;
        int reload = 0;
        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;
        essPerf ePerf(am, aname, __func__);

        const char *pcsch = (const char*)"pcs";
        if (aV->gotParam("pcs"))
        {
            pcsch = aV->getcParam("pcs");
        }

        auto relname = fmt::format("{}_{}", __func__, pcsch).c_str();
        assetVar* cAv = amap[relname];
        if (!cAv || (reload = cAv->getiVal()) == 0)
        {
            reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
        }

        if(reload == 0){

            linkVals(*vm, vmap, amap, pcsch, "/reload", reload, relname);
            cAv = amap[relname];

            std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
                // /assets/pcs/summary/maint_mode
                FunctionUtility::AssetVarInfo("/assets/pcs/summary", "maint_mode", assetVar::ATypes::ABOOL),
                // /assets/pcs/summary/stop
                FunctionUtility::AssetVarInfo("/assets/pcs/summary", "stop", assetVar::ATypes::ABOOL),
                // /status/pcs/SystemState
                FunctionUtility::AssetVarInfo("/status/pcs", "SystemState", assetVar::ATypes::ASTRING),
                // /status/pcs/StopEnabled
                FunctionUtility::AssetVarInfo("/status/pcs", "StopEnabled", assetVar::ATypes::ABOOL),
                // /controls/pcs/StopEnable
                FunctionUtility::AssetVarInfo("/controls/pcs", "StopEnable", assetVar::ATypes::ABOOL)
            };


            amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            reload = 1;
            cAv->setVal(reload);
        }

        if(reload == 1){
            char* systemStateStatus = amap["SystemState"]->getcVal();

            bool systemState = false;

            if(!(systemStateStatus == nullptr)){
                std::string compareString = systemStateStatus;
                systemState = (compareString != "Stop");
            }

            std::string message = "";

            if (amap["maint_mode"]->getbVal() && systemState) {
                message += fmt::format("{} TRUE", __func__);
                amap["stop"]->setParam("enabled", true);
                amap["StopEnabled"]->setVal(true);
            } else {

                std::string systemStateVal = "";
                if(amap["SystemState"]->getcVal() == nullptr) {
                    systemStateVal += "nullptr";
                } else {
                    systemStateVal += amap["SystemState"]->getcVal();
                }

                message += fmt::format(
                    " ---> Condition(s): [{}:{}] == true && [{}:{}] != Stop",
                    amap["maint_mode"]->getfName(), 
                    amap["maint_mode"]->getbVal(),
                    amap["SystemState"]->getfName(), 
                    systemStateVal
                );
                FunctionUtility::UpdateEnabledLogicMessage("stop", message);
                amap["stop"]->setParam("enabled", false);
                amap["StopEnabled"]->setVal(false);
            }

            if(0)FPS_PRINT_INFO("{}", message);
        }


    }

    /**
    * @brief Function that can be put on the scheduler and uses logic to enable /ess/controls/pcs/CloseContactors
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    */
    void StandbyEnable(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*aV)
    {
        if(0)FPS_PRINT_INFO("{}", __func__);


        // bool debug = false;
        int reload = 0;
        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;
        essPerf ePerf(am, aname, __func__);

        const char *pcsch = (const char*)"pcs";
        if (aV->gotParam("pcs"))
        {
            pcsch = aV->getcParam("pcs");
        }

        auto relname = fmt::format("{}_{}", __func__, pcsch).c_str();
        assetVar* cAv = amap[relname];
        if (!cAv || (reload = cAv->getiVal()) == 0)
        {
            reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
        }

        if(reload == 0){

            linkVals(*vm, vmap, amap, pcsch, "/reload", reload, relname);
            cAv = amap[relname];

            std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
                // /status/bms/DCClosed
                FunctionUtility::AssetVarInfo("/status/bms", "DCClosed", assetVar::ATypes::ABOOL),
                // /assets/pcs/summary/maint_mode
                FunctionUtility::AssetVarInfo("/assets/pcs/summary", "maint_mode", assetVar::ATypes::ABOOL),
                // /assets/pcs/summary/standby
                FunctionUtility::AssetVarInfo("/assets/pcs/summary", "standby", assetVar::ATypes::ABOOL),
                // /status/pcs/SystemState
                FunctionUtility::AssetVarInfo("/status/pcs", "SystemState", assetVar::ATypes::ASTRING),
                // /status/pcs/StandbyEnabled
                FunctionUtility::AssetVarInfo("/status/pcs", "StandbyEnabled", assetVar::ATypes::ABOOL),
                // /controls/pcs/StandbyEnable
                FunctionUtility::AssetVarInfo("/controls/pcs", "StandbyEnable", assetVar::ATypes::ABOOL)
            };


            amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            reload = 1;
            cAv->setVal(reload);
        }

        if(reload == 1){
            char* systemStateStatus = amap["SystemState"]->getcVal();

            bool systemState = false;


            if(!(systemStateStatus == nullptr)){
                std::string compareString = systemStateStatus;
                systemState = (compareString == "Stop" || compareString == "Run");
            }

            std::string message = "";


            if (amap["DCClosed"]->getbVal() && amap["maint_mode"]->getbVal() && (systemState)) {
                message += fmt::format("{} TRUE", __func__);
                amap["standby"]->setParam("enabled", true);
                amap["StandbyEnabled"]->setVal(true);
            } else {

                std::string systemStateVal = "";
                if(amap["SystemState"]->getcVal() == nullptr) {
                    systemStateVal += "nullptr";
                } else {
                    systemStateVal += amap["SystemState"]->getcVal();
                }

                message += fmt::format(
                    " ---> Condition(s): [{}:{}] == true && [{}:{}] == true && [{}:{}] == (Stop or Run)",
                    amap["DCClosed"]->getfName(), 
                    amap["DCClosed"]->getbVal(),
                    amap["maint_mode"]->getfName(), 
                    amap["maint_mode"]->getbVal(),
                    amap["SystemState"]->getfName(), 
                    systemStateVal
                );

                FunctionUtility::UpdateEnabledLogicMessage("standby", message);

                amap["standby"]->setParam("enabled", false);
                amap["StandbyEnabled"]->setVal(false);
            }

            if(0)FPS_PRINT_INFO("{}", message);
        }

    }





}


#endif