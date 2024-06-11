/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Manual.h>
#include <Site_Controller_Utils.h>

features::Manual::Manual() {
    feature_vars = {
        &manual_solar_kW_cmd, &manual_ess_kW_cmd, &manual_gen_kW_cmd, &manual_solar_kW_slew_rate, &manual_ess_kW_slew_rate, &manual_gen_kW_slew_rate,
    };

    variable_ids = {
        { &enable_flag, "manual_mode_enable_flag" },
        { &manual_solar_kW_cmd, "manual_solar_kW_cmd" },
        { &manual_ess_kW_cmd, "manual_ess_kW_cmd" },
        { &manual_gen_kW_cmd, "manual_gen_kW_cmd" },
        { &manual_solar_kW_slew_rate, "manual_solar_kW_slew_rate" },
        { &manual_ess_kW_slew_rate, "manual_ess_kW_slew_rate" },
        { &manual_gen_kW_slew_rate, "manual_gen_kW_slew_rate" },
    };
}

void features::Manual::execute(Asset_Cmd_Object& asset_cmd) {
    External_Inputs inputs{
        asset_cmd.ess_data.max_potential_kW,
        asset_cmd.solar_data.max_potential_kW,
        asset_cmd.gen_data.max_potential_kW,
    };
    External_Outputs outputs = execute_helper(inputs);
    asset_cmd.ess_data.kW_request = outputs.ess_kW_request;
    asset_cmd.ess_data.max_potential_kW = outputs.ess_max_potential_kW;
    asset_cmd.solar_data.kW_request = outputs.solar_kW_request;
    asset_cmd.solar_data.max_potential_kW = outputs.solar_max_potential_kW;
    asset_cmd.gen_data.kW_request = outputs.gen_kW_request;
    asset_cmd.gen_data.max_potential_kW = outputs.gen_max_potential_kW;
}

features::Manual::External_Outputs features::Manual::execute_helper(const External_Inputs& inputs) {
    // When in multiple inputs it can be annoying monitoring slew rates
    // While there is probably a more elegant solution simply checking at the start of 
    // each execution if it's different will suffice for now.
    // TODO(Jud): Do something smarter.
    if (manual_solar_kW_slew.get_slew_rate() != manual_solar_kW_slew_rate.value.value_int) {
        manual_solar_kW_slew.set_slew_rate(manual_solar_kW_slew_rate.value.value_int);
    }
    if (manual_ess_kW_slew.get_slew_rate() != manual_ess_kW_slew_rate.value.value_int) {
        manual_ess_kW_slew.set_slew_rate(manual_ess_kW_slew_rate.value.value_int);
    }
    if (manual_gen_kW_slew.get_slew_rate() != manual_gen_kW_slew_rate.value.value_int) {
        manual_gen_kW_slew.set_slew_rate(manual_gen_kW_slew_rate.value.value_int);
    }

    // Assign ess request, passing off limits check to dispatch
    float new_ess_kW_request = manual_ess_kW_slew.get_slew_target(manual_ess_kW_cmd.value.value_float);
    // Ensure that any extra available ESS is not used to service the site demand
    float new_ess_max_potential_kW = std::min(inputs.ess_max_potential_kW, zero_check(new_ess_kW_request));

    // Assign positive solar request, passing off limits check to dispatch
    float new_solar_kW_request = manual_solar_kW_slew.get_slew_target(zero_check(manual_solar_kW_cmd.value.value_float));
    // Ensure that any extra available solar is not used to service the site demand
    float new_solar_max_potential_kW = std::min(inputs.solar_max_potential_kW, new_solar_kW_request);

    // Assign positive generator request, passing off limits check to dispatch
    float new_gen_kW_request = manual_gen_kW_slew.get_slew_target(zero_check(manual_gen_kW_cmd.value.value_float));
    // Ensure that any extra available generator power is not used to service the site demand
    float new_gen_max_potential_kW = std::min(inputs.gen_max_potential_kW, new_gen_kW_request);

    // No load compensation
    return External_Outputs{
        new_ess_kW_request, new_ess_max_potential_kW, new_solar_kW_request, new_solar_max_potential_kW, new_gen_kW_request, new_gen_max_potential_kW,
    };
}

void features::Manual::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_Number) {
        // Manual Mode
        manual_solar_kW_cmd.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        manual_ess_kW_cmd.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        manual_gen_kW_cmd.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        // Update slew rates
        if (manual_solar_kW_slew_rate.set_fims_int(uri_endpoint.c_str(), range_check(msg_value.valueint, MAX_SLEW, 1))) {
            manual_solar_kW_slew.set_slew_rate(manual_solar_kW_slew_rate.value.value_int);
        }
        if (manual_ess_kW_slew_rate.set_fims_int(uri_endpoint.c_str(), range_check(msg_value.valueint, MAX_SLEW, 1))) {
            manual_ess_kW_slew.set_slew_rate(manual_ess_kW_slew_rate.value.value_int);
        }
        if (manual_gen_kW_slew_rate.set_fims_int(uri_endpoint.c_str(), range_check(msg_value.valueint, MAX_SLEW, 1))) {
            manual_gen_kW_slew.set_slew_rate(manual_gen_kW_slew_rate.value.value_int);
        }
    }
}
