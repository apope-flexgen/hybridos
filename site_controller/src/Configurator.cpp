/**
 * Configurator.cpp
 * Contains variables and functions relevant to asset and asset type configuration
 *
 * Created on March 31, 2021
 *      Author: Jack Timothy (jtimothy)
 */

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include "Types.h"
#include <Configurator.h>
#include <Type_Manager.h>
#include <Asset.h>
#include <cjson/cJSON.h>

/**
 * Constructor that assigns internal variables.
 * @param pMan pointer to the Type_Manager being configured
 * @param pCVM pointer to full component variable map containing all component uris and Fims_Objects
 * @param pc pointer to the primary status flag for this controller
 */
Type_Configurator::Type_Configurator(Type_Manager* pMan, std::map<std::string, std::vector<Fims_Object*>>* pCVM, bool* pc) {
    // Set internal pointers to passed-in pointers
    p_manager = pMan;
    pCompVarMap = pCVM;
    p_is_primary_controller = pc;

    // config_validation should be true in default case
    config_validation = true;
}

/**
 * @brief Extracts asset type variables for this type and configures all asset instances of this type
 *
 * @return Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
Config_Validation_Result Type_Configurator::create_assets(void) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    // The "asset_instances" array in assets.json is a mix of concrete asset instances and template-defined asset instances
    cJSON* assetInstanceArray = cJSON_GetObjectItem(asset_type_root, "asset_instances");
    int numInstances = 0;
    if (assetInstanceArray == NULL) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to find asset_instances in config.", p_manager->get_asset_type_id())));
    } else {
        // Count the number of asset instances, including multiple instances contained within one template
        std::pair<int, Config_Validation_Result> instance_config_result = count_num_asset_instances(assetInstanceArray);
        numInstances = instance_config_result.first;
        if (numInstances > MAX_NUM_ASSETS) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: exceeded maximum number of asset instances, limit is {}, config file has {}.", MAX_NUM_ASSETS, numInstances, p_manager->get_asset_type_id())));
        }
        validation_result.absorb(instance_config_result.second);
    }

    // Proceed with configuration if there are asset instance objects found in assets.json
    // Else, print an error and kill configuration because if there are no asset instances, there should not have been an object for this asset type
    if (numInstances > 0) {
        int numArrayEntries = cJSON_GetArraySize(assetInstanceArray);
        // Iterate through the asset array to configure all instances represented by each entry
        // If there are too many assets provided, at least report any errors encountered up to the maximum number we're able to process
        for (int i = 0; i < numArrayEntries && i < MAX_NUM_ASSETS; ++i) {
            cJSON* array_entry = cJSON_GetArrayItem(assetInstanceArray, i);
            if (array_entry == NULL) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: asset array entry {} is invalid or undefined", p_manager->get_asset_type_id(), i + 1)));
                continue;
            }
            Config_Validation_Result entry_config_result = configure_asset_array_entry(i, array_entry);
            if (!entry_config_result.is_valid_config) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure asset array entry {}, error in assets.json.", p_manager->get_asset_type_id(), i + 1)));
            }
            validation_result.absorb(entry_config_result);
        }
    } else {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: error with asset instance count. Expected positive integer, but got {}.", p_manager->get_asset_type_id(), numInstances)));
    }

    // Carry out any configuration actions unique to the derived asset type managers
    Config_Validation_Result type_config_result = p_manager->configure_type_manager(this);
    if (!type_config_result.is_valid_config) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: error configuring asset type.", p_manager->get_asset_type_id())));
    }
    validation_result.absorb(type_config_result);

    // Give base class functions access to asset instance pointers
    p_manager->configure_base_class_list();

    // Used for testing
    // p_manager->print_alarm_fault_map(p_asset_var_map);

    return validation_result;
}

/**
 * Helper that ensures the name and id are defined. If they are missing, a nontemplated placeholder name or id will be inserted into the cJSON object.
 * Many subsequent functions make the assumption that these values are valid so they must be defined for configuration to continue.
 * @param index index in the asset entry array
 * @param array_entry the asset entry parsed from configuration
 */
