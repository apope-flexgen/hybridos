/*
 * Asset_Generator.cpp
 *
 *  Created on: May 9, 2018
 *      Author: jcalcagni
 */

/* C Standard Library Dependencies */
#include <cstring>
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
/* Local Internal Dependencies */
#include <Asset_Generator.h>
#include <Data_Endpoint.h>
#include <Configurator.h>
#include <Site_Controller_Utils.h>

Asset_Generator::Asset_Generator() {
    ldss = NULL;
    block_ldss_static_starts = false;
    block_ldss_static_stops = true;

    start_value = 0;
    stop_value = 0;
    grid_forming_value = 0;
    grid_following_value = 0;
    maint_active_power_setpoint = 0.0;
    maint_reactive_power_setpoint = 0.0;
    cooldown_countdown = 0;
    warmup_countdown = 0;
    cooling_down = false;
    warming_up = false;

    isStarting = false;
    isStopped = false;
    isStopping = false;
    isBalanced = false;

    dynamic_start_priority = 0;
    dynamic_stop_priority = 0;

    stopped_status_mask = 0;
    starting_status_mask = 0;
    stopping_status_mask = 0;

    asset_type_id = GENERATORS_TYPE_ID;
    asset_type_value = GENERATORS;

    set_required_variables();
}

// required variables checked for in configuration validation
void Asset_Generator::set_required_variables(void) {
    required_variables.push_back("status");
    required_variables.push_back("active_power_setpoint");
    required_variables.push_back("reactive_power_setpoint");
    required_variables.push_back("active_power");
    required_variables.push_back("reactive_power");
    required_variables.push_back("alarms");
    required_variables.push_back("faults");
}

bool Asset_Generator::start(void) {
    if (start_command_throttle.command_trigger()) {
        // isStarting is set to true when start command is sent, and is set to false once gen reports it is Running
        isStarting = true;
        isStopping = false;
        reset_warmup_timer();
        warming_up = true;
        block_ldss_static_starts = true;

        return send_to_comp_uri(start_value, uri_start);
    }
    return false;
}

bool Asset_Generator::stop(void) {
    if (stop_command_throttle.command_trigger()) {
        isStarting = false;
        // isStopping is set to true while a generator is ramped down (done when in grid following mode)
        // but once stop command is sent, generator is considered stopped
        isStopping = false;
        reset_cooldown_timer();
        cooling_down = true;
        block_ldss_static_stops = true;

        return send_to_comp_uri(stop_value, uri_stop);
    }
    return false;
}

bool Asset_Generator::is_starting(void) const {
    return isStarting;
}

bool Asset_Generator::is_stopping(void) {
    return isStopping;
}

void Asset_Generator::set_stopping_flag(bool flag) {
    isStopping = flag;
}

bool Asset_Generator::is_balanced(void) {
    return isBalanced;
}

void Asset_Generator::set_balanced(bool flag) {
    isBalanced = flag;
}

bool Asset_Generator::send_active_power_setpoint(void) {
    if (active_power_setpoint_throttle.setpoint_trigger(active_power_setpoint.component_control_value.value_float)) {
        if (round(active_power_setpoint.component_control_value.value_float) != round(active_power_setpoint.value.value_float)) {
            return active_power_setpoint.send_to_component(false, true);
        }
    }
    return false;
}

void Asset_Generator::set_active_power_setpoint(float setpoint) {
    // generator command must be bounded by setpoint and can't be negative
    active_power_setpoint.component_control_value.value_float = range_check(setpoint, max_limited_active_power, min_limited_active_power);
}

void Asset_Generator::process_potential_active_power(void)  // overriden from the base class, called every 100ms
{
    Asset::process_potential_active_power();

    // max and min values are capped when generator is shutting down at current active power production
    // while the generator is still listed as controllable, do not want site_manager or asset_manager to have control of the generator's power production during shutdown
    // refer to Generator_Manager.cpp::calculate_gen_power()
    if (isStopping)
        max_potential_active_power = min_potential_active_power = active_power.value.value_float;

    // Further limit based on reactive power and apparent power if appropriate
    if (reactive_power_priority) {
        active_power_limit = sqrtf(powf(rated_apparent_power_kva, 2) - powf(reactive_power_setpoint.value.value_float, 2));
        max_potential_active_power = std::min(max_potential_active_power, active_power_limit);
    }
    max_limited_active_power = max_potential_active_power;
}

