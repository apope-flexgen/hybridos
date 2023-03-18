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
/* External Dependencies */
#include <cjson/cJSON.h>
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Value_Object.h>

class Step
{
private:
    char* step_name;
    char* entry_route;
    char* exit_route;
    Value_Object entry_value;
    Value_Object exit_value;
    int tolerance;
    int debounce_timer;
    bool path_switch;
    int next_path;
    int index;

public:
    Step();
    ~Step();
    bool configure_step(cJSON* object, int step_index);
    bool get_path_switch();
    int get_next_path();
    const char* get_name();
    const char* get_entry_route();
    const char* get_exit_route();
    Value_Object* get_entry_value();
    Value_Object* get_exit_value();
    int get_tolerance() const;
    int get_debounce_timer() const;
};



#endif /* INCLUDE_STEP_H_ */
