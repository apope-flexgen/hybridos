#ifndef ESS_UTILS_HPP
#define ESS_UTILS_HPP

#include "asset.h"
#include "formatters.hpp"
#include "calculator.hpp"

// Namespace for common utility operations used in ESS (ex.: getting an assetVar from another assetVar's parameter)
namespace ESSUtils
{
    /**
     * @brief Decrease the current time and update the av's time
     * 
     * @param av the assetVar to update the current time for
     * @param currTime the current time to decrement
     * @param tDiff the time value used to decrement the current time
     * @param param the name of the time param to update
     */
    inline void decrementTime(assetVar* av, double currTime, double tDiff, const char* param)
    {
        FPS_PRINT_DEBUG("Decrementing time for av [{}:{}].   {} [{:2.3f}] tDiff [{:2.3f}]", av->comp, av->name, param, currTime, tDiff);
        currTime -= tDiff;
        currTime = currTime >= 0.0 ? currTime : 0.0;
        av->setParam(param, currTime);
    }

    /**
     * @brief Increase the current time and update the av's time
     * 
     * @param av the assetVar to update the current time for
     * @param currTime the current time to increase
     * @param tDiff the time value used to increment the current time
     * @param timeCfg the max time defined in the config that the current time can be updated to
     * @param param the name of the time param to update
     */
    inline void incrementTime(assetVar* av, double currTime, double tDiff, double& timeCfg, const char* param)
    {
        FPS_PRINT_DEBUG("Incrementing time for av [{}:{}].   {} [{:2.3f}] tDiff [{:2.3f}] timeCfg [{:2.3f}]", av->comp, av->name, param, currTime, tDiff, timeCfg);
        currTime += tDiff;
        currTime = currTime <= timeCfg ? currTime : timeCfg;
        av->setParam(param, currTime);
    }

    /**
     * @brief Sets the target status assetVar to either true or false
     * 
     * @param vmap the global data map shared by all assets/asset managers
     * @param amap the local data map used by an asset/asset manager
     * @param aname the name of the asset/asset manager
     * @param av the assetVar
     * @param targetAvName the name of the target status assetVar
     * @param result the boolean value to set to the target status assetVar
     */
    inline void setAvStatus(varsmap& vmap, varmap& amap, const char* aname, assetVar* av, const std::string& targetAvName, bool result,
                            const std::string& logName, bool logToFile = true)
    {
        VarMapUtils* vm = av->am->vm;
        if (!amap[targetAvName])
            amap[targetAvName] = vm->setLinkVal(vmap, aname, "/status", targetAvName.c_str(), result);

        // Log FaultShutdown set to console and file
        char* essName               = vm->getSysName(vmap);
        const std::string sysCfgUri = fmt::format("/config/{}", essName);
        const auto logDir = getLogDir(vmap, *vm);
        const std::string fileName  = fmt::format("{}_{}_{}_{}", essName, aname, av->name, logName.empty() ? targetAvName : logName);
        const std::string dir       = fmt::format("{}/{}.txt", logDir, fileName);
        const std::string msg       = fmt::format("Setting /status/{}:{} for [{}] to {}", aname, targetAvName, av->getfName(), result);

        ESSLogger::get().info(msg.c_str());
        if (logToFile && getLoggingEnabled(vmap, *vm))
            ESSLogger::get().logIt(dir, getLoggingTimestamp(vmap, *vm));

        vm->setVal(vmap, amap[targetAvName]->comp.c_str(), amap[targetAvName]->name.c_str(), result);
    }

    /**
     * @brief Sets the target status assetVar to a string value
     * 
     * @param vmap the global data map shared by all assets/asset managers
     * @param amap the local data map used by an asset/asset manager
     * @param aname the name of the asset/asset manager
     * @param av the assetVar
     * @param targetAvName the name of the target status assetVar
     * @param result the string value to set to the target status assetVar
     */
    inline void setAvStatus(varsmap& vmap, varmap& amap, const char* aname, assetVar* av, const std::string& targetAvName, const std::string& result,
                            const std::string& logName, bool logToFile = true)
    {
        VarMapUtils* vm = av->am->vm;
        char* resultStr = nullptr;
        asprintf(&resultStr, "%s", result.c_str());

        if (!amap[targetAvName])
            amap[targetAvName] = vm->setLinkVal(vmap, aname, "/status", targetAvName.c_str(), resultStr);

        // Log FaultShutdown set to console and file
        char* essName               = vm->getSysName(vmap);
        const std::string sysCfgUri = fmt::format("/config/{}", essName);
        const auto logDir = getLogDir(vmap, *vm);
        const std::string fileName  = fmt::format("{}_{}_{}_{}", essName, aname, av->name, logName.empty() ? targetAvName : logName);
        const std::string dir       = fmt::format("{}/{}.txt", logDir, fileName);
        const std::string msg       = fmt::format("Setting /status/{}:{} for [{}] to {}", aname, targetAvName, av->getfName(), result);

        ESSLogger::get().info(msg.c_str());
        if (logToFile && getLoggingEnabled(vmap, *vm))
            ESSLogger::get().logIt(dir, getLoggingTimestamp(vmap, *vm));

        vm->setVal(vmap, amap[targetAvName]->comp.c_str(), amap[targetAvName]->name.c_str(), resultStr);

        if (resultStr)
            free((void*)resultStr);
    }

