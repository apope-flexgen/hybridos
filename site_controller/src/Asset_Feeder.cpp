/*
 * Asset_Feeders.cpp
 *
 *  Created on: August 14, 2018
 *      Author: kbrezina
 */

/* C Standard Library Dependencies */
#include <cstring>
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
#include <Site_Controller_Utils.h>
/* Local Internal Dependencies */
#include <Asset_Feeder.h>
#include <Data_Endpoint.h>
#include <Configurator.h>

Asset_Feeder::Asset_Feeder() {
    open_value = 0;
    close_value = 0;
    close_permissive_value = 0;
    close_permissive_remove_value = 0;
    reset_value = 0;
    breaker_close_permissive_status = false;

    asset_type_id = FEEDERS_TYPE_ID;
    asset_type_value = FEEDERS;

    set_required_variables();
}

// required variables checked for in configuration validation
void Asset_Feeder::set_required_variables(void) {
    required_variables.push_back("breaker_status");
    required_variables.push_back("grid_frequency");
    required_variables.push_back("active_power");
    required_variables.push_back("reactive_power");
}

bool Asset_Feeder::get_breaker_status(void) {
    // breaker status can either a mask or a boolean
    return (is_running() || breaker_status.value.value_bool);
}

/**
 * Status of the utility tracked by this feed register
 * Only supported for some sites, else will always be false
 */
bool Asset_Feeder::get_utility_status(void) {
    return utility_status.value.value_bool;
}

float Asset_Feeder::get_gridside_frequency(void) {
    return grid_frequency.value.value_float;
}

float Asset_Feeder::get_gridside_avg_voltage(void) {
    float sumVolts = grid_voltage_l1.value.value_float + grid_voltage_l2.value.value_float + grid_voltage_l3.value.value_float;
    return (numPhases != 0.0 ? sumVolts / numPhases : 0);
}

float Asset_Feeder::get_power_factor() {
    return power_factor.value.value_float;
}

void Asset_Feeder::breaker_reset(void) {
    // component reset command clear should be handled by component
    send_to_comp_uri(reset_value, uri_breaker_reset);
    clear_alerts();
}

bool Asset_Feeder::breaker_close(void) {
    return send_to_comp_uri(close_value, uri_breaker_close);
}

bool Asset_Feeder::breaker_close_permissive(void) {
    return send_to_comp_uri(close_permissive_value, uri_breaker_close_permissive);
}

bool Asset_Feeder::breaker_open(void) {
    return send_to_comp_uri(open_value, uri_breaker_open);
}

bool Asset_Feeder::breaker_close_permissive_remove(void) {
    return send_to_comp_uri(close_permissive_remove_value, uri_breaker_close_permissive_remove);
}

void Asset_Feeder::set_active_power_setpoint(float setpoint) {
    active_power_setpoint.component_control_value.value_float = setpoint;
}

