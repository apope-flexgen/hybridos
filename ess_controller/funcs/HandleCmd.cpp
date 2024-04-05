#ifndef HANDLECMD_CPP
#define HANDLECMD_CPP

#include "asset.h"
#include "calculator.hpp"
#include "ess_utils.hpp"
#include "formatters.hpp"

extern "C++" {
int HandleCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}

/**
 * @brief Determines if the system command has executed by checking whether the
 * value has been updated
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param av the assetVar that contains the command to check
 */
void checkCmd(varsmap& vmap, varmap& amap, const char* aname, assetVar* av)
{
    FPS_PRINT_DEBUG("Checking command for asset [{}] with [{}:{}]", cstr{ aname }, av->comp, av->name);
    VarMapUtils* vm = av->am->vm;

    double tNow = vm->get_time_dbl();
    double tDiff = tNow - av->getdParam("tLast");
    double checkCmdHoldTimeout = av->getdParam("checkCmdHoldTimeout");
    double checkCmdTimeout = av->getdParam("checkCmdTimeout");
    double currCheckCmdHoldTime = av->getdParam("currCheckCmdHoldTime");
    double currCheckCmdTime = av->getdParam("currCheckCmdTime");
    double maxCmdTries = av->getdParam("maxCmdTries");
    double currCmdTries = av->getdParam("currCmdTries");
    bool enableAlert = av->getbParam("enableAlert");
    bool checkCmdDone = false;
    bool triggerCmd = true;

    // Check to see if we have a cmdVar that we need to refer to
    assetVar* cmdAv = av;
    assetVar* retrievedAv = ESSUtils::getAvFromParam(vmap, amap, av, "cmdVar");
    if (retrievedAv)
    {
        FPS_PRINT_DEBUG("We have an assetVar [{}:{}] in the cmdVar param in [{}:{}]", retrievedAv->comp,
                        retrievedAv->name, av->comp, av->name);
        cmdAv = retrievedAv;
    }

    // Wait until hold time reaches 0 before checking the system command
    if (currCheckCmdHoldTime > 0)
        ESSUtils::decrementTime(av, currCheckCmdHoldTime, tDiff, "currCheckCmdHoldTime");

    if (currCheckCmdHoldTime <= 0)
    {
        FPS_PRINT_DEBUG(
            "Check command hold time reached 0. Now check if command "
            "value has been updated for asset [{}] with [{}:{}]",
            cstr{ aname }, cmdAv->comp, cmdAv->name);

        // Check if the system command value has been updated
        if (av->getdParam("lastCmdVal") != cmdAv->getdVal())
        {
            // Wait until check command wait time reaches 0 before determining that
            // the system command did not update successfully
            if (currCheckCmdTime > 0)
                ESSUtils::decrementTime(av, currCheckCmdTime, tDiff, "currCheckCmdTime");

            if (currCheckCmdTime <= 0)
            {
                currCmdTries++;
                int remainCmdTries = (maxCmdTries - currCmdTries) >= 0 ? (maxCmdTries - currCmdTries) : 0;
                FPS_PRINT_INFO(
                    "currCheckCmdTime reached 0 and currCmdTries is now [{}] for asset "
                    "[{}] with [{}:{}]. Remaining number of command tries [{}]",
                    currCmdTries, cstr{ aname }, av->comp, av->name, remainCmdTries);

                if (remainCmdTries <= 0)
                {
                    // Record check command failure
                    if (enableAlert)
                    {
                        const std::string msg = fmt::format(
                            "Reached max command tries [{}]. Cannot verify that command "
                            "value [{}] has been sent to [{}:{}]",
                            maxCmdTries, av->getdVal(), cmdAv->comp, cmdAv->name);
                        ESSUtils::record(vmap, amap, aname, av, msg, tNow, "ResponseFailure", Severity::Alarm);
                    }

                    // Set command result assetVar to false, indicating that the system
                    // command ran unsuccessfully
                    const std::string cmdSuccessAvName = fmt::format("{}Success", av->name);
                    ESSUtils::setAvStatus(vmap, amap, aname, av, cmdSuccessAvName, false, cmdSuccessAvName, false);
                    currCmdTries = 0;
                    triggerCmd = false;
                }

                // Reset hold and check times and increment current # of command tries .
                // At this point, we're done checking the command
                ESSUtils::incrementTime(av, currCheckCmdHoldTime, checkCmdHoldTimeout, checkCmdHoldTimeout,
                                        "currCheckCmdHoldTime");
                ESSUtils::incrementTime(av, currCheckCmdTime, checkCmdTimeout, checkCmdTimeout, "currCheckCmdTime");
                av->setParam("currCmdTries", currCmdTries);
                checkCmdDone = true;
            }
        }
        else
        {
            // Set command result assetVar to true, indicating that the system command
            // ran successfully
            const std::string cmdSuccessAvName = fmt::format("{}Success", av->name);
            ESSUtils::setAvStatus(vmap, amap, aname, av, cmdSuccessAvName, true, cmdSuccessAvName, false);

            // Reset hold and check times and reset current # of command tries back to
            // 0. At this point, we're done checking the command
            ESSUtils::incrementTime(av, currCheckCmdHoldTime, checkCmdHoldTimeout, checkCmdHoldTimeout,
                                    "currCheckCmdHoldTime");
            ESSUtils::incrementTime(av, currCheckCmdTime, checkCmdTimeout, checkCmdTimeout, "currCheckCmdTime");
            av->setParam("currCmdTries", 0);
            checkCmdDone = true;
            triggerCmd = false;

            // Record command value has been received
            if (enableAlert)
            {
                const std::string msg = fmt::format("Received command value [{}] from [{}:{}]", cmdAv->getdVal(),
                                                    cmdAv->comp, cmdAv->name);
                ESSUtils::record(vmap, amap, aname, av, msg, tNow, "ResponseSuccess", Severity::Info);
            }
        }
    }

    // Check if we have finished checking the command successfully or
    // unsuccessfully
    if (checkCmdDone && av->gotParam("cmdSent"))
    {
        FPS_PRINT_DEBUG(
            "Command check is done for [{}:{}]. Setting cmdSent to "
            "false and triggerCmd to {}",
            cmdAv->comp, cmdAv->name, triggerCmd);
        av->setParam("cmdSent", false);
        av->setParam("triggerCmd", triggerCmd);
    }
}

