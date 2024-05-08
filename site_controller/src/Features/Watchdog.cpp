/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Watchdog.h>
#include <Site_Controller_Utils.h>
#include <climits>

features::Watchdog::Watchdog() {
    feature_vars = {
        &watchdog_duration_ms,
        &watchdog_pet,
        &heartbeat_counter,
        &heartbeat_duration_ms,
    };

    variable_ids = {
        { &enable_flag, "watchdog_enable" },         { &watchdog_duration_ms, "watchdog_duration_ms" },   { &watchdog_pet, "watchdog_pet" },
        { &heartbeat_counter, "heartbeat_counter" }, { &heartbeat_duration_ms, "heartbeat_duration_ms" },
    };

    optional_feature_vars = {
        &max_heartbeat,
        &min_heartbeat,
    };

    optional_variable_ids = {
        { &max_heartbeat, "max_heartbeat" },
        { &min_heartbeat, "min_heartbeat" },
    };

    watchdog_old_pet = 0;

    clock_gettime(CLOCK_MONOTONIC, &heartbeat_timer);
    clock_gettime(CLOCK_MONOTONIC, &watchdog_timeout);
    increment_timespec_ms(watchdog_timeout, watchdog_duration_ms.value.value_int);
}

Config_Validation_Result features::Watchdog::parse_json_config(cJSON* JSON_config, bool* primary_flag, Input_Source_List* inputs, const Fims_Object& field_defaults,
                                                               std::vector<Fims_Object*>& multiple_inputs) {
    Config_Validation_Result result;
    result.is_valid_config = true;
    for (auto& variable_id_pair : variable_ids) {
        cJSON* JSON_variable = cJSON_GetObjectItem(JSON_config, variable_id_pair.second.c_str());
        if (JSON_variable == NULL) {
            result.ERROR_details.push_back(Result_Details(fmt::format("Required variable \"{}\" is missing from variables.json configuration.", variable_id_pair.second)));
            result.is_valid_config = false;
            continue;
        }
        if (!variable_id_pair.first->configure(variable_id_pair.second, primary_flag, inputs, JSON_variable, field_defaults, multiple_inputs)) {
            result.ERROR_details.push_back(Result_Details(fmt::format("Failed to parse variable with ID \"{}\"", variable_id_pair.second)));
            result.is_valid_config = false;
        }
    }

    for (auto& variable_id_pair : optional_variable_ids) {
        cJSON* JSON_variable = cJSON_GetObjectItem(JSON_config, variable_id_pair.second.c_str());
        if (JSON_variable == NULL) {
            continue;
        }
        if (!variable_id_pair.first->configure(variable_id_pair.second, primary_flag, inputs, JSON_variable, field_defaults, multiple_inputs)) {
            result.ERROR_details.push_back(Result_Details(fmt::format("Failed to parse optional variable with ID \"{}\"", variable_id_pair.second)));
            result.is_valid_config = false;
        }
    }
    return result;
};

// add all variables associated with this feature, both status and controls
void features::Watchdog::add_feature_vars_to_JSON_buffer(fmt::memory_buffer& buf, const char* const var) {
    enable_flag.add_to_JSON_buffer(buf, var);
    for (auto* it : feature_vars) {
        it->add_to_JSON_buffer(buf, var);
    }

    // add optional vars if they are configured
    for (auto* it : optional_feature_vars) {
        if (it->configured) {
            it->add_to_JSON_buffer(buf, var);
        }
    }
}

void features::Watchdog::beat(timespec current_time) {
    // If it is time for the heart to send out another beat (signaling to master controller that site_controller is still operational)
    if (check_expired_time(current_time, heartbeat_timer)) {
        int min_beat = min_heartbeat.configured ? min_heartbeat.value.value_int : 0;
        int max_beat = max_heartbeat.configured ? max_heartbeat.value.value_int : INT_MAX;

        // Update counter that will be published for master controller
        ++heartbeat_counter.value.value_int;

        if (heartbeat_counter.value.value_int > max_beat) {
            heartbeat_counter.value.set(min_beat);
        }
        if (heartbeat_counter.value.value_int < min_beat) {
            heartbeat_counter.value.set(min_beat);
        }

        // reset timer
        clock_gettime(CLOCK_MONOTONIC, &heartbeat_timer);                               // Get current time
        increment_timespec_ms(heartbeat_timer, heartbeat_duration_ms.value.value_int);  // Reset heartbeat timer
    }
}

bool features::Watchdog::should_bark(timespec current_time) {
    bool trigger_bark = false;
    // If the watchdog has received a new pet from the master controller
    if (watchdog_old_pet != watchdog_pet.value.value_int) {
        watchdog_old_pet = watchdog_pet.value.value_int;                                // Update old pet
        clock_gettime(CLOCK_MONOTONIC, &watchdog_timeout);                              // Get current time
        increment_timespec_ms(watchdog_timeout, watchdog_duration_ms.value.value_int);  // Reset watchdog expiration time
    }
    // If no new pet and the watchdog has timed out
    else if (check_expired_time(current_time, watchdog_timeout)) {
        // Execute watchdog failure responses
        trigger_bark = true;
    }

    return trigger_bark;
}

void features::Watchdog::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_Number) {
        watchdog_pet.set_fims_int(uri_endpoint.c_str(), std::abs(msg_value.valueint));
        // UI configured timer duration
        watchdog_duration_ms.set_fims_int(uri_endpoint.c_str(), std::abs(msg_value.valueint));
        if (min_heartbeat.configured) {
            min_heartbeat.set_fims_int(uri_endpoint.c_str(), msg_value.valueint);
        }
        if (max_heartbeat.configured) {
            max_heartbeat.set_fims_int(uri_endpoint.c_str(), msg_value.valueint);
        }
    } else if (msg_value.type == cJSON_True || msg_value.type == cJSON_False) {
        bool value_bool = msg_value.type != cJSON_False;
        if (available) {
            enable_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
        }
    }
}
