/*
 * Path.cpp
 *
 *  Created on: Nov 5, 2019
 *      Author: kbrezina
 */

/* C Standard Library Dependencies */
#include <cstring>
/* C++ Standard Library Dependencies */
#include <string>
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
/* Local Internal Dependencies */
#include <Path.h>
#include <Site_Controller_Utils.h>

Path::Path(Site_Manager* siteref)
{
    pSite = siteref;
    path_name = NULL;
    return_id = NULL;
    steps = NULL;
    return_state = Error;
    steps_size = 0;
    timeout = 0;
    num_active_faults = 0;
    num_active_alarms = 0;
}

Path::~Path()
{
    if (steps != NULL)
        delete [] steps;
    if (path_name != NULL)
        free(path_name);
    if (return_id != NULL)
        delete [] return_id;
}

const char* Path::get_name()
{
    return path_name;
}

Step* Path::get_step(int n)
{
    if (n < steps_size)
        return &(steps[n]);
    return NULL;
}

int Path::get_steps_size()
{
    return steps_size;
}

states Path::get_return_state()
{
    return return_state;
}

/**
 * Checks either alarms or faults depending on the alert type passed.
 * @param alertType indicates checking alarms or checking faults.
 * @return True if there are alerts of the given type, or false if there are none.
 */
