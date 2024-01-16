/*
 * Path.cpp
 *
 *  Created on: Nov 5, 2019
 *      Author: kbrezina
 */

/* C Standard Library Dependencies */
#include <climits>
#include <cstdint>
#include <cstring>
/* C++ Standard Library Dependencies */
#include <numeric>
#include <string>
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
/* Local Internal Dependencies */
#include <Path.h>
#include <Site_Manager.h>
#include <Site_Controller_Utils.h>
#include "Asset_ESS.h"
#include "Asset_Feeder.h"
#include "Types.h"

/**
 * @brief No arg constructor for Path. This is used when setting up actions. (actions.json)
 * Responsibility of the caller to set Sequence_Type and the correct variable in the union including
 * site* and each asset_type*.
 */
Path::Path() {
    site = nullptr;
    path_name = "";
    return_state = Error;
    timeout = 0;
    num_active_faults = 0;
    num_active_alarms = 0;
}

Path::Path(Site_Manager* siteref) {
    site = siteref;
    return_state = Error;
    timeout = 0;
    num_active_faults = 0;
    num_active_alarms = 0;
}

/**
 * @brief Perfoms any checks for faults and alarms at the Site level using a site_manager*
 * and reports to an int& num_active_alerts.
 *
 * @param alert_type (type_of_alert) Fault/alarm
 */