    /**
     * @brief Retrieves the assetVar that is in the param
     * 
     * @param vmap the global data map shared by all assets/asset managers
     * @param amap the local data map used by an asset/asset manager
     * @param av the assetVar that contains the command to run
     * @param param the parameter that contains the uri and name of the assetVar
     * 
     * @return the assetVar if the assetVar exists as defined in the param or null otherwise
     */
    inline assetVar* getAvFromParam(varsmap& vmap, varmap& amap, assetVar* av, const std::string& param)
    {
        FPS_PRINT_DEBUG("getting assetVar from {} param in [{}:{}]", param, av->comp, av->name);
        std::string avUri = "";
        if (av->getcParam(param.c_str()))
            avUri = fmt::format("{}", av->getcParam(param.c_str()));

        if (!avUri.empty())
        {
            // Check if we have a ":", which indicates a uri and variable name
            size_t idx = avUri.find(":");
            if (idx == std::string::npos && amap[avUri])
                return amap[avUri];

            // If we have found a ":", then we may have a uri and a variable name. Something like this: /component/asset_name:var_name
            else
            {
                VarMapUtils* vm = av->am->vm;
                assetVar* avParam = vm->getVar(vmap, avUri.c_str(), nullptr);
                if (avParam)
                    return avParam;
            }
        }
        return nullptr;
    }

    /**
     * @brief Checks if the condition(s) are satisfied, meaning if the evaluated expression
     * returns true
     * 
     * @param vmap the global data map shared by all assets/asset managers
     * @param amap the local data map used by an asset/asset manager
     * @param av the assetVar that contains the command to run
     * @param exprParam the expression parameter name
     * 
     * @return true if conditions are satisfied or false if the expression is invalid or conditions are not satisfied
     */
    inline bool checkConditions(varsmap& vmap, varmap& amap, assetVar* av, const char* exprParam)
    {
        const std::string& expr = CalculatorUtils::parseExpr(vmap, amap, av, exprParam);
        if (expr.empty())
        {
            FPS_PRINT_ERROR("expression [{}] is not valid for [{}:{}]", cstr{exprParam}, av->comp, av->name);
            return false;
        }
        else
        {
            Expression::Result result = CalculatorUtils::evaluateExpr(expr);
            if (result.type != Expression::DataType::BOOL)
            {
                FPS_PRINT_ERROR("expression [{}] result is not a boolean, which is not valid for [{}:{}]", cstr{exprParam}, av->comp, av->name);
                return false;
            }
            else
            {
                FPS_PRINT_DEBUG("Condition [{}] = {} for [{}:{}]", expr, result.bval, av->comp, av->name);
                return result.bval;
            }
        }
    }

    /**
     * @brief Creates and returns a string to use as the source name for events
     * 
     * @param vm the VarMapUtils instance
     * @param vmap the global data map shared by all assets/asset managers
     * @param aname the name of the asset/asset manager
     * @return the source name to include in events
     */
    inline std::string getEventSourceName(VarMapUtils* vm, varsmap& vmap, const char* aname)
    {
        char* essName = vm->getSysName(vmap);
        std::string source = fmt::format("{}->{}", essName, aname); // default to essName->assetName
        const std::string sysCfgUri = fmt::format("/config/{}", essName);
        assetVar* avEventSrc = vm->getVar(vmap, sysCfgUri.c_str(), "EventSourceFormat");
        if (avEventSrc)
        {
            const char* formatParam = avEventSrc->getcVal();
            if (formatParam)
            {
                FPS_PRINT_DEBUG("Got a format from {}, which is {}", avEventSrc->getfName(), formatParam);
                if (strcmp(formatParam, "full") == 0)
                {
                    // Use essName<sep>assetName as the source name, where <sep> is the separator
                    // Default to -> if sep is not defined
                    const char* sep = avEventSrc->getcParam("sep");
                    if (sep)
                    {
                        FPS_PRINT_DEBUG("Using {}{}{} as the source name", essName, sep, aname);
                        source = fmt::format("{}{}{}", essName, sep, aname);
                    }
                }
                else if (strcmp(formatParam, "essName") == 0)
                {
                    // Use essName as the source name
                    FPS_PRINT_DEBUG("Using essName {} as the source name", essName);
                    source = fmt::format("{}", essName);
                }
                else if (strcmp(formatParam, "assetName") == 0)
                {
                    // Use assetName as the source name
                    FPS_PRINT_DEBUG("Using assetName {} as the source name", aname);
                    source = fmt::format("{}", aname);
                }
            }
        }

        return source;
    }

