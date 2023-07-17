/*
 * Asset_Manager.cpp
 *
 *  Created on: Sep 4, 2018
 *      Author: ghoward
 */

/* C Standard Library Dependencies */
#include <cstring>
/* C++ Standard Library Dependencies */
#include <iterator>
#include <map>
#include <vector>
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
/* Local Internal Dependencies */
#include <Asset_Manager.h>
#include <Site_Controller_Utils.h>
#include <Configurator.h>

extern fims *p_fims;

/****************************************************************************************/
Asset_Manager::Asset_Manager ()
{
    ess_manager = new ESS_Manager();
    feeder_manager = new Feeder_Manager();
    generator_manager = new Generator_Manager();
    solar_manager = new Solar_Manager();

    debugLoopCount = 0;

    send_FIMS_buf = fmt::memory_buffer();
}

/****************************************************************************************/
Asset_Manager::~Asset_Manager ()
{
    delete ess_manager;
    delete feeder_manager;
    delete generator_manager;
    delete solar_manager;

    delete ess_configurator;
    delete feeder_configurator;
    delete generator_configurator;
    delete solar_configurator;

    // Clear maps
    for (auto asset_it = asset_var_map.begin(); asset_it != asset_var_map.end(); ++asset_it)
        if (asset_it->second)
        {
            delete asset_it->second;
            // Assign NULL so pointer in other containers doesn't delete
            asset_it->second = NULL;
        }

    for (auto component_it = component_var_map.begin(); component_it != component_var_map.end(); ++component_it)
    {
        // Iterate through list of Fims_Objects to which this component URI acts as a source. Delete them
        for (auto var_it = component_it->second.begin(); var_it != component_it->second.end(); ++var_it)
        {
            if (*var_it)
            {
                delete *var_it;
                // Assign NULL so pointer in other containers does not delete
                *var_it = NULL;
            }
        }
    }
}

/****************************************************************************************/
void Asset_Manager::fims_data_parse(fims_message *pmsg)
{
    if (strcmp(pmsg->method, "pub") == 0)
    {
        FPS_DEBUG_LOG("\n\n***HybridOS Step 1: Receive Component Data.\nIn Asset_Manager::fims_data_parse\n");
        Asset_Manager::handle_pubs(pmsg->pfrags, pmsg->nfrags, pmsg->body);
    }
    else if (strcmp(pmsg->method, "get") == 0)
    {
        Asset_Manager::handle_get(pmsg);
    }
    else if (strcmp(pmsg->method, "set") == 0)
    {
        Asset_Manager::handle_set(*pmsg);
    }
    else if (strcmp(pmsg->method, "post") == 0)
    {
        Asset_Manager::handle_post(pmsg->nfrags, pmsg->body);
    }
    else if (strcmp(pmsg->method, "del") == 0)
    {
        Asset_Manager::handle_del(pmsg->nfrags, pmsg->body);
    }
}

/* this function accepts publishes from components, updates internal variables, and replublishes
   updated asset to 'assets', for use by UI */
/****************************************************************************************/
void Asset_Manager::handle_pubs(char** pfrags, int nfrags, char* body)
{
    if (nfrags >= 2 && strncmp(pfrags[0], "components", strlen("components")) == 0)
    {
        cJSON* bodyObject = cJSON_Parse(body);
        if (bodyObject == NULL)
        {
            FPS_ERROR_LOG("Asset_Manager::handle_pubs NULL parsed data updateAsset(), %s\n", pfrags[0]);
            return;
        }

        // Use manual for loop - ArrayForEach may be deprecated in future cJSON releases
        for (cJSON* cur = bodyObject->child; cur != NULL; cur=cur->next)
        {
            // Construct the uri
            std::string uri = "/" + std::string(pfrags[0]) + "/" + std::string(cur->string);
            // Find the component in the map
            auto comp_it = component_var_map.find(uri);
            if (comp_it != component_var_map.end())
            {
                // component variable map gives list of any Fims_Objects that get their values from this component URI, so iterate through each one of them
                for (auto var_it = comp_it->second.begin(); var_it != comp_it->second.end(); ++var_it)
                {
                    if (*var_it != NULL)
                    {
                        Fims_Object* current = *var_it;
                        int varArraySize = cJSON_GetArraySize(cur);
                        // Received at least one name value pair in our options value array from the component publish
                        if (varArraySize > 0)
                        {
                            // Initialize the vectors
                            if (current->options_name.empty())
                                current->options_name.resize(MAX_STATUS_BITS);
                            if (current->options_value.empty())
                                current->options_value.resize(MAX_STATUS_BITS);

                            // Status requires unique handling of the status bit strings
                            if (strcmp(current->get_type(), "Status") == 0)
                            {
                                handle_pub_status_options(cur, current, varArraySize);
                            }
                            else if (strcmp(current->get_ui_type(), "alarm") == 0 || strcmp(current->get_ui_type(), "fault") == 0)
                            {
                                handle_pub_alarm_or_fault_options(cur, current, varArraySize);
                            }
                            else
                            {   
                                // Not status case but we have at least one options name value pair received from components
                                handle_pub_other_options(cur, current, varArraySize);
                            }
                        }
                        // No options array, only the value received from component publish
                        else
                        {
                            // Alarm set false from components -> empty options array
                            // No need to handle fault case. Faults are latching and would therefore keep their current bitfield value
                            if (strcmp(current->get_ui_type(), "alarm") == 0)
                            {
                                // Alarms nonlatching
                                // Set the value directly, do not change the data type
                                current->value.value_bit_field = uint64_t(0);
                            }
                            else if (current->get_type() && strcmp(current->get_type(), "Int") == 0)
                            {
                                current->set_fims_int(current->get_variable_id(), cur->valueint);
                            }
                            else if (current->get_type() && strcmp(current->get_type(), "Float") == 0)
                            {
                                current->set_fims_float(current->get_variable_id(), cur->valuedouble);
                            }
                            else if (current->get_type() && strcmp(current->get_type(), "Bool") == 0)
                            {
                                bool bool_value = ((cur->type == cJSON_True) || ((cur->type == cJSON_Number) && (cur->valueint == 1)));
                                current->set_fims_bool(current->get_variable_id(), bool_value);
                            }
                        }
                    }
                }
            }            
        }
        // Also frees any cJSON items retreived from bodyObject
        cJSON_Delete(bodyObject);
        return;
    }
}

/**
 * Helper function used by handle_pubs(), handles options in the case of a status pub
 * @param cJcomp        The published cJSON data for a component
 * @param fimsComp      A component in the component_var_map that gets its values from the component URI
 * @param varArraySize  Size of the options value array
 */
