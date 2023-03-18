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

Step::Step()
{
    step_name = NULL;
    entry_route = NULL;
    exit_route = NULL;
    tolerance = 0;
    debounce_timer = 0;
    path_switch = false;
    next_path = 0;
    index = 0;
}

Step::~Step()
{
    if (step_name != NULL)
        free(step_name);
    if (entry_route != NULL)
        free(entry_route);
    if (exit_route != NULL)
        free(exit_route);
}

bool Step::get_path_switch()
{
    return path_switch;
}

int Step::get_next_path()
{
    return next_path;
}

const char* Step::get_name()
{
    return step_name;
}

const char* Step::get_entry_route()
{
    return entry_route;
}

const char* Step::get_exit_route()
{
    return exit_route;
}

Value_Object* Step::get_entry_value()
{
    return &entry_value;
}

Value_Object* Step::get_exit_value()
{
    return & exit_value;
}

int Step::get_tolerance() const
{
    return tolerance;
}

int Step::get_debounce_timer() const
{
    return debounce_timer;
}

bool Step::configure_step(cJSON* object, int step_index)
{
    index = step_index;
    cJSON* JSON_step_name = cJSON_GetObjectItem(object, "step_name");
    if (JSON_step_name == NULL)
    {
        FPS_ERROR_LOG("no step_name found");
        return false;
    }
    step_name = strdup(JSON_step_name->valuestring);
    FPS_INFO_LOG("    step %i: %s \n",index, step_name);

    cJSON* JSON_path_switch = cJSON_GetObjectItem(object, "path_switch");
    path_switch = JSON_path_switch == NULL ? false : JSON_path_switch->type == cJSON_True;

    cJSON* JSON_next_path = cJSON_GetObjectItem(object, "next_path");
    next_path = JSON_next_path == NULL ? 0 : JSON_next_path->valueint;
    if (path_switch)
        FPS_INFO_LOG("      path_switch - next path: %i \n", next_path);

    cJSON* JSON_entry_action = cJSON_GetObjectItem(object, "entry_actions");
    if (JSON_entry_action == NULL)
    {
        FPS_ERROR_LOG("no entry_action found");
        return false;
    }

    cJSON* JSON_entry_route = cJSON_GetObjectItem(JSON_entry_action, "route");
    if (JSON_entry_route == NULL)
    {
        FPS_ERROR_LOG("no entry route found");
        return false;
    }
    entry_route = strdup(JSON_entry_route->valuestring);
    FPS_INFO_LOG("      entry route: %s\n", entry_route);

    cJSON* JSON_entry_value = cJSON_GetObjectItem(JSON_entry_action, "value");
    if (JSON_entry_value == NULL)
    {
        FPS_ERROR_LOG("no entry value found\n");
        return false;
    }
    char* body = cJSON_Print(JSON_entry_value);
    if (body != NULL)
    {
        FPS_INFO_LOG("      entry value: %s\n", body);
        free (body);
    }

    if ((JSON_entry_value->type == cJSON_False) || (JSON_entry_value->type == cJSON_True))
        entry_value.set(JSON_entry_value->type == cJSON_True);
    else if (JSON_entry_value->type == cJSON_Number)
        entry_value.set((float)JSON_entry_value->valuedouble);
    else if (JSON_entry_value->type == cJSON_String)
        entry_value.set(JSON_entry_value->valuestring);
    else
    {
        FPS_ERROR_LOG("entry value type %d not supported\n", JSON_entry_value->type);
        return false;
    }

    cJSON* JSON_exit_condition = cJSON_GetObjectItem(object, "exit_conditions");
    if (JSON_exit_condition == NULL) {
        FPS_ERROR_LOG("no exit_condition found");
        return false;
    }

    cJSON* JSON_exit_route = cJSON_GetObjectItem(JSON_exit_condition, "route");
    if (JSON_exit_route == NULL) {
        FPS_ERROR_LOG("no exit route found\n");
        return false;
    }
    exit_route = strdup(JSON_exit_route->valuestring);
    FPS_INFO_LOG("      exit route: %s\n", exit_route);

    cJSON* JSON_exit_value = cJSON_GetObjectItem(JSON_exit_condition, "value");
    if (JSON_exit_value == NULL)
    {
        FPS_ERROR_LOG("no exit value found");
        return false;
    }
    char *body2 = cJSON_Print(JSON_exit_value);
    if (body2 != NULL)
    {
        FPS_INFO_LOG("      exit value: %s\n", body2);
        free (body2);
    }

    if ((JSON_exit_value->type == cJSON_False) || (JSON_exit_value->type == cJSON_True))
        exit_value.set(JSON_exit_value->type == cJSON_True);
    else if (JSON_exit_value->type == cJSON_Number)
        exit_value.set((float)JSON_exit_value->valuedouble);
    else if (JSON_exit_value->type == cJSON_String)
        exit_value.set(JSON_exit_value->valuestring);
    else
    {
        FPS_ERROR_LOG("exit value type %d not supported\n", JSON_exit_value->type);
        return false;
    }

    cJSON* JSON_tolerance = cJSON_GetObjectItem(JSON_exit_condition, "tolerance");
    tolerance = JSON_tolerance == NULL ? 0 : JSON_tolerance->valueint;
    if (tolerance != 0)
        FPS_INFO_LOG("      tolerance: %d \n", tolerance);

    cJSON* JSON_debounce_timer = cJSON_GetObjectItem(JSON_exit_condition, "debounce_timer");
    debounce_timer = JSON_debounce_timer == NULL ? 0 : JSON_debounce_timer->valueint;
    if (debounce_timer != 0)
        FPS_INFO_LOG("      debounce timer: %d ms\n", debounce_timer);

    return true;
}



