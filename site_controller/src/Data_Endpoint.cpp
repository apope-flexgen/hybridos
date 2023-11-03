/**
 * Module used to communicate with the InfluxDB storage module.
 * Handles getting and setting configuration and setpoint data.
 */

/* C Standard Library Dependencies */
#include <string.h>
#include <unistd.h>
/* C++ Standard Library Dependencies */
#include <thread>
#include <memory>
/* System Internal Dependencies */
#include <Logger.h>
/* Local Internal Dependencies */
#include <Data_Endpoint.h>
#include <Site_Controller_Utils.h>

extern Asset_Manager* assetMgr;
extern Site_Manager* siteMgr;

/**
 * Setpoints can be read in before site state is changed to init
 * This means the ESS instances have not received their start from components yet so site/enter standby cannot succeed
 * Wait for init to complete before sending the specific setpoints
 */
void delayed_set(fims_message* msg) {
    sleep(10);
    // Send the msg to the appropriate manager
    if (strncmp(msg->pfrags[0], "site", strlen("site")) == 0)
        siteMgr->fims_data_parse(msg);
    else
        assetMgr->fims_data_parse(msg);
    free_fims_msg(msg);
}

Data_Endpoint::Data_Endpoint() {
    // Construct the setpoint pairs hash table as a bidirection map, so that a maps to b and b maps to a
    // Whenever whenever we receive a true value for the key, the associated value(s) will be set false
    opposite_setpoints.insert(std::make_pair(std::string("breaker_close"), std::vector<std::string>({ "breaker_open" })));
    opposite_setpoints.insert(std::make_pair(std::string("breaker_open"), std::vector<std::string>({ "breaker_close" })));
    opposite_setpoints.insert(std::make_pair(std::string("breaker_close_permissive_remove"), std::vector<std::string>({ "breaker_close_permissive" })));
    opposite_setpoints.insert(std::make_pair(std::string("breaker_close_permissive"), std::vector<std::string>({ "breaker_close_permissive_remove" })));
    opposite_setpoints.insert(std::make_pair(std::string("disable_flag"), std::vector<std::string>({ "enable_flag" })));
    opposite_setpoints.insert(std::make_pair(std::string("enable_flag"), std::vector<std::string>({ "disable_flag" })));
}

/**
 * Send get request to storage for the parameters given and block for response
 * @param uri The URI of the request
 * @param replyto The replyto URI of the request
 * @return A malloc'd char pointer to the response
 *         Caller is responsible for memory management
 */
char* Data_Endpoint::get_from_uri(std::string uri, std::string replyto) {
    bool response_received = false;
    fims_message* pmsg = NULL;
    char* response_body = NULL;
    timespec entry_time, exit_time;
    int perioduS = 0;
    // Report the current request, unless it's for the documents list in DBI
    if (uri.find("_show_documents") == std::string::npos)
        FPS_INFO_LOG("Requesting storage entries for uri: %s\n", uri.c_str());

    // Mock sending the request to storage
    if (!p_fims->Send("get", uri.c_str(), replyto.c_str(), NULL)) {
        FPS_ERROR_LOG("Failed to send configuration request to storage for uri: %s\n", uri.c_str());
        return NULL;
    }

    // Block until a response is received or timeout
    while (!response_received && p_fims->Connected() && perioduS < DB_RESPONSE_TIMEOUT) {
        clock_gettime(CLOCK_MONOTONIC, &entry_time);

        pmsg = p_fims->Receive_Timeout(DB_RESPONSE_TIMEOUT);  // blocking with timeout in uSec.
        if (pmsg != NULL) {
            // Check that the response uri matches
            if (pmsg->nfrags > 0 && replyto.find(pmsg->pfrags[0]) < replyto.size()) {
                response_body = strdup(pmsg->body);
                response_received = true;
            }
            p_fims->free_message(pmsg);
        } else {
            FPS_ERROR_LOG("Failed to get response from dbi for: %s\n", uri.c_str());
        }

        clock_gettime(CLOCK_MONOTONIC, &exit_time);
        perioduS += (exit_time.tv_sec - entry_time.tv_sec) * 1000000;
    }
    return response_body;
}

