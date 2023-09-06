/**
 * Configurator.cpp
 * Contains variables and functions relevant to configuration
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
 * @return true if configuration was successful
 * @return false if configuration was unsuccessful
 */
bool Type_Configurator::create_assets(void) {
    // The "asset_instances" array in assets.json is a mix of concrete asset instances and template-defined asset instances
    cJSON* assetInstanceArray = cJSON_GetObjectItem(asset_type_root, "asset_instances");
    if (assetInstanceArray == NULL) {
        FPS_ERROR_LOG("Type_Configurator::create_assets ~ Failed to find asset_instances in config.");
        return false;
    }

    // Count the number of asset instances, including multiple instances contained within one template
    int numInstances = count_num_asset_instances(assetInstanceArray);
    if (numInstances > MAX_NUM_ASSETS) {
        FPS_ERROR_LOG("Type_Configurator::create_assets ~ Exceeded maximum number of asset instances, limit is %d, config file has %d.", MAX_NUM_ASSETS, numInstances);
        return false;
    }

    // Proceed with configuration if there are asset instance objects found in assets.json
    // Else, print an error and kill configuration because if there are no asset instances, there should not have been an object for this asset type
    if (numInstances > 0) {
        int numArrayEntries = cJSON_GetArraySize(assetInstanceArray);
        // Iterate through the asset array to configure all instances represented by each entry
        for (int i = 0; i < numArrayEntries; ++i) {
            cJSON* array_entry = cJSON_GetArrayItem(assetInstanceArray, i);
            if (array_entry == NULL) {
                FPS_ERROR_LOG("Type_Configurator::create_assets ~ Asset array entry %d, Invalid or NULL entry.", i);
                return false;
            }
            if (!configure_asset_array_entry(array_entry)) {
                FPS_ERROR_LOG("Type_Configurator::create_assets ~ Failed to configure asset array entry %d, error in assets.json.", i);
                return false;
            }
        }
    } else {
        FPS_ERROR_LOG("Type_Configurator::create_assets ~ Error with asset instance count. Expected positive integer, got %d.", numInstances);
        return false;
    }

    // Carry out any configuration actions unique to the derived asset type managers
    if (!p_manager->configure_type_manager(this)) {
        FPS_ERROR_LOG("Type_Configurator::create_assets ~ Error configuring asset type. Asset Type is: %s", p_manager->get_asset_type_id());
        return false;
    }

    // Give base class functions access to asset instance pointers
    p_manager->configure_base_class_list();

    // Used for testing
    // p_manager->print_alarm_fault_map(p_asset_var_map);

    return true;
}

/**
 * @brief Figures out if asset array entry is template representing multiple asset instances or just represents one asset instance.
 * Calls configuration functions based on the result.
 *
 * @param array_entry: Pointer to the asset array entry
 * @return true if configuration was successful
 * @return false if configuration was unsuccessful
 */
bool Type_Configurator::configure_asset_array_entry(cJSON* array_entry) {
    asset_config.asset_instance_root = array_entry;  // Set the asset instance root to the array entry
    int is_template = entry_is_template(asset_config.asset_instance_root);
    if (is_template == -1) {
        FPS_ERROR_LOG("Type_Configurator::configure_asset_array_entry ~ Error within entry_is_template().");
        return false;
    }

    cJSON* asset_name = cJSON_GetObjectItem(asset_config.asset_instance_root, "name");
    cJSON* asset_id = cJSON_GetObjectItem(asset_config.asset_instance_root, "id");

    asset_config.asset_id_to_asset_number_map.insert(std::pair<std::string, std::map<int, bool>>(asset_id->valuestring, std::map<int, bool>()));
    asset_config.asset_name_to_asset_number_map.insert(std::pair<std::string, std::map<int, bool>>(asset_name->valuestring, std::map<int, bool>()));

    switch (is_template) {
        case NON_TEMPLATE:
            return configure_single_asset();
        case TRADITIONAL:
            return configure_traditional_templated_asset();
        case RANGED:
            return configure_ranged_templated_asset();
        case TEMPLATING_ERROR:
            FPS_ERROR_LOG("Type_Configurator::configure_asset_array_entry ~ Error within entry_is_template().");
            return false;
        default:
            FPS_ERROR_LOG("Error within configure_asset_array_entry(). In default case.");
            return false;
    }
}

