/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/AVR.h>

features::AVR::AVR() {
    feature_vars = {};  // initialized below by initialize_feature_vars()

    variable_ids = { { &enable_flag, "avr_enable_flag" },       { &over_deadband, "avr_over_deadband_volts" }, { &over_droop, "avr_over_droop_volts" }, { &over_rated_kVAR, "avr_over_rated_kVAR" }, { &under_deadband, "avr_under_deadband_volts" },
                     { &under_droop, "avr_under_droop_volts" }, { &under_rated_kVAR, "avr_under_rated_kVAR" }, { &voltage_cmd, "avr_cmd_volts" },       { &actual_volts, "avr_actual_volts" },       { &status_flag, "avr_status_flag" },
                     { &request, "avr_request_kVAR" } };
}

/**
 * @brief Parses a cJSON object for AVR's configuration data.
 * @param JSON_config cJSON object containing configuration data.
 * @param primary_flag Pointer to the site_controller primary mode flag.
 * @param inputs Pointer to the list of input sources for Multiple Input control variables.
 * @param field_defaults Reference to default values for JSON fields.
 * @param multiple_inputs Mutable list of Multiple Input Command Variables that may be appended to.
 * @returns Result detailing whether or not the config is valid
 */
Config_Validation_Result features::AVR::parse_json_config(cJSON* JSON_config, bool* primary_flag, Input_Source_List* inputs, const Fims_Object& field_defaults, std::vector<Fims_Object*>& multiple_inputs) {
    Config_Validation_Result result;
    result.is_valid_config = true;
    std::map<std::string, bool> undervoltage_variables_found = { { "avr_under_deadband_volts", false }, { "avr_under_droop_volts", false }, { "avr_under_rated_kVAR", false } };
    for (auto& variable_id_pair : variable_ids) {
        cJSON* JSON_variable = cJSON_GetObjectItem(JSON_config, variable_id_pair.second.c_str());
        if (JSON_variable == NULL) {
            if (undervoltage_variables_found.find(variable_id_pair.second) == undervoltage_variables_found.end()) {
                result.ERROR_details.push_back(Result_Details(fmt::format("Required variable \"{}\" is missing from variables.json configuration.", variable_id_pair.second)));
                result.is_valid_config = false;
                continue;
            }
        } else {
            // keep track of which undervoltage variables are found because if one is present, they all must be or else there is a config error
            if (undervoltage_variables_found.find(variable_id_pair.second) != undervoltage_variables_found.end()) {
                symmetric_variables = false;
                undervoltage_variables_found[variable_id_pair.second] = true;
            }
        }
        if (!variable_id_pair.first->configure(variable_id_pair.second, primary_flag, inputs, JSON_variable, field_defaults, multiple_inputs)) {
            result.ERROR_details.push_back(Result_Details(fmt::format("Failed to parse variable with ID \"{}\"", variable_id_pair.second)));
            result.is_valid_config = false;
        }
    }
    if (!symmetric_variables) {
        for (auto& variable : undervoltage_variables_found) {
            if (!variable.second) {
                result.ERROR_details.push_back(Result_Details(fmt::format("Required variable \"{}\" is missing from variables.json configuration.", variable.first)));
                result.is_valid_config = false;
            }
        }
    }
    return result;
}

/**
 * @brief Initializes feature variables based on whether or not undervoltage variables were found during config parsing
 */
void features::AVR::initialize_feature_vars() {
    if (symmetric_variables) {
        feature_vars = { &over_deadband, &over_droop, &over_rated_kVAR, &voltage_cmd, &actual_volts, &status_flag, &request };
    } else {
        feature_vars = { &over_deadband, &over_droop, &over_rated_kVAR, &under_deadband, &under_droop, &under_rated_kVAR, &voltage_cmd, &actual_volts, &status_flag, &request };
    }
}

void features::AVR::execute(Asset_Cmd_Object& asset_cmd, bool& asset_pf_flag) {
    External_Inputs inputs = External_Inputs{
        asset_cmd.total_potential_kVAR,
    };
    External_Outputs outputs = execute_helper(inputs);
    asset_cmd.site_kVAR_demand = outputs.site_kVAR_demand;
    asset_pf_flag = outputs.asset_pf_flag;
}

features::AVR::External_Outputs features::AVR::execute_helper(const External_Inputs& inputs) {
    // ensure all signs are correct before calculations
    over_deadband.value.set(fabsf(over_deadband.value.value_float));
    over_droop.value.set(fabsf(over_droop.value.value_float));
    over_rated_kVAR.value.set(fabsf(over_rated_kVAR.value.value_float));
    if (symmetric_variables) {
        under_deadband.value.set(over_deadband.value.value_float);
        under_droop.value.set(over_droop.value.value_float);
        under_rated_kVAR.value.set(over_rated_kVAR.value.value_float);
    } else {
        under_deadband.value.set(fabsf(under_deadband.value.value_float));
        under_droop.value.set(fabsf(under_droop.value.value_float));
        under_rated_kVAR.value.set(fabsf(under_rated_kVAR.value.value_float));
    }

    bool over_voltage = (actual_volts.value.value_float > voltage_cmd.value.value_float);

    // calculate reference_volts, the voltage at which AVR will begin to produce a response (nominal volts +/- deadband)
    float reference_volts = (over_voltage) ? voltage_cmd.value.value_float + over_deadband.value.value_float : voltage_cmd.value.value_float - under_deadband.value.value_float;

    // calculate delta_volts, how far actual voltage has deviated from reference_volts
    // negative in overvoltage event, positive in undervoltage event, 0 if actual volts is within deadband and no AVR response is needed (determines sign of response)
    float delta_volts;
    if (over_voltage) {
        delta_volts = (actual_volts.value.value_float > reference_volts) ? reference_volts - actual_volts.value.value_float : 0;
    } else {
        delta_volts = (actual_volts.value.value_float < reference_volts) ? reference_volts - actual_volts.value.value_float : 0;
    }

    // if there is an AVR response, set status flag true
    if (delta_volts != 0) {
        status_flag.value.set(true);
    } else {
        status_flag.value.set(false);
    }

    // total request is delta_volts scaled by droop and rated kVAR (if droop is 0, kVAR_request is set to 0)
    float kVAR_request = 0;
    if (over_voltage && over_droop.value.value_float != 0) {
        kVAR_request = (delta_volts / over_droop.value.value_float) * over_rated_kVAR.value.value_float;
    } else if (under_droop.value.value_float != 0) {
        kVAR_request = (delta_volts / under_droop.value.value_float) * under_rated_kVAR.value.value_float;
    }
    // cap request by rated power
    if (over_voltage) {
        kVAR_request = std::max(kVAR_request, -1.0f * over_rated_kVAR.value.value_float);
    } else {
        kVAR_request = std::min(kVAR_request, under_rated_kVAR.value.value_float);
    }
    request.value.set(kVAR_request);

    float new_site_kVAR_demand = std::min(inputs.total_potential_kVAR, fabsf(kVAR_request));
    new_site_kVAR_demand *= (std::signbit(kVAR_request)) ? -1 : 1;

    // this mode does not use power factor control
    bool new_asset_pf_flag = false;

    return External_Outputs{ new_site_kVAR_demand, new_asset_pf_flag };
}

void features::AVR::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_Number) {
        voltage_cmd.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        over_deadband.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        over_droop.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        over_rated_kVAR.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        if (!symmetric_variables) {
            under_deadband.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
            under_droop.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
            under_rated_kVAR.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        }
    }
}