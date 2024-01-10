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

        if (reload < 2)
        {

            linkVals(*vm, vmap, amap, bmsch, "/reload", reload, relname);
            cAv = amap[relname];

            std::vector<DataUtility::AssetVarInfo> assetVarVector = {
                // /assets/bms/summary/maint_mode
                DataUtility::AssetVarInfo("/assets/bms/summary", "maint_mode", assetVar::ATypes::ABOOL),
                // /assets/bms/summary/close_contactors
                DataUtility::AssetVarInfo("/assets/bms/summary", "close_contactors", assetVar::ATypes::ABOOL),
                // /status/bms/DCClosed
                DataUtility::AssetVarInfo("/status/bms", "DCClosed", assetVar::ATypes::ABOOL),
                // /status/bms/IsFaulted
                DataUtility::AssetVarInfo("/status/bms", "IsFaulted", assetVar::ATypes::ABOOL),
                // /status/bms/CloseContactorsEnabled
                DataUtility::AssetVarInfo("/status/bms", "CloseContactorsEnabled", assetVar::ATypes::ABOOL),
                // /controls/bms/CloseContactorsEnable
                DataUtility::AssetVarInfo("/controls/bms", "CloseContactorsEnable", assetVar::ATypes::ABOOL)
            };

            amap = DataUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            if (reload == 0)
            {

            }
            reload = 2;
            cAv->setVal(reload);
        }



        bool maintMode = amap["maint_mode"]->getbVal();
        bool dcClosed = amap["DCClosed"]->getbVal();
        bool isFaulted = amap["IsFaulted"]->getbVal();

        std::string message = "";

        if (!dcClosed && maintMode && !isFaulted) {
            message += "CloseContactorsEnable TRUE";
            amap["close_contactors"]->setParam("enabled", true);
            amap["CloseContactorsEnabled"]->setVal(true);
        } else {
            message += fmt::format(
                "CloseContactorsEnable FALSE ---> Condition(s): [{}:{}] == false && [{}:{}] == true && [{}:{}] == false",
                amap["DCClosed"]->getfName(), 
                amap["DCClosed"]->getbVal(),
                amap["maint_mode"]->getfName(), 
                amap["maint_mode"]->getbVal(),
                amap["IsFaulted"]->getfName(), 
                amap["IsFaulted"]->getbVal()
            );
            amap["close_contactors"]->setParam("enabled", false);
            amap["CloseContactorsEnabled"]->setVal(false);
        }

        if(0)FPS_PRINT_INFO("{}", message);


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

        if (reload < 2)
        {

            linkVals(*vm, vmap, amap, bmsch, "/reload", reload, relname);
            cAv = amap[relname];

            std::vector<DataUtility::AssetVarInfo> assetVarVector = {
                // /assets/bms/summary/maint_mode
                DataUtility::AssetVarInfo("/assets/bms/summary", "maint_mode", assetVar::ATypes::ABOOL),
                // /assets/bms/summary/open_contactors
                DataUtility::AssetVarInfo("/assets/bms/summary", "open_contactors", assetVar::ATypes::ABOOL),
                // /status/bms/DCClosed
                DataUtility::AssetVarInfo("/status/bms", "DCClosed", "DCClosed_BMS", assetVar::ATypes::ABOOL),
                // /status/bms/OpenContactorsEnabled
                DataUtility::AssetVarInfo("/status/bms", "OpenContactorsEnabled", assetVar::ATypes::ABOOL),
                // /controls/bms/OpenContactorsEnable
                DataUtility::AssetVarInfo("/controls/bms", "OpenContactorsEnable", assetVar::ATypes::ABOOL),
                // /status/pcs/DCClosed
                DataUtility::AssetVarInfo("/status/pcs", "DCClosed", "DCClosed_PCS", assetVar::ATypes::ABOOL)
            };

            amap = DataUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            if (reload == 0)
            {

            }
            reload = 2;
            cAv->setVal(reload);
        }



        std::string message = "";

        if (amap["DCClosed_BMS"]->getbVal() && !amap["DCClosed_PCS"]->getbVal() && amap["maint_mode"]->getbVal()) {
            message += fmt::format("{} TRUE", __func__);
            amap["open_contactors"]->setParam("enabled", true);
            amap["OpenContactorsEnabled"]->setVal(true);
        } else {
            message += fmt::format(
                "{} FALSE ---> Condition(s): [{}:{}] == true && [{}:{}] == false && [{}:{}] == true",
                __func__,
                amap["DCClosed_BMS"]->getfName(), 
                amap["DCClosed_BMS"]->getbVal(),
                amap["DCClosed_PCS"]->getfName(), 
                amap["DCClosed_PCS"]->getbVal(),
                amap["maint_mode"]->getfName(), 
                amap["maint_mode"]->getbVal()
            );
            amap["open_contactors"]->setParam("enabled", false);
            amap["OpenContactorsEnabled"]->setVal(false);
        }

        if(0)FPS_PRINT_INFO("{}", message);


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

        if (reload < 2)
        {

            linkVals(*vm, vmap, amap, pcsch, "/reload", reload, relname);
            cAv = amap[relname];

            std::vector<DataUtility::AssetVarInfo> assetVarVector = {
                // /status/bms/DCClosed
                DataUtility::AssetVarInfo("/status/bms", "DCClosed", assetVar::ATypes::ABOOL),
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

            if (reload == 0)
            {

            }
            reload = 2;
            cAv->setVal(reload);
        }


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
                "{} FALSE ---> Condition(s): [{}:{}] == true && [{}:{}] == true && [{}:{}] == (Stop or Standby) && [{}:{}] == false",
                __func__,
                amap["DCClosed"]->getfName(), 
                amap["DCClosed"]->getbVal(),
                amap["maint_mode"]->getfName(), 
                amap["maint_mode"]->getbVal(),
                amap["SystemState"]->getfName(), 
                systemStateVal,
                amap["IsFaulted"]->getfName(), 
                amap["IsFaulted"]->getbVal()
            );
            amap["start"]->setParam("enabled", false);
            amap["StartEnabled"]->setVal(false);
        }

        if(0)FPS_PRINT_INFO("{}", message);


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

        if (reload < 2)
        {

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

            if (reload == 0)
            {

            }
            reload = 2;
            cAv->setVal(reload);
        }


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
                "{} FALSE ---> Condition(s): [{}:{}] == true && [{}:{}] != Stop",
                __func__,
                amap["maint_mode"]->getfName(), 
                amap["maint_mode"]->getbVal(),
                amap["SystemState"]->getfName(), 
                systemStateVal
            );
            amap["stop"]->setParam("enabled", false);
            amap["StopEnabled"]->setVal(false);
        }

        if(0)FPS_PRINT_INFO("{}", message);


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

        if (reload < 2)
        {

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

            if (reload == 0)
            {

            }
            reload = 2;
            cAv->setVal(reload);
        }

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
                "{} FALSE ---> Condition(s): [{}:{}] == true && [{}:{}] == true && [{}:{}] == (Stop or Run)",
                __func__,
                amap["DCClosed"]->getfName(), 
                amap["DCClosed"]->getbVal(),
                amap["maint_mode"]->getfName(), 
                amap["maint_mode"]->getbVal(),
                amap["SystemState"]->getfName(), 
                systemStateVal
            );

            amap["standby"]->setParam("enabled", false);
            amap["StandbyEnabled"]->setVal(false);
        }

        if(0)FPS_PRINT_INFO("{}", message);

    }





}


#endif