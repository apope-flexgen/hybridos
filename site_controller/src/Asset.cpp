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

Write_Rate_Throttle::Write_Rate_Throttle() {
    throttle_timeout = 0;
    status_time = 0;

    control_feedback = 0;
    rated_power = 0;
    deadband_percentage = 0;
}

Write_Rate_Throttle::~Write_Rate_Throttle() {}

Asset::Asset() {
    // Status variables
    isAvail = false;
    isRunning = false;
    inMaintenance = false;
    inStandby = false;
    newly_faulted = false;

    // Control variables
    clearFaultsControlEnable = false;
    clear_fault_registers_flag = false;
    clock_gettime(CLOCK_MONOTONIC, &clear_faults_time);

    // Configuration variables
    running_status_mask = 0;
    nominal_voltage = 0;
    nominal_frequency = 0;

    numPhases = 3;

    prev_watchdog_heartbeat = -1;  // Value of -1 forces check_fims_timeout() to initialize fims_timer in the first function call

    // Status variables
    numAssetComponents = 0;

    status_type = invalid;
    local_mode_status_type = invalid;
    local_mode_configured = false;

    internal_status = 0;
    raw_status = 0;

    // go ahead and make the vector long enough
    compNames.resize(MAX_COMPS, "");

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

    assetControl = Uncontrolled;

    send_FIMS_buf = fmt::memory_buffer();
}