/**
 * @brief Sends out the system command value to the appropriate assetVar
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param p_fims the interface used for data interchange
 * @param av the assetVar that contains the command value to send
 */
void sendCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    FPS_PRINT_DEBUG("Sending command for asset [{}] with [{}:{}]", cstr{ aname }, av->comp, av->name);
    VarMapUtils* vm = av->am->vm;

    double tNow = vm->get_time_dbl();
    double tDiff = tNow - av->getdParam("tLast");
    double sendCmdHoldTimeout = av->getdParam("sendCmdHoldTimeout");
    double sendCmdTimeout = av->getdParam("sendCmdTimeout");
    double currSendCmdHoldTime = av->getdParam("currSendCmdHoldTime");
    double currSendCmdTime = av->getdParam("currSendCmdTime");
    bool enableAlert = av->getbParam("enableAlert");
    bool sendCmdOk = true;
    bool sendCmdDone = false;

    // Wait until hold time reaches 0 before sending the system command
    if (currSendCmdHoldTime > 0)
        ESSUtils::decrementTime(av, currSendCmdHoldTime, tDiff, "currSendCmdHoldTime");

    if (currSendCmdHoldTime <= 0)
    {
        FPS_PRINT_DEBUG(
            "Send command hold time reached 0. Now attempt to send "
            "command to asset [{}] with [{}:{}]",
            cstr{ aname }, av->comp, av->name);

        // Check if conditions are satisfied if the expression param exists
        if (av->gotParam("useExpr") && av->getbParam("useExpr"))
        {
            sendCmdOk = ESSUtils::checkConditions(vmap, amap, av, "expression");
            if (sendCmdOk)
                FPS_PRINT_INFO("Condition is satisfied for asset [{}] with [{}:{}]", cstr{ aname }, av->comp, av->name);
            else
            {
                FPS_PRINT_DEBUG("Condition is not satisfied or valid for asset [{}] with [{}:{}]", cstr{ aname },
                                av->comp, av->name);
            }
        }

        // Send command
        // If conditions are defined, then only send command if all conditions are
        // satisfied
        if (!sendCmdOk)
        {
            // Wait until send command wait time reaches 0 before determining that we
            // cannot send a command
            if (currSendCmdTime > 0)
                ESSUtils::decrementTime(av, currSendCmdTime, tDiff, "currSendCmdTime");

            if (currSendCmdTime <= 0)
            {
                // Record send command failure
                if (enableAlert)
                {
                    std::string msg = fmt::format("Cannot send command value [{}] to [{}:{}]", av->getdVal(), av->comp,
                                                  av->name);

                    assetVar* retrievedAv = ESSUtils::getAvFromParam(vmap, amap, av, "cmdVar");
                    if (retrievedAv)
                        msg = fmt::format("Cannot send command value [{}] to [{}:{}]", av->getdVal(), retrievedAv->comp,
                                          retrievedAv->name);

                    // If condition(s) are defined, include all conditions in the message
                    // for reporting
                    if (av->gotParam("useExpr") && av->getbParam("useExpr"))
                    {
                        const std::string exprInfo = CalculatorUtils::getExpressionInfo(vmap, amap, av, "expression");
                        if (!exprInfo.empty())
                            msg += fmt::format(". Condition(s): {}", exprInfo);
                    }

                    ESSUtils::record(vmap, amap, aname, av, msg, tNow, "SendFailure", Severity::Alarm);
                }

                // Set command result assetVar to false, indicating that the system
                // command ran unsuccessfully
                const std::string cmdSuccessAvName = fmt::format("{}Success", av->name);
                ESSUtils::setAvStatus(vmap, amap, aname, av, cmdSuccessAvName, false, cmdSuccessAvName, false);

                // Reset hold and send cmd times
                ESSUtils::incrementTime(av, currSendCmdHoldTime, sendCmdHoldTimeout, sendCmdHoldTimeout,
                                        "currSendCmdHoldTime");
                ESSUtils::incrementTime(av, currSendCmdTime, sendCmdTimeout, sendCmdTimeout, "currSendCmdTime");
                sendCmdDone = true;
            }
        }
        else
        {
            // Check to see if we have a cmdVar that we need to refer to
            assetVar* cmdAv = av;
            assetVar* retrievedAv = ESSUtils::getAvFromParam(vmap, amap, av, "cmdVar");
            if (retrievedAv)
            {
                FPS_PRINT_DEBUG("We have an assetVar [{}:{}] in the cmdVar param in [{}:{}]", retrievedAv->comp,
                                retrievedAv->name, av->comp, av->name);
                cmdAv = retrievedAv;
                cmdAv->setVal(av->getdVal());
            }

            av->setParam("lastCmdVal", av->getdVal());
            // vm->sendAssetVar(cmdAv, p_fims, "set");

            // Send out command value
            varsmap* vlist = vm->createVlist();
            vm->addVlist(vlist, cmdAv);
            vm->sendVlist(p_fims, "set", vlist);
            vm->clearVlist(vlist);

            // Set this to true to call subsequent calls to checkCmd
            if (av->gotParam("cmdSent"))
                av->setParam("cmdSent", true);

            // Reset hold and send cmd times
            ESSUtils::incrementTime(av, currSendCmdHoldTime, sendCmdHoldTimeout, sendCmdHoldTimeout,
                                    "currSendCmdHoldTime");
            ESSUtils::incrementTime(av, currSendCmdTime, sendCmdTimeout, sendCmdTimeout, "currSendCmdTime");
            sendCmdDone = true;

            // Record command has been sent
            if (enableAlert)
            {
                const std::string msg = fmt::format("Sent command value [{}] to [{}:{}]", cmdAv->getdVal(), cmdAv->comp,
                                                    cmdAv->name);
                ESSUtils::record(vmap, amap, aname, av, msg, tNow, "SendSuccess", Severity::Info);
            }
        }
    }

    // Set this to false to avoid having to call runCmd for every iteration
    if (sendCmdDone && av->gotParam("triggerCmd"))
    {
        av->setParam("triggerCmd", false);

        // Immediately call check operation to see if the command value has been
        // received
        if (av->getbParam("cmdSent"))
            checkCmd(vmap, amap, aname, av);
    }
}

