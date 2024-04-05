#ifndef FAULTFUNCS_CPP
#define FAULTFUNCS_CPP

#include "asset.h"
#include "varMapUtils.h"
//#include "testUtils.h"
#include "funcRef.h"
#include "systemEnums.h"

#include <chrono>
#include <string>
#define PCSStopCmd 1
#define PCSStopResp 5
#define PCSCmdTime 0.5
// TODO FROM PHIL:
// take a look at getMapsCJ
//
// #ifndef SYSTEM_ENUMS
// #define SYSTEM_ENUMS

// // TODO: Put this enum in systemEnums with CATL stuff later:
// namespace Power_Electronics
// {
//     enum Inverter_Status_Messages
//     {
//         POWERUP = 0,            // PUP
//         INIT = 1,               // initialization
//         OFF = 2,                // Off
//         PRECHARGE = 3,          // PCHG
//         READY = 4,              // REA
//         WAIT = 5,               // Wait
//         ON = 6,                 // On
//         STOP = 7,               // Stop
//         DISCHARGE = 8,          // DISC
//         FAULT = 9,              // FLT
//         LVRT = 10,              // Low Voltage Ride Through (algorithm is
//         running) OVRT = 11,              // Over Voltage Ride Through
//         (algorithm is running) NIGHT_MODE = 12,        // NGHT NIGHT_DC_OFF =
//         13,      // NDCO STANDBY = 14,           // STB HVPL = 15,
//         // high voltage phase lost
//         // What happened to 16?!
//         PRE_ON = 17,            // PRON
//         SELF_DIAGNOSIS = 18,    // DIAG
//         LCON = 19,              // LC filter contactors on/activated
//         PREMAGENTIZATION = 20,  // PRMG
//         BANK_BALANCING = 21,    // BBAL
//         CVSB = 22               // cv standby algorithm running
//     };
// };

// namespace CATL
// {
//     enum BMS_Status_Messages
//     {
//         INIT = 0,
//         NORMAL = 1,
//         FULLCHARGE = 2,
//         FULLDISCHARGE = 3,
//         WARNING = 4,
//         FAULT = 5
//     };
//     enum BMS_Commands
//     {
//         INITIAL = 0,
//         STAYSTATUS = 1,
//         POWERON = 2,
//         POWEROFF = 3
//     };
// };
// #endif

// using namespace testUtils;

/**
 * @brief TODO:
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

namespace defaultFault_module
{
// These are the "unsafe" functions that could be loaded as a module if the
// engineers need them.
namespace internal_funcs
{
// For external reference to these functions if another program needs it.
extern "C" {
// TODO: Section these off (a comment with section name will do) so they become
// easier to manage.
// TODO: Make sure they aren't external/safe functions (move them down if they
// are)

// Commands:
void openDCContactor();
void closeDCContactor();
void sendKeyStopCmd();

// Comms Checks:
bool checkIfBMSOnline();
bool checkIfPCSOnline();
bool checkIfEMUOnline();
bool checkIfEMSOnline();
bool checkIfLCOnline();
bool checkIfEMMUOnline();

// Fault Checks:
bool checkIfBMSFault();
void enterFaultMode();
}

constexpr int INTERNAL_FUNC_COUNT = 2;  // Make sure to change this as needed.
struct Internal_funcRefArray
{
    const funcRef func_array[INTERNAL_FUNC_COUNT] = {
        funcRef{ "openDCContactor", (void*)&openDCContactor },
        funcRef{ "closeDCContactor", (void*)&closeDCContactor },
    };
};

void loadModule(VarMapUtils& vm, varsmap& vmap, const char* aname = "ess")
{
    const Internal_funcRefArray funcArray;
    for (int i = 0; i < INTERNAL_FUNC_COUNT; ++i)
    {
        vm.setFunc(vmap, aname, funcArray.func_array[i].func_name, funcArray.func_array[i].func_ptr);
    }
}

// Put funcs here:
extern "C" {
void openDCContactor()
{
    printf("opening DC Contactor!\n");
}
void closeDCContactor()
{
    printf("closing DC Contactor!\n");
}
void sendKeyStopCmd()
{
    printf("Sending keystop cmd!\n");
}
bool checkIfEMMUOnline()
{
    return false;
}
bool checkIfEMUOnline()
{
    return false;
}
bool checkIfBMSOnline()
{
    return false;
}
bool checkIfPCSOnline()
{
    return false;
}
void enterFaultMode()
{
    printf("Entering Fault Mode!\n");
}
}
}  // namespace internal_funcs

// Extra useful things to use internally (internal_funs:: can get repetitive.)
// using namespace defaultFault_module::internal_funcs;
// #define i_f internal_funcs

// For external reference to these functions if another program needs it.
extern "C" {
// TODO: Section these off so they become easier to manage.
// TODO: Make sure they aren't internal/unsafe functions (move them up if they
// are)
int ShutdownSystem(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int ShutdownPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int ShutdownBMS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int ShutdownPCSasset(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int ShutdownBMSasset(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int RunKeyCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
// void sendKeyStopCmd();
// void sendKeyStopCmd();
// void sendKeyStopCmd();
// void sendStandbyCmd();
// void handleMaxCellVoltage();
// void CheckBatteryShutdownSystem();

// // Reports:
}

constexpr int FUNC_COUNT = 5;  // Make sure to change this as needed.
struct funcRefArray
{
    const funcRef func_array[FUNC_COUNT] = {
        funcRef{ "ShutdownSystem", (void*)&ShutdownSystem },
        funcRef{ "ShutdownPCS", (void*)&ShutdownPCS },
        funcRef{ "ShutdownBMS", (void*)&ShutdownBMS },
        funcRef{ "ShutdownPCSasset", (void*)&ShutdownPCSasset },
        funcRef{ "ShutdownBMSasset", (void*)&ShutdownBMSasset },
        // funcRef{"sendKeyStopCmd", (void*)&sendKeyStopCmd},
        // funcRef{"sendStandbyCmd", (void*)&sendStandbyCmd},
    };
};

void loadModule(VarMapUtils& vm, varsmap& vmap, const char* aname = "ess")
{
    const funcRefArray funcArray;
    for (int i = 0; i < FUNC_COUNT; ++i)
    {
        vm.setFunc(vmap, aname, funcArray.func_array[i].func_name, funcArray.func_array[i].func_ptr);
    }
}

// Put fault sequences below (combination of above internal functions, to test
// for safety): There should NOT be anything over than calls to internal
// functions inside these functions
//      If NOT then you could just put it as an internal func above.
extern "C" {
// TODO: TAKE STRCMP'S OUT!!! - Possibly replace them with hashes. Just compare
// hashes instead. Check into std::strcmp vs. strcmp. A system Enum also works -
// just assign the states numbers and compare it that way. Put dummy functions
// inside each of the managers for their Shutdown's dummy the lower level
// functions as needed "Possibly" use "cascade" (Check ) FOR SAFETY: Call these
// functions directly. Possibly, use a WAKEUP_LEVEL (might not be necessary for
// this.)
/**
 * @brief Proper ShutdownSystem sequence, what the final produce will look like:
 *
 * This function will be called by (ESS Controller - where this function is
 located under) on the following conditions:
 *     1) eSTOP from ANYWHERE!
 *     2) stop command from Site_Controller
 *     3) Internal fault (ANY fault), shut down EVERYTHING! (a "fault stop")
 *     4) Through the User Interface (press a button!) - Web UI
 *     ... more? (don't know yet)
 *
 * This function will consist of the following sequence (run by ESS controller)
 *     1) Send to PCS_Manager the ShutdownPCS's command (SIDENOTE: have
 PCS_Manager set [ActivePowerSetPoint && ReactivePowerSetPoint] to 0 )
 *     2) Wait for PCS_Manager to respond (get ok that all PCS's are Shutdown)
 *     3) Send to BMS_Manager the ShutdownBMS's command
 *     4) Wait for BMS_Manager to respond (get ok that all BMS's are Shutdown)
 *     ... add other system as needed
 *     General form:
 *         - Send ShutdownAsset's to their manager
 *         - Wait for response before proceeding
 *
 * NOTE: Whenever we say "wait" we need a "what-if" and a "duration - for how
 long?"
 * CLARIFICATION: What do we do when something we tell to shut down doesn't?
 * OPTIONS:
 *      1) Set PCS_Manager's [ActivePowerSetPoint && ReactivePowerSetPoint] to 0
 - not allow to do anything!
 *      2) retry 5 times (retry "x" times, variable from config)
 *      3) issue alarms
 *      4) bypass wait and move on. These are our "what-ifs"
 * EMERGENCYACTIONS: (Do if all else fails?)
 *
 *      1) Track PCS_Manager's [Status's] - for all PCS's
 *      2) IMPORTANT: PCS_Managers are responsible for opening/closing DC
 Contactors, ESS Controller has no control.
 *
 * STATECOMMANDS:
 *      1) We DO NOT manually control each hardware piece, we only send it a
 single command to shut it down.
 *          1a) "CATL" is our main battery vendor. "Power electronics" is our
 main PCS vendor.
 *      2) NEED TO GET particular asset status's
 *          2a)
 *
 * SUGGESTIONS: (For discussion with Ben, Vinay, Aarabi, John, Tony, etc.
 tomorrow)
 *      1)
 *
 * @param vmap
 * @param amap
 * @param aname
 * @param p_fims
 * @param am
 *
 * Put each variable here and how it is used before writing a function from now
 on.
 *  // Statuses, etc.:
        char *
        /status/ess/SystemState
               used to indicate system state : Init, Fault, Ready , StandBy ,
 Run /status/ess/SystemStateStep/status/ess/ used to indicate system state step

        double
        /status/ess/CurrentSetpoint
                     the current Setpoint +ve for discharge -Ve for charge

        /status/ess/ShutdownTime
                    time the system took to shutdown.

        /config/ess/maxPCSShutdownTime
        /config/ess/maxBMSShutdownTime
                    time limits to allow for BMS and PCS system to complete
 shutdown.


        linkVals(*vm, vmap, amap, aname, "/status", ival, "
        //?? /status/ess/BmsStatus   status of the BmsSystem   from modbus input
        //?? /status/ess/PCsStatus   status of the PcsSystem
        //?? /status/ess/PCsStatus   status of the EmuSystem    from ??
        //?? /status/ess/PCsStatus   status of the EmmuSystem    from ??

        // other variables:
        bool
        /status/ess/EStop             estop input flag , several possible
 sources. TODO Should be attached directly to Shutdown. /status/ess/UiShutdown
 UserInterface shutdown request /status/ess/SMShutdown        Site Manage
 Shutdown request /status/ess/FaultShutdown     Fault Shutdown Request. Set by
 the fault system /status/ess/EMMUFaulted       (TODO) Indication that the EMMU
 system was faulted /status/ess/BMSFaulted        Indication that the BMS system
 was faulted /status/ess/PCSFaulted        Indication that the PCS system was
 faulted /status/ess/EMUFaulted        (TODO) Indication that the EMMU system
 was faulted

        NOTE we may set the next three up as integers. to keep track of multiple
 requests. /status/ess/ShutdownRequest      Main trigger for a shutdown start ,
 set by any system that wants to initiate a shutdown. We'll attach an alarm to
 it later. /status/ess/ShutdownRequested    Indication that we have responded to
 the latest shutdown request. /status/ess/ShutdownCompleted    Indication that
 we have completed the latest Shutdown Request bool
        /status/ess/PCSShutdownForced     Indication that we forced the PCS to
 shutdown It did not  resond to the current request.
        /status/ess/BMSShutdownForced     Indication that we forces the BMS
 shutdown.

        /status/ess/PCSShutdown           Indication that the PCSShutdown has
 completed /status/ess/BMSShutdown           Indication that the BMSShutdown has
 completed

        /status/ess/ShutdownSim           Request for simulation code path
 tester  reset as soon as SimSeen is set. /status/ess/ShutdownSimSeen
 Indication that the simulation request has been seen , reset when the
 simulation has completed

        /status/ess/ShutdownCompleted     Indication that Shutdown has been
 completed. To Be reset when another transition brigs the system out of
 shutdown.


        int
        /status/pcs/PCSStatusResp            Code indicating the current PCS
 status.  used in sim /status/bms/BMSStatusResp            Code indicating the
 current BMS status.  used n sim /status/pcs/PCSKeyCmd            Place to send
 PCS command.  used in sim /status/bms/BMSKeyCmd            Place to Send  BMS
 command.  used n sim


        TODO maybe
        /status/pcs/BMSStatusResp

 *
 */
