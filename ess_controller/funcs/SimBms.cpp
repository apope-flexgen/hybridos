#ifndef SIMHANDLEBMS_CPP
#define SIMHANDLEBMS_CPP

#include "asset.h"
#include "formatters.hpp"
#include "RNG.hpp"

#ifndef FPS_ERROR_FMT
#define FPS_ERROR_FMT(...)     fmt::print(stderr,__VA_ARGS__)
#endif

/**
 * Phil's Simulator
 * 25/06/2021
 * Simulate a Generic BMS unit
 * responds to Start Stop 
 *
 * Used in:
 * Test Script: 
 */
//         SOC    Volts 
//  > 5%    < 15%    3.1 +  (3.4 - 3.1) * (soc /15)
//  >= 15%  < 80%    3.4
//  >= 80%  < 100%   3.4 + (3.6 - 3.4) * ((soc-80)/20)

// python
// for soc in range (1,100):
//   volts = 0.0
//   current = 2.8
//   if soc< 15:
//      volts = 3.1 + (3.4 - 3.1) * (soc/15.0)
//   elif soc < 80:
//      volts = 3.4
//   else:
//     current = 2.8 - 2.8 * (soc-80)/20.0
//     volts = 3.4 + (3.6 - 3.4) * ((soc-80.0) /20.0)
//   print volts, current , volts*current


// set /ess/components/pcs_registers_fast {"current_status":2}    2021-07-06 08:48:15.991769
// set /ess/components/catl_bms_ems_r     {"bms_status":1}        2021-07-06 08:48:15.994855
// set /ess/components/catl_bms_ems_r     {"bms_poweron":0}       2021-07-06 08:48:15.997846
// set  /components/catl_ems_bms_rw       {"ems_cmd":{"value":2}} 2021-07-06 08:48:17.101748
// set /ess/components/catl_bms_ems_r     {"bms_poweron":1}       2021-07-06 08:48:18.7299

// set /components/catl_ems_bms_rw        {"ems_cmd":{"value":1}} 2021-07-06 08:48:20.201921
// set /components/catl_ems_bms_rw        {"ems_cmd":{"value":2}} 2021-07-06 08:48:21.201561
// set /components/catl_ems_bms_rw        {"ems_cmd":{"value":1}} 2021-07-06 08:48:24.102077
// set /components/catl_ems_bms_rw        {"ems_cmd":{"value":2}} 2021-07-06 08:48:25.201726
// set /components/catl_ems_bms_rw        {"ems_cmd":{"value":1}} 2021-07-06 08:48:28.102130

// set /ess/components/pcs_registers_fast {"current_status":3}    2021-07-06 08:48:28.18239

// set /components/catl_ems_bms_rw        {"ems_cmd":{"value":2}} 2021-07-06 08:48:29.201626

// set /ess/components/pcs_registers_fast {"current_status":18}   2021-07-06 08:48:29.22987

// set /components/catl_ems_bms_rw        {"ems_cmd":{"value":1}} 2021-07-06 08:48:32.201679
// set /components/catl_ems_bms_rw        {"ems_cmd":{"value":2}} 2021-07-06 08:48:33.101942
// set /ess/components/pcs_registers_fast {"current_status":3}    2021-07-06 08:48:34.27697

// set /components/catl_ems_bms_rw        {"ems_cmd":{"value":1}} 2021-07-06 08:48:36.101799

// set /ess/components/pcs_registers_fast  {"current_status":4}   2021-07-06 08:48:37.33031
// set /ess/components/pcs_registers_fast  {"current_status":6}   2021-07-06 08:48:42.37560





// ./fims_send -m set -r /me -u /ess/assets/pcs/summary/start false
// ./fims_send -m set -r /me -u /ess/assets/bms/summary/start false
// ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast/vdc_bus_1 1400
// ./fims_send -m set -r /me -u /ess/status/bms/BMSCurrent 0
// # ./fims_send -m set -r /me -u /ess/components/pcsm_dc_inputs/vdc_bus_1 '{"value":1300}'

// echo "PCS Off, BMS Off, Batteries normal"
// ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":2}'                 # PCS is ready
// ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_status":1}'
// # BMS is powered off batteries normal
// ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":0}'

// echo "BMS Start command"
// ./fims_send -m set -r /me -u /ess/assets/bms/summary/start true
// echo "Waiting 5 seconds"
// sleep 5
// echo "BMS Turning On"
// ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":1}'
// echo "Waiting 5 seconds"
// sleep 5
// echo "PCS Start command"
// ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":4}'
// ./fims_send -m set -r /me -u /ess/assets/pcs/summary/start true
// echo "Waiting 5 seconds for PCS to respond"
// sleep 5
// # echo "PCS Precharging for 5 seconds"
// # ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":3}'
// # sleep 5

// # echo "PCS going to ready"
// # ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_stat
// us":4}'
// # echo "PCS Start command"
// # ./fims_send -m set -r /me -u /ess/assets/pcs/summary/start true
// # echo "Waiting 10 seconds"
// # sleep 10
// echo "PCS Turning On"
// ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status
// ":6}'                 # PCS is On
// sleep 3
// # ./fims_send -m set -r /me -u /ess/components/catl_mbmu_summary_r/mbmu_min_cell_
// voltage 2.4
// # ./fims_send -m set -r /me -u /ess/components/gpio/EStop true
// ./fims_send -m set -r /me -u /ess/status/bms/BMSCurrent 3501
// sleep 3
// ./fims_send -m set -r /me -u /ess/components/gpio/EStop false
// echo "PCS going to Ready in 5 seconds"
// sleep 5
// ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status
// ":4}'
// echo "PCS going to Fault in 5 seconds"
// sleep 5
// # ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_stat
// us":9}'
// # echo "Waiting 5 seconds"
// # sleep 5
// # ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":0}'

// # ./fims_send -m set -r /me -u /ess/assets/pcs/summary/shutdown false
// # echo "PCS Stop Command"
// # ./fims_send -m set -r /me -u /ess/assets/pcs/summary/shutdown true
// # echo "Waiting 7 seconds"
// # sleep 7
// # echo "PCS going to ready state"
// # ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_stat
// us":4}'
// # echo "BMS Stop Command in 5 seconds"
// # ./fims_send -m set -r /me -u /ess/assets/bms/summary/stop false
// # sleep 5
// # ./fims_send -m set -r /me -u /ess/assets/bms/summary/stop true
// # echo "Waiting 10 seconds"
// # sleep 10
// # echo "BMS Turning Off"
// # ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":0}'
//                  # BMS is Off

// # ./fims_send -m set -r /me -u /ess/assets/bms/summary/start false
// # echo "PCS Off, BMS Off, Batteries normal"
// # ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":2}' && ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_status":1}' && ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":0}'

//Method:  set
//Uri:     /components/pcs_registers_fast
//ReplyTo: (null)
//Body:    {"drop_grad_p_enable":{"value":1},"start_grad_p_enable":{"value":1},"start_grad_q_enable":{"value":1},"stop_grad_p_enable":{"value":1},"stop_grad_q_enable":{"value":1}}
//Timestamp:   2021-06-27 04:29:49.427936
//Method:  set
//Uri:     /components/pcs_registers_fast
//ReplyTo: (null)
//Body:    {"drop_grad_p_enable":{"value":1},"start_grad_p_enable":{"value":1},"start_grad_q_enable":{"value":1},"stop_grad_p_enable":{"value":1},"stop_grad_q_enable":{"value":1}}
//Timestamp:   2021-06-27 04:29:49.427975

 extern "C++" {

    int SimHandleBms(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);
    int SimHandleBmsRack(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV);

};

//         SOC    Volts 
//  > 5%    < 15%    3.1 +  (3.4 - 3.1) * (soc /15)
//  >= 15%  < 80%    3.4
//  >= 80%  < 100%   3.4 + (3.6 - 3.4) * ((soc-80)/20)

