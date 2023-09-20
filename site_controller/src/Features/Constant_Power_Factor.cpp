/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Constant_Power_Factor.h>
#include <Site_Controller_Utils.h>

features::Constant_Power_Factor::Constant_Power_Factor() {
    feature_vars = {
        &power_factor_setpoint, &lagging_limit, &leading_limit, &absolute_mode, &lagging_direction,
    };

    variable_ids = {
        { &enable_flag, "constant_power_factor_mode_enable_flag" }, { &power_factor_setpoint, "constant_power_factor_setpoint" }, { &lagging_limit, "constant_power_factor_lagging_limit" },
        { &leading_limit, "constant_power_factor_leading_limit" },  { &absolute_mode, "constant_power_factor_absolute_mode" },    { &lagging_direction, "constant_power_factor_lagging_direction" },
    };
}

void features::Constant_Power_Factor::execute(Asset_Cmd_Object& asset_cmd, bool& asset_pf_flag) {
    External_Inputs inputs{
        asset_cmd.uncorrected_site_kW_demand,
        asset_cmd.total_potential_kVAR,
    };
    External_Outputs outputs = execute_helper(inputs);
    asset_cmd.site_kVAR_demand = outputs.site_kVAR_demand;
    asset_pf_flag = outputs.asset_pf_flag;
}

features::Constant_Power_Factor::External_Outputs features::Constant_Power_Factor::execute_helper(const External_Inputs& inputs) {
    // Ensure limits are bounded
    lagging_limit.value.set(range_check(-1.0f * std::abs(lagging_limit.value.value_float), 0.0f, -1.0f));
    leading_limit.value.set(range_check(std::abs(leading_limit.value.value_float), 1.0f, 0.0f));
    // Establish command direction
    bool cpf_direction;
    // If direction flag is enabled, use it to determine direction
    if (absolute_mode.value.value_bool)
        cpf_direction = lagging_direction.value.value_bool;
    else
        cpf_direction = std::signbit(power_factor_setpoint.value.value_float);

    // Apply the appropriate limit based on direction (negative is lagging)
    if (cpf_direction) {
        // Explicitly set direction of the setpoint
        power_factor_setpoint.value.set(-1.0f * std::abs(power_factor_setpoint.value.value_float));
        // Bound the setpoint using lagging limit, -1.0 < pf < lagging limit
        power_factor_setpoint.value.set(range_check(power_factor_setpoint.value.value_float, lagging_limit.value.value_float, -1.0f));
    } else {
        // Explicitly set direction of the setpoint
        power_factor_setpoint.value.set(std::abs(power_factor_setpoint.value.value_float));
        // Bound the setpoint using leading limit < pf < 1.0
        power_factor_setpoint.value.set(range_check(power_factor_setpoint.value.value_float, 1.0f, leading_limit.value.value_float));
    }

    // Calculate site_kVAR_demand based on active power (site_kW_demand without POI corrections) and
    // power factor setpoint received

    // Calculate magnitude of reactive power
    float new_site_kVAR_demand;
    if (power_factor_setpoint.value.value_float != 0.0f) {
        // Dispatch Q based on P and pf
        // Equation derived from power factor equation:
        //    power factor = active power / apparent power
        //    apparent power = sqrt(active power^2 + reactive power^2)
        new_site_kVAR_demand = inputs.uncorrected_site_kW_demand * std::sqrt(1 / std::pow(power_factor_setpoint.value.value_float, 2) - 1);
        // Limit by total potential for small power factor setpoints
        new_site_kVAR_demand = std::min(std::abs(new_site_kVAR_demand), std::abs(inputs.total_potential_kVAR));
    } else
        // Dispatch full reactive power capability (and no active power) when pf is 0
        new_site_kVAR_demand = std::abs(inputs.total_potential_kVAR);

    // Apply configured sign (true is negative, false is positive)
    new_site_kVAR_demand = cpf_direction ? -1.0f * new_site_kVAR_demand : new_site_kVAR_demand;

    // this mode does not use power factor control (it will if it is ever implemented)
    bool new_asset_pf_flag = false;

    return External_Outputs{
        new_site_kVAR_demand,
        new_asset_pf_flag,
    };
}

void features::Constant_Power_Factor::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_Number) {
        power_factor_setpoint.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        lagging_limit.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        leading_limit.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
    } else if (msg_value.type == cJSON_True || msg_value.type == cJSON_False) {
        bool value_bool = msg_value.type != cJSON_False;
        absolute_mode.set_fims_bool(uri_endpoint.c_str(), value_bool);
        lagging_direction.set_fims_bool(uri_endpoint.c_str(), value_bool);
    }
}