fimsCtl::fimsCtl() {
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

fimsCtl::~fimsCtl() {
    if (unit != NULL)
        free(unit);
    if (varName != NULL)
        free(varName);
    if (reg_name != NULL)
        free(reg_name);
    if (obj_name != NULL)
        free(obj_name);
}

/**
 * @brief Replace the first # character in a string with a number
 *
 * @param templated_string: The string to replace the wildcard character in
 * @param replacement_number: The number to replace the wildcard character with
 * @param type: The type of entry being processed. Either an asset type specifier or component id specifier
 * @return Pair containing the template replaced id and Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
std::pair<std::string, Config_Validation_Result> replace_wildcard_with_number(std::string templated_string, int replacement_number, std::string type) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    const char* wildcard_character("#");
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << replacement_number;
    std::string new_number = ss.str();

    auto wildcard_location = templated_string.find(wildcard_character);
    std::string copy = templated_string;
    if (wildcard_location == std::string::npos) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{} wildcard character {} not found in string {}.", type, wildcard_character, templated_string.c_str())));
        return std::make_pair("", validation_result);
    }
    copy.replace(wildcard_location, strlen(wildcard_character), new_number);
    return std::make_pair(copy, validation_result);
}

/**
 * @brief Replace the first # character in a string with a number found in a asset range map.
 * Will replace with the first asset that is yet to be used.
 *
 * @param templated_string: The string to replace the wildcard character in
 * @param asset_range_map: The map of asset numbers and bools indicating whether they have been used yet
 * @param type: The type of entry being processed. Either an asset type specifier or component id specifier
 * @return Pair containing the template replaced id and Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
std::pair<std::string, Config_Validation_Result> find_next_asset_to_replace_wildcard(std::string templated_string, std::map<int, bool>& asset_range_map, std::string type) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    if (asset_range_map.empty()) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: internal configuration map is empty.", type)));
        return std::make_pair("", validation_result);
    }

    // Iterate through map until you find a false (aka not used yet)
    int replacement_number = -1;
    for (auto& it : asset_range_map) {
        if (it.second == false) {
            replacement_number = it.first;
            it.second = true;
            break;
        }
    }

    if (replacement_number == -1) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure using a templated range. All entries in internal configuration map already configured.", type)));
        if (asset_range_map.size() == 0)
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: internal configuration map is empty.", type)));
        else
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: internal configuration map size: {}.", type, asset_range_map.size())));
        std::string map_string;
        for (auto& it : asset_range_map) {
            map_string += std::to_string(it.first) + ":" + std::to_string(it.second) + ", ";
        }
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: internal configuration map: {}.", type, map_string.c_str())));
        return std::make_pair("", validation_result);
    }

    std::pair<std::string, Config_Validation_Result> replaced_result = replace_wildcard_with_number(templated_string, replacement_number, type);
    validation_result.absorb(replaced_result.second);
    return std::make_pair(replaced_result.first, validation_result);
}

/**
 * @brief Call find_next_asset_to_replace_wildcard() for each templated asset_id in the cJSON array.
 * Make sure that the provided replacement is yet to be used.
 *
 * @param map: <asset_id, <asset_number, used>>
 * @param validation: std::set of asset_ids that have been used
 * @param id: templated asset_id
 * @param type: the type of entry being processed. Either an asset type specifier or component id specifier
 * @return Pair containing the template replaced id and Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
std::pair<std::string, Config_Validation_Result> handle_templated_replace_check_for_duplicate(std::map<std::string, std::map<int, bool>>& map, std::set<std::string>& validation, std::string id, std::string type) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);
    if (id.empty()) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: templated string is not defined.", type)));
        return std::make_pair("", validation_result);
    }

    std::pair<std::string, Config_Validation_Result> replaced_result = find_next_asset_to_replace_wildcard(id, map[id], type);
    if (replaced_result.first == "") {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to expand templated string {}.", type, id)));
        validation_result.absorb(replaced_result.second);
        return std::make_pair("duplicate", validation_result);
    } else {
        auto check = validation.insert(replaced_result.first);
        if (check.second == false) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to insert string {}: already exists in map", type, replaced_result.first)));
        }
    }
    validation_result.absorb(replaced_result.second);
    return std::make_pair(replaced_result.first, validation_result);
}

/**
 * @brief Call find_next_asset_to_replace_wildcard() for each templated asset_id in the cJSON array.
 * Make sure that the provided replacement is yet to be used.
 *
 * @param map: <asset_id, <asset_number, used>>
 * @param validation: std::set of asset_ids that have been used
 * @param id: templated asset_id
 * @param type: the type of entry being processed. Either an asset type specifier or component id specifier
 * @return Pair containing the template replaced id and Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
std::pair<std::string, Config_Validation_Result> handle_templated_replace_check_for_duplicate(std::map<std::string, std::map<int, bool>>& map, std::set<std::string>& validation, cJSON* id, std::string type) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);
    if (id == NULL) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: templated string is not defined.", type)));
        return std::make_pair("", validation_result);
    }
    if (id->type != cJSON_String) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: templated string is not a string.", type)));
        return std::make_pair("", validation_result);
    }

    std::pair<std::string, Config_Validation_Result> replaced_result = find_next_asset_to_replace_wildcard(std::string(id->valuestring), map[id->valuestring], type);
    if (replaced_result.first == "") {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to expand templated string {}.", type, id->valuestring)));
        validation_result.absorb(replaced_result.second);
        replaced_result.first = fmt::format("duplicate", id->valuestring);
    } else {
        auto check = validation.insert(replaced_result.first);
        if (check.second == false) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to insert string {}: already exists in map", type, replaced_result.first)));
            replaced_result.first = fmt::format("duplicate", id->valuestring);
        }
    }
    validation_result.absorb(replaced_result.second);
    return std::make_pair(replaced_result.first, validation_result);
}

/**
 * @brief Make sure this non templated asset is yet to be used.
 *
 * @param validation: std::set of asset_ids that have been used
 * @param id: non templated asset_id
 * @return Pair containing the template replaced id if it has yet to be used
 * and Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
std::pair<std::string, Config_Validation_Result> no_replace_check_for_duplicate(std::set<std::string>& validation, std::string id, std::string type) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    if (id.empty()) {
        validation_result.is_valid_config = true;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: id is not defined", type)));
        return std::make_pair("", validation_result);
    }
    auto check = validation.insert(std::string(id));
    if (check.second == false) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to insert entry {}: already exists in map.", type, id)));
        return std::make_pair("duplicate", validation_result);
    }
    return std::make_pair(id, validation_result);
}

/**
 * @brief Make sure this non templated asset is yet to be used.
 *
 * @param validation: std::set of asset_ids that have been used
 * @param id: non templated asset_id
 * @return Pair containing the template replaced id if it has yet to be used
 * and Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
std::pair<std::string, Config_Validation_Result> no_replace_check_for_duplicate(std::set<std::string>& validation, cJSON* id, std::string type) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    if (id == NULL) {
        validation_result.is_valid_config = true;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: id is not defined", type)));
        return std::make_pair("", validation_result);
    }
    if (id->type != cJSON_String) {
        validation_result.is_valid_config = true;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: id is not a string.", type)));
        return std::make_pair("", validation_result);
    }
    auto check = validation.insert(std::string(id->valuestring));
    if (check.second == false) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to insert entry {}: already exists in map.", type, id->valuestring)));
        return std::make_pair("duplicate", validation_result);
    }
    return std::make_pair(id->valuestring, validation_result);
}

/**
 * @brief Configure an asset.
 *
 * @param configurator: The configurator object
 * @return Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
Config_Validation_Result Asset::configure(Type_Configurator* configurator) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    is_primary = configurator->p_is_primary_controller;  // This is a pointer to a broader controller status flag - neither an asset instance var nor a component var

    Config_Validation_Result variables_config_result = configure_common_asset_instance_vars(configurator);
    if (!variables_config_result.is_valid_config) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure common asset instance vars.", name)));
    }
    validation_result.absorb(variables_config_result);

    variables_config_result = configure_component_vars(configurator);
    if (!variables_config_result.is_valid_config) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure component vars", name)));
    }
    validation_result.absorb(variables_config_result);

    variables_config_result = configure_common_asset_fims_vars(configurator);
    if (!variables_config_result.is_valid_config) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure common FIMS setpoints.", name)));
    }
    validation_result.absorb(variables_config_result);

    variables_config_result = configure_typed_asset_fims_vars(configurator);
    if (!variables_config_result.is_valid_config) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure typed asset FIMS setpoints.", name)));
    }
    validation_result.absorb(variables_config_result);

    variables_config_result = replace_typed_raw_fims_vars();
    if (!variables_config_result.is_valid_config) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to replace typed asset FIMS setpoints.", name)));
    }
    validation_result.absorb(variables_config_result);

    add_dynamic_variables_to_maps(configurator);

    variables_config_result = configure_typed_asset_instance_vars(configurator);
    if (!variables_config_result.is_valid_config) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure typed asset instance vars.", name)));
    }
    validation_result.absorb(variables_config_result);

    variables_config_result = configure_ui_controls(configurator);
    if (!variables_config_result.is_valid_config) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure UI controls.", name)));
    }
    validation_result.absorb(variables_config_result);

    // Check configured component variables against list of required component variables to ensure all were found and configured
    // non-POI feeders don't have any required variables. POI feeder does but it will be checked separately
    // Duplicate assets do not need any additional config validation as the configuration will have been checked in the first entry
    if (strcmp(get_asset_type(), "feeders") != 0 && configurator->config_validation && !(name == "duplicate" || asset_id == "duplicate")) {
        validation_result.absorb(validate_config());
    }

    return validation_result;
}

/**
 * @brief Configure common asset instance variables.
 *
 * @param configurator: The configurator object
 * @return Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
Config_Validation_Result Asset::configure_common_asset_instance_vars(Type_Configurator* configurator) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    // If this asset instance was configured from a template, replace wildcard character `#` with template_index
    // else, just copy the id/name string
    // make sure the asset id/name is unique
    std::pair<std::string, Config_Validation_Result> id_result;
    std::pair<std::string, Config_Validation_Result> name_result;
    if (configurator->asset_config.is_template_flag) {
        id_result = handle_templated_replace_check_for_duplicate(configurator->asset_config.asset_id_to_asset_number_map, configurator->asset_config.asset_id_collision_set, configurator->current_asset_id, asset_type_id);
        name_result = handle_templated_replace_check_for_duplicate(configurator->asset_config.asset_name_to_asset_number_map, configurator->asset_config.asset_name_collision_set, configurator->current_asset_name, asset_type_id);
    } else {
        id_result = no_replace_check_for_duplicate(configurator->asset_config.asset_id_collision_set, configurator->current_asset_id, asset_type_id);
        name_result = no_replace_check_for_duplicate(configurator->asset_config.asset_name_collision_set, configurator->current_asset_name, asset_type_id);
    }
    asset_id = id_result.first;
    name = name_result.first;

    if (asset_id.empty()) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure asset id.", asset_type_id)));
        // Give placeholder names so configuration can continue and report further errors
        asset_id = asset_type_id;
    }
    if (name.empty()) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure asset name.", asset_type_id)));
        // Give placeholder names so configuration can continue and report further errors
        name = asset_type_id;
    }
    validation_result.absorb(id_result.second);
    validation_result.absorb(name_result.second);

    // Don't try to continue configuration if the asset has already been configured
    if (asset_id == "duplicate" || name == "duplicate") {
        validation_result.is_valid_config = false;
        return validation_result;
    }

    cJSON* item = cJSON_GetObjectItem(configurator->asset_config.asset_instance_root, "status_type");
    status_type = (item != NULL && strcmp(item->valuestring, "random_enum") == 0) ? random_enum : bit_field;

    // Set local mode status type to use existing status type, unless it has been manually configured
    cJSON* parsed_local_status_type = cJSON_GetObjectItem(configurator->asset_config.asset_instance_root, "local_mode_status_type");
    if (parsed_local_status_type && parsed_local_status_type->valuestring)
        local_mode_status_type = strcmp(parsed_local_status_type->valuestring, "random_enum") == 0 ? random_enum : bit_field;
    else {
        local_mode_status_type = status_type;
        if (asset_type_value != FEEDERS)
            validation_result.INFO_details.push_back(Result_Details(fmt::format("Asset {}: reusing status_type {} for local_mode_status_type", name, local_mode_status_type ? "random_enum" : "bit_field")));
    }

    cJSON* runMask = cJSON_GetObjectItem(configurator->asset_config.asset_instance_root, "running_status_mask");
    // If the mask is NULL and is not a feeder, throw an error
    if (runMask == NULL && strcmp(get_asset_type(), "feeders") != 0 && configurator->config_validation)  // Not required for feeders
    {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("Asset {}: failed to find running_status_mask in config.", name)));
    }
    // If the mask isn't NULL and is a hex string, try to parse it
    else if (runMask != NULL && runMask->valuestring != NULL) {
        try {
            // Casts the provided hex string as an unsigned int64
            running_status_mask = (uint64_t)std::stoul(runMask->valuestring, NULL, 16);
        } catch (...) {
            // Could not parse the provided mask
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("Asset {}: received an invalid running_status_mask.", name)));
        }
    }
    // If the mask isn't null and isn't a hex string, throw an error
    else if (runMask != NULL) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("Asset {}: received an invalid input type instead of a hexadecimal string for the running_status_mask.", name)));
    }

    cJSON* parsed_local_mask = cJSON_GetObjectItem(configurator->asset_config.asset_instance_root, "local_mode_status_mask");
    // if the mask isn't NULL and is a hex string, try to parse it
    if (parsed_local_mask && parsed_local_mask->valuestring) {
        try {
            // Casts the provided hex string as an unsigned int64
            local_mode_status_mask = (uint64_t)std::stoul(parsed_local_mask->valuestring, NULL, 16);
        } catch (...) {
            // Could not parse the provided mask
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("Asset {}: received an invalid local_mode_status_mask.", name)));
        }
        local_mode_configured = true;
    }
    // If the mask isn't null and isn't a hex string, throw an error
    else if (parsed_local_mask != NULL) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("Asset {}: received an invalid input type instead of a hexadecimal string for the local_mode_status_mask.", name)));
    } else if (asset_type_value != FEEDERS)
        validation_result.INFO_details.push_back(Result_Details(fmt::format("Asset {}: optional local_mode_status_mask was not provided in configuration. Will not be able to read component level local/remote control.", name)));

    cJSON* rated_active_pwr_kw = cJSON_GetObjectItem(configurator->asset_config.asset_instance_root, "rated_active_power_kw");
    if (rated_active_pwr_kw == NULL || rated_active_pwr_kw->valuedouble == 0.0) {
        // rated_active_pwr_kw not required if asset is a non-POI feeder. POI feeder will be checked for rated_active_power_kw later
        if ((strcmp(get_asset_type(), "feeders") != 0) && configurator->config_validation) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("Asset {}: must provide a nonzero rated_active_power_kw in config.", name)));
        }
    } else
        rated_active_power_kw = rated_active_pwr_kw->valuedouble;

    cJSON* rated_reactive_pwr_kvar = cJSON_GetObjectItem(configurator->asset_config.asset_instance_root, "rated_reactive_power_kvar");
    if (rated_reactive_pwr_kvar == NULL || rated_reactive_pwr_kvar->valuedouble == 0.0) {
        // rated_reactive_pwr_kvar not required if asset is a feeder
        if ((strcmp(get_asset_type(), "feeders") != 0) && configurator->config_validation) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("Asset {}: must provide a nonzero rated_reactive_power_kvar in config.", name)));
        }
    } else
        rated_reactive_power_kvar = rated_reactive_pwr_kvar->valuedouble;

    cJSON* rated_apparent_pwr_kva = cJSON_GetObjectItem(configurator->asset_config.asset_instance_root, "rated_apparent_power_kva");
    if (rated_apparent_pwr_kva == NULL || rated_apparent_pwr_kva->valuedouble == 0.0) {
        // rated_apparent_pwr_kva not required if asset is a feeder
        if ((strcmp(get_asset_type(), "feeders") != 0) && configurator->config_validation) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("Asset {}: must provide a nonzero rated_apparent_power_kva in config.", name)));
        }
    } else
        rated_apparent_power_kva = rated_apparent_pwr_kva->valuedouble;

    // Validate apparent power is at least as large as active and reactive so each can reach their rated value
    // The power factor relationship apparent^2 = active^2 + reactive^2 will be enforced by reducing one of active or reactive as need based
    // on the power_priority flag configured in variables.json. This will happen later on when processing active power
    if ((strcmp(get_asset_type(), "feeders") != 0) && configurator->config_validation) {
        if (rated_apparent_power_kva < rated_active_power_kw || rated_apparent_power_kva < rated_reactive_power_kvar) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("Asset {}: invalid power factor triangle. rated_apparent_power_kva must be at least as large as rated_active_power_kw and rated_reactive_power_kvar.", name)));
        }
    }

    cJSON* slew_rate = cJSON_GetObjectItem(configurator->asset_config.asset_instance_root, "slew_rate");
    if (slew_rate == NULL) {
        // slew_rate not required if asset is a non-POI feeder. POI feeder will be checked for slew_rate later
        if ((strcmp(get_asset_type(), "feeders") != 0) && configurator->config_validation) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("Asset {}: failed to find slew_rate in config.", name)));
        }
    } else
        active_power_slew.set_slew_rate(slew_rate->valueint);

    cJSON* throttle_timeout_fast = cJSON_GetObjectItem(configurator->asset_config.asset_instance_root, "throttle_timeout_fast_ms");
    if (throttle_timeout_fast != NULL)
        throttle_timeout_fast_ms = throttle_timeout_fast->valuedouble;

    cJSON* throttle_timeout_slow = cJSON_GetObjectItem(configurator->asset_config.asset_instance_root, "throttle_timeout_slow_ms");
    if (throttle_timeout_slow != NULL)
        throttle_timeout_slow_ms = throttle_timeout_slow->valuedouble;

    cJSON* deadband_percent = cJSON_GetObjectItem(configurator->asset_config.asset_instance_root, "deadband_percent");
    if (deadband_percent != NULL)
        throttle_deadband_percentage = deadband_percent->valuedouble;

    active_power_setpoint_throttle.configure(throttle_timeout_fast_ms, rated_active_power_kw, throttle_deadband_percentage);
    reactive_power_setpoint_throttle.configure(throttle_timeout_fast_ms, rated_reactive_power_kvar, throttle_deadband_percentage);
    start_command_throttle.configure(throttle_timeout_slow_ms);
    stop_command_throttle.configure(throttle_timeout_slow_ms);

    cJSON* wd_enable = cJSON_GetObjectItem(configurator->asset_config.asset_instance_root, "watchdog_enable");
    watchdog_enable = (wd_enable == NULL) ? false : (wd_enable->type == cJSON_True);

    if (watchdog_enable) {  // Only look for watchdog timeout if watchdog feature is enabled
        cJSON* wd_timeout_ms = cJSON_GetObjectItem(configurator->asset_config.asset_instance_root, "watchdog_timeout_ms");
        if (wd_timeout_ms == NULL) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("Asset {}: watchdog feature is enabled but failed to find watchdog timeout ms in config.", name)));
        }
        watchdog_timeout_ms = wd_timeout_ms->valueint;
    }

    cJSON* d_ctrl = cJSON_GetObjectItem(configurator->asset_config.asset_instance_root, "demand_control");
    if (d_ctrl != NULL) {
        if (strcmp("Uncontrolled", d_ctrl->valuestring) == 0) {
            assetControl = Uncontrolled;
        } else if (strcmp("Indirect", d_ctrl->valuestring) == 0) {
            assetControl = Indirect;
        } else if (strcmp("Direct", d_ctrl->valuestring) == 0) {
            assetControl = Direct;
        } else if (strcmp("Manual", d_ctrl->valuestring) == 0) {
            assetControl = Manual;
        } else {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("Asset {}: unrecognized demand control type {}.", name, d_ctrl->valuestring)));
        }
    } else {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("Asset {}: contains no demand_control variable.", name)));
    }
    return validation_result;
}

/**
 * @brief Configures the component variables for each component in the asset
 *
 * @param configurator: The configurator object that contains the asset configuration
 * @return Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
Config_Validation_Result Asset::configure_component_vars(Type_Configurator* configurator) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    // Asset instances are data aggregators for one or many components, described in the "components" array. This array is required for any asset instance
    cJSON* comp_array = cJSON_GetObjectItem(configurator->asset_config.asset_instance_root, "components");
    if (comp_array == NULL && configurator->config_validation) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: missing a components array.", name)));
        return validation_result;
    }

    // There can be one or many components
    numAssetComponents = cJSON_GetArraySize(comp_array);
    if (numAssetComponents > MAX_COMPS) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: exceeded maximum number of component entries, limit is {}, config file has {}.", name, MAX_COMPS, numAssetComponents)));
        return validation_result;
    }

    // For each component, configure its component variables
    for (size_t i = 0; i < numAssetComponents; i++) {
        cJSON* comp = cJSON_GetArrayItem(comp_array, i);
        if (comp == NULL) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: component array entry {} is invalid or undefined.", name, i + 1)));
            continue;
        }
        cJSON* comp_id = cJSON_GetObjectItem(comp, "component_id");
        cJSON* variables = cJSON_GetObjectItem(comp, "variables");
        if (variables == NULL || comp_id == NULL || comp_id->valuestring == NULL) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: component array entry {}: variables or component_id are invalid or undefined", name, i + 1)));
            continue;
        }
        if (!configurator->asset_config.asset_component_to_asset_number_map.count(comp_id->valuestring)) {
            // New component insert
            configurator->asset_config.asset_component_to_asset_number_map.insert(std::pair<std::string, std::map<int, bool>>(comp_id->valuestring, std::map<int, bool>()));
        }
        auto check = configurator->asset_config.asset_component_to_asset_number_map[comp_id->valuestring].insert(std::pair<int, bool>(configurator->asset_config.asset_num, false));
        if (!check.second) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: component {} with id {} was already configured for another asset.", name, configurator->asset_config.asset_num, comp_id->valuestring)));
        }

        std::pair<std::string, Config_Validation_Result> component_id_result;
        if (configurator->asset_config.is_template_flag) {
            component_id_result = handle_templated_replace_check_for_duplicate(configurator->asset_config.asset_component_to_asset_number_map, configurator->asset_config.asset_component_collision_set, comp_id, asset_type_id);
        } else {
            component_id_result = no_replace_check_for_duplicate(configurator->asset_config.asset_component_collision_set, comp_id, asset_type_id);
        }
        compNames[i] = component_id_result.first;
        validation_result.absorb(component_id_result.second);

        // verify that the component name is not empty
        if (compNames[i] == "" || compNames[i] == "duplicate") {
            validation_result.is_valid_config = false;
            // Give a placeholder template name so we can continue checking for config errors
            compNames[i] = fmt::format("{}_component_{}", asset_type_id, i);
        }

        // Iterate through each variable in this component and insert its information into a FIMS_Object
        // that will be in the component variables map and asset variables map owned by Asset Manager
        cJSON* component_variable = variables->child;  // Grab the first variable object within the component object
        while (component_variable)                     // Iterate until there is not a new variable object to deal with
        {
            // Insert the variable information into the FIMS_Object. Add any hard-coded variables to the asset and component maps as well
            Config_Validation_Result component_config_result = parse_variable(component_variable, compNames[i].c_str());
            if (!component_config_result.is_valid_config) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: insertion into component variable map failed.", name)));
            }
            validation_result.absorb(component_config_result);
            // Get the next variable object in this component
            component_variable = component_variable->next;
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
Config_Validation_Result Asset::configure_common_asset_fims_vars(Type_Configurator* configurator) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    validation_result.absorb(configure_single_fims_var(&active_power_setpoint, "active_power_setpoint", configurator));
    validation_result.absorb(configure_single_fims_var(&reactive_power_setpoint, "reactive_power_setpoint", configurator));
    validation_result.absorb(configure_single_fims_var(&active_power, "active_power", configurator));
    validation_result.absorb(configure_single_fims_var(&reactive_power, "reactive_power", configurator));
    validation_result.absorb(configure_single_fims_var(&apparent_power, "apparent_power", configurator));
    validation_result.absorb(configure_single_fims_var(&voltage_l1_n, "voltage_l1_n", configurator));
    validation_result.absorb(configure_single_fims_var(&voltage_l2_n, "voltage_l2_n", configurator));
    validation_result.absorb(configure_single_fims_var(&voltage_l3_n, "voltage_l3_n", configurator));
    validation_result.absorb(configure_single_fims_var(&voltage_l1_l2, "voltage_l1_l2", configurator));
    validation_result.absorb(configure_single_fims_var(&voltage_l2_l3, "voltage_l2_l3", configurator));
    validation_result.absorb(configure_single_fims_var(&voltage_l3_l1, "voltage_l3_l1", configurator));
    validation_result.absorb(configure_single_fims_var(&current_l1, "current_l1", configurator));
    validation_result.absorb(configure_single_fims_var(&current_l2, "current_l2", configurator));
    validation_result.absorb(configure_single_fims_var(&current_l3, "current_l3", configurator));
    validation_result.absorb(configure_single_fims_var(&power_factor, "power_factor", configurator));
    validation_result.absorb(configure_single_fims_var(&watchdog_heartbeat, "watchdog_heartbeat", configurator, Int));
    validation_result.absorb(configure_single_fims_var(&component_connected, "component_connected", configurator, Bool));
    validation_result.absorb(configure_single_fims_var(&is_faulted, "is_faulted", configurator, Bool, 0, 0, false, false, "Is Faulted"));
    validation_result.absorb(configure_single_fims_var(&is_alarmed, "is_alarmed", configurator, Bool, 0, 0, false, false, "Is Alarmed"));
    // Configure watchdog variables if the feature is enabled
    validation_result.absorb(configure_watchdog_vars());
    // Configure local/remote status of the component
    validation_result.absorb(configure_component_local_mode_vars(configurator));

    return validation_result;
}

// Helper function used by configure_single_fims_var. Could maybe be moved into the Fims_Object class
void set_default_value(Fims_Object** fims_var, valueType type, float default_float, int default_int, bool default_bool) {
    switch (type) {
        case Int:
            (*fims_var)->value.set((int)default_int);
            (*fims_var)->component_control_value.set((int)default_int);
            break;
        case Float:
            (*fims_var)->value.set((float)default_float);
            (*fims_var)->component_control_value.set((float)default_float);
            break;
        case Bool:
            (*fims_var)->value.set((bool)default_bool);
            (*fims_var)->component_control_value.set((bool)default_bool);
            break;
        // Other types not currently needed, add if necessary
        default:
            break;
    }
}

// Update a Fims_Object with the new fields provided
void Asset::update_fims_var(Fims_Object* fims_var, valueType type, float default_float, int default_int, bool default_bool, const char* var_id, const char* var_name, const char* var_units, int var_scaler) {
    fims_var->set_variable_id(var_id);
    fims_var->set_name(var_name);
    fims_var->set_ui_type("status");
    fims_var->set_unit(var_units);
    fims_var->scaler = var_scaler;
    fims_var->is_primary = is_primary;

    switch (type) {
        case Int:
            fims_var->value.set((int)default_int);
            fims_var->component_control_value.set((int)default_int);
            break;
        case Float:
            fims_var->value.set((float)default_float);
            fims_var->component_control_value.set((float)default_float);
            break;
        case Bool:
            fims_var->value.set((bool)default_bool);
            fims_var->component_control_value.set((bool)default_bool);
            break;
        // Other types not currently needed, add if necessary
        default:
            break;
    }
}

/**
 *  For variables that are members of the asset instance (all variables that use this function),
 *  they are either inputs from components or outputs to the /assets/ publish.
 *
 *      1. Inputs: Replace the existing dynamic variable with this hard-coded variable, copying over any existing configuration
 *                                  in the process.
 *      2. Outputs: These will not have already been configured in the dynamic variables map, so they must be configured here.
 *      3. Special case variables:  These variables will have a "raw" Fims_Object coming from the component, and a calculated
 *                                  Fims_Object that gets published on /assets/. At this stage of configuration, the raw object
 *                                  will have already been configured in both the asset_var_map and the component_var_map.
 *                                  We want the raw object to stay in the component_var_map as an input and the calculated object
 *                                  to replace the raw object in asset_var_map as an output.
 *                                  Ex: state of charge. "soc_raw" will have been configured into both maps at this point, but there
 *                                  is a calculation that uses soc_raw as an input and outputs a calculated "soc" object. So we leave
 *                                  soc_raw in component_var_map to access the input coming from the component. We take soc_raw out of
 *                                  asset_var_map and replace it with soc, using the same URI.
 */
Config_Validation_Result Asset::configure_single_fims_var(Fims_Object* fims_var, const char* var_id, Type_Configurator* configurator, valueType type, float default_float, int default_int, bool default_bool, bool comes_from_component,
                                                          const char* var_name, const char* var_units, int var_scaler) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    if (fims_var == NULL) {
        FPS_ERROR_LOG("%s: there is something wrong with this build. Single fims variable passed is NULL.", name);
        exit(1);
    }

    // Set primary flag for all variables regardless of type
    fims_var->is_primary = is_primary;

    if (comes_from_component) {
        // Check if the variable has been provided in configuration. If not, it will not be used so it does not need to be configured
        if (dynamic_variables.find(var_id) == dynamic_variables.end()) {
            validation_result.INFO_details.push_back(Result_Details(fmt::format("{}: variable {} was not provided in configuration. Leaving it out of the variable map.", name, var_id)));
            return validation_result;
        }

        // The variable was found in configuration and stored in the dynamic variables map
        // move ownership of the configured variable from the map to the hard-coded reference for this member variable
        *fims_var = dynamic_variables.at(var_id);
        dynamic_variables.erase(var_id);

        if (!configurator || !configurator->pCompVarMap) {
            FPS_ERROR_LOG("%s: there is something wrong with this build. Component map passed to function is NULL. Cannot insert variable.", name);
            exit(1);
        }
        (*configurator->pCompVarMap)[fims_var->get_component_uri()].push_back(fims_var);
    } else {
        // Update the variable with the provided fields
        update_fims_var(fims_var, type, default_float, default_int, default_bool, var_id, var_name, var_units, var_scaler);
    }

    // Make URI key and insert into asset var map
    std::string variable_uri = build_asset_variable_uri(var_id);
    // Add the member variable into the assets map.
    // This will replace any existing variable such as replacing _raw variables with their calculated counterparts
    asset_var_map[variable_uri] = fims_var;

    return validation_result;
}

