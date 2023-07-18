/*
 * Step.cpp
 *
 *  Created on: Nov 5, 2019
 *      Author: kbrezina
 */

/* C Standard Library Dependencies */
#include <cstring>
#include <cstdio>
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
/* Local Internal Dependencies */
#include <fims/libfims.h>
#include <Step.h>


bool Step::get_path_switch()
{
    return path_switch;
}

int Step::get_next_path()
{
    return next_path;
}

const std::string Step::get_name()
{
    return step_name;
}

std::vector<Step_Action>& Step::get_entry_actions()
{
    return entry_actions;
}

std::vector<Step_Action>& Step::get_exit_conditions()
{
    return exit_conditions;
}

/**
 * Parse and handle step-level configuration
 * @param object the current step object being parsed
 * @param step_index index of the current step, used for logging
 * @return Whether the step was configured successfully
 */
bool Step::configure_step(cJSON* object, int step_index)
{
    cJSON* JSON_step_name = cJSON_GetObjectItem(object, "step_name");
    if (JSON_step_name == NULL || JSON_step_name->valuestring == NULL)
    {
        FPS_ERROR_LOG("no step_name found");
        return false;
    }
    step_name = JSON_step_name->valuestring;
    FPS_INFO_LOG("    step %i: %s \n",step_index, step_name);

    cJSON* JSON_path_switch = cJSON_GetObjectItem(object, "path_switch");
    path_switch = JSON_path_switch == NULL ? false : JSON_path_switch->type == cJSON_True;

    cJSON* JSON_next_path = cJSON_GetObjectItem(object, "next_path");
    next_path = JSON_next_path == NULL ? 0 : JSON_next_path->valueint;
    if (path_switch)
        FPS_INFO_LOG("      path_switch - next path: %i \n", next_path);

    cJSON* JSON_entry_actions = cJSON_GetObjectItem(object, "entry_actions");
    if (JSON_entry_actions == NULL)
    {
        FPS_ERROR_LOG("no entry_actions found");
        return false;
    }

    // (new format) If an array of entry actions is given, parse each action as its own object
    if (cJSON_IsArray(JSON_entry_actions))
    {
        for (int i = 0; i < cJSON_GetArraySize(JSON_entry_actions); ++i)
        {
            cJSON* entry_action = cJSON_GetArrayItem(JSON_entry_actions, i);
            if (!configure_action(entry_actions, entry_action))
                return false;
        }
    }
    // (legacy format) Parse a single action and insert it into the list of actions
    else
    {
        if (!configure_action(entry_actions, JSON_entry_actions))
            return false;
    }

    cJSON* JSON_exit_conditions = cJSON_GetObjectItem(object, "exit_conditions");
    if (JSON_exit_conditions == NULL) {
        FPS_ERROR_LOG("no exit_condition found");
        return false;
    }

    // (new format) If an array of actions is given, parse each action as its own object
    if (cJSON_IsArray(JSON_exit_conditions))
    {
        for (int i = 0; i < cJSON_GetArraySize(JSON_exit_conditions); ++i)
        {
            cJSON* exit_condition = cJSON_GetArrayItem(JSON_exit_conditions, i);
            if (!configure_action(exit_conditions, exit_condition))
                return false;
        }
    }
    // (legacy format) Parse a single action and insert it into the list of actions
    else
    {
        if (!configure_action(exit_conditions, JSON_exit_conditions))
            return false;
    }

    return true;
}

/**
 * Parse and handle action level configuration
 * @param action_list The entry action/exit condition list to which the action should be added
 * @param JSON_action The current action being parsed from configuration
 * @return Whether the action was configured successfully
 */
bool Step::configure_action(std::vector<Step_Action>& action_list, cJSON* JSON_action)
{
    // Current action being parsed
    Step_Action current_action;

    cJSON* JSON_entry_route = cJSON_GetObjectItem(JSON_action, "route");
    if (JSON_entry_route == NULL || JSON_entry_route->valuestring == NULL)
    {
        FPS_ERROR_LOG("no entry route found");
        return false;
    }
    if (JSON_entry_route->valuestring[0] != '/')
    {
        FPS_ERROR_LOG("entry route must begin with a slash");
        return false;
    }
    current_action.route = std::string(JSON_entry_route->valuestring);
    FPS_INFO_LOG("      entry route: %s\n", current_action.route);

    cJSON* JSON_entry_value = cJSON_GetObjectItem(JSON_action, "value");
    if (JSON_entry_value == NULL)
    {
        FPS_ERROR_LOG("no entry value found\n");
        return false;
    }
    if ((JSON_entry_value->type == cJSON_False) || (JSON_entry_value->type == cJSON_True))
        current_action.value.set(JSON_entry_value->type == cJSON_True);
    else if (JSON_entry_value->type == cJSON_Number)
        current_action.value.set((float)JSON_entry_value->valuedouble);
    else if (JSON_entry_value->type == cJSON_String)
        current_action.value.set(JSON_entry_value->valuestring);
    
    const char* body = current_action.value.print();
    if (body == NULL)
    {
        FPS_ERROR_LOG("failed to print entry value");
        return false;
    }
    FPS_INFO_LOG("      entry value: %s\n", body);
    delete body;

    cJSON* JSON_tolerance = cJSON_GetObjectItem(JSON_action, "tolerance");
    current_action.tolerance = JSON_tolerance == NULL ? 0 : JSON_tolerance->valueint;
    if (current_action.tolerance != 0)
        FPS_INFO_LOG("      tolerance: %d \n",  current_action.tolerance);

    cJSON* JSON_debounce_timer = cJSON_GetObjectItem(JSON_action, "debounce_timer");
    current_action.debounce_timer_ms = JSON_debounce_timer == NULL ? 0 : JSON_debounce_timer->valueint;
    if (current_action.debounce_timer_ms != 0)
        FPS_INFO_LOG("      debounce timer: %d ms\n", current_action.debounce_timer_ms);
    
    action_list.push_back(current_action);
    return true;
}

