/*
 * Fims_Object.cpp
 *
 *  Created on: Nov 5, 2019
 *      Author: kbrezina
 */

/* C Standard Library Dependencies */
#include <cstring>
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
/* Local Internal Dependencies */
#include <Site_Manager.h>
#include <Fims_Object.h>
#include <Site_Controller_Utils.h>

extern fims* p_fims;

Fims_Object::Fims_Object() {
    scaler = 1;
    ui_enabled = true;
    num_options = 0;
    multiple_inputs = false;
    // Default status values published externally. These values are not used internally and can be set to anything
    default_status_name = "";
    default_status_value = -1;

    send_FIMS_buf = fmt::memory_buffer();
}

// Copy constructor. Excludes the fims memory buffer
Fims_Object::Fims_Object(const Fims_Object& other) {
    options_name = other.options_name;
    options_value = other.options_value;
    default_status_value = other.default_status_value;
    default_status_name = other.default_status_name;
    scaler = other.scaler;
    num_options = other.num_options;
    ui_enabled = other.ui_enabled;
    write_uri = other.write_uri;
    is_primary = other.is_primary;
    multiple_inputs = other.multiple_inputs;
    inputs = other.inputs;
    input_source_settings = other.input_source_settings;
    component_control_value = other.component_control_value;
    value = other.value;
    variable_id = other.variable_id;
    register_id = other.register_id;
    component_uri = other.component_uri;
    name = other.name;
    unit = other.unit;
    ui_type = other.ui_type;
    type = other.type;
}

Fims_Object& Fims_Object::operator=(Fims_Object other) {
    swap(*this, other);
    return *this;
}

void Fims_Object::set_component_uri(const char* uri) {
    component_uri = uri;
}

void Fims_Object::set_register_id(const char* id) {
    register_id = id;
}

void Fims_Object::set_variable_id(const char* id) {
    variable_id = id;
}

void Fims_Object::set_variable_id(const std::string& id) {
    variable_id = id;
}

void Fims_Object::set_name(const char* _name) {
    name = _name;
}

void Fims_Object::set_unit(const char* _unit) {
    unit = _unit;
}

void Fims_Object::set_ui_type(const char* _ui_type) {
    ui_type = _ui_type;
}

void Fims_Object::set_value_type(valueType value_type) {
    value.type = value_type;
    component_control_value.type = value_type;
}

void Fims_Object::set_type(const char* _type) {
    type = _type;
}

/**
 * If the uri endpoint given matches this object's variable id, this function will update the value of the object with the
 * float value received. Will set both numeric registers, value_int and value_float. If this object has multiple inputs,
 * will set the value of the associated input instead.
 * @param uri_endpoint variable id of the input to be set
 * @param body_float float value to set
 * @return whether the value was set
 */
bool Fims_Object::set_fims_float(const char* uri_endpoint, float body_float) {
    if (multiple_inputs) {
        // if the endpoint does not match this variable's ID, the edit is meant for another variable
        // strncmp used here since a correct endpoint, in this case, would include a URI suffix
        if (strncmp(uri_endpoint, variable_id.c_str(), variable_id.length()) != 0) {
            return false;
        }
        try {
            uint input_source_index = parse_input_source_index(uri_endpoint);
            inputs[input_source_index].set(body_float);
            return true;
        } catch (const std::exception& e) {
            FPS_ERROR_LOG("Failed to process target URI of SET to Multiple Inputs variable %s: %s.\n", variable_id.c_str(), e.what());
            return false;
        }
    } else {
        if (variable_id.compare(uri_endpoint) == 0) {
            value.set(body_float);
            return true;
        }
        return false;
    }
}

/**
 * If the uri endpoint given matches this object's variable id, this function will update the value of the object with the
 * int value received. Will set both numeric registers, value_float and value_int. If this object has multiple inputs,
 * will set the value of the associated input instead.
 * @param uri_endpoint variable id of the input to be set
 * @param body_int integer value to set
 * @return whether the value was set
 */
