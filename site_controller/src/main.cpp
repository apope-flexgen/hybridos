/* C Standard Library Dependencies */
#include <cstring>
#include <csignal>
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
#include <Site_Controller_Utils.h>
/* Local Internal Dependencies */
#include <Data_Endpoint.h>
#include <Asset_Manager.h>
#include <Site_Manager.h>
#include <version.h>
#include "Types.h"

#ifdef FPS_TEST_MODE
#include "hybridos_test.h"
#endif

fims* p_fims = NULL;
volatile bool running = true;
Data_Endpoint* data_endpoint;
// TODO: is this an acceptable place to expose these?
//      It makes it much more convenient for calling Data_Endpoint::setpoint_readin() in Site_Manager,
//      as the Asset_Manager and Site_Manager parameters no longer need to be passed
Asset_Manager* assetMgr;
Site_Manager* siteMgr;

void signal_handler(int sig) {
    running = false;
    FPS_WARNING_LOG("signal of type %d caught.\n", sig);
    signal(sig, SIG_DFL);
}

/*******************************************************************************************************************************************************************/
/**
 * Parses the config cJSON from the file provided by the filepath
 */
cJSON* parseJSONConfig(char* file_path) {
    FPS_INFO_LOG("Site_Controller reading configuration from files\n");
    // Open a file//
    if (file_path == NULL) {
        FPS_ERROR_LOG(" Failed to get the path of the test file. \n");  // a check to make sure that args[1] is not NULL//
        return NULL;
    }
    FILE* fp = fopen(file_path, "r");
    if (fp == NULL) {
        FPS_ERROR_LOG("Failed to open file %s\n", file_path);
        return NULL;
    } else {
        FPS_DEBUG_LOG("Opened file %s\n", file_path);
    }

    // obtain file size
    fseek(fp, 0, SEEK_END);
    long unsigned file_size = ftell(fp);
    rewind(fp);

    // create configuration file
    char* configFile = new char[file_size];
    if (configFile == NULL) {
        FPS_ERROR_LOG("Memory allocation error config file\n");
        fclose(fp);
        return NULL;
    }
    size_t bytes_read = fread(configFile, 1, file_size, fp);
    if (bytes_read != file_size) {
        FPS_ERROR_LOG("Read error: read: %lu, file size: %lu .\n", (unsigned long)bytes_read, (unsigned long)file_size);
        fclose(fp);
        delete[] configFile;
        return NULL;
    } else {
        FPS_DEBUG_LOG("File size %lu\n", file_size);
    }

    fclose(fp);
    cJSON* pJsonRoot = cJSON_Parse(configFile);
    delete[] configFile;
    if (pJsonRoot == NULL)
        FPS_ERROR_LOG("cJSON_Parse returned NULL.\n");
    return (pJsonRoot);
}

/*******************************************************************************************************************************************************************/
/**
 * Parses the config cJSON from storage given the configuration type (Assets/Sequences/Variables)
 */
cJSON* parseJSONConfig(fims* p_fims, std::string config_type) {
    FPS_INFO_LOG("Site_Controller %s reading configuration from dbi\n", config_type.c_str());
    if (p_fims == NULL) {
        FPS_ERROR_LOG("Fims not configured\n");
        return NULL;
    }

    if (config_type.empty()) {
        FPS_ERROR_LOG("Invalid configuration type received\n");
        return NULL;
    }

    // construct the request uris
    std::string uri = "/dbi/site_controller/" + config_type;
    std::string replyto = "/site_controller/" + config_type;

    // Pass off to Data_Endpoint to get response from storage
    char* response_body = data_endpoint->get_from_uri(uri, replyto);

    // Parse the JSON response
    cJSON* pJsonRoot = cJSON_Parse(response_body);
    free(response_body);
    if (pJsonRoot == NULL) {
        FPS_ERROR_LOG("Site_Controller received NULL response for configuration.\n");
        cJSON_Delete(pJsonRoot);
        // Ensure that we return NULL and not the uninitialized pJsonRoot
        return NULL;
    }

    return pJsonRoot;
}

