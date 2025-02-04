/*
 * Asset_ESS.cpp
 *
 *  Created on: August 14, 2018
 *      Author: kbrezina
 */

/* C Standard Library Dependencies */
#include <cmath>
#include <cstdlib>
#include <cstring>
#include "Types.h"
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
/* Local Internal Dependencies */
#include <Asset_ESS.h>
#include <Data_Endpoint.h>
#include <Configurator.h>
#include <Site_Controller_Utils.h>

Asset_ESS::Asset_ESS() {
    maint_active_power_setpoint = 0.0;
    maint_reactive_power_setpoint = 0.0;
    maint_chargeable_min_limit = 0.0;
    maint_dischargeable_min_limit = 0.0;
    maint_min_soc_limit = 0.0;
    maint_max_soc_limit = 0.0;
    maint_soc_protection_buffers_disable_flag = false;  // this is the buffers at top and bottom soc
    maint_soc_limits_enable_flag = false;               // this is toggling on changing those buffers in maint mode
    maint_cell_voltage_limits_enable_flag = false;      // this is toggling on voltage limits in maint mode
    maint_rack_voltage_limits_enable_flag = false;      // this is toggling on voltage limits in maint mode
    maint_min_charge_discharge_enable_flag = false;     // this is toggling on min charge/discharge limits in maint mode
    maint_min_cell_voltage_limit = 0.0;
    maint_max_cell_voltage_limit = 0.0;
    maint_min_rack_voltage_limit = 0.0;
    maint_max_rack_voltage_limit = 0.0;

    reactive_power_mode_value = 0;
    power_factor_mode_value = 0;

    start_value = 0;
    stop_value = 0;
    enter_standby_value = 0;
    exit_standby_value = 0;
    // Valid standby status value
    // Set nonzero in case mask is not provided to avoid conflict with running/stopped mask
    standby_status_mask = 8192;
    bms_control_close = 0;
    bms_control_open = 0;
    bms_control_reset = 0;
    dc_contactor_restriction = false;
    grid_forming_value = 0;
    grid_following_value = 0;
    setpoint_status = ACCEPTED;

    potential_reactive_power = 0.0;

    asset_type_id = ESS_TYPE_ID;
    asset_type_value = ESS;
    maxRawSoc = 0;
    minRawSoc = 0;
    rated_capacity = 0;
    dischargeable_soc_limit = 0.0f;
    chargeable_soc_limit = 0.0f;

    rated_chargeable_power = 0;
    energy_configured = false;
    rated_dischargeable_power = 0;

    soc_protection_buffers_disable_flag = false;
    cell_voltage_limits_flag = false;
    rack_voltage_limits_flag = false;
    chgSocBegin = 0.0;
    chgSocEnd = 0.0;
    dischgSocBegin = 0.0;
    dischgSocEnd = 0.0;

    set_required_variables();
}

// required variables checked for in configuration validation
void Asset_ESS::set_required_variables(void) {
    required_variables.push_back("active_power_setpoint");
    required_variables.push_back("reactive_power_setpoint");
    required_variables.push_back("system_chargeable_power");
    required_variables.push_back("system_dischargeable_power");
    required_variables.push_back("status");
    required_variables.push_back("alarms");
    required_variables.push_back("active_power");
    required_variables.push_back("reactive_power");
    required_variables.push_back("faults");
    required_variables.push_back("soc");
}

int Asset_ESS::get_soh(void) {
    return soh.value.value_float;
}

float Asset_ESS::get_soc(void) {
    return soc.value.value_float;
}

float Asset_ESS::get_max_temp(void) {
    return max_temp.value.value_float;
}

setpoint_states Asset_ESS::get_setpoint_status(void) {
    return setpoint_status;
}

float Asset_ESS::get_chargeable_power(void) {
    return chargeable_power.value.value_float;
}

float Asset_ESS::get_dischargeable_power(void) {
    return dischargeable_power.value.value_float;
}

float Asset_ESS::get_chargeable_energy(void) {
    return chargeable_energy.value.value_float;
}

float Asset_ESS::get_dischargeable_energy(void) {
    return dischargeable_energy.value.value_float;
}

float Asset_ESS::get_min_limited_active_power(void) {
    return -1.0f * std::min(active_power_limit, std::abs(chargeable_power.value.value_float));
}

float Asset_ESS::get_max_limited_active_power(void) {
    return std::min(active_power_limit, dischargeable_power.value.value_float);
}

float Asset_ESS::get_active_power_setpoint(void) {
    return active_power_setpoint.value.value_float;
}

/** ESS_Manager needs to get the updated setpoint for its assets map **/
float Asset_ESS::get_active_power_setpoint_control(void) {
    return active_power_setpoint.component_control_value.value_float;
}

float Asset_ESS::get_reactive_power_setpoint(void) {
    return reactive_power_setpoint.value.value_float;
}

/** ESS_Manager needs to get the updated setpoint for its assets map **/
float Asset_ESS::get_reactive_power_setpoint_control(void) {
    return reactive_power_setpoint.component_control_value.value_float;
}

float Asset_ESS::get_power_factor_setpoint(void) {
    return power_factor_setpoint.value.value_float;
}

float Asset_ESS::get_pcs_nominal_voltage_setting(void) {
    // Current logic makes control value for PCS A always the same as control value for PCS B
    // If that assumption changes, this function should get split into one for A and one for B
    return pcs_a_nominal_voltage_setpoint.component_control_value.value_float;
}

bool Asset_ESS::send_voltage_setpoint(void) {
    if ((int)voltage_setpoint.component_control_value.value_float != (int)voltage_setpoint.value.value_float)  // This is comparing the voltage control setpoint to the voltage status setpoint but with 3 decimal-place precision
        return voltage_setpoint.send_to_component();
    return false;
}

void Asset_ESS::set_voltage_setpoint(float setpoint) {
    voltage_setpoint.component_control_value.value_float = setpoint;
}

bool Asset_ESS::send_frequency_setpoint(void) {
    // Get 2 decimal-place precision
    float control_frequency = ((float)((int)(frequency_setpoint.component_control_value.value_float * 100))) / 100;
    float status_frequency = ((float)((int)(frequency_setpoint.value.value_float * 100))) / 100;

    if (control_frequency != status_frequency)  // This is comparing the frequency control setpoint to the frequency status setpoint but with 3 decimal-place precision
        return frequency_setpoint.send_to_component();
    return false;
}

void Asset_ESS::set_frequency_setpoint(float setpoint) {
    frequency_setpoint.component_control_value.value_float = setpoint;
}

bool Asset_ESS::enter_standby(void) {
    if (!inStandby && (active_power_setpoint.value.value_float == 0.0) && (reactive_power_setpoint.value.value_float == 0.0)) {
        return send_to_comp_uri(enter_standby_value, uri_enter_standby);
    }
    return false;
}

bool Asset_ESS::exit_standby(void) {
    if (inStandby)
        return send_to_comp_uri(exit_standby_value, uri_exit_standby);
    return false;
}

bool Asset_ESS::start(void) {
    // TODO: add debounce timer if needed
    if (!isRunning && start_command_throttle.command_trigger()) {
        return send_to_comp_uri(start_value, uri_start);
    }
    return false;
}

bool Asset_ESS::stop(void) {
    // TODO: add debounce timer if needed
    if (isRunning && stop_command_throttle.command_trigger()) {
        return send_to_comp_uri(stop_value, uri_stop);
    }
    return false;
}

// Write the autobalancing status (enabled/disabled) to components
// Same function used for both UI controls as they write to the same component register
bool Asset_ESS::set_autobalancing(bool status) {
    // Auto balancing will work in any state we track (running, standby, stopped)
    if (inMaintenance) {
        return send_to_comp_uri(status, uri_set_autobalancing);
    }
    return false;
}

bool Asset_ESS::send_active_power_setpoint(void) {
    if (active_power_setpoint_throttle.setpoint_trigger(active_power_setpoint.component_control_value.value_float)) {
        if (round(active_power_setpoint.component_control_value.value_float) != round(active_power_setpoint.value.value_float)) {
            return active_power_setpoint.send_to_component(false, true);
        }
    }
    return false;
}

/**
 * @brief Sets the ESS active power setpoint control to the given value after applying maximum and minimum limits.
 * @param setpoint The value to which the active power setpoint control should be set, pre-limiting.
 * @param use_strict_limits If false, will only limit based on dischargeable power and chargeable power.
 * If true, will also limit based on asset slews and reactive power priority limiting.
 * Possible reason to NOT use strict limits: setpoint decided by Site Manager which already took into account slew limits and may have decided to ignore them.
 * Possible reason to use strict limits: asset is in maintenance mode so limits such as reactive power priority have not been applied to the given setpoint.
 */