/**
 * @brief Initializes the parameters to be used for running system command
 *
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param av the assetVar to initialize parameters for
 */
void setupCmdParams(varsmap& vmap, varmap& amap, const char* aname, assetVar* av)
{
    if (!av->am)
    {
        FPS_PRINT_DEBUG("asset manager for [{}:{}] is null", av->comp, av->name);
        return;
    }
    FPS_PRINT_INFO("Setting up params for [{}:{}]", av->comp, av->name);

    // Last command value that was sent
    if (!av->gotParam("lastCmdVal"))
        av->setParam("lastCmdVal", -1);

    // Amount of times to send the command before determining the command did not
    // send successfully
    if (!av->gotParam("maxCmdTries"))
        av->setParam("maxCmdTries", 0);
    if (!av->gotParam("currCmdTries"))
        av->setParam("currCmdTries", 0);

    // Amount of time to wait before sending system command - default to 0,
    // indicating immediate send
    if (!av->gotParam("sendCmdHoldTimeout"))
        av->setParam("sendCmdHoldTimeout", 0);
    if (!av->gotParam("currSendCmdHoldTime"))
        av->setParam("currSendCmdHoldTime", av->getdParam("sendCmdHoldTimeout"));

    // Amount of time to wait before checking system command - default to 0,
    // indicating immediate check
    if (!av->gotParam("checkCmdHoldTimeout"))
        av->setParam("checkCmdHoldTimeout", 0);
    if (!av->gotParam("currCheckCmdHoldTime"))
        av->setParam("currCheckCmdHoldTime", av->getdParam("checkCmdHoldTimeout"));

    // Amount of time to wait before determing that we cannot send the system
    // command
    if (!av->gotParam("sendCmdTimeout"))
        av->setParam("sendCmdTimeout", 0);
    if (!av->gotParam("currSendCmdTime"))
        av->setParam("currSendCmdTime", av->getdParam("sendCmdTimeout"));

    // Amount of time to wait before determing that running the system command was
    // unsuccessful
    if (!av->gotParam("checkCmdTimeout"))
        av->setParam("checkCmdTimeout", 10);
    if (!av->gotParam("currCheckCmdTime"))
        av->setParam("currCheckCmdTime", av->getdParam("checkCmdTimeout"));

    // Boolean to check whether the system command has been sent
    if (!av->gotParam("cmdSent"))
        av->setParam("cmdSent", false);

    // Check if the assetVar has a cmdVar param. This param is used for sending a
    // value to a specific command
    VarMapUtils* vm = av->am->vm;
    if (av->gotParam("cmdVar") && av->getcParam("cmdVar"))
    {
        const std::string avUri(av->getcParam("cmdVar"));
        FPS_PRINT_DEBUG("cmdVar  avUri {}  av {}", avUri, av->name);
        if (!avUri.empty())
        {
            assetUri my(avUri.c_str());

            // See if the assetVar exists in the data map if the uri is provided like
            // so: var_name
            if (!my.Var)
            {
                if (av->gotParam("cmdVar") && av->getcParam("cmdVar") && !amap[avUri])
                {
                    double dval = 0.0;
                    FPS_PRINT_INFO("Creating assetVar [{}] for cmdVar in {}", avUri, aname);
                    amap[avUri] = vm->setLinkVal(vmap, aname, "/controls", avUri.c_str(), dval);
                }
            }
            // Otherwise, we may have a full uri and a variable name. Something like
            // this: /component/asset_name:var_name
            else
            {
                assetVar* avParam = vm->getVar(vmap, avUri.c_str(), nullptr);
                // If the assetVar does not exist, create a new one. This should be
                // added in vmap
                if (!avParam)
                {
                    FPS_PRINT_INFO("assetVar param {} does not exist. Creating new one", avUri);
                    double dval = 0.0;
                    vm->makeVar(vmap, avUri.c_str(), nullptr, dval);
                }
            }
        }
    }

    // Condition(s) to check before sending the command
    if (!av->gotParam("expression"))
        av->setParam("expression", (char*)"n/a");

    // Set the option to include current value in assetVar to false by default
    if (!av->gotParam("includeCurrVal"))
        av->setParam("includeCurrVal", false);

    // Set up remaining params
    if (!av->gotParam("triggerCmd"))
        av->setParam("triggerCmd", false);

    if (!av->gotParam("useExpr"))
        av->setParam("useExpr", false);

    if (!av->gotParam("enableAlert"))
        av->setParam("enableAlert", true);
}