// Help Jimmy
#define PCSStopCmd 1
#define PCSStopResp 5
#define BMSStopCmd 1
#define BMSStopResp 5
int ShutdownSystem(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    int reload = 0;
    bool bval = false;
    char* tVal = (char*)"Init";
    double dval = 0.0;
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    assetVar Avc;
    essPerf ePerf(am, aname, __func__);

    // char* tVal = (char*)"Test TimeStamp";
    // in future use tval for sending messages out - for UI and alert purposes.

    double tNow = vm->get_time_dbl();

    assetVar* essAv = amap[__func__];
    if (!essAv || (reload = essAv->getiVal()) == 0)
    {
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }

    if (reload < 2)
    {
        // TODO: clean these up!!!

        // reload
        linkVals(*vm, vmap, amap, aname, "/reload", reload, __func__);
        linkVals(*vm, vmap, amap, "bms", "/reload", reload, "ShutdownBMS");
        linkVals(*vm, vmap, amap, "pcs", "/reload", reload, "ShutdownPCS");

        // Statuses, etc.:
        linkVals(*vm, vmap, amap, aname, "/status", tVal, "SystemState", "SystemStateStep");
        linkVals(*vm, vmap, amap, aname, "/status", dval, "CurrentSetpoint", "ShutdownTime");

        // other variables:
        linkVals(*vm, vmap, amap, aname, "/status", bval, "EStop", "UiShutdown", "SMShutdown", "FaultShutdown",
                 "FullShutdown", "FullFaultShutdown");
        linkVals(*vm, vmap, amap, aname, "/status", bval, "EStopSeen", "UiShutdownSeen", "SMShutdownSeen",
                 "ShutdownCmd", "FaultShutdownSeen");
        linkVals(*vm, vmap, amap, aname, "/status", bval, "BMSFaulted", "PCSFaulted");
        linkVals(*vm, vmap, amap, aname, "/status", bval, "ShutdownRequest", "ShutdownRequested", "ShutdownCompleted",
                 "PCSShutdownForced", "BMSShutdownForced");

        // NOTE: These are all under "ess" where it should be, EStop especially
        // needs this.
        linkVals(*vm, vmap, amap, aname, "/status", bval, "PCSShutdown", "BMSShutdown");
        // linkVals(*vm, vmap, amap, "pcs", "/status", ival, "PCSStatusResp");  //
        // is this needed here ?? linkVals(*vm, vmap, amap, "bms", "/status", ival,
        // "BMSStatusResp");  // is this needed here ??

        if (reload == 0)  // complete restart
        {
            // TODO: Clean these up!!!

            essAv = amap[__func__];
            amap["UiShutdown"]->setVal(false);  // Shutdown from UI command
            amap["UiShutdown"]->setParam("ShutdownBMS", false);
            amap["UiShutdown"]->setParam("ShutdownPCS", false);

            amap["SMShutdown"]->setVal(false);         // Shutdown from site_manager
            amap["FaultShutdown"]->setVal(false);      // Shutdown from fault condition
            amap["FullShutdown"]->setVal(false);       // Shutdown PCS and BMS
            amap["FullFaultShutdown"]->setVal(false);  // Hard Shutdown PCS and BMS

            amap["ShutdownCmd"]->setVal(false);
            amap["ShutdownCmd"]->setParam("ShutdownRequested", false);
            // amap["ShutdownCmd"]->setParam("ShutdownCompleted", false);
            amap["ShutdownCmd"]->setParam("FullShutdown", false);
            amap["ShutdownCmd"]->setParam("HardShutdown", false);

            // BELOW: this runs reloads for asset_managers (does no processing - only
            // for linkVals).
            amap["ShutdownBMS"]->setVal(0);   // full reset
            amap["ShutdownPCS"]->setVal(0);   // full reset
            for (auto& ix : am->assetManMap)  // should get (const?) references to
                                              // pairs NOT copies.
            {
                FPS_ERROR_PRINT("     %s >>>>>> INIT for asset_managers\n", ix.second->name.c_str());
                // For MVP - We need at least PCS and BMS Shutdowns. Will add more.
                asset_manager* amc = ix.second;
                Avc.am = amc;
                if (amc->name == "pcs")
                {
                    ShutdownPCS(vmap, amc->amap, amc->name.c_str(), p_fims, &Avc);
                }
                if (amc->name == "bms")
                {
                    ShutdownBMS(vmap, amc->amap, amc->name.c_str(), p_fims, &Avc);
                }
                FPS_ERROR_PRINT("     %s >>>>>> INIT DONE for asset_managers\n", ix.second->name.c_str());
            }
            amap["SystemState"]->setVal(tVal);
        }
        reload = 2;
        essAv->setVal(reload);
    }

    // double tNow = vm->get_time_dbl();
    bool UiShutdown = amap["UiShutdown"]->getbVal();
    bool SMShutdown = amap["SMShutdown"]->getbVal();
    bool FaultShutdown = amap["FaultShutdown"]->getbVal();
    bool FullFaultShutdown = amap["FullFaultShutdown"]->getbVal();
    // bool ShutdownCompleted =
    // amap["ShutdownCmd"]->getbParam("ShutdownCompleted");
    bool ShutdownRequested = amap["ShutdownCmd"]->getbParam("ShutdownRequested");
    double ShutdownTime = amap["ShutdownTime"]->getdVal();
    bool PCSShutdown = amap["PCSShutdown"]->getbVal();
    bool BMSShutdown = amap["BMSShutdown"]->getbVal();
    bool PCSFaulted = amap["PCSShutdown"]->getbParam("Fault");
    bool BMSFaulted = amap["BMSShutdown"]->getbParam("Fault");
    char* cstate = amap["SystemState"]->getcVal();
    char* csstate = amap["SystemStateStep"]->getcVal();
    double sdtime = amap["ShutdownTime"]->getdVal();  // seems redundant

    // char* PCSStatusResp = amap["PCSStatusResp"]->getcVal();
    // int BMSStatusResp = amap["BMSStatusResp"]->getiVal();

    assetVar* UiShutdownAv = amap["UiShutdown"];        //->getbVal();
    assetVar* SMShutdownAv = amap["SMShutdown"];        //->getbVal();
    assetVar* FaultShutdownAv = amap["FaultShutdown"];  //->getbVal();
    assetVar* FullFaultShutdownAv = amap["FullFaultShutdown"];
    // assetVar* FullShutdownAv = amap["FullShutdown"];
    if (0)
        FPS_ERROR_PRINT("%s >> Running Shutdown  at time: %f\n", __func__, tNow);

    // TODO allow a state change in any input command to change this
    // if (ShutdownCompleted) // Need this under bms and pcs Shutdowns
    // {
    //     if (1) FPS_ERROR_PRINT("""""""""""""I AM COMPLETE!!!!""""""""""\n");
    //     // if (ShutdownRequested)
    //     // {
    //     //     amap["ShutdownRequested"]->setVal(false);
    //     // }
    //     // amap["ShutdownCompleted"]->setVal(false);

    //     essAv->setVal(0);
    //     return 0;
    // }
    // This is the start of the real code (this is our flow diagram):
    if (UiShutdownAv->getbVal() && UiShutdownAv->valueChangedReset())
    {
        amap["ShutdownCmd"]->setVal(true);
    }
    if (SMShutdownAv->getbVal() && SMShutdownAv->valueChangedReset())
    {
        amap["ShutdownCmd"]->setVal(true);
    }
    if (FaultShutdownAv->getbVal() && FaultShutdownAv->valueChangedReset())
    {
        amap["ShutdownCmd"]->setVal(true);
        amap["ShutdownCmd"]->setParam("HardShutdown", true);
    }
    if (FullFaultShutdownAv->getbVal() && FullFaultShutdownAv->valueChangedReset())
    {
        amap["ShutdownCmd"]->setVal(true);
        amap["ShutdownCmd"]->setParam("FullShutdown", true);
        amap["ShutdownCmd"]->setParam("HardShutdown", true);
    }
    // deprecating FullShutdown...only time batteries shut off is
    // FullFaultShutdown if (FullShutdownAv->getbVal() &&
    // FullShutdownAv->valueChangedReset())
    // {
    //     amap["ShutdownCmd"]->setVal(true);
    //     amap["ShutdownCmd"]->setParam("FullShutdown", true);
    // }
    // want to stop PCS if BMS cuts off
    // if ((PCSStatusResp != Power_Electronics::OFF && PCSStatusResp !=
    // Power_Electronics::POWERUP) && (BMSStatusResp == CATL::POWEROFFREADY ||
    // BMSStatusResp == CATL::POWERONFAULT || BMSStatusResp ==
    // CATL::POWEROFFFAULT))
    // {
    //     // amap["ShutdownCmd"]->setVal(true);
    //     FPS_ERROR_PRINT("%s >>> E-STOPPING!!! PCSStatusResp [%d] BMSStatusResp
    //     [%d]\n", __func__, PCSStatusResp, BMSStatusResp);
    //     // amap["ShutdownCmd"]->setParam("FullShutdown", true);
    // }
    // if (0) FPS_ERROR_PRINT("%s >>> PCSStatusResp [%s] BMSStatusResp [%d]\n",
    // __func__, PCSStatusResp, BMSStatusResp);
    bool ShutdownCmd = amap["ShutdownCmd"]->getbVal();

    if (ShutdownCmd)  // TODO: Make these one-shots
    {
        if (amap["ShutdownCmd"]->getbParam("ShutdownRequested"))
        {
            if (1)
                FPS_ERROR_PRINT(
                    "%s >> Shutdown Request Ignored,  In request at time: "
                    "%f elap %f \n",
                    __func__, tNow, (tNow - ShutdownTime));
            amap["ShutdownCmd"]->setVal(false);
        }
        if (1)
            FPS_ERROR_PRINT(
                "UIShutdown: %s, SMShutdown: %s, FaultShutdown: %s, "
                "FullFaultShutdown: %s\n",
                UiShutdown ? "true" : "false", SMShutdown ? "true" : "false", FaultShutdown ? "true" : "false",
                FullFaultShutdown ? "true" : "false");
        if (!ShutdownRequested)  // we are having a shutdown requested here when you
                                 // set ShutdownCompleted to false.
        {
            if (1)
                FPS_ERROR_PRINT("%s >> Func Starting Shutdown at time: %f elap %f \n", __func__, tNow,
                                (tNow - ShutdownTime));
            amap["ShutdownCmd"]->setVal(false);

            const char* cVal = "Shutdown";
            amap["SystemState"]->setVal(cVal);
            cVal = "Waiting PCS Shutdown";
            amap["SystemStateStep"]->setVal(cVal);
            amap["ShutdownCmd"]->setParam("ShutdownRequested", true);

            // Partial reset of functions
            amap["ShutdownPCS"]->setVal(1);
            amap["ShutdownBMS"]->setVal(1);
            for (auto& ix : am->assetManMap)  // should get (const?) references to
                                              // pairs NOT copies.
            {
                asset_manager* amc = ix.second;
                Avc.am = amc;
                if (amc->name == "pcs")
                {
                    FPS_ERROR_PRINT("%s >> Resetting shutdown PCS\n", __func__);
                    ShutdownPCS(vmap, amc->amap, amc->name.c_str(), p_fims, &Avc);
                }
                if (amc->name == "bms")
                {
                    FPS_ERROR_PRINT("%s >> Resetting shutdown BMS\n", __func__);
                    ShutdownBMS(vmap, amc->amap, amc->name.c_str(), p_fims, &Avc);
                }
            }

            // Start of the Shutdown:
            tNow = am->vm->get_time_dbl();
            amap["ShutdownTime"]->setVal(tNow);
            if (1)
                FPS_ERROR_PRINT("%s >> Setting ShutdownTime to %f\n", __func__, tNow);

            vm->sendAlarm(vmap, "/status/ess:ShutdownRequested", "/components/ess:alarms", "Shutdown",
                          "Shutdown Started", 1);
            assetVar* temp_av = amap["ShutdownRequested"];
            if (temp_av)
                temp_av->sendEvent("ESS", am->p_fims, Severity::Info, "Setting shutdown time at time %2.3f",
                                   vm->get_time_dbl());
        }
    }

    ShutdownRequested = amap["ShutdownCmd"]->getbParam("ShutdownRequested");
    if (!ShutdownRequested)
    {
        if (0)
            FPS_ERROR_PRINT("%s >> Func Quitting Shutdown at  time: %f\n", __func__, tNow);
        return 0;
    }

    // After this we start processing the Shutdown request:         WHY?
    if (amap["SystemState"]->valueChangedReset() | amap["SystemStateStep"]->valueChangedReset())
    {
        cstate = amap["SystemState"]->getcVal();
        csstate = amap["SystemStateStep"]->getcVal();

        if (1)
            FPS_ERROR_PRINT("%s >> Shutdown  state [%s] step [%s]  time elapsed : %f\n", __func__, cstate, csstate,
                            tNow - sdtime);
    }

    PCSShutdown = amap["PCSShutdown"]->getbVal();
    BMSShutdown = amap["BMSShutdown"]->getbVal();

    // assets are in assetMap sub managers are in assetManMap
    for (auto& ix : am->assetManMap)  // should get (const?) references to pairs NOT copies.
    {
        asset_manager* amc = ix.second;
        Avc.am = amc;
        if (0)
            FPS_ERROR_PRINT("manager_name: [%s] PCSFaulted [%s] BMSFaulted [%s]\n", amc->name.c_str(),
                            PCSFaulted ? "true" : "false", BMSFaulted ? "true" : "false");

        if (amc->name == "pcs")
        {
            if (0)
                FPS_ERROR_PRINT("manager_name: [%s] PCSShutdown [%s]\n", amc->name.c_str(),
                                PCSShutdown ? "true" : "false");

            if (!PCSFaulted && !PCSShutdown)  // || BMSFaulted))
            {
                if (0)
                    FPS_ERROR_PRINT("%s >>>> cascading function to >>>>> Manager [%s] at time: %f\n ", __func__,
                                    amc->name.c_str(), tNow);
                if (UiShutdownAv->gotParam("ShutdownPCS") && UiShutdownAv->getbParam("ShutdownPCS"))
                {
                    ShutdownPCS(vmap, amc->amap, amc->name.c_str(), p_fims, &Avc);
                }
            }
        }
        if (amc->name == "bms")
        {
            if (0)
                std::cout << std::boolalpha << "PCSFaulted?: " << PCSFaulted << '\n';
            if (0)
                FPS_ERROR_PRINT("manager_name: [%s] BMSShutdown [%s] PCSShutdown [%s]\n", amc->name.c_str(),
                                BMSShutdown ? "true" : "false", PCSShutdown ? "true" : "false");

            if (amap["ShutdownCmd"]->getbParam("FullShutdown"))  // && (!strcmp(PCSStatusResp, "OFF") ||
                                                                 // !strcmp(PCSStatusResp, "FLT"))) // ||
                                                                 // PCSStatusResp == Power_Electronics::POWERUP ))
                                                                 // //
            {
                if (!BMSFaulted && !BMSShutdown && PCSShutdown)  // || PCSFaulted)) // This logic was added to guarantee
                                                                 // no cascade is done without PCS being shutdown first.
                                                                 // Might need to be changed.
                {
                    if (0)
                        FPS_ERROR_PRINT(
                            "%s >>>> cascading function to >>>>> Manager [%s] "
                            "at time: %f\n ",
                            __func__, amc->name.c_str(), tNow);
                    if (UiShutdownAv->gotParam("ShutdownBMS") && UiShutdownAv->getbParam("ShutdownBMS"))
                    {
                        ShutdownBMS(vmap, amc->amap, amc->name.c_str(), p_fims, &Avc);
                    }
                }
            }
        }
    }

    // This is the manager determining if everything is shutting down on time:
    // double ShutdownTime = amap["ShutdownTime"]->getdVal();
    // double maxPCSShutdownTime = amap["maxPCSShutdownTime"]->getdVal();
    // double maxBMSShutdownTime = amap["maxBMSShutdownTime"]->getdVal();
    // ShutdownTime = amap["ShutdownTime"]->getdVal();
    // if (!PCSShutdown && (tNow - ShutdownTime) > maxPCSShutdownTime)
    // {
    //     amap["PCSShutdown"]->setVal(true);
    //     amap["PCSShutdownForced"]->setVal(true);
    //     vm->sendAlarm(vmap, "/status/ess:ShutdownRequested",
    //     "/components/ess:alarms", "PCSShutdown", "PCS Shutdown Forced", 1);

    //     // TODO: put an alarm here
    //     if (1) FPS_ERROR_PRINT(" %s >>>> Forced PCS Shutdown State at time %f
    //     SDtime %f elap %f max %f \n"
    //         , __func__
    //         , tNow
    //         , ShutdownTime
    //         , (tNow - ShutdownTime)
    //         , maxPCSShutdownTime
    //     );
    // }
    // May have to delay timing to allow for PCS Shutdown
    // if (PCSShutdown && (!BMSShutdown && (tNow - ShutdownTime) >
    // maxBMSShutdownTime))
    // {
    //     amap["BMSShutdown"]->setVal(true);
    //     amap["BMSShutdownForced"]->setVal(true);
    //     // TODO: put an alarm here
    //     if (1) FPS_ERROR_PRINT(" %s >> Forced BMS Shutdown State at time %f
    //     \n", __func__, tNow); vm->sendAlarm(vmap,
    //     "/status/ess:ShutdownRequested", "/components/ess:alarms",
    //     "BMSShutdown", "BMS Shutdown Forced", 1);
    // }

    if (amap["BMSShutdown"]->getbVal() && amap["PCSShutdown"]->getbVal())  // Fully shut down
    {
        const char* cVal = "Full Shutdown";
        amap["SystemState"]->setVal(cVal);
        cVal = "Full Shutdown Completed";
        amap["SystemStateStep"]->setVal(cVal);

        amap["ShutdownCmd"]->setParam("ShutdownRequested", false);
        amap["ShutdownCmd"]->setParam("HardShutdown", false);
        amap["ShutdownCmd"]->setParam("FullShutdown", false);
        if (1)
            FPS_ERROR_PRINT("%s >> Full Shutdown Completed, elapsed time: %f \n", __func__, (tNow - sdtime));
        vm->sendAlarm(vmap, "/status/ess:ShutdownRequested", "/components/ess:alarms", "Shutdown", "Shutdown Completed",
                      1);
    }
    else if (!amap["ShutdownCmd"]->getbParam("FullShutdown") &&
             amap["PCSShutdown"]->getbVal())  ///*(!amap["ShutdownCmd"]->getbParam("FullShutdown") ||
                                              ///*/
    {
        const char* cVal = "PCS Shutdown";
        amap["SystemState"]->setVal(cVal);
        cVal = "PCS Shutdown Completed";
        amap["SystemStateStep"]->setVal(cVal);

        amap["ShutdownCmd"]->setParam("ShutdownRequested", false);
        amap["ShutdownCmd"]->setParam("HardShutdown", false);
        if (1)
            FPS_ERROR_PRINT("%s >> PCS Shutdown Completed, elapsed time: %f \n", __func__, (tNow - sdtime));
        vm->sendAlarm(vmap, "/status/ess:ShutdownRequested", "/components/ess:alarms", "Shutdown", "Shutdown Completed",
                      1);
    }
    else if (amap["PCSShutdown"]->getbParam("Fault"))
    {
        const char* cVal = "PCS Shutdown";
        amap["SystemState"]->setVal(cVal);
        cVal = "PCS Shutdown Failed";
        amap["SystemStateStep"]->setVal(cVal);

        amap["ShutdownCmd"]->setParam("ShutdownRequested", false);
        amap["ShutdownCmd"]->setParam("HardShutdown", false);
        if (1)
            FPS_ERROR_PRINT("%s >> PCS Shutdown Failed, elapsed time: %f \n", __func__, (tNow - sdtime));
        vm->sendAlarm(vmap, "/status/ess:ShutdownRequested", "/components/ess:alarms", "Shutdown", "Shutdown Completed",
                      1);
    }
    else if (amap["BMSShutdown"]->getbParam("Fault"))
    {
        const char* cVal = "BMS Shutdown";
        amap["SystemState"]->setVal(cVal);
        cVal = "BMS Shutdown Failed";
        amap["SystemStateStep"]->setVal(cVal);

        amap["ShutdownCmd"]->setParam("ShutdownRequested", false);
        amap["ShutdownCmd"]->setParam("HardShutdown", false);
        if (1)
            FPS_ERROR_PRINT("%s >> BMS Shutdown Failed, elapsed time: %f \n", __func__, (tNow - sdtime));
        vm->sendAlarm(vmap, "/status/ess:ShutdownRequested", "/components/ess:alarms", "Shutdown", "Shutdown Completed",
                      1);
    }

    return 0;
}