void Asset_ESS::set_active_power_setpoint(float setpoint, bool use_strict_limits) {
    if (use_strict_limits) {
        active_power_setpoint.component_control_value.value_float = range_check(setpoint, max_limited_active_power, min_limited_active_power);
    } else {
        active_power_setpoint.component_control_value.value_float = range_check(setpoint, dischargeable_power.value.value_float, -1 * chargeable_power.value.value_float);
    }
}

bool Asset_ESS::send_reactive_power_setpoint(void) {
    if (reactive_power_setpoint_throttle.setpoint_trigger(reactive_power_setpoint.component_control_value.value_float)) {
        if (round(reactive_power_setpoint.component_control_value.value_float) != round(reactive_power_setpoint.value.value_float)) {
            return reactive_power_setpoint.send_to_component(false, true);
        }
    }
    return false;
}

void Asset_ESS::set_reactive_power_setpoint(float setpoint) {
    reactive_power_setpoint.component_control_value.value_float = range_check(setpoint, potential_reactive_power, -1.0f * potential_reactive_power);
}

bool Asset_ESS::send_power_factor_setpoint(void) {
    // Get 3 decimal-place precision
    float control_power_factor = ((float)((int)(power_factor_setpoint.component_control_value.value_float * 1000))) / 1000;
    float status_power_factor = ((float)((int)(power_factor_setpoint.value.value_float * 1000))) / 1000;

    if (control_power_factor != status_power_factor)  // This is comparing the power factor control setpoint to the power factor status setpoint but with 3 decimal-place precision
        return power_factor_setpoint.send_to_component();
    return false;
}

void Asset_ESS::set_power_factor_setpoint(float setpoint) {
    power_factor_setpoint.component_control_value.value_float = setpoint > 1.0 ? 1.0 : setpoint < -1.0 ? -1.0 : setpoint;
}

bool Asset_ESS::send_power_mode(void) {
    if (power_mode_setpoint.component_control_value.value_int != power_mode_setpoint.value.value_int)
        return power_mode_setpoint.send_to_component();
    return false;
}

void Asset_ESS::set_power_mode(powerMode mode) {
    if (mode == powerMode::REACTIVEPWR)
        power_mode_setpoint.component_control_value.value_int = reactive_power_mode_value;
    else if (mode == powerMode::PWRFCTR)
        power_mode_setpoint.component_control_value.value_int = power_factor_mode_value;
    else
        FPS_ERROR_LOG("Asset_ESS::set_power_mode received invalid mode.\n");
}

bool Asset_ESS::send_grid_mode(void) {
    if ((grid_mode_setpoint.component_control_value.value_int != grid_mode_setpoint.value.value_int) && grid_mode_setpoint.allow_set) {
        return grid_mode_setpoint.send_to_component();
    }
    return false;
}

void Asset_ESS::set_grid_mode(gridMode mode) {
    if (mode == gridMode::FOLLOWING) {
        grid_mode_setpoint.allow_set = true;
        grid_mode_setpoint.component_control_value.value_int = grid_following_value;
    } else if (mode == gridMode::FORMING) {
        grid_mode_setpoint.allow_set = true;
        grid_mode_setpoint.component_control_value.value_int = grid_forming_value;
    } else {
        FPS_ERROR_LOG("Asset_ESS::set_grid_mode received invalid mode.\n");
    }
}

bool Asset_ESS::send_pcs_nominal_voltage_setting(void) {
    bool status = false;
    if (pcs_a_nominal_voltage_setpoint.component_control_value.value_float != pcs_a_nominal_voltage_setpoint.value.value_float)
        status = pcs_a_nominal_voltage_setpoint.send_to_component();

    if (pcs_b_nominal_voltage_setpoint.component_control_value.value_float != pcs_b_nominal_voltage_setpoint.value.value_float)
        status |= pcs_b_nominal_voltage_setpoint.send_to_component();

    return status;
}

void Asset_ESS::set_pcs_nominal_voltage_setting(float setpoint) {
    pcs_a_nominal_voltage_setpoint.component_control_value.value_float = setpoint;
    pcs_b_nominal_voltage_setpoint.component_control_value.value_float = setpoint;
}

/**
 * @brief Set the variables used for ESS Calibration mode passed as a struct
 *
 * @param settings Struct containing all ESS instance level settings
 */
void Asset_ESS::set_calibration_vars(ESS_Calibration_Settings settings) {
    calibration_flag = settings.calibration_flag;
    soc_limits_flag = settings.soc_limits_flag;
    dischargeable_soc_limit = settings.min_soc_limit;
    chargeable_soc_limit = settings.max_soc_limit;
    cell_voltage_limits_flag = settings.cell_voltage_limits;
    rack_voltage_limits_flag = settings.rack_voltage_limits;
    dischargeable_cell_voltage_limit = settings.min_cell_voltage_limit;
    chargeable_cell_voltage_limit = settings.max_cell_voltage_limit;
    dischargeable_rack_voltage_limit = settings.min_rack_voltage_limit;
    chargeable_rack_voltage_limit = settings.max_rack_voltage_limit;
    raw_calibration_setpoint = settings.raw_feature_setpoint;

    if (!inMaintenance) {
        // the Calibration feature will send true when it's enabled and false when it's disabled
        soc_protection_buffers_disable_flag = settings.soc_protection_buffers_disable;
    } else {
        soc_protection_buffers_disable_flag = maint_soc_protection_buffers_disable_flag;
    }
}

bool Asset_ESS::close_bms_contactors(void) {
    return (send_to_comp_uri(bms_control_close, uri_close_dc_contacts));
}

bool Asset_ESS::open_bms_contactors(void) {
    return (send_to_comp_uri(bms_control_open, uri_open_dc_contacts));
}

gridMode Asset_ESS::get_grid_mode(void) const {
    if (grid_mode_setpoint.value.value_int == grid_following_value) {
        return gridMode::FOLLOWING;
    } 
    if (grid_mode_setpoint.value.value_int == grid_forming_value) {
        return gridMode::FORMING;
    }          
    return gridMode::UNDEFINED;
}

float Asset_ESS::get_voltage_slew_setpoint(void) {
    return voltage_slew_setpoint.value.value_float;
}

bool Asset_ESS::send_voltage_slew_setpoint(void) {
    // Get 3 decimal-place precision
    float control_voltage_slew = ((float)((int)(voltage_slew_setpoint.component_control_value.value_float * 1000))) / 1000;
    float status_voltage_slew = ((float)((int)(voltage_slew_setpoint.value.value_float * 1000))) / 1000;

    if (control_voltage_slew != status_voltage_slew)  // This is comparing control_setpoints.voltage_slew to status_setpoints.voltage_slew but with 3 decimal-place precision
        return voltage_slew_setpoint.send_to_component();
    return false;
}

void Asset_ESS::set_voltage_slew_setpoint(float setpoint) {
    voltage_slew_setpoint.component_control_value.value_float = setpoint;
}

