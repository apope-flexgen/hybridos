#ifndef FEATURES_WATCHDOG_H_
#define FEATURES_WATCHDOG_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Feature.h>
#include <Asset_Cmd_Object.h>

/**
 * Watchdog will timeout if there is no pet from a master controller for a configured timeout duration.
 * It also maintains a process heartbeat for the master controller's visibility.
 * In this context, a "pet" is an updated master controller heartbeat value and the
 * master controller is a third-party process that listens for this process' heartbeat and sends it pets.
 */
class features::Watchdog : public Feature {
public:
    Watchdog();

    std::vector<Fims_Object*> optional_feature_vars;                          // All feature-specific variables (except the enable_flag)

    Fims_Object watchdog_duration_ms;   // Time since last pet from master controller
    Fims_Object watchdog_pet;           // Last received pet value
    Fims_Object heartbeat_counter;      // Current heartbeat value
    Fims_Object heartbeat_duration_ms;  // Time since heartbeat value was updated

    int watchdog_old_pet;       // Pet value at last pet update
    timespec heartbeat_timer;   // Time of last heartbeat update
    timespec watchdog_timeout;  // Time of last pet update

    // OPTIONAL VARIABLES
    std::vector<std::pair<Fims_Object*, std::string>> optional_variable_ids;  // optional_variable_ids
    Fims_Object max_heartbeat;                                                // Optional configuration to specify the highest the heart will get before wrapping
    Fims_Object min_heartbeat;                                                // Optional configuration to specify the lowest the heart will start at

    // Updates heartbeat timer 
    void beat(timespec current_time);
    // returns true if the watchdog should bark
    bool should_bark(timespec current_time);

    void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) override;
    void add_feature_vars_to_JSON_buffer(fmt::memory_buffer& buf, const char* const var = NULL) override;
    virtual Config_Validation_Result parse_json_config(cJSON* JSON_config, bool* primary_flag, Input_Source_List* inputs, const Fims_Object& field_defaults,
                                                       std::vector<Fims_Object*>& multiple_inputs) override;
};

#endif /* FEATURES_WATCHDOG_H_ */
