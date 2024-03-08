/**
 * Configurator.h
 * Header for configuration object
 *
 * Created on March 31st, 2021
 *      Author: Jack Timothy (jtimothy)
 */

#ifndef CONFIGURATOR_H_
#define CONFIGURATOR_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
#include <map>
#include <set>
/* External Dependencies */
#include <cjson/cJSON.h>
/* System Internal Dependencies */
#include <Logger.h>
/* Local Internal Dependencies */
#include <Value_Object.h>
#include <Fims_Object.h>
#include <Types.h>
#include <Reference_Configs.h>
#include "Action.h"
class Type_Manager;

class Asset_Configurator {
public:
    Asset_Configurator(void);
    cJSON* asset_instance_root;
    bool is_template_flag;
    size_t asset_num;  // used to track the current instance being configured
    // The following Data structs are used during configuration to ensure that templating is correct
    // and that no one uses the same config twice.
    // The first map is a map of asset names to a map of asset numbers to a bool.
    // Each asset name contains another map containing all the asset numbers that have been used for that asset name.
    // As a result an asset with name ess_# can contain many instances etc.
    // The bool is used to indicate whether or not the asset number has been used.
    // The second set is a set of monikers already used. This is used to ensure that no two assets have the same moniker (name/id/comp_id).
    std::map<std::string, std::map<int, bool>> asset_name_to_asset_number_map;
    std::set<std::string> asset_name_collision_set;
    std::map<std::string, std::map<int, bool>> asset_id_to_asset_number_map;
    std::set<std::string> asset_id_collision_set;

    // use multi here to allow for duplicates
    std::map<std::string, std::multimap<int, bool>> asset_component_to_asset_number_map;
    std::multiset<std::string> asset_component_collision_set;
};

class Type_Configurator {
public:
    // functions
    Type_Configurator(Type_Manager* pMan, std::map<std::string, std::vector<Fims_Object*>>* pCVM, bool* pc);
    Config_Validation_Result create_assets();   // assets.json
    Config_Validation_Result create_actions();  // actions.json
    // variables
    cJSON* asset_type_root;
    cJSON* actions_root;
    Asset_Configurator asset_config;
    Type_Manager* p_manager;
    std::map<std::string, std::vector<Fims_Object*>>* pCompVarMap;
    bool* p_is_primary_controller;
    std::string current_asset_name;  // holds the id of the asset currently being configured
    std::string current_asset_id;    // holds the name of the asset currently being configured
    bool config_validation;          // flag to improve readability. removing config validation allows only necessary components to be configured in unit tests
    std::vector<Action> actions;     // vector of asset specific automated actions (actions.json)

private:
    // functions
    Config_Validation_Result check_name_and_id(int index, cJSON* array_entry);
    int extract_num_asset_instances_represented(cJSON* asset_array_entry);
    std::pair<std::vector<int>, Config_Validation_Result> generate_list_of_asset_instances_represented(cJSON* asset_array_entry);
    bool range_provided(cJSON* asset_array_entry);
    std::pair<int, Config_Validation_Result> count_num_asset_instances(cJSON* asset_array);
    Config_Validation_Result configure_asset_array_entry(int index, cJSON* array_entry);
    Config_Validation_Result configure_traditional_templated_asset();
    Config_Validation_Result configure_ranged_templated_asset();
    std::pair<int, Config_Validation_Result> gather_assets(cJSON* asset_array);
    int count_traditional_templated_assets(cJSON* entry);
    std::pair<int, Config_Validation_Result> count_ranged_templated_assets(cJSON* array_entry);
    std::pair<int, Config_Validation_Result> entry_is_template(cJSON* asset_array_entry);
    Config_Validation_Result configure_single_asset(void);
};

#endif /* CONFIGURATOR_H_ */