void Asset_Manager::handle_pub_status_options(cJSON* cJcomp, Fims_Object* fimsComp, int varArraySize) {
    // Iterate through the parsed options value array received from component publish
    for (int i = 0; i < varArraySize; i++)
    {
        // Get the current item and check its validity
        cJSON* cJitem = cJSON_GetArrayItem(cJcomp, i);
        if (cJitem == NULL)
        {
            FPS_ERROR_LOG("Asset_Manager::process_pub NULL cJitem\n");
            return;
        }

        // Parsed items from the pub
        cJSON* cJitemValue = cJSON_GetObjectItem(cJitem, "value");
        cJSON* cJitemString = cJSON_GetObjectItem(cJitem, "string");

        if (cJitemValue == NULL || cJitemString == NULL || cJitemString->valuestring == NULL)
        {
            FPS_ERROR_LOG("Asset_Manager::process_pub NULL cJitemValue\n");
            return;
        }

        // Ensure the status value received is valid
        // As the value represents the position in the bit field and we use type uint64_t to represent it,
        // the position can be 63 at most representing a value of 2^63
        if (cJitemValue->valueint < MAX_STATUS_BITS)
        {
            if (fimsComp->options_name[cJitemValue->valueint].empty())
            {
                fimsComp->num_options++;
                fimsComp->options_name[cJitemValue->valueint] = cJitemString->valuestring;
                Value_Object new_val = Value_Object();
                new_val.type = Bit_Field;
                new_val.set((uint64_t) cJitemValue->valueint);
                fimsComp->options_value[cJitemValue->valueint] = new_val;
            }
            // In the case of multiple statuses this will always be set to the last status value which may be incorrect
            // However, it is assumed that the system can only be in one state at a time
            // Otherwise we could do the same aggregation as is done with alarms/faults
            fimsComp->value.set((uint64_t) cJitemValue->valueint);
        }
        else
            FPS_ERROR_LOG("Asset_Manager::process_pub Index into component status string array out of bounds, %d, updateAsset()\n", cJitemValue->valueint);
    }

    // Find the matching Fims_Objects in the assets map to update Asset-level variables (only if ID name = "status")
    if (strcmp(fimsComp->get_variable_id(), "status") != 0) {
        return;
    }
    auto asset_it = asset_var_map.begin();
    for (asset_it = asset_it; asset_it != asset_var_map.end(); asset_it++) {
        if (asset_it->second == fimsComp)
        {
            if (asset_it->first.find("/assets/ess/") < asset_it->first.size())
                ess_manager->process_pub(asset_it->first, &fimsComp->options_name, fimsComp->value.value_bit_field);
            else if (asset_it->first.find("/assets/feeders/") < asset_it->first.size())
                feeder_manager->process_pub(asset_it->first, &fimsComp->options_name, fimsComp->value.value_bit_field);
            else if (asset_it->first.find("/assets/generators/") < asset_it->first.size())
                generator_manager->process_pub(asset_it->first, &fimsComp->options_name, fimsComp->value.value_bit_field);
            else if (asset_it->first.find("/assets/solar/") < asset_it->first.size())
                solar_manager->process_pub(asset_it->first, &fimsComp->options_name, fimsComp->value.value_bit_field);
        }
    }
}

/**
 * Helper function used by handle_pubs(), handles options in the case of an alarm or fault pub
 * @param cJcomp        The published cJSON data for a component
 * @param fimsComp      A component in the component_var_map that gets its values from the component URI
 * @param varArraySize  Size of the options value array
 */
void Asset_Manager::handle_pub_alarm_or_fault_options(cJSON* cJcomp, Fims_Object* fimsComp, int varArraySize) {
    // OR Aggregate of all values in the object 
    uint64_t bit_field_agg = 0;
    for (int i = 0; i < varArraySize; i++)
    {
        cJSON* cJitem = cJSON_GetArrayItem(cJcomp, i);
        if (cJitem == NULL)
        {
            FPS_ERROR_LOG("Asset_Manager::process_pub NULL cJitem\n");
            return;
        }

        // Parsed items from the pub
        cJSON* cJitemValue = cJSON_GetObjectItem(cJitem, "value");
        cJSON* cJitemString = cJSON_GetObjectItem(cJitem, "string");

        if (cJitemValue == NULL || cJitemString == NULL || cJitemString->valuestring == NULL)
        {
            FPS_ERROR_LOG("Asset_Manager::process_pub NULL cJitemValue\n");
            return;
        }

        if (cJitemValue->valueint < MAX_STATUS_BITS)
        {
            // Add the bit of the current position
            bit_field_agg |= uint64_t(1) << cJitemValue->valueint;

            // Add the new options names and values
            if (fimsComp->options_name[cJitemValue->valueint].empty())
            {
                fimsComp->num_options++;
                fimsComp->options_name[cJitemValue->valueint] = cJitemString->valuestring;
                Value_Object new_val = Value_Object();
                new_val.set((uint64_t) cJitemValue->valueint);
                fimsComp->options_value[cJitemValue->valueint] = new_val;
            }
        }
        else
            FPS_ERROR_LOG("Asset_Manager::process_pub Index into component status string array out of bounds, %d, updateAsset()\n", cJitemValue->valueint);
    }

    // All value options parsed
    // And with the alarm's mask
    uint64_t masked_value = fimsComp->value.value_mask & bit_field_agg;
    fimsComp->value.type = bit_field;
    if (strcmp(fimsComp->get_ui_type(), "alarm") == 0)
    {
        // Alarms nonlatching
        fimsComp->value.value_bit_field = masked_value;
    }
    else
        // Faults latching
        // Or with the current value to latch (maintain high bits)
        fimsComp->value.value_bit_field |= masked_value;

    // No need for Type_Managers->Asset_instance to process pub
    // Alarm values updated in process_asset() step for each Asset
}

/**
 * Helper function used by handle_pubs(), handles options in the case a pub that has options but is not a status, alarm, or fault
 * @param cJcomp        The published cJSON data for a component
 * @param fimsComp      A component in the component_var_map that gets its values from the component URI
 * @param varArraySize  Size of the options value array
 */
