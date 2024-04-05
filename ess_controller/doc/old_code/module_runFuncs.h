#ifndef RUNFUNCS_HPP
#define RUNFUNCS_HPP

#include "asset.h"
#include "funcRef.h"
#include "testUtils.h"
#include "varMapUtils.h"

#include <chrono>
#include <string>
#define PCSStartCmd 1
#define PCSStartResp 5
#define PCSCmdTime 0.5
// TODO FROM PHIL:
// take a look at getMapsCJ

// TODO: Put this enum in systemEnums with CATL stuff later:
namespace Power_Electronics
{
enum Inverter_Status_Messages
{
    POWERUP = 0,        // PUP
    INIT = 1,           // initialization
    OFF = 2,            // Off
    PRECHARGE = 3,      // PCHG
    READY = 4,          // REA
    WAIT = 5,           // Wait
    ON = 6,             // On
    STOP = 7,           // Stop
    DISCHARGE = 8,      // DISC
    FAULT = 9,          // FLT
    LVRT = 10,          // Low Voltage Ride Through (algorithm is running)
    OVRT = 11,          // Over Voltage Ride Through (algorithm is running)
    NIGHT_MODE = 12,    // NGHT
    NIGHT_DC_OFF = 13,  // NDCO
    STANDBY = 14,       // STB
    HVPL = 15,          // high voltage phase lost
    // What happened to 16?!
    PRE_ON = 17,            // PRON
    SELF_DIAGNOSIS = 18,    // DIAG
    LCON = 19,              // LC filter contactors on/activated
    PREMAGENTIZATION = 20,  // PRMG
    BANK_BALANCING = 21,    // BBAL
    CVSB = 22               // cv standby algorithm running
};
};

namespace CATL
{
enum BMS_Status_Messages
{
    INIT = 0,
    NORMAL = 1,
    FULLCHARGE = 2,
    FULLDISCHARGE = 3,
    WARNING = 4,
    FAULT = 5
};
enum BMS_Commands
{
    INITIAL = 0,
    STAYSTATUS = 1,
    POWERON = 2,
    POWEROFF = 3
};
};  // namespace CATL

using namespace testUtils;

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

namespace defaultRun_module
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
// void openDCContactor();
// void closeDCContactor();
// void sendKeystartCmd();

// Comms Checks:
// bool checkIfBMSOnline();
// bool checkIfPCSOnline();
// bool checkIfEMUOnline();
// bool checkIfEMSOnline();
// bool checkIfLCOnline();
// bool checkIfEMMUOnline();

// Fault Checks:
bool checkIfBMSFault();
void enterFaultMode();
}

constexpr int INTERNAL_FUNC_COUNT = 2;  // Make sure to change this as needed.
struct Internal_funcRefArray
{
    const funcRef func_array[INTERNAL_FUNC_COUNT] = {
        // funcRef{"openDCContactor", (void*)&openDCContactor},
        // funcRef{"closeDCContactor", (void*)&closeDCContactor},
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
// void openDCContactor()
// {
//     printf("opening DC Contactor!\n");
// }
// void closeDCContactor()
// {
//     printf("closing DC Contactor!\n");
// }
// void sendKeystartCmd()
// {
//     printf("Sending keystart cmd!\n");
// }
// bool checkIfEMMUOnline()
// {
//     return false;
// }
// bool checkIfEMUOnline()
// {
//     return false;
// }
// bool checkIfBMSOnline()
// {
//     return false;
// }
// bool checkIfPCSOnline()
// {
//     return false;
// }
// void enterFaultMode()
// {
//     printf("Entering Fault Mode!\n");
// }
}
}  // namespace internal_funcs
// END INTERNAL_FUNCS NAMESPACE
// ...
// ...

// Extra useful things to use internally (internal_funs:: can get repetitive.)
// using namespace defaultFault_module::internal_funcs;
// #define i_f internal_funcs

// For external reference to these functions if another program needs it.
extern "C" {
// TODO: Section these off so they become easier to manage (single line
// name/comment will do).
// TODO: Make sure they aren't internal/unsafe functions (move them up if they
// are)

// Startups:
void StartupSystem(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am);
void StartupPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am);
void StartupBMS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am);
void StartupPCSasset(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* ai);
void StartupBMSasset(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* ai);

// Other:
}