/**
 * @brief Configure watchdog specific variables.
 *
 * @param asset_var_map: The map of asset variables
 * @return Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
Config_Validation_Result Asset::configure_watchdog_vars() {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    if (!watchdog_enable)
        return validation_result;

    // Startup connected as true, else it will cause a fault on startup before it reads the modbus value
    component_connected.value.set(true);

    // Create a watchdog status Fims_Object if the watchdog timer is enabled
    validation_result.absorb(configure_single_fims_var(&watchdog_status, "watchdog_status", NULL, Bool, 0, 0, false, false, "Watchdog Status"));
    validation_result.absorb(configure_single_fims_var(&watchdog_fault, "watchdog_fault", NULL, Int, 0, 0, false, false, "Watchdog Connection Fault"));
    if (!validation_result.is_valid_config) {
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure watchdog_status register for watchdog feature.", name)));
        return validation_result;
    }

    // Create a watchdog fault Fims_Object if the watchdog timer is enabled and watchdog_status was configured successfully
    std::string assets_uri = build_asset_variable_uri("watchdog_fault");

    // Manually configure additional watchdog fault fields
    watchdog_fault.set_ui_type("fault");
    watchdog_fault.set_value_type(Int);
    watchdog_fault.num_options = 1;
    watchdog_fault.options_name.insert(watchdog_fault.options_name.begin(), "Watchdog Component Disconnected");
    watchdog_fault.options_value.insert(watchdog_fault.options_value.begin(), 0);
    latched_faults[watchdog_fault.get_variable_id()] = 0;
    return validation_result;
}

/**
 * Configure Local/Remote mode variables, including the status of the component and the alarm notifying the
 * user of this status
 *
 * @return Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
Config_Validation_Result Asset::configure_component_local_mode_vars(Type_Configurator* configurator) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    // Configure variables for tracking local/remote mode status of the component
    validation_result.absorb(configure_single_fims_var(&local_mode_signal, "local_mode_signal", configurator, Int));
    validation_result.absorb(configure_single_fims_var(&local_mode_status, "local_mode_status", NULL, Bool, 0, 0, false, false, "Local Mode Status"));
    validation_result.absorb(configure_single_fims_var(&local_mode_alarm, "local_mode_alarm", NULL, Int, 0, 0, false, false, "Component Local Mode Alarm"));
    if (!validation_result.is_valid_config) {
        local_mode_configured = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure registers for component local/remote status tracking.", name)));
        return validation_result;
    }
    // Warn the user if no valid source of component local/remote signal has been provided
    if (!local_mode_signal.get_component_uri()) {
        local_mode_configured = false;
        if (asset_type_value != FEEDERS)
            validation_result.INFO_details.push_back(Result_Details(fmt::format("{} optional local_mode_signal was not provided in configuration. Will not be able to read component level local/remote control.", name)));
    }

    // Create a local mode alarm Fims_Object if the local mode variabels were configured successfully
    std::string assets_uri = build_asset_variable_uri("local_mode_alarm");

    // Manually configure additional local mode alarm fields
    local_mode_alarm.set_ui_type("alarm");
    local_mode_alarm.set_value_type(Int);
    local_mode_alarm.num_options = 1;
    local_mode_alarm.options_name.insert(local_mode_alarm.options_name.begin(), "Component is in local mode");
    local_mode_alarm.options_value.insert(local_mode_alarm.options_value.begin(), 0);
    saved_alarms[local_mode_alarm.get_variable_id()] = 0;
    return validation_result;
}

/**
 * Once the dynamic variables map has been finalized, pointers to its underlying Fims_Objects can be added to the appropriate maps.
 * @param configurator Configurator conataining the component variable map to which variables should be added
 */
