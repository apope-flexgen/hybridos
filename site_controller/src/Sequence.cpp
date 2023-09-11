/*
 * Sequence.cpp
 *
 *  Created on: Nov 5, 2019
 *      Author: kbrezina
 */

/* C Standard Library Dependencies */
#include <cstring>
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
/* Local Internal Dependencies */
#include <Sequence.h>
#include <Site_Manager.h>
#include <Site_Controller_Utils.h>

#define ENTRY '0'
#define EXIT '1'
#define DEBOUNCE '0'
#define CONDITIONAL '1'

Sequence::Sequence(Site_Manager* siteref) {
    pSite = siteref;
    state = Init;
    current_step_index = 0;
    check_current_step_index = 0;
    current_path_index = 0;
    check_current_path_index = 0;
    step_reset = true;
    initial_entry_attempt = true;
    initial_exit_attempt = true;
    debounce_reset = true;
    sequence_bypass = false;
    entry_exit_flag = ENTRY;
}

bool Sequence::check_faults() {
    return paths[current_path_index].check_alerts(FAULT_ALERT);
}

bool Sequence::check_alarms() {
    return paths[current_path_index].check_alerts(ALARM_ALERT);
}

// code to parse each sequence from sequences.json (filling out local Sequences variables)
bool Sequence::parse_sequence(cJSON* object, states s) {
    state = s;
    cJSON* JSON_sequence_name = cJSON_GetObjectItem(object, "sequence_name");
    if (JSON_sequence_name == NULL || JSON_sequence_name->valuestring == NULL) {
        FPS_ERROR_LOG("no sequence_name found");
        return false;
    }
    sequence_name = JSON_sequence_name->valuestring;
    FPS_INFO_LOG("%s sequence:", state_name[s]);
    cJSON* JSON_paths = cJSON_GetObjectItem(object, "paths");
    if (JSON_paths == NULL || cJSON_GetArraySize(JSON_paths) == 0) {
        FPS_ERROR_LOG("no paths found or paths array empty");
        return false;
    }

    for (int i = 0; i < cJSON_GetArraySize(JSON_paths); i++) {
        // Current path being configured
        paths.emplace_back(pSite);
        Path& current_path = paths.back();

        cJSON* JSON_paths_index = cJSON_GetArrayItem(JSON_paths, i);
        if (JSON_paths_index == NULL) {
            FPS_ERROR_LOG("No object found in paths array index %d", i);
            return false;
        }
        if (current_path.configure_path(JSON_paths_index, i) == false) {
            FPS_ERROR_LOG("Failed to parse path %d", i);
            return false;
        }
    }
    return true;
}

// call_sequence executes the sequence.  this involves a series of entry actions and exit condition checks until the sequence is complete
void Sequence::call_sequence() {
    // reset target_time when a new step is entered or the the sequence is complete
    // intentionally first in function to ensure that reset happens on iteration AFTER step reset is asserted
    if (pSite->sequence_reset)
        reset_sequence();

    if (step_reset)
        reset_step();

    // sequence is complete and calling same sequence again.  so bypass until new sequence call
    if (sequence_bypass) {
        return;
    }

    // check if step or path changed and print messages
    check_path_step_change();

    // ENTRY ACTION - this counter check makes sure the entry action is only performed once
    if (check_current_step_index == current_step_index)
        call_sequence_entry();

    // EXIT CONDITION - only perform check for exit condition if entry action has successfully occurred
    if (check_current_step_index > current_step_index)
        call_sequence_exit();

    // when sequence has performed all steps, set vars to trigger new sequence execution
    if (current_step_index >= paths[current_path_index].steps.size()) {
        pSite->step_change = true;
        pSite->path_change = true;
        sequence_bypass = true;

        pSite->current_state = paths[current_path_index].return_state;
        pSite->set_site_status(sequence_name.c_str());

        if (pSite->current_state != state)
            FPS_INFO_LOG("Site Manager completed %s state, next state is %s", state_name[state], state_name[pSite->current_state]);
        else
            FPS_INFO_LOG("Site Manager completed %s state - sequence is bypassed", state_name[state]);
    }

    // if exit_timer trips, set fault
    if ((paths[current_path_index].timeout != (-1)) && check_expired_time(pSite->current_time, pSite->exit_target_time)) {
        char event_message[MEDIUM_MSG_LEN];
        snprintf(event_message, MEDIUM_MSG_LEN, "Sequence step failed: %s", paths[current_path_index].steps[current_step_index].get_name().c_str());
        emit_event("Site", event_message, FAULT_ALERT);
        pSite->set_faults(2);
        clock_gettime(CLOCK_MONOTONIC, &pSite->exit_target_time);
        pSite->exit_target_time.tv_sec += paths[current_path_index].timeout;
    }

    return;
}

