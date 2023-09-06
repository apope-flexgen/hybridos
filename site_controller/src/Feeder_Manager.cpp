/**
 * Feeder_Manager.cpp
 * Source for Feeder-specific Manager class
 * Refactored from Asset_Manager.cpp
 *
 * Created on Sep 30th, 2020
 *      Author: Jack Shade (jnshade)
 */

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Feeder_Manager.h>
#include <Configurator.h>
#include <Site_Controller_Utils.h>

extern fims* p_fims;

/****************************************************************************************/
Feeder_Manager::Feeder_Manager() : Type_Manager(FEEDERS_TYPE_ID) {
    sync_frequency_offset = 0.0;
    pPointOfInterConnect = NULL;
    pSyncFeeder = NULL;
}

/****************************************************************************************/
Feeder_Manager::~Feeder_Manager() {
    if (!pFeeder.empty())
        for (size_t j = 0; j < pFeeder.size(); j++)
            if (pFeeder[j] != NULL)
                delete pFeeder[j];
}

bool Feeder_Manager::get_poi_feeder_state(void) {
    return pPointOfInterConnect->get_breaker_status();
}

bool Feeder_Manager::set_poi_feeder_state_open() {
    return pPointOfInterConnect->breaker_open();
}

bool Feeder_Manager::set_poi_feeder_state_closed() {
    return pPointOfInterConnect->breaker_close();
}

bool Feeder_Manager::get_poi_feeder_close_permissive_state(void) {
    return (pPointOfInterConnect->get_breaker_status());
}

float Feeder_Manager::get_poi_gridside_frequency(void) {
    return (pPointOfInterConnect->get_gridside_frequency());
}

float Feeder_Manager::get_poi_gridside_avg_voltage(void) {
    return (pPointOfInterConnect->get_voltage_avg_line_to_line());
}

float Feeder_Manager::get_poi_power_factor() {
    return pPointOfInterConnect->get_power_factor();
}

bool Feeder_Manager::get_sync_feeder_status(void) {
    if (pSyncFeeder != NULL) {
        FPS_DEBUG_LOG("Feeder_Manager::get_sync_feeder_state - get_feeder_status: %d\n", pSyncFeeder->get_breaker_status());
        return pSyncFeeder->get_breaker_status();
    }
    return false;
}

float Feeder_Manager::get_sync_feeder_gridside_frequency(void) {
    if (pSyncFeeder != NULL) {
        FPS_DEBUG_LOG("Feeder_Manager::get_sync_feeder_frequency - get_gridside_frequency: %f\n", pSyncFeeder->get_gridside_frequency());
        return pSyncFeeder->get_gridside_frequency();
    }
    return -1;
}

float Feeder_Manager::get_sync_frequency_offset(void) {
    if (pSyncFeeder != NULL) {
        FPS_DEBUG_LOG("Feeder_Manager::get_sync_feeder_frequency() get_sync_frequency_offset: %f\n", sync_frequency_offset);
        return sync_frequency_offset;
    }
    return -1;
}

float Feeder_Manager::get_sync_feeder_gridside_avg_voltage(void) {
    if (pSyncFeeder != NULL) {
        FPS_DEBUG_LOG("Feeder_Manager::get_sync_feeder_voltage - get_gridside_avg_voltage: %f\n", pSyncFeeder->get_gridside_avg_voltage());
        return pSyncFeeder->get_gridside_avg_voltage();
    }
    return -1;
}

bool Feeder_Manager::set_sync_feeder_close_permissive_remove() {
    if (pSyncFeeder != NULL) {
        // FPS_DEBUG_LOG("Feeder_Manager::set_sync_feeder_open_permissive\n");
        return pSyncFeeder->breaker_close_permissive_remove();
    }
    return false;
}

bool Feeder_Manager::set_sync_feeder_close_permissive() {
    if (pSyncFeeder != NULL) {
        FPS_DEBUG_LOG("Feeder_Manager::set_sync_feeder_close_permissive\n");
        return pSyncFeeder->breaker_close_permissive();
    }
    return false;
}

// NOTE: if the specified feeder_ID is not found in the feeder assets list, this function
// returns a nullptr instead of an Feeder asset
Asset_Feeder* Feeder_Manager::validate_feeder_id(const char* feeder_ID) {
    for (int i = 0; i < numParsed; i++) {
        if (pFeeder[i]->get_id() == feeder_ID)
            return pFeeder[i];
    }
    FPS_ERROR_LOG("Could not find specified feeder \n");
    return nullptr;
}