/**
 * Configure ESS-specific variables that are provided for the asset instance level
 * @param configurator The Type_Configurator used to configure this asset
 * @return Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
Config_Validation_Result Asset_ESS::configure_typed_asset_instance_vars(Type_Configurator* configurator) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);
    Asset_Configurator* asset_config = &configurator->asset_config;

    // Set flag if component will be providing chargeable/dischargeable energy values via Modbus
    energy_configured = chargeable_energy_raw.get_component_uri() != NULL && dischargeable_energy_raw.get_component_uri() != NULL;

    cJSON* object = cJSON_GetObjectItem(asset_config->asset_instance_root, "system_rated_chargeable_power");
    rated_chargeable_power = object ? object->valuedouble : rated_active_power_kw;  // if not present, set to rated active power for backward compatibility

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "system_rated_dischargeable_power");
    rated_dischargeable_power = object ? object->valuedouble : rated_active_power_kw;  // if not present, set to rated active power for backward compatibility

    // asset configuration must include rated_capacity
    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "rated_capacity");
    if (object) {
        rated_capacity = object->valuedouble;
    } else if (configurator->config_validation) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: contains no rated_capacity.", name)));
    }

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "nominal_voltage");
    if (object)
        nominal_voltage = object->valuedouble;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "nominal_frequency");
    if (object)
        nominal_frequency = object->valuedouble;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "start_value");
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

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "standby_status_mask");
    // if the mask isn't NULL and is a hex string, try to parse it
    if (object && object->valuestring) {
        try {
            // Casts the provided hex string as an unsigned int64
            standby_status_mask = (uint64_t)std::stoul(object->valuestring, NULL, 16);
        } catch (...) {
            // Could not parse the provided mask
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: received an invalid standby_status_mask.", name)));
        }
    }
    // if the mask isn't null and isn't a hex string, throw an error
    else if (object) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: received an invalid input type instead of a hexadecimal string for the running_status_mask.", name)));
    }

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "DC_contactor_closed");
    if (object)
        bms_control_close = object->valueint;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "DC_contactor_open");
    if (object)
        bms_control_open = object->valueint;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "DC_contactor_reset");
    if (object)
        bms_control_reset = object->valueint;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "DC_contactor_restriction");
    if (object)
        dc_contactor_restriction = (object->type == cJSON_True);

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "chg_soc_begin");
    if (object)
        chgSocBegin = object->valuedouble;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "chg_soc_end");
    if (object)
        chgSocEnd = object->valuedouble;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "dischg_soc_begin");
    if (object)
        dischgSocBegin = object->valuedouble;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "dischg_soc_end");
    if (object)
        dischgSocEnd = object->valuedouble;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "min_raw_soc");
    if (object)
        minRawSoc = (float)object->valuedouble;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "max_raw_soc");
    if (object)
        maxRawSoc = (float)object->valuedouble;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "chargeable_min_limit_kW");
    if (object)
        chargeable_min_limit_kW = object->valuedouble;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "dischargeable_min_limit_kW");
    if (object)
        dischargeable_min_limit_kW = object->valuedouble;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "reactive_power_mode_q");
    if (object)
        reactive_power_mode_value = object->valueint;

    object = cJSON_GetObjectItem(asset_config->asset_instance_root, "reactive_power_mode_pf");
    if (object)
        power_factor_mode_value = object->valueint;

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
Config_Validation_Result Asset_ESS::configure_ui_controls(Type_Configurator* configurator) {
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

        Config_Validation_Result control_result;

        // when adding a new UI control, make sure to add it to the list of valid UI controls in Asset_Manager.cpp
        // TODO: Move these all to the base class (quick_config_slider quick_config_numeric quick_config_button)
        // SLIDERS
        Asset::quick_config_slider(ui_controls, "maint_mode", maint_mode, inMaintenance, name, validation_result);
        Asset::quick_config_slider(ui_controls, "lock_mode", lock_mode, inLockdown, name, validation_result);
        Asset::quick_config_slider(ui_controls, "maint_soc_protection_buffers_disable", maint_soc_protection_buffers_disable_ctl, maint_soc_protection_buffers_disable_flag, name, validation_result);
        Asset::quick_config_slider(ui_controls, "maint_soc_limits_enable", maint_soc_limits_enable_ctl, maint_soc_limits_enable_flag, name, validation_result);
        Asset::quick_config_slider(ui_controls, "maint_cell_voltage_limits_enable", maint_cell_voltage_limits_enable_ctl, maint_cell_voltage_limits_enable_flag, name, validation_result);
        Asset::quick_config_slider(ui_controls, "maint_rack_voltage_limits_enable", maint_rack_voltage_limits_enable_ctl, maint_rack_voltage_limits_enable_flag, name, validation_result);
        Asset::quick_config_slider(ui_controls, "maint_min_charge_discharge_enable", maint_min_charge_discharge_enable_ctl, maint_min_charge_discharge_enable_flag, name, validation_result);

        // NUMBERS
        Asset::quick_config_numeric(ui_controls, "maint_active_power_setpoint", maint_active_power_setpoint_ctl, maint_active_power_setpoint, name, validation_result);
        Asset::quick_config_numeric(ui_controls, "maint_reactive_power_setpoint", maint_reactive_power_setpoint_ctl, maint_reactive_power_setpoint, name, validation_result);
        Asset::quick_config_numeric(ui_controls, "maint_chargeable_min_limit", maint_chargeable_min_limit_ctl, maint_chargeable_min_limit, name, validation_result);
        Asset::quick_config_numeric(ui_controls, "maint_dischargeable_min_limit", maint_dischargeable_min_limit_ctl, maint_dischargeable_min_limit, name, validation_result);
        Asset::quick_config_numeric(ui_controls, "maint_min_soc_limit", maint_min_soc_limit_ctl, maint_min_soc_limit, name, validation_result);
        Asset::quick_config_numeric(ui_controls, "maint_max_soc_limit", maint_max_soc_limit_ctl, maint_max_soc_limit, name, validation_result);
        Asset::quick_config_numeric(ui_controls, "maint_min_cell_voltage_limit", maint_min_cell_voltage_limit_ctl, maint_min_cell_voltage_limit, name, validation_result);
        Asset::quick_config_numeric(ui_controls, "maint_max_cell_voltage_limit", maint_max_cell_voltage_limit_ctl, maint_max_cell_voltage_limit, name, validation_result);
        Asset::quick_config_numeric(ui_controls, "maint_min_rack_voltage_limit", maint_min_rack_voltage_limit_ctl, maint_min_rack_voltage_limit, name, validation_result);
        Asset::quick_config_numeric(ui_controls, "maint_max_rack_voltage_limit", maint_max_rack_voltage_limit_ctl, maint_max_rack_voltage_limit, name, validation_result);
        Asset::quick_config_numeric(ui_controls, "maint_active_power_slew_rate", maint_active_power_slew_rate_ctl, maint_slew_rate, name, validation_result);

        // BUTTONS
        Asset::quick_config_button(ui_controls, "clear_faults", clear_faults_ctl, compNames[i], uri_clear_faults, name, validation_result, true);  // use true here because resetOption
        Asset::quick_config_button(ui_controls, "start", start_ctl, compNames[i], uri_start, name, validation_result, false);
        Asset::quick_config_button(ui_controls, "stop", stop_ctl, compNames[i], uri_stop, name, validation_result, false);
        Asset::quick_config_button(ui_controls, "enter_standby", enter_standby_ctl, compNames[i], uri_enter_standby, name, validation_result, false);
        Asset::quick_config_button(ui_controls, "exit_standby", exit_standby_ctl, compNames[i], uri_exit_standby, name, validation_result, false);
        Asset::quick_config_button(ui_controls, "open_dc_contactors", open_dc_contactors_ctl, compNames[i], uri_open_dc_contacts, name, validation_result, false);
        Asset::quick_config_button(ui_controls, "close_dc_contactors", close_dc_contactors_ctl, compNames[i], uri_close_dc_contacts, name, validation_result, false);

        // If one of the autobalancing controls is provided, the other must be provided as well
        cJSON* enable_ctrl_obj = cJSON_GetObjectItem(ui_controls, "autobalancing_enable");
        cJSON* disable_ctrl_obj = cJSON_GetObjectItem(ui_controls, "autobalancing_disable");
        if ((enable_ctrl_obj != NULL) != (disable_ctrl_obj != NULL)) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: only received one autobalancing control: {}. Either remove this control or provide {} as well.", name,
                                                                                 enable_ctrl_obj ? "autobalancing_enable" : "autobalancing_disable", enable_ctrl_obj ? "autobalancing_disable" : "autobalancing_enable")));
        }

        // can use quick config for only one side
        quick_config_button(ui_controls, "autobalancing_enable", autobalancing_enable_ctl, compNames[i], uri_set_autobalancing, name, validation_result, false);

        // leaving below logic explicit because it's a bit novel
        if (disable_ctrl_obj != NULL) {
            control_result = autobalancing_disable_ctl.configure(disable_ctrl_obj, onOffOption, NULL, Bool, buttonStr, false);
            if (!control_result.is_valid_config) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure autobalancing_disable UI control.", name)));
            }
            if (enable_ctrl_obj != NULL) {
                // If both autobalancing registers were provided, ensure that their names match, as they should be writing to the same register
                if (strcmp(autobalancing_enable_ctl.reg_name, autobalancing_disable_ctl.reg_name) != 0) {
                    validation_result.is_valid_config = false;
                    validation_result.ERROR_details.push_back(
                        Result_Details(fmt::format("{}: mismatch between register names for autobalancing_enable and autobalancing_disable. The controls must write to the same register. Received enable register: {}, and disable register: {}", name,
                                                   autobalancing_enable_ctl.reg_name, autobalancing_disable_ctl.reg_name)));
                }
            }
            validation_result.absorb(control_result);
        }

        // ######## HANDLE ACTIONS HERE ########
        cJSON* maint_actions_ctl_obj = cJSON_GetObjectItem(ui_controls, "maint_actions_ctl");
        if (maint_actions_ctl_obj != nullptr) {
            control_result = maint_actions_select_ctl.configure_actions_UI(maint_actions_ctl_obj);
            if (!control_result.is_valid_config) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("Failed to configure actions. (maint_actions_ctl)")));
            }
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
 */
