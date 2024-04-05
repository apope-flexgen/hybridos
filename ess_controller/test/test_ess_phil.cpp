/*
 * ess test , opens up the master config and creates all the peripherals
 * bms , pcr , drc
 * Starts all those threads if needed
 * handles the interface between hos ( via modbus server pubs) and hte
 * controllers. It has registers and links. g++ -std=c++11 -g -o ./test_ess -I
 * ./include test/test_ess.cpp -lpthread -lcjson -lfims
 */

#include "asset.h"
#include "channel.h"
#include "chrono_utils.hpp"
// #include "monitor.h"
// //#include "ess.h"
// //#include "bms.h"

// set this to 0 to stop
volatile int running = 1;
asset_manager* ess_man = nullptr;

void signal_handler(int sig)
{
    running = 0;
    FPS_ERROR_PRINT("signal of type %d caught.\n", sig);
    if (ess_man)
    {
        ess_man->running = 0;
        ess_man->wakechan.put(-1);
    }
    signal(sig, SIG_DFL);
}

int sendTestMessage(fims* p_fims, int tnum)
{
    const char* method;
    const char* replyto = nullptr;
    const char* uri = nullptr;
    const char* body = nullptr;

    switch (tnum)
    {
        case 1:
        {
            method = "set";
            replyto = "/test/foo";
            uri = "/components/test_1";
            body = "{\"var_set_one\":21}";
        }
        break;
        case 2:
        {
            method = "set";
            // replyto = "/test/foo";
            uri = "/components/test_2";
            body = "{\"var_set_one_again\":21,\"var_set_two\":334.5}";
        }
        break;
        case 3:
        {
            method = "set";
            replyto = "/test/foo_2";
            uri = "/components/test_2";
            body = "{\"var_set_one_again\":21,\"var_set_two\":334.5}";
        }
        break;
        case 4:
        {
            method = "set";
            replyto = "/test/foo_4";
            uri = "/components/test_3";
            body = "{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
        }
        break;
        case 5:
        {
            method = "get";
            replyto = "/test/foo_5";
            uri = "/components/test_3";
            // body="{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
        }
        break;
        case 6:
        {
            method = "get";
            replyto = "/test/foo_6";
            uri = "/components/test_3/var_set_twox";
            // body="{\"var_set_one_with_value\":{\"value\":2100},\"var_set_two\":334.5}";
        }
        break;
        case 7:
        {
            method = "set";
            replyto = "/test/foo_7";
            uri = "/assets/bms_1";
            body = "{\"ctrlword1\":{\"value\":3}}";
        }
        break;
        case 8:
        {
            method = "set";
            replyto = "/test/foo_8";
            uri = "/assets/bms_1";
            body = "{\"ctrlword2\":{\"value\":1},\"ctrlword2\":{\"value\":2}}";
        }
        break;
        case 9:
        {
            method = "set";
            replyto = "/test/foo_9";
            uri = "/components/catl_ems_bms_rw";
            body = "{\"ems_test_status\":{\"value\":\"Running\"}}";
        }
        break;
        default:
            break;
    }
    if (uri)
        p_fims->Send(method, uri, replyto, body);
    return 0;
}

// lot of notes here but we'll condense them
// add a config uri or method
// add an actions mode to the var
// onset will do the ctrlword translations.

//
// these are the ess commands from the hos_manager
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

// These are the ess responses
// bms_bus_voltage(array)
// bms_bus_current(array)
// ess_soc
// ess_soh
// ess_power
// operational_status (Standby Charging Discharging ( Fault))
// ess_current_chargeable_capacity   ( how much we can charge the thing)
// ess_current_dischargeable_capacity
// ess_circuit_breaker_control_word
// ess_circuit_breaker_status
// ess_maximum_charging_power_limit
// ess_maximum_discharging_power_limit
// ess_alarms
// ess_warnings
//
// this is our map utils factory
VarMapUtils defvm;
VarMapUtils* vm = &defvm;

// this is a map of local variables as known to the asset
varmap* amap;

