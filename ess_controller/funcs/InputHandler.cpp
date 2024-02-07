



#ifndef INPUTHANDLER_CPP
#define INPUTHANDLER_CPP

#include "asset.h"
#include "assetVar.h"
#include "formatters.hpp"
#include "FunctionUtility.hpp"
#include "OutputHandler.hpp"
#include "InfoMessageUtility.hpp"
#include "SiteCommandUtility.hpp"
#include "DataUtility.hpp"
#include "BatteryBalancingUtility.hpp"




namespace InputHandler
{

// ==================== BMS Functions ====================

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
        // DataUtility::PrintAssetVar(aV, assetVar::ATypes::AINT);
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


// ==================== PCS Functions ====================

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



// ==================== Site Level Functions ====================

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
            bmsAv = amap[bmsRelName];
            linkVals(*vm, vmap, amap, aname, "/reload", reload, pcsRelName);
            pcsAv = amap[pcsRelName];

            std::vector<DataUtility::AssetVarInfo> assetVarVector = {
                // /site/ess/start_stop
                DataUtility::AssetVarInfo("/site/ess", siteUri.c_str(), assetVar::ATypes::AINT),
                // /assets/bms/summary/maint_mode
                DataUtility::AssetVarInfo("/assets/bms/summary", "maint_mode", "maint_mode_BMS", assetVar::ATypes::ABOOL),
                // /assets/pcs/summary/maint_mode
                DataUtility::AssetVarInfo("/assets/pcs/summary", "maint_mode", "maint_mode_PCS", assetVar::ATypes::ABOOL),
                // /status/bms/IsFaulted
                DataUtility::AssetVarInfo("/status/bms", "IsFaulted", assetVar::ATypes::ABOOL)
            };
            amap = DataUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

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
                    returnValue = SiteCommandUtility::RemoteStop(vmap, amap, aname, p_fims, aV, __func__);
                    break;
                //Startup
                case 1:
                    returnValue = SiteCommandUtility::RemoteStart(vmap, amap, aname, p_fims, aV, __func__);
                    break;
                //Standby
                case 2:
                    returnValue = SiteCommandUtility::RemoteStandby(vmap, amap, aname, p_fims, aV, __func__);
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
    * @brief 
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
            std::vector<DataUtility::AssetVarInfo> assetVarVector = {
                // /site/ess/start_stop
                DataUtility::AssetVarInfo("/site/ess", siteUri.c_str(), assetVar::ATypes::AINT),
                // /status/bms/IsFaulted
                DataUtility::AssetVarInfo("/status/bms", "IsFaulted", assetVar::ATypes::ABOOL)
            };
            amap = DataUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            reload = 1;
            essAv->setVal(reload);
            return;
        }

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
                    returnValue = FunctionUtility::SharedInputHandlerRemoteFunction(vmap, amap, aname, p_fims, aV, siteUri, __func__, "CloseContactors");
                    break;
                //OpenContactors
                case 3:
                    returnValue = FunctionUtility::SharedInputHandlerRemoteFunction(vmap, amap, aname, p_fims, aV, siteUri, __func__, "OpenContactors");
                    break;
                default:
                    returnValue = FAILURE;
                    break;
            }



            if(returnValue == SUCCESS || returnValue == FAILURE) {
                reload = 1;
                essAv->setVal(reload);
            }
        }
    }

    /**
    * @brief 
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
            std::vector<DataUtility::AssetVarInfo> assetVarVector = {
                // /site/ess/start_stop
                DataUtility::AssetVarInfo("/site/ess", siteUri.c_str(), assetVar::ATypes::AINT),
                // /status/bms/IsFaulted
                DataUtility::AssetVarInfo("/status/pcs", "IsFaulted", assetVar::ATypes::ABOOL)
            };
            amap = DataUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

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

            FPS_PRINT_INFO("reload == 2");
        
            int stCommand = amap[siteUri.c_str()]->getiVal();

            FPS_PRINT_INFO("start_stop_standby_command == {}", stCommand);

            int returnValue = 0;

            switch (stCommand) {
                //Stop
                case 0:
                    returnValue = FunctionUtility::SharedInputHandlerRemoteFunction(vmap, amap, aname, p_fims, aV, siteUri, __func__, "Stop");
                    break;
                //Start
                case 1:
                    returnValue = FunctionUtility::SharedInputHandlerRemoteFunction(vmap, amap, aname, p_fims, aV, siteUri, __func__, "Start");
                    break;
                //Standby
                case 2:
                    returnValue = FunctionUtility::SharedInputHandlerRemoteFunction(vmap, amap, aname, p_fims, aV, siteUri, __func__, "Standby");
                    break;
                default:
                    returnValue = FAILURE;
                    break;
            }



            if(returnValue == SUCCESS || returnValue == FAILURE) {
                reload = 1;
                essAv->setVal(reload);
            }
        }

    }


    /**
    * @brief 
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    */
    void BatteryRackBalanceCoarse(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
    {

        if(0)FPS_PRINT_INFO("{}", __func__);
        // FPS_PRINT_INFO("Before Test");
        // BatteryBalancingUtility::TestVoltageArbitration();
        // BatteryBalancingUtility::TestActivePowerBalancing();
        // FPS_PRINT_INFO("After Test");
       

        int reload = 0;
        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;

        const char *bmsch = (const char*)"bms_1";
        if (aV->gotParam("bms"))
        {
            bmsch = aV->getcParam("bms");
        }

        int returnValue = -1;
        std::string message = "";


        std::string uri = "battery_rack_balance_coarse";

        auto relname = fmt::format("{}_{}_{}", __func__, "ess", bmsch).c_str();
        assetVar* essAv = amap[relname];
        if (!essAv || (reload = essAv->getiVal()) == 0)
        {
            reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
        }

        // Set up
        if (reload == 0)
        {
            FPS_PRINT_INFO("reload == 0");

            linkVals(*vm, vmap, amap, aname, "/reload", reload, relname);
            essAv = amap[relname];

            std::vector<DataUtility::AssetVarInfo> assetVarVector = {
                // /sched/ess/BatteryRackBalanceCoarse
                DataUtility::AssetVarInfo("/sched/ess", "BatteryRackBalanceCoarse", assetVar::ATypes::ABOOL),
                // /assets/ess/summary/battery_rack_balance_coarse
                DataUtility::AssetVarInfo("/assets/ess/summary", "battery_rack_balance_coarse", assetVar::ATypes::ABOOL),
                // /config/bms/NumRacks
                DataUtility::AssetVarInfo("/config/bms", "NumRacks", assetVar::ATypes::AINT),
                // // /status/bms/NumRacksTotal
                // DataUtility::AssetVarInfo("/status/bms", "NumRacksTotal", assetVar::ATypes::AINT),
                // /status/bms/DCVoltage
                DataUtility::AssetVarInfo("/status/bms", "DCVoltage", assetVar::ATypes::AFLOAT),
                // /status/bms/SOC
                DataUtility::AssetVarInfo("/status/bms", "SOC", assetVar::ATypes::AFLOAT),
                // /status/bms/DCClosed
                DataUtility::AssetVarInfo("/status/bms", "DCClosed", assetVar::ATypes::ABOOL),
                // /status/bms/IsFaulted
                DataUtility::AssetVarInfo("/status/bms", "IsFaulted", "IsFaulted_BMS", assetVar::ATypes::ABOOL),
                // /status/pcs/IsFaulted
                DataUtility::AssetVarInfo("/status/pcs", "IsFaulted", "IsFaulted_PCS", assetVar::ATypes::ABOOL),
                // /controls/pcs/ActivePowerSetpoint
                DataUtility::AssetVarInfo("/controls/pcs", "ActivePowerSetpoint", assetVar::ATypes::AFLOAT)
            };

            amap = DataUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);


            // DataUtility::PrintAssetVar(amap["battery_rack_balance_coarse"], assetVar::ATypes::ABOOL);
            // FPS_PRINT_INFO("Before Test");
            // BatteryBalancingUtility::TestVoltageArbitration(amap);
            // FPS_PRINT_INFO("After Test");


            reload = 1;
            essAv->setVal(reload);

            if(amap[uri.c_str()]->getbVal() == false) {
                FunctionUtility::PullOffScheduler(amap, aV, uri.c_str());
            }

            return;
        }

        // Set up
        if (reload == 1)
        {

            int numRacks = amap["NumRacks"]->getiVal();
            FPS_PRINT_INFO("reload == 1 | numRacks[{}]", numRacks);
            if(numRacks > 0) {
                 std::vector<DataUtility::AssetVarInfo> rackVector = {};
                for (int i = 1; i > numRacks; i++) {
                    std::string rackUri = fmt::format("/status/{}_rack_{}", bmsch, i);
                    std::string dcClosedName = fmt::format("DCClosed_rack_{}", i);
                    std::string dcVoltageName = fmt::format("DCVoltage_rack_{}", i);

                    // FPS_PRINT_INFO("rackUri - {}", rackUri);
                    // FPS_PRINT_INFO("dcClosedName - {}", dcClosedName);
                    // FPS_PRINT_INFO("dcVoltageName - {}", dcVoltageName);

                    rackVector.push_back(
                        DataUtility::AssetVarInfo(rackUri.c_str(), "DCClosed", dcClosedName.c_str(), assetVar::ATypes::ABOOL)
                    );
                    rackVector.push_back(
                        DataUtility::AssetVarInfo(rackUri.c_str(), "DCVoltage", dcVoltageName.c_str(), assetVar::ATypes::AFLOAT)
                    );

                } 

                amap = DataUtility::PopulateAmapWithManyAvs(vmap, amap, vm, rackVector);

                reload = 2;
                essAv->setVal(reload);

            }
            

            // DataUtility::PrintAssetVar(amap["battery_rack_balance_coarse"], assetVar::ATypes::ABOOL);
            // FPS_PRINT_INFO("Before Test");
            // BatteryBalancingUtility::TestVoltageArbitration(amap);
            // FPS_PRINT_INFO("After Test");

            return;
        }


        // Fault Checking
        if (reload == 2)
        {

            if(amap[uri.c_str()]->getbVal() == false) {
                FunctionUtility::PullOffScheduler(amap, aV, uri.c_str());
                return;
            }

            if(amap["IsFaulted_BMS"]->getbVal()) {
                std::string message = fmt::format(
                    "Condition(s): [{}:{}] == false",
                    amap["IsFaulted_BMS"]->getfName(), 
                    amap["IsFaulted_BMS"]->getbVal()
                );
                FunctionUtility::FunctionResultHandler(-1, vmap, amap, aname, p_fims, aV, __func__, uri.c_str(), message.c_str());
                return;
            }

            if(amap["IsFaulted_PCS"]->getbVal()) {
                std::string message = fmt::format(
                    "Condition(s): [{}:{}] == false",
                    amap["IsFaulted_PCS"]->getfName(), 
                    amap["IsFaulted_PCS"]->getbVal()
                );
                FunctionUtility::FunctionResultHandler(-1, vmap, amap, aname, p_fims, aV, __func__, uri.c_str(), message.c_str());
                return;
            }


            reload = 2;
            essAv->setVal(reload);
            return;  
        }

        //Start BMS
        if(reload == 3){

            amap[uri.c_str()]->setParam("every", aV->getdParam("every"));

            returnValue = FunctionUtility::SharedInputHandlerRemoteFunction(vmap, amap, aname, p_fims, aV, uri, __func__, "CloseContactors");
            // FunctionUtility::FunctionReturnObj returnObject = OutputHandler::CloseContactors(vmap, amap, aname, p_fims, aV, "battery_rack_balance_coarse");
            // returnValue = returnObject.statusIndicator;
            // message = returnObject.message;


            if(returnValue == SUCCESS) {
                aV->setParam("endTime", 0);
                reload = 3;
                essAv->setVal(reload);
                returnValue = IN_PROGRESS;
            }

            return;

        }

        //Start PCS
        if(reload == 4){

            amap[uri.c_str()]->setParam("every", aV->getdParam("every"));

            returnValue = FunctionUtility::SharedInputHandlerRemoteFunction(vmap, amap, aname, p_fims, aV, uri, __func__, "Start");
            // FunctionUtility::FunctionReturnObj returnObject = OutputHandler::StartPCS(vmap, amap, aname, p_fims, aV, "battery_rack_balance_coarse");
            // returnValue = returnObject.statusIndicator;
            // message = returnObject.message;


            if(returnValue == SUCCESS) {
                aV->setParam("endTime", 0);
                reload = 4;
                essAv->setVal(reload);
                returnValue = IN_PROGRESS;
            }

            amap["ActivePowerSetpoint"]->setVal(-100);
            return;

        }

        //Set Active Power Setpoint
        // TODO make this function
        if(reload == 5){

            double currentActivePowerSetpoint = amap["ActivePowerSetpoint"]->getdVal();

            if(currentActivePowerSetpoint >= 100){
                reload = 5;
                essAv->setVal(reload);
            } else {

                // double controlRate = amap["battery_rack_balance_coarse"]->getdParam("every");
                // double controlRate = amap["battery_rack_balance_coarse"]->getdParam("ControlRate");
                double gain = amap["battery_rack_balance_coarse"]->getdParam("Gain");
                double newActivePowerSetpoint = currentActivePowerSetpoint + gain;

                amap["ActivePowerSetpoint"]->setVal(newActivePowerSetpoint);
            }

            //state machine in this state
            //state 1 is voltage arbitration
            //state 2 is active power balancing
            //state 3 is maninpulating rack contactors

            // double deadband = amap["battery_rack_balance_coarse"]->getdParam("targetVoltageDeadband");
            // //TODO make getting rack info a 1 time thing
            // std::vector<BatteryBalancingUtility::RackInfoObject> racks = BatteryBalancingUtility::GetRackInfoList(amap);
            // BatteryBalancingUtility::VoltageArbitrationResult result = BatteryBalancingUtility::VoltageArbitration(deadband, racks);

            // if(result.balancingNeeded){
                
            //     BatteryBalancingUtility::ActivePowerBalancingInput inputs = BatteryBalancingUtility::GetActivePowerBalancingInfo(amap, result);
            //     double setpoint = BatteryBalancingUtility::ActivePowerBalancing(inputs);
            //     amap["ActivePowerSetpoint"]->setVal(setpoint);

            // } else {
            //     reload = 5;
            //     essAv->setVal(reload);
            // }

            return;
        }

        //Stop PCS
        if(reload == 6){

            amap[uri.c_str()]->setParam("every", aV->getdParam("every"));

            returnValue = FunctionUtility::SharedInputHandlerRemoteFunction(vmap, amap, aname, p_fims, aV, uri, __func__, "Stop");
            // FunctionUtility::FunctionReturnObj returnObject = OutputHandler::StopPCS(vmap, amap, aname, p_fims, aV, "battery_rack_balance_coarse");
            // returnValue = returnObject.statusIndicator;
            // message = returnObject.message;


            if(returnValue == SUCCESS) {
                aV->setParam("endTime", 0);
                reload = 6;
                essAv->setVal(reload);
                returnValue = IN_PROGRESS;
            }
            return;

        }

        //Stop BMS
        if(reload == 7){

            amap[uri.c_str()]->setParam("every", aV->getdParam("every"));

            returnValue = FunctionUtility::SharedInputHandlerRemoteFunction(vmap, amap, aname, p_fims, aV, uri, __func__, "OpenContactors");
            // FunctionUtility::FunctionReturnObj returnObject = OutputHandler::OpenContactors(vmap, amap, aname, p_fims, aV, "battery_rack_balance_coarse");
            // returnValue = returnObject.statusIndicator;
            // message = returnObject.message;

        }

        if(returnValue == SUCCESS || returnValue == FAILURE) {
            reload = 1;
            essAv->setVal(reload);
        }

        // FunctionUtility::FunctionResultHandler(returnValue, vmap, amap, aname, p_fims, aV, __func__, uri.c_str(), message.c_str());

    }


}


#endif