Config_Validation_Result Asset_ESS::configure_typed_asset_fims_vars(Type_Configurator* configurator) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    validation_result.absorb(configure_single_fims_var(&power_factor_setpoint, "power_factor_setpoint", configurator));
    validation_result.absorb(configure_single_fims_var(&voltage_slew_setpoint, "voltage_slew_setpoint", configurator));
    validation_result.absorb(configure_single_fims_var(&voltage_setpoint, "voltage_setpoint", configurator));
    validation_result.absorb(configure_single_fims_var(&frequency_setpoint, "frequency_setpoint", configurator));
    validation_result.absorb(configure_single_fims_var(&pcs_a_nominal_voltage_setpoint, "pcs_a_nominal_voltage_setting", configurator));
    validation_result.absorb(configure_single_fims_var(&pcs_b_nominal_voltage_setpoint, "pcs_b_nominal_voltage_setting", configurator));
    validation_result.absorb(configure_single_fims_var(&soc_raw, "soc", configurator));
    validation_result.absorb(configure_single_fims_var(&soh, "soh", configurator, Float, 100));
    validation_result.absorb(configure_single_fims_var(&max_temp, "max_temp", configurator));
    validation_result.absorb(configure_single_fims_var(&min_temp, "min_temp", configurator));
    validation_result.absorb(configure_single_fims_var(&chargeable_power_raw, "system_chargeable_power", configurator));
    validation_result.absorb(configure_single_fims_var(&dischargeable_power_raw, "system_dischargeable_power", configurator));
    validation_result.absorb(configure_single_fims_var(&chargeable_energy_raw, "system_chargeable_energy", configurator));
    validation_result.absorb(configure_single_fims_var(&dischargeable_energy_raw, "system_dischargeable_energy", configurator));
    validation_result.absorb(configure_single_fims_var(&power_mode_setpoint, "reactive_power_mode", configurator, Int, 0, REACTIVEPWR));
    validation_result.absorb(configure_single_fims_var(&racks_in_service, "racks_in_service", configurator, Int));
    validation_result.absorb(configure_single_fims_var(&dc_contactors_closed, "dc_contactors_closed", configurator, Bool));
    validation_result.absorb(configure_single_fims_var(&autobalancing_status, "autobalancing_status", configurator, Bool));
    validation_result.absorb(configure_single_fims_var(&min_cell_voltage, "min_cell_voltage", configurator, Bool));
    validation_result.absorb(configure_single_fims_var(&max_cell_voltage, "max_cell_voltage", configurator, Bool));
    validation_result.absorb(configure_single_fims_var(&min_rack_voltage, "min_rack_voltage", configurator, Bool));
    validation_result.absorb(configure_single_fims_var(&max_rack_voltage, "max_rack_voltage", configurator, Bool));
    validation_result.absorb(configure_single_fims_var(&status, "status", configurator, Status));

    // FOLLOWING here is actually invalid. grid_mode_setpoint will only be allowed to send setpoints AFTER.
    // A call to one of the grid baseds sequences functions have been called. "set_all_<asset>_form..."
    // TODO(JUD): could rip out and redo configuration to make this not confusing, but not deemed worth the effort atm
    validation_result.absorb(configure_single_fims_var(&grid_mode_setpoint, "grid_mode", configurator, Int, 0,
                FOLLOWING, false, true, "", "", 1, false)); // need the last one false to prevent set spam.

    return validation_result;
}

/**
 * Here is where the connection between asset and component var maps is severed, so that the raw values will still be sourced from
 * components, while the calculated values will be sourced from the asset's calculated variables
 */
Config_Validation_Result Asset_ESS::replace_typed_raw_fims_vars() {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    validation_result.absorb(configure_single_fims_var(&soc, "soc", NULL, Float, 0, 0, false, false, "State of Charge", "%"));
    validation_result.absorb(configure_single_fims_var(&chargeable_power, "system_chargeable_power", NULL, Float, 0, 0, false, false, "System Chargeable Power", "W", 1000));
    validation_result.absorb(configure_single_fims_var(&dischargeable_power, "system_dischargeable_power", NULL, Float, 0, 0, false, false, "System Dischargeable Power", "W", 1000));
    validation_result.absorb(configure_single_fims_var(&chargeable_energy, "system_chargeable_energy", NULL, Float, 0, 0, false, false, "System Chargeable Energy", "Wh", 1000));
    validation_result.absorb(configure_single_fims_var(&dischargeable_energy, "system_dischargeable_energy", NULL, Float, 0, 0, false, false, "System Dischargeable Energy", "Wh", 100));

    return validation_result;
}

// Todo: This function has a strange unconventional fims hierarchy. Usually there is 2 layers (body->value) this one has 3("metabody"->body->value). Might should figure out and change why this is the case.
// Todo: grab_naked... is a temporary fix. The real goal should be to do pure naked sets, but dbi expects clothed values so this function clothes naked sets before they are handed to dbi.
/* following is called when asset manager receives a FIMS message of method "set"  */
bool Asset_ESS::handle_set(std::string uri, cJSON& body) {
    // The current setpoint being parsed from those available
    cJSON* current_setpoint = NULL;

    // The value of the setpoint object received
    cJSON* value = NULL;

    // Whether the setpoint is supported by persistent settings
    // For instance, sets that modify the system state should not persist as they will default to the published component state on restart
    bool persistent_setpoint = false;

    if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "clear_faults")) != nullptr) {
        clear_alerts();
    } else if (((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "stop")) != nullptr) && inMaintenance) {
        stop();
    } else if (((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "start")) != nullptr) && inMaintenance) {
        start();
    } else if (((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "enter_standby")) != nullptr) && inMaintenance) {
        enter_standby();
    } else if (((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "exit_standby")) != nullptr) && inMaintenance) {
        exit_standby();
    } else if ((current_setpoint = grab_naked_or_clothed(body, current_setpoint, "soc_protection_buffers_disable")) != nullptr) {
        if ((value = cJSON_GetObjectItem(current_setpoint, "value")) != nullptr) {
            soc_protection_buffers_disable_flag = static_cast<bool>(value->valueint);
            persistent_setpoint = true;
        }
    } else if ((current_setpoint = grab_naked_or_clothed(body, current_setpoint, "maint_soc_protection_buffers_disable")) != nullptr) {
        if (((value = cJSON_GetObjectItem(current_setpoint, "value")) != nullptr) && inMaintenance) {
            maint_soc_protection_buffers_disable_flag = static_cast<bool>(value->valueint);
            persistent_setpoint = true;
        }
    } else if ((current_setpoint = grab_naked_or_clothed(body, current_setpoint, "maint_soc_limits_enable")) != nullptr) {
        if (((value = cJSON_GetObjectItem(current_setpoint, "value")) != nullptr) && inMaintenance) {
            maint_soc_limits_enable_flag = static_cast<bool>(value->valueint);
            persistent_setpoint = true;
        }
    } else if ((current_setpoint = grab_naked_or_clothed(body, current_setpoint, "maint_cell_voltage_limits_enable")) != nullptr) {
        if (((value = cJSON_GetObjectItem(current_setpoint, "value")) != nullptr) && inMaintenance) {
            maint_cell_voltage_limits_enable_flag = static_cast<bool>(value->valueint);
            persistent_setpoint = true;
        }
    } else if ((current_setpoint = grab_naked_or_clothed(body, current_setpoint, "maint_rack_voltage_limits_enable")) != nullptr) {
        if (((value = cJSON_GetObjectItem(current_setpoint, "value")) != nullptr) && inMaintenance) {
            maint_rack_voltage_limits_enable_flag = static_cast<bool>(value->valueint);
            persistent_setpoint = true;
        }
    } else if ((current_setpoint = grab_naked_or_clothed(body, current_setpoint, "maint_min_charge_discharge_enable")) != nullptr) {
        if (((value = cJSON_GetObjectItem(current_setpoint, "value")) != nullptr) && inMaintenance) {
            maint_min_charge_discharge_enable_flag = static_cast<bool>(value->valueint);
            persistent_setpoint = true;
        }
    } else if ((current_setpoint = grab_naked_or_clothed(body, current_setpoint, "maint_soc_limits_enable")) != nullptr) {
        if (((value = cJSON_GetObjectItem(current_setpoint, "value")) != nullptr) && inMaintenance) {
            maint_soc_limits_enable_flag = static_cast<bool>(value->valueint);
            persistent_setpoint = true;
        }
    } else if ((current_setpoint = grab_naked_or_clothed(body, current_setpoint, "maint_min_charge_discharge_enable")) != nullptr) {
        if (((value = cJSON_GetObjectItem(current_setpoint, "value")) != nullptr) && inMaintenance) {
            maint_min_charge_discharge_enable_flag = static_cast<bool>(value->valueint);
            persistent_setpoint = true;
        }
    } else if (((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "autobalancing_enable")) != nullptr) && inMaintenance) {
        // Write the autobalancing status (enabled) to the component register
        // Since enable and disable share the same register, a single function will write for both controls
        set_autobalancing(true);
    } else if (((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "autobalancing_disable")) != nullptr) && inMaintenance) {
        // Write the autobalancing status (enabled) to the component register
        // Since enable and disable share the same register, a single function will write for both controls
        set_autobalancing(false);
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_Number, "maint_active_power_setpoint")) != nullptr) {
        maint_active_power_setpoint = cJSON_GetObjectItem(current_setpoint, "value")->valuedouble;
        persistent_setpoint = true;
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_Number, "maint_reactive_power_setpoint")) != nullptr) {
        maint_reactive_power_setpoint = cJSON_GetObjectItem(current_setpoint, "value")->valuedouble;
        persistent_setpoint = true;
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_Number, "maint_chargeable_min_limit")) != nullptr) {
        maint_chargeable_min_limit = cJSON_GetObjectItem(current_setpoint, "value")->valuedouble;
        persistent_setpoint = true;
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_Number, "maint_dischargeable_min_limit")) != nullptr) {
        maint_dischargeable_min_limit = cJSON_GetObjectItem(current_setpoint, "value")->valuedouble;
        persistent_setpoint = true;
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_Number, "maint_min_soc_limit")) != nullptr) {
        maint_min_soc_limit = cJSON_GetObjectItem(current_setpoint, "value")->valuedouble;
        persistent_setpoint = true;
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_Number, "maint_max_soc_limit")) != nullptr) {
        maint_max_soc_limit = cJSON_GetObjectItem(current_setpoint, "value")->valuedouble;
        persistent_setpoint = true;
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_Number, "maint_min_cell_voltage_limit")) != nullptr) {
        maint_min_cell_voltage_limit = cJSON_GetObjectItem(current_setpoint, "value")->valuedouble;
        persistent_setpoint = true;
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_Number, "maint_max_cell_voltage_limit")) != nullptr) {
        maint_max_cell_voltage_limit = cJSON_GetObjectItem(current_setpoint, "value")->valuedouble;
        persistent_setpoint = true;
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_Number, "maint_min_rack_voltage_limit")) != nullptr) {
        maint_min_rack_voltage_limit = cJSON_GetObjectItem(current_setpoint, "value")->valuedouble;
        persistent_setpoint = true;
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_Number, "maint_max_rack_voltage_limit")) != nullptr) {
        maint_max_rack_voltage_limit = cJSON_GetObjectItem(current_setpoint, "value")->valuedouble;
        persistent_setpoint = true;
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_Number, "maint_active_power_slew_rate")) != nullptr) {
        maint_slew_rate = cJSON_GetObjectItem(current_setpoint, "value")->valuedouble;
        persistent_setpoint = true;
        if (inMaintenance) {
            active_power_slew.set_slew_rate(maint_slew_rate);
        }
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "open_dc_contactors")) != nullptr) {
        open_bms_contactors();
    } else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "close_dc_contactors")) != nullptr) {
        close_bms_contactors();
    }

    // if target setpoint was found, back it up to DBI if it is a persistent setpoint.
    // otherwise, send it to the generic controls handler
    if (current_setpoint == nullptr) {
        return handle_generic_asset_controls_set(uri, body);
    }

    if (!persistent_setpoint) {
        return true;
    }

    return Asset::send_setpoint(uri, current_setpoint);
}