// look for a change in /status/ess/heartbeat
// compare against /status/bms_xxheartbeat // send hartbeat and TOD if changed
int HandleBMSAssetHeartBeat(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* am = nullptr)
{
    int reload;
    double dval = 0.0;
    // double dvalHB = 0.0;
    int ival = 0;
    VarMapUtils* vmp = vm;
    if (am)
    {
        vmp = am->vm;
    }

    reload = vmp->CheckReload(vmap, amap, aname, __func__);
    assetVar* HandleBMSHeartBeat = amap[__func__];

    if (reload < 2)
    {
        // reload = 0;
        amap["HeartBeatLast"] = vmp->setLinkVal(vmap, aname, "/status", "HeartBeatLast", dval);
        amap["bms_todSec"] = vmp->setLinkVal(vmap, aname, "/status", "bms_todSec", ival);
        amap["bms_todMin"] = vmp->setLinkVal(vmap, aname, "/status", "bms_todMin", ival);
        amap["bms_todHr"] = vmp->setLinkVal(vmap, aname, "/status", "bms_todHr", ival);
        amap["bms_todDay"] = vmp->setLinkVal(vmap, aname, "/status", "bms_todDay", ival);
        amap["bms_todMon"] = vmp->setLinkVal(vmap, aname, "/status", "bms_todMon", ival);
        amap["bms_todYr"] = vmp->setLinkVal(vmap, aname, "/status", "bms_todYr", ival);
        amap["todSec"] = vmp->setLinkVal(vmap, "ess", "/status", "todSec", ival);
        amap["todMin"] = vmp->setLinkVal(vmap, "ess", "/status", "todMin", ival);
        amap["todHr"] = vmp->setLinkVal(vmap, "ess", "/status", "todHr", ival);
        amap["todDay"] = vmp->setLinkVal(vmap, "ess", "/status", "todDay", ival);
        amap["todMon"] = vmp->setLinkVal(vmap, "ess", "/status", "todMon", ival);
        amap["todYr"] = vmp->setLinkVal(vmap, "ess", "/status", "todYr", ival);

        amap["HandleBMSHeartBeat"] = vmp->setLinkVal(vmap, aname, "/status", "HandleBMSHeartBeat", reload);
        amap["HeartBeat"] = vmp->setLinkVal(vmap, "ess", "/status", "HeartBeat", ival);
        amap["HandleBMSHeartBeat"]->setVal(2);  // revert reload
        if (reload == 0)                        // complete restart
        {
            if (vmp->notMissing(amap, __func__, "HeartBeatLast") &&
                vmp->notMissing(amap, __func__, "HandleBMSHeartBeat") && vmp->notMissing(amap, __func__, "HeartBeat"))
            {
                ival = amap["HeartBeat"]->getiVal();
                amap["HeartBeatLast"]->setVal(ival);
                amap["HandleBMSHeartBeat"]->setVal(2);
            }
        }
    }

    // if get_time_dbl() > HBLast + HBInterval) recalc HB and tod
    double HBLast = amap["HeartBeatLast"]->getdVal();
    double HBnow = amap["HeartBeat"]->getdVal();
    dval = 1.0;
    // dont use alueChanged it resets the change currently
    if (HBLast != HBnow)
    {
        amap["HeartBeatLast"]->setVal(HBnow);

        dval = 1.0;
        // this value is used to trigger the heartbeats for all the assets that need
        // it
        amap["HeartBeat"]->addVal(dval);
        amap["bms_todSec"]->setVal(amap["todSec"]->getiVal());
        amap["bms_todMin"]->setVal(amap["todMin"]->getiVal());
        amap["bms_todHr"]->setVal(amap["todHr"]->getiVal());
        amap["bms_todDay"]->setVal(amap["todDay"]->getiVal());
        amap["bms_todMon"]->setVal(amap["todMon"]->getiVal());
        amap["bms_todYr"]->setVal(amap["todYr"]->getiVal());

        // this stuff collects a bunch of assetVars and send them out to their
        // default locations. the link will determine where that location is. if the
        // link is defined in the config file then that destination will be
        // maintained.

        varsmap* vlist = vmp->createVlist();
        vmp->addVlist(vlist, amap["HeartBeat"]);
        vmp->addVlist(vlist, amap["bms_todSec"]);
        vmp->addVlist(vlist, amap["bms_todMin"]);
        vmp->addVlist(vlist, amap["bms_todHr"]);
        vmp->addVlist(vlist, amap["bms_todDay"]);
        vmp->addVlist(vlist, amap["bms_todYr"]);
        vmp->sendVlist(p_fims, "set", vlist);
        vmp->clearVlist(vlist);
    }

    return 0;
}

int HandleESSHeartBeat(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am = nullptr)
{
    int reload;
    double dval = 0.0;
    double dvalHB = 1.0;
    int ival = 0;
    assetVar* HandleESSHeartBeat = amap["HandleESSHeartBeat"];
    VarMapUtils* vmp = vm;

    if (am)
    {
        vmp = am->vm;
    }

    reload = vmp->CheckReload(vmap, amap, aname, __func__);
    assetVar* HandleESSHeartBeat = amap[__func__];

    if (reload < 2)
    {
        // reload = 0;
        amap["HeartBeatLast"] = vmp->setLinkVal(vmap, aname, "/status", "HeartBeatLast", dval);
        amap["HeartBeatInterval"] = vmp->setLinkVal(vmap, aname, "/status", "HeartBeatInterval", dvalHB);
        amap["todSec"] = vmp->setLinkVal(vmap, aname, "/status", "todSec", ival);
        amap["todMin"] = vmp->setLinkVal(vmap, aname, "/status", "todMin", ival);
        amap["todHr"] = vmp->setLinkVal(vmap, aname, "/status", "todHr", ival);
        amap["todDay"] = vmp->setLinkVal(vmap, aname, "/status", "todDay", ival);
        am->amap["todMon"] = vmp->setLinkVal(vmap, aname, "/status", "todMon", ival);
        am->amap["todYr"] = vmp->setLinkVal(vmap, aname, "/status", "todYr", ival);

        amap["HandleESSHeartBeat"] = vmp->setLinkVal(vmap, aname, "/status", "HandleESSHeartBeat", reload);
        amap["HeartBeat"] = vmp->setLinkVal(vmap, aname, "/status", "HeartBeat", dval);
        amap["HandleESSHeartBeat"]->setVal(2);  // revert reload
        if (reload == 0)                        // complete restart
        {
            amap["HeartBeat"]->setVal(0);
            amap["HandleESSHeartBeat"]->setVal(2);
        }
    }

    // if get_time_dbl() > HBLast + HBInterval) recalc HB and tod
    double HBLast = amap["HeartBeatLast"]->getdVal();
    double HBInt = amap["HeartBeatInterval"]->getdVal();
    double timeNow = vmp->get_time_dbl();

    FPS_ERROR_PRINT("%s %2.3f >>>>>>>>>>>>>>>HBLast %2.3f HBInt %2.3f  \n", __func__, timeNow, HBLast, HBInt);

    dval = 1.0;
    if (timeNow > (HBLast + HBInt))
    {
        time_t tnow = vmp->getTNow(timeNow);
        tm* ltm = localtime(&tnow);

        amap["HeartBeatLast"]->setVal(timeNow);

        dval = 1.0;
        // this value is used to trigger the heartbeats for all the assets that need
        // it
        amap["HeartBeat"]->addVal(dval);
        amap["todSec"]->setVal(ltm->tm_sec);
        amap["todMin"]->setVal(ltm->tm_min);
        amap["todHr"]->setVal(ltm->tm_hour);
        amap["todDay"]->setVal(ltm->tm_mday);
        amap["todMon"]->setVal(ltm->tm_mon);
        amap["todYr"]->setVal(ltm->tm_year + 1900);
    }

    return 0;
}