void Path::handle_site_check_alerts(alert_type type_of_alert) {
    // count how many active alerts there are
    int num_active_alerts = 0;
    for (auto name_mask_pair : (type_of_alert == FAULT_ALERT ? faults : alarms)) {
        // allocate memory for an event message to be sent if alert detected
        char event_message[MEDIUM_MSG_LEN];

        // split asset/fault name into fragments separated by "/"s
        std::vector<std::string> name_fragments = split(name_mask_pair.first, "/");

        // no alert to check for in this path
        if (name_fragments[0] == "bypass") {
            continue;
            // debug test fault "test_fault"
        }

        if (type_of_alert == FAULT_ALERT && name_fragments[0] == "test_fault") {
            num_active_alerts++;
            // only emit event if new
            if (!site->get_active_faults(0)) {
                FPS_ERROR_LOG("test fault detected: %s", name_fragments[0]);
                snprintf(event_message, MEDIUM_MSG_LEN, "Test fault detected");
                emit_event("Site", event_message, type_of_alert);
                site->set_faults(0);
            }
        }
        // debug test alarm "test_alarm"
        else if (type_of_alert == ALARM_ALERT && name_fragments[0] == "test_alarm") {
            num_active_alerts++;
            // only emit event if new
            if (!site->get_active_alarms(0)) {
                FPS_ERROR_LOG("test alarm detected: %s", name_fragments[0]);
                snprintf(event_message, MEDIUM_MSG_LEN, "Test alarm detected");
                emit_event("Site", event_message, type_of_alert);
                site->set_alarms(0);
            }
        }
        // The following all use site fault 1 or site alarm 1, depending on alert type being checked
        else if (name_fragments.size() >= 2) {
            std::string asset_cmd = name_fragments[1];
            // asset faults/alarms
            if (name_fragments[0] == "assets") {
                //
                // ESS SECTION
                //
                // aggregate ESS fault check (true if any faults)
                if (asset_cmd == "get_any_ess_faults") {
                    if (site->pAssets->get_num_active_faults(ESS) > 0) {
                        num_active_alerts++;
                        if (type_of_alert == FAULT_ALERT ? !site->get_active_faults(1) : !site->get_active_alarms(1)) {
                            // print and raise alarm/fault based on type
                            FPS_ERROR_LOG("get_any_ess_faults %s detected: %s", alert_names_lower[type_of_alert], name_mask_pair.first);
                            snprintf(event_message, MEDIUM_MSG_LEN, "%s: ESS fault(s) detected", alert_names_upper[type_of_alert]);
                            emit_event("Site", event_message, type_of_alert);
                            (type_of_alert == FAULT_ALERT) ? site->set_faults(1) : site->set_alarms(1);
                        }
                    }
                }
                // aggregate ESS fault check (true if any faults)
                else if (asset_cmd == "get_any_ess_alarms") {
                    if (site->pAssets->get_num_active_alarms(ESS) > 0) {
                        num_active_alerts++;
                        if (type_of_alert == FAULT_ALERT ? !site->get_active_faults(1) : !site->get_active_alarms(1)) {
                            // print and raise alarm/fault based on type
                            FPS_ERROR_LOG("get_any_ess_alarms %s detected: %s", alert_names_lower[type_of_alert], name_mask_pair.first);
                            snprintf(event_message, MEDIUM_MSG_LEN, "%s: ESS alarm(s) detected", alert_names_upper[type_of_alert]);
                            emit_event("Site", event_message, type_of_alert);
                            (type_of_alert == FAULT_ALERT) ? site->set_faults(1) : site->set_alarms(1);
                        }
                    }
                }
                // check for number of ess running. compare read value to first element and fault if less/equal
                // In this case asset_active_faults value is used as the expected number
                else if (asset_cmd == "get_num_ess_running") {
                    // assert the number of expected running based on type
                    int expected_num_running = static_cast<int>(name_mask_pair.second);
                    int actual_num_running = site->pAssets->get_num_ess_running();
                    if (expected_num_running >= actual_num_running) {
                        num_active_alerts++;
                        if (type_of_alert == FAULT_ALERT ? !site->get_active_faults(1) : !site->get_active_alarms(1)) {
                            FPS_ERROR_LOG("get_num_ess_running %s detected: %s", alert_names_lower[type_of_alert], name_mask_pair.first);
                            snprintf(event_message, MEDIUM_MSG_LEN, "%s: insufficient number of ESS running. Expected at least %d but detected %d", alert_names_upper[type_of_alert], expected_num_running + 1, actual_num_running);
                            emit_event("Site", event_message, type_of_alert);
                            (type_of_alert == FAULT_ALERT) ? site->set_faults(1) : site->set_alarms(1);
                        }
                    }
                }
                // check for number of ess available. compare read value to first element and fault if less/equal
                // In this case asset_active_faults value is used as the expected number
                else if (asset_cmd == "get_num_ess_available") {
                    // assert the number of expected available based on type
                    int expected_num_avail = static_cast<int>(name_mask_pair.second);
                    int actual_num_avail = site->pAssets->get_num_ess_avail();
                    if (expected_num_avail >= actual_num_avail) {
                        num_active_alerts++;
                        if (type_of_alert == FAULT_ALERT ? !site->get_active_faults(1) : !site->get_active_alarms(1)) {
                            FPS_ERROR_LOG("get_num_ess_available %s detected: %s", alert_names_lower[type_of_alert], name_mask_pair.first);
                            snprintf(event_message, MEDIUM_MSG_LEN, "%s: insufficient number of ESS available. Expected at least %d but detected %d", alert_names_upper[type_of_alert], expected_num_avail + 1, actual_num_avail);
                            emit_event("Site", event_message, type_of_alert);
                            (type_of_alert == FAULT_ALERT) ? site->set_faults(1) : site->set_alarms(1);
                        }
                    }
                }
                // check for number of ess controllable. compare read value to first element and fault if less/equal
                // In this case asset_active_faults value is used as the expected number
                else if (asset_cmd == "get_num_ess_controllable") {
                    // assert the number of expected controllable based on type
                    int expected_num_controllable = static_cast<int>(name_mask_pair.second);
                    int actual_num_controllable = site->pAssets->get_num_ess_controllable();
                    if (expected_num_controllable >= actual_num_controllable) {
                        num_active_alerts++;
                        if (type_of_alert == FAULT_ALERT ? !site->get_active_faults(1) : !site->get_active_alarms(1)) {
                            FPS_ERROR_LOG("get_num_ess_controllable %s detected: %s", alert_names_lower[type_of_alert], name_mask_pair.first);
                            snprintf(event_message, MEDIUM_MSG_LEN, "%s: insufficient number of ESS controllable. Expected at least %d but detected %d", alert_names_upper[type_of_alert], expected_num_controllable + 1, actual_num_controllable);
                            emit_event("Site", event_message, type_of_alert);
                            (type_of_alert == FAULT_ALERT) ? site->set_faults(1) : site->set_alarms(1);
                        }
                    }
                }
                //
                // SOLAR SECTION
                //
                // aggregate SOLAR fault check (true if any faults)
                else if (asset_cmd == "get_any_solar_faults") {
                    if (site->pAssets->get_num_active_faults(SOLAR) > 0) {
                        num_active_alerts++;
                        if (type_of_alert == FAULT_ALERT ? !site->get_active_faults(1) : !site->get_active_alarms(1)) {
                            // print and raise alarm/fault based on type
                            FPS_ERROR_LOG("get_any_solar_faults %s detected: %s", alert_names_lower[type_of_alert], name_mask_pair.first);
                            snprintf(event_message, MEDIUM_MSG_LEN, "%s: solar fault(s) detected", alert_names_upper[type_of_alert]);
                            emit_event("Site", event_message, type_of_alert);
                            (type_of_alert == FAULT_ALERT) ? site->set_faults(1) : site->set_alarms(1);
                        }
                    }
                }
                // aggregate SOLAR fault check (true if any faults)
                else if (asset_cmd == "get_any_solar_alarms") {
                    if (site->pAssets->get_num_active_alarms(SOLAR) > 0) {
                        num_active_alerts++;
                        if (type_of_alert == FAULT_ALERT ? !site->get_active_faults(1) : !site->get_active_alarms(1)) {
                            // print and raise alarm/fault based on type
                            FPS_ERROR_LOG("get_any_solar_alarms %s detected: %s", alert_names_lower[type_of_alert], name_mask_pair.first);
                            snprintf(event_message, MEDIUM_MSG_LEN, "%s: solar alarm(s) detected", alert_names_upper[type_of_alert]);
                            emit_event("Site", event_message, type_of_alert);
                            (type_of_alert == FAULT_ALERT) ? site->set_faults(1) : site->set_alarms(1);
                        }
                    }
                }
                // check for number of solar running. compare read value to first element and fault if less/equal
                // In this case asset_active_faults value is used as the expected number
                else if (asset_cmd == "get_num_solar_running") {
                    // assert the number of expected running based on type
                    int expected_num_running = static_cast<int>(name_mask_pair.second);
                    int actual_num_running = site->pAssets->get_num_solar_running();
                    if (expected_num_running >= actual_num_running) {
                        num_active_alerts++;
                        if (type_of_alert == FAULT_ALERT ? !site->get_active_faults(1) : !site->get_active_alarms(1)) {
                            FPS_ERROR_LOG("get_num_solar_running %s detected: %s", alert_names_lower[type_of_alert], name_mask_pair.first);
                            snprintf(event_message, MEDIUM_MSG_LEN, "%s: insufficient number of solar running. Expected at least %d but detected %d", alert_names_upper[type_of_alert], expected_num_running + 1, actual_num_running);
                            emit_event("Site", event_message, type_of_alert);
                            (type_of_alert == FAULT_ALERT) ? site->set_faults(1) : site->set_alarms(1);
                        }
                    }
                }
                // check for number of solar available. compare read value to first element and fault if less/equal
                // In this case asset_active_faults value is used as the expected number
                else if (asset_cmd == "get_num_solar_available") {
                    // assert the number of expected available based on type
                    int expected_num_avail = static_cast<int>(name_mask_pair.second);
                    int actual_num_avail = site->pAssets->get_num_solar_avail();
                    if (expected_num_avail >= actual_num_avail) {
                        num_active_alerts++;
                        if (type_of_alert == FAULT_ALERT ? !site->get_active_faults(1) : !site->get_active_alarms(1)) {
                            FPS_ERROR_LOG("get_num_solar_available %s detected: %s", alert_names_lower[type_of_alert], name_mask_pair.first);
                            snprintf(event_message, MEDIUM_MSG_LEN, "%s: insufficient number of solar available. Expected at least %d but detected %d", alert_names_upper[type_of_alert], expected_num_avail + 1, actual_num_avail);
                            emit_event("Site", event_message, type_of_alert);
                            (type_of_alert == FAULT_ALERT) ? site->set_faults(1) : site->set_alarms(1);
                        }
                    }
                }
                // check for number of solar controllable. compare read value to first element and fault if less/equal
                // In this case asset_active_faults value is used as the expected number
                else if (asset_cmd == "get_num_solar_controllable") {
                    // assert the number of expected controllable based on type
                    int expected_num_controllable = static_cast<int>(name_mask_pair.second);
                    int actual_num_controllable = site->pAssets->get_num_solar_controllable();
                    if (expected_num_controllable >= actual_num_controllable) {
                        num_active_alerts++;
                        if (type_of_alert == FAULT_ALERT ? !site->get_active_faults(1) : !site->get_active_alarms(1)) {
                            FPS_ERROR_LOG("get_num_solar_controllable %s detected: %s", alert_names_lower[type_of_alert], name_mask_pair.first);
                            snprintf(event_message, MEDIUM_MSG_LEN, "%s: insufficient number of solar controllable. Expected at least %d but detected %d", alert_names_upper[type_of_alert], expected_num_controllable + 1, actual_num_controllable);
                            emit_event("Site", event_message, type_of_alert);
                            (type_of_alert == FAULT_ALERT) ? site->set_faults(1) : site->set_alarms(1);
                        }
                    }
                }
                //
                // GENERATORS SECTION
                //
                // aggregate GENERATORS fault check (true if any faults)
                else if (asset_cmd == "get_any_gen_faults") {
                    if (site->pAssets->get_num_active_faults(GENERATORS) > 0) {
                        num_active_alerts++;
                        if (type_of_alert == FAULT_ALERT ? !site->get_active_faults(1) : !site->get_active_alarms(1)) {
                            // print and raise alarm/fault based on type
                            FPS_ERROR_LOG("get_any_gen_faults %s detected: %s", alert_names_lower[type_of_alert], name_mask_pair.first);
                            snprintf(event_message, MEDIUM_MSG_LEN, "%s: generator fault(s) detected", alert_names_upper[type_of_alert]);
                            emit_event("Site", event_message, type_of_alert);
                            (type_of_alert == FAULT_ALERT) ? site->set_faults(1) : site->set_alarms(1);
                        }
                    }
                }
                // aggregate GENERATORS fault check (true if any faults)
                else if (asset_cmd == "get_any_gen_alarms") {
                    if (site->pAssets->get_num_active_alarms(GENERATORS) > 0) {
                        num_active_alerts++;
                        if (type_of_alert == FAULT_ALERT ? !site->get_active_faults(1) : !site->get_active_alarms(1)) {
                            // print and raise alarm/fault based on type
                            FPS_ERROR_LOG("get_any_gen_alarms %s detected: %s", alert_names_lower[type_of_alert], name_mask_pair.first);
                            snprintf(event_message, MEDIUM_MSG_LEN, "%s: generator alarm(s) detected", alert_names_upper[type_of_alert]);
                            emit_event("Site", event_message, type_of_alert);
                            (type_of_alert == FAULT_ALERT) ? site->set_faults(1) : site->set_alarms(1);
                        }
                    }
                }
                // check for number of generators running. compare read value to first element and fault if less/equal
                // In this case asset_active_faults value is used as the expected number
                else if (asset_cmd == "get_num_gen_running") {
                    // assert the number of expected running based on type
                    int expected_num_running = static_cast<int>(name_mask_pair.second);
                    int actual_num_running = site->pAssets->get_num_gen_running();
                    if (expected_num_running >= actual_num_running) {
                        num_active_alerts++;
                        if (type_of_alert == FAULT_ALERT ? !site->get_active_faults(1) : !site->get_active_alarms(1)) {
                            FPS_ERROR_LOG("get_num_gen_running %s detected: %s", alert_names_lower[type_of_alert], name_mask_pair.first);
                            snprintf(event_message, MEDIUM_MSG_LEN, "%s: insufficient number of generators running. Expected at least %d but detected %d", alert_names_upper[type_of_alert], expected_num_running + 1, actual_num_running);
                            emit_event("Site", event_message, type_of_alert);
                            (type_of_alert == FAULT_ALERT) ? site->set_faults(1) : site->set_alarms(1);
                        }
                    }
                }
                // check for number of gen available. compare read value to first element and fault if less/equal
                // In this case asset_active_faults value is used as the expected number
                else if (asset_cmd == "get_num_gen_available") {
                    // assert the number of expected available based on type
                    int expected_num_avail = static_cast<int>(name_mask_pair.second);
                    int actual_num_avail = site->pAssets->get_num_gen_avail();
                    if (expected_num_avail >= actual_num_avail) {
                        num_active_alerts++;
                        if (type_of_alert == FAULT_ALERT ? !site->get_active_faults(1) : !site->get_active_alarms(1)) {
                            FPS_ERROR_LOG("get_num_gen_available %s detected: %s", alert_names_lower[type_of_alert], name_mask_pair.first);
                            snprintf(event_message, MEDIUM_MSG_LEN, "%s: insufficient number of generators available. Expected at least %d but detected %d", alert_names_upper[type_of_alert], expected_num_avail + 1, actual_num_avail);
                            emit_event("Site", event_message, type_of_alert);
                            (type_of_alert == FAULT_ALERT) ? site->set_faults(1) : site->set_alarms(1);
                        }
                    }
                }
                // check for number of gen controllable. compare read value to first element and fault if less/equal
                // In this case asset_active_faults value is used as the expected number
                else if (asset_cmd == "get_num_gen_controllable") {
                    // assert the number of expected controllable based on type
                    int expected_num_controllable = static_cast<int>(name_mask_pair.second);
                    int actual_num_controllable = site->pAssets->get_num_gen_controllable();
                    if (expected_num_controllable >= actual_num_controllable) {
                        num_active_alerts++;
                        if (type_of_alert == FAULT_ALERT ? !site->get_active_faults(1) : !site->get_active_alarms(1)) {
                            FPS_ERROR_LOG("get_num_gen_controllable %s detected: %s", alert_names_lower[type_of_alert], name_mask_pair.first);
                            snprintf(event_message, MEDIUM_MSG_LEN, "%s: insufficient number of generators controllable. Expected at least %d but detected %d", alert_names_upper[type_of_alert], expected_num_controllable + 1, actual_num_controllable);
                            emit_event("Site", event_message, type_of_alert);
                            (type_of_alert == FAULT_ALERT) ? site->set_faults(1) : site->set_alarms(1);
                        }
                    }
                }
                // check for number of solar, ess, gen controllable.  if none controllable, fault
                // In this case asset_active_faults value is used as the expected number
                else if (asset_cmd == "get_num_assets_controllable") {
                    // assert the number of expected controllable based on type
                    int expected_num_controllable = static_cast<int>(name_mask_pair.second);
                    int actual_num_controllable = site->pAssets->get_num_ess_controllable() + site->pAssets->get_num_gen_controllable() + site->pAssets->get_num_solar_controllable();
                    if (expected_num_controllable >= actual_num_controllable) {
                        num_active_alerts++;
                        if (type_of_alert == FAULT_ALERT ? !site->get_active_faults(1) : !site->get_active_alarms(1)) {
                            FPS_ERROR_LOG("get_num_assets_controllable %s detected: %s", alert_names_lower[type_of_alert], name_mask_pair.first);
                            snprintf(event_message, MEDIUM_MSG_LEN, "%s: insufficient number of assets controllable across all types. Expected at least %d but detected %d", alert_names_upper[type_of_alert], expected_num_controllable + 1,
                                    actual_num_controllable);
                            emit_event("Site", event_message, type_of_alert);
                            (type_of_alert == FAULT_ALERT) ? site->set_faults(1) : site->set_alarms(1);
                        }
                    }
                }
                // if none of the previous names matched, then only remaining matches are of the format /assets/<asset type>/<asset id>/<alert id>
                else {
                    if (site->pAssets->check_asset_alert(name_mask_pair)) {
                        num_active_alerts++;
                        if (type_of_alert == FAULT_ALERT ? !site->get_active_faults(1) : !site->get_active_alarms(1)) {
                            if (name_fragments.size() != 4) {
                                FPS_WARNING_LOG("Received invalid sequence %s endpoint %s", alert_names_lower[type_of_alert], name_mask_pair.first);
                                break;
                            }
                            FPS_ERROR_LOG("Asset %s detected: %s", alert_names_lower[type_of_alert], name_mask_pair.first);
                            snprintf(event_message, MEDIUM_MSG_LEN, "%s: %s alert(s) detected on %s register", alert_names_upper[type_of_alert], name_fragments[2].c_str(), name_fragments[3].c_str());
                            emit_event("Site", event_message, type_of_alert);
                            (type_of_alert == FAULT_ALERT) ? site->set_faults(1) : site->set_alarms(1);
                        }
                    }
                }
            } else if (asset_cmd == "get_feeder_on_array") {
                Asset_Feeder* feeder_instance = site->pAssets->validate_feeder_id(name_fragments[0].c_str());
                if (feeder_instance == nullptr) {
                    FPS_ERROR_LOG("feeder ID: %s was not found in the assets list", name_fragments[0]);
                }
                // if first element is 0, then alert if feeder_state is false (alert on open breaker)
                else if (name_mask_pair.second == 0 && !site->pAssets->get_feeder_state(feeder_instance)) {
                    num_active_alerts++;
                    if (type_of_alert == FAULT_ALERT ? !site->get_active_faults(1) : !site->get_active_alarms(1)) {
                        FPS_ERROR_LOG("get_feeder_on_array %s detected: %s", alert_names_lower[type_of_alert], name_mask_pair.first);
                        snprintf(event_message, MEDIUM_MSG_LEN, "%s: %s breaker is open", alert_names_upper[type_of_alert], name_fragments[0].c_str());
                        emit_event("Site", event_message, type_of_alert);
                        (type_of_alert == FAULT_ALERT) ? site->set_faults(1) : site->set_alarms(1);
                    }
                }
                // if first element is 1, then alert if feeder_state is true (alert on closed breaker)
                else if (name_mask_pair.second == 1 && site->pAssets->get_feeder_state(feeder_instance)) {
                    num_active_alerts++;
                    if (type_of_alert == FAULT_ALERT ? !site->get_active_faults(1) : !site->get_active_alarms(1)) {
                        FPS_ERROR_LOG("get_feeder_on_array %s detected: %s", alert_names_lower[type_of_alert], name_mask_pair.first.c_str());
                        snprintf(event_message, MEDIUM_MSG_LEN, "%s: %s breaker is closed", alert_names_upper[type_of_alert], name_fragments[0].c_str());
                        emit_event("Site", event_message, type_of_alert);
                        (type_of_alert == FAULT_ALERT) ? site->set_faults(1) : site->set_alarms(1);
                    }
                }
            }
        }
        // alert may want to refer to one of the reserved_bool_# or reserved_float_# variables in Site Manager since an on-site component can be configured to send a warning value to one of those variables
        else if (strncmp(name_fragments[0].c_str(), "reserved_", strlen("reserved_")) == 0) {
            // parse data identifying which variable is being referenced
            std::string var_name = name_fragments[0];
            std::vector<std::string> var_name_fragments = split(var_name, "_");
            if (var_name_fragments.size() < 3) {
                FPS_ERROR_LOG("Reserved variable name is not valid: %s. Expected format is reserved_<varType>_<varIndex>.", var_name.c_str());
                continue;
            }
            std::string var_type = var_name_fragments[1];
            std::string var_index = var_name_fragments[2];

            // check the referenced variable
            if (var_type == "bool") {
                int bool_index;  // Need to declare n outside of try block so switch statement can access it afterwards
                try {
                    bool_index = std::stoi(var_index);  // Determine what # is in reserved_bool_#
                } catch (const std::exception& e) {     // std::stoi can throw an error if value in string is not within limits of int (such as if there are characters in there that aren't numbers)
                    bool_index = 0;
                    FPS_ERROR_LOG("Invalid # in reserved_bool_# in sequences.json active faults list: %s is invalid.", var_index.c_str());
                }

                if (bool_index != 0) {
                    Fims_Object reserved_bool;
                    std::string fault_name;
                    switch (bool_index) {  // sequences.json can't store "true" or "false" in asset_active_faults, so 0 stands for false and 1 stands for true
                        case 1:
                            reserved_bool = site->get_reserved_bool_1();
                            break;
                        case 2:
                            reserved_bool = site->get_reserved_bool_2();
                            break;
                        case 3:
                            reserved_bool = site->get_reserved_bool_3();
                            break;
                        case 4:
                            reserved_bool = site->get_reserved_bool_4();
                            break;
                        case 5:
                            reserved_bool = site->get_reserved_bool_5();
                            break;
                        case 6:
                            reserved_bool = site->get_reserved_bool_6();
                            break;
                        case 7:
                            reserved_bool = site->get_reserved_bool_7();
                            break;
                        case 8:
                            reserved_bool = site->get_reserved_bool_8();
                            break;
                        case 9:
                            reserved_bool = site->get_reserved_bool_9();
                            break;
                        case 10:
                            reserved_bool = site->get_reserved_bool_10();
                            break;
                        case 11:
                            reserved_bool = site->get_reserved_bool_11();
                            break;
                        case 12:
                            reserved_bool = site->get_reserved_bool_12();
                            break;
                        case 13:
                            reserved_bool = site->get_reserved_bool_13();
                            break;
                        case 14:
                            reserved_bool = site->get_reserved_bool_14();
                            break;
                        case 15:
                            reserved_bool = site->get_reserved_bool_15();
                            break;
                        case 16:
                            reserved_bool = site->get_reserved_bool_16();
                            break;
                        default:
                            FPS_ERROR_LOG("reserved_bool_%d not found!", bool_index);
                            break;
                    }

                    // compare Site Manager's stored value with the expected value that is configured in sequences.json
                    // TODO: this creates duplicate faults, and requires extra faults to be configured in variables
                    int fault_alarm_index = RESERVED_BOOL_FAULTS_OFFSET + bool_index;
                    if (reserved_bool.value.value_bool != static_cast<bool>(name_mask_pair.second)) {
                        num_active_alerts++;
                        if (type_of_alert == FAULT_ALERT ? !site->get_active_faults(fault_alarm_index) : !site->get_active_alarms(fault_alarm_index)) {
                            FPS_ERROR_LOG("Site Manager reserved_bool_%d %s detected", bool_index, alert_names_lower[type_of_alert]);
                            snprintf(event_message, MEDIUM_MSG_LEN, "%s: %s", alert_names_upper[type_of_alert], reserved_bool.get_name());
                            emit_event("Site", event_message, type_of_alert);
                            (type_of_alert == FAULT_ALERT) ? site->set_faults(fault_alarm_index) : site->set_alarms(fault_alarm_index);
                        }
                    }
                }
            } else if (var_type == "float") {
                // TODO: add functionality for reserved floats
            }
        }
    }

    // update total active alerts
    if (type_of_alert == FAULT_ALERT) {
        num_active_faults = num_active_alerts;
    } else if (type_of_alert == ALARM_ALERT) {
        num_active_alarms = num_active_alerts;
    }
}

