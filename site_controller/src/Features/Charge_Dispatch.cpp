/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Charge_Dispatch.h>
#include <Site_Controller_Utils.h>

features::Charge_Dispatch::Charge_Dispatch() {
    feature_vars = {
        &kW_command,  // include in this list as a status is required for the feature to display
        &solar_enable_flag,
        &gen_enable_flag,
        &feeder_enable_flag,
    };

    variable_ids = {
        { &enable_flag, "_charge_dispatch_enable_flag" },
        { &kW_command, "charge_dispatch_kW_command" },
        { &gen_enable_flag, "charge_dispatch_gen_enable_flag" },
        { &solar_enable_flag, "charge_dispatch_solar_enable_flag" },
        { &feeder_enable_flag, "charge_dispatch_feeder_enable_flag" },
    };
}

void features::Charge_Dispatch::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_True || msg_value.type == cJSON_False) {
        bool value_bool = msg_value.type != cJSON_False;
        solar_enable_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
        gen_enable_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
        feeder_enable_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
    }
}