int HandleHeartBeat(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims)
{
    int reload;
    double dval = 0.0;

    reload = vm->CheckReload(vmap, amap, aname, __func__);
    assetVar* HandleESSHeartBeat = amap[__func__];

    if (reload < 2)
    {
        // reload = 0;
        amap["HandleHeartBeat"] = vm->setLinkVal(vmap, aname, "/config", "HandleHeartBeat", reload);
        amap["HeartBeat"] = vm->setLinkVal(vmap, aname, "/status", "HeartBeat", dval);
        amap["HandleHeartBeat"]->setVal(2);  // revert reload
        if (reload == 0)                     // complete restart
        {
            amap["HeartBeat"]->setVal(0);
            amap["HandleHeartBeat"]->setVal(2);
        }
    }
    // get the reference to the variable
    assetVar* hb = amap["HeartBeat"];
    // double ival;
    dval = hb->getbVal();
    dval++;
    if (dval > 255)
        dval = 0;
    if (1)
        printf("HeartBeat %s val %f ", aname, dval);

    hb->setVal(dval);
    dval = hb->getbVal();
    if (1)
        printf("HeartBeat val after set %f\n", dval);

    vm->sendAssetVar(hb, p_fims);
    return dval;
}
// int HandleHeartBeat(varsmap &vmap, varmap &amap, fims* p_fims)
// {
//     // get the reference to the variable
//     assetVar* hb = amap["HeartBeat"];
//     int ival;
//     hb->getiVal();
//     ival++;
//     if(ival > 255) ival = 0;

//     hb->setVal(ival);
//     vm->sendAssetVar(hb, p_fims);
//     return ival;
// }

// This is the pcs handle power function.

//#include "../src/pcs_functions.cpp"
// we have to make sure BMS has done its job first so run this at LEVEL3
int HandlePower(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims)
{
    double dval;
    static double Pmax, Pset_dbl, Pcmd_dbl;
    int rc = 0;
    static int first = 1;
    if (first == 1)
    {
        amap["ActivePowerSetpoint"] = vm->setLinkVal(vmap, aname, "/controls", "ActivePowerSetpoint", dval);
        amap["ActivePowerDeadband"] = vm->setLinkVal(vmap, aname, "/controls", "ActivePowerDeadband", dval);
        amap["maxChargeCurrent"] = vm->setLinkVal(vmap, "bms", "/status", "maxChargeCurrent", dval);
        amap["maxDischargeCurrent"] = vm->setLinkVal(vmap, "bms", "/status", "maxDischargeCurrent", dval);
        amap["ActivePowerCmd"] = vm->setLinkVal(vmap, aname, "/controls", "ActivePowerCmd", dval);
        amap["Vdc"] = vm->setLinkVal(vmap, aname, "/status", "pcs_vdc_bus_1", dval);
        first = 0;
    }
    assetVar* Pset = amap["ActivePowerSetpoint"];
    assetVar* Pdb = amap["ActivePowerDeadband"];
    assetVar* ImaxC = amap["maxChargeCurrent"];
    assetVar* ImaxD = amap["maxDischargeCurrent"];
    assetVar* Pcmd = amap["ActivePowerCmd"];
    assetVar* Vdc = amap["Vdc"];

    Pdb->setDbVal(Pdb->getbVal());
    Pset_dbl = Pset->getbVal();

    if (Pset->valueChanged(dval))
    {
        printf("Power limiting now\n");
        if (Pset_dbl > 0)
        {
            Pmax = ImaxD->getbVal() * Vdc->getbVal();
            Pcmd_dbl = Pset_dbl > Pmax ? Pmax : Pset_dbl;
        }
        else
        {
            Pmax = ImaxC->getbVal() * Vdc->getbVal();
            Pcmd_dbl = abs(Pset_dbl) > Pmax ? Pmax * -1 : Pset_dbl;
        }
        Pcmd->setVal(Pcmd_dbl);
        rc++;
    }

    std::cout << "Pset: " << Pset_dbl << " Pmax: " << Pmax << " Pcmd: " << Pcmd_dbl << std::endl;
    return rc;
}