Config_Validation_Result Type_Configurator::check_name_and_id(int index, cJSON* array_entry) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    cJSON* asset_name = cJSON_GetObjectItem(array_entry, "name");
    cJSON* asset_id = cJSON_GetObjectItem(array_entry, "id");
    // If the name or id are not defined, try to continue on with a placeholder name to check for other config errors
    if ((asset_name == nullptr) || (asset_name->valuestring == nullptr)) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: name is missing for asset entry {}.", p_manager->get_asset_type_id(), index)));
        current_asset_name = fmt::format("{}_{}", p_manager->get_asset_type_id(), index);
    } else {
        current_asset_name = asset_name->valuestring;
    }
    if (!asset_id || !asset_id->valuestring) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: id is missing for asset entry {}.", p_manager->get_asset_type_id(), index)));
        current_asset_id = fmt::format("{}_{}", p_manager->get_asset_type_id(), index);
    } else {
        current_asset_id = asset_id->valuestring;
    }
    return validation_result;
}

/**
 * @brief Figures out if asset array entry is template representing multiple asset instances or just represents one asset instance.
 * Calls configuration functions based on the result.
 *
 * @param array_entry: Pointer to the asset array entry
 * @return Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
Config_Validation_Result Type_Configurator::configure_asset_array_entry(int index, cJSON* array_entry) {
    // Struct to aggregate and report on all asset type configuration issues
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    asset_config.asset_instance_root = array_entry;
    validation_result.absorb(check_name_and_id(index, array_entry));

    asset_config.asset_instance_root = array_entry;  // Set the asset instance root to the array entry
    std::pair<int, Config_Validation_Result> template_config_result = entry_is_template(asset_config.asset_instance_root);
    // Continue to gather more configuration errors
    if (template_config_result.first == TEMPLATING_ERROR) {
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: error processing templating", p_manager->get_asset_type_id())));
    }
    validation_result.absorb(template_config_result.second);

    // Place the asset name <ess_#> and create a map for it's asset numbering
    asset_config.asset_name_to_asset_number_map.emplace(current_asset_name, std::map<int, bool>());
    // Place the asset id <ess_#> and create a map for it's asset numbering
    asset_config.asset_id_to_asset_number_map.emplace(current_asset_id, std::map<int, bool>());

    Config_Validation_Result asset_config_result = Config_Validation_Result(true);
    switch (template_config_result.first) {
        case NON_TEMPLATE:
            asset_config_result = configure_single_asset();
            break;
        case TEMPLATING_ERROR:
            // Passing this to gather more errors
            // Others just call configure_single_asset() in a loop anyway
            asset_config_result = configure_single_asset();
            break;
        case TRADITIONAL:
            asset_config_result = configure_traditional_templated_asset();
            break;
        case RANGED:
            asset_config_result = configure_ranged_templated_asset();
            break;
        default:
            asset_config_result.is_valid_config = false;
            asset_config_result.ERROR_details.push_back(Result_Details(fmt::format("{}: got invalid asset template type", current_asset_name)));
            break;
    }
    validation_result.absorb(asset_config_result);
    return validation_result;
}

/**
 * @brief Traditional templating: get the num of instances represented by the template and configure each instance
 * TODO: There is now two types of templating. They don't play well together. Which is intentional.
 *     If you want to configure assets non-sequentially, use the new templating method.
 *     Should we make them more cohabitable?
 * @param id the id received from configuration. If not defined, a placeholder id will be passed in the form <asset type>_<asset index> i.e. ess_1
 * @param id the name received from configuration. If not defined, a placeholder id will be passed in the form <asset type>_<asset index> i.e. ess_1
 *
 * @return Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
Config_Validation_Result Type_Configurator::configure_traditional_templated_asset() {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    // get the number of instances represented by the template
    int num_instances_represented = extract_num_asset_instances_represented(asset_config.asset_instance_root);
    if (num_instances_represented >= 1) {
        for (int j = 0; j < num_instances_represented; ++j) {
            // Insert number into range map
            auto check_name = (&asset_config.asset_name_to_asset_number_map[current_asset_name])->emplace(std::pair<int, bool>(j + 1, false));
            auto check_id = (&asset_config.asset_id_to_asset_number_map[current_asset_id])->emplace(std::pair<int, bool>(j + 1, false));

            if (!check_id.second || !check_name.second) {  // You just tried to insert a duplicate asset number which is not allowed
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: template entry {} with index {} overlaps with existing entries.", p_manager->get_asset_type_id(), current_asset_name, j + 1)));
                // This is a templated entry where all values are the same for each asset,
                // so only try to configure a single asset when overlapping entries are found
                // this prevents the same error from being reported multiple times
                if (j > 0) {
                    break;
                }
            }
            asset_config.asset_num = j + 1;

            // Acutally configure the asset by calling configure_single_asset()
            Config_Validation_Result asset_config_result = configure_single_asset();
            // Verify
            if (!asset_config_result.is_valid_config) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure template entry {}.", current_asset_name, j + 1)));
            }
            validation_result.absorb(asset_config_result);
        }
    } else {  // Why are you configuring a template with no instances?
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: invalid number_of_instances provided for templated asset. Expected positive integer, but got {}.", current_asset_name, num_instances_represented)));
    }
    return validation_result;
}

/**
 * @brief A ranged template is passed a range of strings or numbers and creates an asset instance for each string or number in the range
 * A string comprises 2 numbers and a delimiter(..) that represents a range of numbers
 * TODO: There is now two types of templating. They don't play well together. Which is intentional.
 *      Should we make them more cohabitable?
 *
 * @return Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
Config_Validation_Result Type_Configurator::configure_ranged_templated_asset() {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    // We generate the list and then iterate through this is the range <1, 5, 7, 9> etc.
    std::pair<std::vector<int>, Config_Validation_Result> instance_config_result = generate_list_of_asset_instances_represented(asset_config.asset_instance_root);

    // Insert all asset instances into map.
    for (const auto& asset_num : instance_config_result.first) {
        asset_config.asset_num = asset_num;
        auto check_name = asset_config.asset_name_to_asset_number_map[current_asset_name].emplace(std::pair<int, bool>(asset_config.asset_num, false));
        auto check_id = asset_config.asset_id_to_asset_number_map[current_asset_id].emplace(std::pair<int, bool>(asset_config.asset_num, false));
        // This means you are trying to re-configure ess_02 twice for example.
        if (!check_id.second || !check_name.second) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: template entry {} with index {} overlaps with existing entries.", p_manager->get_asset_type_id(), current_asset_name, asset_num)));

            // This is a templated entry where all values are the same for each asset,
            // so only try to configure a single asset when overlapping entries are found
            // this prevents the same error from being reported multiple times
            if (asset_num > instance_config_result.first[0]) {
                break;
            }
        }

        // Actually configure the asset by calling configure_single_asset()
        Config_Validation_Result asset_config_result = configure_single_asset();
        if (!asset_config_result.is_valid_config) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure template entry {}.", current_asset_name, asset_num)));
        }
        validation_result.absorb(asset_config_result);
    }
    if (instance_config_result.first.empty()) {  // broken range?
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: invalid range provided for templated asset. Expected a positive number of assets, but got zero.", current_asset_name)));
    }
    return validation_result;
}

/**
 * @brief Allocates memory for new asset, configures that individual asset, then adds new asset to the list of assets.
 *
 * @return Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
Config_Validation_Result Type_Configurator::configure_single_asset(void) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    // Here is the allocation of memory for the asset
    Asset* asset = p_manager->build_new_asset();
    if (asset == NULL) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: error allocating memory for new asset instance.", p_manager->get_asset_type_id())));
    }

    Config_Validation_Result asset_config_result = asset->configure(this);  // Call the asset's configure function different for each asset type
    if (!asset_config_result.is_valid_config) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: failed to configure asset.", p_manager->get_asset_type_id())));
    }
    validation_result.absorb(asset_config_result);

    p_manager->append_new_asset(asset);
    return validation_result;
}

/**
 * @brief Helper function for counting the number of asset instances of a given type in assets.json,
 * including multiple instances contained within one template.
 *
 * @param asset_array: The array of assets of a given type in assets.json
 * @return Pair containing the number of instances as well as a
 * Config_Validation_Result struct indicating whether counting was successful and any errors that occurred
 */