/****************************************************************************************/
// Variable map moved to ESS_Manager
bool Asset_ESS::generate_asset_ui(fmt::memory_buffer& buf, const char* const var) {
    bool goodBody = true;
    // Only check that contactors are closed if the restriction has been configured, else default true
    bool valid_contactors_state = dc_contactor_restriction ? dc_contactors_closed.value.value_bool : true;

    // add the lockdown mode control
    lock_mode.enabled = inMaintenance;
    goodBody = lock_mode.makeJSONObject(buf, var, true) && goodBody;

    // add the manual mode control
    maint_mode.enabled = !inLockdown;
    goodBody = maint_mode.makeJSONObject(buf, var, true) && goodBody;

    // TODO: make clear faults in base class
    clear_faults_ctl.enabled = (get_num_active_faults() != 0 || get_num_active_alarms() != 0);
    goodBody = clear_faults_ctl.makeJSONObject(buf, var, true) && goodBody;

    maint_soc_protection_buffers_disable_ctl.enabled = inMaintenance;
    goodBody = maint_soc_protection_buffers_disable_ctl.makeJSONObject(buf, var, true) && goodBody;

    maint_soc_limits_enable_ctl.enabled = inMaintenance;
    goodBody = maint_soc_limits_enable_ctl.makeJSONObject(buf, var, true) && goodBody;

    maint_cell_voltage_limits_enable_ctl.enabled = inMaintenance;
    goodBody = maint_cell_voltage_limits_enable_ctl.makeJSONObject(buf, var, true) && goodBody;

    maint_rack_voltage_limits_enable_ctl.enabled = inMaintenance;
    goodBody = maint_rack_voltage_limits_enable_ctl.makeJSONObject(buf, var, true) && goodBody;

    maint_min_charge_discharge_enable_ctl.enabled = inMaintenance;
    goodBody = maint_min_charge_discharge_enable_ctl.makeJSONObject(buf, var, true) && goodBody;

    maint_soc_limits_enable_ctl.enabled = inMaintenance;
    goodBody = maint_soc_limits_enable_ctl.makeJSONObject(buf, var, true) && goodBody;

    maint_min_charge_discharge_enable_ctl.enabled = inMaintenance;
    goodBody = maint_min_charge_discharge_enable_ctl.makeJSONObject(buf, var, true) && goodBody;

    // Start only if running (not standby)
    start_ctl.enabled = (inMaintenance && !isRunning && !inStandby && valid_contactors_state && !is_in_local_mode());
    goodBody = start_ctl.makeJSONObject(buf, var, true) && goodBody;

    // Stop if in running or standby, no contactor restriction
    stop_ctl.enabled = (inMaintenance && (isRunning || inStandby) && !is_in_local_mode());
    goodBody = stop_ctl.makeJSONObject(buf, var, true) && goodBody;

    // Enter standby only if running and not in standby and contactors closed
    // TODO: setpoint requirements? Require the user to set them to 0 or set them to 0 automatically?
    enter_standby_ctl.enabled = (inMaintenance && (isRunning && !inStandby) && (active_power_setpoint.value.value_float == 0) && (reactive_power_setpoint.value.value_float == 0) && valid_contactors_state && !is_in_local_mode());
    goodBody = enter_standby_ctl.makeJSONObject(buf, var, true) && goodBody;

    // Exit standby only if in standby and contactors closed
    exit_standby_ctl.enabled = (inMaintenance && inStandby && valid_contactors_state && !is_in_local_mode());
    goodBody = exit_standby_ctl.makeJSONObject(buf, var, true) && goodBody;

    // Autobalancing will work in any state we track (running, standby, stopped)
    autobalancing_enable_ctl.enabled = inMaintenance && !autobalancing_status.value.value_bool && !is_in_local_mode();
    goodBody = autobalancing_enable_ctl.makeJSONObject(buf, var, true) && goodBody;

    // Autobalancing will work in any state we track (running, standby, stopped)
    autobalancing_disable_ctl.enabled = inMaintenance && autobalancing_status.value.value_bool && !is_in_local_mode();
    goodBody = autobalancing_disable_ctl.makeJSONObject(buf, var, true) && goodBody;

    maint_active_power_setpoint_ctl.enabled = inMaintenance && isRunning && !is_in_local_mode();
    goodBody = maint_active_power_setpoint_ctl.makeJSONObject(buf, var, true) && goodBody;

    maint_reactive_power_setpoint_ctl.enabled = inMaintenance && isRunning && !is_in_local_mode();
    goodBody = maint_reactive_power_setpoint_ctl.makeJSONObject(buf, var, true) && goodBody;

    // lock these variables behind the maint_min_charge_discharge_enable_flag
    maint_chargeable_min_limit_ctl.enabled = inMaintenance && maint_min_charge_discharge_enable_flag;
    goodBody = maint_chargeable_min_limit_ctl.makeJSONObject(buf, var, true) && goodBody;

    maint_dischargeable_min_limit_ctl.enabled = inMaintenance && maint_min_charge_discharge_enable_flag;
    goodBody = maint_dischargeable_min_limit_ctl.makeJSONObject(buf, var, true) && goodBody;
    // end lock

    // lock these variables behind the maint_soc_limits_enable_flag
    maint_min_soc_limit_ctl.enabled = inMaintenance && maint_soc_limits_enable_flag;
    goodBody = maint_min_soc_limit_ctl.makeJSONObject(buf, var, true) && goodBody;

    maint_max_soc_limit_ctl.enabled = inMaintenance && maint_soc_limits_enable_flag;
    goodBody = maint_max_soc_limit_ctl.makeJSONObject(buf, var, true) && goodBody;
    // end lock

    // lock these variables behind the maint_cell_voltage_limits_enable_flag
    maint_min_cell_voltage_limit_ctl.enabled = inMaintenance && maint_cell_voltage_limits_enable_flag;
    goodBody = maint_min_cell_voltage_limit_ctl.makeJSONObject(buf, var, true) && goodBody;

    maint_max_cell_voltage_limit_ctl.enabled = inMaintenance && maint_cell_voltage_limits_enable_flag;
    goodBody = maint_max_cell_voltage_limit_ctl.makeJSONObject(buf, var, true) && goodBody;
    // end lock

    // lock these variables behind the maint_rack_voltage_limits_enable_flag
    maint_min_rack_voltage_limit_ctl.enabled = inMaintenance && maint_rack_voltage_limits_enable_flag;
    goodBody = maint_min_rack_voltage_limit_ctl.makeJSONObject(buf, var, true) && goodBody;

    maint_max_rack_voltage_limit_ctl.enabled = inMaintenance && maint_rack_voltage_limits_enable_flag;
    goodBody = maint_max_rack_voltage_limit_ctl.makeJSONObject(buf, var, true) && goodBody;
    // end lock

    // Open contactors only if stopped (not faulted) and contactors are closed
    open_dc_contactors_ctl.enabled = inMaintenance && !isRunning && !inStandby && dc_contactors_closed.value.value_bool && !is_in_local_mode();
    goodBody = open_dc_contactors_ctl.makeJSONObject(buf, var, true) && goodBody;

    // Close contactors only if stopped (not faulted) and contactors are not closed
    close_dc_contactors_ctl.enabled = inMaintenance && !isRunning && !inStandby && !dc_contactors_closed.value.value_bool && !is_in_local_mode();
    goodBody = close_dc_contactors_ctl.makeJSONObject(buf, var, true) && goodBody;

    maint_active_power_slew_rate_ctl.enabled = inMaintenance && isRunning && !is_in_local_mode();
    goodBody = maint_active_power_slew_rate_ctl.makeJSONObject(buf, var, true) && goodBody;

    // Only need to maintain the maint_actions_select_ctl if there are any actions
    // (TODO: JUD) Might could get away with not pubbing this info at all.
    maint_actions_select_ctl.enabled = inMaintenance && action_status.current_sequence_name.empty();
    goodBody = maint_actions_select_ctl.makeJSONObjectWithActionOptions(buf, var, actions);

    return (goodBody);
}