/**
 * Echo site controllers set/del requests to dbi persistence across restarts
 * @param uri The base uri of the setpoint
 * @param endpoint the endpoint of the uri e.g. the logical setting
 * @param valueObject the body of the request containing the updated value
 */
bool Data_Endpoint::setpoint_writeout(std::string uri, std::string endpoint, cJSON** valueObject) {
    // Temp variable that will be used when clothing naked sets
    if (p_fims == NULL) {
        FPS_ERROR_LOG("Fims not configured.");
        return false;
    }

    // Extract the value from the setpoint
    cJSON* parsed_value = cJSON_GetObjectItem(*valueObject, "value");
    if (parsed_value == NULL) {
        // cloth the value if Null do not throw an error
        FPS_DEBUG_LOG("In Data_Endpoint::setpoint_writeout() parsed_value was NULL causing a clothing sequence on valueObject.");
        *valueObject = clothe_naked_cJSON(*valueObject);
        parsed_value = cJSON_GetObjectItem(*valueObject, "value");
        if (parsed_value == NULL) {
            FPS_ERROR_LOG("Received NULL setpoint on %s, cannot writeout to dbi/cops.\n", uri.c_str());
            return false;
        }
    }

    // Preserve the original value prior to its manipulation
    // std::string does not free the allocated memory properly
    char* original_value = cJSON_PrintUnformatted(*valueObject);
    // Send along the current setpoint
    std::string original_dbi_uri = "/dbi/site_controller/setpoints" + uri;
    p_fims->Send("set", original_dbi_uri.c_str(), NULL, original_value);

    // Create a list of uris to which the setpoint will be sent (DBI)
    // Includes replacement for default setpoint pairs e.g. stop -> start
    std::vector<std::string> uris;
    bool should_invalidate = parsed_value->type == cJSON_True;
    if (should_invalidate) {
        uris = construct_writeout_uris(uri, endpoint);
        parsed_value->type = cJSON_False;
    }

    // Invalidate any pairs associated with the current setpoint
    for (size_t i = 0; i < uris.size(); ++i) {
        if (uris[i].empty()) {
            FPS_ERROR_LOG("Received NULL URI for writeout Data_Endpoint::setpoint_writeout().");
            return false;
        }

        // std::string does not free the allocated memory properly
        char* value_string = cJSON_PrintUnformatted(*valueObject);
        p_fims->Send("set", uris[i].c_str(), NULL, value_string);
        free(value_string);
    }

    free(original_value);
    return true;
}

/**
 * Handles reading in all setpoints written out by the last instance of HybridOS
 * After parsing, will send along these sets to Asset/Site_Manager for further configuration
 * @param asset_manager Reference to Asset_Manager to manually call its fims_data_parse()
 * @param site_manager Reference to Site_Manager to manually call its fims_data_parse()
 */