void Asset::add_dynamic_variables_to_maps(Type_Configurator* configurator) {
    for (auto& variable : dynamic_variables)
        var_maps_insert(&variable.second, configurator->pCompVarMap);
}

/**
 * @brief Asset::validate_config checks the asset_var_map created during configuration to make sure that all required component
 * variables for an asset instance are successfully configured. The required variables are held in a list that lives in
 * each of the derived classes Asset_ESS, Asset_Feeder, Asset_Generator, and Asset_Solar.
 *
 * @param asset_var_map: The map of asset variables
 * @return Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
Config_Validation_Result Asset::validate_config() {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    std::vector<const char*> missing_variables;

    // Iterate through the list of required variables
    for (auto it = required_variables.begin(); it != required_variables.end(); ++it) {
        // Construct the expected key of the form "/assets/<asset_type>/<asset_id>/<variable_name>"
        std::string variable_uri = build_asset_variable_uri(*it);
        // Check if the variable is in the asset variables map
        if (asset_var_map.find(variable_uri) == asset_var_map.end()) {
            // If the variable is not in the asset variables map, add it to the list of missing variables
            missing_variables.push_back(*it);
        }
    }

    // Were any required variables not found and added to the missing variables list?
    if (!missing_variables.empty()) {
        // If there are missing variables, print out which ones are missing and return false to kill HybridOS
        for (auto it = missing_variables.begin(); it != missing_variables.end(); ++it) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: variable {} is missing or misconfigured.", name, *it)));
        }
    }
    return validation_result;
}

/**
 * @brief Tells the component to clear its faults and starts timer to clear alarm/fault registers
 * Timer gives component time to clear faults and stop reporting them so registers do not get reset
 * Internal alerts will then be cleared when the timer expires in the process_asset() step
 */
void Asset::clear_alerts(void) {
    send_to_comp_uri(true, uri_clear_faults);
    clear_fault_registers_flag = true;
    clock_gettime(CLOCK_MONOTONIC, &clear_faults_time);
    increment_timespec_ms(clear_faults_time, 1000);
}

/**
 * @brief Clears all latched alert values and clears all unlatched component-connected
 * alert registers. Component registers should be cleared, even if they are
 * unlatched, because some components may only report an alert value when it is
 * non-zero.
 */
void Asset::lower_alert_bits(void) {
    // Clear the component-connected unlatched fault registers
    for (auto component_fault_register : component_fault_registers) {
        component_fault_register->clear_fims_bit_field();
    }

    // Clear the asset level unlatched fault registers - only modbus connection fault for now
    watchdog_fault.value.value_bit_field = 0;

    // Clear the latched fault values
    for (auto&& latched_fault : latched_faults) {
        latched_fault.second = 0;
    }

    // Clear the component-connected volatile alarm registers
    for (auto component_alarm_register : component_alarm_registers) {
        component_alarm_register->clear_fims_bit_field();
    }

    // Clear the saved alarm values
    for (auto&& saved_alarm : saved_alarms) {
        saved_alarm.second = 0;
    }
}

/**
 * @brief Single endpoint for all asset types to pass along the set for the given uri and valueObject to DBI.
 * More receiving modules can be added by simply adding them to the list of URIs in data_endpoint
 *
 * @param uri: The URI of the setpoint
 * @param valueObject: The cJSON object containing the setpoint value
 */
bool Asset::send_setpoint(std::string uri, cJSON* valueObject) {
    if (valueObject == NULL || valueObject->string == NULL) {
        FPS_WARNING_LOG("Received NULL setpoint valueObject, Asset::send_setpoint().");
    }

    // Passing the specific endpoint is useful for parsing for pairs of opposite sets
    std::string endpoint = valueObject->string;
    return data_endpoint->setpoint_writeout(uri, endpoint, &valueObject);
}

/**
 * @brief Handles any GETs to URIs beginning with /assets/<asset type>/<asset ID>.
 * @param pmsg Pointer to the FIMS GET message.
 * @return True if GET handled successfully, false otherwise.
 */
bool Asset::handle_get(fims_message* pmsg) {
    // Clear buffer for use
    send_FIMS_buf.clear();

    // URI is /assets/<asset type>/<asset ID>
    if (pmsg->nfrags < 4) {
        if (!add_asset_data_to_buffer(send_FIMS_buf, strcmp(asset_type_id, FEEDERS_TYPE_ID) == 0)) {
            return false;
        }
        return send_buffer_to(pmsg->replyto, send_FIMS_buf);
    }

    // URI must be /assets/<asset type>/<asset ID>/<variable ID>
    if (!add_variable_to_buffer(pmsg->uri, pmsg->pfrags[3], send_FIMS_buf)) {
        return false;
    }
    return send_buffer_to(pmsg->replyto, send_FIMS_buf);
}

/**
 * @brief Adds all data for this asset instance to the given buffer.
 * @param clothed: True if the asset is clothed, false otherwise.
 * @param buf Buffer to which the data must be added.
 * @return True if the data was added to the buffer successfully, or false if there was an error.
 */
bool Asset::add_asset_data_to_buffer(fmt::memory_buffer& buf, bool clothed) {
    // Begin asset instance object with opening curly brace
    bufJSON_StartObject(buf);

    // Name will not be in asset var map, so add it explicitly
    bufJSON_AddString(buf, "name", name.c_str());

    // Add UI variables, which will not be in asset var map
    if (!generate_asset_ui(buf)) {
        FPS_ERROR_LOG("Error adding asset UI variables to data object for %s.", asset_id);
        return false;
    }

    // add every asset variable belonging to this asset instance
    for (auto& variable : asset_var_map) {
        // error checking
        if (variable.second == NULL) {
            FPS_ERROR_LOG("NULL Fims_Object found while adding asset variables to object for asset %s.", asset_id);
            return false;
        }
        if (variable.second->get_name() == NULL) {
            FPS_ERROR_LOG("NULL Fims_Object name found while adding asset variables to object for asset %s.", asset_id);
            return false;
        }
        // add asset variable
        variable.second->add_to_JSON_buffer(buf, NULL, clothed);
    }

    // End asset instance object with closing curly brace
    bufJSON_EndObjectNoComma(buf);
    return true;
}

/**
 * @brief Adds the identified variable's data to the given buffer.
 * @param uri URI of the desired variable.
 * @param variable_id ID of the desired variable.
 * @param buf Buffer to which the variable's data must be added.
 * @return True if the data was added to the buffer successfully, or false if there was an error.
 */
bool Asset::add_variable_to_buffer(std::string uri, const char* variable_id, fmt::memory_buffer& buf) {
    // Component controls are nested within the variable for which they are the control,
    // so the appropriate URI to search for in the map (if looking for component control)
    // is the URI without the component control suffix at the end
    trim_suffix_if_exists(uri, COMPONENT_CONTROL_SUFFIX);

    // Look for the match in the map
    auto variable = asset_var_map.find(uri);

    // target variable was found in asset var map
    if (variable != asset_var_map.end()) {
        // error checking
        if (!variable->second || !variable->second->get_variable_id()) {
            FPS_ERROR_LOG("%s add_variable_to_buffer found NULL Fims_Object or Fims_Object with NULL variable_id.", asset_type_id);
            return false;
        }

        // Start clothed objects with an opening curly brace
        bool clothed = strcmp(asset_type_id, FEEDERS_TYPE_ID) == 0;
        if (clothed)
            bufJSON_StartObject(buf);
        // Add the Fims_Object data
        variable->second->add_to_JSON_buffer(buf, variable_id, clothed);
        // End clothed objects with closing curly brace
        if (clothed)
            bufJSON_EndObjectNoComma(buf);
    }
    // Target variable was NOT found in asset var map. check if the request is for a UI control
    else {
        // UI controls are always clothed bodies so start and end with curly braces
        bufJSON_StartObject(buf);
        generate_asset_ui(buf, variable_id);
        bufJSON_EndObjectNoComma(buf);
    }

    if (to_string(buf).substr(0, 2) == "{}") {
        FPS_ERROR_LOG("Failed to add variable %s of asset %s to buffer.", variable_id, asset_type_id);
        return false;
    }
    return true;
}

/**
 * @brief Called by an asset when it cannot find a current setpoint.
 *
 * @param uri: URI of the target control.
 * @param body: Desired new value of the target control.
 * @return True on success, false on failure.
 */