bool Asset_Generator::send_reactive_power_setpoint(void) {
    if (reactive_power_setpoint_throttle.setpoint_trigger(reactive_power_setpoint.component_control_value.value_float)) {
        if (round(reactive_power_setpoint.component_control_value.value_float) != round(reactive_power_setpoint.value.value_float)) {
            return reactive_power_setpoint.send_to_component(false, true);
        }
    }
    return false;
}

void Asset_Generator::set_reactive_power_setpoint(float setpoint) {
    // generator command must be bounded by setpoint and TODO: can't be negative?
    reactive_power_setpoint.component_control_value.value_float = range_check(setpoint, potential_reactive_power, 0.0f);
}

float Asset_Generator::get_active_power_setpoint(void) {
    return active_power_setpoint.value.value_float;
}

float Asset_Generator::get_active_power_setpoint_control(void) {
    return active_power_setpoint.component_control_value.value_float;
}

float Asset_Generator::get_reactive_power_setpoint(void) {
    return reactive_power_setpoint.value.value_float;
}

float Asset_Generator::get_reactive_power_setpoint_control(void) {
    return reactive_power_setpoint.component_control_value.value_float;
}

/**
 * Configure gen-specific variables that are provided for the asset instance level
 * @param configurator The Type_Configurator used to configure this asset
 * @return Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
Config_Validation_Result Asset_Generator::configure_typed_asset_instance_vars(Type_Configurator* configurator) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    Asset_Configurator* asset_config = &configurator->asset_config;

    cJSON* stoppedMask = cJSON_GetObjectItem((&configurator->asset_config)->asset_instance_root, "stopped_status_mask");
    // Stop Mask is required if config validation is enabled
    if (stoppedMask == NULL && configurator->config_validation) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("Asset {}: failed to find stopped_status_mask in config.", name)));
    }
    // If the mask isn't NULL and is a hex string, try to parse it
    else if (stoppedMask != NULL && stoppedMask->valuestring != NULL) {
        try {
            // Casts the provided hex string as an unsigned int64
            stopped_status_mask = (uint64_t)std::stoul(stoppedMask->valuestring, NULL, 16);
        } catch (...) {
            // Could not parse the provided mask
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("Asset {}: received an invalid stopped_status_mask.", name)));
        }
    }
    // If the mask isn't null and isn't a hex string, throw an error
    else if (stoppedMask != NULL) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("Asset {}: received an invalid input type instead of a hexadecimal string for the stopped_status_mask.", name)));
    }

    cJSON* object = cJSON_GetObjectItem(asset_config->asset_instance_root, "starting_status_mask");
    if (object)
        starting_status_mask = (uint64_t)std::stoul(object->valuestring, NULL, 16);

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "stopping_status_mask");
    if (object)
        stopping_status_mask = (uint64_t)std::stoul(object->valuestring, NULL, 16);

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "start_value");
    if (object)
        start_value = object->valueint;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "stop_value");
    if (object)
        stop_value = object->valueint;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "grid_form_cmd");
    if (object)
        grid_forming_value = object->valueint;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "grid_follow_cmd");
    if (object)
        grid_following_value = object->valueint;

    return validation_result;
}

/**
 * Configure ui_controls provided in the asset's components array
 * @param configurator The Type_Configurator used to configure this asset
 * @return Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
Config_Validation_Result Asset_Generator::configure_ui_controls(Type_Configurator* configurator) {
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

        ctrl_obj = cJSON_GetObjectItem(ui_controls, "clear_faults");
        if (ctrl_obj != NULL) {
            control_result = clear_faults_ctl.configure(ctrl_obj, resetOption, NULL, Bool, buttonStr, false);
            if (!control_result.is_valid_config) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure clear_faults UI control.", name)));
            } else {
                uri_clear_faults = build_uri(compNames[i], clear_faults_ctl.reg_name);
            }
            validation_result.absorb(control_result);
        }

        ctrl_obj = cJSON_GetObjectItem(ui_controls, "start");
        if (ctrl_obj != NULL) {
            control_result = start_ctl.configure(ctrl_obj, onOffOption, NULL, Bool, buttonStr, false);
            if (!control_result.is_valid_config) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure start asset UI control.", name)));
            } else {
                uri_start = build_uri(compNames[i], start_ctl.reg_name);
            }
            validation_result.absorb(control_result);
        }

        ctrl_obj = cJSON_GetObjectItem(ui_controls, "stop");
        if (ctrl_obj != NULL) {
            control_result = stop_ctl.configure(ctrl_obj, onOffOption, NULL, Bool, buttonStr, false);
            if (!control_result.is_valid_config) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure stop asset UI control.", name)));
            } else {
                uri_stop = build_uri(compNames[i], stop_ctl.reg_name);
            }
            validation_result.absorb(control_result);
        }

        ctrl_obj = cJSON_GetObjectItem(ui_controls, "start_next");
        if (ctrl_obj != NULL) {
            control_result = start_next_ctl.configure(ctrl_obj, onOffOption, NULL, Bool, buttonStr, true);
            if (!control_result.is_valid_config) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure start_next UI control.", name)));
            }
            validation_result.absorb(control_result);
        }

        ctrl_obj = cJSON_GetObjectItem(ui_controls, "stop_next");
        if (ctrl_obj != NULL) {
            control_result = stop_next_ctl.configure(ctrl_obj, onOffOption, NULL, Bool, buttonStr, true);
            if (!control_result.is_valid_config) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure stop_next UI control.", name)));
            }
            validation_result.absorb(control_result);
        }

        ctrl_obj = cJSON_GetObjectItem(ui_controls, "maint_active_power_setpoint");
        if (ctrl_obj != NULL) {
            control_result = maint_active_power_setpoint_ctl.configure(ctrl_obj, nullJson, &maint_active_power_setpoint, Float, numberStr, false);
            if (!control_result.is_valid_config) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure maint_active_power_setpoint UI control.", name)));
            }
            validation_result.absorb(control_result);
        }

        ctrl_obj = cJSON_GetObjectItem(ui_controls, "maint_reactive_power_setpoint");
        if (ctrl_obj != NULL) {
            control_result = maint_reactive_power_setpoint_ctl.configure(ctrl_obj, nullJson, &maint_reactive_power_setpoint, Float, numberStr, false);
            if (!control_result.is_valid_config) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure maint_reactive_power_setpoint UI control.", name)));
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
 * Raw variables are currently unused for this Asset type
 */
