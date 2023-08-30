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
#include <Frequency_Response.h>
#include <Site_Controller_Utils.h>

/**
 * @brief Parses a cJSON object for the Frequency Response feature's configuration data.
 * @param JSON_config cJSON object containing configuration data.
 * @param p_flag Pointer to the site_controller primary mode flag.
 * @param inputs Pointer to the list of input sources for Multiple Input control variables.
 * @param defaults Reference to default
 * @param multiple_inputs Mutable list of Multiple Input Command Variables that may need to be added to
 * if any of Frequency Response's inputs are configured to be Multiple Input Command Variables.
 * @returns True if parsing is successful or false if parsing failed.
 */
bool Frequency_Response::parse_json_config(cJSON* JSON_config, bool* p_flag, Input_Source_List* inputs, const Fims_Object& defaults, std::vector<Fims_Object*>& multiple_inputs) {
    // parse high-level variables from variables.json
    for (auto& variable_id_pair : variable_ids) {
        cJSON* JSON_variable = cJSON_GetObjectItem(JSON_config, variable_id_pair.second.c_str());
        if (JSON_variable == NULL) {
            FPS_ERROR_LOG("Could not find variable %s in frequency_response object in variables.json.", variable_id_pair.second.c_str());
            return false;
        }
        if (!variable_id_pair.first->configure(variable_id_pair.second, p_flag, inputs, JSON_variable, defaults, multiple_inputs)) {
            FPS_ERROR_LOG("Failed to configure frequency response variable %s.", variable_id_pair.second.c_str());
            return false;
        }
    }

    // extract response components array
    cJSON* JSON_fr_components = cJSON_GetObjectItem(JSON_config, "components");
    if (JSON_fr_components == NULL) {
        FPS_ERROR_LOG("Could not find components in frequency_response object of variables.json.");
        return false;
    }
    if (!cJSON_IsArray(JSON_fr_components)) {
        FPS_ERROR_LOG("Parsed components object in frequency_response is not an array.");
        return false;
    }
    FPS_INFO_LOG("Found %d response components in frequency response components array.", cJSON_GetArraySize(JSON_fr_components));

    // parse each frequency response component and build response components list
    int i = 0;
    cJSON* JSON_fr_component;
    cJSON_ArrayForEach(JSON_fr_component, JSON_fr_components) {
        auto new_source = std::make_shared<Freq_Resp_Component>();
        if (!new_source->parse_json_config(JSON_fr_component, p_flag, inputs, defaults, multiple_inputs)) {
            FPS_ERROR_LOG("Failed to parse frequency response component with index %d.", i);
            return false;
        }
        if (!new_source->initialize_state(target_freq_hz.value.value_float)) {
            FPS_ERROR_LOG("Could not initialize state for frequency response component with index %d.", i);
            return false;
        }
        response_components.push_back(new_source);
        i++;
    }
    return true;
}

/**
 * @brief Handles FIMS SETs to URIs belonging to the Frequency Response active power feature.
 * @param msg The FIMS message containing the SET.
 */
void Frequency_Response::handle_fims_set(const fims_message& msg) {
    // extra target endpoint and value
    std::string uri_endpoint = msg.pfrags[msg.nfrags - 1];
    cJSON* JSON_body = cJSON_Parse(msg.body);
    if (JSON_body == NULL) {
        FPS_ERROR_LOG("FIMS message body is NULL or incorrectly formatted: (%s)", JSON_body->valuestring);
        return;
    }
    defer { cJSON_Delete(JSON_body); };

    cJSON* body_value = cJSON_GetObjectItem(JSON_body, "value");
    float body_float = (body_value) ? body_value->valuedouble : JSON_body->valuedouble;
    int body_int = (body_value) ? body_value->valueint : JSON_body->valueint;

    // check if target endpoint matches any Frequency Response high-level endpoints and handle the SET if so
    if (enable_mask.set_fims_int(uri_endpoint.c_str(), body_int))
        return;
    if (baseload_cmd_kw.set_fims_float(uri_endpoint.c_str(), body_float))
        return;
    if (freq_offset_hz.set_fims_float(uri_endpoint.c_str(), body_float))
        return;

    // check if target endpoint is prefixed with any response component IDs. if so, pass it to that response component for it to handle the SET
    // take the longest match in the case of two matches e.g. uf_ffr and uf_ffrs
    size_t longest_match = 0;
    std::shared_ptr<Freq_Resp_Component> matched_comp = NULL;
    for (auto resp_comp : response_components) {
        if (strncmp(uri_endpoint.c_str(), resp_comp->component_id.value.value_string, strlen(resp_comp->component_id.value.value_string)) == 0) {
            if (strlen(resp_comp->component_id.value.value_string) > longest_match) {
                longest_match = strlen(resp_comp->component_id.value.value_string);
                matched_comp = resp_comp;
            }
        }
    }
    // If a frequency endpoint was matched, check its component variables
    if (matched_comp)
        matched_comp->handle_fims_set(JSON_body, uri_endpoint.c_str());
}

/**
 * @brief Calculates the outputs of each frequency response component and adds the baseload command
 * in order to get a full frequency response command.
 * @param input_settings Struct containing necessary inputs for Frequency Response algorithm. See
 * definition of Frequency_Response_Inputs for details.
 * @returns Struct containing the output requests of the Frequency Response algorithm.
 * See definition of Frequency_Response_Outputs for details.
 */
Frequency_Response_Outputs Frequency_Response::aggregate_response_components(Frequency_Response_Inputs input_settings) {
    Frequency_Response_Outputs aggregate_values{
        baseload_cmd_kw.value.value_float,
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
 * @brief Loads the given vector with pointers to all Frequency Response parameters that
 * an external interface should know about.
 * @param var_list List of variable pointers that will be published periodically.
 */
void Frequency_Response::get_feature_vars(std::vector<Fims_Object*>& var_list) {
    var_list.push_back(&target_freq_hz);
    var_list.push_back(&freq_offset_hz);
    var_list.push_back(&enable_mask);
    var_list.push_back(&baseload_cmd_kw);
    var_list.push_back(&total_output_kw);

    for (auto curr_freq_resp_comp : response_components) {
        curr_freq_resp_comp->get_feature_vars(var_list);
    }
}

/**
 * @brief Loads the given vector with pointers to only the Frequency Response parameters
 * that have been requested by Commissioning/Integration/etc. teams to be displayed in
 * Site Controller's features summary.
 * @param var_list List of variable pointers that will be included in the features summary
 * object.
 */
void Frequency_Response::get_summary_vars(std::vector<Fims_Object*>& var_list) {
    var_list.push_back(&baseload_cmd_kw);
}