std::pair<int, Config_Validation_Result> Type_Configurator::count_num_asset_instances(cJSON* asset_array) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    int asset_array_size = cJSON_GetArraySize(asset_array);
    if (asset_array_size == 0) {
        validation_result.WARNING_details.push_back(Result_Details(fmt::format("{}: Asset array is empty.", p_manager->get_asset_type_id())));
        return std::make_pair(0, validation_result);
    }

    return gather_assets(asset_array);
}

/**
 * @brief Helper function for counting the number of asset instances,
 * including multiple instances contained within one template
 * if not a template, returns 1
 *
 * @param asset_array: The array of assets of a given type in assets.json
 * @return Pair containing the number of instances as well as a
 * Config_Validation_Result struct indicating whether counting was successful and any errors that occurred
 */
std::pair<int, Config_Validation_Result> Type_Configurator::gather_assets(cJSON* asset_array) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    int num_asset_instances = 0;
    int asset_array_size = cJSON_GetArraySize(asset_array);
    for (int i = 0; i < asset_array_size; ++i) {
        int instances = -1;
        cJSON* entry = cJSON_GetArrayItem(asset_array, i);
        if (entry == NULL) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: asset array entry {}, Invalid or NULL entry.", p_manager->get_asset_type_id(), i + 1)));
            return std::make_pair(0, validation_result);
        }

        validation_result.absorb(check_name_and_id(i, entry));

        std::pair<int, Config_Validation_Result> template_config_result = entry_is_template(entry);
        std::pair<int, Config_Validation_Result> count_result = std::make_pair(0, Config_Validation_Result(true));
        switch (template_config_result.first) {
            case TRADITIONAL:
                instances = count_traditional_templated_assets(entry);
                break;
            case RANGED:
                count_result = count_ranged_templated_assets(entry);
                instances = count_result.first;
                break;
            case NON_TEMPLATE:
                instances = 1;
                break;
            default:  // This is the TEMPLATING_ERROR case
                instances = 1;
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: invalid template type when gathering assets.", p_manager->get_asset_type_id())));
                validation_result.absorb(template_config_result.second);
        }
        num_asset_instances += instances;
        validation_result.absorb(template_config_result.second);
        validation_result.absorb(count_result.second);
    }

    return std::make_pair(num_asset_instances, validation_result);
}