int HandleEMSChargeL1(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims)
{
    double dval;
    // static double Pmax, Pset_dbl, Pcmd_dbl;
    int rc = 0;
    static int first = 1;
    if (first == 1)
    {
        // amap["ActivePowerSetpoint"]     = vm->setLinkVal(vmap, aname,
        // "/controls", "ActivePowerSetpoint", dval);  amap["ActivePowerDeadband"]
        // = vm->setLinkVal(vmap, aname, "/controls", "ActivePowerDeadband", dval);
        amap["maxChargeCurrent"] = vm->setLinkVal(vmap, "bms", "/status", "maxChargeCurrent", dval);
        amap["maxDischargeCurrent"] = vm->setLinkVal(vmap, "bms", "/status", "maxDischargeCurrent", dval);
        amap["totChargeCurrent"] = vm->setLinkVal(vmap, "bms", "/status", "totChargeCurrent", dval);
        amap["totDischargeCurrent"] = vm->setLinkVal(vmap, "bms", "/status", "totDischargeCurrent", dval);
        amap["numBMS"] = vm->setLinkVal(vmap, "bms", "/status", "nmuBMS", dval);

        // amap["ActivePowerCmd"]          = vm->setLinkVal(vmap, aname,
        // "/controls", "ActivePowerCmd", dval);  amap["Vdc"]                     =
        // vm->setLinkVal(vmap, aname, "/status", "pcs_vdc_bus_1", dval);
        first = 0;
    }
    // assetVar * Pset = amap["ActivePowerSetpoint"];
    // assetVar * Pdb = amap["ActivePowerDeadband"];
    assetVar* ImaxC = amap["maxChargeCurrent"];
    assetVar* ImaxD = amap["maxDischargeCurrent"];
    assetVar* totMaxC = amap["totChargeCurrent"];
    assetVar* totMaxD = amap["totDischargeCurrent"];
    assetVar* numBMS = amap["numBMS"];

    // assetVar * Pcmd = amap["ActivePowerCmd"];
    // assetVar * Vdc = amap["Vdc"];

    ImaxC->setVal(0.0);
    ImaxD->setVal(0.0);
    totMaxC->setVal(0.0);
    totMaxD->setVal(0.0);
    numBMS->setVal(0.0);
    return rc;
}

// run by eavh BMS asset

// for more than one asset This wont work
int HandleBMSChargeL2(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims)
{
    double dval;
    double dval2;
    // static double Pmax, Pset_dbl, Pcmd_dbl;
    int rc = 0;
    int reload;

    reload = vm->CheckReload(vmap, amap, aname, __func__);
    assetVar* HandleBMSChargeL2 = amap[__func__];

    if (reload < 2)
    {
        amap["HandleBMSChargeL2"] = vm->setLinkVal(vmap, aname, "/status", "HandleBMSChargeL2", reload);
        amap["bmsMaxChargeCurrent"] = vm->setLinkVal(vmap, aname, "/status", "bmsMaxChargeCurrent", dval);
        amap["bmsMaxDischargeCurrent"] = vm->setLinkVal(vmap, aname, "/status", "bmsMaxDischargeCurrent", dval);
        amap["maxChargeCurrent"] = vm->setLinkVal(vmap, "bms", "/status", "maxChargeCurrent", dval);
        amap["maxDischargeCurrent"] = vm->setLinkVal(vmap, "bms", "/status", "maxDischargeCurrent", dval);
        amap["totChargeCurrent"] = vm->setLinkVal(vmap, "bms", "/status", "totChargeCurrent", dval);
        amap["totDischargeCurrent"] = vm->setLinkVal(vmap, "bms", "/status", "totDischargeCurrent", dval);
        amap["numBMS"] = vm->setLinkVal(vmap, "bms", "/status", "nmuBMS", dval);
        // amap["ActivePowerCmd"]          = vm->setLinkVal(vmap, aname,
        // "/controls", "ActivePowerCmd", dval);  amap["Vdc"]                     =
        // vm->setLinkVal(vmap, aname, "/status", "pcs_vdc_bus_1", dval);
        if (reload == 0)  // complete restart
        {
            // amap["HeartBeat"]->setVal(0);
            amap["HandleBMSChargeL2"]->setVal(2);
        }
    }
    // assetVar * Pset = amap["ActivePowerSetpoint"];
    // assetVar * Pdb = amap["ActivePowerDeadband"];
    assetVar* ImaxC = amap["maxChargeCurrent"];
    assetVar* ImaxD = amap["maxDischargeCurrent"];
    assetVar* bmsMaxC = amap["bmsMaxChargeCurrent"];
    assetVar* bmsMaxD = amap["bmsMaxDischargeCurrent"];
    assetVar* totMaxC = amap["totChargeCurrent"];
    assetVar* totMaxD = amap["totDischargeCurrent"];
    assetVar* numBMS = amap["numBMS"];
    // assetVar * Pcmd = amap["ActivePowerCmd"];
    // assetVar * Vdc = amap["Vdc"];

    if ((dval = bmsMaxC->getdVal()) > ImaxC->getdVal())
        ImaxC->setVal(dval);
    totMaxC->addVal(dval);

    if ((dval = bmsMaxD->getdVal()) > ImaxD->getdVal())
        ImaxD->setVal(dval);
    totMaxD->addVal(dval);

    dval = 1.0;
    numBMS->addVal(dval);

    return rc;
}

// her we handle the incoming commands
int HandleCmd(varsmap& vmap, varmap& amap, fims* p_fims)
{
    int rc = 0;
    // Turn on if conditions allow it
    bool fval = false;
    // int ival;
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
    bool OffCmdval = amap["OffCmd"]->getbVal();
    // bool Offval = amap["Off"]->getbVal();

    bool OnCmdval = amap["OnCmd"]->getbVal();
    bool Onval = amap["On"]->getbVal();
    bool Standbyval = amap["Standby"]->getbVal();

    bool AcContactorval = amap["AcContactor"]->getbVal();
    bool DcContactorval = amap["DcContactor"]->getbVal();

    if (OnCmdval && (!Onval || Standbyval) && AcContactorval && DcContactorval)
    {
        rc++;
        amap["On"]->setVal(true);
        // Send On to device
        amap["OnCmd"]->setVal(false);
        amap["Standby"]->setVal(false);
    }
    else if (Onval && OffCmdval)
    {
        rc++;
        amap["On"]->setVal(false);
        amap["Off"]->setVal(true);
        // Send Off to device
        amap["Offcmd"]->setVal(false);
        // Send Off to device
        amap["Standby"]->setVal(false);
    }
    if (amap["On"]->getbVal() && amap["StandbyCmd"]->getbVal())
    {
        rc++;
        amap["Standby"]->setVal(true);
        // send standby to device
        amap["StandbyCmd"]->setVal(false);
    }
    // TODO send stuff to FIMS
    return rc;
}
// TODO add timeout

