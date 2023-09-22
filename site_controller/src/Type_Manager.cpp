/**
 * Type_Manager.cpp
 * Base Class for the four Asset Type Managers
 * Refactored from Asset_Manager.cpp
 *
 * Created on January 28th, 2020
 *      Author: Jack Shade (jnshade)
 */

/* C Standard Library Dependencies */
#include <cstdlib>
#include <cstring>
#include <cmath>
/* C++ Standard Library Dependencies */
#include <iterator>
#include <map>
#include <vector>
#include <tuple>
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
#include <fims/defer.hpp>
/* Local Internal Dependencies */
#include <Asset_Manager.h>
#include <Type_Manager.h>
#include <Site_Controller_Utils.h>

extern fims* p_fims;

/****************************************************************************************/
Type_Manager::Type_Manager(const char* _asset_type_id) {
    asset_type_id = _asset_type_id;
    numRunning = 0;
    numAvail = 0;
    numParsed = 0;

    send_FIMS_buf = fmt::memory_buffer();
}

/****************************************************************************************/
Type_Manager::~Type_Manager() {
    // pAssets list is destroyed by derived classes
}

/****************************************************************************************/
const char* Type_Manager::get_asset_type_id() {
    return asset_type_id;
}

/**
 * @brief Handles GETs to URIs beginning with /assets/<asset type>.
 * @param pmsg Pointer to FIMS message struct containing important data like target URI, reply-to URI, etc.
 * @return True if the GET was handled successfully, or false if there was an error.
 */
bool Type_Manager::handle_get(fims_message* pmsg) {
    // clear buffer for use
    send_FIMS_buf.clear();

    // URI is /assets/<asset type>
    if (pmsg->nfrags < 3) {
        if (!add_type_data_to_buffer(send_FIMS_buf))
            return false;
        return send_buffer_to(pmsg->replyto, send_FIMS_buf);
    }

    // URI begins with /assets/<asset type>/summary
    if (strncmp(pmsg->pfrags[2], "summary", strlen("summary")) == 0) {
        return handle_summary_get(pmsg);
    }

    // URI begins with /assets/<asset type>/<asset ID> so get target asset instance
    Asset* target_asset = (pmsg->nfrags == 3) ? get_asset_instance(pmsg->pfrags[2]) : get_asset_instance_using_uri_frags(pmsg->pfrags, 2);
    if (target_asset == NULL) {
        FPS_ERROR_LOG("Could not find asset GET was targeting based on URI %s.", pmsg->uri);
        return false;
    }

    // let target asset instance handle the GET
    return target_asset->handle_get(pmsg);
}

/**
 * @brief Handles GETs to URIs beginning with /assets/<asset type>/summary.
 * @param pmsg Pointer to FIMS GET message.
 * @return True if the GET was handled successfully, or false if there was an error.
 */
bool Type_Manager::handle_summary_get(fims_message* pmsg) {
    // request is for all vars if 3 frags or specific var if 4
    const char* target_variable_id = (pmsg->nfrags != 4) ? NULL : pmsg->pfrags[3];
    generate_asset_type_summary_json(send_FIMS_buf, target_variable_id);
    return send_buffer_to(pmsg->replyto, send_FIMS_buf);
}

/**
 * @brief Adds a JSON object to the given buffer with all of the Type Manager's data.
 * @param buf The buffer to which the JSON object must be added.
 * @return True if the data was added successfully, or false if there was an error.
 */
bool Type_Manager::add_type_data_to_buffer(fmt::memory_buffer& buf) {
    // begin asset type data with opening curly brace
    bufJSON_StartObject(buf);

    // add the asset type summary data
    bufJSON_AddId(buf, "summary");
    generate_asset_type_summary_json(buf);
    bufJSON_AddComma(buf);

    // add data for each asset instance
    for (size_t i = 0; i < pAssets.size(); ++i) {
        // add asset instance's ID with colon
        bufJSON_AddId(buf, pAssets[i]->get_id().c_str());
        // add asset instance's data
        if (!pAssets[i]->add_asset_data_to_buffer(buf, strcmp(asset_type_id, FEEDERS_TYPE_ID) == 0)) {
            FPS_ERROR_LOG("Error adding asset instance with type %s and index %zu to the type manager data object.", asset_type_id, i);
            return false;
        }
        // separate each asset instance from the rest with a comma, but avoid leaving a trailing comma at the end
        if (i != pAssets.size()) {
            bufJSON_AddComma(buf);
        }
    }

    // end asset type data with closing curly brace
    bufJSON_EndObjectNoComma(buf);
    return true;
}

/**
 * @brief Handles SETs to URIs beginning with /assets/{asset type}.
 * @param msg FIMS SET message.
 */
