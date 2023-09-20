/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Watt_Watt.h>
#include <Site_Controller_Utils.h>

features::Watt_Watt::Watt_Watt() {
    feature_vars = { &watt_watt_points };

    variable_ids = {
        { &enable_flag, "watt_watt_adjustment_enable_flag" },
        { &watt_watt_points, "watt_watt_points" },
    };
}

void features::Watt_Watt::init_curve() {
    set_curve_points(&watt_watt_points, watt_watt_curve);
}

void features::Watt_Watt::execute(Asset_Cmd_Object& asset_cmd) {
    External_Inputs inputs{
        asset_cmd.site_kW_demand,
    };
    External_Outputs outputs = execute_helper(inputs);
    asset_cmd.site_kW_demand = outputs.site_kW_demand;
}

features::Watt_Watt::External_Outputs features::Watt_Watt::execute_helper(const External_Inputs& inputs) {
    float new_site_kW_demand = get_curve_cmd(inputs.site_kW_demand, watt_watt_curve);
    site_kW_demand_correction = new_site_kW_demand - inputs.site_kW_demand;

    return External_Outputs{
        new_site_kW_demand,
    };
}

void features::Watt_Watt::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_True || msg_value.type == cJSON_False) {
        bool value_bool = msg_value.type != cJSON_False;
        if (available) {
            enable_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
        }
    }
}