bool Feeder_Manager::get_feeder_state(Asset_Feeder* feeder) {
    return feeder->get_breaker_status();
}

/**
 * Status of the utility tracked by the feeder
 * @param feeder the validated feeder asset
 */
bool Feeder_Manager::get_utility_status(Asset_Feeder* feeder) {
    return feeder->get_utility_status();
}

float Feeder_Manager::get_feeder_active_power(const char* feeder_ID) {
    for (int i = 0; i < numParsed; i++) {
        if (pFeeder[i]->get_id() == feeder_ID)
            return pFeeder[i]->get_active_power();
    }
    return 0;
}

float Feeder_Manager::get_feeder_reactive_power(const char* feeder_ID) {
    for (int i = 0; i < numParsed; i++) {
        if (pFeeder[i]->get_id() == feeder_ID)
            return pFeeder[i]->get_reactive_power();
    }
    return 0;
}

float Feeder_Manager::get_feeder_nameplate_active_power(const char* feeder_ID) {
    for (int i = 0; i < numParsed; i++) {
        if (pFeeder[i]->get_id() == feeder_ID)
            return pFeeder[i]->get_rated_active_power();
    }
    return -1;  // exit failure
}

float Feeder_Manager::get_avg_ac_voltage(const char* feeder_ID) {
    for (int i = 0; i < numParsed; i++) {
        if (pFeeder[i]->get_id() == feeder_ID)
            return pFeeder[i]->get_voltage_avg_line_to_line();
    }
    return -1;  // failure exit
}

bool Feeder_Manager::set_feeder_state_open(Asset_Feeder* feeder) {
    return feeder->breaker_open();
}

bool Feeder_Manager::set_feeder_state_closed(Asset_Feeder* feeder) {
    return feeder->breaker_close();
}

void Feeder_Manager::set_poi_target_active_power(float desiredkW) {
    pPointOfInterConnect->set_active_power_setpoint(desiredkW);
}

bool Feeder_Manager::aggregate_feeder_data(void) {
    FPS_DEBUG_LOG("Processing Feeder data; Feeder_Manager::aggregate_feeder_data\n");

    // TODO: no behavior for now

    // uint64_t feedAggFaults = 0;

    return true;
}

void Feeder_Manager::generate_asset_type_summary_json(fmt::memory_buffer& buf, const char* const var) {
    if (var == NULL)
        bufJSON_StartObject(buf);  // summary {

    bufJSON_AddStringCheckVar(buf, "name", "Feeder Summary", var);
    if (pSyncFeeder != NULL) {
        bufJSON_AddBoolCheckVar(buf, "sync_feeder_status", pSyncFeeder->get_breaker_status(), var);
    }
    char temp_name[MEDIUM_MSG_LEN];
    for (auto it : pFeeder) {
        const char* feeder_id = it->get_id().c_str();
        snprintf(temp_name, MEDIUM_MSG_LEN, "%s_breaker_status", feeder_id);
        bufJSON_AddStringCheckVar(buf, temp_name, it->get_breaker_status() ? "Closed" : "Open", var);
        snprintf(temp_name, MEDIUM_MSG_LEN, "%s_active_power", feeder_id);
        bufJSON_AddNumberCheckVar(buf, temp_name, it->get_active_power(), var);
        if (it == pPointOfInterConnect) {
            snprintf(temp_name, MEDIUM_MSG_LEN, "%s_voltage", feeder_id);
            bufJSON_AddNumberCheckVar(buf, temp_name, it->get_gridside_avg_voltage(), var);
        }
        // Add a binary 1/0 if any alarms/faults are present for each asset
        snprintf(temp_name, MEDIUM_MSG_LEN, "%s_alarms", feeder_id);
        bufJSON_AddNumberCheckVar(buf, temp_name, get_num_active_alarms() > 0 ? 1 : 0, var);
        snprintf(temp_name, MEDIUM_MSG_LEN, "%s_faults", feeder_id);
        bufJSON_AddNumberCheckVar(buf, temp_name, get_num_active_faults() > 0 ? 1 : 0, var);
    }
    bufJSON_AddNumberCheckVar(buf, "feeder_num_alarmed", get_num_alarmed(), var);
    bufJSON_AddNumberCheckVar(buf, "feeder_num_faulted", get_num_faulted(), var);

    if (var == NULL)
        bufJSON_EndObjectNoComma(buf);  // } summary
}

