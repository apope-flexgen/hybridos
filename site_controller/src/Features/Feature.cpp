#include <Features/Feature.h>
#include <Site_Controller_Utils.h>

// add all variables associated with this feature, both status and controls
void Feature::add_feature_vars_to_JSON_buffer(fmt::memory_buffer& buf, const char* const var) {
    enable_flag.add_to_JSON_buffer(buf, var);
    for (auto it : feature_vars) {
        it->add_to_JSON_buffer(buf, var);
    }
}

// add only the variables associated with this feature that are desired to be displayed in summary
void Feature::add_summary_vars_to_JSON_buffer(fmt::memory_buffer& buf, const char* const var) {
    for (auto it : summary_vars)
        it->add_status_of_control_to_JSON_buffer(buf, var, false);
}

// set the ui_enabled field of all feature variables
void Feature::toggle_ui_enabled(bool flag) {
    for (auto it : feature_vars) {
        it->ui_enabled = flag;
    }
}

bool Feature::parse_json_config(cJSON* JSON_config, bool* primary_flag, Input_Source_List* inputs, const Fims_Object& field_defaults, std::vector<Fims_Object*>& multiple_inputs) {
    for (auto& variable_id_pair : variable_ids) {
        cJSON* JSON_variable = cJSON_GetObjectItem(JSON_config, variable_id_pair.second.c_str());
        if (JSON_variable == NULL) {
            FPS_ERROR_LOG("Could not find variable with ID %s in variables.json", variable_id_pair.second.c_str());
            return false;
        }
        if (!variable_id_pair.first->configure(variable_id_pair.second, primary_flag, inputs, JSON_variable, field_defaults, multiple_inputs)) {
            FPS_ERROR_LOG("Failed to parse variable with ID %s", variable_id_pair.second.c_str());
            return false;
        }
    }
    return true;
}