/****************************************************************************************************************/
void fims_data_pump(fims* p_fims, Site_Manager* siteMgr, Asset_Manager* assetMgr) {
    fims_message* pmsg;
    timespec last_loop_time, current_loop_time;
    uint since_last_update_uS = 0;
    uint since_last_publish_uS = 0;

    clock_gettime(CLOCK_MONOTONIC, &current_loop_time);

    while (running == true && p_fims->Connected()) {
        // update time variables
        last_loop_time = current_loop_time;
        clock_gettime(CLOCK_MONOTONIC, &current_loop_time);
        uint since_last_loop_uS = (current_loop_time.tv_sec - last_loop_time.tv_sec) * 1000000;
        since_last_loop_uS += (current_loop_time.tv_nsec - last_loop_time.tv_nsec) / 1000;

        since_last_publish_uS += since_last_loop_uS;
        since_last_update_uS += since_last_loop_uS;
        // negative value indicates that we should publish or update
        int until_next_publish_or_update_uS = std::min(ASSET_PUBLISH_RATE_uS - int(since_last_publish_uS), STATE_MACHINE_INTERVAL_uS - int(since_last_update_uS));

        // process incoming fims messages in time between publishes and state updates
        // run this on initial "frame" aka until_next_publish_or_update_uS = 0
        if (until_next_publish_or_update_uS > 0 && running == true && p_fims->Connected()) {
            pmsg = p_fims->Receive_Timeout(until_next_publish_or_update_uS);  // blocking with timeout in uSec.
            if (pmsg != NULL) {
                if (pmsg->nfrags > 0) {
                    if ((strncmp(pmsg->pfrags[0], "features", strlen("features")) == 0) || (strncmp(pmsg->pfrags[0], "site", strlen("site")) == 0)) {
                        siteMgr->fims_data_parse(pmsg);
                    } else if ((strncmp(pmsg->pfrags[0], "components", strlen("components")) == 0) || (strncmp(pmsg->pfrags[0], "assets", strlen("assets")) == 0)) {
                        assetMgr->fims_data_parse(pmsg);
                    }
                } else {
                    FPS_WARNING_LOG("Uri error in site manager message parse nfrags %d .\n", pmsg->nfrags);
                }
                p_fims->free_message(pmsg);
            }
        }

        // publish if publish period has passed
        if (since_last_publish_uS > ASSET_PUBLISH_RATE_uS && running == true && p_fims->Connected()) {
            assetMgr->publish_assets();
            siteMgr->publish_all_FIMS();
            since_last_publish_uS %= ASSET_PUBLISH_RATE_uS;
        }

        // update state if update period has passed
        if (since_last_update_uS > STATE_MACHINE_INTERVAL_uS && running == true && p_fims->Connected()) {
            assetMgr->process_asset_data();  // process and aggregate data based component status
            siteMgr->process_state();        // allow site manager to evaluate site state and dispatch power
            assetMgr->update_asset_data();   // update asset data based on site manager targets
            assetMgr->send_to_components();  // send updated control setpoints data to components
            since_last_update_uS %= STATE_MACHINE_INTERVAL_uS;
        }
    }
}

