#ifndef FAULTFUNCS_CPP
#define FAULTFUNCS_CPP

#include <chrono>
#include <string>

#include "asset.h"
#include "varMapUtils.h"
//#include "testUtils.h"
//#include "funcRef.h"
#include "systemEnums.h"

/**
 * @brief
 *
 * Make sure that func arguments are correct.
 *      - Might need error checking
 *      - (check to make sure that config is correct, so you aren't calling a
 * command that doesn't exist for that asset/amap)
 *
 * Need to check how many registers we can keep track of (what modbus registers
 * to change to elicit certain sequences)
 *      - amap reference might not be enough.
 *
 *
 */

extern "C++" {
int ShutdownPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int ShutdownBMS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int RunKeyCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int CheckGPIO(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}

int ShutdownPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    // Timer ShutdownBMSTimer("ShutdownBMSTimer", true);

    int reload = 0;
    bool bval = false;
    int ival = 0;
    double dval = 0.0;
    char* cval = (char*)"";
    char* tValInit = (char*)"Init";
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    char* essName = vm->getSysName(vmap);
    double tNow = vm->get_time_dbl();
    essPerf ePerf(am, aname, __func__);

    bool logging_enabled = getLoggingEnabled(vmap, *aV->am->vm);
    char* LogDir = getLogDir(vmap, *aV->am->vm);

    // char* tVal = (char*)"Test TimeStamp";
    // in future use tval for sending messages out - for UI and alert purposes.

    assetVar* pcsAv = amap[__func__];  // Need to figure out
                                       // the resolution here!
    // FPS_ERROR_PRINT("%s /// pcsAv->getiVal()
    // %d\n",__func__,pcsAv->getiVal());
    if (!pcsAv || (reload = pcsAv->getiVal()) == 0)
    {
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }

    if (reload == 0)
    {
        // reload
        linkVals(*vm, vmap, amap, aname, "/reload", reload, __func__);
        pcsAv = amap[__func__];  // Need to figure out the
                                 // resolution here!
        linkVals(*vm, vmap, amap, aname, "/reload", reload, "pcsRunKeyCmd");

        // Statuses, etc.:
        // linkVals(*vm, vmap, amap, essName, "/status", tValInit,
        // "SystemStateStep");
        linkVals(*vm, vmap, amap, essName, "/status", bval, "PCSShutdown",
                 "ShutdownBMS");  //, "ShutdownCmd", "HardShutdown");
        linkVals(*vm, vmap, amap, essName, "/controls", dval, "ActivePowerSetpoint", "ReactivePowerSetpoint");
        linkVals(*vm, vmap, amap, aname, "/sched", cval, "schedShutdownPCS");

        linkVals(*vm, vmap, amap, aname, "/status", tValInit, "SystemState", "PCSSystemStateStep");
        linkVals(*vm, vmap, amap, aname, "/status", cval, "PCSStatusResp");
        linkVals(*vm, vmap, amap, aname, "/status", dval, "PCSShutdownTime");
        linkVals(*vm, vmap, amap, aname, "/status", bval, "HardShutdown", "FullShutdown");
        linkVals(*vm, vmap, amap, aname, "/status", ival, "KeyCmd", "PCSStopKeyCmd", "PCSEStopKeyCmd");
        linkVals(*vm, vmap, amap, aname, "/controls", ival, "PCSRun");

        linkVals(*vm, vmap, amap, "bms", "/status", dval, "BMSCurrentCheckStop");

        linkVals(*vm, vmap, amap, aname, "/config", dval, "maxShutdownTime", "maxKeyCmdOnTime", "maxKeyCmdTime");
        linkVals(*vm, vmap, amap, aname, "/config", ival, "maxKeyCmdTries");

        linkVals(*vm, vmap, amap, aname, "/faults", cval, "sys_fault");
        if (1)
            FPS_ERROR_PRINT(
                "%s >>>>>>>>>>>>>>>>PCS Shutdown system step 1 reload "
                "[%d] at time: %f \n",
                __func__, reload, tNow);

        pcsAv->setVal(1);
        return 0;
    }
    if (reload < 2)
    {
        amap["PCSSystemStateStep"]->setVal(tValInit);
        amap["PCSShutdown"]->setVal(false);
        amap["PCSShutdown"]->setParam("Fault", false);
        amap["PCSShutdownTime"]->setVal(tNow);

        if (!amap["HardShutdown"])
            amap["HardShutdown"]->setVal(false);
        amap["HardShutdown"]->setParam("Latch", amap["HardShutdown"]->getbVal());
        if (!amap["FullShutdown"])
            amap["FullShutdown"]->setVal(false);
        amap["FullShutdown"]->setParam("Latch", false);
        amap["FullShutdown"]->setParam("WaitingDCCurrent", false);
        amap["FullShutdown"]->setParam("WaitingDCCurrentDone", false);

        amap["KeyCmd"]->setParam("maxKeyCmdOnTime", amap["maxKeyCmdOnTime"]->getdVal());
        amap["KeyCmd"]->setParam("maxKeyCmdTime", amap["maxKeyCmdTime"]->getdVal());
        amap["KeyCmd"]->setParam("maxKeyCmdTries", amap["maxKeyCmdTries"]->getiVal());
        if (1)
            FPS_ERROR_PRINT(
                "%s >>>>>>>>>>>>>>>>PCS Shutdown system state [%s] "
                "reload [%d] APS: [%f] at time: %f \n",
                __func__, amap["SystemState"]->getcVal(), reload, amap["ActivePowerSetpoint"]->getdVal(), tNow);
        FPS_ERROR_PRINT("HardShutdown: %d\n", amap["HardShutdown"]->getbVal());

        pcsAv->setVal(2);
    }

    assetVar* PCSKeyCmd = amap["PCSStopKeyCmd"];
    if (amap["HardShutdown"]->getbParam("Latch"))
    {
        PCSKeyCmd = amap["PCSEStopKeyCmd"];
    }

    char* SystemState = amap["SystemState"]->getcVal();
    char* PCSSystemStateStep = amap["PCSSystemStateStep"]->getcVal();
    if (0)
        FPS_ERROR_PRINT(
            "%s >> Running reload %d state [%s] step [%s] "
            "FullShutdownLatch [%d] at time: %f\n",
            __func__, reload, SystemState, PCSSystemStateStep, amap["FullShutdown"]->getbParam("Latch"), tNow);

    char* PCSStatusResp = amap["PCSStatusResp"]->getcVal();

    if (!PCSStatusResp)  // comms OK as well?
    {
        amap["PCSShutdown"]->setParam("Fault", true);
        PCSSystemStateStep = (char*)"PCS Shutdown Communication Failed";
        amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
        if (1)
            FPS_ERROR_PRINT("%s >> No communication with PCS at time: %f \n", __func__, tNow);
        // Send Event - fault
        const auto now = flex::get_time_dbl();

        ESSLogger::get().critical("[PCS comms lost]");
        if (logging_enabled)
        {
            std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "PCSCommsLost", "txt");
            ESSLogger::get().logIt(dirAndFile);
        }

        assetVar* temp_av = amap["PCSShutdown"];
        if (temp_av)
            temp_av->sendEvent("PCS", am->p_fims, Severity::Fault, "PCS comms lost at time %2.3f", now.count());

        vm->schedStop(vmap, amap, "schedShutdownPCS", 1);
        pcsAv->setVal(1);

        return 0;
    }

    // SystemState holds state from PCS decoded into either Ready, Starting, Ready
    // and Shutdown
    if (amap["SystemState"]->valueChangedReset() || amap["HardShutdown"]->getbVal() ||
        !strcmp(PCSSystemStateStep, "Init"))
    {
        amap["pcsRunKeyCmd"]->setVal(0);  // Reset key cmd function
        RunKeyCmd(vmap, amap, aname, NULL, aV);
        amap["PCSSystemStateStep"]->setVal(SystemState);
        amap["HardShutdown"]->setVal(false);
        if (1)
            FPS_ERROR_PRINT("%s >>> #1 State step changed to %s\n", __func__, SystemState);
    }

    // if (1) FPS_ERROR_PRINT("%s >>> APS DEADBAND %f\n",
    // __func__, amap["ActivePowerSetpoint"]->dbV,
    // amap["ReactivePowerSetpoint"]->dbV.value());
    if (std::abs(amap["ActivePowerSetpoint"]->getdVal()) > amap["ActivePowerSetpoint"]->getDbVal() ||
        std::abs(amap["ReactivePowerSetpoint"]->getdVal()) > amap["ActivePowerSetpoint"]->getDbVal())
    {
        const auto now = flex::get_time_dbl();
        if (1)
            FPS_ERROR_PRINT("%s >> Sending 0kW 0kVAr to PCS at time %f\n", __func__, tNow);
        ESSLogger::get().info("Sending 0kW 0kVAr to PCS");
        assetVar* temp_av = amap["PCSShutdown"];
        if (temp_av)
            temp_av->sendEvent("PCS", am->p_fims, Severity::Info, "Sending 0kW 0kVAr to PCS at time %2.3f",
                               now.count());
        // amap["FullShutdown"]->setParam("WaitingDCCurrent", true);
        amap["ActivePowerSetpoint"]->setVal(0.0);
        amap["ReactivePowerSetpoint"]->setVal(0.0);
    }

    if ((strcmp(SystemState, "Running") == 0 || strcmp(SystemState, "Ready") == 0) &&
        ((tNow - amap["PCSShutdownTime"]->getdVal()) > amap["maxShutdownTime"]->getdVal()))
    {
        if (amap["HardShutdown"]->getbParam("Latch"))
        {
            amap["PCSShutdown"]->setVal(true);
            amap["PCSShutdown"]->setParam("Fault", true);
            PCSSystemStateStep = (char*)"PCS Failed Shutdown";
            amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
            if (1)
                FPS_ERROR_PRINT("%s >> PCS stuck in hold state %s at time: %f \n", __func__, PCSStatusResp, tNow);
            // Send Event - fault
            const auto now = flex::get_time_dbl();

            ESSLogger::get().critical("PCS stuck in hold state [{}]", PCSStatusResp);
            if (logging_enabled)
            {
                std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "PCSStuckInHold", "txt");
                ESSLogger::get().logIt(dirAndFile);
            }

            assetVar* temp_av = amap["PCSShutdown"];
            if (temp_av)
                temp_av->sendEvent("PCS", am->p_fims, Severity::Fault, "PCS stuck in hold state [%s] at time %2.3f",
                                   PCSStatusResp, now.count());

            vm->schedStop(vmap, amap, "schedShutdownPCS", 1);
            pcsAv->setVal(1);
        }
        else if (!amap["FullShutdown"]->getbParam("WaitingDCCurrent"))
        {
            amap["HardShutdown"]->setVal(true);
            amap["HardShutdown"]->setParam("Latch", true);
            PCSSystemStateStep = (char*)"PCS Starting Hard Shutdown";
            amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
            amap["PCSShutdownTime"]->setVal(tNow);
            if (1)
                FPS_ERROR_PRINT(
                    "%s >> PCS stuck in hold state %s, trying e-stop and "
                    "resetting PCSShutdown timer at time: %f \n",
                    __func__, PCSStatusResp, tNow);
            // Send Event - Alarm
            const auto now = flex::get_time_dbl();
            ESSLogger::get().warn("PCS stuck in hold state [{}], trying estop", PCSStatusResp);
            assetVar* temp_av = amap["ShutdownCmd"];
            if (temp_av)
                temp_av->sendEvent("PCS", am->p_fims, Severity::Alarm,
                                   "PCS stuck in hold state [%s], trying estop at time %2.3f", PCSStatusResp,
                                   now.count());
        }
        else
        {
            PCSSystemStateStep = (char*)"Timed out waiting for DC current to reduce";
            amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
            amap["PCSShutdownTime"]->setVal(tNow);
            if (1)
                FPS_ERROR_PRINT("%s >> PCS Current still above shutdown threshold at time: %f\n", __func__, tNow);
            // Send Event - info
            const auto now = flex::get_time_dbl();
            ESSLogger::get().info("[PCS Current holding above threshold]");
            assetVar* temp_av = amap["PCSShutdown"];
            if (temp_av)
                temp_av->sendEvent("PCS", am->p_fims, Severity::Info,
                                   "PCS Current holding above threshold at time %2.3f", now.count());
            amap["FullShutdown"]->setParam("WaitingDCCurrent", false);
            amap["FullShutdown"]->setParam("WaitingDCCurrentDone", true);
        }
    }
    else if (!amap["HardShutdown"]->getbParam("Latch") && !amap["FullShutdown"]->getbParam("Latch") &&
             !amap["FullShutdown"]->getbParam("WaitingDCCurrentDone"))
    {
        if (amap["BMSCurrentCheckStop"]->getbParam("seenMaxAlarm") ||
            amap["BMSCurrentCheckStop"]->getbParam("seenMinAlarm"))
        {
            if (!amap["FullShutdown"]->getbParam("WaitingDCCurrent"))
            {
                double Idc = amap["BMSCurrentCheckStop"]->getdVal();
                if (1)
                    FPS_ERROR_PRINT("%s >> PCS Stop waiting for [DC Current %f A] to decrease\n", __func__, Idc);
                const auto now = flex::get_time_dbl();
                ESSLogger::get().warn("PCS Stop waiting for [DC Current {} A] to decrease", Idc);
                assetVar* temp_av = amap["PCSShutdown"];
                if (temp_av)
                    temp_av->sendEvent("PCS", am->p_fims, Severity::Alarm,
                                       "PCS Stop waiting for [DC Current %f A] to "
                                       "decrease at time %2.3f",
                                       Idc, now.count());
                amap["FullShutdown"]->setParam("WaitingDCCurrent", true);
            }
        }
        else
        {
            PCSSystemStateStep = (char*)"PCS Current below shutdown threshold";
            amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
            if (1)
                FPS_ERROR_PRINT("%s >> PCS Current below shutdown threshold at time: %f\n", __func__, tNow);
            // Send Event - info
            const auto now = flex::get_time_dbl();
            ESSLogger::get().info("[PCS Current below turnoff threshold]");
            assetVar* temp_av = amap["PCSShutdown"];
            if (temp_av)
                temp_av->sendEvent("PCS", am->p_fims, Severity::Info,
                                   "PCS Current below turnoff threshold at time %2.3f", now.count());
            amap["FullShutdown"]->setParam("WaitingDCCurrent", false);
            amap["FullShutdown"]->setParam("WaitingDCCurrentDone", true);
        }
    }
    else if (strcmp(SystemState, "Off") == 0 || strcmp(SystemState, "Fault") == 0)
    {
        amap["PCSShutdown"]->setVal(true);
        PCSSystemStateStep = (char*)"PCS Shutdown Complete";
        amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
        if (1)
            FPS_ERROR_PRINT("%s >> All PCS's succesfully shut down at time: %f \n", __func__, tNow);

        if (amap["FullShutdown"]->gotParam("ShutdownBMS") && amap["FullShutdown"]->getbParam("ShutdownBMS"))
        {
            FPS_ERROR_PRINT("%s >>> Going to shut down BMS\n", __func__);
            bool ShutdownBMS = true;
            vm->setVal(vmap, amap["ShutdownBMS"]->comp.c_str(), amap["ShutdownBMS"]->name.c_str(), ShutdownBMS);
            amap["FullShutdown"]->setParam("ShutdownBMS", false);
        }
        // Send Event - info
        const auto now = flex::get_time_dbl();
        char* ftype = amap["sys_fault"]->getcVal();

        // PSW 10.1 must have a default fault type
        if (ftype == nullptr)
        {
            // 10.1 bug if the config is not correct
            FPS_ERROR_PRINT("%s >>> sys_fault NOT defined \n", __func__);
            ftype = (char*)"(DEF) Default fault created";
            amap["sys_fault"]->setVal(ftype);
        }

        if (strcmp(amap["sys_fault"]->getcVal(), "(NOFLTS) No faults"))
        {
            // todo: this kinda cheats, a little, npos check is in, but they can still
            // give me an empty string or a single character
            std::string faultType{ amap.at("sys_fault")->getcVal() };
            auto pos = faultType.find(')');
            if (pos != faultType.npos)
            {
                // caveat: requires that first character is "(" - will skip the first
                // character on this assumption
                faultType = faultType.substr(1, pos - 1);  // still a cheaty mccheatyface solution
            }
            else  // error -> no ")" found, can't get substring
            {
                faultType = "error_no_parenthesis";
            }

            ESSLogger::get().critical("PCS faulted with message [{}]", amap.at("sys_fault")->getcVal());
            if (logging_enabled)
            {
                std::string dirAndFile = fmt::format("{}/{}_{}.{}", LogDir, "PCSFault", faultType, "txt");
                ESSLogger::get().logIt(dirAndFile);
            }

            assetVar* temp_av = amap["PCSShutdown"];
            if (temp_av)
                temp_av->sendEvent("PCS", am->p_fims, Severity::Fault, "PCS faulted with message [%s] at time %2.3f",
                                   amap.at("sys_fault")->getcVal(), now.count());
        }
        else
        {
            ESSLogger::get().critical("PCS shutdown complete");
            if (logging_enabled)
            {
                std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "PCSShutdownComplete", "txt");
                ESSLogger::get().logIt(dirAndFile);
            }

            assetVar* temp_av = amap["PCSShutdown"];
            if (temp_av)
                temp_av->sendEvent("PCS", am->p_fims, Severity::Fault, "PCS shutdown complete at time %2.3f",
                                   now.count());
        }

        vm->schedStop(vmap, amap, "schedShutdownPCS", 0.001);
        pcsAv->setVal(1);
    }
    else if (strcmp(SystemState, "Ready") == 0 && !amap["HardShutdown"]->getbParam("Latch") &&
             !amap["FullShutdown"]->getbParam("Latch"))
    {
        if (amap["FullShutdown"]->getbVal())
        {
            amap["PCSRun"]->setVal(0);
            // vm->sendAssetVar(amap["PCSRun"], p_fims, amap["PCSRun"]->comp.c_str());
            varsmap* vlist = vm->createVlist();
            vm->addVlist(vlist, amap["PCSRun"]);
            vm->sendVlist(p_fims, "set", vlist);
            vm->clearVlist(vlist);
            if (1)
                FPS_ERROR_PRINT("%s >> Setting PCSRun to 0 at time %f\n", __func__, tNow);
        }
        else
        {
            PCSSystemStateStep = (char*)"PCS In Standby";
            amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
            if (1)
                FPS_ERROR_PRINT("%s >> PCS In Standby at time: %f. ShutdownBMS? [%d] \n", __func__, tNow,
                                amap["FullShutdown"]->getbParam("ShutdownBMS"));
            // Send Event - info
            const auto now = flex::get_time_dbl();
            ESSLogger::get().info("[PCS In Standby]");
            assetVar* temp_av = amap["PCSShutdown"];
            if (temp_av)
                temp_av->sendEvent("PCS", am->p_fims, Severity::Info, "PCS In Standby at time %2.3f", now.count());
            vm->schedStop(vmap, amap, "schedShutdownPCS", 0.001);
            pcsAv->setVal(1);
        }
    }
    else if (amap["FullShutdown"]->getbParam("Latch") && amap["PCSRun"]->getdVal() != 0)
    {
        if ((tNow - amap["PCSShutdownTime"]->getdVal()) > amap["maxShutdownTime"]->getdVal())
        {
            amap["PCSStartup"]->setParam("Fault", true);
            PCSSystemStateStep = (char*)"PCS Startup Failed";
            amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
            if (1)
                FPS_ERROR_PRINT("%s >> PCS Start command not setting: [%s] tnow %2.3f \n", __func__, PCSStatusResp,
                                tNow);
            // Send Event - fault
            const auto now = flex::get_time_dbl();

            ESSLogger::get().critical("PCS Start command not setting");
            if (logging_enabled)
            {
                std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "PCSStuckInHold", "txt");
                ESSLogger::get().logIt(dirAndFile);
            }

            assetVar* temp_av = amap["PCSStartup"];
            if (temp_av)
                temp_av->sendEvent("PCS", am->p_fims, Severity::Fault, "PCS Start command not setting at time %2.3f",
                                   now.count());

            vm->schedStop(vmap, amap, "schedStartupPCS", 1);
            pcsAv->setVal(1);
        }
        varsmap* vlist = vm->createVlist();
        amap["PCSRun"]->setVal(0);
        vm->addVlist(vlist, amap["PCSRun"]);
        vm->sendVlist(p_fims, "set", vlist);
        vm->clearVlist(vlist);
    }
    else if (amap["KeyCmd"]->getbParam("KeyCmdDone"))
    {
        if (amap["HardShutdown"]->getbParam("Latch"))
        {
            amap["PCSShutdown"]->setVal(true);
            amap["PCSShutdown"]->setParam("Fault", true);
            PCSSystemStateStep = (char*)"PCS Forced Shutdown";
            amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
            if (1)
                FPS_ERROR_PRINT("%s >> PCS not responding still in state %s at time: %f \n", __func__, PCSStatusResp,
                                tNow);
            // Send Event - fault
            const auto now = flex::get_time_dbl();

            ESSLogger::get().critical("PCS not responding in state [{}]", PCSStatusResp);
            if (logging_enabled)
            {
                std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "PCSNotResponding", "txt");
                ESSLogger::get().logIt(dirAndFile);
            }

            assetVar* temp_av = amap["PCSShutdown"];
            if (temp_av)
                temp_av->sendEvent("PCS", am->p_fims, Severity::Fault, "PCS not responding in state [%s] at time %2.3f",
                                   PCSStatusResp, now.count());

            vm->schedStop(vmap, amap, "schedShutdownPCS", 1);
            pcsAv->setVal(1);
        }
        else
        {
            amap["HardShutdown"]->setVal(true);
            amap["HardShutdown"]->setParam("Latch", true);
            PCSSystemStateStep = (char*)"PCS Starting Hard Shutdown";
            amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
            if (1)
                FPS_ERROR_PRINT("%s >> PCS not responding to stop, trying e-stop at time: %f \n", __func__, tNow);
            // Send Event - info, alarm?
            const auto now = flex::get_time_dbl();
            ESSLogger::get().warn("PCS not responding, [trying e-stop]");
            assetVar* temp_av = amap["PCSShutdown"];
            if (temp_av)
                temp_av->sendEvent("PCS", am->p_fims, Severity::Alarm,
                                   "PCS not responding, trying e-stop at time %2.3f", now.count());
        }
    }
    else if ((strcmp(SystemState, "Running") == 0 || strcmp(SystemState, "Ready") == 0 ||
              amap["HardShutdown"]->getbParam("Latch")) &&
             !amap["FullShutdown"]->getbParam("Latch"))
    {
        RunKeyCmd(vmap, amap, aname, NULL, aV);
    }

    if (amap["PCSShutdown"]->getbVal())
    {
        if (amap["FullShutdown"]->getbParam("Latch"))
        {
            amap["PCSShutdown"]->setVal(false);
            amap["FullShutdown"]->setParam("Latch", false);
            amap["FullShutdown"]->setVal(false);
        }
        if (amap["KeyCmd"]->getbVal())
        {
            amap["KeyCmd"]->setVal(false);
            FPS_ERROR_PRINT("%s > One last keycmd to false\n", __func__);
        }
    }

    if (amap["KeyCmd"]->valueChangedReset())
    {
        varsmap* vlist = vm->createVlist();
        if (amap["KeyCmd"]->getbVal())
        {
            PCSKeyCmd->setVal(1);
            vm->addVlist(vlist, PCSKeyCmd);
        }
        else
        {
            PCSKeyCmd->setVal(0);
            vm->addVlist(vlist, PCSKeyCmd);
        }
        vm->sendVlist(p_fims, "set", vlist);
        vm->clearVlist(vlist);
    }

    return 0;
}