bool Fims_Object::set_fims_int(const char* uri_endpoint, int body_int) {
    if (multiple_inputs) {
        // if the endpoint does not match this variable's ID, the edit is meant for another variable
        // strncmp used here since a correct endpoint, in this case, would include a URI suffix
        if (strncmp(uri_endpoint, variable_id.c_str(), variable_id.length()) != 0) {
            return false;
        }
        try {
            uint input_source_index = parse_input_source_index(uri_endpoint);
            inputs[input_source_index].set(body_int);
            return true;
        } catch (const std::exception& e) {
            FPS_ERROR_LOG("Failed to process target URI of SET to Multiple Inputs variable %s: %s.\n", variable_id.c_str(), e.what());
            return false;
        }
    } else {
        if (variable_id.compare(uri_endpoint) == 0) {
            value.set(body_int);
            return true;
        }
        return false;
    }
}

bool Fims_Object::set_fims_bool(const char* uri_endpoint, bool body_bool) {
    if (multiple_inputs) {
        // if the endpoint does not match this variable's ID, the edit is meant for another variable
        // strncmp used here since a correct endpoint, in this case, would include a URI suffix
        if (strncmp(uri_endpoint, variable_id.c_str(), variable_id.length()) != 0) {
            return false;
        }
        try {
            uint input_source_index = parse_input_source_index(uri_endpoint);
            inputs[input_source_index].set(body_bool);
            return true;
        } catch (const std::exception& e) {
            FPS_ERROR_LOG("Failed to process target URI of SET to Multiple Inputs variable %s: %s.\n", variable_id.c_str(), e.what());
            return false;
        }
    } else {
        if (variable_id.compare(uri_endpoint) == 0) {
            value.set(body_bool);
            return true;
        }
        return false;
    }
}

bool Fims_Object::set_fims_bit_field(const char* uri_endpoint, uint64_t body_bit_field) {
    if (variable_id.compare(uri_endpoint) == 0) {
        value.set(body_bit_field);
        return true;
    }
    return false;
}

bool Fims_Object::set_fims_masked_int(const char* uri_endpoint, int body_int, uint64_t mask) {
    if (mask & get_int_from_bit_position(body_int))  // check int against mask
    {
        if (multiple_inputs) {
            // if the endpoint does not match this variable's ID, the edit is meant for another variable
            // strncmp used here since a correct endpoint, in this case, would include a URI suffix
            if (strncmp(uri_endpoint, variable_id.c_str(), variable_id.length()) != 0) {
                return false;
            }
            try {
                uint input_source_index = parse_input_source_index(uri_endpoint);
                inputs[input_source_index].set(body_int);
                return true;
            } catch (const std::exception& e) {
                FPS_ERROR_LOG("Failed to process target URI of SET to Multiple Inputs variable %s: %s.\n", variable_id.c_str(), e.what());
                return false;
            }
        } else {
            if (variable_id.compare(uri_endpoint) == 0) {
                value.set(body_int);
                return true;
            }
            return false;
        }
    }
    return false;
}

// This function is used to clear alarms and faults represented in Fims_Objects
void Fims_Object::clear_fims_bit_field(void) {
    value.value_bit_field = 0;
    num_options = 0;
    options_name.clear();
    options_value.clear();
}

const char* Fims_Object::get_component_uri() const {
    if (component_uri.empty())
        return NULL;
    return component_uri.c_str();
}

const char* Fims_Object::get_register_id() const {
    if (register_id.empty())
        return NULL;
    return register_id.c_str();
}

const char* Fims_Object::get_variable_id() const {
    if (variable_id.empty())
        return NULL;
    return variable_id.c_str();
}

const char* Fims_Object::get_name() const {
    if (name.empty())
        return NULL;
    return name.c_str();
}

const char* Fims_Object::get_unit() const {
    return unit.c_str();
}

const char* Fims_Object::get_ui_type() const {
    if (ui_type.empty())
        return NULL;
    return ui_type.c_str();
}

const char* Fims_Object::get_type() const {
    if (type.empty())
        return NULL;
    return type.c_str();
}

int Fims_Object::get_scaler() const {
    return scaler;
}

/**
 * Generates and sends a set containing the appropriate value on the uri configured
 * @param use_custom_uri Optional parameter defaults to false. If true, try to send to the configured write_uri instead of the component_uri
 * @param round_float Optional parameter defaults to false. If true, round the float value of the variable to the nearest integer when sending,
 *                    but preserve the original value
 */