/**
 * Update the asset status with the measurement received on the status Fims_Object
 */
void Asset_ESS::set_raw_status() {
    raw_status = status.value.value_bit_field;
}

/**
 * Get the string representation of this asset's status
 */
const char* Asset_ESS::get_status_string() const {
    return status.get_status_string();
}

/**
 * Return the status register
 */
Fims_Object& Asset_ESS::get_status_register() {
    return status;
}

/**
 * Update the Asset instance level data with values received from publish
 */
void Asset_ESS::process_asset() {
    set_raw_status();
    Asset::process_asset();
    if (!actions.empty()) {
        Asset::process_asset_actions();
    }

    // Local variables used for calculation to preserve values configured
    float _minRawSoc = minRawSoc;
    float _maxRawSoc = maxRawSoc;

    // Override soc if specified by the UI
    if (soc_protection_buffers_disable_flag) {
        // Remove soc scaling
        _maxRawSoc = 100.0f;
        _minRawSoc = 0.0f;
        soc.value.value_float = soc_raw.value.value_float;
    }
    // Else, derate soc
    else
        soc.value.value_float = process_soc(soc_raw.value.value_float);

    // limit chargeable power as soc reaches high end of range
    if (!soc_protection_buffers_disable_flag && greater_than_or_near(soc.value.value_float, chgSocBegin, 0.001f)) {
        float max_limit = 0.0f;
        if (inMaintenance && maint_min_charge_discharge_enable_flag)
            max_limit = maint_chargeable_min_limit;
        else
            max_limit = chargeable_min_limit_kW;
        if (greater_than_or_near(soc.value.value_float, 100.0f, 0.001f) || greater_than_or_near(soc.value.value_float, chgSocEnd, 0.001f) || chgSocBegin >= chgSocEnd) {
            chargeable_power.value.value_float = max_limit;
        } else {
            // Ensure chargeable power is at least as large as its limit if derated or capped here
            chargeable_power.value.value_float = std::max((chgSocEnd - soc.value.value_float) / (chgSocEnd - chgSocBegin) * rated_chargeable_power, max_limit);
        }
        // Ensure it does not exceed the component limit
        chargeable_power.value.value_float = std::min(chargeable_power.value.value_float, fabsf(chargeable_power_raw.value.value_float));
    } else
        chargeable_power.value.value_float = fabsf(chargeable_power_raw.value.value_float);

    // limit dischargeable power as soc reaches low end of range
    if (!soc_protection_buffers_disable_flag && less_than_or_near(soc.value.value_float, dischgSocBegin, 0.001f)) {
        float min_limit = 0.0f;
        if (inMaintenance && maint_min_charge_discharge_enable_flag)
            min_limit = maint_dischargeable_min_limit;
        else
            min_limit = dischargeable_min_limit_kW;
        if (less_than_or_near(soc.value.value_float, 0.0f, 0.001f) || less_than_or_near(soc.value.value_float, dischgSocEnd, 0) || dischgSocBegin <= dischgSocEnd) {
            dischargeable_power.value.value_float = min_limit;
        } else {
            // Ensure dischargeable power is at least as large as its limit if derated or capped here
            dischargeable_power.value.value_float = std::max((dischgSocEnd - soc.value.value_float) / (dischgSocEnd - dischgSocBegin) * rated_dischargeable_power, min_limit);
        }
        // Ensure it does not exceed the component limit
        dischargeable_power.value.value_float = std::min(dischargeable_power.value.value_float, fabsf(dischargeable_power_raw.value.value_float));
    } else
        dischargeable_power.value.value_float = fabsf(dischargeable_power_raw.value.value_float);

    // limit chargeable and dischargeable power based on rated power
    chargeable_power.value.value_float = (chargeable_power.value.value_float > rated_chargeable_power) ? rated_chargeable_power : chargeable_power.value.value_float;
    dischargeable_power.value.value_float = (dischargeable_power.value.value_float > rated_dischargeable_power) ? rated_dischargeable_power : dischargeable_power.value.value_float;

    // Apply calibration mode limits
    if (calibration_flag) {
        // Limit (dis)chargeable power to 0 if soc or voltage are beyond their acceptable thresholds
        if ((soc_limits_flag && soc.value.value_float >= chargeable_soc_limit) || (cell_voltage_limits_flag && max_cell_voltage.value.value_float >= chargeable_cell_voltage_limit) ||
            (rack_voltage_limits_flag && max_rack_voltage.value.value_float >= chargeable_rack_voltage_limit)) {
            chargeable_power.value.set(0.0f);
        }
        if ((soc_limits_flag && soc.value.value_float <= dischargeable_soc_limit) || (cell_voltage_limits_flag && min_cell_voltage.value.value_float <= dischargeable_cell_voltage_limit) ||
            (rack_voltage_limits_flag && min_rack_voltage.value.value_float <= dischargeable_rack_voltage_limit)) {
            dischargeable_power.value.set(0.0f);
        }

        // Determine setpoint status based on intended command passed through from Site Manager and available (dis)chargeable power
        if ((raw_calibration_setpoint < 0.0f && near(chargeable_power.value.value_float, 0.0f, 0.001)) || (raw_calibration_setpoint > 0.0f && near(dischargeable_power.value.value_float, 0.0f, 0.001))) {
            setpoint_status = ZERO;
        } else if ((raw_calibration_setpoint < -1.0f * chargeable_power.value.value_float) || (raw_calibration_setpoint > dischargeable_power.value.value_float)) {
            setpoint_status = LIMITED;
        } else {
            setpoint_status = ACCEPTED;
        }
    }

    // Use the maint_mode soc limits if inMaintenance and in use
    if (inMaintenance) {
        // Limit (dis)chargeable power to 0 if soc or voltage are beyond their acceptable thresholds
        if ((maint_soc_limits_enable_flag && soc.value.value_float >= maint_max_soc_limit) || (maint_cell_voltage_limits_enable_flag && max_cell_voltage.value.value_float >= maint_max_cell_voltage_limit) ||
            (maint_rack_voltage_limits_enable_flag && max_rack_voltage.value.value_float >= maint_max_rack_voltage_limit)) {
            chargeable_power.value.set(0.0f);
        }
        if ((maint_soc_limits_enable_flag && soc.value.value_float <= maint_min_soc_limit) || (maint_cell_voltage_limits_enable_flag && min_cell_voltage.value.value_float <= maint_min_cell_voltage_limit) ||
            (maint_rack_voltage_limits_enable_flag && min_rack_voltage.value.value_float <= maint_min_rack_voltage_limit)) {
            dischargeable_power.value.set(0.0f);
        }
    }

    // No component values provided, use old formula
    if (!energy_configured) {
        chargeable_energy.value.value_float = rated_capacity * (soh.value.value_float / 100) * (1 - (soc.value.value_float / 100)) * ((_maxRawSoc / 100) - (_minRawSoc / 100));
        dischargeable_energy.value.value_float = rated_capacity * (soh.value.value_float / 100) * (soc.value.value_float / 100) * ((_maxRawSoc / 100) - (_minRawSoc / 100));
    }
    // energy values provided, use new formula
    else {
        // Less than min derate case
        if (chargeable_energy_raw.value.value_float < (100 - _maxRawSoc) / 100 * rated_capacity)
            chargeable_energy.value.value_float = 0;
        // Greater than max derate case
        else if (chargeable_energy_raw.value.value_float > (100 - _minRawSoc) / 100 * rated_capacity)
            chargeable_energy.value.value_float = (_maxRawSoc - _minRawSoc) / 100 * rated_capacity;
        // Default case
        else
            chargeable_energy.value.value_float = chargeable_energy_raw.value.value_float - rated_capacity * (100 - _maxRawSoc) / 100;

        // Less than min derate case
        if (dischargeable_energy_raw.value.value_float < _minRawSoc / 100 * rated_capacity)
            dischargeable_energy.value.value_float = 0;
        // Greater than max derate case
        else if (dischargeable_energy_raw.value.value_float > _maxRawSoc / 100 * rated_capacity)
            dischargeable_energy.value.value_float = (_maxRawSoc - _minRawSoc) / 100 * rated_capacity;
        // Default case
        else
            dischargeable_energy.value.value_float = dischargeable_energy_raw.value.value_float - rated_capacity * _minRawSoc / 100;
    }

    // Process unique ESS standby state
    if (status.get_status_type() == random_enum)
        inStandby = internal_status == standby_status_mask;
    else if (status.get_status_type() == bit_field)
        // The internal status parsed from the component publish will already be bit shifted
        // Theoretically, multiple values can be set, so check if any of them are true
        // e.g. valid standby states: 4, 5; mask (binary): 110000 (start counting from 0)
        // for status value 4, verify: 110000 & 010000
        inStandby = static_cast<bool>(standby_status_mask & internal_status);
}

