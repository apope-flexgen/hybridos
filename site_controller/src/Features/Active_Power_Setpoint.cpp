/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Active_Power_Setpoint.h>
#include <Site_Controller_Utils.h>
#include <Slew_Object.h>

features::Active_Power_Setpoint::Active_Power_Setpoint() {
    feature_vars = std::vector<Fims_Object*>{ &enable_flag, &kW_cmd, &kW_slew_rate, &load_method, &absolute_mode_flag, &direction_flag, &maximize_solar_flag, &ess_charge_support_enable_flag };

    variable_ids = std::vector<std::pair<Fims_Object*, std::string>>{
        { &enable_flag, "active_power_setpoint_mode_enable_flag" },
        { &ess_charge_support_enable_flag, "ess_charge_support_enable_flag" },
        { &kW_cmd, "active_power_setpoint_kW_cmd" },
        { &load_method, "active_power_setpoint_load_method" },
        { &kW_slew_rate, "active_power_setpoint_kW_slew_rate" },
        { &absolute_mode_flag, "active_power_setpoint_absolute_mode_flag" },
        { &direction_flag, "active_power_setpoint_direction_flag" },
        { &maximize_solar_flag, "active_power_setpoint_maximize_solar_flag" },
    };
}

void features::Active_Power_Setpoint::execute(Asset_Cmd_Object& asset_cmd) {
    External_Inputs feature_inputs = External_Inputs{ asset_cmd.solar_data.max_potential_kW, asset_cmd.site_kW_load, asset_cmd.site_kW_demand, asset_cmd.ess_data.kW_request, asset_cmd.gen_data.kW_request, asset_cmd.solar_data.kW_request };

    External_Outputs feature_outputs = execute_helper(feature_inputs);
    asset_cmd.solar_data.kW_request = feature_outputs.solar_kW_request;
    asset_cmd.poi_cmd = feature_outputs.poi_cmd;
    asset_cmd.load_method = feature_outputs.asset_cmd_load_method;
    asset_cmd.additional_load_compensation = feature_outputs.additional_load_compensation;
    asset_cmd.site_kW_demand = feature_outputs.site_kW_demand;
}

features::Active_Power_Setpoint::External_Outputs features::Active_Power_Setpoint::execute_helper(const External_Inputs& inputs) {
    float local_kW_cmd = kW_cmd.value.value_float;
    if (absolute_mode_flag.value.value_bool) {
        local_kW_cmd = fabsf(local_kW_cmd) * (direction_flag.value.value_bool ? -1 : 1);
    }

    // Uncurtailed solar
    float new_solar_kW_request;
    if (maximize_solar_flag.value.value_bool) {
        new_solar_kW_request = inputs.solar_max_potential_kW;
    } else {
        new_solar_kW_request = inputs.solar_kW_request;
    }

    // Setup desired command at the POI
    float poi_cmd = local_kW_cmd;

    // Load must be included in the reference command and routed through the slewed target
    float additional_load_compensation = asset_cmd_utils::calculate_additional_load_compensation(load_compensation(load_method.value.value_int), inputs.site_kW_load, inputs.site_kW_demand, inputs.ess_kW_request, inputs.gen_kW_request,
                                                                                                 new_solar_kW_request);
    local_kW_cmd += additional_load_compensation;
    float site_kW_demand = kW_slew.get_slew_target(local_kW_cmd);
    additional_load_compensation = asset_cmd_utils::track_slewed_load(load_compensation(load_method.value.value_int), site_kW_demand, local_kW_cmd, additional_load_compensation, kW_slew);
    return External_Outputs{ new_solar_kW_request, poi_cmd, load_compensation(load_method.value.value_int), additional_load_compensation, site_kW_demand };
}

void features::Active_Power_Setpoint::charge_support_execute(Asset_Cmd_Object& asset_cmd) {
    Charge_Support_External_Inputs feature_inputs = Charge_Support_External_Inputs{
        asset_cmd.ess_data.kW_cmd, asset_cmd.ess_data.min_potential_kW, asset_cmd.gen_data.min_potential_kW, asset_cmd.solar_data.min_potential_kW, asset_cmd.site_kW_load,
    };

    Charge_Support_External_Outputs feature_outputs = charge_support_execute_helper(feature_inputs);
    asset_cmd.ess_data.kW_cmd = feature_outputs.ess_kW_cmd;
}

features::Active_Power_Setpoint::Charge_Support_External_Outputs features::Active_Power_Setpoint::charge_support_execute_helper(const Charge_Support_External_Inputs& feature_inputs) {
    // float temp_ess_kW = feature_inputs.ess_kW_cmd;  //used to calculate delta change
    // calculate the amount of power that is overloaded
    // TODO: change to solve load formula for feeder instead, then determine if beyond poi limits?
    float overload_kW = feature_inputs.gen_min_potential_kW + feature_inputs.solar_min_potential_kW + feature_inputs.ess_kW_cmd - zero_check(feature_inputs.site_kW_load - kW_cmd.value.value_float);

    // calculate unused ess charge kW
    float unused_ess_kW = feature_inputs.ess_kW_cmd - feature_inputs.ess_min_potential_kW;
    // if overloaded, charge ess as much as possible to support
    if (overload_kW > 0.0f && unused_ess_kW > 0.0f) {
        return Charge_Support_External_Outputs{ feature_inputs.ess_kW_cmd - std::min(overload_kW, unused_ess_kW) };
    } else {
        return Charge_Support_External_Outputs{ feature_inputs.ess_kW_cmd };
    }

    // FPS_DEBUG_LOG("OVERLOAD: %f, UNUSED: %f, DELTA: %f", overload_kW, unused_ess_kW, ess_data.kW_cmd - temp_ess_kW);
}

void features::Active_Power_Setpoint::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_Number) {
        kW_cmd.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        load_method.set_fims_int(uri_endpoint.c_str(), msg_value.valueint);

        // update slew rate internally if changed
        if (kW_slew_rate.set_fims_int(uri_endpoint.c_str(), range_check(msg_value.valueint, 100000000, 1)))
            kW_slew.set_slew_rate(kW_slew_rate.value.value_int);
    } else if (msg_value.type == cJSON_True || msg_value.type == cJSON_False) {
        bool value_bool = msg_value.type != cJSON_False;
        enable_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
        absolute_mode_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
        direction_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
        maximize_solar_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
        ess_charge_support_enable_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
    }
}