bool Fims_Object::send_to_component(bool use_write_uri, bool round_float) {
    send_FIMS_buf.clear();  // Clear the buffer for use
    // Disable publish if secondary controller (shadow mode)
    if (!*is_primary)
        return true;

    // if no comp uri, this is probably not a controllable variable for this asset. check assets.json
    if (!use_write_uri && component_uri.empty())
        return false;

    bufJSON_StartObject(send_FIMS_buf);  // rootObject {

    // If typical component write, try to add the component control value, otherwise try to add the value itself
    if ((!use_write_uri && !component_control_value.add_value_to_JSON_buffer(send_FIMS_buf, round_float)) || (use_write_uri && !value.add_value_to_JSON_buffer(send_FIMS_buf, round_float))) {
        FPS_ERROR_LOG("Fims_Object::send_to_component - error adding value to JSON buffer\n");
        return false;
    }

    bufJSON_EndObject(send_FIMS_buf);  // } rootObject
    bufJSON_RemoveTrailingComma(send_FIMS_buf);
    std::string body = to_string(send_FIMS_buf);
    if (body.empty()) {
        FPS_ERROR_LOG("Fims_Object::send_to_component - JSON body is empty\n");
        return false;
    }

    // Send to the component uri as usual
    if (!use_write_uri)
        p_fims->Send("set", component_uri.c_str(), NULL, body.c_str());
    // Send to the custom write_uri configured
    else if (!write_uri.empty())
        p_fims->Send("set", write_uri.c_str(), NULL, body.c_str());
    else
        FPS_ERROR_LOG("Fims_Object::send_to_component - write uri undefined\n");

    return true;
}

/**
 * @brief Loads the value field with the value of the input found at the given index of the inputs vector.
 * Only used by Multiple-Input Command Variables.
 *
 * @param input_source_index Index of the input from which the present value should be sourced.
 */
void Fims_Object::update_present_register(uint input_source_index) {
    if (input_source_index < inputs.size()) {
        value.set(inputs[input_source_index]);
    } else {
        FPS_ERROR_LOG("Tried to set Multiple-Input Command Variable %s with an input index %u that is not in the bounds of the configured inputs (size = %d). \n", variable_id.c_str(), input_source_index, (int)inputs.size());
    }
}

/**
 * Return a string representing the status for appropriate Fims_Objects (status, breaker_status).
 * If an invalid register is requested, an empty string will be returned
 */
const char* Fims_Object::get_status_string() const {
    if (value.type == Bool && variable_id.compare("breaker_status") == 0) {
        return value.value_bool ? "Closed" : "Open";
    } else if (type.compare("Status") == 0) {
        // Status with valid options array
        if (!options_name.empty() && !options_name[value.value_bit_field].empty()) {
            // Add the string representing the Bit_Field value
            return options_name[value.value_bit_field].c_str();
        }
        // Status with empty options array (error state, possible missing publish from components)
        else {
            // Invalid publish received, add the default value instead
            return default_status_name.c_str();
        }
    }
    return "";
}

/**
 * @brief Constructs the JSON representation of this Fims_Object and adds it to a buffer.
 * Acts as a helper function for other functions that add this object to a buffer, i.e. add_to_JSON_buffer()
 * and add_status_of_control_to_JSON_buffer()
 *
 * @param buf Reference to a JSON formatted string buffer to which we will add this object.
 * @param control2status Whether this is a control object that should be represented as a status.
 * @param clothed If the object is clothed, a new object with the "value" key is constructed and added. Otherwise the value is added with the variable_id as its key.
 */