constexpr int FUNC_COUNT = 1;  // Make sure to change this as needed.
struct funcRefArray
{
    const funcRef func_array[FUNC_COUNT] = {
        funcRef{ "StartupSystem", (void*)&StartupSystem },
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
// inside each of the managers for their Startup's dummy the lower level
// functions as needed "Possibly" use "cascade" (Check ) FOR SAFETY: Call these
// functions directly. Possibly, use a WAKEUP_LEVEL (might not be necessary for
// this.)
/**
 * @brief Proper StartupSystem sequence, what the final produce will look like:
 *
 * This function will be called by (ESS Controller - where this function is
 located under) on the following conditions:
 *     1) estart from ANYWHERE!
 *     2) start command from Site_Controller
 *     3) Internal fault (ANY fault), shut down EVERYTHING! (a "fault start")
 *     4) Through the User Interface (press a button!) - Web UI
 *     ... more? (don't know yet)
 *
 * This function will consist of the following sequence (run by ESS controller)
 *     1) Send to PCS_Manager the StartupPCS's command (SIDENOTE: have
 PCS_Manager set [ActivePowerSetPoint && ReactivePowerSetPoint] to 0 )
 *     2) Wait for PCS_Manager to respond (get ok that all PCS's are Startup)
 *     3) Send to BMS_Manager the StartupBMS's command
 *     4) Wait for BMS_Manager to respond (get ok that all BMS's are Startup)
 *     ... add other system as needed
 *     General form:
 *         - Send StartupAsset's to their manager
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

        /status/ess/StartupTime
                    time the system took to shutdown.

        /config/ess/maxPCSStartupTime
        /config/ess/maxBMSStartupTime
                    time limits to allow for BMS and PCS system to complete
 shutdown.


        linkVals(*vm, vmap, amap, aname, "/status", ival, "
        //?? /status/ess/BmsStatus   status of the BmsSystem   from modbus input
        //?? /status/ess/PCsStatus   status of the PcsSystem
        //?? /status/ess/PCsStatus   status of the EmuSystem    from ??
        //?? /status/ess/PCsStatus   status of the EmmuSystem    from ??

        // other variables:
        bool
        /status/ess/Estart             estart input flag , several possible
 sources. TODO Should be attached directly to Startup. /status/ess/UiStartup
 UserInterface shutdown request /status/ess/SMStartup        Site Manage Startup
 request /status/ess/FaultStartup     Fault Startup Request. Set by the fault
 system /status/ess/EMMUFaulted       (TODO) Indication that the EMMU system was
 faulted /status/ess/BMSFaulted        Indication that the BMS system was
 faulted /status/ess/PCSFaulted        Indication that the PCS system was
 faulted /status/ess/EMUFaulted        (TODO) Indication that the EMMU system
 was faulted

        NOTE we may set the next three up as integers. to keep track of multiple
 requests. /status/ess/StartupRequest      Main trigger for a shutdown start ,
 set by any system that wants to initiate a shutdown. We'll attach an alarm to
 it later. /status/ess/StartupRequested    Indication that we have responded to
 the latest shutdown request. /status/ess/StartupCompleted    Indication that we
 have completed the latest Startup Request bool /status/ess/PCSStartupForced
 Indication that we forced the PCS to shutdown It did not  resond to the current
 request. /status/ess/BMSStartupForced     Indication that we forces the BMS
 shutdown.

        /status/ess/PCSStartup           Indication that the PCSStartup has
 completed /status/ess/BMSStartup           Indication that the BMSStartup has
 completed

        /status/ess/StartupSim           Request for simulation code path tester
 reset as soon as SimSeen is set. /status/ess/StartupSimSeen       Indication
 that the simulation request has been seen , reset when the simulation has
 completed

        /status/ess/StartupCompleted     Indication that Startup has been
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
#define PCSstartCmd 1
#define PCSstartResp 5
#define BMSstartCmd 1
#define BMSstartResp 5
void StartupSystem(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    // TODO: Sim flag, with variables coming out.
    // TODO: estart sim (ignore timers, just Startup everything without waiting),
    // Timer StartupSystemTimer("StartupSystemTimer", true);

    int reload = 0;
    bool bval = false;
    char* tVal = (char*)"Init";
    int ival = 0;
    double dval = 0.0;
    VarMapUtils* vm = am->vm;
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

        // Statuses, etc.:
        linkVals(*vm, vmap, amap, "ess", "/status", tVal, "SystemState", "SystemStateStep");
        linkVals(*vm, vmap, amap, "ess", "/status", dval, "CurrentSetpoint", "StartupTime");
        linkVals(*vm, vmap, amap, "ess", "/config", dval, "maxPCSStartupTime", "maxBMSStartupTime");
        // linkVals(*vm, vmap, amap, aname, "/status", ival, "BmsStatus",
        // "PcsStatus", "EmuStatus", "EmmuStatus");

        // other variables:
        linkVals(*vm, vmap, amap, aname, "/status", bval, "Estart", "UiStartup", "SMStartup", "FaultStartup");
        linkVals(*vm, vmap, amap, aname, "/status", bval, "EstartSeen", "UiStartupSeen", "SMStartupSeen", "StartupCmd",
                 "FaultStartupSeen");
        linkVals(*vm, vmap, amap, "ess", "/status", bval, "EMMUFaulted", "BMSFaulted", "PCSFaulted", "EMUFaulted");
        linkVals(*vm, vmap, amap, aname, "/status", bval, "StartupRequest", "StartupRequested", "StartupCompleted",
                 "PCSStartupForced", "BMSStartupForced");
        // NOTE: These are all under "ess" where it should be, Estart especially
        // needs this.
        linkVals(*vm, vmap, amap, "ess", "/status", bval, "PCSStartup", "BMSStartup", "StartupSim", "StartupSimSeen",
                 "StartupCompleted");
        linkVals(*vm, vmap, amap, "pcs", "/config", ival, "PCSKeyCmd");
        linkVals(*vm, vmap, amap, "pcs", "/status", ival,
                 "PCSStatusResp");  // is this needed here ??
        linkVals(*vm, vmap, amap, "bms", "/config", ival, "BMSKeyCmd");
        linkVals(*vm, vmap, amap, "bms", "/status", ival,
                 "BMSStatusResp");  // is this needed here ??

        // other variables:
        // linkVals(*vm, vmap, amap, aname, "/status", ival, "PCSKeyStatus",
        // "PCSKeyCmd");

        if (reload == 0)  // complete restart
        {
            // TODO: Clean these up!!!

            essAv = amap[__func__];
            // amap["PCSKeyStatus"]->setVal(0);
            // amap["PCSKeyCmd"]->setVal(0);
            // amap["Estart"]->setVal(false); // Startup from estart command -
            // replaced, wording is probably wrong.
            amap["UiStartup"]->setVal(false);         // Startup from UI command
            amap["SMStartup"]->setVal(false);         // Startup from site_manager
            amap["FaultStartup"]->setVal(false);      // Startup from fault condition
            amap["maxPCSStartupTime"]->setVal(15.0);  // config?
            amap["maxBMSStartupTime"]->setVal(15.0);  // config?

            amap["StartupRequested"]->setVal(false);
            amap["StartupSim"]->setVal(false);      // simulated Startup
            amap["StartupSimSeen"]->setVal(false);  // flag to make sure sim doesn't start up again.

            // BELOW: this runs reloads for asset_managers (does no processing - only
            // for linkVals).
            amap["BMSStartup"]->setVal(true);  // flag to make sure sim doesn't start up again.
            amap["PCSStartup"]->setVal(true);  // flag to make sure sim doesn't start up again.
            for (auto& ix : am->assetManMap)   // should get (const?) references to
                                               // pairs NOT copies.
            {
                // For MVP - We need at least PCS and BMS Startups. Will add more.
                asset_manager* amc = ix.second;
                if (amc->name == "bms")
                {
                    StartupBMS(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
                }
                if (amc->name == "pcs")
                {
                    StartupPCS(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
                }
            }
            // Below is the real reset (above is only for linkVals and init)
            amap["BMSStartup"]->setVal(false);  // flag to indicate Startup complete
            amap["PCSStartup"]->setVal(false);  // flag to indicate Startup complete
            amap["BMSFaulted"]->setVal(false);  // flag to indicate Startup complete
            amap["PCSFaulted"]->setVal(false);  // flag to indicate Startup complete
            amap["StartupCmd"]->setVal(false);
        }
        reload = 2;
        essAv->setVal(reload);
    }

    // double tNow = vm->get_time_dbl();

    // This function will be called by (ESS Controller - where this function is
    // located under) on the following conditions:
    //          *     1) estart from ANYWHERE!
    //          *     2) start command from Site_Controller
    //          *     3) Internal fault (ANY fault), shut down EVERYTHING! (a
    //          "fault start")
    //          *     4) Through the User Interface (press a button!) - Web UI
    //          *     ... more? (don't know yet)
    bool UiStartup = amap["UiStartup"]->getbVal();
    bool SMStartup = amap["SMStartup"]->getbVal();
    bool FaultStartup = amap["FaultStartup"]->getbVal();
    bool StartupCompleted = amap["StartupCompleted"]->getbVal();
    double StartupTime = amap["StartupTime"]->getdVal();
    bool StartupRequested = amap["StartupRequested"]->getbVal();
    bool PCSStartup = amap["PCSStartup"]->getbVal();
    bool BMSStartup = amap["BMSStartup"]->getbVal();
    char* cstate = amap["SystemState"]->getcVal();
    char* csstate = amap["SystemStateStep"]->getcVal();
    double sdtime = amap["StartupTime"]->getdVal();  // seems redundant

    bool StartupSim = amap["StartupSim"]->getbVal();
    bool StartupSimSeen = amap["StartupSimSeen"]->getbVal();
    int PCSStatusResp = amap["PCSStatusResp"]->getiVal();
    int BMSStatusResp = amap["BMSStatusResp"]->getiVal();

    assetVar* UiStartupAv = amap["UiStartup"];        //->getbVal();
    assetVar* SMStartupAv = amap["SMStartup"];        //->getbVal();
    assetVar* FaultStartupAv = amap["FaultStartup"];  //->getbVal(); // Is this necessary?
    if (StartupSim)
    {
        if (!StartupSimSeen)
        {
            if (1)
                FPS_ERROR_PRINT("%s >> Starting Startup Sim at time: %f\n", __func__, tNow);

            amap["StartupTime"]->setVal(tNow);
            amap["StartupSim"]->setVal(false);
            amap["StartupSimSeen"]->setVal(true);
            amap["SMStartup"]->setVal(true);
            amap["StartupCompleted"]->setVal(false);
            SMStartup = true;
            StartupTime = tNow;
        }

        if (tNow - StartupTime > 35.0)  // 35 for sim?
        {
            // amap["StartupSim"]->setVal(false);
            amap["StartupSimSeen"]->setVal(false);
            amap["StartupCompleted"]->setVal(false);  // false?

            // TODO: Check these for correctness: - they don't help
            amap["StartupBMS"]->setVal(true);
            amap["StartupPCS"]->setVal(true);
        }
        else if (tNow - StartupTime > 15.0)
        {
            if (1)
                FPS_ERROR_PRINT("%s >> startping StartupSim at time: %f (tNow-StartupTime) %f\n", __func__, tNow,
                                (tNow - StartupTime));
            // amap["StartupSim"]->setVal(false);
        }
        else if (tNow - StartupTime > 10.0)
        {
            // forced shutdown is being caused here after 10 seconds.
            if (!BMSStartup)
            {
                if (1)
                    FPS_ERROR_PRINT("%s >>Forcing  Startup BMS at time: %f\n", __func__, tNow);
                amap["BMSStartup"]->setVal(true);
            }
        }
        else if (tNow - StartupTime > 5.0)
        {
            if (!PCSStartup)
            {
                if (1)
                    FPS_ERROR_PRINT("%s >>Forcing  Startup PCS at time: %f\n", __func__, tNow);
                amap["PCSStartup"]->setVal(true);
            }
        }
        else if (tNow - StartupTime > 4.0)
        {
            if (PCSStatusResp != PCSstartResp)
            {
                if (1)
                    FPS_ERROR_PRINT("%s >> Forcing pcs start reply at time: %f\n", __func__, tNow);
                amap["PCSStatusResp"]->setVal(PCSstartResp);
            }
        }
        else if (tNow - StartupTime > 3.0)
        {
            if (BMSStatusResp != BMSstartResp)
            {
                if (1)
                    FPS_ERROR_PRINT("%s >> Forcing pcs start reply at time: %f\n", __func__, tNow);
                amap["BMSStatusResp"]->setVal(BMSstartResp);
            }
        }
    }
    // TODO allow a state change in any input command to change this
    if (StartupCompleted)  // Need this under bms and pcs Startups
    {
        if (0)
            FPS_ERROR_PRINT(
                ""
                ""
                ""
                ""
                ""
                ""
                "I AM COMPLETE!!!!"
                ""
                ""
                ""
                ""
                "\n");
        if (StartupRequested)
        {
            amap["StartupRequested"]->setVal(false);
        }
        amap["StartupCompleted"]->setVal(false);

        return;
    }
    // This is the start of the real code (this is our flow diagram):
    if (UiStartupAv->getbVal() && UiStartupAv->valueChangedReset())
    {
        amap["StartupCmd"]->setVal(true);
    }
    if (SMStartupAv->getbVal() && SMStartupAv->valueChangedReset())
    {
        amap["StartupCmd"]->setVal(true);
    }
    if (FaultStartupAv->getbVal() && FaultStartupAv->valueChangedReset())  // Is there going to be a faulted start?
    {
        amap["StartupCmd"]->setVal(true);
    }
    bool StartupCmd = amap["StartupCmd"]->getbVal();

    if (StartupCmd)  // TODO: Make these one-shots
    {
        if (1)
            FPS_ERROR_PRINT("UIStartup: %s, SMStartup: %s, FaultStartup: %s\n", UiStartup ? "true" : "false",
                            SMStartup ? "true" : "false", FaultStartup ? "true" : "false");
        if (!StartupRequested)  // we are having a shutdown requested here when you
                                // set StartupCompleted to false.
        {
            if (1)
                FPS_ERROR_PRINT("%s >> Func Starting Startup at time: %f elap %f \n", __func__, tNow,
                                (tNow - StartupTime));
            amap["StartupCmd"]->setVal(false);

            const char* cVal = "Startup";
            bool bval = false;
            // amap["SystemState"]->setVal(cVal);
            cVal = "Waiting PCS Startup";
            amap["SystemStateStep"]->setVal(cVal);

            amap["StartupRequested"]->setVal(true);
            // Sadly we have to be specific here tried to get away from the list
            amap["PCSStartup"]->setVal(bval);
            amap["BMSStartup"]->setVal(bval);
            amap["BMSFaulted"]->setVal(false);  // flag to indicate Startup complete ?
            amap["PCSFaulted"]->setVal(false);  // flag to indicate Startup complete ?

            amap["StartupCompleted"]->setVal(false);
            // Start of the Startup:
            tNow = am->vm->get_time_dbl();
            amap["StartupTime"]->setVal(tNow);  // This is what is likely causing it.
            if (1)
                FPS_ERROR_PRINT("%s >> Setting StartupTime to %f\n", __func__, tNow);

            // vm->sendAlarm(vmap, "/status/ess:StartupRequested",
            // "/components/ess:alarms", "Startup", "Startup Started", 1);
        }
    }

    StartupRequested = amap["StartupRequested"]->getbVal();
    if (!StartupRequested)
    {
        if (0)
            FPS_ERROR_PRINT("%s >> Func Quitting Startup at  time: %f\n", __func__, tNow);
        return;
    }

    // After this we start processing the Startup request:
    if (amap["SystemState"]->valueChangedReset() || amap["SystemStateStep"]->valueChangedReset())
    {
        cstate = amap["SystemState"]->getcVal();
        csstate = amap["SystemStateStep"]->getcVal();

        if (1)
            FPS_ERROR_PRINT("%s >> Startup  state [%s] step [%s]  time elapsed : %f\n", __func__, cstate, csstate,
                            tNow - sdtime);
    }

    PCSStartup = amap["PCSStartup"]->getbVal();
    BMSStartup = amap["BMSStartup"]->getbVal();

    // assets are in assetMap sub managers are in assetManMap
    for (auto& ix : am->assetManMap)  // should get (const?) references to pairs NOT copies.
    {
        asset_manager* amc = ix.second;
        if (0)
            printf("manager_name: [%s] PCSStartup [%s] BMSStartup [%s]\n", amc->name.c_str(),
                   PCSStartup ? "true" : "false", BMSStartup ? "true" : "false");

        if (amc->name == "bms")  // swapped to do bms startup before pcs startup. Keep this?
        {
            if (0)
                printf("manager_name: [%s] BMSStartup [%s]\n", amc->name.c_str(), BMSStartup ? "true" : "false");

            if (!BMSStartup)  // This logic was added to guarantee no cascade is done
                              // without PCS being shutdown first. Might need to be
                              // changed.
            {
                if (1)
                    FPS_ERROR_PRINT("%s >>>> cascading function to >>>>> Manager [%s] at time: %f\n ", __func__,
                                    amc->name.c_str(), tNow);
                StartupBMS(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
            }
        }
        if (amc->name == "pcs")
        {
            if (0)
                printf("manager_name: [%s] PCSStartup [%s]\n", amc->name.c_str(), PCSStartup ? "true" : "false");

            if (!PCSStartup && BMSStartup)
            {
                if (0)
                    FPS_ERROR_PRINT("%s >>>> cascading function to >>>>> Manager [%s] at time: %f\n ", __func__,
                                    amc->name.c_str(), tNow);
                StartupPCS(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
            }
        }
    }

    /**
     * @brief Analysis of the below code.
     *
     * Everything below here probably needs to be removed, we aren't "forcing" a
     * startup or timing out.
     *
     */

    // This is the manager determining if everything is shutting down on time:
    // double StartupTime = amap["StartupTime"]->getdVal();
    double maxPCSStartupTime = amap["maxPCSStartupTime"]->getdVal();
    double maxBMSStartupTime = amap["maxBMSStartupTime"]->getdVal();
    StartupTime = amap["StartupTime"]->getdVal();
    if (!PCSStartup && (tNow - StartupTime) > maxPCSStartupTime)
    {
        amap["PCSStartup"]->setVal(true);
        amap["PCSStartupForced"]->setVal(true);
        vm->sendAlarm(vmap, "/status/ess:StartupRequested", "/components/ess:alarms", "PCSStartup",
                      "PCS Startup Forced", 1);

        // TODO: put an alarm here
        if (1)
            FPS_ERROR_PRINT(
                " %s >>>> Forced PCS Startup State at time %f SDtime %f "
                "elap %f max %f \n",
                __func__, tNow, StartupTime, (tNow - StartupTime), maxPCSStartupTime);
    }
    // May have to delay timing to allow for PCS Startup
    if (PCSStartup && (!BMSStartup && (tNow - StartupTime) > maxBMSStartupTime))
    {
        amap["BMSStartup"]->setVal(true);
        amap["BMSStartupForced"]->setVal(true);
        // TODO: put an alarm here
        if (1)
            FPS_ERROR_PRINT(" %s >> Forced BMS Startup State at time %f \n", __func__, tNow);
        vm->sendAlarm(vmap, "/status/ess:StartupRequested", "/components/ess:alarms", "BMSStartup",
                      "BMS Startup Forced", 1);
    }

    if (BMSStartup && PCSStartup)  // then it goes here and does this.
    {
        if (!StartupCompleted)
        {
            const char* cVal = "Ready";
            amap["SystemState"]->setVal(cVal);
            cVal = "Waiting for Start";
            amap["SystemStateStep"]->setVal(cVal);

            amap["StartupRequested"]->setVal(false);
            amap["StartupCompleted"]->setVal(true);
            if (1)
                FPS_ERROR_PRINT("%s >> Startup Completed, elapsed time: %f \n", __func__, (tNow - sdtime));
            vm->sendAlarm(vmap, "/status/ess:StartupRequested", "/components/ess:alarms", "Startup",
                          "Startup Completed", 1);
        }
    }

    return;
}

// StartupPCS
// * This function will consist of the following sequence (run by ESS
// controller)
//  *     1) Send to PCS_Manager the StartupPCS's command (SIDENOTE: have
//  PCS_Manager set [ActivePowerSetPoint && ReactivePowerSetPoint] to 0 )
//  *     2) Wait for PCS_Manager to respond (get ok that all PCS's are Startup)
//  *     3) Send to BMS_Manager the StartupBMS's command
//  *     4) Wait for BMS_Manager to respond (get ok that all BMS's are Startup)
//  *     ... add other system as needed
//  *     General form:
//  *         - Send StartupAsset's to their manager
//  *         - Wait for response before proceeding
// TODO: Similar to BMS, we might only need a single register, just set global
// active/reactivepower setpoints to zero. Statuses, etc.:
/*


        // check this linkVals(*vm, vmap, amap, aname, "/status", ival,
   "PCSStatusResp", "PCSKeystart");   // returned status

        // other variables:
        // todo: link Estart to aname = "ess" not itself.
        // Check this linkVals(*vm, vmap, amap, "ess", "/status", bval,
   "PCSFaulted", "PCSFaultSeen", "StartupRequest", "StartupRequested");
        ///linkVals(*vm, vmap, amap, aname, "/status", bval, "PCSKeystartSent");
        //linkVals(*vm, vmap, amap, aname, "/config", ival, "PCSKeystartCmd");
        //linkVals(*vm, vmap, amap, aname, "/config", ival, "PCSFaultStatus");

        // Estart:
        linkVals(*vm, vmap, amap, "ess", "/status", bval, "Estart");
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

        /status/ess/StartupTime
                    time the system took to shutdown.
        /status/pcs/PCSStartupTime
                    time the system took to shutdown.
        /status/pcs/PCSStartupTimeStart
                    time the system started the shutdown.

        // Config
        /config/ess/maxPCSStartupTime
                    time limits to allow for PCS system to complete shutdown.


        linkVals(*vm, vmap, amap, aname, "/status", ival, "
        // ?? /status/ess/BmsStatus   status of the BmsSystem   from modbus
   input
        // ?? /status/ess/PCsStatus   status of the PcsSystem
        // ?? /status/ess/PCsStatus   status of the EmuSystem    from ??
        // ?? /status/ess/PCsStatus   status of the EmmuSystem    from ??

        // other variables:
        bool

        /status/ess/Estart             estart input flag , several possible
   sources. TODO Should be attached directly to Startup.

        /status/ess/PCSStartup         Check this
        /status/ess/PCSStatus           and this

        /status/ess/PCSFaulted        Indication that the PCS system was faulted
        /status/pcs/PCSFaultSeen      Indication that the PCS fault has been
   seen

        NOTE we may set the next three up as integers. to keep track of multiple
   requests. /status/ess/StartupRequest      Main trigger for a shutdown start ,
   set by any system that wants to initiate a shutdown. We'll attach an alarm to
   it later. /status/ess/StartupRequested    Indication that we have responded
   to the latest shutdown request. /status/ess/StartupCompleted    Indication
   that we have completed the latest Startup Request bool
        /status/ess/PCSStartupForced     Indication that we forced the PCS to
   shutdown It did not  resond to the current request.
        /status/ess/BMSStartupForced     Indication that we forces the BMS
   shutdown.

        /status/ess/PCSStartup           Indication that the PCSStartup has
   completed /status/ess/BMSStartup           Indication that the BMSStartup has
   completed

        /status/ess/StartupSim           Request for simulation code path tester
   reset as soon as SimSeen is set. /status/ess/StartupSimSeen       Indication
   that the simulation request has been seen , reset when the simulation has
   completed

        /status/ess/StartupCompleted     Indication that Startup has been
   completed. To Be reset when another transition brigs the system out of
   shutdown.

        double
        /status/pcs/PCSKeyCmdTime            Place to send  PCS command.  from
   ModBus Map Jimmy

        int
        /status/ess/PCSstartSent           command sent to  start to the PCS
        /status/pcs/PCSStatusResp         Code indicating the current PCS
   status.  from ModBus Map Jimmy /status/bms/BMSStatusResp         Code
   indicating the current BMS status.  from ModBudMap Jimmy
        /status/pcs/PCSKeyCmd            Place to send  PCS command.  from
   ModBus Map Jimmy /status/pcs/PCSKeyCmdTries       NUmber of attempts to send
   PCS Command



*/
void StartupPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    // Timer StartupPCSTimer("StartupPCSTimer", true);

