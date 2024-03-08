/*
 * Asset_Manager.cpp
 *
 *  Created on: Sep 4, 2018
 *      Author: ghoward
 */

/* C Standard Library Dependencies */
#include <cstring>
/* C++ Standard Library Dependencies */
#include <map>
#include <vector>
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
/* Local Internal Dependencies */
#include <Asset_Manager.h>
#include <Site_Controller_Utils.h>
#include <Configurator.h>

extern fims* p_fims;

/****************************************************************************************/
Asset_Manager::Asset_Manager() {
    ess_manager = new ESS_Manager();
    feeder_manager = new Feeder_Manager();
    generator_manager = new Generator_Manager();
    solar_manager = new Solar_Manager();

    send_FIMS_buf = fmt::memory_buffer();
}

/****************************************************************************************/
Asset_Manager::~Asset_Manager() {
    delete ess_manager;
    delete feeder_manager;
    delete generator_manager;
    delete solar_manager;

    delete ess_configurator;
    delete feeder_configurator;
    delete generator_configurator;
    delete solar_configurator;
}

/****************************************************************************************/
void Asset_Manager::fims_data_parse(fims_message* pmsg) {
    if (strcmp(pmsg->method, "pub") == 0) {
        FPS_DEBUG_LOG("\n\n***HybridOS Step 1: Receive Component Data.\nIn Asset_Manager::fims_data_parse\n");
        Asset_Manager::handle_pubs(pmsg->pfrags, pmsg->nfrags, pmsg->body);
    } else if (strcmp(pmsg->method, "get") == 0) {
        Asset_Manager::handle_get(pmsg);
    } else if (strcmp(pmsg->method, "set") == 0) {
        Asset_Manager::handle_set(*pmsg);
    } else if (strcmp(pmsg->method, "post") == 0) {
        Asset_Manager::handle_post(pmsg->nfrags, pmsg->body);
    } else if (strcmp(pmsg->method, "del") == 0) {
        Asset_Manager::handle_del(pmsg->nfrags, pmsg->body);
    }
}

/* this function accepts publishes from components, updates internal variables, and replublishes
   updated asset to 'assets', for use by UI */
