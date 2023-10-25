/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Reactive_Power_Closed_Loop_Control.h>
#include <Site_Controller_Utils.h>

features::Reactive_Power_Closed_Loop_Control::Reactive_Power_Closed_Loop_Control() {
    feature_vars = {
        &step_size_kW, &default_offset, &min_offset, &max_offset, &total_correction, &zero_bypass_enable, &zero_bypass_deadband_kVAR, &steady_state_deadband_kW, &regulation_deadband_kW, &update_rate_ms, &decrease_timer_ms, &increase_timer_ms,
    };

    variable_ids = {
        { &enable_flag, "reactive_power_closed_loop_enable" },
        { &default_offset, "reactive_power_closed_loop_default_offset" },
        { &min_offset, "reactive_power_closed_loop_min_offset" },
        { &max_offset, "reactive_power_closed_loop_max_offset" },
        { &step_size_kW, "reactive_power_closed_loop_step_size_kW" },
        { &total_correction, "reactive_power_closed_loop_total_correction" },
        { &zero_bypass_enable, "reactive_power_closed_loop_zero_bypass_enable" },
        { &zero_bypass_deadband_kVAR, "reactive_power_closed_loop_zero_bypass_deadband_kVAR" },
        { &steady_state_deadband_kW, "reactive_power_closed_loop_steady_state_deadband_kW" },
        { &regulation_deadband_kW, "reactive_power_closed_loop_regulation_deadband_kW" },
        { &update_rate_ms, "reactive_power_closed_loop_update_rate_ms" },
        { &decrease_timer_ms, "reactive_power_closed_loop_decrease_timer_ms" },
        { &increase_timer_ms, "reactive_power_closed_loop_increase_timer_ms" },
    };

    prev_reactive_power_feature_cmd = 0.0f;
}

void features::Reactive_Power_Closed_Loop_Control::init() {
    // initialize reactive closed loop control Variable_Regulator
    regulator.min_offset = min_offset.value.value_int;
    regulator.max_offset = max_offset.value.value_int;
    regulator.offset = default_offset.value.value_int;
    regulator.default_offset = default_offset.value.value_int;
    // Set up update rate (updates per second) based on the millisecond rate given, with the fastest rate being 10ms
    update_rate_ms.value.set(std::max(update_rate_ms.value.value_int, 10));
    regulator.set_update_rate(1000 / update_rate_ms.value.value_int);
    // Set up steady state deadband condition as false for any value above the deadband
    regulator.set_default_condition(steady_state_deadband_kW.value.value_float, Variable_Regulator::VALUE_ABOVE);
    // Set up regulation deadband condition as false for any value above the accuracy deadband compared to the POI
    regulator.set_control_high_threshold(regulation_deadband_kW.value.value_float);
    regulator.set_decrease_timer_duration_ms(decrease_timer_ms.value.value_int);
    // Set up regulation deadband condition as false for any value below the accuracy deadband compared to the POI
    regulator.set_control_low_threshold(-1.0f * regulation_deadband_kW.value.value_float);
    regulator.set_increase_timer_duration_ms(increase_timer_ms.value.value_int);
}

void features::Reactive_Power_Closed_Loop_Control::execute(Asset_Cmd_Object& asset_cmd, float feeder_actual_kVAR, float total_site_kVAR_rated_charge, float total_site_kVAR_rated_discharge) {
    External_Inputs inputs{
        feeder_actual_kVAR,
        asset_cmd.site_kVAR_demand,
        total_site_kVAR_rated_charge,
        total_site_kVAR_rated_discharge,
    };
    External_Outputs outputs = execute_helper(inputs);
    asset_cmd.site_kVAR_demand = outputs.site_kVAR_demand;
}

features::Reactive_Power_Closed_Loop_Control::External_Outputs features::Reactive_Power_Closed_Loop_Control::execute_helper(const External_Inputs& inputs) {
    // Always invert POI, using the convention that positive power flows into the site and negative power flows out of the site at the POI
    float poi_value = -1.0f * inputs.feeder_actual_kVAR;
    float current_cmd = inputs.site_kVAR_demand;

    // If zero bypass is enabled and command is within deadband, reset regulator offset and do not apply any closed loop control correction
    if (zero_bypass_enable.value.value_bool && fabsf(current_cmd) <= fabsf(zero_bypass_deadband_kVAR.value.value_float)) {
        regulator.offset = 0;
        total_correction.value.set(0.0f);
        float new_site_kVAR_demand = inputs.site_kVAR_demand;
        return External_Outputs{ new_site_kVAR_demand };
    }

    // Make sure a step size is configured to prevent divide by zero -> NaN
    if (step_size_kW.value.value_float == 0.0f)
        step_size_kW.value.value_float = 1.0f;
    // Reset offset limits
    regulator.min_offset = min_offset.value.value_int;
    regulator.max_offset = max_offset.value.value_int;

    // First calculate remaining available power for negative corrections
    float negative_charge_increase = less_than_zero_check(inputs.total_site_kVAR_rated_charge - current_cmd);
    float negative_discharge_decrease = std::min(inputs.total_site_kVAR_rated_discharge, zero_check(current_cmd));
    float negative_power_available = negative_charge_increase - negative_discharge_decrease;
    // Next calculate remaining available power for positive corrections
    float positive_charge_decrease = std::max(inputs.total_site_kVAR_rated_charge, less_than_zero_check(current_cmd));
    float positive_discharge_increase = zero_check(inputs.total_site_kVAR_rated_discharge - current_cmd);
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
    bool command_accepted = regulator.regulate_variable_offset(std::abs(current_cmd - prev_reactive_power_feature_cmd), poi_value - current_cmd);

    // TODO: if the offset is beyond the new min/max, reset it so it doesn't get stuck
    //       in the future we should count up/down until we're within range again, but changing the regulator itself carries more risk
    regulator.offset = range_check(regulator.offset, regulator.max_offset, regulator.min_offset);

    // Take the product of the offset and step size as the total correction applied to the reactive power feature's command
    total_correction.value.set(regulator.offset * step_size_kW.value.value_float);
    float new_site_kVAR_demand = inputs.site_kVAR_demand + total_correction.value.value_float;
    // Preserve the reactive power feature command to be dispatched, but only if closed loop control had a chance to act on the command
    if (command_accepted)
        prev_reactive_power_feature_cmd = current_cmd;

    return External_Outputs{ new_site_kVAR_demand };
}

void features::Reactive_Power_Closed_Loop_Control::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_Number) {
        min_offset.set_fims_int(uri_endpoint.c_str(), -1 * fabs(msg_value.valueint));
        max_offset.set_fims_int(uri_endpoint.c_str(), fabs(msg_value.valueint));
        step_size_kW.set_fims_float(uri_endpoint.c_str(), fabsf(msg_value.valuedouble));
        zero_bypass_deadband_kVAR.set_fims_float(uri_endpoint.c_str(), fabsf(msg_value.valuedouble));
        steady_state_deadband_kW.set_fims_float(uri_endpoint.c_str(), fabsf(msg_value.valuedouble));
        regulation_deadband_kW.set_fims_float(uri_endpoint.c_str(), fabsf(msg_value.valuedouble));
    } else if (msg_value.type == cJSON_True || msg_value.type == cJSON_False) {
        bool value_bool = msg_value.type != cJSON_False;
        if (available) {
            enable_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
            zero_bypass_enable.set_fims_bool(uri_endpoint.c_str(), value_bool);
        }
    }
}
