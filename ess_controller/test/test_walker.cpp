/*
 * bms test , opens up a bms instance and runs the
 * bms controller in a thread with its own fims interface
 * uses a timer to coordinate service and publish times.
 * adding a monior feature
 *  we probably want a pub timer thread and a process timer thread
 * the test messages can act as a sort of simulator
 * IE send the start command and the test message will reflect the status
 */

#include <string>
#include "asset.h"
#include "channel.h"
#include "monitor.h"
#include <gtest/gtest.h>
#include "assetFunc.cpp"
#include "chrono_utils.hpp"
 //#include "bms.h"

int bms_debug = 0;
// set this to 0 to stop
volatile int running = 1;
asset_manager* bms_man = nullptr;

void signal_handler(int sig)
{
    running = 0;
    if (bms_man)
        bms_man->running = 0;
    FPS_ERROR_PRINT("signal of type %d caught.\n", sig);
    signal(sig, SIG_DFL);
}


// both the asset manager and the asset has a varsmap name space

int sendTestMessage(fims* p_fims, int tnum)
{
    const char* method;
    const char* replyto = nullptr;
    const char* uri = nullptr;
    const char* body = nullptr;
    // turn it off for initial manual testing

    switch (tnum) {
    case 1:
    {
        method = "get";
        replyto = "/test/bms";
        uri = "/params/bms_1";
        //body="{\"var_set_one\":21}";
    }
    break;
    // case 2:
    //     {
    //         method = "get";
    //         replyto = "/test/bms";
    //         uri="/params/bms_1/max_load_threshold";
    //         //body="{\"var_set_one\":21}";
    //     }
    //     break;
    // case 3:
    //     {
    //         method = "set";
    //         replyto = "/test/bms";
    //         uri="/params/bms_1";
    //         body="{\"max_load_threshold\":21000}";
    //     }
    //     break;
    // case 4:
    //     {
    //         method = "get";
    //         replyto = "/test/bms";
    //         uri="/params/bms_1/max_load_threshold";
    //         //body="{\"var_set_one\":21}";
    //     }
    //     break;

    // case 2:
    //     {
    //         method = "set";
    //         //replyto = "/test/foo";
    //         uri="/components/test_2";
    //         body="{\"var_set_one_again\":21,\"var_set_two\":334.5}";
    //     }
    //     break;
    // case 3:
    //     {
    //         method = "set";
    //         replyto = "/test/foo_2";
    //         uri="/components/test_2";
    //         body="{\"var_set_one_again\":21,\"var_set_two\":334.5}";
    //     }
    //     break;
    // case 4:
    //     {
    //         method = "set";
    //         replyto = "/test/foo_4";
    //         uri="/components/test_3";
    //         body="{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
    //     }
    //     break;
    // case 5:
    //     {
    //         method = "get";
    //         replyto = "/test/foo_5";
    //         uri="/components/test_3";
    //         //body="{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
    //     }
    //     break;
    // case 6:
    //     {
    //         method = "get";
    //         replyto = "/test/foo_6";
    //         uri="/components/test_3/var_set_twox";
    //         //body="{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
    //     }
    //     break;
    // case 7:
    //     {
    //         method = "set";
    //         replyto = "/test/foo_7";
    //         uri="/assets/bms_1";
    //         body="{\"ctrlword1\":{\"value\":3}}";
    //     }
    //     break;
    // case 8:
    //     {
    //         method = "set";
    //         replyto = "/test/foo_8";
    //         uri="/assets/bms_1";
    //         body="{\"ctrlword2\":{\"value\":1},\"ctrlword2\":{\"value\":2}}";
    //     }
    //     break;
    // case 9:
    //     {
    //         method = "set";
    //         replyto = "/test/foo_9";
    //         uri="/components/catl_ems_bms_rw";
    //         body="{\"ems_test_status\":{\"value\":\"Running\"}}";
    //     }
    //     break;
    default:
        break;
    }
    if (uri)
        p_fims->Send(method, uri, replyto, body);
    return 0;
}

//lot of notes here but we'll condense them 
// add a config uri or method 
// add an actions mode to the var
// onset will do the ctrlword translations.

//
// these are the bms commands from the ess_manager
//
// "ctrlword1[cfg]":[
//          0:0   { "field": "oncmd", "value": true },
//          0:1   { "field": "kacclosecmd", "value": true }
//          1:0   { "field": "offcmd", "value": true },
//          1:1   { "field": "kacopencmd", "value": true }
// "ctrlword2[cfg]":[
//          0:0   { "field": "kdcclosecmd", "value": true }
//          1:0   { "field": "kdcopencmd", "value": true }
// "ctrlword4[cfg]": [
//          0:0   { "field": "standbycmd", "value": true }
//          1:0   { "field": "oncmd", "value": true }
// we have control commands
// code (in go)
// // ContactorControl allows discrete control over DC and AC contactors
    // If not set, only Oncmd or Offcmd are needed

//  start_stop
// 
//  run_mode
//
//
// active_power_setpoint
// reactive_power_setpoint
// active_power_rising_slope (PCS)
// active_power_droop_slope (PCS)
// reactive_power_rising_slope (PCS)
// reactive_power_droop_slope (PCS)
//


// if !e.ContactorControl {
    // 	if !e.On && e.Oncmd {
    // 		e.AcContactorCloseCmd, e.DcContactorCloseCmd = true, true
    // 	} else if e.On && e.Offcmd {
    // 		e.AcContactorOpenCmd, e.DcContactorOpenCmd = true, true
    // 	}
    // }