// python
// for soc in range (1,100):
//   volts = 0.0
//   current = 2.8
//   if soc< 15:
//      volts = 3.1 + (3.4 - 3.1) * (soc/15.0)
//   elif soc < 80:
//      volts = 3.4
//   else:
//     current = 2.8 - 2.8 * (soc-80)/20.0
//     volts = 3.4 + (3.6 - 3.4) * ((soc-80.0) /20.0)
//   print volts, current , volts*current
//
// Discharge is simulated by providing a dummy impedance 
// each rack will provide currrent into that impedance based on the bms_volts
// this will provide an estimate of the discharge performance of this rack
// given a desired overall discharge power we'll adjust the bms_ohms and then perform the soc change
// if this is an estimate run then the soc is ignored
// if this is the real run then the bms_soc is adjusted   
// amap["discharge_test"]->getbVal();
// amap["bms_discharge_current"]->setVal(discharge_current);
// amap["bms_discharge_power"]->setVal(discharge_power);
// amap["bms_discharge_soc"]->setVal(discharge_power);
// amap["total_discharge_power"]->setVal(total_discharge_power);
// amap["total_discharge_current"]->setVal(total_discharge_current);
int SimHandleBmsDischarge(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    //int reload;
    double dval = 0.0;
    //bool bval = false;
    //bool showMod = false;
    const auto now = flex::get_time_dbl();
    double tNow = now.count();
    double lastTime              = amap["lastTime"]->getdVal();
    double bms_volts             = amap["bms_volts"]->getdVal();
    double bms_ohms              = amap["bms_ohms"]->getdVal();
    double bms_soc               = amap["bms_soc"]->getdVal();
    bool discharge_test          = amap["discharge_test"]->getbVal();
    // The discharge request
    double  bms_discharge_power_cmd    = amap["bms_discharge_power_cmd"]->getdVal();
    // our allocated part
    double  bms_discharge_power        = amap["bms_discharge_power"]->getdVal();
    //double  bms_discharge_current      = amap["bms_discharge_current"]->getdVal();

    //this is the current we'll take  should be maxed out at 280
    double discharge_current = bms_discharge_power/ bms_volts;
    // something went wrong 
    if(bms_soc > 0.01)
    {
        if(discharge_current > 300.0)
        {
            dval = 280.0;
            FPS_ERROR_FMT("\n{} bms_ohms error {:2.3f}; limiting discharge current from {:2.3f} to {:2.3f}\n"
            , __func__
            , bms_ohms
            , discharge_current
            , dval
            );
            discharge_current = dval;
        }
    }

    double discharge_power = bms_volts * discharge_current;

    if(bms_soc < 0.01)
    {
        discharge_current = 0.0;
        discharge_power = 0.0;
    
    }
    if(bms_soc > 0.01)
    {
        FPS_ERROR_FMT(
            "{} >> {} volts {:2.3f} ohms {:2.3f} dc {:2.3f} dp {:2.3f} bms_soc {:2.3f}\n"
            , __func__
            , aname
            , bms_volts
            , bms_ohms
            , discharge_current
            , discharge_power
            , bms_soc
            );
    }
    FPS_ERROR_FMT(
        "{} >> {} bms_dp_cmd {:2.3f} bms_dp {:2.3f} dp {:2.3f} bms_soc {:2.3f} test {}\n"
        , __func__
        , aname
        , bms_discharge_power_cmd
        , bms_discharge_power
        , discharge_power
        , bms_soc
        , discharge_test
        );

    amap["bms_discharge_current"]->setVal(discharge_current);
    amap["bms_discharge_power"]->setVal(discharge_power);

    discharge_power *=(tNow-lastTime);
    amap["bms_discharge_soc"]->setVal(discharge_power);
    if(!discharge_test)
    {
        bms_soc -= discharge_power;
        if(bms_soc < 0.0 )
        {
            bms_soc = 0.0;
        }
        amap["bms_soc"]->setVal(bms_soc);
    }
    // this is an accumulation of available discharge power
    double total_discharge_power = amap["total_discharge_power"]->getdVal();
    if(discharge_test)
    {
        total_discharge_power += (bms_volts * 280.0  *(bms_soc/100.0));
    }
    else
    {
        total_discharge_power += (bms_volts * discharge_current);
    }
    amap["total_discharge_power"]->setVal(total_discharge_power);
    double total_discharge_current = amap["total_discharge_current"]->getdVal();
    total_discharge_current += discharge_current;
    amap["total_discharge_current"]->setVal(total_discharge_current);
    return 0;
}

// turns soc into a rack voltage and calculates charge power  and charge current 
int SimHandleBmsSoc(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    //int reload;
    //double dval = 0.0;
    //bool bval = false;
    //bool showMod = false;
    double soc = amap["bms_soc"]->getdVal();
    double volts = 3.4;
    double current =  amap["bms_base_current"]->getdVal();
    double base_current =  amap["bms_base_current"]->getdVal();
    double power;
    int num_cells = amap["bms_num_cells"]->getiVal();
    double soc_min =  amap["bms_soc_min"]->getdVal();  // 15.0
    double soc_max =  amap["bms_soc_max"]->getdVal();  // 80.00
    double cell_volt_min =  amap["bms_cell_volt_min"]->getdVal();  // 3.1
    double cell_volt_nom =  amap["bms_cell_volt_nom"]->getdVal();  // 3.4
    double cell_volt_max =  amap["bms_cell_volt_max"]->getdVal();  // 3.6
    if(soc < soc_min)
    {
        volts = cell_volt_min + (cell_volt_nom - cell_volt_min) * (soc/soc_min);
    }
    if(soc > soc_max)
    {
        current = base_current - base_current * (soc-soc_max)/(100.0 - soc_max);
        volts = cell_volt_nom + (cell_volt_max - cell_volt_nom) * ((soc-soc_max)/(100.0 - soc_max));
    }
    power = volts*current*num_cells;
    volts = volts*num_cells;
    amap["bms_charge_current"]->setVal(current);
    amap["bms_volts"]->setVal(volts);
    amap["bms_charge_power"]->setVal(power);
    return 0;
}
// we simply are asked for a discharge or charge current
// we dischargs the current and deplete our SOC
// we charge the current and increase our SOC 
// pick this up after th basic sequence is completed.
int SimHandleBmsRack(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    int reload;
    double dval = 0.0;
    bool bval = false;
    bool showMod = false;

    int ival = 0;
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;
    //char* fname = nullptr;//(char*)"Test TimeStamp";
    auto fName = fmt::format("{}_{}", __func__, aname);
    char* fname= (char*)fName.c_str();
    assetVar* SimHandleBmsRack = amap[fname];

    if (!SimHandleBmsRack || (reload = SimHandleBmsRack->getiVal()) == 0)
    {
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }

    if (reload < 2)
    {
        // these are at the pcs level
    // double pcs_power_increase_slope  = amap["pcs_power_increase_slope"]->getdVal();
    // double pcs_power_decrease_slope  = amap["pcs_power_decrease_slope"]->getdVal();
    // int current_status               = amap["current_status"]->getiVal();
    // // pcs_volts is now fixed at 1350 ;
    // double pcs_volts                 = amap["pcs_volts"]->getdVal();


    // // these are at the module level
    // double MaxPCSActivePower    = amap["MaxPCSActivePower"]->getdVal();
    // double PCSActivePowerCmd    = amap["PCSActivePowerCmd"]->getdVal();
    // double pcs_power            = amap["pcs_power"]->getdVal();
    // double pcs_loss             = amap["pcs_loss"]->getdVal(); // in % ie 5 or 6 %
    // double pcs_current          = amap["pcs_current"]->getdVal();

        amap[fname]                  = vm->setLinkVal(vmap, aname,     "/reload",  fname, reload);
        amap["num_running_racks"]    = vm->setLinkVal(vmap, "bms",     "/configsim",    "num_running_racks",   dval);
        amap["bms_total_current"]    = vm->setLinkVal(vmap, "bms",     "/configsim",    "bms_total_current",   dval);
        amap["bms_total_power"]      = vm->setLinkVal(vmap, "bms",     "/configsim",    "bms_total_power",     dval);
        amap["bms_volts"]            = vm->setLinkVal(vmap, aname,     "/configsim",    "bms_volts",           dval);
        amap["bms_ohms"]             = vm->setLinkVal(vmap, "bms",     "/configsim",    "bms_ohms",            dval);
        amap["PCSActivePowerCmd"]    = vm->setLinkVal(vmap, "pcs",     "/configsim",    "PCSActivePowerCmd",   dval);
        amap["lastTime"]             = vm->setLinkVal(vmap, "bms",     "/configsim",    "lastTime",            dval);
        amap["bms_power_cmd"]        = vm->setLinkVal(vmap, aname,     "/configsim",    "bms_power_cmd",       dval);
        amap["bms_max_power"]        = vm->setLinkVal(vmap, aname,     "/configsim",    "bms_max_power",       dval);
        amap["bms_power"]            = vm->setLinkVal(vmap, aname,     "/configsim",    "bms_power",           dval);
        amap["bms_current"]          = vm->setLinkVal(vmap, aname,     "/configsim",    "bms_current",         dval);
        amap["bms_internal_current"] = vm->setLinkVal(vmap, aname,     "/configsim",    "bms_internal_current",       dval);
        amap["bms_capacity"]         = vm->setLinkVal(vmap, aname,  "/configsim", "bms_capacity", dval);
        amap["bms_max_capacity"]     = vm->setLinkVal(vmap, aname,  "/configsim", "bms_max_capacity", dval);
        amap["bms_min_capacity"]     = vm->setLinkVal(vmap, aname,  "/configsim", "bms_min_capacity", dval);
        amap["bms_soc"]              = vm->setLinkVal(vmap, aname,  "/configsim", "bms_soc", dval);
        dval =  280.0;
        amap["bms_base_current"]     = vm->setLinkVal(vmap, "bms",  "/configsim", "bms_base_current", dval);
        ival = 8 * 52;
        amap["bms_num_cells"]        = vm->setLinkVal(vmap, "bms",  "/configsim", "bms_num_cells", ival);
        dval =  15.0;
        amap["bms_soc_min"]         = vm->setLinkVal(vmap, "bms",  "/configsim", "bms_soc_min", dval);
        dval =  80.0;
        amap["bms_soc_max"]         = vm->setLinkVal(vmap, "bms",  "/configsim", "bms_soc_max", dval);
        dval =  3.1;
        amap["bms_cell_volt_min"]   = vm->setLinkVal(vmap, "bms",  "/configsim", "bms_cell_volt_min", dval);
        dval =  3.4;
        amap["bms_cell_volt_nom"]   = vm->setLinkVal(vmap, "bms",  "/configsim", "bms_cell_volt_nom", dval);
        dval =  3.6;
        amap["bms_cell_volt_max"]   = vm->setLinkVal(vmap, "bms",  "/configsim", "bms_cell_volt_max", dval);

        amap["bms_loss"]             = vm->setLinkVal(vmap, aname,     "/configsim",    "bms_loss",         dval);
        amap["module_enabled"]       = vm->setLinkVal(vmap, aname,     "/configsim",    "module_enabled", bval);

        amap["module_temp"]          = vm->setLinkVal(vmap, aname,     "/configsim",    "module_temp", dval);
        amap["module_cool"]          = vm->setLinkVal(vmap, aname,     "/configsim",    "module_cool", dval);
        amap["temp_inc"]             = vm->setLinkVal(vmap, aname,     "/configsim",    "temp_inc", dval);
        dval = -100.0;
        amap["bms_temp"]             = vm->setLinkVal(vmap, "bms",     "/configsim",    "bms_temp",       dval);
        dval = 1.0;
        amap["temp_coeff"]           = vm->setLinkVal(vmap, "bms",     "/configsim",    "temp_coeff",       dval);
        amap["max_cool"]             = vm->setLinkVal(vmap, "bms",     "/configsim",    "max_cool", dval);
        amap["min_cool"]             = vm->setLinkVal(vmap, "bms",     "/configsim",    "min_cool", dval);
        amap["module_rand"]          = vm->setLinkVal(vmap, aname,     "/configsim",    "module_rand", dval);
        
        //amap["pcs_power_increase_slope"] = vm->setLinkVal(vmap, "pcs",  "/configsim", "pcs_power_increase_slope", dval);
        //amap["pcs_power_decrease_slope"] = vm->setLinkVal(vmap, "pcs",  "/configsim", "pcs_power_decrease_slope", dval);
        amap["current_status"]       = vm->setLinkVal(vmap, "bms",  "/configsim", "current_status", dval);

        amap["bms_charge_current"]   = vm->setLinkVal(vmap, aname,     "/configsim",    "bms_charge_current", dval);
        amap["bms_charge_power"]     = vm->setLinkVal(vmap, aname,     "/configsim",    "bms_charge_power", dval);
 
        bval = true;
        amap["discharge_test"]       = vm->setLinkVal(vmap, "bms",     "/configsim",    "discharge_test", bval);
        dval = 0.0;
        amap["bms_discharge_current"]  = vm->setLinkVal(vmap, aname,   "/configsim",    "bms_discharge_current", dval);
        //amap["bms_discharge_power"]    = vm->setLinkVal(vmap, aname,   "/configsim",    "bms_discharge_power", dval);
        amap["bms_discharge_soc"]      = vm->setLinkVal(vmap, aname,   "/configsim",    "bms_discharge_soc", dval);
        amap["total_discharge_power"]  = vm->setLinkVal(vmap, "bms",   "/configsim",    "total_discharge_power", dval);
        amap["total_discharge_current"]= vm->setLinkVal(vmap, "bms",   "/configsim",    "total_discharge_current", dval);
        amap["bms_discharge_power_cmd"]= vm->setLinkVal(vmap, "bms",   "/configsim",    "bms_discharge_power_cmd", dval);
        amap["bms_discharge_power"]    = vm->setLinkVal(vmap, aname,   "/configsim",    "bms_discharge_power", dval);


        if(reload < 1)
        {
            bval= true;
            amap["module_enabled"]->setVal(bval);

            dval = 5.0 + RNG::get().randReal(-2.0, 2.0);
            amap["bms_loss"]->setVal(dval);

            dval = 25.0;
            amap["module_temp"]->setVal(dval);
            dval = 5000000.0;
            amap["module_cool"]->setVal(dval);
            dval = amap["bms_temp"]->getdVal();
            if(dval < -50)
            {
                dval = 25;
                amap["bms_temp"]->setVal(dval);
                dval = 6000000.0;
                amap["max_cool"]->setVal(dval);
                dval = 4000000.0;
                amap["min_cool"]->setVal(dval);
            }
            dval = RNG::get().randReal(0.0, 10.0)/10.0;
            amap["module_rand"]->setVal(dval);
            // pretend to have an efficiency
            //double eff =  93.0 + dval*4.0;
            //amap["module_eff"]->setVal(eff); 
        }
        showMod = true;
        reload = 2;
        amap[fname]->setVal(reload); 
    }
    // we need to provide module_power
    // we are given module_volts  and a power ; current  = power/vdc_bus_1
    // power_out 
    bool module_enabled = amap["module_enabled"]->getbVal();
    if(module_enabled)
    {
        // double module_eff = 1;// amap["module_eff"]->getdVal();
        // double module_power = amap["module_power"]->getdVal();
        // double module_volts = amap["module_volts"]->getdVal();
        // //double module_iohms = amap["module_iohms"]->getdVal();
        // double module_current = (module_power *100.0) /(module_volts*module_eff);
        // amap["module_current"]->setVal(module_current);
        // double module_output_power = module_volts * module_current;
        // amap["module_output_power"]->setVal(module_output_power);
        // // double pcs_output_power = amap["pcs_output_power"]->getdVal();
        // // pcs_output_power += module_output_power;
        // // amap["pcs_output_power"]->setVal(pcs_output_power);

        // // double pcs_current = amap["pcs_current"]->getdVal();
        // // pcs_current += module_current;
        // // amap["pcs_current"]->setVal(pcs_current);

        // double module_temp = amap["module_temp"]->getdVal();
        // double module_cool = amap["module_cool"]->getdVal();
        // double temp_coeff = amap["temp_coeff"]->getdVal();
        // double pcs_temp = amap["pcs_temp"]->getdVal();
        // double temp_inc = (module_power - module_cool) * temp_coeff;
        // double max_cool = amap["max_cool"]->getdVal();
        // double min_cool = amap["min_cool"]->getdVal();

        // amap["temp_inc"]->setVal(temp_inc);
        // module_temp += temp_inc;
        // // TODO cause module_cool to increase as module temp increases up to a maximum
        // if(module_temp > (pcs_temp+60))
        // {
        //     module_cool *= 1.1;
        //     if(module_cool > max_cool)
        //     {
        //         module_cool = max_cool;
        //     }
        // }
        // else
        // {
        //     module_cool *= 0.9;
        //     if(module_cool < min_cool)
        //     {
        //         module_cool = min_cool;
        //     }
        // }
        // amap["module_cool"]->setVal(module_cool);

        // if(module_temp < pcs_temp)
        // {
        //     module_temp = pcs_temp;
        // }

        //amap["module_temp"]->setVal(module_temp);
        ival = amap["num_running_racks"]->getiVal();
        ival++;
        amap["num_running_racks"]->setVal(ival);
    }
    // else
    // {
    //     dval = 0;
    //     // amap["module_current"]->setVal(dval);
    //     // amap["module_volts"]->setVal(dval);
    //     // amap["module_power"]->setVal(dval);
    //     // amap["module_output_power"]->setVal(dval);
    // }

    // double module_output_power = amap["module_output_power"]->getdVal();
    // double pcs_output_power = amap["pcs_output_power"]->getdVal();
    if(showMod)FPS_ERROR_FMT(" {} >> aname [{}] am [{}] enabled [{}]-> {}  "
       //"output power module [{:2.3f}] pcs [{:2.3f}]"
       " \n"
        , __func__, aname, am->name
        , amap["module_enabled"]->getfName()
        , module_enabled
        // , module_output_power
        // , pcs_output_power
        );

    return 0;
}


