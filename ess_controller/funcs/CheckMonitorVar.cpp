#ifndef CHECKMONITORVAR_CPP
#define CHECKMONITORVAR_CPP

#include "asset.h"
#include "formatters.hpp"
#include "ess_utils.hpp"

char* strtime(const struct tm *timeptr);

extern "C++"
{
    int UpdateSysTime(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar*Av);
    int CheckMonitorVar(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av);
}

// Namespace for functions dealing with time adjustments for alarm, fault, and recovery states
namespace MonitorVarUtils
{

    /**
     * @brief Send an alarm/event and log information to a file
     * 
     * @param vmap the global data map shared by all assets/asset managers
     * @param amap the local data map used by an asset/asset manager
     * @param aname the name of the asset/asset manager
     * @param av the asset var to monitor
     * @param msg the alarm/fault message
     * @param fault true if the alert pertains to a fault event
     */
    void alert(varsmap& vmap, varmap& amap, const char* aname, assetVar* av, const std::string& msg, bool fault)
    {
        FPS_DEBUG_PRINT("%s >> av %s  msg %s\n", __func__, av->name.c_str(), msg.c_str());
        VarMapUtils* vm = av->am->vm;
        char* fltDest = amap["FaultDestination"]->getcVal();
        char* alrmDest = amap["AlarmDestination"]->getcVal();
        const std::string dest = fault ? fmt::format("{}", fltDest) : fmt::format("{}", alrmDest);

        // Send alarm info to UI
        vm->sendAlarm(vmap, av, dest.c_str(), nullptr, msg.c_str(), 2);

        // Send alarm info to log file
        const auto now = flex::get_time_dbl(); // might need to use this instead of direct calls to ensure times are the same.
        if (fault) 
        {
            // Send fault info to log file
            ESSLogger::get().critical("{}", msg);

            // Send fault info to events
            const std::string source = ESSUtils::getEventSourceName(vm, vmap, aname);
            av->sendEvent(source.c_str(), vm->p_fims, Severity::Fault, msg.c_str(), av->name.c_str(), now.count());
            FPS_DEBUG_PRINT("%s >> Fault Sent dest [%s] msg [%s]  am %p \n", __func__, dest.c_str(), msg.c_str(), (void*)av->am);
        }
        else
        {
            // Send alarm info to log file
            ESSLogger::get().warn("{}", msg);

            // Send alarm info to events
            const std::string source = ESSUtils::getEventSourceName(vm, vmap, aname);
            av->sendEvent(source.c_str(), vm->p_fims, Severity::Alarm, msg.c_str(), av->name.c_str(), now.count());
            FPS_DEBUG_PRINT("%s >> Alarm Sent dest [%s] msg [%s]  am %p \n", __func__, dest.c_str(), msg.c_str(), (void*)av->am);
        }
    }
}

/**
 * @brief Assigns limit parameters for the assetVar to monitor
 * 
 * @param aname the name of the asset/asset manager
 * @param av the assetVar to monitor
 */
void SetupLimitParams(const char* aname, assetVar* av)
{
    FPS_DEBUG_PRINT("%s >>  setup for  %s  %s\n", __func__, aname, av->name.c_str());              

    if (!av->gotParam("EnableFaultCheck"))  av->setParam("EnableFaultCheck", false);
    if (!av->gotParam("EnableMaxValCheck")) av->setParam("EnableMaxValCheck", false);       
    if (!av->gotParam("EnableMinValCheck")) av->setParam("EnableMinValCheck", false);
    if (!av->gotParam("MaxAlarmThreshold")) av->setParam("MaxAlarmThreshold", 0.0);
    if (!av->gotParam("MaxFaultThreshold")) av->setParam("MaxFaultThreshold", 0.0);
    if (!av->gotParam("MaxResetValue"))     av->setParam("MaxResetValue", 0.0);
    if (!av->gotParam("MinAlarmThreshold")) av->setParam("MinAlarmThreshold", 0.0);
    if (!av->gotParam("MinFaultThreshold")) av->setParam("MinFaultThreshold", 0.0);
    if (!av->gotParam("MinResetValue"))     av->setParam("MinResetValue", 0.0);
    if (!av->gotParam("MaxAlarmTimeout"))   av->setParam("MaxAlarmTimeout", 0.0);
    if (!av->gotParam("MaxFaultTimeout"))   av->setParam("MaxFaultTimeout", 0.0);
    if (!av->gotParam("MaxRecoverTimeout")) av->setParam("MaxRecoverTimeout", 0.0);
    if (!av->gotParam("MinAlarmTimeout"))   av->setParam("MinAlarmTimeout", 0.0);
    if (!av->gotParam("MinFaultTimeout"))   av->setParam("MinFaultTimeout", 0.0);
    if (!av->gotParam("MinRecoverTimeout")) av->setParam("MinRecoverTimeout", 0.0);

    av->setParam("MaxAlarmTime", av->getdParam("MaxAlarmTimeout"));                
    av->setParam("MaxFaultTime", av->getdParam("MaxFaultTimeout"));                
    av->setParam("MaxRecoverTime", av->getdParam("MaxRecoverTimeout"));
    av->setParam("MinAlarmTime", av->getdParam("MinAlarmTimeout"));                
    av->setParam("MinFaultTime", av->getdParam("MinFaultTimeout"));                
    av->setParam("MinRecoverTime", av->getdParam("MinRecoverTimeout"));    

    av->setParam("seenMaxFault",false);                
    av->setParam("seenMinFault",false);
    av->setParam("seenMaxAlarm", false);
    av->setParam("seenMinAlarm", false);
    av->setParam("seenMaxReset", false);
    av->setParam("seenMinReset", false);
}

/**
 * @brief Assigns state parameters for the assetVar to monitor
 * 
 * @param aname the name of the asset/asset manager
 * @param av the assetVar to monitor
 */
void SetupStateParams(const char* aname, assetVar* av)
{
    FPS_DEBUG_PRINT("%s >>  setup start for  %s  %s\n", __func__, aname?aname:"no Aname", av->getfName());              

    if (!av->gotParam("AlarmTimeout"))    av->setParam("AlarmTimeout", 0.0);
    if (!av->gotParam("FaultTimeout"))    av->setParam("FaultTimeout", 0.0);
    if (!av->gotParam("RecoverTimeout"))  av->setParam("RecoverTimeout", 0.0);
    if (!av->gotParam("Type"))            av->setParam("Type", (char*)"int");
    if (!av->gotParam("lastVal"))
    {
        if (!strcmp(av->getcParam("Type"), "string")) av->setParam("lastVal", (char*)"Init val");
        else                                          av->setParam("lastVal", 0);
    }
    av->setParam("AlarmTime", av->getdParam("AlarmTimeout"));                
    av->setParam("FaultTime", av->getdParam("FaultTimeout"));                
    av->setParam("RecoverTime", av->getdParam("RecoverTimeout"));            
    av->setParam("seenFault",false);                
    av->setParam("seenAlarm", false);
    av->setParam("seenReset", false);
}

/**
 * @brief Assigns condition parameters for the assetVar to monitor
 * 
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param av the assetVar to monitor
 */
