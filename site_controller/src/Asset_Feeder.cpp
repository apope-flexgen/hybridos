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

Asset_Feeder::Asset_Feeder ()
{
    open_value = 0;
    close_value = 0;
    close_permissive_value = 0;
    close_permissive_remove_value = 0;
    reset_value = 0;
    breaker_close_permissive_status = false;

    asset_type_id = FEEDERS_TYPE_ID;

    set_required_variables();
}

Asset_Feeder::~Asset_Feeder ()
{
    if (grid_voltage_l1)    delete grid_voltage_l1;
    grid_voltage_l1 = NULL;
    if (grid_voltage_l2)    delete grid_voltage_l2;
    grid_voltage_l2 = NULL;
    if (grid_voltage_l3)    delete grid_voltage_l3;
    grid_voltage_l3 = NULL;
    if (grid_frequency)     delete grid_frequency;
    grid_frequency = NULL;
    if (breaker_status)     delete breaker_status;
    breaker_status = NULL;
    if (utility_status)     delete utility_status;
    utility_status = NULL;
}

// required variables checked for in configuration validation
void Asset_Feeder::set_required_variables(void)
{
    required_variables.push_back("breaker_status");
    required_variables.push_back("grid_frequency");
    required_variables.push_back("active_power");
    required_variables.push_back("reactive_power");
}

bool Asset_Feeder::get_breaker_status(void)
{
    // breaker status can either a mask or a boolean
    return (is_running() || breaker_status->value.value_bool);
}

/**
 * Status of the utility tracked by this feed register
 * Only supported for some sites, else will always be false
 */
bool Asset_Feeder::get_utility_status(void)
{
    return utility_status->value.value_bool;
}

float Asset_Feeder::get_gridside_frequency(void)
{
    return grid_frequency->value.value_float;
}

float Asset_Feeder::get_gridside_avg_voltage(void)
{
    float sumVolts = grid_voltage_l1->value.value_float + grid_voltage_l2->value.value_float + grid_voltage_l3->value.value_float;
    return (numPhases != 0.0 ? sumVolts/numPhases : 0);
}

float Asset_Feeder::get_power_factor()
{
    return power_factor->value.value_float;
}

void Asset_Feeder::breaker_reset(void)
{
    // component reset command clear should be handled by component
    send_to_comp_uri(reset_value, uri_breaker_reset);
    clear_alerts();
}

bool Asset_Feeder::breaker_close(void)
{
    return send_to_comp_uri(close_value, uri_breaker_close);
}

bool Asset_Feeder::breaker_close_permissive(void)
{
    return send_to_comp_uri(close_permissive_value, uri_breaker_close_permissive);
}

bool Asset_Feeder::breaker_open(void)
{
    return send_to_comp_uri(open_value, uri_breaker_open);
}

bool Asset_Feeder::breaker_close_permissive_remove(void)
{
    return send_to_comp_uri(close_permissive_remove_value, uri_breaker_close_permissive_remove);
}

void Asset_Feeder::set_active_power_setpoint(float setpoint)
{
    active_power_setpoint->component_control_value.value_float = setpoint;
}

bool Asset_Feeder::configure_typed_asset_instance_vars(Type_Configurator* configurator)
{
    Asset_Configurator* assetConfig = &configurator->assetConfig;

    cJSON *object = cJSON_GetObjectItem(assetConfig->assetInstanceRoot, "value_open");
    if (object)
        open_value = object->valueint;

    object = cJSON_GetObjectItem(assetConfig->assetInstanceRoot, "value_close");
    if (object)
        close_value = object->valueint;

    object = cJSON_GetObjectItem(assetConfig->assetInstanceRoot, "value_close_permissive");
    if (object)
        close_permissive_value = object->valueint;

    object = cJSON_GetObjectItem(assetConfig->assetInstanceRoot, "value_close_permissive_remove");
    if (object)
        close_permissive_remove_value = object->valueint;

    object = cJSON_GetObjectItem(assetConfig->assetInstanceRoot, "value_reset");
    if (object)
        reset_value = object->valueint;

    // Configure status strings
    // Publishes expected to report one of these strings based on status value
    // Only an integer is received in component publishes so we rely on hard coded values for now
    statusStrings[close_value] = {"Closed"};
    statusStrings[open_value] = {"Open"};
    return true;
}

