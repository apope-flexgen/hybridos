#ifndef CHECKDBIVAR_CPP
#define CHECKDBIVAR_CPP

#include "asset.h"

extern "C++" {
int CheckDbiVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int SendDbiVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int CheckDbiResp(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* dbiAv);
int SetDbiDoc(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}

/**
 * @brief Check if the parameters in av are also in dbiAv and if the param
 * values are different. If so, update av parameters by copying the parameter
 * values in dbiAv to av
 *
 * @param av the assetVar referencing the dbi
 * @param dbiAv the dbi assetVar
 */
void updateParamsFromDbi(assetVar* av, assetVar* dbiAv)
{
    // For error-checking, make sure both dbiAv and av have assetExtras, a
    // baseDict, and values in the featMap (where parameters are stored)
    if (!dbiAv->extras || !av->extras)
    {
        FPS_PRINT_ERROR(
            "assetExtras is null for either dbiAv or av %s. Skipping "
            "param set to dbi and assetVar\n",
            av->name);
        return;
    }

    assetFeatDict* dbiDict = dbiAv->extras->baseDict;
    assetFeatDict* avDict = av->extras->baseDict;
    if (!dbiDict || !avDict)
    {
        FPS_PRINT_ERROR(
            "baseDict is null for either dbiAv or av %s. Skipping "
            "param set to dbi and assetVar\n",
            av->name);
        return;
    }

    std::map<std::string, assFeat*>& dbiAvParams = dbiDict->featMap;
    if (dbiAvParams.empty())
    {
        FPS_PRINT_ERROR(
            "%s >> featMap that contains the params is empty for dbiAv "
            "%s. Skipping param set to dbi and assetVar\n",
            av->name);
        return;
    }

    // Copy the parameter values in dbiAv to av
    for (auto& param : dbiAvParams)
    {
        const std::string& paramName = param.first;

        // In the future, we can possibly have a list of parameters found in the
        // dbiAv that we do not want to include in av For now, we don't want dbiResp
        // to be included
        if (paramName == "dbiSet")
        {
            FPS_DEBUG_PRINT(
                "%s >> param %s is found in dbiAv %s. This will not be "
                "included in av.\n",
                __func__, paramName.c_str(), dbiAv->name.c_str());
            return;
        }
        switch (dbiAvParams[paramName]->type)
        {
            case assFeat::AFTypes::AINT:
            {
                int ival = 0;
                dbiDict->getFeat(paramName.c_str(), &ival);
                FPS_DEBUG_PRINT(
                    "%s >> av param %s is in dbiAv and both param values are "
                    "different. Updating av param to int val %d\n",
                    __func__, paramName.c_str(), ival);
                avDict->setFeat(paramName.c_str(), ival);
                break;
            }
            case assFeat::AFTypes::AFLOAT:
            {
                double dval = 0.0;
                dbiDict->getFeat(paramName.c_str(), &dval);
                FPS_DEBUG_PRINT(
                    "%s >> av param %s is in dbiAv and both param values are "
                    "different. Updating av param to double val %f\n",
                    __func__, paramName.c_str(), dval);
                avDict->setFeat(paramName.c_str(), dval);
                break;
            }
            case assFeat::AFTypes::ASTRING:
            {
                char* cval = (char*)"val";
                dbiDict->getFeat(paramName.c_str(), &cval);
                FPS_DEBUG_PRINT(
                    "%s >> av param %s is in dbiAv and both param values are "
                    "different. Updating av param to string val %s\n",
                    __func__, paramName.c_str(), cval);
                avDict->setFeat(paramName.c_str(), cval);
                break;
            }
            case assFeat::AFTypes::ABOOL:
            {
                bool bval = false;
                dbiDict->getFeat(paramName.c_str(), &bval);
                FPS_DEBUG_PRINT(
                    "%s >> av param %s is in dbiAv and both param values are "
                    "different. Updating av param to bool val %s\n",
                    __func__, paramName.c_str(), bval ? "true" : "false");
                avDict->setFeat(paramName.c_str(), bval);
                break;
            }
            default:
                FPS_ERROR_PRINT(
                    "%s >> assFeat type not supported for dbi variable. "
                    "Supported types are: AINT, AFLOAT, ASTRING, and ABOOL. "
                    "Skipping param set to dbi and assetVar %s\n",
                    __func__, dbiAv->name.c_str());
        }
    }
}

/**
 * @brief Check if the actions in av are also in dbiAv and if the action values
 * are different. If so, update av actions by copying the action values in dbiAv
 * to av
 *
 * @param av the assetVar referencing the dbi
 * @param dbiAv the dbi assetVar
 */
void updateActionsFromDbi(assetVar* av, assetVar* dbiAv)
{
    // For error-checking, make sure both dbiAv and av have assetExtras
    if (!dbiAv->extras || !av->extras)
    {
        FPS_ERROR_PRINT(
            "%s >> assetExtras is null for either dbiAv or av %s. "
            "Skipping action set to dbi and assetVar\n",
            __func__, av->name.c_str());
        return;
    }
    std::map<std::string, std::vector<assetAction*>> dbiAvActions = dbiAv->extras->actVec;
    std::map<std::string, std::vector<assetAction*>>& avActions = av->extras->actVec;

    // Might be enough to just copy the contents of the dbiAv actions vec here
    avActions = dbiAvActions;
}

/**
 * @brief Check if the options in av are also in dbiAv and if the option values
 * are different. If so, update av options by copying the option values in dbiAv
 * to av
 *
 * @param av the assetVar referencing the dbi
 * @param dbiAv the dbi assetVar
 */
void updateOptionsFromDbi(assetVar* av, assetVar* dbiAv)
{
    // For error-checking, make sure both dbiAv and av have assetExtras, an
    // optDict, and values in the featMap (where options are stored)
    if (!dbiAv->extras || !av->extras)
    {
        FPS_ERROR_PRINT(
            "%s >> assetExtras is null for either dbiAv or av %s. "
            "Skipping option set to dbi and assetVar\n",
            __func__, av->name.c_str());
        return;
    }

    assetFeatDict* dbiDict = dbiAv->extras->optDict;
    assetFeatDict* avDict = av->extras->optDict;
    if (!dbiDict || !avDict)
    {
        FPS_ERROR_PRINT(
            "%s >> optDict is null for either dbiAv or av %s. Skipping "
            "option set to dbi and assetVar\n",
            __func__, av->name.c_str());
        return;
    }

    std::map<std::string, assFeat*>& dbiAvOptions = dbiDict->featMap;
    if (dbiAvOptions.empty())
    {
        FPS_ERROR_PRINT(
            "%s >> featMap that contains the options is empty for "
            "dbiAv %s. Skipping option set to dbi and assetVar\n",
            __func__, av->name.c_str());
        return;
    }

    // Copy the option values in dbiAv to av
    for (auto& option : dbiAvOptions)
    {
        const std::string& optionName = option.first;
        switch (dbiAvOptions[optionName]->type)
        {
            case assFeat::AFTypes::AINT:
            {
                int ival = 0;
                dbiDict->getFeat(optionName.c_str(), &ival);
                FPS_DEBUG_PRINT(
                    "%s >> av option %s is in dbiAv and both option values "
                    "are different. Updating av option to int val %d\n",
                    __func__, optionName.c_str(), ival);
                avDict->setFeat(optionName.c_str(), ival);
                break;
            }
            case assFeat::AFTypes::AFLOAT:
            {
                double dval = 0.0;
                dbiDict->getFeat(optionName.c_str(), &dval);
                FPS_DEBUG_PRINT(
                    "%s >> av option %s is in dbiAv and both option values "
                    "are different. Updating av option to double val %f\n",
                    __func__, optionName.c_str(), dval);
                avDict->setFeat(optionName.c_str(), dval);
                break;
            }
            case assFeat::AFTypes::ASTRING:
            {
                char* cval = (char*)"val";
                dbiDict->getFeat(optionName.c_str(), &cval);
                FPS_DEBUG_PRINT(
                    "%s >> av option %s is in dbiAv and both option values "
                    "are different. Updating av option to string val %s\n",
                    __func__, optionName.c_str(), cval);
                avDict->setFeat(optionName.c_str(), cval);
                break;
            }
            case assFeat::AFTypes::ABOOL:
            {
                bool bval = false;
                dbiDict->getFeat(optionName.c_str(), &bval);
                FPS_DEBUG_PRINT(
                    "%s >> av option %s is in dbiAv and both option values "
                    "are different. Updating av option to bool val %s\n",
                    __func__, optionName.c_str(), bval ? "true" : "false");
                avDict->setFeat(optionName.c_str(), bval);
                break;
            }
            default:
                FPS_ERROR_PRINT(
                    "%s >> assFeat type not supported for dbi variable. "
                    "Supported types are: AINT, AFLOAT, ASTRING, and ABOOL. "
                    "Skipping option set to dbi and assetVar %s\n",
                    __func__, dbiAv->name.c_str());
        }
    }
}

/**
 * @brief Gets the variable from the dbi data storage system.
 *        An assetVar will not have a dbi value on start up, so this indicates
 * that the dbi needs to be fetched. This should then trigger the CheckDbiResp
 * function
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param av the assetVar referencing the dbi
 */
/*
We send a get /dbi/ess_controller/status_pcs

Sadly we get this back from dbi
Method:  pub
Uri:     /ess/dbi/status/pcs
ReplyTo: (null)
Body:
{"status_pcs":{"MaxDeratedChargeCmd":{"value":-3510,"EnableDbiUpdate":true,"UpdateTimeCfg":5,"UpdateTimeRemain":5,"dbiStatus":"OK","tLast":1542.074574999977},"MaxDeratedDischargeCmd":{"value":3510,"EnableDbiUpdate":true,"UpdateTimeCfg":5,"UpdateTimeRemain":0,"dbiStatus":"OK","tLast":1542.0746829999844}}}
Timestamp:   2021-07-16 12:32:44.492924
so anything coming in for dbi gets the first object child inside the body rather
than the body
*/
int GetDbiVar(varsmap& vmap, varmap& amap, fims* p_fims, const char* aname, assetVar* av)
{
    UNUSED(vmap);
    UNUSED(amap);
    // char* uri = nullptr;
    // char* replyto = nullptr;

    // get /dbi/ess_controller/config/bms:DemoChargeCurrent
    // replyto /ess/dbi/config/bms
    // /dbi/config/bms:DemoChargeCurrent will have an action to check and then
    // update the value
    //  to /config/bms:DemoChargeCurrent
    // it will also update dbiState to "dbiOK"

    // asprintf(&replyto, ("/ess/dbi" + av->comp).c_str());
    std::string uristr = fmt::format("/dbi/ess_controller/status_{}", aname);
    // std::string repstr = fmt::format("/ess/dbi{}/{}", av->comp, av->name);
    std::string repstr = fmt::format("/ess/dbi{}", av->comp);

    if (av->gotParam("dbiDest") && av->getcParam("dbiDest"))
    {
        // asprintf(&uri, av->getcParam("dbiDest"));
        uristr = av->getcParam("dbiDest");
    }
    // asprintf(&uri, ("/dbi/ess_controller" + av->comp).c_str());
    if (0)
        FPS_ERROR_PRINT(
            "%s >> Sending get request to uri %s with replyto %s from "
            "assetVar %s\n",
            __func__, uristr.c_str(), repstr.c_str(), av->getfName());
    p_fims->Send("get", uristr.c_str(), repstr.c_str(), nullptr);

    return 0;
}

// this needs to send it into the dbi/ess_controller space
// This runs whenever /config/bms:DemoChargeCurrent changes state or every 5
// seconds anyway
/**
 * @brief Set the dbi assetVar to the dbi data storage system. This will update
 * all dbi assetVar's parameters and actions
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param p_fims the fims object for data interchange between ess controller and
 * dbi
 * @param av the assetVar referencing the dbi
 */

int SetDbiVar(varsmap& vmap, varmap& amap, fims* p_fims, const char* aname, assetVar* av)
{
    UNUSED(amap);
    FPS_PRINT_INFO("Setting dbi var for aname [{}] var [{}] ", aname, av->getfName());

    VarMapUtils* vm = av->am->vm;

    varsmap* vlist = vm->createVlist();

    // In order to properly update a variable in the dbi, we'll need to compose a
    // list of the rest of the dbi variables that contains the same component uri
    // in the vlist before updating the dbi. This is to ensure all of the dbi
    // variables are intact after the update

    // we need to take the comp and replace /a/b/c_d with /a_b_c__d
    // used in clone
    // char* res = vm->run_replace(const char* inStr, const char* replace, const
    // char* with)

    const std::string dbiUri = fmt::format("/dbi{}", av->comp);
    if (vmap.find(av->comp) != vmap.end())
    {
        varmap avMap = vmap[av->comp];
        varmap dbiAvMap = vmap[dbiUri];
        for (const auto& pair : avMap)
        {
            assetVar* var = pair.second;
            assetVar* dbiVar = dbiAvMap[var->name];
            if (dbiVar && var->name == dbiVar->name)
            {
                FPS_DEBUG_PRINT("%s >> adding assetVar [%s] to dbiVlist\n", __func__, var->getfName());
                vm->addVlist(vlist, var, (fmt::format("/status_{}", aname).c_str()));
            }
        }
    }

    vm->sendDbiVlist(p_fims, "set", vlist);
    vm->clearVlist(vlist);

    return 0;
}

/**
 * @brief Assigns parameters for the assetVar referencing the dbi
 *
 * @param aname the name of the asset/asset manager
 * @param av the assetVar referencing the dbi
 */
int SetupAvParams(const char* aname, assetVar* av)
{
    double dval = 5.0;
    char* cval = (char*)"init";
    bool bval = true;
    double tNow = av->am->vm->get_time_dbl();
    FPS_PRINT_INFO("setup for  [{}]  [{}]", aname, av->name.c_str());

    av->setParam("tLast", tNow);
    av->setParam("dbiStatus", cval);
    if (!av->gotParam("UpdateTimeCfg"))
        av->setParam("UpdateTimeCfg", dval);
    av->setParam("UpdateTimeRemain", av->getdParam("UpdateTimeCfg"));
    if (!av->gotParam("EnableDbiUpdate"))
        av->setParam("EnableDbiUpdate",
                     bval);  // Enable dbi update by default if param is not already provided

    return 0;
}

/**
 * @brief Assigns parameters for the dbi assetVar
 *
 * @param aname the name of the asset/asset manager
 * @param av the dbi assetVar
 */

// no need to do this
int SetupDbiAvParams(const char* aname, assetVar* dbiAv)
{
    UNUSED(aname);
    bool bval = false;
    dbiAv->setParam("dbiSet", bval);

    return 0;
}

/**
 * @brief Performs an update to dbi or a fetch to dbi on startup.
 *        If the assetVar value is different to the dbi value, then the assetVar
 * needs to be sent to the dbi. The time of last update is also saved. An update
 * will also be sent to the dbi after a certain amount of time.
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param p_fims the fims object for data interchange between ess controller and
 * dbi
 * @param av the assetVar referencing the dbi
 */
void UpdateDbi(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    if (0)
        FPS_PRINT_INFO("checking dbi aname [{}] with assetVar [{}]", aname, av->getfName());

    VarMapUtils* vm = av->am->vm;

    double tNow = vm->get_time_dbl();
    double tLast = av->getdParam("tLast");
    double tDiff = tNow - tLast;
    av->setParam("tLast", tNow);

    double updateTimeCfg = av->getdParam("UpdateTimeCfg");
    double updateTimeRemain = av->getdParam("UpdateTimeRemain");
    bool enableDbiUpdate = av->getbParam("EnableDbiUpdate");
    int dbiCnt = 0;
    // On init,  set the state to fetch
    // On fetch, grab the value from the dbi and store that to the assetVar
    if (av->gotParam("dbiStatus") && av->getcParam("dbiStatus") && !strcmp(av->getcParam("dbiStatus"), "init"))
    {
        FPS_ERROR_PRINT("%s >> dbiStatus [init] for assetVar [%s]\n", __func__, av->getfName());
        av->setParam("dbiStatus", (char*)"fetch");
        av->setParam("dbiCnt", dbiCnt);
        double dval = 0.0;
        av->setParam("UpdateTimeRemain", dval);
        // force the transition from init to fetch
        updateTimeRemain = 0;
        tDiff = 1;
        FPS_PRINT_INFO("dbiStatus is init for assetVar [{}], so we're checking dbi first\n", av->name);
    }
    if (av->gotParam("dbiStatus") && av->getcParam("dbiStatus") && !strcmp(av->getcParam("dbiStatus"), "fetch"))
    {
        if (0)
            FPS_PRINT_INFO("dbiStatus [fetch] for assetVar [{}] tDiff [{:2.3f}]", av->getfName(), tDiff);
        if (tDiff > updateTimeRemain)
        {
            dbiCnt = av->getiParam("dbiCnt");
            dbiCnt++;
            av->setParam("dbiCnt", dbiCnt);
            av->setParam("UpdateTimeRemain", updateTimeCfg);
            GetDbiVar(vmap, amap, p_fims, aname, av);
        }
        else
        {
            updateTimeRemain -= tDiff;
            updateTimeRemain = updateTimeRemain >= 0.0 ? updateTimeRemain : 0.0;
            av->setParam("UpdateTimeRemain", updateTimeRemain);
        }
        // this is done by checkdbiresp
        // av->setParam("dbiStatus", (char*)"fetch");
    }

    // Otherwise, check if the assetVar changes its value or 5 seconds (or
    // configurable time) have passed and then set to dbi
    else
    {
        dbiCnt = av->getiParam("dbiCnt");
        dbiCnt++;
        av->setParam("dbiCnt", dbiCnt);
        if (!enableDbiUpdate)
        {
            FPS_PRINT_INFO(
                "enableDbiUpdate for assetVar [{}] with uri {} is false. "
                "Skipping update to dbi",
                av->name, av->comp);
            return;
        }
        // may not use valueChangedRest others may use that
        // use lastdbiVal
        // may not continually update either
        if (av->valueChangedReset() /*|| updateTimeRemain <= 0.0*/)
        {
            av->setParam("dbiAct", dbiCnt);
            FPS_PRINT_INFO("calling SetDbiVar assetVar [{}] with uri [{}].", av->name, av->comp);
            SetDbiVar(vmap, amap, p_fims, aname, av);
            // Reset the dbi refresh time
            av->setParam("UpdateTimeRemain", updateTimeCfg);
        }
        else
        {
            av->setParam("dbiSkip", dbiCnt);
        }

        // Decremement refresh time if the assetVar value remains unchanged
        if (updateTimeRemain > 0)
        {
            updateTimeRemain -= tDiff;
            updateTimeRemain = updateTimeRemain >= 0.0 ? updateTimeRemain : 0.0;
            av->setParam("UpdateTimeRemain", updateTimeRemain);
        }
    }
}

/**
 * @brief Performs an update to the assetVar referencing the dbi
 *        If the dbi assetVar value has changed, or if the dbiSet param is true,
 * then the assetVar referencing the dbi needs to be updated.
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param p_fims the fims object for data interchange between ess controller and
 * dbi
 * @param av the dbi assetVar
 */
void UpdateAvFromDbi(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* dbiAv)
{
    UNUSED(amap);
    UNUSED(aname);
    UNUSED(p_fims);
    asset_manager* am = dbiAv->am;
    VarMapUtils* vm;

    vm = am->vm;
    // Since this function is expected to run periodically, we'll want to update
    // av if the dbiAv value has changed or if the param dbiSet is true

    // dbiSet may be set to true here. If so, set this back to false so we don't
    // repeatedly run the code block below
    if (dbiAv->gotParam("dbiSet") && dbiAv->getbParam("dbiSet"))
        dbiAv->setParam("dbiSet", false);

    if (0)
    {
        FPS_DEBUG_PRINT(
            "%s >> dbiAv val has changed or dbiSet is true. Updating "
            "av and dbi %s\n",
            __func__, dbiAv->name.c_str());
    }
    const char* avComp = NULL;
    size_t idx = dbiAv->comp.find("/dbi");

    // Make sure dbiVar actually contains /dbi
    // We'll take the substring of the component uri (excluding /dbi)

    // Why search just look for the name
    bool update = false;
    if (idx != std::string::npos && idx == 0)
    {
        avComp = dbiAv->getfName();
        avComp += strlen("/dbi");
        assetVar* av = vm->getVar(vmap, avComp, nullptr);
        if (!av)
        {
            FPS_PRINT_ERROR("no av found for dbiAv  [{}]", avComp);
            return;
        }
        // for this release we'll just set up doubles   but maybe not
        if (av->gotParam("dbiStatus"))
        {
            std::string stat = av->getcParam("dbiStatus");
            if ((stat == "init") || (stat == "fetch"))
            {
                update = true;
                av->setcParam("dbsStatus", (char*)"OK");
            }
        }
        if (dbiAv->valueChangedReset() || (dbiAv->gotParam("dbiSet") && !dbiAv->getbParam("dbiSet")))
        {
            update = true;
        }
        if (!update)
        {
            if (0)
                FPS_PRINT_INFO(
                    "dbiAv val has not changed or dbiSet is false."
                    " Skipping update to av and dbi [{}]",
                    dbiAv->getfName());
            return;
        }
        dbiAv->setParam("dbiSet", false);

        // if av has a param "dbiStatus":"init",

        // then we update anyway and change status to OK
        // need to look for setval from an av
        // Check to make sure both av and dbiAv data types are the same
        // av->setVal(dbiAv);
        // note this will NOT trigger any actions

        if (av->type == dbiAv->type)
        {
            switch (av->type)
            {
                case assetVar::ATypes::AINT:
                    av->setVal(dbiAv->getiVal());
                    break;
                case assetVar::ATypes::AFLOAT:
                    av->setVal(dbiAv->getdVal());
                    break;
                case assetVar::ATypes::ASTRING:
                    av->setVal(dbiAv->getcVal());
                    break;
                case assetVar::ATypes::ABOOL:
                    av->setVal(dbiAv->getbVal());
                    break;
                default:
                    FPS_PRINT_ERROR(
                        "Type not supported for dbi variable."
                        " Supported types are: AINT, AFLOAT, ASTRING, and "
                        "ABOOL. Skipping set to dbi and assetVar {}",
                        dbiAv->getfName());
                    break;
            }

            // Check dbi parameters, actions, and options and set those values to av
            // as appropriate

            // updateParamsFromDbi(av, dbiAv);
            // updateActionsFromDbi(av, dbiAv);
            // updateOptionsFromDbi(av, dbiAv);  // Note OPtions are used
        }
        else
        {
            FPS_PRINT_ERROR(
                "The data types for dbiAv and the assetVar referencing "
                "the dbiAv are not the same."
                " Skipping set to dbi and assetVar {}",
                dbiAv->name);
        }
    }
    else
    {
        FPS_PRINT_ERROR(
            "dbiAv uri {} does not contain /dbi. Skipping set to dbi "
            "and assetVar {}",
            dbiAv->comp, dbiAv->name);
    }
}

/**
 * @brief Periodically check the response received by the dbi assetVar before
 * setting to the dbi and the assetVar the dbi is referencing.
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param p_fims the fims object for data interchange between ess controller and
 * dbi
 * @param dbiAv the dbi assetVar
 */
int CheckDbiResp(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* dbiAv)
{
    // FPS_DEBUG_PRINT("%s >> Running \n", __func__);
    if (0)
        FPS_PRINT_INFO("Running for assetVar [{}] with aname [{}] value [{:2.3f}]", dbiAv->getfName(), aname,
                       dbiAv->getdVal());
    asset_manager* am = dbiAv->am;
    if (!am)
    {
        FPS_PRINT_ERROR("no am assigned to dbiVar [{}]", dbiAv->getfName());
        return 0;
    }
    // VarMapUtils* vm = am->vm;
    // char *logName = (char*) "DBIVarLog";
    // char* logFile = (char*) "file:DBIVarLog.txt";
    essPerf ePerf(dbiAv->am, aname, __func__);

    // std::string reloadStr = fmt::format("{}_{}", dbiAv->getfName(), "reload");
    // int reload = vm->CheckReload(vmap, amap, aname, reloadStr.c_str());
    // if(reload < 2)
    // {
    //     int ival = 0;
    //     amap[logName] = vm->setLinkVal(vmap, essName, "/logs", logName,
    //     logFile); if (!amap[logName]->getcVal())
    //     amap[logName]->setVal(logFile);

    //     if(reload < 1)
    //     {
    //         FPS_DEBUG_PRINT("%s >> Not set up for [%s] reloadStr [%s]\n",
    //         __func__, dbiAv->name.c_str(),
    //         reloadStr.c_str());

    //         // Check if the assetVar we're working with already has a log file
    //         char* logFile = nullptr;
    //         if(dbiAv->gotParam("logName"))
    //             logFile =  dbiAv->getcParam("logName");
    //         if (!logFile)
    //         {
    //             // If the assetVar does not have the log file, check if the
    //             local map has a log file if (amap[logName])
    //             {
    //                 logFile = amap[logName]->getcVal();
    //                 FPS_DEBUG_PRINT("%s >> Log file name from amap %s is %s\n",
    //                 __func__, aname, logFile);
    //             }
    //             else
    //             {
    //                 logFile = (char*)"file:DBIVarLog.txt";
    //             }
    //         }
    //         // Param for checking if logging is enabled or not
    //         amap[logName]->setParam("enablePerf", true);

    //         //amap[logName]->setVal(logFile);
    //         //
    //         dbiAv->am->amap[logName]->openLog(vm->runLogDir?vm->runLogDir:(char*)"run_logs",logFile);
    //         // dbiAv->am->amap[logName]->logAlways();

    //         SetupDbiAvParams(aname, dbiAv);
    //     }
    //     ival = 2;
    //     amap[reloadStr] = dbiAv->am->vm->setLinkVal(vmap, aname, "/reload",
    //     reloadStr.c_str(), ival); amap[reloadStr]->setVal(ival);
    // }

    UpdateAvFromDbi(vmap, amap, aname, p_fims, dbiAv);

    return 0;
}

/**
 * @brief Periodically check if the variables need to be updated in the dbi
 * system.
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param p_fims the fims object for data interchange between ess controller and
 * dbi
 * @param av the assetVar referencing the dbi
 */
int CheckDbiVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* dbiAv)
{
    // FPS_DEBUG_PRINT("%s >> Running \n", __func__);
    if (0)
        FPS_PRINT_INFO("Running for assetVar [{}] with aname [{}]\n", dbiAv->getfName(), aname);
    // return 0;
    assetVar* av = dbiAv;
    VarMapUtils* vm = av->am->vm;
    char* essName = vm->getSysName(vmap);
    char* logName = (char*)"DBIVarLog";
    char* logFile = (char*)"file:DBIVarLog.txt";
    essPerf ePerf(av->am, aname, __func__);

    std::string reloadStr = fmt::format("{}_{}", av->name, "reload");
    int reload = vm->CheckReload(vmap, amap, aname, reloadStr.c_str());
    if (reload < 2)
    {
        int ival = 0;
        amap[logName] = vm->setLinkVal(vmap, essName, "/logs", logName, logFile);
        if (!amap[logName]->getcVal())
            amap[logName]->setVal(logFile);

        if (reload < 1)
        {
            FPS_PRINT_INFO("Not set up for [{}] reloadStr [{}]\n", av->name, reloadStr);

            // Check if the assetVar we're working with already has a log file
            char* logFile = nullptr;
            if (dbiAv->gotParam("logName"))
                logFile = dbiAv->getcParam("logName");
            if (!logFile)
            {
                // If the assetVar does not have the log file, check if the local map
                // has a log file
                if (amap[logName])
                {
                    logFile = amap[logName]->getcVal();
                    FPS_PRINT_INFO("Log file name from amap [{}] is [{}]", aname, logFile);
                }
                else
                {
                    logFile = (char*)"file:DBIVarLog.txt";
                }
            }
            // Param for checking if logging is enabled or not
            amap[logName]->setParam("enablePerf", true);

            SetupAvParams(aname, av);
        }
        ival = 2;
        amap[reloadStr] = av->am->vm->setLinkVal(vmap, aname, "/reload", reloadStr.c_str(), ival);
        amap[reloadStr]->setVal(ival);
    }

    UpdateDbi(vmap, amap, aname, p_fims, av);
    return 0;
}
// TODO use (or create) the common util
bool setValfromAf(assetVar* av, assFeat* af)
{
    bool ret = false;
    if (af->type == assFeat::AINT)
    {
        ret = true;
        av->setVal(af->valueint);
        // setaVType(av, (int)assetVar::AINT);
    }
    else if (af->type == assFeat::AFLOAT)
    {
        ret = true;
        av->setVal(af->valuedouble);
        // setaVType(av, (int)assetVar::AFLOAT);
    }
    else if (af->type == assFeat::ASTRING)
    {
        ret = true;
        av->setVal(af->valuestring);
        // setaVType(av, (int)assetVar::ASTRING);
    }
    else if (af->type == assFeat::ABOOL)
    {
        ret = true;
        av->setVal(af->valuebool);
        // setaVType(av, (int)assetVar::ABOOL);
    }
    else
    {
        if (1)
            FPS_PRINT_ERROR(" Unable to set value type {} for {}", af->type, av->getfName());
    }
    return ret;
}

