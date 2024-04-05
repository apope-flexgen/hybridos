#ifndef SIMHANDLEHEARTBEAT_CPP
#define SIMHANDLEHEARTBEAT_CPP
// SimHandleHeartbeat.cpp
#include "RNG.hpp"
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

int SimHandleSbmu(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
}

double getSbmuIVoltsFromSoc(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(vmap);
    UNUSED(aname);
    UNUSED(p_fims);
    UNUSED(aV);
    // if soc >= 20
    //      ivolts = max_volts *(1 - (soc-20)* soc1_derate)/100)
    // else
    //    ivolts = max_volts * 20/100 * (soc) *soc_derate2
    // ivolts += temp_volts  (temp -25 ) * temp_coeff
    // ivolts +- rand

    double ivolts = 0.0;
    double bms_volts = amap["bms_voltage"]->getdVal();
    double max_volts = amap["bms_max_voltage"]->getdVal();
    double min_volts = amap["bms_min_voltage"]->getdVal();
    double sbmu_soc = amap["sbmu_soc"]->getdVal();
    double soc_derate1 = amap["soc_derate1"]->getdVal();
    double soc_derate2 = amap["soc_derate2"]->getdVal();
    double soc_v1 = amap["soc_v1"]->getdVal();
    // double temp_coef   = amap["sbmu_temp_coef"]->getdVal();
    // double temp_volts = 0.0;
    // double rand_volts = 0.0;
    // double sbmu_ivolts = amap["sbmu_ivolts"]->getdVal();
    // double sbmu_iohms = amap["sbmu_iohms"]->getdVal();
    // double bms_voltage = amap["bms_voltage"]->getdVal();
    ivolts = max_volts * (1 - (100.0 - sbmu_soc) * soc_derate1 / 100.0);
    FPS_ERROR_PRINT(
        " max_volts [%2.3f]  soc [%2.3f] derate [%2.3f] ivolts "
        "[%2.3f] bms [%2.3f]\n",
        max_volts, sbmu_soc, (100.0 - sbmu_soc) * soc_derate1 / 100.0, ivolts, bms_volts);
    if (sbmu_soc < soc_v1)
    {
        double lowvolts = max_volts * (1 - (soc_v1)*soc_derate1 / 100.0);
        ivolts = min_volts + ((lowvolts - min_volts) * (sbmu_soc)*soc_derate2 / 100.0);
    }

    amap["sbmu_ivolts"]->setVal(ivolts);
    return ivolts;
}

double getSbmuCurrent(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(vmap);
    UNUSED(aname);
    UNUSED(p_fims);
    UNUSED(aV);
    // if sbmu_breaker is closed
    // current = (ivolts - bms_volts) / smbu_iohms
    double current = 0.0;
    double sbmu_ivolts = amap["sbmu_ivolts"]->getdVal();
    double sbmu_iohms = amap["sbmu_iohms"]->getdVal();
    double bms_voltage = amap["bms_voltage"]->getdVal();
    bool sbmu_dc_breaker = amap["sbmu_dc_breaker"]->getbVal();
    bool dc_breaker = amap["dc_breaker"]->getbVal();
    double soc = amap["sbmu_soc"]->getdVal();
    if (dc_breaker && sbmu_dc_breaker)
    {
        current = (sbmu_ivolts - bms_voltage) / sbmu_iohms;
        if (current < 0 && soc > 99.999)
        {
            current = 0;
        }
    }

    return current;
}

