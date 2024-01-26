#ifndef SCHEDULEDENABLEFUNCTIONS_CPP
#define SCHEDULEDENABLEFUNCTIONS_CPP

#include "asset.h"
#include "assetVar.h"
#include "formatters.hpp"
#include "FunctionUtility.hpp"
#include "OutputHandler.hpp"
#include "InfoMessageUtility.hpp"
#include "SiteCommandUtility.hpp"
#include "DataUtility.hpp"



namespace ScheduledEnableFunctions
{

// ==================== Enable Functions ====================

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


        std::string maint_mode = (fmt::format("maint_mode_{}", bmsch));
        std::string close_contactors = (fmt::format("close_contactors_{}", bmsch));
        std::string DCClosed = (fmt::format("DCClosed_{}", bmsch));
        std::string IsFaulted = (fmt::format("IsFaulted_{}", bmsch));
        std::string CloseContactorsEnabled = (fmt::format("CloseContactorsEnabled_{}", bmsch));
        std::string CloseContactorsEnable = (fmt::format("CloseContactorsEnable_{}", bmsch));


        if(reload == 0){


            std::string assetsString = "";
            if (std::strcmp(bmsch, "bms") == 0) {
                assetsString = "summary";
            } else {
                assetsString = bmsch;
            }

            std::string assets = (fmt::format("/assets/bms/{}", assetsString));
            std::string status = (fmt::format("/status/{}", bmsch));
            std::string controls = (fmt::format("/controls/{}", bmsch));

            linkVals(*vm, vmap, amap, bmsch, "/reload", reload, relname);
            cAv = amap[relname];


            std::string maint_mode = (fmt::format("maint_mode_{}", bmsch));
            std::string close_contactors = (fmt::format("close_contactors_{}", bmsch));
            std::string DCClosed = (fmt::format("DCClosed_{}", bmsch));
            std::string IsFaulted = (fmt::format("IsFaulted_{}", bmsch));
            std::string CloseContactorsEnabled = (fmt::format("CloseContactorsEnabled_{}", bmsch));
            std::string CloseContactorsEnable = (fmt::format("CloseContactorsEnable_{}", bmsch));
            

            std::vector<DataUtility::AssetVarInfo> assetVarVector = {
                // /assets/bms/summary/maint_mode
                DataUtility::AssetVarInfo(assets.c_str(), "maint_mode", maint_mode.c_str(), assetVar::ATypes::ABOOL),
                // /assets/bms/summary/close_contactors
                DataUtility::AssetVarInfo(assets.c_str(), "close_contactors", close_contactors.c_str(), assetVar::ATypes::ABOOL),
                // /status/bms/DCClosed
                DataUtility::AssetVarInfo(status.c_str(), "DCClosed", DCClosed.c_str(), assetVar::ATypes::ABOOL),
                // /status/bms/IsFaulted
                DataUtility::AssetVarInfo(status.c_str(), "IsFaulted", IsFaulted.c_str(), assetVar::ATypes::ABOOL),
                // /status/bms/CloseContactorsEnabled
                DataUtility::AssetVarInfo(status.c_str(), "CloseContactorsEnabled", CloseContactorsEnabled.c_str(), assetVar::ATypes::ABOOL),
                // /controls/bms/CloseContactorsEnable
                DataUtility::AssetVarInfo(controls.c_str(), "CloseContactorsEnable", CloseContactorsEnable.c_str(), assetVar::ATypes::ABOOL)
            };

            amap = DataUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            reload = 1;
        }

        if(reload == 1){
            std::string message = "";

            if (!amap[DCClosed.c_str()]->getbVal() && amap[maint_mode.c_str()]->getbVal() && !amap[IsFaulted.c_str()]->getbVal()) {
                message += "CloseContactorsEnable TRUE";
                amap[close_contactors.c_str()]->setParam("enabled", true);
                amap[CloseContactorsEnabled.c_str()]->setVal(true);
                cAv->setVal(reload);
            } else {
                message += fmt::format(
                    " ---> Condition(s): [{}:{}] == false && [{}:{}] == true && [{}:{}] == false",
                    amap[DCClosed.c_str()]->getfName(), 
                    amap[DCClosed.c_str()]->getbVal(),
                    amap[maint_mode.c_str()]->getfName(), 
                    amap[maint_mode.c_str()]->getbVal(),
                    amap[IsFaulted.c_str()]->getfName(), 
                    amap[IsFaulted.c_str()]->getbVal()
                );
                DataUtility::UpdateEnabledLogicMessage("close_contactors", message);
                amap[close_contactors.c_str()]->setParam("enabled", false);
                amap[CloseContactorsEnabled.c_str()]->setVal(false);
                cAv->setVal(reload);
            }

            if(0)FPS_PRINT_INFO("{}", message);
        }

