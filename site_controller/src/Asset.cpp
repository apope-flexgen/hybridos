/*
 * Asset.cpp
 *
 *  Created on: May 9, 2018
 *      Author: jcalcagni
 */

/* C Standard Library Dependencies */
#include <cstring>
/* C++ Standard Library Dependencies */
#include <sstream>
#include <iomanip>
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
/* Local Internal Dependencies */
#include <Asset.h>
#include <Data_Endpoint.h>
#include <Site_Controller_Utils.h>
#include <Configurator.h>

extern fims* p_fims;
extern Data_Endpoint* data_endpoint;

Write_Rate_Throttle::Write_Rate_Throttle ()
{
    throttle_timeout = 0;
    status_time = 0;

    control_feedback = 0;
    rated_power = 0;
    deadband_percentage = 0;
}

Write_Rate_Throttle::~Write_Rate_Throttle ()
{

}

Asset::Asset ()
{
    name = NULL;
    asset_id = NULL;

    // status variables
    isAvail = false;
    isRunning = false;
    inMaintenance = false;
    inStandby = false;
    newly_faulted = false;

    // control variables
    clearFaultsControlEnable = false;
    clear_fault_registers_flag = false;
    clock_gettime(CLOCK_MONOTONIC, &clear_faults_time);

    // configuration variables
    running_status_mask = 0;
    nominal_voltage = 0;
    nominal_frequency = 0;

    numPhases = 3;

    prev_modbus_heartbeat = -1;    // Value of -1 forces check_fims_timeout() to initialize fims_timer in the first function call

    // status variables
    numAssetComponents = 0;

    status_type = invalid;

    status = 0;
    raw_status = 0;

    memset(statusStrings, 0, sizeof(char*) * MAX_STATUS_BITS);

    memset(compNames, 0, sizeof(char*) * MAX_COMPS);

    watchdog_enable = NULL;
    connected_rising_edge_detect = true;

    active_power_slew.reset_slew_target();
    max_potential_active_power = 0.0;
    min_potential_active_power = 0.0;

    rated_active_power_kw = 0;
    rated_reactive_power_kvar = 0;
    rated_apparent_power_kva = 0;

    throttle_timeout_fast_ms = 0.0;
    throttle_timeout_slow_ms = 0.0;
    throttle_deadband_percentage = 0.0;

    // uris
    uri_clear_faults = NULL;

    assetControl = Uncontrolled;

    send_FIMS_buf = fmt::memory_buffer();
}

Asset::~Asset ()
{
    if (name != NULL) free(name);
    if (asset_id != NULL) free(asset_id);
    if (uri_clear_faults != NULL) free(uri_clear_faults);
    if (uri_component_connected != NULL) free(uri_component_connected);
    if (uri_modbus_connected != NULL) free(uri_modbus_connected);

    for (uint i = 0; i < numAssetComponents; i++)
    {
        if (compNames[i] != NULL)
            free(compNames[i]);
    }

    // delete component alarm registers
    for (auto alarm_register_ptr : component_alarm_registers)
    {
        if (alarm_register_ptr)
        {
            delete alarm_register_ptr;
            // Assign NULL so pointer in other containers doesn't delete
            alarm_register_ptr = NULL;
        }
    }

    // delete component fault registers
    for (auto fault_register_ptr : component_fault_registers)
    {
        if (fault_register_ptr)
        {
            delete fault_register_ptr;
            // Assign NULL so pointer in other containers doesn't delete
            fault_register_ptr = NULL;
        }
    }

    // control points
    if (active_power_setpoint)  delete active_power_setpoint;
    active_power_setpoint = NULL;
    if (reactive_power_setpoint)    delete reactive_power_setpoint;
    reactive_power_setpoint = NULL;
    if (active_power)   delete active_power;
    active_power = NULL;
    if (reactive_power)     delete reactive_power;
    reactive_power = NULL;
    if (apparent_power)     delete apparent_power;
    apparent_power = NULL;
    // status points
    if (voltage_l1_l2)  delete voltage_l1_l2; // Line 1 - Line 2
    voltage_l1_l2 = NULL;
    if (voltage_l2_l3)  delete voltage_l2_l3; // Line 2 - Line 3
    voltage_l2_l3 = NULL;
    if (voltage_l3_l1)  delete voltage_l3_l1; // Line 3 - Line 1
    voltage_l3_l1 = NULL;
    if (voltage_l1_n)   delete voltage_l1_n; // Line 1 - N
    voltage_l1_n = NULL;
    if (voltage_l2_n)   delete voltage_l2_n; // Line 2 - N
    voltage_l2_n = NULL;
    if (voltage_l3_n)   delete voltage_l3_n; // Line 3 - N
    voltage_l3_n = NULL;
    if (current_l1)    delete current_l1;
    current_l1 = NULL;
    if (current_l2)    delete current_l2;
    current_l2 = NULL;
    if (current_l3)    delete current_l3;
    current_l3 = NULL;
    if (power_factor)   delete power_factor;
    power_factor = NULL;
    if (modbus_heartbeat)   delete modbus_heartbeat;
    modbus_heartbeat = NULL;
    if (component_connected)    delete component_connected;
    component_connected = NULL;
    if (modbus_connected)   delete modbus_connected;
    modbus_connected = NULL;

    #ifdef FPS_TEST_MODE
    delete data_endpoint;
    #endif
}

fimsCtl::fimsCtl()
{
    pDisplay = NULL;
    varName = NULL;
    unit = NULL;
    reg_name = NULL;
    obj_name = NULL;
    uiType = emptyNull;
    enabled = false;
    boolString = false;
    vt = Invalid;
    options = nullJson;
    scaler = 1;
    configured = false;
}

fimsCtl::~fimsCtl()
{
    if (unit != NULL)
        free(unit);
    if (varName != NULL)
        free(varName);
    if (reg_name != NULL)
        free(reg_name);
    if (obj_name != NULL)
        free(obj_name);
}

const char* replace_wildcard_with_number(char* templated_string, int replacement_number)
{
    const char* wildcard_character("#");
    std::string new_string(templated_string);
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << replacement_number;
    std::string new_number = ss.str();

    auto wildcard_location = new_string.find(wildcard_character);
    if (wildcard_location == std::string::npos) {
        FPS_ERROR_LOG("Wildcard character %s not found in string %s\n", wildcard_character, new_string.c_str());
        return NULL;
    }
    new_string.replace(wildcard_location, strlen(wildcard_character), new_number);

    return new_string.c_str();
}

// configures variables. Calls config validation function if validate flag is set
// each type of variable uses the previously configured variables for its own configuration, so order is important here
bool Asset::configure(Type_Configurator* configurator)
{
    is_primary = configurator->pIsPrimaryController; // this is a pointer to a broader controller status flag - neither an asset instance var nor a component var

    if ( !configure_common_asset_instance_vars(configurator) )
    {
        FPS_ERROR_LOG("Asset::configure ~ Failed to configure common asset instance vars.\n");
        return false;
    }

    if ( !configure_component_vars(configurator) )
    {
        FPS_ERROR_LOG("Asset::configure ~ Failed to configure component vars.\n");
        return false;
    }

    if ( !configure_common_asset_fims_vars(configurator->pAssetVarMap) )
    {
        FPS_ERROR_LOG("Asset::configure ~ Failed to configure common FIMS setpoints.\n");
        return false;
    }

    if ( !configure_typed_asset_fims_vars(configurator->pAssetVarMap) )
    {
        FPS_ERROR_LOG("Asset::configure_typed_asset ~ Failed to configure typed asset FIMS setpoints.\n");
        return false;
    }

    if ( !configure_typed_asset_instance_vars(configurator) )
    {
        FPS_ERROR_LOG("Asset::configure_typed_asset ~ Failed to configure typed asset instance vars.\n");
        return false;
    }

    if ( !configure_ui_controls(configurator) )
    {
        FPS_ERROR_LOG("Asset::configure_typed_asset ~ Failed to configure UI controls.\n");
        return false;
    }

    // check configured component variables against list of required component variables to ensure all were found and configured
    // non-POI feeders don't have any required variables. POI feeder does but it will be checked separately
    if ( strcmp(get_asset_type(),"feeders") != 0 && configurator->config_validation )
    {
        return validate_config(configurator->pAssetVarMap);
    }

    return true;
}

