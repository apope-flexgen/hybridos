#ifndef ALARMFAULTSHANDLER_CPP
#define ALARMFAULTSHANDLER_CPP

#include "asset.h"
#include "formatters.hpp"
#include "ess_utils.hpp"

char* strtime(const struct tm *timeptr);

extern "C++" 
{
    int process_sys_alarm(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,assetVar* av);
}

namespace AlarmFaultHandlerUtils
{
    /**
     * @brief Initialize the alarm/fault variables (ex.: destination, normal fault/alarm condition messages, etc.)
     * 
     * @param vmap the global data map shared by all assets/asset managers
     * @param amap the local data map used by an asset/asset manager
     * @param aname the name of the asset/asset manager
     * @param av the alarm/fault assetVar
     */
    void initAlarmFaultVars(varsmap& vmap, varmap& amap, const char* aname, assetVar* av)
    {
        FPS_PRINT_DEBUG("Initializing all alarm/fault variable for asset [{}] with [{}]", cstr{aname}, av->getfName());

        VarMapUtils* vm = av->am->vm;

        // Set up /config/[asset]:FaultDestination if that variable does not exist yet
        if (!amap["FaultDestination"])
        {
            char* fltDest = nullptr;
            asprintf(&fltDest,  "/assets/%s/summary:faults", aname);

            amap["FaultDestination"] = vm->setLinkVal(vmap, aname, "/config", "FaultDestination", fltDest);
            if (!amap["FaultDestination"]->getcVal())
                amap["FaultDestination"]->setVal(fltDest);
            
            if (fltDest)
                free((void*)fltDest);
        }

        // Set up /config/[asset]:NoFaultMsg if that variable does not exist yet
        if (!amap["NoFaultMsg"])
        {
            char* noFltMsg = (char*)"Normal";
            amap["NoFaultMsg"] = vm->setLinkVal(vmap, aname, "/config", "NoFaultMsg", noFltMsg);
            if (!amap["NoFaultMsg"]->getcVal())
                amap["NoFaultMsg"]->setVal(noFltMsg);
        }

        // Set up /status/[asset]:FaultCnt if that variable does not exist yet
        if (!amap["FaultCnt"])
        {
            int ival = 0;
            amap["FaultCnt"] = vm->setLinkVal(vmap, aname, "/status", "FaultCnt", ival); 
        }

        FPS_PRINT_DEBUG("Fault Destination [{}]", amap["FaultDestination"]->getcVal() ? amap["FaultDestination"]->getcVal() : "did not find val from config");
        FPS_PRINT_DEBUG("No fault msg [{}]\n", amap["NoFaultMsg"]->getcVal() ? amap["NoFaultMsg"]->getcVal() : "did not find val from config");
        
        // Set up /config/[asset]:AlarmDestination if that variable does not exist yet
        if (!amap["AlarmDestination"])
        {
            char* alrmDest = nullptr;
            asprintf(&alrmDest, "/assets/%s/summary:alarms", aname);

            amap["AlarmDestination"] = vm->setLinkVal(vmap, aname, "/config", "AlarmDestination", alrmDest);
            if (!amap["AlarmDestination"]->getcVal())
                amap["AlarmDestination"]->setVal(alrmDest);

            if (alrmDest)
                free((void*)alrmDest);
        }

        // Set up /config/[asset]:NoAlarmMsg if that variable does not exist yet
        if (!amap["NoAlarmMsg"])
        {
            char* noAlrmMsg = (char*)"Normal";
            amap["NoAlarmMsg"] = vm->setLinkVal(vmap, aname, "/config", "NoAlarmMsg", noAlrmMsg);
            if (!amap["NoAlarmMsg"]->getcVal())
                amap["NoAlarmMsg"]->setVal(noAlrmMsg);
        }

        // Set up /status/[asset]:AlarmCnt if that variable does not exist yet
        if (!amap["AlarmCnt"])
        {
            int ival = 0;
            amap["AlarmCnt"] = vm->setLinkVal(vmap, aname, "/status", "AlarmCnt", ival); 
        }

        FPS_PRINT_DEBUG("Alarm Destination [{}]", amap["AlarmDestination"]->getcVal() ? amap["AlarmDestination"]->getcVal() : "did not find val from config");
        FPS_PRINT_DEBUG("No alarm msg [{}]", amap["NoAlarmMsg"]->getcVal() ? amap["NoAlarmMsg"]->getcVal() : "did not find val from config");
        
    }