/**
 * Configure feeder-specific variables that are provided for the asset instance level
 * @param configurator The Type_Configurator used to configure this asset
 * @return Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
Config_Validation_Result Asset_Feeder::configure_typed_asset_instance_vars(Type_Configurator* configurator) {
    Asset_Configurator* asset_config = &configurator->asset_config;

    cJSON* object = cJSON_GetObjectItem(asset_config->asset_instance_root, "value_open");
    if (object)
        open_value = object->valueint;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "value_close");
    if (object)
        close_value = object->valueint;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "value_close_permissive");
    if (object)
        close_permissive_value = object->valueint;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "value_close_permissive_remove");
    if (object)
        close_permissive_remove_value = object->valueint;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "value_reset");
    if (object)
        reset_value = object->valueint;

    return Config_Validation_Result(true);
}

/**
 * Configure ui_controls provided in the asset's components array
 * @param configurator The Type_Configurator used to configure this asset
 * @return Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
Config_Validation_Result Asset_Feeder::configure_ui_controls(Type_Configurator* configurator) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    // asset instances are data aggregators for one or many components, described in the "components" array. this array is required for any asset instance
    cJSON* components_array = cJSON_GetObjectItem(configurator->asset_config.asset_instance_root, "components");
    if (components_array == NULL) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: components control array is NULL.", name)));
        return validation_result;
    }

    // for each component in the components array, parse out the UI control variables. other component variables are handled by the base class configure function
    for (uint i = 0; i < numAssetComponents; i++) {
        cJSON* component = cJSON_GetArrayItem(components_array, i);
        if (component == NULL) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: component control array entry {} is invalid or undefined.", name, i + 1)));
            continue;
        }

        // UI controls are optional
        cJSON* ui_controls = cJSON_GetObjectItem(component, "ui_controls");
        if (ui_controls == NULL)
            continue;

        Config_Validation_Result control_result;

        // when adding a new UI control, make sure to add it to the list of valid UI controls in Asset_Manager.cpp
        cJSON* ctrl_obj;
        ctrl_obj = cJSON_GetObjectItem(ui_controls, "maint_mode");
        if (ctrl_obj != NULL) {
            control_result = maint_mode.configure(ctrl_obj, yesNoOption, &inMaintenance, Bool, sliderStr, true);
            if (!control_result.is_valid_config) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure maint_mode UI control.", name)));
            }
            validation_result.absorb(control_result);
        }

        ctrl_obj = cJSON_GetObjectItem(ui_controls, "lock_mode");
        if (ctrl_obj != NULL) {
            control_result = lock_mode.configure(ctrl_obj, yesNoOption, &inLockdown, Bool, sliderStr, true);
            if (!control_result.is_valid_config) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure lock_mode UI control.", name)));
            }
            validation_result.absorb(control_result);
        }

        ctrl_obj = cJSON_GetObjectItem(ui_controls, "breaker_close");
        if (ctrl_obj != NULL) {
            control_result = breaker_close_ctl.configure(ctrl_obj, onOffOption, NULL, Int, buttonStr, false);
            if (!control_result.is_valid_config) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure breaker_close UI control.", name)));
            } else {
                uri_breaker_close = build_uri(compNames[i], breaker_close_ctl.reg_name);
            }
            validation_result.absorb(control_result);
        }

        ctrl_obj = cJSON_GetObjectItem(ui_controls, "breaker_close_permissive");
        if (ctrl_obj != NULL) {
            control_result = breaker_close_perm_ctl.configure(ctrl_obj, onOffOption, NULL, Int, buttonStr, false);
            if (!control_result.is_valid_config) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure breaker_close_permissive UI control.", name)));
            } else {
                uri_breaker_close_permissive = build_uri(compNames[i], breaker_close_perm_ctl.reg_name);
            }
            validation_result.absorb(control_result);
        }

        ctrl_obj = cJSON_GetObjectItem(ui_controls, "breaker_close_permissive_remove");
        if (ctrl_obj != NULL) {
            control_result = breaker_close_perm_remove_ctl.configure(ctrl_obj, onOffOption, NULL, Int, buttonStr, false);
            if (!control_result.is_valid_config) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure breaker_close_permissive_remove UI control.", name)));
            } else {
                uri_breaker_close_permissive_remove = build_uri(compNames[i], breaker_close_perm_remove_ctl.reg_name);
            }
            validation_result.absorb(control_result);
        }

        ctrl_obj = cJSON_GetObjectItem(ui_controls, "breaker_open");
        if (ctrl_obj != NULL) {
            control_result = breaker_open_ctl.configure(ctrl_obj, onOffOption, NULL, Int, buttonStr, false);
            if (!control_result.is_valid_config) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure breaker_open UI control.", name)));
            } else {
                uri_breaker_open = build_uri(compNames[i], breaker_open_ctl.reg_name);
            }
            validation_result.absorb(control_result);
        }

        ctrl_obj = cJSON_GetObjectItem(ui_controls, "breaker_reset");
        if (ctrl_obj != NULL) {
            control_result = breaker_reset_ctl.configure(ctrl_obj, resetOption, NULL, Int, buttonStr, false);
            if (!control_result.is_valid_config) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure breaker_reset UI control.", name)));
            } else {
                uri_breaker_reset = build_uri(compNames[i], breaker_reset_ctl.reg_name);
            }
            validation_result.absorb(control_result);
        }
    }
    return validation_result;
}

/**
 * Each asset has hard-coded Fims_Object member variables that will always be used internally by this asset. This function
 * the asset_var_map to these variables, which also sets up a connection between these variables and the component_var_map
 * down the line.
 * WARNING: "raw" values MUST be configured BEFORE their associated calculated values. This is because calculated
 *          values will sever the asset_var_map connection to the component variable and the raw values need to
 *          come from the component
 * Ex: `dischargeable_power_raw` must be configured here, and `dischargeable_power` must be configured in the
 * associated replace_raw_fims_vars() function
 *
 * Raw values are currently unused for this Asset type
 */
Config_Validation_Result Asset_Feeder::configure_typed_asset_fims_vars(Type_Configurator* configurator) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    validation_result.absorb(configure_single_fims_var(&breaker_status, "breaker_status", configurator, Bool));
    validation_result.absorb(configure_single_fims_var(&utility_status, "utility_status", configurator, Bool));
    validation_result.absorb(configure_single_fims_var(&grid_voltage_l1, "grid_voltage_l1", configurator));
    validation_result.absorb(configure_single_fims_var(&grid_voltage_l2, "grid_voltage_l2", configurator));
    validation_result.absorb(configure_single_fims_var(&grid_voltage_l3, "grid_voltage_l3", configurator));
    validation_result.absorb(configure_single_fims_var(&grid_frequency, "grid_frequency", configurator));

    return validation_result;
}

