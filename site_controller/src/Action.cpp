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
#include <vector>
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
    shutdown_sequence = ""; 
    is_shutdown_sequence = false;
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
    
    cJSON* shutdown_name = cJSON_GetObjectItem(object, "shutdown_sequence");
    if (shutdown_name != nullptr) {
        if ((!static_cast<bool>(cJSON_IsString(shutdown_name)) && (shutdown_name->valuestring != nullptr))) {
            FPS_ERROR_LOG("Provided a shutdown_sequence for action \"%s\", but did not provide a string.", sequence_name);
            return false;
        }
        shutdown_sequence = shutdown_name->valuestring;
    }

    cJSON* is_shutdown = cJSON_GetObjectItem(object, "is_shutdown_sequence");
    if (is_shutdown != nullptr) {
        if (!static_cast<bool>(cJSON_IsBool(is_shutdown))) {
            FPS_ERROR_LOG("Attempted to configure is_shutdown parameter for action %s, but used a type other than Bool.", sequence_name);
            return false;
        }
        is_shutdown_sequence = static_cast<bool>(is_shutdown->valueint);
    }

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
        case ACTION_STATUS_STATE::EXITING:
            return "Exiting";
        default:
            FPS_ERROR_LOG("Something very strange has happened. Unrecognized action_status_state: %d", status);
            return "Error State";
    }
}

/**
 * @brief assigns this actions->asset->quick_action_access variable to nullptr.
 */
void Action::decouple_quick_action_access() {
    // #### disable out quick access pointer ####
    switch (sequence_type) {
        case (Sequence_Type::Asset_ESS):
            asset_ess->quick_action_access = nullptr;
            break;
        case (Sequence_Type::Asset_Solar):
            asset_solar->quick_action_access = nullptr;
            break;
        case (Sequence_Type::Asset_Generator):
            asset_generator->quick_action_access = nullptr;
            break;
        case (Sequence_Type::Asset_Feeder):
            asset_feeder->quick_action_access = nullptr;
            break;
        default:
            FPS_ERROR_LOG("Entered state not intended. Action has invalid sequence_type.");
    }
}

/**
 * @brief assigns a pointer to this actions->asset->quick_action_access variable
 * to this action. Prevents the need to loop over the actions vector multiple times.
 */
void Action::couple_quick_action_access() {
    // #### disable out quick access pointer ####
    switch (sequence_type) {
        case (Sequence_Type::Asset_ESS):
            asset_ess->quick_action_access = this;
            break;
        case (Sequence_Type::Asset_Solar):
            asset_solar->quick_action_access = this;
            break;
        case (Sequence_Type::Asset_Generator):
            asset_generator->quick_action_access = this;
            break;
        case (Sequence_Type::Asset_Feeder):
            asset_feeder->quick_action_access = this;
            break;
        default:
            FPS_ERROR_LOG("Entered state not intended. Action has invalid sequence_type.");
    }
}

/**
 * @brief Grab a pointer to this assets shutdown_actions vector.
 * Does the following 
 * this->asset->shutdown_actions
 *
 * @return std::vector<Action>* 
 */
std::vector<Action>* Action::get_shutdown_actions() {
    // #### Handle the Shutdown sequence ####
    std::vector<Action>* actions_ref = nullptr;
    switch (sequence_type) {
        case (Sequence_Type::Asset_ESS):
            actions_ref = &asset_ess->shutdown_actions;
            break;
        case (Sequence_Type::Asset_Solar):
            actions_ref = &asset_ess->shutdown_actions;
            break;
        case (Sequence_Type::Asset_Generator):
            actions_ref = &asset_ess->shutdown_actions;
            break;
        case (Sequence_Type::Asset_Feeder):
            actions_ref = &asset_ess->shutdown_actions;
            break;
        default:
            FPS_ERROR_LOG("Entered state not intended. Action has invalid sequence_type.");
    }
    return actions_ref;
}