// uses num_modules ( mappes to num_hv_systems) to set up the amaps and assets for Pcs modules
int SimSetupBmsRacks(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV
        , const char* modfmt )
{
    //const auto now = flex::get_time_dbl();
    int num_racks = amap["num_racks"]->getiVal(); 
    asset_manager*am = aV->am;
    VarMapUtils* vm = am->vm;

    if(1)FPS_ERROR_FMT("{} >> >> Running manager [{}] vm [{}] aname [{}] num_modules {} fmt [{}]\n"
            , __func__, am->name, static_cast<void*>(am->vm), aname, num_racks, modfmt);
    //int ival = 0;
    //double dval = 0;
    //amap["num_running_modules"]->setVal(ival); 
    //amap["pcs_power"]->setVal(dval); 
    int idx = 1;
    while(idx <= num_racks)
    {
        auto mname = fmt::format(modfmt, idx);
        asset* ami = vm->getaI(vmap, mname.c_str());
        if(1)FPS_ERROR_FMT("{} >> >> seeking  [{}] ami {}\n"
            , __func__, mname, static_cast<void*>(ami));
        idx++;
        if (!ami)
        {
            if(1)FPS_ERROR_FMT("{} >> >> creating instance [{}] for am [{}]\n"
                , __func__, mname, am->name);
            ami = am->addInstance(mname.c_str());
            ami->setVmap(&vmap);
            ami->setVm(vm);
            ami->am = am;
            ami->p_fims = p_fims;
            vm->setaI(vmap, mname.c_str(), ami);
        }
    }

    return 0;
}

// if current_status == 3 (precharge) and i_dc_volts < v_dc_volts && contact_closed = false
// then increase dc_volts by dc_charge slope
// If we are ready close contactors if needed
// if we have a charge / discharge request then ste PCS power accordingly.
// For now Max Charge/Discharge is 350000 W 
// current_status = 0 Power UP
    // current_status = 1 Init
    // current_status = 2 Off
    // current_status = 3 Precharge
    // current_status = 4 Ready          close and then set pcs_volts 
    // current_status = 5 Wait
    // current_status = 6 On             set pcs_volts
    // current_status = 7 Stop
    // current_status = 8 Discharge
    // current_status = 9 Fault

