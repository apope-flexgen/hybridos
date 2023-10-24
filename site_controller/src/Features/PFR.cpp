/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Features/PFR.h>
#include <Site_Controller_Utils.h>

features::PFR::PFR() {
    feature_vars = {};  // initialized below by initialize_feature_vars()

    summary_vars = { &status_flag };

    variable_ids = { { &enable_flag, "pfr_enable_flag" },    { &over_deadband, "pfr_over_deadband_hz" }, { &over_droop, "pfr_over_droop_hz" },        { &max_rated_kW, "pfr_max_rated_kW" }, { &under_deadband, "pfr_under_deadband_hz" },
                     { &under_droop, "pfr_under_droop_hz" }, { &min_rated_kW, "pfr_min_rated_kW" },      { &site_nominal_hz, "pfr_site_nominal_hz" }, { &status_flag, "pfr_status_flag" },   { &request, "pfr_request_kW" },
                     { &offset_hz, "pfr_offset_hz" } };
}

/**
 * @brief Parses a cJSON object for PFR's configuration data.
 * @param JSON_config cJSON object containing configuration data.
 * @param primary_flag Pointer to the site_controller primary mode flag.
 * @param inputs Pointer to the list of input sources for Multiple Input control variables.
 * @param field_defaults Reference to default values for JSON fields.
 * @param multiple_inputs Mutable list of Multiple Input Command Variables that may be appended to.
 * @returns Result detailing whether or not the config is valid
 */
Config_Validation_Result features::PFR::parse_json_config(cJSON* JSON_config, bool* primary_flag, Input_Source_List* inputs, const Fims_Object& field_defaults, std::vector<Fims_Object*>& multiple_inputs) {
    Config_Validation_Result result;
    result.is_valid_config = true;
    std::map<std::string, bool> underfrequency_variables_found = { { "pfr_under_deadband_hz", false }, { "pfr_under_droop_hz", false }, { "pfr_min_rated_kW", false } };
    for (auto& variable_id_pair : variable_ids) {
        cJSON* JSON_variable = cJSON_GetObjectItem(JSON_config, variable_id_pair.second.c_str());
        if (JSON_variable == NULL) {
            if (underfrequency_variables_found.find(variable_id_pair.second) == underfrequency_variables_found.end()) {
                result.ERROR_details.push_back(Result_Details(fmt::format("Required variable \"{}\" is missing from variables.json configuration.", variable_id_pair.second)));
                result.is_valid_config = false;
                continue;
            }
        } else {
            // keep track of which underfrequency variables are found because if one is present, they all must be or else there is a config error
            if (underfrequency_variables_found.find(variable_id_pair.second) != underfrequency_variables_found.end()) {
                symmetric_variables = false;
                underfrequency_variables_found[variable_id_pair.second] = true;
            }
        }
        if (!variable_id_pair.first->configure(variable_id_pair.second, primary_flag, inputs, JSON_variable, field_defaults, multiple_inputs)) {
            result.ERROR_details.push_back(Result_Details(fmt::format("Failed to parse variable with ID \"{}\"", variable_id_pair.second)));
            result.is_valid_config = false;
        }
    }
    if (!symmetric_variables) {
        for (auto& variable : underfrequency_variables_found) {
            if (!variable.second) {
                result.ERROR_details.push_back(Result_Details(fmt::format("Required variable \"{}\" is missing from variables.json configuration.", variable.first)));
                result.is_valid_config = false;
            }
        }
    }
    return result;
}

/**
 * @brief Initializes feature variables based on whether or not underfrequency variables were found during config parsing
 */
void features::PFR::initialize_feature_vars() {
    if (symmetric_variables) {
        feature_vars = { &over_deadband, &over_droop, &max_rated_kW, &site_nominal_hz, &status_flag, &request, &offset_hz };
    } else {
        feature_vars = { &over_deadband, &over_droop, &max_rated_kW, &under_deadband, &under_droop, &min_rated_kW, &site_nominal_hz, &status_flag, &request, &offset_hz };
    }
}

void features::PFR::execute(Asset_Cmd_Object& asset_cmd, float site_frequency, float& total_site_kW_charge_limit, float& total_site_kW_discharge_limit) {
    External_Inputs inputs{
        site_frequency,
        asset_cmd.site_kW_demand,
        total_site_kW_charge_limit,
        total_site_kW_discharge_limit,
    };
    External_Outputs outputs = execute_helper(inputs);
    asset_cmd.site_kW_demand = outputs.site_kW_demand;
    total_site_kW_charge_limit = outputs.total_site_kW_charge_limit;
    total_site_kW_discharge_limit = outputs.total_site_kW_discharge_limit;
}