void Asset_ESS::update_asset(void) {
    // tick the clock
    clock_gettime(CLOCK_MONOTONIC, &action_status.current_time);
    // override setpoints in maintenance mode
    if (inMaintenance) {
        set_power_mode(powerMode::REACTIVEPWR);  // only allow reactive power mode while in maintenance

        if (this->maint_active_power_slew_rate_ctl.configured) {
            auto slewed_cmd = maint_active_power_setpoint;
            // Essentially this is 
            // slewed_cmd = min_limited_active_power < maint_active_power_setpoint < max_limited_active_power;
            slewed_cmd = std::max(slewed_cmd, this->min_limited_active_power);
            slewed_cmd = std::min(slewed_cmd, this->max_limited_active_power);
            set_active_power_setpoint(slewed_cmd, true); // <-- WE HAVE A RAW SET HERE UNLIMITED. WE SLAM.
        } else {
            set_active_power_setpoint(maint_active_power_setpoint, true); // <-- lEGACY BEHAVIOR
        }
        set_reactive_power_setpoint(maint_reactive_power_setpoint);
    }
}

void Asset_ESS::send_to_components(void) {
    if (is_in_local_mode())
        return;

    if ((get_num_active_faults() != 0) && isRunning)
        stop();

    send_grid_mode();
    send_power_mode();

    send_active_power_setpoint();
    send_reactive_power_setpoint();
    send_power_factor_setpoint();

    send_voltage_slew_setpoint();
    send_voltage_setpoint();
    send_frequency_setpoint();

    send_pcs_nominal_voltage_setting();
}

void Asset_ESS::process_potential_active_power(void)  // overriden from the base class, called every 100ms
{
    // Base active power limit reference
    active_power_limit = rated_active_power_kw;

    active_power_slew.update_slew_target(active_power_setpoint.component_control_value.value_float);

    // max value is capped based on dischargeable power (implicitly bounded by rated power)
    max_potential_active_power = active_power_slew.get_max_target();
    max_potential_active_power = (max_potential_active_power > dischargeable_power.value.value_float) ? dischargeable_power.value.value_float : max_potential_active_power;

    // min value is capped based on chargeable power, reported as a positive value (implicitly bounded by rated power)
    min_potential_active_power = active_power_slew.get_min_target();
    min_potential_active_power = (min_potential_active_power < fabsf(chargeable_power.value.value_float) * -1.0) ? fabsf(chargeable_power.value.value_float) * -1.0 : min_potential_active_power;

    // Further limit based on reactive power and apparent power if appropriate
    if (reactive_power_priority) {
        // TODO: revisit if (dis)chargeable should be limited as well. Any limits set here get overwritten in the process_asset step
        active_power_limit = sqrtf(powf(rated_apparent_power_kva, 2) - powf(reactive_power_setpoint.value.value_float, 2));
        min_potential_active_power = std::max(min_potential_active_power, -1.0f * active_power_limit);
        max_potential_active_power = std::min(max_potential_active_power, active_power_limit);
    }
    min_limited_active_power = min_potential_active_power;
    max_limited_active_power = max_potential_active_power;
}

float Asset_ESS::process_soc(float rawSoc) {
    float maxRaw = maxRawSoc;
    float minRaw = minRawSoc;
    float maxScl = 100;
    float scaledSocInternal;

    float temp = (rawSoc - minRaw);     // scale 0 to 97
    temp /= (maxRaw - minRaw);          // scale 0 to 1
    scaledSocInternal = temp * maxScl;  // scale 0 to 100

    // limit value
    if (scaledSocInternal > 100)
        scaledSocInternal = 100;
    if (scaledSocInternal < 0)
        scaledSocInternal = 0;

    return (scaledSocInternal);
}

/**
 * @brief Call ESS specific asset functions coming from actions.json entry and exit conditions.
 *
 * @param cmd (const char*) the function name.
 * @param value (Value_Object*) the expected return value.
 * @param tolerance_percent (int) allows for a tolerance percent.
 * @return success (bool)
 */
