/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/ESS_Discharge_Prevention.h>
#include <Site_Controller_Utils.h>

features::ESS_Discharge_Prevention::ESS_Discharge_Prevention() {
    feature_vars = {
        &edp_soc,
    };

    variable_ids = {
        { &enable_flag, "ess_discharge_prevention_enable" },
        { &edp_soc, "ess_discharge_prevention_soc" },
    };
}

void features::ESS_Discharge_Prevention::execute(Asset_Cmd_Object& asset_cmd, float soc_avg_running, float& max_potential_ess_kW, float& min_potential_ess_kW, float ess_total_kW_discharge_limit, float& total_asset_kW_discharge_limit) {
    External_Inputs inputs{
        soc_avg_running, ess_total_kW_discharge_limit, max_potential_ess_kW, asset_cmd.ess_data.max_potential_kW, min_potential_ess_kW, asset_cmd.ess_data.min_potential_kW, total_asset_kW_discharge_limit,
    };
    External_Outputs outputs = execute_helper(inputs);
    max_potential_ess_kW = outputs.site_manager_max_potential_ess_kW;
    asset_cmd.ess_data.max_potential_kW = outputs.asset_cmd_ess_max_potential_kW;
    min_potential_ess_kW = outputs.site_manager_min_potential_ess_kW;
    asset_cmd.ess_data.min_potential_kW = outputs.asset_cmd_ess_min_potential_kW;
    total_asset_kW_discharge_limit = outputs.total_asset_kW_discharge_limit;
}

features::ESS_Discharge_Prevention::External_Outputs features::ESS_Discharge_Prevention::execute_helper(const External_Inputs& inputs) {
    // Start with outputs equal to corresponding inputs so any skipped conditional logic results in no change
    External_Outputs outputs = {
        inputs.site_manager_max_potential_ess_kW, inputs.asset_cmd_ess_max_potential_kW, inputs.site_manager_min_potential_ess_kW, inputs.asset_cmd_ess_min_potential_kW, inputs.total_asset_kW_discharge_limit,
    };

    if (inputs.soc_avg_running <= edp_soc.value.value_float) {
        if (inputs.asset_cmd_ess_max_potential_kW > 0) {
            outputs.site_manager_max_potential_ess_kW = 0.0f;
            outputs.asset_cmd_ess_max_potential_kW = 0.0f;

            // Adjusted the amount of ESS power considered available for discharge
            outputs.total_asset_kW_discharge_limit -= inputs.ess_total_kW_discharge_limit;
        }
        if (inputs.asset_cmd_ess_min_potential_kW > 0) {
            outputs.site_manager_min_potential_ess_kW = 0.0f;
            outputs.asset_cmd_ess_min_potential_kW = 0.0f;
        }
    }

    return outputs;
}

void features::ESS_Discharge_Prevention::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_Number) {
        edp_soc.set_fims_float(uri_endpoint.c_str(), range_check(msg_value.valuedouble, 100.0f, 0.0f));
    } else if (msg_value.type == cJSON_True || msg_value.type == cJSON_False) {
        bool value_bool = msg_value.type != cJSON_False;
        if (available) {
            enable_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
        }
    }
}