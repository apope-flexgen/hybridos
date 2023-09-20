/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Reactive_Power_POI_Limits.h>
#include <Site_Controller_Utils.h>

features::Reactive_Power_POI_Limits::Reactive_Power_POI_Limits() {
    feature_vars = {
        &min_kVAR,
        &max_kVAR,
    };
    summary_vars = {
        &min_kVAR,
        &max_kVAR,
    };

    variable_ids = {
        { &enable_flag, "reactive_power_poi_limits_enable" },
        { &min_kVAR, "reactive_power_poi_limits_min_kVAR" },
        { &max_kVAR, "reactive_power_poi_limits_max_kVAR" },
    };
}

void features::Reactive_Power_POI_Limits::execute(Asset_Cmd_Object& asset_cmd) {
    External_Inputs inputs{
        asset_cmd.site_kVAR_demand,
    };
    External_Outputs outputs = execute_helper(inputs);
    asset_cmd.site_kVAR_demand = outputs.site_kVAR_demand;
}

features::Reactive_Power_POI_Limits::External_Outputs features::Reactive_Power_POI_Limits::execute_helper(const External_Inputs& inputs) {
    float new_site_kVAR_demand = range_check(inputs.site_kVAR_demand, max_kVAR.value.value_float, min_kVAR.value.value_float);

    return External_Outputs{
        new_site_kVAR_demand,
    };
}

void features::Reactive_Power_POI_Limits::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_Number) {
        min_kVAR.set_fims_float(uri_endpoint.c_str(), -1 * fabsf(msg_value.valuedouble));
        max_kVAR.set_fims_float(uri_endpoint.c_str(), fabsf(msg_value.valuedouble));
    } else if (msg_value.type == cJSON_True || msg_value.type == cJSON_False) {
        bool value_bool = msg_value.type != cJSON_False;
        if (available) {
            enable_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
        }
    }
}