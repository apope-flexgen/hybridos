#ifndef OUTPUTHANDLER_CPP
#define OUTPUTHANDLER_CPP

#include "asset.h"
#include "formatters.hpp"
#include "FunctionUtility.hpp"
#include "OutputHandler.hpp"
#include "DataUtility.hpp"




char* strtime(const struct tm *timeptr);

/**
 * 
 */
 extern "C++" {
    int HandleCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}

namespace OutputHandler {

    /**
    * @brief Internal function called by an InputHandler Function to OpenContactors
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    */
    FunctionUtility::FunctionReturnObj OpenContactors(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, const char* processOriginUri)
    {

        if(1)FPS_PRINT_INFO("{} | {}", __func__, processOriginUri);

        FunctionUtility::FunctionReturnObj returnObject; 

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

        auto relname = fmt::format("{}_{}", __func__, bmsch).c_str() ;
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
                // /status/bms/OpenContactorsSuccess
                DataUtility::AssetVarInfo("/status/bms", "OpenContactorsSuccess", assetVar::ATypes::ABOOL),
                // /status/bms/VerifyOpenContactorsSuccess
                DataUtility::AssetVarInfo("/status/bms", "VerifyOpenContactorsSuccess", assetVar::ATypes::ABOOL),
                // /controls/bms/OpenContactors
                DataUtility::AssetVarInfo("/controls/bms", "OpenContactors", assetVar::ATypes::AFLOAT),
                // /controls/bms:VerifyOpenContactors
                DataUtility::AssetVarInfo("/controls/bms", "VerifyOpenContactors", assetVar::ATypes::AFLOAT),
                // /alarms/bms:OpenContactors
                DataUtility::AssetVarInfo("/alarms/bms", "OpenContactors", "OpenContactors_ALARM", assetVar::ATypes::ASTRING),
                // /alarms/bms:VerifyOpenContactors
                DataUtility::AssetVarInfo("/alarms/bms", "VerifyOpenContactors", "VerifyOpenContactors_ALARM", assetVar::ATypes::ASTRING)
            };

