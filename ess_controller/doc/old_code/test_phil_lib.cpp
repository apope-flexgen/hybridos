/*
 * ess test , opens up the master config and creates all the peripherals
 * bms , pcs , drc
 * Starts all those threads if needed
 * handles the interface between hos ( via modbus server pubs) and hte
 * controllers. It has registers and links. g++ -std=c++11 -g -o ./test_ess -I
 * ./include test/test_ess.cpp -lpthread -lcjson -lfims
 */

#include "asset.h"
//#include "channel.h"
//#include "monitor.h"
//#include "ess.h"
//#include "bms.h"

int CheckAmHeartbeat(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am);
// int CheckAmComms(varsmap &vmap, varmap &amap, const char* aname, fims*
// p_fims, asset_manager*am);
int HandlePower(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am);
int HandleBMSChargeL2(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset* am);

namespace defaultFault_module
{
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

// // Reports:
}
}  // namespace defaultFault_module

namespace defaultRun_module
{
// For external reference to these functions if another program needs it.
extern "C" {
// TODO: Section these off so they become easier to manage.
// TODO: Make sure they aren't internal/unsafe functions (move them up if they
// are)
void StartupSystem(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, asset_manager* am);
// void ShutdownPCS(varsmap& vmap, varmap& amap, const char* aname, fims*
// p_fims, asset_manager* am); void ShutdownBMS(varsmap& vmap, varmap& amap,
// const char* aname, fims* p_fims, asset_manager* am); void
// ShutdownPCSasset(varsmap& vmap, varmap& amap, const char* aname, fims*
// p_fims, asset* ai); void ShutdownBMSasset(varsmap& vmap, varmap& amap, const
// char* aname, fims* p_fims, asset* ai);

// // Reports:
}
}  // namespace defaultRun_module
   // set this to 0 to stop