void Asset_Manager::handle_pub_other_options(cJSON* cJcomp, Fims_Object* fimsComp, int varArraySize) {
    // Not status case but we have at least one options name value pair received from components
    // Iterate through the received options value array
    for (int i = 0; i < varArraySize; i++)
    {
        // Get the current item and check its validity
        cJSON* cJitem = cJSON_GetArrayItem(cJcomp, i);
        if (cJitem == NULL)
        {
            FPS_ERROR_LOG("Asset_Manager::process_pub NULL cJitem\n");
            return;
        }

        // Parsed items from pub
        cJSON* cJitemValue = cJSON_GetObjectItem(cJitem, "value");
        cJSON* cJitemString = cJSON_GetObjectItem(cJitem, "string");

        if (cJitemValue == NULL || cJitemString == NULL || cJitemString->valuestring == NULL)
        {
            FPS_ERROR_LOG("Asset_Manager::process_pub NULL cJitemValue\n");
            return;
        }
        fimsComp->num_options = varArraySize;
        fimsComp->options_name[i] = cJitemString->valuestring;

        if (fimsComp->get_type() && strcmp(fimsComp->get_type(), "Int") == 0)
        {
            fimsComp->options_value[i].set(cJitemValue->valueint);
            fimsComp->set_fims_int(fimsComp->get_variable_id(), cJitemValue->valueint);
        }
        else if (fimsComp->get_type() && strcmp(fimsComp->get_type(), "Float") == 0)
        {
            fimsComp->options_value[i].set((float) cJitemValue->valuedouble);
            fimsComp->set_fims_float(fimsComp->get_variable_id(), cJcomp->valuedouble);
        }
        else if (fimsComp->get_type() && strcmp(fimsComp->get_type(), "Bool") == 0)
        {
            bool bool_value = ((cJcomp->type == cJSON_True) || ((cJcomp->type == cJSON_Number) && (cJitemValue->valueint == 1)));
            fimsComp->options_value[i].set(bool_value);
            fimsComp->set_fims_bool(fimsComp->get_variable_id(), bool_value);
        }
    }
}

/**
 * @brief Handles GETs to URIs beginning with /assets.
 * @param pmsg Pointer to FIMS GET message.
*/
void Asset_Manager::handle_get(fims_message *pmsg)
{
    // error checking
    if ((strncmp(pmsg->pfrags[0], "assets", strlen("assets")) != 0 || pmsg->replyto == NULL) || !*is_primary)
        return;
    if (pmsg->nfrags < 1 || pmsg->nfrags > 4) {
        p_fims->Send("set", pmsg->replyto, NULL, "Invalid number of URI fragments.");
        return;
    }
    
    // clear buffer for use
    send_FIMS_buf.clear();

    // URI is /assets. return an object with data from all asset types
    if (pmsg->nfrags == 1) {
        send_all_asset_data(pmsg->replyto);
        return;
    }

    // URI starts with /assets/<asset type>. determine which Type Manager should handle the GET
    Type_Manager* manager = get_type_manager(pmsg->pfrags[1]);
    if (manager == NULL) {
        p_fims->Send("set", pmsg->replyto, NULL, "Invalid second URI fragment.");
        return;
    }

    // have the appropriate Type Manager handle the GET.
    // the ESS page is currently the only Assets page on the UI that handles naked values
    if (!manager->handle_get(pmsg, &asset_var_map)) {
        p_fims->Send("set", pmsg->replyto, NULL, "Error building response. Check site_controller logs.");
        return;
    }
}

/**
 * @brief Handles SETs to URIs beginning with /assets.
 * @param msg FIMS SET message.
*/
void Asset_Manager::handle_set(fims_message &msg)
{
    // if SET is to something other than "assets" (example: "components"), do not process it here
    if (strncmp(msg.pfrags[0], "assets", strlen("assets")) != 0)
        return;
    FPS_DEBUG_LOG("Received SET to %s.", msg.uri);

    // only SETs to specific variables are supported so expected URI format is /assets/{asset type}/{instance ID}/{variable ID}
    if (msg.nfrags != 4) {
        FPS_ERROR_LOG("Received SET to %s, but URIs beginning with /assets are expected to have exactly 4 fragments.", msg.uri);
        if (msg.replyto != NULL)
            p_fims->Send("set", msg.replyto, NULL, "Invalid URI");
        return;
    }

    // URI starts with /assets/<asset type>. determine which Type Manager should handle the SET
    Type_Manager* manager = get_type_manager(msg.pfrags[1]);
    if (manager == NULL) {
        FPS_ERROR_LOG("Invalid asset type '%s' in SET to URI %s.", msg.pfrags[1], msg.uri);
        if (msg.replyto != NULL)
            p_fims->Send("set", msg.replyto, NULL, "Invalid Asset Type");
        return;
    }

    // have Type Manager handle the SET
    manager->handle_set(msg);
}

void Asset_Manager::handle_post(int nfrags, char* body)
{
    FPS_DEBUG_LOG("handled post: %s frags: %d\n", body, nfrags);

    (void) nfrags;
    (void) body;
}

void Asset_Manager::handle_del(int nfrags, char* body)
{
    FPS_DEBUG_LOG("handled del: %s frags: %d\n", body, nfrags);

    (void) nfrags;
    (void) body;
}

/**
 * @brief Sends a JSON object containing all asset data to the specified URI.
 *        If there is a problem building the object, sends an error message to the URI.
 * @param uri The URI to which the asset data is to be sent.
*/
void Asset_Manager::send_all_asset_data(char* uri)
{
    // begin asset data with opening curly brace
    bufJSON_StartObject(send_FIMS_buf);
    // add data for each asset type in the form: <asset type ID>: { <asset type data }
    Type_Manager* managers[NUM_TYPE_MANAGERS] = {ess_manager, feeder_manager, generator_manager, solar_manager};
    for(size_t i = 0; i < NUM_TYPE_MANAGERS; ++i) {
        // add asset type ID (i.e. "ess", "solar", etc.) with colon
        bufJSON_AddId(send_FIMS_buf, managers[i]->get_asset_type_id());
        // add asset type data surrounded by braces
        if (!managers[i]->add_type_data_to_buffer(send_FIMS_buf, &asset_var_map)) {
            p_fims->Send("set", uri, NULL, "Error adding data to response.");
            return;
        }
        // separate each asset type ID-data pair with a comma, but avoid leaving a trailing comma at the end
        if (i != NUM_TYPE_MANAGERS - 1) {
            bufJSON_AddComma(send_FIMS_buf);
        }
    }
    // end asset data with closing curly brace
    bufJSON_EndObjectNoComma(send_FIMS_buf);
    // send the object
    std::string body = to_string(send_FIMS_buf);
    p_fims->Send("set", uri, NULL, body.c_str());
    return;
}

