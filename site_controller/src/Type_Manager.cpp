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
/* Local Internal Dependencies */
#include <Asset_Manager.h>
#include <Type_Manager.h>
#include <Site_Controller_Utils.h>

extern fims *p_fims;


/****************************************************************************************/
Type_Manager::Type_Manager(const char* _asset_type_id)
{
    asset_type_id = _asset_type_id;
    numRunning = 0;
    numAvail = 0;
    numParsed = 0;

    send_FIMS_buf = fmt::memory_buffer();
}

/****************************************************************************************/
Type_Manager::~Type_Manager()
{
    // pAssets list is destroyed by derived classes
}

/****************************************************************************************/
const char* Type_Manager::get_asset_type_id()
{
    return asset_type_id;
}

/**
 * @brief Handles GETs to URIs beginning with /assets/<asset type>.
 * @param pmsg Pointer to FIMS message struct containing important data like target URI, reply-to URI, etc.
 * @param asset_var_map Map of all asset variables to be referenced when building the response.
 * @return True if the GET was handled successfully, or false if there was an error.
*/
bool Type_Manager::handle_get(fims_message *pmsg, std::map<std::string, Fims_Object*> *asset_var_map)
{
    // clear buffer for use
    send_FIMS_buf.clear();

    // URI is /assets/<asset type>
    if (pmsg->nfrags < 3) {
        if (!add_type_data_to_buffer(send_FIMS_buf, asset_var_map))
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
    return target_asset->handle_get(pmsg, asset_var_map);
}

/**
 * @brief Handles GETs to URIs beginning with /assets/<asset type>/summary.
 * @param pmsg Pointer to FIMS GET message.
 * @return True if the GET was handled successfully, or false if there was an error.
*/
bool Type_Manager::handle_summary_get(fims_message *pmsg)
{
    // request is for all vars if 3 frags or specific var if 4
    const char* target_variable_id = (pmsg->nfrags != 4) ? NULL : pmsg->pfrags[3];
    generate_asset_type_summary_json(send_FIMS_buf, target_variable_id);
    return send_buffer_to(pmsg->replyto, send_FIMS_buf);
}

/**
 * @brief Adds a JSON object to the given buffer with all of the Type Manager's data.
 * @param buf The buffer to which the JSON object must be added.
 * @param asset_var_map The map containing all Asset endpoints in the form of the std::string uri and Fims_Object Asset data.
 * @return True if the data was added successfully, or false if there was an error.
*/
bool Type_Manager::add_type_data_to_buffer(fmt::memory_buffer &buf, std::map<std::string, Fims_Object*> *asset_var_map)
{
    // begin asset type data with opening curly brace
    bufJSON_StartObject(buf);

    // add the asset type summary data
    bufJSON_AddId(buf, "summary");
    generate_asset_type_summary_json(buf);
    bufJSON_AddComma(buf);

    // add data for each asset instance
    auto asset_var_it = asset_var_map->begin();
    for(size_t i = 0; i < pAssets.size(); ++i) {
        // add asset instance's ID with colon
        char *asset_instance_id = pAssets[i]->get_id();
        bufJSON_AddId(buf, asset_instance_id);
        // add asset instance's data. the map iterator is passed by reference so it is being moved through the map and will be at the
        // beginning of the next asset instance's vars after having been iterated across the previous asset instance's vars
        if (!pAssets[i]->add_asset_data_to_buffer(buf, asset_var_map, asset_var_it, strcmp(asset_type_id, ESS_TYPE_ID) != 0)) {
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

/****************************************************************************************/
void Type_Manager::handle_set(assetType type, char** pfrags, int nfrags, char* replyto, char* body)
{
    (void) nfrags;
    std::string uri = "/" + std::string(pfrags[0]);

    FPS_DEBUG_LOG("Asset type: %s\n", asset_type_id);
    
    // Flag for whether the asset instance was found
    bool found = false;
    for (int i = 0; i < numParsed; i++)
    {
        // Match Asset Instance id (asset_1/2/3/4) with our uri to find the Asset Instance
        if (uri.find(pAssets[i]->get_id()) < uri.size())
        {
            found = true;
            FPS_DEBUG_LOG("Message body: %s\n", body);
            cJSON* cur_body = cJSON_Parse(body);
            if(cur_body == NULL)
            {
                FPS_DEBUG_LOG("current body is null\n");
                if(replyto != NULL)
                    p_fims->Send("set", replyto, NULL, "Error, Invalid JSON Object.");
                return;
            }

            cJSON* new_body = cJSON_CreateObject();
            cJSON_AddItemToObject(new_body, pfrags[3], cur_body);
            cur_body = new_body;

            // Indicates whether Asset Instance set successfully
            bool success = false;
            // Set only allowed on Asset ui control variables
            switch (type)
            {
                case ESS:
                    success = static_cast<Asset_ESS*>(pAssets[i])->process_set(uri, cur_body);
                    break;
                case FEEDERS:
                    success = static_cast<Asset_Feeder*>(pAssets[i])->process_set(uri, cur_body);
                    break;
                case GENERATORS:
                    success = static_cast<Asset_Generator*>(pAssets[i])->process_set(uri, cur_body);
                    break;
                case SOLAR:
                    success = static_cast<Asset_Solar*>(pAssets[i])->process_set(uri, cur_body);
                    break;
                default:
                    FPS_ERROR_LOG("Invalid type received: %d Type Manager handle_set()\n", type);
                    break;
            }

            // If the user has hit the Start Next button on a generator, we must handle it at or above the Generator Manager level so we can access all generators to adjust priorities
            cJSON* start_next_obj = cJSON_GetObjectItem(cur_body, "start_next");
            if (start_next_obj != NULL)
            {
                cJSON* valueObject = cJSON_GetObjectItem(start_next_obj, "value");
                if(valueObject != NULL && valueObject->type == cJSON_True && type == GENERATORS)
                    static_cast<Generator_Manager*>(this)->make_gen_highest_start_priority( static_cast<Asset_Generator*>(pAssets[i]) );
            }

            // If the user has hit the Stop Next button on a generator, we must handle it at or above the Generator Manager level so we can access all generators to adjust priorities
            cJSON* stop_next_obj = cJSON_GetObjectItem(cur_body, "stop_next");
            if (stop_next_obj != NULL)
            {
                cJSON* valueObject = cJSON_GetObjectItem(stop_next_obj, "value");
                if(valueObject != NULL && valueObject->type == cJSON_True && type == GENERATORS)
                    static_cast<Generator_Manager*>(this)->make_gen_highest_stop_priority( static_cast<Asset_Generator*>(pAssets[i]) );
            }
            
            // Report error occurring in Asset Instances
            if (!success)
            {
                FPS_DEBUG_LOG("Could not perform set for %s\n", uri.c_str());
                if(replyto != NULL)
                    p_fims->Send("set", replyto, NULL, "Error, Invalid JSON Object.");
            }

            cJSON_Delete(cur_body);
            return;
        }
    }
    
    if (!found)
    {
        FPS_DEBUG_LOG( "did not find %s asset in asset map\n", uri.c_str());
        if(replyto != NULL)
            p_fims->Send("set", replyto, NULL, "Error, Asset not found.");
        return;
    }
}

/**
 * New function for processing status/alarm/fault. Acts as an endpoint for Asset_Manager and
 * Asset-instance level variables by finding the appropriate Asset instance based on uri
 * @param uri_endpoint The uri of the Fims_Object in the component map
 * @param names The names of the status/alarm/fault
 * @param value The status/alarm/fault value
 */
bool Type_Manager::process_pub(std::string uri, std::vector<std::string>* names, uint64_t value)
{
    // Find and update the asset level alarm/fault variables
    for (int i = 0; i < numParsed; i++)
    {
        // Find the matching asset instance
        if (uri.find(pAssets[i]->get_id()) < uri.size())
        {
            // Send the updated value
            return pAssets[i]->process_status_pub(names, value);
        }
    }
    return false;
}

int Type_Manager::get_num_avail(void)
{
    return numAvail;
}

int Type_Manager::get_num_parsed(void)
{
    return numParsed;
}

int Type_Manager::get_num_running(void)
{
    return numRunning;
}

// Refactored to publish all data in the Fims_Object asset_var_map as well as aggregate instance and asset level data
void Type_Manager::publish_assets(assetType type, std::map <std::string, Fims_Object*> *asset_var_map)
{
    // Clear buffer for use
    send_FIMS_buf.clear();

    // Uri of the publish
    char uriString[128];
    std::string printedRetBody;
    // Uri of the current Asset instance
    std::string asset_uri;
    // Index in the Asset instance list
    int asset_index = -1;
    std::map <std::string, Fims_Object*>::iterator asset_it;
    std::string base_uri = "/assets/" + std::string(asset_type_id);
    // Find the first match in the list
    for (asset_it = asset_var_map->begin(); asset_it != asset_var_map->end(); asset_it++)
    {
        if (asset_it->first.compare(0, base_uri.length(), base_uri) == 0)
        {
            break;
        }
    } 

    // Continue while we have a match
    for (asset_it = asset_it; asset_it != asset_var_map->end() && asset_it->first.compare(0, base_uri.length(), base_uri) == 0; asset_it++)
    {
        // Parse the Asset Instance name (e.g. feed_1)
        asset_uri = asset_it->first.substr(0, asset_it->first.size() - strlen(asset_it->second->get_variable_id()) - 1);

        // Iterator reached new Asset Instance
        if (asset_uri.compare(uriString) != 0)
        {
            // Ensure this is not the first entry in the object (current buffer has more than just a "{")
            if (send_FIMS_buf.size() > 1)
            {
                // Add the remaining Asset-level data
                if (asset_index >= 0)
                {
                    // It is acceptable that generate_asset_ui() returns false and only adds the valid controls
                    pAssets[asset_index]->generate_asset_ui(send_FIMS_buf);
                    bufJSON_EndObject(send_FIMS_buf); // } retBody
                    bufJSON_RemoveTrailingComma(send_FIMS_buf);
                    printedRetBody = to_string(send_FIMS_buf);
                    if (!printedRetBody.empty())
                    {
                        p_fims->Send("pub", uriString, NULL, printedRetBody.c_str());
                        // Cleanup to prepare for sending more
                        send_FIMS_buf.clear();
                        printedRetBody.clear();
                    }
                    else
                    {
                        FPS_DEBUG_LOG("\n Null cJSON printedRetBody publish_asset(%s): %s\n", asset_type_id, uriString);
                    }
                }
                else
                {
                    FPS_DEBUG_LOG("\n invalid index: %s\n", asset_uri.c_str());
                }
            }

            // Find the Asset Instance from the list
            asset_index = map_to_array_translation(asset_uri);

            // Create the next Asset
            bufJSON_StartObject(send_FIMS_buf); // retBody {

            // Add the new instance to the return object
            if (asset_index >= 0)
            {
                bufJSON_AddString(send_FIMS_buf, "name", pAssets[asset_index]->get_name());
            }
            else
            {
                FPS_ERROR_LOG("\n Could not find Asset ID in list in publish_asset(%s)\n", asset_type_id);
                send_FIMS_buf.clear();
                return;
            }
            
        }

        // Add the Fims_Object data
        snprintf(uriString, 128, "%s", asset_uri.c_str());

        // This if/else determines what assets are configureable assets and can be published naked. 
        // Currently only ess is being published naked based on this if statement others can be added as support increases. 
        if (type != ESS) {
            // Making clothed pub
            asset_it->second->add_to_JSON_buffer(send_FIMS_buf);
        } else {
            // Making naked pub
            asset_it->second->add_to_JSON_buffer(send_FIMS_buf, NULL, false);
        }
    }
    // Exited due to reaching next asset type or end of map
    // Publish the last instance
    if (send_FIMS_buf.size() > 1)
    {
        // Add the remaining Asset-level data
        if (asset_index >= 0)
        {
            // It is acceptable that generate_asset_ui() returns false and only adds the valid controls
            pAssets[asset_index]->generate_asset_ui(send_FIMS_buf);
            bufJSON_EndObject(send_FIMS_buf); // } retBody
            bufJSON_RemoveTrailingComma(send_FIMS_buf);
            printedRetBody = to_string(send_FIMS_buf);
            if (!printedRetBody.empty())
            {
                p_fims->Send("pub", uriString, NULL, printedRetBody.c_str());
                // Cleanup to prepare for sending more
                send_FIMS_buf.clear();
                printedRetBody.clear();
            }
            else
            {
                FPS_DEBUG_LOG("\n Null cJSON printedRetBody publish_asset(%s): %s\n", asset_type_id, uriString);
            }
        }
        else
        {
            FPS_DEBUG_LOG("\n invalid index: %s\n", asset_uri.c_str());
        }
    }

    if (numParsed > 0)
    {
        // Generate summary data
        send_FIMS_buf.clear();
        generate_asset_type_summary_json(send_FIMS_buf);

        if (send_FIMS_buf.size() == 0)
        {
            FPS_ERROR_LOG("\n Empty JSON buffer value publish_asset(%s)\n", asset_type_id);
            return;
        }

        std::string pub_uri = "/assets/" + std::string(asset_type_id) + "/summary";
        printedRetBody = to_string(send_FIMS_buf);
        p_fims->Send("pub", pub_uri.c_str(), NULL, printedRetBody.c_str());
    }
}

// HybridOS Step 5: Send to Components
void Type_Manager::send_to_components(void)
{
    for (auto it = pAssets.begin(); it != pAssets.end(); ++it)
        (*it)->send_to_components();
}

void Type_Manager::set_reactive_power_priority(bool priority)
{
    for (auto it : pAssets)
        it->set_reactive_power_priority(priority);
}

void Type_Manager::set_clear_faults(void)
{
    // tell each individual asset to clear its own faults/alarms
    for (int i = 0; i < numParsed; ++i)
    {
        pAssets[i]->clear_component_faults();
    }
}

/**
 * Counts how many alarms are active across all assets of the managed type.
 * @return Number of active alarms across all assets of the managed type.
 */
int Type_Manager::get_num_active_alarms() const
{
    int num_alarms = 0;
    for (auto& pAsset : pAssets)
    {
        num_alarms += pAsset->get_num_active_alarms();
    }
    return num_alarms;
}

/**
 * Counts how many faults are active across all assets of the managed type.
 * @return Number of active faults across all assets of the managed type.
 */
int Type_Manager::get_num_active_faults() const
{
    int num_faults = 0;
    for (auto& pAsset : pAssets)
    {
        num_faults += pAsset->get_num_active_faults();
    }
    return num_faults;
}

/**
 * Checks a specific asset for a specific alert and applies a mask to that alert during the check.
 * @param asset_id Asset ID of the asset to check for the alert, or "aggregate" if all assets of this type should be checked.
 * @param alert_id ID of the alert to check.
 * @param mask 64-bit mask to apply to the alert value.
 * @return True if any of the bits of the alert value that are covered by the mask are 1, indicating an active alert. False otherwise.
 */
bool Type_Manager::check_asset_for_alert(std::string& asset_id, std::string& alert_id, uint64_t& mask)
{
    bool alert_found = false;
    for(auto& pAsset : pAssets)
    {
        if (asset_id == pAsset->get_id() || asset_id == "aggregate")
        {
            alert_found = alert_found || pAsset->check_alert(alert_id, mask);
        }
    }
    return alert_found;
}

/**
 * Helper function for translating the Asset Instance name from asset_var_map
 * to the index of the asset in the Asset instance list pAssets
 * @param name The name of the Asset instance
 *             Accepts either a name or the uri which will be parsed for the matching Asset Instance id
 * @return The index of the same Asset instance in pAssets
 *         returns -1 if not found
 */
int Type_Manager::map_to_array_translation(std::string name)
{
    int index = -1;
    for (int i = 0; i < numParsed; i++)
    {
        // Check that the Instance name matches our name
        if (name.find(pAssets[i]->get_id()) < name.size())
        {
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
Asset* Type_Manager::get_asset_instance(const char* asset_id)
{
    for(auto asset : pAssets) {
        if (strcmp(asset_id, asset->get_id()) == 0)
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
Asset* Type_Manager::get_asset_instance_using_uri_frags(char** pfrags, size_t asset_id_index)
{
    // verify that target frag can fit within memory allocated for it
    char* frag_with_id = pfrags[asset_id_index];
    if (strlen(frag_with_id) > 128) {
        FPS_ERROR_LOG("Cannot parse asset ID when given string length is greather than 128. Given string: %s.", frag_with_id);
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