volatile int running = 1;
asset_manager* ess_man = nullptr;
int run_secs = 0;

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
avarmap avmap;
std::vector<std::string> sysVec;
void run_ess_init(asset_manager* am);
#if 0
int main ()
{
    varsmap vmap;
    varsmap pmap;
    varmap dummyvmap;
    //varMapUtils vm;
    
    // TODO make these more flexible
    system("mkdir -p run_configs");
    system("mkdir -p run_logs");

    // TODO make this an asset manager
       //ess_man = new asset_manager("ess_controller");
    ess_man = new asset_manager("ess");
    ess_man->am = nullptr;
    ess_man->running = 1;
    vm->sysVec = &sysVec;

    asset_manager* am = ess_man;
    am->vmap = &vmap;
    am->vm = vm;
    bool bval = false;

    vm->setFunc(vmap, "ess", "run_init", (void*)&run_ess_init);
    am->amap["ReactivePowerDeadBand"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/params", "ReactivePowerDeadband", bval);
        // this a test for our config.
        cJSON* cjbm = nullptr;
        cjbm = vm->getMapsCj(vmap);
        //const char* fname = "run_configs/ess_2_after_links.json";
        //vm->write_cjson(fname, cjbm);
        //cJSON* cjbm = ess_man->getConfig();
        char* res = cJSON_Print(cjbm);
        printf("Maps (possibly) with links at beginning  cjbm %p \n%s\n <<< done\n", (void *)cjbm, res);
        free((void*)res);
        cJSON_Delete(cjbm);

    return 0;
}
#endif

#if 1
// // #include "../test/release_fcns.cpp"
// // #include "../test/test_release_fcns.cpp"
// #include "../test/test_phil_fcns.cpp"
// #include "../funcs/CheckAmHeartbeat.cpp"
// #include "../funcs/CheckAmTimestamp.cpp"
// #include "../funcs/CheckMonitorVar.cpp"
// #include "../funcs/SimHandleHeartbeat.cpp"
// #include "../funcs/HandleAssetHeartbeat.cpp"
// #include "../funcs/UpdateSysTime.cpp"
// #include "../funcs/module_runFuncs.cpp"
// #include "../funcs/module_faultFuncs.cpp"
// #include "../funcs/CheckEssStatus.cpp"

// replace this in asset.h in the future - pesky "expression preceding
// parentheses of apparent call must have (pointer-to-) function type" error
// occuring.
template <typename T>
inline void setAmapAm(asset_manager* am, varmap amap, const char* vname, const char* aname, const char* comp, T ival)
{
    amap[vname] = am->vm->setLinkVal(*am->vmap, aname, comp, vname, ival);
}
#endif

// HMM
void run_ess_init(asset_manager* am)
{
    if (1)
        FPS_ERROR_PRINT(">>>>>>>>>ESS>>>>>>>>>>>%s running for ESS Manager\n", __func__);
    am->vm->setTime();
    double dval = 0.0;
    double dvalHB = 1.0;
    bool bval = false;
    int ival;
    // double dval;

    if (am->reload < 2)
    {
        // reload = 0;
        // setAmapAm(am, am->amap, "TestHB1", am->name.c_str(), "/config", dval);
        // setAmapAm(am, am->amap, "TestHB2", am->name.c_str(), "/config", dval);
        // defAvar(am->amap, TestHB2, dval);

        am->amap["HandleHeartBeat"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/config", "HandleHeartBeat",
                                                         am->reload);
        am->amap["HeartBeat"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/status", "HeartBeat", dval);
        // am->amap["HeartBeatLast"]        = am->vm->setLinkVal(*am->vmap,
        // am->name.c_str(), "/status",    "HeartBeatLast",         dval);
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
            // am->amap["HeartBeatLast"]->setVal(dval);
            // double dval2 = getAvalAm(am->amap, TestHB1, dval);
            // TestHB2->setVal((dval2*1.5));
        }
        am->reload = 2;
        // SetupRunPCRCmd(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);
    }
}

#if 1
void run_bms_init(asset_manager* am)
{
    FPS_ERROR_PRINT(">>>>>>>>>BMS>>>>>>>>>>>%s running for BMS Manager %s\n", __func__, am->name.c_str());
}
void run_bms_asset_init(asset* am)
{
    FPS_ERROR_PRINT(">>>>>>>>>BMS>>>>>>>>>>>%s running for BMS ASSET %s\n", __func__, am->name.c_str());
}
void run_pcs_init(asset_manager* am)
{
    FPS_ERROR_PRINT(">>>>>>>>>BMS>>>>>>>>>>>%s running for PCS Manager %s\n", __func__, am->name.c_str());
}
void run_pcs_asset_init(asset* am)
{
    FPS_ERROR_PRINT(">>>>>>>>>BMS>>>>>>>>>>>%s running for PCS ASSET %s\n", __func__, am->name.c_str());
}
void run_dcr_init(asset_manager* am)
{
    FPS_ERROR_PRINT(">>>>>>>>>DCR>>>>>>>>>>>%s running for DCR Manager %s\n", __func__, am->name.c_str());
}
void run_dcr_asset_init(asset* am)
{
    FPS_ERROR_PRINT(">>>>>>>>>DCR>>>>>>>>>>>%s running for DCR ASSET %s\n", __func__, am->name.c_str());
}

void dummyPub(asset_manager* am, const char* uri, const char* puri)
{
    if (0)
        FPS_ERROR_PRINT("%s >>>>>>>Publish fims  Start uri [%s]  \n", __func__, uri);
    // return;
    // TODO define opt defs 0x0100  means send for ui
    cJSON* cjp = am->vm->getMapsCj(*am->vmap, uri, nullptr, 0x0100);
    if (0)
        FPS_ERROR_PRINT("%s >> aname [%s] << publish status cjp %p \n", __func__, am->name.c_str(), cjp);
    // pname ="Unknown2";
    // res = "After pub";
    // // cJSON* cjbm = am->getConfig();
    // // if(0)printf("%s >>>>>>>Publish cjbm %p\n",
    // __func__, (void *)cjbm);
    // // // TODO fix name
    // am->p_fims->Send("pub", puri, nullptr, res);
    char* res = nullptr;
    if (cjp && cjp->child && cjp->child->string && cjp->child->child)
    {
        res = cJSON_PrintUnformatted(cjp->child->child);

        if (0)
            FPS_ERROR_PRINT("%s >>>>>>>Publish fims  res >>%s<< cjp %p\n", __func__, res, cjp);
        if (0)
            FPS_ERROR_PRINT("%s >>>>>>>Publish fims cj->string [%s]\n", __func__, cjp->string);
        if (0)
            FPS_ERROR_PRINT("%s >>>>>>>Publish fims cj->child->string [%s]\n", __func__, cjp->child->string);

        if (0)
            FPS_ERROR_PRINT("%s >>>>>>>Able to publish to [%s] [%s] fims \n", __func__, uri, puri);
        am->p_fims->Send("pub", cjp->child->string, nullptr, res);
    }
    else
    {
        cjp = am->vm->getMapsCj(*am->vmap, uri, nullptr, 0x0);
        if (cjp && cjp->child)
        {
            res = cJSON_PrintUnformatted(cjp->child);
            if (0)
                FPS_ERROR_PRINT(
                    "%s >>>>>>>Able to publish to [%s] [%s] fims cjp %p "
                    "cjp->child %p >>%s<<\n",
                    __func__, uri, puri, cjp, cjp->child, res);
            am->p_fims->Send("pub", cjp->child->string, nullptr, res);
        }
        else
        {
            if (1)
                FPS_ERROR_PRINT("%s >>>>>>>Unable to publish to [%s] [%s] fims cjp %p \n", __func__, uri, puri, cjp);
        }
    }
    if (cjp)
        cJSON_Delete(cjp);
    if (res)
        free((void*)res);
}

// We have presently have two wakeup messages sent
bool run_ess_wakeup(asset_manager* am, int wakeup)
{
    char* item3;
    fims_message* msg;
    if (wakeup != 0)
    {
        // if(1)std::cout << " ESS >> wakeup ival "<< wakeup << "\n";
        if (0)
            FPS_ERROR_PRINT("%s %2.3f >>>>>>>>>>>>>>>>>>>>> running for %s Manager wval %d \n", __func__,
                            am->vm->get_time_dbl(), am->name.c_str(), wakeup);
    }
    if (am->reload != 2)
    {
        if (1)
            FPS_ERROR_PRINT(
                "%s %2.3f >>>>>>>>>>>>>>>>>>>>> running init for %s "
                "Manager wval %d \n",
                __func__, am->vm->get_time_dbl(), am->name.c_str(), wakeup);
        am->run_init(am);
    }

    if (wakeup == -1)
    {
        // quit
        FPS_ERROR_PRINT("%s MANAGER LOOP %s QUITTING\n", __func__, am->name.c_str());
        running = 0;
        // break;//
        return false;
    }

    if (wakeup == 0)
    {
        if (0)
            std::cout << am->name << ">> MANAGER LOOP  process another channel\n";
        // break;
    }
    // now process all the events
    // avarmap avmap;
    // Do the manager first
    if (wakeup == WAKE_LEVEL1)
    {
        am->vm->setTime();

        if (run_secs == 30)
        {
            run_secs++;
            const char* fname = "run_configs/ess_4_after_30 seconds.json";
            // this a test for our config with links
            cJSON* cjbm = am->vm->getMapsCj(*am->vmap, nullptr, nullptr, 0x0010);
            if (cjbm)
            {
                am->vm->write_cjson(fname, cjbm);
                cJSON_Delete(cjbm);
            }
        }

        // namespace defaultFault_module
        // {
        //     // For external reference to these functions if another program needs
        //     it. extern "C"
        //     {
        //         // TODO: Section these off so they become easier to manage.
        //         // TODO: Make sure they aren't internal/unsafe functions (move
        //         them up if they are) void ShutdownSystem(varsmap& vmap, varmap&
        //         amap, const char* aname, fims* p_fims, asset_manager* am); void
        //         ShutdownPCS(varsmap& vmap, varmap& amap, const char* aname, fims*
        //         p_fims, asset_manager* am); void ShutdownBMS(varsmap& vmap,
        //         varmap& amap, const char* aname, fims* p_fims, asset_manager*
        //         am); void ShutdownPCSasset(varsmap& vmap, varmap& amap, const
        //         char* aname, fims* p_fims, asset* ai); void
        //         ShutdownBMSasset(varsmap& vmap, varmap& amap, const char* aname,
        //         fims* p_fims, asset* ai);

        //         // // Reports:

        //     }
        // }
        // namespace defaultRun_module;

        // int defaultFault_module::ShutdownSystem(varsmap &vmap, varmap &amap,
        // const char* aname, fims* p_fims, asset*am);  int
        // defaultRun_module::StartupSystem(varsmap &vmap, varmap &amap, const char*
        // aname, fims* p_fims, asset*am);

        if (0)
            FPS_ERROR_PRINT("%s MANAGER LOOP %s CheckComms\n", __func__, am->name.c_str());
        if (1)
        {
            // Run the Monitor System
            // CheckAmComms(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);
            // UpdateSysTime(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);
            // CheckAmHeartbeat(*am->vmap, am->amap, am->name.c_str(), am->p_fims,
            // am); CheckAmTimestamp(*am->vmap, am->amap, am->name.c_str(),
            // am->p_fims, am); CheckEssStatus(*am->vmap, am->amap, am->name.c_str(),
            // am->p_fims, am); SetupLimitsVec(*am->vmap, am->amap, avmap,
            // am->name.c_str(), "MaxCellCurrent", am); SetupLimitsVec(*am->vmap,
            // am->amap, avmap, am->name.c_str(), "MaxCellVolt", am);
            // CheckOverLimitsVec(*am->vmap, avmap, am->name.c_str(),
            // "MaxCellCurrent", am->p_fims, am); CheckOverLimitsVec(*am->vmap, avmap,
            // am->name.c_str(), "MaxCellVolt", am->p_fims, am);

            // //CheckMonitorVar(am, "MaxCellTemperature");
            // CheckMonitorVar(am, "MaxCellTemperature", "temperature");
            // CheckMonitorVar(am, "MinCellTemperature", "temperature");
            // CheckMonitorVar(am, "MaxCellVolt", "voltage");
            // CheckMonitorVar(am, "MinCellVolt", "voltage");
            // CheckMonitorVar(am, "SOC");
            // CheckMonitorVar(am, "SOH");

            // using namespace defaultFault_module;
            // if(1) UpdateSysTime(*am->vmap, am->amap,  am->name.c_str(), am->p_fims,
            // am);//test/test_walker.cpp: if(0) HandleAssetHeartbeat(*am->vmap,
            // am->amap,  am->name.c_str(), am->p_fims, am);//test/test_walker.cpp:
            // SimHandleHeartbeat(*am->vmap, *am->getAmap(), am->name.c_str(),
            // am->p_fims, am);//test/test_walker.cpp:
            defaultFault_module::ShutdownSystem(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);

            // ??
            defaultRun_module::StartupSystem(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);

            // int CheckAmComms(varsmap &vmap, varmap &amap, const char* aname, fims*
            // p_fims, asset *am)

            // these will override CommsOK or HeartBeatOk for individual units.

            // CheckCommsOk(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);
            // CheckHeartBeatOk(*am->vmap, am->amap, am->name.c_str(), am->p_fims,
            // am);

            // HandleESSCmd(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);

            // //CheckOverLimitsVec(*am->vmap, avmap, am->name.c_str(),
            // "MaxBusBarCurrent", am->p_fims,

            // // main output command handler
            // SendAssetCmd(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);
            // TestAssetCmdAm(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);
        }
        // CheckHeartBeat(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);

        if (0)
            std::cout << am->name << " >>get_state\n";
        // Sends out heartbeats and TODAY
        // HandleESSHeartBeat(*am->vmap, am->amap, "ess", am->p_fims, am);

        // This will set/clear faults
        // CheckStatus(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);

        // This looks got the stop/go and contactor commands
        // RunCmd(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);

        // this provides a report on the system Status
        // ReportStatus(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);

        if (0)
            std::cout << am->name << "  >>process_cmds\n";
        // HandleCmd(*am->vmap, am->amap, am->p_fims);
        if (0)
            std::cout << am->name << "  >>process_power\n";
        // HandleEMSChargeL1(*am->vmap, am->amap, "ess", am->p_fims);
        // TODO fix up asset manager
        // now get the Managed assets and wake them all up

        // now get all the managed assets , wake those up too
    }

    if (wakeup == WAKE_LEVEL3)
    {
        if (0)
            std::cout << am->name << " >>HandlePower\n";
        HandlePower(*am->vmap, am->amap, "ess", am->p_fims, am);
        if (0)
            std::cout << am->name << "  >>process_cmds\n";
        // HandleCmd(*am->vmap, am->amap, am->p_fims);
        if (0)
            std::cout << am->name << "  >>process_power\n";
        // HandlePower(*am->vmap, am->amap, am->p_fims);
        // TODO fix up asset manager
        // now get the Managed assets and wake them all up

        // now get all the managed assets , wake those up too
    }
    if (wakeup == WAKE_LEVEL_MANAGE)
    {
        am->vm->setTime();
        // TODO Manage functions
    }
    if (wakeup == WAKE_LEVEL_PUB)
    {
        if (0)
            FPS_ERROR_PRINT("%s >> aname [%s] >> publish status\n", __func__, am->name.c_str());
        // ess_manager * ess = (ess_manager*)am;
        // asset_manager* bm2 = am->getManAsset("bms");

        // note this assumes that the bms is running threaded
        // let the builtin functions do this bms ma not be running a manager thread.
        // bm2->wakechan.put(wakeup);
        // publ
        // this sends them all out.. may not be cool
        // TODO use the vecMap when it's ready
        ////TODO
        // TODOTODO
        // const char* pname ="Unknown1";
        // const char* res = "Before pub";
        // am->p_fims->Send("pub", pname, nullptr, res);

        // so who added links/ess  and config/ess to the pmap
        // Pubs is the list
        // if(0)vm->showvecMap(vecs);
        // todo Tidy all this up
        // std::vector<std::string>* vx = am->vm->vecMapGetVec(*am->vecs, "Pubs",
        // std::string("dummy"));
        // int idx = 0;
        // if (vx)
        // {
        //     for (auto ix : *vx)
        //     {
        //         FPS_ERROR_PRINT("%s key [%s] > entry [%d] is [%s]\n",
        //         __func__, key, idx++, ix.c_str());
        //     }
        // }
        // am->vm->vListPartialSendFims(*am->vmap, *vx, "pub", am->p_fims);
        // HACK HACK HACK
        // this is for the UI
        for (auto sv : sysVec)
        {
            std::string auri = "/assets/";
            auri += sv;
            // printf(" sysVec -> [%s]\n",auri.c_str());
            if (1)
                dummyPub(am, auri.c_str(), auri.c_str());
        }
        // HACK HACK HACK
        // this is for the site manager
        dummyPub(am, "/site/ess", "/site/ess");
        dummyPub(am, "/site/ess_hs", "/site/ess_hs");
        // dummyPub(am, "/assets/pcs/pcs_1", "/assets/pcs/pcs_1");
        // dummyPub(am, "/assets/bms/summary", "/assets/bms/summary");
        // dummyPub(am, "/assets/bms/bms_1", "/assets/bms/bms_1");
        // dummyPub(am, "/assets/bms/bms_2", "/assets/bms/bms_2");
        // dummyPub(am, "/assets/bms/bms_3", "/assets/bms/bms_3");
        // dummyPub(am, "/assets/bms/bms_4", "/assets/bms/bms_4");

        // void dummyPub(asset_manager *am, const char *uri)
        // void dummyPub(asset_manager *am, const char *uri)
        // {

        //     if (0)FPS_ERROR_PRINT("%s >>>>>>>Publish fims  Start \n",
        //     __func__); cJSON* cjp =
        //     am->vm->getMapsCj(*am->vmap, uri, nullptr, 0x0100);
        //     // if(1)FPS_ERROR_PRINT("%s >> aname [%s] << publish
        //     status\n",__func__, am->name.c_str());
        //     // pname ="Unknown2";
        //     // res = "After pub";
        //     // // cJSON* cjbm = am->getConfig();
        //     // // if(0)printf("%s >>>>>>>Publish cjbm %p\n",
        //     __func__, (void
        //     *)cjbm); char* res = cJSON_PrintUnformatted(cjp); if (0)
        //     FPS_ERROR_PRINT("%s >>>>>>>Publish fims  res >>%s<<\n",
        //     __func__, res); cJSON_Delete(cjp);
        //     // // // TODO fix name
        //     am->p_fims->Send("pub", "/assets", nullptr, res);
        //     free((void*)res);
        // }

        // break;
    }
    // NOT using this lot Yet
    if (am->msgchan.get(item3, false))
    {
        // sendTestMessage(p_fims, tnum);
        if (0)
            std::cout << am->name << "  >>item3-> data  " << item3 << " tnum :" << am->tnum << "\n";
        am->tnum++;
        // sendTestMessage(p_fims, tnum);
        if (0)
            std::cout << am->name << "  >>item3-> data  " << item3 << " tnum :" << am->tnum << "\n";
        am->tnum++;
        free((void*)item3);
    }

    // this gets in fims messages
    if (am->fimschan.get(msg, false))
    {
        if (0)
            FPS_ERROR_PRINT(
                " %s >> >>>>>>>>>>>  %2.3f  %s  >>>>BEFORE FIMS MESSAGE  "
                "method [%s] replyto[%s]  uri [%s]\n",
                __func__, vm->get_time_dbl(), am->name.c_str(), msg->method, msg->replyto ? msg->replyto : "No Reply",
                msg->uri);
        // we need to collect responses
        // cJSON *cj = nullptr;
        // either here of the bms instance
        // bms_man->vmap
        am->vm->sysVec = &sysVec;
        am->vm->runFimsMsg(*am->vmap, msg, am->p_fims);  // fims* p_fims)
        if (0)
            FPS_ERROR_PRINT(" %s >> >>>>>>>>>>>  %2.3f  %s  <<<<AFTER FIMS MESSAGE \n", __func__, vm->get_time_dbl(),
                            am->name.c_str());
    }
    bool foo = true;
    // if(wakeup != 0)
    // {
    //     if(1)FPS_ERROR_PRINT("%s >>>>>>>>%s Manager Loop >>>>>>>>>>>  looking
    //     for the kids wakeup %d\n",__func__,
    //     am->name.c_str(), wakeup); foo = am->runChildren(wakeup);
    //     if(1)FPS_ERROR_PRINT("%s >>>>>>>>%s Manager Loop
    //     >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> done with kids wakeup
    //     %d\n",__func__, am->name.c_str(), wakeup);
    // }
    return foo;
}

bool run_bms_asset_wakeup(asset* am, int wakeup)
{
    if (0)
        FPS_ERROR_PRINT("%s %2.3f >>>>>>>>>>>>>>>>>>>>> running for %s Asset wval %d \n", __func__,
                        am->vm->get_time_dbl(), am->name.c_str(), wakeup);
    // FPS_ERROR_PRINT("%s >>>>>>>>>BMS>>>>>>>>>>> running for BMS Asset
    // [%s]\n",__func__,ass->name.c_str());
    // WAKE_LEVEL1
    if (wakeup == WAKE_LEVEL1)
    {
        // CheckAssetComms(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);
        //            varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,
        //            asset *am)
        // HandleBMSAssetHeartBeat(*am->vmap, am->amap, am->name.c_str(),
        // am->p_fims, am);  HandleBMSChargeL2(varsmap &vmap, varmap &amap, const
        // char*aname, fims* p_fims)
    }
    else if (wakeup == WAKE_LEVEL2)
    {
        HandleBMSChargeL2(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);
    }

    return true;
}

// this one assumes channels are working..
bool run_bms_wakeup(asset_manager* am, int wakeup)
{
    if (0)
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
        // HandleHeartBeat(*am->vmap, am->amap, "bms", am->p_fims);
        // HandleHeartBeat(vmap, *amap, p_fims);
        // std::cout << " BMS >>process_cmds\n";
        // HandleCmd(vmap, *amap, p_fims);
        // std::cout << " BMS >> process_power\n";
        // HandlePower(vmap, *amap, p_fims);
    }

    if (wakeup == WAKE_LEVEL_MANAGE)
    {
        am->vm->setTime();
        // TODO Manage functions
    }

    if (wakeup == 2)
    {
        if (0)
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
        // cJSON *cj = nullptr;
        // either here of the bms instance
        // bms_man->vmap
        am->vm->runFimsMsg(*am->vmap, msg,
                           am->p_fims);  // fims* p_fims)
                                         // am->vm->processFims(*am->vmap,  msg, &cj);
                                         // if (cj)
                                         // {
                                         //     if(msg->replyto)
                                         //     {
                                         //         char* tmp = cJSON_PrintUnformatted(cj);
                                         //         if(tmp)
                                         //         {
                                         //             am->p_fims->Send("set",msg->replyto, nullptr, tmp);
                                         //             free((void*)tmp);
                                         //         }
                                         //     }
                                         // }
                                         // am->p_fims->free_message(msg);
                                         // free((void *) item3);
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
    if (0)
        FPS_ERROR_PRINT("%s %2.3f >>>>>>>>>>>>>>>>>>>>> running for %s Asset wval %d \n", __func__,
                        am->vm->get_time_dbl(), am->name.c_str(), wakeup);
    // FPS_ERROR_PRINT("%s >>>>>>>>>BMS>>>>>>>>>>> running for BMS Asset
    // [%s]\n",__func__,ass->name.c_str());
    // WAKE_LEVEL1
    if (wakeup == WAKE_LEVEL1)
    {
        // CheckAssetComms(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);
        //            varsmap &vmap, varmap &amap, const char* aname, fims* p_fims,
        //            asset *am)
        // HandleBMSAssetHeartBeat(*am->vmap, am->amap, am->name.c_str(),
        // am->p_fims, am);  HandleBMSChargeL2(varsmap &vmap, varmap &amap, const
        // char*aname, fims* p_fims)
    }
    else if (wakeup == WAKE_LEVEL2)
    {
        // HandleBMSChargeL2(*am->vmap, am->amap, am->name.c_str(), am->p_fims);
    }

    return true;
}

// this one assumes channels are working..
bool run_pcs_wakeup(asset_manager* am, int wakeup)
{
    if (0)
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
        // HandleHeartBeat(*am->vmap, am->amap, "bms", am->p_fims);
        // HandleHeartBeat(vmap, *amap, p_fims);
        // std::cout << " BMS >>process_cmds\n";
        // HandleCmd(vmap, *amap, p_fims);
        // std::cout << " BMS >> process_power\n";
        // HandlePower(vmap, *amap, p_fims);
    }

    if (wakeup == 2)
    {
        if (0)
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
    if (wakeup == WAKE_LEVEL_MANAGE)
    {
        am->vm->setTime();
        // TODO Manage functions
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
        std::cout << " PCS >>fims_msg uri " << msg->uri << "\n";
        // we need to collect responses
        // cJSON *cj = nullptr;
        // either here of the bms instance
        // bms_man->vmap
        am->vm->runFimsMsg(*am->vmap, msg,
                           am->p_fims);  // fims* p_fims)
                                         // am->vm->processFims(*am->vmap,  msg, &cj);
                                         // if (cj)
                                         // {
                                         //     if (msg->replyto)
                                         //     {
                                         //         char* tmp = cJSON_PrintUnformatted(cj);
                                         //         if(tmp)
                                         //         {
                                         //             am->p_fims->Send("set",msg->replyto, nullptr, tmp);
                                         //             free((void*)tmp);
                                         //         }
                                         //     }
                                         // }
                                         // am->p_fims->free_message(msg);
                                         // free((void *) item3);
    }
    bool foo = true;
    // if (wakeup!=0)
    // {
    //     foo = am->runChildren(wakeup);
    // }
    return foo;
}

int dummy_system_fault(varsmap& vmap, assetVar* av)
{
    FPS_ERROR_PRINT(" %s >> running \n", __func__);
    return 0;
}

// int  dummy_bms_fault(varsmap& vmap, assetVar* av)
// {
//     if (av)
//     {
//         char* dest;
//         asprintf(&dest, "%s:faults", av->comp.c_str());
//         char* msg;
//         asprintf(&msg, "%s alarm generated [%s]", av->name.c_str(),
//         av->aVal->valuestring ? av->aVal->valuestring : "No Data");
//         av->am->vm->sendAlarm(vmap, av->am->name.c_str(), dest, nullptr, msg,
//         2); FPS_ERROR_PRINT(" %s >> dest [%s] msg [%s]  \n",
//         __func__, dest, msg);

//         if (dest)free((void*)dest);
//         if (msg)free((void*)msg);
//     }
//     else
//     {
//         FPS_ERROR_PRINT(" %s >> running \n", __func__);
//     }
//     return 0;
// }
// still all a bit flaky
// query with  /usr/local/bin/fims/fims_send -m get -r /$$ -u
// /ess/full/assets/bms/summary/alarms | jq set with
// /usr/local/bin/fims/fims_send -m pub -u /components/catl_mbmu_sum_r
// '{"mbmu_warning_11": 1234}' and this .. fims_send -m set -r /$$ -u
// /ess/alarms/bms/alarms '"Clear"'
int dummy_bms_fault(varsmap& vmap, assetVar* av)
{
    if (1)
        FPS_ERROR_PRINT(" %s >>  av %p  av->am %p \n", __func__, (void*)av, av ? (void*)av->am : nullptr);
    if (av)
    {
        char* dest = nullptr;
        char* msg = nullptr;
        char* avVal = av->getcVal() ? av->getcVal() : (char*)"noval";
        char* avLVal = av->getcLVal() ? av->getcLVal() : (char*)"noLval";
        VarMapUtils* vm = av->am->vm;

        if (1)
            FPS_ERROR_PRINT(" %s >>  name [%s] value [%s]  lastValue[%s] \n", __func__, av->name.c_str(), avVal,
                            avLVal);

        char* almsg = av->getcVal() ? av->getcVal() : (char*)"Normal";

        if (almsg && (strcmp(almsg, "Clear") == 0))
        {
            asprintf(&dest, "/assets/%s/summary:faults", av->am->name.c_str());
            vm->clearAlarms(vmap, dest);
            if (1)
                FPS_ERROR_PRINT(" %s >> Clear Faults dest [%s]   \n", __func__, dest);
        }
        else if (strcmp(avVal, avLVal) != 0)
        {
            av->setLVal((const char*)avVal);

            double tNow = vm->get_time_dbl();
            asprintf(&dest, "/assets/%s/summary:faults", av->am->name.c_str());
            asprintf(&msg, "%s fault  [%s] at %2.3f ", av->name.c_str(), almsg, tNow);
            {
                if (av->am && av->am->vm)
                {
                    vm->sendAlarm(vmap, av, dest, nullptr, msg, 2);
                    if (1)
                        FPS_ERROR_PRINT(" %s >> Fault Sent dest [%s] msg [%s]  am %p \n", __func__, dest, msg,
                                        (void*)av->am);
                }
            }
            // av->am->vm->sendAlarm(vmap, "smbu", dest, nullptr, msg, 2);
            if (0)
                FPS_ERROR_PRINT(" %s >> dest [%s] msg [%s]  am %p \n", __func__, dest, msg, (void*)av->am);
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
// /ess/alarms/bms/alarms '"Clear"'
int dummy_bms_alarm(varsmap& vmap, assetVar* av)
{
    if (0)
        FPS_ERROR_PRINT(" %s >>  av %p  av->am %p \n", __func__, (void*)av, av ? (void*)av->am : nullptr);
    if (av)
    {
        char* dest = nullptr;
        char* msg = nullptr;
        char* avVal = av->getcVal() ? av->getcVal() : (char*)"noval";
        char* avLVal = av->getcLVal() ? av->getcLVal() : (char*)"noLval";
        VarMapUtils* vm = av->am->vm;

        if (1)
            FPS_ERROR_PRINT(" %s >>  name [%s] value [%s]  lastValue[%s] \n", __func__, av->name.c_str(), avVal,
                            avLVal);

        char* almsg = av->getcVal() ? av->getcVal() : (char*)"Normal";

        if (almsg && (strcmp(almsg, "Clear") == 0))
        {
            // TODO add this to config
            asprintf(&dest, "/assets/%s/summary:alarms", av->am->name.c_str());
            vm->clearAlarms(vmap, dest);
            if (1)
                FPS_ERROR_PRINT(" %s >> ClearAlarms dest [%s]   \n", __func__, dest);
        }
        else if (strcmp(avVal, avLVal) != 0)
        {
            av->setLVal((const char*)avVal);

            double tNow = vm->get_time_dbl();
            asprintf(&dest, "/assets/%s/summary:alarms", av->am->name.c_str());
            asprintf(&msg, "%s alarm  [%s] at %2.3f ", av->name.c_str(), almsg, tNow);
            {
                if (av->am && av->am->vm)
                {
                    vm->sendAlarm(vmap, av, dest, nullptr, msg, 2);
                    if (1)
                        FPS_ERROR_PRINT(" %s >> AlarmSent dest [%s] msg [%s]  am %p \n", __func__, dest, msg,
                                        (void*)av->am);
                }
            }
            // av->am->vm->sendAlarm(vmap, "smbu", dest, nullptr, msg, 2);
            if (0)
                FPS_ERROR_PRINT(" %s >> dest [%s] msg [%s]  am %p \n", __func__, dest, msg, (void*)av->am);
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

// the main ess_controller start up
int main(int argc, char* argv[])
{
    varsmap vmap;
    varsmap pmap;
    varmap dummyvmap;
    // varMapUtils vm;

    // TODO make these more flexible
    system("mkdir -p run_configs");
    system("mkdir -p run_logs");

    // TODO make this an asset manager
    // ess_man = new asset_manager("ess_controller");
    ess_man = new asset_manager("ess");
    ess_man->am = nullptr;
    ess_man->running = 1;
    vm->sysVec = &sysVec;

    vm->setFunc(vmap, "comp", "/fault", (void*)&dummy_system_fault);
    vm->setFunc(vmap, "comp", "/faults/bms", (void*)&dummy_bms_fault);
    vm->setFunc(vmap, "comp", "/alarms/bms", (void*)&dummy_bms_alarm);

    vm->setFunc(vmap, "ess", "run_init", (void*)&run_ess_init);
    vm->setFunc(vmap, "ess", "run_wakeup", (void*)&run_ess_wakeup);
    vm->setFunc(vmap, "bms", "run_init", (void*)&run_bms_init);
    vm->setFunc(vmap, "bms", "run_wakeup", (void*)&run_bms_wakeup);
    vm->setFunc(vmap, "bms", "run_asset_init", (void*)&run_bms_asset_init);
    vm->setFunc(vmap, "bms", "run_asset_wakeup", (void*)&run_bms_asset_wakeup);

    vm->setFunc(vmap, "pcs", "run_init", (void*)&run_pcs_init);
    vm->setFunc(vmap, "pcs", "run_wakeup", (void*)&run_pcs_wakeup);
    vm->setFunc(vmap, "pcs", "run_asset_init", (void*)&run_pcs_asset_init);
    vm->setFunc(vmap, "pcs", "run_asset_wakeup", (void*)&run_pcs_asset_wakeup);

    vm->setFunc(vmap, "dcr", "run_init", (void*)&run_bms_init);
    vm->setFunc(vmap, "dcr", "run_wakeup", (void*)&run_bms_wakeup);
    vm->setFunc(vmap, "dcr", "run_asset_init", (void*)&run_bms_asset_init);
    vm->setFunc(vmap, "dcr", "run_asset_wakeup", (void*)&run_bms_asset_wakeup);

    // void (*tf)(void *) = (void (*tf)(void *))
    void* res1 = vm->getFunc(vmap, "ess", "run_init");
    void* res2 = vm->getFunc(vmap, "ess", "run_wakeup");
    void* res3 = vm->getFunc(vmap, "comp", "/fault/bms");

    typedef void (*myCompFcn_t)(varsmap & vm, assetVar * data);
    // void (*tf)(void *) = (void (*tf)(void *))(res3);

    if (res3)
    {
        myCompFcn_t myCompFcn = myCompFcn_t(res3);

        myCompFcn(vmap, nullptr);
    }

    myAmInit_t myessMinit = myAmInit_t(res1);
    myAmWake_t myessMwake = myAmWake_t(res2);

    // ess_man->run_init = run_ess_init;
    // ess_man->run_wakeup = run_ess_wakeup;
    ess_man->run_init = myessMinit;
    ess_man->run_wakeup = myessMwake;

    ess_man->setVmap(&vmap);
    ess_man->setPmap(&pmap);  // pubs map
    ess_man->running = 1;
    ess_man->reload = 0;

    amap = ess_man->getAmap();  // may not need this

    // TODO this may be redundant
    // ess_man->setVmap(&vmap);

    // right this thing is ready to go
    // Just for kicks read in the config file and see if it had our links in it
    // this should be the case after the first run.
    // printf(" Getting initial configuration\n");

    const char* cfgname = "configs/test_ess_config.json";

    FPS_ERROR_PRINT("%s >> Getting initial configuration   from [%s]\n", __func__, cfgname);

    vm->configure_vmap(vmap, cfgname, nullptr, ess_man);
    // ess_man->configure(cfgname , nullptr);

    {
        // this a test for our config.
        cJSON* cjbm = nullptr;
        cjbm = vm->getMapsCj(vmap);
        const char* fname = "run_configs/ess_1_at_start.json";
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
    // bit ugly at the moment .. its a copy of the asset config

    if (0)
    {
        // this a test for our config.
        cJSON* cjbm = nullptr;
        cjbm = vm->getMapsCj(vmap);
        const char* fname = "run_configs/ess_2_after_links.json";
        vm->write_cjson(fname, cjbm);
        // cJSON* cjbm = ess_man->getConfig();
        char* res = cJSON_Print(cjbm);
        // printf("Maps (possibly) with links at beginning  cjbm %p \n%s\n <<<
        // done\n", (void *)cjbm, res);
        free((void*)res);
        cJSON_Delete(cjbm);
    }
    // no dont do this now
    // ess_man->setup(vmap, "/links/ess");
    // ess_man->cfgwrite("run_configs/ess_after_links.json");

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

    FPS_ERROR_PRINT("%s >>ess test OK so far\n", __func__);
    // this is a low level configure with no subs.
    // ess_man->configure("configs/test_ess.json");
    // Pmap was a publish Map
    // {
    //     cJSON* cj = vm->getMapsCj(*ess_man->pmap);
    //     char* res = cJSON_Print(cj);
    //     printf("ESS >>Pmap before config \n%s\n", res);
    //     free((void*)res);
    //     cJSON_Delete(cj);
    // }
    // else
    // {
    //     printf("ESS >>Pmap before run skipped\n");

    // }
    // todo mo
    cfgname = "configs/test_ess.json";  // TODO move this into asset manager assconfig
    // TODO skip this
    ess_man->configure(&vmap, cfgname, "ess", &sysVec, nullptr, ess_man);

    // HACK HACK HACK
    // sysvec orders the comps for pub to UI interface
    if (sysVec.size() == 0)
    {
        // sysVec->push_back(aname);
        // HACK for UI
        std::string sname = "ess";
        sname += "/summary";
        sysVec.push_back(sname);
        FPS_ERROR_PRINT(" %s >> addded [%s] to sysVec size %d \n", __func__, sname.c_str(), (int)sysVec.size());
    }
    if (1)
        FPS_ERROR_PRINT(" %s >> ess_man >> sysVec size [%d]\n", __func__, (int)sysVec.size());

    // This loads in the system_controller interface
    // cJSON* cjsite = cJSON_GetObjectItem(cj, "/site/ess");
    auto ixs = vmap.find("/site/ess");
    if (ixs != vmap.end())
    {
        for (auto iy : ixs->second)
        {
            if (iy.second->name == "ess")
            {
                char* cfgname = iy.second->getcVal();  // aVal->valuestring;
                cJSON* cj = vm->get_cjson(cfgname);

                FPS_ERROR_PRINT(
                    "%s >> site_ess lets run site config for  [%s] from "
                    "[%s] file [%s] cj %p\n",
                    __func__, iy.first.c_str(), iy.second->name.c_str(), cfgname, cj);
                // this is for the site interface
                if (cj)
                    vm->loadSiteMap(vmap, cj);
            }
        }
    }
    else
    {
        if (1)
            FPS_ERROR_PRINT(" %s >> ess_man >> no site interface found \n", __func__);
    }
    // ess_man->configure(cfgname , (char *)"ess");
    // no worries at this stage

    // this sets up the amap for bss_running vars not needed if the links are in
    // the config....
    // ess_man->initLinks();

    // if (ess_man->pmap)
    // {
    //     cJSON* cj = vm->getMapsCj(*ess_man->pmap);
    //     char* res = cJSON_Print(cj);
    //     printf("ESS >>Pmap after config \n%s\n", res);
    //     free((void*)res);
    //     cJSON_Delete(cj);
    // }
    // else
    // {
    //     printf("ESS >>Pmap before run skipped\n");

    // }
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

    // the timer generates  channel wakes every 100 mS
    ess_man->run_timer(100);

    // run the message loop eery 1.5 seconds

    // the message  generates test message  channel wakes every 1500 mS
    ess_man->run_message(1500);
    int ccnt = 0;
    char** subs2 = nullptr;
    vecmap vecs;
    ess_man->vecs = &vecs;

    // TODO all weuse are the Subs at the moment
    vm->getVList(vecs, vmap, ess_man->amap, "ess", "Blocks", ccnt);

    // char**subs3 = nullptr;
    ccnt = 0;
    vm->getVList(vecs, vmap, ess_man->amap, "ess", "Pubs", ccnt);
    vm->getVList(vecs, vmap, ess_man->amap, "ess", "Subs", ccnt);

    // subs3 = vm->getList(*ess_man->vecs, vmap, ess_man->amap, "ess", "Pubs",
    // ccnt);

    FPS_ERROR_PRINT("%s >> >>>>found %d  Pubs in config file here they are:\n", __func__, ccnt);
    // vm->showList(subs3, "ess", ccnt);
    // vm->addListToVec(vecs, subs3, "Pubs", ccnt);

    FPS_ERROR_PRINT("%s >> >>>>looking at ess_man->vecs ccnt %d\n", __func__, ccnt);

    vm->showvecMap(*ess_man->vecs);

    FPS_ERROR_PRINT("%s >> <<<<<<<<<<<<<<end of Pubs\n", __func__);

    subs2 = vm->getList(vecs, vmap, ess_man->amap, "ess", "Subs", ccnt);
    if (!subs2)
    {
        subs2 = vm->getDefList(vecs, vmap, ess_man->amap, "ess", "Subs", ccnt);
    }
    FPS_ERROR_PRINT("%s >> found %d  subs in config file here they are:\n", __func__, ccnt);
    vm->showList(subs2, "ess", ccnt);
    FPS_ERROR_PRINT("%s >> <<<<<<<<<<<<<<end of subs\n", __func__);

    FPS_ERROR_PRINT("%s >>  >>>>>>>>>>>>>start of vecs\n", __func__);
    vm->showvecMap(vecs);
    FPS_ERROR_PRINT("%s >> <<<<<<<<<<<<<<end of vecs\n", __func__);

    // the fims system will get pubs and create a varsMap for  the items.
    // TODO restrict fims to known components

    // this will start serviceing the incoming fims messages but , as of yet , no
    // one is listening.
    ess_man->run_fims(1500, (char**)subs2, "essMan", ccnt);

    // if(ccnt > 0)
    // {
    //   vm->ClearSubs(subs2,"ess", ccnt);
    // }
    // the manager runs the channel receiver thread
    if (ess_man->pmap)
    {
        cJSON* cj = vm->getMapsCj(*ess_man->pmap);
        char* res = cJSON_Print(cj);
        printf("ESS >>Pmap before manager \n%s\n", res);
        free((void*)res);
        cJSON_Delete(cj);
    }
    else
    {
        printf("ESS >>Pmap before run skipped\n");
    }

    // // this will run the manager
    // ess_man->run_manager(p_fims);

    // now we try to create and configure the assets.
    // vmap /assets/ess contains the goodies.
    // do it the long way first
    // vmap["/links/bms_1"]
    asset_manager* ass_man = nullptr;

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
                if (iy.first == "bms" || iy.first == "pcs")

                {
                    const char* aname = iy.first.c_str();
                    printf(" setting up a %s manager\n", aname);
                    // TODO get the names from the config file...
                    ass_man = new asset_manager(aname);
                    void* res3 = vm->getFunc(vmap, aname, "run_init");
                    void* res4 = vm->getFunc(vmap, aname, "run_wakeup");
                    // void *res5 = vm->getFunc(vmap, aname,"run_asset_init" );
                    void* res6 = vm->getFunc(vmap, aname, "run_asset_wakeup");
                    myAmInit_t myMinit = myAmInit_t(res3);
                    myAmWake_t myMwake = myAmWake_t(res4);
                    // myAssInit_t mybmsAinit = myAssInit_t(res5);
                    myAssWake_t myAwake = myAssWake_t(res6);

                    //              ess_man->run_init = myessMinit;
                    //              ess_man->run_wakeup = myessMwake;

                    ass_man->run_init = myMinit;
                    ass_man->run_wakeup = myMwake;
                    ass_man->p_fims = p_fims;
                    // TODO move to the "real Fault handlers"
                    vm->setAmFunc(vmap, "comp", "/alarms", aname, ass_man, (void*)&dummy_bms_alarm);
                    vm->setAmFunc(vmap, "comp", "/faults", aname, ass_man, (void*)&dummy_bms_fault);

                    ass_man->setVmap(&vmap);
                    ass_man->vm = vm;  //??
                    ass_man->am = ess_man;
                    ass_man->vecs = ess_man->vecs;
                    // TODO add any Pubs into Vecs
                    ccnt = 0;
                    if (1)
                        FPS_ERROR_PRINT(" %s >> running with vmap [%p]\n", __func__, &vmap);
                    if (1)
                        FPS_ERROR_PRINT(" %s >> sysVec size [%d]\n", __func__, (int)sysVec.size());
                    // now get the asset_manager to configure itsself
                    ass_man->configure(&vmap, fname, aname, &sysVec, myAwake, ass_man);

                    int ccntam = 0;
                    vm->getVList(*ess_man->vecs, vmap, ass_man->amap, aname, "Pubs", ccntam);
                    printf("%s >> setting up a %s manager pubs found %d \n", __func__, aname, ccntam);

                    // add it to the assetList
                    ess_man->addManAsset(ass_man, aname);

                    printf(" done setting up a %s manager varsmap must be fun now\n", aname);
                    // we should be able to do things like get status from the
                    // bms_manager. first see it it unmaps correctly.
                    asset_manager* bm2 = ess_man->getManAsset(aname);
                    if (bm2)
                    {
                        printf(" @@@@@@@  found %s asset manager with %d assets\n", aname, bm2->getNumAssets());
                    }
                    ass_man->running = 1;
                    ass_man->setupChannels();
                    // bms_man->run_timer(500);
                    // bms_man->run_message(500);

                    // bms_man->run_manager(p_fims); // DO we need this
                    // bms_man->man_wakechan.put(2);
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
    // sysVec holds the assets in order

    if (1)
    {
        FPS_ERROR_PRINT("%s >> list of assets\n", __func__);
        for (int i = 0; i < (int)sysVec.size(); i++)
        {
            FPS_ERROR_PRINT("%s >> idx [%d] name [%s]\n", __func__, i, sysVec[i].c_str());
        }
    }

    {
        const char* fname = "run_configs/ess_4_after_assets.json";
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
        // printf("ESS >>Maps before run \n%s\n", res);
        free((void*)res);
        cJSON_Delete(cj);
    }
    else
    {
        FPS_ERROR_PRINT("ESS >>Maps before run skipped\n");
    }
    if (ess_man->pmap)
    {
        cJSON* cj = vm->getMapsCj(*ess_man->pmap);
        char* res = cJSON_Print(cj);
        FPS_ERROR_PRINT("ESS >>Pmap before run \n%s\n", res);
        free((void*)res);
        cJSON_Delete(cj);
    }
    else
    {
        FPS_ERROR_PRINT("ESS >>Pmap before run skipped\n");
    }

    // config all done run the manager
    // set up vlinks
    vm->setVLinks(vmap);
    // config all done run the manager
    // this will run the manager
    ess_man->run_manager(p_fims);

    // NOTE this could collide with the  wake manager thread so lets put it in
    // there
    while (ess_man->running)
    {
        poll(nullptr, 0, 1000);
        run_secs++;
        // if (secs == 30)
        // {
        //     const char* fname = "run_configs/ess_4_after_30 seconds.json";
        //     // this a test for our config with links
        //     cJSON* cjbm = nullptr;
        //     cjbm = vm->getMapsCj(vmap, nullptr, nullptr, 0x0010);
        //     vm->write_cjson(fname, cjbm);
        // }
    }
    FPS_ERROR_PRINT("ESS >> Shutting down\n");
    ass_man->running = 0;

    ess_man->t_data.cthread.join();
    FPS_ERROR_PRINT("ESS >>t_data all done \n");
    ess_man->m_data.cthread.join();
    FPS_ERROR_PRINT("ESS >>m_data all done \n");
    ess_man->f_data.cthread.join();
    FPS_ERROR_PRINT("ESS >>f_data all done \n");

    if (ess_man->vmap)
    {
        cJSON* cj = vm->getMapsCj(*ess_man->vmap);
        char* res = cJSON_Print(cj);
        FPS_ERROR_PRINT("ESS >>Maps at end \n%s\n", res);
        free((void*)res);
        cJSON_Delete(cj);
    }
    else
    {
        FPS_ERROR_PRINT("ESS >>Maps at end skipped\n");
    }

    delete ess_man;

    return 0;
}
#endif
