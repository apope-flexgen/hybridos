



#ifndef INPUTHANDLER_CPP
#define INPUTHANDLER_CPP

#include "asset.h"
#include "assetVar.h"
#include "formatters.hpp"
#include "FunctionUtility.hpp"



namespace InputHandler
{
    /**
    * @brief Trigger CloseContactors if conditions are met (maint_mode is true)
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param av the assetVar
    */
    void LocalStartBMS(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*aV)
    {
    
        if(0)FPS_PRINT_INFO("LOCAL START BMS", nullptr);

        asset_manager * am = aV->am;

        if(0)FPS_PRINT_INFO("aname [{}]", cstr{aname});
        if(0)FPS_PRINT_INFO("val [{}]", aV->getdVal());
        if(0)FPS_PRINT_INFO("assetVar [{}] ", aV->getfName());

        VarMapUtils* vm = am->vm;

        std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
            // /assets/bms/summary/close_contactors
           FunctionUtility::AssetVarInfo("/assets/bms/summary", "close_contactors", assetVar::ATypes::ABOOL),
            // /assets/bms/summary/maint_mode
            FunctionUtility::AssetVarInfo("/assets/bms/summary", "maint_mode", assetVar::ATypes::ABOOL),
            // /status/bms/IsFaulted
            FunctionUtility::AssetVarInfo("/status/bms", "IsFaulted", assetVar::ATypes::ABOOL)
        };

        amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

        //Function has been called but the assets uri hasn't been updated so it shouldn't have been called
        if(!amap["close_contactors"]->getbVal()) return;