// this is the simulation
    // if !e.AcContactor && e.AcContactorCloseCmd {
    // 	e.AcContactor, e.AcContactorCloseCmd = true, false
    // } else if e.AcContactor && e.AcContactorOpenCmd {
    // 	e.AcContactor, e.AcContactorOpenCmd = false, false
    // 	e.Offcmd = true
    // }

    // if !e.DcContactor && e.DcContactorCloseCmd {
    // 	e.DcContactor, e.DcContactorCloseCmd = true, false
    // 	e.RacksInService = e.Racks
    // } else if e.DcContactor && e.DcContactorOpenCmd {
    // 	e.DcContactor, e.DcContactorOpenCmd = false, false
    // 	e.RacksInService = 0
    // 	e.Offcmd = true

    // }
//    more simulation
// Turn on if conditions allow it
    // if e.Oncmd && (!e.On || e.Standby) && e.AcContactor && e.DcContactor {
    // 	e.On = true
    // 	e.Oncmd = false
    // 	e.Standby = false
    // } else if e.On && e.Offcmd {
    // 	e.On = false
    // 	e.Offcmd = false
    // 	e.Standby = false
    // }
    // if e.On && e.StandbyCmd {
    // 	e.Standby = true
    // 	e.StandbyCmd = false
    // }


// this is our map utils factory
VarMapUtils vm;

// this is a map of local variables as known to the asset 
varmap* amap;


int HandleHeartBeat(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims)
{
    int reload;
    double dval = 0.0;

    reload = vm.CheckReload(vmap, amap, aname, __func__);
    assetVar* HandleHeartBeat = amap[__func__];

    if (reload < 2)
    {
        //reload = 0;
        amap["HandleHeartBeat"] = vm.setLinkVal(vmap, aname, "/config", "HandleHeartBeat", reload);
        amap["HeartBeat"] = vm.setLinkVal(vmap, aname, "/status", "HeartBeat", dval);
        amap["HandleHeartBeat"]->setVal(2);  // revert reload
        if (reload == 0) // complete restart 
        {
            amap["HeartBeat"]->setVal(0);
        }
        HandleHeartBeat->setVal(2);
    }
    // get the reference to the variable 
    assetVar* hb = amap["HeartBeat"];
    //double ival;
    dval = hb->getdVal();
    dval++;
    if (dval > 255) dval = 0;
    if (0)printf("HeartBeat val %f ", dval);

    hb->setVal(dval);
    dval = hb->getdVal();
    if (0)printf("HeartBeat val after set %f\n", dval);

    vm.sendAssetVar(hb, p_fims);
    return dval;
}
// char buf[1024];
//         const char* value = getdefLink(name, base, var, buf, sizeof(buf));
//         return setVal2(vmap, comp, var, value);
// when this is run inside the asset manager object aname will be given
// however the asset manager may want to tune this on behalf of one of the managed assets
// inwhich case the aname will be bms_1, bms_2 or what ever.
int HandleLoadRequest(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims)
{
    double dval;
    bool  bval;
    char* cval = (char*)"Dummy";
    int reload;

    // setLinkVal 
    // 1/ looks for /links/<aname> and if so picks up the assetVar by reference to the link valuestring
    // for example /links/bms:maxLoadRequest -> /params/bms:maxLoadRequest this is a global for all bms units
    //             /links/bms_1:LoadSetpoint -> /controls/bms_1:LoadSetpoint this is a setpoint for unit bms_1 
    //             /links/ess:EStop -> /controls/ess:EStop this is a global command
    // setLinkval will look for an established link from the config file
    // if not found it will create a link called /links/<aname>  to the default agroup (/params/<aname>:<aval> etc)
    //   it will then look for the "linked to" variable for example /components/catl_bms_ess_01:bms_soc
    //     it will create  this variable ( with the given type) if needed
    //  thus the loop is closed we have a link and an associated variable. creates for us or predefined.
    // to force the links to be reevaluated then /controls/<anmae>:FunName should be set to 1
    // to cause a complete reset set reload to 0;
    //
    reload = vm.CheckReload(vmap, amap, aname, __func__);
    assetVar* HandleLoadRequest = amap[__func__];

    if (reload < 2)
    {
        //reload = 0;
        amap["HandleLoadRequest"] = vm.setLinkVal(vmap, aname, "/config", "HandleLoadRequest", reload);
        amap["maxLoadRequest"] = vm.setLinkVal(vmap, "bms", "/params", "maxLoadRequest", dval);
        amap["LoadRequest"] = vm.setLinkVal(vmap, "bms", "/controls", "LoadRequest", dval);
        amap["LoadRequestDeadband"] = vm.setLinkVal(vmap, "bms", "/params", "LoadRequestDeadband", dval);
        amap["lastLoadRequest"] = vm.setLinkVal(vmap, "bms", "/controls", "lastLoadRequest", dval);
        amap["LoadSetpoint"] = vm.setLinkVal(vmap, aname, "/controls", "LoadSetpoint", dval);
        amap["LoadState"] = vm.setLinkVal(vmap, aname, "/status", "LoadState", cval);
        amap["StateResetCmd"] = vm.setLinkVal(vmap, "bms", "/controls", "StateResetCmd", bval);
        amap["lastStateResetCmd"] = vm.setLinkVal(vmap, "bms", "/status", "lastStateResetCmd", bval);
        amap["EStop"] = vm.setLinkVal(vmap, "ess", "/controls", "EStop", bval);

        HandleLoadRequest->setVal(2);  // revert reload
        if (reload == 0) // complete restart 
        {
            amap["LoadRequestDeadband"]->setVal(25.0);
            amap["lastLoadRequest"]->setVal(25.0);
            amap["LoadRequest"]->setVal(26.0);
            amap["StateResetCmd"]->setVal(false);
            amap["lastStateResetCmd"]->setVal(false);

            amap["LoadState"]->setVal("Init");
        }
    }

    assetVar* maxLoadRequest = amap["maxLoadRequest"];  // all these will crash if the link vars are not set up correctly
    assetVar* LoadRequest = amap["LoadRequest"];
    assetVar* LoadRequestDeadband = amap["LoadRequestDeadband"];
    assetVar* lastLoadRequest = amap["lastLoadRequest"];
    assetVar* LoadSetpoint = amap["LoadSetpoint"];
    assetVar* LoadState = amap["LoadState"];
    assetVar* StateResetCmd = amap["StateResetCmd"];
    assetVar* lastStateResetCmd = amap["lastStateResetCmd"];
    //assetVar* EStop               = amap["EStop"];

    if (0)printf("%s >>>>> STATUS %s  (comp %s) %f to %f (deadband %f)\n"
        , __func__
        , LoadRequest->name.c_str()
        , LoadRequest->comp.c_str()
        , lastLoadRequest->getdVal()
        , LoadRequest->getdVal()
        , LoadRequestDeadband->getdVal()
    );

    if (vm.valueChanged(LoadRequest, lastLoadRequest, LoadRequestDeadband, dval, 0.0))
    {
        printf("%s >>>>> load value changed from %f to %f (deadband %f)\n"
            , __func__
            , lastLoadRequest->getdVal()
            , LoadRequest->getdVal()
            , LoadRequestDeadband->getdVal()
        );
        lastLoadRequest->setVal(LoadRequest->getdVal());
        if (LoadRequest->getdVal() < maxLoadRequest->getdVal())
        {
            LoadSetpoint->setVal(LoadRequest->getdVal());
            cval = (char*)"Running";LoadState->setVal(cval);
        }
        else
        {
            LoadSetpoint->setVal(maxLoadRequest->getdVal());
            cval = (char*)"Limit"; LoadState->setVal(cval);
        }

    }

    if (vm.valueChangednodb(LoadRequest, lastLoadRequest, dval, 0.0))
    {

        if (abs(LoadRequest->getdVal()) < 2.0)
        {
            LoadSetpoint->setVal(LoadRequest->getdVal());
            cval = (char*)"Standby";LoadState->setVal(cval);
        }
    }

    if (vm.valueChangednodb(StateResetCmd, lastStateResetCmd, bval, 0.0))
    {
        lastStateResetCmd->setVal(StateResetCmd->getbVal());
        if (StateResetCmd->getbVal())
        {
            LoadSetpoint->setVal(0.0);
            cval = (char*)"Reset";LoadState->setVal(cval);
        }
        else
        {
            cval = (char*)"Standby";LoadState->setVal(cval);
        }

    }
    return 0;
}

int HandleCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims)
{
    int rc = 0;
    // Turn on if conditions allow it
    bool fval = false;
    //int ival;
    //double dval;
    bool  bval;
    //char* cval = (char *) "Dummy"; 
    int reload;

    // setLinkVal 
    // 1/ looks for /links/<aname> and if so picks up the assetVar by reference to the link valuestring
    // for example /links/bms:maxLoadRequest -> /params/bms:maxLoadRequest this is a global for all bms units
    //             /links/bms_1:LoadSetpoint -> /controls/bms_1:LoadSetpoint this is a setpoint for unit bms_1 
    //             /links/ess:EStop -> /controls/ess:EStop this is a global command
    // setLinkval will look for an established link from the config file
    // if not found it will create a link called /links/<aname>  to the default agroup (/params/<aname>:<aval> etc)
    //   it will then look for the "linked to" variable for example /components/catl_bms_ess_01:bms_soc
    //     it will create  this variable ( with the given type) if needed
    //  thus the loop is closed we have a link and an associated variable. creates for us or predefined.
    // to force the links to be reevaluated then /controls/<anmae>:FunName should be set to 1
    // to cause a complete reset set reload to 0;
    //
    reload = vm.CheckReload(vmap, amap, aname, __func__);
    assetVar* HandleCmd = amap[__func__];

    if (reload < 2)
    {
        //reload = 0;
        amap["HandleCmd"] = vm.setLinkVal(vmap, aname, "/config", "HandleCmd", reload);
        amap["On"] = vm.setLinkVal(vmap, "bms", "/controls", "On", fval);
        amap["OnCmd"] = vm.setLinkVal(vmap, "bms", "/controls", "OnCmd", fval);
        amap["Off"] = vm.setLinkVal(vmap, "bms", "/controls", "Off", fval);
        amap["OffCmd"] = vm.setLinkVal(vmap, "bms", "/controls", "OffCmd", fval);
        amap["Standby"] = vm.setLinkVal(vmap, "bms", "/controls", "Standby", fval);
        amap["StandbyCmd"] = vm.setLinkVal(vmap, "bms", "/controls", "StandbyCmd", fval);
        amap["AcContactor"] = vm.setLinkVal(vmap, "bms", "/controls", "AcContactor", fval);
        amap["DcContactor"] = vm.setLinkVal(vmap, "bms", "/controls", "DcContactor", fval);
        amap["EStop"] = vm.setLinkVal(vmap, "ess", "/controls", "EStop", bval);

        HandleCmd->setVal(2);  // revert reload
        if (reload == 0) // complete restart 
        {
            amap["On"]->setVal(true);
            amap["OnCmd"]->setVal(false);
            amap["Off"]->setVal(true);
            amap["OffCmd"]->setVal(false);
            amap["Standby"]->setVal(false);
            amap["StandbyCmd"]->setVal(false);
            amap["AcContactor"]->setVal(false);
            amap["DcContactor"]->setVal(false);
        }
    }

    assetVar* Off = amap["Off"];
    assetVar* OffCmd = amap["OffCmd"];  // all these will crash if the link vars are not set up correctly
    assetVar* On = amap["On"];
    assetVar* OnCmd = amap["OnCmd"];
    assetVar* Standby = amap["Standby"];
    assetVar* StandbyCmd = amap["StandbyCmd"];
    assetVar* AcContactor = amap["AcContactor"];
    assetVar* DcContactor = amap["DcContactor"];
    //assetVar* EStop                 = amap["EStop"];

    // if e.Oncmd && (!e.On || e.Standby) && e.AcContactor && e.DcContactor {
    // 	e.On = true
    // 	e.Oncmd = false
    // 	e.Standby = false
    // } else if e.On && e.Offcmd {
    // 	e.On = false
    // 	e.Offcmd = false
    // 	e.Standby = false
    // }
    // if e.On && e.StandbyCmd {
    // 	e.Standby = true
    // 	e.StandbyCmd = false
    // }
    bool OffCmdval = OffCmd->getbVal();
    //bool Offval    = Off->getbVal();

    bool OnCmdval = OnCmd->getbVal();
    bool Onval = On->getbVal();
    bool Standbyval = Standby->getbVal();

    bool AcContactorval = AcContactor->getbVal();
    bool DcContactorval = DcContactor->getbVal();

    if (OnCmdval && (!Onval || Standbyval) && AcContactorval && DcContactorval)
    {
        rc++;
        On->setVal(true);
        // Send On to device
        OnCmd->setVal(false);
        Standby->setVal(false);
    }
    else if (Onval && OffCmdval)
    {
        rc++;
        On->setVal(false);
        Off->setVal(true);
        // Send Off to device
        OffCmd->setVal(false);
        // Send Off to device
        Standby->setVal(false);
    }
    if (Onval && Standbyval)
    {
        rc++;
        Standby->setVal(true);
        // send standby to device
        StandbyCmd->setVal(false);
    }
    //TODO send stuff to FIMS
    return rc;
}




