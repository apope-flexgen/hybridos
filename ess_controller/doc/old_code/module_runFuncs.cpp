#ifndef RUNFUNCS_CPP
#define RUNFUNCS_CPP

#include "asset.h"
#include "varMapUtils.h"
#include "testUtils.h"
#include "funcRef.h"
#include "systemEnums.h"

#include <chrono>
#include <string>
#define PCSStartCmd 1
#define PCSStartResp 5
#define PCSCmdTime 0.5
// TODO FROM PHIL: 
// take a look at getMapsCJ

using namespace testUtils;

/**
 * @brief TODO:
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

namespace defaultRun_module
{

    // These are the "unsafe" functions that could be loaded as a module if the engineers need them.
    namespace internal_funcs
    {
        // For external reference to these functions if another program needs it.
        extern "C"
        {
            // TODO: Section these off (a comment with section name will do) so they become easier to manage.
            // TODO: Make sure they aren't external/safe functions (move them down if they are)

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

        constexpr int INTERNAL_FUNC_COUNT = 2; // Make sure to change this as needed.
        struct Internal_funcRefArray
        {
            const funcRef func_array[INTERNAL_FUNC_COUNT] =
            {
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

        //Put funcs here:
        extern "C"
        {
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
    }
    // END INTERNAL_FUNCS NAMESPACE
    // ...
    // ...

    // Extra useful things to use internally (internal_funs:: can get repetitive.)
    // using namespace defaultFault_module::internal_funcs;
    // #define i_f internal_funcs


    // For external reference to these functions if another program needs it.
    extern "C"
    {
        // TODO: Section these off so they become easier to manage (single line name/comment will do).
        // TODO: Make sure they aren't internal/unsafe functions (move them up if they are)

        // Startups:
        int StartupSystem(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
        int StartupPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
        int StartupBMS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
        int StartupPCSasset(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
        int StartupBMSasset(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
        int RunKeyCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);

        // Other:

    }

    constexpr int FUNC_COUNT = 1; // Make sure to change this as needed.
    struct funcRefArray
    {
        const funcRef func_array[FUNC_COUNT] =
        {
            funcRef{"StartupSystem", (void*)&StartupSystem},
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

    // Put fault sequences below (combination of above internal functions, to test for safety):
    // There should NOT be anything over than calls to internal functions inside these functions
    //      If NOT then you could just put it as an internal func above.
    extern "C"
    {
        // TODO: TAKE STRCMP'S OUT!!! - Possibly replace them with hashes. Just compare hashes instead.
        // Check into std::strcmp vs. strcmp.
        // A system Enum also works - just assign the states numbers and compare it that way.
        // Put dummy functions inside each of the managers for their Startup's
        // dummy the lower level functions as needed
        // "Possibly" use "cascade" (Check )
        // FOR SAFETY: Call these functions directly.
        // Possibly, use a WAKEUP_LEVEL (might not be necessary for this.)
        /**
         * @brief Proper StartupSystem sequence, what the final produce will look like:
         *
         * This function will be called by (ESS Controller - where this function is located under) on the following conditions:
         *     1) estart from ANYWHERE!
         *     2) start command from Site_Controller
         *     3) Internal fault (ANY fault), shut down EVERYTHING! (a "fault start")
         *     4) Through the User Interface (press a button!) - Web UI
         *     ... more? (don't know yet)
         *
         * This function will consist of the following sequence (run by ESS controller)
         *     1) Send to PCS_Manager the StartupPCS's command (SIDENOTE: have PCS_Manager set [ActivePowerSetPoint && ReactivePowerSetPoint] to 0 )
         *     2) Wait for PCS_Manager to respond (get ok that all PCS's are Startup)
         *     3) Send to BMS_Manager the StartupBMS's command
         *     4) Wait for BMS_Manager to respond (get ok that all BMS's are Startup)
         *     ... add other system as needed
         *     General form:
         *         - Send StartupAsset's to their manager
         *         - Wait for response before proceeding
         *
         * NOTE: Whenever we say "wait" we need a "what-if" and a "duration - for how long?"
         * CLARIFICATION: What do we do when something we tell to shut down doesn't?
         * OPTIONS:
         *      1) Set PCS_Manager's [ActivePowerSetPoint && ReactivePowerSetPoint] to 0 - not allow to do anything!
         *      2) retry 5 times (retry "x" times, variable from config)
         *      3) issue alarms
         *      4) bypass wait and move on. These are our "what-ifs"
         * EMERGENCYACTIONS: (Do if all else fails?)
         *
         *      1) Track PCS_Manager's [Status's] - for all PCS's
         *      2) IMPORTANT: PCS_Managers are responsible for opening/closing DC Contactors, ESS Controller has no control.
         *
         * STATECOMMANDS:
         *      1) We DO NOT manually control each hardware piece, we only send it a single command to shut it down.
         *          1a) "CATL" is our main battery vendor. "Power electronics" is our main PCS vendor.
         *      2) NEED TO GET particular asset status's
         *          2a)
         *
         * SUGGESTIONS: (For discussion with Ben, Vinay, Aarabi, John, Tony, etc. tomorrow)
         *      1)
         *
         * @param vmap
         * @param amap
         * @param aname
         * @param p_fims
         * @param am
         *
         * Put each variable here and how it is used before writing a function from now on.
         *  // Statuses, etc.:
                char *
                /status/ess/SystemState
                       used to indicate system state : Init, Fault, Ready , StandBy , Run
                /status/ess/SystemStateStep/status/ess/
                       used to indicate system state step

                double
                /status/ess/CurrentSetpoint
                             the current Setpoint +ve for discharge -Ve for charge

                /status/ess/StartupTime
                            time the system took to shutdown.

                /config/ess/maxPCSStartupTime
                /config/ess/maxBMSStartupTime
                            time limits to allow for BMS and PCS system to complete shutdown.


                linkVals(*vm, vmap, amap, aname, "/status", ival, "
                //?? /status/ess/BmsStatus   status of the BmsSystem   from modbus input
                //?? /status/ess/PCsStatus   status of the PcsSystem
                //?? /status/ess/PCsStatus   status of the EmuSystem    from ??
                //?? /status/ess/PCsStatus   status of the EmmuSystem    from ??

                // other variables:
                bool
                /status/ess/Estart             estart input flag , several possible sources. TODO Should be attached directly to Startup.
                /status/ess/UiStartup        UserInterface shutdown request
                /status/ess/SMStartup        Site Manage Startup request
                /status/ess/FaultStartup     Fault Startup Request. Set by the fault system
                /status/ess/EMMUFaulted       (TODO) Indication that the EMMU system was faulted
                /status/ess/BMSFaulted        Indication that the BMS system was faulted
                /status/ess/PCSFaulted        Indication that the PCS system was faulted
                /status/ess/EMUFaulted        (TODO) Indication that the EMMU system was faulted

                NOTE we may set the next three up as integers. to keep track of multiple requests.
                /status/ess/StartupRequest      Main trigger for a shutdown start , set by any system that wants to initiate a shutdown. We'll attach an alarm to it later.
                /status/ess/StartupRequested    Indication that we have responded to the latest shutdown request.
                /status/ess/StartupCompleted    Indication that we have completed the latest Startup Request
                 bool
                /status/ess/PCSStartupForced     Indication that we forced the PCS to shutdown It did not  resond to the current request.
                /status/ess/BMSStartupForced     Indication that we forces the BMS shutdown.

                /status/ess/PCSStartup           Indication that the PCSStartup has completed
                /status/ess/BMSStartup           Indication that the BMSStartup has completed

                /status/ess/StartupSim           Request for simulation code path tester  reset as soon as SimSeen is set.
                /status/ess/StartupSimSeen       Indication that the simulation request has been seen , reset when the simulation has completed

                /status/ess/StartupCompleted     Indication that Startup has been completed.
                                                    To Be reset when another transition brigs the system out of shutdown.


                int
                /status/pcs/PCSStatusResp            Code indicating the current PCS status.  used in sim
                /status/bms/BMSStatusResp            Code indicating the current BMS status.  used n sim
                /status/pcs/PCSKeyCmd            Place to send  PCS command.  used in sim
                /status/bms/BMSKeyCmd            Place to Send  BMS command.  used n sim


                TODO maybe
                /status/pcs/BMSStatusResp

         *
         */
        int StartupSystem(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
        {

            int reload = -1;
            bool bval = false;
            char* tVal = (char*)"Init";
            double dval = 0.0;
            asset_manager* am = aV->am;
            VarMapUtils* vm = am->vm;
            assetVar Av;
            essPerf ePerf(am, aname, __func__);

            // char* tVal = (char*)"Test TimeStamp";
            // in future use tval for sending messages out - for UI and alert purposes.

            double tNow = vm->get_time_dbl();

            assetVar* essAv = amap[__func__];
            if (!essAv || (reload = essAv->getiVal()) == 0)
            {
                FPS_ERROR_PRINT(" %s >> reload %d \n", __func__, reload);

                reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
            }
            if(reload==2)
            {
                FPS_ERROR_PRINT(" %s >> reload %d \n", __func__, reload);
                essAv->setVal(3);
            }

            if (reload < 2)
            {
                //std::cout << "break\n";
                // TODO: clean these up!!!

                // reload
                linkVals(*vm, vmap, amap, aname, "/reload", reload, __func__);
                linkVals(*vm, vmap, amap, "bms", "/reload", reload, "StartupBMS");
                linkVals(*vm, vmap, amap, "pcs", "/reload", reload, "StartupPCS");

                // Statuses, etc.:
                linkVals(*vm, vmap, amap, aname, "/status", tVal, "SystemState", "SystemStateStep");
                linkVals(*vm, vmap, amap, aname, "/status", dval, "StartupTime");

                // other variables:
                linkVals(*vm, vmap, amap, aname, "/status", bval, "UiStartup", "BMSStartupCmd", "UiStartupSeen", "StartupCmd", "StartupRequest");
                linkVals(*vm, vmap, amap, aname, "/status", bval, "PCSStartup", "BMSStartup", "BMSFaulted", "PCSFaulted");
                linkVals(*vm, vmap, amap, aname, "/sched", bval, "schedStartupPCS");
                // linkVals(*vm, vmap, amap, "pcs", "/status", cval, "PCSStatusResp");  // is this needed here ??
                // linkVals(*vm, vmap, amap, "bms", "/status", ival, "BMSStatusResp");  // is this needed here ??

                if (reload < 1) // complete restart
                {
                    FPS_ERROR_PRINT(" %s >>running complete restart\n", __func__ );
                    // TODO: Clean these up!!!

                    essAv = amap[__func__];
                    amap["StartupCmd"]->setVal(false);
                    amap["StartupCmd"]->setParam("StartupRequested", false);
                    amap["StartupCmd"]->setParam("StartupCompleted", false);
                    amap["StartupCmd"]->setParam("StartupCompletedOK", false);

                    // NOTE if we do this after init we'll loose requests
                    amap["UiStartup"]->setVal(false);
                    amap["UiStartup"]->setParam("StartBMS", false);
                    amap["UiStartup"]->setParam("StartPCS", false);

                    amap["BMSStartupCmd"]->setVal(false);
                    amap["SystemState"]->setVal(tVal);
                    amap["SystemStateStep"]->setVal(tVal);

                    // BELOW: this runs reloads for asset_managers (does no processing - only for linkVals).
                    amap["StartupBMS"]->setVal(0); // full reset
                    amap["StartupPCS"]->setVal(0); // full reset

                    for (auto& ix : am->assetManMap) // should get (const?) references to pairs NOT copies.
                    {
                        // For MVP - We need at least PCS and BMS Startups. Will add more.
                        asset_manager* amc = ix.second;
                        Av.am = amc;

                        if (amc->name == "bms")
                        {
                            FPS_ERROR_PRINT("%s >> running startup BMS\n",__func__);
                            StartupBMS(vmap, amc->amap, amc->name.c_str(), p_fims, &Av);
                        }
                        if (amc->name == "pcs")
                        {
                            FPS_ERROR_PRINT("%s >> running startup PCS\n",__func__);
                            StartupPCS(vmap, amc->amap, amc->name.c_str(), p_fims, &Av);
                        }
                    }
                    FPS_ERROR_PRINT("%s >> after running setup , reloads PCS %d BMS %d \n"
                                ,__func__
                                , amap["StartupPCS"]->getiVal()
                                , amap["StartupBMS"]->getiVal()
                                );
 
                }
                reload = 2;
                essAv->setVal(reload);
            }

            //double tNow = vm->get_time_dbl();
            bool StartupRequested = amap["StartupCmd"]->getbParam("StartupRequested");
            bool UiStartup = amap["UiStartup"]->getbVal();
            bool BMSStartupCmd = amap["BMSStartupCmd"]->getbVal();
            // bool StartupCompleted = amap["StartupCmd"]->getbParam("StartupCompleted");
            // bool StartupCompletedOK = amap["StartupCmd"]->getbParam("StartupCompletedOK");
            double StartupTime = amap["StartupTime"]->getdVal();
            bool PCSStartup = amap["PCSStartup"]->getbVal();
            bool BMSStartup = amap["BMSStartup"]->getbVal();
            char* cstate = amap["SystemState"]->getcVal();
            char* csstate = amap["SystemStateStep"]->getcVal();
            double sdtime = amap["StartupTime"]->getdVal(); // seems redundant
            // char* PCSStatusResp = amap["PCSStatusResp"]->getcVal();
            // int BMSStatusResp = amap["BMSStatusResp"]->getiVal();

            assetVar* UiStartupAv = amap["UiStartup"];//->getbVal();
            assetVar* BMSStartupCmdAv = amap["BMSStartupCmd"];//->getbVal();

            // This is the start of the real code (this is our flow diagram):
            // thse inputs must toggle from false to true to trigger this..

            // the reload 0 was forcing a false reset.
            if (UiStartupAv->getbVal() && UiStartupAv->valueChangedReset())
            {
                if (1)FPS_ERROR_PRINT(" %s >>> Set UiStartup StartupCmd true\n", __func__);
                amap["StartupCmd"]->setVal(true);
            }
            if (BMSStartupCmdAv->getbVal() && BMSStartupCmdAv->valueChangedReset())
            {
                if (1)FPS_ERROR_PRINT(" %s >>> Set BMSStartupCmd true\n", __func__);
                amap["StartupCmd"]->setVal(true);
            }

            bool StartupCmd = amap["StartupCmd"]->getbVal();

            if (StartupCmd) // TODO: Make these one-shots
            {
                if (1) FPS_ERROR_PRINT("%s >> UIStartup: %s, BMSStartupCmd: %s\n"
                    , __func__
                    , UiStartup ? "true" : "false"
                    , BMSStartupCmd ? "true" : "false"
                );
                if (!StartupRequested) // we are having a shutdown requested here when you set StartupCompleted to false.
                {
                    if (1) FPS_ERROR_PRINT("%s >> Func Starting Startup at time: %f elap %f \n", __func__, tNow, (tNow - StartupTime));
                    amap["StartupCmd"]->setVal(false);

                    const char* cVal = "Startup";
                    amap["SystemState"]->setVal(cVal);
                    cVal = "Waiting BMS Startup";
                    amap["SystemStateStep"]->setVal(cVal);

                    amap["StartupCmd"]->setParam("StartupRequested", true);

                    // Partial reset
                    amap["StartupBMS"]->setVal(1);
                    amap["StartupPCS"]->setVal(1);
                    for (auto& ix : am->assetManMap) // should get (const?) references to pairs NOT copies.
                    {
                        // For MVP - We need at least PCS and BMS Startups. Will add more.
                        asset_manager* amc = ix.second;
                        Av.am = amc;

                        if (amc->name == "bms")
                        {
                            FPS_ERROR_PRINT("%s >> Resetting startup BMS\n",__func__);
                            StartupBMS(vmap, amc->amap, amc->name.c_str(), p_fims, &Av);
                        }
                        if (amc->name == "pcs")
                        {
                            FPS_ERROR_PRINT("%s >> Resetting startup PCS\n",__func__);
                            StartupPCS(vmap, amc->amap, amc->name.c_str(), p_fims, &Av);
                        }
                    }

                    // Start of the Startup:
                    tNow = am->vm->get_time_dbl();
                    amap["StartupTime"]->setVal(tNow);
                    if (1) FPS_ERROR_PRINT("%s >> Setting StartupTime to %f\n", __func__, tNow);

                    // vm->sendAlarm(vmap, "/status/ess:StartupRequested", "/components/ess:alarms", "Startup", "Startup Started", 1);
                }
            }
            else
            {
                if (0) FPS_ERROR_PRINT("%s >> No startup cmd  at time: %f\n", __func__, tNow);

            }

            StartupRequested = amap["StartupCmd"]->getbParam("StartupRequested");
            if (!StartupRequested)
            {
                if (0) FPS_ERROR_PRINT("%s >> Func Quitting Startup at  time: %f\n", __func__, tNow);
                return 0;
            }

            // After this we start processing the Startup request:
            if (amap["SystemState"]->valueChangedReset() || amap["SystemStateStep"]->valueChangedReset())
            {
                cstate = amap["SystemState"]->getcVal();
                csstate = amap["SystemStateStep"]->getcVal();

                if (1) FPS_ERROR_PRINT("%s >> Startup  state [%s] step [%s]  time elapsed : %f\n", __func__, cstate, csstate, tNow - sdtime);
            }

            PCSStartup = amap["PCSStartup"]->getbVal();
            BMSStartup = amap["BMSStartup"]->getbVal();

            // assets are in assetMap sub managers are in assetManMap
            for (auto& ix : am->assetManMap) // should get (const?) references to pairs NOT copies.
            {
                asset_manager* amc = ix.second;
                if (0)FPS_ERROR_PRINT("%s >> manager_name: [%s] PCSStartup [%s] BMSStartup [%s]\n"
                    , __func__
                    , amc->name.c_str()
                    , PCSStartup ? "true" : "false"
                    , BMSStartup ? "true" : "false"
                );

                if (amc->name == "bms") // swapped to do bms startup before pcs startup. Keep this?
                {
                    if (0)FPS_ERROR_PRINT("%s >> manager_name: [%s] BMSStartup [%s]\n"
                        , __func__
                        , amc->name.c_str()
                        , BMSStartup ? "true" : "false"
                    );

                    if (!BMSStartup) // This logic was added to guarantee no cascade is done without PCS being shutdown first. Might need to be changed.
                    {
                        if (0) FPS_ERROR_PRINT("%s >>>> cascading function to >>>>> Manager [%s] at time: %f\n ", __func__, amc->name.c_str(), tNow);
                        StartupBMS(vmap, amc->amap, amc->name.c_str(), p_fims, aV);
                    }
                    else
                    {
                        const char* cVal = "Waiting PCS Startup";
                        amap["SystemStateStep"]->setVal(cVal);
                    }
                }
                if (amc->name == "pcs")
                {
                    if (0)FPS_ERROR_PRINT("%s>> manager_name: [%s] PCSStartup [%s] BMSStartup [%s]\n"
                        , __func__
                        , amc->name.c_str()
                        , PCSStartup ? "true" : "false"
                        , BMSStartup ? "true" : "false"
                    );

                    if (!PCSStartup && BMSStartup)
                    {
                        if (0) FPS_ERROR_PRINT("%s >>>> cascading function to >>>>> Manager [%s] at time: %f\n ", __func__, amc->name.c_str(), tNow);
                        if (UiStartupAv->gotParam("StartPCS") && UiStartupAv->getbParam("StartPCS"))
                        {
                            StartupPCS(vmap, amc->amap, amc->name.c_str(), p_fims, aV);
                        }
                    }
                }
            }

            if (amap["PCSStartup"]->getbVal() && amap["BMSStartup"]->getbVal()) // then it goes here and does this.
            {
                const char* cVal = "Ready";
                amap["SystemState"]->setVal(cVal);
                cVal = "Waiting for Start";
                amap["SystemStateStep"]->setVal(cVal);

                amap["StartupCmd"]->setParam("StartupRequested", false);
                if (1) FPS_ERROR_PRINT("%s >> Startup Completed, elapsed time: %f \n", __func__, (tNow - sdtime));
                vm->sendAlarm(vmap, "/status/ess:StartupRequested", "/components/ess:alarms", "Startup", "Startup Completed", 1);

                //assetVar* temp_av = amap["StartupRequested"];
                assetVar* temp_av = amap["StartupCmd"];
                if (temp_av) temp_av->sendEvent("ESS", am->p_fims, Severity::Info, "startup completed at time %2.3f"
                    , vm->get_time_dbl()
                    );
                
                amap["schedStartupPCS"]->setParam("endTime", 1);
                amap["schedStartupPCS"]->setVal("schedStartupPCS");
            }
            else if (amap["BMSStartup"]->getbVal() && UiStartupAv->gotParam("StartPCS") && !UiStartupAv->getbParam("StartPCS"))
            {
                const char* cVal = "Ready";
                amap["SystemState"]->setVal(cVal);
                cVal = "Waiting for Start";
                amap["SystemStateStep"]->setVal(cVal);

                amap["StartupCmd"]->setParam("StartupRequested", false);
                if (1) FPS_ERROR_PRINT("%s >> Startup Completed, elapsed time: %f \n", __func__, (tNow - sdtime));
                vm->sendAlarm(vmap, "/status/ess:StartupRequested", "/components/ess:alarms", "Startup", "Startup Completed", 1);

                //assetVar* temp_av = amap["StartupRequested"];
                assetVar* temp_av = amap["StartupCmd"];
                if (temp_av) temp_av->sendEvent("ESS", am->p_fims, Severity::Info, "UI startup completed at time %2.3f"
                    , vm->get_time_dbl()
                    );
                
                amap["schedStartupPCS"]->setParam("endTime", 1);
                amap["schedStartupPCS"]->setVal("schedStartupPCS");
            }
            // NOTE if we are faulted we cannot auto restart
            // do we need 
            //else if ((amap["BMSStartup"]->getbParam("Fault")) &&   !(amap["StartupCmd"]->getbParam("StartupCompleted"))
            else if (amap["BMSStartup"]->getbParam("Fault"))
            {
                if (1) FPS_ERROR_PRINT("%s >>>> BMS Startup failed, quitting Startup at time: %f\n ", __func__, tNow);
                amap["StartupCmd"]->setParam("StartupRequested", false);
                amap["BMSFaulted"]->setVal(true);

                const char* cVal = "BMS Startup Failed";
                amap["SystemStateStep"]->setVal(cVal);
                
                amap["schedStartupPCS"]->setParam("endTime", 1);
                amap["schedStartupPCS"]->setVal("schedStartupPCS");
            }
            // do we need 
            //else if ((amap["PCSStartup"]->getbParam("Fault")) &&   !(amap["StartupCmd"]->getbParam("StartupCompleted"))
            else if (amap["PCSStartup"]->getbParam("Fault"))
            {
                if (1) FPS_ERROR_PRINT("%s >>>> PCS Startup failed, quitting Startup at time: %f\n ", __func__, tNow);
                amap["StartupCmd"]->setParam("StartupRequested", false);
                amap["PCSFaulted"]->setVal(true);

                const char* cVal = "PCS Startup Failed";
                amap["SystemStateStep"]->setVal(cVal);

                amap["schedStartupPCS"]->setParam("endTime", 1);
                amap["schedStartupPCS"]->setVal("schedStartupPCS");
            }

            return 0;
        }

        // StartupPCS
        // * This function will consist of the following sequence (run by ESS controller)
        //  *     1) Send to PCS_Manager the StartupPCS's command (SIDENOTE: have PCS_Manager set [ActivePowerSetPoint && ReactivePowerSetPoint] to 0 )
        //  *     2) Wait for PCS_Manager to respond (get ok that all PCS's are Startup)
        //  *     3) Send to BMS_Manager the StartupBMS's command
        //  *     4) Wait for BMS_Manager to respond (get ok that all BMS's are Startup)
        //  *     ... add other system as needed
        //  *     General form:
        //  *         - Send StartupAsset's to their manager
        //  *         - Wait for response before proceeding
        // TODO: Similar to BMS, we might only need a single register, just set global active/reactivepower setpoints to zero.
         // Statuses, etc.:
        /*


                // check this linkVals(*vm, vmap, amap, aname, "/status", ival, "PCSStatusResp", "PCSKeystart");   // returned status

                // other variables:
                // todo: link Estart to aname = "ess" not itself.
                // Check this linkVals(*vm, vmap, amap, "ess", "/status", bval, "PCSFaulted", "PCSFaultSeen", "StartupRequest", "StartupRequested");
                ///linkVals(*vm, vmap, amap, aname, "/status", bval, "PCSKeystartSent");
                //linkVals(*vm, vmap, amap, aname, "/config", ival, "PCSKeystartCmd");
                //linkVals(*vm, vmap, amap, aname, "/config", ival, "PCSFaultStatus");

                // Estart:
                linkVals(*vm, vmap, amap, "ess", "/status", bval, "Estart");
                char *
                /status/ess/SystemState
                       used to indicate system state : Init, Fault, Ready , StandBy , Run
                /status/ess/SystemStateStep
                       used to indicate system state step
                /status/pcs/PCSSystemState
                       used to indicate system state : Init, Fault, Ready , StandBy , Run
                /status/pcs/PCSSystemStateStep
                       used to indicate system state step

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
                // ?? /status/ess/BmsStatus   status of the BmsSystem   from modbus input
                // ?? /status/ess/PCsStatus   status of the PcsSystem
                // ?? /status/ess/PCsStatus   status of the EmuSystem    from ??
                // ?? /status/ess/PCsStatus   status of the EmmuSystem    from ??

                // other variables:
                bool

                /status/ess/Estart             estart input flag , several possible sources. TODO Should be attached directly to Startup.

                /status/ess/PCSStartup         Check this
                /status/ess/PCSStatus           and this

                /status/ess/PCSFaulted        Indication that the PCS system was faulted
                /status/pcs/PCSFaultSeen      Indication that the PCS fault has been seen

                NOTE we may set the next three up as integers. to keep track of multiple requests.
                /status/ess/StartupRequest      Main trigger for a shutdown start , set by any system that wants to initiate a shutdown. We'll attach an alarm to it later.
                /status/ess/StartupRequested    Indication that we have responded to the latest shutdown request.
                /status/ess/StartupCompleted    Indication that we have completed the latest Startup Request
                 bool
                /status/ess/PCSStartupForced     Indication that we forced the PCS to shutdown It did not  resond to the current request.
                /status/ess/BMSStartupForced     Indication that we forces the BMS shutdown.

                /status/ess/PCSStartup           Indication that the PCSStartup has completed
                /status/ess/BMSStartup           Indication that the BMSStartup has completed

                /status/ess/StartupSim           Request for simulation code path tester  reset as soon as SimSeen is set.
                /status/ess/StartupSimSeen       Indication that the simulation request has been seen , reset when the simulation has completed

                /status/ess/StartupCompleted     Indication that Startup has been completed.
                                                    To Be reset when another transition brigs the system out of shutdown.

                double
                /status/pcs/PCSKeyCmdTime            Place to send  PCS command.  from ModBus Map Jimmy

                int
                /status/ess/PCSstartSent           command sent to  start to the PCS
                /status/pcs/PCSStatusResp         Code indicating the current PCS status.  from ModBus Map Jimmy
                /status/bms/BMSStatusResp         Code indicating the current BMS status.  from ModBudMap Jimmy
                /status/pcs/PCSKeyCmd            Place to send  PCS command.  from ModBus Map Jimmy
                /status/pcs/PCSKeyCmdTries       NUmber of attempts to send PCS Command



        */

        int StartupPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
        {
            int reload = 0;
            bool bval = false;
            int ival = 0;
            double dval = 0.0;
            char* cval = (char*)"";
            char* tValInit = (char*)"Init";
            asset_manager * am = aV->am;
            //if (am == nullptr) 
            if (0) FPS_ERROR_PRINT("%s >> Running  for aV [%s:%s] am %p, amap %p aname [%s] \n"
                , __func__
                , aV->comp.c_str()
                , aV->name.c_str()
                , aV->am
                , &amap
                , aname
                );
            if (0) FPS_ERROR_PRINT("%s >>>>>>>>>>>PCS Startup check amap %p system state [%s] %p  >>pcssystem state [%s] %p \n"
                , __func__
                , &amap
                , amap["SystemState"]?amap["SystemState"]->getcVal():"No System State"
                , amap["SystemState"]
                , amap["PCSSystemState"]?amap["PCSSystemState"]->getcVal():"No PCS System State"
                , amap["PCSSystemState"]
                );
            VarMapUtils* vm = am->vm;
            double tNow = vm->get_time_dbl();
            essPerf ePerf(am, aname, __func__);

            assetVar* pcsAv = amap[__func__]; // Need to figure out the resolution here!
            if (!pcsAv || (reload = pcsAv->getiVal()) == 0)
            {
                if (0) FPS_ERROR_PRINT("%s >> Running reload %d \n", __func__, reload);
                reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
            }

            if (reload == 0)
            {
                // reload
                linkVals(*vm, vmap, amap, aname, "/reload", reload, __func__);
                pcsAv = amap[__func__]; // Need to figure out the resolution here!
                linkVals(*vm, vmap, amap, aname, "/reload", reload, "pcsRunKeyCmd");

                // Statuses, etc.:
                // linkVals(*vm, vmap, amap, "ess", "/status", tValInit, "SystemStateStep");
                linkVals(*vm, vmap, amap, "ess", "/status", bval, "PCSStartup");
                linkVals(*vm, vmap, amap, aname, "/sched", cval, "schedStartupPCS");

                linkVals(*vm, vmap, amap, aname, "/status", tValInit, "SystemState", "PCSSystemState", "PCSSystemStateStep");
                linkVals(*vm, vmap, amap, aname, "/status", cval, "PCSStatusResp");
                linkVals(*vm, vmap, amap, aname, "/status", dval, "PCSStartupTime");
                linkVals(*vm, vmap, amap, aname, "/status", ival, "KeyCmd", "PCSStartKeyCmd");

                linkVals(*vm, vmap, amap, aname, "/config", dval, "maxStartupTime", "maxKeyCmdOnTime", "maxKeyCmdTime");
                linkVals(*vm, vmap, amap, aname, "/config", ival, "maxKeyCmdTries");
                if (1) FPS_ERROR_PRINT("%s >>>>>>>>>>>PCS Startup reload 0 time %2.3f\n", __func__, tNow);
            }
            if(reload < 2)
            {
                amap["PCSSystemStateStep"]->setVal(tValInit);
                amap["PCSSystemState"]->setVal(tValInit);
                amap["PCSStartupTime"]->setVal(tNow);

                amap["PCSStartup"]->setVal(false);
                // BUT do we wamt to reset the fault here ??    yes on a complete reset but not after that time
                amap["PCSStartup"]->setParam("Fault", false);

                amap["KeyCmd"]->setParam("maxKeyCmdOnTime", amap["maxKeyCmdOnTime"]->getdVal());
                amap["KeyCmd"]->setParam("maxKeyCmdTime", amap["maxKeyCmdTime"]->getdVal());
                amap["KeyCmd"]->setParam("maxKeyCmdTries", amap["maxKeyCmdTries"]->getiVal());

                if (1) FPS_ERROR_PRINT("%s >>>>>>>>>>>PCS Startup reload 1 amap %p system state [%s] %p  >>pcssystem state [%s] %p at time: %f \n"
                    , __func__
                    , &amap
                    , amap["SystemState"]->getcVal()
                    , amap["SystemState"]
                    , amap["PCSSystemState"]->getcVal()
                    , amap["PCSSystemState"]
                    , tNow
                    );
                // Send Event - info
                assetVar* temp_av = amap["PCSStartup"];
                if (temp_av) temp_av->sendEvent("PCS", am->p_fims, Severity::Info, "PCS startup reload complete at time %2.3f"
                    , vm->get_time_dbl()
                    );

                pcsAv->setVal(2);
                // catch the setup request from the manager
                
                // amap["schedStartupPCS"]->setParam("endTime", .000000001);
                // assetUri my("/sched/ess:schedStartupPCS");
                // char* temp = (char*) "schedStartupPCS"; 
                // vm->setVal(vmap, my, temp);
                
                // amap["schedStartupPCS"]->setVal("schedStartupPCS");
                // return 0;
            }
            // if (1) FPS_ERROR_PRINT("%s >>>>>>>>>>>>>>>>PCS Startup system state [%s]  at time: %f \n",0, __func__, amap["SystemStatsxe"]->getcVal(), tNow);

            char* SystemState = amap["SystemState"]->getcVal();
            char* PCSSystemStateStep = amap["PCSSystemStateStep"]->getcVal();
            if (0) FPS_ERROR_PRINT("%s >> Running reload %d state [%s] step [%s]\n", __func__, reload, SystemState, PCSSystemStateStep );

            char* PCSStatusResp = amap["PCSStatusResp"]->getcVal();
            if (0) FPS_ERROR_PRINT("%s >> Running reload %d StatusResp [%s]\n", __func__, reload, PCSStatusResp);
            if (0) FPS_ERROR_PRINT("aname %s comp %s name %s\n", aname, amap["PCSStatusResp"]->comp.c_str(), amap["PCSStatusResp"]->name.c_str());

            if (!PCSStatusResp)
            {
                amap["PCSStartup"]->setParam("Fault", true);
                PCSSystemStateStep = (char*)"PCS Startup Communication Failed";
                amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
                if (1) FPS_ERROR_PRINT("%s >> No communication with PCS at time: %f \n", __func__, tNow);
                // Send Event - fault
                assetVar* temp_av = amap["PCSStatusResp"];
                if (temp_av) temp_av->sendEvent("PCS", am->p_fims, Severity::Fault, "PCS startup comms failed at time %2.3f"
                    , vm->get_time_dbl()
                    );
                amap["schedStartupPCS"]->setParam("endTime", 1);
                amap["schedStartupPCS"]->setVal("schedStartupPCS");
                pcsAv->setVal(1);
                return 0;
            }

            // SystemState holds state from PCS decoded into either Ready, Starting, Ready and Shutdown
            if (amap["SystemState"]->valueChangedReset() || strcmp(PCSSystemStateStep, "Init") == 0)
            {
                amap["pcsRunKeyCmd"]->setVal(0);           // Reset key cmd function
                RunKeyCmd(vmap, amap, aname, nullptr, aV);
                amap["PCSStartupTime"]->setVal(tNow);
                if (1) FPS_ERROR_PRINT("%s >>> State step changed from %s to %s time %2.3f \n", __func__, PCSSystemStateStep , SystemState, tNow);
                // Send Event - info
                assetVar* temp_av = amap["PCSStartupTime"];
                if (temp_av) temp_av->sendEvent("PCS", am->p_fims, Severity::Info, "PCS state changed to [%s] at time %2.3f"
                    , PCSSystemStateStep
                    , vm->get_time_dbl()
                    );
                amap["PCSSystemStateStep"]->setVal(SystemState);
            }

            if (strcmp(SystemState, "Starting") == 0 && ((tNow - amap["PCSStartupTime"]->getdVal()) > amap["maxStartupTime"]->getdVal()))
            {
                amap["PCSStartup"]->setParam("Fault", true);
                PCSSystemStateStep = (char*)"PCS Startup Failed";
                amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
                if (1) FPS_ERROR_PRINT("%s >> PCS stuck in hold state %s at time: %f \n", __func__, PCSStatusResp, tNow);
                // Send Event - fault
                assetVar* temp_av = amap["PCSStartup"];
                if (temp_av) temp_av->sendEvent("PCS", am->p_fims, Severity::Fault, "PCS stuck in hold state [%s] at time %2.3f"
                    , PCSStatusResp
                    , vm->get_time_dbl()
                    );
                amap["schedStartupPCS"]->setParam("endTime", 1);
                amap["schedStartupPCS"]->setVal("schedStartupPCS");
                pcsAv->setVal(1);
            }
            else if (strcmp(SystemState, "Running") == 0)
            {
                amap["PCSStartup"]->setVal(true);
                PCSSystemStateStep = (char*)"PCS Startup Complete";
                amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
                if (1) FPS_ERROR_PRINT("%s >> All PCS's succesfully Startup at time: %f \n", __func__, tNow);
                // Send Event - info
                assetVar* temp_av = amap["PCSStartup"];
                if (temp_av) temp_av->sendEvent("PCS", am->p_fims, Severity::Info, "PCS startup completed at time %2.3f"
                    , vm->get_time_dbl()
                    );
                amap["schedStartupPCS"]->setParam("endTime", 1);
                amap["schedStartupPCS"]->setVal("schedStartupPCS");
                pcsAv->setVal(1);
            }
            // DO we need to only detect this once
            else if (strcmp(SystemState, "Shutdown") == 0 || strcmp(SystemState, "Fault") == 0)
            {
                amap["PCSStartup"]->setParam("Fault", true);
                PCSSystemStateStep = (char*)"PCS Startup Failed";
                amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
                if (1) FPS_ERROR_PRINT("%s >> PCS failed in state %s at time: %f \n", __func__, PCSStatusResp, tNow);
                // Send Event - fault
                assetVar* temp_av = amap["PCSStartup"];
                if (temp_av) temp_av->sendEvent("PCS", am->p_fims, Severity::Fault, "PCS startup failed at time %2.3f"
                    , vm->get_time_dbl()
                    );
                amap["schedStartupPCS"]->setParam("endTime", 1);
                amap["schedStartupPCS"]->setVal("schedStartupPCS");
                pcsAv->setVal(1);
            }
            else if (amap["KeyCmd"]->getbParam("KeyCmdDone"))
            {
                amap["PCSStartup"]->setParam("Fault", true);
                PCSSystemStateStep = (char*)"PCS Startup Failed";
                amap["PCSSystemStateStep"]->setVal(PCSSystemStateStep);
                if (1) FPS_ERROR_PRINT("%s >> No response from PCS at time: %f \n", __func__, tNow);
                // Send Event - fault
                assetVar* temp_av = amap["PCSStartup"];
                if (temp_av) temp_av->sendEvent("PCS", am->p_fims, Severity::Fault, "PCS response failed at time %2.3f"
                    , vm->get_time_dbl()
                    );
                amap["schedStartupPCS"]->setParam("endTime", 1);
                amap["schedStartupPCS"]->setVal("schedStartupPCS");
                pcsAv->setVal(1);
            }
            else if (strcmp(SystemState, "Ready") == 0 || strcmp(SystemState, "Off") == 0)
            {
                RunKeyCmd(vmap, amap, aname, nullptr, aV);
            }
            if (amap["PCSStartup"]->getbVal() && amap["KeyCmd"])
            {
                amap["KeyCmd"]->setVal(false);
            }

            if (amap["KeyCmd"]->valueChangedReset())
            {
                varsmap* vlist = vm->createVlist();
                if (amap["KeyCmd"]->getbVal())
                {
                    amap["PCSStartKeyCmd"]->setVal(1); // todo: config
                    vm->addVlist(vlist, amap["PCSStartKeyCmd"]);
                }
                else
                {
                    amap["PCSStartKeyCmd"]->setVal(0); // todo: config
                    vm->addVlist(vlist, amap["PCSStartKeyCmd"]);
                }
                vm->sendVlist(p_fims, "set", vlist);
                vm->clearVlist(vlist);
            }
            
            return 0;
            
        }


        int StartupBMS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
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
                // linkVals(*vm, vmap, amap, "ess", "/status", tValInit, "SystemState");
                linkVals(*vm, vmap, amap, "ess", "/status", bval, "BMSStartup");
                linkVals(*vm, vmap, amap, aname, "/sched", cval, "schedStartupBMS");

                linkVals(*vm, vmap, amap, aname, "/status", tValInit, "BMSSystemState", "BMSSystemStateStep");
                linkVals(*vm, vmap, amap, aname, "/status", cval, "BMSPowerOn", "BMSStatus");
                linkVals(*vm, vmap, amap, aname, "/status", dval, "BMSStartupTime");
                linkVals(*vm, vmap, amap, aname, "/status", ival, "KeyCmd", "BMSKeyCmd");

                linkVals(*vm, vmap, amap, aname, "/config", dval, "maxStartupTime", "maxKeyCmdOnTime", "maxKeyCmdTime");
                linkVals(*vm, vmap, amap, aname, "/config", ival, "maxKeyCmdTries");
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

                if (1) FPS_ERROR_PRINT("%s >>>>>>>>>>>>>>>>BMS Startup system state [%s]  at time: %f \n", __func__, amap["BMSSystemState"]->getcVal(), tNow);
                bmsAv->setVal(2);

                return 0;
            }

            char* BMSSystemState = amap["BMSSystemState"]->getcVal();
            char* BMSSystemStateStep = amap["BMSSystemStateStep"]->getcVal();
            if (0) FPS_ERROR_PRINT("%s >> Running reload %d state [%s] step [%s]\n", __func__, reload, BMSSystemState, BMSSystemStateStep );

            if (amap["BMSSystemState"]->valueChangedReset() || amap["BMSSystemStateStep"]->valueChangedReset())
            {
                if (0) FPS_ERROR_PRINT("%s >>  BMS State [%s] Step [%s] time: %f \n", __func__, BMSSystemState, BMSSystemStateStep, tNow);
            }
            char* BMSStatus = amap["BMSStatus"]->getcVal();
            char* BMSPowerOn = amap["BMSPowerOn"]->getcVal();

            if (!BMSStatus || !BMSPowerOn)
            {
                amap["BMSStartup"]->setParam("Fault", true);
                BMSSystemStateStep = (char*)"BMS Startup Communication Failed";
                amap["BMSSystemStateStep"]->setVal(BMSSystemStateStep);
                if (1) FPS_ERROR_PRINT("%s >> No communication with BMS at time: %f \n", __func__, tNow);
                return 0;
            }
            if (0) FPS_ERROR_PRINT("%s >> status [%s] PowerOn [%s]\n", __func__, BMSStatus, BMSPowerOn);

            if (amap["BMSStatus"]->valueChangedReset() || !strcmp(BMSSystemStateStep, "Init"))
            {
                if (/*!strcmp(BMSStatus, "Warning") ||*/ !strcmp(BMSStatus, "Fault"))
                {
                    BMSSystemStateStep = (char*)"Startup Failed";
                }
            }

            if (0) FPS_ERROR_PRINT("%s >>> BMSPowerOn [%s] changed [%s] BMSSystemStateStep %s at time %2.3f \n",
                 __func__
                 , amap["BMSPowerOn"]->getcVal()
                 , amap["BMSPowerOn"]->valueChanged()?"true":"false"
                 , BMSSystemStateStep
                 , tNow
                 );
            if ((amap["BMSPowerOn"]->valueChangedReset() || !strcmp(BMSSystemStateStep, "Init")) && strcmp(BMSSystemStateStep, "Startup Failed"))
            {
                if (!strcmp(BMSPowerOn, "On Fault") || !strcmp(BMSPowerOn, "Off Fault"))
                {
                    BMSSystemStateStep = (char*)"Startup Failed";
                }
                else if (!strcmp(BMSPowerOn, "Off Ready"))
                {
                    BMSSystemStateStep = (char*)"Ready";
                    amap["bmsRunKeyCmd"]->setVal(0);           // Reset key cmd function
                    RunKeyCmd(vmap, amap, aname, nullptr, aV);
                }
                else if (!strcmp(BMSPowerOn, "On Ready"))
                {
                    BMSSystemStateStep = (char*)"Running";
                }
                amap["BMSSystemStateStep"]->setVal(BMSSystemStateStep);
                FPS_ERROR_PRINT("%s >>> State step changed to %s at time %2.3f \n", __func__, BMSSystemStateStep, tNow);
            }

            if (strcmp(BMSSystemStateStep, "Running") == 0)
            {
                amap["BMSStartup"]->setVal(true);
                BMSSystemStateStep = (char*)"BMS Startup Complete";
                amap["BMSSystemStateStep"]->setVal(BMSSystemStateStep);
                if (1) FPS_ERROR_PRINT("%s >> All BMS's succesfully Startup at time: %f \n", __func__, tNow);
                // Send Event - info
                auto temp_av = amap["BMSStartup"];
                if (temp_av) temp_av->sendEvent("BMS", am->p_fims, Severity::Info, "All BMS succesfully started up at time %2.3f"
                        , vm->get_time_dbl()
                        );
                amap["schedStartupBMS"]->setParam("endTime", 1);
                amap["schedStartupBMS"]->setVal("schedStartupBMS");
                bmsAv->setVal(1);
            }
            else if ((tNow - amap["BMSStartupTime"]->getdVal()) > amap["maxStartupTime"]->getdVal())
            {
                amap["BMSStartup"]->setParam("Fault", true);
                BMSSystemStateStep = (char*)"BMS Startup Failed";
                amap["BMSSystemStateStep"]->setVal(BMSSystemStateStep);
                if (1) FPS_ERROR_PRINT("%s >> BMS Startup timed out in hold state %s at time: %f \n", __func__, BMSPowerOn, tNow);
                // Send Event - fault
                auto temp_av = amap["BMSStartup"];
                if (temp_av) temp_av->sendEvent("BMS", am->p_fims, Severity::Fault, "BMS startup timed out in state [%s] at time %2.3f"
                        , BMSPowerOn
                        , vm->get_time_dbl()
                        );
                amap["schedStartupBMS"]->setParam("endTime", 1);
                amap["schedStartupBMS"]->setVal("schedStartupBMS");
                bmsAv->setVal(1);
            }
            else if (amap["KeyCmd"]->getbParam("KeyCmdDone"))
            {
                amap["BMSStartup"]->setParam("Fault", true);
                BMSSystemStateStep = (char*)"BMS Startup Failed";
                amap["BMSSystemStateStep"]->setVal(BMSSystemStateStep);
                if (1) FPS_ERROR_PRINT("%s >> No response from BMS at time: %f \n", __func__, tNow);
                // Send Event - fault
                auto temp_av = amap["BMSStartup"];
                if (temp_av) temp_av->sendEvent("BMS", am->p_fims, Severity::Fault, "BMS not responding at time %2.3f"
                        , vm->get_time_dbl()
                        );
                amap["schedStartupBMS"]->setParam("endTime", 1);
                amap["schedStartupBMS"]->setVal("schedStartupBMS");
                bmsAv->setVal(1);
            }
            else if (strcmp(BMSSystemStateStep, "Startup Failed") == 0)
            {
                amap["BMSStartup"]->setParam("Fault", true);
                BMSSystemStateStep = (char*)"BMS Startup Failed";
                amap["BMSSystemStateStep"]->setVal(BMSSystemStateStep);
                if (1) FPS_ERROR_PRINT("%s >> BMS in failed state at time: %f \n", __func__, tNow);
                // Send Event - fault
                auto temp_av = amap["BMSStartup"];
                if (temp_av) temp_av->sendEvent("BMS", am->p_fims, Severity::Fault, "BMS in failed state at time %2.3f"
                        , vm->get_time_dbl()
                        );
                amap["schedStartupBMS"]->setParam("endTime", 1);
                amap["schedStartupBMS"]->setVal("schedStartupBMS");
                bmsAv->setVal(1);
            }
            else if (strcmp(BMSSystemStateStep, "Ready") == 0)
            {
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
                    amap["BMSKeyCmd"]->setVal(2); // todo: config
                    vm->addVlist(vlist, amap["BMSKeyCmd"]);
                }
                else
                {
                    amap["BMSKeyCmd"]->setVal(1); // todo: config
                    vm->addVlist(vlist, amap["BMSKeyCmd"]);
                }
                vm->sendVlist(p_fims, "set", vlist);
                vm->clearVlist(vlist);
            }
            
            return 0;
        }

        // DO NOT need anything below this comment:

        // NOTE: Needs aname to be a particular bms_number - Might be enum instead.
        // IMPORTANT: This is DEPRECATED, we only control a single register to tell all of the BMS's to Startup.
        int StartupBMSasset(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
        {
            //Timer StartupBMSTimer("StartupBMSTimer", true);

            int reload = 0;
            bool bval = false;
            int ival = 0;
            char* tValInit = (char*)"Init";
            char* tValRun = (char*)"Run";
            //char *tValPowerOffReady = (char*)"Power off Ready";
            //char *tValPowerOnReady = (char*)"Power on Ready";
            //char *tValPowerOffFault = (char*)"Power off Fault";
            //char *tValPowerOnFault = (char*)"Power on Fault";
            double dval = 0.0;
            asset_manager* am = aV->am;
            VarMapUtils* vm = am->vm;

            double tNow = vm->get_time_dbl();
            essPerf ePerf(am, aname, __func__);

            if (0) FPS_ERROR_PRINT("%s >> Startup BMSAsset called for (BMS: %s) at time: %f \n", __func__, aname, tNow);


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
                //linkVals(*vm, vmap, amap, "ess", "/config", dval, /*"ActivePowerSetpoint", "ReactivePowerSetpoint",*/ "StartupTime");
                // linkVals(*vm, vmap, amap, "ess", "/config", dval, "PCSKeyActivePowerSetpoint", "PCSKeyReactivePowerSetpoint");
                // linkVals(*vm, vmap, amap, "ess", "/status", dval, "PCSActivePower", "PCSReactivePower");   // responses
                linkVals(*vm, vmap, amap, "ess", "/config", dval, "maxBMSStartupTime");
                //linkVals(*vm, vmap, amap, "ess", "/config", dval, "PowerSetpointDeadband");
                linkVals(*vm, vmap, amap, aname, "/status", tValInit, "BMSSystemState", "BMSSystemStateStep");
                linkVals(*vm, vmap, amap, aname, "/status", dval, "BMSStartupTime", "BMSStartupStart");
                linkVals(*vm, vmap, amap, "ess", "/config", dval, "maxBMSStartupTime");
                linkVals(*vm, vmap, amap, "bms", "/status", ival, "BMSnumFaults", "BMSnumPresent");

                linkVals(*vm, vmap, amap, aname, "/status", tValInit, "BMSStatus");//"Power off Ready"
                linkVals(*vm, vmap, amap, aname, "/status", ival, "BMSKeyCmd", "BMSKeyStatus");
                linkVals(*vm, vmap, amap, aname, "/status", bval, "BMSFaulted");


                // other variables:
                //amap["EMMUFaulted"] = vm->setLinkVal(vmap, aname, "/status", "EMMUFaulted", bval);
                //amap["StartupRequest"] = vm->setLinkVal(vmap, aname, "/status", "StartupRequest", bval);
                //amap["StartupRequested"] = vm->setLinkVal(vmap, aname, "/status", "StartupRequested", bval);

                if (reload < 1) // complete restart 
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

            // want to see: timer for regular Startup vs. Estart Startup. Use tNow (differences) in FPS_ERROR_PRINTs.

            // get the reference to the variable 
            //assetVar* StartupRequestedAV = amap["StartupRequested"];
            //assetVar* PCSkeystartCmdAV = amap["PCSKeystartCmd"];
            //assetVar* PCSkeyCmdAV = amap["PCSKeyCmd"];


            // storing asset states:
            //bool EMMUFaulted = amap["EMMUFaulted"]->getbVal();
            // bool Estart = amap["Estart"]->getbVal();

            //bool BMSFaulted = amap["BMSFaulted"]->getbVal();
            bool BMSStartup = amap["BMSStartup"]->getbVal();
            double BMSStartupTime = amap["BMSStartupTime"]->getdVal();
            double maxBMSStartupTime = amap["maxBMSStartupTime"]->getdVal();
            char* BMSStatus = amap["BMSStatus"]->getcVal();

            //int PowerOffStatus = 3;  // todo config
            int PowerOnCmd = 3;  // todo config

            //TODO: Is this logic complete below?

            if (BMSStartup)
            {
                amap["BMSnumPresent"]->subVal(1);
                return 0;
            }
            // if (BMSFaulted || Estart)
            // {
            //     amap["BMSnumFaults"]->addVal(1);
            //     amap["BMSStartup"]->setVal(true);
            //     amap["BMSnumPresent"]->subVal(1); // going to be used elsewhere?
            //     // send activePower reactivepower to 0
            //     amap["BMSKeyCmd"]->setVal(PowerOffCmd);
            //     return 0;
            // }

            // PowerOffReady 
            //char* tOnReady = (char *)"Power on Ready";
            //char* tOffFault = (char *)"Power on Fault";

            //char* tInitStat = (char*)"Initial status";
            //char* tNormStat = (char*)"Normal status";

            //char* tFullChargeStat = (char *)"Full Charge status";
            //char* tFullDischargeStat = (char *)"Full Discharge status";

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
            //     return 0;
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
                return 0;
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
                //amap["BMSKeyCmd"]->setVal(PowerOffCmd);
                //amap["BMSStartupStart"]->setVal(tNow);
                amap["BMSnumFaults"]->addVal(1);
                amap["BMSStartup"]->setVal(true);
                amap["BMSnumPresent"]->subVal(1);
                return 0;
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
                return 0;
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
                if (1) FPS_ERROR_PRINT("%s >>>>>> Forced BMS Startup at %f \n ", __func__, tNow);
                amap["BMSnumPresent"]->subVal(1);
                amap["BMSnumFaults"]->addVal(1);
            }
            return 0;

        }

        // NOTE: Needs aname to be a particular pcs_number - Might be enum instead.
        // TODO: Need to actually implement this properly, only BMS is done right now.
        // IMPORTANT: Might be deprecated, need to check out registers.
        int StartupPCSasset(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
        {
            //Timer StartupBMSTimer("StartupBMSTimer", true);

            //TODO: do this function in the future.

            int reload = 0;
            bool bval = false;
            int ival = 0;
            //double dval = 0.0;
            asset_manager* am = aV->am; 

            VarMapUtils* vm = am->vm;
            essPerf ePerf(am, aname, __func__);

            // char* tVal = (char*)"Test TimeStamp";
            // in future use tval for sending messages out - for UI and alert purposes.

            assetVar* pcsAv = amap[__func__]; // Need to figure out the resolution here!
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


                if (reload < 1) // complete restart 
                {
                    amap["PCSKeyStatus"]->setVal(0);
                    amap["PCSKeyCmd"]->setVal(0);
                }
                reload = 2;
                pcsAv->setVal(reload);
            }

            // get the reference to the variable 
            //assetVar* StartupRequestedAV = amap["StartupRequested"];
            //assetVar* PCSkeystartCmdAV = amap["PCSKeystartCmd"];
            //assetVar* PCSkeyCmdAV = amap["PCSKeyCmd"];


            // storing asset states:
            //bool EMMUFaulted = amap["EMMUFaulted"]->getbVal();
            //bool BMSFaulted = amap["BMSFaulted"]->getbVal();
            //bool PCSFaulted = amap["PCSFaulted"]->getbVal();
            //bool EMUFaulted = amap["EMUFaulted"]->getbVal(); // Supposed to be EMS?
            //bool StartupRequest = amap["StartupRequest"]->getbVal();
            //bool StartupRequested = amap["StartupRequested"]->getbVal();
            //int PCSKeystartVal = amap["PCSKeystartCmd"]->getiVal();
            //int PCSKeyStatusVal = amap["PCSKeyStatus"]->getiVal();
            //int PCSKeyCmdVal = amap["PCSKeyCmd"]->getiVal();
            return 0;
        }

        // void sendKeystartCmd(/* faultFuncArgs, other args? */) //LC send KEYstart command to PCS
        // {
        //     printf("sendKeystartCmd!\n");
        //     //     if(1) FPS_ERROR_PRINT("%s >> %s is sending keystart cmd to pcs\n", __func__, aname);
        //     //
        //     //     VarMapUtils* vm = ai->vm;
        //     //
        //     //     if (!amap[Keystart])
        //     //     {
        //     //         bool bval;
        //     //         amap[Keystart] = vm->setLinkVal(vmap, "pcs", "/controls", "KeystartCmd", bval);
        //     //     }
        //     //
        //     //     // Need to check if pcs has already received keystart cmd (keystart = true)
        //     //     
        //     //     bool keystartCmdActive = amap[Keystart]->getVal(keystartCmdActive);
        //     //     int sysState = amap[SystemStateNum]->getVal(sysState);
        //     //     if (!keystartCmdActive && sysState == System_Fault && aname != "pcs")
        //     //     {
        //     //         amap[keystartCmdActive]->setVal(true);
        //     //         vm->sendAssetVar(amap[keystartCmdActive], p_fims);
        //     //     }
        //     //
        //     //     // After KeystartCmd has been sent to PCS, we'll probably need to check if the cmd actually went through
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
}

#endif
