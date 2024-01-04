/*
 * Frequency_Response.cpp
 *
 * Created: Summer 2022
 *
 */
/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
#include <fims/fps_utils.h>
#include <fims/defer.hpp>
/* Local Internal Dependencies */
#include <Features/Frequency_Response.h>
#include <Site_Controller_Utils.h>

features::Frequency_Response::Frequency_Response() {
    // feature_vars and summary_vars are set after configuration is parsed

    // Note: Frequency Response is a special case because all of these variables are actually sub-variables
    // of a top-level frequency_response variable, rather than being top-level variables themselves
    variable_ids = {
        // inputs
        { &enable_mask, "fr_enable_mask" },
        { &target_freq_hz, "fr_target_freq_hz" },
        { &freq_offset_hz, "fr_freq_offset_hz" },
        // outputs
        { &enable_flag, "fr_mode_enable_flag" },
        { &total_output_kw, "fr_total_output_kw" },
    };
}

/**
 * @brief Parses a cJSON object for the Frequency Response feature's configuration data.
 * @param JSON_config cJSON object containing configuration data.
 * @param p_flag Pointer to the site_controller primary mode flag.
 * @param inputs Pointer to the list of input sources for Multiple Input control variables.
 * @param defaults Reference to default
 * @param multiple_inputs Mutable list of Multiple Input Command Variables that may need to be added to
 * if any of Frequency Response's inputs are configured to be Multiple Input Command Variables.
 * @returns Result detailing whether or not the config is valid
 */
Config_Validation_Result features::Frequency_Response::parse_json_config(cJSON* JSON_config, bool* p_flag, Input_Source_List* inputs, const Fims_Object& defaults, std::vector<Fims_Object*>& multiple_inputs) {
    Config_Validation_Result result;

    cJSON* JSON_frequency_response = cJSON_GetObjectItem(JSON_config, "frequency_response");
    if (JSON_frequency_response == NULL) {
        result.ERROR_details.push_back(Result_Details(fmt::format("Required variable \"{}\" is missing from variables.json configuration.", "frequency_response")));
        result.is_valid_config = false;
        return result;
    }

    // parse high-level variables from variables.json
    for (auto& variable_id_pair : variable_ids) {
        cJSON* JSON_variable = NULL;
        // the enable flag is a top-level variable, all others are members of the frequency response object
        JSON_variable = variable_id_pair.first == &enable_flag ? cJSON_GetObjectItem(JSON_config, variable_id_pair.second.c_str()) : cJSON_GetObjectItem(JSON_frequency_response, variable_id_pair.second.c_str());

        if (JSON_variable == NULL) {
            result.ERROR_details.push_back(Result_Details(fmt::format("Could not find variable \"{}\" in frequency_response object in variables.json.", variable_id_pair.second.c_str())));
            result.is_valid_config = false;
            return result;
        }
        if (!variable_id_pair.first->configure(variable_id_pair.second, p_flag, inputs, JSON_variable, defaults, multiple_inputs)) {
            result.ERROR_details.push_back(Result_Details(fmt::format("Failed to configure frequency response variable \"{}\".", variable_id_pair.second.c_str())));
            result.is_valid_config = false;
            return result;
        }
    }

    // extract response components array
    cJSON* JSON_fr_components = cJSON_GetObjectItem(JSON_frequency_response, "components");
    if (JSON_fr_components == NULL) {
        result.ERROR_details.push_back(Result_Details("Could not find components in frequency_response object of variables.json."));
        result.is_valid_config = false;
        return result;
    }
    if (!cJSON_IsArray(JSON_fr_components)) {
        result.ERROR_details.push_back(Result_Details("Parsed components object in frequency_response is not an array."));
        result.is_valid_config = false;
        return result;
    }
    FPS_INFO_LOG("Found %d response components in frequency response components array.", cJSON_GetArraySize(JSON_fr_components));

    // parse each frequency response component and build response components list
    int i = 0;
    cJSON* JSON_fr_component;
    cJSON_ArrayForEach(JSON_fr_component, JSON_fr_components) {
        auto new_source = std::make_shared<Freq_Resp_Component>();
        if (!new_source->parse_json_config(JSON_fr_component, p_flag, inputs, defaults, multiple_inputs)) {
            FPS_ERROR_LOG("Failed to parse frequency response component with index %d.", i);
            result.is_valid_config = false;
            return result;
        }
        if (!new_source->initialize_state(target_freq_hz.value.value_float)) {
            FPS_ERROR_LOG("Could not initialize state for frequency response component with index %d.", i);
            result.is_valid_config = false;
            return result;
        }
        response_components.push_back(new_source);
        i++;
    }
    result.is_valid_config = true;
    return result;
}

/**
 * @brief Returns a list of only the top-level frequency response variable ids
 */
std::vector<std::string> features::Frequency_Response::get_variable_ids_list() const {
    return { "frequency_response", "fr_mode_enable_flag" };
}

