/*
 * Asset_Solar.cpp
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
/* Local Internal Dependencies */
#include <Asset_Solar.h>
#include <Data_Endpoint.h>
#include <Configurator.h>
#include <Site_Controller_Utils.h>

Asset_Solar::Asset_Solar() {
    start_value = 0;
    stop_value = 0;
    enter_standby_value = 0;
    exit_standby_value = 0;
    reactive_power_mode_value = 0;
    power_factor_mode_value = 0;

    asset_type_id = SOLAR_TYPE_ID;
    asset_type_value = SOLAR;

    maint_active_power_setpoint = 0.0;
    maint_reactive_power_setpoint = 0.0;

    curtailed_status = false;

    curtailed_active_power_limit = 0.0;
    potential_reactive_power = 0.0;

    set_required_variables();
}

// required variables checked for in configuration validation
void Asset_Solar::set_required_variables(void) {
    required_variables.push_back("active_power_setpoint");
    required_variables.push_back("reactive_power_setpoint");
    required_variables.push_back("status");
    required_variables.push_back("alarms");
    required_variables.push_back("faults");
    required_variables.push_back("active_power");
    required_variables.push_back("reactive_power");
}

bool Asset_Solar::start() {
    if (start_command_throttle.command_trigger())
        return send_to_comp_uri(start_value, uri_start);
    return false;
}

bool Asset_Solar::stop() {
    if (stop_command_throttle.command_trigger()) {
        inStandby = false;
        return (send_to_comp_uri(stop_value, uri_stop));
    }
    return false;
}

bool Asset_Solar::enter_standby(void) {
    if (isRunning && !inStandby && (active_power_setpoint.value.value_float == 0.0) && (reactive_power_setpoint.value.value_float == 0.0)) {
        inStandby = true;
        return send_to_comp_uri(enter_standby_value, uri_enter_standby);
    }
    return false;
}

bool Asset_Solar::exit_standby(void) {
    inStandby = false;
    return send_to_comp_uri(exit_standby_value, uri_exit_standby);
}

float Asset_Solar::get_active_power_setpoint(void) {
    return active_power_setpoint.value.value_float;
}

float Asset_Solar::get_reactive_power_setpoint(void) {
    return reactive_power_setpoint.value.value_float;
}

float Asset_Solar::get_power_factor_setpoint(void) {
    return power_factor_setpoint.value.value_float;
}

bool Asset_Solar::send_active_power_setpoint(void) {
    if (active_power_setpoint_throttle.setpoint_trigger(active_power_setpoint.component_control_value.value_float)) {
        if (round(active_power_setpoint.component_control_value.value_float) != round(active_power_setpoint.value.value_float)) {
            return active_power_setpoint.send_to_component(false, true);
        }
    }
    return false;
}

void Asset_Solar::set_active_power_setpoint(float setpoint) {
    // solar asset cannot absorb active power, and must also bounded by rated value
    active_power_setpoint.component_control_value.value_float = range_check(setpoint, max_limited_active_power, min_limited_active_power);
}

bool Asset_Solar::send_reactive_power_setpoint(void) {
    if (reactive_power_setpoint_throttle.setpoint_trigger(reactive_power_setpoint.component_control_value.value_float)) {
        if (round(reactive_power_setpoint.component_control_value.value_float) != round(reactive_power_setpoint.value.value_float)) {
            return reactive_power_setpoint.send_to_component(false, true);
        }
    }
    return false;
}

void Asset_Solar::set_reactive_power_setpoint(float setpoint) {
    // solar asset can generate negative reactive power, but has to be bounded by potential
    reactive_power_setpoint.component_control_value.value_float = range_check(setpoint, potential_reactive_power, -1 * potential_reactive_power);
}

bool Asset_Solar::send_power_factor_setpoint(void) {
    if (power_factor_setpoint.component_control_value.value_float != power_factor_setpoint.value.value_float)
        return power_factor_setpoint.send_to_component();
    return false;
}

