/*
 * Sequence.h
 *
 *  Created on: Nov 5, 2019
 *      Author: kbrezina
 */

#ifndef INCLUDE_SEQUENCE_H_
#define INCLUDE_SEQUENCE_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Path.h>


class Sequence
{
public:
    char* sequence_name;
    int paths_size;
    states state;
    Path** paths;

    Site_Manager* pSite;

    Sequence(Site_Manager* siteref);
    virtual ~Sequence();

    bool parse_sequence(cJSON* object, states s);
    Path* get_path(int n);
    int get_paths_size();
    bool check_faults();
    bool check_alarms();
    void call_sequence();
    bool call_function();
    void reset_sequence();
    void reset_step();
    void call_sequence_entry();
    void call_sequence_exit();
    void check_path_step_change();

    bool step_reset;  //used to track when a step has completed
    bool debounce_reset; //used for resetting the debounce timer in sequence steps
    bool sequence_bypass;  //set when a sequence has completed and no steps need to be called again
    bool initial_entry_attempt; //makes sure certain fprintf messages are only sent once per instance
    bool initial_exit_attempt; //makes sure exit condition failure message only sent once per exit
    int entry_exit_flag;       //specifies if asset call is for an entry action or exit condition
    int current_step_index, check_current_step_index;  //contains the current state index, and a 2nd copy for comparison/tracking
    int current_path_index, check_current_path_index;  //contains the current path index
};

#endif /* INCLUDE_SEQUENCE_H_ */