    int reload = 0;
    bool bval = false;
    int ival = 0;
    double dval = 0.0;
    char* tValInit = (char*)"Init";
    char* tValRun = (char*)"Run";
    char* tValReady = (char*)"Ready";
    VarMapUtils* vm = am->vm;
    double tNow = vm->get_time_dbl();

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
        // TODO: Clean this up!!!
        // reload
        linkVals(*vm, vmap, amap, aname, "/reload", reload, __func__);
        pcsAv = amap[__func__];  // Need to figure out the
                                 // resolution here!

        // Statuses, etc.:
        linkVals(*vm, vmap, amap, "ess", "/status", tValInit, "SystemState", "SystemStateStep");
        linkVals(*vm, vmap, amap, "ess", "/config", dval, "ActivePowerSetpoint", "ReactivePowerSetpoint");
        linkVals(*vm, vmap, amap, "ess", "/status", dval, "StartupTime");
        linkVals(*vm, vmap, amap, "ess", "/config", dval, "maxPCSStartupTime");
        linkVals(*vm, vmap, amap, "ess", "/config", dval, "PowerSetpointDeadband");
        linkVals(*vm, vmap, amap, "ess", "/config", dval, "maxPCSStartupTime");

        linkVals(*vm, vmap, amap, "ess", "/config", dval, "PCSKeyActivePowerSetpoint", "PCSKeyReactivePowerSetpoint");
        linkVals(*vm, vmap, amap, "ess", "/status", bval, "PCSStartup", "PCSStatus", "PCSStartSent");
        linkVals(*vm, vmap, amap, "ess", "/status", bval, "PCSFaulted", "PCSFaultSeen", "StartupRequest",
                 "StartupRequested");