void Asset_Solar::set_power_factor_setpoint(float setpoint) {
    // power factor must be bounded between 1.0 and -1.0
    power_factor_setpoint.component_control_value.value_float = setpoint > 1.0 ? 1.0 : setpoint < -1.0 ? -1.0 : setpoint;
}

bool Asset_Solar::send_power_mode(void) {
    if ((power_mode_setpoint.component_control_value.value_int != powerMode::UNKNOWN) && (power_mode_setpoint.component_control_value.value_int != power_mode_setpoint.value.value_int))
        return power_mode_setpoint.send_to_component();
    return false;
}

void Asset_Solar::set_power_mode(powerMode mode) {
    if (mode == powerMode::REACTIVEPWR)
        power_mode_setpoint.component_control_value.value_int = reactive_power_mode_value;
    else if (mode == powerMode::PWRFCTR)
        power_mode_setpoint.component_control_value.value_int = power_factor_mode_value;
    else
        FPS_ERROR_LOG("Asset_Solar::set_power_mode received invalid mode.\n");
}

/**
 * Configure solar-specific variables that are provided for the asset instance level
 * @param configurator The Type_Configurator used to configure this asset
 * @return Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
Config_Validation_Result Asset_Solar::configure_typed_asset_instance_vars(Type_Configurator* configurator) {
    Asset_Configurator* asset_config = &configurator->asset_config;

    cJSON* object = cJSON_GetObjectItem(asset_config->asset_instance_root, "start_value");
    if (object)
        start_value = object->valueint;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "stop_value");
    if (object)
        stop_value = object->valueint;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "enter_standby_value");
    if (object)
        enter_standby_value = object->valueint;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "exit_standby_value");
    if (object)
        exit_standby_value = object->valueint;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "reactive_power_mode_q");
    if (object)
        reactive_power_mode_value = object->valueint;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "reactive_power_mode_pf");
    if (object)
        power_factor_mode_value = object->valueint;

    return Config_Validation_Result(true);
}

/**
 * Configure ui_controls provided in the asset's components array
 * @param configurator The Type_Configurator used to configure this asset
 * @return Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
Config_Validation_Result Asset_Solar::configure_ui_controls(Type_Configurator* configurator) {
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
        if (ui_controls == NULL) {
            continue;
        }

        // SLIDERS
        Asset::quick_config_slider(ui_controls, "maint_mode", maint_mode, inMaintenance, name, validation_result);
        Asset::quick_config_slider(ui_controls, "lock_mode", lock_mode, inLockdown, name, validation_result);
        // NUMBERS
        Asset::quick_config_numeric(ui_controls, "maint_active_power_setpoint", maint_active_power_setpoint_ctl, maint_active_power_setpoint, name, validation_result);
        Asset::quick_config_numeric(ui_controls, "maint_reactive_power_setpoint", maint_reactive_power_setpoint_ctl, maint_reactive_power_setpoint, name, validation_result);
        Asset::quick_config_numeric(ui_controls, "maint_active_power_slew_rate", maint_active_power_slew_rate_ctl, maint_slew_rate, name, validation_result);

        // BUTTONS
        Asset::quick_config_button(ui_controls, "clear_faults", clear_faults_ctl, compNames[i], uri_clear_faults, name, validation_result, true);  // use true here because resetOption
        Asset::quick_config_button(ui_controls, "start", start_ctl, compNames[i], uri_start, name, validation_result, false);
        Asset::quick_config_button(ui_controls, "stop", stop_ctl, compNames[i], uri_stop, name, validation_result, false);
        Asset::quick_config_button(ui_controls, "enter_standby", enter_standby_ctl, compNames[i], uri_enter_standby, name, validation_result, false);
        Asset::quick_config_button(ui_controls, "exit_standby", exit_standby_ctl, compNames[i], uri_exit_standby, name, validation_result, false);
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
Config_Validation_Result Asset_Solar::configure_typed_asset_fims_vars(Type_Configurator* configurator) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    validation_result.absorb(configure_single_fims_var(&power_mode_setpoint, "reactive_power_mode", configurator, Int, 0, REACTIVEPWR));
    validation_result.absorb(configure_single_fims_var(&power_factor_setpoint, "power_factor_setpoint", configurator));
    validation_result.absorb(configure_single_fims_var(&status, "status", configurator, Status));

    return validation_result;
}

// Variable map moved to ESS_Manager
bool Asset_Solar::generate_asset_ui(fmt::memory_buffer& buf, const char* const var) {
    bool goodBody = true;

    // add the maint mode control if defined
    maint_mode.enabled = !inLockdown;
    goodBody = maint_mode.makeJSONObject(buf, var, true) && goodBody;

    // add the maint mode control if defined
    lock_mode.enabled = inMaintenance;
    goodBody = lock_mode.makeJSONObject(buf, var, true) && goodBody;

    clear_faults_ctl.enabled = (get_num_active_faults() != 0 || get_num_active_alarms() != 0);
    goodBody = clear_faults_ctl.makeJSONObject(buf, var, true) && goodBody;

    // add the start button
    start_ctl.enabled = (!isRunning && inMaintenance) && !is_in_local_mode();
    goodBody = start_ctl.makeJSONObject(buf, var, true) && goodBody;

    // add the stop button
    stop_ctl.enabled = (isRunning && inMaintenance) && !is_in_local_mode();
    goodBody = stop_ctl.makeJSONObject(buf, var, true) && goodBody;

    enter_standby_ctl.enabled = (isRunning && inMaintenance && !inStandby && (active_power_setpoint.value.value_float == 0.0) && (reactive_power_setpoint.value.value_float == 0.0)) && !is_in_local_mode();
    goodBody = enter_standby_ctl.makeJSONObject(buf, var) && goodBody;

    exit_standby_ctl.enabled = (isRunning && inMaintenance && inStandby) && !is_in_local_mode();
    goodBody = exit_standby_ctl.makeJSONObject(buf, var, true) && goodBody;

    // now add the rest of the controls
    maint_active_power_setpoint_ctl.enabled = (inMaintenance && isRunning && !inStandby) && !is_in_local_mode();
    goodBody = maint_active_power_setpoint_ctl.makeJSONObject(buf, var, true) && goodBody;

    maint_reactive_power_setpoint_ctl.enabled = (inMaintenance && isRunning && !inStandby) && !is_in_local_mode();
    goodBody = maint_reactive_power_setpoint_ctl.makeJSONObject(buf, var, true) && goodBody;

    maint_active_power_slew_rate_ctl.enabled = inMaintenance && isRunning && !is_in_local_mode();
    goodBody = maint_active_power_slew_rate_ctl.makeJSONObject(buf, var, true) && goodBody;

    return (goodBody);
}

// Todo: This function has a strange unconventional fims hierarchy. Usually there is 2 layers (body->value) this one has 3("metabody"->body->value). Might should figure out and change why this is the case.
// Todo: grab_naked... is a temporary fix. The real goal should be to do pure naked sets, but dbi expects clothed values so this function clothes naked sets before they are handed to dbi.
bool Asset_Solar::handle_set(std::string uri, cJSON& body) {
    // The current setpoint being parsed from those available
    cJSON* current_setpoint = NULL;
    // The value of the setpoint object received
    cJSON* value = NULL;
    // Whether the setpoint is supported by persistent settings
    // For instance, sets that modify the system state should not persist as they will default to the published component state on restart
    bool persistent_setpoint = false;

    if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "clear_faults"))) {
        clear_alerts();
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_Number, "maint_active_power_setpoint"))) {
        value = cJSON_GetObjectItem(current_setpoint, "value");
        maint_active_power_setpoint = value->valuedouble;
        persistent_setpoint = true;
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_Number, "maint_reactive_power_setpoint"))) {
        value = cJSON_GetObjectItem(current_setpoint, "value");
        maint_reactive_power_setpoint = value->valuedouble;
        persistent_setpoint = true;
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_Number, "maint_active_power_slew_rate")) != nullptr) {
        maint_slew_rate = cJSON_GetObjectItem(current_setpoint, "value")->valuedouble;
        persistent_setpoint = true;
        if (inMaintenance) {
            active_power_slew.set_slew_rate(maint_slew_rate);
        }
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "stop"))) {
        stop();
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "start"))) {
        start();
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "enter_standby"))) {
        enter_standby();
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "exit_standby"))) {
        exit_standby();
    }

    // if target setpoint was found, back it up to DBI if it is a persistent setpoint.
    // otherwise, send it to the generic controls handler
    if (!current_setpoint)
        return handle_generic_asset_controls_set(uri, body);
    if (!persistent_setpoint)
        return true;
    return Asset::send_setpoint(uri, current_setpoint);
}

void Asset_Solar::set_potential_active_power_limit(float limit) {
    curtailed_active_power_limit = limit;
}

void Asset_Solar::process_potential_active_power(void)  // overriden from the base class, called every 100ms
{
    Asset::process_potential_active_power();

    // Further limit based on reactive power and apparent power if appropriate (do not include curtailment potential limit)
    // TODO: curtailed_active_power_limit is intentionally not included is it can create a feedback loop that gets us stuck at 0. Full feedback loop:
    //       (curtailed_active_power_limit -> max_potential_active_power -> active_power_setpoint -> active_power -> curtailed_active_power_limit)
    //       curtailed_active_power_limit also appears to not function fully as intended as it can be bypassed in some cases. Revisit if its behavior can
    //       be changed so that it can be included in the active power limit calculation without feedback loop.
    if (reactive_power_priority) {
        active_power_limit = sqrtf(powf(rated_apparent_power_kva, 2) - powf(reactive_power_setpoint.value.value_float, 2));
        max_limited_active_power = max_potential_active_power = std::min(max_potential_active_power, active_power_limit);
    }

    // max value is capped by find_next_curtailment_state curtailment state
    max_potential_active_power = (max_potential_active_power > curtailed_active_power_limit) ? curtailed_active_power_limit : max_potential_active_power;
}

/**
 * Update the asset status with the measurement received on the status Fims_Object
 */
