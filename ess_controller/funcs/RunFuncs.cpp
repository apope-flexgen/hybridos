#ifndef RUNFUNCS_CPP
#define RUNFUNCS_CPP


#include <chrono>
#include <string>

#include "asset.h"
#include "varMapUtils.h"
#include "testUtils.h"
//#include "funcRef.h"
#include "systemEnums.h"
#include "formatters.hpp"

using namespace testUtils;

/**
 * @brief 
 *
 * Make sure that func arguments are correct.
 *      - Might need error checking
 *      - (check to make sure that config is correct, so you aren't calling a command that doesn't exist for that asset/amap)
 *
 * Need to check how many registers we can keep track of (what modbus registers to change to elicit certain sequences)
 *      - amap reference might not be enough.
 *
 *
 */

extern "C++"
{

    int StartupPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
    int StartupBMS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
    int RunKeyCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);

}

int StartupPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    if(!checkAv(vmap, amap, aname, p_fims, aV))
    {
        FPS_PRINT_ERROR(">> ERROR unable to continue aname [{}]", aname);
        return -1;
    }
    bool logging_enabled = getLoggingEnabled(vmap, *aV->am->vm);
    char* LogDir = getLogDir(vmap, *aV->am->vm);

    int reload = 0;
    bool bval = false;
    int ival = 0;
    double dval = 0.0;
    char* cval = (char*)"";
    char* tValInit = (char*)"Init";
    asset_manager * am = aV->am;
    //if (am == nullptr) 
    if (0) FPS_PRINT_INFO("Running for aV [{}:{}] am {}, amap {} aname [{}]"
        , aV->comp
        , aV->name
        , fmt::ptr(aV->am)
        , fmt::ptr(&amap)
        , aname
        );
    if (0) FPS_PRINT_INFO(">>>>>>>>>>>PCS Startup check amap {} system state [{}] {} >> pcssystem state [{}] {}"
        , fmt::ptr(&amap)
        , amap["SystemState"] ? cstr{amap["SystemState"]->getcVal()} : cstr{"No System State"}
        , fmt::ptr(amap["SystemState"])
        , amap["PCSSystemState"] ? cstr{amap["PCSSystemState"]->getcVal()} : cstr{"No PCS System State"}
        , fmt::ptr(amap["PCSSystemState"])
        );
    VarMapUtils* vm = am->vm;
    char* essName =  vm->getSysName(vmap);
    double tNow = vm->get_time_dbl();
    essPerf ePerf(am, aname, __func__);

    assetVar* pcsAv = amap[__func__]; // Need to figure out the resolution here!
    if (!pcsAv || (reload = pcsAv->getiVal()) == 0)
    {
        if (0) FPS_PRINT_INFO("Running reload {}", reload);
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }

    if (reload == 0)
    {
        // reload
        linkVals(*vm, vmap, amap, aname, "/reload", reload, __func__);
        pcsAv = amap[__func__]; // Need to figure out the resolution here!
        linkVals(*vm, vmap, amap, aname, "/reload", reload, "pcsRunKeyCmd");

        // Statuses, etc.:
        // linkVals(*vm, vmap, amap, essName, "/status", tValInit, "SystemStateStep");
        linkVals(*vm, vmap, amap, essName, "/status", bval, "PCSStartup");
        linkVals(*vm, vmap, amap, aname, "/sched", cval, "schedStartupPCS");

        linkVals(*vm, vmap, amap, aname, "/status", tValInit, "SystemState", "PCSSystemState", "PCSSystemStateStep");
        linkVals(*vm, vmap, amap, aname, "/status", cval, "PCSStatusResp");
        linkVals(*vm, vmap, amap, aname, "/status", dval, "PCSStartupTime");
        linkVals(*vm, vmap, amap, aname, "/status", ival, "PCSStartKeyCmd");
        linkVals(*vm, vmap, amap, aname, "/status", bval, "KeyCmd", "SoftStartup");
        ival= 1; linkVals(*vm, vmap, amap, aname, "/config", ival, "PCSon");  
        ival= 0; linkVals(*vm, vmap, amap, aname, "/config", ival, "PCSoff");
        ival= 1; linkVals(*vm, vmap, amap, aname, "/config", ival, "PCSstart");
        // ival= 0; linkVals(*vm, vmap, amap, aname, "/config", ival, "PCSstop");
        ival= 0; linkVals(*vm, vmap, amap, aname, "/config", ival, "PCSstay");
        
        linkVals(*vm, vmap, amap, aname, "/controls", ival, "PCSRun"); // pcs selector status variable
        linkVals(*vm, vmap, amap, "bms", "/status", ival, "DCClosed");

        linkVals(*vm, vmap, amap, aname, "/config", dval, "maxStartupTime", "maxFullStartupTime", "maxKeyCmdOnTime", "maxKeyCmdTime");
        linkVals(*vm, vmap, amap, aname, "/config", ival, "maxKeyCmdTries");
        linkVals(*vm, vmap, amap, aname, "/config", cval, "FaultDestination", "AlarmDestination");
        if (1) FPS_PRINT_INFO(">>>>>>>>>>>PCS Startup reload 0 ; PCSon {} PCSoff {}"
                    , amap["PCSon"]->getiVal()
                    , amap["PCSoff"]->getiVal()
                    );
        
        pcsAv->setVal(1);
        return 0;
    }
    if(reload < 2)
    {
        amap["PCSSystemStateStep"]->setVal(tValInit);
        amap["PCSSystemState"]->setVal(tValInit);
        amap["PCSStartupTime"]->setVal(tNow);

        amap["PCSStartup"]->setVal(false);
        amap["PCSStartup"]->setParam("Fault", false);
        amap["PCSStartup"]->setParam("FullStartup", false);

        amap["KeyCmd"]->setParam("maxKeyCmdOnTime", amap["maxKeyCmdOnTime"]->getdVal());
        amap["KeyCmd"]->setParam("maxKeyCmdTime", amap["maxKeyCmdTime"]->getdVal());
        amap["KeyCmd"]->setParam("maxKeyCmdTries", amap["maxKeyCmdTries"]->getiVal());

        if (1) FPS_PRINT_INFO(">>>>>>>>>>> PCS Startup reload 1 amap {} system state [{}] {} >> pcssystem state [{}] {}"
            , fmt::ptr(&amap)
            , cstr{amap["SystemState"]->getcVal()}
            , fmt::ptr(amap["SystemState"])
            , cstr{amap["PCSSystemState"]->getcVal()}
            , fmt::ptr(amap["PCSSystemState"])
            );

        pcsAv->setVal(2);
    }
    if (0) FPS_PRINT_INFO(">>>>>>>>>>>>>>>>PCS Startup system state [{}]", cstr{amap["SystemState"]->getcVal()});

    char* SystemState = amap["SystemState"]->getcVal();
    char* PCSSystemStateStep = amap["PCSSystemStateStep"]->getcVal();
    if (0) FPS_PRINT_INFO("Running reload {} state [{}] step [{}]", reload, cstr{SystemState}, cstr{PCSSystemStateStep});

    char* PCSStatusResp = amap["PCSStatusResp"]->getcVal();
    if (0) FPS_PRINT_INFO("Running reload {} StatusResp [{}]", reload, cstr{PCSStatusResp});
    if (0) FPS_PRINT_INFO("aname {} comp {} name {}", aname, amap["PCSStatusResp"]->comp, amap["PCSStatusResp"]->name);

    // Command values for the PCS control registers (values can be defined in config)
    int PCSon = amap["PCSon"]->getiVal();
    //int PCSoff = amap["PCSoff"]->getiVal();
    int PCSstart = amap["PCSstart"]->getiVal();
    int PCSstay = amap["PCSstay"]->getiVal();

    if (!PCSStatusResp)
    {
        amap["PCSStartup"]->setParam("Fault", true);
        PCSSystemStateStep = (char*)"PCS Startup Communication Failed";
        amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
        if (1) FPS_PRINT_WARN("No communication with PCS");
        // Send Event - fault
        const auto now = flex::get_time_dbl();

        ESSLogger::get().critical("[PCS startup comms failed]");
        if (logging_enabled)
        {
            std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "PCSStartupCommsFailed", "txt");
            ESSLogger::get().logIt(dirAndFile);
        }

        assetVar* temp_av = amap["PCSStatusResp"];
        if (temp_av) temp_av->sendEvent("PCS", vm->p_fims, Severity::Fault, "PCS startup comms failed at time %2.3f"
            , now.count()
            );

        vm->schedStop(vmap, amap,"schedStartupPCS", 1);

        pcsAv->setVal(1);
        return 0;
    }

    // SystemState holds state from PCS decoded into either Ready, Starting, Ready and Shutdown
    if (amap["SystemState"]->valueChangedReset() || strcmp(PCSSystemStateStep, "Init") == 0)
    {
        amap["pcsRunKeyCmd"]->setVal(0);           // Reset key cmd function
        RunKeyCmd(vmap, amap, aname, nullptr, aV);
        amap["PCSStartupTime"]->setVal(tNow);
        amap["PCSSystemStateStep"]->setVal(SystemState);
        if (0) FPS_PRINT_INFO("State step changed from {} to {}", cstr{PCSSystemStateStep}, cstr{SystemState});
    }
    // If state is Off and not DCClosed Schedule Stop
    // If full startup send start to PCS
    //
    if (strcmp(SystemState, "Off") == 0)
    {
        if (!amap["DCClosed"]->getiVal())
        {
            amap["PCSStartup"]->setParam("Fault", true);
            PCSSystemStateStep = (char*)"PCS initial DC voltage condition not met";
            amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
            if (1) FPS_PRINT_WARN("PCS initial DC voltage condition not met");
            const auto now = flex::get_time_dbl();
            ESSLogger::get().info("[PCS initial DC voltage condition not met]");
            assetVar* temp_av = amap["PCSStartup"];
            if (temp_av) temp_av->sendEvent("PCS", vm->p_fims, Severity::Info, "PCS initial DC voltage condition not met at time %2.3f"
                , now.count()
                );

            vm->schedStop(vmap, amap,"schedStartupPCS", 1);
            pcsAv->setVal(1);
        }
        else if (!amap["PCSStartup"]->getbParam("FullStartup"))
        {
            amap["PCSStartup"]->setParam("FullStartup", true);
            PCSSystemStateStep = (char*)"PCS Starting";
            amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
            if (1) FPS_PRINT_INFO("Sending Start to PCS");
            const auto now = flex::get_time_dbl();
            ESSLogger::get().info("[Sending Start to PCS]");
            assetVar* temp_av = amap["PCSStartup"];
            if (temp_av) temp_av->sendEvent("PCS", vm->p_fims, Severity::Info, "Sending Start to PCS at time %2.3f"
                , now.count()
                );

            varsmap* vlist = vm->createVlist();
            // sends start see config for links to modbus 
            amap["PCSRun"]->setVal(PCSon);
            vm->addVlist(vlist, amap["PCSRun"]);
            vm->sendVlist(p_fims, "set", vlist);
            vm->clearVlist(vlist);
        }
        // if not in run mode , ie did not respond go to stop maode after timed out
        // set fault param
        else if (amap["PCSRun"]->getdVal() != PCSon)
        {
            if ((tNow - amap["PCSStartupTime"]->getdVal()) > amap["maxStartupTime"]->getdVal())
            {
                amap["PCSStartup"]->setParam("Fault", true);
                PCSSystemStateStep = (char*)"PCS Startup Failed";
                amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
                if (1) FPS_PRINT_WARN("PCS Start command not setting in state {}", cstr{PCSStatusResp});
                // Send Event - fault
                const auto now = flex::get_time_dbl();

                ESSLogger::get().critical("PCS Start command not setting");
                if (logging_enabled)
                {
                    std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "PCSStuckInHold", "txt");
                    ESSLogger::get().logIt(dirAndFile);
                }

                assetVar* temp_av = amap["PCSStartup"];
                if (temp_av) temp_av->sendEvent("PCS", vm->p_fims, Severity::Fault, "PCS Start command not setting at time %2.3f"
                    , now.count()
                    );

                vm->schedStop(vmap, amap,"schedStartupPCS", 1);
                pcsAv->setVal(1);
            }
            //not timed out send run again
            varsmap* vlist = vm->createVlist();
            amap["PCSRun"]->setVal(PCSon);
            vm->addVlist(vlist, amap["PCSRun"]);
            vm->sendVlist(p_fims, "set", vlist);
            vm->clearVlist(vlist);
        }
    }
    // if in soft start and ready enter standby
    // set start up mode flag precharge completed 
    else if (amap["SoftStartup"]->getbVal() && strcmp(SystemState, "Ready") == 0)
    {
        amap["PCSStartup"]->setVal(true);
        PCSSystemStateStep = (char*)"PCS DC Charge Complete";
        amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
        if (1) FPS_PRINT_INFO("PCS succesfully precharged, in Standby");
        // Send Event - info
        const auto now = flex::get_time_dbl();
        ESSLogger::get().info("[PCS DC Contactors closed, in Standby]");
        assetVar* temp_av = amap["PCSStartup"];
        if (temp_av) temp_av->sendEvent("PCS", vm->p_fims, Severity::Info, "PCS DC Contactors closed, in Standby at time %2.3f"
            , now.count()
            );
        amap["SoftStartup"]->setVal(false);

        vm->schedStop(vmap, amap,"schedStartupPCS", 1);
        pcsAv->setVal(1);
    }
    // if in full start and ready enter start
    else if (amap["PCSStartup"]->getbParam("FullStartup") && strcmp(SystemState, "Ready") == 0)
    {
        PCSSystemStateStep = (char*)"PCS DC Charge Complete";
        amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
        if (1) FPS_PRINT_INFO("PCS succesfully precharged, in Standby going to start");
        const auto now = flex::get_time_dbl();
        ESSLogger::get().info("[PCS DC Contactors closed, in Standby going to start ]");
        assetVar* temp_av = amap["PCSStartup"];
        if (temp_av) temp_av->sendEvent("PCS", vm->p_fims, Severity::Info, "PCS DC Contactors closed, in Standby going to start  at time %2.3f"
            , now.count()
            );

        amap["PCSStartup"]->setParam("FullStartup", false);
    }
    // check for exceeding max startup time (sched stop) 
    else if (strcmp(SystemState, "Starting") == 0 && amap["PCSStartup"]->getbParam("FullStartup")
         && ((tNow - amap["PCSStartupTime"]->getdVal()) > amap["maxFullStartupTime"]->getdVal()))
    {
        // TODO preston check
        amap["PCSStartup"]->setParam("Fault", true);
        PCSSystemStateStep = (char*)"PCS Startup Failed";
        amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
        if (1) FPS_ERROR_PRINT("%s >> PCS stuck in hold state %s at time: %f \n", __func__, PCSStatusResp, tNow);

        // Send Event - fault
        const auto now = flex::get_time_dbl();

        const std::string msg = fmt::format("PCS stuck in hold state {} for {} seconds at time {:2.3f}"
            , PCSStatusResp
            , amap["maxFullStartupTime"]->getdVal()
            , now.count()
            );
        char* fltDest = amap["FaultDestination"]->getcVal();
        const std::string dest = fmt::format("{}", fltDest);
        vm->sendAlarm(vmap, amap["PCSStartup"], dest.c_str(), NULL, msg.c_str(), 2);

        ESSLogger::get().critical("PCS stuck in hold state [{}]",
            PCSStatusResp);
        if (logging_enabled)
        {
            std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "PCSStuckInHold", "txt");
            ESSLogger::get().logIt(dirAndFile);
        }

        assetVar* temp_av = amap["PCSStartup"];
        if (temp_av) temp_av->sendEvent("PCS", vm->p_fims, Severity::Fault, msg.c_str());

        vm->schedStop(vmap, amap,"schedStartupPCS", 1);
        pcsAv->setVal(1);
    }
    // another check for maxStartupTime
    else if (strcmp(SystemState, "Starting") == 0 && !amap["PCSStartup"]->getbParam("FullStartup")
         && ((tNow - amap["PCSStartupTime"]->getdVal()) > amap["maxStartupTime"]->getdVal()))
    {
        amap["PCSStartup"]->setParam("Fault", true);
        PCSSystemStateStep = (char*)"PCS Startup Failed";
        amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
        if (1) FPS_PRINT_INFO("PCS stuck in hold state {}", cstr{PCSStatusResp});
        // Send Event - fault
        const auto now = flex::get_time_dbl();

        ESSLogger::get().critical("PCS stuck in hold state [{}]",
            PCSStatusResp);
        if (logging_enabled)
        {
            std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "PCSStuckInHold", "txt");
            ESSLogger::get().logIt(dirAndFile);
        }

        assetVar* temp_av = amap["PCSStartup"];
        if (temp_av) temp_av->sendEvent("PCS", vm->p_fims, Severity::Fault, "PCS stuck in hold state [%s] at time %2.3f"
            , PCSStatusResp
            , now.count()
            );

        vm->schedStop(vmap, amap,"schedStartupPCS", 1);
        pcsAv->setVal(1);  // force reload init of this function 
    }
    // not in full startup (cold start needs  a long time, warm start needs a shorter timer)
    else if (strcmp(SystemState, "Starting") == 0 && !amap["PCSStartup"]->getbParam("FullStartup")
         && ((tNow - amap["PCSStartupTime"]->getdVal()) > amap["maxStartupTime"]->getdVal()))
    {
        amap["PCSStartup"]->setParam("Fault", true);
        PCSSystemStateStep = (char*)"PCS Startup Failed";
        amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
        if (1) FPS_PRINT_INFO("PCS stuck in hold state {}", cstr{PCSStatusResp});
        // Send Event - fault
        const auto now = flex::get_time_dbl();

        ESSLogger::get().critical("PCS stuck in hold state [{}]",
            PCSStatusResp);
        if (logging_enabled)
        {
            std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "PCSStuckInHold", "txt");
            ESSLogger::get().logIt(dirAndFile);
        }

        assetVar* temp_av = amap["PCSStartup"];
        if (temp_av) temp_av->sendEvent("PCS", vm->p_fims, Severity::Fault, "PCS stuck in hold state [%s] at time %2.3f"
            , PCSStatusResp
            , now.count()
            );

        vm->schedStop(vmap, amap,"schedStartupPCS", 1);
        pcsAv->setVal(1);
    }
    // if running stop this function.
    else if (strcmp(SystemState, "Running") == 0)
    {
        amap["PCSStartup"]->setVal(true);
        PCSSystemStateStep = (char*)"PCS Startup Complete";
        amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
        if (1) FPS_PRINT_INFO("All PCS's succesfully Startup");
        // Send Event - info
        const auto now = flex::get_time_dbl();
        ESSLogger::get().info("[PCS startup completed]");
        assetVar* temp_av = amap["PCSStartup"];
        if (temp_av) temp_av->sendEvent("PCS", vm->p_fims, Severity::Info, "PCS startup completed at time %2.3f"
            , now.count()
            );

        vm->schedStop(vmap, amap,"schedStartupPCS", 1);
        pcsAv->setVal(1);
    }
    // if shutdown or fault mode then we are also done.
    else if (strcmp(SystemState, "Shutdown") == 0 || strcmp(SystemState, "Fault") == 0)
    {
        amap["PCSStartup"]->setParam("Fault", true);
        PCSSystemStateStep = (char*)"PCS Startup Failed";
        amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
        if (1) FPS_PRINT_ERROR("PCS failed in state {}", cstr{PCSStatusResp});
        // Send Event - fault
        const auto now = flex::get_time_dbl();

        ESSLogger::get().critical("[PCS startup failed]");
        if (logging_enabled)
        {
            std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "PCSStartupFailed", "txt");
            ESSLogger::get().logIt(dirAndFile);
        }

        assetVar* temp_av = amap["PCSStartup"];
        if (temp_av) temp_av->sendEvent("PCS", vm->p_fims, Severity::Fault, "PCS startup failed at time %2.3f"
            , now.count()
            );

        vm->schedStop(vmap, amap,"schedStartupPCS", 1);
        pcsAv->setVal(1);
    }
    // else continue if in full startup
    else if (strcmp(SystemState, "Starting") == 0 && amap["PCSStartup"]->getbParam("FullStartup"))
    {
        if (0) FPS_PRINT_INFO("Waiting for Full Startup");
    }
    // wait but handle no response from keycmd
    else if (amap["KeyCmd"]->getbParam("KeyCmdDone"))
    {
        amap["PCSStartup"]->setParam("Fault", true);
        PCSSystemStateStep = (char*)"PCS Startup Failed";
        amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
        if (1) FPS_PRINT_WARN("No response from PCS");
        // Send Event - fault
        const auto now = flex::get_time_dbl();

        ESSLogger::get().critical("[PCS response failed]");
        if (logging_enabled)
        {
            std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "PCSResponseFailed", "txt");
            ESSLogger::get().logIt(dirAndFile);
        }

        assetVar* temp_av = amap["PCSStartup"];
        if (temp_av) temp_av->sendEvent("PCS", vm->p_fims, Severity::Fault, "PCS response failed at time %2.3f"
            , now.count()
            );

        vm->schedStop(vmap, amap,"schedStartupPCS", 1);
        pcsAv->setVal(1);
    }
    // this is where we use keycmd to start when ready and off
    else if (strcmp(SystemState, "Ready") == 0 || strcmp(SystemState, "Off") == 0)
    {
        RunKeyCmd(vmap, amap, aname, nullptr, aV);
    }
    // turn off keycmd if needed
    if (amap["PCSStartup"]->getbVal() && amap["KeyCmd"])
    {
        amap["KeyCmd"]->setVal(false);
    }
    // if KeyCmd changed then send it ( 1 or 0)  TODO use config values
    // command goes from 0 to 1 to request a start for PCS
    if (amap["KeyCmd"]->valueChangedReset())
    {
        varsmap* vlist = vm->createVlist();
        if (amap["KeyCmd"]->getbVal())
        {
            //amap["PCSStartKeyCmd"]->setVal(1);
            amap["PCSStartKeyCmd"]->setVal(PCSstart);
            vm->addVlist(vlist, amap["PCSStartKeyCmd"]);
        }
        else
        {
            //amap["PCSStartKeyCmd"]->setVal(0);
            amap["PCSStartKeyCmd"]->setVal(PCSstay);
            vm->addVlist(vlist, amap["PCSStartKeyCmd"]);
        }
        vm->sendVlist(p_fims, "set", vlist);
        vm->clearVlist(vlist);
    }
    return 0;    
}