Config_Validation_Result Asset_Feeder::validate_poi_feeder_configuration(Type_Configurator* configurator) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    // if config validation flag is false, no need to validate
    if (!configurator->config_validation) {
        return validation_result;
    }

    // does config validation that was not done earlier since we did not know which feeder was POI at the time
    Config_Validation_Result base_config_result = validate_config();
    if (!base_config_result.is_valid_config) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: POI feeder failed base Asset validate config check.", name)));
    }
    validation_result.absorb(base_config_result);

    // checks to make sure required base class vars were configured. other assets had these checked in configure_base function
    cJSON* obj = cJSON_GetObjectItem(configurator->asset_config.asset_instance_root, "rated_active_power_kw");
    if (obj == NULL || obj->valuedouble == 0.0) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: POI must provide a nonzero rated_active_power_kw in config.", name)));
    }

    obj = cJSON_GetObjectItem(configurator->asset_config.asset_instance_root, "slew_rate");
    if (obj == NULL) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: POI feeder missing required slew_rate variable.", name)));
    }

    return validation_result;
}

// Todo: This function has a strange unconventional fims hierarchy. Usually there is 2 layers (body->value) this one has 3("metabody"->body->value). Might should figure out and change why this is the case.
// Todo: grab_naked... is a temporary fix. The real goal should be to do pure naked sets, but dbi expects clothed values so this function clothes naked sets before they are handed to dbi.
bool Asset_Feeder::handle_set(std::string uri, cJSON& body) {
    // The current setpoint being parsed from those available
    cJSON* current_setpoint = NULL;

    // Whether the setpoint is supported by persistent settings
    // For instance, sets that modify the system state should not persist as they will default to the published component state on restart
    bool persistent_setpoint = false;

    if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "breaker_close")) && inMaintenance) {
        breaker_close();
        persistent_setpoint = true;
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "breaker_close_permissive")) && inMaintenance) {
        breaker_close_permissive_status = true;
        breaker_close_permissive();
        persistent_setpoint = true;
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "breaker_close_permissive_remove")) && inMaintenance) {
        breaker_close_permissive_status = false;
        breaker_close_permissive_remove();
        persistent_setpoint = true;
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "breaker_open")) && inMaintenance) {
        breaker_open();
        persistent_setpoint = true;
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "breaker_reset"))) {
        if (inMaintenance)
            breaker_reset();  // hard reset, only when in maint mode
        else {
            clear_alerts();  // graceful, component-level reset
        }
    }

    // if target setpoint was found, back it up to DBI if it is a persistent setpoint.
    // otherwise, send it to the generic controls handler
    if (!current_setpoint)
        return handle_generic_asset_controls_set(uri, body);
    if (!persistent_setpoint)
        return true;
    return Asset::send_setpoint(uri, current_setpoint);
}

/**
 * Update the asset status with the measurement received on the breaker_status Fims_Object
 */
void Asset_Feeder::set_raw_status() {
    raw_status = breaker_status.value.value_bool;
}

/**
 * Return the status register
 */
Fims_Object& Asset_Feeder::get_status_register() {
    return breaker_status;
}

/**
 * Get the string representation of this asset's status
 */
const char* Asset_Feeder::get_status_string() const {
    return breaker_status.get_status_string();
}

void Asset_Feeder::process_asset() {
    set_raw_status();
    return Asset::process_asset();
}

void Asset_Feeder::update_asset(void) {
    // TODO:: add maintenance mode handling here
}

void Asset_Feeder::process_potential_active_power(void) {
    if (!get_breaker_status()) {
        max_potential_active_power = 0.0f;
        min_potential_active_power = 0.0f;
        return;
    }
    Asset::process_potential_active_power();
}

void Asset_Feeder::send_to_components(void) {
    FPS_DEBUG_LOG("Currently nothing to send to Feeders; Asset_Feeder::send_to_components.\n");
}

/****************************************************************************************/
// Variable map moved to Feeder_Manager
bool Asset_Feeder::generate_asset_ui(fmt::memory_buffer& buf, const char* const var) {
    bool goodBody = true;

    // add the manual mode control if defined
    maint_mode.enabled = (!inLockdown);
    goodBody = maint_mode.makeJSONObject(buf, var) && goodBody;

    // add the lockdown mode control if defined
    lock_mode.enabled = (inMaintenance);
    goodBody = lock_mode.makeJSONObject(buf, var) && goodBody;

    // now add the rest of the controls
    breaker_close_ctl.enabled = !breaker_status.value.value_bool && inMaintenance;
    goodBody = breaker_close_ctl.makeJSONObject(buf, var) && goodBody;

    breaker_close_perm_ctl.enabled = !breaker_status.value.value_bool && !breaker_close_permissive_status && inMaintenance;
    goodBody = breaker_close_perm_ctl.makeJSONObject(buf, var) && goodBody;

    breaker_close_perm_remove_ctl.enabled = breaker_close_permissive_status && inMaintenance;
    goodBody = breaker_close_perm_remove_ctl.makeJSONObject(buf, var) && goodBody;

    breaker_open_ctl.enabled = breaker_status.value.value_bool && inMaintenance;
    goodBody = breaker_open_ctl.makeJSONObject(buf, var) && goodBody;

    breaker_reset_ctl.enabled = !breaker_status.value.value_bool && inMaintenance;
    goodBody = breaker_reset_ctl.makeJSONObject(buf, var) && goodBody;

    return (goodBody);
}