// initializes values of "asset instance variables", which are variables that are specific to each individual asset and do not come from components
bool Asset::configure_common_asset_instance_vars(Type_Configurator* configurator)
{
    Asset_Configurator* assetConfig = &configurator->assetConfig;

    // #ifdef FPS_TEST_MODE
    // data_endpoint = new Data_Endpoint();
    // #endif

    // The asset id represents the asset instance. Typically is <asset_type>_<2-digit number> E.g. ess_04, gen_03, solar_02, feed_01, etc. The asset id is required
    cJSON *id = cJSON_GetObjectItem(assetConfig->assetInstanceRoot, "id");
    if (id == NULL)
    {
        FPS_ERROR_LOG("Asset::configure_common_asset_instance_vars ~ Failed to find ID in config.\n");
        return false;
    }

    // if this asset instance was configured from a template, replace wildcard character `#` with template_index
    if (assetConfig->is_template()) {
        const char* id_with_replaced_wildcard = replace_wildcard_with_number(id->valuestring, assetConfig->template_index);
        if (id_with_replaced_wildcard == NULL) {
            FPS_ERROR_LOG("Failed to expand templated asset_id %s\n", id->valuestring);
            return false;
        } else {
            asset_id = strdup(id_with_replaced_wildcard);
        }
    } else {
        asset_id = strdup(id->valuestring);
    }
    
    // The asset name is a string and is what will be displayed on the UI. It can be verbose and descriptive. The asset name is required
    cJSON* asset_name = cJSON_GetObjectItem(assetConfig->assetInstanceRoot, "name");
    if (asset_name == NULL)
    {
        FPS_ERROR_LOG("Asset::configure_common_asset_instance_vars ~ Failed to find Name in config.\n");
        return false;
    }

    // if this asset instance was configured from a template, replace wildcard character `#` with template_index
    if (assetConfig->is_template()) {
        const char* name_with_replaced_wildcard = replace_wildcard_with_number(asset_name->valuestring, assetConfig->template_index);
        if (name_with_replaced_wildcard == NULL) {
            FPS_ERROR_LOG("Failed to expand templated asset name %s\n", asset_name->valuestring);
            return false;
        } else {
            name = strdup(name_with_replaced_wildcard);
        }
    } else {
        name = strdup(asset_name->valuestring);
    }

    cJSON* item = cJSON_GetObjectItem(assetConfig->assetInstanceRoot, "status_type");
    status_type = (item != NULL && strcmp(item->valuestring, "random_enum") == 0) ? random_enum : bit_field;

    cJSON *runMask = cJSON_GetObjectItem(assetConfig->assetInstanceRoot, "running_status_mask");
    //if the mask is NULL and is not a feeder, throw an error
    if (runMask == NULL && strcmp(get_asset_type(),"feeders") != 0 && configurator->config_validation) // not required for feeders
    {
        FPS_ERROR_LOG("Asset::configure_common_asset_instance_vars ~ Failed to find running_status_mask in config: %s\n", asset_id);
        return false;
    }
    // if the mask isn't NULL and is a hex string, try to parse it
    else if (runMask != NULL && runMask->valuestring != NULL)
        try
        {
            // Casts the provided hex string as an unsigned int64
            running_status_mask = (uint64_t) std::stoul(runMask->valuestring, NULL, 16);
        }
        catch (...)
        {
            // Could not parse the provided mask
            FPS_ERROR_LOG("Asset %s received an invalid running_status_mask\n", name);
            return false;
        }
    // if the mask isn't null and isn't a hex string, throw an error
    else if (runMask != NULL) 
    {
        FPS_ERROR_LOG("Asset %s received an invalid input type instead of a hexadecimal string for the running_status_mask\n", name);
        return false;
    }

    cJSON *stoppedMask = cJSON_GetObjectItem(assetConfig->assetInstanceRoot, "stopped_status_mask");
    //if the mask is NULL and is not a feeder, throw an error
    if (stoppedMask == NULL && strcmp(get_asset_type(),"feeders") != 0 && configurator->config_validation) // not required for feeders
    {
        FPS_ERROR_LOG("Asset::configure_common_asset_instance_vars ~ Failed to find stopped_status_mask in config: %s\n", asset_id);
        return false;
    }
    // if the mask isn't NULL and is a hex string, try to parse it
    else if (stoppedMask != NULL && stoppedMask->valuestring != NULL)
        try
        {
            // Casts the provided hex string as an unsigned int64
            stopped_status_mask = (uint64_t) std::stoul(stoppedMask->valuestring, NULL, 16);
        }
        catch (...)
        {
            // Could not parse the provided mask
            FPS_ERROR_LOG("Asset %s received an invalid stopped_status_mask\n", name);
            return false;
        }
    // if the mask isn't null and isn't a hex string, throw an error
    else if (stoppedMask != NULL) 
    {
        FPS_ERROR_LOG("Asset %s received an invalid input type instead of a hexadecimal string for the running_status_mask\n", name);
        return false;
    }

    cJSON* rated_active_pwr_kw = cJSON_GetObjectItem(assetConfig->assetInstanceRoot, "rated_active_power_kw");
    if (rated_active_pwr_kw == NULL)
    {
        // rated_active_pwr_kw not required if asset is a non-POI feeder. POI feeder will be checked for rated_active_power_kw later
        if ((strcmp(get_asset_type(),"feeders") != 0 ) && configurator->config_validation)
        {
            FPS_ERROR_LOG("Asset::configure_common_asset_instance_vars ~ Failed to find rated_active_pwr_kw in config: %s\n", asset_id);
            return false;
        }
    }
    else rated_active_power_kw = rated_active_pwr_kw->valuedouble;

    cJSON* rated_reactive_pwr_kvar = cJSON_GetObjectItem(assetConfig->assetInstanceRoot, "rated_reactive_power_kvar");
    if (rated_reactive_pwr_kvar != NULL)
        rated_reactive_power_kvar = rated_reactive_pwr_kvar->valuedouble;

    cJSON* rated_apparent_pwr_kva = cJSON_GetObjectItem(assetConfig->assetInstanceRoot, "rated_apparent_power_kva");
    if (rated_apparent_pwr_kva != NULL)
        rated_apparent_power_kva = rated_apparent_pwr_kva->valuedouble;

    cJSON* slew_rate = cJSON_GetObjectItem(assetConfig->assetInstanceRoot, "slew_rate");
    if (slew_rate == NULL)
    {
        // slew_rate not required if asset is a non-POI feeder. POI feeder will be checked for slew_rate later
        if ((strcmp(get_asset_type(),"feeders") != 0 ) && configurator->config_validation)
        {
            FPS_ERROR_LOG("Asset::configure_common_asset_instance_vars ~ Failed to find slew_rate in config: %s\n", asset_id);
            return false;
        }
    }
    else active_power_slew.set_slew_rate(slew_rate->valueint);

    cJSON* throttle_timeout_fast = cJSON_GetObjectItem(assetConfig->assetInstanceRoot, "throttle_timeout_fast_ms");
    if(throttle_timeout_fast != NULL)
        throttle_timeout_fast_ms = throttle_timeout_fast->valuedouble;

    cJSON* throttle_timeout_slow = cJSON_GetObjectItem(assetConfig->assetInstanceRoot, "throttle_timeout_slow_ms");
    if(throttle_timeout_slow != NULL)
        throttle_timeout_slow_ms = throttle_timeout_slow->valuedouble;

    cJSON* deadband_percent = cJSON_GetObjectItem(assetConfig->assetInstanceRoot, "deadband_percent");
    if(deadband_percent != NULL)
        throttle_deadband_percentage = deadband_percent->valuedouble;

    cJSON* wd_enable = cJSON_GetObjectItem(assetConfig->assetInstanceRoot, "watchdog_enable");
    watchdog_enable = (wd_enable == NULL) ? false : (wd_enable->type == cJSON_True);

    if(watchdog_enable)
    {   // Only look for watchdog timeout if watchdog feature is enabled
        cJSON* wd_timeout_ms = cJSON_GetObjectItem(assetConfig->assetInstanceRoot, "watchdog_timeout_ms");
        if (wd_timeout_ms == NULL)
        {
            FPS_ERROR_LOG("Asset::configure_common_asset_instance_vars ~ Failed to find watchdog timeout ms in config.\n");
            return false;
        }
        watchdog_timeout_ms = wd_timeout_ms->valueint;
    }

    cJSON* d_ctrl = cJSON_GetObjectItem(assetConfig->assetInstanceRoot, "demand_control");
    if (d_ctrl != NULL)
    {
        if( strcmp("Uncontrolled", d_ctrl->valuestring) == 0 )
        {
            assetControl = Uncontrolled;
        }
        else if( strcmp("Indirect", d_ctrl->valuestring) == 0 )
        {
            assetControl = Indirect;
        }
        else if( strcmp("Direct", d_ctrl->valuestring) == 0 )
        {
            assetControl = Direct;
        }
        else if( strcmp("Manual", d_ctrl->valuestring) == 0 )
        {
            assetControl = Manual;
        }
        else
        {
            FPS_ERROR_LOG("Asset::configure_common_asset_instance_vars ~ Unrecognized demand control type %s\n", d_ctrl->valuestring);
            return false;
        }
    }
    else
    {
        FPS_ERROR_LOG("Asset::configure_common_asset_instance_vars ~ Asset %s contains no demand_control variable\n", name);
        return false;
    }

    return true;
}

bool Asset::configure_component_vars(Type_Configurator* configurator)
{
    Asset_Configurator* assetConfig = &configurator->assetConfig;

    // Asset instances are data aggregators for one or many components, described in the "components" array. This array is required for any asset instance
    cJSON* comp_array = cJSON_GetObjectItem(assetConfig->assetInstanceRoot, "components");
    if (comp_array == NULL && configurator->config_validation)
    {
        FPS_ERROR_LOG("Asset::configure_component_vars ~ Asset %s contains NULL components array\n", name);
        return false;
    }

    // There can be one or many components
    numAssetComponents = cJSON_GetArraySize(comp_array);
    if (numAssetComponents > MAX_COMPS)
    {
        FPS_ERROR_LOG("Asset::configure_component_vars ~ Too many components included for %s.\n", name);
        return false;
    }

    // For each component, configure its component variables
    for (uint i = 0; i < numAssetComponents; i++)
    {
        cJSON* comp = cJSON_GetArrayItem(comp_array, i);
        if (comp == NULL)
        {
            FPS_ERROR_LOG("Asset::configure_component_vars ~ Invalid or NULL component #%d of %s\n", i, name);
            return false;
        }
        cJSON* comp_id = cJSON_GetObjectItem(comp, "component_id");
        cJSON* variables = cJSON_GetObjectItem(comp, "variables");
        if (variables == NULL || comp_id == NULL || comp_id->valuestring == NULL)
        {
            FPS_ERROR_LOG("Asset::configure_component_vars ~ NULL variables or component_id for %d of %s \n", i, name);
            return false;
        }

        // checks if asset is a feeder by looking for "breaker_control" variable in components
        cJSON* breaker_control = cJSON_GetObjectItem(variables, "breaker_control");
        if (breaker_control != NULL) {
            Asset_Feeder* asset_feeder = static_cast<Asset_Feeder*>(this);
            cJSON* reg_id = cJSON_GetObjectItem(breaker_control, "register_id");
            char *breaker_close_uri = strdup(build_uri(comp_id->valuestring, reg_id->valuestring).c_str());
            char *breaker_open_uri = strdup(build_uri(comp_id->valuestring, reg_id->valuestring).c_str());
            asset_feeder->set_breaker_close_uri(breaker_close_uri);
            asset_feeder->set_breaker_open_uri(breaker_open_uri);
            delete [] breaker_close_uri;
            delete [] breaker_open_uri;
        }
        compNames[i] = assetConfig->is_template() ? strdup(replace_wildcard_with_number(comp_id->valuestring, assetConfig->template_index)) : strdup(comp_id->valuestring);

        // Iterate through each variable in this component and insert its information into a FIMS_Object
        // that will be in the component variables map and asset variables map owned by Asset Manager
        cJSON *component_variable = variables->child; // grab the first variable object within the component object
        while( component_variable ) // iterate until there is not a new variable object to deal with
        {
            // Insert the variable information into the FIMS_Object and component variables map and abort if failure
            if ( !var_maps_insert(component_variable, compNames[i], configurator->pCompVarMap, configurator->pAssetVarMap, configurator->pIsPrimaryController) )
            {
                FPS_ERROR_LOG("Asset::configure_component_vars ~ Insertion into component variables map failed.\n");
                return false;
            }
            // Get the next variable object in this component
            component_variable = component_variable->next;
        }
    }

    #ifdef FPS_TEST_MODE
    data_endpoint = new Data_Endpoint();
    #endif
    return true;
}

