/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Aggregated_Asset_Limit.h>
#include <Site_Controller_Utils.h>

features::Aggregated_Asset_Limit::Aggregated_Asset_Limit() {
    feature_vars = {
        &agg_asset_limit_kw,
    };

    variable_ids = {
        { &enable_flag, "agg_asset_limit_enable" },
        { &agg_asset_limit_kw, "agg_asset_limit_kw" },
    };
}

void features::Aggregated_Asset_Limit::execute(Asset_Cmd_Object& asset_cmd, float uncontrolled_ess_kW, float uncontrolled_solar_kW, float& max_potential_ess_kW, float& min_potential_ess_kW, float& total_asset_kW_discharge_limit,
                                               float ess_total_kW_discharge_limit) {
    External_Inputs inputs{
        asset_cmd.solar_data.actual_kW,      uncontrolled_solar_kW,          uncontrolled_ess_kW, ess_total_kW_discharge_limit, max_potential_ess_kW, asset_cmd.ess_data.max_potential_kW, min_potential_ess_kW,
        asset_cmd.ess_data.min_potential_kW, total_asset_kW_discharge_limit,
    };
    External_Outputs outputs = execute_helper(inputs);
    max_potential_ess_kW = outputs.site_manager_max_potential_ess_kW;
    asset_cmd.ess_data.max_potential_kW = outputs.asset_cmd_ess_max_potential_kW;
    min_potential_ess_kW = outputs.site_manager_min_potential_ess_kW;
    asset_cmd.ess_data.min_potential_kW = outputs.asset_cmd_ess_min_potential_kW;
    total_asset_kW_discharge_limit = outputs.total_asset_kW_discharge_limit;
}

features::Aggregated_Asset_Limit::External_Outputs features::Aggregated_Asset_Limit::execute_helper(const External_Inputs& inputs) {
    // Start with outputs equal to corresponding inputs so any skipped conditional logic results in no change
    External_Outputs outputs{
        inputs.site_manager_max_potential_ess_kW, inputs.asset_cmd_ess_max_potential_kW, inputs.site_manager_min_potential_ess_kW, inputs.asset_cmd_ess_min_potential_kW, inputs.total_asset_kW_discharge_limit,
    };

    float solar_power = inputs.solar_actual_kW + inputs.uncontrolled_solar_kW;
    float ess_limit = zero_check(agg_asset_limit_kw.value.value_float - solar_power - inputs.uncontrolled_ess_kW);

    // if calculated limit is below current limit, lower the current limit
    if (ess_limit < inputs.asset_cmd_ess_max_potential_kW) {
        outputs.site_manager_max_potential_ess_kW = (zero_check(ess_limit));
        outputs.asset_cmd_ess_max_potential_kW = outputs.site_manager_max_potential_ess_kW;

        // Adjusted the amount of ESS power considered available for discharge
        outputs.total_asset_kW_discharge_limit -= inputs.ess_total_kW_discharge_limit + ess_limit;

        // if new max is below what the min was, lower the min as well
        if (outputs.asset_cmd_ess_min_potential_kW > outputs.site_manager_max_potential_ess_kW) {
            outputs.site_manager_min_potential_ess_kW = outputs.site_manager_max_potential_ess_kW;
            outputs.asset_cmd_ess_min_potential_kW = outputs.site_manager_min_potential_ess_kW;
        }
    }

    return outputs;
}

void features::Aggregated_Asset_Limit::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_True || msg_value.type == cJSON_False) {
        bool value_bool = msg_value.type != cJSON_False;
        if (available) {
            enable_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
        }
    }
}
