#ifndef ALARMFAULTSHANDLER_CPP
#define ALARMFAULTSHANDLER_CPP

#include "asset.h"
//#include "../funcs/UpdateSysTime.cpp"
//#include "../funcs/SendClearFaultCmd.cpp"
char* strtime(const struct tm* timeptr);
int SendClearFaultCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int UpdateSysTime(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* Av);
extern "C" {
int process_alarm(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int process_fault(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int process_status(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}

// still all a bit flaky
// query with  /usr/local/bin/fims/fims_send -m get -r /$$ -u
// /ess/full/assets/bms/summary/alarms | jq set with
// /usr/local/bin/fims/fims_send -m pub -u /components/catl_mbmu_sum_r
// '{"mbmu_warning_11": 1234}' and this .. fims_send -m set -r /$$ -u
// /ess/warn/bms/alarms '"Clear"'
/**
 * @brief Looks for any changes to the warning state of the pcs, sends out an
 * alarm if the pcs is sending out fault values, and proceeds to shutdown the
 * system due to fault
 *
 * @param vmap the global data map
 * @param av the asset var we are checking the fault state of
 */
int process_fault(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    if (1)
        FPS_ERROR_PRINT(" %s >>  av %p  av->am %p \n", __func__, (void*)av, av ? (void*)av->am : nullptr);

    if (av)
    {
        char* dest = nullptr;
        char* msg = nullptr;
        char* avVal = av->getcVal() ? av->getcVal() : (char*)"noval";
        char* avLVal = av->getcLVal() ? av->getcLVal() : (char*)"noLval";
        if (!av->am)
        {
            FPS_ERROR_PRINT(" %s >>  ERROR  name [%s]  no associated AM \n", __func__, av->name.c_str());
            return 0;
        }
        VarMapUtils* vm = av->am->vm;

        if (0)
            FPS_ERROR_PRINT(" %s >>  name [%s] value [%s]  lastValue[%s] \n", __func__, av->name.c_str(), avVal,
                            avLVal);

        // Grab fault destination and no fault string value from config
        char* noFltMsg = (char*)"(NOFLTS) No faults";
        char* fltDest = (char*)(std::string("/assets/") + av->am->name.c_str() + "/summary:faults").c_str();

        if (!av->am->amap["FaultDestination"])
            av->am->amap["FaultDestination"] = vm->setLinkVal(vmap, av->am->name.c_str(), "/config", "FaultDestination",
                                                              fltDest);
        if (!av->am->amap["FaultDestination"]->getcVal())
            av->am->amap["FaultDestination"]->setVal(noFltMsg);

        if (!av->am->amap["NoFaultMsg"])
            av->am->amap["NoFaultMsg"] = vm->setLinkVal(vmap, av->am->name.c_str(), "/config", "NoFaultMsg", noFltMsg);
        if (!av->am->amap["NoFaultMsg"]->getcVal())
            av->am->amap["NoFaultMsg"]->setVal(noFltMsg);

        if (0)
            FPS_ERROR_PRINT("%s >> Fault Destination [%s]\n", __func__,
                            av->am->amap["FaultDestination"]->getcVal() ? av->am->amap["FaultDestination"]->getcVal()
                                                                        : "did not find val from config");
        if (0)
            FPS_ERROR_PRINT("%s >> No fault msg [%s]\n", __func__,
                            av->am->amap["NoFaultMsg"]->getcVal() ? av->am->amap["NoFaultMsg"]->getcVal()
                                                                  : "did not find val from config");

        char* almsg = av->getcVal() ? av->getcVal() : noFltMsg;
        noFltMsg = av->am->amap["NoFaultMsg"]->getcVal();
        fltDest = av->am->amap["FaultDestination"]->getcVal();

        if (almsg && (strcmp(almsg, "Clear") == 0))
        {
            asprintf(&dest, fltDest, av->am->name.c_str());
            vm->clearAlarms(vmap, dest);
            if (1)
                FPS_ERROR_PRINT(" %s >> Clear Faults dest [%s]   \n", __func__, dest);

            // Faults should be cleared now, so reset fault system also
            if (!av->am->amap["FaultShutdown"])
            {
                bool bval = false;
                av->am->amap["FaultShutdown"] = vm->setLinkVal(vmap, "ess", "/status", "FaultShutdown", bval);
            }
            if (av->am->amap["FaultShutdown"]->getbVal())
                av->am->amap["FaultShutdown"]->setVal(false);

            // Send clear fault cmd to devices here
            SendClearFaultCmd(vmap, av->am->amap, av->am->name.c_str(), av->am->p_fims, av);
        }
        else if (strcmp(avVal, avLVal) != 0)
        {
            av->setLVal((const char*)avVal);

            // If we are not in a fault state, we probably don't want to send out an
            // alarm and shutdown the system Hardcoded at the moment...
            if (strcmp(avVal, noFltMsg) != 0)
            {
                // double tNow = vm->get_time_dbl();
                std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
                time_t tnow = std::chrono::system_clock::to_time_t(now);
                tm* local_tm = localtime(&tnow);

                asprintf(&dest, fltDest, av->am->name.c_str());
                asprintf(&msg, "%s fault  [%s] at %s", av->name.c_str(), almsg, strtime(local_tm));
                // asprintf(&msg, "%s fault  [%s] at %2.3f ", av->name.c_str(), almsg,
                // tNow);
                if (av->am && av->am->vm)
                {
                    vm->sendAlarm(vmap, av, dest, nullptr, msg, 2);
                    if (1)
                        FPS_ERROR_PRINT(" %s >> Fault Sent dest [%s] msg [%s]  am %p \n", __func__, dest, msg,
                                        (void*)av->am);

                    // Proceed to shutdown system due to fault condition
                    if (!av->am->amap["FaultShutdown"])
                    {
                        // May need to change "ess" to "pcs" if pcs manager can also
                        // initiate shutdown process Right now, ShutdownSystem function is
                        // called from ess wake up
                        bool bval = true;
                        av->am->amap["FaultShutdown"] = vm->setLinkVal(vmap, "ess", "/status", "FaultShutdown", bval);
                    }
                    if (!av->am->amap["FaultShutdown"]->getbVal())
                        av->am->amap["FaultShutdown"]->setVal(true);
                }
                if (0)
                    FPS_ERROR_PRINT(" %s >> dest [%s] msg [%s]  am %p \n", __func__, dest, msg, (void*)av->am);
            }
        }
        if (dest)
            free((void*)dest);
        if (msg)
            free((void*)msg);
    }
    else
    {
        FPS_ERROR_PRINT(" %s >> running No av !!\n", __func__);
    }
    return 0;
}

// still all a bit flaky
// query with  /usr/local/bin/fims/fims_send -m get -r /$$ -u
// /ess/full/assets/bms/summary/alarms | jq set with
// /usr/local/bin/fims/fims_send -m pub -u /components/catl_mbmu_sum_r
// '{"mbmu_warning_11": 1234}' and this .. fims_send -m set -r /$$ -u
// /ess/warn/bms/alarms '"Clear"'
/**
 * @brief Looks for any changes to the warning state of the pcs and sends out an
 * alarm if the pcs is sending out warning values
 *
 * @param vmap the global data map
 * @param av the asset var we are checking the warning state of
 */
int process_alarm(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    if (1)
        FPS_ERROR_PRINT(" %s >>  av %p  av->am %p \n", __func__, (void*)av, av ? (void*)av->am : nullptr);
    if (!av->am)
    {
        FPS_ERROR_PRINT(" %s >>  ERROR  name [%s]  no associated AM \n", __func__, av->name.c_str());
        return 0;
    }
    if (av)
    {
        char* dest = nullptr;
        char* msg = nullptr;
        char* avVal = av->getcVal() ? av->getcVal() : (char*)"noval";
        char* avLVal = av->getcLVal() ? av->getcLVal() : (char*)"noLval";
        VarMapUtils* vm = av->am->vm;

        if (0)
            FPS_ERROR_PRINT(" %s >>  name [%s] value [%s]  lastValue[%s] \n", __func__, av->name.c_str(), avVal,
                            avLVal);

        // Grab fault destination and no fault string value from config
        char* noAlrmMsg = (char*)"(NOWRN) No warnings";
        char* alrmDest = (char*)(std::string("/assets/") + av->am->name.c_str() + "/summary:alarms").c_str();

        if (!av->am->amap["AlarmDestination"])
            av->am->amap["AlarmDestination"] = vm->setLinkVal(vmap, av->am->name.c_str(), "/config", "AlarmDestination",
                                                              alrmDest);
        if (!av->am->amap["AlarmDestination"]->getcVal())
            av->am->amap["AlarmDestination"]->setVal(noAlrmMsg);

        if (!av->am->amap["NoAlarmMsg"])
            av->am->amap["NoAlarmMsg"] = vm->setLinkVal(vmap, av->am->name.c_str(), "/config", "NoAlarmMsg", noAlrmMsg);
        if (!av->am->amap["NoAlarmMsg"]->getcVal())
            av->am->amap["NoAlarmMsg"]->setVal(noAlrmMsg);

        if (0)
            FPS_ERROR_PRINT("%s >> Alarm Destination [%s]\n", __func__,
                            av->am->amap["AlarmDestination"]->getcVal() ? av->am->amap["AlarmDestination"]->getcVal()
                                                                        : "did not find val from config");
        if (0)
            FPS_ERROR_PRINT("%s >> No alarm msg [%s]\n", __func__,
                            av->am->amap["NoAlarmMsg"]->getcVal() ? av->am->amap["NoAlarmMsg"]->getcVal()
                                                                  : "did not find val from config");

        char* almsg = av->getcVal() ? av->getcVal() : noAlrmMsg;
        noAlrmMsg = av->am->amap["NoAlarmMsg"]->getcVal();
        alrmDest = av->am->amap["AlarmDestination"]->getcVal();

        if (almsg && (strcmp(almsg, "Clear") == 0))
        {
            asprintf(&dest, "/assets/%s/summary:alarms", av->am->name.c_str());
            vm->clearAlarms(vmap, dest);
            if (1)
                FPS_ERROR_PRINT(" %s >> ClearAlarms dest [%s]   \n", __func__, dest);
        }
        else if (strcmp(avVal, avLVal) != 0)
        {
            av->setLVal((const char*)avVal);

            // If we are not in a warning state, we probably don't want to send out an
            // alarm Hardcoded at the moment...
            if (strcmp(avVal, noAlrmMsg) != 0)
            {
                // double tNow = vm->get_time_dbl();
                std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
                time_t tnow = std::chrono::system_clock::to_time_t(now);
                tm* local_tm = localtime(&tnow);

                asprintf(&dest, alrmDest, av->am->name.c_str());
                asprintf(&msg, "%s alarm  [%s] at %s", av->name.c_str(), almsg, strtime(local_tm));
                // asprintf(&msg, "%s alarm  [%s] at %2.3f ", av->name.c_str(), almsg,
                // tNow);
                if (av->am && av->am->vm)
                {
                    // see if it works now
                    if (1)
                        vm->sendAlarm(vmap, av, dest, nullptr, msg, 2);
                    if (0)
                        FPS_ERROR_PRINT(" %s >> AlarmSent dest [%s] msg [%s]  am %p \n", __func__, dest, msg,
                                        (void*)av->am);
                }
                // av->am->vm->sendAlarm(vmap, "smbu", dest, nullptr, msg, 2);
                if (0)
                    FPS_ERROR_PRINT(" %s >> dest [%s] msg [%s]  am %p \n", __func__, dest, msg, (void*)av->am);
            }
        }
        if (dest)
            free((void*)dest);
        if (msg)
            free((void*)msg);
    }
    else
    {
        FPS_ERROR_PRINT(" %s >> running No av !!\n", __func__);
    }
    return 0;
}

/**
 * @brief Looks for any changes to the pcs status
 *
 * @param vmap the global data map
 * @param av the asset var we are checking the status of
 */
int process_status(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    if (0)
        FPS_ERROR_PRINT(" %s >>  av %p  av->am %p \n", __func__, (void*)av, av ? (void*)av->am : nullptr);
    if (!av->am)
    {
        FPS_ERROR_PRINT(" %s >>  ERROR  name [%s]  no associated AM \n", __func__, av->name.c_str());
        return 0;
    }
    if (av)
    {
        char* avVal = av->getcVal() ? av->getcVal() : (char*)"noval";
        char* avLVal = av->getcLVal() ? av->getcLVal() : (char*)"noLval";
        // VarMapUtils* vm = av->am->vm;

        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        time_t tnow = std::chrono::system_clock::to_time_t(now);
        tm* local_tm = localtime(&tnow);

        if (1)
            FPS_ERROR_PRINT(" %s >>  name [%s] value [%s]  lastValue[%s] at %s\n", __func__, av->name.c_str(), avVal,
                            avLVal, strtime(local_tm));

        if (strcmp(avVal, avLVal) != 0)
            av->setLVal((const char*)avVal);

        // Add additional tasks for pcs status here
    }
    else
    {
        FPS_ERROR_PRINT(" %s >> running No av !!\n", __func__);
    }
    return 0;
}
#endif