bool Asset_Feeder::configure_ui_controls(Type_Configurator* configurator)
{
    // asset instances are data aggregators for one or many components, described in the "components" array. this array is required for any asset instance
    cJSON *components_array = cJSON_GetObjectItem(configurator->assetConfig.assetInstanceRoot, "components");
    if (components_array == NULL) {
        FPS_ERROR_LOG("Components array is NULL.");
        return false;
    }

    // for each component in the components array, parse out the UI control variables. other component variables are handled by the base class configure function
    for (uint i = 0; i < numAssetComponents; i++) {
        cJSON *component = cJSON_GetArrayItem(components_array, i);
        if (component == NULL) {
            FPS_ERROR_LOG("Component with index %d has NULL configuration.", i);
            return false;
        }
        
        // UI controls are optional
        cJSON* ui_controls = cJSON_GetObjectItem(component, "ui_controls");
        if (ui_controls == NULL)
            continue;
        
        // when adding a new UI control, make sure to add it to the list of valid UI controls in Asset_Manager.cpp
        cJSON* ctrl_obj;
        ctrl_obj = cJSON_GetObjectItem(ui_controls, "maint_mode");
        if (ctrl_obj != NULL && !maint_mode.configure(ctrl_obj, yesNoOption, &inMaintenance, Bool, sliderStr, true)) {
            FPS_ERROR_LOG("Failed to configure maint_mode UI control.");
            return false;
        }

        ctrl_obj = cJSON_GetObjectItem(ui_controls, "lock_mode");
        if (ctrl_obj != NULL && !lock_mode.configure(ctrl_obj, yesNoOption, &inLockdown, Bool, sliderStr, true)) {
            FPS_ERROR_LOG("Failed to configure lock_mode UI control.");
            return false;
        }

        ctrl_obj = cJSON_GetObjectItem(ui_controls, "breaker_close");
        if (ctrl_obj != NULL) {
            if (!breaker_close_ctl.configure(ctrl_obj, onOffOption, NULL, Int, buttonStr, false)) {
                FPS_ERROR_LOG("Failed to configure breaker_close UI control.");
                return false;
            }
            uri_breaker_close = build_uri(compNames[i], breaker_close_ctl.reg_name);
        }

        ctrl_obj = cJSON_GetObjectItem(ui_controls, "breaker_close_permissive");
        if (ctrl_obj != NULL) {
            if (!breaker_close_perm_ctl.configure(ctrl_obj, onOffOption, NULL, Int, buttonStr, false)) {
                FPS_ERROR_LOG("Failed to configure breaker_close_permissive UI control.");
                return false;
            }
            uri_breaker_close_permissive = build_uri(compNames[i], breaker_close_perm_ctl.reg_name);
        }
        
        ctrl_obj = cJSON_GetObjectItem(ui_controls, "breaker_close_permissive_remove");
        if (ctrl_obj != NULL) {
            if (!breaker_close_perm_remove_ctl.configure(ctrl_obj, onOffOption, NULL, Int, buttonStr, false)) {
                FPS_ERROR_LOG("Failed to configure breaker_close_permissive_remove UI control.");
                return false;
            }
            uri_breaker_close_permissive_remove = build_uri(compNames[i], breaker_close_perm_remove_ctl.reg_name);
        }

        ctrl_obj = cJSON_GetObjectItem(ui_controls, "breaker_open");
        if (ctrl_obj != NULL) {
            if (!breaker_open_ctl.configure(ctrl_obj, onOffOption, NULL, Int, buttonStr, false)) {
                FPS_ERROR_LOG("Failed to configure breaker_open UI control.");
                return false;
            }
            uri_breaker_open = build_uri(compNames[i], breaker_open_ctl.reg_name);
        }

        ctrl_obj = cJSON_GetObjectItem(ui_controls, "breaker_reset");
        if (ctrl_obj != NULL) {
            if (!breaker_reset_ctl.configure(ctrl_obj, resetOption, NULL, Int, buttonStr, false)) {
                FPS_ERROR_LOG("Failed to configure breaker_reset UI control.");
                return false;
            }
            uri_breaker_reset = build_uri(compNames[i], breaker_reset_ctl.reg_name);
        }
    }
    return true;
}

/*
    Each asset has Fims_Object pointers to relevant component variables for that asset. This function points
    those pointers at the correct variables in the variable maps.
    WARNING: "raw" values MUST be configured BEFORE their associated calculated values. This is because calculated
             values will sever the asset_var_map connection to the component variable and the raw values need to
             come from the component
    Ex: `dischargeable_power_raw` must be configured before `dischargeable_power`
*/
bool Asset_Feeder::configure_typed_asset_fims_vars(std::map <std::string, Fims_Object*> * const asset_var_map)
{
    configure_single_fims_var(asset_var_map,&breaker_status,"breaker_status",Bool);
    configure_single_fims_var(asset_var_map,&utility_status,"utility_status",Bool);
    configure_single_fims_var(asset_var_map,&grid_voltage_l1,"grid_voltage_l1");
    configure_single_fims_var(asset_var_map,&grid_voltage_l2,"grid_voltage_l2");
    configure_single_fims_var(asset_var_map,&grid_voltage_l3,"grid_voltage_l3");
    configure_single_fims_var(asset_var_map,&grid_frequency,"grid_frequency");
    return true;
}