        linkVals(*vm, vmap, amap, aname, "/status", tValInit, "PCSSystemState", "PCSSystemStateStep");
        linkVals(*vm, vmap, amap, aname, "/status", dval, "PCSStartupTime", "PCSStartupStart", "PCSKeyCmdTime");
        linkVals(*vm, vmap, amap, aname, "/status", dval, "PCSActivePower",
                 "PCSReactivePower");  // ModBus responses
        linkVals(*vm, vmap, amap, aname, "/status", ival, "PCSStatusResp", "PCSKeyStart",
                 "PCSKeyCmdTries");  // returned status    PCSKEYCmd perhaps
        // Jimmy ... PCSStatusResp is mapped to the pcs_status
        // other variables:
        linkVals(*vm, vmap, amap, aname, "/status", ival, "PCSKeyStartSent");
        linkVals(*vm, vmap, amap, aname, "/status", ival, "PCSKeyCmd");
        linkVals(*vm, vmap, amap, aname, "/config", ival, "PCSKeyStartCmd");
        linkVals(*vm, vmap, amap, aname, "/config", ival, "PCSFaultStatus");

        // other variables
        // linkVals(*vm, vmap, amap, aname, "/status", ival, "PCSKeyStatus",
        // "PCSKeyCmd");

        if (reload == 0)  // complete restart
        {
            amap["PCSSystemState"]->setVal(tValInit);
            amap["PCSFaulted"]->setVal(false);
            amap["PCSStartup"]->setVal(false);
            amap["PCSStartSent"]->setVal(false);
            amap["PowerSetpointDeadband"]->setVal(0.01);
            amap["PCSActivePower"]->setVal(0.0);
            amap["PCSReactivePower"]->setVal(0.0);
            amap["maxPCSStartupTime"]->setVal(15.0);  // config?

            amap["PCSStatusResp"]->setVal(0);
            amap["PCSKeyCmdTries"]->setVal(0);
            amap["PCSKeyCmdTime"]->setVal(tNow);
            amap["PCSKeyStartSent"]->setVal(0);
            if (1)
                FPS_ERROR_PRINT("%s >>>>>>>>>>>>>>>>PCS Startup system state [%s]  at time: %f \n", __func__,
                                amap["PCSSystemState"]->getcVal(), tNow);
        }
        reload = 2;
        pcsAv->setVal(reload);
    }

    char* PCSSystemState = amap["PCSSystemState"]->getcVal();
    char* PCSSystemStateStep = amap["PCSSystemStateStep"]->getcVal();
    if (amap["PCSSystemState"]->valueChangedReset() || amap["PCSSystemStateStep"]->valueChangedReset())
    {
        if (1)
            FPS_ERROR_PRINT("%s >>  PCS State [%s] Step [%s] time: %f \n", __func__, PCSSystemState, PCSSystemStateStep,
                            tNow);
    }
    bool PCSStartup = amap["PCSStartup"]->getbVal();
    char* SystemStateStep = amap["PCSSystemStateStep"]->getcVal();

    if (PCSStartup)
    {
        if (strcmp(SystemStateStep, "Waiting PCS Startup") == 0)
        {
            SystemStateStep = (char*)"PCS Startup Complete";
            amap["SystemStateStep"]->setVal(SystemStateStep);
            if (1)
                FPS_ERROR_PRINT("%s >> All PCS's succesfully Startup at elapsed time: %f \n", __func__, tNow);
        }
        return;
    }
    // bool PCSFaulted = amap["PCSFaulted"]->getbVal();

    double PCSActivePower = amap["PCSActivePower"]->getdVal();
    double PCSReactivePower = amap["PCSReactivePower"]->getdVal();
    // double  PowerSetpointDeadband = amap["PowerSetpointDeadband"]->getdVal();

    PCSSystemState = amap["PCSSystemState"]->getcVal();
    // char* PCSSystemStateStep = amap["PCSSystemStateStep"]->getcVal();
    // IF Run or Ready start Shtdown
    // Do not run if in Init The PCS monitor will bring things out of init or we
    // have to do it by hand
    if (0)
        FPS_ERROR_PRINT("%s >>>> PCS Startup system state [%s]  at time: %f \n", __func__, PCSSystemState, tNow);

    // OK to Run Startup ??
    if (/*(strcmp(PCSSystemState, tValInit) == 0) ||*/ (strcmp(PCSSystemState, tValRun) == 0) ||
        (strcmp(PCSSystemState, tValReady) == 0))
    {
        if (1)
            FPS_ERROR_PRINT("%s >>>>>>>>>>>>>>>>PCS Startup system state [%s]  at time: %f \n", __func__,
                            PCSSystemState, tNow);
        if (1)
            FPS_ERROR_PRINT("%s >>>> PCS ActivePower %f ReactivePower %f  time: %f \n", __func__, PCSActivePower,
                            PCSReactivePower, tNow);

        char* cval = (char*)"Startup";
        char* cvals = (char*)"Setting Zero Power";
        amap["PCSSystemState"]->setVal(cval);
        amap["PCSSystemStateStep"]->setVal(cvals);

        // send activePower reactivepower to 0
        amap["PCSKeyActivePowerSetpoint"]->setVal(0.0);
        amap["PCSKeyReactivePowerSetpoint"]->setVal(0.0);
        amap["PCSStartupStart"]->setVal(tNow);
        amap["PCSStartupTime"]->setVal(tNow);
        amap["PCSKeyCmdTries"]->setVal(0);

        amap["PCSKeyStart"]->setVal(0);
        amap["PCSKeyStartSent"]->setVal(0);
        // amap["PCSKeystart"]->setVal(PCSstartCmd);
    }

    // double  PCSStartupTime = amap["PCSStartupTime"]->getdVal();
    double maxPCSStartupTime = amap["maxPCSStartupTime"]->getdVal();
    double PCSStartupStart = amap["PCSStartupStart"]->getdVal();  // weird variable name - need to check this.
    int PCSStatusResp = amap["PCSStatusResp"]->getiVal();
    int PCSKeyStart = amap["PCSKeyStart"]->getiVal();
    // bool    PCSStartSent = amap["PCSStartSent"]->getbVal();
    int PCSKeyStartSent = amap["PCSKeyStartSent"]->getiVal();
    int PCSKeyCmdTries = amap["PCSKeyCmdTries"]->getiVal();

    // #define PCSstartCmd 1   //Jimmy
    // #define PCSstartResp 5  //Jimmy
    // #define PCSCmdTime 0.5  //config
    // TODO check timeout
    // if we have not sent the Keystart then wait for the power setpoints to
    // settle
    if (PCSStatusResp == Power_Electronics::OFF || PCSStatusResp == Power_Electronics::READY)
    {
        if (PCSKeyStart == 0 && PCSStartup)
        {
            // We need a warning if it was due to timeout.
            if (1)
                FPS_ERROR_PRINT("%s >> PCS power OK sending Keystart at time: %f elspsed: %f \n", __func__, tNow,
                                (tNow - PCSStartupStart));
            char* cvals = (char*)"Sending Start";
            amap["PCSSystemStateStep"]->setVal(cvals);
            amap["PCSKeyStart"]->setVal(PCSStartCmd);
            amap["PCSKeyCmd"]->setVal(PCSStartCmd);
            // TODO send to Fims

            amap["PCSKeyStartSent"]->setVal(PCSStartCmd);
            amap["PCSStartupTime"]->setVal(tNow);
            amap["PCSStartupStart"]->setVal(tNow);
            PCSStartupStart = tNow;
            amap["PCSKeyCmdTime"]->setVal(tNow);
        }
    }
    else if (PCSKeyStart && PCSStatusResp != Power_Electronics::ON)
    {
        if ((tNow - PCSStartupStart) > maxPCSStartupTime)
        {
            // We need a warning if it was due to timeout.
            if (1)
                FPS_ERROR_PRINT("%s >> PCS Timeout sending Keystart at elapsed time: %f \n", __func__,
                                (tNow - PCSStartupStart));
            char* cvals = (char*)"Startup Timeout";
            amap["PCSSystemStateStep"]->setVal(cvals);
            amap["PCSKeyStart"]->setVal(PCSStartCmd);
            amap["PCSKeyCmd"]->setVal(PCSStartCmd);
            // TODO send to Fims

            amap["PCSKeyStartSent"]->setVal(PCSStartCmd);
            amap["PCSStartupTime"]->setVal(tNow);
            amap["PCSKeyCmdTime"]->setVal(tNow);

            vm->sendAlarm(vmap, "/status/ess:StartupRequested", "/components/pcs:alarms", "PCSStartup",
                          "PCS Forced Startup", 1);
        }
    }

    PCSKeyStartSent = amap["PCSKeyStartSent"]->getiVal();
    PCSKeyStart = amap["PCSKeyStart"]->getiVal();
    // TODO add PCSKeyCmdReset
    // PCSKeystart != 0 if we have sent a command
    if (!PCSStartup && (PCSKeyStart != 0))
    {
        if (PCSStatusResp == PCSstartResp)  // TODO: Will be configed - probably come
                                            // in as a text stream  JIMMY
        {
            if (1)
                FPS_ERROR_PRINT("%s >> PCS Status start Reponse found at elapsed time: %f \n", __func__,
                                (tNow - PCSStartupStart));
            // char* cval = (char*)"PCS Startup";
            char* cvals = (char*)"Startup Complete";
            // amap["SystemState"]->setVal(cval);
            amap["PCSSystemStateStep"]->setVal(cvals);
            cvals = (char*)"PCS Startup Complete";
            amap["SystemStateStep"]->setVal(cvals);
            amap["PCSStartup"]->setVal(true);
            amap["PCSKeyCmdTries"]->setVal(0);
            amap["PCSKeyStartSent"]->setVal(0);  // allow a resend
            amap["PCSKeyCmd"]->setVal(0);
            // TODO send to Fims

            return;
        }
    }

    // KeystartSent used to turn off command Keystart used to indicate command
    // sent Return Cmd to zero after PCSCmdTime ?? Config
    double PCSKeyCmdTime = amap["PCSKeyCmdTime"]->getdVal();
    PCSKeyCmdTries = amap["PCSKeyCmdTries"]->getiVal();
    double maxPCSKeyCmdOnTime = 0.5;  // config
    double maxPCSKeyCmdTime = 2.5;    // config
    int maxPCSKeyCmdTries = 5;        // config
    PCSKeyStartSent = amap["PCSKeyStartSent"]->getiVal();
    PCSKeyStart = amap["PCSKeyStart"]->getiVal();
    if (PCSKeyStartSent != 0)
    {
        if ((tNow - PCSKeyCmdTime) > maxPCSKeyCmdOnTime)
        {
            if (1)
                FPS_ERROR_PRINT("%s >> PCS Reset Cmd  at elapsed time: %f \n", __func__, (tNow - PCSKeyCmdTime));

            amap["PCSKeyStartSent"]->setVal(0);  // force a resend
            amap["PCSKeyCmd"]->setVal(0);
            // TODO send to Fims
            // TODO send to Fims

            //}
            // vm->sendAlarm(vmap ,amap["PCSKeystart"],"/components/pcs:alarms",
            // "PCSKEYstart","PCS Ignored Command",1);
        }
    }
    // Allow a resend
    if (0)
        FPS_ERROR_PRINT(
            "%s >> PCS start waiting for PCSStatusResp (%d) cmdTime %f "
            "Max %f at  elap time: %f \n",
            __func__, PCSStatusResp, PCSKeyCmdTime, maxPCSKeyCmdTime, (tNow - PCSKeyCmdTime));
    if (PCSKeyStart != 0)
    {
        if ((tNow - PCSKeyCmdTime) > maxPCSKeyCmdTime)
        {
            if (PCSKeyCmdTries < maxPCSKeyCmdTries)
            {
                amap["PCSKeyStart"]->setVal(0);      // force a resend
                amap["PCSKeyStartSent"]->setVal(0);  // force a resend

                amap["PCSKeyCmdTries"]->addVal(1);
                amap["PCSKeyCmd"]->setVal(0);

                // TODO send to Fims done at another place
                // TODO send to Fims
                // amap["PCSKeyCmdTime"]->setVal(tNow);

                vm->sendAlarm(vmap, amap["PCSKeyCmd"], "/components/pcs:alarms", "PCSKEYstart", "PCS Ignored Command",
                              1);
                if (1)
                    FPS_ERROR_PRINT(
                        "%s >> PCS start No PCSStatusResp (%d != %d)  resend "
                        "after %d tries  attime: %f \n",
                        __func__, PCSStatusResp, PCSstartResp, PCSKeyCmdTries, tNow);
                // amap["PCSFaulted"]->setVal(true);
            }
            else if (PCSKeyCmdTries == maxPCSKeyCmdTries)
            {
                amap["PCSKeyCmdTries"]->addVal(1);
                if (1)
                    FPS_ERROR_PRINT(
                        "%s >> PCS start No Cmd Response startped after %d "
                        "tries  attime: %f \n",
                        __func__, PCSKeyCmdTries, tNow);
            }
        }
        // TODO Fault the PCS
    }

    PCSStartupStart = amap["PCSStartupStart"]->getdVal();
    if (((tNow - PCSStartupStart) > maxPCSStartupTime) && !PCSStartup)
    {
        // there is where forced shutdown occurs
        char* cval = (char*)"PCS Startup";
        char* cvals = (char*)"Startup Timeout";
        amap["SystemState"]->setVal(cval);
        amap["SystemStateStep"]->setVal(cvals);
        amap["PCSFaulted"]->setVal(true);
        amap["PCSStartup"]->setVal(true);
        amap["PCSKeyCmdTries"]->setVal(0);
        amap["PCSKeyStart"]->setVal(0);      // allow a resend
        amap["PCSKeyStartSent"]->setVal(0);  // reset possible next pass

        // ToDo Send Alarm
        if (1)
            FPS_ERROR_PRINT("%s >>>>>> Time Forced PCS Startup at time %f start %f elap %f  \n ", __func__, tNow,
                            PCSStartupStart, (tNow - PCSStartupStart));
    }
}

