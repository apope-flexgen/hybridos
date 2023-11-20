#ifndef OUTPUTHANDLER_CPP
#define OUTPUTHANDLER_CPP

#include "asset.h"
#include "formatters.hpp"
#include "FunctionUtility.hpp"
#include "OutputHandler.hpp"



char* strtime(const struct tm *timeptr);

/**
 * 
 */
 extern "C++" {
    int HandleCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}

namespace OutputHandler {


    FunctionUtility::FunctionReturnObj OpenContactors(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
    {


        std::string funcName = __func__;
        std::string assetUriName = "open_contactors";
        std::string assetName = aname;

        if(0)FPS_PRINT_INFO("{}", __func__);

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

            std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
                // /status/bms/DCClosed
                FunctionUtility::AssetVarInfo("/status/bms", "DCClosed", assetVar::ATypes::ABOOL),
                // /status/bms/OpenContactorsSuccess
                FunctionUtility::AssetVarInfo("/status/bms", "OpenContactorsSuccess", assetVar::ATypes::ABOOL),
                // /controls/bms/OpenContactors
                FunctionUtility::AssetVarInfo("/controls/bms", "OpenContactors", assetVar::ATypes::AFLOAT),
                // /controls/bms:VerifyOpenContactors
                FunctionUtility::AssetVarInfo("/controls/bms", "VerifyOpenContactors", assetVar::ATypes::AFLOAT),
                // /status/pcs/SystemState
                FunctionUtility::AssetVarInfo("/status/pcs", "SystemState", assetVar::ATypes::ASTRING),
                // /alarms/bms:OpenContactors
                FunctionUtility::AssetVarInfo("/alarms/bms", "OpenContactors", "OpenContactors_ALARM", assetVar::ATypes::ASTRING),
                // /alarms/bms:VerifyOpenContactors
                FunctionUtility::AssetVarInfo("/alarms/bms", "VerifyOpenContactors", "VerifyOpenContactors_ALARM", assetVar::ATypes::ASTRING)
            };

            amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);



            if (reload == 0)
            {

                bool dcClosed = amap["DCClosed"]->getbVal();
                char* systemStateStatus = amap["SystemState"]->getcVal();

                bool systemState = false;

                if(!(systemStateStatus == nullptr)){
                    std::string compareString = systemStateStatus;
                    systemState = (compareString == "Stop" || "Fault");
                }

                if(!systemState){
                    returnObject.statusIndicator = FAILURE;
                    returnObject.message = "To open contactors the SystemState of the PCS needs to be 'Stop' or 'Fault' but it was not";
                    return returnObject;
                }
                if(!dcClosed){
                    returnObject.statusIndicator = FAILURE;
                    returnObject.message = "To open contactors DC Contactors need to be closed but they were open";
                    return returnObject;                
                }
            }