// ShutdownPCS
// * This function will consist of the following sequence (run by ESS
// controller)
//  *     1) Send to PCS_Manager the ShutdownPCS's command (SIDENOTE: have
//  PCS_Manager set [ActivePowerSetPoint && ReactivePowerSetPoint] to 0 )
//  *     2) Wait for PCS_Manager to respond (get ok that all PCS's are
//  Shutdown)
//  *     3) Send to BMS_Manager the ShutdownBMS's command
//  *     4) Wait for BMS_Manager to respond (get ok that all BMS's are
//  Shutdown)
//  *     ... add other system as needed
//  *     General form:
//  *         - Send ShutdownAsset's to their manager
//  *         - Wait for response before proceeding
// TODO: Similar to BMS, we might only need a single register, just set global
// active/reactivepower setpoints to zero. Statuses, etc.:
/*


        // check this linkVals(*vm, vmap, amap, aname, "/status", ival,
   "PCSStatusResp", "PCSKeyStop");   // returned status

        // other variables:
        // todo: link EStop to aname = "ess" not itself.
        // Check this linkVals(*vm, vmap, amap, "ess", "/status", bval,
   "PCSFaulted", "PCSFaultSeen", "ShutdownRequest", "ShutdownRequested");
        ///linkVals(*vm, vmap, amap, aname, "/status", bval, "PCSKeyStopSent");
        //linkVals(*vm, vmap, amap, aname, "/config", ival, "PCSKeyStopCmd");
        //linkVals(*vm, vmap, amap, aname, "/config", ival, "PCSFaultStatus");

        // EStop:
        linkVals(*vm, vmap, amap, "ess", "/status", bval, "EStop");
        char *
        /status/ess/SystemState
               used to indicate system state : Init, Fault, Ready , StandBy ,
   Run /status/ess/SystemStateStep used to indicate system state step
        /status/pcs/PCSSystemState
               used to indicate system state : Init, Fault, Ready , StandBy ,
   Run /status/pcs/PCSSystemStateStep used to indicate system state step

        double
        /status/ess/CurrentSetpoint
                     the current Setpoint +ve for discharge -Ve for charge  ??

        /config/ess/ActivePowerSetpoint
                     the current activePower Setpoint  From SM or UI
        /config/ess/ReactivePowerSetpoint
                     the current reactivePower Setpoint  From SM or UI

        // Config
        /config/ess/PowerSetpointDeadband
                     the current reactivePower Setpoint  From SM or UI

        // Feedback from PCS Jimmy ???
        /status/pcs/PCSActivePower
                     the current activePower   From PCS
        /status/pcs/PCSReactivePower
                     the current reactivePower   From PCS

        /status/ess/ShutdownTime
                    time the system took to shutdown.
        /status/pcs/PCSShutdownTime
                    time the system took to shutdown.
        /status/pcs/PCSShutdownTimeStart
                    time the system started the shutdown.

        // Config
        /config/ess/maxPCSShutdownTime
                    time limits to allow for PCS system to complete shutdown.


        linkVals(*vm, vmap, amap, aname, "/status", ival, "
        // ?? /status/ess/BmsStatus   status of the BmsSystem   from modbus
   input
        // ?? /status/ess/PCsStatus   status of the PcsSystem
        // ?? /status/ess/PCsStatus   status of the EmuSystem    from ??
        // ?? /status/ess/PCsStatus   status of the EmmuSystem    from ??

        // other variables:
        bool

        /status/ess/EStop             estop input flag , several possible
   sources. TODO Should be attached directly to Shutdown.

        /status/ess/PCSShutdown         Check this
        /status/ess/PCSStatus           and this

        /status/ess/PCSFaulted        Indication that the PCS system was faulted
        /status/pcs/PCSFaultSeen      Indication that the PCS fault has been
   seen

        NOTE we may set the next three up as integers. to keep track of multiple
   requests. /status/ess/ShutdownRequest      Main trigger for a shutdown start
   , set by any system that wants to initiate a shutdown. We'll attach an alarm
   to it later. /status/ess/ShutdownRequested    Indication that we have
   responded to the latest shutdown request. /status/ess/ShutdownCompleted
   Indication that we have completed the latest Shutdown Request bool
        /status/ess/PCSShutdownForced     Indication that we forced the PCS to
   shutdown It did not  resond to the current request.
        /status/ess/BMSShutdownForced     Indication that we forces the BMS
   shutdown.

        /status/ess/PCSShutdown           Indication that the PCSShutdown has
   completed /status/ess/BMSShutdown           Indication that the BMSShutdown
   has completed

        /status/ess/ShutdownSim           Request for simulation code path
   tester  reset as soon as SimSeen is set. /status/ess/ShutdownSimSeen
   Indication that the simulation request has been seen , reset when the
   simulation has completed

        /status/ess/ShutdownCompleted     Indication that Shutdown has been
   completed. To Be reset when another transition brigs the system out of
   shutdown.

        double
        /status/pcs/PCSKeyCmdTime            Place to send  PCS command.  from
   ModBus Map Jimmy

        int
        /status/ess/PCSStopSent           command sent to  stop to the PCS
        /status/pcs/PCSStatusResp         Code indicating the current PCS
   status.  from ModBus Map Jimmy /status/bms/BMSStatusResp         Code
   indicating the current BMS status.  from ModBudMap Jimmy
        /status/pcs/PCSKeyCmd            Place to send  PCS command.  from
   ModBus Map Jimmy /status/pcs/PCSKeyCmdTries       NUmber of attempts to send
   PCS Command



*/

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
    double tNow = vm->get_time_dbl();
    essPerf ePerf(am, aname, __func__);

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
        // linkVals(*vm, vmap, amap, "ess", "/status", tValInit, "SystemStateStep");
        linkVals(*vm, vmap, amap, "ess", "/status", bval,
                 "PCSShutdown");  //, "ShutdownCmd", "HardShutdown");
        linkVals(*vm, vmap, amap, aname, "/sched", cval, "schedShutdownPCS");

        linkVals(*vm, vmap, amap, aname, "/status", tValInit, "SystemState", "PCSSystemStateStep");
        linkVals(*vm, vmap, amap, aname, "/status", cval, "PCSStatusResp");
        linkVals(*vm, vmap, amap, aname, "/status", dval, "PCSShutdownTime");
        linkVals(*vm, vmap, amap, aname, "/status", bval, "HardShutdown");
        linkVals(*vm, vmap, amap, aname, "/status", ival, "KeyCmd", "PCSStopKeyCmd", "PCSEStopKeyCmd");

        linkVals(*vm, vmap, amap, aname, "/config", dval, "maxShutdownTime", "maxKeyCmdOnTime", "maxKeyCmdTime");
        linkVals(*vm, vmap, amap, aname, "/config", ival, "maxKeyCmdTries");
        if (1)
            FPS_ERROR_PRINT(
                "%s >>>>>>>>>>>>>>>>PCS Shutdown system step 1 reload "
                "[%d] at time: %f \n",
                __func__, reload, tNow);
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

        amap["KeyCmd"]->setParam("maxKeyCmdOnTime", amap["maxKeyCmdOnTime"]->getdVal());
        amap["KeyCmd"]->setParam("maxKeyCmdTime", amap["maxKeyCmdTime"]->getdVal());
        amap["KeyCmd"]->setParam("maxKeyCmdTries", amap["maxKeyCmdTries"]->getiVal());
        if (1)
            FPS_ERROR_PRINT(
                "%s >>>>>>>>>>>>>>>>PCS Shutdown system state [%s] "
                "reload [%d] at time: %f \n",
                __func__, amap["SystemState"]->getcVal(), reload, tNow);
        FPS_ERROR_PRINT("HardShutdown: %d\n", amap["HardShutdown"]->getbVal());

        pcsAv->setVal(2);

        return 0;
    }

    assetVar* PCSKeyCmd = amap["HardShutdown"]->getbVal() ? amap["PCSEStopKeyCmd"] : amap["PCSStopKeyCmd"];
    char* SystemState = amap["SystemState"]->getcVal();
    char* PCSSystemStateStep = amap["PCSSystemStateStep"]->getcVal();
    if (0)
        FPS_ERROR_PRINT("%s >> Running reload %d state [%s] step [%s]\n", __func__, reload, SystemState,
                        PCSSystemStateStep);

    char* PCSStatusResp = amap["PCSStatusResp"]->getcVal();

    if (!PCSStatusResp)  // comms OK as well?
    {
        amap["PCSShutdown"]->setParam("Fault", true);
        PCSSystemStateStep = (char*)"PCS Shutdown Communication Failed";
        amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
        if (1)
            FPS_ERROR_PRINT("%s >> No communication with PCS at time: %f \n", __func__, tNow);
        // Send Event - fault
        assetVar* temp_av = amap["PCSShutdown"];
        if (temp_av)
            temp_av->sendEvent("PCS", am->p_fims, Severity::Fault, "PCS comms lost at time %2.3f", vm->get_time_dbl());
        amap["schedShutdownPCS"]->setParam("endTime", 1);
        amap["schedShutdownPCS"]->setVal("schedShutdownPCS");
        pcsAv->setVal(1);
        return 0;
    }

    // SystemState holds state from PCS decoded into either Ready, Starting, Ready
    // and Shutdown
    if (amap["SystemState"]->valueChangedReset() || amap["HardShutdown"]->getbVal() ||
        !strcmp(PCSSystemStateStep, "Init"))
    {
        amap["pcsRunKeyCmd"]->setVal(0);  // Reset key cmd function
        RunKeyCmd(vmap, amap, aname, nullptr, aV);
        amap["PCSSystemStateStep"]->setVal(SystemState);
        amap["HardShutdown"]->setVal(false);
        if (1)
            FPS_ERROR_PRINT("%s >>> State step changed to %s\n", __func__, SystemState);
        // Send Event - info
        assetVar* temp_av = amap["pcsRunKeyCmd"];
        if (temp_av)
            temp_av->sendEvent("PCS", am->p_fims, Severity::Info, "PCS state changed to [%s] at time %2.3f",
                               SystemState, vm->get_time_dbl());
    }

    if (strcmp(SystemState, "Running") == 0 &&
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
            assetVar* temp_av = amap["PCSShutdown"];
            if (temp_av)
                temp_av->sendEvent("PCS", am->p_fims, Severity::Fault, "PCS stuck in hold state [%s] at time %2.3f",
                                   PCSStatusResp, vm->get_time_dbl());
            amap["schedShutdownPCS"]->setParam("endTime", 1);
            amap["schedShutdownPCS"]->setVal("schedShutdownPCS");
            pcsAv->setVal(1);
        }
        else
        {
            amap["HardShutdown"]->setVal(true);
            amap["HardShutdown"]->setParam("Latch", true);
            PCSSystemStateStep = (char*)"PCS Starting Hard Shutdown";
            amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
            if (1)
                FPS_ERROR_PRINT("%s >> PCS stuck in hold state %s, trying e-stop at time: %f \n", __func__,
                                PCSStatusResp, tNow);
            // Send Event - info
            assetVar* temp_av = amap["ShutdownCmd"];
            if (temp_av)
                temp_av->sendEvent("PCS", am->p_fims, Severity::Info,
                                   "PCS stuck in hold state [%s], trying estop at time %2.3f", PCSStatusResp,
                                   vm->get_time_dbl());
        }
    }
    else if (strcmp(SystemState, "Shutdown") == 0 || strcmp(SystemState, "Fault") == 0 ||
             strcmp(SystemState, "Off") == 0)
    {
        amap["PCSShutdown"]->setVal(true);
        PCSSystemStateStep = (char*)"PCS Shutdown Complete";
        amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
        if (1)
            FPS_ERROR_PRINT("%s >> All PCS's succesfully shut down at time: %f \n", __func__, tNow);
        // Send Event - info
        assetVar* temp_av = amap["PCSShutdown"];
        if (temp_av)
            temp_av->sendEvent("PCS", am->p_fims, Severity::Info, "All PCS shutdown successfully at time %2.3f",
                               vm->get_time_dbl());
        amap["schedShutdownPCS"]->setParam("endTime", 1);
        amap["schedShutdownPCS"]->setVal("schedShutdownPCS");
        pcsAv->setVal(1);
    }
    else if (strcmp(SystemState, "Ready") == 0 && !amap["HardShutdown"]->getbParam("Latch"))
    {
        amap["PCSShutdown"]->setVal(true);
        PCSSystemStateStep = (char*)"PCS Shutdown Complete";
        amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
        if (1)
            FPS_ERROR_PRINT("%s >> All PCS's succesfully shut down at time: %f \n", __func__, tNow);
        // Send Event - info
        assetVar* temp_av = amap["PCSShutdown"];
        if (temp_av)
            temp_av->sendEvent("PCS", am->p_fims, Severity::Info, "PCS hard-shutdown success at time %2.3f",
                               vm->get_time_dbl());
        amap["schedShutdownPCS"]->setParam("endTime", 1);
        amap["schedShutdownPCS"]->setVal("schedShutdownPCS");
        pcsAv->setVal(1);
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
            assetVar* temp_av = amap["PCSShutdown"];
            if (temp_av)
                temp_av->sendEvent("PCS", am->p_fims, Severity::Fault, "PCS not responding in state [%s] at time %2.3f",
                                   PCSStatusResp, vm->get_time_dbl());
            amap["schedShutdownPCS"]->setParam("endTime", 1);
            amap["schedShutdownPCS"]->setVal("schedShutdownPCS");
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
            assetVar* temp_av = amap["PCSShutdown"];
            if (temp_av)
                temp_av->sendEvent("PCS", am->p_fims, Severity::Info, "PCS not responding, trying e-stop at time %2.3f",
                                   vm->get_time_dbl());
        }
    }
    else if (strcmp(SystemState, "Running") == 0 || strcmp(SystemState, "Ready") == 0)
    {
        RunKeyCmd(vmap, amap, aname, nullptr, aV);
    }

    if (amap["PCSShutdown"]->getbVal() && amap["KeyCmd"]->getbVal())
    {
        amap["KeyCmd"]->setVal(false);
        FPS_ERROR_PRINT("%s > One last keycmd to false\n", __func__);
    }

    if (amap["KeyCmd"]->valueChangedReset())
    {
        varsmap* vlist = vm->createVlist();
        if (amap["KeyCmd"]->getbVal())
        {
            PCSKeyCmd->setVal(1);  // todo: config
            vm->addVlist(vlist, PCSKeyCmd);
            // am->Send("set", stopUri, nullptr, "{\"value\":1}");
        }
        else
        {
            PCSKeyCmd->setVal(0);  // todo: config
            vm->addVlist(vlist, PCSKeyCmd);
            // am->Send("set", stopUri, nullptr, "{\"value\":0}");
        }
        vm->sendVlist(p_fims, "set", vlist);
        vm->clearVlist(vlist);
    }

    return 0;
}

