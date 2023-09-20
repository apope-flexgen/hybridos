/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Watt_Var.h>
#include <Site_Controller_Utils.h>

features::Watt_Var::Watt_Var() {
    feature_vars = {
        &watt_var_points,
    };

    variable_ids = {
        { &enable_flag, "watt_var_mode_enable_flag" },
        { &watt_var_points, "watt_var_points" },
    };
}

void features::Watt_Var::init_curve() {
    set_curve_points(&watt_var_points, watt_var_curve);
}

void features::Watt_Var::execute(Asset_Cmd_Object& asset_cmd, float active_power, bool& asset_pf_flag) {
    External_Inputs inputs{
        asset_cmd.site_kVAR_demand,
        active_power,
    };
    External_Outputs outputs = execute_helper(inputs);
    asset_cmd.site_kVAR_demand = outputs.site_kVAR_demand;
    asset_pf_flag = outputs.asset_pf_flag;
}

features::Watt_Var::External_Outputs features::Watt_Var::execute_helper(const External_Inputs& inputs) {
    float new_site_kVAR_demand = get_curve_cmd(inputs.active_power, watt_var_curve);
    site_kVAR_demand_correction = new_site_kVAR_demand - inputs.site_kVAR_demand;

    // this mode does not use power factor control
    bool new_asset_pf_flag = false;

    return External_Outputs{
        new_site_kVAR_demand,
        new_asset_pf_flag,
    };
}