            reload = 2;
            cAv->setVal(reload);
        }


        amap = FunctionUtility::SharedAmapReset(amap, vm, "open_contactors", "OpenContactors", "VerifyOpenContactors");
        returnObject = FunctionUtility::SharedHandleCmdProcess(vmap, amap, aname, p_fims, aV, "OpenContactors");

        return returnObject;

    }

    FunctionUtility::FunctionReturnObj CloseContactors(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
    {

        std::string funcName = __func__;
        std::string assetUriName = "close_contactors";
        std::string assetName = aname;

        if(0)FPS_PRINT_INFO("{}", __func__);


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

        assetName = bmsch;


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

            std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
                // /status/bms/DCClosed
                FunctionUtility::AssetVarInfo("/status/bms", "DCClosed", assetVar::ATypes::ABOOL),
                // /status/bms/CloseContactorsSuccess
                FunctionUtility::AssetVarInfo("/status/bms", "CloseContactorsSuccess", assetVar::ATypes::ABOOL),
                // /controls/bms/CloseContactors
                FunctionUtility::AssetVarInfo("/controls/bms", "CloseContactors", assetVar::ATypes::AFLOAT),
                // /controls/bms:VerifyCloseContactors
                FunctionUtility::AssetVarInfo("/controls/bms", "VerifyCloseContactors", assetVar::ATypes::AFLOAT),
                // /status/pcs/SystemState
                FunctionUtility::AssetVarInfo("/status/pcs", "SystemState", assetVar::ATypes::ASTRING),
                // /alarms/bms:CloseContactors
                FunctionUtility::AssetVarInfo("/alarms/bms", "CloseContactors", "CloseContactors_ALARM", assetVar::ATypes::ASTRING),
                // /alarms/bms:VerifyCloseContactors
                FunctionUtility::AssetVarInfo("/alarms/bms", "VerifyCloseContactors", "VerifyCloseContactors_ALARM", assetVar::ATypes::ASTRING)
            };

            amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            if (reload == 0)
            {

                bool dcClosed = amap["DCClosed"]->getbVal();
                char* systemStateStatus = amap["SystemState"]->getcVal();

                bool systemState = false;

                if(!(systemStateStatus == nullptr)){
                    std::string compareString = systemStateStatus;
                    systemState = (compareString == "Stop");
                }

                if(!systemState){
                    returnObject.statusIndicator = FAILURE;
                    returnObject.message = "To close contactors the SystemState of the PCS needs to be 'Stop' but it was not";
                    return returnObject;
                }
                if(dcClosed){
                    returnObject.statusIndicator = FAILURE;
                    returnObject.message = "To close contactors DC Contactors need to be open but they were closed";
                    return returnObject;                
                }
            }
            cAv->setVal(2);
        }


        amap = FunctionUtility::SharedAmapReset(amap, vm, "close_contactors", "CloseContactors", "VerifyCloseContactors");
        returnObject = FunctionUtility::SharedHandleCmdProcess(vmap, amap, aname, p_fims, aV, "CloseContactors");

        return returnObject;
    }

    FunctionUtility::FunctionReturnObj StartPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
    {

        if(1)FPS_PRINT_INFO("StartPCS");

        FunctionUtility::FunctionReturnObj returnObject; 

        // bool debug = false;
        int reload = 0;
        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;
        essPerf ePerf(am, aname, __func__);

        // const char *bmsch = (const char*)"bms";
        // const char *pcsch = (const char*)"pcs";
        const char *essch = (const char*)"ess";
        // if (aV->gotParam("bms"))
        // {
        //     bmsch = aV->getcParam("bms");
        // }
        // if (aV->gotParam("pcs"))
        // {
        //     pcsch = aV->getcParam("pcs");
        // }
        if (aV->gotParam("ess"))
        {
            essch = aV->getcParam("ess");
        }
        // if (aV->gotParam("debug"))
        // {
        //     debug = aV->getbParam("debug");
        // }


        auto relname = fmt::format("{}_{}", __func__, essch).c_str();
        assetVar* cAv = amap[relname];
        if (!cAv || (reload = cAv->getiVal()) == 0)
        {
            reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
        }

        if (reload < 2)
        {

            linkVals(*vm, vmap, amap, essch, "/reload", reload, relname);
            cAv = amap[relname];

            std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
                // /status/bms/DCClosed
                FunctionUtility::AssetVarInfo("/status/bms", "DCClosed", assetVar::ATypes::ABOOL),
                // /status/pcs/SystemState
                FunctionUtility::AssetVarInfo("/status/pcs", "SystemState", assetVar::ATypes::ASTRING),
                // /status/pcs/StartSuccess
                FunctionUtility::AssetVarInfo("/status/pcs", "StartSuccess", assetVar::ATypes::ABOOL),
                // /controls/pcs/Start
                FunctionUtility::AssetVarInfo("/controls/pcs", "Start", assetVar::ATypes::AFLOAT),
                // /controls/pcs:VerifyStart
                FunctionUtility::AssetVarInfo("/controls/pcs", "VerifyStart", assetVar::ATypes::AFLOAT),
                // /alarms/pcs:Start
                FunctionUtility::AssetVarInfo("/alarms/pcs", "Start", "Start_ALARM", assetVar::ATypes::ASTRING),
                // /alarms/pcs:VerifyStart
                FunctionUtility::AssetVarInfo("/alarms/pcs", "VerifyStart", "VerifyStart_ALARM", assetVar::ATypes::ASTRING)
            };

            amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            if (reload == 0)
            {

                bool dcClosed = amap["DCClosed"]->getbVal();
                bool isFaulted = amap["IsFaulted"]->getbVal();
                char* systemStateStatus = amap["SystemState"]->getcVal();

                bool systemState = false;

                if(!(systemStateStatus == nullptr)){
                    std::string compareString = systemStateStatus;
                    systemState = (compareString == "Stop" || compareString == "Standby");
                }

                // "expression": "({1} == Stop or {1} == Standby) and {2} and not {3}",
                // "variable1": "/status/pcs:SystemState",
                // "variable2": "/status/bms:DCClosed",
                // "variable3": "/status/pcs:IsFaulted"

                if(!systemState || !dcClosed || isFaulted) {
                    returnObject.statusIndicator = FAILURE;
                    returnObject.message = "";
                    return returnObject;
                }
            }
            reload = 2;
            cAv->setVal(reload);
        }

        amap = FunctionUtility::SharedAmapReset(amap, vm, "start", "Start", "");
        returnObject = FunctionUtility::SharedHandleCmdProcess(vmap, amap, aname, p_fims, aV, "Start");

        return returnObject;
    }

    FunctionUtility::FunctionReturnObj StopPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
    {

        if(1)FPS_PRINT_INFO("StopPCS");

        FunctionUtility::FunctionReturnObj returnObject; 

        // bool debug = false;
        int reload = 0;
        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;
        essPerf ePerf(am, aname, __func__);

        // const char *bmsch = (const char*)"bms";
        const char *pcsch = (const char*)"pcs";
        // const char *essch = (const char*)"ess";
        // if (aV->gotParam("bms"))
        // {
        //     bmsch = aV->getcParam("bms");
        // }
        if (aV->gotParam("pcs"))
        {
            pcsch = aV->getcParam("pcs");
        }
        // if (aV->gotParam("ess"))
        // {
        //     essch = aV->getcParam("ess");
        // }
        // if (aV->gotParam("debug"))
        // {
        //     debug = aV->getbParam("debug");
        // }


        auto relname = fmt::format("{}_{}", __func__, pcsch).c_str() ;
        assetVar* cAv = amap[relname];
        if (!cAv || (reload = cAv->getiVal()) == 0)
        {
            reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
        }

        if (reload < 2)
        {

            linkVals(*vm, vmap, amap, pcsch, "/reload", reload, relname);
            cAv = amap[relname];

            std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
                // /status/bms/DCClosed
                FunctionUtility::AssetVarInfo("/status/bms", "DCClosed", assetVar::ATypes::ABOOL),
                // /status/bms/CurrentBeforeStopIsOK
                FunctionUtility::AssetVarInfo("/status/bms", "CurrentBeforeStopIsOK", assetVar::ATypes::ABOOL),
                // /status/pcs/SystemState
                FunctionUtility::AssetVarInfo("/status/pcs", "SystemState", assetVar::ATypes::ASTRING),
                // /status/pcs/StopSuccess
                FunctionUtility::AssetVarInfo("/status/pcs", "StopSuccess", assetVar::ATypes::ABOOL),
                // /controls/pcs/Stop
                FunctionUtility::AssetVarInfo("/controls/pcs", "Stop", assetVar::ATypes::AFLOAT),
                // /controls/pcs:VerifyStop
                FunctionUtility::AssetVarInfo("/controls/pcs", "VerifyStop", assetVar::ATypes::AFLOAT),
                // /alarms/pcs:Stop
                FunctionUtility::AssetVarInfo("/alarms/pcs", "Stop", "Stop_ALARM", assetVar::ATypes::ASTRING),
                // /alarms/pcs:VerifyStop
                FunctionUtility::AssetVarInfo("/alarms/pcs", "VerifyStop", "VerifyStop_ALARM", assetVar::ATypes::ASTRING)
            };

            amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            if (reload == 0)
            {

                bool dcClosed = amap["DCClosed"]->getbVal();
                bool currentIsOk = amap["CurrentBeforeStopIsOK"]->getbVal();
                char* systemStateStatus = amap["SystemState"]->getcVal();

                bool systemState = false;

                if(!(systemStateStatus == nullptr)){
                    std::string compareString = systemStateStatus;
                    systemState = (compareString == "Standby" || "Run");
                }

                

                if(!currentIsOk || !systemState || !dcClosed) {
                    returnObject.statusIndicator = FAILURE;
                    returnObject.message = "";
                    return returnObject;
                }

                // "expression": "{1} and ({2} == Standby or {2} == Run) and {3}",
                // "variable1": "/status/bms:CurrentBeforeStopIsOK",
                // "variable2": "/status/pcs:SystemState",
                // "variable3": "/status/bms:DCClosed"
            }
            reload = 2;
            cAv->setVal(reload);
        }

        amap = FunctionUtility::SharedAmapReset(amap, vm, "stop", "Stop", "");
        returnObject = FunctionUtility::SharedHandleCmdProcess(vmap, amap, aname, p_fims, aV, "Stop");

        return returnObject;
    }

    FunctionUtility::FunctionReturnObj StandbyPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
    {

        if(1)FPS_PRINT_INFO("StandbyPCS");

        FunctionUtility::FunctionReturnObj returnObject; 


        int reload = 0;
        asset_manager * am = aV->am;
        VarMapUtils* vm = am->vm;
        essPerf ePerf(am, aname, __func__);

        // const char *bmsch = (const char*)"bms";
        const char *pcsch = (const char*)"pcs";
        // const char *essch = (const char*)"ess";
        // if (aV->gotParam("bms"))
        // {
        //     bmsch = aV->getcParam("bms");
        // }
        if (aV->gotParam("pcs"))
        {
            pcsch = aV->getcParam("pcs");
        }
        // if (aV->gotParam("ess"))
        // {
        //     essch = aV->getcParam("ess");
        // }
        // if (aV->gotParam("debug"))
        // {
        //     debug = aV->getbParam("debug");
        // }


        auto relname = fmt::format("{}_{}", __func__, pcsch).c_str() ;
        assetVar* cAv = amap[relname];
        if (!cAv || (reload = cAv->getiVal()) == 0)
        {
            reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
        }

        if (reload < 2)
        {

            linkVals(*vm, vmap, amap, pcsch, "/reload", reload, relname);
            cAv = amap[relname];

            std::vector<FunctionUtility::AssetVarInfo> assetVarVector = {
                // /status/bms/DCClosed
                FunctionUtility::AssetVarInfo("/status/bms", "DCClosed", assetVar::ATypes::ABOOL),
                // /status/bms/CurrentBeforeStopIsOK
                FunctionUtility::AssetVarInfo("/status/bms", "CurrentBeforeStopIsOK", assetVar::ATypes::ABOOL),
                // /status/pcs/SystemState
                FunctionUtility::AssetVarInfo("/status/pcs", "SystemState", assetVar::ATypes::ASTRING),
                // /status/pcs/StandbySuccess
                FunctionUtility::AssetVarInfo("/status/pcs", "StandbySuccess", assetVar::ATypes::ABOOL),
                // /controls/pcs/Standby
                FunctionUtility::AssetVarInfo("/controls/pcs", "Standby", assetVar::ATypes::AFLOAT),
                // /controls/pcs:VerifyStandby
                FunctionUtility::AssetVarInfo("/controls/pcs", "VerifyStandby", assetVar::ATypes::AFLOAT),
                // /alarms/pcs:Standby
                FunctionUtility::AssetVarInfo("/alarms/pcs", "Standby", "Standby_ALARM", assetVar::ATypes::ASTRING),
                // /alarms/pcs:VerifyStandby
                FunctionUtility::AssetVarInfo("/alarms/pcs", "VerifyStandby", "VerifyStandby_ALARM", assetVar::ATypes::ASTRING)
            };

            amap = FunctionUtility::PopulateAmapWithManyAvs(vmap, amap, vm, assetVarVector);

            
            if (reload == 0)
            {

                bool dcClosed = amap["DCClosed"]->getbVal();
                bool currentIsOk = amap["CurrentBeforeStopIsOK"]->getbVal();
                char* systemStateStatus = amap["SystemState"]->getcVal();

                bool systemState = false;

                if(!(systemStateStatus == nullptr)){
                    std::string compareString = systemStateStatus;
                    systemState = (compareString == "Stop" || "Run");
                }

                if(!currentIsOk || !systemState || !dcClosed) {
                    returnObject.statusIndicator = FAILURE;
                    returnObject.message = "";
                    return returnObject;
                }

                // "expression": "{1} and ({2} == Stop or {2} == Run) and {3}",
                // "variable1": "/status/bms:CurrentBeforeStopIsOK",
                // "variable2": "/status/pcs:SystemState",
                // "variable3": "/status/bms:DCClosed"
            }
            reload = 2;
            cAv->setVal(reload);
        }

        amap = FunctionUtility::SharedAmapReset(amap, vm, "standby", "Standby", "");
        returnObject = FunctionUtility::SharedHandleCmdProcess(vmap, amap, aname, p_fims, aV, "Standby");

        return returnObject;
    }

}
#endif