/**
 * @brief Periodically check if the variables need to be updated in the dbi
 * system.
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param p_fims the fims object for data interchange between ess controller and
 * dbi
 * @param av the assetVar referencing the dbi
 */

// we have to set up  a check request
// I think the wake_monitor stuff just ends the time to the value.
int SendDbiVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    UNUSED(p_fims);
    // FPS_DEBUG_PRINT("%s >> Running \n", __func__);

    // return 0;
    VarMapUtils* vm = nullptr;
    asset_manager* am = av->am;
    if (am)
    {
        vm = am->vm;
    }
    if (!vm)
    {
        FPS_PRINT_ERROR("no Vm  for assetVar [{}] with aname [{}]", av->getfName(), aname);
        return -1;
    }
    char* logName = (char*)"DBIVarLog";
    char* logFile = (char*)"file:DBIVarLog.txt";
    essPerf ePerf(av->am, aname, __func__);
    double tNow = vm->get_time_dbl();
    char* essName = vm->getSysName(vmap);

    std::string reloadStr = fmt::format("{}_{}", av->name, "reload");
    int reload = vm->CheckReload(vmap, amap, aname, reloadStr.c_str());
    bool debug = av->getbParam("debug");

    if (debug)
        FPS_PRINT_INFO("Running for assetVar [{}] with aname [{}]", av->getfName(), aname ? aname : "no aname");

    if (reload < 2)
    {
        int ival = 0;
        amap[logName] = vm->setLinkVal(vmap, essName, "/logs", logName, logFile);
        if (!amap[logName]->getcVal())
            amap[logName]->setVal(logFile);

        if (reload < 1)
        {
            if (debug)
                FPS_PRINT_INFO("Not set up for [{}] reloadStr [{}]\n", av->name, reloadStr);

            // Check if the assetVar we're working with already has a log file
            char* logFile = av->getcParam("logName");
            if (!logFile)
            {
                // If the assetVar does not have the log file, check if the local map
                // has a log file
                if (amap[logName])
                {
                    logFile = amap[logName]->getcVal();
                    if (debug)
                        FPS_PRINT_INFO("Log file name from amap [{}] is [{}]", aname, logFile);
                }
                else
                {
                    logFile = (char*)"file:DBIVarLog.txt";
                }
            }
            // Param for checking if logging is enabled or not
            amap[logName]->setParam("enablePerf", true);

            //            SetupAvParams(aname, av);
        }
        ival = 2;
        amap[reloadStr] = av->am->vm->setLinkVal(vmap, aname, "/reload", reloadStr.c_str(), ival);
        amap[reloadStr]->setVal(ival);
    }
    // from wake_monitor we get a double value
    // from dbi we get a string value
    char* vactch = av->getcVal();
    if (!vactch)
    {
        if (av->gotParam("_dbAction"))
        {
            vactch = av->getcParam("_dbAction");
        }
    }

    if (!vactch)
    {
        FPS_PRINT_ERROR(" no _dbAction for  [{}] ", av->getfName());
        return -1;
    }
    std::string vact = vactch;
    if (debug)
        FPS_PRINT_INFO(" vact for  [{}] is [{}]", av->getfName(), vact);

    // init

    // set
    // char *VarMapUtils::getDbiNameAv(std::string& comp)

    if (vact == "set")
    {
        bool enable = true;
        double upTimeCfg = av->getdParam("_UpdateTimeCfg");
        if (upTimeCfg == 0)
        {
            upTimeCfg = 5;  // default to 5 seconds
        }
        double upTime = av->getdParam("_UpdateTime");
        if (upTime < tNow)
        {
            upTime = tNow + upTimeCfg;
            av->setParam("_UpdateTime", upTime);
        }
        else
        {
            if (debug)
                FPS_PRINT_INFO("[{}] db update too soon ", av->getfName());
            return 0;
        }

        if (av->gotParam("_EnableDbiUpdate"))
        {
            enable = av->getbParam("_EnableDbiUpdate");
        }

        if (!enable)
        {
            FPS_PRINT_INFO("[{}] db update not enabled", av->getfName());
            return 0;
        }
        if (av->gotParam("_pending"))
        {
            int pending = av->getiParam("_pending");
            if (pending == 0)
            {
                FPS_PRINT_INFO("[{}] db update no pending updates", av->getfName());
                return 0;
            }
            pending = 0;
            av->setParam("_pending", pending);
        }

        int idx = 0;
        char* comp = nullptr;
        if (av->gotParam("_comp"))
        {
            comp = av->getcParam("_comp");
        }
        if (comp)
        {
            for (auto xx : av->extras->baseDict->featMap)
            {
                auto var = vm->getVar(vmap, comp, xx.first.c_str());
                if (var)
                {
                    av->extras->baseDict->setFeatfromAv(xx.first.c_str(), var);
                }

                if (debug)
                    FPS_PRINT_INFO(" [{}] name  [{}] var [{}]", idx++, xx.first, var ? var->getfName() : "noVar");
            }
        }
        // char *essName = vm->sysName?vm->sysName:(char*)essName;
        av->setVal("resp");
        av->setcParam("_dbAction", (char*)"resp");

        av->setParam("_DbiDoc", (char*)av->name.c_str());

        auto body = fmt::format("{:f}", av);
        // go back to set
        av->setVal("set");
        av->setcParam("_dbAction", (char*)"set");

        if (debug)
            FPS_PRINT_INFO(" [{}] body  [{}]", av->getfName(), body);
        // auto uristr =
        // fmt::format("/dbi/ess_controller/dbivars{}",av->getfName());
        auto uristr = fmt::format("{}/{}", av->comp, av->name);
        auto uripart = vm->getDbiNameAv(uristr);
        if (debug)
            FPS_PRINT_INFO(" [{}] uri  [/dbi/ess_controller/dbivars{}]", av->getfName(), uripart);
        auto uri = fmt::format("/dbi/ess_controller/dbivars{}", uripart);
        // auto ruri = fmt::format("/{}/test/dbiresp/{}", essName, av->name);
        // vm->p_fims->Send("set", uri.c_str(), ruri.c_str(), body.c_str());
        vm->p_fims->Send("set", uri.c_str(), nullptr, body.c_str());
        // free(uripart);
    }

    else if (vact == "get")
    {
        av->setVal("wait");
        av->setcParam("_dbAction", (char*)"wait");
        double dval = 0.0;
        double rtime = 0.0;
        // used to trigger default value setup
        dval = vm->get_time_dbl();
        rtime = av->getdParam("_RespTimeCfg");
        if (rtime == 0)
        {
            rtime = 0.5;
        }
        dval += rtime;
        av->setParam("_RespTime", dval);

        // auto body = fmt::format("{:f}", av);
        // FPS_PRINT_INFO(" [{}] body  [{}]", av->getfName(), body);
        // auto uristr =
        // fmt::format("/dbi/ess_controller/dbivars{}",av->getfName());
        auto uristr = fmt::format("{}/{}", av->comp, av->name);
        auto repstr = fmt::format("/ess/resp{}/{}", av->comp, av->name);
        auto uripart = vm->getDbiNameAv(uristr);
        if (debug)
            FPS_PRINT_INFO(" [{}] uri  [/dbi/ess_controller/dbivars{}]", av->getfName(), uripart);
        auto uri = fmt::format("/dbi/ess_controller/dbivars{}", uripart);
        vm->p_fims->Send("get", uri.c_str(), repstr.c_str(), nullptr);
        // free(uripart);
    }
    // this is the resp from dbi
    else if (vact == "resp")
    {
        char* comp = nullptr;
        if (av->gotParam("_comp"))
        {
            comp = av->getcParam("_comp");
        }
        // we need to let _comp:av->name know that we are done
        // to stop it doing the default thing.
        if (debug)
            FPS_PRINT_INFO("DBI RESP 1>>>av [{}]  handle resp params :", av->getfName());
        char* dname = nullptr;
        if (av->gotParam("_DbiDoc"))
        {
            dname = av->getcParam("_DbiDoc");
        }
        if (dname)
        {
            auto docVar = fmt::format("{}:{}", comp, dname);
            assetVar* docAv = vm->getVar(vmap, docVar.c_str(), nullptr);
            if (docAv)
            {
                docAv->setParam("_RespTime", tNow);
                docAv->setVal("set");
                docAv->setcParam("_dbAction", (char*)"set");
            }
        }
        if (debug)
            FPS_PRINT_INFO("DBI RESP 2>>>av [{}]  handle resp params :", av->getfName());

        int idx = 0;
        if (!av->extras->baseDict)
        {
            FPS_PRINT_ERROR("DBI RESP 2++>>>av [{}]  Params gone !! :", av->getfName());
        }
        else
        {
            for (auto xx : av->extras->baseDict->featMap)
            {
                if (comp)
                {
                    auto var = vm->getVar(vmap, comp, xx.first.c_str());
                    if (debug)
                        if (var)
                            FPS_PRINT_INFO(
                                "DBI RESP 3>>> xx.first [{}:{}]  var [{}] handle "
                                "resp params :",
                                comp, xx.first, fmt::ptr(var));

                    if (var)
                    {
                        // var->
                        setValfromAf(var, xx.second);
                        // av->extras->baseDict->setFeatfromAv(xx.first.c_str(), var);
                        if (debug)
                            FPS_PRINT_INFO(
                                "DBI RESP >>>  idx  [{}] setting value for av "
                                "[{}] name  [{}]",
                                idx, var->getfName(), xx.first);
                    }
                    else
                    {
                        if (0)
                            FPS_PRINT_ERROR("DBI RESP >>>  idx  [{}] var [{}:{}] not found", idx, comp, xx.first);
                    }
                    idx++;
                }
            }
            if (debug)
                FPS_PRINT_INFO("DBI RESP 4++>>>av [{}]  deleting Params :", av->getfName());
            delete (av->extras->baseDict);
            av->extras->baseDict = nullptr;
            av->setVal("none");
            av->setcParam("_dbAction", (char*)"none");
        }
    }
    else if (vact == "Init")
    {
        av->setVal("get");
        av->setcParam("_dbAction", (char*)"get");
    }
    else
    {
        FPS_PRINT_ERROR("[{}] operation for dbiAction [{}] is not defined", av->getfName(), vact);
    }
    // get
    // UpdateDbi(vmap, amap, aname, p_fims, av);
    return 0;
}