/****************************************************************************************/
void Asset_Manager::handle_pubs(char** pfrags, int nfrags, char* body) {
    if (nfrags >= 2 && strncmp(pfrags[0], "components", strlen("components")) == 0) {
        cJSON* bodyObject = cJSON_Parse(body);
        if (bodyObject == NULL) {
            FPS_ERROR_LOG("Asset_Manager::handle_pubs NULL parsed data updateAsset(), %s\n", pfrags[0]);
            return;
        }

        // Use manual for loop - ArrayForEach may be deprecated in future cJSON releases
        for (cJSON* cur = bodyObject->child; cur != NULL; cur = cur->next) {
            // Construct the uri
            std::string uri = "/" + std::string(pfrags[0]) + "/" + std::string(cur->string);
            // Find the component in the map
            auto comp_it = component_var_map.find(uri);
            if (comp_it != component_var_map.end()) {
                // component variable map gives list of any Fims_Objects that get their values from this component URI, so iterate through each one of them
                for (auto var_it = comp_it->second.begin(); var_it != comp_it->second.end(); ++var_it) {
                    if (*var_it != NULL) {
                        Fims_Object* current = *var_it;
                        int varArraySize = cJSON_GetArraySize(cur);
                        // Received at least one name value pair in our options value array from the component publish
                        if (varArraySize > 0) {
                            // Status requires unique handling of the status bit strings
                            if (strcmp(current->get_type(), "Status") == 0) {
                                handle_pub_status_options(cur, current, varArraySize);
                            } else if (strcmp(current->get_ui_type(), "alarm") == 0 || strcmp(current->get_ui_type(), "fault") == 0) {
                                handle_pub_alarm_or_fault_options(cur, current, varArraySize);
                            } else {
                                // Not status case but we have at least one options name value pair received from components
                                handle_pub_other_options(cur, current, varArraySize);
                            }
                        }
                        // No options array, only the value received from component publish
                        else {
                            // Special case for alarm/status empty options array. Clear the internal map as well
                            if (strcmp(current->get_ui_type(), "alarm") == 0 || strcmp(current->get_type(), "Status") == 0) {
                                // Set the value directly, do not change the data type
                                current->value.value_bit_field = uint64_t(0);
                                current->options_map.clear();
                            } else if (strcmp(current->get_ui_type(), "fault") == 0) {
                                // Set the value directly, do not change the data type
                                current->value.value_bit_field |= uint64_t(0);
                                // Faults are latching, do not clear the map
                            } else if (current->get_type() && strcmp(current->get_type(), "Int") == 0) {
                                current->set_fims_int(current->get_variable_id(), cur->valueint);
                            } else if (current->get_type() && strcmp(current->get_type(), "Float") == 0) {
                                current->set_fims_float(current->get_variable_id(), cur->valuedouble);
                            } else if (current->get_type() && strcmp(current->get_type(), "Bool") == 0) {
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
    if (cJcomp == nullptr || fimsComp == nullptr) {
        FPS_ERROR_LOG("NULL component or options array received in processing publish.");
        return;
    }

    // Value of the register. OR of all values for bit fields. The last value parsed for random enums
    uint64_t register_value = 0;
    // Iterate through the parsed options value array received from component publish
    for (int i = 0; i < varArraySize; i++) {
        // Get the current item and check its validity
        cJSON* cJitem = cJSON_GetArrayItem(cJcomp, i);
        if (cJitem == NULL) {
            FPS_ERROR_LOG("Asset_Manager::process_pub NULL cJitem\n");
            return;
        }

        // Parsed items from the pub
        cJSON* cJitemValue = cJSON_GetObjectItem(cJitem, "value");
        cJSON* cJitemString = cJSON_GetObjectItem(cJitem, "string");

        if (cJitemValue == NULL || cJitemString == NULL || cJitemString->valuestring == NULL) {
            FPS_ERROR_LOG("Asset_Manager::process_pub NULL cJitemValue\n");
            return;
        }

        switch (fimsComp->get_status_type()) {
            case statusType::bit_field:
                // If bit_field, only values up to 63 are supported, representing internal values up to 2^63
                if (cJitemValue->valueint >= MAX_STATUS_BITS) {
                    FPS_ERROR_LOG("%s: bit field value out of range: %d", fimsComp->get_name(), cJitemValue->valueint);
                    continue;
                }
                // Bit shift the value
                register_value |= uint64_t(1) << cJitemValue->valueint;
                break;
            case statusType::random_enum:
                // Use the enumerated value
                register_value = cJitemValue->valueint;
                break;
            case statusType::invalid:
                FPS_ERROR_LOG("%s: invalid status type configured", fimsComp->get_name());
                return;
        }
        fimsComp->options_map[cJitemValue->valueint] = std::pair<std::string, Value_Object>(cJitemString->valuestring, uint64_t(cJitemValue->valueint));
    }

    // All value options parsed
    // And with the register's mask
    uint64_t masked_value = fimsComp->value.value_mask & register_value;
    // Both bit_field and random_enum use bitfield internally
    fimsComp->value.type = valueType::Bit_Field;
    fimsComp->value.value_bit_field = masked_value;

    // No need for Type_Managers->Asset_instance to process pub
    // Alarm/Fault/Status values updated in process_asset() step for each Asset
}

/**
 * Helper function used by handle_pubs(), handles options in the case of an alarm or fault pub
 * @param cJcomp        The published cJSON data for a component
 * @param fimsComp      A component in the component_var_map that gets its values from the component URI
 * @param varArraySize  Size of the options value array
 */
void Asset_Manager::handle_pub_alarm_or_fault_options(cJSON* cJcomp, Fims_Object* fimsComp, int varArraySize) {
    if (cJcomp == nullptr || fimsComp == nullptr) {
        FPS_ERROR_LOG("NULL component or options array received in processing publish.");
        return;
    }

    // OR Aggregate of all values in the object
    uint64_t bit_field_agg = 0;
    for (int i = 0; i < varArraySize; i++) {
        cJSON* cJitem = cJSON_GetArrayItem(cJcomp, i);
        if (cJitem == NULL) {
            FPS_ERROR_LOG("Asset_Manager::process_pub NULL cJitem\n");
            return;
        }

        // Parsed items from the pub
        cJSON* cJitemValue = cJSON_GetObjectItem(cJitem, "value");
        cJSON* cJitemString = cJSON_GetObjectItem(cJitem, "string");

        if (cJitemValue == NULL || cJitemString == NULL || cJitemString->valuestring == NULL) {
            FPS_ERROR_LOG("Asset_Manager::process_pub NULL cJitemValue\n");
            return;
        }

        if (cJitemValue->valueint < MAX_STATUS_BITS) {
            // Add the bit of the current position
            bit_field_agg |= uint64_t(1) << cJitemValue->valueint;

            // Add the new options names and values
            fimsComp->options_map[cJitemValue->valueint] = std::pair<std::string, Value_Object>(cJitemString->valuestring, uint64_t(cJitemValue->valueint));
        } else
            FPS_ERROR_LOG("Asset_Manager::process_pub Index into component status string array out of bounds, %d, updateAsset()\n", cJitemValue->valueint);
    }

    // All value options parsed
    // And with the mask
    uint64_t masked_value = fimsComp->value.value_mask & bit_field_agg;
    fimsComp->value.type = valueType::Bit_Field;
    if (strcmp(fimsComp->get_ui_type(), "alarm") == 0) {
        // Alarms nonlatching
        fimsComp->value.value_bit_field = masked_value;
    } else
        // Faults latching
        // Or with the current value to latch (maintain high bits)
        fimsComp->value.value_bit_field |= masked_value;

    // No need for Type_Managers->Asset_instance to process pub
    // Alarm values updated in process_asset() step for each Asset
}

/**
 * TODO: Preserving legacy behavior but this use case doesn't really make sense. It's the same thing as the Status registers
 *       but only supports random_enum behavior
 * Helper function used by handle_pubs(), handles options in the case a pub that has options but is not a status, alarm, or fault
 * @param cJcomp        The published cJSON data for a component
 * @param fimsComp      A component in the component_var_map that gets its values from the component URI
 * @param varArraySize  Size of the options value array
 */
void Asset_Manager::handle_pub_other_options(cJSON* cJcomp, Fims_Object* fimsComp, int varArraySize) {
    if (cJcomp == nullptr || fimsComp == nullptr) {
        FPS_ERROR_LOG("NULL component or options array received in processing publish.");
        return;
    }

    // Not status case but we have at least one options name value pair received from components
    // Iterate through the received options value array
    for (int i = 0; i < varArraySize; i++) {
        // Get the current item and check its validity
        cJSON* cJitem = cJSON_GetArrayItem(cJcomp, i);
        if (cJitem == NULL) {
            FPS_ERROR_LOG("Asset_Manager::process_pub NULL cJitem\n");
            return;
        }

        // Parsed items from pub
        cJSON* cJitemValue = cJSON_GetObjectItem(cJitem, "value");
        cJSON* cJitemString = cJSON_GetObjectItem(cJitem, "string");

        if (cJitemValue == NULL || cJitemString == NULL || cJitemString->valuestring == NULL) {
            FPS_ERROR_LOG("Asset_Manager::process_pub NULL cJitemValue\n");
            return;
        }
        fimsComp->options_map[i] = std::pair<std::string, Value_Object>(cJitemString->valuestring, Value_Object());

        if (fimsComp->get_type() && strcmp(fimsComp->get_type(), "Int") == 0) {
            fimsComp->options_map[i].second.set(cJitemValue->valueint);
            fimsComp->set_fims_int(fimsComp->get_variable_id(), cJitemValue->valueint);
        } else if (fimsComp->get_type() && strcmp(fimsComp->get_type(), "Float") == 0) {
            fimsComp->options_map[i].second.set((float)cJitemValue->valuedouble);
            fimsComp->set_fims_float(fimsComp->get_variable_id(), cJcomp->valuedouble);
        } else if (fimsComp->get_type() && strcmp(fimsComp->get_type(), "Bool") == 0) {
            bool bool_value = ((cJcomp->type == cJSON_True) || ((cJcomp->type == cJSON_Number) && (cJitemValue->valueint == 1)));
            fimsComp->options_map[i].second.set(bool_value);
            fimsComp->set_fims_bool(fimsComp->get_variable_id(), bool_value);
        }
    }
}

/**
 * @brief Handles GETs to URIs beginning with /assets.
 * @param pmsg Pointer to FIMS GET message.
 */
void Asset_Manager::handle_get(fims_message* pmsg) {
    // error checking
    if ((strncmp(pmsg->pfrags[0], "assets", strlen("assets")) != 0 || pmsg->replyto == NULL) || !*is_primary)
        return;
    if (pmsg->nfrags < 1 || pmsg->nfrags > 5) {
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
    if (!manager->handle_get(pmsg)) {
        p_fims->Send("set", pmsg->replyto, NULL, "Error building response. Check site_controller logs.");
        return;
    }
}

inline bool is_action_set(fims_message& msg) {
    return (msg.nfrags == 6 && strncmp(msg.pfrags[3], "actions", 7) == 0);
}

/**
 * @brief Handles SETs to URIs beginning with /assets.
 * @param msg FIMS SET message.
 */
void Asset_Manager::handle_set(fims_message& msg) {
    // if SET is to something other than "assets" (example: "components"), do not process it here
    if (strncmp(msg.pfrags[0], "assets", strlen("assets")) != 0) {
        return;
    }
    FPS_DEBUG_LOG("Received SET to %s.", msg.uri);

    // only SETs to specific variables are supported so expected URI format is /assets/{asset type}/{instance ID}/{variable ID}
    // caveat for actions endpoints of length 6 (/assets/ess/ess_1/actions/<action_name>/start)
    if (msg.nfrags != 4 && !is_action_set(msg)) {
        FPS_ERROR_LOG("Received SET to %s, but URIs beginning with /assets are expected to have either exactly 4 or 6 fragments.", msg.uri);
        if (msg.replyto != NULL) {
            p_fims->Send("set", msg.replyto, NULL, "Invalid URI");
        }
        return;  // early return because of msg failure
    }

    // URI starts with /assets/<asset type>. determine which Type Manager should handle the SET
    Type_Manager* manager = get_type_manager(msg.pfrags[1]);
    if (manager == NULL) {
        FPS_ERROR_LOG("Invalid asset type '%s' in SET to URI %s.", msg.pfrags[1], msg.uri);
        if (msg.replyto != NULL) {
            p_fims->Send("set", msg.replyto, NULL, "Invalid Asset Type");
        }
        return;
    }

    // have Type Manager handle the SET
    manager->handle_set(msg);
}

void Asset_Manager::handle_post(int nfrags, char* body) {
    FPS_DEBUG_LOG("handled post: %s frags: %d\n", body, nfrags);

    (void)nfrags;
    (void)body;
}

void Asset_Manager::handle_del(int nfrags, char* body) {
    FPS_DEBUG_LOG("handled del: %s frags: %d\n", body, nfrags);

    (void)nfrags;
    (void)body;
}

/**
 * @brief Sends a JSON object containing all asset data to the specified URI.
 *        If there is a problem building the object, sends an error message to the URI.
 * @param uri The URI to which the asset data is to be sent.
 */
void Asset_Manager::send_all_asset_data(char* uri) {
    // begin asset data with opening curly brace
    bufJSON_StartObject(send_FIMS_buf);
    // add data for each asset type in the form: <asset type ID>: { <asset type data }
    Type_Manager* managers[NUM_TYPE_MANAGERS] = { ess_manager, feeder_manager, generator_manager, solar_manager };
    for (size_t i = 0; i < NUM_TYPE_MANAGERS; ++i) {
        // add asset type ID (i.e. "ess", "solar", etc.) with colon
        bufJSON_AddId(send_FIMS_buf, managers[i]->get_asset_type_id());
        // add asset type data surrounded by braces
        if (!managers[i]->add_type_data_to_buffer(send_FIMS_buf)) {
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

/*
 * @brief This function, asset_create, is called from site_controller.cpp during initial
 *      configuration. It is passed a pointer to the cJSON object parsed from assets.json.
 *      The function breaks the large assets object up into its constituent asset type
 *      cJSON objects: generators, feeders, ess, solar. It passes these cJSON objects to
 *      the Type Managers which continue the configuration process.
 *
 * @param assetsRoot (assets.json)
 * @param actionsRoot (actions.json)
 * @param primary_controller pointer to the bool indicating whether the system is currently
 *      the primary controller. This pointer will be passed to Site_Manager
 *      and down to the Asset Instances, and can be modified through the
 *      fims endpoint /site/operation/primary_controller true/false
 *      Testing will now require this pointer to be set.
 * @return success (bool)
 */
bool Asset_Manager::asset_create(cJSON* assetsRoot, cJSON* actionsRoot, bool* primary_controller) {
    is_primary = primary_controller;

    if (!build_configurators()) {
        FPS_ERROR_LOG("There is something wrong with this build. Error when allocating memory for Type Configurators");
        exit(1);
    }
    cJSON* jsonActions = cJSON_GetObjectItem(actionsRoot, "actions");

    // Extract the large "assets" cJSON object
    cJSON* jsonAssets = cJSON_GetObjectItem(assetsRoot, "assets");
    if (jsonAssets == NULL) {
        FPS_ERROR_LOG("Failed to get object item 'assets' in JSON");
        return false;
    }

    // Struct to aggregate and report on all asset type configuration issues
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    // Extract the "generators" cJSON object and pass it to Generator Manager for configuration
    generator_configurator->asset_type_root = cJSON_GetObjectItem(jsonAssets, "generators");
    if (generator_configurator->asset_type_root != NULL) {
        Config_Validation_Result gen_config_result = generator_configurator->create_assets();
        if (!gen_config_result.is_valid_config) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details("Failed to configure generators."));
        }
        validation_result.absorb(gen_config_result);
    }

    // Extract the "feeders" cJSON object and pass it to Feeder Manager for configuration
    feeder_configurator->asset_type_root = cJSON_GetObjectItem(jsonAssets, "feeders");
    if (feeder_configurator->asset_type_root != NULL) {
        Config_Validation_Result feed_config_result = feeder_configurator->create_assets();
        if (!feed_config_result.is_valid_config) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details("Failed to configure feeders."));
        }
        validation_result.absorb(feed_config_result);
    }

    // Extract the "ess" cJSON object and pass it to ESS Manager for configuration
    ess_configurator->asset_type_root = cJSON_GetObjectItem(jsonAssets, "ess");
    ess_configurator->actions_root = jsonActions;
    if (ess_configurator->asset_type_root != NULL) {
        Config_Validation_Result ess_config_result;

        // if actions present configure them
        if (jsonActions != nullptr) {
            ess_config_result = ess_configurator->create_actions();
            if (!ess_config_result.is_valid_config) {
                validation_result.is_valid_config = false;
                validation_result.ERROR_details.push_back(Result_Details("Failed to configure ESS actions"));
            }
            validation_result.absorb(ess_config_result);
        }

        ess_config_result = ess_configurator->create_assets();
        if (!ess_config_result.is_valid_config) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details("Failed to configure ESS."));
        }
        validation_result.absorb(ess_config_result);
    }

    // Extract the "solar" cJSON object and pass it to Solar Manager for configuration
    solar_configurator->asset_type_root = cJSON_GetObjectItem(jsonAssets, "solar");
    if (solar_configurator->asset_type_root != NULL) {
        Config_Validation_Result solar_config_result = solar_configurator->create_assets();
        if (!solar_config_result.is_valid_config) {
            validation_result.is_valid_config = false;
            validation_result.ERROR_details.push_back(Result_Details("Failed to configure solar."));
        }
        validation_result.absorb(solar_config_result);
    }

    // Report on any/all configuration issues
    validation_result.log_details();

    // Print variable maps to terminal (optional)
    // print_component_var_map();
    // print_asset_var_map();

#ifndef FPS_TEST_MODE
    emit_event("Assets", "Assets initialized", INFO_ALERT);
#endif
    return validation_result.is_valid_config;
}

bool Asset_Manager::build_configurators(void) {
    ess_configurator = new Type_Configurator(ess_manager, &component_var_map, is_primary);
    feeder_configurator = new Type_Configurator(feeder_manager, &component_var_map, is_primary);
    generator_configurator = new Type_Configurator(generator_manager, &component_var_map, is_primary);
    solar_configurator = new Type_Configurator(solar_manager, &component_var_map, is_primary);

    if (!ess_configurator || !feeder_configurator || !generator_configurator || !solar_configurator) {
        return false;
    }
    return true;
}

int Asset_Manager::get_num_ess_avail(void) {
    return ess_manager->get_num_avail();
}

int Asset_Manager::get_num_solar_avail(void) {
    return solar_manager->get_num_avail();
}

int Asset_Manager::get_num_gen_avail(void) {
    return generator_manager->get_num_avail();
}

int Asset_Manager::get_num_feeder_avail(void) {
    return feeder_manager->get_num_avail();
}

int Asset_Manager::get_num_ess_parsed(void) {
    return ess_manager->get_num_parsed();
}

int Asset_Manager::get_num_solar_parsed(void) {
    return solar_manager->get_num_parsed();
}

int Asset_Manager::get_num_gen_parsed(void) {
    return generator_manager->get_num_parsed();
}

int Asset_Manager::get_num_feeder_parsed(void) {
    return feeder_manager->get_num_parsed();
}

int Asset_Manager::get_num_ess_running(void) {
    return ess_manager->get_num_running();
}

int Asset_Manager::get_num_solar_running(void)  // TODO manage returned values
{
    return solar_manager->get_num_running();
}

int Asset_Manager::get_num_gen_running(void) {
    return generator_manager->get_num_running();
}

int Asset_Manager::get_num_feeder_running(void) {
    return feeder_manager->get_num_running();
}

int Asset_Manager::get_num_ess_startable(void) {
    return ess_manager->get_num_ess_startable();
}

int Asset_Manager::get_num_solar_startable(void) {
    return solar_manager->get_num_solar_startable();
}

int Asset_Manager::get_num_ess_in_standby(void) {
    return ess_manager->get_num_ess_in_standby();
}

int Asset_Manager::get_num_solar_in_standby(void) {
    return solar_manager->get_num_solar_in_standby();
}

int Asset_Manager::get_num_gen_controllable(void) {
    return generator_manager->get_num_gen_controllable();
}

int Asset_Manager::get_num_ess_controllable(void) {
    return ess_manager->get_num_ess_controllable();
}

int Asset_Manager::get_num_solar_controllable(void) {
    return solar_manager->get_num_solar_controllable();
}

bool Asset_Manager::get_poi_feeder_state(void) {
    return feeder_manager->get_poi_feeder_state();
}

bool Asset_Manager::get_poi_feeder_close_permissive_state(void) {
    return feeder_manager->get_poi_feeder_close_permissive_state();
}

float Asset_Manager::get_poi_gridside_frequency(void) {
    return feeder_manager->get_poi_gridside_frequency();
}

float Asset_Manager::get_poi_gridside_avg_voltage(void) {
    return feeder_manager->get_poi_gridside_avg_voltage();
}

float Asset_Manager::get_poi_power_factor() {
    return feeder_manager->get_poi_power_factor();
}

bool Asset_Manager::get_sync_feeder_status(void) {
    return feeder_manager->get_sync_feeder_status();
}

float Asset_Manager::get_sync_feeder_gridside_frequency(void) {
    return feeder_manager->get_sync_feeder_gridside_frequency();
}

float Asset_Manager::get_sync_frequency_offset(void) {
    return feeder_manager->get_sync_frequency_offset();
}

float Asset_Manager::get_sync_feeder_gridside_avg_voltage(void) {
    return feeder_manager->get_sync_feeder_gridside_avg_voltage();
}

bool Asset_Manager::set_sync_feeder_close_permissive_remove() {
    return feeder_manager->set_sync_feeder_close_permissive_remove();
}

bool Asset_Manager::set_sync_feeder_close_permissive() {
    return feeder_manager->set_sync_feeder_close_permissive();
}

float Asset_Manager::get_ess_total_active_power(void) {
    return ess_manager->get_ess_total_active_power();
}

float Asset_Manager::get_solar_total_active_power(void) {
    return solar_manager->get_solar_total_active_power();
}

float Asset_Manager::get_gen_total_active_power(void) {
    return (generator_manager->get_gen_total_active_power());
}

float Asset_Manager::get_ess_total_reactive_power(void) {
    return (ess_manager->get_ess_total_reactive_power());
}

float Asset_Manager::get_solar_total_reactive_power(void) {
    return solar_manager->get_solar_total_reactive_power();
}

float Asset_Manager::get_gen_total_reactive_power(void) {
    return generator_manager->get_gen_total_reactive_power();
}

float Asset_Manager::get_ess_total_uncontrollable_active_power(void) {
    return ess_manager->get_ess_total_uncontrollable_active_power();
}

float Asset_Manager::get_solar_total_uncontrollable_active_power(void) {
    return solar_manager->get_solar_total_uncontrollable_active_power();
}

float Asset_Manager::get_gen_total_uncontrollable_active_power(void) {
    return generator_manager->get_gen_total_uncontrollable_active_power();
}

bool Asset_Manager::get_feeder_state(Asset_Feeder* target_feeder) {
    return feeder_manager->get_feeder_state(target_feeder);
}

/**
 * Status of the utility tracked by the feeder
 * @param target_feeder the validated feeder asset
 */
bool Asset_Manager::get_feeder_utility_status(Asset_Feeder* target_feeder) {
    return feeder_manager->get_utility_status(target_feeder);
}

Asset_Feeder* Asset_Manager::validate_feeder_id(const char* feeder_ID) {
    return feeder_manager->validate_feeder_id(feeder_ID);
}

float Asset_Manager::get_gen_total_max_potential_active_power(void) {
    return generator_manager->get_gen_total_max_potential_active_power();
}

float Asset_Manager::get_gen_total_min_potential_active_power(void) {
    return generator_manager->get_gen_total_min_potential_active_power();
}

float Asset_Manager::get_gen_total_potential_reactive_power(void) {
    return generator_manager->get_gen_total_potential_reactive_power();
}

float Asset_Manager::get_feeder_active_power(const char* feeder_ID) {
    return feeder_manager->get_feeder_active_power(feeder_ID);
}

float Asset_Manager::get_feeder_reactive_power(const char* feeder_ID) {
    return feeder_manager->get_feeder_reactive_power(feeder_ID);
}

float Asset_Manager::get_ess_total_max_potential_active_power(void) {
    return ess_manager->get_ess_total_max_potential_active_power();
}

float Asset_Manager::get_ess_total_min_potential_active_power(void) {
    return ess_manager->get_ess_total_min_potential_active_power();
}

float Asset_Manager::get_ess_total_potential_reactive_power(void) {
    return ess_manager->get_ess_total_potential_reactive_power();
}

float Asset_Manager::get_ess_total_rated_active_power(void) {
    return ess_manager->get_ess_total_rated_active_power();
}

float Asset_Manager::get_ess_total_rated_reactive_power(void) {
    return ess_manager->get_ess_total_rated_reactive_power();
}

float Asset_Manager::get_ess_total_rated_apparent_power(void) {
    return ess_manager->get_ess_total_rated_apparent_power();
}

float Asset_Manager::get_ess_total_kW_charge_limit(void) {
    return ess_manager->get_total_kW_charge_limit();
}

float Asset_Manager::get_ess_total_kW_discharge_limit(void) {
    return ess_manager->get_total_kW_discharge_limit();
}

float Asset_Manager::get_ess_total_chargeable_power_kW(void) {
    return ess_manager->get_total_chargeable_power_kW();
}

float Asset_Manager::get_ess_total_dischargeable_power_kW(void) {
    return ess_manager->get_total_dischargeable_power_kW();
}

float Asset_Manager::get_ess_total_chargeable_energy_kWh(void) {
    return ess_manager->get_total_chargeable_energy_kWh();
}

float Asset_Manager::get_ess_total_dischargeable_energy_kWh(void) {
    return ess_manager->get_total_dischargeable_energy_kWh();
}

std::vector<int> Asset_Manager::get_ess_setpoint_statuses(void) {
    return ess_manager->get_setpoint_statuses();
}

float Asset_Manager::get_ess_soc_max(void) {
    return ess_manager->get_ess_soc_max();
}

float Asset_Manager::get_ess_soc_min(void) {
    return ess_manager->get_ess_soc_min();
}

float Asset_Manager::get_ess_soc_avg(void) {
    return ess_manager->get_ess_soc_avg();
}

float Asset_Manager::get_all_ess_soc_max(void) {
    return ess_manager->get_all_ess_soc_max();
}

float Asset_Manager::get_all_ess_soc_min(void) {
    return ess_manager->get_all_ess_soc_min();
}

float Asset_Manager::get_all_ess_soc_avg(void) {
    return ess_manager->get_all_ess_soc_avg();
}

float Asset_Manager::get_ess_total_nameplate_active_power(void) {
    return ess_manager->get_ess_total_nameplate_active_power();
}

float Asset_Manager::get_ess_total_nameplate_reactive_power(void) {
    return ess_manager->get_ess_total_nameplate_reactive_power();
}

float Asset_Manager::get_ess_total_nameplate_apparent_power(void) {
    return ess_manager->get_ess_total_nameplate_apparent_power();
}

float Asset_Manager::get_soc_balancing_factor(void) {
    return ess_manager->get_soc_balancing_factor();
}

float Asset_Manager::get_feeder_nameplate_active_power(const char* feeder_id) {
    return feeder_manager->get_feeder_nameplate_active_power(feeder_id);
}

float Asset_Manager::get_poi_nameplate_active_power(void) {
    return feeder_manager->get_feeder_nameplate_active_power(feeder_manager->get_poi_id().c_str());
}

float Asset_Manager::get_gen_total_rated_active_power(void) {
    return generator_manager->get_gen_total_active_power();
}

float Asset_Manager::get_gen_total_nameplate_active_power(void) {
    return generator_manager->get_gen_total_nameplate_active_power();
}

float Asset_Manager::get_solar_total_nameplate_active_power(void) {
    return solar_manager->get_solar_total_nameplate_active_power();
}

float Asset_Manager::get_solar_total_nameplate_reactive_power(void) {
    return solar_manager->get_solar_total_nameplate_reactive_power();
}

float Asset_Manager::get_solar_total_nameplate_apparent_power(void) {
    return solar_manager->get_solar_total_nameplate_apparent_power();
}

float Asset_Manager::get_pcs_nominal_voltage_setting(void) {
    return ess_manager->get_pcs_nominal_voltage_setting();
}

bool Asset_Manager::set_pcs_nominal_voltage_setting(float mPcsNominalVoltageSetting) {
    return ess_manager->set_pcs_nominal_voltage_setting(mPcsNominalVoltageSetting);
}

float Asset_Manager::get_avg_ac_voltage(const char* feeder_ID) {
    return feeder_manager->get_avg_ac_voltage(feeder_ID);
}

/**
 * Starts any available Solar that are currently stopped/in standby.
 * Recall functionality like auto_restart before calling this function.
 */
void Asset_Manager::start_available_solar(void) {
    // if any solar are available but not running, start them
    if (solar_manager->get_num_solar_startable() > 0)
        solar_manager->start_all_solar();

    // if any solar are in standby, have them exit standby
    if (solar_manager->get_num_solar_in_standby() > 0)
        solar_manager->exit_standby_all_solar();
}

bool Asset_Manager::enter_standby_all_solar(void) {
    return solar_manager->enter_standby_all_solar();
}

bool Asset_Manager::exit_standby_all_solar(void) {
    return solar_manager->exit_standby_all_solar();
}

bool Asset_Manager::start_all_solar(void) {
    return solar_manager->start_all_solar();
}

bool Asset_Manager::stop_all_solar(void) {
    return solar_manager->stop_all_solar();
}

void Asset_Manager::set_all_gen_grid_form(void) {
    generator_manager->set_all_gen_grid_form();
    return;
}

void Asset_Manager::set_all_gen_grid_follow(void) {
    generator_manager->set_all_gen_grid_follow();
    return;
}

void Asset_Manager::set_all_ess_grid_form(void) {
    ess_manager->set_all_ess_grid_form();
    return;
}

void Asset_Manager::set_all_ess_grid_follow(void) {
    ess_manager->set_all_ess_grid_follow();
    return;
}

/**
 * Enable or disable runmode 1 solar curtailment algorithm
 * @param enable solar curtailment is enabled if and only if enable is true
 */
void Asset_Manager::set_solar_curtailment_enabled(bool enable) {
    solar_manager->set_solar_curtailment_enabled(enable);
    return;
}

/**
 * Starts any available ESS that are currently stopped/in standby.
 * Recall functionality like auto_restart before calling this function.
 */
void Asset_Manager::start_available_ess(void) {
    // if any ESSs/solar are available but not running, start them
    if (ess_manager->get_num_ess_startable() > 0)
        ess_manager->start_all_ess();

    // if any ESSs/solar are in standby, have them exit standby
    if (ess_manager->get_num_ess_in_standby() > 0)
        ess_manager->exit_standby_all_ess();
}

bool Asset_Manager::enter_standby_all_ess(void) {
    return ess_manager->enter_standby_all_ess();
}

bool Asset_Manager::exit_standby_all_ess(void) {
    return ess_manager->exit_standby_all_ess();
}

bool Asset_Manager::start_all_ess(void) {
    return ess_manager->start_all_ess();
}

bool Asset_Manager::stop_all_ess(void) {
    return ess_manager->stop_all_ess();
}

bool Asset_Manager::close_all_bms_contactors() {
    return ess_manager->close_all_bms_contactors();
}

bool Asset_Manager::open_all_bms_contactors() {
    return ess_manager->open_all_bms_contactors();
}

bool Asset_Manager::direct_start_gen(void) {
    return generator_manager->direct_start_gen();
}

void Asset_Manager::start_all_gen(void) {
    generator_manager->start_all_gen();
}

bool Asset_Manager::stop_all_gen(void) {
    return generator_manager->stop_all_gen();
}

bool Asset_Manager::set_feeder_state_open(Asset_Feeder* target_feeder) {
    return feeder_manager->set_feeder_state_open(target_feeder);
}

bool Asset_Manager::set_feeder_state_closed(Asset_Feeder* target_feeder) {
    return feeder_manager->set_feeder_state_closed(target_feeder);
}

bool Asset_Manager::set_poi_feeder_state_open() {
    return feeder_manager->set_poi_feeder_state_open();
}

bool Asset_Manager::set_poi_feeder_state_closed() {
    return feeder_manager->set_poi_feeder_state_closed();
}

bool Asset_Manager::set_gen_target_active_power(float desiredkW) {
    generator_manager->set_gen_target_active_power(desiredkW);
    return true;
}

bool Asset_Manager::set_ess_target_active_power(float desiredkW) {
    ess_manager->set_ess_target_active_power(desiredkW);
    return true;
}

bool Asset_Manager::set_poi_target_active_power(float desiredkW) {
    feeder_manager->set_poi_target_active_power(desiredkW);
    return true;
}

bool Asset_Manager::set_ess_target_reactive_power(float desiredkW) {
    ess_manager->set_ess_target_reactive_power(desiredkW);
    return true;
}

bool Asset_Manager::set_gen_target_reactive_power(float desiredkW) {
    generator_manager->set_gen_target_reactive_power(desiredkW);
    return true;
}

void Asset_Manager::set_ess_voltage_setpoint(float setpoint) {
    ess_manager->set_ess_voltage_setpoint(setpoint);
    return;
}

void Asset_Manager::set_ess_frequency_setpoint(float setpoint) {
    ess_manager->set_ess_frequency_setpoint(setpoint);
    return;
}

void Asset_Manager::set_ess_calibration_vars(ESS_Calibration_Settings settings) {
    ess_manager->set_all_ess_calibration_vars(settings);
}

void Asset_Manager::set_reactive_power_priority(bool priority) {
    ess_manager->set_reactive_power_priority(priority);
    generator_manager->set_reactive_power_priority(priority);
    solar_manager->set_reactive_power_priority(priority);
}

void Asset_Manager::set_ess_reactive_kvar_mode(void) {
    ess_manager->set_ess_reactive_kvar_mode();
    return;
}

void Asset_Manager::set_ess_pwr_factor_mode(void) {
    ess_manager->set_ess_pwr_factor_mode();
    return;
}

void Asset_Manager::set_solar_reactive_kvar_mode(void) {
    for (int i = 0; i < get_num_solar_parsed(); i++) {
        if (ess_manager->in_maint_mode(i))
            solar_manager->set_solar_reactive_kvar_mode(i);
    }
    return;
}

void Asset_Manager::set_solar_pwr_factor_mode(void) {
    for (int i = 0; i < get_num_solar_parsed(); i++) {
        if (ess_manager->in_maint_mode(i))
            solar_manager->set_solar_pwr_factor_mode(i);
    }
    return;
}

/* power factor setpoints */
void Asset_Manager::set_solar_target_power_factor(float pwr_factor) {
    solar_manager->set_solar_target_power_factor(pwr_factor);
    return;
}

void Asset_Manager::set_ess_target_power_factor(float pwr_factor) {
    ess_manager->set_ess_target_power_factor(pwr_factor);
    return;
}

void Asset_Manager::set_grid_forming_voltage_slew(float slope) {
    ess_manager->set_grid_forming_voltage_slew(slope);
    return;
}

/**
 * Passes down Site Manager-configured parameters to the LDSS feature.
 * @param settings Group of parameters set by Site Manager to control the LDSS feature.
 */
void Asset_Manager::update_ldss_settings(LDSS_Settings&& settings) {
    settings.pEss = ess_manager;
    generator_manager->update_ldss_settings(settings);
}

float Asset_Manager::get_grid_forming_voltage_slew(void) {
    return ess_manager->get_grid_forming_voltage_slew();
}

float Asset_Manager::get_solar_total_rated_active_power(void) {
    return solar_manager->get_solar_total_rated_active_power();
}

float Asset_Manager::get_solar_total_rated_reactive_power(void) {
    return solar_manager->get_solar_total_rated_reactive_power();
}

float Asset_Manager::get_solar_total_rated_apparent_power(void) {
    return solar_manager->get_solar_total_rated_apparent_power();
}

float Asset_Manager::get_solar_total_max_potential_active_power(void) {
    return solar_manager->get_solar_total_max_potential_active_power();
}

float Asset_Manager::get_solar_total_min_potential_active_power(void) {
    return solar_manager->get_solar_total_min_potential_active_power();
}

float Asset_Manager::get_solar_total_potential_reactive_power(void) {
    return solar_manager->get_solar_total_potential_reactive_power();
}

bool Asset_Manager::set_solar_target_reactive_power(float desiredkW) {
    solar_manager->set_solar_target_reactive_power(desiredkW);
    return true;
}

void Asset_Manager::set_solar_target_active_power(float command) {
    solar_manager->set_solar_target_active_power(command);
}

float Asset_Manager::get_total_kW_charge_limit(void) {
    return get_ess_total_kW_charge_limit();
}

float Asset_Manager::get_total_kW_discharge_limit(void) {
    return get_ess_total_kW_discharge_limit() + generator_manager->get_total_kW_discharge_limit() + solar_manager->get_total_kW_discharge_limit();
}

void Asset_Manager::publish_assets() {
    // Disable publish if second controller (shadow mode)
    if (!*is_primary)
        return;

    ess_manager->publish_assets();
    feeder_manager->publish_assets();
    generator_manager->publish_assets();
    solar_manager->publish_assets();
    return;
}

float Asset_Manager::charge_control(float target_soc_desired, bool charge_disable, bool discharge_disable) {
    return ess_manager->charge_control(target_soc_desired, charge_disable, discharge_disable);
}

// HybridOS Step 2: Process Asset Data
void Asset_Manager::process_asset_data(void) {
    ess_manager->process_asset_data();
    feeder_manager->process_asset_data();
    generator_manager->process_asset_data();
    solar_manager->process_asset_data();
}

// HybridOS Step 4: Update Asset Data
void Asset_Manager::update_asset_data(void) {
    // No Feeder updates
    if (get_num_ess_parsed() > 0)
        ess_manager->update_asset_data();
    if (get_num_gen_parsed() > 0)
        generator_manager->update_asset_data();
    if (get_num_solar_parsed() > 0)
        solar_manager->update_asset_data();
}

// HybridOS Step 5: Send to Components
void Asset_Manager::send_to_components(void) {
    // Disable publish if second controller (shadow mode)
    if (*is_primary) {
        ess_manager->send_to_components();
        feeder_manager->send_to_components();
        generator_manager->send_to_components();
        solar_manager->send_to_components();
    }
}

bool Asset_Manager::site_clear_faults(void) {
    FPS_DEBUG_LOG("Site clear faults \n");
    set_gen_clear_faults();
    set_ess_clear_faults();
    set_solar_clear_faults();
    set_feeder_clear_faults();
    return true;
}

void Asset_Manager::set_feeder_clear_faults(void) {
    feeder_manager->set_clear_faults();
    return;
}

void Asset_Manager::set_gen_clear_faults(void) {
    generator_manager->set_clear_faults();
    return;
}

void Asset_Manager::set_ess_clear_faults(void) {
    ess_manager->set_clear_faults();
    return;
}

void Asset_Manager::set_solar_clear_faults(void) {
    solar_manager->set_clear_faults();
    return;
}

/**
 * Sums how many alarms out of all alarms are active across a specific asset type.
 * @param type The asset type that is being checked.
 * @return The number of alarms that are active.
 */
int Asset_Manager::get_num_active_alarms(asset_type type) const {
    switch (type) {
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
int Asset_Manager::get_num_active_faults(asset_type type) const {
    switch (type) {
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
 * @param alert The name-mask pair for the alert that is being checked. Expected name format is "/assets/<asset type>/<asset ID>/<alert ID>".
 * If <asset ID> field is "aggregate", all assets of type <asset type> are checked.
 * @return True if the alert is found to be active on the identified asset instance(s).
 */
bool Asset_Manager::check_asset_alert(std::pair<std::string, uint64_t>& alert) {
    // split the name into fragments: (0) "assets", (1) asset type, (2) asset ID or "aggregate", (3) alert ID
    std::vector<std::string> name_frags = split(alert.first, "/");
    if (name_frags.size() != 4) {
        FPS_ERROR_LOG("Invalid asset alert name: %s. Expecting 4 slashes.\n", alert.first.c_str());
        return false;
    }

    // asset type frag identifies which Type Manager holds the target asset(s)
    Type_Manager* type_manager = get_type_manager(name_frags[1]);
    if (type_manager == NULL) {
        FPS_ERROR_LOG("Invalid asset type in asset alert name: %s. Expecting ess, solar, generators, or feeders.\n", alert.first.c_str());
        return false;
    }

    // ask the Type Manager if the target asset(s) has an active alert that matches the alert ID being checked
    return type_manager->check_asset_for_alert(name_frags[2], name_frags[3], alert.second);
}

void Asset_Manager::enable_ldss(bool enable) {
    generator_manager->enable_ldss(enable);
}

void Asset_Manager::start_first_gen(bool enable) {
    generator_manager->start_first_gen(enable);
}

void Asset_Manager::set_first_gen_is_starting_flag(bool flag) {
    generator_manager->set_first_gen_is_starting_flag(flag);
}

void Asset_Manager::set_min_generators_active(int minGensActive) {
    generator_manager->set_min_generators_active(minGensActive);
}

const std::string Asset_Manager::get_poi_id(void) {
    return feeder_manager->get_poi_id();
}

float Asset_Manager::get_poi_max_potential_active_power(void) {
    return feeder_manager->get_poi_max_potential_active_power();
}

float Asset_Manager::get_poi_min_potential_active_power(void) {
    return feeder_manager->get_poi_min_potential_active_power();
}

/**
 * Maps a string to the proper Type Manager.
 * @param type String identifying which Type Manager to get. Allowed values are "ess", "solar", "generators", and "feeders".
 * @return Pointer to identified Type Manager, or NULL if passed-in type is not valid.
 */
Type_Manager* Asset_Manager::get_type_manager(std::string type) {
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
Type_Manager* Asset_Manager::get_type_manager(const char* type) {
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