/**
 * @brief Traditional templating: get the num of instances represented by the template and configure each instance
 * TODO: There is now two types of templating. They don't play well together. Which is intentional.
 *     If you want to configure assets non-sequentially, use the new templating method.
 *     Should we make them more cohabitatable?
 *
 * @return true if configuration was successful
 * @return false if configuration was unsuccessful
 */
bool Type_Configurator::configure_traditional_templated_asset(void) {
    cJSON* asset_name = cJSON_GetObjectItem(asset_config.asset_instance_root, "name");
    cJSON* asset_id = cJSON_GetObjectItem(asset_config.asset_instance_root, "id");

    int num_instances_represented = extract_num_asset_instances_represented(asset_config.asset_instance_root);
    if (num_instances_represented >= 1) {
        for (int j = 0; j < num_instances_represented; ++j) {
            // Insert number into range map
            auto check_id = (&asset_config.asset_id_to_asset_number_map[asset_id->valuestring])->insert(std::pair<int, bool>(j + 1, false));
            auto check_name = (&asset_config.asset_name_to_asset_number_map[asset_name->valuestring])->insert(std::pair<int, bool>(j + 1, false));

            if (!check_id.second || !check_name.second) {
                FPS_ERROR_LOG("Type_Configurator::configure_traditional_templated_asset ~ Duplicate asset ID or Name found in assets.json.");
                return false;
            }
            asset_config.asset_num = j + 1;
            if (!configure_single_asset()) {
                FPS_ERROR_LOG("Type_Configurator::configure_asset_array_entry ~ configure_single_asset() failure.");
                return false;
            }
        }
    } else {
        FPS_ERROR_LOG("Type_Configurator::configure_asset_array_entry ~ Mistake in configuration of number_of_instances for an asset array entry, expected positive integer.");
        return false;
    }
    return true;
}

/**
 * @brief A ranged template is passed a range of strings or numbers and creates an asset instance for each string or number in the range
 * A string comprises 2 numbers and a delimiter(..) that represents a range of numbers
 * TODO: There is now two types of templating. They don't play well together. Which is intentional.
 *      Should we make them more cohabitatable?
 *
 * @return true if configuration was successful
 * @return false if configuration was unsuccessful
 */
bool Type_Configurator::configure_ranged_templated_asset(void) {
    cJSON* asset_name = cJSON_GetObjectItem(asset_config.asset_instance_root, "name");
    cJSON* asset_id = cJSON_GetObjectItem(asset_config.asset_instance_root, "id");
    std::vector<int> asset_list = generate_list_of_asset_instances_represented(asset_config.asset_instance_root);

    // Insert all asset instances into map.
    for (const auto& asset_num : asset_list) {
        asset_config.asset_num = asset_num;
        auto check_id = asset_config.asset_id_to_asset_number_map[asset_id->valuestring].insert(std::pair<int, bool>(asset_config.asset_num, false));
        auto check_name = asset_config.asset_name_to_asset_number_map[asset_name->valuestring].insert(std::pair<int, bool>(asset_config.asset_num, false));
        if (!check_id.second || !check_name.second) {
            // This means you are trying to re-configure ess_02 twice for example.
            FPS_ERROR_LOG("Duplicate asset ID found in assets.json. Asset_id: %s Asset_num: %d.", asset_id->valuestring, asset_num);
            return false;
        }
        if (!configure_single_asset()) {
            FPS_ERROR_LOG("Type_Configurator::configure_asset_array_entry ~ configure_single_asset() failure.");
            return false;
        }
    }
    if (asset_list.empty()) {
        FPS_ERROR_LOG("Type_Configurator::configure_ranged_templated_asset ~ No asset instances represented by this template see range field in asset template.");
    }
    return asset_list.empty() ? false : true;
}

/**
 * @brief Allocates memory for new asset, configures that individual asset, then adds new asset to the list of assets.
 *
 * @return true if configuration was successful
 * @return false if configuration was unsuccessful
 */
bool Type_Configurator::configure_single_asset(void) {
    Asset* asset = p_manager->build_new_asset();
    if (asset == NULL) {
        FPS_ERROR_LOG("Type_Configurator::configure_single_asset ~ Error allocating memory for new asset instance.");
        return false;
    }

    if (!asset->configure(this)) {
        FPS_ERROR_LOG("Type_Configurator::configure_single_asset ~ Failed to configure asset.");
        return false;
    }

    p_manager->append_new_asset(asset);
    return true;
}