void Fims_Object::build_JSON_Object(fmt::memory_buffer& buf, bool control2status, bool clothed, const char* const search_id) {
    // handle GETs for component control commands
    if (has_suffix(search_id, COMPONENT_CONTROL_SUFFIX)) {
        if (clothed) {
            bufJSON_AddId(buf, "value");
        }
        component_control_value.add_naked_value_to_JSON_buffer(buf);
        return;
    }

    // The key/id of the JSON item being produced
    std::string item_name;
    if (clothed) {
        // Start a new object with the variable id
        bufJSON_AddId(buf, variable_id);
        bufJSON_StartObject(buf);  // JSON_object {
        // Use "value" as the name for the item created e.g. "value": 100
        item_name = "value";
    } else {
        // Use the object passed rather than creating a subobject and adding it
        // Use the variable_id as the JSON id e.g. "active_power_setpoint": 100
        item_name = variable_id;
    }

    // specific variables have component control commands that are desired to be included in the FIMS interface for the variable.
    // Example: active_power_setpoint. when active_power_setpoint is a clothed variable, the component control command is an
    // attribute that should be wrapped in the active_power_setpoint object. when active_power_setpoint is a naked variable, the
    // component control command should NOT be wrapped in the active_power_setpoint object, and should instead exist as a
    // separate variable active_power_setpoint_component_ctrl_cmd that is included in the larger asset instance object alongside
    // active_power_setpoint. the same logic applies to all variables that include their component control command in their
    // FIMS interface
    if (clothed || search_id == NULL) {
        const char* ids_of_vars_with_component_control_commands[] = {
            "active_power_setpoint",
            "reactive_power_setpoint",
        };
        for (const char* id : ids_of_vars_with_component_control_commands) {
            if (variable_id.compare(id) != 0)
                continue;
            bufJSON_AddId(buf, variable_id + COMPONENT_CONTROL_SUFFIX);
            component_control_value.add_naked_value_to_JSON_buffer(buf);
            bufJSON_AddComma(buf);
            break;
        }
    }

    // The following conditional adds the key/value object, used for both naked and clothed
    if (value.type == Float) {
        // Floats rounded correctly in the UI
        bufJSON_AddNumber(buf, item_name.c_str(), value.value_float);
    } else if (value.type == Int) {
        // UI expects value > 0 if alarms/faults are present
        // In the case that alarms/faults are masked, num_options will still be nonzero
        if (ui_type.compare("alarm") == 0 || ui_type.compare("fault") == 0) {
            // Alarms/Faults have a configured type of Int but use their Bit_Field value
            // TODO: safe to change all configuration to simply Bit_Field?
            bufJSON_AddNumber(buf, item_name.c_str(), value.value_bit_field);
        } else {
            bufJSON_AddNumber(buf, item_name.c_str(), value.value_int);
        }
    } else if (value.type == Bool) {
        if (variable_id.compare("breaker_status") == 0) {
            bufJSON_AddString(buf, item_name.c_str(), get_status_string());
        } else {
            bufJSON_AddBool(buf, item_name.c_str(), value.value_bool);
        }
    } else if (value.type == String) {
        bufJSON_AddString(buf, item_name.c_str(), value.value_string.c_str());
    } else if (value.type == Bit_Field) {
        // Type Bit_Field should currently be unused, as alarms/faults use Int and Status has it's own type now
        // TODO: leave for future support
        if (!options_name.empty() && !options_name[value.value_bit_field].empty()) {
            // Add the string representing the Bit_Field value, will only work for a single value
            bufJSON_AddString(buf, item_name.c_str(), options_name[value.value_bit_field].c_str());
        }
    }
    // Only type, not value.type should be Status
    else if (type.compare("Status") == 0) {
        bufJSON_AddString(buf, item_name.c_str(), get_status_string());
    }

    // If not clothed, do not need to add auxiliary data
    if (!clothed) {
        return;
    }
    // Since clothed, add the auxiliary data
    // Alarm and faults will always be clothed
    bufJSON_AddString(buf, "name", name.c_str());
    bufJSON_AddString(buf, "unit", unit.c_str());
    bufJSON_AddNumber(buf, "scaler", scaler);
    bufJSON_AddBool(buf, "enabled", ui_enabled);
    bufJSON_AddString(buf, "type", type.c_str());
    if (control2status && ui_type.compare("control") == 0) {
        bufJSON_AddString(buf, "ui_type", "status");
    } else {
        bufJSON_AddString(buf, "ui_type", ui_type.c_str());
    }

    // Add any options arrays
    bufJSON_AddId(buf, "options");
    bufJSON_StartArray(buf);  // JSON_array [
    // Only add the high bit alarms/faults
    if (ui_type.compare("alarm") == 0 || ui_type.compare("fault") == 0) {
        // Check each bit individually
        for (uint64_t bit_value = 1, pos = 0; pos < MAX_STATUS_BITS; bit_value <<= 1, pos++) {
            // Ensure valid name and value (use value type to ensure options_value initialization)
            if (value.value_bit_field & bit_value && !options_name[pos].empty() && options_value[pos].type != Invalid) {
                bufJSON_StartObject(buf);  // JSON_options {
                bufJSON_AddString(buf, "name", options_name[pos].c_str());
                if (options_value[pos].type == Float)
                    bufJSON_AddNumber(buf, "return_value", options_value[pos].value_float);
                else if (options_value[pos].type == Int)
                    bufJSON_AddNumber(buf, "return_value", options_value[pos].value_int);
                else if (options_value[pos].type == Bool)
                    bufJSON_AddBool(buf, "return_value", options_value[pos].value_bool);
                else if (options_value[pos].type == String)
                    bufJSON_AddString(buf, "return_value", options_value[pos].value_string.c_str());
                else if (options_value[pos].type == Bit_Field)
                    bufJSON_AddNumber(buf, "return_value", options_value[pos].value_bit_field);
                bufJSON_EndObject(buf);  // } JSON_options
            }
        }
    } else {
        for (size_t i = 0; i < options_name.size(); i++) {
            if (!options_name[i].empty() && options_value[i].type != Invalid) {
                bufJSON_StartObject(buf);  // JSON_options {
                bufJSON_AddString(buf, "name", options_name[i].c_str());
                if (options_value[i].type == Float)
                    bufJSON_AddNumber(buf, "return_value", options_value[i].value_float);
                else if (options_value[i].type == Int)
                    bufJSON_AddNumber(buf, "return_value", options_value[i].value_int);
                else if (options_value[i].type == Bool)
                    bufJSON_AddBool(buf, "return_value", options_value[i].value_bool);
                else if (options_value[i].type == String)
                    bufJSON_AddString(buf, "return_value", options_value[i].value_string.c_str());
                else if (options_value[i].type == Bit_Field)
                    bufJSON_AddNumber(buf, "return_value", options_value[i].value_bit_field);
                bufJSON_EndObject(buf);  // } JSON_options
            }
        }
    }
    // Status cannot have empty options array, publish the default string value pair instead
    if (type.compare("Status") == 0) {
        // Create the options object
        bufJSON_StartObject(buf);  // JSON_options {
        bufJSON_AddString(buf, "name", default_status_name.c_str());
        bufJSON_AddNumber(buf, "return_value", default_status_value);
        // Add the options object to the array and the array to the published object
        bufJSON_EndObject(buf);  // } JSON_options
    }
    // Finish the clothed object because there is an early return for unclothed objects.
    bufJSON_EndArray(buf);  // ] JSON_array
    // If clothed, end the object we started
    bufJSON_EndObject(buf);  // } JSON_object
}

