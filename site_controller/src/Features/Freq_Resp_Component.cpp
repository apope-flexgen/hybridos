/*
 * Frequency_Object.cpp
 *
 * Created: Summer 2022
 */

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
#include <algorithm>
/* External Dependencies */
/* System Internal Dependencies */
#include <fims/fps_utils.h>
/* Local Internal Dependencies */
#include <Freq_Resp_Component.h>
#include <Site_Controller_Utils.h>

/**
 * @brief Updates the state of the response component based on time, current frequency, and previous state. Then
 * uses the updated state to decide how much power to output and if the ESS slew rates should be overriden.
 * @param inputs Struct containing necessary inputs for Frequency Response algorithm. See
 * definition of Frequency_Response_Inputs for details.
 * @returns Struct containing the output requests of the Frequency Response algorithm.
 * See definition of Frequency_Response_Outputs for details.
 */
Frequency_Response_Outputs Freq_Resp_Component::frequency_response(const Frequency_Response_Inputs& inputs) {
    // for any variables which require an internal update when changed, check if they changed and perform the internal updates if so
    if (prev_active_cmd_kw != active_cmd_kw.value.value_float || prev_droop_limit_flag != droop_limit_flag.value.value_bool) {
        if (prev_active_cmd_kw != active_cmd_kw.value.value_float) {
            sync_active_cmd_kw();
        }
        build_droop_curve();
        prev_active_cmd_kw = active_cmd_kw.value.value_float;
        prev_droop_limit_flag = droop_limit_flag.value.value_bool;
    }
    if (prev_slew_rate_kw != slew_rate_kw.value.value_int) {
        slew_cmd_kw.set_slew_rate(slew_rate_kw.value.value_int);
        prev_slew_rate_kw = slew_rate_kw.value.value_int;
    }

    // Set limits on how large response can be this time based on how big it was last time
    slew_cmd_kw.update_slew_target(output_kw.value.value_float);

    // Update state based on user inputs, timers, and frequency
    update_state(inputs.current_time, inputs.site_frequency);

    // Determine new output based on updated state
    float raw_output_kw = calculate_kw_output(inputs.site_frequency);
    output_kw.value.value_float = slew_cmd_kw.get_slew_target(raw_output_kw);

    if (active_response_status.value.value_bool && ess_slew_override.value.value_bool) {
        // Instead of making min and max both equal component_output_kw, we give full range so
        // POI limiting and Watt-Watt features can make adjustments
        return Frequency_Response_Outputs{
            output_kw.value.value_float,
            +inputs.ess_total_rated_active_power,
            -inputs.ess_total_rated_active_power,
        };
    }

    return Frequency_Response_Outputs{
        output_kw.value.value_float,
        inputs.ess_max_potential,
        inputs.ess_min_potential,
    };
}

/**
 * @brief Updates the state of the frequency response component based on previous state,
 * current time, and the site frequency.
 * @param current_time. A timestamp passed into algorithm so all time-based decisions in
 * algorithm use same current time as reference.
 * @param site_frequency. Current frequency of the grid.
 */
void Freq_Resp_Component::update_state(timespec current_time, float site_frequency) {
    // if in cooldown, check if it is over or not
    if (in_cooldown.value.value_bool) {
        if (!check_expired_time(current_time, cooldown_over_time)) {
            return;  // if still in cooldown then nothing else to do
        }
        in_cooldown.value.value_bool = false;
    }
    // check if trigger event should start
    if (!active_response_status.value.value_bool && is_beyond_trigger(site_frequency)) {
        start_active_response();
    }
    // if there is no trigger event currently active, there is nothing left to do
    if (!active_response_status.value.value_bool) {
        return;
    }
    // if instant recovery or maxed out trigger time, event can be immediately ended
    if (check_expired_time(current_time, trigger_over_time) || is_within_instant_recovery(site_frequency)) {
        end_active_response();
        return;
    }
    // standard recovery states are only relevant if not using the droop response
    if (!droop_bypass_flag.value.value_bool) {
        return;
    }
    // if not in recovery, reset the recovery timer and check if recovery state should be entered
    if (!in_recovery.value.value_bool) {
        clock_gettime(CLOCK_MONOTONIC, &recovery_over_time);
        recovery_over_time.tv_sec += recovery_duration_sec.value.value_int;
        in_recovery.value.value_bool = is_within_recovery(site_frequency);
        return;
    }
    // if we are in a recovery state and option is allowed for discarding recovery state, check if it can be discarded
    if (!recovery_latch.value.value_bool && is_beyond_recovery(site_frequency)) {
        in_recovery.value.value_bool = false;
        clock_gettime(CLOCK_MONOTONIC, &recovery_over_time);
        recovery_over_time.tv_sec += recovery_duration_sec.value.value_int;
    }
    // end event if recovery has lasted long enough
    if (in_recovery.value.value_bool && check_expired_time(current_time, recovery_over_time)) {
        end_active_response();
    }
}

