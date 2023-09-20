/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Watchdog.h>
#include <Site_Controller_Utils.h>

features::Watchdog::Watchdog() {
    feature_vars = {
        &watchdog_duration_ms,
        &watchdog_pet,
        &heartbeat_counter,
        &heartbeat_duration_ms,
    };

    variable_ids = {
        { &enable_flag, "watchdog_enable" }, { &watchdog_duration_ms, "watchdog_duration_ms" }, { &watchdog_pet, "watchdog_pet" }, { &heartbeat_counter, "heartbeat_counter" }, { &heartbeat_duration_ms, "heartbeat_duration_ms" },
    };

    watchdog_old_pet = 0;

    clock_gettime(CLOCK_MONOTONIC, &heartbeat_timer);
    clock_gettime(CLOCK_MONOTONIC, &watchdog_timeout);
    increment_timespec_ms(watchdog_timeout, watchdog_duration_ms.value.value_int);
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

    // If it is time for the heart to send out another beat (signaling to master controller that site_controller is still operational)
    if (check_expired_time(current_time, heartbeat_timer)) {
        ++heartbeat_counter.value.value_int;                                            // Update counter that will be published for master controller
        clock_gettime(CLOCK_MONOTONIC, &heartbeat_timer);                               // Get current time
        increment_timespec_ms(heartbeat_timer, heartbeat_duration_ms.value.value_int);  // Reset heartbeat timer
    }
    return trigger_bark;
}

void features::Watchdog::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_Number) {
        watchdog_pet.set_fims_int(uri_endpoint.c_str(), fabsf(msg_value.valueint));
        // UI configured timer duration
        watchdog_duration_ms.set_fims_int(uri_endpoint.c_str(), fabsf(msg_value.valueint));
    } else if (msg_value.type == cJSON_True || msg_value.type == cJSON_False) {
        bool value_bool = msg_value.type != cJSON_False;
        if (available) {
            enable_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
        }
    }
}