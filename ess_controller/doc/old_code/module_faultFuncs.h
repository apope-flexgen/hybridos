#ifndef FAULTFUNCS_HPP
#define FAULTFUNCS_HPP

#include "asset.h"
#include "funcRef.h"
#include "testUtils.h"
#include "varMapUtils.h"

#include <chrono>
#include <string>
#define PCSStopCmd 1
#define PCSStopResp 5
#define PCSCmdTime 0.5
// TODO FROM PHIL:
// take a look at getMapsCJ
//

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
void ShutdownSystem(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am);
void ShutdownPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am);
void ShutdownBMS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am);
void ShutdownPCSasset(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* ai);
void ShutdownBMSasset(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* ai);
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
void ShutdownSystem(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
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
        linkVals(*vm, vmap, amap, "ess", "/status", dval, "CurrentSetpoint", "ShutdownTime");
        linkVals(*vm, vmap, amap, "ess", "/config", dval, "maxPCSShutdownTime", "maxBMSShutdownTime");
        // linkVals(*vm, vmap, amap, aname, "/status", ival, "BmsStatus",
        // "PcsStatus", "EmuStatus", "EmmuStatus");

        // other variables:
        linkVals(*vm, vmap, amap, aname, "/status", bval, "EStop", "UiShutdown", "SMShutdown", "FaultShutdown");
        linkVals(*vm, vmap, amap, aname, "/status", bval, "EStopSeen", "UiShutdownSeen", "SMShutdownSeen",
                 "ShutdownCmd", "FaultShutdownSeen");
        linkVals(*vm, vmap, amap, "ess", "/status", bval, "EMMUFaulted", "BMSFaulted", "PCSFaulted", "EMUFaulted");
        linkVals(*vm, vmap, amap, aname, "/status", bval, "ShutdownRequest", "ShutdownRequested", "ShutdownCompleted",
                 "PCSShutdownForced", "BMSShutdownForced");
        // NOTE: These are all under "ess" where it should be, EStop especially
        // needs this.
        linkVals(*vm, vmap, amap, "ess", "/status", bval, "PCSShutdown", "BMSShutdown", "ShutdownSim",
                 "ShutdownSimSeen", "ShutdownCompleted");
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
            // amap["EStop"]->setVal(false); // Shutdown from eStop command
            amap["UiShutdown"]->setVal(false);         // Shutdown from UI command
            amap["SMShutdown"]->setVal(false);         // Shutdown from site_manager
            amap["FaultShutdown"]->setVal(false);      // Shutdown from fault condition
            amap["maxPCSShutdownTime"]->setVal(15.0);  // config?
            amap["maxBMSShutdownTime"]->setVal(15.0);  // config?

            amap["ShutdownRequested"]->setVal(false);
            amap["ShutdownSim"]->setVal(false);      // simulated Shutdown
            amap["ShutdownSimSeen"]->setVal(false);  // flag to make sure sim doesn't start up again.

            // BELOW: this runs reloads for asset_managers (does no processing - only
            // for linkVals).
            amap["BMSShutdown"]->setVal(true);  // flag to make sure sim doesn't start up again.
            amap["PCSShutdown"]->setVal(true);  // flag to make sure sim doesn't start up again.
            for (auto& ix : am->assetManMap)    // should get (const?) references to
                                                // pairs NOT copies.
            {
                FPS_ERROR_PRINT("     %s >>>>>> INIT for asset_managers\n", ix.second->name.c_str());
                // For MVP - We need at least PCS and BMS Shutdowns. Will add more.
                asset_manager* amc = ix.second;
                if (amc->name == "pcs")
                {
                    ShutdownPCS(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
                }
                if (amc->name == "bms")
                {
                    ShutdownBMS(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
                }
                FPS_ERROR_PRINT("     %s >>>>>> INIT DONE for asset_managers\n", ix.second->name.c_str());
            }
            // Below is the real reset (above is only for linkVals and init)
            amap["BMSShutdown"]->setVal(false);  // flag to indicate Shutdown complete
            amap["PCSShutdown"]->setVal(false);  // flag to indicate Shutdown complete
            amap["BMSFaulted"]->setVal(false);   // flag to indicate Shutdown complete
            amap["PCSFaulted"]->setVal(false);   // flag to indicate Shutdown complete
            amap["ShutdownCmd"]->setVal(false);
            amap["ShutdownCmd"]->setParam("HardShutdown", false);
            amap["SystemState"]->setVal(tVal);
        }
        reload = 2;
        essAv->setVal(reload);
    }

    // double tNow = vm->get_time_dbl();

    // This function will be called by (ESS Controller - where this function is
    // located under) on the following conditions:
    //          *     1) eSTOP from ANYWHERE!
    //          *     2) stop command from Site_Controller
    //          *     3) Internal fault (ANY fault), shut down EVERYTHING! (a
    //          "fault stop")
    //          *     4) Through the User Interface (press a button!) - Web UI
    //          *     ... more? (don't know yet)
    // bool EStop = amap["EStop"]->getbVal();
    bool UiShutdown = amap["UiShutdown"]->getbVal();
    bool SMShutdown = amap["SMShutdown"]->getbVal();
    bool FaultShutdown = amap["FaultShutdown"]->getbVal();
    bool ShutdownCompleted = amap["ShutdownCompleted"]->getbVal();
    double ShutdownTime = amap["ShutdownTime"]->getdVal();
    bool ShutdownRequested = amap["ShutdownRequested"]->getbVal();
    bool PCSShutdown = amap["PCSShutdown"]->getbVal();
    bool BMSShutdown = amap["BMSShutdown"]->getbVal();
    bool PCSFaulted = amap["PCSFaulted"]->getbVal();
    bool BMSFaulted = amap["BMSFaulted"]->getbVal();
    char* cstate = amap["SystemState"]->getcVal();
    char* csstate = amap["SystemStateStep"]->getcVal();
    double sdtime = amap["ShutdownTime"]->getdVal();  // seems redundant

    bool ShutdownSim = amap["ShutdownSim"]->getbVal();
    bool ShutdownSimSeen = amap["ShutdownSimSeen"]->getbVal();
    int PCSStatusResp = amap["PCSStatusResp"]->getiVal();
    int BMSStatusResp = amap["BMSStatusResp"]->getiVal();

    assetVar* UiShutdownAv = amap["UiShutdown"];        //->getbVal();
    assetVar* SMShutdownAv = amap["SMShutdown"];        //->getbVal();
    assetVar* FaultShutdownAv = amap["FaultShutdown"];  //->getbVal();
    if (0)
        FPS_ERROR_PRINT("%s >> Running Shutdown  at time: %f\n", __func__, tNow);

    if (ShutdownSim)
    {
        if (!ShutdownSimSeen)
        {
            if (1)
                FPS_ERROR_PRINT("%s >> Starting Shutdown Sim at time: %f\n", __func__, tNow);

            amap["ShutdownTime"]->setVal(tNow);
            amap["ShutdownSim"]->setVal(false);
            amap["ShutdownSimSeen"]->setVal(true);
            amap["SMShutdown"]->setVal(true);
            amap["ShutdownCompleted"]->setVal(false);
            SMShutdown = true;
            ShutdownTime = tNow;
        }

        if (tNow - ShutdownTime > 35.0)  // 35 for sim?
        {
            // amap["ShutdownSim"]->setVal(false);
            amap["ShutdownSimSeen"]->setVal(false);
            amap["ShutdownCompleted"]->setVal(false);  // false?

            // TODO: Check these for correctness: - they don't help
            amap["ShutdownBMS"]->setVal(true);
            amap["ShutdownPCS"]->setVal(true);
        }
        else if (tNow - ShutdownTime > 15.0)
        {
            if (1)
                FPS_ERROR_PRINT("%s >> Stopping ShutdownSim at time: %f (tNow-ShutdownTime) %f\n", __func__, tNow,
                                (tNow - ShutdownTime));
            // amap["ShutdownSim"]->setVal(false);
        }
        else if (tNow - ShutdownTime > 10.0)
        {
            // forced shutdown is being caused here after 10 seconds.
            if (!BMSShutdown)
            {
                if (1)
                    FPS_ERROR_PRINT("%s >>Forcing  Shutdown BMS at time: %f\n", __func__, tNow);
                amap["BMSShutdown"]->setVal(true);
            }
        }
        else if (tNow - ShutdownTime > 5.0)
        {
            if (!PCSShutdown)
            {
                if (1)
                    FPS_ERROR_PRINT("%s >>Forcing  Shutdown PCS at time: %f\n", __func__, tNow);
                amap["PCSShutdown"]->setVal(true);
            }
        }
        else if (tNow - ShutdownTime > 4.0)
        {
            if (PCSStatusResp != PCSStopResp)
            {
                if (1)
                    FPS_ERROR_PRINT("%s >> Forcing pcs stop reply at time: %f\n", __func__, tNow);
                amap["PCSStatusResp"]->setVal(PCSStopResp);
            }
        }
        else if (tNow - ShutdownTime > 3.0)
        {
            if (BMSStatusResp != BMSStopResp)
            {
                if (1)
                    FPS_ERROR_PRINT("%s >> Forcing pcs stop reply at time: %f\n", __func__, tNow);
                amap["BMSStatusResp"]->setVal(BMSStopResp);
            }
        }
    }
    // TODO allow a state change in any input command to change this
    if (ShutdownCompleted)  // Need this under bms and pcs Shutdowns
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
        if (ShutdownRequested)
        {
            amap["ShutdownRequested"]->setVal(false);
        }
        amap["ShutdownCompleted"]->setVal(false);

        return;
    }
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
    bool ShutdownCmd = amap["ShutdownCmd"]->getbVal();

    if (ShutdownCmd)  // TODO: Make these one-shots
    {
        if (amap["ShutdownRequested"]->getbVal())
        {
            if (1)
                FPS_ERROR_PRINT(
                    "%s >> Shutdown Request Ignored,  In request at time: "
                    "%f elap %f \n",
                    __func__, tNow, (tNow - ShutdownTime));
        }
        if (1)
            FPS_ERROR_PRINT("UIShutdown: %s, SMShutdown: %s, FaultShutdown: %s\n", UiShutdown ? "true" : "false",
                            SMShutdown ? "true" : "false", FaultShutdown ? "true" : "false");
        if (!ShutdownRequested)  // we are having a shutdown requested here when you
                                 // set ShutdownCompleted to false.
        {
            if (1)
                FPS_ERROR_PRINT("%s >> Func Starting Shutdown at time: %f elap %f \n", __func__, tNow,
                                (tNow - ShutdownTime));
            amap["ShutdownCmd"]->setVal(false);

            const char* cVal = "Shutdown";
            bool bval = false;
            amap["SystemState"]->setVal(cVal);
            cVal = "Waiting PCS Shutdown";
            amap["SystemStateStep"]->setVal(cVal);

            amap["ShutdownRequested"]->setVal(true);
            // Sadly we have to be specific here tried to get away from the list
            amap["PCSShutdown"]->setVal(bval);
            amap["BMSShutdown"]->setVal(bval);
            amap["BMSFaulted"]->setVal(false);  // flag to indicate Shutdown complete
            amap["PCSFaulted"]->setVal(false);  // flag to indicate Shutdown complete

            amap["ShutdownCompleted"]->setVal(false);
            // Start of the Shutdown:
            tNow = am->vm->get_time_dbl();
            amap["ShutdownTime"]->setVal(tNow);  // This is what is likely causing it.
            if (1)
                FPS_ERROR_PRINT("%s >> Setting ShutdownTime to %f\n", __func__, tNow);

            vm->sendAlarm(vmap, "/status/ess:ShutdownRequested", "/components/ess:alarms", "Shutdown",
                          "Shutdown Started", 1);
        }
    }

    ShutdownRequested = amap["ShutdownRequested"]->getbVal();
    if (!ShutdownRequested)
    {
        if (0)
            FPS_ERROR_PRINT("%s >> Func Quitting Shutdown at  time: %f\n", __func__, tNow);
        return;
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
        if (0)
            FPS_ERROR_PRINT("manager_name: [%s] PCSFaulted [%s] BMSFaulted [%s]\n", amc->name.c_str(),
                            PCSFaulted ? "true" : "false", BMSFaulted ? "true" : "false");

        if (amc->name == "pcs")
        {
            if (0)
                FPS_ERROR_PRINT("manager_name: [%s] PCSShutdown [%s]\n", amc->name.c_str(),
                                PCSShutdown ? "true" : "false");

            if (!PCSFaulted && (!PCSShutdown || BMSFaulted))
            {
                if (1)
                    FPS_ERROR_PRINT("%s >>>> cascading function to >>>>> Manager [%s] at time: %f\n ", __func__,
                                    amc->name.c_str(), tNow);
                ShutdownPCS(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
            }
        }
        if (amc->name == "bms")
        {
            std::cout << std::boolalpha << "PCSFaulted?: " << PCSFaulted << '\n';
            if (0)
                FPS_ERROR_PRINT("manager_name: [%s] BMSShutdown [%s]\n", amc->name.c_str(),
                                BMSShutdown ? "true" : "false");

            if (amap["ShutdownCmd"]->getbParam("HardShutdown"))
            {
                if (!BMSFaulted &&
                    ((PCSShutdown && !BMSShutdown) || PCSFaulted))  // This logic was added to guarantee no
                                                                    // cascade is done without PCS being
                                                                    // shutdown first. Might need to be
                                                                    // changed.
                {
                    if (1)
                        FPS_ERROR_PRINT(
                            "%s >>>> cascading function to >>>>> Manager [%s] "
                            "at time: %f\n ",
                            __func__, amc->name.c_str(), tNow);
                    ShutdownBMS(vmap, amc->amap, amc->name.c_str(), p_fims, amc);
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

    if (BMSShutdown && PCSShutdown)  // then it goes here and does this.
    {
        if (!ShutdownCompleted)
        {
            const char* cVal = "Full Shutdown";
            amap["SystemState"]->setVal(cVal);
            cVal = "Full Shutdown Completed";
            amap["SystemStateStep"]->setVal(cVal);

            amap["ShutdownRequested"]->setVal(false);
            amap["ShutdownCompleted"]->setVal(true);
            if (1)
                FPS_ERROR_PRINT("%s >> Full Shutdown Completed, elapsed time: %f \n", __func__, (tNow - sdtime));
            vm->sendAlarm(vmap, "/status/ess:ShutdownRequested", "/components/ess:alarms", "Shutdown",
                          "Shutdown Completed", 1);
        }
    }
    else if (!amap["ShutdownCmd"]->getbParam("HardShutdown") && PCSShutdown)
    {
        if (!ShutdownCompleted)
        {
            const char* cVal = "PCS Shutdown";
            amap["SystemState"]->setVal(cVal);
            cVal = "PCS Shutdown Completed";
            amap["SystemStateStep"]->setVal(cVal);

            amap["ShutdownRequested"]->setVal(false);
            amap["ShutdownCompleted"]->setVal(true);
            if (1)
                FPS_ERROR_PRINT("%s >> PCS Shutdown Completed, elapsed time: %f \n", __func__, (tNow - sdtime));
            vm->sendAlarm(vmap, "/status/ess:ShutdownRequested", "/components/ess:alarms", "Shutdown",
                          "Shutdown Completed", 1);
        }
    }

    return;
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
void ShutdownPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    // Timer ShutdownBMSTimer("ShutdownBMSTimer", true);

    int reload = 0;
    bool bval = false;
    int ival = 0;
    double dval = 0.0;
    char* tValInit = (char*)"Init";
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
        linkVals(*vm, vmap, amap, "ess", "/status", dval, "ShutdownTime");
        linkVals(*vm, vmap, amap, "ess", "/config", dval, "maxPCSShutdownTime");
        linkVals(*vm, vmap, amap, "ess", "/config", dval, "PowerSetpointDeadband");
        linkVals(*vm, vmap, amap, "ess", "/config", dval, "maxPCSShutdownTime");

        linkVals(*vm, vmap, amap, "ess", "/config", dval, "PCSKeyActivePowerSetpoint", "PCSKeyReactivePowerSetpoint");
        linkVals(*vm, vmap, amap, "ess", "/status", bval, "PCSShutdown", "PCSStatus", "PCSStopSent");
        linkVals(*vm, vmap, amap, "ess", "/status", bval, "PCSFaulted", "PCSFaultSeen", "ShutdownRequest",
                 "ShutdownRequested", "ShutdownCmd");

        linkVals(*vm, vmap, amap, aname, "/status", tValInit, "PCSSystemState", "PCSSystemStateStep");
        linkVals(*vm, vmap, amap, aname, "/status", dval, "PCSShutdownTime", "PCSShutdownStart", "PCSKeyCmdTime");
        linkVals(*vm, vmap, amap, aname, "/status", dval, "PCSActivePower",
                 "PCSReactivePower");  // ModBus responses
        linkVals(*vm, vmap, amap, aname, "/status", ival, "PCSStatusResp", "PCSKeyStop",
                 "PCSKeyCmdTries");  // returned status    PCSKEYCmd perhaps
        // Jimmy ... PCSStatusResp is mapped to the pcs_status
        // other variables:
        // todo: link EStop to aname = "ess" not itself.
        linkVals(*vm, vmap, amap, aname, "/status", ival, "PCSKeyStopSent");
        linkVals(*vm, vmap, amap, aname, "/status", ival, "PCSKeyCmd");
        linkVals(*vm, vmap, amap, aname, "/config", ival, "PCSKeyStopCmd");
        linkVals(*vm, vmap, amap, aname, "/config", ival, "PCSFaultStatus");

        // other variables
        // linkVals(*vm, vmap, amap, aname, "/status", ival, "PCSKeyStatus",
        // "PCSKeyCmd");

        if (reload == 0)  // complete restart
        {
            amap["PCSSystemState"]->setVal(tValInit);
            amap["PCSFaulted"]->setVal(false);
            amap["PCSShutdown"]->setVal(false);
            amap["PCSStopSent"]->setVal(false);
            amap["PowerSetpointDeadband"]->setVal(0.01);
            amap["PCSActivePower"]->setVal(0.0);
            amap["PCSReactivePower"]->setVal(0.0);
            amap["maxPCSShutdownTime"]->setVal(15.0);  // config?

            amap["PCSStatusResp"]->setVal(0);
            amap["PCSKeyCmdTries"]->setVal(0);
            amap["PCSKeyCmdTime"]->setVal(tNow);
            amap["PCSKeyStopSent"]->setVal(0);
            if (0)
                FPS_ERROR_PRINT("%s >>>>>>>>>>>>>>>>PCS Shutdown system state [%s]  at time: %f \n", __func__,
                                amap["PCSSystemState"]->getcVal(), tNow);
        }
        reload = 2;
        pcsAv->setVal(reload);
    }
    const char* stopUri = amap["ShutdownCmd"]->getbParam("HardShutdown") ? "/components/remote_control/e_stop"
                                                                         : "/components/remote_control/stop";

    char* PCSSystemState = amap["PCSSystemState"]->getcVal();
    char* PCSSystemStateStep = amap["PCSSystemStateStep"]->getcVal();
    if (amap["PCSSystemState"]->valueChangedReset() | amap["PCSSystemStateStep"]->valueChangedReset())
    {
        if (1)
            FPS_ERROR_PRINT("%s >> PCS State [%s] Step [%s] time: %f \n", __func__, PCSSystemState, PCSSystemStateStep,
                            tNow);
    }
    bool PCSShutdown = amap["PCSShutdown"]->getbVal();
    char* SystemStateStep = amap["PCSSystemStateStep"]->getcVal();
    int PCSStatusResp = amap["PCSStatusResp"]->getiVal();
    bool ShutdownRequested = amap["ShutdownRequested"]->getbVal();

    if (PCSShutdown)
    {
        if (strcmp(SystemStateStep, "Waiting PCS Shutdown") == 0)
        {
            SystemStateStep = (char*)"PCS Shutdown Complete";
            amap["SystemStateStep"]->setVal(SystemStateStep);
            if (1)
                FPS_ERROR_PRINT("%s >> All PCS's succesfully Shutdown at elapsed time: %f \n", __func__, tNow);
        }
        return;
    }

    bool PCSFaulted = amap["PCSFaulted"]->getbVal();

    // May have to do other stuff
    if (PCSFaulted)
    {
        if (strcmp(SystemStateStep, "Waiting PCS Shutdown") == 0)
        {
            SystemStateStep = (char*)"PCS Shutdown Fault";
            amap["SystemStateStep"]->setVal(SystemStateStep);
            if (1)
                FPS_ERROR_PRINT("%s >> PCS Faulted at time: %f \n", __func__, tNow);
        }

        // May have to do other stuff for example
        vm->sendAlarm(vmap, "/status/ess:ShutdownRequested", "/components/pcs:alarms", "Shutdown", "PCS Fault Shutdown",
                      1);

        amap["PCSShutdown"]->setVal(true);
        return;
    }

    // double  PCSShutdownTime = amap["PCSShutdownTime"]->getdVal();
    double maxPCSShutdownTime = amap["maxPCSShutdownTime"]->getdVal();
    double PCSShutdownStart = amap["PCSShutdownStart"]->getdVal();
    int PCSKeyStop = amap["PCSKeyStop"]->getiVal();
    // KeyStopSent used to turn off command KeyStop used to indicate command sent
    int PCSKeyStopSent = amap["PCSKeyStopSent"]->getiVal();
    int PCSKeyCmdTries = amap["PCSKeyCmdTries"]->getiVal();
    double maxPCSKeyCmdOnTime = 0.5;  // config
    double maxPCSKeyCmdTime = 2.5;    // config
    int maxPCSKeyCmdTries = 5;        // config

    PCSSystemState = amap["PCSSystemState"]->getcVal();
    PCSSystemStateStep = amap["PCSSystemStateStep"]->getcVal();
    // char* PCSSystemStateStep = amap["PCSSystemStateStep"]->getcVal();
    // IF Run or Ready start Shtdown
    // Do not run if in Init The PCS monitor will bring things out of init or we
    // have to do it by hand
    if (0)
        FPS_ERROR_PRINT("%s >>>> PCS Shutdown system state [%s] step [%s]  at time: %f \n", __func__, PCSSystemState,
                        PCSSystemStateStep, tNow);

    if (ShutdownRequested)
    {
        if (strcmp(PCSSystemState, tValInit) == 0)
        {
            char* cvals = (char*)"PCS still in Init";

            if (!PCSShutdown || !PCSSystemStateStep || (strcmp(PCSSystemStateStep, cvals) != 0))
            {
                amap["PCSSystemStateStep"]->setVal(cvals);
                amap["PCSShutdown"]->setVal(true);
                if (1)
                    FPS_ERROR_PRINT(
                        "%s >>>> PCS Shutdown  Forced because we're in "
                        "system state [%s]  at time: %f \n",
                        __func__, PCSSystemState, tNow);
            }
            return;
        }
        else if (PCSStatusResp == Power_Electronics::STOP ||
                 PCSStatusResp == Power_Electronics::FAULT)  // TODO: Will be
                                                             // configed - probably
                                                             // come in as a text
                                                             // stream  JIMMY
        {
            if (1)
                FPS_ERROR_PRINT("%s >> PCS Status Stop Reponse found at elapsed time: %f \n", __func__,
                                (maxPCSKeyCmdTime * PCSKeyCmdTries) + (tNow - PCSShutdownStart));
            // char* cval = (char*)"PCS Shutdown";
            char* cvals = (char*)"Shutdown Complete";
            // amap["SystemState"]->setVal(cval);
            amap["PCSSystemStateStep"]->setVal(cvals);
            cvals = (char*)"PCS Shutdown Complete";
            amap["SystemStateStep"]->setVal(cvals);
            amap["PCSShutdown"]->setVal(true);
            amap["PCSKeyCmdTries"]->setVal(0);
            amap["PCSKeyStopSent"]->setVal(0);  // allow a resend
            amap["PCSKeyCmd"]->setVal(0);
            p_fims->Send("set", stopUri, nullptr,
                         "{\"value\":0}");  // need to turn off stop command?

            return;
        }
    }
    else
    {
        return;
    }

    // TODO check timeout
    // if we have not sent the KeyStop then wait for the power setpoints to settle
    if (PCSKeyStop == 0)
    {
        // We need a warning if it was due to timeout.
        if (1)
            FPS_ERROR_PRINT("%s >> PCS power OK sending KeyStop at time: %f elspsed: %f \n", __func__, tNow,
                            (tNow - PCSShutdownStart));
        char* cval = (char*)"Shutdown";
        char* cvals = (char*)"Sending Stop";
        amap["PCSSystemState"]->setVal(cval);
        amap["PCSSystemStateStep"]->setVal(cvals);
        amap["PCSKeyStop"]->setVal(PCSStopCmd);
        amap["PCSKeyCmd"]->setVal(PCSStopCmd);
        p_fims->Send("set", stopUri, nullptr, "{\"value\":1}");

        amap["PCSKeyStopSent"]->setVal(PCSStopCmd);
        amap["PCSShutdownTime"]->setVal(tNow);
        amap["PCSShutdownStart"]->setVal(tNow);
        PCSShutdownStart = tNow;
        amap["PCSKeyCmdTime"]->setVal(tNow);
    }

    double PCSKeyCmdTime = amap["PCSKeyCmdTime"]->getdVal();

    // TODO add PCSKeyCmdReset
    // PCSKeyStop != 0 if we have sent a command
    if (!PCSShutdown)  // && (PCSKeyStop != 0))
    {
        if ((tNow - PCSKeyCmdTime) > maxPCSKeyCmdTime)
        {
            if (PCSKeyCmdTries < maxPCSKeyCmdTries - 1)
            {
                amap["PCSKeyStop"]->setVal(0);      // force a resend
                amap["PCSKeyStopSent"]->setVal(0);  // force a resend

                amap["PCSKeyCmdTries"]->addVal(1);
                amap["PCSKeyCmd"]->setVal(0);
                // amap["PCSKeyCmdTime"]->setVal(tNow);

                vm->sendAlarm(vmap, amap["PCSKeyCmd"], "/components/pcs:alarms", "PCSKEYSTOP", "PCS Ignored Command",
                              1);
                if (1)
                    FPS_ERROR_PRINT(
                        "%s >> PCS Stop No PCSStatusResp (%d != %d)  resend "
                        "after %d tries  attime: %f \n",
                        __func__, PCSStatusResp, PCSStopResp, PCSKeyCmdTries + 1, tNow);
                // amap["PCSFaulted"]->setVal(true);
            }
            else if (PCSKeyCmdTries == maxPCSKeyCmdTries - 1)
            {
                amap["PCSKeyCmdTries"]->addVal(1);
                if (1)
                    FPS_ERROR_PRINT(
                        "%s >> PCS Stop No Cmd Response Stopped after %d "
                        "tries  attime: %f \n",
                        __func__, PCSKeyCmdTries + 1, tNow);
                // p_fims->Send("set", "/components/remote_control/e_stop", nullptr,
                // "{\"value\":0}"); there is where forced shutdown occurs
                char* cval = (char*)"PCS Shutdown";
                char* cvals = (char*)"Shutdown Timeout";
                amap["SystemState"]->setVal(cval);
                amap["SystemStateStep"]->setVal(cvals);
                amap["PCSFaulted"]->setVal(true);
                amap["PCSShutdown"]->setVal(true);
                amap["PCSKeyCmdTries"]->setVal(0);
                amap["PCSKeyStop"]->setVal(0);      // allow a resend
                amap["PCSKeyStopSent"]->setVal(0);  // reset possible next pass

                // ToDo Send Alarm
                if (1)
                    FPS_ERROR_PRINT(
                        "%s >>>>>> Time Forced PCS Shutdown at time %f start "
                        "%f elap %f  \n ",
                        __func__, tNow, PCSShutdownStart);
            }
        }
        if (1)
            FPS_ERROR_PRINT("%s >>> PCS Shutdown attempt no. %d time since shutdown start %f\n ", __func__,
                            PCSKeyCmdTries + 1, tNow - PCSShutdownStart);

        vm->sendAlarm(vmap, "/status/ess:ShutdownRequested", "/components/pcs:alarms", "PCSShutdown",
                      "PCS Forced Shutdown", 1);
    }

    if (PCSKeyStopSent != 0)
    {
        if ((tNow - PCSKeyCmdTime) > maxPCSKeyCmdOnTime)
        {
            if (1)
                FPS_ERROR_PRINT("%s >> PCS Reset Cmd  at elapsed time: %f \n", __func__, (tNow - PCSKeyCmdTime));

            amap["PCSKeyStopSent"]->setVal(0);  // force a resend
            amap["PCSKeyCmd"]->setVal(0);
            p_fims->Send("set", stopUri, nullptr, "{\"value\":0}");
            // vm->sendAlarm(vmap ,amap["PCSKeyStop"],"/components/pcs:alarms",
            // "PCSKEYSTOP","PCS Ignored Command",1);
        }
    }

    PCSShutdownStart = amap["PCSShutdownStart"]->getdVal();
    if (((maxPCSKeyCmdTime * PCSKeyCmdTries) + (tNow - PCSShutdownStart) > maxPCSShutdownTime) && !PCSShutdown)
    {
        // there is where forced shutdown occurs
        char* cval = (char*)"PCS Shutdown";
        char* cvals = (char*)"Shutdown Timeout";
        amap["SystemState"]->setVal(cval);
        amap["SystemStateStep"]->setVal(cvals);
        amap["PCSFaulted"]->setVal(true);
        amap["PCSShutdown"]->setVal(true);
        amap["PCSKeyCmdTries"]->setVal(0);
        amap["PCSKeyStop"]->setVal(0);      // allow a resend
        amap["PCSKeyStopSent"]->setVal(0);  // reset possible next pass

        // ToDo Send Alarm
        if (1)
            FPS_ERROR_PRINT("%s >>>>>> Time Forced PCS Shutdown at time %f start %f elap %f  \n ", __func__, tNow,
                            PCSShutdownStart, (tNow - PCSShutdownStart));
    }

    // Fault/e-stop
    // if (amap["ShutdownCmd"]->getbParam("HardShutdown"))
    // {
    //     if (1) FPS_ERROR_PRINT("%s >> Hard Shutdown\n",
    //     __func__); if (strcmp(SystemStateStep, "Waiting
    //     PCS Shutdown") == 0)
    //     {
    //         SystemStateStep = (char*)"PCS Shutdown Fault";
    //         amap["SystemStateStep"]->setVal(SystemStateStep);
    //         if (1) FPS_ERROR_PRINT("%s >> PCSFault at time: %f \n",
    //         __func__, tNow);
    //     }

    //     vm->sendAlarm(vmap, "/status/ess:ShutdownRequested",
    //     "/components/pcs:alarms", "Shutdown", "PCS Fault Shutdown", 1);

    //     // May have to do other stuff for example
    //     // NOTE: Set PCSShutDown to false then EStop will run again.
    //     // amap["PCSShutdown"]->setVal(true);
    //     char* cval = (char*)"Fault";
    //     char* cvals = (char*)"E-Stop sent to PCS";
    //     amap["PCSSystemState"]->setVal(cval);
    //     amap["PCSSystemStateStep"]->setVal(cvals);
    //     amap["PCSKeyStop"]->setVal(PCSStopCmd);     // PCSKeyCmd ??
    //     amap["PCSKeyCmd"]->setVal(PCSStopCmd);     // PCSKeyCmd ??
    //     // TODO Send to Fims
    //     amap["PCSKeyStopSent"]->setVal(PCSStopCmd); // Check use
    //     p_fims->Send("set", stopUri, nullptr, "{\"value\":1}");
    //     return;
    // }
}

// ShutdownBMS
// * This function will consist of the following sequence (run by ESS
// controller)
//  *    1) Send to BMS_Asset the ShutdownBMS command
//  *    2) Wait for BMS_Asset to respond (get ok that all BMS's are Shutdown)
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
   "ActivePowerSetpoint", "ReactivePowerSetpoint", "ShutdownTime");
    ////OK linkVals(*vm, vmap, amap, "ess", "/config", dval,
   "maxBMSShutdownTime");
    ////Remove linkVals(*vm, vmap, amap, "ess", "/config", dval,
   "PowerSetpointDeadband");
    ////OK linkVals(*vm, vmap, amap, aname, "/status", tValInit,
   "BMSSystemState", "BMSSystemStateStep");
    ////OK linkVals(*vm, vmap, amap, aname, "/status", dval, "BMSShutdownTime",
   "BMSShutdownStart");
    ////Fixed linkVals(*vm, vmap, amap, "ess", "/config", dval,
   "maxBMSShutdownTime");
    ////OK linkVals(*vm, vmap, amap, "ess", "/status", bval, "BMSShutdown",
   "BMSStatus");
    ////Remove linkVals(*vm, vmap, amap, "bms", "/status", ival, "BMSnumFaults",
   "BMSnumPresent"); // Need keyCmd here?
        // linkVals(*vm, vmap, amap, aname, "/status", ival, "BMSKeyCmd",
   "BMSKeyStatus");


    ////Remove for now linkVals(*vm, vmap, amap, aname, "/status", ival,
   "BmsStatus");

        // other variables:
        // TODO: Estop to aname = "ess" not itself.
        ////linkVals(*vm, vmap, amap, "ess", "/status", bval, "BMSFaulted",
   "BMSFaultSeen", "ShutdownRequest", "ShutdownRequested");
    ////Fixed linkVals(*vm, vmap, amap, aname, "/status", ival,
   "BMSKeyCmdSent");
        ////linkVals(*vm, vmap, amap, aname, "/status", ival, "BMSKeyCmd",
   "BMSKeyStatus"); // is Status necessary yes for feedback
        //linkVals(*vm, vmap, amap, aname, "/status", bval, "BMSShutdown",
   "BMSStatus");
        // linkVals(*vm, vmap, amap, aname, "/config", ival, "PCSKeyStopCmd");
        ////linkVals(*vm, vmap, amap, aname, "/config", ival, "BMSFaultStatus");

        //EStop:
        ////linkVals(*vm, vmap, amap, "ess", "/status", bval, "EStop"); // Might
   not need this?


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
        ////linkVals(*vm, vmap, amap, "ess", "/status", bval, "EStop");



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

        /status/ess/ShutdownTime
                    time the system took to shutdown.
        /status/bms/BMSShutdownTime
                    time the system took to shutdown.
        /status/bms/BMSShutdownTimeStart
                    time the system started the shutdown.

        // Config
        /config/ess/maxBMSShutdownTime
                    time limits to allow for BMS system to complete shutdown.


        linkVals(*vm, vmap, amap, aname, "/status", ival, "
        //?? /status/ess/BmsStatus   status of the BmsSystem   from modbus input
        //?? /status/ess/PCsStatus   status of the PcsSystem
        //?? /status/ess/PCsStatus   status of the EmuSystem    from ??
        //?? /status/ess/PCsStatus   status of the EmmuSystem    from ??

        // other variables:
        bool

        /status/ess/EStop             estop input flag , several possible
   sources. TODO Should be attached directly to Shutdown.

        /status/ess/BMSShutdown         Check this
        /status/ess/BMSStatus           and this
        /status/bms/BMSStopSent         we have sent the stop to the BMS

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
void ShutdownBMS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am)
{
    // Timer ShutdownBMSTimer("ShutdownBMSTimer", true);

    // TODO: keyCMD is a shared mbmu register that will shut down all of the
    // BMS's.

    int reload = 0;
    bool bval = false;
    int ival = 0;
    double dval = 0.0;
    char* tValInit = (char*)"Init";
    // char* tValRun = (char*)"Run";
    // char* tValReady = (char*)"Ready";
    VarMapUtils* vm = am->vm;

    double tNow = vm->get_time_dbl();

    if (0)
        FPS_ERROR_PRINT("%s >> Shutdown BMS called for (BMS_Manager: %s) at time: %f \n", __func__, aname, tNow);

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
        linkVals(*vm, vmap, amap, "ess", "/status", dval, "ShutdownTime");
        linkVals(*vm, vmap, amap, "ess", "/config", dval, "maxBMSShutdownTime");
        linkVals(*vm, vmap, amap, aname, "/status", tValInit, "BMSSystemState", "BMSSystemStateStep");
        linkVals(*vm, vmap, amap, aname, "/status", dval, "BMSShutdownTime", "BMSShutdownStart");
        // linkVals(*vm, vmap, amap, "ess", "/config", dval, "maxBMSShutdownTime");
        linkVals(*vm, vmap, amap, "ess", "/status", bval, "BMSShutdown");
        linkVals(*vm, vmap, amap, "bms", "/status", ival, "BMSnumFaults",
                 "BMSnumPresent");  // Need keyCmd here?
        // linkVals(*vm, vmap, amap, aname, "/status", ival, "BMSKeyCmd",
        // "BMSKeyStatus");

        // other variables:
        linkVals(*vm, vmap, amap, "ess", "/status", bval, "BMSFaulted", "BMSFaultSeen", "ShutdownRequest",
                 "ShutdownRequested", "ShutdownCmd");
        linkVals(*vm, vmap, amap, "ess", "/status", ival,
                 "BMSKeyStatus");  // maybe /asset/bms
        linkVals(*vm, vmap, amap, aname, "/status", ival, "BMSKeyCmd", "BMSKeyCmdSent",
                 "BMSKeyCmdTries");  // is Status necessary?
        linkVals(*vm, vmap, amap, aname, "/config", dval,
                 "BMSKeyCmdTime");  // is Status necessary?
        linkVals(*vm, vmap, amap, aname, "/config", ival, "BMSFaultStatus");
        // linkVals(*vm, vmap, amap, aname, "/status", ival, "num_hv_subsystem");
        // amap["NumSBMUConnected"] = vm->setLinkVal(vmap, aname, "/status",
        // "num_hv_subsystem", ival);
        amap["BMSPowerOn"] = vm->setLinkVal(vmap, aname, "/status", "bms_poweron", ival);

        if (reload == 0)  // complete restart
        {
            amap["BMSSystemState"]->setVal(tValInit);
            amap["BMSFaulted"]->setVal(false);
            amap["BMSShutdown"]->setVal(false);
            amap["maxBMSShutdownTime"]->setVal(15.0);  // TODO: config
            amap["BMSKeyCmd"]->setVal(0);
            amap["BMSKeyCmdSent"]->setVal(0);
            amap["BMSKeyCmdTries"]->setVal(0);
            amap["BMSKeyCmdTime"]->setVal(tNow);
        }
        reload = 2;
        bmsAv->setVal(reload);
    }

    char* BMSSystemState = amap["BMSSystemState"]->getcVal();
    char* BMSSystemStateStep = amap["BMSSystemStateStep"]->getcVal();
    if (amap["BMSSystemState"]->valueChangedReset() | amap["BMSSystemStateStep"]->valueChangedReset())
    {
        if (1)
            FPS_ERROR_PRINT("%s >>  BMS State [%s] Step [%s] time: %f \n", __func__, BMSSystemState, BMSSystemStateStep,
                            tNow);
    }
    bool BMSShutdown = amap["BMSShutdown"]->getbVal();
    char* SystemStateStep = amap["SystemStateStep"]->getcVal();

    if (BMSShutdown)
    {
        if (strcmp(SystemStateStep, "Waiting BMS Shutdown") == 0)
        {
            SystemStateStep = (char*)"BMS Shutdown Complete";
            amap["SystemStateStep"]->setVal(SystemStateStep);
            if (1)
                FPS_ERROR_PRINT("%s >> All BMS's succesfully Shutdown at elapsed time: %f \n", __func__, tNow);
        }
        return;
    }

    // May have to do other stuff
    bool BMSFaulted = amap["BMSFaulted"]->getbVal();
    if (BMSFaulted)
    {
        if (strcmp(SystemStateStep, "Waiting BMS Shutdown") == 0)
        {
            SystemStateStep = (char*)"BMS Shutdown Fault";
            amap["SystemStateStep"]->setVal(SystemStateStep);
            if (1)
                FPS_ERROR_PRINT("%s >> BMS Faulted at time: %f \n", __func__, tNow);
        }

        // May have to do other stuff for example
        vm->sendAlarm(vmap, "/status/ess:ShutdownRequested", "/components/bms:alarms", "Shutdown", "BMS Fault Shutdown",
                      1);

        amap["BMSShutdown"]->setVal(true);
        return;
    }

    int BMSKeyCmdSent = amap["BMSKeyCmdSent"]->getiVal();
    bool BMSOn = amap["BMSPowerOn"]->getiVal() == 1 ? true : false;
    bool ShutdownRequested = amap["ShutdownRequested"]->getbVal();
    FPS_ERROR_PRINT("%s >>>> Num SBMU: %d On?? %d\n", __func__, amap["BMSPowerOn"]->getiVal(), BMSOn);

    if (ShutdownRequested)
    {
        if (!BMSOn)
        {
            char* cval = (char*)"BMS Shutdown";
            char* cvals = (char*)"Shutdown Complete";
            amap["BMSSystemState"]->setVal(cval);
            amap["BMSSystemStateStep"]->setVal(cvals);
            amap["BMSShutdown"]->setVal(true);
            amap["BMSKeyCmdSent"]->setVal(0);  // force retry
        }
        else if (BMSKeyCmdSent == 0)
        {
            if (strcmp(SystemStateStep, "Waiting BMS Shutdown") == 0)
            {
                SystemStateStep = (char*)"BMS Shutdown EStop";
                amap["SystemStateStep"]->setVal(SystemStateStep);
                if (1)
                    FPS_ERROR_PRINT("%s >> BMSEStop at time: %f \n", __func__, tNow);
            }

            vm->sendAlarm(vmap, "/status/ess:ShutdownRequested", "/components/bms:alarms", "Shutdown",
                          "PCS EStop Shutdown", 1);

            // May have to do other stuff for example
            // NOTE: Set PCSShutDown to false then EStop will run again.
            char* cval = (char*)"Fault";
            char* cvals = (char*)"Stop sent to BMS";
            amap["BMSSystemState"]->setVal(cval);
            amap["BMSSystemStateStep"]->setVal(cvals);
            // EStop crash out
            amap["BMSKeyCmd"]->setVal(BMSStopCmd);  // BMSKeyCmd ??
            // TODO Send to Fims
            p_fims->Send("set", "/components/catl_ems_bms_rw/ems_cmd", nullptr, "{\"value\":3}");
            amap["BMSKeyCmdSent"]->setVal(BMSStopCmd);  // Check use
            amap["BMSKeyCmdTime"]->setVal(tNow);
            return;
        }
    }
    else
    {
        return;
    }

    BMSSystemState = amap["BMSSystemState"]->getcVal();
    // char* PCSSystemStateStep = amap["PCSSystemStateStep"]->getcVal();

    // double  BMSShutdownTime = amap["BMSShutdownTime"]->getdVal();
    double maxBMSShutdownTime = amap["maxBMSShutdownTime"]->getdVal();
    int BMSKeyCmd = amap["BMSKeyCmd"]->getiVal();
    BMSKeyCmdSent = amap["BMSKeyCmdSent"]->getiVal();

    double BMSKeyCmdTime = amap["BMSKeyCmdTime"]->getdVal();
    int BMSKeyCmdTries = amap["BMSKeyCmdTries"]->getiVal();

    double BMSKeyCmdWaitTime = 5.0;   // config
    int maxBMSKeyCmdTries = 5;        // Config
    double maxBMSKeyCmdOnTime = 0.5;  // config
    // check for cmd received
    int BMSKeyStatus = amap["BMSKeyStatus"]->getiVal();
    int BMSKeyStatusStop = 5;  // config JIMMY

    FPS_ERROR_PRINT("bmsshutdown time %f keycmd try no. %d keycmd %d\n",
                    (BMSKeyCmdWaitTime * BMSKeyCmdTries) + (tNow - BMSKeyCmdTime), BMSKeyCmdTries + 1, BMSKeyCmd);

    if (BMSKeyCmdSent != 0)
    {
        if (BMSKeyCmd != 0)
        {
            if ((tNow - BMSKeyCmdTime) > maxBMSKeyCmdOnTime)
            {
                if (1)
                    FPS_ERROR_PRINT("%s >> BMS Reset Cmd  at elapsed time: %f \n", __func__, (tNow - BMSKeyCmdTime));
                amap["BMSKeyCmd"]->setVal(0);
                p_fims->Send("set", "/components/catl_ems_bms_rw/ems_cmd", nullptr, "{\"value\":1}");
            }
        }
        if ((tNow - BMSKeyCmdTime) > BMSKeyCmdWaitTime)
        {
            if (BMSKeyCmdTries < maxBMSKeyCmdTries - 1)
            {
                // TODO send Fims and log
                amap["BMSKeyCmdSent"]->setVal(0);  // force retry
                amap["BMSKeyCmdTries"]->addVal(1);
                BMSKeyCmdTries = amap["BMSKeyCmdTries"]->getiVal();
                if (1)
                    FPS_ERROR_PRINT(
                        "%s >>>>>> BMS cmd status [%d!=%d] Retry Cmd attempt "
                        "%d at %f \n ",
                        __func__, BMSKeyStatus, BMSKeyStatusStop, BMSKeyCmdTries, tNow);
            }
            else
            {
                if (1)
                    FPS_ERROR_PRINT("%s >>>>>> BMS Retry Cmd failed at %f \n ", __func__, tNow);
                amap["BMSShutdown"]->setVal(true);  // add "failedShutdown"?
                amap["BMSKeyCmdTries"]->setVal(0);
                amap["BMSKeyCmdSent"]->setVal(0);
            }
        }
        if (((((BMSKeyCmdWaitTime * (BMSKeyCmdTries - 1)) + (tNow - BMSKeyCmdTime)) > maxBMSShutdownTime)) &&
            !BMSShutdown)
        {
            FPS_ERROR_PRINT("time %f max time %f\n", (BMSKeyCmdWaitTime * BMSKeyCmdTries) + (tNow - BMSKeyCmdTime),
                            maxBMSShutdownTime);
            char* cval = (char*)"BMS Shutdown";
            char* cvals = (char*)"Shutdown Timeout";
            amap["BMSSystemState"]->setVal(cval);
            amap["BMSSystemStateStep"]->setVal(cvals);
            if (1)
                FPS_ERROR_PRINT("%s >>>>>> Forced BMS Shutdown at %f \n ", __func__, tNow);
            amap["BMSShutdown"]->setVal(true);
            amap["BMSKeyCmdTries"]->setVal(0);
            amap["BMSKeyCmdSent"]->setVal(0);  // force retry
        }
    }
    // We can do this but maybe no need since we only have one
    // amap["BMSnumFaults"]->setVal(0);
    // amap["BMSnumPresent"]->setVal((int)am->assetMap.size());

    // int nump = amap["BMSnumPresent"]->getiVal();
    // if (1) FPS_ERROR_PRINT("%s >>>>>> BMS Num Present  [%d] \n ",
    // __func__, nump); bool EStop =
    // amap["EStop"]->getbVal();

    // if (EStop)
    // {
    //     char* cval = (char*)"BMS Shutdown";
    //     char* cvals = (char*)"E-Stop";
    //     amap["SystemState"]->setVal(cval);
    //     amap["SystemStateStep"]->setVal(cvals);
    //     amap["BMSFaulted"]->setVal(true);
    //     amap["BMSShutdown"]->setVal(true);
    //     amap["BMSKeyCmdSent"]->setVal(0);

    //     // amap["BMSKeyCmd"]->setVal(PowerOffCmd); // Is this necessary? Isn't
    //     this an error state?
    // }
}

// DO NOT need anything below this comment:

// NOTE: Needs aname to be a particular bms_number - Might be enum instead.
// IMPORTANT: This is DEPRECATED, we only control a single register to tell all
// of the BMS's to Shutdown.
void ShutdownBMSasset(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* ai)
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
    VarMapUtils* vm = ai->am->vm;
    double tNow = vm->get_time_dbl();

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
        return;
    }
    if (BMSFaulted || EStop)
    {
        amap["BMSnumFaults"]->addVal(1);
        amap["BMSShutdown"]->setVal(true);
        amap["BMSnumPresent"]->subVal(1);  // going to be used elsewhere?
        // send activePower reactivepower to 0
        amap["BMSKeyCmd"]->setVal(PowerOffCmd);
        return;
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
    //     return;
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
        return;
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
        return;
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
        return;
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
}

// NOTE: Needs aname to be a particular pcs_number - Might be enum instead.
// TODO: Need to actually implement this properly, only BMS is done right now.
// IMPORTANT: Might be deprecated, need to check out registers.
void ShutdownPCSasset(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* ai)
{
    // Timer ShutdownBMSTimer("ShutdownBMSTimer", true);

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