/****************************************************************************************/
/*
    This function, asset_create, is called from site_controller.cpp during initial
    configuration. It is passed a pointer to the cJSON object parsed from assets.json.
    The function breaks the large assets object up into its constituent asset type
    cJSON objects: generators, feeders, ess, solar. It passes these cJSON objects to 
    the Type Managers which continue the configuration process.
    
    @param primary_controller pointer to the bool indicating whether the system is currently
                              the primary controller. This pointer will be passed to Site_Manager
                              and down to the Asset Instances, and can be modified through the
                              fims endpoint /site/operation/primary_controller true/false
                              Testing will now require this pointer to be set.
*/
bool Asset_Manager::asset_create(cJSON *pJsonRoot, bool* primary_controller)
{
    is_primary = primary_controller;

    if (!build_configurators())
    {
        FPS_ERROR_LOG("Asset_Manager::asset_create ~ Error when allocating memory for Type Configurators. Failing configuration.\n");
        return false;
    }

    // Extract the large "assets" cJSON object
    cJSON *jsonAssets = cJSON_GetObjectItem(pJsonRoot, "assets");
    if (jsonAssets == NULL)
    {
        FPS_ERROR_LOG("Failed to get object item 'assets' in JSON.\n");
        return false;
    }

    // Extract the "generators" cJSON object and pass it to Generator Manager for configuration
    generator_configurator->assetTypeRoot = cJSON_GetObjectItem(jsonAssets, "generators");
    if ( generator_configurator->assetTypeRoot != NULL && !generator_configurator->create_assets() )
    {
        FPS_ERROR_LOG("Asset_Manager::asset_create ~ Failed to configure generators.\n");
        return false;
    }

    // Extract the "feeders" cJSON object and pass it to Feeder Manager for configuration
    feeder_configurator->assetTypeRoot = cJSON_GetObjectItem(jsonAssets, "feeders");
    if ( feeder_configurator->assetTypeRoot != NULL && !feeder_configurator->create_assets() )
    {
        FPS_ERROR_LOG("Asset_Manager::asset_create ~ Failed to configure feeders.\n");
        return false;
    }

    // Extract the "ess" cJSON object and pass it to ESS Manager for configuration
    ess_configurator->assetTypeRoot = cJSON_GetObjectItem(jsonAssets, "ess");
    if ( ess_configurator->assetTypeRoot != NULL && !ess_configurator->create_assets() )
    {
        FPS_ERROR_LOG("Asset_Manager::asset_create ~ Failed to configure ESSs.\n");
        return false;
    }

    // Extract the "solar" cJSON object and pass it to Solar Manager for configuration
    solar_configurator->assetTypeRoot = cJSON_GetObjectItem(jsonAssets, "solar");
    if ( solar_configurator->assetTypeRoot != NULL && !solar_configurator->create_assets() )
    {
        FPS_ERROR_LOG("Asset_Manager::asset_create ~ Failed to configure solar.\n");
        return false;
    }

    // Print variable maps to terminal (optional)
    // print_component_var_map();
    // print_asset_var_map();

    #ifndef FPS_TEST_MODE
    emit_event("Assets", "Asset Manager Initialized", 1);
    #endif
    return true;
}

bool Asset_Manager::build_configurators(void)
{
    ess_configurator = new Type_Configurator(ess_manager, &component_var_map, &asset_var_map, is_primary);
    feeder_configurator = new Type_Configurator(feeder_manager, &component_var_map, &asset_var_map, is_primary);
    generator_configurator = new Type_Configurator(generator_manager, &component_var_map, &asset_var_map, is_primary);
    solar_configurator = new Type_Configurator(solar_manager, &component_var_map, &asset_var_map, is_primary);

    if (!ess_configurator || !feeder_configurator || !generator_configurator || !solar_configurator)
    {
        FPS_ERROR_LOG("Asset_Manager::build_configurators ~ Error when allocating memory for Type Configurators. Failing configuration.\n");
        return false;
    }
    return true;
}

int Asset_Manager::get_num_ess_avail(void)
{
    return ess_manager->get_num_avail();
}

int Asset_Manager::get_num_solar_avail(void)
{
    return solar_manager->get_num_avail();
}

int Asset_Manager::get_num_gen_avail(void)
{
    return generator_manager->get_num_avail();
}

int Asset_Manager::get_num_feeder_avail(void)
{
    return feeder_manager->get_num_avail();
}

int Asset_Manager::get_num_ess_parsed(void)
{
    return ess_manager->get_num_parsed();
}

int Asset_Manager::get_num_solar_parsed(void)
{
    return solar_manager->get_num_parsed();
}

int Asset_Manager::get_num_gen_parsed(void)
{
    return generator_manager->get_num_parsed();
}

int Asset_Manager::get_num_feeder_parsed(void)
{
    return feeder_manager->get_num_parsed();
}

int Asset_Manager::get_num_ess_running(void)
{
   return ess_manager->get_num_running();
}

int Asset_Manager::get_num_solar_running(void)  // TODO manage returned values 
{
   return solar_manager->get_num_running();
}

int Asset_Manager::get_num_gen_running(void)
{
   return generator_manager->get_num_running();
}

int Asset_Manager::get_num_feeder_running(void)
{
    return feeder_manager->get_num_running();
}

int Asset_Manager::get_num_ess_startable(void)
{
    return ess_manager->get_num_ess_startable();
}

int Asset_Manager::get_num_solar_startable(void)
{
    return solar_manager->get_num_solar_startable();
}

int Asset_Manager::get_num_ess_in_standby(void)
{
    return ess_manager->get_num_ess_in_standby();
}

int Asset_Manager::get_num_solar_in_standby(void)
{
    return solar_manager->get_num_solar_in_standby();
}

int Asset_Manager::get_num_gen_controllable(void)
{
    return generator_manager->get_num_gen_controllable();
}

int Asset_Manager::get_num_ess_controllable(void)
{
    return ess_manager->get_num_ess_controllable();
}

int Asset_Manager::get_num_solar_controllable(void)
{
    return solar_manager->get_num_solar_controllable();
}

bool Asset_Manager::get_poi_feeder_state(void)
{
    return feeder_manager->get_poi_feeder_state();
}

bool Asset_Manager::get_poi_feeder_close_permissive_state(void)
{
    return feeder_manager->get_poi_feeder_close_permissive_state();
}

float Asset_Manager::get_poi_gridside_frequency(void)
{
    return feeder_manager->get_poi_gridside_frequency();
}

float Asset_Manager::get_poi_gridside_avg_voltage(void)
{
    return feeder_manager->get_poi_gridside_avg_voltage();
}

float Asset_Manager::get_poi_power_factor()
{
    return feeder_manager->get_poi_power_factor();
}

bool Asset_Manager::get_sync_feeder_status(void)
{
    return feeder_manager->get_sync_feeder_status();
}

float Asset_Manager::get_sync_feeder_gridside_frequency(void)
{
    return feeder_manager->get_sync_feeder_gridside_frequency();
}

float Asset_Manager::get_sync_frequency_offset(void)
{
    return feeder_manager->get_sync_frequency_offset();
}

float Asset_Manager::get_sync_feeder_gridside_avg_voltage(void)
{
    return feeder_manager->get_sync_feeder_gridside_avg_voltage();
}