/**
 * @brief Perfoms any checks for faults and alarms at the ESS instance level using an Asset_ESS*
 * and reports to an int& num_active_alerts.
 *
 * @param alert_type (type_of_alert) Fault/alarm
 */
void Path::handle_ess_check_alerts(alert_type type_of_alert) {
    // count how many active alerts there are
    int num_active_alerts = 0;

    std::string alarm;
    for (auto name_mask_pair : (type_of_alert == FAULT_ALERT ? faults : alarms)) {
        char event_message[MEDIUM_MSG_LEN];

        std::vector<std::string> name_fragments = split(name_mask_pair.first, "/");
        // make sure it's a single fragment otherwise it's an error.
        if (name_fragments.size() != 1) {
            FPS_ERROR_LOG("Bad URI in actions.json. URI: %s", name_mask_pair.first);
            return;
        }
        std::string frag = name_fragments[0];

        if (frag == "bypass") {
            continue;
        }

        if (type_of_alert == FAULT_ALERT && frag == "test_fault") {
            num_active_alerts++;
            alarm = "test_fault";
            if ((asset_ess->actions_faults.value.value_bit_field & static_cast<uint64_t>(0x1)) == 0) {
                FPS_ERROR_LOG("test fault detected: %s", frag);
                snprintf(event_message, MEDIUM_MSG_LEN, "Test fault detected");
                asset_ess->actions_faults.value.value_bit_field |= 0x1;
            }
        }
        else if (type_of_alert == ALARM_ALERT && frag == "test_alarm") {
            num_active_alerts++;
            alarm = "test_alarm";
            if ((asset_ess->actions_alarms.value.value_bit_field & static_cast<uint64_t>(0x1)) == 0) {
                FPS_ERROR_LOG("test alarm detected: %s", frag);
                snprintf(event_message, MEDIUM_MSG_LEN, "Test alarm detected");
                asset_ess->actions_alarms.value.value_bit_field |= 0x1;
            }
        } else if (frag == "is_faulted") {
            if (asset_ess->get_num_active_faults() > 0) {
                num_active_alerts++;
            }
        }
        else if (frag == "is_alarmed") {
            if (asset_ess->get_num_active_alarms() > 0) {
                num_active_alerts++;
            }
        }
        else if (frag == "is_not_running") {
            if (!asset_ess->is_running()) {
                num_active_alerts++;
            }
        }
        else if (frag == "is_not_available") {
            if (!asset_ess->is_available()) {
                num_active_alerts++;
            }
        }
        else if (frag == "is_not_controllable") {
            if (!asset_ess->is_controllable()) {
                num_active_alerts++;
            }
        }
    }

    // update total active alerts
    if (type_of_alert == FAULT_ALERT) {
        num_active_faults = num_active_alerts;
    } else if (type_of_alert == ALARM_ALERT) {
        num_active_alarms = num_active_alerts;
    }
}

