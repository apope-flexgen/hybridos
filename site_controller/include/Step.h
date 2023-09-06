/*
 * Step.h
 *
 *  Created on: Nov 5, 2019
 *      Author: kbrezina
 */

#ifndef INCLUDE_STEP_H_
#define INCLUDE_STEP_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
#include <vector>
/* External Dependencies */
#include <cjson/cJSON.h>
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Value_Object.h>

// Struct encapsulating entry action/exit condition
struct Step_Action {
    std::string route;
    Value_Object value;
    bool result;
    int tolerance;
    int reason_for_exit_failure;  // specifies if exit failed due to debounce timer or conditional
    int debounce_timer_ms;        // Time during which the value must match the expected value for the sequence to pass. Default 0
    timespec debounce_target_time;

    Step_Action() { clock_gettime(CLOCK_MONOTONIC, &debounce_target_time); }
};

class Step {
public:
    std::string step_name;
    std::vector<Step_Action> entry_actions;
    std::vector<Step_Action> exit_conditions;
    bool path_switch;
    uint next_path;

    bool configure_step(cJSON* object, int step_index);
    bool configure_action(std::vector<Step_Action>& action_list, cJSON* JSON_action);
    bool get_path_switch();
    const std::string get_name();
};

#endif /* INCLUDE_STEP_H_ */