/*
    Each asset has Fims_Object pointers to relevant component variables for that asset. This function points
    those pointers at the correct variables in the variable maps. This function is called from its overriding
    function in the derived class
    WARNING: "raw" values MUST be configured BEFORE their associated calculated values. This is because calculated
             values will sever the asset_var_map connection to the component variable and the raw values need to
             come from the component
    Ex: `dischargeable_power_raw` must be configured before `dischargeable_power`
*/
bool Asset::configure_common_asset_fims_vars(std::map <std::string, Fims_Object*> * const asset_var_map)
{
    configure_single_fims_var(asset_var_map,&active_power_setpoint,"active_power_setpoint");
    configure_single_fims_var(asset_var_map,&reactive_power_setpoint,"reactive_power_setpoint");
    configure_single_fims_var(asset_var_map,&active_power,"active_power");
    configure_single_fims_var(asset_var_map,&reactive_power,"reactive_power");
    configure_single_fims_var(asset_var_map,&apparent_power,"apparent_power");
    configure_single_fims_var(asset_var_map,&voltage_l1_n,"voltage_l1_n");
    configure_single_fims_var(asset_var_map,&voltage_l2_n,"voltage_l2_n");
    configure_single_fims_var(asset_var_map,&voltage_l3_n,"voltage_l3_n");
    configure_single_fims_var(asset_var_map,&voltage_l1_l2,"voltage_l1_l2");
    configure_single_fims_var(asset_var_map,&voltage_l2_l3,"voltage_l2_l3");
    configure_single_fims_var(asset_var_map,&voltage_l3_l1,"voltage_l3_l1");
    configure_single_fims_var(asset_var_map,&current_l1,"current_l1");
    configure_single_fims_var(asset_var_map,&current_l2,"current_l2");
    configure_single_fims_var(asset_var_map,&current_l3,"current_l3");
    configure_single_fims_var(asset_var_map,&power_factor,"power_factor");
    configure_single_fims_var(asset_var_map,&modbus_heartbeat,"modbus_heartbeat",Int);
    configure_single_fims_var(asset_var_map,&component_connected,"component_connected",Bool);
    configure_single_fims_var(asset_var_map,&modbus_connected,"modbus_connected",Bool);
    return true;
}

// Helper function used by configure_single_fims_var. Could maybe be moved into the Fims_Object class
void set_default_value(Fims_Object** fimsVar, valueType type, float defaultFloat, int defaultInt, bool defaultBool)
{
    switch(type)
    {
    case Int:
        (*fimsVar)->value.set( (int)defaultInt );
        (*fimsVar)->component_control_value.set( (int)defaultInt );
        break;
    case Float:
        (*fimsVar)->value.set( (float)defaultFloat );
        (*fimsVar)->component_control_value.set( (float)defaultFloat );
        break;
    case Bool:
        (*fimsVar)->value.set( (bool)defaultBool );
        (*fimsVar)->component_control_value.set( (bool)defaultBool );
        break;
    // Other types not currently needed, add if necessary
    default:
        break;
    }
}

// Helper function used by configure_single_fims_var
bool make_new_fimsVar(Fims_Object** fimsVar, valueType type, float defaultFloat, int defaultInt, bool defaultBool, const char* varID="", const char* varName="", const char* varUnits="", int varScaler=1)
{
    // Make a new Fims_Object
    try
    {
        *fimsVar = new Fims_Object();
    }
    catch(const std::exception& e)
    {
        FPS_ERROR_LOG("Asset::configure_single_fims_var ~ Error allocating new memory\n");
        return false;
    }

    (*fimsVar)->set_variable_id(varID);
    (*fimsVar)->set_name(varName);
    (*fimsVar)->set_ui_type("status");
    (*fimsVar)->set_unit(varUnits);
    (*fimsVar)->scaler = varScaler;

    switch(type)
    {
    case Int:
        (*fimsVar)->value.set( (int)defaultInt );
        (*fimsVar)->component_control_value.set( (int)defaultInt );
        break;
    case Float:
        (*fimsVar)->value.set( (float)defaultFloat );
        (*fimsVar)->component_control_value.set( (float)defaultFloat );
        break;
    case Bool:
        (*fimsVar)->value.set( (bool)defaultBool );
        (*fimsVar)->component_control_value.set( (bool)defaultBool );
        break;
    // Other types not currently needed, add if necessary
    default:
        break;
    }

    return true;
}

/************************************************************************************************************************************************
    For variables that have pointers in the asset instance (all variables that use this function), 
    they are either inputs from components or outputs to the /assets/ publish.

        1. Inputs: Just point the pointer at the Fims_Object which has already been configured in the asset_var_map and component_var_map
        2. Outputs: These will not have already been configured in the asset_var_map, so they must be configured here
        3. Special case variables:  These variables will have a "raw" Fims_Object coming from the component, and a calculated
                                    Fims_Object that gets published on /assets/. At this stage of configuration, the raw
                                    object will have already been configured in both the asset_var_map and the component_var_map.
                                    We want the raw object to stay in the component_var_map as an input and the calculated object 
                                    to replace the raw object in asset_var_map as an output.
                                    Ex: state of charge. "soc_raw" will have been configured into both maps at this point, but there
                                    is a calculation that uses soc_raw as an input and outputs a calculated "soc" object. So we leave
                                    soc_raw in component_var_map to access the input coming from the component. We take soc_raw out of
                                    asset_var_map and replace it with soc, using the same URI.
*/
bool Asset::configure_single_fims_var(std::map <std::string, Fims_Object*> * const asset_var_map, Fims_Object** fimsVar, const char* varID, valueType type, float defaultFloat, int defaultInt, bool defaultBool, bool comesFromComponent, const char* varName, const char* varUnits, int varScaler)
{
    if (fimsVar == NULL)
    {
        FPS_ERROR_LOG("Asset::configure_single_fims_var ~ variable passed to function is NULL\n");
        return false;
    }

    // Make URI key
    std::string assets_uri = build_assets_uri(varID);

    // Inputs and special case raw values
    if ( comesFromComponent )
    {
        // Point the pointer at the Fims_Object in the map
        *fimsVar = asset_var_map->find( assets_uri ) != asset_var_map->end() ? asset_var_map->at( assets_uri ) : NULL;
        // If this is not a required variable and it was left out of assets.json, make it a skeleton Fims_Object to avoid seg faulting
        // TODO: Put checks in place so that if a component variable does not exist in assets.json, we do not try to access it in hybridos
        if (*fimsVar == NULL)
        {
            make_new_fimsVar(fimsVar, type, defaultFloat, defaultInt, defaultBool);
        }
    }
    // Outputs and special case calculated values
    else
    {
        make_new_fimsVar(fimsVar, type, defaultFloat, defaultInt, defaultBool, varID, varName, varUnits, varScaler);

        // If this is a special case variable, delete the raw value from asset_var_map so the calculated value can replace it
        if ( asset_var_map->find( assets_uri ) != asset_var_map->end() )
            asset_var_map->erase( build_assets_uri(varID) );

        // Insert the newly configured Fims_Object into asset_var_map
        asset_var_map->insert(std::make_pair(assets_uri, *fimsVar));
    }
    (*fimsVar)->is_primary = is_primary;
    return true;
}

/*
    Asset::validate_config checks the asset_var_map created during configuration to make sure that all required component
    variables for an asset instance are successfully configured. The required variables are held in a list that lives in 
    each of the derived classes Asset_ESS, Asset_Feeder, Asset_Generator, and Asset_Solar.
*/
bool Asset::validate_config(std::map <std::string, Fims_Object*> * const asset_var_map)
{
    std::vector<const char*> missing_variables;

    // Iterate through the list of required variables
    for( auto it = required_variables.begin() ; it != required_variables.end() ; ++it )
    {
        // Construct the expected key of the form "/assets/<asset_type>/<asset_id>/<variable_name>"
        std::string variable_uri = build_assets_uri( *it );
        // Check if the variable is in the asset variables map
        if ( asset_var_map->find( variable_uri ) == asset_var_map->end() )
        {
            // If the variable is not in the asset variables map, add it to the list of missing variables
            missing_variables.push_back( *it );
        }
    }

    // Were any required variables not found and added to the missing variables list?
    if ( missing_variables.empty() )
    {
        // If no missing variables, configuration was successful
        return true;
    }
    else
    {
        // If there are missing variables, print out which ones are missing and return false to kill HybridOS
        for(auto it = missing_variables.begin(); it != missing_variables.end(); ++it)
        {
            FPS_ERROR_LOG("\nAsset::validate_config ~ CONFIG FAILURE - Variable %s of asset %s is missing or misconfigured!\n\n", *it, asset_id);
        }
        return false;
    }
}

// Tells the component to clear its faults and starts timer to clear alarm/fault registers
// Timer gives component time to clear faults and stop reporting them so registers do not get reset
void Asset::clear_component_faults(void)
{
    send_to_comp_uri(true, uri_clear_faults);
    clear_fault_registers_flag = true;
    clock_gettime(CLOCK_MONOTONIC, &clear_faults_time);
    increment_timespec_ms(clear_faults_time, 1000);
}