bool Asset_Manager::set_sync_feeder_close_permissive_remove()
{
    return feeder_manager->set_sync_feeder_close_permissive_remove();
}

bool Asset_Manager::set_sync_feeder_close_permissive()
{
    return feeder_manager->set_sync_feeder_close_permissive();
}

float Asset_Manager::get_ess_total_active_power(void)
{
   return ess_manager->get_ess_total_active_power();
}

float Asset_Manager::get_solar_total_active_power(void)
{
   return solar_manager->get_solar_total_active_power(); 
}

float Asset_Manager::get_gen_total_active_power(void)
{
    return (generator_manager->get_gen_total_active_power());
}

float Asset_Manager::get_ess_total_reactive_power(void)
{
    return (ess_manager->get_ess_total_reactive_power());
}

float Asset_Manager::get_solar_total_reactive_power(void)
{
   return solar_manager->get_solar_total_reactive_power(); 
}

float Asset_Manager::get_gen_total_reactive_power(void)
{
   return generator_manager->get_gen_total_reactive_power();
}

float Asset_Manager::get_ess_total_uncontrollable_active_power(void)
{
    return ess_manager->get_ess_total_uncontrollable_active_power();
}

float Asset_Manager::get_solar_total_uncontrollable_active_power(void)
{
    return solar_manager->get_solar_total_uncontrollable_active_power();
}

float Asset_Manager::get_gen_total_uncontrollable_active_power(void)
{
    return generator_manager->get_gen_total_uncontrollable_active_power();
}

bool Asset_Manager::get_feeder_state(Asset_Feeder *found_feeder)
{
    return feeder_manager->get_feeder_state(found_feeder);
}
Asset_Feeder* Asset_Manager::validate_feeder_id(const char* feeder_ID) {
    return feeder_manager->validate_feeder_id(feeder_ID);
}

float Asset_Manager::get_gen_total_max_potential_active_power(void)
{
    return generator_manager->get_gen_total_max_potential_active_power();
}

float Asset_Manager::get_gen_total_min_potential_active_power(void)
{
    return generator_manager->get_gen_total_min_potential_active_power();
}

float Asset_Manager::get_gen_total_potential_reactive_power(void)
{
    return generator_manager->get_gen_total_potential_reactive_power();
}

float Asset_Manager::get_feeder_active_power(const char* feeder_ID)
{
    return feeder_manager->get_feeder_active_power(feeder_ID);
}

float Asset_Manager::get_feeder_reactive_power(const char* feeder_ID)
{
    return feeder_manager->get_feeder_reactive_power(feeder_ID);
}

float Asset_Manager::get_ess_total_max_potential_active_power(void)
{
    return ess_manager->get_ess_total_max_potential_active_power();
}

float Asset_Manager::get_ess_total_min_potential_active_power(void)
{
    return ess_manager->get_ess_total_min_potential_active_power();
}

float Asset_Manager::get_ess_total_potential_reactive_power(void)
{
    return ess_manager->get_ess_total_potential_reactive_power();
}

float Asset_Manager::get_ess_total_rated_active_power(void)
{
    return ess_manager->get_ess_total_rated_active_power();
}

float Asset_Manager::get_ess_total_rated_reactive_power(void)
{
    return ess_manager->get_ess_total_rated_reactive_power();
}

float Asset_Manager::get_ess_total_rated_apparent_power(void)
{
    return ess_manager->get_ess_total_rated_apparent_power();
}

float Asset_Manager::get_ess_total_kW_charge_limit(void)
{
    return ess_manager->get_total_kW_charge_limit();
}

float Asset_Manager::get_ess_total_kW_discharge_limit(void)
{
    return ess_manager->get_total_kW_discharge_limit();
}

float Asset_Manager::get_ess_total_chargeable_power_kW(void)
{
    return ess_manager->get_total_chargeable_power_kW();
}

float Asset_Manager::get_ess_total_dischargeable_power_kW(void)
{
    return ess_manager->get_total_dischargeable_power_kW();
}

float Asset_Manager::get_ess_total_chargeable_energy_kWh(void)
{
    return ess_manager->get_total_chargeable_energy_kWh();
}

float Asset_Manager::get_ess_total_dischargeable_energy_kWh(void)
{
    return ess_manager->get_total_dischargeable_energy_kWh();
}

std::vector<int> Asset_Manager::get_ess_setpoint_statuses(void)
{
    return ess_manager->get_setpoint_statuses();
}

float Asset_Manager::get_ess_soc_max(void)
{
    return ess_manager->get_ess_soc_max();
}

float Asset_Manager::get_ess_soc_min(void)
{
    return ess_manager->get_ess_soc_min();
}

float Asset_Manager::get_ess_soc_avg(void)
{
    return ess_manager->get_ess_soc_avg();
}

float Asset_Manager::get_all_ess_soc_max(void)
{
    return ess_manager->get_all_ess_soc_max();
}

float Asset_Manager::get_all_ess_soc_min(void)
{
    return ess_manager->get_all_ess_soc_min();
}

float Asset_Manager::get_all_ess_soc_avg(void)
{
    return ess_manager->get_all_ess_soc_avg();
}

float Asset_Manager::get_ess_total_nameplate_active_power(void)
{
    return ess_manager->get_ess_total_nameplate_active_power();
}

float Asset_Manager::get_ess_total_nameplate_reactive_power(void)
{
    return ess_manager->get_ess_total_nameplate_reactive_power();
}

float Asset_Manager::get_ess_total_nameplate_apparent_power(void)
{
    return ess_manager->get_ess_total_nameplate_apparent_power();
}

float Asset_Manager::get_soc_balancing_factor(void)
{
    return ess_manager->get_soc_balancing_factor();
}

float Asset_Manager::get_feeder_nameplate_active_power(const char* feeder_id)
{
    return feeder_manager->get_feeder_nameplate_active_power(feeder_id);
}

float Asset_Manager::get_poi_nameplate_active_power(void)
{
    return feeder_manager->get_feeder_nameplate_active_power(feeder_manager->get_poi_id());
}

float Asset_Manager::get_gen_total_rated_active_power(void)
{
    return generator_manager->get_gen_total_active_power();
}

float Asset_Manager::get_gen_total_nameplate_active_power(void)
{
    return generator_manager->get_gen_total_nameplate_active_power();
}

float Asset_Manager::get_solar_total_nameplate_active_power(void)
{
    return solar_manager->get_solar_total_nameplate_active_power();
}

float Asset_Manager::get_solar_total_nameplate_reactive_power(void)
{
    return solar_manager->get_solar_total_nameplate_reactive_power();
}