// NOTE: Needs aname to be a particular bms_number - Might be enum instead.
// TODO: review ShutdownBMS BEFORE MVP Will just send out a single command to an
// MBMU register.
int ShutdownBMS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    int reload = 0;
    bool bval = false;
    int ival = 0;
    double dval = 0.0;
    char* cval = (char*)"";
    char* tValInit = (char*)"Init";
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    double tNow = vm->get_time_dbl();
    bool logging_enabled = getLoggingEnabled(vmap, *aV->am->vm);
    char* LogDir = getLogDir(vmap, *aV->am->vm);

    char* essName = vm->getSysName(vmap);

    essPerf ePerf(am, aname, __func__);

    assetVar* bmsAv = amap[__func__];  // Need to figure out
                                       // the resolution here!
    if (!bmsAv || (reload = bmsAv->getiVal()) == 0)
    {
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }

    if (reload == 0)
    {
        // reload
        linkVals(*vm, vmap, amap, aname, "/reload", reload, __func__);
        bmsAv = amap[__func__];  // Need to figure out the
                                 // resolution here!
        linkVals(*vm, vmap, amap, aname, "/reload", reload, "bmsRunKeyCmd");

        // Statuses, etc.:
        // linkVals(*vm, vmap, amap, essName, "/status", tValInit, "SystemState",
        // "SystemStateStep");
        linkVals(*vm, vmap, amap, essName, "/status", bval, "BMSShutdown");
        linkVals(*vm, vmap, amap, aname, "/sched", cval, "schedShutdownBMS");

        linkVals(*vm, vmap, amap, aname, "/status", tValInit, "BMSSystemState", "BMSSystemStateStep");
        linkVals(*vm, vmap, amap, aname, "/status", cval, "BMSPowerOn", "BMSStatus");
        linkVals(*vm, vmap, amap, aname, "/status", dval, "BMSShutdownTime");
        linkVals(*vm, vmap, amap, aname, "/status", ival, "KeyCmd", "BMSKeyCmd");

        linkVals(*vm, vmap, amap, aname, "/config", dval, "maxShutdownTime", "maxKeyCmdOnTime", "maxKeyCmdTime");
        linkVals(*vm, vmap, amap, aname, "/config", ival, "maxKeyCmdTries");

        bmsAv->setVal(1);
        return 0;
    }
    if (reload < 2)
    {
        amap["BMSSystemState"]->setVal(tValInit);
        amap["BMSSystemStateStep"]->setVal(tValInit);
        amap["BMSShutdownTime"]->setVal(tNow);

        amap["BMSShutdown"]->setVal(false);
        amap["BMSShutdown"]->setParam("Fault", false);

        amap["KeyCmd"]->setParam("maxKeyCmdOnTime", amap["maxKeyCmdOnTime"]->getdVal());
        amap["KeyCmd"]->setParam("maxKeyCmdTime", amap["maxKeyCmdTime"]->getdVal());
        amap["KeyCmd"]->setParam("maxKeyCmdTries", amap["maxKeyCmdTries"]->getiVal());

        amap["bmsRunKeyCmd"]->setVal(0);  // Reset key cmd function
        RunKeyCmd(vmap, amap, aname, NULL, aV);
        if (1)
            FPS_ERROR_PRINT(
                "%s >>>>>>>>>>>>>>>>BMS Shutdown system state [%s] "
                "reload %d at time: %f \n",
                __func__, amap["BMSSystemState"]->getcVal(), reload, tNow);

        bmsAv->setVal(2);
    }

    char* BMSSystemState = amap["BMSSystemState"]->getcVal();
    char* BMSSystemStateStep = amap["BMSSystemStateStep"]->getcVal();
    if (0)
        FPS_ERROR_PRINT("%s >> Running reload %d state [%s] step [%s]\n", __func__, reload, BMSSystemState,
                        BMSSystemStateStep);

    if (amap["BMSSystemState"]->valueChangedReset() || amap["BMSSystemStateStep"]->valueChangedReset())
    {
        if (0)
            FPS_ERROR_PRINT("%s >>  BMS State [%s] Step [%s] time: %f \n", __func__, BMSSystemState, BMSSystemStateStep,
                            tNow);
    }
    char* BMSStatus = amap["BMSStatus"]->getcVal();
    char* BMSPowerOn = amap["BMSPowerOn"]->getcVal();

    if (!BMSStatus || !BMSPowerOn)
    {
        amap["BMSShutdown"]->setParam("Fault", true);
        BMSSystemStateStep = (char*)"BMS Shutdown Communication Failed";
        amap["BMSSystemStateStep"]->setVal(BMSSystemStateStep);
        if (1)
            FPS_ERROR_PRINT("%s >> No communication with BMS at time: %f \n", __func__, tNow);
        // Send Event - fault
        const auto now = flex::get_time_dbl();

        ESSLogger::get().critical("[BMS comms lost]");
        if (logging_enabled)
        {
            std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "BMSCommsLost", "txt");
            ESSLogger::get().logIt(dirAndFile);
        }

        auto temp_av = amap["BMSShutdown"];
        if (temp_av)
            temp_av->sendEvent("BMS", am->p_fims, Severity::Fault, "BMS comms lost at time %2.3f", now.count());

        vm->schedStop(vmap, amap, "schedShutdownBMS", 1);

        bmsAv->setVal(1);
        return 0;
    }

    if (0)
        FPS_ERROR_PRINT("%s >> reload [%d] status [%s] PowerOn [%s] BMSSystemStateStep [%s]\n", __func__, reload,
                        BMSStatus, BMSPowerOn, BMSSystemStateStep);

    // can't check for changes in 2 statuses in 1 line...
    if (amap["BMSStatus"]->valueChangedReset() || !strcmp(BMSSystemStateStep, "Init"))
    {
        if (/*!strcmp(BMSStatus, "Warning") || */ !strcmp(BMSStatus, "Fault"))
        {
            BMSSystemStateStep = (char*)"Shutdown Failed";
            amap["BMSSystemStateStep"]->setVal(BMSSystemStateStep);
            if (1)
                FPS_ERROR_PRINT("%s >>> BMSStatus changed? %d State step changed to %s\n", __func__,
                                amap["BMSStatus"]->valueChangedReset(), BMSSystemStateStep);
        }
    }

    if ((amap["BMSPowerOn"]->valueChangedReset() || !strcmp(BMSSystemStateStep, "Init")) &&
        strcmp(BMSSystemStateStep, "Startup Failed"))
    {
        if (!strcmp(BMSPowerOn, "On Fault") || !strcmp(BMSPowerOn, "Off Fault"))
        {
            BMSSystemStateStep = (char*)"Shutdown Fault";
        }
        else if (!strcmp(BMSPowerOn, "On Ready"))
        {
            BMSSystemStateStep = (char*)"Ready";
            amap["bmsRunKeyCmd"]->setVal(0);  // Reset key cmd function
            RunKeyCmd(vmap, amap, aname, NULL, aV);
        }
        else if (!strcmp(BMSPowerOn, "Off Ready"))
        {
            BMSSystemStateStep = (char*)"Shutdown";
        }
        else if (!strcmp(BMSPowerOn, "Off Ready"))
        {
            BMSSystemStateStep = (char*)"Shutdown";
        }
        amap["BMSSystemStateStep"]->setVal(BMSSystemStateStep);
        if (1)
            FPS_ERROR_PRINT("%s >>> State step changed to %s\n", __func__, BMSSystemStateStep);
    }

    if (strcmp(BMSSystemStateStep, "Shutdown") == 0)
    {
        amap["BMSShutdown"]->setVal(true);
        BMSSystemStateStep = (char*)"BMS Shutdown Complete";
        amap["BMSSystemStateStep"]->setVal(BMSSystemStateStep);
        if (1)
            FPS_ERROR_PRINT("%s >> All BMS's succesfully Shutdown at time: %f \n", __func__, tNow);
        // Send Event - info
        const auto now = flex::get_time_dbl();
        ESSLogger::get().info("[All BMS shutdown successfully]");
        auto temp_av = amap["BMSShutdown"];
        if (temp_av)
            temp_av->sendEvent("BMS", am->p_fims, Severity::Info, "All BMS shutdown successfully at time %2.3f",
                               now.count());

        vm->schedStop(vmap, amap, "schedShutdownBMS", 1);
        bmsAv->setVal(1);
    }
    else if ((tNow - amap["BMSShutdownTime"]->getdVal()) > amap["maxShutdownTime"]->getdVal())
    {
        amap["BMSShutdown"]->setParam("Fault", true);
        BMSSystemStateStep = (char*)"BMS Shutdown Failed";
        amap["BMSSystemStateStep"]->setVal(BMSSystemStateStep);
        FPS_ERROR_PRINT("BMSShutdownTime %f maxShutdownTime %f\n", tNow - amap["BMSShutdownTime"]->getdVal(),
                        amap["maxShutdownTime"]->getdVal());
        if (1)
            FPS_ERROR_PRINT("%s >> BMS Shutdown timed out in hold state %s at time: %f \n", __func__, BMSPowerOn, tNow);
        // Send Event - fault
        const auto now = flex::get_time_dbl();

        ESSLogger::get().critical("BMS shutdown timed out in state [{}]", BMSPowerOn);
        if (logging_enabled)
        {
            std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "BMSShutdownTimeout", "txt");
            ESSLogger::get().logIt(dirAndFile);
        }

        auto temp_av = amap["BMSShutdown"];
        if (temp_av)
            temp_av->sendEvent("BMS", am->p_fims, Severity::Fault, "BMS shutdown timed out in state [%s] at time %2.3f",
                               BMSPowerOn, now.count());

        vm->schedStop(vmap, amap, "schedShutdownBMS", 1);
        bmsAv->setVal(1);
    }
    else if (amap["KeyCmd"]->getbParam("KeyCmdDone"))
    {
        amap["BMSShutdown"]->setParam("Fault", true);
        BMSSystemStateStep = (char*)"BMS Shutdown Failed";
        amap["BMSSystemStateStep"]->setVal(BMSSystemStateStep);
        if (1)
            FPS_ERROR_PRINT("%s >> No response from BMS at time: %f \n", __func__, tNow);
        // Send Event - fault
        const auto now = flex::get_time_dbl();

        ESSLogger::get().critical("[BMS not responding]");
        if (logging_enabled)
        {
            std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "BMSNotResponding", "txt");
            ESSLogger::get().logIt(dirAndFile);
        }

        auto temp_av = amap["BMSShutdown"];
        if (temp_av)
            temp_av->sendEvent("BMS", am->p_fims, Severity::Fault, "BMS not responding at time %2.3f", now.count());

        vm->schedStop(vmap, amap, "schedShutdownBMS", 1);
        bmsAv->setVal(1);
    }
    else if (strcmp(BMSSystemStateStep, "Shutdown Fault") == 0)
    {
        char* savedBMSSystemStateStep = amap["BMSSystemStateStep"]->getcVal();
        if (strcmp(savedBMSSystemStateStep, "BMS Shutdown Failed") != 0)
        {
            amap["BMSShutdown"]->setParam("Fault", true);
            BMSSystemStateStep = (char*)"BMS Shutdown Failed";
            amap["BMSSystemStateStep"]->setVal(BMSSystemStateStep);
            if (1)
                FPS_ERROR_PRINT("%s >> BMS in failed state at time: %f \n", __func__, tNow);
            // Send Event - fault
            const auto now = flex::get_time_dbl();

            ESSLogger::get().critical("[BMS in failed state]");
            if (logging_enabled)
            {
                std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "BMSFailedState", "txt");
                ESSLogger::get().logIt(dirAndFile);
            }

            auto temp_av = amap["BMSShutdown"];
            if (temp_av)
                temp_av->sendEvent("BMS", am->p_fims, Severity::Fault, "BMS in failed state at time %2.3f",
                                   now.count());

            vm->schedStop(vmap, amap, "schedShutdownBMS", 1);
            bmsAv->setVal(1);
        }
    }
    else if (strcmp(BMSSystemStateStep, "Ready") == 0)
    {
        RunKeyCmd(vmap, amap, aname, NULL, aV);
    }

    if (amap["BMSShutdown"]->getbVal() && amap["KeyCmd"]->getbVal())
    {
        amap["KeyCmd"]->setVal(false);
    }

    if (amap["KeyCmd"]->valueChangedReset())
    {
        varsmap* vlist = vm->createVlist();
        if (amap["KeyCmd"]->getbVal())
        {
            // TODO after MVP put the command definitions into config
            amap["BMSKeyCmd"]->setVal(3);
            vm->addVlist(vlist, amap["BMSKeyCmd"]);
            // am->Send("set", "/components/catl_ems_bms_rw/ems_cmd", NULL,
            // "{\"value\":3}");
        }
        else
        {
            amap["BMSKeyCmd"]->setVal(1);
            vm->addVlist(vlist, amap["BMSKeyCmd"]);
            // am->Send("set", "/components/catl_ems_bms_rw/ems_cmd", NULL,
            // "{\"value\":1}");
        }
        vm->sendVlist(p_fims, "set", vlist);
        vm->clearVlist(vlist);
    }

    return 0;
}