bool Asset::handle_generic_asset_controls_set(std::string uri, cJSON& body) {
    char event_msg[MEDIUM_MSG_LEN];
    cJSON* maint_mode_obj = NULL;
    cJSON* lock_mode_obj = NULL;
    cJSON* lockValueObject = NULL;
    cJSON* maintValueObject = NULL;
    maint_mode_obj = grab_naked_or_clothed(body, maint_mode_obj, "maint_mode");
    lock_mode_obj = grab_naked_or_clothed(body, lock_mode_obj, "lock_mode");
    if (lock_mode_obj == NULL && maint_mode_obj == NULL)
        return false;

    // Received maintenance mode update
    if (maint_mode_obj != NULL) {
        maintValueObject = cJSON_GetObjectItem(maint_mode_obj, "value");
        bool maint_mode_set = (bool)maintValueObject->valueint;
        if (!inMaintenance && maint_mode_set) {
            FPS_INFO_LOG("Switching %s asset %s to maintenance mode.", get_asset_type(), get_name());
            char msgMess[MEDIUM_MSG_LEN];
            snprintf(msgMess, MEDIUM_MSG_LEN, "Maintenance Mode entered: %s asset: %s", get_asset_type(), get_name().c_str());
            emit_event("Assets", msgMess, INFO_ALERT);
            inMaintenance = true;
            inLockdown = false;
            lock_mode.enabled = true;
        } else if (inMaintenance && !maint_mode_set && !inLockdown) {
            FPS_INFO_LOG("Switching %s asset %s out of maintenance mode.", get_asset_type(), get_name());
            snprintf(event_msg, MEDIUM_MSG_LEN, "Maintenance Mode exited: %s asset: %s", get_asset_type(), get_name().c_str());
            emit_event("Assets", event_msg, INFO_ALERT);
            inMaintenance = false;
            inLockdown = false;
            lock_mode.enabled = false;
        } else if (inMaintenance && !maint_mode_set && inLockdown) {
            FPS_INFO_LOG("Cannot switch %s asset %s out of maintenance mode due to lockdown.", get_asset_type(), get_name());
            snprintf(event_msg, MEDIUM_MSG_LEN, "Cannot switch %s asset out of maintenance mode due to lockdown: %s", get_asset_type(), get_name().c_str());
            emit_event("Assets", event_msg, INFO_ALERT);
            return false;
        } else if (inMaintenance == maint_mode_set) {
            // ignore set to maintenance mode since asset is already in desired state
            return true;
        } else {
            // this case should be unreachable
            FPS_ERROR_LOG("Reached invalid maintenance mode set handling case for %s asset: %s with maintenance %s, lockdown %s, set %s", get_asset_type(), get_name(), inMaintenance ? "true" : "false", inLockdown ? "true" : "false",
                          maint_mode_set ? "true" : "false");
            return false;
        }
        return send_setpoint(uri, maint_mode_obj);
    }

    // Received lockdown mode update
    else {
        lockValueObject = cJSON_GetObjectItem(lock_mode_obj, "value");
        bool lockdown_set = (bool)lockValueObject->valueint;
        // Allows lockdown sets only when in maintenance mode
        if (inLockdown && !inMaintenance) {
            // this case should be unreachable since maintenance mode can't be exited if in lockdown
            FPS_ERROR_LOG("Switching asset out of lockdown mode to rectify invalid lockdown state seen in lockdown mode set handling case for %s asset: %s with maintenance %s, lockdown %s, set %s", get_asset_type(), get_name().c_str(),
                          inMaintenance ? "true" : "false", inLockdown ? "true" : "false", lockdown_set ? "true" : "false");
            inLockdown = false;
            // return true if the set wanted to exit lockdown mode, otherwise return false
            return !lockdown_set;
        } else if (inLockdown && inMaintenance && !lockdown_set) {
            FPS_INFO_LOG("Switching %s asset %s out of lockdown mode.", get_asset_type(), get_name());
            snprintf(event_msg, MEDIUM_MSG_LEN, "Lockdown Mode exited: %s asset: %s", get_asset_type(), get_name().c_str());
            emit_event("Assets", event_msg, INFO_ALERT);
            inLockdown = false;
            maint_mode.enabled = true;
        } else if (!inLockdown && inMaintenance && lockdown_set) {
            FPS_INFO_LOG("Switching %s asset %s into lockdown mode.", get_asset_type(), get_name());
            snprintf(event_msg, MEDIUM_MSG_LEN, "Lockdown Mode entered: %s asset: %s", get_asset_type(), get_name().c_str());
            emit_event("Assets", event_msg, INFO_ALERT);
            inLockdown = true;
            maint_mode.enabled = false;
        } else if (!inLockdown && !inMaintenance && lockdown_set) {
            FPS_INFO_LOG("Cannot enter lockdown mode when not in maintenance mode for %s asset %s.", get_asset_type(), get_name());
            snprintf(event_msg, MEDIUM_MSG_LEN, "Cannot enter lockdown mode for %s asset: %s not in maintenance", get_asset_type(), get_name().c_str());
            emit_event("Assets", event_msg, INFO_ALERT);
            // default lockdown to false
            inLockdown = false;
            return false;
        } else if (inLockdown == lockdown_set) {
            // ignore set to lockdown mode since asset is already in desired state
            return true;
        } else {
            // this case should be unreachable
            FPS_ERROR_LOG("Reached invalid lockdown mode set handling case for %s asset: %s with maintenance %s, lockdown %s, set %s", get_asset_type(), get_name(), inMaintenance ? "true" : "false", inLockdown ? "true" : "false",
                          lockdown_set ? "true" : "false");
            return false;
        }
        // Echo set to storage db
        return send_setpoint(uri, lock_mode_obj);
    }
}

/**
 * @brief Process watchdog feature to determine availability of the asset
 *
 * @return True on success, false on failure.
 */
bool Asset::process_watchdog_status() {
    if (!watchdog_enable)
        return true;

    if (!component_connected.value.value_bool)
        FPS_ERROR_LOG("Component not connected");

    char event_msg[MEDIUM_MSG_LEN];
    watchdog_status.value.value_bool = !check_fims_timeout() && component_connected.value.value_bool;
    // Check watchdog_fault after getting first watchdog_status value
    auto latched_fault_it = latched_faults.find("watchdog_fault");
    if (latched_fault_it == latched_faults.end()) {
        // Map initialized incorrectly when processing configuration
        FPS_ERROR_LOG("Fault value undefined for watchdog_fault in %s process_asset()", get_id());
    } else if (latched_fault_it->second == 0 && !watchdog_status.value.value_bool) {
        clearFaultsControlEnable = true;
        watchdog_fault.value.value_bit_field = 1;
        latched_fault_it->second = 1;
        snprintf(event_msg, MEDIUM_MSG_LEN, "Fault received: %s, asset: %s", watchdog_fault.options_name[0].c_str(), name.c_str());
        emit_event("Assets", event_msg, FAULT_ALERT);
    }

    if (connected_rising_edge_detect != component_connected.value.value_bool) {
        if (component_connected.value.value_bool) {
            snprintf(event_msg, MEDIUM_MSG_LEN, "The Asset %s was connected", get_name().c_str());
            emit_event("Assets", event_msg, STATUS_ALERT);
        } else {
            snprintf(event_msg, MEDIUM_MSG_LEN, "The Asset %s was disconnected", get_name().c_str());
            emit_event("Assets", event_msg, FAULT_ALERT);
        }
        connected_rising_edge_detect = component_connected.value.value_bool;
    }
    return watchdog_status.value.value_bool;
}

/**
 * process local mode status of the component, alarming if the component is in local mode and no longer controllable.
 *
 * @return True on success, false on failure.
 */
void Asset::process_local_mode_status() {
    if (!local_mode_configured)
        return;

    // Set the bool value based on the int value received over modbus
    if (local_mode_status_type == random_enum)
        local_mode_status.value.value_bool = uint64_t(local_mode_signal.value.value_int) == local_mode_status_mask;
    else if (local_mode_status_type == bit_field)
        // Check that the bit in the position given by the signal is valid
        // e.g. valid local mode states: 4, 5; mask (binary): 110000 (start counting from 0)
        // for status value 4, verify: 110000 & 010000
        local_mode_status.value.value_bool = (local_mode_status_mask & (uint64_t(1) << local_mode_signal.value.value_int));
    // Update internal alarm register to reflect the value of the component register
    local_mode_alarm.value.value_bit_field = local_mode_status.value.value_bool;

    char event_msg[MEDIUM_MSG_LEN];
    // Make sure local mode alarm is configured
    auto alarm_it = saved_alarms.find("local_mode_alarm");
    if (alarm_it == saved_alarms.end()) {
        // Map initialized incorrectly when processing configuration
        FPS_ERROR_LOG("Alarm value undefined for local_mode_alarm in %s process_asset()", get_id());
        // Only use saved alarms to track new changes in the local/remote value
    } else if (alarm_it->second == 0 && local_mode_status.value.value_bool) {
        alarm_it->second = 1;
        snprintf(event_msg, MEDIUM_MSG_LEN, "alarm received: %s, asset: %s", local_mode_alarm.options_name[0].c_str(), name.c_str());
        emit_event("Assets", event_msg, ALARM_ALERT);
    } else if (alarm_it->second == 1 && !local_mode_status.value.value_bool) {
        alarm_it->second = 0;
    }
}

/**
 * @brief Process asset and look for faults, alarms, and status updates
 *
 * @return True on success, false on failure.
 */
void Asset::process_asset(void) {
    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    if (clear_fault_registers_flag && check_expired_time(now, clear_faults_time)) {
        lower_alert_bits();
        clear_fault_registers_flag = false;
        send_to_comp_uri(false, uri_clear_faults);  // End clear faults pulse to component
    }

    // Process component level local mode status
    process_local_mode_status();

    // Process component fault registers
    bool previously_no_faults = get_num_active_faults() == 0;
    for (auto fault_register : component_fault_registers) {
        // If the fault register contains a non-zero value, the component is reporting a fault
        if (fault_register->value.value_bit_field > 0) {
            // Find the corresponding latched fault value
            auto latched_fault_it = latched_faults.find(fault_register->get_variable_id());
            if (latched_fault_it == latched_faults.end()) {
                // Map initialized incorrectly when processing configuration
                FPS_ERROR_LOG("Fault value undefined for %s in %s process_asset().", fault_register->get_variable_id(), get_id());
                continue;
            }

            // Check the latched fault value against the received fault value
            if (latched_fault_it->second != fault_register->value.value_bit_field) {
                // Check for new faults
                uint64_t new_faults = ~latched_fault_it->second & fault_register->value.value_bit_field;
                if (new_faults != 0) {
                    char faultMsg[MEDIUM_MSG_LEN];
                    clearFaultsControlEnable = true;
                    // Translate fault bit to fault message
                    for (int i = 0; new_faults != 0; i++) {
                        // The current bit to check
                        if ((new_faults & 1) == 1) {
                            snprintf(faultMsg, MEDIUM_MSG_LEN, "Asset %s received fault: %s", name.c_str(), fault_register->options_name[i].c_str());
                            emit_event("Assets", faultMsg, FAULT_ALERT);
                        }
                        new_faults >>= 1;
                    }
                }
            }
            // Update the latched fault value
            latched_fault_it->second = fault_register->value.value_bit_field;
        }
    }

    is_faulted.value.set(get_num_active_faults() != 0);
    newly_faulted = is_faulted.value.value_bool ? previously_no_faults : newly_faulted;
    is_alarmed.value.set(get_num_active_alarms() != 0);

    // Process component alarm registers
    for (auto alarm_register : component_alarm_registers) {
        // Find the corresponding saved alarm value
        auto saved_alarm = saved_alarms.find(alarm_register->get_variable_id());
        if (saved_alarm == saved_alarms.end()) {
            // Map initialized incorrectly on processing configuration
            FPS_ERROR_LOG("alarm value undefined for %s in %s process_asset().", alarm_register->get_variable_id(), get_id());
            break;
        }
        uint64_t old_alarm_value = saved_alarm->second;

        // If the alarm register contains a non-zero value, the component is reporting an alarm
        if (alarm_register->value.value_bit_field > 0) {
            // Check the saved alarm value against the received alarm value
            if (old_alarm_value != alarm_register->value.value_bit_field) {
                // Check for new alarms
                uint64_t new_alarms = ~old_alarm_value & alarm_register->value.value_bit_field;
                if (new_alarms != 0) {
                    char alarmMsg[MEDIUM_MSG_LEN];
                    clearFaultsControlEnable = true;
                    // Translate fault bit to fault message
                    for (int i = 0; new_alarms != 0; i++) {
                        if ((new_alarms & 1) == 1) {
                            snprintf(alarmMsg, MEDIUM_MSG_LEN, "Asset %s received alarm: %s", name.c_str(), alarm_register->options_name[i].c_str());
                            emit_event("Assets", alarmMsg, ALARM_ALERT);
                        }
                        new_alarms >>= 1;
                    }
                }
            }
        }

        // Update the saved alarm value
        saved_alarm->second = alarm_register->value.value_bit_field;
    }

    if (status_type != random_enum && status_type != bit_field)
        FPS_ERROR_LOG("Asset::process_asset - invalid status type received\n");
    else {
        // emit event if status has changed
        if (internal_status != raw_status) {
            char statusMsg[SHORT_MSG_LEN];
            internal_status = raw_status;
            // For ess/gen/solar use the status string received from components
            // For feeder, status is a boolean so use the hard-coded values
            snprintf(statusMsg, SHORT_MSG_LEN, "Asset %s status changed to: %s", name.c_str(), get_status_string());
            emit_event("Assets", statusMsg, STATUS_ALERT);
        }
    }

    // Availability logic, including local mode status and watchdog status if the feature is enabled
    isAvail = (get_num_active_faults() == 0) && !inMaintenance && !local_mode_status.value.value_bool && process_watchdog_status();

    // Process running status
    if (status_type == random_enum)
        isRunning = internal_status == running_status_mask;
    else if (status_type == bit_field)
        // Check that the bit in the position given by the status value is valid
        // e.g. valid running states: 4, 5; mask (binary): 110000 (start counting from 0)
        // for status value 4, verify: 110000 & 010000
        isRunning = (running_status_mask & (uint64_t(1) << internal_status));

    // Process potential power values
    process_potential_active_power();
    process_potential_reactive_power();
}