/**
 * @brief Runs the system command for a particular asset and updates the asset's
 * state
 *
 * @param vmap the global data map
 * @param amap the local data map
 * @param aname the asset manager/asset name
 * @param p_fims the interface used for data interchange
 * @param av the assetVar that contains the command to run
 */
int HandleCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    if (0)
        FPS_PRINT_INFO("running  [{}]  ", "test");
    if (0)
        FPS_PRINT_INFO("assetVar [{}]  aname [{}]   val [{}]", av->getfName(), cstr{ aname }, av->getdVal());

    VarMapUtils* vm = av->am->vm;
    const std::string reloadStr = "HandleCmd_" + av->name;
    int reload = vm->CheckReload(vmap, amap, aname, reloadStr.c_str());
    if (reload < 2)
    {
        FPS_PRINT_INFO("reload first run [{}] [{}]  reload {}", cstr{ aname }, av->getfName(), reload);
        setupCmdParams(vmap, amap, aname, av);

        if (reload < 1)
        {
            FPS_PRINT_INFO("Adding [{}] to amap in asset [{}]", av->getfName(), cstr{ aname });
            amap[av->name] = vm->setLinkVal(vmap, aname, "/controls", av->name.c_str(), av);
        }
        reload = 2;
        amap[reloadStr] = vm->setLinkVal(vmap, aname, "/reload", reloadStr.c_str(), reload);
        amap[reloadStr]->setVal(reload);
    }

    // Run or check the system command
    if (av->gotParam("triggerCmd") && av->gotParam("cmdSent"))
    {
        if (!av->getbParam("cmdSent"))
        {
            if (av->getbParam("triggerCmd"))
                sendCmd(vmap, amap, aname, p_fims, av);
        }
        else
            checkCmd(vmap, amap, aname, av);
    }

    av->setParam("tLast", vm->get_time_dbl());

    if (0)
        FPS_PRINT_INFO("done  [{}]  ", "test");

    return 0;
}

#endif