int main(int argc, char** argv) {
    // Init Logger, passing optional command line argument for config file path
    Logging::Init("site_controller", argc, argv);

#ifdef FPS_TEST_MODE
    return test_main(argc, argv);
#endif

    // Command line argument determining configuration source
    // argument "storage" indicates reading from storage, else the config file path should be given
    // arguments configured in mcp/config/mcp_site_controller.json
    bool read_from_storage = false;

    char sequencesFilePath[256];
    char actionsFilePath[256];
    char variablesFilePath[256];
    char assetsFilePath[256];

    // Command line argument giving the configuration source "dbi", indicating to read from storage
    if (strcmp(argv[1], "dbi") == 0) {
        read_from_storage = true;
    }
    // Assumed a valid file path was given. Try to read and error below if invalid
    else {
        // get path to sequences file
        snprintf(sequencesFilePath, 256, "%s/sequences.json", argv[1]);
        // get path to "actions.json"
        snprintf(actionsFilePath, 256, "%s/actions.json", argv[1]);
        // get path to variables file
        snprintf(variablesFilePath, 256, "%s/variables.json", argv[1]);
        // get path to "assets.json"
        snprintf(assetsFilePath, 256, "%s/assets.json", argv[1]);
    }

    char* subscriptions[NUM_CONTROLLER_SUBS];
    int num_subs = NUM_CONTROLLER_SUBS;
    cJSON* assetsRoot;
    cJSON* sequenceRoot;
    cJSON* actionsRoot;
    cJSON* varRoot;
    int rtn_val = 0;
    bool rtn_val_bool = false;
    bool* primary_controller = new bool(false);

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    data_endpoint = new Data_Endpoint();

    // access version info
    Version* version = new Version();
    version->init();
    printf("build: %s\n", version->get_build());
    printf("commit: %s\n", version->get_commit());
    printf("tag: %s\n", version->get_tag());

    if ((p_fims = new fims()) == NULL) {
        FPS_ERROR_LOG("Failed to initialize fims class.");
        emit_event("Site", "Failed to create internal communication", FAULT_ALERT);
        return 1;
    }

    if (!p_fims->Connect((char*)"site_controller")) {
        FPS_ERROR_LOG("Failed to connect to fims server.");
        emit_event("Site", "Failed to connect to internal communication", FAULT_ALERT);
        delete p_fims;
        return 1;
    }

    memset(subscriptions, 0, sizeof(char*) * NUM_CONTROLLER_SUBS);
    subscriptions[0] = strdup("/site");
    subscriptions[1] = strdup("/features");
    subscriptions[2] = strdup("/components");
    subscriptions[3] = strdup("/assets");
    subscriptions[4] = strdup("/site_controller");

    FPS_DEBUG_LOG("Calling Subscribe() for %d uri.\n", num_subs);
    if (p_fims->Subscribe((const char**)subscriptions, num_subs)) {
        FPS_DEBUG_LOG("p_fims->Subscribe() succeeded, initializing classes\n");
        if ((assetMgr = new Asset_Manager()) == NULL) {
            FPS_ERROR_LOG("Failed to initialize Asset Manager class.\n");
            rtn_val = 1;
            goto cleanup;
        }
        if ((siteMgr = new Site_Manager(version)) == NULL) {
            FPS_ERROR_LOG("Failed to initialize State Manager class.\n");
            rtn_val = 1;
            goto cleanup;
        }
        sequenceRoot = (read_from_storage) ? parseJSONConfig(p_fims, "sequences") : parseJSONConfig(sequencesFilePath);
        actionsRoot = (read_from_storage) ? parseJSONConfig(p_fims, "actions") : parseJSONConfig(actionsFilePath);
        varRoot = (read_from_storage) ? parseJSONConfig(p_fims, "variables") : parseJSONConfig(variablesFilePath);
        assetsRoot = (read_from_storage) ? parseJSONConfig(p_fims, "assets") : parseJSONConfig(assetsFilePath);

        // Parsing dependent on the configuration type (storage/file)
        if (assetsRoot == NULL) {
            FPS_ERROR_LOG("Failed to create JSON map.");
            emit_event("Site", "Failed to read assets from configuration", FAULT_ALERT);
            rtn_val = 1;
            goto cleanup;
        }
        if (!assetMgr->asset_create(assetsRoot, actionsRoot, primary_controller)) {
            FPS_ERROR_LOG("Failed to create asset instance(s).");
            emit_event("Site", "Failed to initialize assets from configuration", FAULT_ALERT);
            cJSON_Delete(assetsRoot);
            cJSON_Delete(actionsRoot);
            rtn_val = 1;
            goto cleanup;
        }
        // call site_manager.configure to pass asset and fims pointers to site manager
        // Parsing dependent on the configuration type (storage/file)
        rtn_val_bool = siteMgr->configure(assetMgr, p_fims, sequenceRoot, varRoot, primary_controller);

        // clean up cJSON calls
        cJSON_Delete(sequenceRoot);
        cJSON_Delete(varRoot);
        cJSON_Delete(assetsRoot);
        cJSON_Delete(actionsRoot);

        if (!rtn_val_bool) {
            FPS_ERROR_LOG("Site Manager Failed to configure properly.");
            emit_event("Site", "Failed to initialize system from configuration", FAULT_ALERT);
            rtn_val = 1;
        } else {
            // Main configuration has completed, now read in setpoints from last instance
            if (!(data_endpoint->setpoint_readin())) {
                FPS_ERROR_LOG("HybridOS failed to read in user setpoints.");
                rtn_val = 1;
                goto cleanup;
            }
            fims_data_pump(p_fims, siteMgr, assetMgr);  // if all goes well we stay in the loop in fims_data_pump()
        }
    } else {
        FPS_ERROR_LOG("(p_fims->Subscribe() failed.");
        emit_event("Site", "Failed to establish internal communication", FAULT_ALERT);
    }

cleanup:
    FPS_ERROR_LOG("Flushing on Cleanup.");
    FPS_DEBUG_LOG("Leaving HybridOS_Control");
    emit_event("Site", "System Exiting", FAULT_ALERT);
    delete primary_controller;
    delete data_endpoint;
    p_fims->Close();
    delete p_fims;
    for (int i = 0; i < num_subs; i++)
        if (subscriptions[i] != NULL)
            free(subscriptions[i]);
    delete assetMgr;
    delete siteMgr;
    delete version;
    return rtn_val;
}