bool Asset_ESS::call_action_functions(const char* cmd, Value_Object* value, int tolerance_percent) {
    bool command_found = true;
    bool return_value = true;

    // this cmd will pass and move to next step
    if (strcmp(cmd, "bypass") == 0) {
        return true;
        // this cmd will move to next path (requires step to have path switch
    }
    if (strcmp(cmd, "new_path") == 0) {
        return false;
        // all generic boolean and floats for site-specific configuration
    }

    // ##### Here are the setters #####
    if (strcmp(cmd, "start") == 0) {
        start();
    } else if (strcmp(cmd, "stop") == 0) {
        stop();
    } else if (strcmp(cmd, "set_maint_mode") == 0) {
        FPS_DEBUG_LOG("You have reached the set_maint_mode function for ess in asset.cpp");
        if (inMaintenance != value->value_bool) {
            inMaintenance = value->value_bool;
            if (maint_active_power_slew_rate_ctl.configured) {
                if (inMaintenance) {
                    this->active_power_slew.set_slew_rate(this->maint_slew_rate);
                } else {
                    this->active_power_slew.set_slew_rate(this->slew_rate);
                }
            }
        }
    } else if (strcmp(cmd, "set_maint_active_power_setpoint") == 0) {
        FPS_DEBUG_LOG("You have reached the set_maint_active_power function for ess in asset.cpp");
        maint_active_power_setpoint = value->value_float;
    } else if (strcmp(cmd, "set_maint_reactive_power_setpoint") == 0) {
        FPS_DEBUG_LOG("You have reached the set_maint_reactive_power function for ess in asset.cpp");
        maint_reactive_power_setpoint = value->value_float;
    } else if (strcmp(cmd, "set_maint_chargeable_min_limit") == 0) {
        FPS_DEBUG_LOG("You have reached the set_maint_chargeable_min_limit function for ess in asset.cpp");
        maint_chargeable_min_limit = value->value_float;
    } else if (strcmp(cmd, "set_maint_dischargeable_min_limit") == 0) {
        FPS_DEBUG_LOG("You have reached the set_maint_dischargeable_min_limit function for ess in asset.cpp");
        maint_dischargeable_min_limit = value->value_float;
    } else if (strcmp(cmd, "set_maint_min_soc_limit") == 0) {
        FPS_DEBUG_LOG("You have reached the set_maint_min_soc_limit function for ess in asset.cpp");
        maint_min_soc_limit = value->value_float;
    } else if (strcmp(cmd, "set_maint_max_soc_limit") == 0) {
        FPS_DEBUG_LOG("You have reached the set_maint_max_soc_limit function for ess in asset.cpp");
        maint_max_soc_limit = value->value_float;
    } else if (strcmp(cmd, "set_maint_soc_protection_buffers_disable_flag") == 0) {
        FPS_DEBUG_LOG("You have reached the set_maint_soc_protection_buffers_disable_flag function for ess in asset.cpp");
        maint_soc_protection_buffers_disable_flag = value->value_bool;
    } else if (strcmp(cmd, "set_maint_soc_limits_enable_flag") == 0) {
        FPS_DEBUG_LOG("You have reached the set_maint_soc_limits_enable_flag function for ess in asset.cpp");
        maint_soc_limits_enable_flag = value->value_bool;
    } else if (strcmp(cmd, "set_maint_cell_voltage_limits_enable_flag") == 0) {
        FPS_DEBUG_LOG("You have reached the set_maint_cell_voltage_limits_enable_flag function for ess in asset.cpp");
        maint_cell_voltage_limits_enable_flag = value->value_bool;
    } else if (strcmp(cmd, "set_maint_rack_voltage_limits_enable_flag") == 0) {
        FPS_DEBUG_LOG("You have reached the set_maint_rack_voltage_limits_enable_flag function for ess in asset.cpp");
        maint_rack_voltage_limits_enable_flag = value->value_bool;
    } else if (strcmp(cmd, "set_maint_min_charge_discharge_enable_flag") == 0) {
        FPS_DEBUG_LOG("You have reached the set_maint_min_charge_discharge_enable_flag function for ess in asset.cpp");
        maint_min_charge_discharge_enable_flag = value->value_bool;
    } else if (strcmp(cmd, "set_maint_min_cell_voltage_limit") == 0) {
        FPS_DEBUG_LOG("You have reached the set_maint_min_cell_voltage_limit function for ess in asset.cpp");
        maint_min_cell_voltage_limit = value->value_float;
    } else if (strcmp(cmd, "set_maint_max_cell_voltage_limit") == 0) {
        maint_max_cell_voltage_limit = value->value_float;
    } else if (strcmp(cmd, "set_maint_min_rack_voltage_limit") == 0) {
        FPS_DEBUG_LOG("You have reached the set_maint_min_rack_voltage_limit function for ess in asset.cpp");
        maint_min_rack_voltage_limit = value->value_float;
    } else if (strcmp(cmd, "set_maint_max_rack_voltage_limit") == 0) {
        FPS_DEBUG_LOG("You have reached the set_maint_max_rack_voltage_limit function for ess in asset.cpp");
        maint_max_rack_voltage_limit = value->value_float;
    }

    // ##### Here are the getters #####
    else if (strcmp(cmd, "get_maint_active_power_setpoint") == 0) {
        FPS_DEBUG_LOG("You have reached the get_maint_active_power_setpoint function for ess in asset.cpp");
        return_value = ((value->type == Float) && get_tolerance(value->value_float, maint_active_power_setpoint, tolerance_percent));
    } else if (strcmp(cmd, "get_maint_reactive_power_setpoint") == 0) {
        FPS_DEBUG_LOG("You have reached the get_maint_reactive_power function for ess in asset.cpp");
        return_value = ((value->type == Float) && get_tolerance(value->value_float, maint_reactive_power_setpoint, tolerance_percent));
    } else if (strcmp(cmd, "get_maint_chargeable_min_limit") == 0) {
        FPS_DEBUG_LOG("You have reached the set_maint_chargeable_min_limit function for ess in asset.cpp");
        return_value = ((value->type == Float) && get_tolerance(value->value_float, maint_chargeable_min_limit, tolerance_percent));
    } else if (strcmp(cmd, "get_maint_dischargeable_min_limit") == 0) {
        FPS_DEBUG_LOG("You have reached the get_maint_dischargeable_min_limit function for ess in asset.cpp");
        return_value = ((value->type == Float) && get_tolerance(value->value_float, maint_dischargeable_min_limit, tolerance_percent));
    } else if (strcmp(cmd, "get_maint_min_soc_limit") == 0) {
        FPS_DEBUG_LOG("You have reached the get_maint_min_soc_limit function for ess in asset.cpp");
        return_value = ((value->type == Float) && get_tolerance(value->value_float, maint_min_soc_limit, tolerance_percent));
    } else if (strcmp(cmd, "get_maint_max_soc_limit") == 0) {
        FPS_DEBUG_LOG("You have reached the get_maint_max_soc_limit function for ess in asset.cpp");
        return_value = ((value->type == Float) && get_tolerance(value->value_float, maint_max_soc_limit, tolerance_percent));
    } else if (strcmp(cmd, "get_maint_soc_protection_buffers_disable_flag") == 0) {
        FPS_DEBUG_LOG("You have reached the get_maint_soc_protection_buffers_disable_flag function for ess in asset.cpp");
        return_value = (maint_soc_protection_buffers_disable_flag == value->value_bool);
    } else if (strcmp(cmd, "get_maint_soc_limits_enable_flag") == 0) {
        FPS_DEBUG_LOG("You have reached the get_maint_soc_limits_enable_flag function for ess in asset.cpp");
        return_value = (maint_soc_limits_enable_flag == value->value_bool);
    } else if (strcmp(cmd, "get_maint_cell_voltage_limits_enable_flag") == 0) {
        FPS_DEBUG_LOG("You have reached the get_maint_cell_voltage_limits_enable_flag function for ess in asset.cpp");
        return_value = (maint_cell_voltage_limits_enable_flag == value->value_bool);
    } else if (strcmp(cmd, "get_maint_rack_voltage_limits_enable_flag") == 0) {
        FPS_DEBUG_LOG("You have reached the get_maint_rack_voltage_limits_enable_flag function for ess in asset.cpp");
        return_value = (maint_rack_voltage_limits_enable_flag == value->value_bool);
    } else if (strcmp(cmd, "get_maint_min_charge_discharge_enable_flag") == 0) {
        FPS_DEBUG_LOG("You have reached the get_maint_min_charge_discharge_enable_flag function for ess in asset.cpp");
        return_value = (maint_min_charge_discharge_enable_flag == value->value_bool);
    } else if (strcmp(cmd, "get_maint_min_cell_voltage_limit") == 0) {
        FPS_DEBUG_LOG("You have reached the get_maint_min_cell_voltage_limit function for ess in asset.cpp");
        return_value = ((value->type == Float) && get_tolerance(value->value_float, maint_min_cell_voltage_limit, tolerance_percent));
    } else if (strcmp(cmd, "get_maint_max_cell_voltage_limit") == 0) {
        FPS_DEBUG_LOG("You have reached the get_maint_max_cell_voltage_limit function for ess in asset.cpp");
        return_value = ((value->type == Float) && get_tolerance(value->value_float, maint_max_cell_voltage_limit, tolerance_percent));
    } else if (strcmp(cmd, "get_maint_min_rack_voltage_limit") == 0) {
        FPS_DEBUG_LOG("You have reached the get_maint_min_rack_voltage_limit function for ess in asset.cpp");
        return_value = ((value->type == Float) && get_tolerance(value->value_float, maint_min_rack_voltage_limit, tolerance_percent));
    } else if (strcmp(cmd, "get_maint_max_rack_voltage_limit") == 0) {
        FPS_DEBUG_LOG("You have reached the get_maint_max_rack_voltage_limit function for ess in asset.cpp");
        return_value = ((value->type == Float) && get_tolerance(value->value_float, maint_max_rack_voltage_limit, tolerance_percent));
    } else if (strcmp(cmd, "get_soc") == 0) {
        FPS_DEBUG_LOG("You have reached the get_soc function for ess in asset.cpp");
        return_value = ((value->type == Float) && get_tolerance(value->value_float, soc.value.value_float, tolerance_percent));
    } else if (strcmp(cmd, "get_active_power") == 0) {
        FPS_DEBUG_LOG("You have reached the get_soc function for ess in asset.cpp");
        return_value = ((value->type == Float) && get_tolerance(value->value_float, active_power.value.value_float, tolerance_percent));
    } else if (strcmp(cmd, "is_running") == 0) {
        return_value = (value->type == Bool) && is_running();
    } else if (strcmp(cmd, "is_stopped") == 0) {
        return_value = (value->type == Bool) && !is_running();
    } else if (strcmp(cmd, "is_faulted") == 0) {
        return_value = is_faulted.value.value_bool;
    } else if (strcmp(cmd, "soc_greater_than") == 0) {
        return_value = (value->type == Float) && (get_soc() > (value->value_float));
    } else if (strcmp(cmd, "soc_less_than") == 0) {
        return_value = (value->type == Float) && (get_soc() < (value->value_float));
    } else {
        command_found = false;
    }

    // no command executed
    if (!command_found) {
        FPS_ERROR_LOG("Error when running an action.json cmd attached to an ESS. cmd: %s", cmd);
        return false;
    }

    return return_value;
}