// Handle power Generation state between off and gridfollowing
// the system will use text states
//  Off, GridFollowing, Fault  .. etc
int oldHandlePower(varsmap& vmap, varmap& amap, fims* p_fims)
{
    // double dval;
    // char* sval;
    int rc = 0;
    // assetVar* PGState =
    //    amap["PowerGenerationState"];
    // assetVar* PGStateCmd =
    //     amap["PowerGenerationStateCmd"];
    // assetVar* lastPGStateCmd = amap["lastPowerGenerationStateCmd"];
    // assetVar* PCState = amap["PCSState"];
    // assetVar* PCSFault = amap["PCSFault"];// TODO amapGet("PCSFault");
    // assetVar* PCStateCmd = amap["PCSStateCmd"];

    //}
    return rc;
}

//     amap["lastActivePowerSetpoint"]->setVal(
//     amap["ActivePowerSetpoint"]->getbVal()); if
//     (abs(amap["ActivePowerSetpoint"]->getbVal()) <
//     amap["maxActivePower"]->getbVal())
//     {
//         amap["ActivePowerCmd"]->setVal(amap["ActivePowerSetpoint"]->getbVal());
//     }
//     else
//     {
//         amap["ActivePowerCmd"]->setVal((amap["ActivePowerSetpoint"]->getbVal()>0.0)?amap["maxActivePower"]->getbVal()
//                             :-amap["maxActivePower"]->getbVal());
//     }
//     vm->sendAssetVar(amap["ActivePowerCmd"], p_fims);
//     rc++;
// }

// // another way to do this using local vars , prevents the dict lookup
// assetVar * RPSP = amap["ReactivePowerSetpoint"];
// assetVar * lastRPSP = amap["lastReactivePowerSetpoint"];
// assetVar * RPDB = amap["ReactivePowerDeadband"];
// assetVar * maxRP = amap["maxReactivePower"];
// assetVar * RPcmd = amap["ReactivePowerCmd"];

// if ( vm->valueChanged(RPSP, lastRPSP, RPDB ,dval, 2.0))
// {
//     lastRPSP->setVal(  RPSP->getbVal());
//     if (abs(RPSP->getbVal()) < maxRP->getbVal())
//     {
//         RPcmd->setVal(RPSP->getbVal());
//     }
//     else
//     {
//         RPcmd->setVal((RPSP->getbVal()>0.0)?maxRP->getbVal()
//                                                     :-maxRP->getbVal());
//     }
//     vm->sendAssetVar(RPcmd, p_fims);
//     rc++;
// }

//     return rc;
// }