/**
 * @brief Loads the given vector with pointers to all Frequency Response parameters that
 * an external interface should know about.
 * @param var_list List of variable pointers that will be published periodically.
 */
void features::Frequency_Response::get_feature_vars(std::vector<Fims_Object*>& var_list) {
    var_list.push_back(&target_freq_hz);
    var_list.push_back(&freq_offset_hz);
    var_list.push_back(&enable_mask);
    var_list.push_back(&total_output_kw);

    for (auto curr_freq_resp_comp : response_components) {
        curr_freq_resp_comp->get_feature_vars(var_list);
    }
}

void features::Frequency_Response::execute(Asset_Cmd_Object& asset_cmd, float ess_total_rated_active_power, float site_frequency, timespec current_time) {
    // get inputs
    Frequency_Response_Inputs ins{ asset_cmd.site_kW_demand, asset_cmd.ess_data.max_potential_kW, asset_cmd.ess_data.min_potential_kW, ess_total_rated_active_power, site_frequency, current_time };
    // call frequency response algorithm
    Frequency_Response_Outputs outs = aggregate_response_components(ins);
    // set outputs
    asset_cmd.ess_data.max_potential_kW = outs.ess_max_potential;
    asset_cmd.ess_data.min_potential_kW = outs.ess_min_potential;
    asset_cmd.site_kW_demand = outs.output_kw;
}

/**
 * @brief Calculates the outputs of each frequency response component and adds the baseload command
 * in order to get a full frequency response command.
 * @param input_settings Struct containing necessary inputs for Frequency Response algorithm. See
 * definition of Frequency_Response_Inputs for details.
 * @returns Struct containing the output requests of the Frequency Response algorithm.
 * See definition of Frequency_Response_Outputs for details.
 */
Frequency_Response_Outputs features::Frequency_Response::aggregate_response_components(Frequency_Response_Inputs input_settings) {
    Frequency_Response_Outputs aggregate_values{
        input_settings.site_input_kW_demand,
        input_settings.ess_max_potential,
        input_settings.ess_min_potential,
    };

    input_settings.site_frequency += freq_offset_hz.value.value_float;

    int i = 0;
    for (auto curr_freq_obj : response_components) {
        // if the component is enabled, add its output to the aggregate outputs
        if (enable_mask.value.value_int & (1 << i)) {
            Frequency_Response_Outputs fr_values_to_add = curr_freq_obj->frequency_response(input_settings);

            if (fr_values_to_add.ess_max_potential != input_settings.ess_max_potential) {
                aggregate_values.ess_max_potential = fr_values_to_add.ess_max_potential;
            }
            if (fr_values_to_add.ess_min_potential != input_settings.ess_min_potential) {
                aggregate_values.ess_min_potential = fr_values_to_add.ess_min_potential;
            }
            aggregate_values.output_kw += fr_values_to_add.output_kw;
        }
        // if the component is disabled, clear its output so it does not report stale data
        else {
            curr_freq_obj->clear_outputs();
        }
        ++i;
    }

    total_output_kw.value.set(aggregate_values.output_kw);
    return aggregate_values;
}

/**
 * @brief Handles FIMS SETs to URIs belonging to the Frequency Response active power feature.
 * @param uri_endpoint The endpoint parsed from the uri.
 * @param msg_value The new value given by the FIMS message SET.
 */
void features::Frequency_Response::handle_fims_set(std::string uri_endpoint, const cJSON& msg_value) {
    // check if target endpoint matches any Frequency Response high-level endpoints and handle the SET if so
    if (enable_mask.set_fims_int(uri_endpoint.c_str(), msg_value.valueint))
        return;
    if (freq_offset_hz.set_fims_float(uri_endpoint.c_str(), msg_value.valueint))
        return;

    // check if target endpoint is prefixed with any response component IDs. if so, pass it to that response component for it to handle the SET
    // take the longest match in the case of two matches e.g. uf_ffr and uf_ffrs
    size_t longest_match = 0;
    std::shared_ptr<Freq_Resp_Component> matched_comp = NULL;
    for (auto resp_comp : response_components) {
        if (strncmp(uri_endpoint.c_str(), resp_comp->component_id.value.value_string.c_str(), resp_comp->component_id.value.value_string.length()) == 0) {
            if (resp_comp->component_id.value.value_string.length() > longest_match) {
                longest_match = resp_comp->component_id.value.value_string.length();
                matched_comp = resp_comp;
            }
        }
    }
    // If a frequency endpoint was matched, check its component variables
    if (matched_comp)
        matched_comp->handle_fims_set(&msg_value, uri_endpoint.c_str());

    if (msg_value.type == cJSON_True || msg_value.type == cJSON_False) {
        bool value_bool = msg_value.type != cJSON_False;
        if (available) {
            enable_flag.set_fims_bool(uri_endpoint.c_str(), value_bool);
        }
    }
}