            amap = DataUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);



            if (reload == 0)
            {
                // amap["OpenContactors"]->setcParam("expression", "{1} and ({2} == Stop or {2} == Fault)");
                // amap["OpenContactors"]->setParam("numVars", 2);
                // amap["OpenContactors"]->setParam("useExpr", true);
                // amap["OpenContactors"]->setcParam("variable1", "/status/bms:DCClosed");
                // amap["OpenContactors"]->setcParam("variable2", "/status/pcs:SystemState");
            }

            reload = 2;
            cAv->setVal(reload);
        }


        amap = FunctionUtility::SharedAmapReset(amap, vm, processOriginUri, "OpenContactors", "VerifyOpenContactors");
        returnObject = FunctionUtility::SharedHandleCmdProcess(vmap, amap, bmsch, p_fims, aV, "OpenContactors");

        return returnObject;

    }

    /**
    * @brief Internal function called by an InputHandler Function to CloseContactors
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    */
    FunctionUtility::FunctionReturnObj CloseContactors(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, const char* processOriginUri)
    {

        if(1)FPS_PRINT_INFO("{} | {}", __func__, processOriginUri);

 
        FunctionUtility::FunctionReturnObj returnObject; 

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
                // /status/bms/CloseContactorsSuccess
                DataUtility::AssetVarInfo("/status/bms", "CloseContactorsSuccess", assetVar::ATypes::ABOOL),
                // /status/bms/VerifyCloseContactorsSuccess
                DataUtility::AssetVarInfo("/status/bms", "VerifyCloseContactorsSuccess", assetVar::ATypes::ABOOL),
                // /controls/bms/CloseContactors
                DataUtility::AssetVarInfo("/controls/bms", "CloseContactors", assetVar::ATypes::AFLOAT),
                // /controls/bms:VerifyCloseContactors
                DataUtility::AssetVarInfo("/controls/bms", "VerifyCloseContactors", assetVar::ATypes::AFLOAT),
                // /alarms/bms:CloseContactors
                DataUtility::AssetVarInfo("/alarms/bms", "CloseContactors", "CloseContactors_ALARM", assetVar::ATypes::ASTRING),
                // /alarms/bms:VerifyCloseContactors
                DataUtility::AssetVarInfo("/alarms/bms", "VerifyCloseContactors", "VerifyCloseContactors_ALARM", assetVar::ATypes::ASTRING)
            };

            amap = DataUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            if (reload == 0)
            {
                // amap["CloseContactors"]->setcParam("expression", "not {1} and {2} == Stop and not {3}");
                // amap["CloseContactors"]->setParam("numVars", 3);
                // amap["CloseContactors"]->setParam("useExpr", true);
                // amap["CloseContactors"]->setcParam("variable1", "/status/bms:DCClosed");
                // amap["CloseContactors"]->setcParam("variable2", "/status/pcs:SystemState");
                // amap["CloseContactors"]->setcParam("variable3", "/status/bms:IsFaulted");
            }
            cAv->setVal(2);
        }


        amap = FunctionUtility::SharedAmapReset(amap, vm, processOriginUri, "CloseContactors", "VerifyCloseContactors");
        returnObject = FunctionUtility::SharedHandleCmdProcess(vmap, amap, bmsch, p_fims, aV, "CloseContactors");

        return returnObject;
    }

    /**
    * @brief Internal function called by an InputHandler Function to StartPCS
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    */
    FunctionUtility::FunctionReturnObj StartPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, const char* processOriginUri)
    {

        if(1)FPS_PRINT_INFO("{} | {}", __func__, processOriginUri);

        FunctionUtility::FunctionReturnObj returnObject; 

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
                // /status/pcs/StartSuccess
                DataUtility::AssetVarInfo("/status/pcs", "StartSuccess", assetVar::ATypes::ABOOL),
                // /status/bms/VerifyStartSuccess
                DataUtility::AssetVarInfo("/status/bms", "VerifyStartSuccess", assetVar::ATypes::ABOOL),
                // /controls/pcs/Start
                DataUtility::AssetVarInfo("/controls/pcs", "Start", assetVar::ATypes::AFLOAT),
                // /controls/pcs:VerifyStart
                DataUtility::AssetVarInfo("/controls/pcs", "VerifyStart", assetVar::ATypes::AFLOAT),
                // /alarms/pcs:Start
                DataUtility::AssetVarInfo("/alarms/pcs", "Start", "Start_ALARM", assetVar::ATypes::ASTRING),
                // /alarms/pcs:VerifyStart
                DataUtility::AssetVarInfo("/alarms/pcs", "VerifyStart", "VerifyStart_ALARM", assetVar::ATypes::ASTRING)
            };

            amap = DataUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            if (reload == 0)
            {
                // amap["Start"]->setcParam("expression", "({1} == Stop or {1} == Standby) and {2} and not {3}");
                // amap["Start"]->setParam("numVars", 3);
                // amap["Start"]->setParam("useExpr", true);
                // amap["Start"]->setcParam("variable1", "/status/pcs:SystemState");
                // amap["Start"]->setcParam("variable2", "/status/bms:DCClosed");
                // amap["Start"]->setcParam("variable3", "/status/pcs:IsFaulted");
            }
            reload = 2;
            cAv->setVal(reload);
        }

        amap = FunctionUtility::SharedAmapReset(amap, vm, processOriginUri, "Start", "VerifyStart");
        returnObject = FunctionUtility::SharedHandleCmdProcess(vmap, amap, pcsch, p_fims, aV, "Start");

        return returnObject;
    }

    /**
    * @brief Internal function called by an InputHandler Function to StopPCS
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    */
    FunctionUtility::FunctionReturnObj StopPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, const char* processOriginUri)
    {

        if(1)FPS_PRINT_INFO("{} | {}", __func__, processOriginUri);

        FunctionUtility::FunctionReturnObj returnObject; 

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
                // /status/pcs/StopSuccess
                DataUtility::AssetVarInfo("/status/pcs", "StopSuccess", assetVar::ATypes::ABOOL),
                // /status/bms/VerifyStopSuccess
                DataUtility::AssetVarInfo("/status/bms", "VerifyStopSuccess", assetVar::ATypes::ABOOL),
                // /controls/pcs/Stop
                DataUtility::AssetVarInfo("/controls/pcs", "Stop", assetVar::ATypes::AFLOAT),
                // /controls/pcs:VerifyStop
                DataUtility::AssetVarInfo("/controls/pcs", "VerifyStop", assetVar::ATypes::AFLOAT),
                // /alarms/pcs:Stop
                DataUtility::AssetVarInfo("/alarms/pcs", "Stop", "Stop_ALARM", assetVar::ATypes::ASTRING),
                // /alarms/pcs:VerifyStop
                DataUtility::AssetVarInfo("/alarms/pcs", "VerifyStop", "VerifyStop_ALARM", assetVar::ATypes::ASTRING)
            };

            amap = DataUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            if (reload == 0)
            {

                // amap["Stop"]->setcParam("expression", "{1} and ({2} == Standby or {2} == Run) and {3}");
                // amap["Stop"]->setParam("numVars", 3);
                // amap["Stop"]->setParam("useExpr", true);
                // amap["Stop"]->setcParam("variable1", "/status/bms:CurrentBeforeStopIsOK");
                // amap["Stop"]->setcParam("variable2", "/status/pcs:SystemState");
                // amap["Stop"]->setcParam("variable3", "/status/bms:DCClosed");
            }
            reload = 2;
            cAv->setVal(reload);
        }


        amap = FunctionUtility::SharedAmapReset(amap, vm, processOriginUri, "Stop", "VerifyStop");
        returnObject = FunctionUtility::SharedHandleCmdProcess(vmap, amap, pcsch, p_fims, aV, "Stop");

        return returnObject;
    }

    /**
    * @brief Internal function called by an InputHandler Function to StandbyPCS
    * 
    * @param vmap the global data map shared by all assets/asset managers
    * @param amap the local data map used by an asset/asset manager
    * @param aname the name of the asset/asset manager
    * @param p_fims the interface used for data interchange
    * @param av the assetVar that contains the command value to send
    */
    FunctionUtility::FunctionReturnObj StandbyPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, const char* processOriginUri)
    {

        if(1)FPS_PRINT_INFO("{} | {}", __func__, processOriginUri);

        FunctionUtility::FunctionReturnObj returnObject; 


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
                // /status/pcs/StandbySuccess
                DataUtility::AssetVarInfo("/status/pcs", "StandbySuccess", assetVar::ATypes::ABOOL),
                // /status/bms/VerifyStandbySuccess
                DataUtility::AssetVarInfo("/status/bms", "VerifyStandbySuccess", assetVar::ATypes::ABOOL),
                // /controls/pcs/Standby
                DataUtility::AssetVarInfo("/controls/pcs", "Standby", assetVar::ATypes::AFLOAT),
                // /controls/pcs:VerifyStandby
                DataUtility::AssetVarInfo("/controls/pcs", "VerifyStandby", assetVar::ATypes::AFLOAT),
                // /alarms/pcs:Standby
                DataUtility::AssetVarInfo("/alarms/pcs", "Standby", "Standby_ALARM", assetVar::ATypes::ASTRING),
                // /alarms/pcs:VerifyStandby
                DataUtility::AssetVarInfo("/alarms/pcs", "VerifyStandby", "VerifyStandby_ALARM", assetVar::ATypes::ASTRING)
            };

            amap = DataUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            
            if (reload == 0)
            {
                // amap["Standby"]->setcParam("expression", "{1} and ({2} == Stop or {2} == Run) and {3}");
                // amap["Standby"]->setParam("numVars", 3);
                // amap["Standby"]->setParam("useExpr", true);
                // amap["Standby"]->setcParam("variable1", "/status/bms:CurrentBeforeStopIsOK");
                // amap["Standby"]->setcParam("variable2", "/status/pcs:SystemState");
                // amap["Standby"]->setcParam("variable3", "/status/bms:DCClosed");
            }
            reload = 2;
            cAv->setVal(reload);
        }


        amap = FunctionUtility::SharedAmapReset(amap, vm, processOriginUri, "Standby", "VerifyStandby");
        returnObject = FunctionUtility::SharedHandleCmdProcess(vmap, amap, pcsch, p_fims, aV, "Standby");

        return returnObject;
    }

}
#endif