float Asset_Manager::get_solar_total_nameplate_apparent_power(void)
{
    return solar_manager->get_solar_total_nameplate_apparent_power();
}

float Asset_Manager::get_pcs_nominal_voltage_setting(void)
{
   return ess_manager->get_pcs_nominal_voltage_setting();
}

bool Asset_Manager::set_pcs_nominal_voltage_setting(float mPcsNominalVoltageSetting)
{
    return ess_manager->set_pcs_nominal_voltage_setting(mPcsNominalVoltageSetting);
}

float Asset_Manager::get_avg_ac_voltage(const char* feeder_ID)
{    
    return feeder_manager->get_avg_ac_voltage(feeder_ID);
}

/**
 * Starts any available but not running ESSs/solar and tells ESSs/solar to exit standby if any are in standby.
 */
void Asset_Manager::start_available_ess_solar(void)
{
    // if any ESSs/solar are available but not running, start them
    if (ess_manager->get_num_ess_startable() > 0)
        ess_manager->start_all_ess();
    if (solar_manager->get_num_solar_startable() > 0)
        solar_manager->start_all_solar();

    // if any ESSs/solar are in standby, have them exit standby
    if (ess_manager->get_num_ess_in_standby() > 0)
        ess_manager->exit_standby_all_ess();
    if (solar_manager->get_num_solar_in_standby() > 0)
        solar_manager->exit_standby_all_solar();
}

bool Asset_Manager::enter_standby_all_solar(void)
{
    return solar_manager->enter_standby_all_solar();
}

bool Asset_Manager::exit_standby_all_solar(void)
{
    return solar_manager->exit_standby_all_solar();
}

bool Asset_Manager::start_all_solar(void)
{
    return solar_manager->start_all_solar();
}

bool Asset_Manager::stop_all_solar(void)
{
    return solar_manager->stop_all_solar();
}

void Asset_Manager::set_all_gen_grid_form(void)
{
    generator_manager->set_all_gen_grid_form();
    return;
}

void Asset_Manager::set_all_gen_grid_follow(void)
{
    generator_manager->set_all_gen_grid_follow();
    return;
}

void Asset_Manager::set_all_ess_grid_form(void)
{
    ess_manager->set_all_ess_grid_form();
    return;
}

void Asset_Manager::set_all_ess_grid_follow(void)
{
    ess_manager->set_all_ess_grid_follow();
    return;
}

/**
 * Enable or disable runmode 1 solar curtailment algorithm
 * @param enable solar curtailment is enabled if and only if enable is true
 */
void Asset_Manager::set_solar_curtailment_enabled(bool enable)
{
    solar_manager->set_solar_curtailment_enabled(enable);
    return;
}

bool Asset_Manager::enter_standby_all_ess(void)
{
    return ess_manager->enter_standby_all_ess();
}

bool Asset_Manager::exit_standby_all_ess(void)
{
    return ess_manager->exit_standby_all_ess();
}

bool Asset_Manager::start_all_ess(void)
{
    return ess_manager->start_all_ess();
}

bool Asset_Manager::stop_all_ess(void)
{
    return ess_manager->stop_all_ess();
}

bool Asset_Manager::close_all_bms_contactors()
{
    return ess_manager->close_all_bms_contactors();
}

bool Asset_Manager::open_all_bms_contactors()
{
    return ess_manager->open_all_bms_contactors();
}

bool Asset_Manager::direct_start_gen(void)
{
    return generator_manager->direct_start_gen();
}

void Asset_Manager::start_all_gen(void)
{
    generator_manager->start_all_gen();
}

bool Asset_Manager::stop_all_gen(void)
{
    return generator_manager->stop_all_gen();
}

bool Asset_Manager::set_feeder_state_open(Asset_Feeder* found_feeder)
{
    return feeder_manager->set_feeder_state_open(found_feeder);
}

bool Asset_Manager::set_feeder_state_closed(Asset_Feeder* found_feeder)
{
    return feeder_manager->set_feeder_state_closed(found_feeder);
}

bool Asset_Manager::set_poi_feeder_state_open()
{
    return feeder_manager->set_poi_feeder_state_open();
}

bool Asset_Manager::set_poi_feeder_state_closed()
{
    return feeder_manager->set_poi_feeder_state_closed();
}

bool Asset_Manager::set_gen_target_active_power(float desiredkW)
{
    generator_manager->set_gen_target_active_power(desiredkW);
    return true;
}

bool Asset_Manager::set_ess_target_active_power(float desiredkW)
{
    ess_manager->set_ess_target_active_power(desiredkW);
    return true;
}

bool Asset_Manager::set_poi_target_active_power(float desiredkW)
{
    feeder_manager->set_poi_target_active_power(desiredkW);
    return true;
}

bool Asset_Manager::set_ess_target_reactive_power(float desiredkW)
{
    ess_manager->set_ess_target_reactive_power(desiredkW);
    return true;
}

bool Asset_Manager::set_gen_target_reactive_power(float desiredkW)
{
    generator_manager->set_gen_target_reactive_power(desiredkW);
    return true;
}

void Asset_Manager::set_ess_voltage_setpoint(float setpoint)
{
    ess_manager->set_ess_voltage_setpoint(setpoint);
    return;
}

void Asset_Manager::set_ess_frequency_setpoint(float setpoint)
{
    ess_manager->set_ess_frequency_setpoint(setpoint);
    return;
}

void Asset_Manager::set_ess_calibration_vars(ESS_Calibration_Settings settings)
{
    ess_manager->set_all_ess_calibration_vars(settings);
}

void Asset_Manager::set_reactive_power_priority(bool priority)
{
    ess_manager->set_reactive_power_priority(priority);
    generator_manager->set_reactive_power_priority(priority);
    solar_manager->set_reactive_power_priority(priority);
}

void Asset_Manager::set_ess_reactive_kvar_mode(void)
{
    ess_manager->set_ess_reactive_kvar_mode();
    return;
}

void Asset_Manager::set_ess_pwr_factor_mode(void)
{
    ess_manager->set_ess_pwr_factor_mode();
    return;
}

void Asset_Manager::set_solar_reactive_kvar_mode(void)
{
    for (int i = 0; i < get_num_solar_parsed(); i++)
    {
        if (ess_manager->in_maint_mode(i))
            solar_manager->set_solar_reactive_kvar_mode(i);
    }
    return;
}

void Asset_Manager::set_solar_pwr_factor_mode(void)
{
    for (int i = 0; i < get_num_solar_parsed(); i++)
    {
        if (ess_manager->in_maint_mode(i))
            solar_manager->set_solar_pwr_factor_mode(i);
    }
    return;
}

/* power factor setpoints */
void Asset_Manager::set_solar_target_power_factor(float pwr_factor)
{
    solar_manager->set_solar_target_power_factor(pwr_factor);
    return;
}

