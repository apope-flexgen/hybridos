/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Active_Voltage_Regulation.h>

features::Active_Voltage_Regulation::Active_Voltage_Regulation() {
    feature_vars = { &deadband, &droop_percent, &voltage_cmd, &actual_volts, &status_flag, &rated_kVAR };

    variable_ids = {
        { &enable_flag, "active_voltage_mode_enable_flag" }, { &deadband, "active_voltage_deadband" },       { &droop_percent, "active_voltage_droop_percent" }, { &voltage_cmd, "active_voltage_cmd" },
        { &actual_volts, "active_voltage_actual_volts" },    { &status_flag, "active_voltage_status_flag" }, { &rated_kVAR, "active_voltage_rated_kVAR" },
    };
}

void features::Active_Voltage_Regulation::execute(Asset_Cmd_Object& asset_cmd, bool& asset_pf_flag) {
    External_Inputs inputs = External_Inputs{
        asset_cmd.total_potential_kVAR,
    };
    External_Outputs outputs = execute_helper(inputs);
    asset_cmd.site_kVAR_demand = outputs.site_kVAR_demand;
    asset_pf_flag = outputs.asset_pf_flag;
}

features::Active_Voltage_Regulation::External_Outputs features::Active_Voltage_Regulation::execute_helper(const External_Inputs& inputs) {
    // calculate the requested power due to voltage deviation
    // get correct reference_volts for overvolts or undervolts event (nominal volts + or - deadband)
    float reference_volts = (actual_volts.value.value_float > voltage_cmd.value.value_float) ? voltage_cmd.value.value_float + deadband.value.value_float : voltage_cmd.value.value_float - deadband.value.value_float;

    float delta_volts;  // delta voltage deviation

    // delta calculation for overvoltage event (negative delta)
    if (actual_volts.value.value_float > voltage_cmd.value.value_float)
        delta_volts = (actual_volts.value.value_float > reference_volts) ? reference_volts - actual_volts.value.value_float : 0;

    // delta calculation for undervoltage event (positive delta)
    else
        delta_volts = (actual_volts.value.value_float < reference_volts) ? reference_volts - actual_volts.value.value_float : 0;

    // delta voltage deviation scaled by percent slope and nominal frequency
    float kVAR_request = (delta_volts / voltage_cmd.value.value_float) / (.01 * droop_percent.value.value_float) * rated_kVAR.value.value_float;

    float new_site_kVAR_demand = std::min(inputs.total_potential_kVAR, fabsf(kVAR_request));
    new_site_kVAR_demand *= (std::signbit(kVAR_request)) ? -1 : 1;

    // set status flag
    status_flag.value.set(new_site_kVAR_demand != 0);

    // this mode does not use power factor control
    bool new_asset_pf_flag = false;

    return External_Outputs{
        new_site_kVAR_demand,
        new_asset_pf_flag,
    };
}

void features::Active_Voltage_Regulation::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_Number) {
        voltage_cmd.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        deadband.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        droop_percent.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
    }
}