// HybridOS Step 2: Process Asset Data
void Feeder_Manager::process_asset_data() {
    if (numParsed > 0) {
        for (int i = 0; i < numParsed; i++)
            pFeeder[i]->process_asset();
        aggregate_feeder_data();
    }
}

const std::string Feeder_Manager::get_poi_id(void) {
    return pPointOfInterConnect->get_id();
}

float Feeder_Manager::get_poi_max_potential_active_power(void) {
    return pPointOfInterConnect->get_max_potential_active_power();
}

float Feeder_Manager::get_poi_min_potential_active_power(void) {
    return pPointOfInterConnect->get_min_potential_active_power();
}

float Feeder_Manager::get_poi_rated_kW() {
    return pPointOfInterConnect->get_rated_active_power();
}

/****************************************************************************************/
/*
    Overriding configuration functions
*/
void Feeder_Manager::configure_base_class_list() {
    pAssets.assign(pFeeder.begin(), pFeeder.end());
}

Asset* Feeder_Manager::build_new_asset(void) {
    Asset_Feeder* asset = new Asset_Feeder();
    if (asset == NULL) {
        FPS_ERROR_LOG("Feeder %zu: Memory allocation error.", pFeeder.size() + 1);
    }
    numParsed++;
    return asset;
}

void Feeder_Manager::append_new_asset(Asset* asset) {
    pFeeder.push_back((Asset_Feeder*)asset);
}

// After configuring individual asset instances, this function finishes configuring the Feeder Manager
bool Feeder_Manager::configure_type_manager(Type_Configurator* configurator) {
    cJSON* feeder_root = configurator->asset_type_root;
    /************************* Identify POI feeder, which is required ******************************/
    cJSON* object = cJSON_HasObjectItem(feeder_root, "poi_feeder") ? cJSON_GetObjectItem(feeder_root, "poi_feeder") : NULL;
    if (object == NULL) {
        FPS_ERROR_LOG("Failed to find poi_feeder in config.");
        return false;
    } else {
        int poi_index = object->valueint;
        if (poi_index < 0 || uint(poi_index) >= pFeeder.size()) {
            FPS_ERROR_LOG("POI Feeder index %d is outside bounds of parsed feeders.", poi_index);
            return false;
        }
        pPointOfInterConnect = pFeeder[poi_index];
        // point configurator at poi feeder's asset instance json object for validation
        cJSON* feeder_instances_array = cJSON_GetObjectItem(configurator->asset_type_root, "asset_instances");
        configurator->asset_config.asset_instance_root = cJSON_GetArrayItem(feeder_instances_array, poi_index);
    }
    if (pPointOfInterConnect && !pPointOfInterConnect->validate_poi_feeder_configuration(configurator)) {
        FPS_ERROR_LOG("Validation of POI feeder configuration failed. Failing config.");
        return false;
    }

    /************************* asset type variables ******************************/
    object = cJSON_GetObjectItem(feeder_root, "sync_feeder");
    if (object != NULL) {
        int sync_feeder_index = object->valueint;
        if (sync_feeder_index < 0 || uint(sync_feeder_index) >= pFeeder.size()) {
            FPS_ERROR_LOG("Sync Feeder index %d is outside bounds of parsed feeders.", sync_feeder_index);
            return false;
        }
        pSyncFeeder = pFeeder[sync_feeder_index];
        FPS_DEBUG_LOG("sync_feeder index: %d\n", sync_feeder_index);

        // if sync_feeder is specified, a sync_frequency_offset must also be specified
        object = cJSON_GetObjectItem(feeder_root, "sync_frequency_offset");
        if (object == NULL) {
            FPS_ERROR_LOG("Failed to find sync_frequency_offset in config, but sync_feeder exists.");
            return false;
        }
        sync_frequency_offset = object->valuedouble;
        FPS_DEBUG_LOG("sync_frequency_offset: %f\n", sync_frequency_offset);
    }

    return true;
}
/****************************************************************************************/
