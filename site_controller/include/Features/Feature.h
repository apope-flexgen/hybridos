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
#include <Site_Controller_Utils.h>

struct Feature {
    std::vector<Fims_Object*> feature_vars;                          // All feature-specific variables (except the enable_flag)
    std::vector<Fims_Object*> summary_vars;                          // Feature-specific variables that should be displayed in feature summary
    Fims_Object enable_flag;                                         // True if this feature is currently being executed in algorithms
    bool available = false;                                          // True if this feature is available to the customer to turn on
    std::vector<std::pair<Fims_Object*, std::string>> variable_ids;  // Lists the id of each variable in configuration

    void add_feature_vars_to_JSON_buffer(fmt::memory_buffer& buf, const char* const var = NULL);
    void add_summary_vars_to_JSON_buffer(fmt::memory_buffer& buf, const char* const var = NULL);

    // set the ui_enabled field of all feature variables
    virtual void toggle_ui_enabled(bool flag);

    /**
     * @brief Parses a cJSON object for the feature's configuration data.
     * @param JSON_config cJSON object containing configuration data.
     * @param primary_flag Pointer to the site_controller primary mode flag.
     * @param inputs Pointer to the list of input sources for Multiple Input control variables.
     * @param field_defaults Reference to default values for JSON fields.
     * @param multiple_inputs Mutable list of Multiple Input Command Variables that may be appended to.
     * @returns Result detailing whether or not the config is valid
     */
    virtual Config_Validation_Result parse_json_config(cJSON* JSON_config, bool* primary_flag, Input_Source_List* inputs, const Fims_Object& field_defaults, std::vector<Fims_Object*>& multiple_inputs);

    /**
     * @brief Returns a list of the top-level variable ids recognized by the feature
     */
    virtual std::vector<std::string> get_variable_ids_list() const;

    /**
     * @brief Handles FIMS SETs to URIs belonging to the feature.
     * @param uri_endpoint The endpoint parsed from the uri.
     * @param msg_value The new value given by the FIMS message SET.
     */
    virtual void handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) = 0;
};

// Namespace of all feature subclasses
namespace features {
// Runmode 1 Active Power Features
class Active_Power_Setpoint;
class Target_SOC;
class Energy_Arbitrage;
class Frequency_Response;
class ESS_Calibration;
class Manual;

// Runmode 1 Reactive Power Features
class AVR;
class Watt_Var;
class Reactive_Setpoint;
class Direct_Power_Factor;
class Constant_Power_Factor;

// Runmode 1 Standalone Power Features
class Active_Power_Closed_Loop_Control;
class Active_Power_POI_Limits;
class Reactive_Power_POI_Limits;
class PFR;
class Watt_Watt;
class LDSS;  // Also a Runmode 2 Standalone Power Feature
class ESS_Discharge_Prevention;
class Aggregated_Asset_Limit;
class Reactive_Power_Closed_Loop_Control;

// Runmode 2 Active Power Features
class Generator_Charge;

// Runmode 2 Reactive Power Features
// (There are no runmode 2 reactive power features)

// Runmode 2 Standalone Power Features
class Load_Shed;
class Solar_Shed;

// Site Operation Features
class Watchdog;

// Charge Control Features
class Charge_Dispatch;
class Charge_Control;
}  // namespace features

#endif /* FEATURE_H_ */