// NOTE: Needs aname to be a particular bms_number - Might be enum instead.
// TODO: Will just send out a single command to an MBMU register.
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
        // linkVals(*vm, vmap, amap, "ess", "/status", tValInit, "SystemState",
        // "SystemStateStep");
        linkVals(*vm, vmap, amap, "ess", "/status", bval, "BMSShutdown");
        linkVals(*vm, vmap, amap, aname, "/sched", cval, "schedShutdownBMS");

        linkVals(*vm, vmap, amap, aname, "/status", tValInit, "BMSSystemState", "BMSSystemStateStep");
        linkVals(*vm, vmap, amap, aname, "/status", cval, "BMSPowerOn", "BMSStatus");
        linkVals(*vm, vmap, amap, aname, "/status", dval, "BMSShutdownTime");
        linkVals(*vm, vmap, amap, aname, "/status", ival, "KeyCmd", "BMSKeyCmd");

        linkVals(*vm, vmap, amap, aname, "/config", dval, "maxShutdownTime", "maxKeyCmdOnTime", "maxKeyCmdTime");
        linkVals(*vm, vmap, amap, aname, "/config", ival, "maxKeyCmdTries");

        amap["BMSKeyCmd"]->setVal(1);
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
        RunKeyCmd(vmap, amap, aname, nullptr, aV);
        if (1)
            FPS_ERROR_PRINT(
                "%s >>>>>>>>>>>>>>>>BMS Shutdown system state [%s] "
                "reload %d at time: %f \n",
                __func__, amap["BMSSystemState"]->getcVal(), reload, tNow);

        bmsAv->setVal(2);

        return 0;
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
        auto temp_av = amap["BMSShutdown"];
        if (temp_av)
            temp_av->sendEvent("BMS", am->p_fims, Severity::Fault, "BMS comms lost at time %2.3f", vm->get_time_dbl());
        amap["schedShutdownBMS"]->setParam("endTime", 1);
        amap["schedShutdownBMS"]->setVal("schedShutdownBMS");
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
            RunKeyCmd(vmap, amap, aname, nullptr, aV);
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
        auto temp_av = amap["BMSShutdown"];
        if (temp_av)
            temp_av->sendEvent("BMS", am->p_fims, Severity::Info, "All BMS shutdown successfully at time %2.3f",
                               vm->get_time_dbl());
        amap["schedShutdownBMS"]->setParam("endTime", 1);
        amap["schedShutdownBMS"]->setVal("schedShutdownBMS");
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
        auto temp_av = amap["BMSShutdown"];
        if (temp_av)
            temp_av->sendEvent("BMS", am->p_fims, Severity::Fault, "BMS shutdown timed out in state [%s] at time %2.3f",
                               BMSPowerOn, vm->get_time_dbl());
        amap["schedShutdownBMS"]->setParam("endTime", 1);
        amap["schedShutdownBMS"]->setVal("schedShutdownBMS");
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
        auto temp_av = amap["BMSShutdown"];
        if (temp_av)
            temp_av->sendEvent("BMS", am->p_fims, Severity::Fault, "BMS not responding at time %2.3f",
                               vm->get_time_dbl());
        amap["schedShutdownBMS"]->setParam("endTime", 1);
        amap["schedShutdownBMS"]->setVal("schedShutdownBMS");
        bmsAv->setVal(1);
    }
    else if (strcmp(BMSSystemStateStep, "Shutdown Fault") == 0)
    {
        amap["BMSShutdown"]->setParam("Fault", true);
        BMSSystemStateStep = (char*)"BMS Shutdown Failed";
        amap["BMSSystemStateStep"]->setVal(BMSSystemStateStep);
        if (1)
            FPS_ERROR_PRINT("%s >> BMS in failed state at time: %f \n", __func__, tNow);
        // Send Event - fault
        auto temp_av = amap["BMSShutdown"];
        if (temp_av)
            temp_av->sendEvent("BMS", am->p_fims, Severity::Fault, "BMS in failed state at time %2.3f",
                               vm->get_time_dbl());
        amap["schedShutdownBMS"]->setParam("endTime", 1);
        amap["schedShutdownBMS"]->setVal("schedShutdownBMS");
        bmsAv->setVal(1);
    }
    else if (strcmp(BMSSystemStateStep, "Ready") == 0)
    {
        RunKeyCmd(vmap, amap, aname, nullptr, aV);
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
            amap["BMSKeyCmd"]->setVal(3);  // todo: config
            vm->addVlist(vlist, amap["BMSKeyCmd"]);
            // am->Send("set", "/components/catl_ems_bms_rw/ems_cmd", nullptr,
            // "{\"value\":3}");
        }
        else
        {
            amap["BMSKeyCmd"]->setVal(1);  // todo: config
            vm->addVlist(vlist, amap["BMSKeyCmd"]);
            // am->Send("set", "/components/catl_ems_bms_rw/ems_cmd", nullptr,
            // "{\"value\":1}");
        }
        vm->sendVlist(p_fims, "set", vlist);
        vm->clearVlist(vlist);
    }

    return 0;
}

