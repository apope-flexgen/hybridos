/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Active_Power_Closed_Loop_Control.h>
#include <Site_Controller_Utils.h>

features::Active_Power_Closed_Loop_Control::Active_Power_Closed_Loop_Control() {
    feature_vars = {
        &step_size_kW, &default_offset, &min_offset, &max_offset, &total_correction, &steady_state_deadband_kW, &regulation_deadband_kW, &update_rate_ms, &decrease_timer_ms, &increase_timer_ms, &_manual_offset_kW,
    };

    variable_ids = {
        { &enable_flag, "active_power_closed_loop_enable" },
        { &default_offset, "active_power_closed_loop_default_offset" },
        { &min_offset, "active_power_closed_loop_min_offset" },
        { &max_offset, "active_power_closed_loop_max_offset" },
        { &step_size_kW, "active_power_closed_loop_step_size_kW" },
        { &total_correction, "active_power_closed_loop_total_correction" },
        { &steady_state_deadband_kW, "active_power_closed_loop_steady_state_deadband_kW" },
        { &regulation_deadband_kW, "active_power_closed_loop_regulation_deadband_kW" },
        { &update_rate_ms, "active_power_closed_loop_update_rate_ms" },
        { &decrease_timer_ms, "active_power_closed_loop_decrease_timer_ms" },
        { &increase_timer_ms, "active_power_closed_loop_increase_timer_ms" },
        { &_manual_offset_kW, "_active_power_closed_loop_manual_offset_kW" },
    };

    prev_active_power_feature_cmd = 0.0f;
}

void features::Active_Power_Closed_Loop_Control::execute(Asset_Cmd_Object& asset_cmd, float feeder_actual_kW, float total_site_kW_rated_charge, float total_site_kW_rated_discharge) {
    External_Inputs inputs = External_Inputs{
        asset_cmd.site_kW_demand,
        asset_cmd.additional_load_compensation,
        asset_cmd.poi_cmd,
        feeder_actual_kW,
        asset_cmd.ess_data.kW_request,
        asset_cmd.gen_data.kW_request,
        asset_cmd.solar_data.kW_request,
        asset_cmd.load_method,
        asset_cmd.feature_kW_demand,
        total_site_kW_rated_charge,
        total_site_kW_rated_discharge,
        asset_cmd.feeder_data.max_potential_kW,
    };
    External_Outputs outputs = execute_helper(inputs);
    asset_cmd.site_kW_charge_production = outputs.site_kW_charge_production;
    asset_cmd.site_kW_discharge_production = outputs.site_kW_discharge_production;
    total_correction.value.value_float = outputs.total_correction;
    asset_cmd.site_kW_demand = outputs.site_kW_demand;
    asset_cmd.feeder_data.max_potential_kW = outputs.feeder_max_potential_kW;
}

