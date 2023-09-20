/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Load_Shed.h>
#include <Site_Controller_Utils.h>

features::Load_Shed::Load_Shed() {
    feature_vars = { &load_shed_value,        &load_shed_max_value,         &load_shed_min_value,         &load_shed_max_shedding_threshold, &load_shed_high_threshold,        &load_shed_low_threshold,
                     &load_shed_spare_ess_kw, &load_shed_increase_timer_ms, &load_shed_decrease_timer_ms, &load_shed_increase_display_timer, &load_shed_decrease_display_timer };

    variable_ids = {
        { &enable_flag, "load_shed_enable" },
        { &load_shed_value, "load_shed_value" },
        { &load_shed_max_value, "load_shed_max_value" },
        { &load_shed_min_value, "load_shed_min_value" },
        { &load_shed_max_shedding_threshold, "load_shed_max_shedding_threshold" },
        { &load_shed_high_threshold, "load_shed_high_threshold" },
        { &load_shed_decrease_timer_ms, "load_shed_decrease_timer_ms" },
        { &load_shed_low_threshold, "load_shed_low_threshold" },
        { &load_shed_increase_timer_ms, "load_shed_increase_timer_ms" },
        { &load_shed_increase_display_timer, "load_shed_increase_display_timer" },
        { &load_shed_decrease_display_timer, "load_shed_decrease_display_timer" },
        { &load_shed_spare_ess_kw, "load_shed_spare_ess_kw" },
    };
}

void features::Load_Shed::init() {
    load_shed_calculator.min_offset = load_shed_min_value.value.value_int;
    load_shed_calculator.max_offset = load_shed_max_value.value.value_int;
    // the default case is set to the maximum offset configured
    load_shed_calculator.offset = load_shed_calculator.max_offset;
    load_shed_calculator.default_offset = load_shed_calculator.max_offset;
    if (load_shed_value.value.value_int != load_shed_calculator.offset) {
        load_shed_value.value.set(load_shed_calculator.offset);
        load_shed_value.send_to_component(true);
    }
    load_shed_calculator.set_default_condition(load_shed_max_shedding_threshold.value.value_float, Variable_Regulator::VALUE_BELOW);
    load_shed_calculator.set_control_high_threshold(load_shed_high_threshold.value.value_float);
    load_shed_calculator.set_decrease_timer_duration_ms(load_shed_decrease_timer_ms.value.value_int);
    load_shed_calculator.set_control_low_threshold(load_shed_low_threshold.value.value_float);
    load_shed_calculator.set_increase_timer_duration_ms(load_shed_increase_timer_ms.value.value_int);
}

void features::Load_Shed::reset() {
    load_shed_calculator.offset = load_shed_calculator.max_offset;
    if (load_shed_value.value.value_int != load_shed_calculator.offset) {
        load_shed_value.value.set(load_shed_calculator.offset);
        load_shed_value.send_to_component(true);
    }
}

void features::Load_Shed::execute(float soc_avg_running) {
    int prev_load_shed = load_shed_calculator.offset;
    // load shed value depends on soc and (dischargeable power - measured power)
    load_shed_calculator.regulate_variable_offset(soc_avg_running, load_shed_spare_ess_kw.value.value_float);
    int curr_load_shed = load_shed_calculator.offset;
    // Write out the value on a change
    if (curr_load_shed != prev_load_shed) {
        load_shed_value.value.set(curr_load_shed);
        load_shed_value.send_to_component(true);
    }
}

void features::Load_Shed::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_True || msg_value.type == cJSON_False) {
        bool value_bool = msg_value.type != cJSON_False;
        if (available) {
            enable_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
        }
    }
}