// DO NOT need anything below this comment:

// NOTE: Needs aname to be a particular bms_number - Might be enum instead.
// IMPORTANT: This is DEPRECATED, we only control a single register to tell all
// of the BMS's to Shutdown.
int ShutdownBMSasset(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    // Timer ShutdownBMSTimer("ShutdownBMSTimer", true);

    int reload = 0;
    bool bval = false;
    int ival = 0;
    char* tValInit = (char*)"Init";
    char* tValRun = (char*)"Run";
    // char *tValPowerOffReady = (char*)"Power off Ready";
    // char *tValPowerOnReady = (char*)"Power on Ready";
    // char *tValPowerOffFault = (char*)"Power off Fault";
    // char *tValPowerOnFault = (char*)"Power on Fault";
    double dval = 0.0;
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    double tNow = vm->get_time_dbl();
    essPerf ePerf(am, aname, __func__);

    if (0)
        FPS_ERROR_PRINT("%s >> Shutdown BMSAsset called for (BMS: %s) at time: %f \n", __func__, aname, tNow);

    // char* tVal = (char*)"Test TimeStamp";
    // in future use tval for sending messages out - for UI and alert purposes.

    assetVar* bmsAv = amap[__func__];
    if (!bmsAv || (reload = bmsAv->getiVal()) == 0)
    {
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }

    if (reload < 2)
    {
        linkVals(*vm, vmap, amap, aname, "/reload", reload, __func__);
        bmsAv = amap[__func__];
        // reload
        linkVals(*vm, vmap, amap, "ess", "/status", tValInit, "SystemState", "SystemStateStep");
        // linkVals(*vm, vmap, amap, "ess", "/config", dval,
        // /*"ActivePowerSetpoint", "ReactivePowerSetpoint",*/ "ShutdownTime");
        // linkVals(*vm, vmap, amap, "ess", "/config", dval,
        // "PCSKeyActivePowerSetpoint", "PCSKeyReactivePowerSetpoint");
        // linkVals(*vm, vmap, amap, "ess", "/status", dval, "PCSActivePower",
        // "PCSReactivePower");   // responses
        linkVals(*vm, vmap, amap, "ess", "/config", dval, "maxBMSShutdownTime");
        // linkVals(*vm, vmap, amap, "ess", "/config", dval,
        // "PowerSetpointDeadband");
        linkVals(*vm, vmap, amap, aname, "/status", tValInit, "BMSSystemState", "BMSSystemStateStep");
        linkVals(*vm, vmap, amap, aname, "/status", dval, "BMSShutdownTime", "BMSShutdownStart");
        linkVals(*vm, vmap, amap, "ess", "/config", dval, "maxBMSShutdownTime");
        linkVals(*vm, vmap, amap, "ess", "/status", bval, "EStop", "BMSShutdown");
        linkVals(*vm, vmap, amap, "bms", "/status", ival, "BMSnumFaults", "BMSnumPresent");

        linkVals(*vm, vmap, amap, aname, "/status", tValInit,
                 "BMSStatus");  //"Power off Ready"
        linkVals(*vm, vmap, amap, aname, "/status", ival, "BMSKeyCmd", "BMSKeyStatus");
        linkVals(*vm, vmap, amap, aname, "/status", bval, "BMSFaulted");

        // Estop:
        linkVals(*vm, vmap, amap, "ess", "/status", bval, "EStop");

        // amap[__func__] = vm->setLinkVal(vmap, aname,
        // "/reload", __func__, reload);
        bmsAv = amap[__func__];

        // other variables:
        // amap["EMMUFaulted"] = vm->setLinkVal(vmap, aname, "/status",
        // "EMMUFaulted", bval);  amap["ShutdownRequest"] = vm->setLinkVal(vmap,
        // aname, "/status", "ShutdownRequest", bval);  amap["ShutdownRequested"] =
        // vm->setLinkVal(vmap, aname, "/status", "ShutdownRequested", bval);

        if (reload == 0)  // complete restart
        {
            amap["BMSKeyStatus"]->setVal(0);
            amap["BMSFaulted"]->setVal(false);
            amap["BMSSystemState"]->setVal(tValInit);
            amap["BMSSystemStateStep"]->setVal(tValInit);
            amap["BMSStatus"]->setVal(tValInit);
        }
        reload = 2;
        bmsAv->setVal(reload);
    }

    // want to see: timer for regular Shutdown vs. EStop Shutdown. Use tNow
    // (differences) in FPS_ERROR_PRINTs.

    // get the reference to the variable
    // assetVar* ShutdownRequestedAV = amap["ShutdownRequested"];
    // assetVar* PCSkeyStopCmdAV = amap["PCSKeyStopCmd"];
    // assetVar* PCSkeyCmdAV = amap["PCSKeyCmd"];

    // storing asset states:
    // bool EMMUFaulted = amap["EMMUFaulted"]->getbVal();
    bool EStop = amap["EStop"]->getbVal();

    bool BMSFaulted = amap["BMSFaulted"]->getbVal();
    bool BMSShutdown = amap["BMSShutdown"]->getbVal();
    double BMSShutdownTime = amap["BMSShutdownTime"]->getdVal();
    double maxBMSShutdownTime = amap["maxBMSShutdownTime"]->getdVal();
    char* BMSStatus = amap["BMSStatus"]->getcVal();

    // int PowerOffStatus = 3;  // todo config
    int PowerOffCmd = 3;  // todo config

    // TODO: Is this logic complete below?

    if (BMSShutdown)
    {
        amap["BMSnumPresent"]->subVal(1);
        return 0;
    }
    if (BMSFaulted || EStop)
    {
        amap["BMSnumFaults"]->addVal(1);
        amap["BMSShutdown"]->setVal(true);
        amap["BMSnumPresent"]->subVal(1);  // going to be used elsewhere?
        // send activePower reactivepower to 0
        amap["BMSKeyCmd"]->setVal(PowerOffCmd);
        return 0;
    }

    // PowerOffReady
    // char* tOnReady = (char *)"Power on Ready";
    // char* tOffFault = (char *)"Power on Fault";

    // char* tInitStat = (char*)"Initial status";
    // char* tNormStat = (char*)"Normal status";

    // char* tFullChargeStat = (char *)"Full Charge status";
    // char* tFullDischargeStat = (char *)"Full Discharge status";

    char* BMSSystemState = amap["BMSSystemState"]->getcVal();
    // Handle init
    if ((strcmp(BMSSystemState, tValInit) == 0) || (strcmp(BMSSystemState, tValRun) == 0))
    {
        char* cval = (char*)"BMS Shutdown";
        char* cvals = (char*)"Setting Power Off";
        amap["BMSSystemState"]->setVal(cval);
        amap["BMSSystemStateStep"]->setVal(cvals);

        // send activePower reactivepower to 0
        amap["BMSKeyCmd"]->setVal(PowerOffCmd);
        amap["BMSShutdownStart"]->setVal(tNow);
    }

    // Handle BMS warning
    // Must match modbus config

    // char* tWarningStat = (char*)"Warning status";

    // if (strcmp(BMSStatus, tValPowerOffFault) == 0)
    // {
    //     char* cval = (char*)"BMS Shutdown";
    //     char* cvals = (char*)"BMS Status Warning";
    //     amap["BMSSystemState"]->setVal(cval);
    //     amap["BMSSystemStateStep"]->setVal(cvals);

    //     // send activePower reactivepower to 0
    //     amap["BMSKeyCmd"]->setVal(PowerOffCmd);
    //     amap["BMSShutdownStart"]->setVal(tNow);
    //     amap["BMSnumFaults"]->addVal(1);
    //     amap["BMSShutdown"]->setVal(true);
    //     amap["BMSnumPresent"]->subVal(1);
    //     return 0;
    // }
    // Handle BMS Fault Status
    // Must match modbus config
    char* tFaultStat = (char*)"Fault status";
    if (strcmp(BMSStatus, tFaultStat) == 0)
    {
        char* cval = (char*)"BMS Shutdown";
        char* cvals = (char*)"BMS Status Fault";
        amap["BMSSystemState"]->setVal(cval);
        amap["BMSSystemStateStep"]->setVal(cvals);

        // send activePower reactivepower to 0 - This is for PCS
        amap["BMSKeyCmd"]->setVal(PowerOffCmd);
        // todo send fims message
        amap["BMSShutdownStart"]->setVal(tNow);
        amap["BMSnumFaults"]->addVal(1);
        amap["BMSShutdown"]->setVal(true);
        amap["BMSnumPresent"]->subVal(1);
        return 0;
    }
    // Handle BMS Fault Status
    // Must match modbus config
    // NOTE: Does this need keyCmd?
    char* tOffFault = (char*)"Power off Fault";

    if (strcmp(BMSStatus, tOffFault) == 0)
    {
        char* cval = (char*)"BMS Shutdown";
        char* cvals = (char*)"BMS PowerOff  Fault";
        amap["BMSSystemState"]->setVal(cval);
        amap["BMSSystemStateStep"]->setVal(cvals);

        // send activePower reactivepower to 0
        // amap["BMSKeyCmd"]->setVal(PowerOffCmd);
        // amap["BMSShutdownStart"]->setVal(tNow);
        amap["BMSnumFaults"]->addVal(1);
        amap["BMSShutdown"]->setVal(true);
        amap["BMSnumPresent"]->subVal(1);
        return 0;
    }
    // Completed
    // NOTE: Does this need keyCmd?
    char* tOffReady = (char*)"Power off Ready";
    if (strcmp(BMSStatus, tOffReady) == 0)
    {
        char* cval = (char*)"BMS Shutdown";
        char* cvals = (char*)"BMS Shutdown Completed";
        amap["BMSSystemState"]->setVal(cval);
        amap["BMSSystemStateStep"]->setVal(cvals);

        // send activePower reactivepower to 0
        amap["BMSShutdown"]->setVal(true);
        amap["BMSnumPresent"]->subVal(1);
        return 0;
    }

    // timed out ??
    // keyStopCmd for timeout needed? - what to do?
    // Does this need a special protocol?
    if (((tNow - BMSShutdownTime) > maxBMSShutdownTime) && !BMSShutdown)
    {
        char* cval = (char*)"BMS Shutdown";
        char* cvals = (char*)"Shutdown Timeout";
        amap["BMSSystemState"]->setVal(cval);
        amap["BMSSystemStateStep"]->setVal(cvals);
        amap["BMSFaulted"]->setVal(true);
        amap["BMSShutdown"]->setVal(true);
        // ToDo Send Alarm
        if (1)
            FPS_ERROR_PRINT("%s >>>>>> Timeout Forced BMS Shutdown at %f \n ", __func__, tNow);
        amap["BMSnumPresent"]->subVal(1);
        amap["BMSnumFaults"]->addVal(1);
    }
    return 0;
}