bool Asset::is_available(void) {
    return isAvail;
}

bool Asset::is_running(void) {
    return isRunning;
}

demandMode Asset::get_demand_mode(void) const {
    if (inMaintenance)
        return Uncontrolled;
    return assetControl;
}

bool Asset::is_controllable(void) {
    if (isAvail && isRunning && !inStandby && (get_demand_mode() != Uncontrolled))
        return true;
    else
        return false;
}

bool Asset::in_maint_mode(void) {
    return inMaintenance;
}

bool Asset::in_standby(void) {
    return inStandby;
}

// Whether the component is in local mode and ignoring sets from site_controller
bool Asset::is_in_local_mode(void) {
    return local_mode_status.value.value_bool;
}

std::string Asset::get_name() {
    return name;
}

std::string Asset::get_id() {
    return asset_id;
}

const char* Asset::get_asset_type(void) const {
    return asset_type_id;
}

int Asset::get_num_comps(void) const {
    return numAssetComponents;
}

const std::string Asset::get_comp_name(int i) const {
    return compNames[i];
}

uint64_t Asset::get_status(void) const {
    return internal_status;
}

/**
 * @brief Iterates across all saved alarm values and counts how many are active.
 *
 * @return Number of active alarms.
 */
int Asset::get_num_active_alarms(void) const {
    int num_alarms = 0;
    for (auto&& name_value_pair : saved_alarms) {
        if (name_value_pair.second > 0)
            ++num_alarms;
    }
    return num_alarms;
}

/**
 * @brief Iterates across all latched fault values and counts how many are active.
 *
 * @return Number of active faults.
 */
int Asset::get_num_active_faults(void) const {
    int num_faults = 0;
    for (auto&& name_value_pair : latched_faults) {
        if (name_value_pair.second > 0) {
            ++num_faults;
        }
    }
    return num_faults;
}

/**
 * @brief Is Newly Faulted
 * @return The rising edge of an asset being faulted.
 */
bool Asset::is_newly_faulted(void) const {
    return newly_faulted;
}

/**
 * @brief Checks if there is an active alarm or an active fault with the given ID.
 *
 * @param id: ID of the alert to check.
 * @param mask: 64-bit mask to apply to the alert value.
 * @return True if there is an alert with the given ID and its masked value is non-zero, indicating it is active. False otherwise.
 */
bool Asset::check_alert(std::string& id, uint64_t& mask) {
    return check_fault(id, mask) || check_alarm(id, mask);
}

/**
 * @brief Checks if there is an active fault with the given ID.
 *
 * @param id: ID of the fault to check.
 * @param mask: 64-bit mask to apply to the fault value.
 * @return True if there is a fault with the given ID and its masked value is non-zero, indicating it is active. False otherwise.
 */
bool Asset::check_fault(std::string& id, uint64_t& mask) {
    auto fault = latched_faults.find(id);
    if (fault != latched_faults.end()) {
        return fault->second & mask;
    }
    return false;  // If fault ID is not found in map, then consider it not active
}

/**
 * @brief Checks if there is an active alarm with the given ID.
 *
 * @param id: ID of the alarm to check.
 * @param mask: 64-bit mask to apply to the alarm value.
 * @return True if there is an alarm with the given ID and its masked value is non-zero, indicating it is active. False otherwise.
 */
bool Asset::check_alarm(std::string& id, uint64_t& mask) {
    auto alarm = saved_alarms.find(id);
    if (alarm != saved_alarms.end()) {
        return alarm->second & mask;
    }
    return false;  // If alarm ID is not found in map, then consider it not active
}

float Asset::get_voltage_l1_l2(void) const {
    return voltage_l1_l2.value.value_float;
}

float Asset::get_voltage_l2_l3(void) const {
    return voltage_l2_l3.value.value_float;
}

float Asset::get_voltage_l3_l1(void) const {
    return voltage_l3_l1.value.value_float;
}

float Asset::get_voltage_l1_n(void) const {
    return voltage_l1_n.value.value_float;
}

float Asset::get_voltage_l2_n(void) const {
    return voltage_l2_n.value.value_float;
}

float Asset::get_voltage_l3_n(void) const {
    return voltage_l3_n.value.value_float;
}

float Asset::get_voltage_avg_line_to_line(void) const {
    float sumVolts = voltage_l1_l2.value.value_float + voltage_l2_l3.value.value_float + voltage_l3_l1.value.value_float;
    return (numPhases != 0.0 ? sumVolts / numPhases : 0);
}

float Asset::get_voltage_avg_line_to_neutral(void) const {
    float sumVolts = voltage_l1_n.value.value_float + voltage_l2_n.value.value_float + voltage_l3_n.value.value_float;
    return (numPhases != 0.0 ? sumVolts / numPhases : 0);
}

float Asset::get_current_l1(void) const {
    return current_l1.value.value_float;
}

float Asset::get_current_l2(void) const {
    return current_l2.value.value_float;
}

float Asset::get_current_l3(void) const {
    return current_l3.value.value_float;
}

float Asset::get_current_avg(void) const {
    float sumCurrent = current_l1.value.value_float + current_l2.value.value_float + current_l3.value.value_float;
    return (numPhases != 0.0 ? sumCurrent / numPhases : 0);
}

float Asset::get_power_factor(void) const {
    return power_factor.value.value_float;
}

bool Asset::get_watchdog_fault(void) const {
    if (watchdog_enable) {
        return watchdog_fault.value.value_bit_field != 0;
    }
    return false;
}

float Asset::get_active_power(void) const {
    return active_power.value.value_float;
}

float Asset::get_reactive_power(void) const {
    return reactive_power.value.value_float;
}

float Asset::get_apparent_power(void) const {
    return apparent_power.value.value_float;
}

float Asset::get_max_potential_active_power(void) {
    return max_potential_active_power;
}

float Asset::get_min_potential_active_power(void) {
    return min_potential_active_power;
}

float Asset::get_max_limited_active_power(void) {
    return active_power_limit;
}

float Asset::get_potential_reactive_power(void) {
    return potential_reactive_power;
}

int Asset::get_active_power_slew_rate(void) {
    return active_power_slew.get_slew_rate();
}

float Asset::get_active_power_slew_max_target(void) {
    return active_power_slew.get_max_target();
}

float Asset::get_active_power_slew_min_target(void) {
    float value = active_power_slew.get_min_target();
    return (value < 0) ? 0 : value;
}

float Asset::get_active_power_slew_target(float target) {
    return active_power_slew.get_slew_target(target);
}

void Asset::set_reactive_power_priority(bool priority) {
    reactive_power_priority = priority;
}

void Asset::process_potential_active_power(void) {
    // Set the slew to the active power setpoint calculated by Site Manager or set manually
    active_power_slew.update_slew_target(active_power_setpoint.component_control_value.value_float);

    // Max value is capped based on rated power
    max_potential_active_power = active_power_slew.get_max_target();
    max_potential_active_power = max_limited_active_power = (max_potential_active_power > rated_active_power_kw) ? rated_active_power_kw : max_potential_active_power;

    // Min value is capped at 0KW
    min_potential_active_power = active_power_slew.get_min_target();
    min_potential_active_power = min_limited_active_power = zero_check(min_potential_active_power);

    // Base active power limit reference
    active_power_limit = rated_active_power_kw;
}

void Asset::process_potential_reactive_power(void) {
    potential_reactive_power = rated_reactive_power_kvar;
    if (!reactive_power_priority) {
        float reactive_power_limit = sqrtf(powf(rated_apparent_power_kva, 2) - powf(active_power_setpoint.value.value_float, 2));
        potential_reactive_power = std::min(potential_reactive_power, reactive_power_limit);
    }
}

float Asset::get_rated_active_power(void) const {
    return rated_active_power_kw;
}

float Asset::get_rated_reactive_power(void) const {
    return rated_reactive_power_kvar;
}

float Asset::get_rated_apparent_power(void) const {
    return rated_apparent_power_kva;
}

/**
 * @brief This function takes as argument a variable id and returns
 * a variable URI of the form "/assets/<asset_type>/<asset id>/<variable id>"
 *
 * @param var: The variable id
 * @return std::string built variable URI
 */
std::string Asset::build_asset_variable_uri(const char* var) {
    char uriWithAsset[SHORT_MSG_LEN];
    snprintf(uriWithAsset, SHORT_MSG_LEN, "/assets/%s/%s/%s", asset_type_id, asset_id.c_str(), var);
    return std::string(uriWithAsset);
}