/**
 * @brief Simply grab the json number provided.
 *
 * @param entry: The asset array entry that is a trad template.
 * @return int The number of asset instances represented.
 */
int Type_Configurator::count_traditional_templated_assets(cJSON* entry) {
    int num_instances_represented = extract_num_asset_instances_represented(entry);
    return num_instances_represented >= 1 ? num_instances_represented : TEMPLATING_ERROR;
}

/**
 * @brief Parse ranges and return a vector.size().
 *
 * @param entry: The asset array entry that is a ranged template.
 * @return Pair containing the int representation of the template type (see the template_type enum) as well as a
 * Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
std::pair<int, Config_Validation_Result> Type_Configurator::count_ranged_templated_assets(cJSON* entry) {
    if (range_provided(entry)) {
        std::pair<std::vector<int>, Config_Validation_Result> instances_config_result = generate_list_of_asset_instances_represented(entry);
        return std::make_pair(instances_config_result.first.size(), instances_config_result.second);
    }
    // You've provided an invalid config
    return std::make_pair(-1, Config_Validation_Result(false));
}

/**
 * @brief An asset array entry that is a template must contain the "number_of_instances" field or the
 * "range" field to say how many instances the template represents
 * if neither field is there, then array entry is not a template and is a concrete representation of a single asset instance
 *
 * @param asset_array_entry: The asset array entry to be tested for template status
 * @return Pair containing the int representation of the template type (see the template_type enum) as well as a
 * Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
std::pair<int, Config_Validation_Result> Type_Configurator::entry_is_template(cJSON* asset_array_entry) {
    // Struct to aggregate and report on all asset type configuration issues
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    bool ranged_template = false;
    asset_config.is_template_flag = false;
    // If number_of_instances field does not exist, this is a concrete entry and only represents one asset instance
    cJSON* num_instances_obj = cJSON_GetObjectItem(asset_array_entry, "number_of_instances");
    cJSON* range = cJSON_GetObjectItem(asset_array_entry, "range");
    // If no templating variables are provided, then this is a concrete asset
    if (num_instances_obj == NULL && range == NULL) {
        return std::make_pair(NON_TEMPLATE, validation_result);
    }
    // Make sure only one style of templating is being used
    if (num_instances_obj != NULL && range != NULL) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: cannot configure a templated asset with both 'number_of_instances' and 'range' fields.", p_manager->get_asset_type_id())));
    }

    if (num_instances_obj != NULL) {
        if (num_instances_obj->valueint < 1) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: error parsing asset {}: 'number_of_instances' must be a positive integer.", p_manager->get_asset_type_id(), current_asset_name)));
        }
    } else if (range != NULL) {
        if (range->type != cJSON_Array) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: error parsing asset {}: 'range' must be an array.", p_manager->get_asset_type_id(), current_asset_name)));
        }
        int array_size = cJSON_GetArraySize(range);
        for (int i = 0; i < array_size; ++i) {
            cJSON* range_entry = cJSON_GetArrayItem(range, i);
            if (range_entry->type != cJSON_String && range_entry->type != cJSON_Number) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: error parsing asset {}: 'range' array must contain only numbers or strings.", p_manager->get_asset_type_id(), current_asset_name)));
            }
        }
        ranged_template = true;
    }

    // If the name or id are not defined, try to continue on with placeholders to check for other config errors
    bool name_has_wildcard = current_asset_name.find('#') != std::string::npos;
    bool id_has_wildcard = current_asset_id.find('#') != std::string::npos;

    // ID having wildcard is the absolute sign that an asset entry must be a template
    if (id_has_wildcard) {
        // An asset entry being a template means the name must have a wildcard
        if (!name_has_wildcard) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(
                Result_Details(fmt::format("{}: error parsing asset with ID {} and name {}: templates must have wildcard character in asset name.", p_manager->get_asset_type_id(), current_asset_id, current_asset_name)));
        }
        // Both ID and name have wildcards and number_of_instances >= 1, so safe to return non-zero for entry being template
        asset_config.is_template_flag = true;
        if (validation_result.is_valid_config) {
            return std::make_pair(ranged_template ? RANGED : TRADITIONAL, validation_result);
        }
        // If ID is missing wildcard, the entry may pass as a concrete asset instance as long as number_of_instances is exactly 1
    } else {
        if (num_instances_obj->valueint == 1) {
            if (validation_result.is_valid_config) {
                return std::make_pair(NON_TEMPLATE, validation_result);
            }
        } else {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(
                Result_Details(fmt::format("{}: error parsing asset with ID {} and name {}: if 'number_of_instances' is greater than 1, the asset entry is considered templated and must have a wildcard character in the ID and name.",
                                           p_manager->get_asset_type_id(), current_asset_id, current_asset_name)));
        }
    }
    return std::make_pair(TEMPLATING_ERROR, validation_result);
}

/**
 * @brief Returns the number of asset instances a single template represents
 *
 * @param asset_array_entry: The asset array entry
 * @return int The number of asset instances represented
 */
