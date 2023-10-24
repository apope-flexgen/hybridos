/*
 * Variable_Regulator.h
 * Class of objects encapsulating logic of variable regulation used in load and solar shedding in island mode and closed loop control in grid-tied mode
 *
 *  Created on: Oct 6, 2021
 *      Author: Andrew Kwon (akwon)
 */

/* C Standard Library Dependencies */
#include <ctime>
#include <stdio.h>
#include <string>
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
/* Local Internal Dependencies */
#include <Variable_Regulator.h>
#include <Site_Controller_Utils.h>

Variable_Regulator::Variable_Regulator() {
    offset = 0;
    default_offset = 0;
    max_offset = 0;
    min_offset = 0;
    default_threshold = 0;
    default_comparison = VALUE_BELOW;
    control_high_threshold = 0.0;
    updates_per_second = 10;  // The default update rate is used for most standalone features
    decrease_timer_count = 0;
    decrease_timer_duration = 0;
    control_low_threshold = 0.0;
    increase_timer_count = 0;
    increase_timer_duration = 0;

    delta_time_us = 0.0;
    timerStarted = false;
    clock_gettime(CLOCK_MONOTONIC, &prev_time);
    clock_gettime(CLOCK_MONOTONIC, &current_time);
}

Variable_Regulator::~Variable_Regulator() {}

void Variable_Regulator::set_update_rate(int update_rate) {
    updates_per_second = update_rate;
}

void Variable_Regulator::set_default_condition(float threshold, COMPARISON_TYPE check_below_or_above) {
    default_threshold = threshold;
    default_comparison = check_below_or_above;
}

void Variable_Regulator::set_control_high_threshold(float threshold) {
    control_high_threshold = threshold;
}

void Variable_Regulator::set_decrease_timer_duration_ms(int duration_in_ms) {
    decrease_timer_duration = duration_in_ms * updates_per_second / 1000;
    // set timer
    decrease_timer_count = decrease_timer_duration;
}

void Variable_Regulator::set_control_low_threshold(float threshold) {
    control_low_threshold = threshold;
}

void Variable_Regulator::set_increase_timer_duration_ms(int duration_in_ms) {
    increase_timer_duration = duration_in_ms * updates_per_second / 1000;
    // set timer
    increase_timer_count = increase_timer_duration;
}

/**
 * @brief Converts the internal increase timer count to a seconds value then to a display string for the UI.
 *
 * @return std::string A timer display string in the format of MINUTES_LEFT:SECONDS_LEFT.
 */
std::string Variable_Regulator::get_increase_display_timer() {
    return seconds_to_timer_string(increase_timer_count / updates_per_second, increase_timer_duration / updates_per_second);
}

/**
 * @brief Converts the internal decrease timer count to a seconds value then to a display string for the UI.
 *
 * @return std::string A timer display string in the format of MINUTES_LEFT:SECONDS_LEFT.
 */
std::string Variable_Regulator::get_decrease_display_timer() {
    return seconds_to_timer_string(decrease_timer_count / updates_per_second, decrease_timer_duration / updates_per_second);
}

/**
 * Reset to default state
 */
void Variable_Regulator::reset() {
    offset = default_offset;
    reset_timers();
}

/**
 * Main functionality of the Variable_Regulator. Updates the offset based on the provided input variable and timers
 * @param base_case_input   Input determining whether the base case is currently satisfied (soc below threshold, active power feature in deadband)
 *                          If within the base case, regulate the variable through its offset
 *                          If outside the base case, reset to the default value and do not regulate
 * @param regulation_input  The input compared against the low and high thresholds to determine the degree of regulation needed
 * @return true if the command was acted upon and false if waiting for timers
 */
bool Variable_Regulator::regulate_variable_offset(float base_case_input, float regulation_input) {
    // Check time to see if it's time to update, if not then just return
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    if (!timerStarted) {
        timerStarted = true;
        prev_time.tv_sec = current_time.tv_sec;
        prev_time.tv_nsec = current_time.tv_nsec;
        return false;
    }
    delta_time_us = (current_time.tv_sec - prev_time.tv_sec) * 1000000;
    delta_time_us += (current_time.tv_nsec - prev_time.tv_nsec) / 1000;
    if (delta_time_us < (1000000 / updates_per_second))
        return false;

    // Set offset value based on base_case_input and timers
    // Set the offset value to default if appropriate according to the base_case_input comparison
    if ((base_case_input < default_threshold && default_comparison == VALUE_BELOW) || (base_case_input > default_threshold && default_comparison == VALUE_ABOVE)) {
        reset();
    }
    // Update the offset decrease/increase timers
    else {
        update_decrease_timer(regulation_input);
        update_increase_timer(regulation_input);
    }

    // Record the update time
    prev_time.tv_sec = current_time.tv_sec;
    prev_time.tv_nsec = current_time.tv_nsec;
    return true;
}

/**
 * Update decrease timer and decrement offset value if needed
 * @param control_value variable used as reference to determine if regulation through offset is needed
 *                      true when this variable is above the highest tolerable threshold configured
 */
void Variable_Regulator::update_decrease_timer(float control_value) {
    // if the offset value is already at its minimum, no need to count down
    if (offset <= min_offset) {
        return;
    }

    // decrement offset value if control is above threshold for long enough
    if (control_value > control_high_threshold) {
        if (decrease_timer_count > 0) {
            decrease_timer_count--;
        } else if (decrease_timer_count == 0) {
            offset--;
            // reset timer
            decrease_timer_count = decrease_timer_duration;
        }
    }
    // when below threshold, rewind the timer
    else if (decrease_timer_count < decrease_timer_duration) {
        decrease_timer_count++;
    }
}

/**
 * Update increase timer and increment offset value if needed
 * @param control_value variable used as reference to determine if regulation through offset is needed
 *                      true when this variable is below the lowest tolerable threshold configured
 */
void Variable_Regulator::update_increase_timer(float control_value) {
    // if offset value is already at its maximum, no need to count down
    if (offset >= max_offset) {
        return;
    }

    // increment offset value if control is below threshold for long enough
    if (control_value < control_low_threshold) {
        if (increase_timer_count > 0) {
            increase_timer_count--;
        } else if (increase_timer_count == 0) {
            offset++;
            // reset timer
            increase_timer_count = increase_timer_duration;
        }
    }
    // when above threshold, rewind the timer
    else if (increase_timer_count < increase_timer_duration) {
        increase_timer_count++;
    }
}

void Variable_Regulator::reset_timers() {
    decrease_timer_count = decrease_timer_duration;
    increase_timer_count = increase_timer_duration;
}
