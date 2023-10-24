/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Solar_Shed.h>
#include <Site_Controller_Utils.h>

features::Solar_Shed::Solar_Shed() {
    feature_vars = {
        &solar_shed_value,        &solar_shed_max_value,         &solar_shed_min_value,         &solar_shed_max_shedding_threshold, &solar_shed_high_threshold,         &solar_shed_low_threshold,
        &solar_shed_spare_ess_kw, &solar_shed_increase_timer_ms, &solar_shed_decrease_timer_ms, &solar_shed_increase_display_timer, &solar_shed_decrease_display_timer,
    };

    variable_ids = {
        { &enable_flag, "solar_shed_enable" },
        { &solar_shed_value, "solar_shed_value" },
        { &solar_shed_max_value, "solar_shed_max_value" },
        { &solar_shed_min_value, "solar_shed_min_value" },
        { &solar_shed_max_shedding_threshold, "solar_shed_max_shedding_threshold" },
        { &solar_shed_high_threshold, "solar_shed_high_threshold" },
        { &solar_shed_decrease_timer_ms, "solar_shed_decrease_timer_ms" },
        { &solar_shed_low_threshold, "solar_shed_low_threshold" },
        { &solar_shed_increase_timer_ms, "solar_shed_increase_timer_ms" },
        { &solar_shed_increase_display_timer, "solar_shed_increase_display_timer" },
        { &solar_shed_decrease_display_timer, "solar_shed_decrease_display_timer" },
        { &solar_shed_spare_ess_kw, "solar_shed_spare_ess_kw" },
    };
}

void features::Solar_Shed::init() {
    solar_shed_calculator.min_offset = solar_shed_min_value.value.value_int;
    solar_shed_calculator.max_offset = solar_shed_max_value.value.value_int;
    solar_shed_calculator.offset = solar_shed_calculator.max_offset;
    solar_shed_calculator.default_offset = solar_shed_calculator.max_offset;
    solar_shed_value.value.set(solar_shed_calculator.offset);
    solar_shed_calculator.set_default_condition(solar_shed_max_shedding_threshold.value.value_float, Variable_Regulator::VALUE_ABOVE);
    solar_shed_calculator.set_control_high_threshold(solar_shed_high_threshold.value.value_float);
    solar_shed_calculator.set_decrease_timer_duration_ms(solar_shed_decrease_timer_ms.value.value_int);
    solar_shed_calculator.set_control_low_threshold(solar_shed_low_threshold.value.value_float);
    solar_shed_calculator.set_increase_timer_duration_ms(solar_shed_increase_timer_ms.value.value_int);
}

void features::Solar_Shed::reset() {
    solar_shed_calculator.offset = solar_shed_calculator.max_offset;
    solar_shed_value.value.set(solar_shed_calculator.offset);
}

void features::Solar_Shed::execute(Asset_Cmd_Object& asset_cmd, float soc_avg_running, float solar_total_rated_active_power, float& max_potential_solar_kW) {
    External_Inputs inputs{
        soc_avg_running,
        solar_total_rated_active_power,
        asset_cmd.solar_data.max_potential_kW,
    };
    External_Outputs outputs = execute_helper(inputs);

    asset_cmd.solar_data.max_potential_kW = outputs.asset_cmd_solar_max_potential_kW;
    max_potential_solar_kW = outputs.site_max_potential_solar_kW;
}

features::Solar_Shed::External_Outputs features::Solar_Shed::execute_helper(const External_Inputs& inputs) {
    // solar shed value depends on soc and (chargeable power - measured charging power)
    solar_shed_calculator.regulate_variable_offset(inputs.soc_avg_running, solar_shed_spare_ess_kw.value.value_float);
    solar_shed_value.value.set(solar_shed_calculator.offset);
    // set solar max potential active power based on shed value
    // linearly map max shed to 0% power and min shed to 100% power
    float percent_shedding = ((float)solar_shed_calculator.max_offset - solar_shed_calculator.offset) / (solar_shed_calculator.max_offset - solar_shed_calculator.min_offset);
    float solar_max = inputs.solar_total_rated_active_power * percent_shedding;
    // max solar power is still limited by existing max limit
    solar_max = std::min(solar_max, inputs.asset_cmd_solar_max_potential_kW);

    float new_asset_cmd_solar_max_potential_kW = solar_max;
    float new_site_max_potential_solar_kW = solar_max;

    return External_Outputs{
        new_asset_cmd_solar_max_potential_kW,
        new_site_max_potential_solar_kW,
    };
}

void features::Solar_Shed::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_True || msg_value.type == cJSON_False) {
        bool value_bool = msg_value.type != cJSON_False;
        if (available) {
            enable_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
        }
    }
}