int HandlePower(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims)
{
    double dval = 0.0;
    int rc = 0;
    if (vm.valueChanged(amap["ActivePowerSetpoint"], amap["lastActivePowerSetpoint"], amap["ActivePowerDeadband"], dval, 2.0))
    {
        amap["lastActivePowerSetpoint"]->setVal(amap["ActivePowerSetpoint"]->getdVal());
        if (abs(amap["ActivePowerSetpoint"]->getdVal()) < amap["maxActivePower"]->getdVal())
        {
            amap["ActivePowerCmd"]->setVal(amap["ActivePowerSetpoint"]->getdVal());
        }
        else
        {
            amap["ActivePowerCmd"]->setVal((amap["ActivePowerSetpoint"]->getdVal() > 0.0) ? amap["maxActivePower"]->getdVal()
                : -amap["maxActivePower"]->getdVal());
        }
        //vm.sendAssetVar(amap["ActivePowerCmd"], p_fims);
        rc++;
    }

    // another way to do this using local vars , prevents the dict lookup
    assetVar* RPSP = amap["ReactivePowerSetpoint"];
    assetVar* lastRPSP = amap["lastReactivePowerSetpoint"];
    assetVar* RPDB = amap["ReactivePowerDeadband"];
    assetVar* maxRP = amap["maxReactivePower"];
    assetVar* RPcmd = amap["ReactivePowerCmd"];

    if (vm.valueChanged(RPSP, lastRPSP, RPDB, dval, 2.0))
    {
        lastRPSP->setVal(RPSP->getdVal());
        if (abs(RPSP->getdVal()) < maxRP->getdVal())
        {
            RPcmd->setVal(RPSP->getdVal());
        }
        else
        {
            RPcmd->setVal((RPSP->getdVal() > 0.0) ? maxRP->getdVal()
                : -maxRP->getdVal());
        }
        //vm.sendAssetVar(RPcmd, p_fims);
        rc++;
    }
    // if ( valueChanged(amap["ReactivePowerSetpoint"], amap["lastReactivePowerSetpoint"], amap["ReactivePowerDeadband"] ,dval, 2.0))
    // {
    //     amap["lastReactivePowerSetpoint"]->setVal(  amap["ReactivePowerSetpoint"]->getdVal());
    //     if (abs(amap["ReactivePowerSetpoint"]->getdVal()) < amap["maxReactivePower"]->getdVal())
    //     {
    //         amap["ReactivePowerCmd"]->setVal(amap["ReactivePowerSetpoint"]->getdVal());
    //     }
    //     else
    //     {
    //         amap["ReactivePowerCmd"]->setVal((amap["ReactivePowerSetpoint"]->getdVal()>0.0)?amap["maxReactivePower"]->getdVal()
    //                             :-amap["maxReactivePower"]->getdVal());
    //     }
    //     sendAsset(amap["ReactivePowerCmd"], p_fims);
    //     rc++;
    // }

    return rc;
}

// Task from Phil:
/*
The second task entails reading input from the pcs modbus_client like current_fault and setting a fault mode.
The first task should detect the fault mode and set the active power set point to 0.
*/

// FaultManager (if it will exist in the future) could override the level of all of the managers. Fault level = -1?
// FaultManager could take control of the other managers if the other managers aren't doing their jobs.
// Do faultMode logic here based on various factors, subject to change in the future:
// amap/aname will most likely be faultManager - which will call this.
// p_fims will subscribe to (or faultManager will) faultChannel to get the faultMessages
// Format of faultMessages will be: (?)
// How does hybridOS handle faults? How will they be handled in Phil's new system?
// Maybe make this a template function. Specialize based on which asset is calling it (bms vs. pcs)?

// Fault Level?
// Level 1 = asset manages it?
// Level 2 = asset Manager manages it?
// Level 3 = ess controller manages it?
// ...
// Level x = ? (goes higher in management)
// How to get faultLevel, is it handled here?