    /**
     * @brief Resets the monitor assetVar's alarm/fault state. Don't reset the alarm/fault/recover time. 
     * This will allow fault/alarm messages to be reported again if the alarm/fault still exists
     * 
     * @param monitorAv the monitor assetVar to reset the alarm/fault state for
     * @param type the monitor assetVar type (either fault or alarm)
     */
    void resetMonitorVar(assetVar* monitorAv, const std::string& type)
    {
        // Reset upper limit monitor var alarm/fault state
        if (monitorAv->gotParam("EnableMaxValCheck"))
        {
            if (type == "fault")
            {
                if (monitorAv->gotParam("seenMaxFault") && monitorAv->getbParam("seenMaxFault"))
                    monitorAv->setParam("seenMaxFault", false);
            }
            else
            {
                if (monitorAv->gotParam("seenMaxAlarm") && monitorAv->getbParam("seenMaxAlarm"))
                    monitorAv->setParam("seenMaxAlarm", false); 
            }
        }

        // Reset lower limit monitor var alarm/fault state
        if (monitorAv->gotParam("EnableMinValCheck"))
        {
            if (type == "fault")
            {
                if (monitorAv->gotParam("seenMinFault") && monitorAv->getbParam("seenMinFault"))
                    monitorAv->setParam("seenMinFault", false);
            }
            else
            {
                if (monitorAv->gotParam("seenMinAlarm") && monitorAv->getbParam("seenMinAlarm"))
                    monitorAv->setParam("seenMinAlarm", false);
            }
        }

        // Reset state monitor var alarm/fault state
        if (monitorAv->gotParam("EnableStateCheck"))
        {
            if (type == "fault")
            {
                if (monitorAv->gotParam("seenFault") && monitorAv->getbParam("seenFault"))
                    monitorAv->setParam("seenFault", false);
            }
            else
            {
                if (monitorAv->gotParam("seenAlarm") && monitorAv->getbParam("seenAlarm"))
                    monitorAv->setParam("seenAlarm", false);
            }
        }
        // Reset condition monitor var alarm/fault state
        if (monitorAv->gotParam("EnableConditionCheck"))
        {
            if (type == "fault")
            {
                if (monitorAv->gotParam("seenFault") && monitorAv->getbParam("seenFault"))
                    monitorAv->setParam("seenFault", false);
            }
            else
            {
                if (monitorAv->gotParam("seenAlarm") && monitorAv->getbParam("seenAlarm"))
                    monitorAv->setParam("seenAlarm", false);
            }
        }

        // Reset custom monitor var alarm/fault state (e.g. monitoring variables that use CheckMonitorVar_v2)
        if (monitorAv->gotParam("enableMonitor"))
        {
            if (type == "fault")
            {
                if (monitorAv->gotParam("seenFault") && monitorAv->getbParam("seenFault"))
                    monitorAv->setParam("seenFault", false);
            }
            else
            {
                if (monitorAv->gotParam("seenAlarm") && monitorAv->getbParam("seenAlarm"))
                    monitorAv->setParam("seenAlarm", false);
            }
        }
        if (monitorAv->gotParam("enableComms"))
        {
            if (type == "fault")
            {
                if (monitorAv->gotParam("seenFault") && monitorAv->getbParam("seenFault"))
                    monitorAv->setParam("seenFault", false);
            }
        }
    }