//This could be for the whole PCS or for a module
// hence the power of the amap
// we are asked for a given current we provide that but our internal is 100+loss/100 current
// if SOC > min_soc we provide the power 
int SimHandleBmsPower(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    asset_manager*am = aV->am;
    VarMapUtils*vm = am->vm;
    // 1350 volts = 3500000 wats = 2592.59 amps

    const auto now = flex::get_time_dbl();
    double tNow = now.count();
    double lastTime             = amap["lastTime"]->getdVal();

    // these are at the bms level
    int current_status               = amap["current_status"]->getiVal();
    // pcs_volts is now fixed at 1350 ;
    double bms_volts                 = amap["bms_volts"]->getdVal();


    // these are at the module level
    //double bms_max_power        = amap["bms_max_power"]->getdVal();
    //double bms_power_cmd        = amap["bms_power_cmd"]->getdVal();
    double bms_power            = amap["bms_power"]->getdVal();
    double bms_loss             = amap["bms_loss"]->getdVal(); // in % ie 5 or 6 %
    double bms_current          = amap["bms_current"]->getdVal();
    double bms_internal_current = amap["bms_internal_current"]->getdVal();
    //double bms_total_current    = amap["bms_total_current"]->getdVal();
    double bms_capacity         = amap["bms_capacity"]->getdVal();
    double bms_max_capacity     = amap["bms_max_capacity"]->getdVal();
    double bms_min_capacity     = amap["bms_min_capacity"]->getdVal();
    double bms_soc              = amap["bms_soc"]->getdVal();
    double bms_power_adjust = 0;
    if (current_status == 6)
    {
        double timeDiff = tNow-lastTime;
        if(bms_current > 0 )
        {
            bms_internal_current = bms_current * (100.0 + bms_loss)/ 100.0;
        } 
        else
        {
            bms_internal_current = bms_current * (100.0 - bms_loss)/ 100.0;
        } 

        bms_power_adjust = bms_internal_current * bms_volts * timeDiff;
        bms_capacity += bms_power_adjust;
        if(bms_capacity > bms_max_capacity)
        {
            bms_capacity = bms_max_capacity;
        }
        if(bms_capacity < bms_min_capacity)
        {
            bms_capacity = bms_min_capacity;
            if(bms_current > 0)
            {
                bms_current  = 0.0;
                bms_internal_current  = 0.0;
            }
        }
        bms_soc = bms_capacity * 100.0 / bms_max_capacity;  
        amap["bms_soc"]->setVal(bms_soc);
        amap["bms_capacity"]->setVal(bms_capacity);
        amap["bms_internal_current"]->setVal(bms_internal_current);

        FPS_ERROR_FMT("{} >> bms_internal_current {} loss {} \n", __func__, bms_internal_current, bms_loss);

        double bms_total_current   = amap["bms_total_current"]->getdVal();
        double bms_total_power     = amap["bms_total_power"]->getdVal();
        bms_total_current += bms_current;
        bms_total_power += bms_power;
        amap["bms_total_current"]->setVal(bms_total_current);
        amap["bms_total_power"]->setVal(bms_total_power);

    }
    if (true)
    {
        varsmap* vlist = vm->createVlist();
        vm->addVlist(vlist, amap["heartbeat"]);
        vm->addVlist(vlist, amap["Timestamp"]);
        //vm->addVlist(vlist, amap["current_status"]);
        vm->addVlist(vlist, amap["current_fault"]);
        vm->addVlist(vlist, amap["current_warning"]);
        vm->addVlist(vlist, amap["bms_soc"]);
        vm->addVlist(vlist, amap["bms_current"]);
        vm->addVlist(vlist, amap["bms_internal_current"]);
        vm->addVlist(vlist, amap["bms_power"]);
        vm->addVlist(vlist, amap["bms_volts"]);
        vm->addVlist(vlist, amap["bms_charge_current"]);
        vm->addVlist(vlist, amap["bms_charge_power"]);
        //vm->addVlist(vlist, amap["bms_total_current"]);
        //vm->addVlist(vlist, amap["bms_total_power"]);
        vm->sendVlist(p_fims, "pub", vlist);
        vm->clearVlist(vlist);
    }
    return 0;
}
// if current_status == 3 (precharge) and i_dc_volts < v_dc_volts && contact_closed = false
// then increase dc_volts by dc_charge slope

int SimHandleBmsHeartbeat(varsmap& vmap, varmap& amap, const char* aname, fims* p_fims, assetVar* aV)
{
    //asset_manager*am = aV->am;
    //VarMapUtils*vm = am->vm;
    const auto now = flex::get_time_dbl();
    double tNow = now.count();
    int heartbeat               = amap["heartbeat"]->getiVal();
    int heartbeatMax            = amap["heartbeatMax"]->getiVal();
    double heartbeatInterval    = amap["heartbeatInterval"]->getdVal();
    double heartbeatTime        = amap["heartbeatTime"]->getdVal();

    
    if((tNow - heartbeatTime) > heartbeatInterval)
    {
        heartbeat++;
        if (heartbeat > heartbeatMax)
        {
            heartbeat = 0; // heartbeatMin perhaps
        }
        auto tval = fmt::format("{} time [{:2.3f}]", __func__, tNow);
        char* tVal = (char*)tval.c_str();//(char*)"Test TimeStamp";
        amap["Timestamp"]->setVal(tVal);
        amap["heartbeatTime"]->setVal(tNow);
        amap["heartbeat"]->setVal(heartbeat);
    }
    return 0;
}
// if current_status == 3 (precharge) and i_dc_volts < v_dc_volts && contact_closed = false
// then increase dc_volts by dc_charge slope

// int SimHandlePcsPrecharge(varsmap& vmap, varmap& amap, const char* xaname, fims* p_fims, assetVar* aV)
// {
//     const auto now = flex::get_time_dbl();
//     double tNow = now.count();
//     int current_status    = amap["current_status"]->getiVal();
//     double vdc_bus_1      = amap["vdc_bus_1"]->getdVal();

//     double vdc_bus        = amap["vdc_bus"]->getdVal();
//     double min_vdc_bus    = amap["min_vdc_bus"]->getdVal();
//     double vdc_charge     = amap["vdc_charge"]->getdVal();
//     double vdc_discharge  = amap["vdc_discharge"]->getdVal();
//     double lastTime       = amap["lastTime"]->getdVal();
//     bool vdc_contact      = amap["vdc_contact"]->getbVal();

//     if((current_status == 3) && (vdc_contact == false))
//     {
//         if (vdc_bus < vdc_bus_1)
//         {
//             double interval = tNow-lastTime;
//             vdc_bus += interval*vdc_charge;
//             if(vdc_bus > vdc_bus_1) vdc_bus = vdc_bus_1;
//             amap["vdc_bus"]->setVal(vdc_bus);
//         }
//         if (vdc_bus >= vdc_bus_1)
//         {
//             FPS_ERROR_FMT("{} >> Precharge done time {:2.3f}\n"
//                 , __func__ 
//                 , tNow
//                 );

//             vdc_contact = true;
//             amap["vdc_contact"]->setVal(vdc_contact);
//             current_status = 4;
//             amap["current_status"]->setVal(current_status);
            
//         }
//     }

//     if(current_status == 8)
//     {
//         if(vdc_contact == true)
//         {
//             vdc_contact =  false;
//             FPS_ERROR_FMT("{} >> open dc_contact at  {:2.3f}\n"
//                 , __func__ 
//                 , tNow
//                 );
//             amap["vdc_contact"]->setVal(vdc_contact);
//         }

//         if (vdc_bus > min_vdc_bus)
//         {
//             double interval = tNow-lastTime;
//             vdc_bus -= interval*vdc_discharge;
//             amap["vdc_bus"]->setVal(vdc_bus);
//         }
//         if (vdc_bus < min_vdc_bus)
//         {
//             FPS_ERROR_FMT("{} >> Discharge done time {:2.3f}\n"
//                 , __func__ 
//                 , tNow
//                 );

//             current_status = 2;
//             amap["current_status"]->setVal(current_status);
//             vdc_bus = min_vdc_bus;
//             amap["vdc_bus"]->setVal(vdc_bus);
            
//         }
//     }

//     //amap["current_status"]->setVal(current_status);
//     return 0;
// }

int SimHandleBmsCmd(varsmap& vmap, varmap& amap, const char* xaname, fims* p_fims, assetVar* aV)
{
    const auto now = flex::get_time_dbl();
    //double tNow = now.count();
    bool bval =  false;
    int ival = 0;
    int BMSStartKeyCmd = amap["BMSStartKeyCmd"]->getiVal();
    int BMSStopKeyCmd  = amap["BMSStopKeyCmd"]->getiVal();
    //int current_cmd  = amap["current_cmd"]->getiVal();
    //int current_status  = amap["current_status"]->getiVal();
    bool start_seen  = amap["current_cmd"]->getbParam("start_seen");
    bool stop_seen  = amap["current_cmd"]->getbParam("stop_seen");
    bool debug = false;
    if(!start_seen && (BMSStartKeyCmd == 1))
    {
        start_seen =  true;
        bval = false;
        amap["current_cmd"]->setParam("start_seen", start_seen);
        ival = 1;
        amap["current_cmd"]->setVal(ival);
        if(debug)FPS_ERROR_FMT("{} >> [{:2.3f}] >> Running StartKeyCmd set\n", __func__, now.count());      
    }

    if(!stop_seen && (BMSStopKeyCmd == 1))
    {
        stop_seen =  true;
        amap["current_cmd"]->setParam("stop_seen", stop_seen);
        ival = 2;
        amap["current_cmd"]->setVal(ival);
        if(debug)FPS_ERROR_FMT("{} >> [{:2.3f}] >> Running StopKeyCmd set\n", __func__, now.count());      
       
    }    
    if(BMSStartKeyCmd == 0)
    {
        bval =  false;
        amap["current_cmd"]->setParam("start_seen", bval);        
    }
    if(BMSStartKeyCmd == 0)
    {
        bval =  false;
        amap["current_cmd"]->setParam("stop_seen", bval);        
    }
    // work on status
    return 0;
}
// current_status = 0 Power UP
    // current_status = 1 Init
    // current_status = 2 Off
    // current_status = 3 Precharge
    // current_status = 4 Ready
    // current_status = 5 Wait
    // current_status = 6 On
    // current_status = 7 Stop
    // current_status = 8 Discharge
    // current_status = 9 Fault