void Asset_Manager::set_ess_target_power_factor(float pwr_factor)
{
    ess_manager->set_ess_target_power_factor(pwr_factor);
    return;
}
    
void Asset_Manager::set_grid_forming_voltage_slew(float slope)
{
    ess_manager->set_grid_forming_voltage_slew(slope);
    return;
}

/**
 * Passes down Site Manager-configured parameters to the LDSS feature.
 * @param settings Group of parameters set by Site Manager to control the LDSS feature.
 */
void Asset_Manager::update_ldss_settings(LDSS_Settings &&settings)
{
    generator_manager->update_ldss_settings(settings);
}

float Asset_Manager::get_grid_forming_voltage_slew(void)
{
    return ess_manager->get_grid_forming_voltage_slew();
}

float Asset_Manager::get_solar_total_rated_active_power(void)
{
    return solar_manager->get_solar_total_rated_active_power();
}

float Asset_Manager::get_solar_total_rated_reactive_power(void)
{
    return solar_manager->get_solar_total_rated_reactive_power();
}

float Asset_Manager::get_solar_total_rated_apparent_power(void)
{
    return solar_manager->get_solar_total_rated_apparent_power();
}

float Asset_Manager::get_solar_total_max_potential_active_power(void)
{
    return solar_manager->get_solar_total_max_potential_active_power();
}

float Asset_Manager::get_solar_total_min_potential_active_power(void)
{
    return solar_manager->get_solar_total_min_potential_active_power();
}

float Asset_Manager::get_solar_total_potential_reactive_power(void)
{
    return solar_manager->get_solar_total_potential_reactive_power();
}

bool  Asset_Manager::set_solar_target_reactive_power(float desiredkW)
{
    solar_manager->set_solar_target_reactive_power(desiredkW);
    return true;
}

void Asset_Manager::set_solar_target_active_power(float command)
{
    solar_manager->set_solar_target_active_power(command);
}

float Asset_Manager::get_total_kW_charge_limit(void)
{
    return get_ess_total_kW_charge_limit();
}

float Asset_Manager::get_total_kW_discharge_limit(void)
{
    return get_ess_total_kW_discharge_limit() + generator_manager->get_total_kW_discharge_limit() + solar_manager->get_total_kW_discharge_limit();
}

void Asset_Manager::publish_assets()
{
    // Disable publish if second controller (shadow mode)
    if (!*is_primary)
        return;

    ess_manager->publish_assets(ESS, &asset_var_map);
    feeder_manager->publish_assets(FEEDERS, &asset_var_map);
    generator_manager->publish_assets(GENERATORS, &asset_var_map);
    solar_manager->publish_assets(SOLAR, &asset_var_map);
    return;
}

float Asset_Manager::charge_control(float target_soc_desired, bool charge_disable, bool discharge_disable)
{
    return ess_manager->charge_control(target_soc_desired, charge_disable, discharge_disable);
}

// HybridOS Step 2: Process Asset Data
void Asset_Manager::process_asset_data(void)
{
    ess_manager->process_asset_data();
    feeder_manager->process_asset_data(&asset_var_map);
    generator_manager->process_asset_data();
    solar_manager->process_asset_data();
}

// HybridOS Step 4: Update Asset Data
void Asset_Manager::update_asset_data(void)
{
    // No Feeder updates
    if (get_num_ess_parsed() > 0)
        ess_manager->update_asset_data();
    if (get_num_gen_parsed() > 0)
        generator_manager->update_asset_data();
    if (get_num_solar_parsed() > 0)
        solar_manager->update_asset_data();
}

// HybridOS Step 5: Send to Components
void Asset_Manager::send_to_components(void)
{
    // Disable publish if second controller (shadow mode)
    if (*is_primary)
    {
        ess_manager->send_to_components();
        feeder_manager->send_to_components();
        generator_manager->send_to_components();
        solar_manager->send_to_components();
    }
}

bool Asset_Manager::site_clear_faults(void)
{
    FPS_DEBUG_LOG("Site clear faults \n");
    set_gen_clear_faults();
    set_ess_clear_faults();
    set_solar_clear_faults();
    set_feeder_clear_faults();
    return true;
}

void Asset_Manager::set_feeder_clear_faults(void)
{
    feeder_manager->set_clear_faults();
    return;
}

void Asset_Manager::set_gen_clear_faults(void)
{
    generator_manager->set_clear_faults();
    return;
}

void Asset_Manager::set_ess_clear_faults(void)
{
    ess_manager->set_clear_faults();
    return;
}

void Asset_Manager::set_solar_clear_faults(void)
{
    solar_manager->set_clear_faults();
    return;
}

/**
 * Sums how many alarms out of all alarms are active across a specific asset type.
 * @param type The asset type that is being checked.
 * @return The number of alarms that are active.
 */
int Asset_Manager::get_num_active_alarms(assetType type) const
{
    switch (type)
    {
        case ESS:
            return ess_manager->get_num_active_alarms();
        case FEEDERS:
            return feeder_manager->get_num_active_alarms();
        case GENERATORS:
            return generator_manager->get_num_active_alarms();
        case SOLAR:
            return solar_manager->get_num_active_alarms();
        default:
            FPS_ERROR_LOG("Invalid type for get_num_active_alarms");
            return 0;
    }
}

/**
 * Sums how many faults out of all faults are active across a specific asset type.
 * @param type The asset type that is being checked.
 * @return The number of faults that are active.
 */
int Asset_Manager::get_num_active_faults(assetType type) const
{
    switch (type)
    {
        case ESS:
            return ess_manager->get_num_active_faults();
        case FEEDERS:
            return feeder_manager->get_num_active_faults();
        case GENERATORS:
            return generator_manager->get_num_active_faults();
        case SOLAR:
            return solar_manager->get_num_active_faults();
        default:
            FPS_ERROR_LOG("Invalid type for get_num_active_faults");
            return 0;
    }
}

/**
 * Checks if the given alert is active for the asset instance(s) that the alert name references
 * @param alertType Either FAULT or ALARM, depending on what type the given alert is.
 * @param alert The name-mask pair for the alert that is being checked. Expected name format is "/assets/<asset type>/<asset ID>/<alert ID>".
 * If <asset ID> field is "aggregate", all assets of type <asset type> are checked.
 * @return True if the alert is found to be active on the identified asset instance(s).
 */
