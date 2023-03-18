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
#include <Configurator.h>
#include <Type_Manager.h>
#include <Asset.h>

// constructor initializes all pointers
Type_Configurator::Type_Configurator(Type_Manager* pMan, std::map <std::string, std::vector<Fims_Object*>>* pCVM, std::map <std::string, Fims_Object*>* pAVM, bool* pc)
{
    // Set internal pointers to passed-in pointers
    pManager = pMan;
    pCompVarMap = pCVM;
    pAssetVarMap = pAVM;
    pIsPrimaryController = pc;

    // config_validation should be true in default case
    config_validation = true;
}

// extracts asset type variables for this type and configures all asset instances of this type
bool Type_Configurator::create_assets(void)
{
    // The "asset_instances" array in assets.json is a mix of concrete asset instances and template-defined asset instances
    cJSON *assetInstanceArray = cJSON_GetObjectItem(assetTypeRoot, "asset_instances");
    if (assetInstanceArray == NULL)
    {
        FPS_ERROR_LOG("Type_Configurator::create_assets ~ Failed to find asset_instances in config.\n");
        return false;
    }

    // Count the number of asset instances, including multiple instances contained within one template
    int numInstances = count_num_asset_instances(assetInstanceArray);
    if (numInstances > MAX_NUM_ASSETS)
    {
        FPS_ERROR_LOG("\nType_Configurator::create_assets ~ Exceeded maximum number of asset instances, limit is %d, config file has %d\n",MAX_NUM_ASSETS,numInstances);
        return false;
    }

    // Proceed with configuration if there are asset instance objects found in assets.json
    // Else, print an error and kill configuration because if there are no asset instances, there should not have been an object for this asset type
    if (numInstances > 0)
    {
        int numArrayEntries = cJSON_GetArraySize(assetInstanceArray);
        // Iterate through the asset array to configure all instances represented by each entry
        for (int i = 0; i < numArrayEntries; ++i)
        {
            cJSON* arrayEntry = cJSON_GetArrayItem(assetInstanceArray, i);
            if (arrayEntry == NULL)
            {
                FPS_ERROR_LOG("Type_Configurator::create_assets ~ Asset array entry %d, Invalid or NULL entry\n", i);
                return false;
            }
            if (!configure_asset_array_entry(arrayEntry))
            {
                FPS_ERROR_LOG("Type_Configurator::create_assets ~ Failed to configure asset array entry %d, error in assets.json\n", i);
                return false;
            }
        }
    }
    else 
    {
        FPS_ERROR_LOG("Type_Configurator::create_assets ~ Error with asset instance count. Expected positive integer, got %d\n", numInstances);
        return false;
    }

    // Carry out any configuration actions unique to the derived asset type managers
    if(!pManager->configure_type_manager(this))
    {
        FPS_ERROR_LOG("Type_Configurator::create_assets ~ Error configuring asset type\n");
        return false;
    }

    // Give base class functions access to asset instance pointers
    pManager->configure_base_class_list();
    
    // Used for testing
    // pManager->print_alarm_fault_map(pAssetVarMap);
    
    return true;
}

// figures out if asset array entry is template representing multiple asset instances or just represents one asset instance
// calls individual config function multiple times or just once accordingly
bool Type_Configurator::configure_asset_array_entry(cJSON* arrayEntry)
{
    assetConfig.assetInstanceRoot = arrayEntry;
    int is_template = entry_is_template(arrayEntry);
    if (is_template == -1) {
        FPS_ERROR_LOG("Type_Configurator::configure_asset_array_entry ~ Error within entry_is_template()\n");
        return false;
    }
    else if(is_template == 1)
    {
        int num_instances_represented = extract_num_asset_instances_represented(assetConfig.assetInstanceRoot);
        if (num_instances_represented >= 1)
        {
            for(int j = 0; j < num_instances_represented; ++j)
            {
                assetConfig.template_index = j+1;
                if (!configure_single_asset())
                {
                    FPS_ERROR_LOG("Type_Configurator::configure_asset_array_entry ~ configure_single_asset() failure\n");
                    return false;
                }
            }
        }
        else
        {
            FPS_ERROR_LOG("Type_Configurator::configure_asset_array_entry ~ Mistake in configuration of number_of_instances for an asset array entry, expected positive integer\n");
            return false;
        }
    }
    else
    {
        if (!configure_single_asset())
        {
            FPS_ERROR_LOG("Type_Configurator::configure_asset_array_entry ~ configure_single_asset() failure\n");
            return false;
        }
    }
    return true;
}

// allocates memory for new asset, configures that individual asset, then adds new asset to the list of assets
bool Type_Configurator::configure_single_asset(void)
{
    Asset* asset = pManager->build_new_asset();
    if (asset == NULL)
    {
        FPS_ERROR_LOG("Type_Configurator::configure_single_asset ~ Error allocating memory for new asset instance.\n");
        return false;
    }

    if ( !asset->configure(this) )
    {
        FPS_ERROR_LOG("Type_Configurator::configure_single_asset ~ Failed to configure asset.\n");
        return false;
    }
    
    pManager->append_new_asset(asset);
    return true;
}