features::Active_Power_Closed_Loop_Control::External_Outputs features::Active_Power_Closed_Loop_Control::execute_helper(const External_Inputs& inputs) {
    // Default to using site demand as the desired reference, but remove load so it closes in on the correct value
    float current_cmd = inputs.site_kW_demand - inputs.additional_load_compensation;
    // Overwrite with the explicit poi request as the desired reference if it's available
    if (inputs.poi_cmd != 0.0f)
        current_cmd = inputs.poi_cmd;

    // Always invert POI, using the convention that positive power flows into the site and negative power flows out of the site at the POI
    float poi_value = -1.0f * inputs.feeder_actual_kW;

    // Make sure a step size is configured to prevent divide by zero -> NaN
    if (step_size_kW.value.value_float == 0.0f)
        step_size_kW.value.value_float = 1.0f;
    // Reset offset limits
    regulator.min_offset = min_offset.value.value_int;
    regulator.max_offset = max_offset.value.value_int;

    // Determine min and max correction limits based on available power and the currently requested production
    asset_cmd_utils::site_kW_production_limits site_kW_prod_limits = asset_cmd_utils::calculate_site_kW_production_limits(inputs.ess_kW_request, inputs.gen_kW_request, inputs.solar_kW_request, inputs.load_method, inputs.additional_load_compensation,
                                                                                                                          inputs.feature_kW_demand, inputs.site_kW_demand);
    float new_site_kW_charge_production = site_kW_prod_limits.site_kW_charge_production;
    float new_site_kW_discharge_production = site_kW_prod_limits.site_kW_discharge_production;
    // First calculate remaining available power for negative corrections
    float negative_charge_increase = less_than_zero_check(inputs.total_site_kW_rated_charge - new_site_kW_charge_production);
    float negative_discharge_decrease = std::min(inputs.total_site_kW_rated_discharge, new_site_kW_discharge_production);
    float negative_power_available = negative_charge_increase - negative_discharge_decrease;
    // Next calculate remaining available power for positive corrections
    float positive_charge_decrease = std::max(inputs.total_site_kW_rated_charge, new_site_kW_charge_production);
    float positive_discharge_increase = zero_check(inputs.total_site_kW_rated_discharge - new_site_kW_discharge_production);
    float positive_power_available = positive_discharge_increase - positive_charge_decrease;

    // Then calculate the min/max offset
    int min_allowable_offset = negative_power_available / step_size_kW.value.value_float;
    int max_allowable_offset = positive_power_available / step_size_kW.value.value_float;
    // Finally update regulator limits
    if (regulator.min_offset < min_allowable_offset)
        regulator.min_offset = min_allowable_offset;
    if (regulator.max_offset > max_allowable_offset)
        regulator.max_offset = max_allowable_offset;

    // Regulate based on configured deadbands compared to difference in dispatched commands (steady state) and difference in command and POI (accuracy)
    bool command_accepted = regulator.regulate_variable_offset(std::abs(current_cmd - prev_active_power_feature_cmd), poi_value - current_cmd);

    // TODO: if the offset is beyond the new min/max, manually bound it so it does get stuck
    //       in the future we should count up/down until we're within range again, but changing the regulator itself carries more risk
    regulator.offset = range_check(regulator.offset, regulator.max_offset, regulator.min_offset);

    // Take the product of the offset and step size as the total correction applied to the active power feature's command
    float new_total_correction = (_manual_offset_kW.value.value_float + regulator.offset * step_size_kW.value.value_float);
    float new_site_kW_demand = inputs.site_kW_demand;
    new_site_kW_demand += new_total_correction;
    // Give enough headroom to accommodate the full correction if needed
    // Because this feature explicitly takes the POI value into account and has determined that it has not been reached yet, the POI limit should not be violated
    float new_feeder_max_potential_kW = std::max(inputs.feeder_max_potential_kW, current_cmd + zero_check(-1.0f * new_total_correction));
    // Preserve the active power feature command to be dispatched, but only if closed loop control had a chance to act on the command
    if (command_accepted)
        prev_active_power_feature_cmd = current_cmd;

    return External_Outputs{
        new_site_kW_charge_production, new_site_kW_discharge_production, new_total_correction, new_site_kW_demand, new_feeder_max_potential_kW,
    };
}

void features::Active_Power_Closed_Loop_Control::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_Number) {
        // Closed loop control testing endpoint
        _manual_offset_kW.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        min_offset.set_fims_int(uri_endpoint.c_str(), -1 * fabs(msg_value.valueint));
        max_offset.set_fims_int(uri_endpoint.c_str(), fabs(msg_value.valueint));
        step_size_kW.set_fims_float(uri_endpoint.c_str(), fabsf(msg_value.valuedouble));
        steady_state_deadband_kW.set_fims_float(uri_endpoint.c_str(), fabsf(msg_value.valuedouble));
        regulation_deadband_kW.set_fims_float(uri_endpoint.c_str(), fabsf(msg_value.valuedouble));
    } else if (msg_value.type == cJSON_True || msg_value.type == cJSON_False) {
        bool value_bool = msg_value.type != cJSON_False;
        if (available)
            enable_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
    }
}
