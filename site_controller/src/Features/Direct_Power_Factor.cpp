/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Direct_Power_Factor.h>
#include <Site_Controller_Utils.h>

features::Direct_Power_Factor::Direct_Power_Factor() {
    feature_vars = {
        &power_factor_cmd,
    };

    variable_ids = {
        { &enable_flag, "power_factor_mode_enable_flag" },
        { &power_factor_cmd, "power_factor_cmd" },
    };

    prev_asset_pf_cmd = 0.0;
}

void features::Direct_Power_Factor::execute(Asset_Cmd_Object& asset_cmd, bool& asset_pf_flag) {
    External_Outputs outputs = execute_helper();
    asset_cmd.site_kVAR_demand = outputs.site_kVAR_demand;
    asset_pf_flag = outputs.asset_pf_flag;
}

features::Direct_Power_Factor::External_Outputs features::Direct_Power_Factor::execute_helper() {
    float new_site_kVAR_demand = 0;

    // this mode does use power factor control
    bool new_asset_pf_flag = true;

    return External_Outputs{
        new_site_kVAR_demand,
        new_asset_pf_flag,
    };
}

void features::Direct_Power_Factor::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_Number) {
        power_factor_cmd.set_fims_float(uri_endpoint.c_str(), zero_check(msg_value.valuedouble > 1 ? 1 : msg_value.valuedouble));
    }
}