void Type_Manager::handle_set(fims_message& msg) {
    // Find the target asset based on the URI
    Asset* target_asset = get_asset_instance_using_uri_frags(msg.pfrags, 2);
    if (target_asset == NULL) {
        FPS_ERROR_LOG("Could not find asset with ID %s.", msg.pfrags[2]);
        if (msg.replyto != NULL)
            p_fims->Send("set", msg.replyto, NULL, "Error: asset not found.");
        return;
    }

    // Parse the JSON body
    cJSON* cur_body = cJSON_Parse(msg.body);
    if (cur_body == NULL) {
        FPS_ERROR_LOG("Error parsing body.");
        if (msg.replyto != NULL)
            p_fims->Send("set", msg.replyto, NULL, "Error parsing JSON object.");
        return;
    }
    defer { cJSON_Delete(cur_body); };

    // Wrap the JSON body in a key-value pair where the key is the variable ID
    cJSON* new_body = cJSON_CreateObject();
    cJSON_AddItemToObject(new_body, msg.pfrags[3], cur_body);
    cur_body = new_body;

    // Have the asset instance handle the SET
    bool success = target_asset->handle_set(msg.uri, *cur_body);

    // If the user has hit the Start Next button on a generator, we must handle it at or above the Generator Manager level so we can access all generators to adjust priorities
    cJSON* start_next_obj = cJSON_GetObjectItem(cur_body, "start_next");
    if (start_next_obj != NULL) {
        cJSON* valueObject = cJSON_GetObjectItem(start_next_obj, "value");
        if (valueObject != NULL && valueObject->type == cJSON_True && strcmp(asset_type_id, GENERATORS_TYPE_ID) == 0) {
            static_cast<Generator_Manager*>(this)->make_gen_highest_start_priority(static_cast<Asset_Generator*>(target_asset));
            success = true;
        }
    }

    // If the user has hit the Stop Next button on a generator, we must handle it at or above the Generator Manager level so we can access all generators to adjust priorities
    cJSON* stop_next_obj = cJSON_GetObjectItem(cur_body, "stop_next");
    if (stop_next_obj != NULL) {
        cJSON* valueObject = cJSON_GetObjectItem(stop_next_obj, "value");
        if (valueObject != NULL && valueObject->type == cJSON_True && strcmp(asset_type_id, GENERATORS_TYPE_ID) == 0) {
            static_cast<Generator_Manager*>(this)->make_gen_highest_stop_priority(static_cast<Asset_Generator*>(target_asset));
            success = true;
        }
    }

    // Reply with success/failure status
    if (success) {
        if (msg.replyto != NULL)
            p_fims->Send("set", msg.replyto, NULL, msg.body);
    } else {
        FPS_ERROR_LOG("SET to %s failed.", msg.uri);
        if (msg.replyto != NULL)
            p_fims->Send("set", msg.replyto, NULL, "Error processing SET. See logs for details.");
    }
}

int Type_Manager::get_num_avail(void) {
    return numAvail;
}

int Type_Manager::get_num_parsed(void) {
    return numParsed;
}

int Type_Manager::get_num_running(void) {
    return numRunning;
}

// Return the number of assets with components that are in local control mode
int Type_Manager::get_num_in_local_mode(void) {
    return num_in_local_mode;
}

// Sends one PUB for each asset instance and one PUB for the asset type summary data.
// If there are no configured instances for this asset type, the summary will not be published.
// The passed type must match the Type_Manager's asset type.
void Type_Manager::publish_assets(asset_type type) {
    std::string asset_type_base_uri = "/assets/" + std::string(asset_type_id) + "/";

    // publish data for each asset instance
    for (size_t i = 0; i < pAssets.size(); ++i) {
        send_FIMS_buf.clear();
        if (!pAssets[i]->add_asset_data_to_buffer(send_FIMS_buf, type == FEEDERS)) {
            FPS_ERROR_LOG("Error adding asset instance with type %s and index %zu to the type manager publish.", asset_type_id, i);
            return;
        }

        std::string pub_uri = asset_type_base_uri + pAssets[i]->get_id();
        p_fims->Send("pub", pub_uri.c_str(), NULL, to_string(send_FIMS_buf).c_str());
    }

    // publish summary data if instances exist
    if (numParsed > 0) {
        // generate summary data
        send_FIMS_buf.clear();
        generate_asset_type_summary_json(send_FIMS_buf);

        if (send_FIMS_buf.size() == 0) {
            FPS_ERROR_LOG("Error adding %s summary data to JSON buffer for publishing. No summary data was generated.", asset_type_id);
            return;
        }

        std::string pub_uri = asset_type_base_uri + "summary";
        p_fims->Send("pub", pub_uri.c_str(), NULL, to_string(send_FIMS_buf).c_str());
    }
}

// HybridOS Step 5: Send to Components
void Type_Manager::send_to_components(void) {
    for (auto it = pAssets.begin(); it != pAssets.end(); ++it)
        (*it)->send_to_components();
}

