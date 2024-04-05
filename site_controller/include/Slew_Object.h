/*
 * Slew_Object.h
 *
 *  Created on: Nov 5, 2019
 *      Author: kbrezina
 */

#ifndef INCLUDE_SLEW_OBJECT_H_
#define INCLUDE_SLEW_OBJECT_H_

/* C Standard Library Dependencies */
#include <ctime>
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */

class Slew_Object {
public:
    Slew_Object();
    ~Slew_Object();

    float get_slew_target(float target_value) const;  // returns a target value filtered through slew limits
    void update_slew_target(float target_value);      // function to get slew delta time and limits, updates internal vars
    void reset_slew_target();
    void reset_slew_target(int diff);

    void set_slew_rate(int rate);

    int get_slew_rate(void) const;
    float get_max_target(void) const;
    float get_min_target(void) const;

private:
    int slew_rate;          // rate at which slew occurs (delta x per second)
    float current_value;    // present value of slew
    float slew_delta_time;  // delta seconds between slew calls
    float slew_max_value;   // calculated max based on delta time and previous iteration's current value
    float slew_min_value;   // calculated min based on delta time and previous iterations's current value
    timespec slew_current_time, slew_prev_time;
};

#endif /* INCLUDE_SLEW_OBJECT_H_ */