/**
 * @brief Checks either alarms or faults depending on the alert type passed.
 * @param type_of_alert indicates checking alarms or checking faults. Valid values are ALARM_ALERT and FAULT_ALERT
 * @return True if there are alerts of the given type, or false if there are none.
 */
bool Path::check_alerts(alert_type type_of_alert, Sequence_Type sequence_type) {
    if (type_of_alert != ALARM_ALERT && type_of_alert != FAULT_ALERT) {
        FPS_ERROR_LOG("This function only accepts ALARM_ALERT or FAULT_ALERT as an argument. Returning false....");
        return false;
    }

    switch(sequence_type) {
        case Sequence_Type::Site:
            handle_site_check_alerts(type_of_alert);
            return (type_of_alert == FAULT_ALERT) ? site->get_faults() : site->get_alarms();
        case Sequence_Type::Asset_ESS:
            handle_ess_check_alerts(type_of_alert);
            return (type_of_alert == FAULT_ALERT) ? asset_ess->get_num_active_faults() != 0 : asset_ess->get_num_active_alarms() != 0;
        case Sequence_Type::Asset_Solar:
            FPS_ERROR_LOG("Gathering Solar instance faults and alarms not yet supported.");
            return false; // todo update this
        case Sequence_Type::Asset_Generator:
            FPS_ERROR_LOG("Gathering Gen instance faults and alarms not yet supported.");
            return false; // todo update this
        case Sequence_Type::Asset_Feeder:
            FPS_ERROR_LOG("Gathering Feeder instance faults and alarms not yet supported.");
            return false; // todo update this
        default:
            FPS_ERROR_LOG("Error case reached.");
            return false; // todo update this
    }
}

