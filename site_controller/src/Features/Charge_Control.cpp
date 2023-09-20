/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/Charge_Control.h>
#include <Site_Controller_Utils.h>

features::Charge_Control::Charge_Control() {
    feature_vars = {
        &kW_request, &target_soc, &kW_limit, &charge_disable, &discharge_disable,
    };

    variable_ids = {
        { &enable_flag, "_ess_charge_control_enable_flag" }, { &kW_request, "ess_charge_control_kW_request" },         { &kW_limit, "ess_charge_control_kW_limit" },
        { &target_soc, "ess_charge_control_target_soc" },    { &charge_disable, "ess_charge_control_charge_disable" }, { &discharge_disable, "ess_charge_control_discharge_disable" },
    };
}

void features::Charge_Control::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_Number) {
        // write if valid % range
        if (msg_value.valuedouble >= 0 && msg_value.valuedouble <= 100) {
            target_soc.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        }
        kW_limit.set_fims_float(uri_endpoint.c_str(), fabsf(msg_value.valuedouble));
    } else if (msg_value.type == cJSON_True || msg_value.type == cJSON_False) {
        bool value_bool = msg_value.type != cJSON_False;
        charge_disable.set_fims_bool(uri_endpoint.c_str(), value_bool);
        discharge_disable.set_fims_bool(uri_endpoint.c_str(), value_bool);
    }
}