bool Asset_Feeder::validate_poi_feeder_configuration(Type_Configurator* configurator)
{
    // if config validation flag is false, no need to validate
    if (!configurator->config_validation)
    {
        return true;
    }

    // does config validation that was not done earlier since we did not know which feeder was POI at the time
    if (!validate_config(configurator->pAssetVarMap))
    {
        FPS_ERROR_LOG("Asset_Feeder::validate_poi_feeder_configuration ~ POI feeder failed base Asset validate config check\n");
        return false;
    }

    // checks to make sure required base class vars were configured. other assets had these checked in configure_base function
    cJSON* obj = cJSON_GetObjectItem(configurator->assetConfig.assetInstanceRoot, "rated_active_power_kw");
    if (obj == NULL)
    {
        FPS_ERROR_LOG("Asset_Feeder::validate_poi_feeder_configuration ~ POI feeder missing required rated_active_power_kw variable\n");
        return false;
    }
    obj = cJSON_GetObjectItem(configurator->assetConfig.assetInstanceRoot, "slew_rate");
    if (obj == NULL)
    {
        FPS_ERROR_LOG("Asset_Feeder::validate_poi_feeder_configuration ~ POI feeder missing required slew_rate variable\n");
        return false;
    }
    return true;
}

//Todo: This function has a strange unconventional fims hierarchy. Usually there is 2 layers (body->value) this one has 3("metabody"->body->value). Might should figure out and change why this is the case. 
//Todo: grab_naked... is a temporary fix. The real goal should be to do pure naked sets, but dbi expects clothed values so this function clothes naked sets before they are handed to dbi.
bool Asset_Feeder::handle_set(std::string uri, cJSON &body)
{
    // The current setpoint being parsed from those available
    cJSON* current_setpoint = NULL;

    // Whether the setpoint is supported by persistent settings
    // For instance, sets that modify the system state should not persist as they will default to the published component state on restart
    bool persistent_setpoint = false;

    if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "breaker_close")) && inMaintenance)
    {
        breaker_close();
        persistent_setpoint = true;
    }
    else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "breaker_close_permissive")) && inMaintenance)
    {
        breaker_close_permissive_status = true;
        breaker_close_permissive();
        persistent_setpoint = true;
    }
    else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "breaker_close_permissive_remove")) && inMaintenance)
    {
        breaker_close_permissive_status = false;
        breaker_close_permissive_remove();
        persistent_setpoint = true;
    }
    else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "breaker_open")) && inMaintenance)
    {
        breaker_open();
        persistent_setpoint = true;
    }
    else if ((current_setpoint = grab_naked_or_clothed_and_check_type(body, current_setpoint, cJSON_True, "breaker_reset")))
    {
        if (inMaintenance)
            breaker_reset(); // hard reset, only when in maint mode
        else {            
            clear_alerts(); // graceful, component-level reset
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

void Asset_Feeder::process_asset(bool* status)
{
    if (status)
    {
        raw_status = *status;
    }
    return Asset::process_asset();
}

void Asset_Feeder::update_asset(void)
{
    // TODO:: add maintenance mode handling here
}

void Asset_Feeder::process_potential_active_power(void)
{
    if (!get_breaker_status())
    {
        max_potential_active_power = 0.0f;
        min_potential_active_power = 0.0f;
        return;
    }
    Asset::process_potential_active_power();
}

void Asset_Feeder::send_to_components(void)
{
    FPS_DEBUG_LOG("Currently nothing to send to Feeders; Asset_Feeder::send_to_components.\n");
}

/****************************************************************************************/
// Variable map moved to Feeder_Manager
bool Asset_Feeder::generate_asset_ui(fmt::memory_buffer &buf, const char* const var)
{
    bool goodBody = true;

    // add the manual mode control if defined
    maint_mode.enabled = (!inLockdown);
    goodBody = maint_mode.makeJSONObject(buf, var) && goodBody;

    // add the lockdown mode control if defined
    lock_mode.enabled = (inMaintenance);
    goodBody = lock_mode.makeJSONObject(buf, var) && goodBody;

    // now add the rest of the controls
    breaker_close_ctl.enabled = !breaker_status->value.value_bool && inMaintenance;
    goodBody = breaker_close_ctl.makeJSONObject(buf, var) && goodBody;

    breaker_close_perm_ctl.enabled = !breaker_status->value.value_bool && !breaker_close_permissive_status && inMaintenance;
    goodBody = breaker_close_perm_ctl.makeJSONObject(buf, var) && goodBody;

    breaker_close_perm_remove_ctl.enabled = breaker_close_permissive_status && inMaintenance;
    goodBody = breaker_close_perm_remove_ctl.makeJSONObject(buf, var) && goodBody;

    breaker_open_ctl.enabled = breaker_status->value.value_bool && inMaintenance;
    goodBody = breaker_open_ctl.makeJSONObject(buf, var) && goodBody;

    breaker_reset_ctl.enabled = !breaker_status->value.value_bool && inMaintenance;
    goodBody = breaker_reset_ctl.makeJSONObject(buf, var) && goodBody;

    return (goodBody);
}