void SetupConditionParams(varsmap& vmap, varmap& amap, const char* aname, assetVar* av)
{
    if (!av->am)
    {
        FPS_DEBUG_PRINT("%s >> asset manager for %s is null. Skipping monitor check.\n", __func__, av->name.c_str());
        return;
    }
    VarMapUtils* vm = av->am->vm;
    FPS_DEBUG_PRINT("%s >>  setup for  %s  %s\n", __func__, aname, av->name.c_str());              

    if (!av->gotParam("AlarmTimeout"))        av->setParam("AlarmTimeout", 0.0);
    if (!av->gotParam("FaultTimeout"))        av->setParam("FaultTimeout", 0.0);
    if (!av->gotParam("RecoverTimeout"))      av->setParam("RecoverTimeout", 0.0);
    if (!av->gotParam("Type"))                av->setParam("Type", (char*)"int");

    if (!av->gotParam("numConditionVars"))    av->setParam("numConditionVars", 0);
    for (int i = 1; i <= av->getiParam("numConditionVars"); i++)
    {
        char* varParam = nullptr;
        asprintf(&varParam, "conditionVar%d", i);
        char* avName = av->getcParam(varParam);
        if(avName)
        {
            double dval = 0.0;
            if (av->gotParam(varParam) && av->getcParam(varParam) && !amap[avName])
            {
                FPS_DEBUG_PRINT("%s >> Creating assetVar [%s] for %s in %s\n", __func__, avName, varParam, aname);
                amap[avName] = vm->setLinkVal(vmap, aname, "/status", avName, dval);
            }
        }
        if (varParam) free(varParam);

        // Check if the condition variable has a condition type
        char* condTypeParam = nullptr;
        asprintf(&condTypeParam, "conditionType%d", i);
        char* condType = av->getcParam(condTypeParam);
        if (!condType) av->setParam(condTypeParam, (char*)"int");
        if (condTypeParam) free(condTypeParam);
    }

    av->setParam("AlarmTime", av->getdParam("AlarmTimeout"));                
    av->setParam("FaultTime", av->getdParam("FaultTimeout"));                
    av->setParam("RecoverTime", av->getdParam("RecoverTimeout"));            
    av->setParam("seenFault",false);                
    av->setParam("seenAlarm", false);
    av->setParam("seenReset", false);
}

/**
 * @brief Sets up the assetVar for the ess controller to monitor
 * Assigns either state, condition, or limit parameters for the assetVar
 * 
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param av the assetVar we're monitoring
 */
void SetupMonitorVar(varsmap& vmap, varmap& amap, const char* aname, assetVar* av)
{
    if(!av) return;

    if (av->gotParam("EnableStateCheck"))
    {
        FPS_DEBUG_PRINT("%s >> Setting state params for %s\n", __func__, av->getfName());
        SetupStateParams(aname, av);
    }
    else if (av->gotParam("EnableConditionCheck"))
    {
        FPS_DEBUG_PRINT("%s >> Setting condition params for %s\n", __func__, av->name.c_str());
        SetupConditionParams(vmap, amap, aname, av);
    }
    else
    {
        FPS_DEBUG_PRINT("%s >> Setting limit params for %s\n", __func__, av->getfName());
        SetupLimitParams(aname, av);
    }

    // Set up remaining params
    double tNow = av->am->vm->get_time_dbl();
    av->setParam("tLast",tNow);
    if (!av->gotParam("EnableAlert")) av->setParam("EnableAlert", true);

}

/**
 * @brief Check if the assetVar is changing. Can be used to check comms between
 * the ess controller and external devices (ex.: bms, pcs) using either
 * Heartbeat or Timestamp variables
 * 
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param av the asset var to monitor
 */