// we'll move the variable set up here and loose  the othe setup code.
void run_ems_init(asset_manager* am)
{
    FPS_ERROR_PRINT(">>>>>>>>>ESS>>>>>>>>>>>%s running for ESS Manager\n", __func__);
    am->vm->setTime();
    double dval = 0.0;
    double dvalHB = 1.0;
    bool bval = true;
    int ival;

    if (am->reload < 2)
    {
        // reload = 0;
        am->amap["HandleHeartBeat"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/config", "HandleHeartBeat",
                                                         am->reload);
        am->amap["HeartBeat"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/status", "HeartBeat", dval);
        am->amap["HeartBeatLast"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/status", "HeartBeatLast", dval);
        am->amap["HeartBeatInterval"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/status", "HeartBeatInterval",
                                                           dvalHB);
        am->amap["todSec"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/status", "todSec", ival);
        am->amap["todMin"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/status", "todMin", ival);
        am->amap["todHr"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/status", "todHr", ival);
        am->amap["todDay"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/status", "todDay", ival);
        am->amap["todMon"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/status", "todMon", ival);
        am->amap["todYr"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/status", "todYr", ival);

        am->amap["AcContactor"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/status", "AcContactor", bval);
        am->amap["AcContactorCloseCmd"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/controls",
                                                             "AcContactorCloseCmd", bval);
        am->amap["AcContactorOpenCmd"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/controls",
                                                            "AcContactorOpenCmd", bval);

        am->amap["DcContactor"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/status", "DcContactor", bval);
        am->amap["DcContactorCloseCmd"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/controls",
                                                             "DcContactorCloseCmd", bval);
        am->amap["DcContactorOpenCmd"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/controls",
                                                            "DcContactorOpenCmd", bval);
        am->amap["OnCmd"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/controls", "OnCmd", bval);
        am->amap["On"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/status", "On", bval);

        am->amap["OffCmd"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/controls", "OffCmd", bval);
        am->amap["Off"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/status", "Off", bval);

        am->amap["StandbyCmd"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/controls", "StandbyCmd", bval);
        am->amap["Standby"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/status", "Standby", bval);

        am->amap["ActivePower"] = am->vm->setLinkVal(*am->vmap, "pcs_mb_input", "/components", "ActivePower", bval);
        am->amap["ActivePowerSetPoint"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/controls",
                                                             "ActivePowerSetPoint", bval);
        am->amap["ActivePowerDeadBand"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/params",
                                                             "ActivePowerDeadband", bval);

        am->amap["ReactivePower"] = am->vm->setLinkVal(*am->vmap, "pcs_mb_input", "/components", "ReactivePower", bval);
        am->amap["ReactivePowerSetPoint"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/controls",
                                                               "ReactivePowerSetPoint", bval);
        am->amap["ReactivePowerDeadBand"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/params",
                                                               "ReactivePowerDeadband", bval);

        am->amap["HandleHeartBeat"]->setVal(2);  // revert reload
        if (am->reload == 0)                     // complete restart
        {
            am->amap["HeartBeat"]->setVal(0);
            dval = am->vm->get_time_dbl();
            am->amap["HeartBeatLast"]->setVal(dval);
        }
        am->reload = 2;
    }
}

void run_bms_init(asset_manager* am)
{
    FPS_ERROR_PRINT(">>>>>>>>>BMS>>>>>>>>>>>%s running for BMS Manager\n", __func__);
}

// We have presently have two wakeup messages sent
bool run_ems_wakeup(asset_manager* am, int wakeup)
{
    char* item3;
    fims_message* msg;
    std::cout << " EMS >> wakeup ival " << wakeup << "\n";
    FPS_ERROR_PRINT("%s %2.3f >>>>>>>>>>>>>>>>>>>>> running for %s Manager wval %d \n", __func__,
                    am->vm->get_time_dbl(), am->name.c_str(), wakeup);

    if (am->reload != 2)
        am->run_init(am);

    if (wakeup == 0)
    {
        // std::cout << am->name <<">> MANAGER LOOP  process another channel\n";
        // break;
    }
    // now process all the events

    // Do the manager first
    if (wakeup == WAKE_LEVEL1)
    {
        std::cout << am->name << " >>get_state\n";
        HandleESSHeartBeat(*am->vmap, am->amap, "ess", am->p_fims, am);
        std::cout << am->name << "  >>process_cmds\n";
        // HandleCmd(*am->vmap, am->amap, am->p_fims);
        std::cout << am->name << "  >>process_power\n";
        HandleEMSChargeL1(*am->vmap, am->amap, "ess", am->p_fims);
        // TODO fix up asset manager
        // now get the Managed assets and wake them all up

        // now get all the managed assets , wake those up too
    }

    if (wakeup == WAKE_LEVEL3)
    {
        std::cout << am->name << " >>HanlePower\n";
        HandlePower(*am->vmap, am->amap, "ess", am->p_fims);
        std::cout << am->name << "  >>process_cmds\n";
        // HandleCmd(*am->vmap, am->amap, am->p_fims);
        std::cout << am->name << "  >>process_power\n";
        // HandlePower(*am->vmap, am->amap, am->p_fims);
        // TODO fix up asset manager
        // now get the Managed assets and wake them all up

        // now get all the managed assets , wake those up too
    }

    if (wakeup == WAKE_LEVEL_PUB)
    {
        std::cout << am->name << " >>publish status\n";
        // ess_manager * ess = (ess_manager*)am;
        // asset_manager* bm2 = am->getManAsset("bms");

        // note this assumes that the bms is running threaded
        // let the builtin functions do this bms ma not be running a manager thread.
        // bm2->wakechan.put(wakeup);
        // publ
        const char* pname = "Unknown";
        cJSON* cjbm = am->getConfig();
        printf("%s >>>>>>>Publish cjbm %p\n", __func__, (void*)cjbm);
        char* res = cJSON_Print(cjbm);
        printf("%s >>>>>>>Publish fims  %p\n", __func__, (void*)am->p_fims);
        cJSON_Delete(cjbm);
        // TODO fix name
        am->p_fims->Send("pub", pname, nullptr, res);
        free((void*)res);

        // break;
    }

    if (am->msgchan.get(item3, false))
    {
        // sendTestMessage(p_fims, tnum);
        std::cout << am->name << "  >>item3-> data  " << item3 << " tnum :" << am->tnum << "\n";
        am->tnum++;
        // sendTestMessage(p_fims, tnum);
        std::cout << am->name << "  >>item3-> data  " << item3 << " tnum :" << am->tnum << "\n";
        am->tnum++;
        free((void*)item3);
    }

    // this gets in fims messages
    if (am->fimschan.get(msg, false))
    {
        std::cout << am->name << "  >> fims_msg uri " << msg->uri << "\n";
        // we need to collect responses
        cJSON* cj = nullptr;
        // either here of the bms instance
        // bms_man->vmap
        am->vm->processFims(*am->vmap, msg, &cj);
        if (cj)
        {
            char* tmp = cJSON_PrintUnformatted(cj);
            if (tmp)
            {
                am->p_fims->Send("set", msg->replyto, nullptr, tmp);
                free((void*)tmp);
            }
        }
        am->p_fims->free_message(msg);

        // free((void *) item3);
    }

    return true;
}

bool run_bms_asset_wakeup(asset* am, int wakeup)
{
    FPS_ERROR_PRINT("%s %2.3f >>>>>>>>>>>>>>>>>>>>> running for %s Asset wval %d \n", __func__, am->vm->get_time_dbl(),
                    am->name.c_str(), wakeup);
    // FPS_ERROR_PRINT("%s >>>>>>>>>BMS>>>>>>>>>>> running for BMS Asset
    // [%s]\n",__func__,ass->name.c_str());
    // WAKE_LEVEL1
    if (wakeup == WAKE_LEVEL1)
    {
        HandleBMSAssetHeartBeat(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);
        // HandleBMSChargeL2(varsmap &vmap, varmap &amap, const char*aname, fims*
        // p_fims)
    }
    else if (wakeup == WAKE_LEVEL2)
    {
        HandleBMSChargeL2(*am->vmap, am->amap, am->name.c_str(), am->p_fims);
    }

    return true;
}

// this one assumes channels are working..
bool run_bms_wakeup(asset_manager* am, int wakeup)
{
    FPS_ERROR_PRINT("%s %2.3f >>>>>>>>>>>>>>>>>>>>> running for %s Manager wval %d \n", __func__,
                    am->vm->get_time_dbl(), am->name.c_str(), wakeup);
    // then service the other channels
    //
    char* item3;
    fims_message* msg;
    // a wakeup of 0 means service the others
    // a wakeup if 1 means process the asset
    // a wakeup of 2 means pub the asset
    if (wakeup == 0)
    {
        // td::cout << " BMS >> process another channel\n";
        // break;
    }

    if (wakeup == 1)
    {
        am->vm->setTime();
        // std::cout << " BMS >> get_state\n";
        HandleHeartBeat(*am->vmap, am->amap, "bms", am->p_fims);
        // HandleHeartBeat(vmap, *amap, p_fims);
        // std::cout << " BMS >>process_cmds\n";
        // HandleCmd(vmap, *amap, p_fims);
        // std::cout << " BMS >> process_power\n";
        // HandlePower(vmap, *amap, p_fims);
    }

    if (wakeup == 2)
    {
        std::cout << " TODO BMS >> publish status\n";

        // publ
        // for each instance
        // cJSON* cjbm = bm->getConfig();
        // char* res = cJSON_Print(cjbm);
        // //printf("Publish \n%s\n", res);
        // cJSON_Delete(cjbm);
        // p_fims->Send("pub", "/status/bms", nullptr, res);
        // free((void *)res) ;

        // break;
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
        // sendTestMessage(p_fims, tnum);
        std::cout << " BMS >>item3-> data  " << item3 << " tnum :" << am->tnum << "\n";
        am->tnum++;
        // sendTestMessage(p_fims, tnum);
        std::cout << " BMS >>item3-> data  " << item3 << " tnum :" << am->tnum << "\n";
        am->tnum++;
        free((void*)item3);
    }

    // this gets in fims messages
    if (am->fimschan.get(msg, false))
    {
        std::cout << " BMS >>fims_msg uri " << msg->uri << "\n";
        // we need to collect responses
        cJSON* cj = nullptr;
        // either here of the bms instance
        // bms_man->vmap
        am->vm->processFims(*am->vmap, msg, &cj);
        if (cj)
        {
            char* tmp = cJSON_PrintUnformatted(cj);
            if (tmp)
            {
                am->p_fims->Send("set", msg->replyto, nullptr, tmp);
                free((void*)tmp);
            }
        }
        am->p_fims->free_message(msg);
        // free((void *) item3);
    }
    return true;
}
// the main ess_controller start up
int main(int argc, char* argv[])
{
    varsmap vmap;

    // TODO make this an asset manager
    ess_man = new asset_manager("ess_controller");

    ess_man->vmap = &vmap;  //??
    ess_man->vm = vm;       //??

    ess_man->run_init = run_ems_init;
    ess_man->run_wakeup = run_ems_wakeup;

    ess_man->setVmap(&vmap);
    ess_man->running = 1;

    amap = ess_man->getAmap();  // may not need this

    // TODO this may be redundant
    // ess_man->setVmap(&vmap);

    // right this thing is ready to go
    // Just for kicks read in the config file and see if it had our links in it
    // this should be the case after the first run.
    printf(" Getting initial configuration\n");

    const char* cfgname = "configs/test_ess_config.json";
    // ess_man->configure(vmap, cfgname , "ess");
    // varsmap* vmp = ess_man->getVmap();
    vm->configure_vmap(vmap, cfgname);
    // ess_man->configure(cfgname , nullptr);

    {
        // this a test for our config.
        cJSON* cjbm = nullptr;
        cjbm = vm->getMapsCj(vmap);
        const char* fname = "configs/ess_1_base_ess.json";
        vm->write_cjson(fname, cjbm);
        // cJSON* cjbm = ess_man->getConfig();
        char* res = cJSON_Print(cjbm);
        // printf("Maps (possibly) with links at beginning  cjbm %p \n%s\n <<<
        // done\n", (void *)cjbm, res);
        free((void*)res);
        cJSON_Delete(cjbm);
    }
    // we should be able to set up the amap table from the links
    // TODO Fiish this
    vm->CheckLinks(vmap, *amap, "/links/ess");
    // bit ugly at the moment .. its a copy of the aset config

    // no dont do this now
    // ess_man->setup(vmap, "/links/ess");
    ess_man->cfgwrite("configs/ess_after_links.json");

    // this is just a check
    // vmap["/links/bms_1"]
    auto ix = vmap.find("/links/ess");
    if (ix != vmap.end())
    {
        // if this works no need to run the init function below
        printf(" We found our links , we should be able to set up our link amap\n");
        for (auto iy : ix->second)
        {
            if (iy.second->type == assetVar::ASTRING)
            {
                printf(" lets link [%s] to [%s]\n", iy.first.c_str(), iy.second->aVal->valuestring);
                // for example lets link [AcContactor] to the var defined for
                // [/status/bms_1:AcContactor] amap[iy.first] = vm->getVar (vmap,
                // y.second->aVal->valuestring);//  getVar(varsmap &vmap, const char*
                // comp, const char* var=nullptr)
                // amap["AcContactor"]               = vm->linkVal(vmap, link,
                // "AcContactor",            fval);
            }
        }
    }

    // nah not working yet
    // ess_man->cfgwrite("configs/justlinks_ess.json");

    printf("ess test OK so far\n");
    // this is a low level configure with no subs.
    // ess_man->configure("configs/test_ess.json");
    // todo mo
    cfgname = "configs/test_ess.json";  // TODO move this into asset manager assconfig
    ess_man->configure(&vmap, cfgname, "ess");
    // ess_man->configure(cfgname , (char *)"ess");
    // no worries at this stage

    // this sets up the amap for bss_running vars not needed if the links are in
    // the config....
    // ess_man->initLinks();

    // ess_man->cfgwrite("configs/cfg_and_links_ess.json");

    // todo overwrite config with the last running config.
    // bm->configure("data/last_bss_1.json");
    // set up an outgoing fims connection.
    fims* p_fims = new fims;

    // p_fims->Connect((char *)"fimsEssTest");
    bool connectOK = p_fims->Connect((char*)"FimsEssTest");
    if (!connectOK)
    {
        FPS_ERROR_PRINT("%s >> unable to connect to fims server. All is lost.\n", __func__);
        running = 0;
    }
    ess_man->p_fims = p_fims;
    ess_man->running = 1;

    // eeded for the controller, manager and possibly assets
    ess_man->setupChannels();

    // the timer generates  channel wakes every 500 mS
    ess_man->run_timer(500);

    // run the message loop eery 1.5 seconds
    // pcs_man->run_message(&pcs_man->m_data, pcs_man->message_loop, &running,
    // 1500,  &pcs_man->wakechan);

    // the message  generates test message  channel wakes every 1500 mS
    ess_man->run_message(1500);

    const char* subs[] = { "/components", "/assets", "/params", "/status", "/controls", "/test" };
    int sublen = sizeof(subs) / sizeof(subs[0]);

    // the fims system will get pubs and create a varsMap for  the items.
    // TODO restrict fims to known components

    ess_man->run_fims(1500, (char**)subs, "essMan", sublen);

    // the manager runs the channel receiver thread
    ess_man->run_manager(p_fims);

    // now we try to create and configure the assets.
    // vmap /assets/ess contains the goodies.
    // do it the long way first
    // vmap["/links/bms_1"]
    asset_manager* bms_man = nullptr;
    auto ixa = vmap.find("/assets/ess");
    if (ixa != vmap.end())
    {
        // if this works no need to run the init function below
        printf(" ESS >>We found our assets, we should be able to set up our system\n");
        for (auto iy : ixa->second)
        {
            if (iy.second->type == assetVar::ASTRING)
            {
                printf(" lets run assets for  [%s] from [%s]\n", iy.first.c_str(), iy.second->aVal->valuestring);
                const char* fname = iy.second->aVal->valuestring;
                if (iy.first == "bms")
                {
                    printf(" setting up a bms manager\n");

                    bms_man = new asset_manager("bms_man");
                    bms_man->run_init = run_bms_init;
                    bms_man->run_wakeup = run_bms_wakeup;
                    bms_man->p_fims = p_fims;

                    bms_man->setVmap(&vmap);
                    // now get the bms_manager to configure itsself
                    bms_man->configure(&vmap, fname, "bms", run_bms_asset_wakeup);
                    // add it to the assetList
                    ess_man->addManAsset(bms_man, "bms");

                    printf(" done setting up a bms manager varsmap must be fun now\n");
                    // we should be able to do things like get status from the
                    // bms_manager. first see it it unmaps correctly.
                    asset_manager* bm2 = ess_man->getManAsset("bms");
                    if (bm2)
                    {
                        printf(" @@@@@@@  found bms asset manager with %d bms assets\n", bm2->getNumAssets());
                    }
                    bms_man->running = 1;
                    bms_man->setupChannels();
                    // bms_man->run_timer(500);
                    // bms_man->run_message(500);

                    bms_man->run_manager(p_fims);
                    bms_man->man_wakechan.put(2);
                    // const char* subs[] = {
                    //     "/components",
                    //     "/assets/bms_1",
                    //     "/controls/bms_1",
                    //     "/test/bms",
                    //     "/assets/bms",
                    //     "/controls/bms"
                    //         };
                    // bms_man->run_fims(1500, (char**)subs, "bmsMan");

                    // TODO we have to run the bms_manager to get it talking to the
                    // varsmap
                    // first lets sort out the ess_manager's vma problem
                }
            }
        }
    }
    {
        const char* fname = "configs/ess_4_cfg_links_assets_ess.json";
        // this a test for our config with links
        cJSON* cjbm = nullptr;
        cjbm = vm->getMapsCj(vmap);
        vm->write_cjson(fname, cjbm);

        // cJSON* cjbm = ess_man->getConfig();
        char* res = cJSON_Print(cjbm);
        // printf("Maps (should be ) with links and assets  cjbm %p \n%s\n <<<
        // done\n", (void *)cjbm, res);
        free((void*)res);
        cJSON_Delete(cjbm);
    }
    //    ess_man->vmap = &vmap;
    if (ess_man->vmap)
    {
        cJSON* cj = vm->getMapsCj(*ess_man->vmap);
        char* res = cJSON_Print(cj);
        printf("ESS >>Maps before run \n%s\n", res);
        free((void*)res);
        cJSON_Delete(cj);
    }
    else
    {
        printf("ESS >>Maps before runskipped\n");
    }
    while (ess_man->running)
    {
        poll(nullptr, 0, 100);
    }
    printf("ESS >> Shutting down\n");
    bms_man->running = 0;

    ess_man->t_data.cthread.join();
    std::cout << "ESS >>t_data all done "
              << "\n";
    ess_man->m_data.cthread.join();
    std::cout << "ESS >>m_data all done "
              << "\n";
    ess_man->f_data.cthread.join();
    std::cout << "ESS >>f_data all done "
              << "\n";

    if (ess_man->vmap)
    {
        cJSON* cj = vm->getMapsCj(*ess_man->vmap);
        char* res = cJSON_Print(cj);
        printf("ESS >>Maps at end \n%s\n", res);
        free((void*)res);
        cJSON_Delete(cj);
    }
    else
    {
        printf("ESS >>Maps at end skipped\n");
    }

    // monitor M;
    // M.configure("configs/monitor.json");

    delete ess_man;

    return 0;
}