/*
 * Functionality:
 * - Each asset will handle a fault based on its severity.
 * - If a fault is of higher severity than an asset can handle it passes it up to its "manager." This needs to be added in the future to asset.h
 * - Init links for handling what variables to change and where to send fims when handling faults
 * - fims channel that sends faults around (this is a direct line and ignores normal channel "wakeup" rules, should immediately be handled depending on severity)
 * -
 *
 * FaultTypes:
 * - Level 1 asset - bms_x or pcs_x)
 *      - cellDrawButEmpty (battery cell is empty but tried to draw power from it), switch to another one
 *      - drawingTooMuchPower (trying to draw 1 million mW from a battery),
 *      - lose connection (look at heartbeat/something - need feedback from system)
 * - Level 2 assetManager - bms_manager or pcs_manager)
 *      - batteryEmpty (bms_x's battery is out and needs to be charged, bms_manager needs to draw from another bms_x entirely)
 * - Level 3 ess_controller - manager of managers)
 *      - A fire has occured! (panic!?)
 * - Level 4 fleet_manager - manager of managers of managers)? - Are these emergencies?
 *      - No connection (lost ess_controller connection)
 *      - rack out of power entirely!
 *
 * What this is NOT:
 * - Emergency handler: handles fires and other odd emergencies that must be immediately dealt with.
 * - Handles faults only: It does NOT identify them! That is for each asset to determine individually
 *
 *
 * TODO:
 * - Change asset.h to get a reference to asset_manager so an asset can send faults up the management chain.
 * - figure out how to setup a line to go up chain of command fluently. Is a function needed?
 * - Figure out how to make this configurable? John wanted something that is configurable, maybe have user-defined faultTypes? If so, this complicates things.
 *      - Format could be: "faultTypesAndInstructions": {'faultType_x': {instructions (needs to be constrained)}, ...}
 * -
*/
// vmp = regular old vmap, amap = regular old assetMap, aname = regular old 
int HandleFault(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* ass = nullptr, asset_manager* am = nullptr)
{
    /* level system pseudo code:
     *
     * fault:
     *  - level
     *  - avar (assetVar? - something)
     *  - depth (value changes adds to depth - this is how far along the fault is compared to the others) {subject to change}
     *  -
     *
     * for (auto fault : ass->faults()) {
     *      if (fault.level == amap["warningLevel"]) {
     *
     *      }
     * }
     * if (amp["faults"] == myFaultLevel) {
     * }
     * if (ass->level == faultLevel? - how to get) {
     *      switch (faultType) {
     *          case 'faulttype_x':
     *              do faulttype_x stuff (vm.sendAssetVar(APSP, p_fims, "destination"); // This is where we change data per fault Mode. Where to go?)
     *              break;
     *          ...
     *          default:
     *              break; // handle unknown faults here - that or have type 'u' and send it up to some manager immediately.
     *      }
     * } else {
     *      pass to manager (call assetManager.faultPass? - how to pass upwards?)
     * }
    */

    /*
     *
    */

    // Testing values/setup:
    // int reload;
    double dval;
    int rc = 0;
    char* faultMode = (char*)"\0"; // Change this to a string/char and use switches. Maybe add configuration to it?
    // char has to be a pointer for now. Change assetVar.h

    amap["ActivePowerSetpoint"] = vm.setLinkVal(vmap, "bms_1", "/status", "ActivePowerSetpoint", dval);
    amap["CurrentFault"] = vm.setLinkVal(vmap, "bms_1", "/status", "CurrentFault", faultMode); //Change this to modbus_client:current fault
    assetVar* CF = amap["CurrentFault"]; //This comes from "modbus_client: current_fault". Could turn into something more specific.
    assetVar* APSP = amap["ActivePowerSetpoint"];
    // Reload might not be necessary for handling faults, there should be no need to reload fault Handling:
    // if (!CF || !APSP) // 
    // {
    //     reload = 0;  // complete reset  reload = 1 for remap ( links may have changed )
    // }
    // if(reload < 2)
    // {
    //     //reload = 0;
    //     amap["ActivePowerSetpoint"]      = vm.setLinkVal(vmap, "bms_1", "/status",    "ActivePowerSetpoint",            dval);
    //     amap["CurrentFault"]      = vm.setLinkVal(vmap, "bms_1", "/status",    "CurrentFault",            faultMode); //Change this to modbus_client:current fault
    //     // amap["current_fault"]      = vm.setLinkVal(vmap, "modbus_client", "/uri?",    "current_fault",            faultMode);

    //     // amap["HandleHeartBeat"]->setVal(2);  // revert reload
    //     if(reload == 0) // complete restart
    //     {
    //         faultMode = '\0';
    //         //amap["HeartBeat"]->setVal(0);
    //         //amap["HandleHeartBeat"]->setVal(2);
    //     }
    // }
    // CF = amap["CurrentFault"];
    // APSP = amap["ActivePowerSetpoint"];

    // Do faultMode logic here, could have different solutions in the future:
    // Question: Is FaultMode going to have its own function/thread like this? How will that be handled?
    // Question: Where do I "send" the assetVar to in order to get an actual change in the system? PCS?
    char* fm;
    fm = CF->getcVal();
    if (fm) {
        switch (fm[0]) {
        case 'f':
            std::cout << "Entering Fault Mode" << std::endl;
            APSP->setVal(0.0); // Turn off activepowersetpoint
            vm.sendAssetVar(APSP, p_fims, "/status/bms_1"); // This is where we change data per fault Mode. Where to go?
            rc++;
            break;
        case 'o':
            break;
        default:
            std::cout << "Nothing happened, no fault detected" << std::endl;
        }
    }


    return rc;
}

// Function that could be used to determine the type of fault that an asset is encountering. Might be unnecessary.
// No way to make this general enough for each asset type.
int determineFault(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* ass = nullptr);


bool run_bms_asset_wakeup(asset* am, int wakeup)
{
    if (0)FPS_ERROR_PRINT("%s %2.3f >>>>>>>>>>>>>>>>>>>>> running for %s Asset wval %d \n", __func__, am->vm->get_time_dbl(), am->name.c_str(), wakeup);
    //FPS_ERROR_PRINT("%s >>>>>>>>>BMS>>>>>>>>>>> running for BMS Asset [%s]\n",__func__,ass->name.c_str());
    // WAKE_LEVEL1
    if (wakeup == WAKE_LEVEL1)
    {
        //CheckAssetComms(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);
//            varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset *am)
        //HandleBMSAssetHeartBeat(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);
        //HandleBMSChargeL2(varsmap &vmap, varmap &amap, const char*aname, fims* p_fims)
    }
    else if (wakeup == WAKE_LEVEL2)
    {
        //HandleBMSChargeL2(*am->vmap, am->amap, am->name.c_str(), am->p_fims);
    }

    return true;
}

