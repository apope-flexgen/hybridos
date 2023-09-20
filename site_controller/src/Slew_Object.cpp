/*
 * Slew_Object.cpp
 *
 *  Created on: Nov 5, 2019
 *      Author: vagrant
 */

/* C Standard Library Dependencies */
#include <ctime>
/* C++ Standard Library Dependencies */
#include <sys/types.h>
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Slew_Object.h>

Slew_Object::Slew_Object() {
    slew_delta_time = 0.0;
    slew_rate = 1000000;  // 1GW/s default
    current_value = 0.0;
    slew_max_value = 0.0;
    slew_min_value = 0.0;
}

Slew_Object::~Slew_Object() {}

// this function calculates delta time and slew max and min values
void Slew_Object::update_slew_target(float target_value) {
    current_value = get_slew_target(target_value);
    clock_gettime(CLOCK_MONOTONIC, &slew_current_time);
    slew_delta_time = .000001 * (((slew_current_time.tv_sec - slew_prev_time.tv_sec) * 1000000) + ((slew_current_time.tv_nsec - slew_prev_time.tv_nsec) / 1000));
    slew_max_value = current_value + (slew_rate * slew_delta_time);
    slew_min_value = current_value - (slew_rate * slew_delta_time);
    slew_prev_time = slew_current_time;
}

// this function will take a target value and return the value after checking against the slew
float Slew_Object::get_slew_target(float target_value) const {
    if (target_value > current_value) {
        return (target_value > slew_max_value) ? slew_max_value : target_value;
    }
    if (target_value < current_value) {
        return (target_value < slew_min_value) ? slew_min_value : target_value;
    }
    return target_value;
}

// this function resets the slew clock to default values
void Slew_Object::reset_slew_target() {
    clock_gettime(CLOCK_MONOTONIC, &slew_current_time);
    slew_prev_time = slew_current_time;
}

// used for GTest
void Slew_Object::reset_slew_target(int diff) {
    clock_gettime(CLOCK_MONOTONIC, &slew_current_time);
    slew_prev_time = slew_current_time;
    slew_prev_time.tv_sec -= diff;
}

void Slew_Object::set_slew_rate(int rate) {
    slew_rate = rate;
}

int Slew_Object::get_slew_rate(void) const {
    return slew_rate;
}

float Slew_Object::get_max_target(void) const {
    return slew_max_value;
}

float Slew_Object::get_min_target(void) const {
    return slew_min_value;
}
