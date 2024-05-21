/*
 * Input_Source.cpp
 *
 * Created on: Feb 10, 2022
 */

/* C Standard Library Dependencies */
#include <cstring>
/* C++ Standard Library Dependencies */
#include <stdexcept>
#include <string>
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
/* Local Internal Dependencies */
#include <Input_Sources.h>
#include <Site_Controller_Utils.h>

/**
 * @brief Construct a new Input_Source::Input_Source object.
 * Initialize enabled to the given value, which is false by default if not passed by caller.
 *
 */
Input_Source::Input_Source(bool init_enabled) {
    enabled = init_enabled;
}

/**
 * @brief Parses an input source's configuration JSON into the Input_Source object.
 *
 * @param JSON_input_source Pointer to the cJSON object containing the configuration data.
 * @throws runtime_error if parsing fails.
 */
Config_Validation_Result Input_Source::parse_json_obj(cJSON* JSON_input_source) {
    // caller must not pass a NULL cJSON*
    if (JSON_input_source == NULL) {
        return Config_Validation_Result(false, "JSON_input_source is NULL");
    }

    // parse name string
    cJSON* JSON_name = cJSON_GetObjectItem(JSON_input_source, "name");
    if (JSON_name == NULL || JSON_name->valuestring == NULL) {
        return Config_Validation_Result(false, "failed to parse name string from element of input_sources array");
    }
    name = JSON_name->valuestring;

    // parse uri_suffix string
    cJSON* JSON_uri_suffix = cJSON_GetObjectItem(JSON_input_source, "uri_suffix");
    if (JSON_uri_suffix == NULL || JSON_uri_suffix->valuestring == NULL) {
        return Config_Validation_Result(false, "failed to parse uri_suffix string from element " + name + " of input_sources array");
    }
    uri_suffix = JSON_uri_suffix->valuestring;

    // don't allow "brokenly long" uris
    if (name.length() > MAX_VARIABLE_LENGTH) {
        return Config_Validation_Result(false, "name is absurdly long shorten the variable. name is currently: " + name);
    }
    if (uri_suffix.length() > MAX_VARIABLE_LENGTH) {
        return Config_Validation_Result(false, "uri_suffix is absurdly long shorten the variable. uri_suffix is currently: " + uri_suffix);
    }

    // parse ui_type
    // NOTE: use var_ui_type here because the "ui_type" is the ui_type for the control not the actual variable
    cJSON* JSON_default_ui_type = cJSON_GetObjectItem(JSON_input_source, "var_ui_type");
    if (JSON_default_ui_type == NULL || JSON_default_ui_type->valuestring == NULL) {
        return Config_Validation_Result(false, "failed to parse default ui_type from element " + name + " of input_sources array. NOTE: This is the variable \"var_ui_type\".");
    }
    if (strcmp(JSON_default_ui_type->valuestring, "status") == 0) {
        ui_type = STATUS;
    } else if (strcmp(JSON_default_ui_type->valuestring, "control") == 0) {
        ui_type = CONTROL;
    } else if (strcmp(JSON_default_ui_type->valuestring, "none") == 0) {
        ui_type = NONE;
    } else {
        return Config_Validation_Result(false, "parsed ui_type as " + std::string(JSON_default_ui_type->valuestring) + " which is not one of the possible ui_types status, control, or none");
    }

    // parse ui_type
    cJSON* JSON_default_control_ui_type = cJSON_GetObjectItem(JSON_input_source, "ui_type");
    if (JSON_default_control_ui_type == NULL || JSON_default_control_ui_type->valuestring == NULL) {
        return Config_Validation_Result(false, "failed to parse default control_ui_type from element " + name + " of input_sources array");
    }
    if (strcmp(JSON_default_control_ui_type->valuestring, "status") == 0) {
        control_ui_type = STATUS;
    } else if (strcmp(JSON_default_control_ui_type->valuestring, "control") == 0) {
        control_ui_type = CONTROL;
    } else if (strcmp(JSON_default_control_ui_type->valuestring, "none") == 0) {
        control_ui_type = NONE;
    } else {
        return Config_Validation_Result(false, "parsed control_ui_type as " + std::string(JSON_default_control_ui_type->valuestring) + " which is not one of the possible ui_types status, control, or none");
    }

    // parse alt ui types if there are any (optional)
    cJSON* JSON_alt_ui_types = cJSON_GetObjectItem(JSON_input_source, "alt_ui_types");
    if (JSON_alt_ui_types != NULL) {
        cJSON* JSON_array_object = NULL;
        cJSON_ArrayForEach(JSON_array_object, JSON_alt_ui_types) {
            // parse the ID of the variable that wants an alternative UI type for this input source
            cJSON* JSON_var_id = cJSON_GetObjectItem(JSON_array_object, "var_id");
            if (JSON_var_id == NULL || JSON_var_id->valuestring == NULL) {
                return Config_Validation_Result(false, "failed to parse var_id from an entry in the alt_ui_types array for input source " + name);
            }

            // parse the UI type that the variable wants to use instead for this input source
            cJSON* JSON_alt_ui_type = cJSON_GetObjectItem(JSON_array_object, "ui_type");
            if (JSON_alt_ui_type == NULL || JSON_alt_ui_type->valuestring == NULL) {
                return Config_Validation_Result(false, "failed to parse ui_type from an entry in the alt_ui_types array for input source " + name);
            }

            // alternative UI type can only be status or none
            UI_Type alt_ui_type;
            if (strcmp(JSON_alt_ui_type->valuestring, "status") == 0) {
                alt_ui_type = STATUS;
            } else if (strcmp(JSON_alt_ui_type->valuestring, "none") == 0) {
                alt_ui_type = NONE;
            } else {
                return Config_Validation_Result(false, "alt_ui_type entry for input source " + name + " has ui_type " + std::string(JSON_alt_ui_type->valuestring) + "but only status or none are allowed");
            }

            // add the ID-type pair to the alt_ui_types map
            alt_ui_types[JSON_var_id->valuestring] = alt_ui_type;
        }
    }

    // parse initial value of enabled boolean
    cJSON* JSON_enabled = cJSON_GetObjectItem(JSON_input_source, "enabled");
    if (JSON_enabled == NULL) {
        enabled = false;  // default to false if not found
    } else {
        if (JSON_enabled->type > cJSON_True) {
            return Config_Validation_Result(false, "value of 'enabled' for input_source " + name + " is not a boolean");
        }
        enabled = JSON_enabled->type == cJSON_True;
    }
    return Config_Validation_Result(true);
}