// Handles the PCS status change response
int SimHandleBmsStatus(varsmap& vmap, varmap& amap, const char* xaname, fims* p_fims, assetVar* aV)
{
    const auto now = flex::get_time_dbl();
    //double tNow = now.count();

    // work on status
    int current_status = amap["current_status"]->getiVal();
    double lastTime = amap["lastTime"]->getdVal();
    if(current_status == 0)
    {
        current_status = 1;// INIT
        lastTime = now.count();
        amap["lastTime"]->setVal(lastTime);

    }
    if(current_status == 1)
    {
        double start_time = amap["start_time"]->getdVal();
        if(now.count() - lastTime > start_time)
        {
            current_status = 2;// OFF    // we are now waiting for a start
        }
    }
    if(current_status == 2)
    {
        int current_cmd = amap["current_cmd"]->getiVal();
        // We can now accept on command this is latched
        if(current_cmd == 1)
        {
            current_status = 3;// precharge    // we are now waiting for a precharge
            lastTime = now.count();
            amap["lastTime"]->setVal(lastTime);
        }
    }
    // if(current_status == 3)
    // {
    //     double charge_time = amap["charge_time"]->getdVal();
    //     if(now.count() - lastTime > charge_time)
    //     {
    //         FPS_ERROR_FMT(" {} >> Precharge timeout Error at {:2.3f}\n"
    //              ,__func__, tNow);
    //         //current_status = 4;// Ready    // we are now ready  we can accept charge / discharge
    //     }
    // }
    // else if(current_status == 4)
    // {
    //     FPS_ERROR_FMT(" {} >> Precharge  Complete at {:2.3f}\n"
    //              ,__func__, tNow);

    //     current_status = 6;// On   // we are now ready  we can accept charge / discharge
    // }
    // if(current_status == 6)
    // {
    //     int current_cmd = amap["current_cmd"]->getiVal();
    //     if (current_cmd == 2)  // stop  
    //     {
    //         lastTime = now.count();
    //         amap["lastTime"]->setVal(lastTime);
    //         current_status = 8;// Discharge   // we are now ready  we can accept charge / discharge
    //     }

    // }
    // if(current_status == 8)
    // {
    //     double discharge_time = amap["discharge_time"]->getdVal();
    //     if(now.count() - lastTime > discharge_time)
    //     {
    //          FPS_ERROR_FMT(" {} >> Discharge timeout Error at {:2.3f}\n"
    //              ,__func__, tNow);
    //         //current_status = 2;// Stop but we need to charge     // we are now ready  we can accept charge / discharge
    //     }
    // }
    amap["current_status"]->setVal(current_status);
    return 0;
}

// Move away from the volts...
// we demand power but not yet 
// we are given volts so we pull amps
// We have losses and we heat up.
//  go through init for 15 seconds after power up

// First handle start stop