    /**
     * @brief Clear all alarm/fault messages and reset the monitor assetVar's alarm/fault state, if applicable
     * 
     * @param vmap the global data map shared by all assets/asset managers
     * @param amap the local data map used by an asset/asset manager
     * @param aname the name of the asset/asset manager
     * @param av the alarm/fault assetVar
     */
    void clearAlarms(varsmap& vmap, varmap& amap, const char* aname, assetVar* av)
    {
        bool debug = false;
        if (av->gotParam("debug"))
        {
            debug = av->getbParam("debug");
        }
        
        if(debug)FPS_PRINT_INFO("Clearing all alarms/faults for asset [{}] with [{}]", cstr{aname}, av->getfName());
        if (vmap.find(av->comp) != vmap.end())
        {
            varmap avMap = vmap[av->comp];
            for (auto& pair : avMap)
            {
                assetVar* var = pair.second;
                if(debug)FPS_PRINT_DEBUG("avMap key {} assetVar {} lastVal before reset is {}", pair.first, var->name, cstr{var->getcLVal()});
                // Check to make sure we're not resetting the clear fault/alarm assetVar
                if (var->name != av->name)
                {
                    if (av->gotParam("type") && !strcmp(av->getcParam("type"), "fault"))
                        var->setLVal(amap["NoFaultMsg"]->getcVal());
                    else
                        var->setLVal(amap["NoAlarmMsg"]->getcVal());

                    if(debug)FPS_PRINT_DEBUG("assetVar {} lastVal after reset is now {}", var->name, cstr{var->getcLVal()});
                }
            }
        }
        if(debug)FPS_PRINT_INFO("Clearing all alarms/faults 2 for asset [{}] with [{}]", cstr{aname}, av->getfName());
        // If the assetVar name has been provided to the clear fault/alarms assetVar, see if there  is anything else we need
        // to do with that particular assetVar (ex.: reset monitor assetVar's fault/alarm state)
        char* defComp = nullptr;

        if (av->gotParam("defComp"))
        {
            defComp = av->getcParam("defComp");
            if(defComp)
            {
                if(vmap.find(defComp) == vmap.end())
                {
                    defComp = nullptr;
                }
            }
        }
        if (av->gotParam("numVars") && av->getiParam("numVars") > 0)
        {
            if(debug)FPS_PRINT_INFO("numVars {}",av->getiParam("numVars"));
            for (int i = 1; i <= av->getiParam("numVars"); i++)
            {
                const std::string varParam = fmt::format("variable{}", i);
                if (av->gotParam(varParam.c_str()) && av->getcParam(varParam.c_str()))
                {
                    if(debug)FPS_PRINT_INFO("assetVar [{}] has parameter [{}]", av->name, varParam);

                    const std::string avUri(av->getcParam(varParam.c_str()));
                    if (avUri.empty())
                    {
                        FPS_PRINT_WARN("assetVar [{}:{}] does not have parameter [{}]", av->comp, av->name, varParam);
                        continue;
                    }

                    assetUri my(avUri.c_str());
                    if (!my.Uri && !my.Var)
                    {
                        FPS_PRINT_WARN("assetVar [{}:{}] does not have uri [{}] and/or var [{}]", av->comp, av->name, cstr{my.Uri}, cstr{my.Var});
                        continue;
                    }

                    // Check if the assetVar in the variable parameter already exists. If not, skip reset step
                    auto vp = av->getcParam(varParam.c_str());
                    assetVar* monitorAv = amap[vp];
                    // if we can find a good one useit 
                    if (!monitorAv && defComp != nullptr)
                    {
                        if(vmap[defComp].find(vp) != vmap[defComp].end())
                        {
                            if(debug)FPS_PRINT_INFO(" setting up amap for  [{}:{}]", defComp, vp);
                            monitorAv = vmap[defComp][vp];
                            amap[vp] = monitorAv; //vmap[av->comp][av->getcParam(varParam)]
                        }

                    }
                    if (monitorAv)
                    {
                        if(debug)FPS_PRINT_INFO("monitor assetVar {} in parameter [{}] of assetVar [{}] exists. Starting reset step", monitorAv->name, varParam, av->name);
                        std::string type = "";
                        if (av->gotParam("type") && strcmp(av->getcParam("type"), "fault") == 0)
                            type = "fault";
                            
                        AlarmFaultHandlerUtils::resetMonitorVar(monitorAv, type);
                    }
                    else
                        if(debug)FPS_PRINT_INFO("monitor assetVar [{}] in parameter [{}] of assetVar [{}] does not exist. Skipping alarm/fault reset step."
                                        , av->getcParam(varParam.c_str()) ? av->getcParam(varParam.c_str()) : "no monitor var name", varParam, av->name);
                }
                else
                    FPS_PRINT_INFO("assetVar [{}] does not have parameter [{}]. Skipping alarm/fault reset step", av->getfName(), varParam);
            }
        }
        if(debug)FPS_PRINT_INFO("Clearing all alarms/faults 3 for asset [{}] with [{}]", cstr{aname}, av->getfName());

    }
}