// NOTE: Needs aname to be a particular pcs_number - Might be enum instead.
// TODO: Need to actually implement this properly, only BMS is done right now.
// IMPORTANT: Might be deprecated, need to check out registers.
int ShutdownPCSasset(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    // Timer ShutdownBMSTimer("ShutdownBMSTimer", true);

    // TODO: do this function in the future.

    int reload = 0;
    bool bval = false;
    int ival = 0;
    // double dval = 0.0;
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    essPerf ePerf(am, aname, __func__);

    // char* tVal = (char*)"Test TimeStamp";
    // in future use tval for sending messages out - for UI and alert purposes.

    assetVar* pcsAv = amap[__func__];  // Need to figure out
                                       // the resolution here!
    if (!pcsAv || (reload = pcsAv->getiVal()) == 0)
    {
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }

    if (reload < 2)
    {
        // reload
        amap[__func__] = vm->setLinkVal(vmap, aname, "/reload", __func__, reload);
        pcsAv = amap[__func__];

        // other variables:
        amap["EMMUFaulted"] = vm->setLinkVal(vmap, "ess", "/status", "EMMUFaulted", bval);
        amap["BMSFaulted"] = vm->setLinkVal(vmap, "ess", "/status", "BMSFaulted", bval);
        amap["PCSFaulted"] = vm->setLinkVal(vmap, "ess", "/status", "PCSFaulted", bval);
        amap["EMUFaulted"] = vm->setLinkVal(vmap, "ess", "/status", "EMUFaulted", bval);
        amap["ShutdownRequest"] = vm->setLinkVal(vmap, aname, "/status", "ShutdownRequest", bval);
        amap["ShutdownRequested"] = vm->setLinkVal(vmap, aname, "/status", "ShutdownRequested", bval);
        amap["PCSKeyStopCmd"] = vm->setLinkVal(vmap, aname, "/config", "PCSKeyStopCmd", ival);

        // other variables
        amap["PCSKeyStatus"] = vm->setLinkVal(vmap, aname, "/status", "PCSKeyStatus", ival);
        amap["PCSKeyCmd"] = vm->setLinkVal(vmap, aname, "/status", "PCSKeyCmd", ival);

        if (reload == 0)  // complete restart
        {
            amap["PCSKeyStatus"]->setVal(0);
            amap["PCSKeyCmd"]->setVal(0);
        }
        reload = 2;
        pcsAv->setVal(reload);
    }

    // get the reference to the variable
    // assetVar* ShutdownRequestedAV = amap["ShutdownRequested"];
    // assetVar* PCSkeyStopCmdAV = amap["PCSKeyStopCmd"];
    // assetVar* PCSkeyCmdAV = amap["PCSKeyCmd"];

    // storing asset states:
    // bool EMMUFaulted = amap["EMMUFaulted"]->getbVal();
    // bool BMSFaulted = amap["BMSFaulted"]->getbVal();
    // bool PCSFaulted = amap["PCSFaulted"]->getbVal();
    // bool EMUFaulted = amap["EMUFaulted"]->getbVal(); // Supposed to be EMS?
    // bool ShutdownRequest = amap["ShutdownRequest"]->getbVal();
    // bool ShutdownRequested = amap["ShutdownRequested"]->getbVal();
    // int PCSKeyStopVal = amap["PCSKeyStopCmd"]->getiVal();
    // int PCSKeyStatusVal = amap["PCSKeyStatus"]->getiVal();
    // int PCSKeyCmdVal = amap["PCSKeyCmd"]->getiVal();
    return 0;
}