// StartupBMS
// * This function will consist of the following sequence (run by ESS
// controller)
//  *    1) Send to BMS_Asset the StartupBMS command
//  *    2) Wait for BMS_Asset to respond (get ok that all BMS's are Startup)
//  *     ... add other system as needed
// Statuses, etc.:
/*
        / Should an asset be accessing another asset's states? - could this not
   lead to error?
        // TODO: In future make a read-only linkVal (as to prevent errors from
   occuring due to shared links)
        // reload
        linkVals(*vm, vmap, amap, aname, "/reload", reload,
   __func__); bmsAv = amap[__func__];
        // Statuses, etc.:
    ////OK  linkVals(*vm, vmap, amap, "ess", "/status", tValInit, "SystemState",
   "SystemStateStep");
    ////OK linkVals(*vm, vmap, amap, "ess", "/config", dval,
   "ActivePowerSetpoint", "ReactivePowerSetpoint", "StartupTime");
    ////OK linkVals(*vm, vmap, amap, "ess", "/config", dval,
   "maxBMSStartupTime");
    ////Remove linkVals(*vm, vmap, amap, "ess", "/config", dval,
   "PowerSetpointDeadband");
    ////OK linkVals(*vm, vmap, amap, aname, "/status", tValInit,
   "BMSSystemState", "BMSSystemStateStep");
    ////OK linkVals(*vm, vmap, amap, aname, "/status", dval, "BMSStartupTime",
   "BMSStartupStart");
    ////Fixed linkVals(*vm, vmap, amap, "ess", "/config", dval,
   "maxBMSStartupTime");
    ////OK linkVals(*vm, vmap, amap, "ess", "/status", bval, "BMSStartup",
   "BMSStatus");
    ////Remove linkVals(*vm, vmap, amap, "bms", "/status", ival, "BMSnumFaults",
   "BMSnumPresent"); // Need keyCmd here?
        // linkVals(*vm, vmap, amap, aname, "/status", ival, "BMSKeyCmd",
   "BMSKeyStatus");


    ////Remove for now linkVals(*vm, vmap, amap, aname, "/status", ival,
   "BmsStatus");

        // other variables:
        // TODO: Estart to aname = "ess" not itself.
        ////linkVals(*vm, vmap, amap, "ess", "/status", bval, "BMSFaulted",
   "BMSFaultSeen", "StartupRequest", "StartupRequested");
    ////Fixed linkVals(*vm, vmap, amap, aname, "/status", ival,
   "BMSKeyCmdSent");
        ////linkVals(*vm, vmap, amap, aname, "/status", ival, "BMSKeyCmd",
   "BMSKeyStatus"); // is Status necessary yes for feedback
        //linkVals(*vm, vmap, amap, aname, "/status", bval, "BMSStartup",
   "BMSStatus");
        // linkVals(*vm, vmap, amap, aname, "/config", ival, "PCSKeystartCmd");
        ////linkVals(*vm, vmap, amap, aname, "/config", ival, "BMSFaultStatus");

        //Estart:
        ////linkVals(*vm, vmap, amap, "ess", "/status", bval, "Estart"); //
   Might not need this?


        // check this linkVals(*vm, vmap, amap, aname, "/status", ival,
   "PCSStatusResp", "PCSKeystart");   // returned status

        // other variables:
        // todo: link Estart to aname = "ess" not itself.
        // Check this linkVals(*vm, vmap, amap, "ess", "/status", bval,
   "PCSFaulted", "PCSFaultSeen", "StartupRequest", "StartupRequested");
        ///linkVals(*vm, vmap, amap, aname, "/status", bval, "PCSKeystartSent");
        //linkVals(*vm, vmap, amap, aname, "/config", ival, "PCSKeystartCmd");
        //linkVals(*vm, vmap, amap, aname, "/config", ival, "PCSFaultStatus");

        // Estart:
        ////linkVals(*vm, vmap, amap, "ess", "/status", bval, "Estart");



        char *
        /status/ess/SystemState
               used to indicate system state : Init, Fault, Ready , StandBy ,
   Run /status/ess/SystemStateStep used to indicate system state step
        /status/bms/BMSSSystemState
               used to indicate system state : Init, Fault, Ready , StandBy ,
   Run /status/bms/BMSSystemStateStep used to indicate system state step

        double


        /config/ess/ActivePowerSetpoint
                     the current activePower Setpoint  From SM or UI
        /config/ess/ReactivePowerSetpoint
                     the current reactivePower Setpoint  From SM or UI


        // Feedback from PCS Jimmy ???
        /status/pcs/PCSActivePower
                     the current activePower   From PCS
        /status/pcs/PCSReactivePower
                     the current reactivePower   From PCS

        /status/ess/StartupTime
                    time the system took to shutdown.
        /status/bms/BMSStartupTime
                    time the system took to shutdown.
        /status/bms/BMSStartupTimeStart
                    time the system started the shutdown.

        // Config
        /config/ess/maxBMSStartupTime
                    time limits to allow for BMS system to complete shutdown.


        linkVals(*vm, vmap, amap, aname, "/status", ival, "
        //?? /status/ess/BmsStatus   status of the BmsSystem   from modbus input
        //?? /status/ess/PCsStatus   status of the PcsSystem
        //?? /status/ess/PCsStatus   status of the EmuSystem    from ??
        //?? /status/ess/PCsStatus   status of the EmmuSystem    from ??

        // other variables:
        bool

        /status/ess/Estart             estart input flag , several possible
   sources. TODO Should be attached directly to Startup.

        /status/ess/BMSStartup         Check this
        /status/ess/BMSStatus           and this
        /status/bms/BMSstartSent         we have sent the start to the BMS

        /status/ess/PCSFaulted        Indication that the PCS system was faulted
        /status/pcs/PCSFaultSeen      Indication that the PCS fault has been
   seen

        NOTE we may set the next three up as integers. to keep track of multiple
   requests. /status/ess/StartupRequest      Main trigger for a shutdown start ,
   set by any system that wants to initiate a shutdown. We'll attach an alarm to
   it later. /status/ess/StartupRequested    Indication that we have responded
   to the latest shutdown request. /status/ess/StartupCompleted    Indication
   that we have completed the latest Startup Request bool
        /status/ess/PCSStartupForced     Indication that we forced the PCS to
   shutdown It did not  resond to the current request.
        /status/ess/BMSStartupForced     Indication that we forces the BMS
   shutdown.

        /status/ess/PCSStartup           Indication that the PCSStartup has
   completed /status/ess/BMSStartup           Indication that the BMSStartup has
   completed

        /status/ess/StartupSim           Request for simulation code path tester
   reset as soon as SimSeen is set. /status/ess/StartupSimSeen       Indication
   that the simulation request has been seen , reset when the simulation has
   completed

        /status/ess/StartupCompleted     Indication that Startup has been
   completed. To Be reset when another transition brigs the system out of
   shutdown.


        int
        /status/bms/BMSStatusResp            Code indicating the current BMS
   status.  from ModBudMap Jimmy /status/pcs/PCSKeyCmd            Place to send
   PCS command.  from ModBus Map Jimmy /status/bms/BMSKeyCmdSent        Place
   record of  BMS command sent. /status/bms/BMSKeyCmdTries       Place record of
   BMS command send tries. /status/bms/BMSKeyCmd            Place to Send  BMS
   command.  from Modbus Map  Jimmy



*/
// NOTE: Needs aname to be a particular bms_number - Might be enum instead.
// TODO: Will just send out a single command to an MBMU register.
void StartupBMS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    // Timer StartupBMSTimer("StartupBMSTimer", true);

    // TODO: keyCMD is a shared mbmu register that will shut down all of the
    // BMS's.

    int reload = 0;
    bool bval = false;
    int ival = 0;
    double dval = 0.0;
    char* tValInit = (char*)"Init";
    char* tValRun = (char*)"Run";
    char* tValReady = (char*)"Ready";
    VarMapUtils* vm = am->vm;
    double tNow = vm->get_time_dbl();

    // int PowerOffStatus = 3;  // todo config
    // int PowerOffCmd = 3; // TODO: For Jimmy. Must be configed in the future.

    // char* tVal = (char*)"Test TimeStamp";
    // in future use tval for sending messages out - for UI and alert purposes.

    assetVar* bmsAv = amap[__func__];  // Need to figure out
                                       // the resolution here!
    if (!bmsAv || (reload = bmsAv->getiVal()) == 0)
    {
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }

    if (reload < 2)
    {
        // TODO: Clean this up!!!

        // Should an asset be accessing another asset's states? - could this not
        // lead to error?
        // TODO: In future make a read-only linkVal (as to prevent errors from
        // occuring due to shared links) reload
        linkVals(*vm, vmap, amap, aname, "/reload", reload, __func__);
        bmsAv = amap[__func__];
        // Statuses, etc.:
        linkVals(*vm, vmap, amap, "ess", "/status", tValInit, "SystemState", "SystemStateStep");
        linkVals(*vm, vmap, amap, "ess", "/status", dval, "StartupTime");
        linkVals(*vm, vmap, amap, "ess", "/config", dval, "maxBMSStartupTime");
        linkVals(*vm, vmap, amap, aname, "/status", tValInit, "BMSSystemState", "BMSSystemStateStep");
        linkVals(*vm, vmap, amap, aname, "/status", dval, "BMSStartupTime", "BMSStartupStart");
        linkVals(*vm, vmap, amap, aname, "/status", bval, "BMSStartup", "BMSStatus");
        linkVals(*vm, vmap, amap, aname, "/status", ival, "BMSnumFaults",
                 "BMSnumPresent");  // Need keyCmd here?

        // other variables:
        linkVals(*vm, vmap, amap, "ess", "/status", bval, "BMSFaulted", "BMSFaultSeen", "StartupRequest",
                 "StartupRequested");
        linkVals(*vm, vmap, amap, "ess", "/status", ival,
                 "BMSKeyStatus");  // maybe /asset/bms
        linkVals(*vm, vmap, amap, aname, "/status", ival, "BMSKeyCmdSent",
                 "BMSKeyCmdTries");  // is Status necessary?
        linkVals(*vm, vmap, amap, aname, "/controls", ival, "BMSKeyCmd");
        linkVals(*vm, vmap, amap, aname, "/config", dval,
                 "BMSKeyCmdTime");  // is Status necessary?
        linkVals(*vm, vmap, amap, aname, "/config", ival, "BMSFaultStatus");

        if (reload == 0)  // complete restart
        {
            amap["BMSSystemState"]->setVal(tValInit);
            amap["BMSFaulted"]->setVal(false);
            amap["BMSStartup"]->setVal(false);
            amap["maxBMSStartupTime"]->setVal(15.0);  // TODO: config
            // amap["BMSKeyCmd"]->setVal(0);
            amap["BMSKeyCmdSent"]->setVal(0);
            amap["BMSKeyCmdTries"]->setVal(0);
            amap["BMSKeyCmdTime"]->setVal(tNow);
        }
        reload = 2;
        bmsAv->setVal(reload);
    }

    bool BMSStartup = amap["BMSStartup"]->getbVal();
    if (BMSStartup)
    {
        if (0)
            FPS_ERROR_PRINT("%s >> All BMS's succesfully Started up at time: %f \n", __func__, tNow);
        return;
    }

    if (1)
        FPS_ERROR_PRINT("%s >> Startup BMS called for (BMS_Manager: %s) at time: %f \n", __func__, aname, tNow);

    // double  BMSStartupTime = amap["BMSStartupTime"]->getdVal();
    // double  maxBMSStartupTime = amap["maxBMSStartupTime"]->getdVal(); //
    // necessary for a timeout?
    bool BMSKeyCmdSent = amap["BMSKeyCmdSent"]->getiVal();
    bool StartupRequested = amap["StartupRequested"]->getbVal();
    int BMSKeyCmd = amap["BMSKeyCmd"]->getiVal();
    // int PowerOnCmd = 2;  // todo config
    double BMSKeyCmdTime = amap["BMSKeyCmdTime"]->getdVal();
    int BMSKeyCmdTries = amap["BMSKeyCmdTries"]->getiVal();
    int BMSStatus = amap["BMSStatus"]->getiVal();
    char* BMSSystemState = amap["BMSSystemState"]->getcVal();
    // char* PCSSystemStateStep = amap["PCSSystemStateStep"]->getcVal();
    // IF Init or Run start Shtdown
    char* BMSStatusString;
    if (BMSStatus == CATL::INIT)
        BMSStatusString = (char*)"Init";
    else if (BMSStatus == CATL::NORMAL)
        BMSStatusString = (char*)"Normal";
    else if (BMSStatus == CATL::FULLCHARGE)
        BMSStatusString = (char*)"Full Charge";
    else if (BMSStatus == CATL::FULLDISCHARGE)
        BMSStatusString = (char*)"Full Discharge";
    else if (BMSStatus == CATL::WARNING)
        BMSStatusString = (char*)"Warning";

    if (1)
        FPS_ERROR_PRINT(
            "%s >> BMS Status [%d] string [%s] Startup Requested [%d] "
            "BMS Key Cmd [%d]\n",
            __func__, BMSStatus, BMSStatusString, StartupRequested, BMSKeyCmd);

    if ((strcmp(BMSSystemState, tValInit) == 0) || (strcmp(BMSSystemState, tValRun) == 0) ||
        (strcmp(BMSSystemState, tValReady) == 0))
    {
        char* cval = (char*)"BMS Startup";
        // char* cvals = (char*)"Setting Zero Power";
        amap["BMSSystemState"]->setVal(cval);
        // amap["BMSSystemStateStep"]->setVal(cvals);
        amap["BMSStartupTime"]->setVal(tNow);
    }

    if (!BMSKeyCmdSent && StartupRequested)  // annoying bool here - just to get past init call
    {
        if (1)
            FPS_ERROR_PRINT("Sending Key Cmd Power ON at time %f\n", tNow);
        amap["BMSKeyCmd"]->setVal(CATL::POWERON);
        p_fims->Send("set", "/components/catl_ems_bms_rw/ems_cmd", nullptr, "2");
        // TODO send Fims and log
        amap["BMSKeyCmdSent"]->setVal(true);  // JIMMY
        amap["BMSKeyCmdTime"]->setVal(tNow);
    }

    double BMSKeyCmdWaitTime = 3.0;  // config
    double BMSKeyCmdOnTime = 3.0;    // config
    int maxBMSKeyCmdTries = 5;       // Config

    if (BMSKeyCmdSent && !BMSStartup && (BMSStatus == CATL::INIT) &&
        ((tNow - BMSKeyCmdTime) > BMSKeyCmdWaitTime))  // annoying bool here - just to get past init call
    {
        if (BMSKeyCmdTries < maxBMSKeyCmdTries)
        {
            // TODO send Fims and log
            amap["BMSKeyCmdSent"]->setVal(0);  // force retry
            amap["BMSKeyCmdTries"]->addVal(1);
        }
        else
        {
            if (1)
                FPS_ERROR_PRINT("%s >>>>>> BMS Retry Cmd failed at %f \n ", __func__, tNow);
            char* cval = (char*)"BMS Startup";
            char* cvals = (char*)"Startup Timeout";
            amap["BMSSystemState"]->setVal(cval);
            amap["BMSSystemStateStep"]->setVal(cvals);
        }
    }
    else if (BMSStatus == CATL::NORMAL)
    {
        if (1)
            FPS_ERROR_PRINT("%s >>>>>> BMS Left Init stage at %f \n ", __func__, tNow);
        char* cval = (char*)"BMS Startup";
        char* cvals = (char*)"Startup Complete";
        amap["BMSSystemState"]->setVal(cval);
        amap["BMSSystemStateStep"]->setVal(cvals);
        amap["BMSStartup"]->setVal(true);
        amap["BMSKeyCmdTries"]->setVal(0);
        amap["BMSKeyCmdSent"]->setVal(0);  // force retry
    }

    if (((tNow - BMSKeyCmdTime) > BMSKeyCmdOnTime) && BMSKeyCmd != CATL::INITIAL)
    {
        // amap["BMSKeyCmd"]->setVal(CATL::STAYSTATUS);
        amap["BMSKeyCmd"]->setVal(CATL::INITIAL);
    }
}