template <typename T>
void CheckState(varsmap &vmap, varmap& amap, const char* aname, assetVar* av, T& val, T& lastVal)
{
    FPS_DEBUG_PRINT("%s >> checking state for %s with assetVar %s\n", __func__, aname, av->name.c_str());
    VarMapUtils* vm = av->am->vm;
    
    double tNow = vm->get_time_dbl();
    const char* type = av->getcParam("Type");

    double AlarmTimeCfg = av->getdParam("AlarmTimeout");
    double FaultTimeCfg = av->getdParam("FaultTimeout");
    double RecoverTimeCfg = av->getdParam("RecoverTimeout");

    bool enableStateCheck = av->getbParam("EnableStateCheck");
    // Check whether to use alarms/events/logs here
    bool enableAlert = av->getbParam("EnableAlert");

    // Grab remaining values for the variable (not defined in config)
    double currAlarmTime = av->getdParam("AlarmTime");                
    double currFaultTime = av->getdParam("FaultTime");                
    double currRecoverTime = av->getdParam("RecoverTime");

    bool seenFault = av->getbParam("seenFault");
    bool seenAlarm = av->getbParam("seenAlarm");
    bool seenReset = av->getbParam("seenReset");

    double tLast = av->getdParam("tLast");
    double tDiff = tNow - tLast;
    av->setParam("tLast", tNow);
    int debug = av->getiParam("debug");

    if (!strcmp(type, "int"))
    {
        if (debug > 0 && enableStateCheck) FPS_PRINT_INFO("{} Value [{}] lastVal [{}]  Fault Time [{:.4f}] Alarm Time [{:.4f}] Recovery Time [{:.4f}]"
                                            , av->name
                                            , av->getiVal()
                                            , av->getiParam("lastVal")
                                            , currFaultTime
                                            , currAlarmTime
                                            , currRecoverTime
                                        );
        if (debug > 0) av->setParam("debug", debug - 1);
    }
    else if (!strcmp(type, "double"))
    {
        if (debug > 0 && enableStateCheck)  FPS_PRINT_INFO("{}  Value [{:.4f}] lastVal [{:.4f}]  Fault Time [{:.4f}] Alarm Time [{:.4f}] Recovery Time [{:.4f}]"
                                            , av->name
                                            , av->getdVal()
                                            , av->getdParam("lastVal")
                                            , currFaultTime
                                            , currAlarmTime
                                            , currRecoverTime
                                        );
        if (debug > 0) av->setParam("debug", debug - 1);
    }
    else if (!strcmp(type, "string")) 
    {
        if (debug > 0 && enableStateCheck)  FPS_PRINT_INFO("{} Value [{}] lastVal [{}] Fault Time [{:.4f}] Alarm Time [{:.4f}] Recovery Time [{:.4f}]"
                                            , av->name
                                            , cstr{av->getcVal()}
                                            , cstr{av->getcParam("lastVal")}
                                            , currFaultTime
                                            , currAlarmTime
                                            , currRecoverTime
                                        );
        if (debug > 0) av->setParam("debug", debug - 1);
    }

    tm *local_tm = vm->get_local_time_now();
    const char* timestamp = strtime(local_tm);

    if (!enableStateCheck)
    {

        // Reset alarm/fault/reset states and timers
        av->setParam("AlarmTime", av->getdParam("AlarmTimeout"));                
        av->setParam("FaultTime", av->getdParam("FaultTimeout"));                
        av->setParam("RecoverTime", av->getdParam("RecoverTimeout"));            
        av->setParam("seenFault", false);                
        av->setParam("seenAlarm", false);
        av->setParam("seenReset", false);
        return;
    }
    
    // Check if the val is changing
    if (val == lastVal)
    {
        if (currFaultTime > 0) ESSUtils::decrementTime(av, currFaultTime, tDiff, "FaultTime");
        if (currAlarmTime > 0) ESSUtils::decrementTime(av, currAlarmTime, tDiff, "AlarmTime");

        // Set monitor var to fault state
        if (currFaultTime <= 0.0 && !seenFault)
        {
            FPS_DEBUG_PRINT("%s >> Seen fault for %s\n", __func__, av->name.c_str());
            av->setParam("seenFault", true);
            if (seenReset) av->setParam("seenReset", false);
            av->setParam("RecoverTime", RecoverTimeCfg);

            // Send alarm/event and log to file if enabled
            if (enableAlert)
            {
                // Prepare fault msg to send
                std::string msg;
                if (!strcmp(type, "int"))         msg = fmt::format("[{}] value [{}] is not changing for {:.2f} seconds at {}"
                                                            , av->name
                                                            , av->getiVal()
                                                            , FaultTimeCfg
                                                            , timestamp);
                else if (!strcmp(type, "double")) msg = fmt::format("[{}] value [{}] is not changing for {:.2f} seconds at {}"
                                                            , av->name
                                                            , av->getdVal()
                                                            , FaultTimeCfg
                                                            , timestamp);
                else if (!strcmp(type, "string")) msg = fmt::format("[{}] value [{}] is not changing for {:.2f} seconds at {}"
                                                            , av->name
                                                            , av->getcVal()
                                                            , FaultTimeCfg
                                                            , timestamp);

                MonitorVarUtils::alert(vmap, amap, aname, av, msg, true);

                // Increment # of faults
                amap["FaultCnt"]->addVal(1);
            }

            // Proceed to shutdown due to fault here
            ESSUtils::setAvStatus(vmap, amap, aname, av, "FaultShutdown", true, "FaultShutdown", true);

            // Set MonitorVarFault here so that can be used elsewhere in the source code
            std::string val = av->name + "_state";
            const char* monFltAvVal = val.c_str();
            vm->setVal(vmap, amap["MonitorVarFault"]->comp.c_str(), amap["MonitorVarFault"]->name.c_str(), monFltAvVal);

            // If we got EnableCommsCheck as a param, and if that's set to true, then set CommsOK to false
            if (av->gotParam("EnableCommsCheck") && av->getbParam("EnableCommsCheck"))
            {
                ESSUtils::setAvStatus(vmap, amap, aname, av, "CommsOK", false, "CommsLost", true);
            }
        }
            
        // Set monitor var to alarm state
        if (currAlarmTime <= 0.0 && !seenAlarm)
        {
            av->setParam("seenAlarm", true);
            if (seenReset) av->setParam("seenReset", false);
            av->setParam("RecoverTime", RecoverTimeCfg);

            // Send alarm/event and log to file if enabled
            if (enableAlert)
            {
                // Prepare alarm msg to send
                std::string msg;
                if (!strcmp(type, "int"))         msg = fmt::format("[{}] value [{}] is not changing for {:.2f} seconds at {}"
                                                            , av->name.c_str()
                                                            , av->getiVal()
                                                            , FaultTimeCfg
                                                            , timestamp);
                else if (!strcmp(type, "double")) msg = fmt::format("[{}] value [{}] is not changing for {:.2f} seconds at {}"
                                                            , av->name.c_str()
                                                            , av->getdVal()
                                                            , FaultTimeCfg
                                                            , timestamp);
                else if (!strcmp(type, "string")) msg = fmt::format("[{}] value [{}] is not changing for {:.2f} seconds at {}"
                                                            , av->name.c_str()
                                                            , av->getcVal()
                                                            , FaultTimeCfg
                                                            , timestamp);

                MonitorVarUtils::alert(vmap, amap, aname, av, msg, false);
                
                // Increment # of alarms
                amap["AlarmCnt"]->addVal(1);
            }

            // Set MonitorVarAlarm here so that can be used elsewhere in the source code
            std::string val = av->name + "_state";
            const char* monAlrmAvVal = val.c_str();
            vm->setVal(vmap, amap["MonitorVarAlarm"]->comp.c_str(), amap["MonitorVarAlarm"]->name.c_str(), monAlrmAvVal);
        }
    }
    else
    {
        if (!strcmp(type, "int"))         av->setParam("lastVal", av->getiVal());
        else if (!strcmp(type, "double")) av->setParam("lastVal", av->getdVal());
        else if (!strcmp(type, "string")) av->setParam("lastVal", av->getcVal());

        if (currRecoverTime > 0.0) ESSUtils::decrementTime(av, currRecoverTime, tDiff, "RecoverTime");
        if (currRecoverTime <= 0.0 && !seenReset)
        {
            av->setParam("seenReset", true);
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

            // Reset fault shutdown -> this indicates system is good to run
            ESSUtils::setAvStatus(vmap, amap, aname, av, "FaultShutdown", false, "FaultShutdown", false);

            // If we got EnableCommsCheck as a param, and if that's set to true, then set CommsOK to true
            if (av->gotParam("EnableCommsCheck") && av->getbParam("EnableCommsCheck"))
            {
                ESSUtils::setAvStatus(vmap, amap, aname, av, "CommsOK", true, "CommsOK", false);
            }
        }

        // If the value has changed, reset the current alarm/fault time back to the configurable time
        if (currAlarmTime < AlarmTimeCfg) av->setParam("AlarmTime", AlarmTimeCfg);
        if (currFaultTime < FaultTimeCfg) av->setParam("FaultTime", FaultTimeCfg);
    }
}

// Namespace that contains helper functions for CheckCondition function
namespace Condition
{
    /**
     * @brief Helper function for findExpectedVal and findConditionVal to compare the current value to the expected value(s)
     * 
     * @param val the value to check against the expected value(s)
     * @param expectedVals the collection of expected values used for comparison
     * @return true if the value is equal to at least one expected value
     */
    template <typename T>
    bool compareVal(T& val, std::vector<T>& expectedVals)
    {
        bool equalExpected = false;
        // Check if the val is equal to the expected value
        for (const auto& expectedVal : expectedVals)
        {
            if (val == expectedVal) equalExpected = true;
        }

        return equalExpected;
    }