/**
 * Clears all latched alert values and clears all unlatched component-connected
 * alert registers. Component registers should be cleared, even if they are 
 * unlatched, because some components may only report an alert value when it is 
 * non-zero.
 */
void Asset::clear_alerts(void)
{
    // Clear the component-connected unlatched fault registers
    for(auto component_fault_register : component_fault_registers)
    {
        component_fault_register->clear_fims_bit_field();
    }

    // Clear the latched fault values
    for(auto&& latched_fault : latched_faults)
    {
        latched_fault.second = 0;
    }

    // Clear the component-connected volatile alarm registers
    for(auto component_alarm_register : component_alarm_registers)
    {
        component_alarm_register->clear_fims_bit_field();
    }
    
    // Clear the saved alarm values
    for(auto&& saved_alarm : saved_alarms)
    {
        saved_alarm.second = 0;
    }
}

/**
 * Single endpoint for all asset types to pass along the set for the given uri and valueObject to DBI.
 * More receiving modules can be added by simply adding them to the list of URIs in data_endpoint
 */
bool Asset::send_setpoint(std::string uri, cJSON* valueObject)
{
    if (valueObject == NULL || valueObject->string == NULL)
    {
        FPS_WARNING_LOG("Received NULL setpoint valueObject, Asset::send_setpoint()\n");
    }
    
    // Passing the specific endpoint is useful for parsing for pairs of opposite sets
    std::string endpoint = valueObject->string;
    return data_endpoint->setpoint_writeout(uri, endpoint, &valueObject);
}

/**
 * @brief Handles any GETs to URIs beginning with /assets/<asset type>/<asset ID>.
 * @param pmsg Pointer to the FIMS GET message.
 * @param asset_var_map Map of all asset variables.
 * @return True if GET handled successfully, false otherwise.
*/
bool Asset::handle_get(fims_message *pmsg, std::map<std::string, Fims_Object*> *asset_var_map)
{
    // clear buffer for use
    send_FIMS_buf.clear();

    // URI is /assets/<asset type>/<asset ID>
    if (pmsg->nfrags < 4) {
        auto asset_var_it = asset_var_map->begin(); // need to instantiate variable to be able to pass as reference
        if (!add_asset_data_to_buffer(send_FIMS_buf, asset_var_map, asset_var_it, strcmp(asset_type_id, ESS_TYPE_ID) != 0)) {
            return false;
        }
        return send_buffer_to(pmsg->replyto, send_FIMS_buf);
    }

    // URI must be /assets/<asset type>/<asset ID>/<variable ID>
    if (!add_variable_to_buffer(pmsg->uri, pmsg->pfrags[3], send_FIMS_buf, asset_var_map)) {
        return false;
    }
    return send_buffer_to(pmsg->replyto, send_FIMS_buf);
}

/**
 * @brief Adds all data for this asset instance to the given buffer.
 * @param buf Buffer to which the data must be added.
 * @param asset_var_map Map of all asset variables across all asset instances.
 * @param search_it Reference to an iterator of the asset_var_map. When several asset instances are being added to the same buffer, the same iterator can be used
 *                  so that the entire asset var map does not have to be re-searched each time. When only one asset instance is being added to a buffer, it is
 *                  expected that asset_var_map::begin() will be passed as the search iterator.
 * @return True if the data was added to the buffer successfully, or false if there was an error.
*/
bool Asset::add_asset_data_to_buffer(fmt::memory_buffer &buf, std::map<std::string, Fims_Object*> *asset_var_map, std::map<std::string, Fims_Object*>::iterator &search_it, bool clothed)
{
    // begin asset instance object with opening curly brace
    bufJSON_StartObject(buf);

    // name will not be in asset var map, so add it explicitly
    bufJSON_AddString(buf, "name", name);

    // add UI variables, which will not be in asset var map
    if (!generate_asset_ui(buf)) {
        FPS_ERROR_LOG("Error adding asset UI variables to data object for %s.", asset_id);
        return false;
    }

    // search through the map of asset variables until the first variable for this asset instance is found
    std::string asset_instance_uri = "/assets/" + std::string(asset_type_id) + '/' + std::string(asset_id);
    for (; search_it != asset_var_map->end(); ++search_it) {
        if (search_it->first.compare(0, asset_instance_uri.size(), asset_instance_uri) == 0)
            break;
    }
    if (search_it == asset_var_map->end()) {
        FPS_ERROR_LOG("Could not find any asset variables for %s in the asset variable map.", asset_id);
        return false;
    }

    // add every asset variable belonging to this asset instance.
    // maps are sorted. continue while we have a match for this asset instance
    for (; search_it != asset_var_map->end() && search_it->first.compare(0, asset_instance_uri.size(), asset_instance_uri) == 0; ++search_it) {
        // error checking
        if (search_it->second == NULL) {
            FPS_ERROR_LOG("NULL Fims_Object found while adding asset variables to object for asset %s.", asset_id);
            return false;
        }
        if (search_it->second->get_name() == NULL) {
            FPS_ERROR_LOG("NULL Fims_Object name found while adding asset variables to object for asset %s.", asset_id);
            return false;
        }
        // add asset variable
        search_it->second->add_to_JSON_buffer(buf, NULL, clothed);
    }

    // end asset instance object with closing curly brace
    bufJSON_EndObjectNoComma(buf);
    return true;
}

/**
 * @brief Adds the identified variable's data to the given buffer.
 * @param uri URI of the desired variable.
 * @param variable_id ID of the desired variable.
 * @param buf Buffer to which the variable's data must be added.
 * @param asset_var_map Map of all asset variables.
 * @return True if the data was added to the buffer successfully, or false if there was an error.
*/
bool Asset::add_variable_to_buffer(std::string uri, const char* variable_id, fmt::memory_buffer &buf, std::map<std::string, Fims_Object*> *asset_var_map)
{
    // component controls are nested within the variable for which they are the control,
    // so the appropriate URI to search for in the map (if looking for component control)
    // is the URI without the component control suffix at the end
    trim_suffix_if_exists(uri, COMPONENT_CONTROL_SUFFIX);

    // manually find the first match in the map
    // TODO: can we just index into the map with the provided URI?
    auto asset_it = asset_var_map->begin();
    for (asset_it = asset_it; asset_it != asset_var_map->end(); asset_it++) {
        if (asset_it->first.compare(0, uri.size(), uri) == 0)
            break;
    }

    // target variable was found in asset var map
    if (asset_it != asset_var_map->end()) {
        // error checking
        if (!asset_it->second || !asset_it->second->get_variable_id()) {
            FPS_ERROR_LOG("%s add_variable_to_buffer found NULL Fims_Object or Fims_Object with NULL variable_id.", asset_type_id);
            return false;
        }
        
        // start clothed objects with an opening curly brace
        bool clothed = strcmp(asset_type_id, ESS_TYPE_ID) != 0;
        if (clothed) bufJSON_StartObject(buf);
        // add the Fims_Object data
        asset_it->second->add_to_JSON_buffer(buf, variable_id, clothed);
        // end clothed objects with closing curly brace
        if(clothed) bufJSON_EndObjectNoComma(buf);
    }
    // target variable was NOT found in asset var map. check if the request is for a UI control
    else {
        // UI controls are always clothed bodies so start and end with curly braces
        bufJSON_StartObject(buf);
        generate_asset_ui(buf, variable_id);
        bufJSON_EndObjectNoComma(buf);
    }

    if (to_string(buf).substr(0,2) == "{}") {
        FPS_ERROR_LOG("Failed to add variable %s of asset %s to buffer.", variable_id, asset_type_id);
        return false;
    }
    return true;
}

void Asset::find_first_appearance_in_asset_var_map(const char* variable_id, std::map<std::string, Fims_Object*> *asset_var_map, std::map<std::string, Fims_Object*>::iterator &search_it)
{
    std::string search_uri = "/assets/" + std::string(asset_type_id) + std::string(asset_id);
    if (variable_id != NULL) {
        search_uri += '/' + variable_id;
    }

    for (; search_it != asset_var_map->end(); ++search_it) {
        if (search_it->first.compare(0, search_uri.size(), search_uri) == 0)
            return;
    }
}

bool Asset::process_set(std::string uri, cJSON* fimsBody)
{
    cJSON* maint_mode_obj = NULL;
    cJSON* lock_mode_obj = NULL;
    cJSON* lockValueObject = NULL;
    cJSON* maintValueObject = NULL;  
    if (fimsBody == NULL)
    {
        //FPS_DEBUG_LOG("Fims Body is Null; exiting out\n");
        return false;
    }
    maint_mode_obj = grab_naked_or_clothed(fimsBody, maint_mode_obj, "maint_mode");
    lock_mode_obj = grab_naked_or_clothed(fimsBody, lock_mode_obj, "lock_mode");
    if (lock_mode_obj == NULL && maint_mode_obj == NULL)
        return false;

    //received maintenance mode update
    if (maint_mode_obj != NULL) {
        maintValueObject = cJSON_GetObjectItem(maint_mode_obj, "value");
        FPS_INFO_LOG("Received manual mode message\n");
        if ((inMaintenance == false) && (maintValueObject->valueint))
        {
            FPS_INFO_LOG("Switching asset to manual mode\n");
            char msgMess[512];
            sprintf(msgMess, "Maintenance Mode entered: %s asset: %s", get_asset_type(), get_name());
            emit_event("Assets", msgMess, 1);
            inMaintenance = true;
            inLockdown = false;
            lock_mode.enabled = true;
        }
        else if ((inMaintenance == true) && (!maintValueObject->valueint) && (inLockdown == false))
        {
            FPS_INFO_LOG("Switching asset out of manual mode\n");
            char msgMess[512];
            sprintf(msgMess, "Maintenance Mode exited: %s asset: %s", get_asset_type(), get_name());
            emit_event("Assets", msgMess, 1);
            inMaintenance = false;
            inLockdown = false;
            lock_mode.enabled = false;
        }
        else {
            FPS_INFO_LOG("Cannot change manual mode status; in lockdown\n");
            char msgMess[512];
            sprintf(msgMess, "Cannot change maintenance mode of %s asset: %s; in lockdown", get_asset_type(), get_name());
            emit_event("Assets", msgMess, 1);
            return false;
        }
        return send_setpoint(uri, maint_mode_obj);
    }

    //received lockdown mode update
    else {        
        lockValueObject = cJSON_GetObjectItem(lock_mode_obj, "value");
        FPS_INFO_LOG("Received lockdown message\n");
        // allows lockdown sets only when in maintenance mode
        if((inMaintenance == true) && (!lockValueObject->valueint)) {
            FPS_INFO_LOG("Switching asset out of lock mode\n");
            char msgMess[512];
            sprintf(msgMess, "Lockdown Mode exited: %s asset: %s", get_asset_type(), get_name());
            emit_event("Assets", msgMess, 1);
            inLockdown = false;
            maint_mode.enabled = true;
        }
        else if((inMaintenance == true) && (lockValueObject->valueint)) {
            FPS_INFO_LOG("Switching asset into lock mode\n");
            char msgMess[512];
            sprintf(msgMess, "Lockdown Mode entered: %s asset: %s", get_asset_type(), get_name());
            emit_event("Assets", msgMess, 1);
            inLockdown = true;
            maint_mode.enabled = false;
        }
        else {
            FPS_INFO_LOG("Cannot change lock mode status; not in manual mode\n");
            char msgMess[512];
            sprintf(msgMess, "Cannot change lockdown mode of %s asset: %s; not in maintenance", get_asset_type(), get_name());
            emit_event("Assets", msgMess, 1);
            //default lockdown to false
            inLockdown = false;
            return false;
        }
        // Echo set to storage db            
        return send_setpoint(uri, lock_mode_obj);
    }   
}