bool Path::handle_return_id(cJSON* JSON_return_id) {
    if (JSON_return_id == NULL || JSON_return_id->valuestring == NULL) {
        FPS_ERROR_LOG("No return_id found.");
        return false;
    }
    for (int i = 0; i < NUM_STATES; i++) {
        if (strcmp(JSON_return_id->valuestring, state_name[i]) == 0) {
            return_state = static_cast<states>(i);
            break;
        }
    }
    if (return_state == Error) {
        FPS_ERROR_LOG("Invalid return state defined..");
        return false;
    }
    return true;
}

bool Path::handle_faults(cJSON* JSON_active_faults) {
    int active_faults_size = cJSON_GetArraySize(JSON_active_faults);
    if (JSON_active_faults == NULL || active_faults_size == 0) {
        FPS_ERROR_LOG("no active_faults found.");
        return false;
    }

    for (int i = 0; i < active_faults_size; i++) {
        cJSON* JSON_active_faults_index = cJSON_GetArrayItem(JSON_active_faults, i);
        if (JSON_active_faults_index == NULL) {
            FPS_ERROR_LOG("no active_faults index %d found.", i);
            return false;
        }

        // parse fault name
        std::string fault_name;
        cJSON* JSON_fault_name = cJSON_GetObjectItem(JSON_active_faults_index, "name");
        if (JSON_fault_name == NULL || JSON_fault_name->valuestring == NULL) {
            FPS_ERROR_LOG("no active fault name found at index %d.", i);
            return false;
        }
        if (JSON_fault_name->valuestring[0] != '/') {
            FPS_ERROR_LOG("fault name must begin with a slash");
            return false;
        }
        fault_name = JSON_fault_name->valuestring;

        // parse fault mask
        uint64_t fault_mask;
        cJSON* JSON_fault_mask = cJSON_GetObjectItem(JSON_active_faults_index, "mask");
        if (JSON_fault_mask == NULL || JSON_fault_mask->valuestring == NULL) {
            // not every fault requires a mask so give full mask if it is not there
            fault_mask = 0xFFFFFFFFFFFFFFFF;
        } else {
            try {
                fault_mask = std::stoul(JSON_fault_mask->valuestring, NULL, 16);
            } catch (...) {
                FPS_ERROR_LOG("Invalid mask received for %s.", fault_name.c_str());
                return false;
            }
        }

        // print parsed data
        FPS_WARNING_LOG("active faults %d of %d: %s.", i + 1, active_faults_size, fault_name.c_str());
        FPS_WARNING_LOG("fault mask: %ld.", fault_mask);

        // save to faults vector
        faults.push_back(std::pair<std::string, uint64_t>{ fault_name, fault_mask });
    }
    return true;
}