bool Path::check_alerts(UI_Type alertType)
{
    // only UI Types that are alerts are ALARM and FAULT
    if (alertType != ALARM && alertType != FAULT)
    {
        FPS_ERROR_LOG("Function check_alerts(UI_Type alertType) only accepts ALARM or FAULT as an argument. Returning false...\n");
        return false;
    }

    // count how many active alerts there are
    int num_active_alerts = 0;

    for( auto name_mask_pair : (alertType == FAULT ? faults : alarms) )
    {
        // allocate memory for an event message to be sent if alert detected
        char event_message [60] = {};

        // split asset/fault name into fragments separated by "/"s
        std::vector<std::string> name_fragments = split(name_mask_pair.first, "/");

        // no alert to check for in this path
        if (name_fragments[0] == "bypass")
            continue;
        //debug test fault "test_fault"
        else if (alertType == FAULT && name_fragments[0] == "test_fault")
        {
            num_active_alerts++;
            // only emit event if new
            if (!pSite->get_active_faults(0))
            {
                FPS_ERROR_LOG("test fault detected: %s \n", name_fragments[0].c_str());
                sprintf(event_message, "Site Manager %s test fault detected", name_fragments[0].c_str());
                emit_event("Site", event_message, 1);
                pSite->set_faults(0);
            }
        }
        //debug test alarm "test_alarm"
        else if (alertType == ALARM && name_fragments[0] == "test_alarm")
        {
            num_active_alerts++;
            // only emit event if new
            if (!pSite->get_active_alarms(0))
            {
                FPS_ERROR_LOG("test alarm detected: %s \n", name_fragments[0].c_str());
                sprintf(event_message, "Site Manager %s test alarm detected", name_fragments[0].c_str());
                emit_event("Site", event_message, 1);
                pSite->set_alarms(0);
            }
        }
        // The following all use site fault 1 or site alarm 1, depending on alert type being checked
        else if (name_fragments.size() >= 2)
        {
            std::string asset_cmd = name_fragments[1];
            //asset faults/alarms
            if (name_fragments[0] == "assets")
            {
                //aggregate ESS fault check (true if any faults)
                if (asset_cmd == "get_any_ess_faults")
                {
                    if (pSite->pAssets->get_num_active_faults(ESS) > 0)
                    {
                        num_active_alerts++;
                        if (alertType == FAULT ? !pSite->get_active_faults(1) : !pSite->get_active_alarms(1))
                        {
                            // print and raise alarm/fault based on type
                            FPS_ERROR_LOG("get_any_ess_faults %s detected: %s \n", alertType == FAULT ? "fault" : "alarm", name_mask_pair.first.c_str());
                            sprintf(event_message, "Site Manager ESS %s detected: Get Any ESS Faults", alertType == FAULT ? "fault" : "alarm");
                            emit_event("Site", event_message, 1);
                            (alertType == FAULT) ? pSite->set_faults(1) : pSite->set_alarms(1);
                        }
                    }
                }
                //aggregate ESS fault check (true if any faults)
                else if (asset_cmd == "get_any_ess_alarms")
                {
                    if (pSite->pAssets->get_num_active_alarms(ESS) > 0)
                    {
                        num_active_alerts++;
                        if (alertType == FAULT ? !pSite->get_active_faults(1) : !pSite->get_active_alarms(1))
                        {
                            // print and raise alarm/fault based on type
                            FPS_ERROR_LOG("get_any_ess_alarms %s detected: %s \n", alertType == FAULT ? "fault" : "alarm", name_mask_pair.first.c_str());
                            sprintf(event_message, "Site Manager ESS %s detected: Get Any ESS Alarms", alertType == FAULT ? "fault" : "alarm");
                            emit_event("Site", event_message, 1);
                            (alertType == FAULT) ? pSite->set_faults(1) : pSite->set_alarms(1);
                        }
                    }
                }
                //check for number of ess running.  compare read value to first element and fault if less/equal
                // In this case asset_active_faults value is used as the expected number
                else if (asset_cmd == "get_num_ess_running")
                {
                    // asset the number of expected running based on type
                    if ( (int) name_mask_pair.second >= pSite->pAssets->get_num_ess_running())
                    {
                        num_active_alerts++;
                        if (alertType == FAULT ? !pSite->get_active_faults(1) : !pSite->get_active_alarms(1))
                        {
                            FPS_ERROR_LOG("get_num_ess_running %s detected: %s \n",  alertType == FAULT ? "fault" : "alarm", name_mask_pair.first.c_str());
                            sprintf(event_message, "Site Manager #ESS running %s detected", alertType == FAULT ? "fault" : "alarm");
                            emit_event("Site", event_message, 1);
                            (alertType == FAULT) ? pSite->set_faults(1) : pSite->set_alarms(1);
                        }
                    }
                }
                //check for number of ess available.  compare read value to first element and fault if less/equal
                // In this case asset_active_faults value is used as the expected number
                else if (asset_cmd == "get_num_ess_available")
                {
                    // asset the number of expected available based on type
                    if ( (int) name_mask_pair.second >= pSite->pAssets->get_num_ess_avail())
                    {
                        num_active_alerts++;
                        if (alertType == FAULT ? !pSite->get_active_faults(1) : !pSite->get_active_alarms(1))
                        {
                            FPS_ERROR_LOG("get_num_ess_available %s detected: %s \n", alertType == FAULT ? "fault" : "alarm", name_mask_pair.first.c_str());
                            sprintf(event_message, "Site Manager #ESS available %s detected", alertType == FAULT ? "fault" : "alarm");
                            emit_event("Site", event_message, 1);
                            (alertType == FAULT) ? pSite->set_faults(1) : pSite->set_alarms(1);
                        }
                    }
                }
                // check for number of ess controllable.  compare read value to first element and fault if less/equal
                // In this case asset_active_faults value is used as the expected number
                else if (asset_cmd == "get_num_ess_controllable")
                {
                    // assert the number of expected controllable based on type
                    if ( (int) name_mask_pair.second >= pSite->pAssets->get_num_ess_controllable())
                    {
                        num_active_alerts++;
                        if (alertType == FAULT ? !pSite->get_active_faults(1) : !pSite->get_active_alarms(1))
                        {
                            FPS_ERROR_LOG("get_num_ess_controllable %s detected: %s \n", alertType == FAULT ? "fault" : "alarm", name_mask_pair.first.c_str());
                            sprintf(event_message, "Site Manager #ESS controllable %s detected", alertType == FAULT ? "fault" : "alarm");
                            emit_event("Site", event_message, 1);
                            (alertType == FAULT) ? pSite->set_faults(1) : pSite->set_alarms(1);
                        }
                    }
                }
                //check for number of solar, ess, gen controllable.  if none controllable, fault
                // In this case asset_active_faults value is used as the expected number
                else if (asset_cmd == "get_num_assets_controllable")
                {
                    // asset the number of expected controllable based on type
                    //TODO: address use of "get_num_gen_active" function if necessary
                    if (pSite->pAssets->get_num_ess_controllable() == 0 && pSite->pAssets->get_num_solar_controllable() == 0 && pSite->pAssets->get_num_gen_controllable() == 0)
                    {
                        num_active_alerts++;
                        if (alertType == FAULT ? !pSite->get_active_faults(1) : !pSite->get_active_alarms(1))
                        {
                            FPS_ERROR_LOG("get_num_assets_controllable %s detected: %s \n", alertType == FAULT ? "fault" : "alarm", name_mask_pair.first.c_str());
                            sprintf(event_message, "Site Manager #Assets Controllable %s detected", alertType == FAULT ? "fault" : "alarm");
                            emit_event("Site", event_message, 1);
                            (alertType == FAULT) ? pSite->set_faults(1) : pSite->set_alarms(1);
                        }
                    }
                }
                // if none of the previous names matched, then only remaining matches are of the format /assets/<asset type>/<asset id>/<alert id>
                else
                {
                    if (pSite->pAssets->check_asset_alert(name_mask_pair))
                    {
                        num_active_alerts++;
                        if (alertType == FAULT ? !pSite->get_active_faults(1) : !pSite->get_active_alarms(1))
                        {
                            FPS_ERROR_LOG("Asset %s detected: %s\n", alertType == FAULT ? "fault" : "alarm", name_mask_pair.first.c_str());
                            sprintf(event_message, "Site Manager Asset %s detected: %s", alertType == FAULT ? "fault" : "alarm", name_mask_pair.first.c_str());
                            emit_event("Site", event_message, 1);
                            (alertType == FAULT) ? pSite->set_faults(1) : pSite->set_alarms(1);
                        }
                    }
                }
            }
            else if (asset_cmd == "get_feeder_on_array")
            {
                Asset_Feeder *feeder_instance = pSite->pAssets->validate_feeder_id(name_fragments[0].c_str());
                if (feeder_instance == nullptr) {
                    FPS_ERROR_LOG("feeder ID: %s was not found in the assets list\n", name_fragments[0].c_str());
                }
                //if first element is 0, then alert if feeder_state is false (alert on open breaker)
                else if (name_mask_pair.second == 0 && !pSite->pAssets->get_feeder_state(feeder_instance))
                {
                    num_active_alerts++;
                    if (alertType == FAULT ? !pSite->get_active_faults(1) : !pSite->get_active_alarms(1))
                    {
                        FPS_ERROR_LOG("get_feeder_on_array %s detected: %s \n", alertType == FAULT ? "fault" : "alarm", name_mask_pair.first.c_str());
                        sprintf(event_message, "Site Manager %s open feeder %s detected", name_fragments[0].c_str(), alertType == FAULT ? "fault" : "alarm");
                        emit_event("Site", event_message, 1);
                        (alertType == FAULT) ? pSite->set_faults(1) : pSite->set_alarms(1);
                    }
                }
                //if first element is 1, then alert if feeder_state is true (alert on closed breaker)
                else if ( name_mask_pair.second == 1 && pSite->pAssets->get_feeder_state(feeder_instance))
                {
                    num_active_alerts++;
                    if (alertType == FAULT ? !pSite->get_active_faults(1) : !pSite->get_active_alarms(1))
                    {
                        FPS_ERROR_LOG("get_feeder_on_array %s detected: %s \n", alertType == FAULT ? "fault" : "alarm", name_mask_pair.first.c_str());
                        sprintf(event_message, "Site Manager %s closed feeder %s detected", name_fragments[0].c_str(), alertType == FAULT ? "fault" : "alarm");
                        emit_event("Site", event_message, 1);
                        (alertType == FAULT) ? pSite->set_faults(1) : pSite->set_alarms(1);
                    }
                }
            }
        }
        // alert may want to refer to one of the reserved_bool_# or reserved_float_# variables in Site Manager since an on-site component can be configured to send a warning value to one of those variables
        else if(strncmp(name_fragments[0].c_str(), "reserved_", strlen("reserved_")) == 0)
        {
            // parse data identifying which variable is being referenced
            std::string var_name = name_fragments[0];
            std::vector<std::string> var_name_fragments = split(var_name, "_");
            if (var_name_fragments.size() < 3)
            {
                FPS_ERROR_LOG("Reserved variable name is not valid: %s. Expected format is reserved_<varType>_<varIndex>.\n", var_name.c_str() );
                continue;
            }
            std::string var_type = var_name_fragments[1];
            std::string var_index = var_name_fragments[2];

            // check the referenced variable
            if(var_type == "bool")
            {
                int bool_index; // Need to declare n outside of try block so switch statement can access it afterwards
                try
                {
                    bool_index = std::stoi(var_index); // Determine what # is in reserved_bool_#
                }
                catch(const std::exception& e)
                {   // std::stoi can throw an error if value in string is not within limits of int (such as if there are characters in there that aren't numbers)
                    bool_index = 0;
                    FPS_ERROR_LOG("Invalid # in reserved_bool_# in sequences.json active faults list: %s is invalid\n", var_index.c_str());
                }

                if(bool_index != 0)
                {
                    int checkVal = 0;
                    switch (bool_index)
                    {   // sequences.json can't store "true" or "false" in asset_active_faults, so 0 stands for false and 1 stands for true
                        case 1:  checkVal = pSite->get_reserved_bool_1()  ? 1 : 0; break;
                        case 2:  checkVal = pSite->get_reserved_bool_2()  ? 1 : 0; break;
                        case 3:  checkVal = pSite->get_reserved_bool_3()  ? 1 : 0; break;
                        case 4:  checkVal = pSite->get_reserved_bool_4()  ? 1 : 0; break;
                        case 5:  checkVal = pSite->get_reserved_bool_5()  ? 1 : 0; break;
                        case 6:  checkVal = pSite->get_reserved_bool_6()  ? 1 : 0; break;
                        case 7:  checkVal = pSite->get_reserved_bool_7()  ? 1 : 0; break;
                        case 8:  checkVal = pSite->get_reserved_bool_8()  ? 1 : 0; break;
                        case 9:  checkVal = pSite->get_reserved_bool_9()  ? 1 : 0; break;
                        case 10: checkVal = pSite->get_reserved_bool_10() ? 1 : 0; break;
                        case 11: checkVal = pSite->get_reserved_bool_11() ? 1 : 0; break;
                        case 12: checkVal = pSite->get_reserved_bool_12() ? 1 : 0; break;
                        case 13: checkVal = pSite->get_reserved_bool_13() ? 1 : 0; break;
                        case 14: checkVal = pSite->get_reserved_bool_14() ? 1 : 0; break;
                        case 15: checkVal = pSite->get_reserved_bool_15() ? 1 : 0; break;
                        case 16: checkVal = pSite->get_reserved_bool_16() ? 1 : 0; break;
                        default: FPS_ERROR_LOG("reserved_bool_%d not found!\n", bool_index); break;
                    }

                    // compare Site Manager's stored value with the expected value that is configured in sequences.json
                    int fault_alarm_index = RESERVED_BOOL_FAULTS_OFFSET+bool_index;
                    if (checkVal != (int) name_mask_pair.second)
                    {
                        num_active_alerts++;
                        if (alertType == FAULT ? !pSite->get_active_faults(fault_alarm_index) : !pSite->get_active_alarms(fault_alarm_index))
                        {
                            FPS_ERROR_LOG("Site Manager reserved_bool_%d %s detected\n", bool_index, alertType == FAULT ? "fault" : "alarm");
                            (alertType == FAULT) ? pSite->set_faults(fault_alarm_index) : pSite->set_alarms(fault_alarm_index);
                        }
                    }
                }
            }
            else if(var_type == "float")
            {
                //TODO: add functionality for reserved floats
            }
        }
    }

    // update total active alerts
    if (alertType == FAULT)
        num_active_faults = num_active_alerts;
    else if (alertType == ALARM)
        num_active_alarms = num_active_alerts;

    // return true/false if any alerts were set
    return (alertType == FAULT) ? pSite->get_faults() : pSite->get_alarms();
}

