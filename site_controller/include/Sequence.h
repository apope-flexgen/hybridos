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

// Forward declaration of a couple classes to avoid circular dependencies
class Site_Manager;
class Asset;
class Asset_ESS;
class Asset_Solar;
class Asset_Generator;
class Asset_Feeder;

struct Sequences_Status {
    states current_state, check_current_state;
    bool step_change, path_change;  // true whenever a step or path changes
    bool sequence_reset;            // indication that current sequence has changed. used to handle interrupted sequences
    bool should_pub;                // determines if an action (actions.json) will pub TODO: should we pub site_manager only on should pub
    timespec current_time;
    timespec exit_target_time;

    // constructor
    Sequences_Status();
};

class Sequence {
public:
    std::string sequence_name;
    states state;
    std::vector<Path> paths;

    // make this a union that can be either a pointer to a site or a pointer to an asset
    union {
        Site_Manager* site;
        Asset_ESS* asset_ess;
        Asset_Solar* asset_solar;
        Asset_Generator* asset_generator;
        Asset_Feeder* asset_feeder;
    };
    Sequence_Type sequence_type;

    // constructors
    Sequence();
    Sequence(Site_Manager* siteref);

    bool parse(cJSON* object, states s);
    bool check_faults();
    bool check_alarms();
    void call_sequence(Sequences_Status& sequences_status);
    bool call_function(Sequences_Status& sequences_status);
    virtual void reset_sequence(Sequences_Status& sequences_status);
    void reset_step(Sequences_Status& sequences_status);
    void call_sequence_entry(Sequences_Status& sequences_status);
    void call_sequence_exit(Sequences_Status& sequences_status);
    void check_path_step_change(Sequences_Status& sequences_status);
    void handle_entry_action(bool& return_value);
    void handle_exit_action(bool& return_value, Sequences_Status& sequences_status);
    void handle_initial_exit_attempt(Sequences_Status& sequences_status);
    int collect_seconds_remaining_in_current_step();
    int collect_seconds_remaining_in_action();

    bool step_reset;                                    // used to track when a step has completed
    bool debounce_reset;                                // used for resetting the debounce timer in sequence steps
    bool sequence_bypass;                               // set when a sequence has completed and no steps need to be called again
    bool initial_entry_attempt;                         // makes sure certain fprintf messages are only sent once per instance
    bool initial_exit_attempt;                          // makes sure exit condition failure message only sent once per exit
    uint entry_exit_flag;                               // specifies if asset call is for an entry action or exit condition
    uint current_step_index, check_current_step_index;  // contains the current state index, and a 2nd copy for comparison/tracking
    uint current_path_index, check_current_path_index;  // contains the current path index
};

#endif /* INCLUDE_SEQUENCE_H_ */
