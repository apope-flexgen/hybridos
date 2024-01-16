/*
 * Action.cpp
 *
 *  Created on: Nov 20, 2023
 *      Author: Judsen (github: JudsenAtFlexgen)
 */

/* C Standard Library Dependencies */
#include <cstdint>
#include <cstring>
#include <string>
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
/* Local Internal Dependencies */
#include <Action.h>
#include <Site_Manager.h>
#include <Site_Controller_Utils.h>
#include <Asset.h>
#include <Types.h>

#define ENTRY '0'
#define EXIT '1'
#define DEBOUNCE '0'
#define CONDITIONAL '1'

/**
 * @brief Extension of the Sequence_Status constructor.
 * Also initializes the current_sequence_name string to be empty.
 */
Action_Status::Action_Status() {
    current_sequence_name = "";
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    clock_gettime(CLOCK_MONOTONIC, &exit_target_time);
}

/**
 * @brief Action constructor.
 */
Action::Action() {
    sequence_name = "";
    paths = {};
    debounce_reset = true;
    status = ACTION_STATUS_STATE::INACTIVE;
}

/** 
 * @brief Parse and configure paths/steps for this action.
 * @param object (cJSON*) the actual actions.json entry.
 * @return bool (success)
 */
bool Action::parse(cJSON* object) {
    cJSON* JSON_name = cJSON_GetObjectItem(object, "action_name");
    if (JSON_name == NULL || JSON_name->valuestring == NULL) {
        FPS_ERROR_LOG("no action_name found");
        return false;
    }
    sequence_name = JSON_name->valuestring;
    cJSON* JSON_paths = cJSON_GetObjectItem(object, "paths");
    if (JSON_paths == NULL || cJSON_GetArraySize(JSON_paths) == 0) {
        FPS_ERROR_LOG("no paths found or paths array empty");
        return false;
    }

    for (int i = 0; i < cJSON_GetArraySize(JSON_paths); i++) {
        paths.emplace_back();
        Path& current_path = paths.back();
        cJSON* JSON_paths_index = cJSON_GetArrayItem(JSON_paths, i);
        if (JSON_paths_index == NULL) {
            FPS_ERROR_LOG("No object found in paths array index %d", i);
            return false;
        }
        if (!current_path.configure_path(JSON_paths_index, false)) {
            FPS_ERROR_LOG("Failed to parse path %d", i);
            return false;
        }
    }
    return true;
}

/**
 * @brief Maps the status field to the appropriate string.
 * @return The appropriate string.
 */
std::string Action::status_string() const {
    switch (status) {
        case ACTION_STATUS_STATE::INACTIVE:
            return "Inactive";
        case ACTION_STATUS_STATE::IN_PROGRESS:
            return "In Progress";
        case ACTION_STATUS_STATE::COMPLETED:
            return "Completed";
        case ACTION_STATUS_STATE::FAILED:
            return "Failed";
        case ACTION_STATUS_STATE::ABORTED:
            return "Aborted";
        default:
            FPS_ERROR_LOG("Something very strange has happened. Unrecognized action_status_state: %d", status);
            return "Error State";
    }
}

void Action::enter_automation(Action_Status& action_status, std::string action_id) {
    current_step_index = 0;
    check_current_step_index = 0;
    current_path_index = 0;
    check_current_path_index = 0;
    debounce_reset = true;
    step_reset = true;
    initial_entry_attempt = true;
    initial_exit_attempt = true;
    sequence_bypass = false;
    action_status.current_sequence_name = action_id;
    action_status.step_change = true;
    status = ACTION_STATUS_STATE::IN_PROGRESS;
}

void Action::exit_automation(Action_Status& action_status, ACTION_STATUS_STATE set_status) {
    action_status.current_sequence_name.clear();
    action_status.should_pub = true;
    status = set_status;
}

/**
 * @brief Calls and updates sequences functions.
 */
void Action::call_sequence(Action_Status& action_status) {
    if (step_reset) {
        reset_step(action_status);
    }

    // check if step or path changed and print messages
    check_path_step_change(action_status);

    // handle entry/exit conditions
    if (check_current_step_index == current_step_index) {
        call_sequence_entry(action_status);
    }
    if (check_current_step_index > current_step_index) {
        call_sequence_exit(action_status);
    }

    // when sequence has performed all steps, set vars to trigger new sequence execution
    if (current_step_index >= paths[current_path_index].steps.size()) {
        FPS_INFO_LOG("Automated Action: %s completed!", action_status.current_sequence_name);
        exit_automation(action_status, ACTION_STATUS_STATE::COMPLETED);
    }

    // if exit_timer trips, set fault
    if ((paths[current_path_index].timeout != (-1)) && check_expired_time(action_status.current_time, action_status.exit_target_time)) {
        char event_message[MEDIUM_MSG_LEN];
        snprintf(event_message, MEDIUM_MSG_LEN, "Action step failed: %s", paths[current_path_index].steps[current_step_index].get_name().c_str());
        emit_event("Asset", event_message, FAULT_ALERT);
        clock_gettime(CLOCK_MONOTONIC, &action_status.exit_target_time);
        action_status.exit_target_time.tv_sec += paths[current_path_index].timeout;

        switch (sequence_type) {
            case (Sequence_Type::Asset_ESS):
                asset_ess->actions_faults.value.value_bit_field |= static_cast<uint64_t>(0x2);
            case (Sequence_Type::Asset_Solar):
                asset_solar->actions_faults.value.value_bit_field |= static_cast<uint64_t>(0x2);
            case (Sequence_Type::Asset_Generator):
                asset_generator->actions_faults.value.value_bit_field |= static_cast<uint64_t>(0x2);
            case (Sequence_Type::Asset_Feeder):
                asset_feeder->actions_faults.value.value_bit_field |= static_cast<uint64_t>(0x2);
            default:
                FPS_ERROR_LOG("Entered state not intended. Action has invalid sequence_type.");
        }

        exit_automation(action_status, ACTION_STATUS_STATE::FAILED);
    }
}