int CheckGPIO(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(p_fims);
    int reload = 0;
    bool bval = false;
    int ival = 0;
    char* cval = (char*)"";
    // double dval = 0.0;
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    essPerf ePerf(am, aname, __func__);

    assetVar* cgpioAv = amap[__func__];  // Need to figure out
                                         // the resolution
                                         // here!
    if (!cgpioAv || (reload = cgpioAv->getiVal()) == 0)
    {
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }

    if (reload < 2)
    {
        // reload
        amap[__func__] = vm->setLinkVal(vmap, aname, "/reload", __func__, reload);
        cgpioAv = amap[__func__];

        // other variables:
        linkVals(*vm, vmap, amap, aname, "/status", bval, "EStop", "Surge Arrester", "Disconnect Switch", "Fire Alarm",
                 "Fuse Monitoring", "Door Latch", "gpioFault");
        linkVals(*vm, vmap, amap, aname, "/config", cval, "FaultDestination");
        linkVals(*vm, vmap, amap, aname, "/status", ival, "gpioBitfield");

        if (reload == 0)  // complete restart
        {
            amap["gpioBitfield"]->setVal(0);
            for (auto x : vmap["/components/gpio"])
            {
                assetVar* avg = x.second;
                if (avg && avg->gotParam("pin"))
                {
                    bval = false;
                    avg->setVal(bval);
                }
            }
        }
        reload = 2;
        cgpioAv->setVal(reload);
    }

    int gbf = 0;
    bval = false;
    for (auto x : vmap["/components/gpio"])
    {
        assetVar* avg = x.second;
        if (avg && avg->gotParam("pin"))
        {
            if (avg->getbVal())
            {
                gbf |= (1 << avg->getiParam("pin"));
            }
        }
    }

    if (amap["gpioBitfield"]->getiVal() != gbf)
        amap["gpioBitfield"]->setVal(gbf);

    if (0)
        FPS_ERROR_PRINT("%s >>> gbf = %d\n", __func__, gbf);

    if (amap["gpioBitfield"]->valueChangedReset())
    {
        if (amap["gpioBitfield"]->getiVal() > 0)
        {
            char* fltDest = amap["FaultDestination"]->getcVal();

            for (auto x : vmap["/components/gpio"])
            {
                assetVar* avg = x.second;
                if (avg && avg->gotParam("pin"))
                {
                    if (avg->getbVal() && avg->getiParam("pin") > 2)
                    {
                        char* msg = NULL;
                        asprintf(&msg, "GPIO Fault: [%s]", avg->name.c_str());
                        if (1)
                            FPS_ERROR_PRINT("%s, fltDest %s\n", msg, fltDest);
                        vm->sendAlarm(vmap, avg, fltDest, NULL, msg, 2);
                        if (msg)
                            free(msg);
                    }
                }
            }
        }
        bool gFault = amap["gpioBitfield"]->getiVal() > 0 ? true : false;
        vm->setVal(vmap, amap["gpioFault"]->comp.c_str(), amap["gpioFault"]->name.c_str(), gFault);
    }

    return 0;
}

#endif