void Asset_Solar::set_raw_status() {
    raw_status = status.value.value_bit_field;
}

/**
 * Get the string representation of this asset's status
 */
const char* Asset_Solar::get_status_string() const {
    return status.get_status_string();
}

/**
 * Returns the status register
 */
Fims_Object& Asset_Solar::get_status_register() {
    return status;
}

void Asset_Solar::process_asset(void) {
    set_raw_status();
    Asset::process_asset();

    if (get_num_active_faults() != 0 && isRunning)
        stop();
}

void Asset_Solar::update_asset(void) {
    // override setpoints in maintenance mode
    if (inMaintenance) {
        set_power_mode(powerMode::REACTIVEPWR);  // only allow reactive power mode while in maintenance

        if (this->maint_active_power_slew_rate_ctl.configured) {
            auto slewed_cmd = maint_active_power_setpoint;
            // Essentially this is 
            // slewed_cmd = min_limited_active_power < maint_active_power_setpoint < max_limited_active_power;
            slewed_cmd = std::max(slewed_cmd, this->min_limited_active_power);
            slewed_cmd = std::min(slewed_cmd, this->max_limited_active_power);
            set_active_power_setpoint(slewed_cmd); // <-- WE HAVE A RAW SET HERE UNLIMITED. WE SLAM.
        } else {
            set_active_power_setpoint(maint_active_power_setpoint); // <-- lEGACY BEHAVIOR
        }
        set_reactive_power_setpoint(maint_reactive_power_setpoint);
    }
}

void Asset_Solar::send_to_components(void) {
    if (is_in_local_mode())
        return;

    send_active_power_setpoint();
    send_reactive_power_setpoint();
    send_power_factor_setpoint();
    send_power_mode();
}