// this one assumes channels are working..
bool run_bms_wakeup(asset_manager* am, int wakeup)
{
    if (0)FPS_ERROR_PRINT("%s %2.3f >>>>>>>>>>>>>>>>>>>>> running for %s Manager wval %d \n", __func__, am->vm->get_time_dbl(), am->name.c_str(), wakeup);
    // then service the other channels 
    //
    char* item3;
    fims_message* msg;
    // a wakeup of 0 means service the others
    // a wakeup if 1 means process the asset
    // a wakeup of 2 means pub the asset
    if (wakeup == 0)
    {
        //td::cout << " BMS >> process another channel\n";
        //break;
    }

    if (wakeup == 1)
    {
        am->vm->setTime();
        //std::cout << " BMS >> get_state\n";
        //HandleHeartBeat(*am->vmap, am->amap, "bms", am->p_fims);
        //HandleHeartBeat(vmap, *amap, p_fims);
        //std::cout << " BMS >>process_cmds\n";
        //HandleCmd(vmap, *amap, p_fims);
        //std::cout << " BMS >> process_power\n";
        //HandlePower(vmap, *amap, p_fims);
    }

    if (wakeup == 2)
    {
        if (0)std::cout << " TODO BMS >> publish status\n";

        // publ
        // for each instance
        // cJSON* cjbm = bm->getConfig();
        // char* res = cJSON_Print(cjbm);
        // //printf("Publish \n%s\n", res);
        // cJSON_Delete(cjbm);
        // p_fims->Send("pub", "/status/bms", nullptr, res);
        // free((void *)res) ;

        //break;
    }

    if (wakeup == -1)
    {
        // quit
        am->running = 0;
        return false;

    }

    // this is the testmessage
    if (am->msgchan.get(item3, false))
    {
        //sendTestMessage(p_fims, tnum);
        std::cout << " BMS >>item3-> data  " << item3 << " tnum :" << am->tnum << "\n";
        am->tnum++;
        //sendTestMessage(p_fims, tnum);
        std::cout << " BMS >>item3-> data  " << item3 << " tnum :" << am->tnum << "\n";
        am->tnum++;
        free((void*)item3);
    }

    // this gets in fims messages
    if (am->fimschan.get(msg, false))
    {
        std::cout << " BMS >>fims_msg uri " << msg->uri << "\n";
        // we need to collect responses
        //cJSON *cj = nullptr;
        // either here of the bms instance
        //bms_man->vmap  
        am->vm->runFimsMsg(*am->vmap, msg, am->p_fims);//fims* p_fims)
        // //am->vm->processFims(*am->vmap,  msg, &cj);
        // if (cj)
        // {
        //     char* tmp = cJSON_PrintUnformatted(cj);
        //     if(tmp)
        //     {
        //         am->p_fims->Send("set",msg->replyto, nullptr, tmp);
        //         free((void*)tmp);
        //     }
        // }
        // am->p_fims->free_message(msg);
        //free((void *) item3);
    }
    bool foo = true;
    // if(wakeup != 0)
    // {
    //     foo = am->runChildren(wakeup);
    // }
    return foo;
}

bool run_pcs_asset_wakeup(asset* am, int wakeup)
{
    if (0)FPS_ERROR_PRINT("%s %2.3f >>>>>>>>>>>>>>>>>>>>> running for %s Asset wval %d \n", __func__, am->vm->get_time_dbl(), am->name.c_str(), wakeup);
    //FPS_ERROR_PRINT("%s >>>>>>>>>BMS>>>>>>>>>>> running for BMS Asset [%s]\n",__func__,ass->name.c_str());
    // WAKE_LEVEL1
    if (wakeup == WAKE_LEVEL1)
    {
        //CheckAssetComms(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);
//            varsmap &vmap, varmap &amap, const char* aname, fims* p_fims, asset *am)
        //HandleBMSAssetHeartBeat(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);
        //HandleBMSChargeL2(varsmap &vmap, varmap &amap, const char*aname, fims* p_fims)
    }
    else if (wakeup == WAKE_LEVEL2)
    {
        //HandleBMSChargeL2(*am->vmap, am->amap, am->name.c_str(), am->p_fims);
    }

    return true;
}