/**
 * @brief Decides, based on state, how much power to output.
 * @param current_frequency The current frequency of the grid.
 * @returns Determined power output.
 */
float Freq_Resp_Component::calculate_kw_output(float current_frequency) {
    // if no active trigger event, output inactive command
    if (!active_response_status.value.value_bool) {
        return inactive_cmd_kw.value.value_float;
    }
    // if no droop used, always respond with configured command
    if (droop_bypass_flag.value.value_bool) {
        return signed_active_cmd_kw;
    }
    // if droop being used, measure how much of the configured command with which to respond
    return get_curve_cmd(current_frequency, droop_curve);
}

/**
 * @brief Carries out necessary actions to start active response:
 * sets the active response flag to true,
 * resets the trigger timeout timer,
 * makes an info log, and emits an event.
 * Skips the info log and event emit if drooped response to avoid spam.
 */
void Freq_Resp_Component::start_active_response() {
    active_response_status.value.value_bool = true;
    clock_gettime(CLOCK_MONOTONIC, &trigger_over_time);
    trigger_over_time.tv_sec += trigger_duration_sec.value.value_int;
    if (droop_bypass_flag.value.value_bool) {
#ifndef FPS_TEST_MODE  // bypass emit event when running gtest as emit_event() will cause seg fault.
        FPS_INFO_LOG("Frequency Response: response %s triggered.", component_id.value.value_string);
        emit_event("Site", is_underfrequency_component ? "Frequency deviation: underfrequency event triggered" : "Frequency deviation: overfrequency event triggered", 1);
#endif
    }
}

/**
 * @brief Carries out necessary actions to end active response:
 * sets the active response and in-recovery flags to false,
 * starts the cooldown timer, and
 * sets the in-cooldown flag to true.
 */
void Freq_Resp_Component::end_active_response() {
    active_response_status.value.value_bool = false;
    in_recovery.value.value_bool = false;
    clock_gettime(CLOCK_MONOTONIC, &cooldown_over_time);
    cooldown_over_time.tv_sec += cooldown_duration_sec.value.value_int;
    in_cooldown.value.value_bool = true;
}

/**
 * @brief Used by the std::sort algorithm called in Freq_Resp_Component::build_droop_curve. Must be static non-member function.
 * Declares which point is "less than" the other. The "least" point is the one that has the lowest x-coordinate (frequency),
 * with the tie breaker being the point that has the highest y-coordinate (power) so that the points get ordered from the top-left
 * of the graph to the bottom-right.
 * @param p1 Point 1 to compare.
 * @param p2 Point 2 to compare.
 */
bool compare_droop_curve_points(std::pair<float, float> p1, std::pair<float, float> p2) {
    return (p1.first == p2.first) ? (p1.second > p2.second) : (p1.first < p2.first);
}

/**
 * @brief Builds the droop curve that will be used to interpolate or exterpolate response output.
 * Called at initialization and any time a variable is edited that the curve is dependent on.
 */
void Freq_Resp_Component::build_droop_curve(void) {
    // create basic curve
    float directional_offset = is_underfrequency_component ? -1.0 : +1.0;
    droop_curve = {
        std::make_pair(trigger_freq_hz.value.value_float - directional_offset, 0.0),  // if UF, zero for OF frequencies and vice versa
        std::make_pair(trigger_freq_hz.value.value_float, 0.0),                       // zero at the trigger frequency
        std::make_pair(droop_freq_hz.value.value_float, signed_active_cmd_kw)         // full command at droop frequency
    };
    // response at frequencies beyond droop frequency will continue climbing linearly unless configured to stop
    if (droop_limit_flag.value.value_bool) {
        droop_curve.push_back(std::make_pair(droop_freq_hz.value.value_float + directional_offset, signed_active_cmd_kw));
    }
    // sort the vector so points are presented in order from lowest frequency to highest frequency. required for proper interpolation
    std::sort(droop_curve.begin(), droop_curve.end(), compare_droop_curve_points);
}

/**
 * @brief Sets all outputs to their zero-values. Should be called when the response component is
 * disabled so that the component does not report stale data. For example, if the response is active
 * so its status is true, then it gets disabled, the status will remain true even if the frequency
 * deviation event that caused it to become active goes away.
 */