/**
 * @brief Returns the UI type of the variable's input register for this input source.
 * Will either be the default UI type of this input source or something else if the variable is found in the alt_ui_types map.
 *
 * @param var_id ID of the variable.
 * @return std::string UI type of the variable's input register for this input source.
 */
std::string Input_Source::get_ui_type_of_var(std::string var_id) {
    // check the alt_ui_type map for the variable. if not found, then use the default ui_type for this input source
    try {
        UI_Type alt_ui_type = alt_ui_types.at(var_id);
        switch (alt_ui_type) {
            case STATUS:
                return "status";
            case NONE:
                return "none";
            default:
                throw std::runtime_error("alt_ui_type of variable " + var_id + " for input source " + name + "is an invalid value. May only be STATUS or NONE.");
        }
    } catch (...) {
        switch (ui_type) {
            case STATUS:
                return "status";
            case CONTROL:
                return "control";
            case NONE:
                return "none";
            default:  // this case should be prevented from happening by the configuration code
                FPS_ERROR_LOG("Variable %s has an invalid ui_type for the input register %s!\n", var_id.c_str(), name.c_str());
                return "none";
        }
    }
}

/**
 * @brief Parses the JSON array of input source configuration data into the input_sources vector.
 *
 * @param JSON_input_sources Pointer to the cJSON array of input sources configuration data.
 * @returns A result describing whether or not the config is valid
 */
Config_Validation_Result Input_Source_List::parse_json_obj(cJSON* JSON_input_sources) {
    if (cJSON_IsArray(JSON_input_sources) == 0) {
        return Config_Validation_Result(false, "parsed input_sources object is not an array");
    }

    if (cJSON_GetArraySize(JSON_input_sources) == 0) {
        // allow an empty array to indicate no input sources list
        return Config_Validation_Result(true);
    }

    cJSON* JSON_input_source = NULL;
    cJSON_ArrayForEach(JSON_input_source, JSON_input_sources) {
        // instantiate new Input_Source object
        auto new_source = std::make_shared<Input_Source>();

        // parse JSON config
        new_source->parse_json_obj(JSON_input_source);

        // verify uri_suffix is not a duplicate
        for (auto existing_source : input_sources) {
            if (existing_source->uri_suffix == new_source->uri_suffix) {
                return Config_Validation_Result(false, "input source " + existing_source->name + " and input source " + new_source->name + " have the same uri_suffix " + new_source->uri_suffix);
            }
        }

        // add to list of input sources
        input_sources.push_back(new_source);
    }

    // the last input source in the array must start and stay enabled
    input_sources.back()->enabled = true;

    // create the map of input source IDs to input source vector indices. this will avoid having to brute force search through the vector later.
    // while iterating over newly configured input sources, also initialize selected_input_source_index variable.
    selected_input_source_index = input_sources.size() - 1;
    for (uint i = 0; i < input_sources.size(); ++i) {
        id_to_index[input_sources[i]->uri_suffix] = i;
        if (input_sources[i]->enabled && i < selected_input_source_index) {
            selected_input_source_index = i;
        }
    }

    return Config_Validation_Result(true);
}