    /**
     * @brief Helper function for CheckCondition function to determine if the current value is
     * equal to the expected value(s)
     * 
     * @param av the assetVar to check expected values for
     * @return true if the value is equal to at least one expected value, the numExpectedVals is 0, or the expectedVal parameter(s) do not exist
     */
    bool findExpectedVal(assetVar* av)
    {
        // If we do not have the correct parameters initialized before the check, immediately exit this function
        // No need to perform alarm/fault check
        if (!av->gotParam("numExpectedVals") || av->getiParam("numExpectedVals") <= 0)
        {
            FPS_DEBUG_PRINT("%s >> av [%s] numExpectedVals either does not exist or is <= 0. Skipping condition check\n", __func__, av->name.c_str());
            return false;
        }

        // Get the list of expected value(s) of specified type before doing the comparison check
        const char* type = av->getcParam("Type");
        if (!strcmp(type, "int"))
        {
            int val = av->getiVal();
            std::vector<int> expectedVals;
            expectedVals.reserve(av->getiParam("numExpectedVals"));
            for (int i = 1; i <= av->getiParam("numExpectedVals"); i++)
            {
                // Add the expected value(s) of type int to the list
                char* expectedValParam = nullptr;
                asprintf(&expectedValParam, "expectedVal%d", i);
                if (!av->gotParam(expectedValParam))
                {
                    FPS_DEBUG_PRINT("%s >> %s does not exist in av %s. Skipping condition check\n", __func__, expectedValParam, av->name.c_str());
                    free(expectedValParam);
                    return false;
                }
                expectedVals.push_back(av->getiParam(expectedValParam));
                free(expectedValParam);
            }

            // Compare the current value to the expected value(s)
            return compareVal<int>(val, expectedVals);
        }
        else if (!strcmp(type, "double"))
        {
            double val = av->getdVal();
            std::vector<double> expectedVals;
            expectedVals.reserve(av->getiParam("numExpectedVals"));
            for (int i = 1; i <= av->getiParam("numExpectedVals"); i++)
            {
                // Add the expected value(s) of type double to the list
                char* expectedValParam = nullptr;
                asprintf(&expectedValParam, "expectedVal%d", i);
                if (!av->gotParam(expectedValParam))
                {
                    FPS_DEBUG_PRINT("%s >> %s does not exist in av %s. Skipping condition check\n", __func__, expectedValParam, av->name.c_str());
                    free(expectedValParam);
                    return false;
                }
                expectedVals.push_back(av->getdParam(expectedValParam));
                free(expectedValParam);
            }
        
            // Compare the current value to the expected value(s)
            return compareVal<double>(val, expectedVals);
        }
        else if (!strcmp(type, "string"))
        {
            std::string val = av->getcVal() ? av->getcVal() : "null";
            std::vector<std::string> expectedVals;
            expectedVals.reserve(av->getiParam("numExpectedVals"));
            for (int i = 1; i <= av->getiParam("numExpectedVals"); i++)
            {
                // Add the expected value(s) of type string to the list
                char* expectedValParam = nullptr;
                asprintf(&expectedValParam, "expectedVal%d", i);
                if (!av->gotParam(expectedValParam))
                {
                    FPS_DEBUG_PRINT("%s >> %s does not exist in av %s. Skipping condition check\n", __func__, expectedValParam, av->name.c_str());
                    free(expectedValParam);
                    return false;
                }
                expectedVals.push_back(av->getcParam(expectedValParam));
                free(expectedValParam);
            }
        
            // Compare the current value to the expected value(s)
            return compareVal<std::string>(val, expectedVals);
        }
        else 
        {
            FPS_DEBUG_PRINT("%s >> assetVar %s does not have a type. Type needs to be defined for condition check\n", __func__, av->name.c_str());
            return false;
        }
    }

    /**
     * @brief Helper function for findCondition function to determine if the given condition var
     * has a value that is equal to the condition value(s)
     * 
     * @param av the assetVar that contains the condition values to check for
     * @param condAv the condition assetVar
     * @param id the id to look for in the condition parameter
     * @return true if the condtion var has a value that is equal to at least one condition value
     */
    bool findConditionVal(assetVar* av, assetVar* condAv, int id)
    {
        // TODO: use smart ptr to reduce # of times we need to use free()
        //std::unique_ptr<std::string> numCondParam(new std::string("numConditions" + std::to_string(id)));
        char* numCondValParam = nullptr;
        asprintf(&numCondValParam, "numConditions%d", id);
        // If we do not have the correct parameters initialized before the check, immediately exit this function
        // No need to perform alarm/fault check
        if (!av->gotParam(numCondValParam) || av->getiParam(numCondValParam) <= 0)
        {
            FPS_DEBUG_PRINT("%s >> av [%s] %s either does not exist or is <= 0. Skipping condition check\n", __func__, av->name.c_str(), numCondValParam);
            free(numCondValParam);
            return false;
        }

        // Get the list of condition value(s) of specified type before doing the comparison check
        char* condTypeParam = nullptr;
        asprintf(&condTypeParam, "conditionType%d", id);
        const char* condType = av->getcParam(condTypeParam);
        if (!strcmp(condType, "int"))
        {
            int val = condAv->getiVal();
            std::vector<int> condVals;
            condVals.reserve(av->getiParam(numCondValParam));
            for (int i = 1; i <= av->getiParam(numCondValParam); i++)
            {
                // Add the condition value(s) of type int to the list
                char* condValParam = nullptr;
                asprintf(&condValParam, "conditionVal%d_%d", i, id);
                if (!av->gotParam(condValParam))
                {
                    FPS_DEBUG_PRINT("%s >> %s of type %s does not exist in av %s. Skipping condition check\n", __func__, condValParam, condType, av->name.c_str());
                    free(condValParam);
                    free(condTypeParam);
                    free(numCondValParam);
                    return false;
                }
                condVals.push_back(av->getiParam(condValParam));
                free(condValParam);
            }

            free(condTypeParam);
            free(numCondValParam);
            // Compare the current value to the condition value(s)
            return compareVal<int>(val, condVals);
        }
        else if (!strcmp(condType, "double"))
        {
            double val = condAv->getdVal();
            std::vector<double> condVals;
            condVals.reserve(av->getiParam(numCondValParam));
            for (int i = 1; i <= av->getiParam(numCondValParam); i++)
            {
                // Add the condition value(s) of type double to the list
                char* condValParam = nullptr;
                asprintf(&condValParam, "conditionVal%d_%d", i, id);
                if (!av->gotParam(condValParam))
                {
                    FPS_DEBUG_PRINT("%s >> %s of type %s does not exist in av %s. Skipping condition check\n", __func__, condValParam, condType, av->name.c_str());
                    free(condValParam);
                    free(condTypeParam);
                    free(numCondValParam);
                    return false;
                }
                condVals.push_back(av->getdParam(condValParam));
                free(condValParam);
            }

            free(condTypeParam);
            free(numCondValParam);
            // Compare the current value to the condition value(s)
            return compareVal<double>(val, condVals);
        }
        else if (!strcmp(condType, "string"))
        {
            std::string val = condAv->getcVal() ? condAv->getcVal() : "null";
            std::vector<std::string> condVals;
            condVals.reserve(av->getiParam(numCondValParam));
            for (int i = 1; i <= av->getiParam(numCondValParam); i++)
            {
                // Add the condition value(s) of type string to the list
                char* condValParam = nullptr;
                asprintf(&condValParam, "conditionVal%d_%d", i, id);
                if (!av->gotParam(condValParam))
                {
                    FPS_DEBUG_PRINT("%s >> %s of type %s does not exist in av %s. Skipping condition check\n", __func__, condValParam, condType, av->name.c_str());
                    free(condValParam);
                    free(condTypeParam);
                    free(numCondValParam);
                    return false;
                }
                condVals.push_back(av->getcParam(condValParam));
                free(condValParam);
            }
        
            free(condTypeParam);
            free(numCondValParam);
            // Compare the current value to the condition value(s)
            return compareVal<std::string>(val, condVals);
        }
        else 
        {
            FPS_DEBUG_PRINT("%s >> assetVar %s does not have a condition type. Type needs to be defined for condition check\n", __func__, av->name.c_str());
            free(condTypeParam);
            free(numCondValParam);
            return false;
        }
    }

