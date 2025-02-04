/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Reactive_Setpoint.h>
#include <Site_Controller_Utils.h>

features::Reactive_Setpoint::Reactive_Setpoint() {
    feature_vars = { &kVAR_cmd, &kVAR_slew_rate };

    variable_ids = {
        { &enable_flag, "reactive_setpoint_mode_enable_flag" },
        { &kVAR_cmd, "reactive_setpoint_kVAR_cmd" },
        { &kVAR_slew_rate, "reactive_setpoint_kVAR_slew_rate" },
    };
}

void features::Reactive_Setpoint::execute(Asset_Cmd_Object& asset_cmd, bool& asset_pf_flag) {
    External_Inputs inputs{
        asset_cmd.total_potential_kVAR,
    };
    External_Outputs outputs = execute_helper(inputs);
    asset_cmd.site_kVAR_demand = outputs.site_kVAR_demand;
    asset_pf_flag = outputs.asset_pf_flag;
}

features::Reactive_Setpoint::External_Outputs features::Reactive_Setpoint::execute_helper(const External_Inputs& inputs) {
    // When in multiple inputs it can be annoying monitoring slew rates
    // While there is probably a more elegant solution simply checking at the start of 
    // each execution if it's different will suffice for now.
    // TODO(Jud): Do something smarter.
    if (kVAR_cmd_slew.get_slew_rate() != kVAR_slew_rate.value.value_int) {
        kVAR_cmd_slew.set_slew_rate(kVAR_slew_rate.value.value_int);
    }
    float kVAR_request = kVAR_cmd_slew.get_slew_target(kVAR_cmd.value.value_float);

    float new_site_kVAR_demand = std::min(inputs.total_potential_kVAR, fabsf(kVAR_request));
    new_site_kVAR_demand *= (std::signbit(kVAR_request)) ? -1 : 1;

    // this mode does not use power factor control
    bool new_asset_pf_flag = false;

    return External_Outputs{
        new_site_kVAR_demand,
        new_asset_pf_flag,
    };
}

void features::Reactive_Setpoint::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_Number) {
        kVAR_cmd.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        if (kVAR_slew_rate.set_fims_int(uri_endpoint.c_str(), range_check(msg_value.valueint, MAX_SLEW, 1))) {
            kVAR_cmd_slew.set_slew_rate(kVAR_slew_rate.value.value_int);
        }
    }
}