bool Path::handle_alarms(cJSON* JSON_active_alarms) {
    int active_alarms_size = cJSON_GetArraySize(JSON_active_alarms);

    for (int i = 0; i < active_alarms_size; i++) {
        cJSON* JSON_active_alarms_index = cJSON_GetArrayItem(JSON_active_alarms, i);
        if (JSON_active_alarms_index == NULL) {
            FPS_ERROR_LOG("no active_alarms index %d found.", i);
            return false;
        }

        // parse alarm name
        std::string alarm_name;
        cJSON* JSON_alarm_name = cJSON_GetObjectItem(JSON_active_alarms_index, "name");
        if (JSON_alarm_name == NULL || JSON_alarm_name->valuestring == NULL) {
            FPS_ERROR_LOG("no active alarm name found at index %d.", i);
            return false;
        }
        alarm_name = JSON_alarm_name->valuestring;

        // parse alarm mask
        uint64_t alarm_mask;
        cJSON* JSON_alarm_mask = cJSON_GetObjectItem(JSON_active_alarms_index, "mask");
        if (JSON_alarm_mask == NULL || JSON_alarm_mask->valuestring == NULL) {
            // not every alarm requires a mask so give full mask if it is not there
            alarm_mask = 0xFFFFFFFFFFFFFFFF;
        } else {
            try {
                alarm_mask = std::stoul(JSON_alarm_mask->valuestring, NULL, 16);
            } catch (...) {
                FPS_ERROR_LOG("Invalid mask received for %s.", alarm_name.c_str());
                return false;
            }
        }

        // print parsed data
        FPS_WARNING_LOG("active alarms %d of %d: %s.", i + 1, active_alarms_size, alarm_name.c_str());
        FPS_WARNING_LOG("alarm mask: %ld.", alarm_mask);

        // save to alarms vector
        alarms.push_back(std::pair<std::string, uint64_t>{ alarm_name, alarm_mask });
    }
    return true;
}

