/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/ESS_Calibration.h>

features::ESS_Calibration::ESS_Calibration() {
    feature_vars = {
        &kW_cmd, &soc_limits_enable, &min_soc_limit, &max_soc_limit, &voltage_limits_enable, &min_voltage_limit, &max_voltage_limit, &num_setpoint, &num_limited, &num_zero,
    };

    variable_ids = {
        { &enable_flag, "ess_calibration_enable_flag" },
        { &kW_cmd, "ess_calibration_kW_cmd" },
        { &soc_limits_enable, "ess_calibration_soc_limits_enable" },
        { &min_soc_limit, "ess_calibration_min_soc_limit" },
        { &max_soc_limit, "ess_calibration_max_soc_limit" },
        { &voltage_limits_enable, "ess_calibration_voltage_limits_enable" },
        { &min_voltage_limit, "ess_calibration_min_voltage_limit" },
        { &max_voltage_limit, "ess_calibration_max_voltage_limit" },
        { &num_setpoint, "ess_calibration_num_setpoint" },
        { &num_limited, "ess_calibration_num_limited" },
        { &num_zero, "ess_calibration_num_zero" },
    };
}

void features::ESS_Calibration::execute(Asset_Cmd_Object& asset_cmd, int num_ess_controllable) {
    External_Inputs inputs{
        num_ess_controllable,
        asset_cmd.ess_data.max_potential_kW,
        asset_cmd.ess_data.min_potential_kW,
    };
    External_Outputs outputs = execute_helper(inputs);
    asset_cmd.ess_data.max_potential_kW = outputs.ess_max_potential_kW;
    asset_cmd.ess_data.min_potential_kW = outputs.ess_min_potential_kW;
    asset_cmd.ess_data.kW_request = outputs.ess_kW_request;
}

features::ESS_Calibration::External_Outputs features::ESS_Calibration::execute_helper(const External_Inputs& inputs) {
    // Multiple by the number of ESS available to get the total power that should be distributed
    float total_feature_cmd = kW_cmd.value.value_float * inputs.num_ess_controllable;
    // Provide at least enough potential to meet the command, as the potentials can limit the command due to a shared slew target but unbalanced SoCs
    // Commands will still be limited safely by the amount of (dis)chargeable power available
    float new_ess_max_potential_kW = (std::max(total_feature_cmd, inputs.ess_max_potential_kW));
    float new_ess_min_potential_kW = (std::min(total_feature_cmd, inputs.ess_min_potential_kW));

    float new_ess_kW_request = total_feature_cmd;
    // This feature does not track load

    return External_Outputs{
        new_ess_max_potential_kW,
        new_ess_min_potential_kW,
        new_ess_kW_request,
    };
}

void features::ESS_Calibration::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_Number) {
        min_voltage_limit.set_fims_float(uri_endpoint.c_str(), fabsf(msg_value.valuedouble));
        max_voltage_limit.set_fims_float(uri_endpoint.c_str(), fabsf(msg_value.valuedouble));
        kW_cmd.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);

        // write if valid % range
        if (msg_value.valuedouble >= 0 && msg_value.valuedouble <= 100) {
            min_soc_limit.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
            max_soc_limit.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        }
    } else if (msg_value.type == cJSON_True || msg_value.type == cJSON_False) {
        bool value_bool = msg_value.type != cJSON_False;
        soc_limits_enable.set_fims_bool(uri_endpoint.c_str(), value_bool);
        voltage_limits_enable.set_fims_bool(uri_endpoint.c_str(), value_bool);
    }
}