// DO NOT need anything below this comment:

// NOTE: Needs aname to be a particular bms_number - Might be enum instead.
// IMPORTANT: This is DEPRECATED, we only control a single register to tell all
// of the BMS's to Startup.
void StartupBMSasset(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* ai)
{
    // Timer StartupBMSTimer("StartupBMSTimer", true);

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
    VarMapUtils* vm = ai->am->vm;
    double tNow = vm->get_time_dbl();

    if (0)
        FPS_ERROR_PRINT("%s >> Startup BMSAsset called for (BMS: %s) at time: %f \n", __func__, aname, tNow);

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
        // /*"ActivePowerSetpoint", "ReactivePowerSetpoint",*/ "StartupTime");
        // linkVals(*vm, vmap, amap, "ess", "/config", dval,
        // "PCSKeyActivePowerSetpoint", "PCSKeyReactivePowerSetpoint");
        // linkVals(*vm, vmap, amap, "ess", "/status", dval, "PCSActivePower",
        // "PCSReactivePower");   // responses
        linkVals(*vm, vmap, amap, "ess", "/config", dval, "maxBMSStartupTime");
        // linkVals(*vm, vmap, amap, "ess", "/config", dval,
        // "PowerSetpointDeadband");
        linkVals(*vm, vmap, amap, aname, "/status", tValInit, "BMSSystemState", "BMSSystemStateStep");
        linkVals(*vm, vmap, amap, aname, "/status", dval, "BMSStartupTime", "BMSStartupStart");
        linkVals(*vm, vmap, amap, "ess", "/config", dval, "maxBMSStartupTime");
        linkVals(*vm, vmap, amap, "bms", "/status", ival, "BMSnumFaults", "BMSnumPresent");

        linkVals(*vm, vmap, amap, aname, "/status", tValInit,
                 "BMSStatus");  //"Power off Ready"
        linkVals(*vm, vmap, amap, aname, "/status", ival, "BMSKeyCmd", "BMSKeyStatus");
        linkVals(*vm, vmap, amap, aname, "/status", bval, "BMSFaulted");

        // amap[__func__] = vm->setLinkVal(vmap, aname,
        // "/reload", __func__, reload);
        bmsAv = amap[__func__];

        // other variables:
        // amap["EMMUFaulted"] = vm->setLinkVal(vmap, aname, "/status",
        // "EMMUFaulted", bval);  amap["StartupRequest"] = vm->setLinkVal(vmap,
        // aname,
        // "/status", "StartupRequest", bval);  amap["StartupRequested"] =
        // vm->setLinkVal(vmap, aname, "/status", "StartupRequested", bval);

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

    // want to see: timer for regular Startup vs. Estart Startup. Use tNow
    // (differences) in FPS_ERROR_PRINTs.

    // get the reference to the variable
    // assetVar* StartupRequestedAV = amap["StartupRequested"];
    // assetVar* PCSkeystartCmdAV = amap["PCSKeystartCmd"];
    // assetVar* PCSkeyCmdAV = amap["PCSKeyCmd"];

    // storing asset states:
    // bool EMMUFaulted = amap["EMMUFaulted"]->getbVal();
    // bool Estart = amap["Estart"]->getbVal();

    // bool BMSFaulted = amap["BMSFaulted"]->getbVal();
    bool BMSStartup = amap["BMSStartup"]->getbVal();
    double BMSStartupTime = amap["BMSStartupTime"]->getdVal();
    double maxBMSStartupTime = amap["maxBMSStartupTime"]->getdVal();
    char* BMSStatus = amap["BMSStatus"]->getcVal();

    // int PowerOffStatus = 3;  // todo config
    int PowerOnCmd = 3;  // todo config

    // TODO: Is this logic complete below?

    if (BMSStartup)
    {
        amap["BMSnumPresent"]->subVal(1);
        return;
    }
    // if (BMSFaulted || Estart)
    // {
    //     amap["BMSnumFaults"]->addVal(1);
    //     amap["BMSStartup"]->setVal(true);
    //     amap["BMSnumPresent"]->subVal(1); // going to be used elsewhere?
    //     // send activePower reactivepower to 0
    //     amap["BMSKeyCmd"]->setVal(PowerOffCmd);
    //     return;
    // }

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
        char* cval = (char*)"BMS Startup";
        char* cvals = (char*)"Setting Power Off";
        amap["BMSSystemState"]->setVal(cval);
        amap["BMSSystemStateStep"]->setVal(cvals);

        // send activePower reactivepower to 0
        amap["BMSKeyCmd"]->setVal(PowerOnCmd);
        amap["BMSStartupStart"]->setVal(tNow);
    }

    // Handle BMS warning
    // Must match modbus config

    // char* tWarningStat = (char*)"Warning status";

    // if (strcmp(BMSStatus, tValPowerOffFault) == 0)
    // {
    //     char* cval = (char*)"BMS Startup";
    //     char* cvals = (char*)"BMS Status Warning";
    //     amap["BMSSystemState"]->setVal(cval);
    //     amap["BMSSystemStateStep"]->setVal(cvals);

    //     // send activePower reactivepower to 0
    //     amap["BMSKeyCmd"]->setVal(PowerOffCmd);
    //     amap["BMSStartupStart"]->setVal(tNow);
    //     amap["BMSnumFaults"]->addVal(1);
    //     amap["BMSStartup"]->setVal(true);
    //     amap["BMSnumPresent"]->subVal(1);
    //     return;
    // }
    // Handle BMS Fault Status
    // Must match modbus config
    char* tFaultStat = (char*)"Fault status";
    if (strcmp(BMSStatus, tFaultStat) == 0)
    {
        char* cval = (char*)"BMS Startup";
        char* cvals = (char*)"BMS Status Fault";
        amap["BMSSystemState"]->setVal(cval);
        amap["BMSSystemStateStep"]->setVal(cvals);

        // send activePower reactivepower to 0 - This is for PCS
        amap["BMSKeyCmd"]->setVal(PowerOnCmd);
        // todo send fims message
        amap["BMSStartupStart"]->setVal(tNow);
        amap["BMSnumFaults"]->addVal(1);
        amap["BMSStartup"]->setVal(true);
        amap["BMSnumPresent"]->subVal(1);
        return;
    }
    // Handle BMS Fault Status
    // Must match modbus config
    // NOTE: Does this need keyCmd?
    char* tOffFault = (char*)"Power off Fault";

    if (strcmp(BMSStatus, tOffFault) == 0)
    {
        char* cval = (char*)"BMS Startup";
        char* cvals = (char*)"BMS PowerOff  Fault";
        amap["BMSSystemState"]->setVal(cval);
        amap["BMSSystemStateStep"]->setVal(cvals);

        // send activePower reactivepower to 0
        // amap["BMSKeyCmd"]->setVal(PowerOffCmd);
        // amap["BMSStartupStart"]->setVal(tNow);
        amap["BMSnumFaults"]->addVal(1);
        amap["BMSStartup"]->setVal(true);
        amap["BMSnumPresent"]->subVal(1);
        return;
    }
    // Completed
    // NOTE: Does this need keyCmd?
    char* tOffReady = (char*)"Power off Ready";
    if (strcmp(BMSStatus, tOffReady) == 0)
    {
        char* cval = (char*)"BMS Startup";
        char* cvals = (char*)"BMS Startup Completed";
        amap["BMSSystemState"]->setVal(cval);
        amap["BMSSystemStateStep"]->setVal(cvals);

        // send activePower reactivepower to 0
        amap["BMSStartup"]->setVal(true);
        amap["BMSnumPresent"]->subVal(1);
        return;
    }

    // timed out ??
    // keystartCmd for timeout needed? - what to do?
    // Does this need a special protocol?
    if (((tNow - BMSStartupTime) > maxBMSStartupTime) && !BMSStartup)
    {
        char* cval = (char*)"BMS Startup";
        char* cvals = (char*)"Startup Timeout";
        amap["BMSSystemState"]->setVal(cval);
        amap["BMSSystemStateStep"]->setVal(cvals);
        amap["BMSFaulted"]->setVal(true);
        amap["BMSStartup"]->setVal(true);
        // ToDo Send Alarm
        if (1)
            FPS_ERROR_PRINT("%s >>>>>> Forced BMS Startup at %f \n ", __func__, tNow);
        amap["BMSnumPresent"]->subVal(1);
        amap["BMSnumFaults"]->addVal(1);
    }
}