/**
 * Publish handling moved to Asset_Manager
 * This function now handles any asset level processing that cannot be accomplished within Asset_Manager
 * Used to update the Asset-instance level status based on publishes from components
 * Alarm/fault publish updates are not necessary as they share pointers with the Fims_Objects updated in
 * Asset_Manager, and we want to perserve their Asset-instance level values to check for updates
 * @param names The names of the active status
 * @param value The bit field value
 */
bool Asset::process_status_pub(std::vector<std::string>* names, uint64_t value)
{
    // Only used for status which uses the return value as its index
    // No need to aggregate bit field value based on positions processed
    if ( !names->empty() && value < MAX_STATUS_BITS)
    {
        raw_status = value;
        statusStrings[value] = names->at(value).c_str();
        return true;
    }
    return false;
}

void Asset::process_asset(void)
{
    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    if (clear_fault_registers_flag && check_expired_time(now, clear_faults_time))
    {
        clear_alerts();
        clear_fault_registers_flag = false;
        send_to_comp_uri(false, uri_clear_faults); // end clear faults pulse to component
    }

    // process component fault registers
    bool previously_no_faults = get_num_active_faults() == 0;
    for (auto fault_register : component_fault_registers)
    {
        // if the fault register contains a non-zero value, the component is reporting a fault
        if (fault_register->value.value_bit_field > 0)
        {
            // find the corresponding latched fault value
            auto latched_fault_it = latched_faults.find(fault_register->get_variable_id());
            if (latched_fault_it == latched_faults.end())
            {
                // map initialized incorrectly when processing configuration
                FPS_ERROR_LOG("Fault value undefined for %s in %s process_asset()\n", fault_register->get_variable_id(), get_id());
                continue;
            }

            // check the latched fault value against the received fault value
            if (latched_fault_it->second != fault_register->value.value_bit_field)
            {
                // check for new faults
                uint64_t new_faults = ~latched_fault_it->second & fault_register->value.value_bit_field;
                if (new_faults != 0)
                {
                    char faultMsg[1024];
                    clearFaultsControlEnable = true;
                    // translate fault bit to fault message
                    for (int i = 0; new_faults != 0; i++)
                    {
                        // The current bit to check
                        if ((new_faults & 1) == 1)
                        {
                            snprintf(faultMsg, 1024, "Fault received :  %s, asset: %s", fault_register->options_name[i].c_str(), name);
                            emit_event("Assets", faultMsg, 4);
                        }
                        new_faults >>= 1;
                    }
                }
            }
            // update the latched fault value
            latched_fault_it->second = fault_register->value.value_bit_field;
        }
    }
    // if there were previously no faults and there is now a fault, mark this asset as newly faulted
    newly_faulted = (previously_no_faults && get_num_active_faults() != 0);

    // process component alarm registers
    for (auto alarm_register : component_alarm_registers)
    {
        // find the corresponding saved alarm value
        auto saved_alarm = saved_alarms.find(alarm_register->get_variable_id());
        if (saved_alarm == saved_alarms.end())
        {
            // map initialized incorrectly on processing configuration
            FPS_ERROR_LOG("alarm value undefined for %s in %s process_asset()\n", alarm_register->get_variable_id(),  get_id());
            break;
        }
        uint64_t old_alarm_value = saved_alarm->second;

        // if the alarm register contains a non-zero value, the component is reporting an alarm
        if (alarm_register->value.value_bit_field > 0)
        {
            // check the saved alarm value against the received alarm value
            if (old_alarm_value != alarm_register->value.value_bit_field)
            {
                // check for new alarms
                uint64_t new_alarms = ~old_alarm_value & alarm_register->value.value_bit_field;
                if (new_alarms != 0)
                {
                    char alarmMsg[1024];
                    clearFaultsControlEnable = true;
                    // translate fault bit to fault message
                    for (int i = 0; new_alarms != 0; i++)
                    {
                        if ((new_alarms & 1) == 1)
                        {
                            snprintf(alarmMsg, 1024, "alarm received :  %s, asset: %s", alarm_register->options_name[i].c_str(), name);
                            emit_event("Assets", alarmMsg, 3);
                        }
                        new_alarms >>= 1;
                    }
                }
            }
        }

        // update the saved alarm value
        saved_alarm->second = alarm_register->value.value_bit_field;
    }

    if (status_type == bit_field)
    {
        if (raw_status != status)
        {
            // alert for new status
            char statusMsg[1024];
            snprintf(statusMsg, 1024, "Asset %s status changed to:  %s", name, statusStrings[raw_status]);
            emit_event("Assets", statusMsg, 2);
            status = raw_status;
        }
    }
    else if (status_type == random_enum)
    {
        // emit event if status has changed
        if (status != raw_status)
        {
            char statusMsg[1024];
            snprintf(statusMsg, 1024, "Asset %s status changed to:  %s", name, statusStrings[raw_status]);
            emit_event("Assets", statusMsg, 2);
            status = raw_status;
        }
    }
    else
    {
        FPS_ERROR_LOG("Asset::process_asset - invalid status type received\n");
    }
    
    // process available status: only available if there are no faults, asset is not in maintenance mode, fims has not timed out, and connected
    if(watchdog_enable){
        modbus_connected->value.value_bool = !check_fims_timeout() && component_connected->value.value_bool;
        isAvail = (get_num_active_faults() == 0) && !inMaintenance && modbus_connected->value.value_bool;
        if(connected_rising_edge_detect != component_connected->value.value_bool)
        {
            char msg[1024];
            if(component_connected->value.value_bool)
            {  
                sprintf(msg, "The Asset %s was connected", get_name());
                emit_event("Assets", msg, 2);
            }
            else
            {
                sprintf(msg, "The Asset %s was disconnected", get_name());
                emit_event("Assets", msg, 3);
            }
            connected_rising_edge_detect = component_connected->value.value_bool;   
        }
    }
    else
        isAvail = (get_num_active_faults() == 0) && !inMaintenance;

    // process running status
    if (status_type == random_enum)
        isRunning = status == running_status_mask;
    else if (status_type == bit_field)
        // Check that the bit in the position given by the status value is valid
        // e.g. valid running states: 4, 5; mask (binary): 110000 (start counting from 0)
        // for status value 4, verify: 110000 & 010000
        isRunning = (running_status_mask & (1 << status));

    // process potential power values
    process_potential_active_power();
    process_potential_reactive_power();
}

bool Asset::is_available(void)
{
    return isAvail;
}

bool Asset::is_running(void)
{
    return isRunning;
}

demandMode Asset::get_demand_mode(void) const
{
    if (inMaintenance)
        return Uncontrolled;
    return assetControl;
}

bool Asset::is_controllable(void)
{
    if (isAvail && isRunning && !inStandby && (get_demand_mode() != Uncontrolled))
        return true;
    else
        return false;
}

bool Asset::in_maint_mode(void)
{
    return inMaintenance;
}

bool Asset::in_standby(void)
{
    return inStandby;
}

char* Asset::get_name()
{
    return name;
}

char* Asset::get_id()
{
    return asset_id;
}

const char* Asset::get_asset_type(void) const
{
    return asset_type_id;
}

int Asset::get_num_comps(void) const
{
    return numAssetComponents;
}

const char* Asset::get_comp_name(int i) const
{
    return compNames[i];
}

uint64_t Asset::get_status(void) const
{
    return status;
}

/**
 * Iterates across all saved alarm values and counts how many are active.
 * @return Number of active alarms.
 */
int Asset::get_num_active_alarms(void) const
{
    int num_alarms = 0;
    for (auto&& name_value_pair : saved_alarms)
    {
        if ( name_value_pair.second > 0 )
            ++num_alarms;
    }
    return num_alarms;
}

/**
 * Iterates across all latched fault values and counts how many are active.
 * @return Number of active faults.
 */
int Asset::get_num_active_faults(void) const
{
    int num_faults = 0;
    for (auto&& name_value_pair : latched_faults)
    {
        if ( name_value_pair.second > 0 )
            ++num_faults;
    }
    return num_faults;
}

/**
 * @return The rising edge of an asset being faulted.
 */
bool Asset::is_newly_faulted(void) const
{
    return newly_faulted;
}

/**
 * Checks if there is an active alarm or an active fault with the given ID.
 * @param id ID of the alert to check.
 * @param mask 64-bit mask to apply to the alert value.
 * @return True if there is an alert with the given ID and its masked value is non-zero, indicating it is active. False otherwise.
 */
bool Asset::check_alert(std::string& id, uint64_t& mask)
{
    return check_fault(id, mask) || check_alarm(id, mask);
}

