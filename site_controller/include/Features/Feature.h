#ifndef FEATURE_H_
#define FEATURE_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
#include <memory>
/* External Dependencies */
#include <spdlog/fmt/fmt.h>
/* System Internal Dependencies */
#include <fims/libfims.h>
/* Local Internal Dependencies */
#include <Fims_Object.h>

struct Feature {
    std::vector<Fims_Object*> feature_vars;                          // All feature-specific variables
    std::vector<Fims_Object*> summary_vars;                          // Feature-specific variables that should be displayed in feature summary
    Fims_Object enable_flag;                                         // True if this feature is currently being executed in algorithms
    bool available = false;                                          // True if this feature is available to the customer to turn on
    std::vector<std::pair<Fims_Object*, std::string>> variable_ids;  // Lists the id of each variable in configuration

    void add_feature_vars_to_JSON_buffer(fmt::memory_buffer& buf, const char* const var = NULL);
    void add_summary_vars_to_JSON_buffer(fmt::memory_buffer& buf, const char* const var = NULL);
    void toggle_ui_enabled(bool);

    /**
     * @brief Parses a cJSON object for the feature's configuration data.
     * @param JSON_config cJSON object containing configuration data.
     * @param primary_flag Pointer to the site_controller primary mode flag.
     * @param inputs Pointer to the list of input sources for Multiple Input control variables.
     * @param field_defaults Reference to default values for JSON fields.
     * @param multiple_inputs Mutable list of Multiple Input Command Variables that may be appended to.
     * @returns True if parsing is successful or false if parsing failed.
     */
    virtual bool parse_json_config(cJSON* JSON_config, bool* primary_flag, Input_Source_List* inputs, const Fims_Object& field_defaults, std::vector<Fims_Object*>& multiple_inputs);

    /**
     * @brief Handles FIMS SETs to URIs belonging to the feature.
     * @param uri_endpoint The endpoint parsed from the uri.
     * @param msg_value The new value given by the FIMS message SET.
     */
    virtual void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) = 0;
};

// Namespace of all feature subclasses
// TODO: as features get their own feature subclasses, add them to the namespace until they are all accounted for
namespace features {
// Runmode 1 Active Power Features
class Active_Power_Setpoint;
class Target_SOC;

// Runmode 1 Reactive Power Features
class Active_Voltage_Regulation;
class Watt_Var;
class Reactive_Setpoint;
class Direct_Power_Factor;
class Constant_Power_Factor;

// Runmode 1 Standalone Power Features
class Active_Power_Closed_Loop_Control;

// Runmode 2 Active Power Features
// Runmode 2 Reactive Power Features
// Runmode 2 Standalone Power Features
}  // namespace features

#endif /* FEATURE_H_ */