// this one assumes channels are working..
bool run_pcs_wakeup(asset_manager* am, int wakeup)
{
    if (0)FPS_ERROR_PRINT("%s %2.3f >>>>>>>>>>>>>>>>>>>>> running for %s Manager wval %d \n", __func__, am->vm->get_time_dbl(), am->name.c_str(), wakeup);
    // then service the other channels 
    //
    char* item3;
    fims_message* msg;
    // a wakeup of 0 means service the others
    // a wakeup if 1 means process the asset
    // a wakeup of 2 means pub the asset
    if (wakeup == 0)
    {
        //td::cout << " BMS >> process another channel\n";
        //break;
    }

    if (wakeup == 1)
    {
        am->vm->setTime();
        //std::cout << " BMS >> get_state\n";
        //HandleHeartBeat(*am->vmap, am->amap, "bms", am->p_fims);
        //HandleHeartBeat(vmap, *amap, p_fims);
        //std::cout << " BMS >>process_cmds\n";
        //HandleCmd(vmap, *amap, p_fims);
        //std::cout << " BMS >> process_power\n";
        //HandlePower(vmap, *amap, p_fims);
    }

    if (wakeup == 2)
    {
        if (0)std::cout << " TODO BMS >> publish status\n";

        // publ
        // for each instance
        // cJSON* cjbm = bm->getConfig();
        // char* res = cJSON_Print(cjbm);
        // //printf("Publish \n%s\n", res);
        // cJSON_Delete(cjbm);
        // p_fims->Send("pub", "/status/bms", nullptr, res);
        // free((void *)res) ;

        //break;
    }

    if (wakeup == -1)
    {
        // quit
        am->running = 0;
        return false;

    }

    // this is the testmessage
    if (am->msgchan.get(item3, false))
    {
        //sendTestMessage(p_fims, tnum);
        std::cout << " BMS >>item3-> data  " << item3 << " tnum :" << am->tnum << "\n";
        am->tnum++;
        //sendTestMessage(p_fims, tnum);
        std::cout << " BMS >>item3-> data  " << item3 << " tnum :" << am->tnum << "\n";
        am->tnum++;
        free((void*)item3);
    }

    // this gets in fims messages
    if (am->fimschan.get(msg, false))
    {
        std::cout << " PCS >>fims_msg uri " << msg->uri << "\n";
        // we need to collect responses
        //cJSON *cj = nullptr;
        // either here of the bms instance
        //bms_man->vmap  
        //cj = 
        am->vm->runFimsMsg(*am->vmap, msg, am->p_fims);//fims* p_fims)
        //am->vm->processFims(*am->vmap,  msg, &cj);
        // if (cj)
        // {
        //     char* tmp = cJSON_PrintUnformatted(cj);
        //     if(tmp)
        //     {
        //         am->p_fims->Send("set",msg->replyto, nullptr, tmp);
        //         free((void*)tmp);
        //     }
        // }
        am->p_fims->free_message(msg);
        //free((void *) item3);
    }
    bool foo = true;
    // if (wakeup!=0)
    // {
    //     foo = am->runChildren(wakeup);
    // }
    return foo;
}


