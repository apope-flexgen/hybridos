/*
 * Sequence.cpp
 *
 *  Created on: Nov 5, 2019
 *      Author: kbrezina
 */

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
/* Local Internal Dependencies */
#include <Sequence.h>
#include <Site_Manager.h>
#include <Site_Controller_Utils.h>
#include <numeric>
#include "Types.h"

#define ENTRY '0'
#define EXIT '1'
#define DEBOUNCE '0'
#define CONDITIONAL '1'

/**
 * @brief Sequences_Status constructor
 */
Sequences_Status::Sequences_Status() {
    current_state = states::Init;
    check_current_state = states::Init;
    step_change = true;
    path_change = true;
    sequence_reset = false;
    should_pub = false;
}

/** 
 * @brief This default sequence is used for the action class
 * Which calls this implicitly in it's constructor.
 */
Sequence::Sequence() {
    site = nullptr;
    sequence_type = Sequence_Type::Site;
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

/** 
 * @brief The original sequence which is used for managing 
 * sequences.json at the Site_Manager level.
 */
Sequence::Sequence(Site_Manager* siteref) {
    site = siteref;
    sequence_type = Sequence_Type::Site;
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
    return paths[current_path_index].check_alerts(FAULT_ALERT, sequence_type);
}

bool Sequence::check_alarms() {
    return paths[current_path_index].check_alerts(ALARM_ALERT, sequence_type);
}

// code to parse each sequence from sequences.json (filling out local Sequences variables)
bool Sequence::parse(cJSON* object, states State) {
    state = State;
    cJSON* JSON_sequence_name = cJSON_GetObjectItem(object, "sequence_name");
    if (JSON_sequence_name == nullptr || JSON_sequence_name->valuestring == nullptr) {
        FPS_ERROR_LOG("no sequence_name found");
        return false;
    }
    sequence_name = JSON_sequence_name->valuestring;
    FPS_INFO_LOG("%s sequence:", state_name[State]);
    cJSON* JSON_paths = cJSON_GetObjectItem(object, "paths");
    if (JSON_paths == nullptr || cJSON_GetArraySize(JSON_paths) == 0) {
        FPS_ERROR_LOG("no paths found or paths array empty");
        return false;
    }

    for (int i = 0; i < cJSON_GetArraySize(JSON_paths); i++) {
        // Current path being configured
        paths.emplace_back(site);
        Path& current_path = paths.back();

        cJSON* JSON_paths_index = cJSON_GetArrayItem(JSON_paths, i);
        if (JSON_paths_index == nullptr) {
            FPS_ERROR_LOG("No object found in paths array index %d", i);
            return false;
        }
        if (!current_path.configure_path(JSON_paths_index)) {
            FPS_ERROR_LOG("Failed to parse path %d", i);
            return false;
        }
    }
    return true;
}

// call_sequence executes the sequence.  this involves a series of entry actions and exit condition checks until the sequence is complete
void Sequence::call_sequence(Sequences_Status& sequences_status) {
    // reset target_time when a new step is entered or the the sequence is complete
    // intentionally first in function to ensure that reset happens on iteration AFTER step reset is asserted
    if (sequences_status.sequence_reset) {
        reset_sequence(sequences_status);
    }

    if (step_reset) {
        reset_step(sequences_status);
    }

    // sequence is complete and calling same sequence again.  so bypass until new sequence call
    if (sequence_bypass) {
        return;
    }

    // check if step or path changed and print messages
    check_path_step_change(sequences_status);

    // ENTRY ACTION - this counter check makes sure the entry action is only performed once
    if (check_current_step_index == current_step_index) {
        call_sequence_entry(sequences_status);
    }

    // EXIT CONDITION - only perform check for exit condition if entry action has successfully occurred
    if (check_current_step_index > current_step_index) {
        call_sequence_exit(sequences_status);
    }

    // when sequence has performed all steps, set vars to trigger new sequence execution
    if (current_step_index >= paths[current_path_index].steps.size()) {
        sequences_status.step_change = true;
        sequences_status.path_change = true;
        sequence_bypass = true;
        sequences_status.current_state = paths[current_path_index].return_state;
        site->set_site_status(sequence_name.c_str());

        if (sequences_status.current_state != state) {
            FPS_INFO_LOG("Site Manager completed %s state, next state is %s.", state_name[state], state_name[sequences_status.current_state]);
        } else {
            FPS_INFO_LOG("Site Manager completed %s state, sequence is bypassed.", state_name[state]);
        }
    }

    // if exit_timer trips, set fault
    if ((paths[current_path_index].timeout != (-1)) && check_expired_time(sequences_status.current_time, sequences_status.exit_target_time)) {
        char event_message[MEDIUM_MSG_LEN];
        snprintf(event_message, MEDIUM_MSG_LEN, "Sequence step failed: %s", paths[current_path_index].steps[current_step_index].get_name().c_str());
        emit_event("Site", event_message, FAULT_ALERT);
        clock_gettime(CLOCK_MONOTONIC, &sequences_status.exit_target_time);
        sequences_status.exit_target_time.tv_sec += paths[current_path_index].timeout;
        site->set_faults(2);
    }
}

void Sequence::handle_entry_action(bool& return_value) {
    for (auto& action : paths[current_path_index].steps[current_step_index].entry_actions) {
        // Get the target asset and cmd
        std::vector<std::string> route_fragments = split(action.route, "/");
        if (route_fragments.empty()) {
            FPS_ERROR_LOG("Failed to parse entry route %s", action.route);
            return;
        }
        std::string target_asset = route_fragments[0];
        std::string asset_cmd = route_fragments.size() < 2 ? target_asset : route_fragments[1];

        if (initial_entry_attempt) {
            FPS_INFO_LOG("ENTRY ACTION: call_function target_asset: %s, asset_cmd: %s", target_asset.c_str(), asset_cmd.c_str());
        }
        switch (sequence_type) {
            case Sequence_Type::Site:
                action.result = site->call_sequence_functions(target_asset.c_str(), asset_cmd.c_str(), &action.value, action.tolerance);
                break;
            case Sequence_Type::Asset_ESS:
                // Any function here will be a single fragment since you can deduce the asset since it's attached to the asset
                // this is why you don't need target_asset for this version of the function
                action.result = asset_ess->call_action_functions(asset_cmd.c_str(), &action.value, action.tolerance);
                break;
            default:
                // TODO(unknown): Add other sequence types
                FPS_ERROR_LOG("Invalid sequence type");
                return;
        }
        return_value = return_value || action.result;
    }
}

void Sequence::handle_exit_action(bool& return_value, Sequences_Status& sequences_status) {
    for (auto& action : paths[current_path_index].steps[current_step_index].exit_conditions) {
        action.reason_for_exit_failure = CONDITIONAL;

        // Get the target asset and cmd
        std::vector<std::string> route_fragments = split(action.route, "/");
        if (route_fragments.empty()) {
            FPS_ERROR_LOG("Failed to parse exit condition %s", action.route.c_str());
            return;
        }
        std::string target_asset = route_fragments[0];
        std::string asset_cmd = route_fragments.size() < 2 ? target_asset : route_fragments[1];

        if (initial_exit_attempt) {
            FPS_INFO_LOG("EXIT CONDITION: call_function target_asset: %s, asset_cmd: %s", target_asset, asset_cmd);
        }

        switch (sequence_type) {
            case Sequence_Type::Site:
                action.result = site->call_sequence_functions(target_asset.c_str(), asset_cmd.c_str(), &action.value, action.tolerance);
                break;
            case Sequence_Type::Asset_ESS:
                action.result = asset_ess->call_action_functions(asset_cmd.c_str(), &action.value, action.tolerance);
                break;
            default:
                // TODO(unknown): Add other sequence Types
                FPS_ERROR_LOG("Invalid sequence type");
                return;
        }

        // debounce check
        if (action.debounce_timer_ms == 0) {
            return_value = return_value || action.result;
            continue;
        }
        // reset target_time when a new step is entered or the return_value is not within range
        if ((debounce_reset) || (!action.result)) {
            debounce_reset = false;
            clock_gettime(CLOCK_MONOTONIC, &action.debounce_target_time);
            action.debounce_target_time.tv_sec += action.debounce_timer_ms / 1000;
            if (!action.result) {
                continue;
            }
        }
        if (check_expired_time(sequences_status.current_time, action.debounce_target_time)) {
            // Debounce timer has run out
            FPS_INFO_LOG("Debounce timer has run out \n");
            return_value = return_value || action.result;
        } else {
            // Debounce timer is still running
            if (initial_exit_attempt) {
                FPS_INFO_LOG("Debounce timer started.  %ld milliseconds on clock \n", get_elapsed_time(sequences_status.current_time, action.debounce_target_time));
            }
            action.reason_for_exit_failure = DEBOUNCE;
            continue;
        }
    }
}

// perform call_function function and return true if expected value is found
bool Sequence::call_function(Sequences_Status& sequences_status) {
    // parse exit_conditions for the target_asset and asset_cmd
    // Aggregated result of all step actions (OR). Ensure every action has a chance to run even if true is found
    bool return_value = false;
    switch (entry_exit_flag) {
        case ENTRY:
            handle_entry_action(return_value);
            break;
        case EXIT:
            handle_exit_action(return_value, sequences_status);
            break;
        default:
            FPS_ERROR_LOG("Invalid entry_exit_flag");
    }
    return return_value;
}

void Sequence::reset_sequence(Sequences_Status& sequences_status) {
    // if sequence_reset is true, reset all counts in case sequence was interrupted before completion
    current_step_index = 0;
    check_current_step_index = 0;
    current_path_index = 0;
    check_current_path_index = 0;
    sequences_status.sequence_reset = false;
    debounce_reset = true;
    step_reset = true;
    initial_entry_attempt = true;
    initial_exit_attempt = true;
    sequence_bypass = false;
}

void Sequence::reset_step(Sequences_Status& sequences_status) {
    step_reset = false;
    initial_entry_attempt = true;
    initial_exit_attempt = true;
    clock_gettime(CLOCK_MONOTONIC, &sequences_status.exit_target_time);
    if (paths[current_path_index].timeout != -1) {
        sequences_status.exit_target_time.tv_sec += paths[current_path_index].timeout;
    }
}

void Sequence::call_sequence_entry(Sequences_Status& sequences_status) {
    debounce_reset = true;  // reset for upcoming exit call

    // message when entry action is first attempted
    if (initial_entry_attempt) {
        switch (sequence_type) {
            case Sequence_Type::Site:
                FPS_INFO_LOG("Site Manager in State: %s, Path: %s, Step: %s", 
                        state_name[state], paths[current_path_index].path_name, 
                        paths[current_path_index].steps[current_step_index].step_name);
            default:
                break;
        }

    }

    entry_exit_flag = ENTRY;
    // perform call_function and increment counter if successful
    if (call_function(sequences_status)) {
        check_current_step_index++;  // iterate count to call exit on next iteration
    }

    initial_entry_attempt = false;
}

void Sequence::handle_initial_exit_attempt(Sequences_Status& sequences_status) {
    initial_exit_attempt = false;  // step called once already, dont print messages repeatedly
    for (auto& action : paths[current_path_index].steps[current_step_index].exit_conditions) {
        switch (action.reason_for_exit_failure) {
            case DEBOUNCE:
                if (action.debounce_timer_ms > 0) {
                    std::string timeout_msg;
                    if (paths[current_path_index].timeout != -1) {
                        timeout_msg = "Timeout: " + std::to_string(get_elapsed_time(sequences_status.current_time, sequences_status.exit_target_time)) + " ms";
                    } else {
                        timeout_msg = "No Timeout";
                    }
                    FPS_INFO_LOG("Exit delay: %s debounce timer active", action.route);
                    FPS_INFO_LOG("Debounce Timer: %ld ms, %s", get_elapsed_time(sequences_status.current_time, action.debounce_target_time), timeout_msg);
                }
                if (paths[current_path_index].timeout != -1) {
                    FPS_WARNING_LOG("Remaining in step until exit conditions are satisfied or step times out");
                }
                break;
            case CONDITIONAL:
                if (!action.result) {
                    FPS_INFO_LOG("Exit failure: %s exit condition failed. Expected value is %s", action.route, action.value.print().c_str());
                }
                // If path_switch true, move from current path to first step of next path, allowing a conditional exit to return early
                if (paths[current_path_index].steps[current_step_index].get_path_switch()) {
                    sequences_status.path_change = true;  // new path to be called
                    sequences_status.step_change = true;  // new step to be called
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

void Sequence::call_sequence_exit(Sequences_Status& sequences_status) {
    entry_exit_flag = EXIT;
    // perform call_function and increment current_step_index if expected value is found
    if (call_function(sequences_status)) {
        for (auto& action : paths[current_path_index].steps[current_step_index].exit_conditions) {
            if (action.result) {
                FPS_INFO_LOG("Exit condition received for: %s. Proceed to next step", action.route);
            }
        }
        current_step_index++;                 // iterate count for next step on next iteration
        sequences_status.step_change = true;  // new step will be called
        step_reset = true;
        initial_exit_attempt = true;  // reset this for next step called
    } else if (initial_exit_attempt) {
        handle_initial_exit_attempt(sequences_status);  // only occurs once per step
    }
}

void Sequence::check_path_step_change(Sequences_Status& sequences_status) {
    char event_message[MEDIUM_MSG_LEN];
    if (sequences_status.path_change || sequences_status.step_change) {
        sequences_status.should_pub = true;
        if (sequences_status.path_change) {
            // FPS_ERROR_LOG("Site Manager path change to: %s", paths[current_path_index].get_name());
            snprintf(event_message, MEDIUM_MSG_LEN, "Path changed to %s", paths[current_path_index].path_name.c_str());
            emit_event("Site", event_message, STATUS_ALERT);
            sequences_status.path_change = false;
        }
        if (sequences_status.step_change) {
            // FPS_ERROR_LOG("Site Manager step change to: %s", paths[current_path_index].steps[current_step_index].get_name());
            snprintf(event_message, MEDIUM_MSG_LEN, "Step changed to %s", paths[current_path_index].steps[current_step_index].step_name.c_str());
            emit_event("Site", event_message, STATUS_ALERT);
            sequences_status.step_change = false;
        }

        snprintf(event_message, MEDIUM_MSG_LEN, "%s: %s", paths[current_path_index].path_name.c_str(), paths[current_path_index].steps[current_step_index].step_name.c_str());
        // if using a site, set the site status
        if (sequence_type == Sequence_Type::Site) {
            site->set_site_status(event_message);
        } else {
            // TODO handle the action status
        }
    }
}

/**
 * @brief Takes the path/step index and returns the debounce_timer_ms of that step.
 * There is not at present a smart way to report on steps without a debounce_timer_ms. 
 * But one could be implemented.
 * @param action (const Action&) The action to index into.
 * @return The number of seconds as an int.
 */
int Sequence::collect_seconds_remaining_in_current_step() {
    if (current_path_index >= paths.size()) {
        return 0;
    }
    if (current_step_index >= paths[current_path_index].steps.size()) {
        return 0;
    }

    // vector of exit_steps (should contain the debounce_timer_ms)
    std::vector<Step_Action> step_actions = paths[current_path_index].steps[current_step_index].exit_conditions;

    auto start_iterator = step_actions.begin();
    return std::accumulate(start_iterator, step_actions.end(), 0,
        [](int currentSum, const Step_Action& step_action) {
            return currentSum + step_action.debounce_timer_ms; // just a sum of total possible time. Not a live decrease.
        }
    );
}

/**
 * @brief Returns the sum of the remaining steps in an action. Will omit steps that have already
 * been run. There is not at present a smart way to report on steps without a debounce_timer_ms. 
 * But one could be implemented.
 * @param action (const Step&) The Step to report on.
 * @return The number of seconds as an int.
 */
int Sequence::collect_seconds_remaining_in_action() {
    if (current_path_index >= paths.size()) {
        return 0;
    }
    if (current_step_index >= paths[current_path_index].steps.size()) {
        return 0;
    }

    // vector of exit_steps (should contain the debounce_timer_ms)
    std::vector<Step> steps = paths[current_path_index].steps;
    // ignore time before this step 
    uint step_index = current_step_index;
    auto start_iterator = steps.begin();
    if (step_index < steps.size()) {
         start_iterator += step_index;
    } else if (step_index > steps.size()) {
        FPS_ERROR_LOG("invalid step index returning 0");
        return 0;
    }

    return std::accumulate(start_iterator, steps.end(), 0,
        [](int currentSum, const Step& step) {
            return currentSum + step.collect_seconds_in_step(); 
        }
    );
}