// we have ivolts and they start at 0 
// when we are told told get ready we precharge
// input to 
int SimHandleBms(varsmap& vmap, varmap& amap, const char* xaname, fims* p_fims, assetVar* aV)
{
    const auto now = flex::get_time_dbl();
    double tNow = now.count();
    int reload = 0;
    double dval = 0.0;
    bool bval = false;
    bool debug =  true;

    int ival = 0;
    if(debug)FPS_ERROR_FMT(" {} >> running at time {:2.3f} \n", __func__, tNow);
    asset_manager* am = aV->am;
    VarMapUtils* vm = am->vm;

    const char *aname = "bms";
    am = vm->getaM(vmap, aname);

    if(!am)
    {
        am = new asset_manager("bms");
        if(debug)FPS_ERROR_FMT(" {} >> adding bms asset manager \n", __func__);
        vm->setaM(vmap, aname, am);
        aV->am = am;
        am->vm = vm;
        am->p_fims = p_fims;
        //return 0;
    }
    if(!am->vm)
    {
        FPS_ERROR_FMT(" {} >> unable to find pcs Varmap(am)  handler vm {} aV->am->vm {}\n"
                , __func__, static_cast<void*>(vm), static_cast<void*>(aV->am->vm));
                // we can repair all this if we now have am and Av->am->vm;
        //return 0;
        am->vm = aV->am->vm;
    }
    const char* cfgbase = "/configsim";

    int heartbeat = 0;
    int heartbeatMax=255;
    double heartbeatInterval=0.9;
    double heartbeatTime=tNow;


    //varmap amap = *am->vmap;
    
    FPS_ERROR_FMT(" {} >> [{:2.3f}] running, aname [{}] am  [{}] var [{}] p_fims {}\n"
            , __func__
            , now.count()
            , aname
            , am->name
            , aV->getfName()
            , static_cast<void*>(p_fims)
            );
    auto tval = fmt::format("{} time [{:2.3f}]", __func__, now.count());
    char* tVal = (char*)tval.c_str();//(char*)"Test TimeStamp";
    // need fname to include sbmu_x  so add aname
    const char *fname = __func__;  //__func__ plus aname

    assetVar* SimHandleBms = amap[fname];
    if (!SimHandleBms || (reload = SimHandleBms->getiVal()) == 0)
    {
        reload = 0;  // complete reset  reload = 1 for remap ( links may have changed)
    }

    if (reload < 2)
    {
    

// {"fims": "set", "uri": "/components/pcsm_gradients:rise_grad_p"},
//                         {"fims": "set", "uri": "/components/pcsm_gradients:drop_grad_p"},
//                         {"fims": "set", "uri": "/components/pcsm_gradients:start_grad_p"},
//                         {"fims": "set", "uri": "/components/pcsm_gradients:stop_grad_p"}
// {"fims": "set", "uri": "/components/pcsm_gradients:rise_grad_q"},
//                         {"fims": "set", "uri": "/components/pcsm_gradients:drop_grad_q"},
//                         {"fims": "set", "uri": "/components/pcsm_gradients:start_grad_q"},
//                         {"fims": "set", "uri": "/components/pcsm_gradients:stop_grad_q"}
// {"value": "/assets/pcs/summary:status",                               "vlink":"/status/pcs:PCSStatusResp"},
//     //"id": "pcsm_gradients",
// "/links/pcs":{
//         "seconds": {             "value": "/components/pcsm_general:seconds"},

    //"ctrlword2": {             "value": "/components/pcsm_control:ctrlword2"         }

    //     "/schedule/wake_monitor/pcs":{
    //         "/components/pcs_registers_fast:vdc_bus_1": { "enable": true, "rate":1, "amap": "pcs", "func":"CheckMonitorVar"},
    //         "/components/pcsm_general:seconds": { "enable": true, "rate":1, "func":"CheckMonitorVar"},
    //         "/components/pcs_registers_slow:num_running_modules": { "enable": true, "rate":1, "func":"CheckMonitorVar"},
    //     },
        amap[fname]             = vm->setLinkVal(vmap, aname,         "/reload",       fname, reload);

    //        seconds": {          "value": "/components/pcsm_general:seconds"},
    //        "Min": {             "value": "/components/pcsm_general:minutes"},
    //        "Hour": {            "value": "/components/pcsm_general:hours"},
    //        "Day": {             "value": "/components/pcsm_general:day"         },
    //        "Month": {            "value": "/components/pcsm_general:month"         },
    //        "Year": {             "value": "/components/pcsm_general:year"         },
    //        "Utc": {              "value": "/components/pcsm_general:utc"         },
    //        "Timestamp": {        "value": "/components/pcsm_general:Timestamp"         },
    // these will all be linked to Modbus Data
        amap["seconds"]     = vm->setLinkVal(vmap, aname,     "/configsim",       "seconds", dval);
        amap["Min"]         = vm->setLinkVal(vmap, aname,     "/configsim",       "Min", dval);
        amap["Hour"]        = vm->setLinkVal(vmap, aname,     "/configsim",       "Hour", dval);
        amap["Day"]         = vm->setLinkVal(vmap, aname,     "/configsim",       "Day", dval);
        amap["Month"]       = vm->setLinkVal(vmap, aname,     "/configsim",       "Month", dval);
        amap["Year"]        = vm->setLinkVal(vmap, aname,     "/configsim",       "Year", dval);
        amap["Utc"]         = vm->setLinkVal(vmap, aname,     "/configsim",       "utc", dval);
        amap["Timestamp"]   = vm->setLinkVal(vmap, aname,     "/configsim",       "Timestamp", tVal);
  
//         "Heartbeat": {             "value": "/components/pcsm_communications:ppc_watchdog_ref"         },
        amap["Heartbeat"]   = vm->setLinkVal(vmap, aname,     "/configsim",       "Heartbeat", dval);
  

        amap["heartbeat"]          = vm->setLinkVal(vmap, aname, cfgbase,  "heartbeat", heartbeat);
        amap["heartbeatMax"]       = vm->setLinkVal(vmap, aname, cfgbase,  "heartbeatMax", heartbeatMax);
        amap["heartbeatInterval"]  = vm->setLinkVal(vmap, aname, cfgbase,  "heartbeatInterval", heartbeatInterval);
        amap["heartbeatTime"]      = vm->setLinkVal(vmap, aname, cfgbase,  "heartbeatTime", heartbeatTime);


            // /components/pcsm_remote_control:start"
            // /components/pcsm_remote_control:stop"
            // /components/pcsm_remote_control:e_stop"
            // /components/pcsm_remote_control:reset"

        // amap["PCSStartKeyCmd"] = vm->setLinkVal(vmap, aname,     "/configsim", "PCSStartKeyCmd", dval); 
        // amap["PCSStopKeyCmd"]  = vm->setLinkVal(vmap, aname,     "/configsim", "PCSStopKeyCmd", dval); 
        // amap["PCSEStopKeyCmd"] = vm->setLinkVal(vmap, aname,     "/configsim", "PCSEStopKeyCmd", dval); 
        // amap["ClearFaultCmd"]  = vm->setLinkVal(vmap, aname,     "/configsim", "ClearFaultCmd", tVal); 
    
        //         "vdc_bus_1": {             "value": "/components/pcsm_dc_inputs:vdc_bus_1"         },
        amap["vdc_bus_1"]  = vm->setLinkVal(vmap, aname,     "/configsim", "vdc_bus_1", dval); 
        ival = 0;
        //amap["PCSStatusResp"]   = vm->setLinkVal(vmap, aname,    "/configsim", "PCSStatusResp", ival); 
        amap["current_cmd"]     = vm->setLinkVal(vmap, aname,    "/configsim", "current_cmd", ival); 
        amap["cmd_time"]        = vm->setLinkVal(vmap, aname,    "/configsim", "cmd_time", dval); 
        amap["current_status"]  = vm->setLinkVal(vmap, aname,    "/configsim", "current_status", ival); 
        amap["current_fault"]   = vm->setLinkVal(vmap, aname,    "/configsim", "current_fault", ival); 
        amap["current_warning"] = vm->setLinkVal(vmap, aname,    "/configsim", "current_warning", ival); 
        amap["bms_power"]       = vm->setLinkVal(vmap, aname,    "/configsim", "bms_power", dval); 
        amap["bms_power_cmd"]   = vm->setLinkVal(vmap, aname,    "/configsim", "bms_power_cmd", dval); 
        amap["module_power"]    = vm->setLinkVal(vmap, aname,    "/configsim", "module_power", dval); 
        bval = true;
        amap["bms_small_pub"]   = vm->setLinkVal(vmap, aname,    "/configsim", "bms_small_pub", bval); 
        amap["bms_poweron"]     = vm->setLinkVal(vmap, aname,    "/configsim", "bms_poweron", ival); 
 

        amap["vdc_bus"]        =  vm->setLinkVal(vmap, aname,    "/configsim", "vdc_bus", dval); 
        amap["min_vdc_bus"]    =  vm->setLinkVal(vmap, aname,    "/configsim", "min_vdc_bus", dval); 
        amap["vdc_charge"]     =  vm->setLinkVal(vmap, aname,    "/configsim", "vdc_charge", dval); 
        amap["vdc_discharge"]  =  vm->setLinkVal(vmap, aname,    "/configsim", "vdc_discharge", dval); 
        amap["vdc_contact"]    =  vm->setLinkVal(vmap, aname,    "/configsim", "vdc_contact", bval); 
        amap["bms_rand"]       =  vm->setLinkVal(vmap, aname,    "/configsim", "bms_rand", dval); 
        amap["bms_volts"]      =  vm->setLinkVal(vmap, aname,    "/configsim", "bms_volts", dval); 
        amap["bms_loss"]       =  vm->setLinkVal(vmap, aname,    "/configsim", "bms_loss", dval); 

            //"MaxPCSActivePower":       "value": "/components/pcsm_limits:p_limit"         },
            //"MaxPCSReactivePower":     "value": "/components/pcsm_limits:q_limit"         },
            //"MaxPCSApparentPower":     "value": "/components/pcsm_limits:s_limit"         },
        // amap["MaxPCSActivePower"]   = vm->setLinkVal(vmap, aname,     "/configsim", "MaxPCSActivePower", dval); 
        // amap["MaxPCSReactivePower"] = vm->setLinkVal(vmap, aname,     "/configsim", "MaxPCSReactivePower", dval); 
        // amap["MaxPCSApparentPower"] = vm->setLinkVal(vmap, aname,     "/configsim", "MaxPCSApparentPower", dval); 

            //"ctrlword_pmode": {             "value": "/components/pcsm_control:p_control_mode"         },
            //"ctrlword_qmode": {             "value": "/components/pcsm_control:q_control_mode"         },
            //"ActivePowerCmd": {             "value": "/components/pcsm_control:p_p_reference"         },
            //"ActiveCurrentCmd": {             "value": "/components/pcsm_control:p_id_reference"         },
            //"CosPhiCmd": {             "value": "/components/pcsm_control:q_cosphi_reference"         },
            //"ReactiveCurrentCmd": {             "value": "/components/pcsm_control:q_iq_reference"         },
            //"ReactivePowerCmd": {             "value": "/components/pcsm_control:q_q_reference"         },
            //"ReactiveVoltageCmd": {             "value": "/components/pcsm_control:q_v_reference"         },
        // leave these in for now
        amap["ctrlword_pmode"]   = vm->setLinkVal(vmap, aname,     "/configsim", "ctrlword_pmode", dval); 
        amap["ctrlword_qmode"]   = vm->setLinkVal(vmap, aname,     "/configsim", "ctrlword_qmode", dval); 
        amap["ActivePowerCmd"]   = vm->setLinkVal(vmap, aname,     "/configsim", "ActivePowerCmd", dval); 
        amap["ActiveCurrentCmd"] = vm->setLinkVal(vmap, aname,     "/configsim", "ActiveCurrentCmd", dval); 
        amap["ActiveVoltageCmd"] = vm->setLinkVal(vmap, aname,     "/configsim", "ActiveVoltageCmd", dval); 
        amap["CosPhiCmd"]        = vm->setLinkVal(vmap, aname,     "/configsim", "CosPhiCmd", dval); 
        amap["ReactivePowerCmd"] = vm->setLinkVal(vmap, aname,     "/configsim", "ReactivePowerCmd", dval); 
        amap["ReactiveCurrentCmd"] = vm->setLinkVal(vmap, aname,   "/configsim", "ReactiveCurrentCmd", dval); 
        amap["ReactiveVoltageCmd"] = vm->setLinkVal(vmap, aname,   "/configsim", "ReactiveVoltageCmd", dval); 

        //"PStartGradient": {             "value": "/components/pcsm_gradients:start_grad_p"         },
        //"EnPStartGradient": {             "value": "/components/pcsm_gradients:start_grad_p_enable"         },
        //"PStopGradient": {             "value": "/components/pcsm_gradients:stop_grad_p"         },
        //"EnPStopGradient": {             "value": "/components/pcsm_gradients:stop_grad_p_enable"         },
        //"EnPRiseGradient": {             "value": "/components/pcsm_gradients:rise_grad_p_enable"         },
        //"PDropGradient": {             "value": "/components/pcsm_gradients:drop_grad_p"         },
        //"EnPDropGradient": {             "value": "/components/pcsm_gradients:drop_grad_p_enable"         },
        //"QStartGradient": {             "value": "/components/pcsm_gradients:start_grad_q"         },
        //"EnQStartGradient": {             "value": "/components/pcsm_gradients:start_grad_q_enable"         },
        //"QStopGradient": {             "value": "/components/pcsm_gradients:stop_grad_q"         },
        //"EnQStopGradient": {             "value": "/components/pcsm_gradients:stop_grad_q_enable"         },
        //"EnQRiseGradient": {             "value": "/components/pcsm_gradients:rise_grad_q_enable"         },
        //"QDropGradient": {             "value": "/components/pcsm_gradients:drop_grad_q"         },
        //"EnQDropGradient": {             "value": "/components/pcsm_gradients:drop_grad_q_enable"         },

        amap["PStartGradient"]   = vm->setLinkVal(vmap, aname, "/configsim", "PStartGradient", dval); 
        amap["EnPStartGradient"] = vm->setLinkVal(vmap, aname, "/configsim", "EnPStartGradient", bval); 
        amap["PStopGradient"]    = vm->setLinkVal(vmap, aname, "/configsim", "PStopGradient", dval); 
        amap["EnPStopGradient"]  = vm->setLinkVal(vmap, aname, "/configsim", "EnPStopGradient", bval); 

        amap["PDropGradient"]    = vm->setLinkVal(vmap, aname, "/configsim", "PDropGradient", dval); 
        amap["EnPDropGradient"]  = vm->setLinkVal(vmap, aname, "/configsim", "EnPDropGradient", bval); 

        amap["QStartGradient"] = vm->setLinkVal(vmap, aname,   "/configsim", "QStartGradient", dval); 
        amap["EnQStartGradient"] = vm->setLinkVal(vmap, aname, "/configsim", "EnQStartGradient", bval); 

        amap["QStopGradient"]        = vm->setLinkVal(vmap,    aname, "/configsim", "QStopGradient", dval); 
        amap["EnQStopGradient"]      = vm->setLinkVal(vmap,    aname, "/configsim", "EnQStopGradient", bval); 

        amap["QDropGradient"]        = vm->setLinkVal(vmap,    aname, "/configsim", "QDropGradient", dval); 
        amap["EnQDropGradient"]      = vm->setLinkVal(vmap,    aname, "/configsim", "EnQDropGradient", bval); 

        amap["num_running_racks"]   = vm->setLinkVal(vmap,    aname,  "/configsim", "num_running_racks", ival);
        amap["num_racks"]           = vm->setLinkVal(vmap,    aname,  "/configsim", "num_racks", ival);
        amap["lastTime"]              = vm->setLinkVal(vmap,    aname,  "/configsim", "lastTime", dval);
        amap["start_time"]            = vm->setLinkVal(vmap,    aname,  "/configsim", "start_time", dval);
        amap["charge_time"]           = vm->setLinkVal(vmap,    aname,  "/configsim", "charge_time", dval);
        amap["discharge_time"]        = vm->setLinkVal(vmap,    aname,  "/configsim", "discharge_time", dval);

        amap["BMSStartKeyCmd"]        = vm->setLinkVal(vmap,    aname,  "/configsim", "BMSStartKeyCmd", ival);
        amap["BMSStopKeyCmd"]         = vm->setLinkVal(vmap,    aname,  "/configsim", "BMSStopKeyCmd", ival);
        amap["BMSActivePower"]        = vm->setLinkVal(vmap,    aname,  "/configsim", "BMSActivePower", dval);
        amap["BMSActivePowerCmd"]     = vm->setLinkVal(vmap,    aname,  "/configsim", "BMSActivePowerCmd", dval);
        amap["BMSReactivePower"]      = vm->setLinkVal(vmap,    aname,  "/configsim", "BMSReactivePower", dval);
        amap["BMSReactivePowerCmd"]   = vm->setLinkVal(vmap,    aname,  "/configsim", "BMSReactivePowerCmd", dval);
        amap["MaxBMSActivePower"]     = vm->setLinkVal(vmap,    aname,  "/configsim", "MaxBMSActivePower", dval);
        amap["MaxBMSReactivePower"]   = vm->setLinkVal(vmap,    aname,  "/configsim", "MaxBMSReactivePower", dval);
        amap["MaxBMSApparentPower"]   = vm->setLinkVal(vmap,    aname,  "/configsim", "MaxBMSApparentPower", dval);
        amap["bms_ohms"]              = vm->setLinkVal(vmap,    aname,  "/configsim", "bms_ohms", dval);
        amap["bms_output_power"]      = vm->setLinkVal(vmap,    aname,  "/configsim", "bms_output_power", dval);
        //amap["pcs_power_increase_slope"] = vm->setLinkVal(vmap, aname,  "/configsim", "pcs_power_increase_slope", dval);
        //amap["pcs_power_decrease_slope"] = vm->setLinkVal(vmap, aname,  "/configsim", "pcs_power_decrease_slope", dval);
        amap["bms_current"]           = vm->setLinkVal(vmap,    aname,  "/configsim", "bms_current", dval);
        amap["bms_internal_current"]  = vm->setLinkVal(vmap,    aname,  "/configsim", "bms_internal_current", dval);
        amap["bms_capacity"]          = vm->setLinkVal(vmap,    aname,  "/configsim", "bms_capacity", dval);
        amap["bms_max_capacity"]      = vm->setLinkVal(vmap,    aname,  "/configsim", "bms_max_capacity", dval);
        amap["bms_min_capacity"]      = vm->setLinkVal(vmap,    aname,  "/configsim", "bms_min_capacity", dval);
        amap["bms_soc"]               = vm->setLinkVal(vmap,    aname,  "/configsim", "bms_soc", dval);
        amap["bms_loss"]             = vm->setLinkVal(vmap,     aname,     "/configsim",    "bms_loss",         dval);
        amap["bms_total_current"]     = vm->setLinkVal(vmap,    aname,  "/configsim", "bms_total_current", dval);
        amap["bms_total_power"]       = vm->setLinkVal(vmap,    aname,  "/configsim", "bms_total_power", dval);
        amap["bms_max_power"]         = vm->setLinkVal(vmap,    aname,  "/configsim", "bms_max_power", dval);
        dval =  280.0;
        amap["bms_base_current"]     = vm->setLinkVal(vmap, "bms",  "/configsim", "bms_base_current", dval);
        ival = 8 * 52;
        amap["bms_num_cells"]        = vm->setLinkVal(vmap, "bms",  "/configsim", "bms_num_cells", ival);
        dval =  15.0;
        amap["bms_soc_min"]         = vm->setLinkVal(vmap, "bms",  "/configsim", "bms_soc_min", dval);
        dval =  80.0;
        amap["bms_soc_max"]         = vm->setLinkVal(vmap, "bms",  "/configsim", "bms_soc_max", dval);
        dval =  3.1;
        amap["bms_cell_volt_min"]   = vm->setLinkVal(vmap, "bms",  "/configsim", "bms_cell_volt_min", dval);
        dval =  3.4;
        amap["bms_cell_volt_nom"]   = vm->setLinkVal(vmap, "bms",  "/configsim", "bms_cell_volt_nom", dval);
        dval =  3.6;
        amap["bms_cell_volt_max"]   = vm->setLinkVal(vmap, "bms",  "/configsim", "bms_cell_volt_max", dval);

        amap["bms_charge_current"]      = vm->setLinkVal(vmap, aname,  "/configsim",  "bms_charge_current", dval);
        amap["total_charge_current"]    = vm->setLinkVal(vmap, "bms",  "/configsim",  "total_charge_current", dval);
        amap["bms_discharge_current"]   = vm->setLinkVal(vmap, aname,  "/configsim",  "bms_discharge_current", dval);
        amap["total_discharge_current"] = vm->setLinkVal(vmap, aname,  "/configsim",  "total_discharge_current", dval);
        amap["bms_discharge_power"]     = vm->setLinkVal(vmap, aname,  "/configsim",  "bms_discharge_power", dval);
        amap["total_discharge_power"]   = vm->setLinkVal(vmap, "bms",  "/configsim",  "total_discharge_power", dval);
        amap["bms_charge_power"]        = vm->setLinkVal(vmap, aname,  "/configsim",  "bms_charge_power", dval);
        amap["total_charge_power"]      = vm->setLinkVal(vmap, "bms",  "/configsim",  "total_charge_power", dval);
        amap["discharge_test"]          = vm->setLinkVal(vmap, "bms",  "/configsim",  "discharge_test", bval);

        amap["bms_discharge_power_cmd"] = vm->setLinkVal(vmap, "bms",  "/configsim",  "bms_discharge_power_cmd", dval);

        if (reload == 0) // complete restart 
        {

            printf(" reload running on [%s] [%s] \n", __func__, aname);
            // amap["PCSStartKeyCmd"]->setVal(dval);
            // amap["PCSStopKeyCmd"]->setVal(dval);
            // amap["PCSEStopKeyCmd"]->setVal(dval);
            // amap["ClearFaultCmd"]->setVal(dval);
            
            //"vdc_bus_1": {             "value": "/components/pcsm_dc_inputs:vdc_bus_1"         },
            dval = 1300;
            amap["vdc_bus_1"]->setVal(dval);

            dval = 0.0;   // time to go from state 0 to state 1
            amap["lastTime"]->setVal(dval);
            dval = 6.0;   // time to go from state 0 to state 1
            amap["start_time"]->setVal(dval);
            dval = 10.0;   // precharge time
            amap["charge_time"]->setVal(dval);
            dval = 5.0;
            amap["discharge_time"]->setVal(dval);

            ival = 0;
            amap["current_status"]->setVal(ival);
            amap["current_cmd"]->setVal(ival);
            amap["current_fault"]->setVal(ival);
            amap["current_warning"]->setVal(ival);
            dval = 1350.0;
            amap["bms_volts"]->setVal(dval);
            dval = 5.0;
            amap["bms_loss"]->setVal(dval);
            ival = 9;
            amap["num_racks"]->setVal(ival);
            ival = 0;
            amap["num_running_racks"]->setVal(ival);
            amap["lastTime"]->setVal(dval);
            bval = true;
            amap["bms_small_pub"]->setVal(bval);
            dval = 2.0;
            amap["vdc_bus"]->setVal(dval);
            dval = 26.0;
            amap["min_vdc_bus"]->setVal(dval);
            dval = 12.0;
            amap["vdc_charge"]->setVal(dval);
            dval = 15.0;
            amap["vdc_discharge"]->setVal(dval);
            bval =  false;
            amap["vdc_contact"]->setVal(bval);

            dval = RNG::get().randReal(-5.0, 5.0);
            amap["bms_rand"]->setVal(dval);
            dval = 1.0; //1350.0/2592.59; // 1350 volts = 3500000 wats = 2592.59 amps
            amap["bms_ohms"]->setVal(dval);

            amap["heartbeat"]->setVal(heartbeat);
            amap["heartbeatMax"]->setVal(heartbeatMax);
            amap["heartbeatInterval"]->setVal(heartbeatInterval);   // TODO adjust for number of modules 
            amap["heartbeatTime"]->setVal(heartbeatTime);

            // dval = 35000.0;
            // amap["pcs_power_increase_slope"]->setVal(dval);
            // dval = -35000.0;
            // amap["pcs_power_decrease_slope"]->setVal(dval);
        
            //"MaxPCSActivePower":       "value": "/components/pcsm_limits:p_limit"         },
            //"MaxPCSReactivePower":     "value": "/components/pcsm_limits:q_limit"         },
            //"MaxPCSApparentPower":     "value": "/components/pcsm_limits:s_limit"         },
            dval = 0;
            amap["BMSActivePower"]->setVal(dval);
            amap["BMSActivePowerCmd"]->setVal(dval);
            amap["BMSReactivePower"]->setVal(dval);
            amap["BMSReactivePowerCmd"]->setVal(dval);
            amap["bms_current"]->setVal(dval);
            amap["bms_total_current"]->setVal(dval);
            amap["bms_total_power"]->setVal(dval);
            dval = 3500000.0;
            amap["MaxBMSActivePower"]->setVal(dval);
            amap["MaxBMSReactivePower"]->setVal(dval);
            amap["MaxBMSApparentPower"]->setVal(dval);
            amap["bms_max_power"]->setVal(dval);
            dval = 0;
            amap["bms_power_cmd"]->setVal(dval);
            ival = 0;
            amap["bms_poweron"]->setVal(ival);
            dval =  280.0;
            amap["bms_base_current"]->setVal(dval);
            ival = 8 * 52;
            amap["bms_num_cells"]->setVal(ival);
            dval =  15.0;
            amap["bms_soc_min"]->setVal(dval);
            dval =  80.0;
            amap["bms_soc_max"]->setVal(dval);
            dval =  3.1;
            amap["bms_cell_volt_min"]->setVal(dval);
            dval =  3.4;
            amap["bms_cell_volt_nom"]->setVal(dval);
            dval =  3.6;
            amap["bms_cell_volt_max"]->setVal(dval);


            SimSetupBmsRacks(vmap, amap, aname, p_fims, aV, "rack_{:02d}");
            printf(" after modules  reload running on [%s] [%s] \n", __func__, aname);

        }

        // this is how we set up the personality for , in this case pcs
        // this is the name on the target system
        // char * tval = (char *)"/components/pcs_registers_fast:renamed_current_fault";
        // this is the local name
        // vm->setVal(vmap, "/links/pcs", "current_fault", tval);

        // char * tval = (char *)"/components/pcs_registers_fast:current_fault";
        // vm->setVal(vmap, "/links/pcs", "current_fault", tval);
        // tval = (char *)"/components/pcs_registers_fast:current_status";
        // vm->setVal(vmap, "/links/pcs", "current_status", tval);
        // tval = (char *)"/components/pcs_registers_fast:current_warning";
        // vm->setVal(vmap, "/links/pcs", "current_warning", tval);     
        //causes crashes 
        // vm->setLinks(vmap, "pcs");
        reload = 2;    amap[fname]->setVal(reload);
    }

    // now the code starts
    tNow = now.count(); ; //vm->get_time_dbl();
    tval = fmt::format("the new time is {}", tNow);
    tVal = (char*)tval.c_str();
    amap["Timestamp"]->setVal(tVal);
    
    // current_fault = 0 no fault
    // current_fault = 56 REMRSTP
    //                 91,  (GUNBDC) Mod. Great unbal DC - DC bus unbalanced more than 500V"},
    //                 92, (DISFLT) Mod. Discharge fault - Module FPGA attempts to discharge the module bus and voltage 
    //                           did not drop below 50V in 40s"},
    //                 2,  (HWVBS) HW Vbus - DC overvoltage detected. Threshold: 1520 Vdc"},
    //                 3, (SFTCHR) Softcharge - Wrong configuration (central AC without DU) OR Timeout, one of soft charge processes did not end successfully"
    //                 4, (DISCHR) Discharge - Timeout, one of the modules has not 
    //                                                   discharged correctly (< 50 Vdc)"
    //                 5, (HGHVAC) High Vac - High AC voltage fault"
    //                 6, (LWVAC) Low Vac - Low AC voltage fault"
    // current_warning

    if (1)// (amap["Heartbeat"]->getLastSetDiff(dval) > 1.0)
    {

        SimHandleBmsHeartbeat(vmap, amap, aname, p_fims, aV);
        //SimHandlePcsFault(vmap, amap, aname, p_fims, aV);
        //SimHandlePcsStartup(vmap, amap, aname, p_fims, aV);
        //SimHandlePcsShutdown(vmap, amap, aname, p_fims, aV);
        SimHandleBmsCmd(vmap, amap, aname, p_fims, aV);
        //SimHandlePcsPrecharge(vmap, amap, aname, p_fims, aV);
        //SimHandleBmsSoc(vmap, amap, aname, p_fims, aV);
        
        SimHandleBmsStatus(vmap, amap, aname, p_fims, aV);
//        SimHandlePcsModules(vmap, amap, aname, p_fims, aV);

        int num_racks = amap["num_racks"]->getiVal(); 
        int num_running_racks = amap["num_running_racks"]->getiVal(); 
        //double pcs_power = amap["pcs_power"]->getdVal(); 
        if(num_running_racks > 0)
        {
            //dval = pcs_power/num_running_modules;
            dval = 0;
            amap["module_power"]->setVal(dval); 

        }

        double BMSActivePowerCmd = amap["BMSActivePowerCmd"]->getdVal();
        double MaxBMSActivePower = amap["MaxBMSActivePower"]->getdVal();
        if(1)FPS_ERROR_FMT("{} >> [{:2.3f}] >> Running manager [{}] aname [{}] num_modules {}\n"
             , __func__, dval, am->name, aname, num_racks);

        int rack_count = amap["num_running_racks"]->getiVal();

        ival = 0;
        dval = 0;
        amap["num_running_racks"]->setVal(ival); 
        amap["bms_output_power"]->setVal(dval); 
        amap["bms_total_power"]->setVal(dval); 
        amap["bms_total_current"]->setVal(dval); 
        amap["total_discharge_power"]->setVal(dval);
        amap["total_discharge_current"]->setVal(dval);
        dval = 1.0;
        amap["bms_ohms"]->setVal(dval); 

        double bms_volts = amap["bms_volts"]->getdVal();
        int idx = 0;
        for (auto& iy : am->assetMap)
        {
            if(idx < num_racks)
            {
                asset* ami = iy.second;
                aV->am = am;
                if (1) FPS_ERROR_FMT("{} >> Pass 1 {} >>"
                    , __func__, ami->name);

                if(rack_count > 0)
                {
                    dval =  BMSActivePowerCmd / rack_count;
                    ami->amap["bms_power_cmd"]->setVal(dval);
                    dval =  MaxBMSActivePower / rack_count;
                    ami->amap["bms_max_power"]->setVal(dval);
                }
                 //SimHandlePcsPower(vmap, amap, aname, p_fims, aV);
                SimHandleBmsRack(vmap, ami->amap, ami->name.c_str(), p_fims, aV);
                SimHandleBmsSoc(vmap, ami->amap, ami->name.c_str(), p_fims, aV);
                SimHandleBmsPower(vmap, ami->amap, ami->name.c_str(), p_fims, aV);
                bval = true;
                amap["discharge_test"]->setVal(bval);

                //amap["bms_discharge_current"]->setVal(discharge_current);
                // amap["bms_discharge_power"]->setVal(discharge_power);
                // amap["bms_discharge_soc"]->setVal(discharge_power);
                // amap["total_discharge_power"]->setVal(total_discharge_power);
                // amap["total_discharge_current"]->setVal(total_discharge_current);
                SimHandleBmsDischarge(vmap, ami->amap, ami->name.c_str(), p_fims, aV);
                //ami->amap["module_volts"]->setVal(pcs_volts);
            }
            idx++;
        }
        

        ival = amap["num_running_racks"]->getiVal(); 
        dval = amap["bms_output_power"]->getdVal(); 
        bms_volts = amap["bms_volts"]->getdVal();
        double bms_current = amap["bms_current"]->getdVal();
        double bms_ohms = amap["bms_ohms"]->getdVal();
        double bms_power = bms_current*bms_volts;
        amap["bms_power"]->setVal(bms_power); 

        double total_discharge_power = amap["total_discharge_power"]->getdVal();
        double bms_discharge_power_cmd = amap["bms_discharge_power_cmd"]->getdVal();
 
        if(bms_discharge_power_cmd > 0.0)
        {
            bms_ohms = total_discharge_power /bms_discharge_power_cmd;

            if(1)amap["bms_ohms"]->setVal(bms_ohms); 
            bval = false;
            amap["discharge_test"]->setVal(bval);
            // once more for real
            for (auto& iy : am->assetMap)
            {
                asset* ami = iy.second;
                if (1) FPS_ERROR_FMT("{} >> Pass 2 {} test {} >>"
                    , __func__, ami->name, bval);
                double bms_discharge_power = bms_discharge_power_cmd/ival;
                // split the load evenly here
                ami->amap["bms_discharge_power"]->setVal(bms_discharge_power);    
                SimHandleBmsDischarge(vmap, ami->amap, ami->name.c_str(), p_fims, aV);
            }
        }
        dval = amap["total_discharge_power"]->getdVal();

        if (1) FPS_ERROR_FMT("{} >> After loop  [{}] as {} discharge power {} bms_ohms {}\n"
            , __func__, amap["num_running_racks"]->getfName(), ival , dval, bms_ohms);
            // /ess/components/pcs_registers_fast '{"current_status":2}

        // do this until we get the links working
        //vm->sendAssetVar(amap["current_status"], p_fims, "pub", "/components/pcs_registers_fast", "current_status");
        //vm->sendAssetVar(amap["current_fault"], p_fims, "pub", "/components/pcs_registers_fast", "current_fault");
        //vm->sendAssetVar(amap["current_warning"], p_fims, "pub", "/components/pcs_registers_fast", "current_warning");
        // to make this work we need the links setup
        // Do this if we just want local output
        bool bms_small_pub = amap["bms_small_pub"]->getbVal();
        if (bms_small_pub)
        {
            varsmap* vlist = vm->createVlist();
            vm->addVlist(vlist, amap["heartbeat"]);
            vm->addVlist(vlist, amap["Timestamp"]);
            vm->addVlist(vlist, amap["current_cmd"]);
            vm->addVlist(vlist, amap["current_status"]);
            vm->addVlist(vlist, amap["current_fault"]);
            vm->addVlist(vlist, amap["current_warning"]);
            vm->addVlist(vlist, amap["vdc_contact"]);
            vm->addVlist(vlist, amap["vdc_bus"]);
            vm->addVlist(vlist, amap["total_discharge_power"]);
            vm->addVlist(vlist, amap["bms_discharge_power"]);
            vm->addVlist(vlist, amap["BMSReactivePower"]);
            vm->addVlist(vlist, amap["BMSActivePowerCmd"]);
            vm->addVlist(vlist, amap["BMSReactivePowerCmd"]);
            vm->addVlist(vlist, amap["bms_volts"]);
            vm->addVlist(vlist, amap["bms_total_current"]);
            vm->addVlist(vlist, amap["bms_total_power"]);
            vm->addVlist(vlist, amap["bms_poweron"]);
            vm->sendVlist(p_fims, "pub", vlist);
            vm->clearVlist(vlist);
        }
        amap["lastTime"]->setVal(tNow);
//Body:    {"drop_grad_p_enable":{"value":1},"start_grad_p_enable":{"value":1},"start_grad_q_enable":{"value":1},"stop_grad_p_enable":{"value":1},"stop_grad_q_enable":{"value":1}}

        // if(avg_soc > 0)
        // {
        //     vm->setVal(vmap, "/components/catl_mbmu_summary_r", "mbmu_soc", avg_soc);
        // }
        //"/components/catl_bms_ems_r": 

        // get the reference to the variable 
        //     if(SimPub)vm->sendAssetVar(hb, p_fims, "pub", "/components/pcs_sim","sys_heartbeat");
        //     vm->setVal(vmap, "/components/pcs_sim", "pcs_heartbeat", dval);
        // }
        // //"/components/catl_mbmu_stat_r:bms_heartbeat"

        // if (SimBmsComms)  {
        //     if(SimPub)vm->sendAssetVar(cd, p_fims, "pub", "/components/bms_sim", "bms_timestamp");
        //     vm->setVal(vmap, "/components/bms_sim", "bms_timestamp", sp);
        // }
        // if (SimBmsHB)     {
        //     if(SimPub)vm->sendAssetVar(hb, p_fims, "pub", "/components/bms_sim", "bms_heartbeat");
        //     vm->setVal(vmap, "/components/bms_sim", "bms_heartbeat", dval);
        // }
    }
    return 0;
}
#endif