/**
 * Parses the variable provided and stores it as a member of this Asset, either as a loose variable for hard-coded variables
 * or within the dynamic_variables vector for variables that are simply passed through over FIMS.
 *
 * If the variable already exists in the asset map as a hard-coded member, this function will update that variable instead
 * of creating a new one.
 * @param var_json The JSON representation of the variable received from configuration
 * @param comp_id Component id of the variable
 * @return Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
Config_Validation_Result Asset::parse_variable(cJSON* var_json, std::string comp_id) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    // The variable string is the name of the component variable object in assets.json. Also is what UI expects in pubs
    char* variable_string = var_json->string;
    // build_asset_variable_uri prepends "/assets/<asset_type>/"
    std::string variable_uri = build_asset_variable_uri(variable_string);

    // The component variable that will be added to the maps
    // emplace() constructs a new object directly in the vector rather than copying a new one
    dynamic_variables.emplace(variable_string, Fims_Object());
    Fims_Object* component_variable = &dynamic_variables.at(variable_string);

    component_variable->set_variable_id(variable_string);

    // Give Fims_Object pointer to is_primary
    component_variable->is_primary = is_primary;

    // The "name" field is a very succinct description of the variable that can be multiple words with spaces, capital letters, etc->
    cJSON* variable_name = cJSON_HasObjectItem(var_json, "name") ? cJSON_GetObjectItem(var_json, "name") : NULL;
    if (variable_name == NULL || variable_name->valuestring == NULL) {  // The variable name is required
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: name is undefined for component variable {}.", comp_id, variable_string)));
    } else {
        component_variable->set_name(variable_name->valuestring);
    }

    // Asset Manager should expect to receive updates from the component as pubs
    // The register id is what the component will use in its body when it pubs
    if (!cJSON_HasObjectItem(var_json, "register_id")) {  // If a component variable object does not have a register id, then it doesn't belong in the component variables map
        return validation_result;
    }

    cJSON* register_id = cJSON_GetObjectItem(var_json, "register_id");
    if (register_id == NULL || register_id->valuestring == NULL) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: register_id is undefined for for component variable {}", comp_id, variable_string)));
    } else {
        component_variable->set_register_id(register_id->valuestring);
    }

    // The "ui_type" field tells the UI how to display the variable: status, control, fault, alarm, ...
    cJSON* ui_type = cJSON_HasObjectItem(var_json, "ui_type") ? cJSON_GetObjectItem(var_json, "ui_type") : NULL;
    if (ui_type != NULL && ui_type->valuestring) {
        // Check against list of valid ui_types
        bool valid = false;
        for (int i = 0; i < NUM_UI_TYPES; i++) {
            if (strcmp(UI_Type_Names[i], ui_type->valuestring) == 0) {
                valid = true;
                break;
            }
        }
        // Set to valid ui_type or float if invalid
        if (!valid) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: invalid ui_type {} provided for variable {}", comp_id, ui_type->valuestring, variable_string)));
        } else {
            component_variable->set_ui_type(ui_type->valuestring);
        }
    } else {  // If no ui_type given, default is "status"
        component_variable->set_ui_type("status");
    }

    // The "type" field is used to know which value to use within Fims_Object's Value_Object
    cJSON* type = cJSON_HasObjectItem(var_json, "type") ? cJSON_GetObjectItem(var_json, "type") : NULL;
    if (type != NULL && type->valuestring) {
        // Check against list of valid types
        if (strcmp(type->valuestring, "Float") == 0 || strcmp(type->valuestring, "Int") == 0 || strcmp(type->valuestring, "Bool") == 0 || strcmp(type->valuestring, "Bit Field") == 0 || strcmp(type->valuestring, "Random Enum") == 0 ||
            strcmp(type->valuestring, "Status") == 0) {
            component_variable->set_type(type->valuestring);
        } else {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: invalid type {} provided for variable {}", comp_id, type->valuestring, variable_string)));
        }
    } else {  // If no type given, default is "Float"
        component_variable->set_type("Float");
    }

    // The rest of the configuration is optional and does not report errors, so we can return early at this point
    if (!validation_result.is_valid_config)
        return validation_result;

    // The "scaler" field is used for displaying the variable correctly on the UI
    cJSON* scaler = cJSON_HasObjectItem(var_json, "scaler") ? cJSON_GetObjectItem(var_json, "scaler") : NULL;
    if (scaler != NULL) {  // The scaler is not required
        component_variable->scaler = scaler->valueint;
    } else if (strcmp(component_variable->get_type(), "Int") == 0 || strcmp(component_variable->get_type(), "Float") == 0) {  // If no scaler is given for an int or a float, default is 1
        component_variable->scaler = 1;
    }

    // The "value" field can be used to initialize the variable
    cJSON* initial_value = cJSON_HasObjectItem(var_json, "value") ? cJSON_GetObjectItem(var_json, "value") : NULL;
    // default values if value field is not provided
    float initial_double = 0;
    int initial_int = 0;
    bool initial_bool = false;
    // Overwrite default values if value field is provided
    if (initial_value != NULL) {
        initial_double = initial_value->valuedouble;
        initial_int = initial_value->valueint;
        initial_bool = initial_value->type == cJSON_True;
    }
    // Set Fims_Object value variables based on type and default or overwritten initial value
    if (strcmp(component_variable->get_type(), "Float") == 0) {
        component_variable->set_fims_float(variable_string, initial_double);
        component_variable->component_control_value.set(initial_double);
        component_variable->set_value_type(Float);
    } else if (strcmp(component_variable->get_type(), "Int") == 0) {
        component_variable->set_fims_int(variable_string, initial_int);
        component_variable->component_control_value.set(initial_int);
        component_variable->set_value_type(Int);
    } else if (strcmp(component_variable->get_type(), "Bool") == 0) {
        component_variable->set_fims_bool(variable_string, initial_bool);
        component_variable->component_control_value.set(initial_bool);
        component_variable->set_value_type(Bool);
    } else if (strcmp(component_variable->get_type(), "Status") == 0) {
        component_variable->set_fims_bit_field(variable_string, uint64_t(initial_int));
        component_variable->component_control_value.set(uint64_t(initial_int));
        component_variable->set_value_type(Status);
    } else if (strcmp(component_variable->get_type(), "Bit Field") == 0) {
        component_variable->set_value_type(Bit_Field);
    } else if (strcmp(component_variable->get_type(), "Random Enum") == 0) {
        component_variable->set_value_type(Random_Enum);
    }

    // The initial mask of the variable
    // Unused unless the variable is an alarm/fault
    cJSON* initial_mask = cJSON_HasObjectItem(var_json, "mask") ? cJSON_GetObjectItem(var_json, "mask") : NULL;
    if (initial_mask != NULL) {
        // Read in mask as string to support 64 bit width
        component_variable->value.value_mask = (uint64_t)std::stoul(initial_mask->valuestring, NULL, 16);
    } else {
        // Represents 64 digits of 1
        component_variable->value.value_mask = 0xFFFFFFFFFFFFFFFF;
    }

    // The "unit" field is useful for the UI display
    cJSON* variable_unit = cJSON_HasObjectItem(var_json, "unit") ? cJSON_GetObjectItem(var_json, "unit") : NULL;
    if (variable_unit != NULL && variable_unit->valuestring != NULL) {  // The variable unit is not required
        component_variable->set_unit(variable_unit->valuestring);
    } else {
        component_variable->set_unit("");
    }

    // Both uri build functions create a C-style string, so use the string constructor function to convert it to a string object
    // build_uri prepends "/components/"
    std::string components_uri(build_uri(comp_id, register_id->valuestring));

    // Component uri is held by Fims_Object's `component_uri` member variable
    component_variable->set_component_uri(components_uri.c_str());

    // For dynamic variables, the list must be static before references can be taken. The variables will be inserted
    // into the other maps once all of them have been parsed
    return validation_result;
}

/**
 * Asset Manager owns a map called component_var_map which holds pointers to all component variables across all assets.
 * The key is a URI with the following form: /components/<component_id>/<register_id> and the value is a pointer
 * to the Fims_Object that holds the corresponding variable.
 *
 * This Asset owns a map called asset_var_map which holds pointers to the same variable objects that component_var_map
 * points to. However, the keys are of the following form: /assets/<asset_type>/<asset_id>/<variable_name>.
 *
 * This function, var_maps_insert, is called on each component variable during configuration from Asset_<Type>::configure().
 * It executes after the variable has already been parsed in parse_variable(), and using the existing component uri or
 * variable id, it then constructs the appropriate uri for each map. Finally, it takes the pointer to the Fims_Object and
 * inserts the URI/pointer pairs into the component variables map and the asset variables map, along with other auxiliary
 * maps with faults or alarms as appropriate.
 *
 * If the variable already exists in the asset map as a hard-coded member, this function will update that variable instead
 * of creating a new one
 * @param var_json The JSON representation of the variable received from configuration
 * @param comp_id Component id of the variable
 * @param component_var_map Pointer to the component var map
 * @return true if inserted successfully
 */
void Asset::var_maps_insert(Fims_Object* variable, std::map<std::string, std::vector<Fims_Object*>>* const component_var_map) {
    // The component var map supports having multiple Fims_Objects source their values from the same component register
    // If there is already a configured variable that uses this component URI, add this variable to that list
    (*component_var_map)[variable->get_component_uri()].push_back(variable);
    // Construct the asset string for this asset
    std::string variable_uri = build_asset_variable_uri(variable->get_variable_id());
    // Insert the Fims_Object pointer into the assets variable map
    asset_var_map[variable_uri] = variable;

    // Add to the alarm/fault data objects as appropriate
    if (strcmp(variable->get_ui_type(), "alarm") == 0) {
        // insert pointer to Fims_Object into list of component alarm registers
        component_alarm_registers.push_back(variable);
        // record alarm ID in map of alarm values with initial value 0
        saved_alarms[variable->get_variable_id()] = 0;
    } else if (strcmp(variable->get_ui_type(), "fault") == 0) {
        // insert pointer to Fims_Object into list of component fault registers
        component_fault_registers.push_back(variable);
        // record fault ID in map of fault values with initial value 0
        latched_faults[variable->get_variable_id()] = 0;
    }
}

/**
 * @brief      Configures the UI control object.
 *
 * @param      cfg_json:         The configuration json
 * @param      build_option:     The build option
 * @param      display:          The display
 * @param      value_type_cfg:   The value type configuration
 * @param      display_type_cfg: The display type configuration
 * @param      enabled_cfg:      The enabled configuration
 * @param      is_bool_string:   Indicates if boolean string
 *
 * @return Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
Config_Validation_Result fimsCtl::configure(cJSON* cfg_json, jsonBuildOption build_option, void* display, valueType value_type_cfg, displayType display_type_cfg, bool enabled_cfg, bool is_bool_string) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    if (cfg_json == NULL && !configured) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details("Empty configuration object given for UI control."));
        return validation_result;
    }
    obj_name = strdup(cfg_json->string);

    if (configured == true) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: UI control object configured more than once.", obj_name)));
        return validation_result;
    }

    cJSON* control_name_json;
    if ((control_name_json = cJSON_GetObjectItem(cfg_json, "name")) == NULL || (control_name_json->valuestring == NULL)) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: no 'name' field given in UI control configuration object.", obj_name)));
    } else {
        varName = strdup(control_name_json->valuestring);
    }

    cJSON* register_id_json = cJSON_GetObjectItem(cfg_json, "register_id");
    if (register_id_json != NULL && register_id_json->valuestring != NULL)
        reg_name = strdup(register_id_json->valuestring);

    cJSON* scaler_json;
    if ((scaler_json = cJSON_GetObjectItem(cfg_json, "scaler")) == NULL)
        scaler = 1;
    else
        scaler = scaler_json->valueint;

    cJSON* unit_json;
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
    return validation_result;
}

/**
 * @brief      Sends a json object over fims. Will not send and return true if the controller
 *             is secondary or the component receiving the message is in local mode
 *
 * @param      value: The value to send
 * @param      uri: The uri to send to
 *
 * @return     True on success, False otherwise.
 */
bool Asset::json_object_send(std::string& value, const std::string& uri) {
    // Disable response if the controller is secondary or the component is unavailable (local control only)
    if (!*is_primary || is_in_local_mode())
        return true;

    if (value.empty() || (uri.empty())) {
        FPS_ERROR_LOG("Asset::json_object_send - value or uri does not exist.");
        return false;
    }

    p_fims->Send("set", uri.c_str(), NULL, value.c_str());
    return true;
}

void Write_Rate_Throttle::reset(void) {
    status_time = 0;
    control_feedback = 0;
}

void Write_Rate_Throttle::configure(long timeout, float rated, float deadband) {
    throttle_timeout = timeout;  // Throttle_timeout is in ms
    rated_power = rated;
    deadband_percentage = deadband;
}

bool Write_Rate_Throttle::command_trigger(void) {
    long control_time = current_timestamp();
    if ((control_time - status_time) > throttle_timeout) {
        status_time = control_time;  // Make status_time the time of most recent write
        return true;
    }
    return false;
}