int StartupBMS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    if(!checkAv(vmap, amap, aname, p_fims, aV))
    {
        FPS_PRINT_ERROR(">> ERROR unable to continue aname [{}]", aname);
        return -1;
    }
    bool logging_enabled = getLoggingEnabled(vmap, *aV->am->vm);
    char* LogDir = getLogDir(vmap, *aV->am->vm);

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

    assetVar* bmsAv = amap[__func__]; // Need to figure out the resolution here!
    if (!bmsAv || (reload = bmsAv->getiVal()) == 0)
    {
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }

    if (reload == 0)
    {
        // reload
        linkVals(*vm, vmap, amap, aname, "/reload", reload, __func__);
        bmsAv = amap[__func__]; // Need to figure out the resolution here!
        linkVals(*vm, vmap, amap, aname, "/reload", reload, "bmsRunKeyCmd");

        // Statuses, etc.:
        // linkVals(*vm, vmap, amap, essName, "/status", tValInit, "SystemState");
        linkVals(*vm, vmap, amap, essName, "/status", bval, "BMSStartup");
        linkVals(*vm, vmap, amap, aname, "/sched", cval, "schedStartupBMS");

        linkVals(*vm, vmap, amap, aname, "/status", tValInit, "BMSSystemState", "BMSSystemStateStep");
        linkVals(*vm, vmap, amap, aname, "/status", cval, "BMSPowerOn", "BMSStatus");
        linkVals(*vm, vmap, amap, aname, "/status", dval, "BMSStartupTime");
        ival = 0;
        linkVals(*vm, vmap, amap, aname, "/status", ival, "BMSKeyCmd", "BMSFaultCnt");
        linkVals(*vm, vmap, amap, aname, "/status", bval, "KeyCmd");

        linkVals(*vm, vmap, amap, aname, "/config", dval, "maxStartupTime", "maxKeyCmdOnTime", "maxKeyCmdTime");
        linkVals(*vm, vmap, amap, aname, "/config", ival, "maxKeyCmdTries");

        ival= 3; linkVals(*vm, vmap, amap, aname, "/config", ival, "BMSstop");
        ival= 2; linkVals(*vm, vmap, amap, aname, "/config", ival, "BMSstart");
        ival= 1; linkVals(*vm, vmap, amap, aname, "/config", ival, "BMSstay");
        ival= 0; linkVals(*vm, vmap, amap, aname, "/config", ival, "BMSreset");


        bmsAv->setVal(1);
        return 0;
    }
    if (reload < 2)   // complete reset
    {
        amap["BMSSystemState"]->setVal(tValInit);
        amap["BMSSystemStateStep"]->setVal(tValInit);
        amap["BMSStartupTime"]->setVal(tNow);

        amap["BMSStartup"]->setVal(false);
        amap["BMSStartup"]->setParam("Fault", false);
        
        amap["BMSKeyCmd"]->setVal(1);       // "stay" command
        amap["KeyCmd"]->setParam("maxKeyCmdOnTime", amap["maxKeyCmdOnTime"]->getdVal());
        amap["KeyCmd"]->setParam("maxKeyCmdTime", amap["maxKeyCmdTime"]->getdVal());
        amap["KeyCmd"]->setParam("maxKeyCmdTries", amap["maxKeyCmdTries"]->getiVal());

        if (1) FPS_PRINT_INFO(">>>>>>>>>>>>>>>BMS Startup system state [{}]", cstr{amap["BMSSystemState"]->getcVal()});
        bmsAv->setVal(2);
    }

    // Command values for the BMS control registers (values can be defined in config)
    int BMSstay  = amap["BMSstay"]->getiVal();
    //int BMSreset = amap["BMSreset"]->getiVal();
    //int BMSstop    = amap["BMSstop"]->getiVal();
    int BMSstart = amap["BMSstart"]->getiVal();

    char* BMSSystemState = amap["BMSSystemState"]->getcVal();
    char* BMSSystemStateStep = amap["BMSSystemStateStep"]->getcVal();
    if (0) FPS_PRINT_INFO("Running reload {} state [{}] step [{}]", reload, cstr{BMSSystemState}, cstr{BMSSystemStateStep});
     // TODO Preston to review
    if (amap["BMSSystemState"]->valueChangedReset() || amap["BMSSystemStateStep"]->valueChangedReset())
    {
        if (0) FPS_PRINT_INFO("BMS State [{}] Step [{}]", cstr{BMSSystemState}, cstr{BMSSystemStateStep});
    }
    char* BMSStatus = amap["BMSStatus"]->getcVal();
    char* BMSPowerOn = amap["BMSPowerOn"]->getcVal();
    //No Status o no poweron then flag comms failed
    if (!BMSStatus || !BMSPowerOn)
    {
        amap["BMSStartup"]->setParam("Fault", true);
        BMSSystemStateStep = (char*)"BMS Startup Communication Failed";
        amap["BMSSystemStateStep"]->setVal(BMSSystemStateStep);
        if (1) FPS_PRINT_INFO("No communication with BMS");
        // TODO Preston to check
        return 0;
    }
    if (0) FPS_PRINT_INFO("status [{}] PowerOn [{}]", cstr{BMSStatus}, cstr{BMSPowerOn});

    if (amap["BMSStatus"]->valueChangedReset() || !strcmp(BMSSystemStateStep, "Init"))
    {
        if (/*!strcmp(BMSStatus, "Warning") ||*/ !strcmp(BMSStatus, "Fault") || amap["BMSFaultCnt"]->getiVal() > 0)
        {
            BMSSystemStateStep = (char*)"Startup Failed";
        }
    }

    if (0) FPS_PRINT_INFO("BMSPowerOn [{}] changed [{}] BMSSystemStateStep {}"
            , cstr{amap["BMSPowerOn"]->getcVal()}
            , amap["BMSPowerOn"]->valueChanged()
            , cstr{BMSSystemStateStep}
            );
    // set up on / off state from BMS info
    if ((amap["BMSPowerOn"]->valueChangedReset() || !strcmp(BMSSystemStateStep, "Init")) && strcmp(BMSSystemStateStep, "Startup Failed"))
    {
        if (!strcmp(BMSPowerOn, "On Fault") || !strcmp(BMSPowerOn, "Off Fault"))
        {
            BMSSystemStateStep = (char*)"Startup Failed";
        }
        else if (!strcmp(BMSPowerOn, "Off Ready"))
        {
            // reset command
            BMSSystemStateStep = (char*)"Ready";
            amap["bmsRunKeyCmd"]->setVal(0);           // Reset key cmd function
            RunKeyCmd(vmap, amap, aname, nullptr, aV);
        }
        else if (!strcmp(BMSPowerOn, "On Ready"))
        {
            BMSSystemStateStep = (char*)"Running";
        }
        if(!strcmp(amap["BMSSystemStateStep"]->getcVal(), BMSSystemStateStep))
        {
            FPS_PRINT_INFO("State step changed from [{}] to [{}]", amap["BMSSystemStateStep"]->getcVal(), cstr{BMSSystemStateStep});
            amap["BMSSystemStateStep"]->setVal(BMSSystemStateStep);
        }
    }
    // are we running 
    if (strcmp(BMSSystemStateStep, "Running") == 0)
    {
        amap["BMSStartup"]->setVal(true);
        BMSSystemStateStep = (char*)"BMS Startup Complete";
        amap["BMSSystemStateStep"]->setVal(BMSSystemStateStep);
        if (1) FPS_PRINT_INFO("All BMS's succesfully Startup");
        // Send Event - info
        const auto now = flex::get_time_dbl();
        ESSLogger::get().info("[All BMS successfully started] up");
        auto temp_av = amap["BMSStartup"];
        if (temp_av) temp_av->sendEvent("BMS", vm->p_fims, Severity::Info, "All BMS succesfully started up at time %2.3f"
                , now.count()
                );
        vm->schedStop(vmap, amap,"schedStartupBMS", 1);

        bmsAv->setVal(1);  // reload (init) function
    }
    // check for timeouts just one
    else if ((tNow - amap["BMSStartupTime"]->getdVal()) > amap["maxStartupTime"]->getdVal())
    {
        amap["BMSStartup"]->setParam("Fault", true);
        BMSSystemStateStep = (char*)"BMS Startup Failed";
        amap["BMSSystemStateStep"]->setVal(BMSSystemStateStep);
        if (1) FPS_PRINT_INFO("BMS Startup timed out in hold state {}", cstr{BMSPowerOn});
        if (1) FPS_PRINT_INFO("maxStartupTime {} BMSStartupTime {}", amap["maxStartupTime"]->getdVal(), amap["BMSStartupTime"]->getdVal());
        // Send Event - fault
        const auto now = flex::get_time_dbl();

        ESSLogger::get().critical("BMS startup timed out in state [{}]",
            BMSPowerOn);
        if (logging_enabled)
        {
            std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "BMSStartupTimeOut", "txt");
            ESSLogger::get().logIt(dirAndFile);
        }

        auto temp_av = amap["BMSStartup"];
        if (temp_av) temp_av->sendEvent("BMS", vm->p_fims, Severity::Fault, "BMS startup timed out in state [%s] at time %2.3f"
                , BMSPowerOn
                , now.count()
                );

        vm->schedStop(vmap, amap,"schedStartupBMS", 1);

        bmsAv->setVal(1);
    }
    // If Failed state detected 
    else if (strcmp(BMSSystemStateStep, "Startup Failed") == 0)
    {
        amap["BMSStartup"]->setParam("Fault", true);
        BMSSystemStateStep = (char*)"BMS Startup Failed";
        amap["BMSSystemStateStep"]->setVal(BMSSystemStateStep);
        if (1) FPS_PRINT_ERROR("BMS in failed state");
        // Send Event - fault
        const auto now = flex::get_time_dbl();

        ESSLogger::get().critical("[BMS in failed state]");
        if (logging_enabled)
        {
            std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "BMSFailedState", "txt");
            ESSLogger::get().logIt(dirAndFile);
        }

        auto temp_av = amap["BMSStartup"];
        if (temp_av) temp_av->sendEvent("BMS", vm->p_fims, Severity::Fault, "BMS in failed state at time %2.3f"
                , now.count()
                );

        vm->schedStop(vmap, amap,"schedStartupBMS", 1);

        bmsAv->setVal(1);
    }
    // command completed but timed out
    else if (amap["KeyCmd"]->getbParam("KeyCmdDone"))
    {
        amap["BMSStartup"]->setParam("Fault", true);
        BMSSystemStateStep = (char*)"BMS Startup Failed";
        amap["BMSSystemStateStep"]->setVal(BMSSystemStateStep);
        if (1) FPS_PRINT_ERROR("No response from BMS");
        // Send Event - fault
        const auto now = flex::get_time_dbl();

        ESSLogger::get().critical("[BMS not responding]");
        if (logging_enabled)
        {
            std::string dirAndFile = fmt::format("{}/{}.{}", LogDir, "BMSNotResponding", "txt");
            ESSLogger::get().logIt(dirAndFile);
        }

        auto temp_av = amap["BMSStartup"];
        if (temp_av) temp_av->sendEvent("BMS", vm->p_fims, Severity::Fault, "BMS not responding at time %2.3f"
                , now.count()
                );

        vm->schedStop(vmap, amap,"schedStartupBMS", 1);

        bmsAv->setVal(1);
    }
    // if in ready state
    else if (strcmp(BMSSystemStateStep, "Ready") == 0)
    {
        // make one signal go true / false
        RunKeyCmd(vmap, amap, aname, nullptr, aV);
    }

    if (amap["BMSStartup"]->getbVal() && amap["KeyCmd"]->getbVal())
    {
        amap["KeyCmd"]->setVal(false);
    }

    if (amap["KeyCmd"]->valueChangedReset())
    {
        varsmap* vlist = vm->createVlist();
        if (amap["KeyCmd"]->getbVal())
        {
            // send out "on" 2 command (2) (needs to be in config)
            amap["BMSKeyCmd"]->setVal(BMSstart); 
            vm->addVlist(vlist, amap["BMSKeyCmd"]);
        }
        else
        {
            // send out "hold" or stay command (1) ( needs to be in config) 
            amap["BMSKeyCmd"]->setVal(BMSstay);
            vm->addVlist(vlist, amap["BMSKeyCmd"]);
        }
        vm->sendVlist(p_fims, "set", vlist);
        vm->clearVlist(vlist);
    }
    
    return 0;
}


#endif