/**
 * @brief Adds one item per input source to the given object, where the key of the item is the input source's URI suffix and the value of the item is the enabled boolean.
 *
 * @param JSON_object JSON object to which the input source's objects should be added.
 */
void Input_Source_List::add_to_JSON_buffer(fmt::memory_buffer& buf, const char* const var) {
    for (auto input_source : input_sources) {
        bufJSON_AddBoolCheckVar(buf, input_source->uri_suffix.c_str(), input_source->enabled, var);
    }
}

/**
 * @brief Sets one of the input sources in the list to be enabled/disabled.
 *
 * @param source_id ID / uri_suffix of the input source that is to be enabled/disabled.
 * @param enable_flag True for enabling; false for disabling.
 * @return std::string Name of the selected input source after the effect of the enabling/disabling (could have changed or remained the same).
 */
std::string Input_Source_List::set_source_enable_flag(std::string source_id, bool enable_flag) {
    // do not attempt to process SET to /site/input_sources/<input source id> if input_source array is empty
    if (input_sources.empty()) {
        FPS_ERROR_LOG("Site Manager received a FIMS SET to /site/input_sources/%s but input_sources array is empty! \n", source_id.c_str());
        return "";
    }

    // check if identified input source exists
    uint source_index;
    try {
        source_index = id_to_index.at(source_id);
    } catch (...) {
        FPS_ERROR_LOG("Site Manager received set to enable/disable input source %s but that source does not exist!\n", source_id.c_str());
        return input_sources[selected_input_source_index]->name;
    }

    // update the input source's enabled flag. last input source in list (lowest priority) can never be disabled
    if (source_index != input_sources.size() - 1) {
        input_sources[source_index]->enabled = enable_flag;
    }

    // update the selected_input_source_index variable
    if (enable_flag && source_index < selected_input_source_index) {
        // if an input source that is higher priority than the currently selected input source is being enabled, it is the new selected source
        selected_input_source_index = source_index;
    } else if (!enable_flag && source_index == selected_input_source_index) {
        // if the currently selected input source is being disabled, need to search for new selected input source
        for (uint i = 0; i < input_sources.size(); ++i) {
            if (input_sources[i]->enabled) {
                selected_input_source_index = i;
                break;
            }
        }
    }
    // else, this enable_flag change does not change the overall selected input source

    return input_sources[selected_input_source_index]->name;
}

/**
 * @return The index of the selected input source.
 */
uint Input_Source_List::get_selected_input_source_index() {
    return selected_input_source_index;
}

/**
 * @brief Accesses the URI suffix of a given input source.
 *
 * @param i Index of the input source to be queried.
 * @return std::string URI suffix of the queried input source.
 */
std::string Input_Source_List::get_uri_suffix_of_input(uint i) {
    return input_sources[i]->uri_suffix;
}

/**
 * @brief Accesses the name of a given input source.
 *
 * @param i Index of the input source to be queried.
 * @return std::string Name of the queried input source.
 */
std::string Input_Source_List::get_name_of_input(uint i) {
    return input_sources[i]->name;
}

/**
 * @brief Accesses the specified input source and queries it for what ui_type should be used for the given variable.
 *
 * @param source_index Index of the input source to be queried.
 * @param var_id ID of the variable that is asking for the UI type.
 * @return std::string UI type for the variable's specified input register.
 */
std::string Input_Source_List::get_ui_type_of_input(uint source_index, std::string var_id) {
    return input_sources[source_index]->get_ui_type_of_var(var_id);
}

size_t Input_Source_List::get_num_sources() {
    return input_sources.size();
}
