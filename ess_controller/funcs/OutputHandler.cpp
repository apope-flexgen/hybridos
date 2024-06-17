#ifndef OUTPUTHANDLER_CPP
#define OUTPUTHANDLER_CPP

#include "asset.h"
#include "formatters.hpp"
#include "FunctionUtility.hpp"
#include "OutputHandler.hpp"
#include "DataUtility.hpp"

char* strtime(const struct tm* timeptr);

/**
 *
 */
extern "C++" {
int HandleCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}

namespace OutputHandler
{
/**
 * @brief Internal function called by an InputHandler Function to OpenContactors
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param p_fims the interface used for data interchange
 * @param av the assetVar that contains the command value to send
 */
FunctionUtility::FunctionReturnObj OpenContactors(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims,
                                                  assetVar* aV, const char* processOriginUri)
{
    if (0)
        FPS_PRINT_INFO("{} | {}", __func__, processOriginUri);

    FunctionUtility::FunctionReturnObj returnObject;

    // bool debug = false;
    int reload = 0;

    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;

    essPerf ePerf(am, aname, __func__);

    const char* bmsch = (const char*)"bms";
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

    FunctionUtility::HandleCmdProcessUris uris;
    uris.controlsUri = (fmt::format("OpenContactors_{}", bmsch));
    uris.verifyControlsUri = (fmt::format("VerifyOpenContactors_{}", bmsch));
    uris.controlsSuccessUri = (fmt::format("OpenContactorsSuccess_{}", bmsch));
    uris.verifyControlsSuccessUri = (fmt::format("VerifyOpenContactorsSuccess_{}", bmsch));
    uris.controlsAlarmUri = (fmt::format("OpenContactors_ALARM_{}", bmsch));
    uris.verifyControlsAlarmUri = (fmt::format("VerifyOpenContactors_ALARM_{}", bmsch));

    if (reload < 2)
    {
        linkVals(*vm, vmap, amap, bmsch, "/reload", reload, relname);
        cAv = amap[relname];

        std::string status = (fmt::format("/status/{}", bmsch));
        std::string controls = (fmt::format("/controls/{}", bmsch));
        std::string alarms = (fmt::format("/alarms/{}", bmsch));

        std::vector<DataUtility::AssetVarInfo> assetVarVector = {
            // /status/bms/OpenContactorsSuccess
            DataUtility::AssetVarInfo(status.c_str(), "OpenContactorsSuccess", uris.controlsSuccessUri.c_str(),
                                      assetVar::ATypes::ABOOL),
            // /status/bms/VerifyOpenContactorsSuccess
            DataUtility::AssetVarInfo(status.c_str(), "VerifyOpenContactorsSuccess",
                                      uris.verifyControlsSuccessUri.c_str(), assetVar::ATypes::ABOOL),
            // /controls/bms/OpenContactors
            DataUtility::AssetVarInfo(controls.c_str(), "OpenContactors", uris.controlsUri.c_str(),
                                      assetVar::ATypes::AFLOAT),
            // /controls/bms:VerifyOpenContactors
            DataUtility::AssetVarInfo(controls.c_str(), "VerifyOpenContactors", uris.verifyControlsUri.c_str(),
                                      assetVar::ATypes::AFLOAT),
            // /alarms/bms:OpenContactors
            DataUtility::AssetVarInfo(alarms.c_str(), "OpenContactors", uris.controlsAlarmUri.c_str(),
                                      assetVar::ATypes::ASTRING),
            // /alarms/bms:VerifyOpenContactors
            DataUtility::AssetVarInfo(alarms.c_str(), "VerifyOpenContactors", uris.verifyControlsAlarmUri.c_str(),
                                      assetVar::ATypes::ASTRING)
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

    amap = FunctionUtility::SharedAmapReset(amap, vm, processOriginUri, uris.controlsUri.c_str(),
                                            uris.verifyControlsUri.c_str());
    returnObject = FunctionUtility::SharedHandleCmdProcess(vmap, amap, bmsch, p_fims, aV, uris);

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
FunctionUtility::FunctionReturnObj CloseContactors(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims,
                                                   assetVar* aV, const char* processOriginUri)
{
    if (0)
        FPS_PRINT_INFO("{} | {}", __func__, processOriginUri);

    FunctionUtility::FunctionReturnObj returnObject;

    int reload = 0;
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    essPerf ePerf(am, aname, __func__);

    const char* bmsch = (const char*)"bms";
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

    FunctionUtility::HandleCmdProcessUris uris;
    uris.controlsUri = (fmt::format("CloseContactors_{}", bmsch));
    uris.verifyControlsUri = (fmt::format("VerifyCloseContactors_{}", bmsch));
    uris.controlsSuccessUri = (fmt::format("CloseContactorsSuccess_{}", bmsch));
    uris.verifyControlsSuccessUri = (fmt::format("VerifyCloseContactorsSuccess_{}", bmsch));
    uris.controlsAlarmUri = (fmt::format("CloseContactors_ALARM_{}", bmsch));
    uris.verifyControlsAlarmUri = (fmt::format("VerifyCloseContactors_ALARM_{}", bmsch));

    if (reload < 2)
    {
        linkVals(*vm, vmap, amap, bmsch, "/reload", reload, relname);
        cAv = amap[relname];

        std::string status = (fmt::format("/status/{}", bmsch));
        std::string controls = (fmt::format("/controls/{}", bmsch));
        std::string alarms = (fmt::format("/alarms/{}", bmsch));

        std::vector<DataUtility::AssetVarInfo> assetVarVector = {
            // /status/bms/CloseContactorsSuccess
            DataUtility::AssetVarInfo(status.c_str(), "CloseContactorsSuccess", uris.controlsSuccessUri.c_str(),
                                      assetVar::ATypes::ABOOL),
            // /status/bms/VerifyCloseContactorsSuccess
            DataUtility::AssetVarInfo(status.c_str(), "VerifyCloseContactorsSuccess",
                                      uris.verifyControlsSuccessUri.c_str(), assetVar::ATypes::ABOOL),
            // /controls/bms/CloseContactors
            DataUtility::AssetVarInfo(controls.c_str(), "CloseContactors", uris.controlsUri.c_str(),
                                      assetVar::ATypes::AFLOAT),
            // /controls/bms:VerifyCloseContactors
            DataUtility::AssetVarInfo(controls.c_str(), "VerifyCloseContactors", uris.verifyControlsUri.c_str(),
                                      assetVar::ATypes::AFLOAT),
            // /alarms/bms:CloseContactors
            DataUtility::AssetVarInfo(alarms.c_str(), "CloseContactors", uris.controlsAlarmUri.c_str(),
                                      assetVar::ATypes::ASTRING),
            // /alarms/bms:VerifyCloseContactors
            DataUtility::AssetVarInfo(alarms.c_str(), "VerifyCloseContactors", uris.verifyControlsAlarmUri.c_str(),
                                      assetVar::ATypes::ASTRING)
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

    amap = FunctionUtility::SharedAmapReset(amap, vm, processOriginUri, uris.controlsUri.c_str(),
                                            uris.verifyControlsUri.c_str());
    returnObject = FunctionUtility::SharedHandleCmdProcess(vmap, amap, bmsch, p_fims, aV, uris);

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
FunctionUtility::FunctionReturnObj StartPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV,
                                            const char* processOriginUri)
{
    if (1)
        FPS_PRINT_INFO("{} | {}", __func__, processOriginUri);

    FunctionUtility::FunctionReturnObj returnObject;

    // bool debug = false;
    int reload = 0;
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    essPerf ePerf(am, aname, __func__);

    const char* pcsch = (const char*)"pcs";
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

    FunctionUtility::HandleCmdProcessUris uris;
    uris.controlsUri = (fmt::format("Start_{}", pcsch));
    uris.verifyControlsUri = (fmt::format("VerifyStart_{}", pcsch));
    uris.controlsSuccessUri = (fmt::format("StartSuccess_{}", pcsch));
    uris.verifyControlsSuccessUri = (fmt::format("VerifyStartSuccess_{}", pcsch));
    uris.controlsAlarmUri = (fmt::format("Start_ALARM_{}", pcsch));
    uris.verifyControlsAlarmUri = (fmt::format("VerifyStart_ALARM_{}", pcsch));

    if (reload < 2)
    {
        linkVals(*vm, vmap, amap, pcsch, "/reload", reload, relname);
        cAv = amap[relname];

        std::vector<DataUtility::AssetVarInfo> assetVarVector = {
            // /status/pcs/StartSuccess
            DataUtility::AssetVarInfo("/status/pcs", "StartSuccess", uris.controlsSuccessUri.c_str(),
                                      assetVar::ATypes::ABOOL),
            // /status/bms/VerifyStartSuccess
            DataUtility::AssetVarInfo("/status/bms", "VerifyStartSuccess", uris.verifyControlsSuccessUri.c_str(),
                                      assetVar::ATypes::ABOOL),
            // /controls/pcs/Start
            DataUtility::AssetVarInfo("/controls/pcs", "Start", uris.controlsUri.c_str(), assetVar::ATypes::AFLOAT),
            // /controls/pcs:VerifyStart
            DataUtility::AssetVarInfo("/controls/pcs", "VerifyStart", uris.verifyControlsUri.c_str(),
                                      assetVar::ATypes::AFLOAT),
            // /alarms/pcs:Start
            DataUtility::AssetVarInfo("/alarms/pcs", "Start", uris.controlsAlarmUri.c_str(), assetVar::ATypes::ASTRING),
            // /alarms/pcs:VerifyStart
            DataUtility::AssetVarInfo("/alarms/pcs", "VerifyStart", uris.verifyControlsAlarmUri.c_str(),
                                      assetVar::ATypes::ASTRING)
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

    amap = FunctionUtility::SharedAmapReset(amap, vm, processOriginUri, uris.controlsUri.c_str(),
                                            uris.verifyControlsUri.c_str());
    returnObject = FunctionUtility::SharedHandleCmdProcess(vmap, amap, pcsch, p_fims, aV, uris);

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
FunctionUtility::FunctionReturnObj StopPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV,
                                           const char* processOriginUri)
{
    if (1)
        FPS_PRINT_INFO("{} | {}", __func__, processOriginUri);

    FunctionUtility::FunctionReturnObj returnObject;

    // bool debug = false;
    int reload = 0;
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    essPerf ePerf(am, aname, __func__);

    const char* pcsch = (const char*)"pcs";
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

    FunctionUtility::HandleCmdProcessUris uris;
    uris.controlsUri = (fmt::format("Stop_{}", pcsch));
    uris.verifyControlsUri = (fmt::format("VerifyStop_{}", pcsch));
    uris.controlsSuccessUri = (fmt::format("StopSuccess_{}", pcsch));
    uris.verifyControlsSuccessUri = (fmt::format("VerifyStopSuccess_{}", pcsch));
    uris.controlsAlarmUri = (fmt::format("Stop_ALARM_{}", pcsch));
    uris.verifyControlsAlarmUri = (fmt::format("VerifyStop_ALARM_{}", pcsch));

    if (reload < 2)
    {
        linkVals(*vm, vmap, amap, pcsch, "/reload", reload, relname);
        cAv = amap[relname];

        std::vector<DataUtility::AssetVarInfo> assetVarVector = {
            // /status/pcs/StopSuccess
            DataUtility::AssetVarInfo("/status/pcs", "StopSuccess", uris.controlsSuccessUri.c_str(),
                                      assetVar::ATypes::ABOOL),
            // /status/bms/VerifyStopSuccess
            DataUtility::AssetVarInfo("/status/bms", "VerifyStopSuccess", uris.verifyControlsSuccessUri.c_str(),
                                      assetVar::ATypes::ABOOL),
            // /controls/pcs/Stop
            DataUtility::AssetVarInfo("/controls/pcs", "Stop", uris.controlsUri.c_str(), assetVar::ATypes::AFLOAT),
            // /controls/pcs:VerifyStop
            DataUtility::AssetVarInfo("/controls/pcs", "VerifyStop", uris.verifyControlsUri.c_str(),
                                      assetVar::ATypes::AFLOAT),
            // /alarms/pcs:Stop
            DataUtility::AssetVarInfo("/alarms/pcs", "Stop", uris.controlsAlarmUri.c_str(), assetVar::ATypes::ASTRING),
            // /alarms/pcs:VerifyStop
            DataUtility::AssetVarInfo("/alarms/pcs", "VerifyStop", uris.verifyControlsAlarmUri.c_str(),
                                      assetVar::ATypes::ASTRING)
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

    amap = FunctionUtility::SharedAmapReset(amap, vm, processOriginUri, uris.controlsUri.c_str(),
                                            uris.verifyControlsUri.c_str());
    returnObject = FunctionUtility::SharedHandleCmdProcess(vmap, amap, pcsch, p_fims, aV, uris);

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
FunctionUtility::FunctionReturnObj StandbyPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims,
                                              assetVar* aV, const char* processOriginUri)
{
    if (1)
        FPS_PRINT_INFO("{} | {}", __func__, processOriginUri);

    FunctionUtility::FunctionReturnObj returnObject;

    int reload = 0;
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    essPerf ePerf(am, aname, __func__);

    const char* pcsch = (const char*)"pcs";
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

    FunctionUtility::HandleCmdProcessUris uris;
    uris.controlsUri = (fmt::format("Standby_{}", pcsch));
    uris.verifyControlsUri = (fmt::format("VerifyStandby_{}", pcsch));
    uris.controlsSuccessUri = (fmt::format("StandbySuccess_{}", pcsch));
    uris.verifyControlsSuccessUri = (fmt::format("VerifyStandbySuccess_{}", pcsch));
    uris.controlsAlarmUri = (fmt::format("Standby_ALARM_{}", pcsch));
    uris.verifyControlsAlarmUri = (fmt::format("VerifyStandby_ALARM_{}", pcsch));

    if (reload < 2)
    {
        linkVals(*vm, vmap, amap, pcsch, "/reload", reload, relname);
        cAv = amap[relname];

        std::vector<DataUtility::AssetVarInfo> assetVarVector = {
            // /status/pcs/StandbySuccess
            DataUtility::AssetVarInfo("/status/pcs", "StandbySuccess", uris.controlsSuccessUri.c_str(),
                                      assetVar::ATypes::ABOOL),
            // /status/bms/VerifyStandbySuccess
            DataUtility::AssetVarInfo("/status/bms", "VerifyStandbySuccess", uris.verifyControlsSuccessUri.c_str(),
                                      assetVar::ATypes::ABOOL),
            // /controls/pcs/Standby
            DataUtility::AssetVarInfo("/controls/pcs", "Standby", uris.controlsUri.c_str(), assetVar::ATypes::AFLOAT),
            // /controls/pcs:VerifyStandby
            DataUtility::AssetVarInfo("/controls/pcs", "VerifyStandby", uris.verifyControlsUri.c_str(),
                                      assetVar::ATypes::AFLOAT),
            // /alarms/pcs:Standby
            DataUtility::AssetVarInfo("/alarms/pcs", "Standby", uris.controlsAlarmUri.c_str(),
                                      assetVar::ATypes::ASTRING),
            // /alarms/pcs:VerifyStandby
            DataUtility::AssetVarInfo("/alarms/pcs", "VerifyStandby", uris.verifyControlsAlarmUri.c_str(),
                                      assetVar::ATypes::ASTRING)
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

    amap = FunctionUtility::SharedAmapReset(amap, vm, processOriginUri, uris.controlsUri.c_str(),
                                            uris.verifyControlsUri.c_str());
    returnObject = FunctionUtility::SharedHandleCmdProcess(vmap, amap, pcsch, p_fims, aV, uris);

    return returnObject;
}
}
#endif