void Freq_Resp_Component::clear_outputs(void) {
    active_response_status.value.value_bool = false;
    in_cooldown.value.value_bool = false;
    in_recovery.value.value_bool = false;
    output_kw.value.value_float = 0.0f;
}

/**
 * @brief Parses a cJSON object for the Frequency Response feature's configuration data.
 * @param JSON_config cJSON object containing configuration data.
 * @param p_flag Pointer to the site_controller primary mode flag.
 * @param inputs Pointer to the list of input sources for Multiple Input control variables.
 * @param defaults Reference to default
 * @param multiple_inputs Mutable list of Multiple Input Command Variables that may need to be added to
 * if any of Frequency Response's inputs are configured to be Multiple Input Command Variables.
 * @returns True if parsing is successful or false if parsing failed.
 */
bool Freq_Resp_Component::parse_json_config(cJSON* JSON_config, bool* p_flag, Input_Source_List* inputs, const Fims_Object& defaults, std::vector<Fims_Object*>& multiple_inputs) {
    // caller must not pass a NULL cJSON*
    if (JSON_config == NULL) {
        FPS_ERROR_LOG("JSON_config is NULL");
        return false;
    }

    // parse id string
    cJSON* JSON_component_id = cJSON_GetObjectItem(JSON_config, "component_id");
    if (JSON_component_id == NULL || JSON_component_id->valuestring == NULL) {
        FPS_ERROR_LOG("Failed to parse component ID string from frequency_response component.");
        return false;
    }
    if (strchr(JSON_component_id->valuestring, ' ') != NULL) {
        FPS_ERROR_LOG("Frequency Response components cannot have spaces or slashes in their IDs. '%s' contains a space.", JSON_component_id->valuestring);
        return false;
    }
    if (strchr(JSON_component_id->valuestring, '/') != NULL) {
        FPS_ERROR_LOG("Frequency Response components cannot have spaces or slashes in their IDs. '%s' contains a slash.", JSON_component_id->valuestring);
        return false;
    }
    component_id.value.value_string = strdup(JSON_component_id->valuestring);

    // parse name string
    cJSON* JSON_component_name = cJSON_GetObjectItem(JSON_config, "component_name");
    if (JSON_component_name == NULL || JSON_component_name->valuestring == NULL) {
        FPS_ERROR_LOG("Failed to parse component name string from frequency_response component.");
        return false;
    }

    // parse rest of variables from variables.json
    for (auto& variable_id_pair : variable_ids) {
        cJSON* JSON_variable = cJSON_GetObjectItem(JSON_config, variable_id_pair.second.c_str());
        if (JSON_variable == NULL) {
            FPS_ERROR_LOG("Could not find variable %s in frequency response component %s", variable_id_pair.second.c_str(), component_id.value.value_string);
            return false;
        }
        // prepend the component ID to the variable ID
        std::string combined_setting_var = std::string(component_id.value.value_string) + '_' + variable_id_pair.second;
        if (!variable_id_pair.first->configure(combined_setting_var, p_flag, inputs, JSON_variable, defaults, multiple_inputs)) {
            FPS_ERROR_LOG("Failed to configure variable %s for frequency response component %s", variable_id_pair.second.c_str(), component_id.value.value_string);
            return false;
        }

        // prepend the component name to the variable name
        fmt::memory_buffer name;
        FORMAT_TO_BUF_C_STRING(name, "{} {}", (const char*)JSON_component_name->valuestring, variable_id_pair.first->get_name());
        variable_id_pair.first->set_name(name.data());
    }
    return true;
}

/**
 * @brief Handles FIMS SETs to URIs that begin with this frequency response component's ID.
 * @param JSON_body JSON containing the SET's value.
 * @param variable_id ID of the target variable to be edited.
 */