/**
 * Checks if there is an active fault with the given ID.
 * @param id ID of the fault to check.
 * @param mask 64-bit mask to apply to the fault value.
 * @return True if there is a fault with the given ID and its masked value is non-zero, indicating it is active. False otherwise.
 */
bool Asset::check_fault(std::string& id, uint64_t& mask)
{
    auto fault = latched_faults.find(id);
    if (fault != latched_faults.end())
    {
        return fault->second & mask;
    }
    return false; // if fault ID is not found in map, then consider it not active
}

/**
 * Checks if there is an active alarm with the given ID.
 * @param id ID of the alarm to check.
 * @param mask 64-bit mask to apply to the alarm value.
 * @return True if there is an alarm with the given ID and its masked value is non-zero, indicating it is active. False otherwise.
 */
bool Asset::check_alarm(std::string& id, uint64_t& mask)
{
    auto alarm = saved_alarms.find(id);
    if (alarm != saved_alarms.end())
    {
        return alarm->second & mask;
    }
    return false; // if alarm ID is not found in map, then consider it not active
}

float Asset::get_voltage_l1_l2(void) const
{
    return voltage_l1_l2->value.value_float;
}

float Asset::get_voltage_l2_l3(void) const
{
    return voltage_l2_l3->value.value_float;
}

float Asset::get_voltage_l3_l1(void) const
{
    return voltage_l3_l1->value.value_float;
}

float Asset::get_voltage_l1_n(void) const
{
    return voltage_l1_n->value.value_float;
}

float Asset::get_voltage_l2_n(void) const
{
    return voltage_l2_n->value.value_float;
}

float Asset::get_voltage_l3_n(void) const
{
    return voltage_l3_n->value.value_float;
}

float Asset::get_voltage_avg_line_to_line(void) const
{
    float sumVolts = voltage_l1_l2->value.value_float + voltage_l2_l3->value.value_float + voltage_l3_l1->value.value_float;
    return (numPhases != 0.0 ? sumVolts/numPhases : 0);
}

float Asset::get_voltage_avg_line_to_neutral(void) const
{
    float sumVolts = voltage_l1_n->value.value_float + voltage_l2_n->value.value_float + voltage_l3_n->value.value_float;
    return (numPhases != 0.0 ? sumVolts/numPhases : 0);
}

float Asset::get_current_l1(void) const
{
    return current_l1->value.value_float;
}

float Asset::get_current_l2(void) const
{
    return current_l2->value.value_float;
}

float Asset::get_current_l3(void) const
{
    return current_l3->value.value_float;
}

float Asset::get_current_avg(void) const
{
    float sumCurrent = current_l1->value.value_float + current_l2->value.value_float + current_l3->value.value_float;
    return (numPhases != 0.0 ? sumCurrent/numPhases : 0);
}

float Asset::get_power_factor(void) const
{
    return power_factor->value.value_float;
}

float Asset::get_active_power(void) const
{
    return active_power->value.value_float;
}

float Asset::get_reactive_power(void) const
{
    return reactive_power->value.value_float;
}

float Asset::get_apparent_power(void) const
{
    return apparent_power->value.value_float;
}

float Asset::get_max_potential_active_power(void)
{
    return max_potential_active_power;
}

float Asset::get_min_potential_active_power(void)
{
    return min_potential_active_power;
}

float Asset::get_max_limited_active_power(void)
{
    return active_power_limit;
}

float Asset::get_potential_reactive_power(void)
{
    return potential_reactive_power;
}

int Asset::get_active_power_slew_rate(void)
{
    return active_power_slew.get_slew_rate();
}

float Asset::get_active_power_slew_max_target(void)
{
    return active_power_slew.get_max_target();
}

float Asset::get_active_power_slew_min_target(void)
{
    float value = active_power_slew.get_min_target();
    return (value < 0 ) ? 0 : value;
}

float Asset::get_active_power_slew_target(float target)
{
    return active_power_slew.get_slew_target(target);
}

void Asset::set_reactive_power_priority(bool priority)
{
    reactive_power_priority = priority;
}

void Asset::process_potential_active_power(void)
{
    // Set the slew to the active power setpoint calculated by Site Manager or set manually
    active_power_slew.update_slew_target(active_power_setpoint->component_control_value.value_float);

    // max value is capped based on rated power
    max_potential_active_power = active_power_slew.get_max_target();
    max_potential_active_power = max_limited_active_power = (max_potential_active_power > rated_active_power_kw) ? rated_active_power_kw : max_potential_active_power;

    // min value is capped at 0KW
    min_potential_active_power = active_power_slew.get_min_target();
    min_potential_active_power = min_limited_active_power = zero_check(min_potential_active_power);

    // Base active power limit reference
    active_power_limit = rated_active_power_kw;
}

void Asset::process_potential_reactive_power(void)
{
    potential_reactive_power = rated_reactive_power_kvar;
    if (!reactive_power_priority)
    {
        float reactive_power_limit = sqrtf(powf(rated_apparent_power_kva, 2) - powf(active_power_setpoint->value.value_float, 2));
        potential_reactive_power = std::min(potential_reactive_power, reactive_power_limit);
    }
}

float Asset::get_rated_active_power(void) const
{
    return rated_active_power_kw;
}

float Asset::get_rated_reactive_power(void) const
{
    return rated_reactive_power_kvar;
}

float Asset::get_rated_apparent_power(void) const
{
    return rated_apparent_power_kva;
}

// This function takes as arguments an asset id and a variable id and returns
// an assets URI of the form "/assets/<asset_type>/<asset id>/<variable id>"
// Changed from malloc'd string pointer to std::string which is deallocated when outside scope
std::string Asset::build_assets_uri(const char* var)
{
    char uriWithAsset[1024];
    snprintf(uriWithAsset, 1024, "/assets/%s/%s/%s", asset_type_id, asset_id, var);
    return std::string(uriWithAsset);
}