/**
 * @brief Helper function for counting the number of asset instances of a given type in assets.json,
 * including multiple instances contained within one template.
 *
 * @param asset_array: The array of assets of a given type in assets.json
 * @return int The number of asset instances represented.
 */
int Type_Configurator::count_num_asset_instances(cJSON* asset_array) {
    int asset_array_size = cJSON_GetArraySize(asset_array);
    if (asset_array_size == 0) {
        FPS_WARNING_LOG("Type_Configurator::asset_count_num_asset_instances ~ Asset array is empty.");
    }
    return asset_array_size > 0 ? gather_assets(asset_array) : 0;
}

/**
 * @brief Helper function for counting the number of asset instances,
 * including multiple instances contained within one template
 * if not a template, returns 1
 *
 * @param asset_array: The array of assets of a given type in assets.json
 * @return int The number of asset instances represented.
 */
int Type_Configurator::gather_assets(cJSON* asset_array) {
    int num_asset_instances = 0;
    int asset_array_size = cJSON_GetArraySize(asset_array);
    for (int i = 0; i < asset_array_size; ++i) {
        int instances = -1;
        cJSON* entry = cJSON_GetArrayItem(asset_array, i);
        if (entry == NULL) {
            FPS_ERROR_LOG("Type_Configurator::asset_count_num_asset_instances ~ Asset array entry %d, Invalid or NULL entry.", i + 1);
            return -1;
        }

        switch (entry_is_template(entry)) {
            case TRADITIONAL:
                instances = count_traditional_templated_assets(entry);
                break;
            case RANGED:
                instances = count_ranged_templated_assets(entry);
                break;
            case NON_TEMPLATE:
                instances = 1;
                break;
            default:
                FPS_ERROR_LOG("Type_Configurator::asset_count_num_asset_instances ~ Invalid template type.");
                return -1;
        }
        num_asset_instances += instances;
    }
    return num_asset_instances;
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
 * @return int The number of asset instances represented.
 * @return -1 if range is invalid.
 */
int Type_Configurator::count_ranged_templated_assets(cJSON* entry) {
    return range_provided(entry) ? generate_list_of_asset_instances_represented(entry).size() : -1;
}

/**
 * @brief An asset array entry that is a template must contain the "number_of_instances" field or the
 * "range" field to say how many instances the template represents
 * if neither field is there, then array entry is not a template and is a concrete representation of a single asset instance
 *
 * @param asset_array_entry: The asset array entry to be tested for template status
 * @return int The template type (see the template_type enum)
 */
int Type_Configurator::entry_is_template(cJSON* asset_array_entry) {
    int ranged_template = false;
    asset_config.is_template_flag = false;
    // If number_of_instances field does not exist, this is a concrete entry and only represents one asset instance
    cJSON* num_instances_obj = cJSON_GetObjectItem(asset_array_entry, "number_of_instances");
    cJSON* range = cJSON_GetObjectItem(asset_array_entry, "range");
    // If no templating variables are provided, then this is a concrete asset
    if (num_instances_obj == NULL && range == NULL) {
        return NON_TEMPLATE;
    }
    // Make sure only one style of templating is being used
    if (num_instances_obj != NULL && range != NULL) {
        FPS_ERROR_LOG("Cannot configure a templated asset with both 'number_of_instances' and 'range' fields.");
        return TEMPLATING_ERROR;
    }

    cJSON* asset_name = cJSON_GetObjectItem(asset_array_entry, "name");
    if (asset_name == NULL) {
        FPS_ERROR_LOG("Error parsing asset: 'name' field not found.");
        return TEMPLATING_ERROR;
    }

    cJSON* asset_id = cJSON_GetObjectItem(asset_array_entry, "id");
    if (asset_id == NULL) {
        FPS_ERROR_LOG("Error parsing asset: 'id' field not found.");
        return TEMPLATING_ERROR;
    }

    if (num_instances_obj != NULL) {
        if (num_instances_obj && num_instances_obj->valueint < 1) {
            FPS_ERROR_LOG("Error parsing asset %s: 'number_of_instances' must be a positive integer.", asset_name->valuestring);
            return TEMPLATING_ERROR;
        }
    } else if (range != NULL) {
        if (range->type != cJSON_Array) {
            FPS_ERROR_LOG("Error parsing asset %s: 'range' must be an array.", asset_name->valuestring);
            return TEMPLATING_ERROR;
        }
        int array_size = cJSON_GetArraySize(range);
        for (int i = 0; i < array_size; ++i) {
            cJSON* range_entry = cJSON_GetArrayItem(range, i);
            if (range_entry->type != cJSON_String && range_entry->type != cJSON_Number) {
                FPS_ERROR_LOG("Error parsing asset %s: 'range' array must contain only numbers or strings.", asset_name->valuestring);
                return TEMPLATING_ERROR;
            }
        }
        ranged_template = true;
    }

    std::string name = asset_name->valuestring;
    std::string id = asset_id->valuestring;
    bool name_has_wildcard = name.find('#') != std::string::npos;
    bool id_has_wildcard = id.find('#') != std::string::npos;

    // ID having wildcard is the absolute sign that an asset entry must be a template
    if (id_has_wildcard) {
        // An asset entry being a template means the name must have a wildcard
        if (!name_has_wildcard) {
            FPS_ERROR_LOG("Error parsing asset with ID %s and name %s: templates must have wildcard character in asset name.", asset_id->valuestring, asset_name->valuestring);
            return TEMPLATING_ERROR;
        }
        // Both ID and name have wildcards and number_of_instances >= 1, so safe to return non-zero for entry being template
        asset_config.is_template_flag = true;
        return ranged_template ? RANGED : TRADITIONAL;
    }

    // If ID is missing wildcard, the entry may pass as a concrete asset instance as long as number_of_instances is exactly 1
    if (num_instances_obj->valueint != 1) {
        FPS_ERROR_LOG("Error parsing asset with ID %s and name %s: if 'number_of_instances' is greater than 1, the asset entry is considered templated and must have a wildcard character in the ID and name.", asset_id->valuestring,
                      asset_name->valuestring);
        return TEMPLATING_ERROR;
    }
    return NON_TEMPLATE;
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
 */
std::vector<int> Type_Configurator::generate_list_of_asset_instances_represented(cJSON* asset_array_entry) {
    std::vector<int> asset_instance_list;
    if (!asset_array_entry) {
        FPS_ERROR_LOG("Error parsing asset: asset_array_entry is NULL.");
        return std::vector<int>();
    }

    auto range = cJSON_GetObjectItem(asset_array_entry, "range");
    if (!range) {
        FPS_ERROR_LOG("Error parsing asset 'range' field not found.");
        return std::vector<int>();
    }

    for (int i = 0; i < cJSON_GetArraySize(range); ++i) {
        auto range_entry = cJSON_GetArrayItem(range, i);
        if (!range_entry) {
            FPS_ERROR_LOG("Error parsing asset 'range' array entry is NULL.");
            return std::vector<int>();
        }
        // String case (e.g. "1..3")
        if (range_entry->type == cJSON_String) {
            // Make sure there is only one ..
            std::string range_entry_str = range_entry->valuestring;
            if (range_entry_str.find("..") != range_entry_str.rfind("..")) {
                FPS_ERROR_LOG(
                    "Error parsing asset %s: 'range' array must contain only numbers or strings \
                        and strings must be in the format (\"x .. y\").",
                    cJSON_GetObjectItem(asset_array_entry, "name")->valuestring);
                return std::vector<int>();
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
                    FPS_ERROR_LOG(
                        "Error parsing asset %s: 'range' array must contain only numbers or strings \
                            and strings must be in the format (\"x .. y\").",
                        cJSON_GetObjectItem(asset_array_entry, "name")->valuestring);
                    return std::vector<int>();
                }
            }
        } else if (range_entry->type == cJSON_Number)  // Number case (e.g. 5)
        {
            asset_instance_list.push_back(range_entry->valueint);
        } else {
            FPS_ERROR_LOG(
                "Error parsing asset %s: 'range' array must contain only numbers or \
            strings.",
                cJSON_GetObjectItem(asset_array_entry, "name")->valuestring);
            return std::vector<int>();
        }
    }
    return asset_instance_list;
}

/**
 * @brief Checks to see if asset range provided for templating
 *
 * @param asset_array_entry: The asset array entry
 * @return int 0 if no range provided, 1 if range provided and valid
 */
int Type_Configurator::range_provided(cJSON* asset_array_entry) {
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