/**
 * @brief Sets up a variable as a member of a DbiDoc(auto created) , updates the
 * DbiDoc Param and pending count when it changes value
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param p_fims the fims object for data interchange between ess controller and
 * dbi
 * @param av the assetVar referencing the dbi
 */
// will create DbiDoc if it needs to
// will create _pending if it needs to
// inserts the value into a DbiDoc Param
int SetDbiDoc(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    UNUSED(amap);
    UNUSED(p_fims);
    char* dname = nullptr;
    asset_manager* am = av->am;
    VarMapUtils* vm = nullptr;

    // FPS_DEBUG_PRINT("%s >> Running \n", __func__);
    bool debug = av->getbParam("debug");

    if (debug)
        FPS_PRINT_INFO("Running for assetVar [{}] with aname [{}]", av->getfName(), aname);
    if (!av->gotParam("DbiDoc"))
    {
        FPS_PRINT_ERROR(" [{}] has no DbiDoc param", av->getfName());
        return -1;
    }
    if (am)
    {
        vm = am->vm;
    }
    else
    {
        FPS_PRINT_ERROR(" [{}] has no am", av->getfName());
        return -1;
    }
    if (!vm)
    {
        FPS_PRINT_ERROR(" [{}] has no vm", av->getfName());
        return -1;
    }

    dname = av->getcParam("DbiDoc");
    if (!dname)
    {
        FPS_PRINT_INFO(" [{}] has no DbiDoc", av->getfName());
        dname = (char*)"DbiDoc";
        av->setParam("DbiDoc", dname);
    }

    auto dbiDoc = fmt::format("{}:{}", av->comp, dname);
    assetVar* dbiAv = vm->getVar(vmap, dbiDoc.c_str(), nullptr);
    if (!dbiAv)
    {
        const char* cVal = "init";
        dbiAv = vm->setVal(vmap, dbiDoc.c_str(), nullptr, cVal);
    }
    dbiAv->setParamfromAv(av->name.c_str(), av);
    int pend = dbiAv->getiParam("_pending");
    pend++;
    dbiAv->setParam("_pending", pend);

    return 0;
}

#endif