// perform call_function function and return true if expected value is found
bool Sequence::call_function() {
    // parse exit_conditions for the target_asset and asset_cmd
    // Aggregated result of all step actions (OR). Ensure every action has a chance to run even if true is found
    bool return_value = false;
    switch (entry_exit_flag) {
        case ENTRY:

            for (auto& action : paths[current_path_index].steps[current_step_index].entry_actions) {
                // Get the target asset and cmd
                std::vector<std::string> route_fragments = split(action.route, "/");
                if (route_fragments.empty()) {
                    FPS_ERROR_LOG("Failed to parse entry route %s", action.route);
                    return false;
                }
                std::string target_asset = route_fragments[0];
                std::string asset_cmd = route_fragments.size() < 2 ? target_asset : route_fragments[1];

                if (initial_entry_attempt)
                    FPS_INFO_LOG("    ENTRY ACTION: call_function target_asset: %s, asset_cmd: %s", target_asset.c_str(), asset_cmd.c_str());

                action.result = pSite->call_sequence_functions(target_asset.c_str(), asset_cmd.c_str(), &action.value, action.tolerance);
                return_value = return_value || action.result;
            }
            break;
        case EXIT:
            for (auto& action : paths[current_path_index].steps[current_step_index].exit_conditions) {
                action.reason_for_exit_failure = CONDITIONAL;

                // Get the target asset and cmd
                std::vector<std::string> route_fragments = split(action.route, "/");
                if (route_fragments.empty()) {
                    FPS_ERROR_LOG("Failed to parse exit condition %s", action.route.c_str());
                    return false;
                }
                std::string target_asset = route_fragments[0];
                std::string asset_cmd = route_fragments.size() < 2 ? target_asset : route_fragments[1];

                if (initial_exit_attempt)
                    FPS_INFO_LOG("    EXIT CONDITION: call_function target_asset: %s, asset_cmd: %s", target_asset, asset_cmd);

                action.result = pSite->call_sequence_functions(target_asset.c_str(), asset_cmd.c_str(), &action.value, action.tolerance);

                // debounce check
                if (action.debounce_timer_ms == 0) {
                    return_value = return_value || action.result;
                    continue;
                }
                // reset target_time when a new step is entered or the return_value is not within range
                else if ((debounce_reset == true) || (action.result == false)) {
                    debounce_reset = false;
                    clock_gettime(CLOCK_MONOTONIC, &action.debounce_target_time);
                    action.debounce_target_time.tv_sec += action.debounce_timer_ms / 1000;
                    if (action.result == false)
                        continue;
                }
                if (check_expired_time(pSite->current_time, action.debounce_target_time)) {
                    // Debounce timer has run out
                    FPS_INFO_LOG("Debounce timer has run out \n");
                    return_value = return_value || action.result;
                } else {
                    // Debounce timer is still running
                    if (initial_exit_attempt)
                        FPS_INFO_LOG("Debounce timer started.  %ld milliseconds on clock \n", get_elapsed_time(pSite->current_time, action.debounce_target_time));
                    action.reason_for_exit_failure = DEBOUNCE;
                    continue;
                }
                if (check_expired_time(pSite->current_time, action.debounce_target_time)) {
                    // Debounce timer has run out
                    FPS_INFO_LOG("Debounce timer has run out");
                    return_value = return_value || action.result;
                } else {
                    // Debounce timer is still running
                    if (initial_exit_attempt)
                        FPS_INFO_LOG("Debounce timer started.  %ld milliseconds on clock", get_elapsed_time(pSite->current_time, action.debounce_target_time));
                    action.reason_for_exit_failure = DEBOUNCE;
                    continue;
                }
            }
            break;
    }
    return return_value;
}

void Sequence::reset_sequence() {
    // if sequence_reset is true, reset all counts in case sequence was interrupted before completion
    current_step_index = 0;
    check_current_step_index = 0;
    current_path_index = 0;
    check_current_path_index = 0;
    pSite->sequence_reset = false;
    debounce_reset = true;
    step_reset = true;
    initial_entry_attempt = true;
    initial_exit_attempt = true;
    sequence_bypass = false;
}

void Sequence::reset_step() {
    step_reset = false;
    initial_entry_attempt = true;
    initial_exit_attempt = true;
    clock_gettime(CLOCK_MONOTONIC, &pSite->exit_target_time);
    if (paths[current_path_index].timeout != -1)
        pSite->exit_target_time.tv_sec += paths[current_path_index].timeout;
}

