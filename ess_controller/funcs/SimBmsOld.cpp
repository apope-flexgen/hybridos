#ifndef SIMHANDLEHEARTBEAT_CPP
#define SIMHANDLEHEARTBEAT_CPP
// SimHandleHeartbeat.cpp
#include "asset.h"

/**
 * Phil's code ready for review
 * 11/06/2020
 * increments  a heart beat ... may need to add a period to this
 * User to simulate asset heartbeats
 *
 * Used in:
 * Test Script: test_sim_hb.sh
 */
extern "C++" {

int SimHandleBms(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
int SimHandleSbmu(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
};

int SimSetupBmsModules(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV, const char* modfmt)
{
    // const auto now = flex::get_time_dbl();
    int num_modules = amap["num_modules"]->getiVal();
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;

    if (1)
        FPS_ERROR_PRINT(
            "%s >> [%2.3s] >> Running manager [%s] aname [%d] "
            "num_modules %s fmt [%s]\n",
            __func__, am->name.c_str(), aname, num_modules, modfmt);
    // int ival = 0;
    // double dval = 0;
    // amap["num_running_modules"]->setVal(ival);
    // amap["pcs_power"]->setVal(dval);
    int idx = 1;
    while (idx <= num_modules)
    {
        char* mname;
        vm->vmlen = asprintf(&mname, "rack_%02d", idx);
        asset* ami = vm->getaI(vmap, mname);
        if (0)
            FPS_ERROR_PRINT("%s >> >> seeking  [%s] ami %p\n", __func__, mname, ami);
        idx++;
        if (!ami)
        {
            if (1)
                FPS_ERROR_PRINT("%s >> >> creating instance [%s] for am [%s]\n", __func__, mname, am->name.c_str());
            ami = am->addInstance(mname);
            ami->setVmap(&vmap);
            ami->setVm(vm);
            ami->am = am;
            ami->p_fims = p_fims;
            vm->setaI(vmap, mname, ami);
        }
    }

    return 0;
}

int SimHandleBms(varsmap& vmap, varmap& amap, const char* xaname, fims* p_fims, assetVar* aV)
{
    int reload;
    double dval = 0.0;
    // bool bval = false;

    int ival = 0;
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    FPS_ERROR_PRINT(" %s >> running aname [%s] asset manager  [%s] \n", __func__, xaname, am->name.c_str());

    const char* aname = "bms";
    am = vm->getaM(vmap, aname);
    if (!am)
    {
        FPS_ERROR_PRINT(" %s >> unable to find bms asset manager \n", __func__);
        return 0;
    }
    // varmap amap = *am->vmap;

    char* tVal = (char*)"Test TimeStamp";
    const auto now = flex::get_time_dbl();
    // need fname to include sbmu_x  so add aname
    const char* fname = __func__;  // __func__ plus aname

    assetVar* SimHandleBms = amap[__func__];
    if (!SimHandleBms || (reload = SimHandleBms->getiVal()) == 0)
    {
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }

    if (reload < 2)
    {
        //     "/components/catl_ems_bms_rw": {
        //     "ems_cmd": {
        // },
        // "/components/catl_mbmu_summary_r": {
        //     "mbmu_voltage": {
        //     "mbmu_current": {
        //     "mbmu_max_cell_voltage": {
        //     "mbmu_min_cell_voltage": {
        //     "mbmu_avg_cell_voltage": {
        //     "mbmu_max_cell_temperature": {
        //     "mbmu_min_cell_temperature": {
        //     "mbmu_avg_cell_temperature": {
        //     "mbmu_soc": {
        //     "mbmu_soh": {
        //     "mbmu_max_charge_current": {
        //     "mbmu_max_discharge_current": {

        //        "bms_heartbeat": {
        //        "bms_timestamp": {
        //        "bms_poweron": {
        //     "value":0,
        //     "actions": {
        //         "onSet": [{
        //             "enum": [
        //                 {"shift": 0, "mask": 255, "inValue": 0, "uri":
        //                 "/status/bms:BMSPowerOn", "outValue": "Off Ready"},
        //                 {"shift": 0, "mask": 255, "inValue": 1, "uri":
        //                 "/status/bms:BMSPowerOn", "outValue": "On Ready"},
        //                 {"shift": 0, "mask": 255, "inValue": 2, "uri":
        //                 "/status/bms:BMSPowerOn", "outValue": "On Fault"},
        //                 {"shift": 0, "mask": 255, "inValue": 3, "uri":
        //                 "/status/bms:BMSPowerOn", "outValue": "Off Fault"},
        //                 {"shift": 0, "mask": 255, "inValue": 0, "uri":
        //                 "/status/bms:SystemFault", "outValue": false},
        //                 {"shift": 0, "mask": 255, "inValue": 1, "uri":
        //                 "/status/bms:SystemFault", "outValue": false},
        //                 {"shift": 0, "mask": 255, "inValue": 2, "uri":
        //                 "/status/bms:SystemFault", "outValue": true},
        //                 {"shift": 0, "mask": 255, "inValue": 3, "uri":
        //                 "/status/bms:SystemFault", "outValue": true},
        //                 {"shift": 0, "mask": 255, "inValue": 0, "uri":
        //                 "/assets/bms/summary:start@enabled", "outValue": true},
        //                 {"shift": 0, "mask": 255, "inValue": 1, "uri":
        //                 "/assets/bms/summary:start@enabled", "outValue": true},
        //                 {"shift": 0, "mask": 255, "inValue": 2, "uri":
        //                 "/assets/bms/summary:start@enabled", "outValue": true},
        //                 {"shift": 0, "mask": 255, "inValue": 3, "uri":
        //                 "/assets/bms/summary:start@enabled", "outValue": true},
        //                 {"shift": 0, "mask": 255, "inValue": 0, "uri":
        //                 "/assets/bms/summary:stop@enabled", "outValue": false},
        //                 {"shift": 0, "mask": 255, "inValue": 1, "uri":
        //                 "/assets/bms/summary:stop@enabled", "outValue": true},
        //                 {"shift": 0, "mask": 255, "inValue": 2, "uri":
        //                 "/assets/bms/summary:stop@enabled", "outValue": true},
        //                 {"shift": 0, "mask": 255, "inValue": 3, "uri":
        //                 "/assets/bms/summary:stop@enabled", "outValue": true},
        //                 {"shift": 0, "mask": 255, "inValue": 0, "uri":
        //                 "/assets/pcs/summary:start@enabled", "outValue": false},
        //                 {"shift": 0, "mask": 255, "inValue": 1, "uri":
        //                 "/assets/pcs/summary:start@enabled", "outValue": true},
        //                 {"shift": 0, "mask": 255, "inValue": 2, "uri":
        //                 "/assets/pcs/summary:start@enabled", "outValue": false},
        //                 {"shift": 0, "mask": 255, "inValue": 3, "uri":
        //                 "/assets/pcs/summary:start@enabled", "outValue": false},

        //                 {"shift": 0, "mask": 255, "inValue": 0, "uri":
        //                 "/site/ess_hs:system_state", "outValue": 1,  "note": "Bit
        //                 0 - Stop"},
        //                 {"shift": 0, "mask": 255, "inValue": 3, "uri":
        //                 "/site/ess_hs:system_state", "outValue": 1,  "note": "Bit
        //                 0 - Stop"}
        //             ]
        //         }]
        //     }
        // },
        // "bms_status": {
        //     "value":0,
        //         "actions": {
        //             "onSet": [{
        //                 "enum": [
        //                 {"shift": 0, "mask": 255, "inValue": 0, "uri":
        //                 "/status/bms:BMSStatus", "outValue": "Initial"},
        //                 {"shift": 0, "mask": 255, "inValue": 1, "uri":
        //                 "/status/bms:BMSStatus", "outValue": "Normal"},
        //                 {"shift": 0, "mask": 255, "inValue": 2, "uri":
        //                 "/status/bms:BMSStatus", "outValue": "Full Charge"},
        //                 {"shift": 0, "mask": 255, "inValue": 3, "uri":
        //                 "/status/bms:BMSStatus", "outValue": "Full Discharge"},
        //                 {"shift": 0, "mask": 255, "inValue": 4, "uri":
        //                 "/status/bms:BMSStatus", "outValue": "Warning"},
        //                 {"shift": 0, "mask": 255, "inValue": 5, "uri":
        //                 "/status/bms:BMSStatus", "outValue": "Fault"}
        //             ]
        //         }]
        //     }
        // },
        // "bms_remain_charge_energy": {
        //       "bms_remain_discharge_energy": {
        //       "bms_max_charge_allowed": {
        //       "bms_max_discharge_allowed": {
        //       "num_hv_subsystem": {
        //   "/components/catl_mbmu_stat_r": {
        //       "mbmu_status": {
        //   "/components/catl_mbmu_sum_r": {
        //       "mbmu_warning_1": {
        // "actions": {
        //     "onSet": [{
        //         "enum": [
        //             { "shift": 0,"mask": 3,"inValue": 0,"uri":
        //             "/alarms/bms:single_cell_overvoltage", "outValue": "Normal"},
        //             { "shift": 0,"mask": 3,"inValue": 1,"uri":
        //             "/alarms/bms:single_cell_overvoltage", "outValue": "Warn 1"},
        //             { "shift": 0,"mask": 3,"inValue": 2,"uri":
        //             "/alarms/bms:single_cell_overvoltage", "outValue": "Warn 2"},
        //             { "shift": 0,"mask": 3,"inValue": 3,"uri":
        //             "/alarms/bms:single_cell_overvoltage", "outValue": "Warn 3"},

        //             { "shift": 2,"mask": 3,"inValue": 0,"uri":
        //             "/alarms/bms:single_cell_undervoltage", "outValue":
        //             "Normal"}, { "shift": 2,"mask": 3,"inValue": 1,"uri":
        //             "/alarms/bms:single_cell_undervoltage", "outValue": "Warn
        //             1"}, { "shift": 2,"mask": 3,"inValue": 2,"uri":
        //             "/alarms/bms:single_cell_undervoltage", "outValue": "Warn
        //             2"}, { "shift": 2,"mask": 3,"inValue": 3,"uri":
        //             "/alarms/bms:single_cell_undervoltage", "outValue": "Warn
        //             3"},

        //             { "shift": 4,"mask": 3,"inValue": 0,"uri":
        //             "/alarms/bms:single_cell_overtemp", "outValue": "Normal"}, {
        //             "shift": 4,"mask": 3,"inValue": 1,"uri":
        //             "/alarms/bms:single_cell_overtemp", "outValue": "Warn 1"}, {
        //             "shift": 4,"mask": 3,"inValue": 2,"uri":
        //             "/alarms/bms:single_cell_overtemp", "outValue": "Warn 2"}, {
        //             "shift": 4,"mask": 3,"inValue": 3,"uri":
        //             "/alarms/bms:single_cell_overtemp", "outValue": "Warn 3"},

        //             { "shift": 6,"mask": 3,"inValue": 0,"uri":
        //             "/alarms/bms:single_cell_undertemp", "outValue": "Normal"},
        //             { "shift": 6,"mask": 3,"inValue": 1,"uri":
        //             "/alarms/bms:single_cell_undertemp", "outValue": "Warn 1"},
        //             { "shift": 6,"mask": 3,"inValue": 2,"uri":
        //             "/alarms/bms:single_cell_undertemp", "outValue": "Warn 2"},
        //             { "shift": 6,"mask": 3,"inValue": 3,"uri":
        //             "/alarms/bms:single_cell_undertemp", "outValue": "Warn 3"},

        //             { "shift": 8,"mask": 3,"inValue": 0,"uri":
        //             "/alarms/bms:SOC_low", "outValue": "Normal"}, { "shift":
        //             8,"mask": 3,"inValue": 1,"uri": "/alarms/bms:SOC_low",
        //             "outValue": "Warn 1"}, { "shift": 8,"mask": 3,"inValue":
        //             2,"uri": "/alarms/bms:SOC_low", "outValue": "Warn 2"}, {
        //             "shift": 8,"mask": 3,"inValue": 3,"uri":
        //             "/alarms/bms:SOC_low", "outValue": "Warn 3"},

        // { "shift": 0,"mask": 3,"inValue": 1,"uri": "/site/ess_ls:bms_alarms[0]",
        // "outValue": true, "note": "Bit 0 - Single Cell Overvoltage Warning 1"},
        // { "shift": 0,"mask": 3,"inValue": 2,"uri": "/site/ess_ls:bms_alarms[1]",
        // "outValue": true, "note": "Bit 1 - Single Cell Overvoltage Warning 2"},
        // { "shift": 0,"mask": 3,"inValue": 3,"uri": "/site/ess_ls:bms_alarms[2]",
        // "outValue": true, "note": "Bit 2 - Single Cell Overvoltage Warning 3"},

        // { "shift": 2,"mask": 3,"inValue": 1,"uri": "/site/ess_ls:bms_alarms[3]",
        // "outValue": true, "note": "Bit 3 - Single Cell Undervoltage Warning 1"},
        // { "shift": 2,"mask": 3,"inValue": 2,"uri": "/site/ess_ls:bms_alarms[4]",
        // "outValue": true, "note": "Bit 4 - Single Cell Undervoltage Warning 2"},
        // { "shift": 2,"mask": 3,"inValue": 3,"uri": "/site/ess_ls:bms_alarms[5]",
        // "outValue": true, "note": "Bit 5 - Single Cell Undervoltage Warning 3"},

        // { "shift": 4,"mask": 3,"inValue": 1,"uri": "/site/ess_ls:bms_alarms[6]",
        // "outValue": true, "note": "Bit 6 - Single Cell Overtemperature Warning
        // 1"}, { "shift": 4,"mask": 3,"inValue": 2,"uri":
        // "/site/ess_ls:bms_alarms[7]", "outValue": true, "note": "Bit 7 - Single
        // Cell Overtemperature Warning 2"}, { "shift": 4,"mask": 3,"inValue":
        // 3,"uri": "/site/ess_ls:bms_alarms[8]", "outValue": true, "note": "Bit 8 -
        // Single Cell Overtemperature Warning 3"},

        // { "shift": 6,"mask": 3,"inValue": 1,"uri": "/site/ess_ls:bms_alarms[9]",
        // "outValue": true, "note": "Bit 9 - Single Cell Undertemperature Warning
        // 1"}, { "shift": 6,"mask": 3,"inValue": 2,"uri":
        // "/site/ess_ls:bms_alarms[10]", "outValue": true, "note": "Bit 10 - Single
        // Cell Undertemperature Warning 2"}, { "shift": 6,"mask": 3,"inValue":
        // 3,"uri": "/site/ess_ls:bms_alarms[11]", "outValue": true, "note": "Bit 11
        // - Single Cell Undertemperature Warning 3"},

        // { "shift": 8,"mask": 3,"inValue": 1,"uri": "/site/ess_ls:bms_alarms[12]",
        // "outValue": true, "note": "Bit 12 - SOC Low Warning 1"}, { "shift":
        // 8,"mask": 3,"inValue": 2,"uri": "/site/ess_ls:bms_alarms[13]",
        // "outValue": true, "note": "Bit 13 - SOC Low Warning 2"}, { "shift":
        // 8,"mask": 3,"inValue": 3,"uri": "/site/ess_ls:bms_alarms[14]",
        // "outValue": true, "note": "Bit 14 - SOC Low Warning 3"}

        //"mbmu_warning_21": {
        // "value": 0,
        // "note":"Mbmu Warning table without degrees reg 1 Appendix 2",
        // "note1":"To be moved to bms_manager.json",
        // "actions": {
        //     "onSet": [{
        //         "enum": [
        //             { "shift": 0,"mask": 1,"inValue": 0,"uri":
        //             "/alarms/bms:current_overlimit", "outValue": "Normal"}, {
        //             "shift": 0,"mask": 1,"inValue": 1,"uri":
        //             "/alarms/bms:current_overlimit", "outValue": "Warn"}, {
        //             "shift": 1,"mask": 1,"inValue": 0,"uri":
        //             "/alarms/bms:single_cell_voltage_overlimit", "outValue":
        //             "Normal"}, { "shift": 1,"mask": 1,"inValue": 1,"uri":
        //             "/alarms/bms:single_cell_voltage_overlimit", "outValue":
        //             "Warn"}, { "shift": 2,"mask": 1,"inValue": 0,"uri":
        //             "/alarms/bms:single_cell_temp_overlimit", "outValue":
        //             "Normal"}, { "shift": 2,"mask": 1,"inValue": 1,"uri":
        //             "/alarms/bms:single_cell_temp_overlimit", "outValue":
        //             "Warn"}, { "shift": 3,"mask": 1,"inValue": 0,"uri":
        //             "/alarms/bms:system_voltage_overlimit", "outValue":
        //             "Normal"}, { "shift": 3,"mask": 1,"inValue": 1,"uri":
        //             "/alarms/bms:system_voltage_overlimit", "outValue": "Warn"},
        //             { "shift": 4,"mask": 1,"inValue": 0,"uri":
        //             "/alarms/bms:system_voltage_underlimit", "outValue":
        //             "Normal"}, { "shift": 4,"mask": 1,"inValue": 1,"uri":
        //             "/alarms/bms:system_voltage_underlimit", "outValue": "Warn"},
        //             { "shift": 5,"mask": 1,"inValue": 0,"uri":
        //             "/alarms/bms:inner_communication", "outValue": "Normal"}, {
        //             "shift": 5,"mask": 1,"inValue": 1,"uri":
        //             "/alarms/bms:inner_communication", "outValue": "Warn"}, {
        //             "shift": 6,"mask": 1,"inValue": 0,"uri":
        //             "/alarms/bms:cell_extreme_temp", "outValue": "Normal"}, {
        //             "shift": 6,"mask": 1,"inValue": 1,"uri":
        //             "/alarms/bms:cell_extreme_temp", "outValue": "Warn"}, {
        //             "shift": 7,"mask": 1,"inValue": 0,"uri":
        //             "/alarms/bms:cell_extreme_voltage", "outValue": "Normal"}, {
        //             "shift": 7,"mask": 1,"inValue": 1,"uri":
        //             "/alarms/bms:cell_extreme_voltage", "outValue": "Warn"}, {
        //             "shift": 8,"mask": 1,"inValue": 0,"uri":
        //             "/alarms/bms:EMS_heartbeat", "outValue": "Normal"}, {
        //             "shift": 8,"mask": 1,"inValue": 1,"uri":
        //             "/alarms/bms:EMS_heartbeat", "outValue": "Warn"},

        //             { "shift": 0,"mask": 1,"inValue": 1,"uri":
        //             "/site/ess_ls:bms_alarms[15]", "outValue": true, "note": "Bit
        //             15 - Current Overlimit Warning"}, { "shift": 1,"mask":
        //             1,"inValue": 1,"uri": "/site/ess_ls:bms_alarms[16]",
        //             "outValue": true, "note": "Bit 16 - Single Cell Voltage
        //             Overlimit Warning"}, { "shift": 2,"mask": 1,"inValue":
        //             1,"uri": "/site/ess_ls:bms_alarms[17]", "outValue": true,
        //             "note": "Bit 17 - Single Cell Temperature Overlimit
        //             Warning"}, { "shift": 3,"mask": 1,"inValue": 1,"uri":
        //             "/site/ess_ls:bms_alarms[18]", "outValue": true, "note": "Bit
        //             18 - System Voltage Overlimit Warning"}, { "shift": 4,"mask":
        //             1,"inValue": 1,"uri": "/site/ess_ls:bms_alarms[19]",
        //             "outValue": true, "note": "Bit 19 - System Voltage Underlimit
        //             Warning"}, { "shift": 5,"mask": 1,"inValue": 1,"uri":
        //             "/site/ess_ls:bms_alarms[20]", "outValue": true, "note": "Bit
        //             20 - Inner Communication Warning"}, { "shift": 6,"mask":
        //             1,"inValue": 1,"uri": "/site/ess_ls:bms_alarms[21]",
        //             "outValue": true, "note": "Bit 21 - Cell Extreme Temperature
        //             Warning"}, { "shift": 7,"mask": 1,"inValue": 1,"uri":
        //             "/site/ess_ls:bms_alarms[22]", "outValue": true, "note": "Bit
        //             22 - Cell Extreme Voltage Warning"}, { "shift": 8,"mask":
        //             1,"inValue": 1,"uri": "/site/ess_ls:bms_alarms[23]",
        //             "outValue": true, "note": "Bit 23 - EMS Heartbeat Warning"}

        //        "mbmu_warning_22": {
        // "actions": {
        //     "onSet": [{
        //         "enum": [
        //             { "shift": 10,"mask": 1,"inValue": 0,"uri":
        //             "/alarms/bms:SOC_difference", "outValue": "Normal"}, {
        //             "shift": 10,"mask": 1,"inValue": 1,"uri":
        //             "/alarms/bms:SOC_difference", "outValue": "Warn"}, { "shift":
        //             11,"mask": 1,"inValue": 0,"uri":
        //             "/alarms/bms:fire_fault_level", "outValue": "Normal"}, {
        //             "shift": 11,"mask": 1,"inValue": 1,"uri":
        //             "/alarms/bms:fire_fault_level", "outValue": "Level 2"},

        //             { "shift": 10,"mask": 1,"inValue": 1,"uri":
        //             "/site/ess_ls:bms_alarms[24]", "outValue": true, "note": "Bit
        //             24 - SOC Difference Warning"}, { "shift": 11,"mask":
        //             1,"inValue": 1,"uri": "/site/ess_ls:bms_alarms[25]",
        //             "outValue": true, "note": "Bit 25 - Fire Fault Level 2
        //             Warning"}

        // reload = 0;

        amap[fname] = vm->setLinkVal(vmap, aname, "/reload", fname, reload);

        amap["bms_current"] = vm->setLinkVal(vmap, aname, "/configsim", "bms_current", dval);
        amap["bms_voltage"] = vm->setLinkVal(vmap, aname, "/configsim", "bms_voltage", dval);
        amap["bms_power"] = vm->setLinkVal(vmap, aname, "/configsim", "bms_power", dval);

        //     "/components/catl_ems_bms_rw": {
        ival = 1;
        amap["ems_cmd"] = vm->setLinkVal(vmap, aname, "/configsim", "ems_cmd", ival);

        // "/components/catl_mbmu_summary_r": {
        amap["mbmu_voltage"] = vm->setLinkVal(vmap, aname, "/configsim", "mbmu_voltage", dval);
        amap["mbmu_current"] = vm->setLinkVal(vmap, aname, "/configsim", "mbmu_current", dval);

        amap["mbmu_max_cell_voltage"] = vm->setLinkVal(vmap, aname, "/configsim", "mbmu_max_cell_voltage", dval);
        amap["mbmu_min_cell_voltage"] = vm->setLinkVal(vmap, aname, "/configsim", "mbmu_min_cell_voltage", dval);
        amap["mbmu_avg_cell_voltage"] = vm->setLinkVal(vmap, aname, "/configsim", "mbmu_avg_cell_voltage", dval);
        amap["mbmu_max_cell_temperature"] = vm->setLinkVal(vmap, aname, "/configsim", "mbmu_max_cell_temperature",
                                                           dval);
        amap["mbmu_min_cell_temperature"] = vm->setLinkVal(vmap, aname, "/configsim", "mbmu_min_cell_temperature",
                                                           dval);
        amap["mbmu_avg_cell_temperature"] = vm->setLinkVal(vmap, aname, "/configsim", "mbmu_avg_cell_temperature",
                                                           dval);
        amap["mbmu_soc"] = vm->setLinkVal(vmap, aname, "/configsim", "mbmu_soc", dval);
        amap["mbmu_soh"] = vm->setLinkVal(vmap, aname, "/configsim", "mbmu_soh", dval);
        amap["mbmu_max_charge_current"] = vm->setLinkVal(vmap, aname, "/configsim", "mbmu_max_charge_current", dval);
        amap["mbmu_max_discharge_current"] = vm->setLinkVal(vmap, aname, "/configsim", "mbmu_max_discharge_current",
                                                            dval);

        // "/components/catl_bms_ems_r": {
        amap["bms_heartbeat"] = vm->setLinkVal(vmap, aname, "/configsim", "bms_heartbeat", dval);
        amap["bms_timestamp"] = vm->setLinkVal(vmap, aname, "/configsim", "bms_timestamp", tVal);
        amap["bms_poweron"] = vm->setLinkVal(vmap, aname, "/configsim", "bms_poweron", ival);
        amap["bms_status"] = vm->setLinkVal(vmap, aname, "/configsim", "bms_status", ival);
        amap["bms_remain_charge_energy"] = vm->setLinkVal(vmap, aname, "/configsim", "bms_remain_charge_energy", dval);
        amap["bms_remain_discharge_energy"] = vm->setLinkVal(vmap, aname, "/configsim", "bms_remain_discharge_energy",
                                                             dval);
        amap["bms_max_charge_allowed"] = vm->setLinkVal(vmap, aname, "/configsim", "bms_max_charge_allowed", dval);
        amap["bms_max_discharge_allowed"] = vm->setLinkVal(vmap, aname, "/configsim", "bms_max_discharge_allowed",
                                                           dval);
        amap["num_hv_subsystem"] = vm->setLinkVal(vmap, aname, "/configsim", "num_hv_subsystem", dval);
        amap["num_modules"] = vm->setLinkVal(vmap, aname, "/configsim", "num_modules", ival);
        amap["sbmu_total_soc"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_total_soc", dval);
        amap["sbmu_total_soh"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_total_soh", dval);
        amap["sbmu_total_volts"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_total_volts", dval);
        amap["sbmu_total_current"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_total_current", dval);
        amap["sbmu_total_temp"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_total_temp", dval);
        amap["sbmu_min_soc"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_min_soc", dval);
        amap["sbmu_min_soh"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_min_soh", dval);
        amap["sbmu_min_volts"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_min_volts", dval);
        amap["sbmu_min_current"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_min_current", dval);
        amap["sbmu_min_temp"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_min_temp", dval);
        amap["sbmu_max_soc"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_max_soc", dval);
        amap["sbmu_max_soh"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_max_soh", dval);
        amap["sbmu_max_volts"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_max_volts", dval);
        amap["sbmu_max_current"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_max_current", dval);
        amap["sbmu_max_temp"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_max_temp", dval);

        //"/components/catl_mbmu_stat_r": {
        amap["mbmu_status"] = vm->setLinkVal(vmap, aname, "/configsim", "mbmu_status", ival);

        //"/components/catl_mbmu_sum_r": {
        amap["mbmu_warning_1"] = vm->setLinkVal(vmap, aname, "/configsim", "mbmu_warning_1", ival);
        amap["mbmu_warning_21"] = vm->setLinkVal(vmap, aname, "/configsim", "mbmu_warning_21", ival);
        amap["mbmu_warning_22"] = vm->setLinkVal(vmap, aname, "/configsim", "mbmu_warning_22", ival);

        if (reload == 0)  // complete restart
        {
            printf(" reload running on [%s] [%s] \n", __func__, aname);

            dval = 1350.0;
            amap["mbmu_voltage"]->setVal(dval);
            // amap["bms_voltage"]->setVal(dval);
            dval = 65;
            amap["mbmu_soc"]->setVal(dval);
            dval = 100;
            amap["mbmu_soh"]->setVal(dval);
            dval = 0;
            amap["mbmu_current"]->setVal(dval);
            // amap["bms_current"]->setVal(dval);
            // amap["bms_power"]->setVal(dval);
            dval = 0;
            // amap["sbmu_current"]->setVal(dval);

            ival = 0;
            amap["bms_heartbeat"]->setVal(ival);
            amap["bms_timestamp"]->setVal(tVal);
            amap["bms_poweron"]->setVal(ival);
            amap["bms_status"]->setVal(ival);
            amap["bms_remain_charge_energy"]->setVal(ival);
            amap["bms_remain_discharge_energy"]->setVal(ival);
            amap["bms_max_charge_allowed"]->setVal(ival);
            amap["bms_max_discharge_allowed"]->setVal(ival);
            amap["num_hv_subsystem"]->setVal(ival);
            amap["mbmu_status"]->setVal(ival);
            amap["mbmu_warning_1"]->setVal(ival);
            amap["mbmu_warning_21"]->setVal(ival);
            amap["mbmu_warning_22"]->setVal(ival);
            ival = 9;
            amap["num_modules"]->setVal(ival);
        }
        SimSetupBmsModules(vmap, amap, aname, p_fims, aV, "rack_%%02d");
        reload = 2;
        amap[fname]->setVal(reload);
    }

    // now the code starts
    // create current + or - from bms_current and soc
    // etc
    dval = now.count();
    ;  // vm->get_time_dbl();
    // asprintf(&tVal, "the new time is %f", dval);
    // amap["Timestamp"]->setVal(tVal);
    // if(0)FPS_ERROR_PRINT("Heartbeat [%s] val [%f] last set diff [%f]", tVal,
    // dval, amap["Heartbeat"]->getLastSetDiff(dval));
    double soc = amap["mbmu_soc"]->getdVal();
    // int warning_23 = amap["sbmu_warning_23"]->getiVal();
    // if(tVal)free((void*)tVal);
    if (1)  // (amap["Heartbeat"]->getLastSetDiff(dval) > 1.0)
    {
        if (1)
            FPS_ERROR_PRINT("%s >> >> Running val [%f] am->name [%s] soc [%s] [%f]\n", __func__, dval,
                            amap["mbmu_soc"]->getfName(), am->name.c_str(), soc);
        // Set func for asset instances
        ival = 0;
        dval = 0;
        amap["num_hv_subsystem"]->setVal(ival);
        amap["sbmu_total_soc"]->setVal(dval);
        amap["sbmu_total_soh"]->setVal(dval);
        amap["sbmu_total_volts"]->setVal(dval);
        amap["sbmu_total_current"]->setVal(dval);
        amap["sbmu_total_temp"]->setVal(dval);
        amap["sbmu_max_soc"]->setVal(dval);
        amap["sbmu_max_soh"]->setVal(dval);
        amap["sbmu_max_temp"]->setVal(dval);
        amap["sbmu_max_volts"]->setVal(dval);
        amap["sbmu_max_current"]->setVal(dval);
        dval = 20000;
        amap["sbmu_min_temp"]->setVal(dval);
        amap["sbmu_min_volts"]->setVal(dval);
        amap["sbmu_min_current"]->setVal(dval);
        amap["sbmu_min_soh"]->setVal(dval);
        amap["sbmu_min_soc"]->setVal(dval);

        for (auto& iy : am->assetMap)
        {
            asset* ami = iy.second;
            if (0)
                FPS_ERROR_PRINT("%s >> Got asset instance %s. Setting func now...\n", __func__, ami->name.c_str());
            SimHandleSbmu(vmap, ami->amap, ami->name.c_str(), p_fims, aV);
            // am->vm->setFunc(*am->vmap, ami->name.c_str(), "process_sys_alarm",
            // (void*)&process_sys_alarm); am->vm->setFunc(*am->vmap,
            // ami->name.c_str(), "CalculateVar", (void*)&CalculateVar);
            // am->vm->setFunc(*am->vmap, ami->name.c_str(), "CheckMonitorVar",
            // (void*)&CheckMonitorVar);
        }
        ival = amap["num_hv_subsystem"]->getiVal();
        double total_soc = amap["sbmu_total_soc"]->getdVal();
        double avg_soc = 0;
        if (ival > 0)
        {
            avg_soc = total_soc / ival;
            amap["mbmu_soc"]->setVal(avg_soc);
        }

        if (1)
            FPS_ERROR_PRINT("%s >> After loop  [%s] as %d ; avg_soc %2.3f\n", __func__,
                            amap["num_hv_subsystem"]->getfName(), ival, avg_soc);
        // "/components/catl_mbmu_summary_r": {
        if (avg_soc > 0)
        {
            vm->setVal(vmap, "/components/catl_mbmu_summary_r", "mbmu_soc", avg_soc);
        }
        //"/components/catl_bms_ems_r":
        ival = amap["num_hv_subsystem"]->getiVal();
        vm->setVal(vmap, "/components/catl_bms_ems_r", "num_hv_subsystem", ival);

        // // this is just a demo
        // if(warning_23 < 45)
        // {
        //     warning_23++;
        //     amap["sbmu_warning_23"]->setVal(warning_23);
        //     vm->setVal(vmap, "/components/catl_sbmu_1", "sbmu_warning_23"
        //             , warning_23);

        // }
        // get the reference to the variable
        // assetVar* hb = amap["Heartbeat"];
        // assetVar* cd = amap["Timestamp"];
        // assetVar* hbmax = amap["HeartbeatMax"];

        // bool SimPcsComms = amap["SimPcsComms"]->getbVal();
        // bool SimPcsHB = amap["SimPcsHB"]->getbVal();
        // bool SimBmsComms = amap["SimBmsComms"]->getbVal();
        // bool SimBmsHB = amap["SimBmsHB"]->getbVal();
        // bool SimPub = amap["SimPub"]->getbVal();
        // bool SimBms_1Comms = amap["SimBms_1Comms"]->getbVal();
        // bool SimBms_1HB = amap["SimBms_1HB"]->getbVal();
        // bool SimBms_2Comms = amap["SimBms_2Comms"]->getbVal();
        // bool SimBms_2HB = amap["SimBms_2HB"]->getbVal();
        // bool SimBms_3Comms = amap["SimBms_3Comms"]->getbVal();
        // bool SimBms_3HB = amap["SimBms_3HB"]->getbVal();
        // bool SimBms_4Comms = amap["SimBms_4Comms"]->getbVal();
        // bool SimBms_4HB = amap["SimBms_4HB"]->getbVal();

        // double ival;
        // double dvalmax = hbmax->getdVal();
        // dval = hb->getdVal();
        // dval++;
        // if (dval > dvalmax) dval = 0;
        // //if(1)FPS_ERROR_PRINT("Heartbeat %s val %f ", aname, dval);

        // hb->setVal(dval);
        // cd->setVal(dval);
        // dval = hb->getdVal();
        // if (1)FPS_ERROR_PRINT("%s >> aname %s \n"
        //     __func__, aname);
        // //char *sp = cd->getcVal();
        // dval = hb->getdVal();
        // if (SimPcsComms)
        // {
        //     if (0)FPS_ERROR_PRINT("Heartbeat aname %s  val after set %f
        //     SimPcsComms [%s] cd %p p_fims %p\n"
        //         , aname, dval, SimPcsComms?"true":"false", cd, p_fims);
        //     if(SimPub)vm->sendAssetVar(cd, p_fims, "pub", "/components/pcs_sim",
        //     "sys_timestamp"); vm->setVal(vmap, "/components/pcs_sim",
        //     "pcs_timestamp", sp);
        // }
        // if (SimPcsHB)
        // {
        //     if(SimPub)vm->sendAssetVar(hb, p_fims, "pub",
        //     "/components/pcs_sim","sys_heartbeat"); vm->setVal(vmap,
        //     "/components/pcs_sim", "pcs_heartbeat", dval);
        // }
        // //"/components/catl_mbmu_stat_r:bms_heartbeat"

        // if (SimBmsComms)  {
        //     if(SimPub)vm->sendAssetVar(cd, p_fims, "pub", "/components/bms_sim",
        //     "bms_timestamp"); vm->setVal(vmap, "/components/bms_sim",
        //     "bms_timestamp", sp);
        // }
        // if (SimBmsHB)     {
        //     if(SimPub)vm->sendAssetVar(hb, p_fims, "pub", "/components/bms_sim",
        //     "bms_heartbeat"); vm->setVal(vmap, "/components/bms_sim",
        //     "bms_heartbeat", dval);
        // }
    }
    return 0;
}
#endif
