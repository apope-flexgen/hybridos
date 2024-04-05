/*
 * ess test , opens up the master config and creates all the peripherals
 * bms , pcs , drc
 * Starts all those threads if needed
 * handles the interface between hos ( via modbus server pubs) and hte
 * controllers. It has registers and links. g++ -std=c++11 -g -o ./test_ess -I
 * ./include test/test_ess.cpp -lpthread -lcjson -lfims
 */

#include "asset.h"
#include "varMapUtils.h"

//#include "channel.h"
//#include "monitor.h"
//#include "ess.h"
//#include "bms.h"
#define MAX_FIMS_CONNECT_TRIES 5

int CheckAmHeartbeat(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int CheckAmComms(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);

extern "C" {

int HandlePowerCmd(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
// int HandleBMSChargeL2(varsmap &vmap, varmap &amap, const char* aname, fims*
// p_fims, asset*am); int HandlePowerLimit(varsmap &vmap, varmap &amap, const
// char* aname, fims* p_fims, assetVar*av); int CheckTableVar(varsmap &vmap,
// varmap &amap, const char* aname, fims* p_fims, assetVar* av1, assetVar* av2,
// assetVar* tblAv);
}

assetVar* getLogAv(asset_manager* am, const char* aname, const char* lname)
{
    bool bval = true;
    assetVar* logav = am->lmap[lname];
    if (!logav)
    {
        logav = am->lmap[lname] = am->vm->setLinkVal(*am->vmap, aname, "/perf", lname, bval);
        FPS_ERROR_PRINT("%s  Logging [%s] [%s] data\n", __func__, aname, lname);
        // int ival = 0;
        // std::string vcnt(lname); vcnt += "cnt";
        // logav->setParam(vcnt.c_str(),ival);
    }
    return logav;
}
void runLogAv2(asset_manager* am, assetVar* logav, const char* lname, double tVal)
{
    std::string vtot(lname);
    vtot += "tot";
    std::string vavg(lname);
    vavg += "avg";
    std::string vmax(lname);
    vmax += "max";
    std::string vcnt(lname);
    vcnt += "cnt";
    int ival = logav->getiParam(vcnt.c_str());
    logav->setParam(vcnt.c_str(), ival + 1);
    double dval = logav->getdParam(vtot.c_str());
    logav->setParam(vtot.c_str(), dval + tVal);
    dval = (dval + tVal) / (ival);
    logav->setParam(vavg.c_str(), dval);
    dval = logav->getdParam(vmax.c_str());
    if (tVal > dval)
    {
        logav->setParam(vmax.c_str(), tVal);
    }
}

void runLogAv(asset_manager* am, assetVar* logav, double tNow)
{
    double tTime = (am->vm->get_time_dbl() - tNow) * 1000.0;

    runLogAv2(am, logav, "timemS", tTime);
}

namespace defaultFault_module
{
// For external reference to these functions if another program needs it.
extern "C" {
// TODO: Section these off so they become easier to manage.
// TODO: Make sure they aren't internal/unsafe functions (move them up if they
// are)
int ShutdownSystem(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int ShutdownPCS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int ShutdownBMS(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int ShutdownPCSasset(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int ShutdownBMSasset(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
// int CheckMonitorVar(varsmap& vmap, varmap& amap, const char* aname, fims*
// p_fims, assetVar* av);  int UpdateSysTime(varsmap &vmap, varmap &amap, const
// char* aname, fims* p_fims, assetVar*av);

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
int StartupSystem(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
// void ShutdownPCS(varsmap& vmap, varmap& amap, const char* aname, fims*
// p_fims, asset_manager* am); void ShutdownBMS(varsmap& vmap, varmap& amap,
// const char* aname, fims* p_fims, asset_manager* am); void
// ShutdownPCSasset(varsmap& vmap, varmap& amap, const char* aname, fims*
// p_fims, asset* ai); void ShutdownBMSasset(varsmap& vmap, varmap& amap, const
// char* aname, fims* p_fims, asset* ai);

// // Reports:
}
}  // namespace defaultRun_module

extern "C" {
int CheckMonitorVar(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int UpdateSysTime(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}

//#include "../funcs/CheckMonitorVar.cpp"
//#include "../funcs/UpdateSysTime.cpp"
//
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

//},
// "mbmu_avg_cell_temperature": {
//   "value": "/components/catl_mbmu_control_r:mbmu_avg_cell_temperature"

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
// varsmap funMap;

// vm->funMap = &funMap;

#if 1
// // #include "../test/release_fcns.cpp"
// // #include "../test/test_release_fcns.cpp"
// #include "../test/test_phil_fcns.cpp"
//#include "../funcs/CheckAmHeartbeat.cpp"
// #include "../funcs/CheckAmTimestamp.cpp"
//#include "../funcs/CheckMonitorVar.cpp"
//#include "../funcs/HandlePowerLimit.cpp"
//#include "../funcs/CheckAmComms.cpp"
// #include "../funcs/SimHandleHeartbeat.cpp"
// #include "../funcs/HandleAssetHeartbeat.cpp"
//#include "../funcs/UpdateSysTime.cpp"
//#include "../funcs/module_runFuncs.cpp"
//#include "../funcs/module_faultFuncs.cpp"
// #include "../funcs/CheckEssStatus.cpp"

// these dont relocate nicely
////#include "../src/AlarmFaultHandler.cpp"

extern "C" {
int process_alarm(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int process_fault(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int process_status(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int process_sys_alarm(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
int HandleCpuStats(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av);
}

int HandleCpuStats(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* av)
{
    double package = 0.0;
    double core0 = 0.0;
    double core1 = 0.0;
    double core2 = 0.0;
    double core3 = 0.0;
    int currRealMem;
    int peakRealMem;
    int currVirtMem;
    int peakVirtMem;

    getTemps(&package, &core0, &core1, &core2, &core3);
    getMemory(&currRealMem, &peakRealMem, &currVirtMem, &peakVirtMem);
    vm->setVal(vmap, "/status/ess", "currRealMem", currRealMem);
    vm->setVal(vmap, "/status/ess", "peakRealMem", peakRealMem);
    vm->setVal(vmap, "/status/ess", "currVirtMem", currVirtMem);
    vm->setVal(vmap, "/status/ess", "peakVirtMem", peakVirtMem);

    vm->setVal(vmap, "/status/ess", "system_temp", package);
    vm->setVal(vmap, "/status/ess", "core0_temp", core0);
    vm->setVal(vmap, "/status/ess", "core1_temp", core1);
    vm->setVal(vmap, "/status/ess", "core2_temp", core2);
    vm->setVal(vmap, "/status/ess", "core3_temp", core3);

    cJSON* cj;
    cj = cJSON_CreateObject();
    cJSON_AddStringToObject(cj, "value", GITBRANCH);
    vm->setValfromCj(vmap, "/status/ess", "git_branch", cj);
    cJSON_Delete(cj);
    // char *tmp;
    // asprintf(&tmp, "%s",GITBRANCH);
    // vm->setVal(vmap, "/build/ess","git_branch",(const char*)GITBRANCH);
    // free(tmp);
    cj = cJSON_CreateObject();
    cJSON_AddStringToObject(cj, "value", GITCOMMIT);
    vm->setValfromCj(vmap, "/status/ess", "git_commit", cj);
    cJSON_Delete(cj);
    // free(tmp);

    return 0;
}

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
        FPS_ERROR_PRINT(">>>>>>>>>ESS>>>>>>>>>>>%s running for ESS Manager vm %p \n", __func__, am->vm);
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
        ival = 500;
        am->amap["RunPub"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/control", "RunPub", bval);
        am->amap["PubTime"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/control", "PubTime", ival);
        ival = 50;
        am->amap["hsPubTime"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/control", "hsPubTime", ival);
        ival = 0;
        am->amap["RunWake100"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/control", "RunWake100", bval);
        am->amap["RunWake1000"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/control", "RunWake1000", bval);
        // TODO remove hard code
        am->amap["SendPcs"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/control", "SendPcs", bval);
        am->amap["SendBms"] = am->vm->setLinkVal(*am->vmap, am->name.c_str(), "/control", "SendBms", bval);

        am->amap["RunPub"]->setVal(true);
        am->amap["RunWake100"]->setVal(true);
        am->amap["RunWake1000"]->setVal(true);
        am->amap["SendPcs"]->setVal(true);
        am->amap["SendBms"]->setVal(true);

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

/**
 * @brief Set functions for all assets/asset managers to use
 * Note: only process_sys_alarm is used here. Add additional functions here
 * (perhaps there's a better way to do this...)
 *
 * @param am the ess controller
 */
void initFuncs(asset_manager* am)
{
    VarMapUtils* vm = am->vm;
    if (1)
        FPS_ERROR_PRINT("%s >> About to set process_sys_alarm for %s\n", __func__, am->name.c_str());
    for (auto& ix : am->assetManMap)
    {
        asset_manager* amc = ix.second;
        if (1)
            FPS_ERROR_PRINT("%s >> Got asset manager %s. Setting func now...\n", __func__, amc->name.c_str());
        vm->setFunc(*am->vmap, amc->name.c_str(), "process_sys_alarm", (void*)&process_sys_alarm);
        // run each of the asst Instances
        for (auto& iy : amc->assetMap)
        {
            asset* ami = iy.second;
            if (1)
                FPS_ERROR_PRINT("%s >> Got asset instance %s. Setting func now...\n", __func__, ami->name.c_str());
            vm->setFunc(*am->vmap, ami->name.c_str(), "process_sys_alarm", (void*)&process_sys_alarm);
        }
    }
}

void dummyPub(asset_manager* am, const char* uri, const char* puri)
{
    if (0)
        FPS_ERROR_PRINT("%s >>>>>>>Publish fims  Start uri [%s]  \n", __func__, uri);
    // return;
    // TODO define opt defs 0x0100  means send for ui
    cJSON* cjp = nullptr;
    cjp = am->vm->getMapsCj(*am->vmap, uri, nullptr, 0x0100);
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
        if (res)
            free(res);
        res = nullptr;
        if (cjp)
            cJSON_Delete(cjp);
        cjp = nullptr;
    }
    else
    {
        if (cjp)
            cJSON_Delete(cjp);
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
            if (res)
                free(res);
            res = nullptr;
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
        free(res);
}

// todo put this into a different varmap like a new lmap
// the /perf prefix  will make sure we dont clash with any other fields  in vmap
// but amap may get clobbered.

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
        static int setup = 0;
        // assetVar *av = am->amap["MaxCellTemperature"];
        if (!setup)
        {
            if (1)
                FPS_ERROR_PRINT("%s MANAGER LOOP %s  setup MonitorVar\n", __func__, am->name.c_str());

            if (0)
                am->vm->setMonitorList(*am->vmap, "bms");
            am->vm->setMonitorList2(*am->vmap, "bms", "wake_monitor");
            am->vm->setMonitorList2(*am->vmap, "pcs", "wake_monitor");
            am->vm->setMonitorList2(*am->vmap, "site", "wake_monitor");
            setup = 1;
            if (1)
                FPS_ERROR_PRINT("%s MANAGER LOOP %s  completed setup MonitorVar\n", __func__, am->name.c_str());

            // double dval = 45.0;
            // assetVar* av =  am->amap["MaxCellTemperature"] =
            // vm->setLinkVal(*am->vmap, "ess", "/status", "MaxCellTemperature",
            // dval); av->am = am; int ival = 10; av->setParam("debug", ival);
            // av->setParam("avPtr", (void*)av);
        }
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
    if (wakeup == WAKE_LEVEL1000)
    {
        bool runMe = true;
        if (am->amap["RunWake1000"])
        {
            runMe = am->amap["RunWake1000"]->getbVal();

            if (!runMe)
                FPS_ERROR_PRINT("%s MANAGER LOOP %s WAKE1000 run_secs %d\n", __func__, am->name.c_str(), run_secs);
        }

        if (am->amap["SendPcs"])
        {
            bool sendPcs = am->amap["SendPcs"]->getbVal();
            asset_manager* am2 = am->getManAsset("pcs");
            if (am2)
            {
                am2->sendOK = sendPcs;
            }
            if (!runMe)
                FPS_ERROR_PRINT("%s MANAGER LOOP %s WAKE1000 sendPcs %s am2 %p [%s]\n", __func__, am->name.c_str(),
                                sendPcs ? "true" : "false", am2, am2->name.c_str());
        }
        if (am->amap["SendBms"])
        {
            bool sendBms = am->amap["SendBms"]->getbVal();
            asset_manager* am2 = am->getManAsset("bms");
            if (am2)
            {
                am2->sendOK = sendBms;
            }
        }
        run_secs++;
        if (run_secs == 30)
        {
            const char* fname = "run_configs/ess_after_30 seconds.json";
            // this a test for our config with links
            cJSON* cjbm = am->vm->getMapsCj(*am->vmap, nullptr, nullptr, 0x10000);
            if (cjbm)
            {
                am->vm->write_cjson(fname, cjbm);
                cJSON_Delete(cjbm);
            }
        }
    }
    if (wakeup == WAKE_LEVEL1)
    {
        am->vm->setTime();

        // if (run_secs == 30)
        // {
        //     run_secs++;
        //     const char* fname = "run_configs/ess_4_after_30 seconds.json";
        //     // this a test for our config with links
        //     cJSON* cjbm = am->vm->getMapsCj(*am->vmap, nullptr, nullptr,
        //     0x10000); if(cjbm)
        //     {
        //         am->vm->write_cjson(fname, cjbm);
        //         cJSON_Delete(cjbm);
        //     }
        // }
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
        // double tNow = 0.0;
        if (am->amap["RunWake100"]->getbVal())
        {
            essPerf ePerf(am, "ess", "sched100mS_Log");
            // assetVar* logav = getLogAv(am, "ess", "wake100Log");

            // bool bval = logav->getbVal();
            // if(bval)
            //     tNow = am->vm->get_time_dbl();

            // Run the Monitor System
            // CheckAmComms(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);
            // int UpdateSysTime(varsmap &vmap, varmap &amap, const char* aname, fims*
            // p_fims, asset_manager* av);  int UpdateSysTime(varsmap &vmap, varmap
            // &amap, const char* aname, fims* p_fims, asset_manager*am);

            // TODO add this to a wake1000 so it runs every second.
            assetVar Av;
            Av.am = am;

            // defaultFault_module::ShutdownSystem(*am->vmap, am->amap,
            // am->name.c_str(), am->p_fims, &Av);

            // // ??
            // defaultRun_module::StartupSystem(*am->vmap, am->amap, am->name.c_str(),
            // am->p_fims, &Av);

            if (1)
                UpdateSysTime(*am->vmap, am->amap, am->name.c_str(), am->p_fims, &Av);
            typedef int (*myAmFunc_t)(varsmap & vmap, varmap & amap, const char* aname, fims* p_fims, assetVar* av);

            void* res1 = vm->getFunc(*am->vmap, "ess", "UpdateSysTime");
            myAmFunc_t myCompFcn = myAmFunc_t(res1);
            if (0)
                FPS_ERROR_PRINT("%s MANAGER LOOP %s UpdateSysTime\n", __func__, am->name.c_str());

            if (0)
                myCompFcn(*am->vmap, am->amap, am->name.c_str(), am->p_fims, &Av);

            // CheckAmHeartbeat(*am->vmap, am->amap, am->name.c_str(), am->p_fims,
            // &Av);  HandlePowerLimit(*am->vmap, am->amap, am->name.c_str(),
            // am->p_fims, &Av);
            HandlePowerCmd(*am->vmap, am->amap, am->name.c_str(), am->p_fims, &Av);

            // CheckAmTimestamp(*am->vmap, am->amap, am->name.c_str(), am->p_fims,
            // am);

            // assetVar* TimestampAv = am->amap["Timestamp"];
            // if (!TimestampAv)
            // {
            //     char* initTimestamp = (char *)" Initial Timestamp";
            //     TimestampAv =  am->amap["Timestamp"] = vm->setLinkVal(*am->vmap,
            //     "ess", "/status", "Timestamp", initTimestamp); TimestampAv->am =
            //     am;
            // }
            // if(TimestampAv && !TimestampAv->am) TimestampAv->am = am;
            // if (TimestampAv && TimestampAv->am && TimestampAv->am->vm)
            //     CheckAmComms(*am->vmap, am->amap, am->name.c_str(), am->p_fims,
            //     &Av);

            // CheckEssStatus(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);
            // SetupLimitsVec(*am->vmap, am->amap, avmap, am->name.c_str(),
            // "MaxCellCurrent", am); SetupLimitsVec(*am->vmap, am->amap, avmap,
            // am->name.c_str(), "MaxCellVolt", am); CheckOverLimitsVec(*am->vmap,
            // avmap, am->name.c_str(), "MaxCellCurrent", am->p_fims, am);
            // CheckOverLimitsVec(*am->vmap, avmap, am->name.c_str(), "MaxCellVolt",
            // am->p_fims, am);
            defaultFault_module::ShutdownSystem(*am->vmap, am->amap, am->name.c_str(), am->p_fims, &Av);
            defaultRun_module::StartupSystem(*am->vmap, am->amap, am->name.c_str(), am->p_fims, &Av);

            if (0)
                FPS_ERROR_PRINT("%s >> running new monitorList2  ess \n", __func__);
            am->vm->runMonitorList2(*am->vmap, am->amap, "ess", am->p_fims,
                                    "wake_monitor");  // am->p_fims,/*am->amap,*/ 0.1, 0.0);
            if (0)
                FPS_ERROR_PRINT("%s >> completed running new monitorList2  ess\n", __func__);

            if (0)
                FPS_ERROR_PRINT("%s >> running new monitorList2  bms \n", __func__);
            am->vm->runMonitorList2(*am->vmap, am->amap, "bms", am->p_fims,
                                    "wake_monitor");  // am->p_fims,/*am->amap,*/ 0.1, 0.0);
            if (0)
                FPS_ERROR_PRINT("%s >> completed running new monitorList2  bms\n", __func__);

            if (0)
                FPS_ERROR_PRINT("%s >> running new monitorList2  pcs \n", __func__);
            am->vm->runMonitorList2(*am->vmap, am->amap, "pcs", am->p_fims,
                                    "wake_monitor");  // am->p_fims,/*am->amap,*/ 0.1, 0.0);

            void* res21 = vm->getFunc(*am->vmap, "ess", "CheckMonitorVar");
            if (0)
                FPS_ERROR_PRINT(
                    "%s >> completed running new monitorList2  pcs  vmap "
                    "%p res21 %p \n",
                    __func__, am->vmap, res21);

            if (0)
                FPS_ERROR_PRINT("%s >> running old monitorList \n", __func__);
            if (0)
                am->vm->runMonitorList(*am->vmap, am->amap, "ess", am->p_fims,
                                       /*am->amap,*/ 0.1, 0.0);
            // wake monitor comes from the wakeup number

            // if(av && !av->am) av->am = am;
            // if (av && av->am && av->am->vm)
            //     CheckMonitorVar(*am->vmap, am->amap, am->name.c_str(), am->p_fims,
            //     av);

            // xCheckMonitorVar(am, "MaxCellTemperature", "temperature");
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
            // if(bval)
            // {
            //     runLogAv(am, logav, tNow);
            // }
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
        // HandlePower(*am->vmap, am->amap, "ess", am->p_fims, am);
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

    // TODO currently running a fixed list of pubs from the asset read operation
    // TODO make this better after MVP
    if (wakeup == WAKE_LEVEL_PUB)
    {
        bool runPub = false;
        double tNow = am->vm->get_time_dbl();
        essPerf ePerf(am, "ess", "pubLog");
        // assetVar* logav = getLogAv(am, "ess", "pubLog");

        // bool bval = logav->getbVal();
        // if(bval)
        //     tNow = am->vm->get_time_dbl();
        assetVar Av;
        Av.am = am;

        HandleCpuStats(*am->vmap, am->amap, am->name.c_str(), am->p_fims, &Av);
        if (am->amap["RunPub"])
            runPub = am->amap["RunPub"]->getbVal();
        if (0)
            FPS_ERROR_PRINT("%s >> aname [%s] >> publish status RunPub %p ->%s time %2.3f \n", __func__,
                            am->name.c_str(), am->amap["RunPub"], runPub ? "true" : "false", tNow);
        if (runPub)
        {
            for (auto sv : sysVec)
            {
                // Jimmy - A hack right now. Don't really want to pub to
                // /assets/site_controller/summary at the moment since that is not in
                // the config file (site_controller_manager.json)
                if (sv != "site/summary")
                {
                    std::string auri = "/assets/";
                    auri += sv;
                    // printf(" PUB sysVec -> [%s]\n",auri.c_str());
                    // TODO move dummyPub to varmaputils
                    if (1)
                        dummyPub(am, auri.c_str(), auri.c_str());
                }
            }
            // HACK HACK HACK
            // this is for the site manager
            dummyPub(am, "/site/ess_ls", "/site/ess_ls");
        }
        // if(bval)
        // {
        //     runLogAv(am, logav, tNow);
        // }
    }

    if (wakeup == WAKE_LEVEL_PUB_HS)
    {
        bool runPub = false;
        double tNow = am->vm->get_time_dbl();
        // assetVar* logav = getLogAv(am, "ess", "hspubLog");
        essPerf ePerf(am, "ess", "hsPubLog");
        // bool bval = logav->getbVal();
        // if(bval)
        //     tNow = am->vm->get_time_dbl();

        if (am->amap["RunPub"])
            runPub = am->amap["RunPub"]->getbVal();
        if (0)
            FPS_ERROR_PRINT("%s >> aname [%s] >> publish status RunPub %p ->%s time %2.3f \n", __func__,
                            am->name.c_str(), am->amap["RunPub"], runPub ? "true" : "false", tNow);

        // TODO HACK HACK HACK
        // this is for the site manager
        if (runPub)
        {
            dummyPub(am, "/site/ess_hs", "/site/ess_hs");
        }
        // if(bval)
        // {
        //     runLogAv(am, logav, tNow);
        // }
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
        double tNow = 0.0;  // am->vm->get_time_dbl();
        double msize = 0.0;
        if (0)
            FPS_ERROR_PRINT(
                " %s >> >>>>>>>>>>>  %2.3f  %s  >>>>BEFORE FIMS MESSAGE  "
                "method [%s] replyto[%s]  uri [%s]\n",
                __func__, vm->get_time_dbl(), am->name.c_str(), msg->method, msg->replyto ? msg->replyto : "No Reply",
                msg->uri);

        // essPerf ePerf(am, "ess_test", "fimsLog2",nullptr);
        assetVar* logav = getLogAv(am, "ess", "fimsLog");
        bool bval = logav->getbVal();
        if (bval)
            tNow = am->vm->get_time_dbl();

        // we need to collect responses
        // cJSON *cj = nullptr;
        // either here of the bms instance
        // bms_man->vmap
        assetVar* uriav = nullptr;
        if (strcmp(msg->method, "set") == 0)
        {
            uriav = getLogAv(am, "uri_set", msg->uri);
        }
        else if (strcmp(msg->method, "get") == 0)
        {
            uriav = getLogAv(am, "uri_get", msg->uri);
        }
        else if (strcmp(msg->method, "pub") == 0)
        {
            uriav = getLogAv(am, "uri_pub", msg->uri);
        }
        am->vm->sysVec = &sysVec;
        if (msg && msg->body)
            msize = (double)strlen(msg->body);

        am->vm->runFimsMsg(*am->vmap, msg, am->p_fims);  // fims* p_fims)
        if (0)
            FPS_ERROR_PRINT(" %s >> >>>>>>>>>>>  %2.3f  %s  <<<<AFTER FIMS MESSAGE \n", __func__, vm->get_time_dbl(),
                            am->name.c_str());
        if (bval)
        {
            // double tElap = am->vm->get_time_dbl() - tNow;
            runLogAv(am, logav, tNow);
            if (uriav)
            {
                runLogAv(am, uriav, tNow);
                runLogAv2(am, uriav, "size", msize);
            }
        }
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
        // HandleBMSChargeL2(*am->vmap, am->amap, am->name.c_str(), am->p_fims, am);
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

extern double g_base_time;

// so how do we init this ??
// typedef std::map<std::string, std::map<std::string, assetVar*>> varsmap;
varsmap vmap;
varsmap* vmapp = new varsmap;

// the main ess_controller start up
int main(int argc, char* argv[])
{
    int rc = 0;
    g_base_time = 0.0;
    // varsmap vmap;
    varsmap pmap;
    varmap dummyvmap;
    // vm = new VarMapUtils;
    // vm->funMap = &funMap;

    if (argc > 1)
    {
        vm->setFname(argv[1]);
    }
    else
    {
        vm->setFname("configs");
    }

    // const char* cfname = vm->getFname("ess_controller.json");
    // printf(" cfgName [%s] configDir [%s] configFile[%s]\n"
    //     , cfname
    //     , vm->configDir
    //     , vm->configFile
    //     );

    // cfname = vm->getFname("configs/bms_manager.json");
    // printf(" cfgName [%s] configDir [%s] configFile[%s]\n"
    //     , cfname
    //     , vm->configDir
    //     , vm->configFile
    //     );

    // return(0);

    vmap.clear();
    // assetVar* av1 = new assetVar();
    // assetVar* av2 = new assetVar();
    char* tmp;
    asprintf(&tmp, "hello");

    // printf("sizeof vmap %d\n", (int)sizeof(vmap));
    // memset(&vmap,0 , sizeof(vmap));

    // TODO make these more flexible
    system("mkdir -p run_configs");
    system("mkdir -p run_logs");

    // TODO make this an asset manager
    // ess_man = new asset_manager("ess_controller");
    ess_man = new asset_manager("ess");
    ess_man->am = nullptr;
    ess_man->vm = vm;

    ess_man->running = 1;
    vm->sysVec = &sysVec;

    vm->setFunc(vmap, "comp", "/fault", (void*)&dummy_system_fault);
    // vm->setFunc(vmap, "comp", "/faults/bms", (void*)&process_fault);
    // vm->setFunc(vmap, "comp", "/alarms/bms", (void*)&process_alarm);

    // vm->setFunc(vmap, "comp", "/faults/sbmu_1", (void*)&process_fault);
    // vm->setFunc(vmap, "comp", "/alarms/sbmu_1", (void*)&process_alarm);
    // vm->setFunc(vmap, "comp", "/faults/sbmu_2", (void*)&process_fault);
    // vm->setFunc(vmap, "comp", "/alarms/sbmu_2", (void*)&process_alarm);
    // vm->setFunc(vmap, "comp", "/faults/sbmu_3", (void*)&process_fault);
    // vm->setFunc(vmap, "comp", "/alarms/sbmu_3", (void*)&process_alarm);
    // vm->setFunc(vmap, "comp", "/faults/sbmu_4", (void*)&process_fault);
    // vm->setFunc(vmap, "comp", "/alarms/sbmu_4", (void*)&process_alarm);
    // vm->setFunc(vmap, "comp", "/faults/sbmu_5", (void*)&process_fault);
    // vm->setFunc(vmap, "comp", "/alarms/sbmu_5", (void*)&process_alarm);
    // vm->setFunc(vmap, "comp", "/faults/sbmu_6", (void*)&process_fault);
    // vm->setFunc(vmap, "comp", "/alarms/sbmu_6", (void*)&process_alarm);
    // vm->setFunc(vmap, "comp", "/faults/sbmu_7", (void*)&process_fault);
    // vm->setFunc(vmap, "comp", "/alarms/sbmu_7", (void*)&process_alarm);
    // vm->setFunc(vmap, "comp", "/faults/sbmu_8", (void*)&process_fault);
    // vm->setFunc(vmap, "comp", "/alarms/sbmu_8", (void*)&process_alarm);
    // vm->setFunc(vmap, "comp", "/faults/sbmu_9", (void*)&process_fault);
    // vm->setFunc(vmap, "comp", "/alarms/sbmu_9", (void*)&process_alarm);

    // vm->setFunc(vmap, "comp", "/faults/pcs", (void*)process_fault);
    // vm->setFunc(vmap, "comp", "/alarms/pcs", (void*)&process_alarm);
    // vm->setFunc(vmap, "bms", "process_sys_alarm", (void*)&process_sys_alarm);
    // vm->setFunc(vmap, "sbmu_1", "process_sys_alarm",
    // (void*)&process_sys_alarm); vm->setFunc(vmap, "sbmu_2",
    // "process_sys_alarm", (void*)&process_sys_alarm); vm->setFunc(vmap,
    // "sbmu_3", "process_sys_alarm", (void*)&process_sys_alarm);
    // vm->setFunc(vmap, "sbmu_4", "process_sys_alarm",
    // (void*)&process_sys_alarm); vm->setFunc(vmap, "sbmu_5",
    // "process_sys_alarm", (void*)&process_sys_alarm); vm->setFunc(vmap,
    // "sbmu_6", "process_sys_alarm", (void*)&process_sys_alarm);
    // vm->setFunc(vmap, "sbmu_7", "process_sys_alarm",
    // (void*)&process_sys_alarm); vm->setFunc(vmap, "sbmu_8",
    // "process_sys_alarm", (void*)&process_sys_alarm); vm->setFunc(vmap,
    // "sbmu_9", "process_sys_alarm", (void*)&process_sys_alarm);
    // vm->setFunc(vmap, "pcs", "process_sys_alarm", (void*)&process_sys_alarm);

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

    int CheckMonitorVarxx(varsmap & vmap, varmap & amap, const char* aname, fims* p_fims, assetVar* av);

    vm->setFunc(vmap, "ess", "CheckMonitorVar", (void*)&CheckMonitorVar);
    vm->setFunc(vmap, "ess", "UpdateSysTime", (void*)&UpdateSysTime);

    // void (*tf)(void *) = (void (*tf)(void *))
    void* res1 = vm->getFunc(vmap, "ess", "run_init");
    void* res2 = vm->getFunc(vmap, "ess", "run_wakeup");
    void* res3 = vm->getFunc(vmap, "comp", "/fault/bms");

    typedef void (*myCompFcn_t)(varsmap & vm, varmap & amap, const char* aname, fims* p_fims, assetVar* data);
    // void (*tf)(void *) = (void (*tf)(void *))(res3);

    if (res3)
    {
        int ival = 0;
        assetVar Av("TestAv", ival);
        Av.am = ess_man;
        myCompFcn_t myCompFcn = myCompFcn_t(res3);
        myCompFcn(vmap, ess_man->amap, "ess_man", nullptr, &Av);
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

    const char* cfgname = vm->getFname("ess_controller.json");
    if (cfgname == nullptr)
    {
        cfgname = vm->getFname("test_ess_config.json");
        FPS_ERROR_PRINT("%s >> ERROR Getting initial configuration   from [%s]\n", __func__, cfgname);
        return (0);
    }
    FPS_ERROR_PRINT("%s >> Getting initial configuration   from [%s]\n", __func__, cfgname);
    // return 0;

    rc = vm->configure_vmap(vmap, cfgname, nullptr, ess_man);
    if (rc < 0)
    {
        FPS_ERROR_PRINT("%s >> Failed to parse initial configuration from [%s]\n", __func__, cfgname);
        exit(0);
    }

    // ess_man->configure(cfgname , nullptr);

    {
        // this a test for our config.
        cJSON* cjbm = nullptr;
        cjbm = vm->getMapsCj(vmap, nullptr, nullptr, 0x10000);
        const char* fname = vm->getFname("run_configs/ess_1_at_start.json");
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
        cjbm = vm->getMapsCj(vmap, nullptr, nullptr, 0x10000);
        const char* fname = vm->getFname("run_configs/ess_2_after_links.json");
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
    cfgname = vm->getFname("patch_configs/test_ess.json");  // TODO move this into
                                                            // asset manager
                                                            // assconfig
    // TODO skip this we dont always expect it to work
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
        sname = "ess";
        sname += "/ess_1";
        sysVec.push_back(sname);
        FPS_ERROR_PRINT(" %s >> HACK HACK addded [%s] to sysVec size %d \n", __func__, sname.c_str(),
                        (int)sysVec.size());
    }

    if (1)
        FPS_ERROR_PRINT(" %s >> ess_man >> sysVec size [%d]\n", __func__, (int)sysVec.size());

    // This loads in the system_controller interface
    // cJSON* cjsite = cJSON_GetObjectItem(cj, "/site/ess");
    auto ixs = vmap.find("/config/ess_server");
    if (ixs != vmap.end())
    {
        for (auto iy : ixs->second)
        {
            if (iy.second->name == "ess")
            {
                char* cfgname = vm->getFname(iy.second->getcVal());
                // char* cfgname = ; //aVal->valuestring;
                cJSON* cj = vm->get_cjson(cfgname);

                FPS_ERROR_PRINT(
                    "%s >> site_ess lets run site config for  [%s] from "
                    "[%s] file [%s] cj %p\n",
                    __func__, iy.first.c_str(), iy.second->name.c_str(), cfgname, cj);
                // this is for the site interface
                if (cj)
                    vm->loadSiteMap(vmap, cj);
                free(cfgname);  // LEAK
                if (cj)
                    cJSON_Delete(cj);
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
    //    fims* p_fims = new fims;

    //        while (!p_fims->Connect((char*)td->name)) {
    //            poll(nullptr, 0, 1000);
    //            FPS_ERROR_PRINT("%s >> name %s waiting to connect to
    //            FIMS,attempt %d\n", __func__, name.c_str(),
    //            cattempt++);
    //        }

    // FPS_ERROR_PRINT("%s >> name %s afer connect to FIMS attempts %d sublen %d
    // subs %p\n"
    //     , __func__, name.c_str(), cattempt++, sublen,
    //     (void*)td->subs);
    // if (sublen > 15)
    //     sublen = 15;
    // p_fims->Subscribe((const char**)td->subs, sublen)

    // //p_fims->Connect((char *)"fimsEssTest");
    // bool connectOK = false;
    // int fims_count = 0;
    // while  (!connectOK)
    // {
    //     connectOK = p_fims->Connect((char*)"FimsEssTest");
    //     if(!connectOK)
    //     {
    //         if(fims_count > MAX_FIMS_CONNECT_TRIES)
    //         {
    //             FPS_ERROR_PRINT("%s >> unable to connect to fims server after
    //             %d attempts. All is lost.\n", __func__,
    //             fims_count); exit(0);
    //         }
    //         FPS_ERROR_PRINT("%s >> failed  to connect to fims server on attempt
    //         %d \n", __func__, fims_count); running = 0;
    //         fims_count++; sleep(5);
    //     }
    // }

    // ess_man->p_fims = p_fims;
    ess_man->running = 1;

    // eeded for the controller, manager and possibly assets
    ess_man->setupChannels();

    // the timer generates  channel wakes every 10 mS
    ess_man->run_timer(10);

    // run the message loop eery 1.5 seconds
    // the message  generates test message  channel wakes every 1500 mS
    ess_man->run_message(1500);
    int ccnt = 0;
    char** subs2 = nullptr;
    vecmap vecs;
    ess_man->vecs = &vecs;

    // TODO all weuse are the Subs at the moment
    // vm->getVList(vecs, vmap, ess_man->amap, "ess", "Blocks", ccnt);

    // char**subs3 = nullptr;
    ccnt = 0;
    // vm->getVList(vecs, vmap, ess_man->amap, "ess", "Pubs", ccnt);
    vm->getVList(vecs, vmap, ess_man->amap, "ess", "Subs", ccnt);

    // subs3 = vm->getList(*ess_man->vecs, vmap, ess_man->amap, "ess", "Pubs",
    // ccnt);

    // FPS_ERROR_PRINT("%s >> >>>>found %d  Pubs in config file here they are:\n",
    // __func__, ccnt);  vm->showList(subs3, "ess", ccnt);
    // vm->addListToVec(vecs, subs3, "Pubs", ccnt);

    FPS_ERROR_PRINT("%s >> >>>>looking at ess_man->vecs ccnt %d\n", __func__, ccnt);

    vm->showvecMap(*ess_man->vecs);

    FPS_ERROR_PRINT("%s >> <<<<<<<<<<<<<<end of Pubs\n", __func__);

    subs2 = vm->getList(vecs, vmap, ess_man->amap, "ess", "Subs", ccnt);
    if (!subs2)
    {
        FPS_ERROR_PRINT("%s >> use deflist for subs\n", __func__);

        subs2 = vm->getDefList(vecs, vmap, ess_man->amap, "ess", "Subs", ccnt);
    }
    FPS_ERROR_PRINT("%s >> found %d  subs in config file, here they are:\n", __func__, ccnt);
    vm->showList(subs2, "ess", ccnt);
    FPS_ERROR_PRINT("%s >> <<<<<<<<<<<<<<end of subs\n", __func__);

    FPS_ERROR_PRINT("%s >>  >>>>>>>>>>>>>start of vecs\n", __func__);
    vm->showvecMap(vecs);
    FPS_ERROR_PRINT("%s >> <<<<<<<<<<<<<<end of vecs\n", __func__);

    fims* p_fims = new fims;

    int cattempt = 0;
    while (!p_fims->Connect((char*)"ess_controller") && (cattempt++ < MAX_FIMS_CONNECT_TRIES))
    {
        poll(nullptr, 0, 1000);
        FPS_ERROR_PRINT("%s >> name %s waiting to connect to FIMS,attempt %d\n", __func__, "ess_controller",
                        cattempt++);
    }
    if (cattempt >= MAX_FIMS_CONNECT_TRIES)
    {
        FPS_ERROR_PRINT("%s >> name %s Unable to  connect to FIMS after %d attempts\n", __func__, "ess_controller",
                        cattempt);
        running = 0;
        ess_man->running = 0;
    }

    // FPS_ERROR_PRINT("%s >> name %s afer connect to FIMS attempts %d sublen %d
    // subs %p\n"
    //     , __func__, name.c_str(), cattempt++, sublen,
    //     (void*)td->subs);
    // if (sublen > 15)
    //     sublen = 15;
    p_fims->Subscribe((const char**)subs2, ccnt);
    // the fims system will get pubs and create a varsMap for  the items.
    // TODO restrict fims to known components

    // this will start serviceing the incoming fims messages but , as of yet , no
    // one is listening.
    ess_man->p_fims = p_fims;
    ess_man->run_fims(1500, (char**)subs2, "essMan", ccnt);

    // if(ccnt > 0)
    // {
    //   vm->ClearSubs(subs2,"ess", ccnt);
    // }
    // the manager runs the channel receiver thread
    // if (ess_man->pmap)
    // {
    //     cJSON* cj = vm->getMapsCj(*ess_man->pmap);
    //     char* res = cJSON_Print(cj);
    //     printf("ESS >>Pmap before manager \n%s\n", res);
    //     free((void*)res);
    //     cJSON_Delete(cj);
    // }
    // else
    // {
    //     printf("ESS >>Pmap before run skipped\n");

    // }

    // // this will run the manager
    // ess_man->run_manager(p_fims);

    // now we try to create and configure the assets.
    // vmap /assets/ess contains the goodies.
    // do it the long way first
    // vmap["/links/bms_1"]
    asset_manager* ass_man = nullptr;

    auto ixa = vmap.find("/config/ess/managers");
    if (ixa != vmap.end())
    {
        // if this works no need to run the init function below
        FPS_ERROR_PRINT(
            "%s >> ESS >>We found our assets, we should be able to set "
            "up our system\n",
            __func__);
        for (auto iy : ixa->second)
        {
            if (iy.second->type == assetVar::ASTRING)
            {
                FPS_ERROR_PRINT("%s >> lets run assets for  [%s] from [%s]\n", __func__, iy.first.c_str(),
                                iy.second->aVal->valuestring);
                // this utility gets the correct file name
                // looks out for the config directory in the args...and its smart about
                // it
                // const char* fname = vm->getFname(iy.second->aVal->valuestring);
                const char* fname = iy.second->aVal->valuestring;

                // TODO add digio
                if (iy.first == "ess" || iy.first == "bms" || iy.first == "pcs" || iy.first == "site")
                {
                    const char* aname = iy.first.c_str();
                    FPS_ERROR_PRINT("%s >> setting up manager [%s]\n", __func__, aname);
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
                    // vm->setAmFunc(vmap, "comp", "/alarms", aname, ass_man,
                    // (void*)&dummy_bms_alarm); vm->setAmFunc(vmap, "comp", "/faults",
                    // aname, ass_man, (void*)&dummy_bms_fault);
                    assetVar* Av = iy.second;
                    Av->am = ass_man;
                    // vm->setAvFunc(vmap, "comp", "/alarms", aname, Av,
                    // (void*)&process_alarm); vm->setAvFunc(vmap, "comp", "/faults",
                    // aname, Av, (void*)&process_fault); vm->setAvFunc(vmap, "comp",
                    // "/status", aname, Av, (void*)&process_status);

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
                    if (ass_man->configure(&vmap, fname, aname, &sysVec, myAwake, ass_man) < 0)
                    {
                        FPS_ERROR_PRINT(" %s >> error in [%s] config file [%s]\n", __func__, aname, fname);
                        exit(0);
                    }

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

    // Call initFunc here to initialize functions for all assets/asset managers
    initFuncs(ess_man);

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
        if (cj)
        {
            char* res = cJSON_Print(cj);
            // printf("ESS >>Maps before run \n%s\n", res);
            free((void*)res);
            cJSON_Delete(cj);
        }
    }
    else
    {
        FPS_ERROR_PRINT("ESS >>Maps before run skipped\n");
    }
    // if (ess_man->pmap)
    // {
    //     cJSON* cj = vm->getMapsCj(*ess_man->pmap);
    //     if(cj)
    //     {
    //         char* res = cJSON_Print(cj);
    //         FPS_ERROR_PRINT("ESS >>Pmap before run \n%s\n", res);
    //         free((void*)res);
    //         cJSON_Delete(cj);
    //     }
    // }
    // else
    // {
    //     FPS_ERROR_PRINT("ESS >>Pmap before run skipped\n");

    // }

    // config all done run the manager
    // set up vlinks
    // std::map<std::string,
    //   std::map<std::string, assetVar*, std::less<std::string>,
    //   std::allocator<std::pair<std::string const, assetVar*> > >,
    //   std::less<std::string>,
    //     std::allocator<std::pair<std::string const, std::map<std::string,
    //     assetVar*, std::less<std::string>, std::allocator<std::pair<std::string
    //     const, assetVar*> > > > > >&,
    //         char const*)
    FPS_ERROR_PRINT("ESS >>Setting vlinks\n");

    vm->setVLinks(vmap, "ess");
    FPS_ERROR_PRINT("ESS >>Done Setting vlinks\n");
    // config all done run the manager
    // this will run the manager
    ess_man->run_manager(p_fims);

    // NOTE this could collide with the  wake manager thread so lets put it in
    // there
    while (ess_man->running)
    {
        poll(nullptr, 0, 1000);
        // run_secs++;
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
    if (p_fims)
        delete (p_fims);

    return 0;
}
#endif