/**
 * @brief Report and record the alarm/fault produced either from the hardware (ex.: BMS, PCS) or the ESS Controller itself
 * 
 * @param vmap the global data map shared by all assets/asset managers
 * @param amap the local data map used by an asset/asset manager
 * @param aname the name of the asset/asset manager
 * @param p_fims the interface used for data interchange
 * @param av the alarm/fault assetVar
 */
int process_sys_alarm(varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, assetVar* av)
{
    FPS_PRINT_DEBUG("av {} av->am {}", fmt::ptr(av), fmt::ptr(av ? av->am : nullptr));
    if (!av) 
    {
        FPS_PRINT_INFO("running No av !!");
        return 0;
    }
    if (!av->am)
    {    
        FPS_PRINT_ERROR("name [{}] has no associated AM"
            , av->name
        );
        return 0;
    }

    VarMapUtils* vm = av->am->vm;
    FPS_PRINT_DEBUG("assetVar [{}] aname [{}] value [{}]  lastValue [{}]"
            , av->name, cstr{aname}, cstr{avVal}, cstr{avLVal});

    // Initialize the alarm/fault variables (ex.: Alarm/FaultDestination; done once)
    AlarmFaultHandlerUtils::initAlarmFaultVars(vmap, amap, aname, av);

    // Grab fault/alarm destination and no fault/alarm string value from config
    char* noFltMsg  = amap["NoFaultMsg"]->getcVal();
    char* fltDest   = amap["FaultDestination"]->getcVal();
    char* noAlrmMsg = amap["NoAlarmMsg"]->getcVal();
    char* alrmDest  = amap["AlarmDestination"]->getcVal();

    // Initialize the destination, normal state, and the current/last value (default values related to the alarm assetVar)
    std::string avVal       = fmt::format("{}", av->getcVal() ? av->getcVal() : noAlrmMsg);
    std::string avLVal      = fmt::format("{}", av->getcLVal() ? av->getcLVal() : noAlrmMsg);
    std::string almsg       = fmt::format("{}", av->getcVal() ? av->getcVal() : noAlrmMsg);
    std::string dest        = fmt::format("{}", alrmDest);
    std::string normalState = fmt::format("{}", noAlrmMsg);

    // If we have a fault variable, reassign the destination and normal state variables to the values related to the fault variable
    if (av->gotParam("type") && !strcmp(av->getcParam("type"), "fault"))
    {
        avVal       = fmt::format("{}", av->getcVal() ? av->getcVal() : noFltMsg);
        avLVal      = fmt::format("{}", av->getcLVal() ? av->getcLVal() : noFltMsg);
        dest        = fmt::format("{}", fltDest); 
        normalState = fmt::format("{}", noFltMsg);
    }

    // If the assetVar has been set to Clear, then proceed to clear all alarm/fault msgs and restore state
    if (avVal == "Clear")
    {
        vm->clearAlarms(vmap, dest.c_str());

        // Reset alarm/fault counter
        if (amap["FaultCnt"])
            amap["FaultCnt"]->setVal(0);
        if (amap["AlarmCnt"])
            amap["AlarmCnt"]->setVal(0);

        // Reset the state of the fault/alarm assetVars (not including the clear fault/alarm assetVar)
        AlarmFaultHandlerUtils::clearAlarms(vmap, amap, aname, av);

        // If we are the asset manager, clear all alarms/faults for asset managers and instances
        if (strcmp(aname, av->am->name.c_str()) == 0)
        {
            // Iterate through all asset managers in the asset map and clear faults/alarms for each manager
            for (auto& pair : av->am->assetManMap)
            {
                asset_manager* ami = pair.second;

                // To clear asset manager alarms/faults, we'll require that the name of the variable responsible for clearing alarms/faults
                // will be the same for both the asset manager and the asset instance (ex.: /faults/[asset]:clear_faults, /alarms/[asset]:clear_alarms)
                const std::string vmapKey = fmt::format("{}/{}", av->comp.substr(0, av->comp.find_last_of("/")), ami->name);
                FPS_PRINT_DEBUG("vmapKey {} for ami {}", vmapKey, ami->name);
                varmap avMap = vmap[vmapKey];
                
                if (avMap[av->name])
                {
                    FPS_PRINT_DEBUG("asset manager [{}] under parent asset manager [{}] has [{}] variable. Clearing alarms/faults for asset manager"
                                    , ami->name, av->am->name, av->name);

                    avMap[av->name]->setVal("Clear");
                    process_sys_alarm(vmap, ami->amap, ami->name.c_str(), p_fims, avMap[av->name]);
                }
            }
            
            // Iterate through all asset instances in the asset map and clear faults/alarms for each instance
            for (auto& pair : av->am->assetMap)
            {
                asset* ai = pair.second;

                // To clear asset instance alarms/faults, we'll require that the name of the variable responsible for clearing alarms/faults
                // will be the same for both the asset manager and the asset instance (ex.: /faults/[asset]:clear_faults, /alarms/[asset]:clear_alarms)
                const std::string vmapKey = fmt::format("{}/{}", av->comp.substr(0, av->comp.find_last_of("/")), ai->name);
                FPS_PRINT_DEBUG("vmapKey {} for ai {}", vmapKey, ai->name);
                varmap avMap = vmap[vmapKey];
                
                if (avMap[av->name])
                {
                    FPS_PRINT_DEBUG("asset instance [{}] under asset manager [{}] has [{}] variable. Clearing alarms/faults for asset instance"
                                    , ai->name, av->am->name, av->name);

                    avMap[av->name]->setVal("Clear");
                    process_sys_alarm(vmap, ai->amap, ai->name.c_str(), p_fims, avMap[av->name]);
                }
            }
        }

        if (av->gotParam("type") && strcmp(av->getcParam("type"), "fault") == 0)
        {
            // Faults should be cleared now, so reset fault system also
            ESSUtils::setAvStatus(vmap, amap, aname, av, "FaultShutdown", false, "FaultShutdown", false);
        }

        // Set a status variable indicating that alarms/faults have been cleared
        ESSUtils::setAvStatus(vmap, amap, aname, av, "ClearFaultsDone", true, "ClearFaultsDone", false);
    }

    // If the alarm/fault assetVar has a value change, check to see if that value is a new alarm/fault state
    else if (avVal != avLVal)
    {
        av->setLVal(avVal.c_str());
    
        // If we have an alarm/fault, report it
        if (avVal != normalState)
        {
            tm* local_tm = vm->get_local_time_now();
            const std::string msg = fmt::format("[{}] received [{}] at [{}]", av->name, avVal, strtime(local_tm));

            vm->sendAlarm(vmap, av, dest.c_str(), nullptr, msg.c_str(), 2);

            const auto now = flex::get_time_dbl();
            if (av->gotParam("type") && !strcmp(av->getcParam("type"), "fault"))
            {
                // Send fault info to log file
                ESSLogger::get().critical("[{}] {}", aname, msg);

                // Send fault info to events
                const std::string source = ESSUtils::getEventSourceName(vm, vmap, aname);
                av->sendEvent(source.c_str(), p_fims, Severity::Fault, msg.c_str(), av->name.c_str(), now.count());
                FPS_PRINT_DEBUG("Fault Sent dest [{}] msg [{}] am {}", dest, msg, fmt::ptr(av->am));

                // Proceed to shutdown system due to fault condition
                ESSUtils::setAvStatus(vmap, amap, aname, av, "FaultShutdown", true, "FaultShutdown", true);

                // Increment # of faults
                amap["FaultCnt"]->addVal(1);
            }
            else
            {
                // Send alarm info to log file
                ESSLogger::get().warn("[{}] {}", aname, msg);

                // Send alarm info to events
                const std::string source = ESSUtils::getEventSourceName(vm, vmap, aname);
                av->sendEvent(source.c_str(), p_fims, Severity::Alarm, msg.c_str(), av->name.c_str(), now.count());
                FPS_PRINT_DEBUG("Alarm Sent dest [{}] msg [{}] am {}", dest}, msg, fmt::ptr(av->am));

                // Increment # of alarms
                amap["AlarmCnt"]->addVal(1);
            }
        }
        FPS_PRINT_DEBUG("dest [{}] msg [{}] am {}", dest, msg, fmt::ptr(av->am));
    }

    return 0;
}

#endif