/*
    Asset Manager owns a map called component_var_map which holds pointers to all component variables across all assets.
    The key is a URI with the following form: /components/<component_id>/<register_id> and the value is a pointer
    to the Fims_Object that holds the corresponding variable.

    Asset Manager also owns a map called asset_var_map which holds pointers to the same variable objects that component_var_map
    points to. However, the keys are of the following form: /assets/<asset_type>/<asset_id>/<variable_name>.

    This function, var_maps_insert, is called on each component variable during configuration from Asset_<Type>::configure(). 
    It takes the component ID and extracts the register ID to create the URI. It uses the header string as the variable name 
    in the asset var map. It parses the variable object to gather the provided information about the variable. Finally it 
    inserts the URI/pointer pairs into the component variables map and the asset variables map.
*/
bool Asset::var_maps_insert(cJSON* varJson, char* compID, std::map <std::string, std::vector<Fims_Object*>> * const component_var_map,
                            std::map <std::string, Fims_Object*> * const asset_var_map, bool* is_primary)
{
    // The Fims_Object will hold all the known information about the component variable
    Fims_Object* component_variable;
    try
    {
        component_variable = new Fims_Object();
    }
    catch(const std::exception& e)
    {
        FPS_ERROR_LOG("Asset::var_maps_insert ~ Error allocating memory for component variable!\n");
        return false;
    }

    // Give Fims_Object pointer to is_primary
    component_variable->is_primary = is_primary;

    // The variable string is the name of the component variable object in assets.json. Also is what UI expects in pubs
    char* variable_string = varJson->string;
    component_variable->set_variable_id(variable_string);

    // The "name" field is a very succinct description of the variable that can be multiple words with spaces, capital letters, etc.
    cJSON* variable_name = cJSON_HasObjectItem(varJson, "name") ? cJSON_GetObjectItem(varJson, "name") : NULL;
    if (variable_name == NULL || variable_name->valuestring == NULL)
    {   // The variable name is required
        FPS_ERROR_LOG("Asset::var_maps_insert ~ Variable name is NULL for component %s variable %s.\n", compID, variable_string);
        return false;
    }
    component_variable->set_name(variable_name->valuestring);

    // Asset Manager should expect to receive updates from the component as pubs
    // The register id is what the component will use in its body when it pubs
    if ( !cJSON_HasObjectItem(varJson, "register_id") )
    {   // If a component variable object does not have a register id, then it doesn't belong in the component variables map
        delete component_variable;
        return true;
    }
    cJSON* register_id = cJSON_GetObjectItem(varJson, "register_id");
    if (register_id == NULL || register_id->valuestring == NULL)
    {
        FPS_ERROR_LOG("Asset::var_maps_insert ~ register_id is NULL for component %s variable %s.\n", compID, variable_string);
        return false;
    }
    component_variable->set_register_id(register_id->valuestring);

    // The "ui_type" field tells the UI how to display the variable: status, control, fault, alarm, ...
    cJSON* ui_type = cJSON_HasObjectItem(varJson, "ui_type") ? cJSON_GetObjectItem(varJson, "ui_type") : NULL;
    if (ui_type != NULL && ui_type->valuestring)
    {
        // Check against list of valid ui_types
        bool valid = false;
        for (int i = 0; i < NUM_UI_TYPES; i++)
        {
            if (strcmp(UI_Type_Names[i], ui_type->valuestring) == 0)
            {
                valid = true;
                break;
            }
        }
        // Set to valid ui_type or float if invalid
        if (!valid)
        {
            FPS_ERROR_LOG("Asset::var_maps_insert ~ invalid ui_type provided %s variable %s.\n", ui_type->valuestring, variable_string);
            return false;
        }
            component_variable->set_ui_type(ui_type->valuestring);

    }
    else
    {   // If no ui_type given, default is "status"
        component_variable->set_ui_type( "status" );
    }

    // The "type" field is used to know which value to use within Fims_Object's Value_Object
    cJSON* type = cJSON_HasObjectItem(varJson, "type") ? cJSON_GetObjectItem(varJson, "type") : NULL;
    if (type != NULL && type->valuestring)
    {
        // Check against list of valid types
        if (strcmp(type->valuestring,"Float") == 0 ||
            strcmp(type->valuestring,"Int") == 0 ||
            strcmp(type->valuestring,"Bool") == 0 ||
            strcmp(type->valuestring,"Bit String") == 0 ||
            strcmp(type->valuestring,"Random Enum") == 0 ||
            strcmp(type->valuestring,"Status") == 0)
        {
            component_variable->set_type(type->valuestring);
        }
        else
        {
            FPS_ERROR_LOG("Asset::var_maps_insert ~ invalid type given for component %s variable %s.\n", compID, variable_string);
            return false;
        }
    }
    else
    {   // If no type given, default is "Float"
        component_variable->set_type( "Float" );
    }

    // The "scaler" field is used for displaying the variable correctly on the UI
    cJSON* scaler = cJSON_HasObjectItem(varJson, "scaler") ? cJSON_GetObjectItem(varJson, "scaler") : NULL;
    if (scaler != NULL)
    {   // The scaler is not required
        component_variable->scaler = scaler->valueint;
    }
    else if ( strcmp(component_variable->get_type(),"Int")==0 || strcmp(component_variable->get_type(),"Float")==0 )
    {   // If no scaler is given for an int or a float, default is 1
        component_variable->scaler = 1;
    }

    // The "value" field can be used to initialize the variable
    cJSON* initial_value = cJSON_HasObjectItem(varJson, "value") ? cJSON_GetObjectItem(varJson, "value") : NULL;
    // default values if value field is not provided
    float initial_double = 0;
    int initial_int = 0;
    bool initial_bool = false;
    // overwrite default values if value field is provided
    if (initial_value != NULL)
    {
        initial_double = initial_value->valuedouble;
        initial_int = initial_value->valueint;
        initial_bool = initial_value->type == cJSON_True;
    }
    // set Fims_Object value variables based on type and default or overwritten initial value
    if ( strcmp(component_variable->get_type(), "Float") == 0 )
    {
        component_variable->set_fims_float( variable_string, initial_double );
        component_variable->component_control_value.set( initial_double );
        component_variable->set_value_type( Float );
    }
    else if ( strcmp(component_variable->get_type(), "Int") == 0 )
    {
        component_variable->set_fims_int( variable_string, initial_int );
        component_variable->component_control_value.set( initial_int );
        component_variable->set_value_type( Int );
    }
    else if ( strcmp(component_variable->get_type(), "Bool") == 0 )
    {
        component_variable->set_fims_bool( variable_string, initial_bool );
        component_variable->component_control_value.set( initial_bool );
        component_variable->set_value_type( Bool );
    }
    else if ( strcmp(component_variable->get_type(), "Status") == 0 )
    {
        component_variable->set_fims_bit_field( variable_string, uint64_t(initial_int) );
        component_variable->component_control_value.set( uint64_t(initial_int) );
        component_variable->set_value_type( Status );
    }
    else if ( strcmp(component_variable->get_type(), "Bit Field") == 0 )
    {
        component_variable->set_value_type( Bit_Field );
    }
    else if ( strcmp(component_variable->get_type(), "Random Enum") == 0 )
    {
        component_variable->set_value_type( Random_Enum );
    }
    
    // The initial mask of the variable
    // Unused unless the variable is an alarm/fault
    cJSON* initial_mask = cJSON_HasObjectItem(varJson, "mask") ? cJSON_GetObjectItem(varJson, "mask") : NULL;
    if (initial_mask != NULL)
    {
        // Read in mask as string to support 64 bit width
        component_variable->value.value_mask = (uint64_t) std::stoul(initial_mask->valuestring, NULL, 16);
    }
    else
    {
        // Represents 64 digits of 1
        component_variable->value.value_mask = 0xFFFFFFFFFFFFFFFF;
    }
    

    // The "unit" field is useful for the UI display
    cJSON* variable_unit = cJSON_HasObjectItem(varJson, "unit") ? cJSON_GetObjectItem(varJson, "unit") : NULL;
    if (variable_unit != NULL && variable_unit->valuestring != NULL)
    {   // The variable unit is not required
        component_variable->set_unit(variable_unit->valuestring);
    }
    else
    {
        component_variable->set_unit("");
    }
    

    // both uri build functions create a C-style string, so use the string constructor function to convert it to a string object
    // build_uri prepends "/components/"
    std::string components_uri( build_uri(compID, register_id->valuestring) );
    // build_assets_uri prepends "/assets/<asset_type>/"
    std::string assets_uri = build_assets_uri(variable_string);
    
    // Component uri is held by Fims_Object's `component_uri` member variable
    component_variable->set_component_uri( components_uri.c_str() );
    // The component var map supports having multiple Fims_Objects source their values from the same component register
    // If there is already a configured variable that uses this component URI, add this variable to that list
    if ( component_var_map->find(components_uri) != component_var_map->end() )
    {
        (*component_var_map)[components_uri].push_back(component_variable);
    }
    // If there is not already a configured variable that uses this component URI, make a new list for the URI and add it to the map
    else
    {
        component_var_map->insert(std::make_pair(components_uri, std::vector<Fims_Object*>({component_variable})));
    }
    // Insert the Fims_Object pointer into the assets variable map
    asset_var_map->insert(std::make_pair(assets_uri, component_variable));

    // Add to the alarm/fault data objects as appropriate
    if (strcmp(component_variable->get_ui_type(), "alarm") == 0)
    {
        // insert pointer to Fims_Object into list of component alarm registers
        component_alarm_registers.push_back(component_variable);
        // record alarm ID in map of alarm values with initial value 0
        saved_alarms.insert(std::make_pair(component_variable->get_variable_id(), 0));
    }
    else if (strcmp(component_variable->get_ui_type(), "fault") == 0)
    {
        // insert pointer to Fims_Object into list of component fault registers
        component_fault_registers.push_back(component_variable);
        // record fault ID in map of fault values with initial value 0
        latched_faults.insert(std::make_pair(component_variable->get_variable_id(), 0));
    }

    return true;
}

bool fimsCtl::configure(cJSON* cfg_json, jsonBuildOption build_option, void* display, valueType value_type_cfg, displayType display_type_cfg, bool enabled_cfg, bool is_bool_string)
{
    if (configured == true) {
        FPS_ERROR_LOG("UI control object configured more than once.");
        return false;
    }

    if (cfg_json == NULL) {
        FPS_ERROR_LOG("NULL configuration object given for UI control.");
        return false;
    }
    obj_name = strdup(cfg_json->string);

    cJSON *control_name_json;
    if ((control_name_json = cJSON_GetObjectItem(cfg_json, "name")) == NULL || (control_name_json->valuestring == NULL)) {
        FPS_ERROR_LOG("No 'name' field given in UI control configuration object.");
        return false;
    }
    varName = strdup(control_name_json->valuestring);

    cJSON *register_id_json = cJSON_GetObjectItem(cfg_json, "register_id");
    if (register_id_json != NULL && register_id_json->valuestring != NULL)
        reg_name = strdup(register_id_json->valuestring);

    cJSON *scaler_json;
    if ((scaler_json = cJSON_GetObjectItem(cfg_json, "scaler")) == NULL)
        scaler = 1;
    else
        scaler = scaler_json->valueint;

    cJSON *unit_json;
    if (((unit_json = cJSON_GetObjectItem(cfg_json, "unit")) == NULL) || (unit_json->valuestring == NULL))
        unit = strdup("");
    else
        unit = strdup(unit_json->valuestring);


    options = build_option;
    enabled = enabled_cfg;
    vt = value_type_cfg;
    uiType = display_type_cfg;
    boolString = is_bool_string;
    pDisplay = display;
    configured = true;
    return true;
}

bool Asset::json_object_send(std::string &value, const char* uri)
{
    // Disable publish if second controller (shadow mode)
    if (!*is_primary)
        return true;

    if (value.empty() || (uri == NULL))
    {
        FPS_ERROR_LOG("Asset::json_object_send - value or uri does not exist\n");
        return false;
    }

    p_fims->Send("set", uri, NULL, value.c_str());
    return true;
}

void Write_Rate_Throttle::reset(void)
{
    status_time = 0;
    control_feedback = 0;
}

void Write_Rate_Throttle::configure(long timeout, float rated, float deadband)
{
    throttle_timeout = timeout; // throttle_timeout is in ms
    rated_power = rated;
    deadband_percentage = deadband;
}

bool Write_Rate_Throttle::command_trigger(void)
{
    long control_time = current_timestamp();
    if((control_time - status_time) > throttle_timeout)
    {
        status_time = control_time; //Make status_time the time of most recent write
        return true;
    }
    return false;
}

bool Write_Rate_Throttle::setpoint_trigger(float control_power)
{
    long control_time = current_timestamp();
    float delta = control_feedback - control_power; //Difference between current active power and incoming active power
    float percent_power = rated_power*deadband_percentage; //Amount of power that will trigger an instant write instead of waiting for timeout

    if((fabsf(delta) > percent_power) || ((control_time - status_time) > throttle_timeout)) //Any negative deltas will be positive
    {
        control_feedback = control_power; //Make control_feedback teh value that was "last written"
        status_time = control_time; //Make status_time the time of most recent write
        return true;
    }

    return false;
}

long Write_Rate_Throttle::current_timestamp(void)
{
    timespec now;
    int rc = clock_gettime(CLOCK_MONOTONIC, &now);
    if (rc != 0) return 0;
    return ((now.tv_sec*1000) + (now.tv_nsec/1000000)); // convert everything into ms
}

bool Asset::send_to_comp_uri(bool value, const char* uri)
{
    // Clear buffer for use
    send_FIMS_buf.clear();

    if (uri == NULL)
    {
        FPS_ERROR_LOG("Asset::send_to_comp_uri (bool) - uri does not exist\n");
        return false;
    }
    
    bufJSON_StartObject(send_FIMS_buf); // object {
    bufJSON_AddBool(send_FIMS_buf, "value", value);
    bufJSON_EndObject(send_FIMS_buf); // } object

    bufJSON_RemoveTrailingComma(send_FIMS_buf);
    std::string object = to_string(send_FIMS_buf);

    json_object_send(object, uri);
    return true;
}