bool Data_Endpoint::setpoint_readin() {
    if (p_fims == NULL) {
        FPS_ERROR_LOG("Fims not configured\n");
        return false;
    }

    // construct the request uris
    std::string uri = "/dbi/site_controller/setpoints";
    std::string replyto = "/site_controller/setpoints";

    // Get the setpoints
    char* response_body = get_from_uri(uri, replyto);
    if (!response_body || strcmp(response_body, "{}") == 0) {
        free(response_body);
        // It's acceptable that DBI wasn't running or setpoints weren't present
        return true;
    }

    cJSON* pJsonRoot = cJSON_Parse(response_body);

    free(response_body);
    // Only error if we failed to parse the json
    if (pJsonRoot == NULL) {
        FPS_ERROR_LOG("HybridOS_Controller received NULL response for configuration.\n");
        return false;
    }

    // Iterate through responses and echo sets
    // The current hook (/assets, /features, /site)
    for (cJSON* cur_hook = pJsonRoot->child; cur_hook != NULL; cur_hook = cur_hook->next) {
        // Parse for the supported site controller setpoint types
        if (cur_hook->string && strcmp(cur_hook->string, "assets") != 0 && strcmp(cur_hook->string, "site") != 0 && strcmp(cur_hook->string, "features") != 0)
            // Other objects such as the id and version number will be included as well, so skip them
            continue;

        // It's acceptable to receive nothing, but if we receive a valid root the rest of the object should be present
        if (!cur_hook->child) {
            FPS_ERROR_LOG("Missing type level Data_Endpoint::setpoint_readin()\n");
            return false;
        }

        // Type level (Assets: ess/feeder/gen/solar, Features:active power/charge control, Site: operation/configuration)
        for (cJSON* cur_type = cur_hook->child; cur_type != NULL; cur_type = cur_type->next) {
            // It's acceptable to receive nothing, but if we receive a valid root the rest of the object should be present
            if (!cur_type->child) {
                FPS_ERROR_LOG("Missing instance or variable level Data_Endpoint::setpoint_readin()\n");
                return false;
            }

            // /assets case
            if (strcmp(cur_hook->string, "assets") == 0) {
                // Assets require an additional level of parsing for the Asset instance (ess_1, ess_2, ... feed_1, etc)
                for (cJSON* cur_instance = cur_type->child; cur_instance != NULL; cur_instance = cur_instance->next) {
                    // It's acceptable to receive nothing, but if we receive a valid root the rest of the object should be present
                    if (!cur_instance->child) {
                        FPS_ERROR_LOG("Missing asset variable level Data_Endpoint::setpoint_readin()\n");
                        return false;
                    }

                    // First handle maintenance mode so we can then support any variables dependent on it
                    cJSON* maint_mode_obj = cJSON_GetObjectItem(cur_instance, "maint_mode");
                    if (maint_mode_obj != NULL) {
                        fims_message* msg = construct_fims_message(maint_mode_obj, "set", 4, cur_hook->string, cur_type->string, cur_instance->string, maint_mode_obj->string);
                        if (msg == NULL)
                            return false;

                        // Send the response to Asset_Manager mocking a fims set
                        assetMgr->fims_data_parse(msg);
                        free_fims_msg(msg);
                    }

                    // All other setpoints dependent on maint_mode will now be supported.
                    // Send all remaining sets. Any dependency checks will be handled by Asset_<type>::process_set()

                    // Variable level (e.g. active_power_setpoint)
                    for (cJSON* variable = cur_instance->child; variable != NULL; variable = variable->next) {
                        // Already handled these setpoints, skip to the next iteration of the loop
                        if (strcmp(variable->string, "maint_mode") == 0)
                            continue;

                        fims_message* msg = construct_fims_message(variable, "set", 4, cur_hook->string, cur_type->string, cur_instance->string, variable->string);
                        if (msg == NULL)
                            return false;

                        // Send the response to Asset_Manager mocking a fims set
                        assetMgr->fims_data_parse(msg);
                        free_fims_msg(msg);
                    }
                }
            }
            // /site and /features case
            else {
                // Variable level
                for (cJSON* variable = cur_type->child; variable != NULL; variable = variable->next) {
                    fims_message* msg = construct_fims_message(variable, "set", 3, cur_hook->string, cur_type->string, variable->string, "");
                    if (msg == NULL)
                        return false;

                    // If the site start command was received
                    if (strcmp(cur_hook->string, "site") == 0 && strcmp(cur_type->string, "operation") == 0 && (strncmp(variable->string, "enable_flag", strlen("enable_flag")) == 0 || strcmp(variable->string, "disable_flag") == 0)) {
                        // Wait to send the command in a new thread so the system can finish configuring first
                        std::thread site_starter(delayed_set, msg);
                        site_starter.detach();
                    } else {
                        // Send the response to Site_Manager mocking a fims set
                        siteMgr->fims_data_parse(msg);
                        free_fims_msg(msg);
                    }
                }
            }
        }
    }

    cJSON_Delete(pJsonRoot);
    return true;
}

/**
 * Construct fims uris pointing to dbi for all associated setpoints contained in the map
 * @param uri The base uri of the setpoint
 * @param endpoint The endpoint of the uri e.g. the logical setting
 * @return The full list of setpoints to send to dbi
 */