// NOTE: Needs aname to be a particular pcs_number - Might be enum instead.
// TODO: Need to actually implement this properly, only BMS is done right now.
// IMPORTANT: Might be deprecated, need to check out registers.
void StartupPCSasset(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* ai)
{
    // Timer StartupBMSTimer("StartupBMSTimer", true);

    // TODO: do this function in the future.

    int reload = 0;
    bool bval = false;
    int ival = 0;
    // double dval = 0.0;
    VarMapUtils* vm = ai->am->vm;
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
        amap["StartupRequest"] = vm->setLinkVal(vmap, aname, "/status", "StartupRequest", bval);
        amap["StartupRequested"] = vm->setLinkVal(vmap, aname, "/status", "StartupRequested", bval);
        amap["PCSKeystartCmd"] = vm->setLinkVal(vmap, aname, "/config", "PCSKeystartCmd", ival);

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
    // assetVar* StartupRequestedAV = amap["StartupRequested"];
    // assetVar* PCSkeystartCmdAV = amap["PCSKeystartCmd"];
    // assetVar* PCSkeyCmdAV = amap["PCSKeyCmd"];

    // storing asset states:
    // bool EMMUFaulted = amap["EMMUFaulted"]->getbVal();
    // bool BMSFaulted = amap["BMSFaulted"]->getbVal();
    // bool PCSFaulted = amap["PCSFaulted"]->getbVal();
    // bool EMUFaulted = amap["EMUFaulted"]->getbVal(); // Supposed to be EMS?
    // bool StartupRequest = amap["StartupRequest"]->getbVal();
    // bool StartupRequested = amap["StartupRequested"]->getbVal();
    // int PCSKeystartVal = amap["PCSKeystartCmd"]->getiVal();
    // int PCSKeyStatusVal = amap["PCSKeyStatus"]->getiVal();
    // int PCSKeyCmdVal = amap["PCSKeyCmd"]->getiVal();
}

