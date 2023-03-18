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
/* External Dependencies */
#include <cjson/cJSON.h>
/* System Internal Dependencies */
#include <Logger.h>
/* Local Internal Dependencies */
#include <Value_Object.h>
#include <Fims_Object.h>
#include <Types.h>
class Type_Manager;

class Asset_Configurator {
public:
    Asset_Configurator(void);
    bool is_template(void);
    cJSON* assetInstanceRoot;
    int template_index;
};

class Type_Configurator {
public:
    // functions
    Type_Configurator(Type_Manager* pMan, std::map <std::string, std::vector<Fims_Object*>>* pCVM, std::map <std::string, Fims_Object*>* pAVM, bool* pc);
    bool create_assets(void);
    // variables
    cJSON* assetTypeRoot;
    Asset_Configurator assetConfig;
    Type_Manager* pManager;
    std::map <std::string, Fims_Object*>* pAssetVarMap;
    std::map <std::string, std::vector<Fims_Object*>>* pCompVarMap;
    bool* pIsPrimaryController;
    bool config_validation; // flag to improve readability. removing config validation allows only necessary components to be configured in unit tests
private:
    // functions
    int extract_num_asset_instances_represented(cJSON* asset_array_entry);
    int count_num_asset_instances(cJSON* asset_array);
    bool configure_asset_array_entry(cJSON* arrEntry);
    int entry_is_template(cJSON* asset_array_entry);
    bool configure_single_asset(void);
};

#endif /* CONFIGURATOR_H_ */