bool Path::configure_path(cJSON* object, bool must_have_return_id) {
    // required
    cJSON* JSON_path_name = cJSON_GetObjectItem(object, "path_name");
    cJSON* JSON_timeout = cJSON_GetObjectItem(object, "timeout");
    cJSON* JSON_active_faults = cJSON_GetObjectItem(object, "active_faults");
    cJSON* JSON_steps = cJSON_GetObjectItem(object, "steps");
    cJSON* JSON_return_id = cJSON_GetObjectItem(object, "return_id");  // not required if it's an action though

    // optional
    cJSON* JSON_active_alarms = cJSON_GetObjectItem(object, "active_alarms");  // Active alarms is optional and may be NULL in sequences.json

    if (JSON_path_name == NULL || JSON_path_name->valuestring == NULL) {
        FPS_ERROR_LOG("No path_name found.");
        return false;
    }
    path_name = JSON_path_name->valuestring;

    if (JSON_steps == NULL || cJSON_GetArraySize(JSON_steps) == 0) {
        FPS_ERROR_LOG("No steps found or steps array empty.");
        return false;
    }

    if (must_have_return_id) {
        handle_return_id(JSON_return_id);
    }

    JSON_timeout == nullptr ? timeout = -1 : timeout = JSON_timeout->valueint;

    // parse faults
    if (!handle_faults(JSON_active_faults)) {
        return false;
    }

    // parse alarms
    handle_alarms(JSON_active_alarms);

    for (int i = 0; i < cJSON_GetArraySize(JSON_steps); i++) {
        // Current step being configured
        steps.emplace_back();
        Step& current_step = steps.back();

        cJSON* JSON_steps_index = cJSON_GetArrayItem(JSON_steps, i);
        if (JSON_steps_index == NULL) {
            FPS_ERROR_LOG("No object found in steps array index %d found.", i);
            return false;
        }
        if (!current_step.configure_step(JSON_steps_index, i)) {
            FPS_ERROR_LOG("Failed to parse step %d.", i);
            return false;
        }
    }
    return true;
}

/**
 * @brief Returns the sum of the remaining steps in an action. Will omit steps that have already
 * been run. There is not at present a smart way to report on steps without a debounce_timer_ms. 
 * But one could be implemented.
 * @param action (const Step&) The Step to report on.
 * @return The number of seconds as an int.
 */
int  Path::collect_seconds_in_path() {
    // vector of exit_steps (should contain the debounce_timer_ms)
    auto start_iterator = steps.begin();

    return std::accumulate(start_iterator, steps.end(), 0,
        [](int currentSum, Step& step) {
            return currentSum + step.collect_seconds_in_step(); 
        }
    );
}