Config_Validation_Result Asset_Generator::configure_typed_asset_fims_vars(Type_Configurator* configurator) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    validation_result.absorb(configure_single_fims_var(&grid_mode_setpoint, "grid_mode", configurator, Int, 0, FOLLOWING));
    validation_result.absorb(configure_single_fims_var(&status, "status", configurator, Status));

    return validation_result;
}

/****************************************************************************************/
// Variable map moved to Generator_Manager
bool Asset_Generator::generate_asset_ui(fmt::memory_buffer& buf, const char* const var) {
    bool goodBody = true;

    // add the manual mode control if defined
    maint_mode.enabled = !inLockdown;
    goodBody = maint_mode.makeJSONObject(buf, var, true) && goodBody;

    // add the manual mode control if defined
    lock_mode.enabled = inMaintenance;
    goodBody = lock_mode.makeJSONObject(buf, var, true) && goodBody;

    clear_faults_ctl.enabled = (get_num_active_faults() != 0 || get_num_active_alarms() != 0);
    goodBody = clear_faults_ctl.makeJSONObject(buf, var, true) && goodBody;

    // add the start control if defined
    start_ctl.enabled = !isRunning && inMaintenance && isStopped && !is_in_local_mode();
    goodBody = start_ctl.makeJSONObject(buf, var, true) && goodBody;

    // now add the rest of the controls
    stop_ctl.enabled = isRunning && inMaintenance && !is_in_local_mode();
    goodBody = stop_ctl.makeJSONObject(buf, var, true) && goodBody;

    start_next_ctl.enabled = ldss->enabled && !inMaintenance && !isRunning && isAvail && (ldss->priority_setting == STATIC ? (get_static_start_priority() > 1 && !block_ldss_static_starts) : get_dynamic_start_priority() > 1) && !is_in_local_mode();
    goodBody = start_next_ctl.makeJSONObject(buf, var) && goodBody;

    stop_next_ctl.enabled = ldss->enabled && !inMaintenance && isRunning && (ldss->priority_setting == STATIC ? (get_static_stop_priority() > 1 && !block_ldss_static_stops) : get_dynamic_stop_priority() > 1) && !is_in_local_mode();
    goodBody = stop_next_ctl.makeJSONObject(buf, var) && goodBody;

    maint_active_power_setpoint_ctl.enabled = (inMaintenance && isRunning) && !is_in_local_mode();
    goodBody = maint_active_power_setpoint_ctl.makeJSONObject(buf, var, true) && goodBody;

    maint_reactive_power_setpoint_ctl.enabled = (inMaintenance && isRunning) && !is_in_local_mode();
    goodBody = maint_reactive_power_setpoint_ctl.makeJSONObject(buf, var, true) && goodBody;

    return (goodBody);
}