void Freq_Resp_Component::handle_fims_set(cJSON* JSON_body, const char* variable_id) {
    // extract value
    cJSON* body_value = cJSON_GetObjectItem(JSON_body, "value");
    int body_type = (body_value) ? body_value->type : JSON_body->type;
    float body_float = (body_value) ? body_value->valuedouble : JSON_body->valuedouble;
    int body_int = (body_value) ? body_value->valueint : JSON_body->valueint;
    bool body_bool = (body_type == cJSON_False) ? false : true;

    // find matching endpoint and handle SET
    if (active_cmd_kw.set_fims_float(variable_id, body_float))
        return;
    if (trigger_duration_sec.set_fims_int(variable_id, body_int))
        return;
    if (droop_limit_flag.set_fims_bool(variable_id, body_bool))
        return;
    if (droop_bypass_flag.set_fims_bool(variable_id, body_bool))
        return;
    if (inactive_cmd_kw.set_fims_float(variable_id, body_float))
        return;
    if (slew_rate_kw.set_fims_int(variable_id, body_int))
        return;
    if (ess_slew_override.set_fims_bool(variable_id, body_bool))
        return;
    if (cooldown_duration_sec.set_fims_int(variable_id, body_int))
        return;
    if (recovery_duration_sec.set_fims_int(variable_id, body_int))
        return;
    if (recovery_latch.set_fims_bool(variable_id, body_bool))
        return;

    FPS_ERROR_LOG("FIMS SET with endpoint %s did not match any frequency response component endpoints.", variable_id);
}

/**
 * @brief Loads the given vector with pointers to all Frequency Response parameters that
 * an external interface should know about.
 * @param var_list List of variable pointers that will be published periodically.
 */
void Freq_Resp_Component::get_feature_vars(std::vector<Fims_Object*>& var_list) {
    for (auto& curr_fims_obj : variable_ids) {
        var_list.push_back(curr_fims_obj.first);
    }
}

/**
 * @brief Carries out required state initialization tasks after configuration JSON has been parsed.
 * Calculates if this response is an under-frequency or over-frequency component, builds the droop
 * curve, resets slew, etc.
 * @param grid_target_freq_hz The frequency that the grid is technically supposed to be at. In the
 * United States, typically 60Hz. This is not used in the algorithm, but is a useful way to validate
 * that the feature was configured accurately.
 * @returns True if state initialization succeeded, or false if there was an error.
 */
bool Freq_Resp_Component::initialize_state(float grid_target_freq_hz) {
    is_underfrequency_component = trigger_freq_hz.value.value_float < grid_target_freq_hz;
    if (is_underfrequency_component) {
        if (droop_freq_hz.value.value_float > trigger_freq_hz.value.value_float) {
            FPS_ERROR_LOG("Invalid config: Frequency Response component %s is under-frequency response but droop frequency is greater than trigger frequency. Must be less than or equal to trigger frequency.", component_id.value.value_string);
            return false;
        }
    } else if (droop_freq_hz.value.value_float < trigger_freq_hz.value.value_float) {
        FPS_ERROR_LOG("Invalid config: Frequency Response component %s is over-frequency response but droop frequency is less than trigger frequency. Must be greater than or equal to trigger frequency.", component_id.value.value_string);
        return false;
    }
    sync_active_cmd_kw();
    build_droop_curve();
    slew_cmd_kw.reset_slew_target();
    slew_cmd_kw.set_slew_rate(slew_rate_kw.value.value_int);
    prev_active_cmd_kw = active_cmd_kw.value.value_float;
    prev_droop_limit_flag = droop_limit_flag.value.value_bool;
    prev_slew_rate_kw = slew_rate_kw.value.value_int;
    return true;
}

/**
 * @brief The active response command should always be displayed externally as a positive value.
 * This function sets the externally-facing variable to the absolute value of itself, then sets
 * the internal variable to have the appropriate sign based on whether the component is an
 * under-frequency or over-frequency response component.
 */
void Freq_Resp_Component::sync_active_cmd_kw(void) {
    active_cmd_kw.value.set(active_cmd_kw.value.value_float < 0.0 ? float(-1.0) * active_cmd_kw.value.value_float : active_cmd_kw.value.value_float);
    signed_active_cmd_kw = is_underfrequency_component ? active_cmd_kw.value.value_float : float(-1.0) * active_cmd_kw.value.value_float;
}

bool Freq_Resp_Component::is_beyond_trigger(float freq) {
    return is_underfrequency_component ? freq < trigger_freq_hz.value.value_float : freq > trigger_freq_hz.value.value_float;
}

bool Freq_Resp_Component::is_beyond_recovery(float freq) {
    return is_underfrequency_component ? freq < recovery_freq_hz.value.value_float : freq > recovery_freq_hz.value.value_float;
}

bool Freq_Resp_Component::is_within_recovery(float freq) {
    return is_underfrequency_component ? freq > recovery_freq_hz.value.value_float : freq < recovery_freq_hz.value.value_float;
}

bool Freq_Resp_Component::is_within_instant_recovery(float freq) {
    return is_underfrequency_component ? freq > instant_recovery_freq_hz.value.value_float : freq < instant_recovery_freq_hz.value.value_float;
}