double getSbmuIVoltsFromTemp(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    UNUSED(vmap);
    UNUSED(aname);
    UNUSED(p_fims);
    UNUSED(aV);
    // if soc >= 20
    //      ivolts = max_volts *(1 - (soc-20)* soc1_derate)/100)
    // else
    //    ivolts = max_volts * 20/100 * (soc) *soc_derate2
    // ivolts += temp_volts  (temp -25 ) * temp_coeff
    // ivolts +- rand

    double temp_coef = amap["sbmu_temp_coef"]->getdVal();  // 2
    double temp_volts = 0.0;
    // double rand_volts = 0.0;
    double sbmu_ivolts = amap["sbmu_ivolts"]->getdVal();
    double bms_temp = amap["bms_temp"]->getdVal();
    ;
    // double bms_current = amap["bms_current"]->getdVal();
    double sbmu_temp = amap["sbmu_temp"]->getdVal();

    //  sbmu_current = getSbmuCurrent(vmap, amap, aname, p_fims, aV);

    temp_volts = (bms_temp - sbmu_temp) * temp_coef;
    sbmu_ivolts += temp_volts;

    amap["sbmu_ivolts"]->setVal(sbmu_ivolts);
    return sbmu_ivolts;
}
double getSbmuSoc(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    double dval = 0.0;
    const auto now = flex::get_time_dbl();
    double sbmu_cap = amap["sbmu_cap"]->getdVal();
    double bms_volts = amap["bms_voltage"]->getdVal();
    double soc = amap["sbmu_soc"]->getdVal();
    double current = amap["bms_current"]->getdVal();
    double lastTime = amap["lastTime"]->getdVal();

    if (lastTime == 0)
    {
        dval = now.count();
        amap["lastTime"]->setVal(dval);
        // if(fname) free(fname);
        return 0;
    }
    double elapsed = now.count() - lastTime;

    double capacity = (soc / 100) * sbmu_cap;
    amap["capacity"]->setVal(capacity);

    getSbmuIVoltsFromSoc(vmap, amap, aname, p_fims, aV);
    getSbmuIVoltsFromTemp(vmap, amap, aname, p_fims, aV);

    double sbmu_ivolts = amap["sbmu_ivolts"]->getdVal();
    current = getSbmuCurrent(vmap, amap, aname, p_fims, aV);
    double cap_used = (current * elapsed) / (60 * 60);  // check formula

    if (1)
        FPS_ERROR_PRINT("%s >> %s :: soc [%2.3f] SocVolt [%2.3f] curr [%2.3f] power [%2.3f]\n", __func__, aname, soc,
                        sbmu_ivolts, current, current * bms_volts);

    double new_soc = ((capacity - cap_used) / sbmu_cap) * 100;
    if (new_soc < 0.1)
    {
        new_soc = 0.1;
        FPS_ERROR_PRINT("%s >> Limited to 0.1\n", __func__);
    }
    if (new_soc >= 100)
    {
        new_soc = 100.0;
        FPS_ERROR_PRINT("%s >> Limited to 100.0\n", __func__);
    }
    amap["sbmu_soc"]->setVal(new_soc);

    return new_soc;
}