    /**
     * @brief Helper function for CheckCondition function to find the condition var and the one or more
     * condition(s) that matches the current value of the condition var
     * 
     * @param amap the local data map used by an asset/asset manager
     * @param av the assetVar that contains the condition values to check for
     * @return true if the condition var's value is equal to at least one of the condition value(s)
     */
    bool findCondition(varmap& amap, assetVar* av)
    {
        // If we do not have the correct parameters initialized before the check, immediately exit this function
        // No need to perform alarm/fault check
        if (!av->gotParam("numConditionVars") || av->getiParam("numConditionVars") <= 0)
        {
            FPS_DEBUG_PRINT("%s >> av [%s] numConditionVars either does not exist or is <= 0. Skipping condition check\n", __func__, av->name.c_str());
            return false;
        }

        for (int i = 1; i <= av->getiParam("numConditionVars"); i++)
        {
            char* condVarParam = nullptr;
            asprintf(&condVarParam, "conditionVar%d", i);
            if (av->gotParam(condVarParam) && av->getcParam(condVarParam))
            {
                FPS_DEBUG_PRINT("%s >> assetVar [%s] has parameter [%s]\n", __func__, av->name.c_str(), condVarParam);

                // Check if the assetVar in the variable parameter already exists. If not, skip calculations
                assetVar* condAv = amap[av->getcParam(condVarParam)];

                if (!condAv)
                {
                    FPS_DEBUG_PRINT("%s >> assetVar in parameter [%s] of assetVar [%s] does not exist. Skipping condition check\n", __func__, condVarParam, av->name.c_str());
                    free(condVarParam);
                    return false;
                }

                // If we did find a condition value match, then set foundConditions to false. This indicates search failure
                // since we expect to find value matches for all condition vars
                if (!findConditionVal(av, condAv, i))
                {
                    FPS_DEBUG_PRINT("%s >> findConditionVal for conditionAv %s and av %s is false\n", __func__, condVarParam, av->name.c_str());
                    free(condVarParam);
                    return false;
                }

            }
            else
            {
                FPS_DEBUG_PRINT("%s >> assetVar [%s] does not have parameter [%s]. Skipping condition check\n", __func__, av->name.c_str(), condVarParam);
                free(condVarParam);
                return false;
            }
            free(condVarParam);
        }

        // We have reached the end of the loop, which means we have found a value match for all condition vars
        // Return true, indicating search success
        return true;
    }
}

/**
 * @brief Check if the assetVar is equal to the expected value(s)
 * 
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param av the asset var to monitor
 */
void CheckCondition(varsmap& vmap, varmap& amap, const char* aname, assetVar* av)
{
    FPS_DEBUG_PRINT("%s >> checking conditions for %s with assetVar %s\n", __func__, aname, av->name.c_str());

    VarMapUtils* vm = av->am->vm;

    double tNow = vm->get_time_dbl();
    const char* type = av->getcParam("Type");

    double AlarmTimeCfg = av->getdParam("AlarmTimeout");
    double FaultTimeCfg = av->getdParam("FaultTimeout");
    double RecoverTimeCfg = av->getdParam("RecoverTimeout");

    bool enableCondCheck = av->getbParam("EnableConditionCheck");
    // Check whether to use alarms/events/logs here
    bool enableAlert = av->getbParam("EnableAlert");

    // Grab remaining values for the variable (not defined in config)
    double currAlarmTime = av->getdParam("AlarmTime");                
    double currFaultTime = av->getdParam("FaultTime");                
    double currRecoverTime = av->getdParam("RecoverTime");

    bool seenFault = av->getbParam("seenFault");
    bool seenAlarm = av->getbParam("seenAlarm");
    bool seenReset = av->getbParam("seenReset");

    double tLast = av->getdParam("tLast");
    double tDiff = tNow - tLast;
    av->setParam("tLast", tNow);

    tm *local_tm = vm->get_local_time_now();
    const char* timestamp = strtime(local_tm);

    // If the condition check is disabled, reset alarm/fault/reset states and timers and then exit
    if (!enableCondCheck)
    {
        av->setParam("AlarmTime", av->getdParam("AlarmTimeout"));                
        av->setParam("FaultTime", av->getdParam("FaultTimeout"));                
        av->setParam("RecoverTime", av->getdParam("RecoverTimeout"));            
        av->setParam("seenFault",false);                
        av->setParam("seenAlarm", false);
        av->setParam("seenReset", false);
        return;
    }

    // Find the condition val. If return value is true, then we have found at least one condition value equal to the condition av value
    bool foundCondVal = Condition::findCondition(amap, av);
    // Find the expected val. If return value is true, then we have found at least one expected value equal to the av value
    bool foundExpectedVal = Condition::findExpectedVal(av);

    // If we have found the expected value and our conditions are satisfied, perform recovery tasks
    if (foundExpectedVal)
    {
        FPS_DEBUG_PRINT("%s >> Found expected value for av %s\n", __func__, av->name.c_str());
        // If we did not find the condition value match when we have condition vars, then skip recovery tasks
        if (!foundCondVal && av->gotParam("numConditionVars") && av->getiParam("numConditionVars") > 0) return;

        if (currRecoverTime > 0.0) ESSUtils::decrementTime(av, currRecoverTime, tDiff, "RecoverTime");
        if (currRecoverTime <= 0.0 && !seenReset)
        {
            av->setParam("seenReset", true);
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

            // Reset fault shutdown -> this indicates system is good to run
            ESSUtils::setAvStatus(vmap, amap, aname, av, "FaultShutdown", false, "FaultShutdown", false);
        }

        // Increment both the current alarm/fault time
        if (currAlarmTime < AlarmTimeCfg) ESSUtils::incrementTime(av, currAlarmTime, tDiff, AlarmTimeCfg, "AlarmTime");
        if (currFaultTime < FaultTimeCfg) ESSUtils::incrementTime(av, currFaultTime, tDiff, FaultTimeCfg, "FaultTime");
    }
    // We did not find the expected value match
    // If the condition values are not satisfied, proceed to decrement alarm/fault time and send out alarm/fault event
    // once alarm/fault time has elapsed
    else
    {
        FPS_DEBUG_PRINT("%s >> Expected value not found for av %s   foundCondVal %s\n", __func__, av->name.c_str(), foundCondVal ? "true" : "false");
        // If we did not find a condition value match when we have condition vars, skip alarm/fault event tasks
        if (!foundCondVal && av->gotParam("numConditionVars") && av->getiParam("numConditionVars") > 0) return;

        // If numExpectedVal is <= 0, then we should exit immediately. Should have to send out alarm/fault
        // if we can't compare the current value to the expected value(s)
        if (!av->gotParam("numExpectedVals") || av->getiParam("numExpectedVals") <= 0) return;

        if (currFaultTime > 0) ESSUtils::decrementTime(av, currFaultTime, tDiff, "FaultTime");
        if (currAlarmTime > 0) ESSUtils::decrementTime(av, currAlarmTime, tDiff, "AlarmTime");
            
        // Set monitor var to fault state
        if(currFaultTime <= 0.0 && !seenFault)
        {
            FPS_DEBUG_PRINT("%s >> Seen fault for %s\n", __func__, av->name.c_str());
            av->setParam("seenFault", true);
            if (seenReset) av->setParam("seenReset", false);
            av->setParam("RecoverTime", RecoverTimeCfg);

            // Send alarm/event and log to file if enabled
            if (enableAlert)
            {
                // Prepare fault msg to send
                std::string msg;
                if (!strcmp(type, "int"))         msg = fmt::format("[{}] value [{}] != expected value(s) for {:.2f} seconds at {}"
                                                            , av->name.c_str()
                                                            , av->getiVal()
                                                            , FaultTimeCfg
                                                            , timestamp);
                else if (!strcmp(type, "double")) msg = fmt::format("[{}] value [{}] != expected value(s) for {:.2f} seconds at {}"
                                                            , av->name.c_str()
                                                            , av->getdVal()
                                                            , FaultTimeCfg
                                                            , timestamp);
                else if (!strcmp(type, "string")) msg = fmt::format("[{}] value [{}] != expected value(s) for {:.2f} seconds at {}"
                                                            , av->name.c_str()
                                                            , av->getcVal()
                                                            , FaultTimeCfg
                                                            , timestamp);

                MonitorVarUtils::alert(vmap, amap, aname, av, msg, true);

                // Increment # of faults
                amap["FaultCnt"]->addVal(1);
            }

            // Proceed to shutdown due to fault here
            ESSUtils::setAvStatus(vmap, amap, aname, av, "FaultShutdown", true, "FaultShutdown", true);

            // Set MonitorVarFault here so that can be used elsewhere in the source code
            std::string val = av->name + "_cond";
            const char* monFltAvVal = val.c_str();
            vm->setVal(vmap, amap["MonitorVarFault"]->comp.c_str(), amap["MonitorVarFault"]->name.c_str(), monFltAvVal);
        }
            
        // Set monitor var to alarm state
        if(currAlarmTime <= 0.0 && !seenAlarm)
        {
            av->setParam("seenAlarm", true);
            if (seenReset) av->setParam("seenReset", false);
            av->setParam("RecoverTime", RecoverTimeCfg);

            // Send alarm/event and log info if enabled
            if (enableAlert)
            {
                // Prepare alarm msg to send
                std::string msg;
                if (!strcmp(type, "int"))         msg = fmt::format("[{}] value [{}] != expected value(s) for {:.2f} seconds at {}"
                                                            , av->name.c_str()
                                                            , av->getiVal()
                                                            , FaultTimeCfg
                                                            , timestamp);
                else if (!strcmp(type, "double")) msg = fmt::format("[{}] value [{}] != expected value(s) for {:.2f} seconds at {}"
                                                            , av->name.c_str()
                                                            , av->getdVal()
                                                            , FaultTimeCfg
                                                            , timestamp);
                else if (!strcmp(type, "string")) msg = fmt::format("[{}] value [{}] != expected value(s) for {:.2f} seconds at {}"
                                                            , av->name.c_str()
                                                            , av->getcVal()
                                                            , FaultTimeCfg
                                                            , timestamp);

                MonitorVarUtils::alert(vmap, amap, aname, av, msg, false);

                // Increment # of alarms
                amap["AlarmCnt"]->addVal(1);
            }

            // Set MonitorVarAlarm here so that can be used elsewhere in the source code
            std::string val = av->name + "_cond";
            const char* monAlrmAvVal = val.c_str();
            vm->setVal(vmap, amap["MonitorVarAlarm"]->comp.c_str(), amap["MonitorVarAlarm"]->name.c_str(), monAlrmAvVal);
        }
    }
}

