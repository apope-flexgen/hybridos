/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/PFR.h>
#include <Site_Controller_Utils.h>

features::PFR::PFR() {
    feature_vars = { &deadband, &droop_percent, &offset_hz, &site_nominal_hz, &status_flag, &nameplate_kW, &limits_max_kW, &limits_min_kW };
    summary_vars = {
        &status_flag,
    };

    variable_ids = {
        { &enable_flag, "pfr_enable_flag" },     { &deadband, "pfr_deadband" },           { &droop_percent, "pfr_droop_percent" },     { &status_flag, "pfr_status_flag" }, { &nameplate_kW, "pfr_nameplate_kW" },
        { &limits_min_kW, "pfr_limits_min_kW" }, { &limits_max_kW, "pfr_limits_max_kW" }, { &site_nominal_hz, "pfr_site_nominal_hz" }, { &offset_hz, "pfr_offset_hz" },
    };
}

void features::PFR::execute(Asset_Cmd_Object& asset_cmd, float site_frequency, float& total_site_kW_charge_limit, float& total_site_kW_discharge_limit) {
    External_Inputs inputs{
        site_frequency,
        asset_cmd.site_kW_demand,
        total_site_kW_charge_limit,
        total_site_kW_discharge_limit,
    };
    External_Outputs outputs = execute_helper(inputs);
    asset_cmd.site_kW_demand = outputs.site_kW_demand;
    total_site_kW_charge_limit = outputs.total_site_kW_charge_limit;
    total_site_kW_discharge_limit = outputs.total_site_kW_discharge_limit;
}

features::PFR::External_Outputs features::PFR::execute_helper(const External_Inputs& inputs) {
    // include offset test input in case it is non-zero
    float freq_input = inputs.site_frequency + offset_hz.value.value_float;

    // get correct reference_hz for overhz or underhz event (nominal hz + or - deadband)
    float reference_hz = (freq_input > site_nominal_hz.value.value_float) ? site_nominal_hz.value.value_float + deadband.value.value_float : site_nominal_hz.value.value_float - deadband.value.value_float;

    // delta frequency deviation calculation
    float delta_hz;

    // delta calculation for overfrequency event
    if (freq_input > site_nominal_hz.value.value_float)
        // if outside pass positive delta. otherwise 0.
        delta_hz = freq_input > reference_hz ? freq_input - reference_hz : 0;
    // delta calculation for underfrequency event
    else
        // if outside the deadband, pass negative delta. otherwise 0.
        delta_hz = freq_input < reference_hz ? freq_input - reference_hz : 0;

    // no PFR event or invalid droop percent
    if (delta_hz == 0 || droop_percent.value.value_float <= 0) {
        status_flag.value.value_bool = false;
        return External_Outputs{
            inputs.site_kW_demand,
            inputs.total_site_kW_charge_limit,
            inputs.total_site_kW_discharge_limit,
        };
    }

    // pfr event detected, set pfr status true
    status_flag.value.value_bool = true;

    // delta frequency deviation scaled by percent slope and nominal frequency (=0 if no PFR event)
    float response_percent = (delta_hz / site_nominal_hz.value.value_float) / (.01 * droop_percent.value.value_float);

    // calculate charge/discharge limits from min/max potential
    float chg_kW_limit = less_than_zero_check(limits_min_kW.value.value_float);
    float dischg_kW_limit = zero_check(limits_max_kW.value.value_float);

    // Calculate the charge magnitude that should be used, original command direction gives us the sign to use
    float delta_chg = (inputs.site_kW_demand < 0) ? fabsf(limits_min_kW.value.value_float) : limits_max_kW.value.value_float;

    // calculate pfr offset request based on overfrequency or underfrequency event
    float response_kW = -1 * response_percent * delta_chg;

    // Get the direction of the original command, or use sign of response if 0
    float base_cmd = (inputs.site_kW_demand != 0) ? inputs.site_kW_demand : response_kW;

    // charge direction power limit (cannot change direction)
    if (base_cmd > 0)
        chg_kW_limit = 0;
    // discharge direction power limit
    else
        dischg_kW_limit = 0;

    // Bound modified site demand by configured limits
    float calculated_response = range_check(inputs.site_kW_demand + response_kW, dischg_kW_limit, chg_kW_limit);

    // Modify request and demand based on the degree of change calculated
    float new_site_kW_demand = calculated_response;
    float new_total_site_kW_charge_limit = (range_check(inputs.total_site_kW_charge_limit + response_kW, 0.0f, limits_min_kW.value.value_float));
    float new_total_site_kW_discharge_limit = (range_check(inputs.total_site_kW_discharge_limit + response_kW, limits_max_kW.value.value_float, 0.0f));

    return External_Outputs{
        new_site_kW_demand,
        new_total_site_kW_charge_limit,
        new_total_site_kW_discharge_limit,
    };
}

void features::PFR::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_Number) {
        offset_hz.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        limits_min_kW.set_fims_float(uri_endpoint.c_str(), -1 * fabsf(msg_value.valuedouble));
        limits_max_kW.set_fims_float(uri_endpoint.c_str(), fabsf(msg_value.valuedouble));
        droop_percent.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        deadband.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
    } else if (msg_value.type == cJSON_True || msg_value.type == cJSON_False) {
        bool value_bool = msg_value.type != cJSON_False;
        if (available) {
            enable_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
        }
    }
}