/**
 * @brief Adds a JSON representation of this Fims_Object to the buffer passed as a parameter.
 * @param buf The buffer to which this Fims_Object will be added.
 * @param clothed Whether the JSON representation should be clothed (true) or naked (false).
 * @param search_id If not null, must match the variable ID for the variable to be added to the buffer.
 */
void Fims_Object::add_to_JSON_buffer(fmt::memory_buffer& buf, const char* const search_id, bool clothed) {
    // handle GETs for component control command values
    if (has_suffix(search_id, COMPONENT_CONTROL_SUFFIX)) {
        if (strncmp(variable_id.c_str(), search_id, strlen(variable_id.c_str()))) {
            FPS_INFO_LOG("compare != 0");
            return;
        }
        build_JSON_Object(buf, false, clothed, search_id);
        return;
    }

    bool var_matches = search_id != NULL && variable_id.compare(search_id) == 0;

    // add present value to object
    if (search_id == NULL || var_matches) {
        // if alarm or fault and has options treat as clothed.
        if ((ui_type.compare("fault") == 0 || ui_type.compare("alarm") == 0) && num_options > 0)
            build_JSON_Object(buf, false, true);  // Needs to be treated as clothed to include options
        else
            build_JSON_Object(buf, false, clothed, search_id);

        // if this variable was specifically requested (as opposed to adding it to a larger requested object), remove the ID wrapping
        if (var_matches) {
            // remove the variable id from the buffer
            std::string prefix = '"' + variable_id + '"' + ':';
            size_t prefix_size = prefix.size();
            if (to_string(buf).substr(0, prefix_size) != prefix) {
                FPS_ERROR_LOG("Fims_Object::add_to_JSON_buffer - error removing variable id for request of specific value\n");
                return;
            }
            // skip variable ID using substring
            bufJSON_RemoveTrailingComma(buf);
            std::string value = to_string(buf).substr(prefix_size);
            buf.clear();
            FORMAT_TO_BUF(buf, "{}", value);
            return;
        }
    }

    // if this is a Multiple Inputs variable, need to add all input registers as well
    if (multiple_inputs) {
        // since the build_JSON_Object method will use the current value/ui_type fields to construct a JSON object,
        // an input register's JSON object can be built by overwritting the current value/ui_type fields, calling build_JSON_Object,
        // then restoring the actual value/ui_type fields

        // save present settings
        std::string real_name = name;
        std::string real_ui_type = ui_type;
        Value_Object real_value = value;
        bool real_ui_enabled = ui_enabled;
        std::string real_variable_id = variable_id;

        // iterate through each input register and add to object
        for (uint i = 0; i < inputs.size(); ++i) {
            name = real_name + ": " + input_source_settings->get_name_of_input(i);
            ui_type = input_source_settings->get_ui_type_of_input(i, real_variable_id);
            value.set(inputs[i]);
            ui_enabled = real_ui_enabled && i == input_source_settings->get_selected_input_source_index();

            std::string uri_suffix = input_source_settings->get_uri_suffix_of_input(i);
            variable_id = real_variable_id + "_" + uri_suffix;

            var_matches = search_id != NULL && variable_id.compare(search_id) == 0;

            // add value to object
            if (search_id == NULL || var_matches) {
                // Add the id and value
                build_JSON_Object(buf, false, clothed);

                // Extract the value without the id if requested
                if (var_matches) {
                    // Remove the variable id from the buffer
                    size_t prefix_size = ("\"" + variable_id + "\":").size();
                    if (to_string(buf).substr(0, prefix_size) != "\"" + variable_id + "\":") {
                        FPS_ERROR_LOG("Fims_Object::add_to_JSON_buffer - error removing variable id for request of specific value\n");
                        return;
                    }
                    std::string value = to_string(buf).substr(prefix_size);
                    buf.clear();
                    FORMAT_TO_BUF(buf, "{}", value);
                    break;  // Don't return immediately, go to settings restore
                }
            }
        }

        // restore present settings
        name = real_name;
        ui_type = real_ui_type;
        value.set(real_value);
        ui_enabled = real_ui_enabled;
        variable_id = real_variable_id;
    }
}

