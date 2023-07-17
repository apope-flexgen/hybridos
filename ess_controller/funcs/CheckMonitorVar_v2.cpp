#ifndef CHECKMONITORVARV2_CPP
#define CHECKMONITORVARV2_CPP

#include "asset.h"
#include "formatters.hpp"
#include "ess_utils.hpp"

char* strtime(const struct tm *timeptr);

extern "C++"
{
    int CheckMonitorVar_v2(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
}

/**
 * @brief Initializes the parameters to be used for monitoring a variable
 * 
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param av the assetVar to initialize parameters for
 */
void setupMonitorParams(varsmap& vmap, varmap& amap, const char* aname, assetVar* av)
{
    if (!av->am)
    {
        FPS_PRINT_DEBUG("asset manager for [{}:{}] is null", av->comp, av->name);
        return;
    }
    FPS_PRINT_INFO("Setting up params for [{}:{}]", av->comp, av->name);

    // Different monitoring behavior will require different parameters
    if (!av->gotParam("enableComms"))
    {
        // Monitoring condition(s) to check
        if (!av->gotParam("alarmCondition"))
            av->setParam("alarmCondition", (char*)"n/a");
        if (!av->gotParam("faultCondition"))
            av->setParam("faultCondition", (char*)"n/a");
        if (!av->gotParam("resetCondition"))
            av->setParam("resetCondition", (char*)"n/a");

        // Set the option to include current value in assetVar to true by default
        if (!av->gotParam("includeCurrVal"))
            av->setParam("includeCurrVal", true);

        // Enable/disable certain monitoring behaviors
        if (!av->gotParam("enableMonitor"))
            av->setParam("enableMonitor", false);
        if (!av->gotParam("enableFault"))
            av->setParam("enableFault", true);

        // Set alarm timeout and state
        if (!av->gotParam("alarmTimeout"))
            av->setParam("alarmTimeout", 0);

        av->setParam("currAlarmTime", av->getdParam("alarmTimeout"));  
        av->setParam("seenAlarm", false);
    }

    // Amount of time to wait before sending fault or resetting
    if (!av->gotParam("faultTimeout"))
        av->setParam("faultTimeout", 0);
    if (!av->gotParam("resetTimeout"))
        av->setParam("resetTimeout", 0);

    // Set up remaining params              
    av->setParam("currFaultTime", av->getdParam("faultTimeout"));                
    av->setParam("currResetTime", av->getdParam("resetTimeout"));

    av->setParam("seenFault",false);                
    av->setParam("seenReset", false);

    if (!av->gotParam("enableAlert"))
        av->setParam("enableAlert", true);

    if (!av->gotParam("FaultShutdownReset")) 
        av->setParam("FaultShutdownReset", false);

    av->setParam("tLast", av->am->vm->get_time_dbl());
}

/**
 * @brief Checks communication state of the assetVar
 * 
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param av the assetVar to monitor
 */
void runCommsCheck(varsmap& vmap, varmap& amap, const char* aname, assetVar* av)
{
    FPS_PRINT_DEBUG("Running comms check for asset [{}] with [{}:{}]", cstr{aname}, av->comp, av->name);
    VarMapUtils* vm = av->am->vm;

    double tNow                = vm->get_time_dbl();
    double tDiff               = tNow - av->getdParam("tLast");
    const char* timestamp      = strtime(vm->get_local_time_now());
    double faultTimeout        = av->getdParam("faultTimeout");
    double resetTimeout        = av->getdParam("resetTimeout");
    double currFaultTime       = av->getdParam("currFaultTime");
    double currResetTime       = av->getdParam("currResetTime");
    bool enableComms           = av->getbParam("enableComms");
    bool enableAlert           = av->getbParam("enableAlert");
    bool seenFault             = av->getbParam("seenFault");
    bool seenReset             = av->getbParam("seenReset");
    bool faultConditionMet     = false;
    bool resetConditionMet     = false;

    av->setParam("tLast", tNow);

    // If comms monitoring is disabled, then reset alarm/fault/reset states and timers and then exit
    if (!enableComms)
    {
        av->setParam("currFaultTime", av->getdParam("faultTimeout"));                
        av->setParam("currResetTime", av->getdParam("resetTimeout"));

        av->setParam("seenFault", false);                
        av->setParam("seenReset", false);
        return;
    }

    // Check fault conditions
    switch (av->type)
    {
    case assetVar::ATypes::AINT:
    {
        FPS_PRINT_DEBUG("assetVar {} is int type", av->name);
        int val = av->getiVal();
        int lVal = av->getiLVal();
        faultConditionMet = (val == lVal);

        // Set the current assetVar's last value (lval). This will help with state checking
        av->setLVal(val);

        break;
    }
    case assetVar::ATypes::AFLOAT:
    {
        FPS_PRINT_DEBUG("assetVar {} is float type", av->name);
        double val = av->getdVal();
        double lVal = av->getdLVal();
        faultConditionMet = (val == lVal);
        
        // Set the current assetVar's last value (lval). This will help with state checking
        av->setLVal(val);

        break;
    }
    case assetVar::ATypes::ASTRING:
    {
        FPS_PRINT_DEBUG("operand val {} is string type", operand.sval);
            
        char* val = av->getcVal();
        char* lVal = av->getcLVal();
        faultConditionMet = (strcmp(val, lVal) == 0);
        
        // Set the current assetVar's last value (lval). This will help with state checking
        av->setLVal(val);
    }
    default:
        FPS_PRINT_ERROR("assetVar {} does not have type supported for communications check. Supported types are: AINT, AFLOAT, ASTRING", av->name);
        return;
    }
    resetConditionMet = !faultConditionMet;

    // Check if communication fault condition (value has not changed) has been met
    if (faultConditionMet)
    {
        FPS_PRINT_DEBUG("Fault condition is satisfied for asset [{}] with [{}:{}]", cstr{aname}, av->comp, av->name);

        // Decrement current fault time
        if (currFaultTime > 0)
            ESSUtils::decrementTime(av, currFaultTime, tDiff, "currFaultTime");

        // Set monitor var to fault state
        if(currFaultTime <= 0 && !seenFault)
        {
            av->setParam("seenFault", true);
            if (seenReset) 
                av->setParam("seenReset", false);
            av->setParam("currResetTime", resetTimeout);

            // Send alarm/event and log info if enabled
            if (enableAlert)
            {
                // Prepare fault msg to send
                const std::string msg = fmt::format("[{}] Lost communication to [{}] for {:.2f} second(s) at {}"
                    , av->name
                    , cstr{aname}
                    , faultTimeout
                    , timestamp);

                ESSUtils::record(vmap, amap, aname, av, msg, tNow, "MonitorFault", Severity::Fault, true, true, false);

                // Increment # of faults
                amap["FaultCnt"]->addVal(1);
            }

            // Proceed to shutdown due to fault here
            ESSUtils::setAvStatus(vmap, amap, aname, av, "FaultShutdown", true, "FaultShutdown", true);

            // If enableComms param is set to true, then set CommsOK to false
            ESSUtils::setAvStatus(vmap, amap, aname, av, "CommsOK", false, "CommsLoss", true);

            // Set MonitorVarFault here so that can be used elsewhere in the source code
            ESSUtils::setAvStatus(vmap, amap, aname, av, "MonitorVarFault", av->name, "MonitorVarFault", false);
        }
    }

    // Check if communication reset condition (value has changed) has been met
    if (resetConditionMet)
    {
        FPS_PRINT_DEBUG("Reset condition is satisfied for asset [{}] with [{}:{}]", cstr{aname}, av->comp, av->name);

        // Decrement current reset time
        if (currResetTime > 0)
            ESSUtils::decrementTime(av, currResetTime, tDiff, "currResetTime");

        // Reset current state of monitor var
        if(currResetTime <= 0 && !seenReset)
        {
            av->setParam("seenReset", true);

            // Reset fault state
            if (seenFault) 
            {
                av->setParam("seenFault", false);
                // Decrement # of faults if alert is enabled
                if (enableAlert)
                {
                    int cnt = amap["FaultCnt"]->getiVal() - 1;
                    amap["FaultCnt"]->setVal(cnt > 0 ? cnt : 0);
                }
            }

            // Reset fault shutdown. This indicates system is good to run
            ESSUtils::setAvStatus(vmap, amap, aname, av, "FaultShutdown", false, "FaultShutdown", false);

            // If enableComms param is set to true, then set CommsOK to true
            ESSUtils::setAvStatus(vmap, amap, aname, av, "CommsOK", true, "CommsOK", false);
        }

        // Reset current fault time
        if (currFaultTime < faultTimeout)
            av->setParam("currFaultTime", faultTimeout);
    }
}

/**
 * @brief Checks if the current value of the assetVar satisfies the alarm/fault conditions.
 *
 * If the alarm/fault conditions are satisfied, then an alarm/fault state will be created.
 * If the reset condition is satisfied, then the alarm/fault state is reset.
 * 
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param av the assetVar to monitor
 */
void runMonitor(varsmap& vmap, varmap& amap, const char* aname, assetVar* av)
{
    FPS_PRINT_DEBUG("Running monitor check for asset [{}] with [{}:{}]", cstr{aname}, av->comp, av->name);
    VarMapUtils* vm = av->am->vm;

    double tNow                = vm->get_time_dbl();
    double tDiff               = tNow - av->getdParam("tLast");
    const char* timestamp      = strtime(vm->get_local_time_now());
    double alarmTimeout        = av->getdParam("alarmTimeout");
    double faultTimeout        = av->getdParam("faultTimeout");
    double resetTimeout        = av->getdParam("resetTimeout");
    double currAlarmTime       = av->getdParam("currAlarmTime");
    double currFaultTime       = av->getdParam("currFaultTime");
    double currResetTime       = av->getdParam("currResetTime");
    bool enableMonitor         = av->getbParam("enableMonitor");
    bool enableFault           = av->getbParam("enableFault");
    bool enableAlert           = av->getbParam("enableAlert");
    bool seenAlarm             = av->getbParam("seenAlarm");
    bool seenFault             = av->getbParam("seenFault");
    bool seenReset             = av->getbParam("seenReset");
    bool alarmConditionMet     = false;
    bool faultConditionMet     = false;
    bool resetConditionMet     = false;

    av->setParam("tLast", tNow);

    // If monitoring is disabled, then reset alarm/fault/reset states and timers and then exit
    if (!enableMonitor)
    {
        av->setParam("currAlarmTime", av->getdParam("alarmTimeout"));  
        av->setParam("currFaultTime", av->getdParam("faultTimeout"));                
        av->setParam("currResetTime", av->getdParam("resetTimeout"));

        av->setParam("seenAlarm", false);
        av->setParam("seenFault", false);                
        av->setParam("seenReset", false);
        return;
    }

    // If fault check is disabled, set FaultShutdown back to false
    if (!enableFault)
    {
        if (!av->getbParam("FaultShutdownReset"))
        {
            ESSUtils::setAvStatus(vmap, amap, aname, av, "FaultShutdown", false, "FaultShutdown", false);
            av->setParam("FaultShutdownReset", true);
        }
    }
    else
    {
        if (av->getbParam("FaultShutdownReset"))
            av->setParam("FaultShutdownReset", false);
    }

    // Check alarm/fault conditions
    faultConditionMet = ESSUtils::checkConditions(vmap, amap, av, "faultCondition");
    alarmConditionMet = ESSUtils::checkConditions(vmap, amap, av, "alarmCondition");
    resetConditionMet = ESSUtils::checkConditions(vmap, amap, av, "resetCondition");

    // Check if fault condition(s) are satisfied
    if (enableFault)
    {
        if (faultConditionMet)
        {
            FPS_PRINT_DEBUG("Fault condition is satisfied for asset [{}] with [{}:{}]", cstr{aname}, av->comp, av->name);

            // Decrement current fault time
            if (currFaultTime > 0)
                ESSUtils::decrementTime(av, currFaultTime, tDiff, "currFaultTime");

            // Set monitor var to fault state
            if(currFaultTime <= 0 && !seenFault)
            {
                av->setParam("seenFault", true);
                if (seenReset) 
                    av->setParam("seenReset", false);
                av->setParam("currResetTime", resetTimeout);

                // Send alarm/event and log info if enabled
                if (enableAlert)
                {
                    // Prepare fault msg to send
                    const std::string msg = fmt::format("[{}] Monitoring condition(s) [{}] met for {:.2f} second(s) at {}"
                        , av->name
                        , CalculatorUtils::getExpressionInfo(vmap, amap, av, "faultCondition")
                        , faultTimeout
                        , timestamp);

                    ESSUtils::record(vmap, amap, aname, av, msg, tNow, "MonitorFault", Severity::Fault, true, true, false);

                    // Increment # of faults
                    amap["FaultCnt"]->addVal(1);
                }

                // Proceed to shutdown due to fault here
                ESSUtils::setAvStatus(vmap, amap, aname, av, "FaultShutdown", true, "FaultShutdown", true);

                // Set MonitorVarFault here so that can be used elsewhere in the source code
                ESSUtils::setAvStatus(vmap, amap, aname, av, "MonitorVarFault", av->name, "MonitorVarFault", false);
            }
        }
    }

    // Check if alarm condition(s) are satisfied
    if (alarmConditionMet)
    {
        FPS_PRINT_DEBUG("Alarm condition is satisfied for asset [{}] with [{}:{}]", cstr{aname}, av->comp, av->name);

        // Decrement current alarm time
        if (currAlarmTime > 0)
            ESSUtils::decrementTime(av, currAlarmTime, tDiff, "currAlarmTime");

        // Set monitor var to alarm state
        if(currAlarmTime <= 0 && !seenAlarm)
        {
            av->setParam("seenAlarm", true);
            if (seenReset)
                av->setParam("seenReset", false);
            av->setParam("currResetTime", resetTimeout);

            // Send alarm/event and log info if enabled
            if (enableAlert)
            {
                // Prepare alarm msg to send out
                const std::string msg = fmt::format("[{}] Monitoring condition(s) [{}] met for {:.2f} second(s) at {}"
                        , av->name
                        , CalculatorUtils::getExpressionInfo(vmap, amap, av, "alarmCondition")
                        , faultTimeout
                        , timestamp);

                ESSUtils::record(vmap, amap, aname, av, msg, tNow, "MonitorAlarm", Severity::Alarm, true, false, false);

                // Increment # of alarms
                amap["AlarmCnt"]->addVal(1);
            }

            // Set MonitorVarAlarm here so that can be used elsewhere in the source code
            ESSUtils::setAvStatus(vmap, amap, aname, av, "MonitorVarAlarm", av->name, "MonitorVarAlarm", false);
        }
    }

    // Check if reset condition(s) are satisfied
    if (resetConditionMet)
    {
        FPS_PRINT_DEBUG("Reset condition is satisfied for asset [{}] with [{}:{}]", cstr{aname}, av->comp, av->name);

        // Decrement current reset time
        if (currResetTime > 0)
            ESSUtils::decrementTime(av, currResetTime, tDiff, "currResetTime");

        // Reset current state of monitor var
        if(currResetTime <= 0 && !seenReset)
        {
            av->setParam("seenReset", true);

            // Reset fault state
            if (seenFault) 
            {
                av->setParam("seenFault", false);
                // Decrement # of faults if alert is enabled
                if (enableAlert)
                {
                    int cnt = amap["FaultCnt"]->getiVal() - 1;
                    amap["FaultCnt"]->setVal(cnt > 0 ? cnt : 0);
                }
            }

            // Reset alarm state
            if (seenAlarm) 
            {
                av->setParam("seenAlarm", false);
                // Decrement # of alarms if alert is enabled
                if (enableAlert)
                {
                    int cnt = amap["AlarmCnt"]->getiVal() - 1;
                    amap["AlarmCnt"]->setVal(cnt > 0 ? cnt : 0);
                }
            }

            // Reset fault shutdown. This indicates system is good to run
            ESSUtils::setAvStatus(vmap, amap, aname, av, "FaultShutdown", false, "FaultShutdown", false);
        }

        // Increment both the current alarm/fault time
        if (currAlarmTime < alarmTimeout)
            ESSUtils::incrementTime(av, currAlarmTime, tDiff, alarmTimeout, "currAlarmTime");

        if (currFaultTime < faultTimeout)
            ESSUtils::incrementTime(av, currFaultTime, tDiff, faultTimeout, "currFaultTime");
    }
}

/**
 * @brief Checks the monitoring variable, depending on defined alarm/fault conditions
 * 
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param p_fims the interface used to send data to
 * @param av the assetVar to monitor
 */
int CheckMonitorVar_v2(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    FPS_PRINT_DEBUG("assetVar [{}:{}]  aname [{}]   val [{}]", av->comp, av->name, cstr{aname}, av->getdVal());//

	VarMapUtils* vm = av->am->vm;
    const std::string reloadStr = "CheckMonitorVar_v2_" + av->name;
    int reload = vm->CheckReload(vmap, amap, aname, reloadStr.c_str());
    if (reload < 2)
    {
        FPS_PRINT_INFO("{}  reload first run [{}] [{}:{}]  reload {}", cstr{aname}, av->comp, av->name, reload);
        setupMonitorParams(vmap, amap, aname, av);

        int ival = 0;
        char* cval = (char*)"Normal";

        // Default fault/alarm destinations should look like /assets/aname/summary:[faults | alarms]
        char* fltDest = nullptr;
        char* alrmDest = nullptr;
        asprintf(&fltDest,  "/assets/%s/summary:faults", aname);
        asprintf(&alrmDest, "/assets/%s/summary:alarms", aname);

        amap["FaultCnt"]              = vm->setLinkVal(vmap, aname,  "/status",     "FaultCnt",              ival);
        amap["AlarmCnt"]              = vm->setLinkVal(vmap, aname,  "/status",     "AlarmCnt",              ival);
        amap["FaultDestination"]      = vm->setLinkVal(vmap, aname,  "/config",     "FaultDestination",      fltDest);
        amap["AlarmDestination"]      = vm->setLinkVal(vmap, aname,  "/config",     "AlarmDestination",      alrmDest);
        amap["MonitorVarAlarm"]       = vm->setLinkVal(vmap, aname,  "/alarms",     "MonitorVarAlarm",       cval);
        amap["MonitorVarFault"]       = vm->setLinkVal(vmap, aname,  "/faults",     "MonitorVarFault",       cval);

        if (!amap["FaultDestination"]->getcVal())
            amap["FaultDestination"]->setVal(fltDest);
        if (!amap["AlarmDestination"]->getcVal())
            amap["AlarmDestination"]->setVal(alrmDest);
        if (!amap["MonitorVarAlarm"]->getcVal())
            amap["MonitorVarAlarm"]->setVal(cval);
        if (!amap["MonitorVarFault"]->getcVal())
            amap["MonitorVarFault"]->setVal(cval);

        if (fltDest)
            free((void*)fltDest);
        if (alrmDest)
            free((void*)alrmDest);

        FPS_PRINT_DEBUG("Fault Destination for {} with assetVar {} is {}", aname, av->name, cstr{amap["FaultDestination"]->getcVal()});
        FPS_PRINT_DEBUG("Alarm Destination for {} with assetVar {} is {}", aname, av->name, cstr{amap["AlarmDestination"]->getcVal()});

        if (reload < 1)
        {
            FPS_PRINT_INFO("Adding [{}] to amap in asset [{}]", av->getfName(), cstr{aname});
            amap[av->name] = vm->setLinkVal(vmap, aname, "/status", av->name.c_str(), av);
        }
        reload = 2;
        amap[reloadStr] = vm->setLinkVal(vmap, aname, "/reload", reloadStr.c_str(), reload);
        amap[reloadStr]->setVal(reload);
    }

    // Run monitoring function
    if (av->gotParam("enableMonitor"))
        runMonitor(vmap, amap, aname, av);
    else if (av->gotParam("enableComms"))
        runCommsCheck(vmap, amap, aname, av);

    return 0;
}

#endif