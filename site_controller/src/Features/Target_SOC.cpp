/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Target_SOC.h>

features::Target_SOC::Target_SOC() {
    feature_vars = {
        &load_enable_flag,
    };

    variable_ids = {
        { &enable_flag, "target_soc_mode_enable_flag" },
        { &load_enable_flag, "target_soc_load_enable_flag" },
    };
}

void features::Target_SOC::execute(Asset_Cmd_Object& asset_cmd) {
    External_Inputs inputs = External_Inputs{ asset_cmd.solar_data.max_potential_kW, asset_cmd.site_kW_load, asset_cmd.site_kW_demand, asset_cmd.ess_data.kW_request, asset_cmd.solar_data.kW_request };
    External_Outputs outputs = execute_helper(inputs);
    asset_cmd.solar_data.kW_request = outputs.solar_kW_request;
    asset_cmd.load_method = outputs.load_method;
    asset_cmd.additional_load_compensation = outputs.additional_load_compensation;
    asset_cmd.site_kW_demand = outputs.site_kW_demand;
}

features::Target_SOC::External_Outputs features::Target_SOC::execute_helper(const External_Inputs& inputs) {
    // Solar uncurtailed
    float new_solar_kW_request = inputs.solar_max_potential_kW;

    // Track load as a demand minimum if appropriate
    load_compensation new_load_method = load_compensation(load_enable_flag.value.value_bool * LOAD_MINIMUM);
    float new_additional_load_compensation = asset_cmd_utils::calculate_additional_load_compensation(new_load_method, inputs.site_kW_load, inputs.site_kW_demand, inputs.ess_kW_request,
                                                                                                     0.0f,  // there will never be a gen request when using this feature
                                                                                                     inputs.solar_kW_request);
    float new_site_kW_demand = inputs.site_kW_demand + new_additional_load_compensation;

    return External_Outputs{
        new_solar_kW_request,
        new_load_method,
        new_additional_load_compensation,
        new_site_kW_demand,
    };
}

void features::Target_SOC::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_True || msg_value.type == cJSON_False) {
        bool value_bool = msg_value.type != cJSON_False;
        // Target SoC load requirement
        load_enable_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
    }
}