/**
 * @brief Adds a JSON representation of a control object as a status object to a buffer, typically for use in summary variables
 *
 * @param buf The buffer to which this Fims_Object will be added
 * @param clothed Whether the JSON should be clothed (true) or naked (false)
 *                If clothed, this Fims_Object's id is added to the buffer just before the JSON object
 * @param var If not null, then a particular variable's value is desired and the control object will only be added if its id matches the requested var
 */
void Fims_Object::add_status_of_control_to_JSON_buffer(fmt::memory_buffer& buf, const char* const var, bool clothed) {
    bool var_matches = var != NULL && strcmp(variable_id.c_str(), var) == 0;

    if (var == NULL || var_matches) {
        if (clothed)
            bufJSON_AddId(buf, variable_id.c_str());

        build_JSON_Object(buf, true, clothed);
    }

    // Extract the value without the id if requested
    if (var_matches) {
        // Remove the variable id from the buffer
        size_t prefix_size = ("\"" + variable_id + "\":").size();
        if (to_string(buf).substr(0, prefix_size) != "\"" + variable_id + "\":") {
            FPS_ERROR_LOG("Fims_Object::add_status_of_control_to_JSON_buffer - error removing variable id for request of specific value\n");
            return;
        }
        std::string value = to_string(buf).substr(prefix_size);
        buf.clear();
        FORMAT_TO_BUF(buf, "{}", value);
        return;
    }
}

/**
 * @brief Parses the Fims_Object's config data from a JSON object.
 *
 * @param JSON_variable The JSON representation of the variable's config data.
 * @param default_vals A Fims_Object template to pull optional field values from when they are not found in the parsed JSON object.
 * @throw std::runtime_error if parsing fails.
 */