// Helper function for counting the number of asset instances of a given type in assets.json, including multiple instances contained within one template
int Type_Configurator::count_num_asset_instances(cJSON* asset_array)
{
    int num_asset_instances = 0;

    int asset_array_size = cJSON_GetArraySize(asset_array);
    if (asset_array_size > 0)
    {
        // Iterate through asset array to see how many asset instances each entry represents
        for (int i = 0; i < asset_array_size; ++i)
        {
            cJSON* entry = cJSON_GetArrayItem(asset_array, i);
            if (entry == NULL)
            {
                FPS_ERROR_LOG("Type_Configurator::asset_count_num_asset_instances ~ Asset array entry %d, Invalid or NULL entry\n", i+1);
                return -1;
            }
            int is_template = entry_is_template(entry);
            if (is_template == 1)
            {
                int num_instances_represented = extract_num_asset_instances_represented(entry);
                if (num_instances_represented >= 1)
                {
                    num_asset_instances += num_instances_represented;
                }
                else
                {
                    FPS_ERROR_LOG("Type_Configurator::asset_count_num_asset_instances ~ Asset array entry %d, Mistake in configuration of number_of_instances, expected positive integer\n", i+1);
                    return -1;
                }
            } else if (is_template == 0) {
                num_asset_instances += 1;
            }
            else
            {
                FPS_ERROR_LOG("Error within entry_is_template\n");
                return -1;
            }
        }
    }

    if (num_asset_instances == 0)
    {
        FPS_WARNING_LOG("Type_Configurator::count_num_asset_instances ~ Found no valid asset instance objects for this type in assets.json.\n");
    }

    return num_asset_instances;
}

// an asset array entry that is a template must contain the "number_of_instances" field to say how many instances the template represents
// if that field is not there, then array entry is not a template and is a concrete representation of a single asset instance
int Type_Configurator::entry_is_template(cJSON* asset_array_entry)
{
    // If number_of_instances field does not exist, this is a concrete entry and only represents one asset instance
    cJSON* num_instances_obj = cJSON_GetObjectItem(asset_array_entry, "number_of_instances");
    if (num_instances_obj == NULL) {
        return 0;
    }

    cJSON* asset_name = cJSON_GetObjectItem(asset_array_entry, "name");
    if (asset_name == NULL) {
        FPS_ERROR_LOG("Error parsing asset: 'name' field not found.\n");
        return -1;
    }

    cJSON* asset_id = cJSON_GetObjectItem(asset_array_entry, "id");
    if (asset_id == NULL) {
        FPS_ERROR_LOG("Error parsing asset: 'id' field not found.\n");
        return -1;
    }

    if (num_instances_obj->valueint < 1) {
        FPS_ERROR_LOG("Error parsing asset %s: 'number_of_instances' must be a positive integer.", asset_name->valuestring);
        return -1;
    }

    std::string name = asset_name->valuestring;
    std::string id = asset_id->valuestring;
    bool name_has_wildcard = name.find('#') != std::string::npos;
    bool id_has_wildcard = id.find('#') != std::string::npos;

    // ID having wildcard is the absolute sign that an asset entry must be a template
    if (id_has_wildcard) {
        // an asset entry being a template means the name must have a wildcard
        if (!name_has_wildcard) {
            FPS_ERROR_LOG("Error parsing asset with ID %s and name %s: templates must have wildcard character in asset name.\n", asset_id->valuestring, asset_name->valuestring);
            return -1;
        }
        // both ID and name have wildcards and number_of_instances >= 1, so safe to return 1 for entry being template
        return 1;
    }

    // if ID is missing wildcard, the entry may pass as a concrete asset instance as long as number_of_instances is exactly 1
    if (num_instances_obj->valueint != 1) {
        FPS_ERROR_LOG("Error parsing asset with ID %s and name %s: if 'number_of_instances' is greater than 1, the asset entry is considered templated and must have a wildcard character in the ID and name.\n", asset_id->valuestring, asset_name->valuestring);
        return -1;
    }
    return 0;
}

// returns the number of asset instances a single template represents
int Type_Configurator::extract_num_asset_instances_represented(cJSON* asset_array_entry)
{
    // If number_of_instances field does not exist, this is a concrete entry and only represents one asset instance
    cJSON* num_instances_obj = cJSON_GetObjectItem(asset_array_entry, "number_of_instances");
    if (num_instances_obj != NULL)
    {
        return num_instances_obj->valueint;
    }
    return -1;
}

// constructor for individual asset configurator class
Asset_Configurator::Asset_Configurator(void)
{
    template_index = 0;
    assetInstanceRoot = NULL;
}

// template_index will be 0 by default and set if an asset comes from a template, so if template_index is still 0 then it is not from a template
bool Asset_Configurator::is_template(void)
{
    return template_index > 0;
}