void Type_Manager::set_reactive_power_priority(bool priority) {
    for (auto it : pAssets)
        it->set_reactive_power_priority(priority);
}

void Type_Manager::set_clear_faults(void) {
    // tell each individual asset to clear its own faults/alarms
    for (int i = 0; i < numParsed; ++i) {
        pAssets[i]->clear_alerts();
    }
}

/**
 * Counts how many alarms are active across all assets of the managed type.
 * @return Number of active alarms across all assets of the managed type.
 */
int Type_Manager::get_num_active_alarms() const {
    int num_alarms = 0;
    for (auto& pAsset : pAssets) {
        num_alarms += pAsset->get_num_active_alarms();
    }
    return num_alarms;
}

/**
 * Counts how many assets have at least 1 alarm among all assets of the managed type.
 * @return Number of assets with at least 1 alarm for managed type.
 */
int Type_Manager::get_num_alarmed() const {
    int num_alarmed = 0;
    for (auto& pAsset : pAssets) {
        if (pAsset->get_num_active_alarms() > 0)
            num_alarmed++;
    }
    return num_alarmed;
}

/**
 * Counts how many faults are active across all assets of the managed type.
 * @return Number of active faults across all assets of the managed type.
 */
int Type_Manager::get_num_active_faults() const {
    int num_faults = 0;
    for (auto& pAsset : pAssets) {
        num_faults += pAsset->get_num_active_faults();
    }
    return num_faults;
}

/**
 * Counts how many assets have at least 1 fault among all assets of the managed type.
 * @return Number of assets with at least 1 fault for managed type.
 */
int Type_Manager::get_num_faulted() const {
    int num_faulted = 0;
    for (auto& pAsset : pAssets) {
        if (pAsset->get_num_active_faults() > 0)
            num_faulted++;
    }
    return num_faulted;
}

/**
 * Checks a specific asset for a specific alert and applies a mask to that alert during the check.
 * @param asset_id Asset ID of the asset to check for the alert, or "aggregate" if all assets of this type should be checked.
 * @param alert_id ID of the alert to check.
 * @param mask 64-bit mask to apply to the alert value.
 * @return True if any of the bits of the alert value that are covered by the mask are 1, indicating an active alert. False otherwise.
 */
bool Type_Manager::check_asset_for_alert(std::string& asset_id, std::string& alert_id, uint64_t& mask) {
    bool alert_found = false;
    for (auto& pAsset : pAssets) {
        if (asset_id == pAsset->get_id() || asset_id == "aggregate") {
            alert_found = alert_found || pAsset->check_alert(alert_id, mask);
        }
    }
    return alert_found;
}

/**
 * Helper function for translating the Asset Instance name from asset_var_map
 * to the index of the asset in the Asset instance list pAssets
 * @param name The name of the Asset instance
 *             Accepts the uri which will be parsed for the matching Asset Instance id
 * @return The index of the same Asset instance in pAssets
 *         returns -1 if not found
 */
int Type_Manager::map_to_array_translation(std::string name) {
    int index = -1;
    for (int i = 0; i < numParsed; i++) {
        // Check that the Instance name matches our name
        if (name.find(std::string(pAssets[i]->get_id()) + '/') < name.size()) {
            index = i;
            break;
        }
    }
    return index;
}

/**
 * @brief Searches for the asset instance with the given ID.
 * @param asset_id ID of the desired asset instance.
 * @return Pointer to the asset instance, or NULL if no instance with such an ID is found.
 */
Asset* Type_Manager::get_asset_instance(const char* asset_id) {
    for (auto asset : pAssets) {
        if (asset_id == asset->get_id())
            return asset;
    }
    return NULL;
}

/**
 * @brief Parses an asset ID out of an array of URI frag pointers, then returns the asset with the found ID.
 * @param pfrags Array of URI frag pointers taken directly from FIMS message.
 * @param asset_id_index The index in the frag pointer array at which the asset ID is expected to be.
 * @return If the parsing is successful and an asset with the found ID exists, a pointer to that asset. Else, returns NULL.
 */
Asset* Type_Manager::get_asset_instance_using_uri_frags(char** pfrags, size_t asset_id_index) {
    // verify that target frag can fit within memory allocated for it
    char* frag_with_id = pfrags[asset_id_index];
    if (strlen(frag_with_id) > 128) {
        FPS_ERROR_LOG("Cannot parse asset ID when given string length is greater than 128. Given string: %s.", frag_with_id);
        return NULL;
    }

    // copy the target frag into a new string so that the original URI string is not corrupted
    char asset_id_copy[128];
    strcpy(asset_id_copy, frag_with_id);

    // isolate the target frag from any frags that may follow it
    char* ptr_to_slash = strchr(asset_id_copy, '/');
    if (ptr_to_slash != NULL) {
        *ptr_to_slash = '\0';
    }

    // return the asset with the parsed ID
    return get_asset_instance(asset_id_copy);
}