std::vector<std::string> Data_Endpoint::construct_writeout_uris(std::string uri, std::string endpoint) {
    // Base URIs (modules)
    std::string dbi_uri = "/dbi/site_controller/setpoints";
    // URIs to be returned
    std::vector<std::string> constructed_uris;

    // If the URI contains a default value for an opposite pair negate the value of its opposite instead
    // In other words, the default value of site's state is stopped, so if we receive stop, it implies that start was entered at some point
    //     Therefore, instead of storing both enable_flag and disable_flag in dbi, simply "remove" the existing enable_flag entry
    // Otherwise, if no match, write out normally.
    // Other examples: breaker_open -> negate breaker_close
    // breaker_close_permissive_remove -> negate breaker_close_permissive
    // This will effectively remove the setpoint from DBI, resetting the status register to its default value
    std::unordered_map<std::string, std::vector<std::string>>::iterator opposite_pair = opposite_setpoints.find(endpoint);
    // Found an associated list of one or more opposites and should invalidate them
    if (opposite_pair != opposite_setpoints.end()) {
        for (size_t i = 0; i < opposite_pair->second.size(); ++i) {
            // Replace the endpoint in the uri: replace(<starting position>, <length>, <value>)
            // e.g. /assets/ess/ess_1/stop -> /assets/ess/ess_1/start
            std::string uri_copy = uri;
            uri_copy.replace(uri.find(opposite_pair->first), opposite_pair->first.length(), opposite_pair->second[i]);
            // Add the modules base uri
            // Add the appropriate strings to the return array
            constructed_uris.push_back(dbi_uri + uri_copy);
        }
    }

    return constructed_uris;
}

/**
 * Construct a fims_message from the parameters provided, returning a pointer to the allocated object
 * Caller is responsible for memory management (free)
 */
fims_message* Data_Endpoint::construct_fims_message(cJSON* variable, std::string method, int nfrags, std::string pfrags_0, std::string pfrags_1, std::string pfrags_2, std::string pfrags_3) {
    fims_message* msg = new fims_message();
    // Setpoints from dbi are naked, clothe them
    char* json_body = cJSON_PrintUnformatted(variable);
    msg->body = strdup(json_body);
    free(json_body);
    // Construct the uri fragments
    msg->nfrags = nfrags;
    msg->pfrags = new char*[msg->nfrags];
    // Construct based on the number of fragments
    if (nfrags == 4) {
        // In the case of 4, start adding at index 3, and add the previous fragment for index 2
        msg->pfrags[3] = strdup(pfrags_3.c_str());
        msg->pfrags[2] = strdup(std::string(pfrags_2 + "/" + pfrags_3).c_str());
    } else if (nfrags == 3) {
        // In the case of 3, start adding at index 2 (no other fragments to add)
        msg->pfrags[2] = strdup(pfrags_2.c_str());
    } else {
        FPS_ERROR_LOG("Received unsupported number of fragments: %d, Data_Endpoint::construct_fims_message()\n", nfrags);
        return NULL;
    }
    // Standard for all
    msg->pfrags[1] = strdup(std::string(pfrags_1 + "/" + msg->pfrags[2]).c_str());
    msg->pfrags[0] = strdup(std::string(pfrags_0 + "/" + msg->pfrags[1]).c_str());

    // Complete the message
    msg->uri = strdup(std::string(std::string("/") + msg->pfrags[0]).c_str());
    msg->method = strdup(method.c_str());
    return msg;
}

// turn_off_start_cmd sends the value 'false' to the endpoint '/dbi/site_controller/setpoints/enable_flag'.
// The msg is sent when the site is shutdown so that if Site Controller tries to restart, it does not try to start
// the site.
void Data_Endpoint::turn_off_start_cmd(void) {
    try {
        // Clear buffer for use
        send_FIMS_buf.clear();

        // make json object
        bufJSON_StartObject(send_FIMS_buf);  // object {
        // add 'false' to the object, which represents turning the start cmd "off"
        bufJSON_AddBool(send_FIMS_buf, "value", false);
        bufJSON_EndObject(send_FIMS_buf);  // } object

        bufJSON_RemoveTrailingComma(send_FIMS_buf);
        std::string body = to_string(send_FIMS_buf);

        // send to the dbi endpoint for the start cmd
        p_fims->Send("set", "/dbi/site_controller/setpoints/site/operation/enable_flag", NULL, body.c_str());
    } catch (const char* err) {
        FPS_ERROR_LOG("Site_Manager::shutdown_state() - %s\n", err);
    }
}