void Fims_Object::parse_json_config(cJSON* JSON_variable, const Fims_Object& default_vals) {
    // parse name string
    cJSON* JSON_name = cJSON_GetObjectItem(JSON_variable, "name");
    set_name((JSON_name == NULL || JSON_name->valuestring == NULL) ? default_vals.get_name() : JSON_name->valuestring);

    // parst unit string
    cJSON* JSON_unit = cJSON_GetObjectItem(JSON_variable, "unit");
    set_unit((JSON_unit == NULL || JSON_unit->valuestring == NULL) ? default_vals.get_unit() : JSON_unit->valuestring);

    // parse UI type string
    cJSON* JSON_ui_type = cJSON_GetObjectItem(JSON_variable, "ui_type");
    set_ui_type((JSON_ui_type == NULL || JSON_ui_type->valuestring == NULL) ? default_vals.get_ui_type() : JSON_ui_type->valuestring);

    // parse type string
    cJSON* JSON_type = cJSON_GetObjectItem(JSON_variable, "type");
    set_type((JSON_type == NULL || JSON_type->valuestring == NULL) ? default_vals.get_type() : JSON_type->valuestring);

    // parse scaler integer
    cJSON* JSON_scaler = cJSON_GetObjectItem(JSON_variable, "scaler");
    scaler = (JSON_scaler == NULL || JSON_scaler->type != cJSON_Number) ? default_vals.scaler : JSON_scaler->valueint;

    // parse UI enabled boolean
    cJSON* JSON_ui_enabled = cJSON_GetObjectItem(JSON_variable, "ui_enabled");
    ui_enabled = (JSON_ui_enabled == NULL || JSON_ui_enabled->type > cJSON_True) ? default_vals.ui_enabled : JSON_ui_enabled->type == cJSON_True;

    // parse multiple inputs boolean
    cJSON* JSON_multiple_inputs = cJSON_GetObjectItem(JSON_variable, "multiple_inputs");
    multiple_inputs = (JSON_multiple_inputs == NULL || JSON_multiple_inputs->type > cJSON_True) ? default_vals.multiple_inputs : JSON_multiple_inputs->type == cJSON_True;

    // parse write URI string
    cJSON* JSON_write_uri = cJSON_GetObjectItem(JSON_variable, "write_uri");
    write_uri = (JSON_write_uri == NULL || JSON_write_uri->valuestring == NULL) ? default_vals.write_uri : JSON_write_uri->valuestring;

    // parse variable type string and value field for processing
    cJSON* JSON_var_type = cJSON_GetObjectItem(JSON_variable, "var_type");
    cJSON* JSON_value = cJSON_GetObjectItem(JSON_variable, "value");

    // if variable type string was not found, use the default variable type. if the value field was found, use it with the default variable type, otherwise use the default value
    if (JSON_var_type == NULL || JSON_var_type->valuestring == NULL) {
        switch (default_vals.value.type) {
            case Float:
                value.set((JSON_value == NULL) ? default_vals.value.value_float : (float)JSON_value->valuedouble);
                break;
            case Int:
                value.set((JSON_value == NULL) ? default_vals.value.value_int : JSON_value->valueint);
                break;
            case Bool:
                value.set((JSON_value == NULL) ? default_vals.value.value_bool : JSON_value->type == cJSON_True);
                break;
            case String:
                value.set((JSON_value == NULL) ? default_vals.value.value_string : JSON_value->valuestring);
                break;
        }
    }
    // if the variable type string was found, use it to process the value field. if the value field was not found, use the default value
    //      and throw an error if the default variable type does not match the parsed variable type
    else {
        if (strcmp(JSON_var_type->valuestring, "Float") == 0) {
            if (JSON_value == NULL && default_vals.value.type != Float) {
                throw std::runtime_error(variable_id + ": did not find value field, and default value is not compatible with found variable type Float");
            }
            value.set((float)JSON_value->valuedouble);
        } else if (strcmp(JSON_var_type->valuestring, "Bool") == 0) {
            if (JSON_value == NULL && default_vals.value.type != Bool) {
                throw std::runtime_error(variable_id + ": did not find value field, and default value is not compatible with found variable type Bool");
            }
            value.set(JSON_value->type == cJSON_True);
        } else if (strcmp(JSON_var_type->valuestring, "String") == 0) {
            if (JSON_value == NULL && default_vals.value.type != String) {
                throw std::runtime_error(variable_id + ": did not find value field, and default value is not compatible with found variable type String");
            }
            value.set(JSON_value->valuestring);
        } else if (strcmp(JSON_var_type->valuestring, "Int") == 0) {
            if (JSON_value == NULL && default_vals.value.type != Int) {
                throw std::runtime_error(variable_id + ": did not find value field, and default value is not compatible with found variable type Int");
            }
            value.set(JSON_value->valueint);
        } else {
            throw std::runtime_error(variable_id + ": invalid var_type");
        }
    }

    // parse the options array
    cJSON* JSON_options = cJSON_GetObjectItem(JSON_variable, "options");
    num_options = (JSON_options != NULL && JSON_options->type == cJSON_Array) ? cJSON_GetArraySize(JSON_options) : 0;
    if (num_options > 0) {
        cJSON* array_object = NULL;
        cJSON_ArrayForEach(array_object, JSON_options) {
            if (array_object == NULL) {
                throw std::runtime_error(variable_id + ": object in options array is NULL");
            }
            cJSON* JSON_option_name = cJSON_GetObjectItem(array_object, "name");
            options_name.push_back((JSON_option_name != NULL && JSON_option_name->valuestring != NULL) ? JSON_option_name->valuestring : NULL);

            cJSON* JSON_option_value = cJSON_GetObjectItem(array_object, "value");
            if (JSON_option_value != NULL) {
                Value_Object val = Value_Object();
                if (JSON_option_value->type == cJSON_Number && value.type == Float)
                    val.set((float)JSON_option_value->valuedouble);
                else if (JSON_option_value->type == cJSON_Number)
                    val.set(JSON_option_value->valueint);
                else if (JSON_option_value->type == cJSON_True || JSON_option_value->type == cJSON_False)
                    val.set(JSON_option_value->type == cJSON_True);
                else if (JSON_option_value->type == cJSON_String)
                    val.set(JSON_option_value->valuestring);
                else {
                    throw std::runtime_error(variable_id + ": options value is of invalid type");
                }
                options_value.push_back(val);
            } else {
                throw std::runtime_error(variable_id + ": option object in options array did not have value");
            }
        }
    }
}