// void sendKeyStopCmd(/* faultFuncArgs, other args? */) //LC send KEYSTOP
// command to PCS
// {
//     printf("sendKeyStopCmd!\n");
//     //     if(1) FPS_ERROR_PRINT("%s >> %s is sending keystop cmd to pcs\n",
//     __func__, aname);
//     //
//     //     VarMapUtils* vm = ai->vm;
//     //
//     //     if (!amap[KeyStop])
//     //     {
//     //         bool bval;
//     //         amap[KeyStop] = vm->setLinkVal(vmap, "pcs", "/controls",
//     "KeyStopCmd", bval);
//     //     }
//     //
//     //     // Need to check if pcs has already received keystop cmd (keystop
//     = true)
//     //
//     //     bool keyStopCmdActive = amap[KeyStop]->getVal(keyStopCmdActive);
//     //     int sysState = amap[SystemStateNum]->getVal(sysState);
//     //     if (!keyStopCmdActive && sysState == System_Fault && aname !=
//     "pcs")
//     //     {
//     //         amap[keyStopCmdActive]->setVal(true);
//     //         vm->sendAssetVar(amap[keyStopCmdActive], p_fims);
//     //     }
//     //
//     //     // After KeyStopCmd has been sent to PCS, we'll probably need to
//     check if the cmd actually went through
//     //     // Could have a /status/pcs/keystop possibly?
// }

void sendStandbyCmd()
{
    printf("Sending standbyCmd!\n");
}

void DCContactorTrip()  // BAT DC switch & contactor trip
{
    printf("DC Contactor Trip!\n");
}

void DCSwitchTrip()  // BAT DC switch & contactor trip
{
    printf("DC switch Trip!\n");
}

void handleMaxCellVoltage()
{
    printf("Handling Max Cell Voltage!\n");
}
}
}  // namespace defaultFault_module
#endif