// void sendKeystartCmd(/* faultFuncArgs, other args? */) //LC send KEYstart
// command to PCS
// {
//     printf("sendKeystartCmd!\n");
//     //     if(1) FPS_ERROR_PRINT("%s >> %s is sending keystart cmd to pcs\n",
//     __func__, aname);
//     //
//     //     VarMapUtils* vm = ai->vm;
//     //
//     //     if (!amap[Keystart])
//     //     {
//     //         bool bval;
//     //         amap[Keystart] = vm->setLinkVal(vmap, "pcs", "/controls",
//     "KeystartCmd", bval);
//     //     }
//     //
//     //     // Need to check if pcs has already received keystart cmd
//     (keystart = true)
//     //
//     //     bool keystartCmdActive =
//     amap[Keystart]->getVal(keystartCmdActive);
//     //     int sysState = amap[SystemStateNum]->getVal(sysState);
//     //     if (!keystartCmdActive && sysState == System_Fault && aname !=
//     "pcs")
//     //     {
//     //         amap[keystartCmdActive]->setVal(true);
//     //         vm->sendAssetVar(amap[keystartCmdActive], p_fims);
//     //     }
//     //
//     //     // After KeystartCmd has been sent to PCS, we'll probably need to
//     check if the cmd actually went through
//     //     // Could have a /status/pcs/keystart possibly?
// }

// void sendStandbyCmd()
// {
//     printf("Sending standbyCmd!\n");
// }

// void DCContactorTrip() // BAT DC switch & contactor trip
// {
//     printf("DC Contactor Trip!\n");
// }

// void DCSwitchTrip() // BAT DC switch & contactor trip
// {
//     printf("DC switch Trip!\n");
// }

// void handleMaxCellVoltage()
// {
//     printf("Handling Max Cell Voltage!\n");
// }
}
}  // namespace defaultRun_module

#endif