features::PFR::External_Outputs features::PFR::execute_helper(const External_Inputs& inputs) {
    // ensure all signs are correct before calculations
    over_deadband.value.set(fabsf(over_deadband.value.value_float));
    over_droop.value.set(fabsf(over_droop.value.value_float));
    max_rated_kW.value.set(fabsf(max_rated_kW.value.value_float));
    if (symmetric_variables) {
        under_deadband.value.set(over_deadband.value.value_float);
        under_droop.value.set(over_droop.value.value_float);
        min_rated_kW.value.set(-1 * max_rated_kW.value.value_float);
    } else {
        under_deadband.value.set(fabsf(under_deadband.value.value_float));
        under_droop.value.set(fabsf(under_droop.value.value_float));
        min_rated_kW.value.set(-1 * fabsf(min_rated_kW.value.value_float));
    }

    // include offset test input in case it is non-zero
    float input_hz = inputs.site_frequency + offset_hz.value.value_float;

    bool over_hz = (input_hz > site_nominal_hz.value.value_float);

    // calculate reference_hz, the frequency at which PFR will begin to produce a response (nominal frequency +/- deadband)
    float reference_hz = (over_hz) ? site_nominal_hz.value.value_float + over_deadband.value.value_float : site_nominal_hz.value.value_float - under_deadband.value.value_float;

    // calculate delta_hz, how far actual frequency has deviated from reference_hz
    // positive in overfrequency event, negative in underfrequency event, 0 if actual frequency is within deadband and no PFR response is needed
    float delta_hz;
    if (over_hz) {
        delta_hz = (input_hz > reference_hz) ? input_hz - reference_hz : 0;
    } else {
        delta_hz = (input_hz < reference_hz) ? input_hz - reference_hz : 0;
    }

    // no PFR or invalid droop
    if (delta_hz == 0 || (over_hz && over_droop.value.value_float <= 0) || (!over_hz && under_droop.value.value_float <= 0)) {
        status_flag.value.value_bool = false;
        request.value.set(0);
        return External_Outputs{ inputs.site_kW_demand, inputs.total_site_kW_charge_limit, inputs.total_site_kW_discharge_limit };
    }

    // if PFR event is detected, set status true
    status_flag.value.value_bool = true;

    // total request is delta_hz scaled by droop and rated kW
    float kW_request;
    if (over_hz) {
        kW_request = delta_hz / over_droop.value.value_float;
    } else {
        kW_request = delta_hz / under_droop.value.value_float;
    }
    // rated kW to be used depends on direction of original site kW demand
    float rated_kW = (inputs.site_kW_demand < 0) ? fabsf(min_rated_kW.value.value_float) : max_rated_kW.value.value_float;
    kW_request = -1 * kW_request * rated_kW;
    request.value.set(kW_request);

    // calculate charge/discharge limits from min/max rated power
    float chg_kW_limit = less_than_zero_check(min_rated_kW.value.value_float);
    float dischg_kW_limit = zero_check(max_rated_kW.value.value_float);
    // get the direction of the original command, or use sign of response if 0
    float base_cmd = (inputs.site_kW_demand != 0) ? inputs.site_kW_demand : kW_request;
    // charge direction power limit (cannot change direction)
    if (base_cmd > 0) {
        chg_kW_limit = 0;
        // discharge direction power limit
    } else {
        dischg_kW_limit = 0;
    }

    // bound modified site demand by charge/discharge limits
    float calculated_response = range_check(inputs.site_kW_demand + kW_request, dischg_kW_limit, chg_kW_limit);

    // modify request and demand
    float new_site_kW_demand = calculated_response;
    float new_total_site_kW_charge_limit = (range_check(inputs.total_site_kW_charge_limit + kW_request, 0.0f, min_rated_kW.value.value_float));
    float new_total_site_kW_discharge_limit = (range_check(inputs.total_site_kW_discharge_limit + kW_request, max_rated_kW.value.value_float, 0.0f));

    return External_Outputs{
        new_site_kW_demand,
        new_total_site_kW_charge_limit,
        new_total_site_kW_discharge_limit,
    };
}

void features::PFR::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    if (msg_value.type == cJSON_Number) {
        over_deadband.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        over_droop.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        max_rated_kW.set_fims_float(uri_endpoint.c_str(), fabsf(msg_value.valuedouble));
        offset_hz.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
        if (!symmetric_variables) {
            under_deadband.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
            under_droop.set_fims_float(uri_endpoint.c_str(), msg_value.valuedouble);
            min_rated_kW.set_fims_float(uri_endpoint.c_str(), -1 * fabsf(msg_value.valuedouble));
        }
    } else if (msg_value.type == cJSON_True || msg_value.type == cJSON_False) {
        bool value_bool = msg_value.type != cJSON_False;
        if (available) {
            enable_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
        }
    }
}