int SimHandleSbmu(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    int reload;
    double dval = 0.0;
    bool bval = false;
    int ival = 0;

    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    char* tVal = nullptr;  // (char*)"Test TimeStamp";
    const auto now = flex::get_time_dbl();
    // need fname to include sbmu_x  so add aname
    char* fname = nullptr;  // = __func__;
                            // //__func__ plus aname
    vm->vmlen = asprintf(&fname, "%s_%s", __func__, (char*)aname);
    assetVar* SimHandleSbmu = amap[fname];
    if (!SimHandleSbmu || (reload = SimHandleSbmu->getiVal()) == 0)
    {
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }

    if (reload < 2)
    {
        // "sbmu_voltage": {
        // "sbmu_current": {
        // "sbmu_max_cell_voltage": {
        // "sbmu_min_cell_voltage": {
        // "sbmu_avg_cell_voltage": {
        // "sbmu_max_cell_temp": {
        // "sbmu_min_cell_temp": {
        // "sbmu_avg_cell_temp": {
        // "sbmu_soc": {
        // "sbmu_max_charge_current": {
        // "sbmu_max_discharge_current": {
        // "sbmu_warning_1": {
        //     "value": 0,
        //     "note":"Sbmu Warning table with degrees reg 1 Appendix 3",
        //     "actions": {
        //         "onSet": [{
        //             "enum": [
        //                 { "shift": 0,"mask": 3,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:single_cell_over_volt", "outValue":
        //                 "Normal"}, { "shift": 0,"mask": 3,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:single_cell_over_volt", "outValue":
        //                 "Warn 1"}, { "shift": 0,"mask": 3,"inValue": 2,"uri":
        //                 "/alarms/@@BMS_ID@@:single_cell_over_volt", "outValue":
        //                 "Warn 2"}, { "shift": 0,"mask": 3,"inValue": 3,"uri":
        //                 "/alarms/@@BMS_ID@@:single_cell_over_volt", "outValue":
        //                 "Warn 3"},

        //                 { "shift": 2,"mask": 3,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:single_cell_under_volt", "outValue":
        //                 "Normal"}, { "shift": 2,"mask": 3,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:single_cell_under_volt", "outValue":
        //                 "Warn 1"}, { "shift": 2,"mask": 3,"inValue": 2,"uri":
        //                 "/alarms/@@BMS_ID@@:single_cell_under_volt", "outValue":
        //                 "Warn 2"}, { "shift": 2,"mask": 3,"inValue": 3,"uri":
        //                 "/alarms/@@BMS_ID@@:single_cell_under_volt", "outValue":
        //                 "Warn 3"},

        //                 { "shift": 4,"mask": 3,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:single_cell_over_temp", "outValue":
        //                 "Normal"}, { "shift": 4,"mask": 3,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:single_cell_over_temp", "outValue":
        //                 "Warn 1"}, { "shift": 4,"mask": 3,"inValue": 2,"uri":
        //                 "/alarms/@@BMS_ID@@:single_cell_over_temp", "outValue":
        //                 "Warn 2"}, { "shift": 4,"mask": 3,"inValue": 3,"uri":
        //                 "/alarms/@@BMS_ID@@:single_cell_over_temp", "outValue":
        //                 "Warn 3"},

        //                 { "shift": 6,"mask": 3,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:single_cell_under_temp", "outValue":
        //                 "Normal"}, { "shift": 6,"mask": 3,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:single_cell_under_temp", "outValue":
        //                 "Warn 1"}, { "shift": 6,"mask": 3,"inValue": 2,"uri":
        //                 "/alarms/@@BMS_ID@@:single_cell_under_temp", "outValue":
        //                 "Warn 2"}, { "shift": 6,"mask": 3,"inValue": 3,"uri":
        //                 "/alarms/@@BMS_ID@@:single_cell_under_temp", "outValue":
        //                 "Warn 3"},

        //                 { "shift": 8,"mask": 3,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:SOC_low", "outValue": "Normal"}, {
        //                 "shift": 8,"mask": 3,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:SOC_low", "outValue": "Warn 1"}, {
        //                 "shift": 8,"mask": 3,"inValue": 2,"uri":
        //                 "/alarms/@@BMS_ID@@:SOC_low", "outValue": "Warn 2"}, {
        //                 "shift": 8,"mask": 3,"inValue": 3,"uri":
        //                 "/alarms/@@BMS_ID@@:SOC_low", "outValue": "Warn 3"},

        //                 { "shift": 10,"mask": 3,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:TMS_fault", "outValue": "Normal"}, {
        //                 "shift": 10,"mask": 3,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:TMS_fault", "outValue": "Warn 1"}, {
        //                 "shift": 10,"mask": 3,"inValue": 2,"uri":
        //                 "/alarms/@@BMS_ID@@:TMS_fault", "outValue": "Warn 2"}, {
        //                 "shift": 10,"mask": 3,"inValue": 3,"uri":
        //                 "/alarms/@@BMS_ID@@:TMS_fault", "outValue": "Warn 3"}
        //             ]
        //         }]
        //     }
        // },
        // "sbmu_warning_21": {
        //     "value": 0,
        //     "note":"Sbmu Warning table without degrees reg 1 Appendix 4",
        //     "actions": {
        //         "onSet": [{
        //             "enum": [
        //                 { "shift": 0,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:charge_overcurrent", "outValue":
        //                 "Normal"}, { "shift": 0,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:charge_overcurrent", "outValue":
        //                 "Warn"}, { "shift": 1,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:discharge_overcurrent", "outValue":
        //                 "Normal"}, { "shift": 1,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:discharge_overcurrent", "outValue":
        //                 "Warn"}, { "shift": 3,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:temp_difference", "outValue":
        //                 "Normal"}, { "shift": 3,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:temp_difference", "outValue":
        //                 "Warn"}, { "shift": 4,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:cell_extreme_temp", "outValue":
        //                 "Normal"}, { "shift": 4,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:cell_extreme_temp", "outValue":
        //                 "Warn"}, { "shift": 5,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:cell_extreme_voltage", "outValue":
        //                 "Normal"}, { "shift": 5,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:cell_extreme_voltage", "outValue":
        //                 "Warn"}, { "shift": 6,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:total_extreme_voltage", "outValue":
        //                 "Normal"}, { "shift": 6,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:total_extreme_voltage", "outValue":
        //                 "Warn"}, { "shift": 7,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:main_positive_relay_close_fail",
        //                 "outValue": "Normal"}, { "shift": 7,"mask": 1,"inValue":
        //                 1,"uri":
        //                 "/alarms/@@BMS_ID@@:main_positive_relay_close_fail",
        //                 "outValue": "Warn"}, { "shift": 8,"mask": 1,"inValue":
        //                 0,"uri":
        //                 "/alarms/@@BMS_ID@@:main_negative_relay_close_fail",
        //                 "outValue": "Normal"}, { "shift": 8,"mask": 1,"inValue":
        //                 1,"uri":
        //                 "/alarms/@@BMS_ID@@:main_negative_relay_close_fail",
        //                 "outValue": "Warn"}, { "shift": 9,"mask": 1,"inValue":
        //                 0,"uri":
        //                 "/alarms/@@BMS_ID@@:main_positive_relay_stuck_fail",
        //                 "outValue": "Normal"}, { "shift": 9,"mask": 1,"inValue":
        //                 1,"uri":
        //                 "/alarms/@@BMS_ID@@:main_positive_relay_stuck_fail",
        //                 "outValue": "Warn"}, { "shift": 10,"mask": 1,"inValue":
        //                 0,"uri":
        //                 "/alarms/@@BMS_ID@@:main_negative_relay_stuck_fail",
        //                 "outValue": "Normal"}, { "shift": 10,"mask": 1,"inValue":
        //                 1,"uri":
        //                 "/alarms/@@BMS_ID@@:main_negative_relay_stuck_fail",
        //                 "outValue": "Warn"}, { "shift": 11,"mask": 1,"inValue":
        //                 0,"uri": "/alarms/@@BMS_ID@@:power_loss", "outValue":
        //                 "Normal"}, { "shift": 11,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:power_loss", "outValue": "Warn"}, {
        //                 "shift": 12,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:balance_function", "outValue":
        //                 "Normal"}, { "shift": 12,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:balance_function", "outValue":
        //                 "Warn"}, { "shift": 15,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:CCAN_communications", "outValue":
        //                 "Normal"}, { "shift": 15,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:CCAN_communications", "outValue":
        //                 "Warn"}
        //             ]
        //         }]
        //     }
        // },
        // "sbmu_warning_22": {
        //     "value": 0,
        //     "note":"Sbmu Warning table without degrees reg 2 Appendix 4",
        //     "actions": {
        //         "onSet": [{
        //             "enum": [
        //                 { "shift": 0,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:SCAN_communications", "outValue":
        //                 "Normal"}, { "shift": 0,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:SCAN_communications", "outValue":
        //                 "Warn"}, { "shift": 2,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:current_sensor_abnormal", "outValue":
        //                 "Normal"}, { "shift": 2,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:current_sensor_abnormal", "outValue":
        //                 "Warn"}, { "shift": 3,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:single_cell_voltage_sampling",
        //                 "outValue": "Normal"}, { "shift": 3,"mask": 1,"inValue":
        //                 1,"uri":
        //                 "/alarms/@@BMS_ID@@:single_cell_voltage_sampling",
        //                 "outValue": "Warn"}, { "shift": 4,"mask": 1,"inValue":
        //                 0,"uri": "/alarms/@@BMS_ID@@:module_temp_sampling",
        //                 "outValue": "Normal"}, { "shift": 4,"mask": 1,"inValue":
        //                 1,"uri": "/alarms/@@BMS_ID@@:module_temp_sampling",
        //                 "outValue": "Warn"}, { "shift": 5,"mask": 1,"inValue":
        //                 0,"uri": "/alarms/@@BMS_ID@@:high_voltage_open_circuit",
        //                 "outValue": "Normal"}, { "shift": 5,"mask": 1,"inValue":
        //                 1,"uri": "/alarms/@@BMS_ID@@:high_voltage_open_circuit",
        //                 "outValue": "Warn"}, { "shift": 6,"mask": 1,"inValue":
        //                 0,"uri": "/alarms/@@BMS_ID@@:MSD_fault", "outValue":
        //                 "Normal"}, { "shift": 6,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:MSD_fault", "outValue": "Warn"}, {
        //                 "shift": 7,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:rack_isolate", "outValue": "Normal"},
        //                 { "shift": 7,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:rack_isolate", "outValue": "Warn"},
        //                 { "shift": 8,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:rack_fuse", "outValue": "Normal"}, {
        //                 "shift": 8,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:rack_fuse", "outValue": "Warn"}
        //             ]
        //         }]
        //     }
        // },
        // "sbmu_warning_23": {
        //     "value": 0,
        //     "note":"Sbmu Warning table without degrees reg 3 Appendix 4",
        //     "actions": {
        //         "onSet": [{
        //             "enum": [
        //                 { "shift": 0,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:TMS_communications", "outValue":
        //                 "Normal"}, { "shift": 0,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:TMS_communications", "outValue":
        //                 "Warn"}, { "shift": 1,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:TMS_mode_conflict",     "outValue":
        //                 "Normal"}, { "shift": 1,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:TMS_mode_conflict",     "outValue":
        //                 "Warn"}, { "shift": 2,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:fire_fault_level_1", "outValue":
        //                 "Normal"}, { "shift": 2,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:fire_fault_level_1", "outValue":
        //                 "Warn"}, { "shift": 3,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:temperature_sensor", "outValue":
        //                 "Normal"}, { "shift": 3,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:temperature_sensor", "outValue":
        //                 "Warn"}, { "shift": 4,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:smoke_sensor", "outValue": "Normal"},
        //                 { "shift": 4,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:smoke_sensor", "outValue": "Warn"},
        //                 { "shift": 5,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:aerosol_state", "outValue":
        //                 "Normal"}, { "shift": 5,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:aerosol_state", "outValue": "Warn"},
        //                 { "shift": 6,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:aerosol_close", "outValue":
        //                 "Normal"}, { "shift": 6,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:aerosol_close", "outValue": "Warn"},
        //                 { "shift": 7,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:aerosol_open", "outValue": "Normal"},
        //                 { "shift": 7,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:aerosol_open", "outValue": "Warn"},
        //                 { "shift": 8,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:rack_door_switch_state", "outValue":
        //                 "Normal"}, { "shift": 8,"mask": 1,"inValue": 1,"uri":
        //                 "/alarms/@@BMS_ID@@:rack_door_switch_state", "outValue":
        //                 "Warn"}, { "shift": 9,"mask": 1,"inValue": 0,"uri":
        //                 "/alarms/@@BMS_ID@@:control_box_oven_temp",
        //                 "outValue": "Normal"}, { "shift": 9,"mask": 1,"inValue":
        //                 1,"uri": "/alarms/@@BMS_ID@@:control_box_oven_temp",
        //                 "outValue": "Warn"}

        // reload = 0;

        amap[fname] = vm->setLinkVal(vmap, aname, "/reload", fname, reload);
        amap["Heartbeat"] = vm->setLinkVal(vmap, aname, "/status", "Heartbeat", dval);
        amap["Timestamp"] = vm->setLinkVal(vmap, aname, "/status", "Timestamp", tVal);
        amap["CommsDummy"] = vm->setLinkVal(vmap, aname, "/status", "CommsDummy", dval);
        amap["SimPcsComms"] = vm->setLinkVal(vmap, aname, "/configsim", "SimPcsComms", bval);
        amap["SimPcsHB"] = vm->setLinkVal(vmap, aname, "/configsim", "SimPcsHB", bval);
        amap["SimBmsComms"] = vm->setLinkVal(vmap, aname, "/configsim", "SimBmsComms", bval);
        amap["SimBmsHB"] = vm->setLinkVal(vmap, aname, "/configsim", "SimBmsHB", bval);
        amap["SimPub"] = vm->setLinkVal(vmap, aname, "/configsim", "SimPub", bval);

        amap["capacity"] = vm->setLinkVal(vmap, aname, "/configsim", "capacity", dval);
        amap["lastTime"] = vm->setLinkVal(vmap, aname, "/configsim", "lastTime", dval);

        amap["sbmu_soc"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_soc", dval);
        amap["sbmu_soh"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_soh", dval);
        amap["sbmu_volts"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_volts", dval);
        amap["sbmu_current"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_current", dval);
        dval = 25.0;
        amap["sbmu_temp"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_temp", dval);
        dval = 52.0;
        amap["sbmu_num_cells"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_num_cells", dval);

        dval = 3.216 * 52.0 * 8.0;  // TODO fix this
        amap["bms_max_voltage"] = vm->setLinkVal(vmap, "bms", "/configsim", "bms_max_voltage", dval);
        dval = 2.216 * 52.0 * 8.0;  // TODO fix this
        amap["bms_min_voltage"] = vm->setLinkVal(vmap, "bms", "/configsim", "bms_max_voltage", dval);
        dval = 3.216;  // TODO fix this
        amap["bms_max_cell_voltage"] = vm->setLinkVal(vmap, "bms", "/configsim", "bms_max_cell_voltage", dval);
        dval = 2.216;  // TODO fix this
        amap["bms_min_cell_voltage"] = vm->setLinkVal(vmap, "bms", "/configsim", "bms_min_cell_voltage", dval);

        amap["bms_current"] = vm->setLinkVal(vmap, "bms", "/configsim", "bms_current", dval);
        amap["bms_voltage"] = vm->setLinkVal(vmap, "bms", "/configsim", "bms_voltage", dval);
        amap["bms_power"] = vm->setLinkVal(vmap, "bms", "/configsim", "bms_power", dval);
        dval = 0.2;
        amap["soc_derate1"] = vm->setLinkVal(vmap, "bms", "/congifsim", "soc_derate1", dval);
        dval = 0.1;
        amap["soc_derate2"] = vm->setLinkVal(vmap, "bms", "/congifsim", "soc_derate2", dval);
        dval = 20;
        amap["soc_v1"] = vm->setLinkVal(vmap, "bms", "/congifsim", "soc_v1", dval);
        dval = 0.0;
        amap["sbmu_warning_1"] = vm->setLinkVal(vmap, aname, "/congifsim", "sbmu_warning_1", ival);
        amap["sbmu_warning_21"] = vm->setLinkVal(vmap, aname, "/congifsim", "sbmu_warning_21", ival);
        amap["sbmu_warning_22"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_warning_22", ival);
        amap["sbmu_warning_23"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_warning_23", ival);
        amap["num_hv_subsystem"] = vm->setLinkVal(vmap, "bms", "/configsim", "num_hv_subsystem", dval);
        amap["sbmu_total_soc"] = vm->setLinkVal(vmap, "bms", "/configsim", "sbmu_total_soc", dval);
        amap["sbmu_max_soc"] = vm->setLinkVal(vmap, "bms", "/configsim", "sbmu_max_soc", dval);
        amap["sbmu_min_soc"] = vm->setLinkVal(vmap, "bms", "/configsim", "sbmu_min_soc", dval);
        amap["sbmu_total_soh"] = vm->setLinkVal(vmap, "bms", "/configsim", "sbmu_total_soh", dval);
        amap["sbmu_max_soh"] = vm->setLinkVal(vmap, "bms", "/configsim", "sbmu_max_soh", dval);
        amap["sbmu_min_soh"] = vm->setLinkVal(vmap, "bms", "/configsim", "sbmu_min_soh", dval);
        amap["sbmu_total_volts"] = vm->setLinkVal(vmap, "bms", "/configsim", "sbmu_total_volts", dval);
        amap["sbmu_max_volts"] = vm->setLinkVal(vmap, "bms", "/configsim", "sbmu_max_volts", dval);
        amap["sbmu_min_volts"] = vm->setLinkVal(vmap, "bms", "/configsim", "sbmu_min_volts", dval);
        amap["sbmu_total_current"] = vm->setLinkVal(vmap, "bms", "/configsim", "sbmu_total_current", dval);
        amap["sbmu_max_current"] = vm->setLinkVal(vmap, "bms", "/configsim", "sbmu_max_current", dval);
        amap["sbmu_min_current"] = vm->setLinkVal(vmap, "bms", "/configsim", "sbmu_min_current", dval);
        amap["sbmu_total_temp"] = vm->setLinkVal(vmap, "bms", "/configsim", "sbmu_total_temp", dval);
        amap["sbmu_max_temp"] = vm->setLinkVal(vmap, "bms", "/configsim", "sbmu_max_temp", dval);
        amap["sbmu_min_temp"] = vm->setLinkVal(vmap, "bms", "/configsim", "sbmu_min_temp", dval);
        bval = true;
        amap["dc_breaker"] = vm->setLinkVal(vmap, "bms", "/configsim", "dc_breaker", bval);

        amap["sbmu_rand"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_rand", dval);
        amap["sbmu_ivolts"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_ivolts", dval);
        amap["sbmu_iohms"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_iohms", dval);
        amap["sbmu_cap"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_cap", dval);
        amap["sbmu_dc_breaker"] = vm->setLinkVal(vmap, aname, "/configsim", "sbmu_dc_breaker", bval);
        amap["bms_temp"] = vm->setLinkVal(vmap, aname, "/configsim", "bms_temp", dval);
        amap["sbmu_temp_coef"] = vm->setLinkVal(vmap, "bms", "/configsim", "sbmu_temp_coef", dval);

        if (reload == 0)  // complete restart
        {
            printf("%s >> [%s] reload running \n", __func__, aname);

            dval = 1350.0;
            amap["sbmu_voltage"]->setVal(dval);
            amap["bms_voltage"]->setVal(dval);
            dval = 65;
            amap["sbmu_soc"]->setVal(dval);
            dval = 100;
            amap["sbmu_soh"]->setVal(dval);
            dval = 0;
            amap["sbmu_current"]->setVal(dval);
            amap["bms_current"]->setVal(dval);
            amap["bms_power"]->setVal(dval);
            amap["lastTime"]->setVal(dval);
            dval = 280.0;
            amap["sbmu_cap"]->setVal(dval);
            dval = RNG::get().randReal(-5.0, 5.0);
            amap["sbmu_rand"]->setVal(dval);
            dval = 00.1;
            amap["sbmu_iohms"]->setVal(dval);
            bval = true;
            amap["sbmu_dc_breaker"]->setVal(bval);
            ival = 0;
            amap["sbmu_warning_1"]->setVal(ival);
            amap["sbmu_warning_21"]->setVal(ival);
            amap["sbmu_warning_22"]->setVal(ival);
            amap["sbmu_warning_23"]->setVal(ival);
        }
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
    double lastTime = amap["lastTime"]->getdVal();
    if (lastTime == 0)
    {
        dval = now.count();
        amap["lastTime"]->setVal(dval);
        if (fname)
            free(fname);
        return 0;
    }
    // double elapsed = now.count() - lastTime;
    // int warning_1 = amap["sbmu_warning_1"]->getiVal();
    // int warning_21 = amap["sbmu_warning_21"]->getiVal();
    // int warning_22 = amap["sbmu_warning_22"]->getiVal();
    // int warning_23 = amap["sbmu_warning_23"]->getiVal();
    double dnum = amap["num_hv_subsystem"]->getdVal();
    dnum++;
    amap["num_hv_subsystem"]->setVal(dnum);

    double soc = amap["sbmu_soc"]->getdVal();
    double total_soc = amap["sbmu_total_soc"]->getdVal();
    total_soc += soc;
    amap["sbmu_total_soc"]->setVal(total_soc);

    double sbmu_max_soc = amap["sbmu_max_soc"]->getdVal();
    double sbmu_min_soc = amap["sbmu_min_soc"]->getdVal();
    if (soc < sbmu_min_soc)
    {
        amap["sbmu_min_soc"]->setVal(soc);
    }
    if (soc > sbmu_max_soc)
    {
        amap["sbmu_max_soc"]->setVal(soc);
    }

    double soh = amap["sbmu_soh"]->getdVal();

    double total_soh = amap["sbmu_total_soh"]->getdVal();
    total_soh += soh;
    amap["sbmu_total_soh"]->setVal(total_soh);
    double sbmu_max_soh = amap["sbmu_max_soh"]->getdVal();
    double sbmu_min_soh = amap["sbmu_min_soh"]->getdVal();
    if (soh < sbmu_min_soh)
    {
        amap["sbmu_min_soh"]->setVal(soh);
    }
    if (soh > sbmu_max_soh)
    {
        amap["sbmu_max_soh"]->setVal(soh);
    }

    double volts = amap["sbmu_volts"]->getdVal();

    double total_volts = amap["sbmu_total_volts"]->getdVal();
    total_volts += volts;
    amap["sbmu_total_volts"]->setVal(total_volts);
    double sbmu_max_volts = amap["sbmu_max_volts"]->getdVal();
    double sbmu_min_volts = amap["sbmu_min_volts"]->getdVal();
    if (volts < sbmu_min_volts)
    {
        amap["sbmu_min_volts"]->setVal(volts);
    }
    if (volts > sbmu_max_volts)
    {
        amap["sbmu_max_volts"]->setVal(volts);
    }
    double current = amap["sbmu_current"]->getdVal();

    double total_current = amap["sbmu_total_current"]->getdVal();
    total_current += current;
    amap["sbmu_total_current"]->setVal(total_current);
    double sbmu_max_current = amap["sbmu_max_current"]->getdVal();
    double sbmu_min_current = amap["sbmu_min_current"]->getdVal();
    if (current < sbmu_min_current)
    {
        amap["sbmu_min_current"]->setVal(current);
    }
    if (current > sbmu_max_current)
    {
        amap["sbmu_max_current"]->setVal(current);
    }
    double temp = amap["sbmu_temp"]->getdVal();

    double total_temp = amap["sbmu_total_temp"]->getdVal();
    total_temp += temp;
    amap["sbmu_total_temp"]->setVal(total_temp);
    double sbmu_max_temp = amap["sbmu_max_temp"]->getdVal();
    double sbmu_min_temp = amap["sbmu_min_temp"]->getdVal();
    if (temp < sbmu_min_temp)
    {
        amap["sbmu_min_temp"]->setVal(temp);
    }
    if (temp > sbmu_max_temp)
    {
        amap["sbmu_max_temp"]->setVal(temp);
    }

    getSbmuSoc(vmap, amap, aname, p_fims, aV);

    if (tVal)
        free((void*)tVal);
    if (1)  // (amap["Heartbeat"]->getLastSetDiff(dval) > 1.0)
    {
        if (1)
            FPS_ERROR_PRINT("%s >> %s :: soc [%s] [%f]\n", __func__, aname, amap["sbmu_soc"]->getfName(), soc);

        // // this is just a demo
        // if(warning_23 < 4)
        // {
        //     warning_23++;
        //     warning_1++;
        //     warning_21++;
        //     warning_22++;
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
    if (fname)
        free(fname);

    return 0;
}
#endif
