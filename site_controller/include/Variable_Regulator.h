/*
 * Variable_Regulator.h
 * Header for class of objects encapsulating logic of variable regulation used for load and solar
 * shedding in island mode and closed loop control for grid-tied mode.
 *
 * Variable regulation works by looking at an input variable and making a determination based on
 * the thresholds and conditions configured. If the input is beyond the desired threshold, the
 * regulator will then increment or decrement its offset over time to apply a correction to the
 * desired output. For load and solar shedding, this would be decrease the site load or solar
 * production based on some input SoC. For closed loop POI, this would be applying a correction
 * to the command requested by the active power feature to bring it in line with the value at
 * the POI.
 *
 *  Created on: Oct 4, 2021
 *      Author: Andrew Kwon (akwon)
 */

#ifndef INCLUDE_VARIABLE_REGULATOR_H_
#define INCLUDE_VARIABLE_REGULATOR_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
#include <string>
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <macros.h>

class Variable_Regulator {
public:
    enum COMPARISON_TYPE { VALUE_BELOW, VALUE_ABOVE };

    Variable_Regulator();
    ~Variable_Regulator();

    void set_default_condition(float threshold, COMPARISON_TYPE check_below_or_above);
    // Note: update rate calculated as updates per second not the ms rate
    // i.e. 1000(ms/s) / update rate(ms) gives the updates per second passed to this function
    void set_update_rate(float update_rate);
    void set_control_high_threshold(float threshold);
    void set_decrease_timer_duration_ms(int duration_in_ms);
    void set_control_low_threshold(float threshold);
    void set_increase_timer_duration_ms(int duration_in_ms);

    std::string get_increase_display_timer();
    std::string get_decrease_display_timer();

    bool regulate_variable_offset(float base_case_input, float regulation_input);
    void reset();

    int offset;  // Current offset value
    int default_offset;
    int max_offset;
    int min_offset;

private:
    float default_threshold;             // The base case condition compared against the base_case_input to determine if regulation should be reset to the default configured
                                         // For Load/Solar shedding, this is resetting the shedding to max if soc is above/below a configured threshold
                                         // For closed loop control, this is resetting the offset back to 0 when outside steady state deadband
    COMPARISON_TYPE default_comparison;  // Comparison check determining the type of comparison that should be made in the base case (value above or value below)
    float control_high_threshold;        // if control value is above this threshold long enough, decrement offset value
    int decrease_timer_count;            // current timer count before decrementing offset value
    int decrease_timer_duration;         // starting count of timer in periods based on the update_rate
    float control_low_threshold;         // if control value is below this threshold long enough, increment offset value
    int increase_timer_count;            // current timer count before incrementing offset value
    int increase_timer_duration;         // starting count of timer in periods on the update_rate
    unsigned long delta_time_us;         // Time since last update in microseconds
    bool timerStarted;
    timespec current_time, prev_time;
    float updates_per_second;

    void update_decrease_timer(float control_value);
    void update_increase_timer(float control_value);
    void reset_timers();
};

#endif /* INCLUDE_VARIABLE_REGULATOR_H_ */
