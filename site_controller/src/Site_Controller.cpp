/* C Standard Library Dependencies */
#include <cstring>
#include <csignal>
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
/* Local Internal Dependencies */
#include <Data_Endpoint.h>
#include <Asset_Manager.h>
#include <Site_Manager.h>
#include <version.h>

#ifdef FPS_TEST_MODE
#include "hybridos_test.h"
#endif

fims *p_fims = NULL;
volatile bool running = true;
Data_Endpoint* data_endpoint;
// TODO: is this an acceptable place to expose these?
//      It makes it much more convenient for calling Data_Endpoint::setpoint_readin() in Site_Manager,
//      as the Asset_Manager and Site_Manager parameters no longer need to be passed
Asset_Manager *assetMgr;
Site_Manager *siteMgr;

void signal_handler (int sig)
{
    running = false;
    FPS_WARNING_LOG("signal of type %d caught.\n", sig);
    signal(sig, SIG_DFL);
}

/*******************************************************************************************************************************************************************/
/**
 * Parses the config cJSON from the file provided by the filepath
 */
cJSON *parseJSONConfig(char *file_path)
{
    FPS_INFO_LOG("Site_Controller reading configuration from files\n");
    //Open a file//
    if (file_path == NULL)
    {
        FPS_ERROR_LOG(" Failed to get the path of the test file. \n"); //a check to make sure that args[1] is not NULL//
        return NULL;
    }
    FILE* fp = fopen(file_path, "r");
    if (fp == NULL)
    {
        FPS_ERROR_LOG("Failed to open file %s\n", file_path);
        return NULL;
    }
    else 
    {
        FPS_DEBUG_LOG("Opened file %s\n", file_path);
    }
    
    // obtain file size
    fseek(fp, 0, SEEK_END);
    long unsigned file_size = ftell(fp);
    rewind(fp);
    
    // create configuration file
    char* configFile = new char[file_size];
    if (configFile == NULL)
    {
        FPS_ERROR_LOG("Memory allocation error config file\n");
        fclose(fp);
        return NULL;
    }
    size_t bytes_read = fread(configFile, 1, file_size, fp);
    if (bytes_read != file_size)
    {
        FPS_ERROR_LOG("Read error: read: %lu, file size: %lu .\n", (unsigned long)bytes_read, (unsigned long)file_size);
        fclose(fp);
        delete[] configFile;
        return NULL;
    }
    else
    {
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
cJSON *parseJSONConfig(fims *p_fims, std::string config_type)
{
    FPS_INFO_LOG("Site_Controller %s reading configuration from dbi\n", config_type.c_str());
    if (p_fims == NULL)
    {
        FPS_ERROR_LOG("Fims not configured\n");
        return NULL;
    }

    if (config_type.empty())
    {
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
    if (pJsonRoot == NULL)
    {
        FPS_ERROR_LOG("Site_Controller received NULL response for configuration.\n");
        cJSON_Delete(pJsonRoot);
        // Ensure that we return NULL and not the uninitialized pJsonRoot
        return NULL;
    }

    return pJsonRoot;
}

/****************************************************************************************************************/
void fims_data_pump(fims *p_fims, Site_Manager * siteMgr, Asset_Manager * assetMgr)
{
    fims_message *pmsg;
    timespec entry_time, exit_time;
    int perioduS;
    int asset_publish_rate_cntr = 0;

    while (running == true && p_fims->Connected())
    {
        /* timeoutDerate is used to keep the effective state machine interval at the desired actual time
         as defined by STATE_MACHINE_INTERVAL_MS */
        perioduS = 0;
        clock_gettime(CLOCK_MONOTONIC, &entry_time);
        do
        {
            pmsg = p_fims->Receive_Timeout((STATE_MACHINE_INTERVAL_uS) - perioduS); // blocking with timeout in uSec.
            if (pmsg != NULL)
            {
                if (pmsg->nfrags > 0)
                {
                    if ((strncmp(pmsg->pfrags[0],"features", strlen("features")) == 0)  || (strncmp(pmsg->pfrags[0],"site", strlen("site")) == 0))
                    {
                        siteMgr->fims_data_parse(pmsg);
                    }
                    else if ((strncmp(pmsg->pfrags[0],"components", strlen("components")) == 0)  || (strncmp(pmsg->pfrags[0],"assets", strlen("assets")) == 0))
                    {
                        assetMgr->fims_data_parse(pmsg);
                    }
                }
                else
                {
                    FPS_WARNING_LOG("Uri error in site manager message parse nfrags %d .\n", pmsg->nfrags);
                }
                p_fims->free_message(pmsg);
            }

            clock_gettime(CLOCK_MONOTONIC, &exit_time);
            perioduS = (exit_time.tv_sec - entry_time.tv_sec) * 1000000;
            perioduS += (exit_time.tv_nsec - entry_time.tv_nsec) / 1000;
        } while (running == true && p_fims->Connected() && perioduS < STATE_MACHINE_INTERVAL_uS);

        /* test for actual loop delay, which we want to be close to STATE_MACHINE_INTERVAL_uS/1000 (ms)
         * begin control loop processing - we got here either with a timeout of Receive_Timout
         * or by receiving a message from fims.
         */
        if (running == true && p_fims->Connected())
        {
            assetMgr->process_asset_data(); // process and aggregate data based component status
            siteMgr->process_state();       // allow site manager to evaluate site state and dispatch power
            assetMgr->update_asset_data();  // update asset data based on site manager targets
            assetMgr->send_to_components(); // send updated control setpoints data to components
            if (asset_publish_rate_cntr++ > (ASSET_PUBLISH_RATE_uS / STATE_MACHINE_INTERVAL_uS))
            {
               asset_publish_rate_cntr = 0;
               assetMgr->publish_assets();
               siteMgr->publish_all_FIMS();
            }
        }
    }
}

int main(int argc, char **argv)
{
    #ifdef FPS_TEST_MODE
    return test_main(argc, argv);
    #endif

    (void) argc;

    // Init Logger
    StaticLogs::Logger::Init("site_controller");

    // Command line argument determining configuration source
    // argument "storage" indicates reading from storage, else the config file path should be given
    // arguments configured in mcp/config/mcp_site_controller.json
    bool read_from_storage = false;

    char sequencesFilePath[256];
    char variablesFilePath[256];
    char assetsFilePath[256];

    // Command line argument giving the configuration source "dbi", indicating to read from storage
    if (strcmp(argv[1], "dbi") == 0)
    {
        read_from_storage = true;
    }
    // Assumed a valid file path was given. Try to read and error below if invalid
    else
    {
        // get path to sequences file
        snprintf(sequencesFilePath, 256, "%s/sequences.json", argv[1]);
        // get path to variables file
        snprintf(variablesFilePath, 256, "%s/variables.json", argv[1]);
        // get path to "assets.json"
        snprintf(assetsFilePath, 256, "%s/assets.json", argv[1]);
    }

    char *subscriptions[NUM_CONTROLLER_SUBS];
    int num_subs = NUM_CONTROLLER_SUBS;
    cJSON* jSonRoot;
    cJSON* sequenceRoot;
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

    if ((p_fims = new fims()) == NULL)
    {
        FPS_ERROR_LOG("Failed to initialize fims class.\n");
        return (1);
    }

    if ( !p_fims->Connect((char *)"site_controller") )
    {
        FPS_ERROR_LOG("Failed to connect to fims server.\n");
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
    if (p_fims->Subscribe((const char**)subscriptions, num_subs))
    {
        FPS_DEBUG_LOG("p_fims->Subscribe() succeeded, initializing classes\n");
        if ((assetMgr = new Asset_Manager()) == NULL)
        {
            FPS_ERROR_LOG("Failed to initialize Asset Manager class.\n");
            rtn_val = 1;
            goto cleanup;
        }
        if ((siteMgr = new Site_Manager(version)) == NULL)
        {
            FPS_ERROR_LOG("Failed to initialize State Manager class.\n");
            rtn_val = 1;
            goto cleanup;
        }
        // Parsing dependent on the configuration type (storage/file)
        if ( (read_from_storage && (jSonRoot = parseJSONConfig(p_fims, "assets")) == NULL) ||
             (!read_from_storage && (jSonRoot = parseJSONConfig(assetsFilePath)) == NULL) )
        {
            FPS_ERROR_LOG("Failed to create JSON map.\n");
            rtn_val = 1;
            goto cleanup;
        }
        if (!assetMgr->asset_create(jSonRoot, primary_controller))
        {
            FPS_ERROR_LOG("Failed to create asset instance(s).\n");
            cJSON_Delete(jSonRoot);
            rtn_val = 1;
            goto cleanup;
        }
        cJSON_Delete(jSonRoot);
        //call site_manager.configure to pass asset and fims pointers to site manager
        // Parsing dependent on the configuration type (storage/file)
        sequenceRoot = (read_from_storage) ? parseJSONConfig(p_fims, "sequences") : parseJSONConfig(sequencesFilePath);
        varRoot = (read_from_storage) ? parseJSONConfig(p_fims, "variables") : parseJSONConfig(variablesFilePath);
        rtn_val_bool = siteMgr->configure(assetMgr, p_fims, sequenceRoot, varRoot, primary_controller);
        cJSON_Delete(sequenceRoot);
        cJSON_Delete(varRoot);
        if (rtn_val_bool == false)
        {
            FPS_ERROR_LOG("Site Manager Failed to configure properly.\n");
            rtn_val = 1;
        }
        else
        {
            // Main configuration has completed, now read in setpoints from last instance
            if (!(data_endpoint->setpoint_readin()))
            {
                FPS_ERROR_LOG("HybridOS failed to read in user setpoints.\n");
                rtn_val = 1;
                goto cleanup;
            }
            fims_data_pump(p_fims, siteMgr, assetMgr);// if all goes well we stay in the loop in fims_data_pump()
        }
    }
    else
        FPS_ERROR_LOG("(p_fims->Subscribe() failed\n");

    cleanup:
        FPS_ERROR_LOG("Flushing on Cleanup.");
        FPS_DEBUG_LOG("\nLeaving HybridOS_Control\n");
        delete primary_controller;
        delete data_endpoint;
        p_fims->Close();
        delete p_fims;
        for (int i=0; i < num_subs; i++)
            if (subscriptions[i] != NULL) free(subscriptions[i]);
        delete assetMgr;
        delete siteMgr;
        delete version;
        return rtn_val;
}