gridMode Asset_Generator::get_grid_mode(void) {
    if (grid_mode_setpoint.value.value_int == grid_following_value)
        return gridMode::FOLLOWING;
    else if (grid_mode_setpoint.value.value_int == grid_forming_value)
        return gridMode::FORMING;
    else
        return gridMode::UNDEFINED;
}

bool Asset_Generator::send_grid_mode(void) {
    if (grid_mode_setpoint.component_control_value.value_int != grid_mode_setpoint.value.value_int)
        return grid_mode_setpoint.send_to_component();
    return false;
}

void Asset_Generator::set_grid_mode(gridMode mode) {
    if (mode == gridMode::FOLLOWING)
        grid_mode_setpoint.component_control_value.value_int = grid_following_value;
    else if (mode == gridMode::FORMING)
        grid_mode_setpoint.component_control_value.value_int = grid_forming_value;
    else
        FPS_ERROR_LOG("Asset_ESS::set_grid_mode received invalid mode.\n");
}

// Todo: This function has a strange unconventional fims hierarchy. Usually there is 2 layers (body->value) this one has 3("metabody"->body->value). Might should figure out and change why this is the case.
// Todo: grab_naked... is a temporary fix. The real goal should be to do pure naked sets, but dbi expects clothed values so this function clothes naked sets before they are handed to dbi.
bool Asset_Generator::handle_set(std::string uri, cJSON& body) {
    // The current setpoint being parsed from those available
    cJSON* current_setpoint = NULL;
    // The value of the setpoint object received
    cJSON* value = NULL;
    // Whether the setpoint is supported by persistent settings
    // For instance, sets that modify the system state should not persist as they will default to the published component state on restart
    bool persistent_setpoint = false;

    if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "clear_faults"))) {
        clear_alerts();
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "start"))) {
        start();  // issue the start command
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "stop"))) {
        stop();  // issue the stop command
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_Number, "active_power_setpoint"))) {
        // TODO: unused, replaced by maint_active_power_setpoint?
        value = cJSON_GetObjectItem(current_setpoint, "value");
        set_active_power_setpoint(value->valuedouble);
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_Number, "reactive_power_setpoint"))) {
        // TODO: unused, replaced by maint_reactive_power_setpoint?
        value = cJSON_GetObjectItem(current_setpoint, "value");
        set_reactive_power_setpoint(value->valuedouble);
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_Number, "maint_active_power_setpoint"))) {
        value = cJSON_GetObjectItem(current_setpoint, "value");
        set_active_power_setpoint(maint_active_power_setpoint = value->valuedouble);
        persistent_setpoint = true;
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_Number, "maint_reactive_power_setpoint"))) {
        value = cJSON_GetObjectItem(current_setpoint, "value");
        set_reactive_power_setpoint(maint_reactive_power_setpoint = value->valuedouble);
        persistent_setpoint = true;
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
 * Update the asset status with the measurement received on the status Fims_Object
 */
void Asset_Generator::set_raw_status() {
    raw_status = status.value.value_bit_field;
}

/**
 * Get the string representation of this asset's status
 */
const char* Asset_Generator::get_status_string() const {
    return status.get_status_string();
}

/**
 * Return the status register
 */
Fims_Object& Asset_Generator::get_status_register() {
    return status;
}

void Asset_Generator::process_asset(void) {
    set_raw_status();
    Asset::process_asset();

    // process stopped status (needed for LDSS timing resets)
    if (status.get_status_type() == random_enum)
        isStopped = internal_status == stopped_status_mask;
    else if (status.get_status_type() == bit_field)
        // The internal status parsed from the component publish will already be bit shifted
        // Theoretically, multiple values can be set, so check if any of them are true
        // e.g. valid stopped states: 0, 1, 2; mask (binary): 0111
        // for status value 2, verify: 0111 & 0100
        isStopped = static_cast<bool>(stopped_status_mask & internal_status);
}

bool Asset_Generator::is_stopped(void) {
    return isStopped;
}

void Asset_Generator::update_asset(void) {
    if (get_num_active_faults() != 0 && !isStopped) {
        stop();
        set_active_power_setpoint(0.0);
        set_reactive_power_setpoint(0.0);
    } else if (inMaintenance) {
        set_active_power_setpoint(maint_active_power_setpoint);
        set_reactive_power_setpoint(maint_reactive_power_setpoint);
    }

    if (isRunning) {
        isStarting = false;
    } else if (isStopped) {
        set_active_power_setpoint(0.0); /* added to handle cases where a generator could be stopped outside of
                                         * Asset_Mananger::process_gen_power() control. This will ensure that
                                         * the internal state of the generator remains at 0kW while stopped.
                                         */
        set_reactive_power_setpoint(0.0);
    }
}

void Asset_Generator::send_to_components(void) {
    if (is_in_local_mode())
        return;

    send_grid_mode();
    send_active_power_setpoint();
    send_reactive_power_setpoint();
}

bool Asset_Generator::set_dynamic_start_priority(int priority_desired) {
    dynamic_start_priority = priority_desired;
    return true;
}

bool Asset_Generator::set_dynamic_stop_priority(int priority_desired) {
    dynamic_stop_priority = priority_desired;
    return true;
}

int Asset_Generator::get_dynamic_start_priority(void) const {
    return dynamic_start_priority;
}

int Asset_Generator::get_dynamic_stop_priority(void) const {
    return dynamic_stop_priority;
}

int Asset_Generator::get_static_start_priority(void) const {
    return static_start_priority;
}

void Asset_Generator::set_static_start_priority(int target) {
    static_start_priority = target;
}

int Asset_Generator::get_static_stop_priority(void) const {
    return static_stop_priority;
}

void Asset_Generator::set_static_stop_priority(int target) {
    static_stop_priority = target;
}

bool Asset_Generator::is_warmup_timer_active(void) const {
    return warmup_countdown > 0;
}

bool Asset_Generator::is_cooldown_timer_active(void) const {
    return cooldown_countdown > 0;
}

void Asset_Generator::reset_warmup_timer() {
    warmup_countdown = ldss->warmup_time;
}

void Asset_Generator::reset_cooldown_timer() {
    cooldown_countdown = ldss->cooldown_time;
}

void Asset_Generator::tick_warmup_timer(void) {
    if (warmup_countdown > 0)
        warmup_countdown--;
}

void Asset_Generator::tick_cooldown_timer(void) {
    if (cooldown_countdown > 0)
        cooldown_countdown--;
}

int Asset_Generator::get_warmup_timer(void) const {
    return warmup_countdown;
}

int Asset_Generator::get_cooldown_timer(void) const {
    return cooldown_countdown;
}

bool Asset_Generator::get_cooling_down_flag(void) {
    return cooling_down;
}

void Asset_Generator::set_cooling_down_flag(bool flag) {
    cooling_down = flag;
}

bool Asset_Generator::get_warming_up_flag(void) {
    return warming_up;
}

void Asset_Generator::set_warming_up_flag(bool flag) {
    warming_up = flag;
}

bool Asset_Generator::get_block_ldss_static_starts_flag(void) {
    return block_ldss_static_starts;
}

bool Asset_Generator::get_block_ldss_static_stops_flag(void) {
    return block_ldss_static_stops;
}

void Asset_Generator::block_starts_based_on_static_priority(bool block_flag) {
    block_ldss_static_starts = block_flag;
}

void Asset_Generator::block_stops_based_on_static_priority(bool block_flag) {
    block_ldss_static_stops = block_flag;
}