/**
 * @brief Grab a pointer to this assets actions vector.
 * Does the following 
 * this->asset->shutdown_actions
 *
 * @return std::vector<Action>* 
 */
std::vector<Action>* Action::get_actions() {
    // #### Handle the Shutdown sequence ####
    std::vector<Action>* actions_ref = nullptr;
    switch (sequence_type) {
        case (Sequence_Type::Asset_ESS):
            actions_ref = &asset_ess->actions;
            break;
        case (Sequence_Type::Asset_Solar):
            actions_ref = &asset_ess->actions;
            break;
        case (Sequence_Type::Asset_Generator):
            actions_ref = &asset_ess->actions;
            break;
        case (Sequence_Type::Asset_Feeder):
            actions_ref = &asset_ess->actions;
            break;
        default:
            FPS_ERROR_LOG("Entered state not intended. Action has invalid sequence_type.");
    }
    return actions_ref;
}

/**
 * @brief Clears an action
 *
 * @param action_status (Action_Status&) 
 * Whenever this is called you should repub that the status is 
 * now inactive. Note that sequence_rest does nothing for Actions but 
 * is modified by this function. You can ignore it. 
 */
void Action::clear_action(Action_Status& action_status) {
    Sequence::reset_sequence(action_status);
    status = ACTION_STATUS_STATE::INACTIVE;
    action_status.should_pub = true;
}

/**
 * @brief Puts an action into an active state. Only one active action is currently supported.
 *
 * @param action_status (Action_Status&) Need to tell actions to pub. And make this the IN_PROGRESS action.
 */
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

/**
 * @brief Puts an action into an exited state (Stopped, completed) state. 
 * Pass whatever state you want to exit with.
 *
 * @param action_status (Action_Status&) Need to tell actions to pub. And that another action
 * can now occur.
 */
void Action::exit_automation(Action_Status& action_status, ACTION_STATUS_STATE set_status) {
    action_status.should_pub = true;
    decouple_quick_action_access();

    if (shutdown_sequence.empty()) {
        // if this is a shutdown_sequence you should also update the EXITING non-shutdown action
        // conveniently it is only possible for 1 action to be exiting so we don't need to know 
        // exactly which action called this shutdown. We can deduce based off of EXITING.
        if (is_shutdown_sequence) {
            for (Action& action : *get_actions()) {
                if (action.status == ACTION_STATUS_STATE::EXITING) {
                    action.status = action.status_swap;
                }
            }
        }
        status = set_status;
        action_status.current_sequence_name.clear();
        return;
    }

    status = ACTION_STATUS_STATE::EXITING;
    status_swap = set_status;

    for (Action& action : *get_shutdown_actions()) {
        if (action.sequence_name == shutdown_sequence) {
            action.enter_automation(action_status, shutdown_sequence);
            action.couple_quick_action_access();
        }
    }
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
                break;
            case (Sequence_Type::Asset_Solar):
                asset_solar->actions_faults.value.value_bit_field |= static_cast<uint64_t>(0x2);
                break;
            case (Sequence_Type::Asset_Generator):
                asset_generator->actions_faults.value.value_bit_field |= static_cast<uint64_t>(0x2);
                break;
            case (Sequence_Type::Asset_Feeder):
                asset_feeder->actions_faults.value.value_bit_field |= static_cast<uint64_t>(0x2);
                break;
            default:
                FPS_ERROR_LOG("Entered state not intended. Action has invalid sequence_type.");
        }

        exit_automation(action_status, ACTION_STATUS_STATE::FAILED);
    }
}

/**
 * @brief Handles calling sequence funtions for an action as well as exit_automation();
 * Updates faults and alarms.
 */
void Action::process(Action_Status& action_status) {
    if (!is_shutdown_sequence) {
        // if the action has faulted "shutdown" the action
        if (check_faults()) {
            check_alarms();
            // goto failed state
            exit_automation(action_status, ACTION_STATUS_STATE::FAILED);
            return;
        } 
        check_alarms(); // update alarms
    }
    call_sequence(action_status); // call the sequence functions
}