bool Asset_Manager::check_asset_alert(std::pair<std::string,uint64_t>& alert)
{
    // split the name into fragments: (0) "assets", (1) asset type, (2) asset ID or "aggregate", (3) alert ID
    std::vector<std::string> name_frags = split(alert.first, "/");
    if (name_frags.size() != 4)
    {
        FPS_ERROR_LOG("Invalid asset alert name: %s. Expecting 4 slashes.\n", alert.first.c_str());
        return false;
    }

    // asset type frag identifies which Type Manager holds the target asset(s)
    Type_Manager* type_manager = get_type_manager(name_frags[1]);
    if (type_manager == NULL)
    {
        FPS_ERROR_LOG("Invalid asset type in asset alert name: %s. Expecting ess, solar, generators, or feeders.\n", alert.first.c_str());
        return false;
    }

    // ask the Type Manager if the target asset(s) has an active alert that matches the alert ID being checked
    return type_manager->check_asset_for_alert(name_frags[2], name_frags[3], alert.second);
}

void Asset_Manager::enable_ldss(bool enable)
{
    generator_manager->enable_ldss(enable);
}

void Asset_Manager::start_first_gen(bool enable)
{
    generator_manager->start_first_gen(enable);
}

void Asset_Manager::set_first_gen_is_starting_flag(bool flag)
{
    generator_manager->set_first_gen_is_starting_flag(flag);
}

void Asset_Manager::start_first_solar(bool enable)
{
    solar_manager->start_first_solar(enable);
}

void Asset_Manager::set_min_generators_active(int minGensActive)
{
    generator_manager->set_min_generators_active(minGensActive); 
}

const char* Asset_Manager::get_poi_id(void)
{
    return feeder_manager->get_poi_id();
}

float Asset_Manager::get_poi_max_potential_active_power(void)
{
    return feeder_manager->get_poi_max_potential_active_power();
}

float Asset_Manager::get_poi_min_potential_active_power(void)
{
    return feeder_manager->get_poi_min_potential_active_power();
}

/**
 * Maps a string to the proper Type Manager.
 * @param type String identifying which Type Manager to get. Allowed values are "ess", "solar", "generators", and "feeders".
 * @return Pointer to identified Type Manager, or NULL if passed-in type is not valid.
 */
Type_Manager* Asset_Manager::get_type_manager(std::string type)
{
    if (type == ESS_TYPE_ID)
        return ess_manager;
    else if (type == SOLAR_TYPE_ID)
        return solar_manager;
    else if (type == GENERATORS_TYPE_ID)
        return generator_manager;
    else if (type == FEEDERS_TYPE_ID)
        return feeder_manager;
    else
        return NULL;
}

/**
 * Maps a string to the proper Type Manager.
 * @param type String identifying which Type Manager to get. Allowed to have characters after the end of the type ID.
 *             For example, passing a URI frag pointer that points to "ess/ess_01", "solar/summary", etc. is allowed.
 * @return Pointer to identified Type Manager, or NULL if passed-in type is not valid.
 */
Type_Manager* Asset_Manager::get_type_manager(const char* type)
{
    if (strncmp(type, ESS_TYPE_ID, strlen(ESS_TYPE_ID)) == 0)
        return ess_manager;
    else if (strncmp(type, SOLAR_TYPE_ID, strlen(SOLAR_TYPE_ID)) == 0)
        return solar_manager;
    else if (strncmp(type, GENERATORS_TYPE_ID, strlen(GENERATORS_TYPE_ID)) == 0)
        return generator_manager;
    else if (strncmp(type, FEEDERS_TYPE_ID, strlen(FEEDERS_TYPE_ID)) == 0)
        return feeder_manager;
    else
        return NULL;
}

// Prints each component register URI along with information about all variables that get sourced from that component register
void Asset_Manager::print_component_var_map()
{
    FPS_INFO_LOG("\n************** COMPONENT VARIABLES MAP **************\n");
    try
    {
        for ( auto it = component_var_map.begin(); it != component_var_map.end(); ++it )
        {
            FPS_INFO_LOG("\nURI: %s\n", it->first.c_str());
            for ( auto var_it = it->second.begin(); var_it != it->second.end(); ++var_it)
            {
                FPS_INFO_LOG("FIMS Object %ld of %ld for this URI...\n", var_it - it->second.begin()+1, it->second.size());
                FPS_INFO_LOG("Fims_Object is at address: %p\n", (void*)*var_it);
                FPS_INFO_LOG("Name: %s\n", (*var_it)->get_name());
                FPS_INFO_LOG("UI ID: %s\n", (*var_it)->get_variable_id());
                FPS_INFO_LOG("Component URI: %s\n", (*var_it)->get_component_uri());
                FPS_INFO_LOG("Type: %s\n", (*var_it)->get_type());
                FPS_INFO_LOG("UI Type: %s\n", (*var_it)->get_ui_type());
                FPS_INFO_LOG("Initial Value: %s\n", (*var_it)->value.type == 3 || (*var_it)->value.type == 4 ? "Bit field or Random enum" : (*var_it)->value.print());
                FPS_INFO_LOG("Scaler: %d\n", (*var_it)->scaler);
                FPS_INFO_LOG("Unit: %s\n", (*var_it)->get_unit() == NULL ? "Unit not provided in assets.json" : (*var_it)->get_unit() );
            }
        }
        FPS_INFO_LOG("**********************************************\n\n");
    }
    catch(const std::exception& e)
    {
        FPS_ERROR_LOG("%s\n", e.what());
    }
}

// Prints each asset variable URI along with information about the asset variable
void Asset_Manager::print_asset_var_map()
{
    FPS_INFO_LOG("************** ASSET VARIABLES MAP **************\n");
    try
    {
        for ( auto it = asset_var_map.begin(); it != asset_var_map.end(); ++it )
        {
            FPS_INFO_LOG("\nURI: %s\n", it->first.c_str());
            FPS_INFO_LOG("Fims_Object is at address: %p\n", (void*)it->second);
            FPS_INFO_LOG("Name: %s\n", it->second->get_name());
            FPS_INFO_LOG("UI ID: %s\n", it->second->get_variable_id());
            FPS_INFO_LOG("Component URI: %s\n", it->second->get_component_uri());
            FPS_INFO_LOG("Type: %s\n", it->second->get_type());
            FPS_INFO_LOG("UI Type: %s\n", it->second->get_ui_type());
            FPS_INFO_LOG("Initial Value: %s\n", it->second->value.type == 3 || it->second->value.type == 4 ? "Bit field or Random enum" : it->second->value.print());
            FPS_INFO_LOG("Scaler: %d\n", it->second->scaler);
            FPS_INFO_LOG("Unit: %s\n", it->second->get_unit() == NULL ? "Unit not provided in assets.json" : it->second->get_unit() );
        }
        FPS_INFO_LOG("**********************************************\n\n");
    }
    catch(const std::exception& e)
    {
        FPS_ERROR_LOG("%s\n", e.what());
    }
}