        if (amap["maint_mode"]->getbVal() && !(amap["IsFaulted"]->getbVal())) {
            // FunctionUtility::InputOutputResultHandler(OutputHandler::CloseContactors, vmap, amap, aname, p_fims, aV, __func__, __func__, "CloseContactors", "close_contactors");
            if(0)FPS_PRINT_INFO("(maint_mode && !IsFaulted) == true", nullptr);
            FunctionUtility::FunctionResultHandler(0, vmap, amap, aname, p_fims, aV, __func__, __func__, "CloseContactors", "close_contactors", "");
        } else {
            if(0)FPS_PRINT_INFO("(maint_mode && !IsFaulted) == false", nullptr);
            FunctionUtility::FunctionResultHandler(-1, vmap, amap, aname, p_fims, aV, __func__, __func__, "", "close_contactors", "(maint_mode && !IsFaulted) == false");
        }
    }

    void LocalStopBMS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
    {

        if(0)FPS_PRINT_INFO("LOCAL STOP BMS", nullptr);

        asset_manager * am = aV->am;

        if(0)FPS_PRINT_INFO("aname [{}]", cstr{aname});
        if(0)FPS_PRINT_INFO("val [{}]", aV->getdVal());
        if(0)FPS_PRINT_INFO("assetVar [{}] ", aV->getfName());
        VarMapUtils* vm = am->vm;


        std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
            // /assets/bms/summary/open_contactors
            FunctionUtility::AssetVarInfo("/assets/bms/summary", "open_contactors", assetVar::ATypes::ABOOL),
            // /assets/bms/summary/maint_mode
            FunctionUtility::AssetVarInfo("/assets/bms/summary", "maint_mode", assetVar::ATypes::ABOOL)
        };
        amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

        //Function has been called but the assets uri hasn't been updated so it shouldn't have been called
        if(!amap["open_contactors"]->getbVal()) return;

        if (amap["maint_mode"]->getbVal()) {
            // FunctionUtility::InputOutputResultHandler(OutputHandler::OpenContactors, vmap, amap, aname, p_fims, aV, __func__, __func__, "OpenContactors", "open_contactors");
            FunctionUtility::FunctionResultHandler(0, vmap, amap, aname, p_fims, aV, __func__, __func__, "OpenContactors", "open_contactors", "");
        } else {
            FunctionUtility::FunctionResultHandler(-1, vmap, amap, aname, p_fims, aV, __func__, __func__, "", "open_contactors", "(maint_mode) == false");
        }

    }

    void LocalStartPCS(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*aV)
    {
       
        if(0)FPS_PRINT_INFO("LOCAL START PCS", nullptr);


        asset_manager * am = aV->am;

        if(0)FPS_PRINT_INFO("aname [{}]", cstr{aname});
        if(0)FPS_PRINT_INFO("val [{}]", aV->getdVal());
        if(0)FPS_PRINT_INFO("assetVar [{}] ", aV->getfName());


        VarMapUtils* vm = am->vm;

        std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
            // /status/bms/DCClosed
            FunctionUtility::AssetVarInfo("/status/bms", "DCClosed", assetVar::ATypes::ABOOL),
            // /assets/pcs/summary/start
            FunctionUtility::AssetVarInfo("/assets/pcs/summary", "start", assetVar::ATypes::ABOOL),
            // /assets/pcs/summary/maint_mode
            FunctionUtility::AssetVarInfo("/assets/pcs/summary", "maint_mode", assetVar::ATypes::ABOOL),
            // /status/pcs/IsFaulted
            FunctionUtility::AssetVarInfo("/status/pcs", "IsFaulted", assetVar::ATypes::ABOOL),
            // /status/pcs/SystemState
            FunctionUtility::AssetVarInfo("/status/pcs", "SystemState", assetVar::ATypes::ASTRING)
        };
        amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

        //Function has been called but the assets uri hasn't been updated so it shouldn't have been called
        if(!amap["start"]->getbVal()) return;


        // "enable": "/config/pcs:enable",
        // "expression": "{1} and {2} and ({3} == Stop or {3} == Standby) and not {4}",
        // "variable1": "/status/bms:DCClosed",
        // "variable2": "/assets/pcs/summary:maint_mode",
        // "variable3": "/status/pcs:SystemState",
        // "variable4": "/status/pcs:IsFaulted"

        bool maintMode = amap["maint_mode"]->getbVal();
        bool dcClosed = amap["DCClosed"]->getbVal();
        bool isFaulted = amap["IsFaulted"]->getbVal();
        char* systemStateStatus = amap["SystemState"]->getcVal();

        bool systemState = false;

        if(!(systemStateStatus == nullptr)){
            std::string compareString = systemStateStatus;
            systemState = (compareString == "Stop" || "Standby");
        }

        if(0)FPS_PRINT_INFO("dcClosed = [{}]", dcClosed);
        if(0)FPS_PRINT_INFO("maintMode = [{}]", maintMode);
        if(0)FPS_PRINT_INFO("systemState = [{}]", systemState);
        if(0)FPS_PRINT_INFO("!isFaulted = [{}]", !isFaulted);
        if(0)FPS_PRINT_INFO("(dcClosed && maintMode && systemState && !isFaulted) = [{}]", (dcClosed && maintMode && systemState && !isFaulted));



        // if (dcClosed && maintMode && systemState && !isFaulted) {
        if (dcClosed && maintMode && !isFaulted) {
            // FunctionUtility::InputOutputResultHandler(OutputHandler::StartPCS, vmap, amap, aname, p_fims, aV, __func__, __func__, "StartPCS", "start");
            FunctionUtility::FunctionResultHandler(0, vmap, amap, aname, p_fims, aV, __func__, __func__, "StartPCS", "start", "");
        } else {
            FunctionUtility::FunctionResultHandler(-1, vmap, amap, aname, p_fims, aV, __func__, __func__, "", "start", "(dcClosed && maintMode && !isFaulted) == false");
        }


    }

    void LocalStopPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
    {

        if(0)FPS_PRINT_INFO("LOCAL STOP PCS", nullptr);

        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;




        // // /assets/pcs/summary/maint_mode
        // assetVar* av = vm->getVar(vmap, "/assets/pcs/summary", "maint_mode");
        // if (!av) av = vm->makeVar(vmap, "/assets/pcs/summary", "maint_mode", FunctionUtility::bval);
        // amap["maint_mode"] = av;

        
        // // /status/pcs/SystemState
        // av = vm->getVar(vmap, "/status/pcs", "SystemState");
        // if (!av) av = vm->makeVar(vmap, "/status/pcs", "SystemState", FunctionUtility::cval);
        // amap["SystemState"] = av;

        std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
            // /assets/pcs/summary/stop
            FunctionUtility::AssetVarInfo("/assets/pcs/summary", "stop", assetVar::ATypes::ABOOL),
            // /assets/bms/summary/maint_mode
            FunctionUtility::AssetVarInfo("/assets/pcs/summary", "maint_mode", assetVar::ATypes::ABOOL),
            // /status/pcs/SystemState
            FunctionUtility::AssetVarInfo("/status/pcs", "SystemState", assetVar::ATypes::ASTRING)
        };
        amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

        //Function has been called but the assets uri hasn't been updated so it shouldn't have been called
        if(!amap["stop"]->getbVal()) return;

        // "enable": "/config/pcs:enable",
        // "expression": "{1} and {2} != Stop",
        // "variable1": "/assets/pcs/summary:maint_mode",
        // "variable2": "/status/pcs:SystemState"


        bool maintMode = amap["maint_mode"]->getbVal();
        char* systemStateStatus = amap["SystemState"]->getcVal();

        bool systemState = false;

        if(!(systemStateStatus == nullptr)){
            std::string compareString = systemStateStatus;
            systemState = (compareString != "Stop");
        }

        if(0)FPS_PRINT_INFO("maintMode = [{}]", maintMode);
        if(0)FPS_PRINT_INFO("systemState = [{}]", systemState);
        if(0)FPS_PRINT_INFO("(maintMode && systemState) = [{}]", (maintMode && systemState));

        // if (maintMode && systemState) {
        if (maintMode) {
            // FunctionUtility::InputOutputResultHandler(OutputHandler::StopPCS, vmap, amap, aname, p_fims, aV, __func__, __func__, "StopPCS", "stop");
            FunctionUtility::FunctionResultHandler(0, vmap, amap, aname, p_fims, aV, __func__, __func__, "StopPCS", "stop", "");
        } else {
            FunctionUtility::FunctionResultHandler(-1, vmap, amap, aname, p_fims, aV, __func__, __func__, "", "stop", "(maintMode) == false");
        }

    }

    void LocalStandbyPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
    {

        if(0)FPS_PRINT_INFO("LOCAL STANDBY PCS", nullptr);

        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;


        std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
            // /assets/pcs/summary/standby
            FunctionUtility::AssetVarInfo("/assets/pcs/summary", "standby", assetVar::ATypes::ABOOL),
            // /assets/bms/summary/maint_mode
            FunctionUtility::AssetVarInfo("/assets/pcs/summary", "maint_mode", assetVar::ATypes::ABOOL),
            // /status/bms/DCClosed
            FunctionUtility::AssetVarInfo("/status/bms", "DCClosed", assetVar::ATypes::ABOOL),
            // /status/pcs/SystemState
            FunctionUtility::AssetVarInfo("/status/pcs", "SystemState", assetVar::ATypes::ASTRING)
        };
        amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);


        //Function has been called but the assets uri hasn't been updated so it shouldn't have been called
        if(!amap["standby"]->getbVal()) return;

        bool maintMode = amap["maint_mode"]->getbVal();
        bool dcClosed = amap["DCClosed"]->getbVal();
        char* systemStateStatus = amap["SystemState"]->getcVal();

        bool systemState = false;

        if(!(systemStateStatus == nullptr)){
            std::string compareString = systemStateStatus;
            systemState = (compareString == "Stop" || "Run");
        }

        if(0)FPS_PRINT_INFO("dcClosed = [{}]", dcClosed);
        if(0)FPS_PRINT_INFO("maintMode = [{}]", maintMode);
        if(0)FPS_PRINT_INFO("systemState = [{}]", systemState);
        if(0)FPS_PRINT_INFO("(dcClosed && maintMode && systemState) = [{}]", (dcClosed && maintMode && systemState));

        // if (dcClosed && maintMode && systemState) {
        if (dcClosed && maintMode) {
            // FunctionUtility::InputOutputResultHandler(OutputHandler::StandbyPCS, vmap, amap, aname, p_fims, aV, __func__, __func__, "StandbyPCS", "standby");
            FunctionUtility::FunctionResultHandler(0, vmap, amap, aname, p_fims, aV, __func__, __func__, "StandbyPCS", "standby", "");
        } else {
            FunctionUtility::FunctionResultHandler(-1, vmap, amap, aname, p_fims, aV, __func__, __func__, "", "standby","(dcClosed && maintMode) == false");
        }

    }

    int RemoteStart(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, const char* scheduledFuncName) 
    {
       
        if(0)FPS_PRINT_INFO("RemoteStart", nullptr);


        int reload = 0;

        auto relname = fmt::format("{}_{}", __func__, "ess").c_str() ;
        assetVar* hpAv = amap[relname];
        if (!hpAv || (reload = hpAv->getiVal()) == 0)
        {
            reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
        }

        if (reload < 2)
        {

 
            if (reload == 0)
            {
                
            }
            reload = 2;
            hpAv->setVal(reload);
        }

        int returnValue = 0;
        // returnValue = OutputHandler::CloseContactors(vmap, amap, aname, p_fims, aV);
        FunctionUtility::FunctionResultHandler(returnValue, vmap, amap, aname, p_fims, aV, scheduledFuncName, __func__, "CloseContactors", "close_contactors", "");
        if(returnValue == 1 || returnValue == -1) return returnValue;


        // returnValue = OutputHandler::StartPCS(vmap, amap, aname, p_fims, aV);
        FunctionUtility::FunctionResultHandler(returnValue, vmap, amap, aname, p_fims, aV, scheduledFuncName, __func__, "StartPCS", "start", "");

        return returnValue;
        

    }

    int RemoteStop(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, const char* scheduledFuncName) 
    {
       
        if(0)FPS_PRINT_INFO("RemoteStop", nullptr);

        int returnValue = 0;

        // returnValue = OutputHandler::StopPCS(vmap, amap, aname, p_fims, aV);
        FunctionUtility::FunctionResultHandler(returnValue, vmap, amap, aname, p_fims, aV, scheduledFuncName, __func__, "StopPCS", "stop", "");
        if(returnValue == 1 || returnValue == -1) return returnValue;


        // returnValue = OutputHandler::OpenContactors(vmap, amap, aname, p_fims, aV);
        FunctionUtility::FunctionResultHandler(returnValue, vmap, amap, aname, p_fims, aV, scheduledFuncName, __func__, "OpenContactors", "open_contactors", "");

        return returnValue;

    }

    int RemoteStandby(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, const char* scheduledFuncName)
    {
       
        if(0)FPS_PRINT_INFO("RemoteStop", nullptr);

        int returnValue = 0;
        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;


        std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
            // /status/bms/DCClosed
            FunctionUtility::AssetVarInfo("/status/bms", "DCClosed", assetVar::ATypes::ABOOL),
            // /status/pcs/SystemState
            FunctionUtility::AssetVarInfo("/status/pcs", "SystemState", assetVar::ATypes::ASTRING)
        };
        amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

        bool dcClosed = amap["DCClosed"]->getbVal();
        char* systemStateStatus = amap["SystemState"]->getcVal();
        bool systemState = false;

        if(!(systemStateStatus == nullptr)){
            std::string compareString = systemStateStatus;
            systemState = (compareString == "On");
        }

        if(systemState){
            
            // returnValue = OutputHandler::StandbyPCS(vmap, amap, aname, p_fims, aV);
            FunctionUtility::FunctionResultHandler(returnValue, vmap, amap, aname, p_fims, aV, scheduledFuncName, __func__, "StandbyPCS", "standby", "");


        } else if(!dcClosed) {
            // returnValue = OutputHandler::CloseContactors(vmap, amap, aname, p_fims, aV);
            FunctionUtility::FunctionResultHandler(returnValue, vmap, amap, aname, p_fims, aV, scheduledFuncName, __func__, "CloseContactors", "close_contactors", "");

        } else {
            FunctionUtility::FunctionResultHandler(-1, vmap, amap, aname, p_fims, aV, scheduledFuncName, __func__, "", "", "SystemState != 'On' && DCClosed == true");
            return -1;
        }

        return returnValue;

        
    }

    void SiteRunCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
    {

        if(0)FPS_PRINT_INFO("SiteRunCmd", nullptr);
       
        int reload = 0;
        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;



        auto relname = fmt::format("{}_{}", __func__, "ess").c_str() ;
        assetVar* essAv = amap[relname];
        if (!essAv || (reload = essAv->getiVal()) == 0)
        {
            reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
        }

        if (reload < 2)
        {

            std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
                // /site/ess/start_stop
                FunctionUtility::AssetVarInfo("/site/ess", "start_stop", assetVar::ATypes::AFLOAT),
                // /assets/bms/summary/maint_mode
                FunctionUtility::AssetVarInfoWithManagerId("/assets/bms/summary", "maint_mode", "maint_mode_BMS", assetVar::ATypes::ABOOL),
                // /assets/pcs/summary/maint_mode
                FunctionUtility::AssetVarInfoWithManagerId("/assets/pcs/summary", "maint_mode", "maint_mode_PCS", assetVar::ATypes::ABOOL),
                // /status/bms/IsFaulted
                FunctionUtility::AssetVarInfo("/status/bms", "IsFaulted", assetVar::ATypes::ABOOL)
            };
            amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            if (reload == 0)
            {
                //If either the bms or the pcs are in maint_mode these methods can't be accessed
                if(amap["maint_mode_BMS"]->getbVal() || amap["maint_mode_PCS"]->getbVal()) {
                    FunctionUtility::FunctionResultHandler(-1, vmap, amap, aname, p_fims, aV, __func__, __func__, "", "", "maint_mode of BMS or PCS is true and both need to be false");
                    return;
                }

                //disabling the maintenance mode of the bms and pcs while running
                amap["maint_mode_BMS"]->setParam("enabled", false);
                amap["maint_mode_PCS"]->setParam("enabled", false);
                
            }
            reload = 2;
            essAv->setVal(reload);
        }


        if(amap["IsFaulted"]->getbVal()) {
            FunctionUtility::FunctionResultHandler(-1, vmap, amap, aname, p_fims, aV, __func__, __func__, "", "", "");
            return;
        }

        int stCommand = amap["start_stop"]->getiVal();
        int returnValue = 0;
        const char* inputHandlerFuncName = "";

        switch (stCommand) {
            //Shutdown
            case 0:
                inputHandlerFuncName = "RemoteStop";
                returnValue = RemoteStop(vmap, amap, aname, p_fims, aV, __func__);
                break;
            //Startup
            case 1:
                inputHandlerFuncName = "RemoteStart";
                returnValue = RemoteStart(vmap, amap, aname, p_fims, aV, __func__);
                break;
            //Standby
            case 2:
                inputHandlerFuncName = "RemoteStandby";
                returnValue = RemoteStandby(vmap, amap, aname, p_fims, aV, __func__);
                break;
            default:
                inputHandlerFuncName = "Unknown";
                returnValue = -1;
        }

        FunctionUtility::FunctionResultHandler(returnValue, vmap, amap, aname, p_fims, aV, __func__, inputHandlerFuncName, "CloseContactors", "close_contactors", "");
    }

}

#endif