bool Path::configure_path(cJSON* object, int current_path_index)
{
    cJSON* JSON_path_name = cJSON_GetObjectItem(object, "path_name");
    if (JSON_path_name == NULL || JSON_path_name->valuestring == NULL)
    {
        FPS_ERROR_LOG("No path_name found \n");
        return false;
    }
    path_name = strdup(JSON_path_name->valuestring);
    cJSON* JSON_return_id = cJSON_GetObjectItem(object, "return_id");
    if (JSON_return_id == NULL || JSON_return_id->valuestring == NULL)
    {
        FPS_ERROR_LOG("No return_id found \n");
        return false;
    }
    for (int i = 0; i < NUM_STATES; i++)
    {
        if (strcmp(JSON_return_id->valuestring, state_name[i]) == 0)
        {
            return_state = (states)i;
            break;
        }
    }
    if (return_state == Error)
    {
        FPS_ERROR_LOG("Invalid return state defined.\n");
        return false;
    }
    cJSON* JSON_timeout = cJSON_GetObjectItem(object, "timeout");
    if (JSON_timeout == NULL)
        timeout = -1;
    else
        timeout = JSON_timeout->valueint;

    FPS_INFO_LOG("  path %i: %s \n    return_id: %s \n", current_path_index, path_name, state_name[return_state]);
    if (timeout != -1)
        FPS_WARNING_LOG("    timeout: %i \n", timeout);

    cJSON* JSON_active_faults = cJSON_GetObjectItem(object, "active_faults");
    // Active alarms is optional and may be NULL in sequences.json
    cJSON* JSON_active_alarms = cJSON_GetObjectItem(object, "active_alarms");

    int active_faults_size = cJSON_GetArraySize(JSON_active_faults);
    // Will return 0 if JSON_active_alarms = NULL
    int active_alarms_size = cJSON_GetArraySize(JSON_active_alarms);

    if (JSON_active_faults == NULL || active_faults_size == 0)
    {
        FPS_ERROR_LOG("no active_faults found \n");
        return false;
    }

    // parse faults
    for (int i = 0; i < active_faults_size; i++)
    {
        cJSON* JSON_active_faults_index = cJSON_GetArrayItem(JSON_active_faults, i);
        if (JSON_active_faults_index == NULL)
        {
            FPS_ERROR_LOG("no active_faults index %d found \n", i);
            return false;
        }

        // parse fault name
        std::string fault_name;
        cJSON* JSON_fault_name = cJSON_GetObjectItem(JSON_active_faults_index, "name");
        if (JSON_fault_name == NULL || JSON_fault_name->valuestring == NULL)
        {
            FPS_ERROR_LOG("no active fault name found at index %d\n", i);
            return false;
        }
        fault_name = JSON_fault_name->valuestring;

        // parse fault mask
        uint64_t fault_mask;
        cJSON* JSON_fault_mask = cJSON_GetObjectItem(JSON_active_faults_index, "mask");
        if (JSON_fault_mask == NULL || JSON_fault_mask->valuestring == NULL)
        {
            // not every fault requires a mask so give full mask if it is not there
            fault_mask = 0xFFFFFFFFFFFFFFFF;
        }
        else
        {
            try
            {
                fault_mask = std::stoul(JSON_fault_mask->valuestring, NULL, 16);
            }
            catch (...)
            {
                FPS_ERROR_LOG("Invalid mask received for %s\n", fault_name.c_str());
                return false;
            }
        }
        
        // print parsed data
        FPS_WARNING_LOG("    active faults %d of %d: %s \n", i+1, active_faults_size, fault_name.c_str());
        FPS_WARNING_LOG("      fault mask: %ld\n", fault_mask);

        // save to faults vector
        faults.push_back(std::pair<std::string,uint64_t>{fault_name, fault_mask});
    }

    // parse alarms
    for (int i = 0; i < active_alarms_size; i++)
    {
        cJSON* JSON_active_alarms_index = cJSON_GetArrayItem(JSON_active_alarms, i);
        if (JSON_active_alarms_index == NULL)
        {
            FPS_ERROR_LOG("no active_alarms index %d found \n", i);
            return false;
        }

        // parse alarm name
        std::string alarm_name;
        cJSON* JSON_alarm_name = cJSON_GetObjectItem(JSON_active_alarms_index, "name");
        if (JSON_alarm_name == NULL || JSON_alarm_name->valuestring == NULL)
        {
            FPS_ERROR_LOG("no active alarm name found at index %d\n", i);
            return false;
        }
        alarm_name = JSON_alarm_name->valuestring;

        // parse alarm mask
        uint64_t alarm_mask;
        cJSON* JSON_alarm_mask = cJSON_GetObjectItem(JSON_active_alarms_index, "mask");
        if (JSON_alarm_mask == NULL || JSON_alarm_mask->valuestring == NULL)
        {
            // not every alarm requires a mask so give full mask if it is not there
            alarm_mask = 0xFFFFFFFFFFFFFFFF;
        }
        else
        {
            try
            {
                alarm_mask = std::stoul(JSON_alarm_mask->valuestring, NULL, 16);
            }
            catch (...)
            {
                FPS_ERROR_LOG("Invalid mask received for %s\n", alarm_name.c_str());
                return false;
            }
        }
        
        // print parsed data
        FPS_WARNING_LOG("    active alarms %d of %d: %s \n", i+1, active_alarms_size, alarm_name.c_str());
        FPS_WARNING_LOG("      alarm mask: %ld\n", alarm_mask);

        // save to alarms vector
        alarms.push_back(std::pair<std::string,uint64_t>{alarm_name, alarm_mask});
    }

    cJSON* JSON_steps = cJSON_GetObjectItem(object, "steps");
    if (JSON_steps == NULL || cJSON_GetArraySize(JSON_steps) == 0)
    {
        FPS_ERROR_LOG("No steps found or steps array empty\n");
        return false;
    }
    steps_size     = cJSON_GetArraySize(JSON_steps);
    steps          = new Step[steps_size];

    for (int i = 0; i < steps_size; i++)
    {
        cJSON* JSON_steps_index = cJSON_GetArrayItem(JSON_steps, i);
        if (JSON_steps_index == NULL)
        {
            FPS_ERROR_LOG("No object found in steps array index %d found \n", i);
            return false;
        }
        if (steps[i].configure_step(JSON_steps_index, i) == false)
        {
            FPS_ERROR_LOG("Failed to parse step %d\n", i);
            return false;
        }
    }
    return true;
}