bool Write_Rate_Throttle::setpoint_trigger(float control_power) {
    long control_time = current_timestamp();
    float delta = control_feedback - control_power;           // Difference between current active power and incoming active power
    float percent_power = rated_power * deadband_percentage;  // Amount of power that will trigger an instant write instead of waiting for timeout

    if ((fabsf(delta) > percent_power) || ((control_time - status_time) > throttle_timeout))  // Any negative deltas will be positive
    {
        control_feedback = control_power;  // Make control_feedback teh value that was "last written"
        status_time = control_time;        // Make status_time the time of most recent write
        return true;
    }

    return false;
}

long Write_Rate_Throttle::current_timestamp(void) {
    timespec now;
    int rc = clock_gettime(CLOCK_MONOTONIC, &now);
    if (rc != 0)
        return 0;
    return ((now.tv_sec * 1000) + (now.tv_nsec / 1000000));  // Convert everything into ms
}

bool Asset::send_to_comp_uri(bool value, const std::string& uri) {
    // Clear buffer for use
    send_FIMS_buf.clear();

    if (uri.empty()) {
        FPS_ERROR_LOG("Asset::send_to_comp_uri (bool) - uri does not exist.");
        return false;
    }

    bufJSON_StartObject(send_FIMS_buf);  // Object {
    bufJSON_AddBool(send_FIMS_buf, "value", value);
    bufJSON_EndObject(send_FIMS_buf);  // } Object

    bufJSON_RemoveTrailingComma(send_FIMS_buf);
    std::string object = to_string(send_FIMS_buf);

    json_object_send(object, uri);
    return true;
}

bool Asset::send_to_comp_uri(const char* value, const std::string& uri) {
    // Clear buffer for use
    send_FIMS_buf.clear();

    if ((value == NULL) || (uri.empty())) {
        FPS_ERROR_LOG("Asset::send_to_comp_uri (char) - value or uri does not exist.");
        return false;
    }

    bufJSON_StartObject(send_FIMS_buf);  // Object {
    bufJSON_AddString(send_FIMS_buf, "value", value);
    bufJSON_EndObject(send_FIMS_buf);  // } Object

    bufJSON_RemoveTrailingComma(send_FIMS_buf);
    std::string object = to_string(send_FIMS_buf);

    json_object_send(object, uri);
    return true;
}

bool Asset::send_to_comp_uri(float value, const std::string& uri) {
    // Clear buffer for use
    send_FIMS_buf.clear();

    if (uri.empty()) {
        FPS_ERROR_LOG("Asset::send_to_comp_uri (float) - uri does not exist.");
        return false;
    }

    bufJSON_StartObject(send_FIMS_buf);  // Object {
    bufJSON_AddNumber(send_FIMS_buf, "value", value);
    bufJSON_EndObject(send_FIMS_buf);  // } Object

    bufJSON_RemoveTrailingComma(send_FIMS_buf);
    std::string object = to_string(send_FIMS_buf);

    json_object_send(object, uri);
    return true;
}

bool Asset::send_to_comp_uri(int value, const std::string& uri) {
    // Clear buffer for use
    send_FIMS_buf.clear();

    if (uri.empty()) {
        FPS_ERROR_LOG("Asset::send_to_comp_uri (int) - uri does not exist.");
        return false;
    }

    bufJSON_StartObject(send_FIMS_buf);  // Object {
    bufJSON_AddNumber(send_FIMS_buf, "value", value);
    bufJSON_EndObject(send_FIMS_buf);  // } Object

    bufJSON_RemoveTrailingComma(send_FIMS_buf);
    std::string object = to_string(send_FIMS_buf);

    json_object_send(object, uri);
    return true;
}

/**
 * @brief Used to make json. If configurable_asset = true then include less information.
 *
 * @param buf: JSON fmt::memory_buffer
 * @param var: A specific variable
 * @param configurable_asset: This asset has been preconfigured and can include less information
 * @return true if the buffer was added successfully
 * @return false if an error occured modifying the buffer
 */
bool fimsCtl::makeJSONObject(fmt::memory_buffer& buf, const char* const var, bool configurable_asset) {
    // Skip building JSON object for UI controls that are not configured
    if (!configured) {
        return true;
    }

    if (var == NULL) {
        bufJSON_AddId(buf, obj_name);
        bufJSON_StartObject(buf);  // UiItemVar {
    }
    // Check if this is not the var we want and return if it isn't
    else if (strcmp(obj_name, var) != 0)
        return true;

    // If not pre-configured then add metadata
    if (!configurable_asset) {
        // name
        if (varName != NULL)
            bufJSON_AddString(buf, "name", varName);
        else {
            FPS_ERROR_LOG("FIMS control has NULL variable name.");
            return false;
        }

        // unit
        if (unit != NULL)
            bufJSON_AddString(buf, "unit", unit);

        // scaler
        bufJSON_AddNumber(buf, "scaler", scaler);

        // ui_type
        bufJSON_AddString(buf, "ui_type", "control");

        // ui_interaction type
        if (uiType != emptyNull) {
            char uiTypeStr[14];
            switch (uiType) {
                case enumStr:
                    strcpy(uiTypeStr, "enum");
                    break;
                case numberStr:
                    strcpy(uiTypeStr, "number");
                    break;
                case sliderStr:
                    strcpy(uiTypeStr, "enum_slider");
                    break;
                case buttonStr:
                    strcpy(uiTypeStr, "enum_button");
                    break;
                default:
                    break;
            }
            bufJSON_AddString(buf, "type", uiTypeStr);
        } else {
            FPS_ERROR_LOG("FIMS control UI type is NULL.");
            return false;
        }
    }

    // Adding the value with this switch always need the value
    switch (vt) {
        case Int:
            if (pDisplay != NULL)
                bufJSON_AddNumber(buf, "value", *(int*)pDisplay);
            break;
        case Float:
            if (pDisplay != NULL)
                bufJSON_AddNumber(buf, "value", *(float*)pDisplay);
            break;
        case Bool:
            if (pDisplay != NULL) {
                if (boolString)
                    bufJSON_AddString(buf, "value", (*(bool*)pDisplay) ? "Closed" : "Open");
                else
                    bufJSON_AddBool(buf, "value", *(bool*)pDisplay);
            }
            break;
        default: {
            FPS_ERROR_LOG("Invalid FIMS control value type %d.", vt);
            return false;
        }
    }

    // Always need enabled when pubbed to my knowledge button controls are enabled from these pubs
    bufJSON_AddBool(buf, "enabled", enabled);

    // No way to pre-configure options always add them here.
    bufJSON_AddId(buf, "options");
    bufJSON_StartArray(buf);  // UiItemOption [
    switch (options) {
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
    bufJSON_EndArray(buf);  // ] UiItemOption
    if (var == NULL)
        bufJSON_EndObject(buf);  // } UiItemVar

    return (true);
}

void build_on_off_option(fmt::memory_buffer& bufJactivePwrSetpointOption) {
    bufJSON_StartObject(bufJactivePwrSetpointOption);  // ObjOne {
    bufJSON_AddString(bufJactivePwrSetpointOption, "name", "On");
    bufJSON_AddBool(bufJactivePwrSetpointOption, "return_value", true);
    bufJSON_EndObject(bufJactivePwrSetpointOption);  // } ObjOne

    bufJSON_StartObject(bufJactivePwrSetpointOption);  // ObjTwo {
    bufJSON_AddString(bufJactivePwrSetpointOption, "name", "Off");
    bufJSON_AddBool(bufJactivePwrSetpointOption, "return_value", false);
    bufJSON_EndObject(bufJactivePwrSetpointOption);  // } ObjTwo
}

void build_yes_no_option(fmt::memory_buffer& bufJmanualModeOption) {
    bufJSON_StartObject(bufJmanualModeOption);  // ObjOne {
    bufJSON_AddString(bufJmanualModeOption, "name", "No");
    bufJSON_AddBool(bufJmanualModeOption, "return_value", false);
    bufJSON_EndObject(bufJmanualModeOption);  // } ObjOne

    bufJSON_StartObject(bufJmanualModeOption);  // ObjTwo {
    bufJSON_AddString(bufJmanualModeOption, "name", "Yes");
    bufJSON_AddBool(bufJmanualModeOption, "return_value", true);
    bufJSON_EndObject(bufJmanualModeOption);  // } ObjTwo
}

void build_close_option(fmt::memory_buffer& bufJstateCloseBreakerOption) {
    bufJSON_StartObject(bufJstateCloseBreakerOption);  // ObjOne {
    bufJSON_AddString(bufJstateCloseBreakerOption, "name", "Close");
    bufJSON_AddBool(bufJstateCloseBreakerOption, "return_value", true);
    bufJSON_EndObject(bufJstateCloseBreakerOption);  // } ObjOne
}

void build_open_option(fmt::memory_buffer& bufJstateOpenBreakerOption) {
    bufJSON_StartObject(bufJstateOpenBreakerOption);  // ObjOne {
    bufJSON_AddString(bufJstateOpenBreakerOption, "name", "Open");
    bufJSON_AddBool(bufJstateOpenBreakerOption, "return_value", true);
    bufJSON_EndObject(bufJstateOpenBreakerOption);  // } ObjOne
}

void build_reset_option(fmt::memory_buffer& bufJresetTargetOption) {
    bufJSON_StartObject(bufJresetTargetOption);  // ObjOne {
    bufJSON_AddString(bufJresetTargetOption, "name", "Clear Faults");
    bufJSON_AddBool(bufJresetTargetOption, "return_value", true);
    bufJSON_EndObject(bufJresetTargetOption);  // } ObjOne
}

/**
 * @brief This function takes as arguments a component id and a register id and returns
 * a component URI of the form "/components/<component id>/<register id>".
 *
 * @param comp: - component id
 * @param reg: - register id
 * @return std::string - component URI
 */
std::string build_uri(std::string comp, char* reg) {
    char uriWithComp[SHORT_MSG_LEN];
    snprintf(uriWithComp, SHORT_MSG_LEN, "/components/%s/%s", comp.c_str(), reg);
    return std::string(uriWithComp);
}

/**
 * @brief Checks if a new counter value has been seen in the past X milliseconds (where X = watchdog_timeout_ms).
 *
 * @return true - success
 * @return false - timeout
 */
bool Asset::check_fims_timeout(void) {
    bool prev_fims_timeout = fims_timeout;
    if (watchdog_heartbeat.value.value_int != prev_watchdog_heartbeat)  // New counter value seen, so reset fims_timer
    {
        prev_watchdog_heartbeat = watchdog_heartbeat.value.value_int;  // Store most recent counter value
        clock_gettime(CLOCK_MONOTONIC, &fims_timer);                   // Get current time
        increment_timespec_ms(fims_timer, watchdog_timeout_ms);        // Reset fims timeout clock
        fims_timeout = false;                                          // Return false since new counter value means fims has not timed out
    } else {
        timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);  // Get current time
        fims_timeout = check_expired_time(now, fims_timer);
    }
    if (prev_fims_timeout != fims_timeout) {
        char event_msg[MEDIUM_MSG_LEN];
        if (fims_timeout) {  // If there was a FIMS timeout, emit an event
            snprintf(event_msg, MEDIUM_MSG_LEN, "Asset %s watchdog timeout", get_name().c_str());
            emit_event("Assets", event_msg, ALARM_ALERT);
        } else {  // If FIMS gets reconnected, emit an event
            snprintf(event_msg, MEDIUM_MSG_LEN, "Asset %s watchdog reconnected", get_name().c_str());
            emit_event("Assets", event_msg, STATUS_ALERT);
        }
    }
    return fims_timeout;
}