/**
 * @brief Check if the current value of the assetVar is above or below the threshold value
 * 
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param av the asset var to monitor
 */
void CheckLimits(varsmap& vmap, varmap& amap, const char* aname, assetVar* av)
{
    FPS_PRINT_DEBUG("Checking limits for {} with assetVar {}", aname, av->name.c_str());

    VarMapUtils* vm = av->am->vm;

    double tNow = vm->get_time_dbl();
    double val = av->getdVal();

    double maxAlrmThreshld = av->getdParam("MaxAlarmThreshold");
    double maxFltThreshld = av->getdParam("MaxFaultThreshold");
    double maxReset = av->getdParam("MaxResetValue");

    double minAlrmThreshld = av->getdParam("MinAlarmThreshold");
    double minFltThreshld = av->getdParam("MinFaultThreshold");
    double minReset = av->getdParam("MinResetValue");

    double maxAlarmTimeCfg = av->getdParam("MaxAlarmTimeout");
    double maxFaultTimeCfg = av->getdParam("MaxFaultTimeout");
    double maxRecoverTimeCfg = av->getdParam("MaxRecoverTimeout");

    double minAlarmTimeCfg = av->getdParam("MinAlarmTimeout");
    double minFaultTimeCfg = av->getdParam("MinFaultTimeout");
    double minRecoverTimeCfg = av->getdParam("MinRecoverTimeout");

    bool enableFaultCheck = av->getbParam("EnableFaultCheck");
    bool enableMaxCheck = av->getbParam("EnableMaxValCheck");
    bool enableMinCheck = av->getbParam("EnableMinValCheck");
    // Check whether to use alarms/events/logs here
    bool enableAlert = av->getbParam("EnableAlert");

    // Grab remaining values for the variable (not defined in config)
    double currMaxAlarmTime = av->getdParam("MaxAlarmTime");                
    double currMaxFaultTime = av->getdParam("MaxFaultTime");                
    double currMaxRecoverTime = av->getdParam("MaxRecoverTime");  

    double currMinAlarmTime = av->getdParam("MinAlarmTime");                
    double currMinFaultTime = av->getdParam("MinFaultTime");                
    double currMinRecoverTime = av->getdParam("MinRecoverTime");

    bool seenMaxFault = av->getbParam("seenMaxFault");
    bool seenMinFault = av->getbParam("seenMinFault");
    bool seenMaxAlarm = av->getbParam("seenMaxAlarm");
    bool seenMinAlarm = av->getbParam("seenMinAlarm");
    bool seenMaxReset = av->getbParam("seenMaxReset");
    bool seenMinReset = av->getbParam("seenMinReset");

    double tLast = av->getdParam("tLast");
    double tDiff = tNow - tLast;
    av->setParam("tLast", tNow);
    int debug = av->getiParam("debug");

    if (debug > 0 && (enableMaxCheck || enableMinCheck))
    {
        if (enableMaxCheck) FPS_PRINT_INFO("{} Value [{:.3f}] Max Alarm [{:.3f}] Max Fault [{:.3f}] Max Reset [{:.3f}] Fault Time [{:.4f}] Alarm Time [{:.4f}] Recovery Time [{:.4f}]"
                , av->name
                , val
                , maxAlrmThreshld
                , maxFltThreshld
                , maxReset
                , currMaxFaultTime
                , currMaxAlarmTime
                , currMaxRecoverTime
            );

        if (enableMinCheck) FPS_PRINT_INFO("{} Value [{:.3f}] Min Alarm [{:.3f}] Min Fault [{:.3f}] Min Reset [{:.3f}]   Fault Time [{:.4f}] Alarm Time [{:.4f}] Recovery Time [{:.4f}]"
                , av->name
                , val
                , minAlrmThreshld
                , minFltThreshld
                , minReset
                , currMinFaultTime
                , currMinAlarmTime
                , currMinRecoverTime
            );
        if (debug > 0) av->setParam("debug", debug - 1);
    }

    tm *local_tm = vm->get_local_time_now();
    const char* timestamp = strtime(local_tm);

    // If fault check is disabled, set FaultShutdown back to false
    if (av->gotParam("EnableFaultCheck") && !av->getbParam("EnableFaultCheck"))
    {
        // If we don't want to repeatedly set FaultShutdown to false, we'll probably
        // need a parameter
        if (!av->gotParam("FaultShutdownReset"))  av->setParam("FaultShutdownReset", false);
        if (!av->getbParam("FaultShutdownReset"))
        {
            ESSUtils::setAvStatus(vmap, amap, aname, av, "FaultShutdown", false, "FaultShutdown", false);
            av->setParam("FaultShutdownReset", true);
        }
    }
    else
    {
        if (!av->gotParam("FaultShutdownReset"))  av->setParam("FaultShutdownReset", true);
        if (av->getbParam("FaultShutdownReset"))  av->setParam("FaultShutdownReset", false);
    }

    // PSW we may have several different thresholds which may be operationally dependent
    // PSW So we may have a limit gradient based on some other value

    // Check if current value is over the threshold value
    if (enableMaxCheck)
    {
        if (enableFaultCheck && val > maxFltThreshld)
        {
            // Decrement fault time
            if (currMaxFaultTime > 0) ESSUtils::decrementTime(av, currMaxFaultTime, tDiff, "MaxFaultTime");
            // Set monitor var to fault state
            if(currMaxFaultTime <= 0.0 && !seenMaxFault)
            {
                av->setParam("seenMaxFault", true);
                if (seenMaxReset) av->setParam("seenMaxReset", false);
                av->setParam("MaxRecoverTime", maxRecoverTimeCfg);

                // Send alarm/event and log info if enabled
                if (enableAlert)
                {
                    // Prepare fault msg to send
                    std::string msg = fmt::format("[{}] value [{:.3f}] > max fault threshold [{:.3f}] for {:.2f} seconds at {}"
                            , av->name.c_str()
                            , val
                            , maxFltThreshld
                            , maxFaultTimeCfg
                            , timestamp);

                    MonitorVarUtils::alert(vmap, amap, aname, av, msg, true);

                    // Increment # of faults
                    amap["FaultCnt"]->addVal(1);
                }

                // Proceed to shutdown due to fault here
                ESSUtils::setAvStatus(vmap, amap, aname, av, "FaultShutdown", true, "FaultShutdown", true);

                // Set MonitorVarFault here so that can be used elsewhere in the source code
                std::string val = av->name + "_limit_max";
                const char* monFltAvVal = val.c_str();
                vm->setVal(vmap, amap["MonitorVarFault"]->comp.c_str(), amap["MonitorVarFault"]->name.c_str(), monFltAvVal);
            }
        }
        if (val > maxAlrmThreshld)
        {
            // Decrement alarm time
            if (currMaxAlarmTime > 0) ESSUtils::decrementTime(av, currMaxAlarmTime, tDiff, "MaxAlarmTime");
            // Set monitor var to alarm state
            if(currMaxAlarmTime <= 0.0 && !seenMaxAlarm)
            {
                av->setParam("seenMaxAlarm", true);
                if (seenMaxReset) av->setParam("seenMaxReset", false);
                av->setParam("MaxRecoverTime", maxRecoverTimeCfg);

                // Send alarm/event and log info if enabled
                if (enableAlert)
                {
                    // Prepare alarm msg to send out
                    std::string msg = fmt::format("[{}] value [{:.3f}] > max alarm threshold [{:.3f}] for {:.2f} seconds at {}"
                                , av->name.c_str()
                                , val
                                , maxAlrmThreshld
                                , maxAlarmTimeCfg
                                , timestamp);
                    
                    MonitorVarUtils::alert(vmap, amap, aname, av, msg, false);

                    // Increment # of alarms
                    amap["AlarmCnt"]->addVal(1);
                }

                // Set MonitorVarAlarm here so that can be used elsewhere in the source code
                std::string val = av->name + "_limit_max";
                const char* monAlrmAvVal = val.c_str();
                vm->setVal(vmap, amap["MonitorVarAlarm"]->comp.c_str(), amap["MonitorVarAlarm"]->name.c_str(), monAlrmAvVal);
            }
        }
        if (val < maxReset)
        {
            // Decrement recovery time
            if (currMaxRecoverTime > 0.0) ESSUtils::decrementTime(av, currMaxRecoverTime, tDiff, "MaxRecoverTime");
            // Reset current state of monitor var
            if(currMaxRecoverTime <= 0.0 && !seenMaxReset)
            {
                av->setParam("seenMaxReset", true);
                if (seenMaxFault) 
                {
                    av->setParam("seenMaxFault", false);
                    // Decrement # of faults if alert is enabled
                    if (enableAlert)
                    {
                        int cnt = amap["FaultCnt"]->getiVal() - 1;
                        amap["FaultCnt"]->setVal(cnt > 0 ? cnt : 0);
                    }
                }
                if (seenMaxAlarm) 
                {
                    av->setParam("seenMaxAlarm", false);
                    // Decrement # of alarms if alert is enabled
                    if (enableAlert)
                    {
                        int cnt = amap["AlarmCnt"]->getiVal() - 1;
                        amap["AlarmCnt"]->setVal(cnt > 0 ? cnt : 0);
                    }
                }

                // Reset fault shutdown -> this indicates system is good to run
                ESSUtils::setAvStatus(vmap, amap, aname, av, "FaultShutdown", false, "FaultShutdown", false);
            }

            // Increment both the current alarm/fault time
            if (currMaxAlarmTime < maxAlarmTimeCfg) ESSUtils::incrementTime(av, currMaxAlarmTime, tDiff, maxAlarmTimeCfg, "MaxAlarmTime");
            if (currMaxFaultTime < maxFaultTimeCfg) ESSUtils::incrementTime(av, currMaxFaultTime, tDiff, maxFaultTimeCfg, "MaxFaultTime");
        }
    }
    else
    {

        // If max limit monitoring is disabled, then reset the alarm/fault/reset states and timers
        av->setParam("MaxAlarmTime", av->getdParam("MaxAlarmTimeout"));                
        av->setParam("MaxFaultTime", av->getdParam("MaxFaultTimeout"));                
        av->setParam("MaxRecoverTime", av->getdParam("MaxRecoverTimeout"));

        av->setParam("seenMaxFault", false);                
        av->setParam("seenMaxAlarm", false);
        av->setParam("seenMaxReset", false);
    }
        
    // Check if the current value is under the threshold value
    if (enableMinCheck)
    {
        if(enableFaultCheck && val < minFltThreshld)
        {
            // Decrement current fault time
            if (currMinFaultTime > 0) ESSUtils::decrementTime(av, currMinFaultTime, tDiff, "MinFaultTime");
            // Set monitor var to fault state
            if(currMinFaultTime <= 0.0 && !seenMinFault)
            {
                av->setParam("seenMinFault", true);
                if (seenMinReset) av->setParam("seenMinReset", false);
                av->setParam("MinRecoverTime", minRecoverTimeCfg);

                // Send alarm/event and log info if enabled
                if (enableAlert)
                {
                    // Prepare fault msg to send out
                    std::string msg = fmt::format("[{}] value [{:.3f}] < min fault threshold [{:.3f}] for {:.2f} seconds at {}"
                            , av->name.c_str()
                            , val
                            , minFltThreshld
                            , minFaultTimeCfg
                            , timestamp);

                    MonitorVarUtils::alert(vmap, amap, aname, av, msg, true);

                    // Increment # of faults
                    amap["FaultCnt"]->addVal(1);
                }

                // Proceed to shutdown due to fault here
                ESSUtils::setAvStatus(vmap, amap, aname, av, "FaultShutdown", true, "FaultShutdown", true);

                // Set MonitorVarFault here so that can be used elsewhere in the source code
                std::string val = av->name + "_limit_min";
                const char* monFltAvVal = val.c_str();
                vm->setVal(vmap, amap["MonitorVarFault"]->comp.c_str(), amap["MonitorVarFault"]->name.c_str(), monFltAvVal);
            }
        }
        if (val < minAlrmThreshld)
        {
            // Decrement current alarm time
            if (currMinAlarmTime > 0) ESSUtils::decrementTime(av, currMinAlarmTime, tDiff, "MinAlarmTime");
            // Set monitor var to alarm state
            if(currMinAlarmTime <= 0.0 && !seenMinAlarm)
            {
                av->setParam("seenMinAlarm", true);
                if (seenMinReset) av->setParam("seenMinReset", false);
                av->setParam("MinRecoverTime", minRecoverTimeCfg);

                // Send alarm/event and log info if enabled
                if (enableAlert)
                {
                    // Prepare alarm msg to send out
                    std::string msg = fmt::format("[{}] value [{:.3f}] < min alarm threshold [{:.3f}] for {:.2f} seconds at {}"
                            , av->name.c_str()
                            , val
                            , minAlrmThreshld
                            , minAlarmTimeCfg
                            , timestamp);
                    
                    MonitorVarUtils::alert(vmap, amap, aname, av, msg, false);

                    // Increment # of alarms
                    amap["AlarmCnt"]->addVal(1);
                }

                // Set MonitorVarAlarm here so that can be used elsewhere in the source code
                std::string val = av->name + "_limit_min";
                const char* monAlrmAvVal = val.c_str();
                vm->setVal(vmap, amap["MonitorVarAlarm"]->comp.c_str(), amap["MonitorVarAlarm"]->name.c_str(), monAlrmAvVal);
            }
        }
        if(val > minReset)
        {
            // Decrement recovery time
            if (currMinRecoverTime > 0.0) ESSUtils::decrementTime(av, currMinRecoverTime, tDiff, "MinRecoverTime");
            // Reset current state of monitor var
            if(currMinRecoverTime <= 0.0 && !seenMinReset)
            {
                av->setParam("seenMinReset", true);
                if (seenMinFault)
                {
                    av->setParam("seenMinFault", false);
                    // Decrement # of faults if alert is enabled
                    if (enableAlert)
                    {
                        int cnt = amap["FaultCnt"]->getiVal() - 1;
                        amap["FaultCnt"]->setVal(cnt > 0 ? cnt : 0);
                    }
                } 
                if (seenMinAlarm) 
                {
                    av->setParam("seenMinAlarm", false);
                    // Decrement # of alarms if alert is enabled
                    if (enableAlert)
                    {
                        int cnt = amap["AlarmCnt"]->getiVal() - 1;
                        amap["AlarmCnt"]->setVal(cnt > 0 ? cnt : 0);
                    }
                }

                // Reset fault shutdown -> this indicates system is good to run
                ESSUtils::setAvStatus(vmap, amap, aname, av, "FaultShutdown", false, "FaultShutdown", false);
            }
            
            // Increment both the current alarm/fault time
            if (currMinAlarmTime < minAlarmTimeCfg) ESSUtils::incrementTime(av, currMinAlarmTime, tDiff, minAlarmTimeCfg, "MinAlarmTime");
            if (currMinFaultTime < minFaultTimeCfg) ESSUtils::incrementTime(av, currMinFaultTime, tDiff, minFaultTimeCfg, "MinFaultTime");
        }
    }
    else
    {

        // If min limit monitoring is disabled, then reset the alarm/fault/reset states and timers
        av->setParam("MinAlarmTime", av->getdParam("MinAlarmTimeout"));                
        av->setParam("MinFaultTime", av->getdParam("MinFaultTimeout"));                
        av->setParam("MinRecoverTime", av->getdParam("MinRecoverTimeout"));    

        av->setParam("seenMinFault", false);
        av->setParam("seenMinAlarm", false);
        av->setParam("seenMinReset", false);
    }
}