int main(int argc, char* argv[])
{
    // chan_data t_data;  // time channel
    // chan_data m_data;  // message channel
    // chan_data f_data;  // fims channel 

    // channel <int> wakechan;         // this is for wakeups
    // channel <char *>msgchan;       // this is for messages ( will probably be fims sort of messages)
    // channel <fims_message *>fimschan;  // this is for real (external) fims messages

    // this is our main data map
    varsmap vmap;


    // we'll have a bms_manager but wont quite use it yet
    bms_man = new asset_manager("bms_man");

    asset* bm;

    bms_man->setVmap(&vmap);
    bm = bms_man->addInstance("bms_1");

    amap = bm->getBmap();
    // TODO this may be redundant
    bm->setVmap(&vmap);
    bms_man->debugConfig(bm, "Maps with links at beginning");
    bm->cfgwrite("configs/bms_1_at_start.json");
    bms_man->running = 1;
    // we should be able to set up the amap table from the links
    vm.CheckLinks(vmap, *amap, "/links/bms_1");

    //bm->setup(vmap, "/links/bms_1");
    bm->cfgwrite("configs/bms_1_after_links.json");
    // we should be able to set up the amap table from the links
    //reload = 0;

    // this is a low level configure with no subs.
    bm->configure("configs/test_bss.json");

    // this sets up the amap for bss_running vars not needed if the links are in the config....

    // todo overwrite config with the last running config.
    //bm->configure("data/last_bss_1.json");
    bms_man->debugConfig(bm, "Maps at beginning");
    bm->cfgwrite("configs/bms_1_cfg_and_links.json");

    // set up an outgoing fims connection. 
    fims* p_fims = new fims;
    p_fims->Connect((char*)"fimsBmsTest");

    int wakeup;
    //std::string item2;
    char* item3;
    fims_message* msg;
    bms_man->setupChannels();
    // timer loop need more expansion
    // its job is to run a timer call back on timer requests
    //  a timer request will have a delay ( from time now) and a call back function
    // the timeer loop sleeps until the next callback is due.
    // call backs are insterted in order.

    //run the timer_loop every 100 mS 
    //pcs_man->run_timer(&pcs_man->t_data, pcs_man->timer_loop,   &running, 500,  &pcs_man->wakechan);
    bms_man->run_timer(500);

    // run the message loop eery 1.5 seconds
    //pcs_man->run_message(&pcs_man->m_data, pcs_man->message_loop, &running, 1500,  &pcs_man->wakechan);

    bms_man->run_message(1500);
    //int scnt = 0;
    //int ccnt = 0;

    const char* subs[] = {
            "/components",
            "/assets",
            "/params",
            "/status",
            "/controls",
            "/test"
    };
    int sublen = sizeof(subs) / sizeof(subs[0]);
    // char** dbsubs = vm.getSubs(vmap, *amap, "bms", 3, ccnt, scnt);
    // vm.addSub(dbsubs, "bms",   ccnt, scnt);
    // vm.addSub(dbsubs, "bms_1", ccnt, scnt);
    // vm.addSub(dbsubs, "bms_2", ccnt, scnt);
    // vm.addSub(dbsubs, "ess", ccnt, scnt);    // added ess sub here - Jimmy
    // vm.addSub(dbsubs, "power_electronics_pcs_1", ccnt, scnt);  // added power_electronics sub here - Jimmy
    // vm.showSubs(dbsubs, "bms", ccnt, scnt);

    // const char* subs[] = {
    //         "/components", 
    //         "/assets/bms",
    //         "/assets/bms_1",
    //         "/controls/bms_1",
    //         "/status/bms_1",
    //         "/params/bms_1",
    //         "/params/bms",
    //         "/controls/bms",
    //         "/test/bms",
    //         "/status/bms",
    //         "/controls/bms"
    //         };
// the fims system will get pubs and create a varsMap for  the items.
//pcs_man->run_fims(&pcs_man->f_data, fims_loop,    &running, 1500,  &pcs_man->wakechan);
    bms_man->run_fims(1500, (char**)subs, "bmsMan", sublen);

    // manager_loop
    //pub_loop

    int tnum = 0;
    // you could use setvar to make sure its there

    // any wake up comes here
    while (bms_man->wakechan.get(wakeup, true)) {
        // then service the other channels 
        //
        if (bms_debug)std::cout << " BMS >> wakeup ival " << wakeup << "\n";
        // a wakeup of 0 means service the others
        // a wakeup if 1 means process the asset
        // a wakeup of 2 means pub the asset
        // we can add special wakeups for things like ESTOP Fast Requests etc
        if (wakeup == 0)
        {
            if (bms_debug)std::cout << " BMS >> process another channel\n";
            //break;
        }
        ///  got some big plans for messages and wakeups
        // we really have two events (poll : 1 pub :2 )
        // but we could (will ) set up more event groups to force an immediate response.
        // 
        //if(wakeup == LoadRequestWakeUp) future option
        if (wakeup == WAKE_LEVEL1) //POLL EVENT
        {
            vm.setTime();
            if (1 || bms_debug)std::cout << " BMS >> get_state\n";

            // HandleLoadRequest(vmap, *amap, "bms", p_fims);

            // if(bms_debug)std::cout << " BMS >> get_state\n";
            // HandleHeartBeat(vmap, *amap, "bms", p_fims);
            // if(bms_debug)std::cout << " BMS >>process_cmds\n";
            // HandleCmd(vmap, *amap, "bms", p_fims);
            // if(bms_debug)std::cout << " BMS >> process_power\n";
            // HandlePower(vmap, *amap, "bms", p_fims);

            HandleFault(vmap, *amap, "bms", p_fims, nullptr, bms_man); // Need to fix segmentation fault - modify jsons.
        }

        if (wakeup == WAKE_LEVEL_PUB) //PUB EVENT
        {
            if (bms_debug)std::cout << " BMS >> publish status\n";

            // publ

            cJSON* cjbm = bm->getConfig("/status/bms_1");
            char* res = cJSON_Print(cjbm);
            //printf("Publish \n%s\n", res);
            cJSON_Delete(cjbm);
            p_fims->Send("pub", "/asset/bms/bms_1", nullptr, res);
            free((void*)res);
            bms_man->wakechan.put(4);// cause wake event 4
            //break;
        }

        // we cant issue the event "send" yet but when this all runs as a thread inside the bms_manager object 
        // we'll have direct access to the wakeup and message channels
        // then a fims set to /controls/bms_1:wakeup '{"value":3}' will cause this to happen 
        // bms_man->wakechan->put(wakeval);
        // once we do this the assetVars will have to have locking but no big deal on that.

        // if(wakeup == 3) // HANDLESETPOINT REQUEST EVENT do it now dont wait for a poll
        // {
        //     vm.setTime();
        //     if(1 ||bms_debug)std::cout << " BMS >> wake event 3\n";

        //     HandleLoadRequest(vmap, *amap, "bms", p_fims);
        // }
        // if(wakeup == 4) // HANDLE TEST REQUEST EVENT do it now dont wait for a poll
        // {
        //     //vm.setTime();
        //     if(1 ||bms_debug)std::cout << " BMS >> >>>>>>>>wake event 4\n";

        //     //HandleLoadRequest(vmap, *amap, "bms", p_fims);
        // }

        if (wakeup == -1) //QUIT
        {
            // quit
            running = 0;
            break;

        }
        // this is the testmessage
        if (bms_man->msgchan.get(item3, false)) {
            sendTestMessage(p_fims, tnum);
            if (bms_debug)std::cout << " BMS >>item3-> data  " << item3 << " tnum :" << tnum << "\n";
            tnum++;
            sendTestMessage(p_fims, tnum);
            if (bms_debug)std::cout << " BMS >>item3-> data  " << item3 << " tnum :" << tnum << "\n";
            tnum++;
            free((void*)item3);
        }

        // this gets in fims messages
        if (bms_man->fimschan.get(msg, false)) {
            if (msg->body)
            {
                std::cout << " BMS >>fims_msg method :" << msg->method << " uri :" << msg->uri << " body :" << msg->body << "\n";
            }
            else
            {
                std::cout << " BMS >>fims_msg method :" << msg->method << " uri :" << msg->uri << " body :" << "NoBody" << "\n";
            }

            std::cout << " BMS >>fims_msg uri " << msg->uri << "\n";
            std::cout << " BMS >>fims_msg method " << msg->method << "\n";
            // we need to collect responses
            cJSON* cj = nullptr;
            // either here of the bms instance
            //bms_man->vmap  
            if (msg->method)
            {
                std::cout << " BMS >> process fims message..." << "\n";

                vm.processFims(*bms_man->vmap, msg, &cj);
                if (cj && msg->replyto)
                {

                    std::cout << " BMS >> process fims reply..." << "\n";

                    char* tmp = cJSON_PrintUnformatted(cj);
                    if (tmp)
                    {
                        p_fims->Send("set", msg->replyto, nullptr, tmp);
                        free((void*)tmp);
                    }
                }
                if (cj)cJSON_Delete(cj);

            }
            p_fims->free_message(msg);

            //free((void *) item3);
        }

    }

    bms_man->t_data.cthread.join();
    std::cout << " BMS >> t_data all done " << "\n";
    bms_man->m_data.cthread.join();
    std::cout << "BMS >> m_data all done " << "\n";
    bms_man->f_data.cthread.join();
    std::cout << "BMS >> f_data all done " << "\n";

    bm->cfgwrite("configs/bms_1_attheend.json");
    cJSON* cj = vm.getMapsCj(*bms_man->vmap);
    char* res = cJSON_Print(cj);
    printf("BMS >>Maps at end \n%s\n", res);
    free((void*)res);
    cJSON_Delete(cj);

    //monitor M;
    //M.configure("configs/monitor.json");

    delete bms_man;

    return 0;

}