        cAv->setVal(reload);
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


        std::string maint_mode = (fmt::format("maint_mode_{}", bmsch));
        std::string open_contactors = (fmt::format("open_contactors_{}", bmsch));
        std::string DCClosed = (fmt::format("DCClosed_{}", bmsch));
        std::string OpenContactorsEnabled = (fmt::format("OpenContactorsEnabled_{}", bmsch));
        std::string OpenContactorsEnable = (fmt::format("OpenContactorsEnable_{}", bmsch));
        

        if(reload == 0){

            std::string assetsString = "";
            if (std::strcmp(bmsch, "bms") == 0) {
                assetsString = "summary";
            } else {
                assetsString = bmsch;
            }

            linkVals(*vm, vmap, amap, bmsch, "/reload", reload, relname);
            cAv = amap[relname];

            std::string assets = (fmt::format("/assets/bms/{}", assetsString));
            std::string status = (fmt::format("/status/{}", bmsch));
            std::string controls = (fmt::format("/controls/{}", bmsch));



            std::vector<DataUtility::AssetVarInfo> assetVarVector = {
                // /assets/bms/summary/maint_mode
                DataUtility::AssetVarInfo(assets.c_str(), "maint_mode", maint_mode.c_str(), assetVar::ATypes::ABOOL),
                // /assets/bms/summary/open_contactors
                DataUtility::AssetVarInfo(assets.c_str(), "open_contactors", open_contactors.c_str(), assetVar::ATypes::ABOOL),
                // /status/bms/DCClosed
                DataUtility::AssetVarInfo(status.c_str(), "DCClosed", DCClosed.c_str(), assetVar::ATypes::ABOOL),
                // /status/bms/OpenContactorsEnabled
                DataUtility::AssetVarInfo(status.c_str(), "OpenContactorsEnabled", OpenContactorsEnabled.c_str(), assetVar::ATypes::ABOOL),
                // /controls/bms/OpenContactorsEnable
                DataUtility::AssetVarInfo(controls.c_str(), "OpenContactorsEnable", OpenContactorsEnable.c_str(), assetVar::ATypes::ABOOL),
                // /status/pcs/DCClosed
                DataUtility::AssetVarInfo("/status/pcs", "DCClosed", "DCClosed_PCS", assetVar::ATypes::ABOOL)
            };

            amap = DataUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            reload = 1;
        }

        if(reload == 1){
            std::string message = "";

            if (amap[DCClosed.c_str()]->getbVal() && !amap["DCClosed_PCS"]->getbVal() && amap[maint_mode.c_str()]->getbVal()) {
                message += fmt::format("{} TRUE", __func__);
                amap[open_contactors.c_str()]->setParam("enabled", true);
                amap[OpenContactorsEnabled.c_str()]->setVal(true);
            } else {
                message += fmt::format(
                    " ---> Condition(s): [{}:{}] == true && [{}:{}] == false && [{}:{}] == true",
                    amap[DCClosed.c_str()]->getfName(), 
                    amap[DCClosed.c_str()]->getbVal(),
                    amap["DCClosed_PCS"]->getfName(), 
                    amap["DCClosed_PCS"]->getbVal(),
                    amap[maint_mode.c_str()]->getfName(), 
                    amap[maint_mode.c_str()]->getbVal()
                );
                DataUtility::UpdateEnabledLogicMessage(open_contactors.c_str(), message);
                amap[open_contactors.c_str()]->setParam("enabled", false);
                amap[OpenContactorsEnabled.c_str()]->setVal(false);
            }

            if(0)FPS_PRINT_INFO("{}", message);
        }

        cAv->setVal(reload);

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