bool Asset::send_to_comp_uri(const char* value, const char* uri)
{
    // Clear buffer for use
    send_FIMS_buf.clear();

    if ((value == NULL) || (uri == NULL))
    {
        FPS_ERROR_LOG("Asset::send_to_comp_uri (char) - value or uri does not exist\n");
        return false;
    }
    
    bufJSON_StartObject(send_FIMS_buf); // object {
    bufJSON_AddString(send_FIMS_buf, "value", value);
    bufJSON_EndObject(send_FIMS_buf); // } object
    
    bufJSON_RemoveTrailingComma(send_FIMS_buf);
    std::string object = to_string(send_FIMS_buf);

    json_object_send(object, uri);
    return true;
}

bool Asset::send_to_comp_uri(float value, const char* uri)
{
    // Clear buffer for use
    send_FIMS_buf.clear();

    if (uri == NULL)
    {
        FPS_ERROR_LOG("Asset::send_to_comp_uri (float) - uri does not exist\n");
        return false;
    }
    
    bufJSON_StartObject(send_FIMS_buf); // object {
    bufJSON_AddNumber(send_FIMS_buf, "value", value);
    bufJSON_EndObject(send_FIMS_buf); // } object

    bufJSON_RemoveTrailingComma(send_FIMS_buf);
    std::string object = to_string(send_FIMS_buf);

    json_object_send(object, uri);
    return true;
}

bool Asset::send_to_comp_uri(int value, const char* uri)
{
    // Clear buffer for use
    send_FIMS_buf.clear();

    if (uri == NULL)
    {
        FPS_ERROR_LOG("Asset::send_to_comp_uri (int) - uri does not exist\n");
        return false;
    }
    
    bufJSON_StartObject(send_FIMS_buf); // object {
    bufJSON_AddNumber(send_FIMS_buf, "value", value);
    bufJSON_EndObject(send_FIMS_buf); // } object

    bufJSON_RemoveTrailingComma(send_FIMS_buf);
    std::string object = to_string(send_FIMS_buf);

    json_object_send(object, uri);
    return true;
}

/**
 * @brief Used to make json representations of ui controls. If configurable_asset = true then include less information. 
 * 
 * @param buf JSON fmt::memory_buffer
 * @param var A specific variable
 * @param configurable_asset This asset has been preconfigured and can include less information
 * @return true if the buffer was added successfully  
 * @return false if an error occured modifying the buffer
 */
bool fimsCtl::makeJSONObject(fmt::memory_buffer &buf, const char* const var, bool configurable_asset)
{
    // skip building JSON object for UI controls that are not configured
    if (!configured) {
        return true;
    }
    
    if (var == NULL)
    {
        bufJSON_AddId(buf, obj_name);
        bufJSON_StartObject(buf); // UiItemVar {
    }
    // Check if this is not the var we want and return if it isn't
    else if (strcmp(obj_name, var) != 0)
        return true;

    // if not pre-configured then add metadata 
    if (!configurable_asset) {

        // name
        if (varName != NULL)
            bufJSON_AddString(buf, "name", varName);
        else
        {
            FPS_ERROR_LOG("FIMS control has NULL variable name.");
            return false;
        }

        // unit
        if (unit !=  NULL)
            bufJSON_AddString(buf, "unit", unit);

        // scaler
        bufJSON_AddNumber(buf, "scaler", scaler);

        // ui_type
        bufJSON_AddString(buf, "ui_type", "control");

        // ui_interaction type
        if (uiType != emptyNull)
        {
            char uiTypeStr[14]; 
            switch (uiType)
            {
            case enumStr:
                strcpy(uiTypeStr,"enum");
                break;
            case numberStr:
                strcpy(uiTypeStr,"number");
                break;
            case sliderStr:
                strcpy(uiTypeStr,"enum_slider");
                break;
            case buttonStr:
                strcpy(uiTypeStr,"enum_button");
                break;
            default:
                break;
            }
            bufJSON_AddString(buf, "type", uiTypeStr);
        }
        else
        {
            FPS_ERROR_LOG("FIMS control UI type is NULL.");
            return false;
        }
    }

    // Adding the value with this switch always need the value
    switch (vt)
    {
    case Int:
        if (pDisplay !=  NULL)
            bufJSON_AddNumber(buf, "value", *(int *)pDisplay);
        break;
    case Float:
        if (pDisplay !=  NULL)
            bufJSON_AddNumber(buf, "value", *(float *)pDisplay);
        break;
    case Bool:
        if (pDisplay !=  NULL)
        {
            if (boolString)
                bufJSON_AddString(buf, "value", (*(bool *)pDisplay) ? "Closed":"Open");
            else
                bufJSON_AddBool(buf, "value", *(bool *)pDisplay);
        }
        break;
    default:
        {
            FPS_ERROR_LOG("Invalid FIMS control value type %d.", vt);
            return false;
        }
    }

    // Always need enabled when pubbed to my knowledge button controls are enabled from these pubs
    bufJSON_AddBool(buf, "enabled", enabled);

    // No way to pre-configure options always add them here.
    bufJSON_AddId(buf, "options");
    bufJSON_StartArray(buf); // UiItemOption [
    switch (options)
    {
    case onOffOption:
        build_on_off_option(buf);
        break;
    case yesNoOption:
        build_yes_no_option(buf);
        break;
    case closeOption:
        build_close_option(buf);
        break;
    case openOption:
        build_open_option(buf);
        break;
    case resetOption:
        build_reset_option(buf);
        break;
    case nullJson:
        break;
    default:
        break;
    }
    bufJSON_EndArray(buf); // ] UiItemOption
    if (var == NULL)
        bufJSON_EndObject(buf); // } UiItemVar

    return (true);
}

void build_on_off_option(fmt::memory_buffer &bufJactivePwrSetpointOption)
{
    bufJSON_StartObject(bufJactivePwrSetpointOption); // objOne {
    bufJSON_AddString(bufJactivePwrSetpointOption, "name", "On");
    bufJSON_AddBool(bufJactivePwrSetpointOption, "return_value", true);
    bufJSON_EndObject(bufJactivePwrSetpointOption); // } objOne

    bufJSON_StartObject(bufJactivePwrSetpointOption); // objTwo {
    bufJSON_AddString(bufJactivePwrSetpointOption, "name", "Off");
    bufJSON_AddBool(bufJactivePwrSetpointOption, "return_value", false);
    bufJSON_EndObject(bufJactivePwrSetpointOption); // } objTwo
}

void build_yes_no_option(fmt::memory_buffer &bufJmanualModeOption)
{
    bufJSON_StartObject(bufJmanualModeOption); // objOne {
    bufJSON_AddString(bufJmanualModeOption, "name", "No");
    bufJSON_AddBool(bufJmanualModeOption, "return_value", false);
    bufJSON_EndObject(bufJmanualModeOption); // } objOne

    bufJSON_StartObject(bufJmanualModeOption); // objTwo {
    bufJSON_AddString(bufJmanualModeOption, "name", "Yes");
    bufJSON_AddBool(bufJmanualModeOption, "return_value", true);
    bufJSON_EndObject(bufJmanualModeOption); // } objTwo
}

void build_close_option(fmt::memory_buffer &bufJstateCloseBreakerOption)
{
    bufJSON_StartObject(bufJstateCloseBreakerOption); // objOne {
    bufJSON_AddString(bufJstateCloseBreakerOption, "name", "Close");
    bufJSON_AddBool(bufJstateCloseBreakerOption, "return_value", true);
    bufJSON_EndObject(bufJstateCloseBreakerOption); // } objOne
}

void build_open_option(fmt::memory_buffer &bufJstateOpenBreakerOption)
{
    bufJSON_StartObject(bufJstateOpenBreakerOption); // objOne {
    bufJSON_AddString(bufJstateOpenBreakerOption, "name", "Open");
    bufJSON_AddBool(bufJstateOpenBreakerOption, "return_value", true);
    bufJSON_EndObject(bufJstateOpenBreakerOption); // } objOne
}

void build_reset_option(fmt::memory_buffer &bufJresetTargetOption)
{
    bufJSON_StartObject(bufJresetTargetOption); // objOne {
    bufJSON_AddString(bufJresetTargetOption, "name", "Clear Faults");
    bufJSON_AddBool(bufJresetTargetOption, "return_value", true);
    bufJSON_EndObject(bufJresetTargetOption); // } objOne
}

// This function takes as arguments a component id and a register id and returns
// a component URI of the form "/components/<component id>/<register id>"
// Changed from malloc'd string pointer to std::string which is deallocated when outside scope
std::string build_uri(char* comp, char* reg)
{
    char uriWithComp[1024];
    snprintf(uriWithComp, 1024, "/components/%s/%s", comp, reg);
    return std::string(uriWithComp);
}

// Checks if a new counter value has been seen in the past X milliseconds (where X = watchdog_timeout_ms). Returns true if X is over or false if X isn't over or has been reset
bool Asset::check_fims_timeout(void)
{
    bool prev_fims_timeout = fims_timeout;
    if(modbus_heartbeat->value.value_int != prev_modbus_heartbeat)     // New counter value seen, so reset fims_timer
    {
        prev_modbus_heartbeat = modbus_heartbeat->value.value_int;               // Store most recent counter value
        clock_gettime(CLOCK_MONOTONIC, &fims_timer);            // Get current time
        increment_timespec_ms(fims_timer, watchdog_timeout_ms); // Reset fims timeout clock
        fims_timeout = false;                                   // Return false since new counter value means fims has not timed out
    }
    else
    {
        timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);   // Get current time
        fims_timeout = check_expired_time(now, fims_timer);
    }
    if(prev_fims_timeout != fims_timeout)
    {
        char msgMess[512];
        if(fims_timeout)
        {   // If there was a FIMS timeout, emit an event
            sprintf(msgMess, "FIMS timeout detected at %s", get_name());
            emit_event("Assets", msgMess, 3);
        }
        else
        {   // If FIMS gets reconnected, emit an event
            sprintf(msgMess, "FIMS connected at %s", get_name());
            emit_event("Assets", msgMess, 2);
        }
    }
    return fims_timeout;
}