void Sequence::call_sequence_entry() {
    debounce_reset = true;  // reset for upcoming exit call

    // message when entry action is first attempted
    if (initial_entry_attempt)
        FPS_INFO_LOG("Site Manager in State: %s, Path: %s, Step: %s", state_name[state], paths[current_path_index].path_name, paths[current_path_index].steps[current_step_index].step_name);

    entry_exit_flag = ENTRY;
    // perform call_function and increment counter if successful
    if (call_function())
        check_current_step_index++;  // iterate count to call exit on next iteration
    initial_entry_attempt = false;
}

void Sequence::call_sequence_exit() {
    entry_exit_flag = EXIT;
    // perform call_function and increment current_step_index if expected value is found
    if (call_function()) {
        for (auto& action : paths[current_path_index].steps[current_step_index].exit_conditions) {
            if (action.result) {
                FPS_INFO_LOG("Exit condition received for: %s. Proceed to next step", action.route);
            }
        }
        current_step_index++;       // iterate count for next step on next iteration
        pSite->step_change = true;  // new step will be called
        step_reset = true;
        initial_exit_attempt = true;  // reset this for next step called
    }
    // only print on first failure - expected value not found or debounce timer
    else if (initial_exit_attempt) {
        initial_exit_attempt = false;  // step called once already, dont print messages repeatedly
        for (auto& action : paths[current_path_index].steps[current_step_index].exit_conditions) {
            switch (action.reason_for_exit_failure) {
                case DEBOUNCE:
                    if (action.debounce_timer_ms > 0) {
                        std::string timeout_msg;
                        if (paths[current_path_index].timeout != -1)
                            timeout_msg = "Timeout: " + std::to_string(get_elapsed_time(pSite->current_time, pSite->exit_target_time)) + " ms";
                        else
                            timeout_msg = "No Timeout";
                        FPS_INFO_LOG("Exit delay: %s debounce timer active", action.route);
                        FPS_INFO_LOG("\tDebounce Timer: %ld ms, %s", get_elapsed_time(pSite->current_time, action.debounce_target_time), timeout_msg);
                    }
                    if (paths[current_path_index].timeout != -1)
                        FPS_WARNING_LOG("Remaining in step until exit conditions are satisfied or step times out");
                    break;
                case CONDITIONAL:
                    if (!action.result) {
                        FPS_INFO_LOG("Exit failure: %s exit condition failed. Expected value is %s", action.route, action.value.print().c_str());
                    }
                    // If path_switch true, move from current path to first step of next path, allowing a conditional exit to return early
                    if (paths[current_path_index].steps[current_step_index].get_path_switch()) {
                        pSite->path_change = true;  // new path to be called
                        pSite->step_change = true;  // new step to be called
                        step_reset = true;
                        current_path_index = paths[current_path_index].steps[current_step_index].next_path;  // get new path index
                        current_step_index = 0;                                                              // new step will be the first in the new path
                        check_current_step_index = 0;                                                        // new step will be the first in the new path
                        FPS_INFO_LOG("Path switch triggered. Moving to next path in sequence");
                        return;
                    }
                    break;
            }
        }
        FPS_WARNING_LOG("Remaining in step until exit conditions are satisfied");
    }
}

void Sequence::check_path_step_change() {
    char event_message[MEDIUM_MSG_LEN];
    if (pSite->path_change || pSite->step_change) {
        if (pSite->path_change) {
            // FPS_ERROR_LOG("Site Manager path change to: %s", paths[current_path_index].get_name());
            snprintf(event_message, MEDIUM_MSG_LEN, "Path changed to %s", paths[current_path_index].path_name.c_str());
            emit_event("Site", event_message, STATUS_ALERT);
            pSite->path_change = false;
        }
        if (pSite->step_change) {
            // FPS_ERROR_LOG("Site Manager step change to: %s", paths[current_path_index].steps[current_step_index].get_name());
            snprintf(event_message, MEDIUM_MSG_LEN, "Step changed to %s", paths[current_path_index].steps[current_step_index].step_name.c_str());
            emit_event("Site", event_message, STATUS_ALERT);
            pSite->step_change = false;
        }

        snprintf(event_message, MEDIUM_MSG_LEN, "%s: %s", paths[current_path_index].path_name.c_str(), paths[current_path_index].steps[current_step_index].step_name.c_str());
        pSite->set_site_status(event_message);
    }
}