/**
 * @brief Parses a URI endpoint for an input source ID and translates that to the input source's index.
 *
 * @param uri_endpoint Last fragment in the URI. Should have form of <variable ID>_<input source ID>
 * @return uint Index of the input source in the Fims_Object::inputs vector.
 *
 * @throws Exception if parsed input source ID is not valid.
 * Calls std::string::substr() which can throw exceptions, but error checking within this method should prevent that.
 */
uint Fims_Object::parse_input_source_index(std::string uri_endpoint) {
    // identify position of last underscore in uri_endpoint, which is what splits the variable ID from the input source ID
    auto underscore_position = uri_endpoint.find_last_of('_');
    // npos indicates underscore not found. size()-1 indicates underscore was found but there is nothing after it
    if (underscore_position == std::string::npos || underscore_position == uri_endpoint.size() - 1) {
        throw std::runtime_error("input source identifier was not found in uri_endpoint " + uri_endpoint);
    }
    // isolate the input source ID
    std::string input_source_id = uri_endpoint.substr(underscore_position + 1);
    // use the Input Source List's ID-to-index map to return the index of the identified input source
    try {
        uint source_index = input_source_settings->id_to_index.at(input_source_id);
        return source_index;
    } catch (const std::exception& e) {
        throw std::runtime_error("uri_endpoint " + uri_endpoint + " is associated with a Multiple Input variable but does not contain a valid input source ID");
    }
}

bool Fims_Object::configure(const std::string& var_id, bool* p_flag, Input_Source_List* input_sources, cJSON* JSON_config, const Fims_Object& default_vals, std::vector<Fims_Object*>& multi_input_command_vars) {
    // configure the variable with its ID, reference pointers, and further config data from variables.json
    set_variable_id(var_id);
    is_primary = p_flag;
    input_source_settings = input_sources;
    try {
        parse_json_config(JSON_config, default_vals);
    } catch (const std::exception& e) {
        FPS_ERROR_LOG("failed to parse JSON config: %s", e.what());
        return false;
    }

    // if the variable is a Multiple-Input Command Variable, configure it as such
    if (multiple_inputs) {
        // init all input regs with variable.json initial value (all except UI reg should be quickly overwritten by their source)
        for (int i = 0; i < (int)input_sources->get_num_sources(); ++i) {
            inputs.push_back(value);
        }
        multi_input_command_vars.push_back(this);
    }
    return true;
}