/**
 * @brief Periodically check if the variables are either over or under the threshold after a certain amount of time
 * 
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param p_fims the interface used to send data to
 * @param av the asset var to monitor
 */
int CheckMonitorVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    if (!av)
    {
        //FPS_PRINT_INFO("Runing with no assetVar. Skipping monitor check");
        FPS_PRINT_INFO("{}", "Runing with no assetVar. Skipping monitor check");
        return 0;
    }
    
    FPS_PRINT_DEBUG("av [{}] av->am [{}] aname [{}]", av->getfName(), av->am ? av->am->name : "null", cstr{aname});
    VarMapUtils* vm = av->am->vm;

    std::string reloadStr = fmt::format("{}_{}", av->name, "reload");
    int reload = vm->CheckReload(vmap, amap, aname, reloadStr.c_str());
    if(reload < 2)
    {
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

        FPS_PRINT_INFO("Fault Destination for {} with assetVar {} is {}"
            , aname, av->getfName(), cstr{amap["FaultDestination"]->getcVal()});
        FPS_PRINT_INFO("Alarm Destination for {} with assetVar {} is {}"
            , aname, av->getfName(), cstr{amap["AlarmDestination"]->getcVal()});

        if (!amap["FaultDestination"]->getcVal())  amap["FaultDestination"]->setVal(fltDest);
        if (!amap["AlarmDestination"]->getcVal())  amap["AlarmDestination"]->setVal(alrmDest);
        if (!amap["MonitorVarAlarm"]->getcVal())   amap["MonitorVarAlarm"]->setVal(cval);
        if (!amap["MonitorVarFault"]->getcVal())   amap["MonitorVarFault"]->setVal(cval);

        if (fltDest)  free((void*)fltDest);
        if (alrmDest) free((void*)alrmDest);

        if(reload < 1)
        {
            FPS_PRINT_INFO("Not set up for [{}] reloadStr [{}]", av->name, reloadStr);

            // Set up monitor parameters for the assetVar
            SetupMonitorVar(vmap, amap, aname, av);

            // Store the monitor assetVar in the amap
            amap[av->name.c_str()] = av;
        }
        ival = 2;
        amap[reloadStr] = vm->setLinkVal(vmap, aname, "/reload", reloadStr.c_str(), ival);
        amap[reloadStr]->setVal(ival);
    }

    if (av->gotParam("EnableStateCheck"))
    {
        const char* type = av->getcParam("Type");
        FPS_DEBUG_PRINT("%s >> type for [%s] is  [%s] \n", __func__, av->getfName(), type?type:"No Type");
        if(!type)
            type = (const char*)"double";

        if (!strcmp(type, "int"))
        {
            FPS_DEBUG_PRINT("%s >> Checking state for %s of type %s\n", __func__, av->name.c_str(), type);
            int val = av->getiVal();
            int lastVal = av->getiParam("lastVal");
            CheckState<int>(vmap, amap, aname, av, val, lastVal);
        }
        else if (!strcmp(type, "double"))
        {
            FPS_DEBUG_PRINT("%s >> Checking state for %s of type %s\n", __func__, av->name.c_str(), type);
            double val = av->getdVal();
            double lastVal = av->getdParam("lastVal");
            CheckState<double>(vmap, amap, aname, av, val, lastVal);
        }
        else if (!strcmp(type, "string"))
        {
            FPS_DEBUG_PRINT("%s >> Checking state for %s of type %s\n", __func__, av->name.c_str(), type);
            std::string val = av->getcVal();
            std::string lastVal = av->getcParam("lastVal");
            CheckState<std::string>(vmap, amap, aname, av, val, lastVal);
        }
        else
        {
            FPS_PRINT_INFO("{} with assetVar {} does not have a type. Type needs to be defined for state check", aname, av->name);
        }
    }
    else if (av->gotParam("EnableConditionCheck")) CheckCondition(vmap, amap, aname, av);
    else                                           CheckLimits(vmap, amap, aname, av);

    return 0;
}

#endif