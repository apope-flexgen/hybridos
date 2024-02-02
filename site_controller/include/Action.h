/*
 * Sequence.h
 *
 *  Created on: Nov 11, 2023
 *      Author: Judsen
 */

#ifndef INCLUDE_ACTION_H_
#define INCLUDE_ACTION_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Path.h>
#include <Sequence.h>
#include "Types.h"

struct Action_Status : Sequences_Status {
    std::string current_sequence_name;

    // constructor
    Action_Status();
};

// The action class inherits from Sequence as much as possible
// This will attach to an individual asset and allow you to
// perform actions on that asset via a configuration file
class Action : public Sequence {
public:
    // ##### CONSTRUCTORS #####
    Action();                     // default

    // ##### PUBLIC METHODS #####
    bool parse(cJSON* object);
    void exit_automation(Action_Status& action_status, ACTION_STATUS_STATE status);
    void enter_automation(Action_Status& action_status, std::string action_id);
    void clear_action(Action_Status& action_status);
    std::string status_string() const;
    void update_status(Action_Status& action_status);
    void call_sequence(Action_Status& action_status);

    // ##### PUBLIC DATA #####
    ACTION_STATUS_STATE status;
};

#endif /* INCLUDE_ACTION_H_ */