    /**
     * @brief Record event to console, the events module, and log
     * 
     * @param vmap the global data map shared by all assets/asset managers
     * @param amap the local data map used by an asset/asset manager
     * @param aname the name of the asset/asset manager
     * @param av the command asset var
     * @param msg the event message
     * @param tNow the time the event occurred
     * @param reason the reason why the event is being recorded
     * @param severity the severity of the event
     * @param sendAlarm if true, then the event will be recorded as an alarm
     * @param fault true if the alert pertains to a fault event
     */
    inline void record(varsmap& vmap, varmap& amap, const char* aname, assetVar* av, const std::string& msg, double tNow
                        , const std::string& reason, Severity severity, bool sendAlarm = false, bool fault = false, bool logToFile = true)
    {
        VarMapUtils* vm = av->am->vm;

        const std::string source = getEventSourceName(vm, vmap, aname);
        av->sendEvent(source.c_str(), vm->p_fims, severity, msg.c_str());

        // Send alarm info to UI if enabled
        if (sendAlarm)
        {
            char* fltDest = amap["FaultDestination"]->getcVal();
            char* alrmDest = amap["AlarmDestination"]->getcVal();
            const std::string dest = fault ? fmt::format("{}", fltDest) : fmt::format("{}", alrmDest);
            vm->sendAlarm(vmap, av, dest.c_str(), nullptr, msg.c_str(), 2);
        }

        // Log event to console and file, depending on severity
        char* essName               = vm->getSysName(vmap);
        const std::string sysCfgUri = fmt::format("/config/{}", essName);
        const auto logDir = getLogDir(vmap, *vm);
        const std::string fileName  = fmt::format("{}_{}_{}_{}", essName, aname, av->name, reason);
        const std::string dir       = fmt::format("{}/{}.txt", logDir, fileName);

        switch (severity)
        {
        case Severity::Alarm :
            FPS_PRINT_WARN("{}", msg);
            ESSLogger::get().warn(msg.c_str());
            break;
        case Severity::Fault :
            FPS_PRINT_ERROR("{}", msg);
            ESSLogger::get().critical(msg.c_str());
            break;
        default :
            FPS_PRINT_INFO("{}", msg);
            ESSLogger::get().info(msg.c_str());
            break;
        }

        if (logToFile && getLoggingEnabled(vmap, *vm))
            ESSLogger::get().logIt(dir, getLoggingTimestamp(vmap, *vm));
    }

    /**
     * @brief Helper function for getting a list of assetVars
     * 
     * @param vmap the global data map shared by all assets/asset managers
     * @param amap the local data map used by an asset/asset manager
     * @param av the asset var to get list of assetVars for
     * @param operands the list of assetVars as operands
     * @return true if retrieval of assetVars is successful
     */
    inline bool getAvList(varsmap& vmap, varmap& amap, assetVar* av, std::vector<assetVar*>& operands)
    {
        VarMapUtils* vm = av->am->vm;

        // Add the assetVar parameters to the list of operands
        int numVars = av->getiParam("numVars");
        if (numVars > 0)
        {
            operands.reserve(numVars);

            // Add the assetVar parameters to the list of operands
            for (int i = 1; i <= numVars; i++)
            {
                const std::string varParam = fmt::format("variable{}", i);
                const char* avParamName = av->getcParam(varParam.c_str());
                if (!avParamName)
                {
                    FPS_PRINT_WARN("assetVar [{}] does not have param {} or param value is null", av->name, varParam);
                    return false;
                }
                const std::string avUri(avParamName);
                if (avUri.empty())
                {
                    FPS_PRINT_WARN("assetVar [{}] does not have parameter [{}]", av->name, varParam);
                    return false;
                }
                assetUri my(avUri.c_str());

                FPS_PRINT_DEBUG("assetVar [{}] has parameter [{}]. Attempting to adding to list of operands", av->name, varParam);
                assetVar* anotherAv;
                if (!my.Var)  // If the parameter contains just name, then check the amap for the variable
                {
                    if (!amap[avUri])
                    {
                        amap[avUri] = vm->getVar(vmap, avUri.c_str(), nullptr);
                    }
                    anotherAv = amap[avUri];
                }
                else  // The parameter contains uri:name, so check the vmap for the variable
                {
                    anotherAv = vm->getVar(vmap, avUri.c_str(), nullptr);
                }
                
                if (!anotherAv)
                {
                    FPS_PRINT_WARN("assetVar in parameter [{}] of assetVar [{}] does not exist", varParam, av->name);
                    return false;
                }

                FPS_PRINT_DEBUG("assetVar {} in parameter [{}] of assetVar [{}] exists. Adding val to list of operands", anotherAv->name, varParam, av->name);

                operands.emplace_back(anotherAv);
            }
        }
        return true;
    }
}

#endif