            std::vector<DataUtility::AssetVarInfo> assetVarVector = {
                // /status/bms/DCClosed
                DataUtility::AssetVarInfo("/status/bms", "DCClosed", "DCClosed_bms", assetVar::ATypes::ABOOL),
                // /assets/pcs/summary/maint_mode
                DataUtility::AssetVarInfo("/assets/pcs/summary", "maint_mode", assetVar::ATypes::ABOOL),
                // /assets/pcs/summary/start
                DataUtility::AssetVarInfo("/assets/pcs/summary", "start", assetVar::ATypes::ABOOL),
                // /status/pcs/SystemState
                DataUtility::AssetVarInfo("/status/pcs", "SystemState", assetVar::ATypes::ASTRING),
                // /status/pcs/IsFaulted
                DataUtility::AssetVarInfo("/status/pcs", "IsFaulted", assetVar::ATypes::ABOOL),
                // /status/pcs/StartEnabled
                DataUtility::AssetVarInfo("/status/pcs", "StartEnabled", assetVar::ATypes::ABOOL),
                // /controls/pcs/StartEnable
                DataUtility::AssetVarInfo("/controls/pcs", "StartEnable", assetVar::ATypes::ABOOL)
            };


            amap = DataUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

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

            if (amap["DCClosed_bms"]->getbVal() && amap["maint_mode"]->getbVal() && (systemState) && !amap["IsFaulted"]->getbVal()) {
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
                    amap["DCClosed_bms"]->getfName(), 
                    amap["DCClosed_bms"]->getbVal(),
                    amap["maint_mode"]->getfName(), 
                    amap["maint_mode"]->getbVal(),
                    amap["SystemState"]->getfName(), 
                    systemStateVal,
                    amap["IsFaulted"]->getfName(), 
                    amap["IsFaulted"]->getbVal()
                );
                DataUtility::UpdateEnabledLogicMessage("start", message);
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

            std::vector<DataUtility::AssetVarInfo> assetVarVector = {
                // /assets/pcs/summary/maint_mode
                DataUtility::AssetVarInfo("/assets/pcs/summary", "maint_mode", assetVar::ATypes::ABOOL),
                // /assets/pcs/summary/stop
                DataUtility::AssetVarInfo("/assets/pcs/summary", "stop", assetVar::ATypes::ABOOL),
                // /status/pcs/SystemState
                DataUtility::AssetVarInfo("/status/pcs", "SystemState", assetVar::ATypes::ASTRING),
                // /status/pcs/StopEnabled
                DataUtility::AssetVarInfo("/status/pcs", "StopEnabled", assetVar::ATypes::ABOOL),
                // /controls/pcs/StopEnable
                DataUtility::AssetVarInfo("/controls/pcs", "StopEnable", assetVar::ATypes::ABOOL)
            };


            amap = DataUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

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
                DataUtility::UpdateEnabledLogicMessage("stop", message);
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

            std::vector<DataUtility::AssetVarInfo> assetVarVector = {
                // /status/bms/DCClosed
                DataUtility::AssetVarInfo("/status/bms", "DCClosed", assetVar::ATypes::ABOOL),
                // /assets/pcs/summary/maint_mode
                DataUtility::AssetVarInfo("/assets/pcs/summary", "maint_mode", assetVar::ATypes::ABOOL),
                // /assets/pcs/summary/standby
                DataUtility::AssetVarInfo("/assets/pcs/summary", "standby", assetVar::ATypes::ABOOL),
                // /status/pcs/SystemState
                DataUtility::AssetVarInfo("/status/pcs", "SystemState", assetVar::ATypes::ASTRING),
                // /status/pcs/StandbyEnabled
                DataUtility::AssetVarInfo("/status/pcs", "StandbyEnabled", assetVar::ATypes::ABOOL),
                // /controls/pcs/StandbyEnable
                DataUtility::AssetVarInfo("/controls/pcs", "StandbyEnable", assetVar::ATypes::ABOOL)
            };


            amap = DataUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

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

                DataUtility::UpdateEnabledLogicMessage("standby", message);

                amap["standby"]->setParam("enabled", false);
                amap["StandbyEnabled"]->setVal(false);
            }

            if(0)FPS_PRINT_INFO("{}", message);
        }

    }



}


#endif