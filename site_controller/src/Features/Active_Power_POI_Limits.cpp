/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Active_Power_POI_Limits.h>
#include <Site_Controller_Utils.h>

features::Active_Power_POI_Limits::Active_Power_POI_Limits() {
    feature_vars = {
        &max_kW, &min_kW, &soc_poi_limits_enable, &target_soc, &soc_low_min_kW, &soc_low_max_kW, &soc_high_min_kW, &soc_high_max_kW,
    };
    summary_vars = {
        &max_kW,
        &min_kW,
    };

    variable_ids = {
        { &enable_flag, "active_power_poi_limits_enable" },
        { &max_kW, "active_power_poi_limits_max_kW" },
        { &min_kW, "active_power_poi_limits_min_kW" },
        { &soc_poi_limits_enable, "active_power_soc_poi_limits_enable" },
        { &target_soc, "active_power_soc_poi_target_soc" },
        { &soc_low_min_kW, "active_power_soc_poi_limits_low_min_kW" },
        { &soc_low_max_kW, "active_power_soc_poi_limits_low_max_kW" },
        { &soc_high_min_kW, "active_power_soc_poi_limits_high_min_kW" },
        { &soc_high_max_kW, "active_power_soc_poi_limits_high_max_kW" },
    };
}

void features::Active_Power_POI_Limits::execute(Asset_Cmd_Object& asset_cmd, float soc_avg_running, int asset_priority_runmode1, float& total_site_kW_charge_limit, float& total_site_kW_discharge_limit) {
    float max_poi_limit = max_kW.value.value_float;
    float min_poi_limit = min_kW.value.value_float;

    // soc-based POI limits are enabled, overwriting the default POI limits
    if (soc_poi_limits_enable.value.value_bool) {
        // Only apply under limits if ESS are below the configured threshold
        if (soc_avg_running <= target_soc.value.value_float) {
            max_poi_limit = soc_low_max_kW.value.value_float;
            min_poi_limit = soc_low_min_kW.value.value_float;
            // Only apply over limits if ESS are above the configured threshold
        } else {
            max_poi_limit = soc_high_max_kW.value.value_float;
            min_poi_limit = soc_high_min_kW.value.value_float;
        }
    }

    // Limit explicit POI request in addition to demand
    asset_cmd.poi_cmd = range_check(asset_cmd.poi_cmd, max_poi_limit, min_poi_limit);

    // POI limits are asymmetric due to there being a single source of POI charge and multiple sources of POI discharge
    // Apply the charge POI limit
    asset_cmd.feeder_data.max_potential_kW = std::min(asset_cmd.feeder_data.max_potential_kW, -1.0f * min_poi_limit);
    asset_cmd.determine_ess_load_requirement(asset_priority_runmode1);

    // Limit site demand, tracking load if enabled to ensure an accurate value at the POI
    float min_limit_with_load = min_poi_limit + asset_cmd.get_site_kW_load_inclusion() * asset_cmd.site_kW_load;
    float max_limit_with_load = max_poi_limit + asset_cmd.get_site_kW_load_inclusion() * asset_cmd.site_kW_load;
    asset_cmd.site_kW_demand = range_check(asset_cmd.site_kW_demand, max_limit_with_load, min_limit_with_load);

    // Apply to reported site limits. Cancelling does not need to be calculated as only one of charge/discharge will be active at a time
    // Load must be included in all cases as the total limit represents the value that will be seen at the POI
    total_site_kW_charge_limit = (range_check(total_site_kW_charge_limit + asset_cmd.site_kW_load, max_poi_limit, min_poi_limit));
    total_site_kW_discharge_limit = (range_check(total_site_kW_discharge_limit + asset_cmd.site_kW_load, max_poi_limit, min_poi_limit));
}

void features::Active_Power_POI_Limits::toggle_ui_enabled(bool flag) {
    // For the main POI limits feature, enable its variables based on the enable status of the feature and the inverse of soc-based limits enable status
    bool poi_limits_enabled_status = flag && !soc_poi_limits_enable.value.value_bool;
    // For soc-based POI limits, enable its variables based on the enable status of the feature and the subfeature enable status
    bool soc_limits_enabled_status = flag && soc_poi_limits_enable.value.value_bool;

    // Apply to all variables in the feature
    for (auto it : feature_vars) {
        it->ui_enabled = poi_limits_enabled_status;
    }
    // Unique behavior for sub feature control: if feature is enabled it will always be enabled
    soc_poi_limits_enable.ui_enabled = flag;
    // Update soc-based subfeature to mirror it's enabled status
    target_soc.ui_enabled = soc_limits_enabled_status;
    soc_low_min_kW.ui_enabled = soc_limits_enabled_status;
    soc_low_max_kW.ui_enabled = soc_limits_enabled_status;
    soc_high_min_kW.ui_enabled = soc_limits_enabled_status;
    soc_high_max_kW.ui_enabled = soc_limits_enabled_status;
}

void features::Active_Power_POI_Limits::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_Number) {
        max_kW.set_fims_float(uri_endpoint.c_str(), fabsf(msg_value.valuedouble));
        min_kW.set_fims_float(uri_endpoint.c_str(), -1 * fabsf(msg_value.valuedouble));
        target_soc.set_fims_float(uri_endpoint.c_str(), fabsf(msg_value.valuedouble));
        soc_low_min_kW.set_fims_float(uri_endpoint.c_str(), -1 * fabsf(msg_value.valuedouble));
        soc_low_max_kW.set_fims_float(uri_endpoint.c_str(), fabsf(msg_value.valuedouble));
        soc_high_min_kW.set_fims_float(uri_endpoint.c_str(), -1 * fabsf(msg_value.valuedouble));
        soc_high_max_kW.set_fims_float(uri_endpoint.c_str(), fabsf(msg_value.valuedouble));
    } else if (msg_value.type == cJSON_True || msg_value.type == cJSON_False) {
        bool value_bool = msg_value.type != cJSON_False;
        if (available) {
            enable_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
        }
        soc_poi_limits_enable.set_fims_bool(uri_endpoint.c_str(), value_bool);
    }
}