int Type_Configurator::extract_num_asset_instances_represented(cJSON* asset_array_entry) {
    // If number_of_instances field does not exist, this is a concrete entry and only represents one asset instance
    cJSON* num_instances_obj = cJSON_GetObjectItem(asset_array_entry, "number_of_instances");
    return num_instances_obj->valueint ? num_instances_obj->valueint : -1;
}

/**
 * @brief Helper function that handles counting a template with a range field instead of number_of_instances
 * Example:
 *      "range": ["1..3", 5, "7..9"]
 *      [1, 2, 3, 5, 7, 8, 9]
 *
 * @param asset_array_entry: The range entry to be parsed
 * @return vector<int> The list of asset instances represented by the range
 * @return vector<int> Empty vector if error
 * @return Config_Validation_Result struct indicating whether configuration was successful and any errors that occurred
 */
std::pair<std::vector<int>, Config_Validation_Result> Type_Configurator::generate_list_of_asset_instances_represented(cJSON* asset_array_entry) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    std::vector<int> asset_instance_list;
    if (!asset_array_entry) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: no asset instance array provided.", p_manager->get_asset_type_id())));
        return std::make_pair(std::vector<int>(), validation_result);
    }

    auto range = cJSON_GetObjectItem(asset_array_entry, "range");
    if (!range) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: asset 'range' field not found.", p_manager->get_asset_type_id())));
        return std::make_pair(std::vector<int>(), validation_result);
    }

    for (int i = 0; i < cJSON_GetArraySize(range); ++i) {
        auto range_entry = cJSON_GetArrayItem(range, i);
        if (!range_entry) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: asset 'range' array entry is empty.", p_manager->get_asset_type_id())));
            return std::make_pair(std::vector<int>(), validation_result);
        }
        // String case (e.g. "1..3")
        if (range_entry->type == cJSON_String) {
            // Make sure there is only one ..
            std::string range_entry_str = range_entry->valuestring;
            if (range_entry_str.find("..") != range_entry_str.rfind("..")) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(
                    Result_Details(fmt::format("{}: 'range' array must contain only numbers or strings and strings must be in the format (\"x .. y\").", cJSON_GetObjectItem(asset_array_entry, "name")->valuestring)));
                return std::make_pair(std::vector<int>(), validation_result);
            } else {
                try {
                    // Split off delimiter(..)
                    std::string left_num = range_entry_str.substr(0, range_entry_str.find(".."));
                    std::string right_num = range_entry_str.substr(range_entry_str.find("..") + 2, range_entry_str.length());
                    int left_num_int = std::stoi(left_num);
                    int right_num_int = std::stoi(right_num);
                    for (int i = left_num_int; i <= right_num_int; ++i) {
                        asset_instance_list.push_back(i);
                    }
                } catch (std::invalid_argument& e) {
                    validation_result.is_valid_config = false;
                    validation_result.ERROR_details.push_back(
                        Result_Details(fmt::format("{}: 'range' array must contain only numbers or strings and strings must be in the format (\"x .. y\").", cJSON_GetObjectItem(asset_array_entry, "name")->valuestring)));
                    return std::make_pair(std::vector<int>(), validation_result);
                }
            }
        } else if (range_entry->type == cJSON_Number)  // Number case (e.g. 5)
        {
            asset_instance_list.push_back(range_entry->valueint);
        } else {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("{}: 'range' array must contain only numbers or strings.", cJSON_GetObjectItem(asset_array_entry, "name")->valuestring)));
            return std::make_pair(std::vector<int>(), validation_result);
        }
    }
    return std::make_pair(asset_instance_list, validation_result);
}

/**
 * @brief Checks to see if asset range provided for templating
 *
 * @param asset_array_entry: The asset array entry
 * @return int 0 if no range provided, 1 if range provided and valid
 */
bool Type_Configurator::range_provided(cJSON* asset_array_entry) {
    // Get the range array and make sure it's size > 0
    cJSON* range = cJSON_GetObjectItem(asset_array_entry, "range");
    return range != NULL ? (cJSON_GetArraySize(range) > 0) : 0;  // 0 if no range provided, 1 if range provided and valid
}

/**
 * @brief Constructor for individual asset configurator class
 */
Asset_Configurator::Asset_Configurator(void) {
    is_template_flag = false;
    asset_instance_root = NULL;
    asset_num = -1;
}
