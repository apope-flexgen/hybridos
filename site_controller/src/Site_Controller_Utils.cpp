/**
 * Library of time and functions shared by various HybridOS components
 * More functions will be added as appropriate
 *
 * Created on: January 28, 2021
 *      Author: jnshade
 */

/* C Standard Library Dependencies */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
/* C++ Standard Library Dependencies */
#include <memory>
#include <stdexcept>
/* External Dependencies */
#include <gtest/gtest.h>
/* System Internal Dependencies */
#include <fims/libfims.h>
#include <Logger.h>
/* Local Internal Dependencies */
#include <Site_Controller_Utils.h>
#include <Fims_Object.h>

extern fims* p_fims;
// pointer to the current state of stdout
int stdout_save;
bool released = true;

/**
 * @brief Uses printf-style formatting to instantiate and initialize a standard string.
 * Modified from https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf.
 * Licensed as open source.
 *
 * @tparam Args Because it is like printf, the arguments can be of any type as long as the format string matches them.
 * @param format String that is desired to be put into the standard string, using printf-like formatters where variable values are to be inserted.
 * @param args A list of the variables that should be inserted into the format's formatters.
 * @return std::string A standard string containing the formatted string.
 * @throws Runtime error if formatting is not valid.
 */
template <typename... Args>
std::string string_format(const std::string& format, Args... args) {
    // when the size limit (in this case 0) passed to snprintf is exceeded, func will return the amount of memory required.
    // use this to figure out how much memory we need to allocate.
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;  // Extra space for '\0'
    if (size_s <= 0) {
        throw std::runtime_error("Error calculating formatted string size.");
    }
    auto size = static_cast<size_t>(size_s);

    // allocate memory for a char* and use snprintf (with the proper limit this time) to actually execute the formatting
    std::unique_ptr<char> buf(new char[size]);
    if (buf.get() == nullptr) {
        throw std::runtime_error("Memory allocation error during formatting.");
    }
    std::snprintf(buf.get(), size, format.c_str(), args...);

    // convert the formatted char* to a std::string for the return
    return std::string(buf.get(), buf.get() + size - 1);  // we don't want the '\0' inside
}

/**
 * Splits the string at every point that the delimiter is found. If the
 * string starts with the delimiter, ignoring empty fragments.
 * @param str The string to be split.
 * @param delimiter The token that, where found in the string to be split,
 * will mark a point of splitting. The delimiter will not appear anywhere
 * in the returned string fragments.
 * @return Vector of strings that are the substrings
 * occurring between the delimiter in the original string.
 */
std::vector<std::string> split(std::string str, std::string delimiter) {
    std::vector<std::string> fragments;
    while (!str.empty()) {
        std::size_t pos = str.find(delimiter);
        std::string fragment = (pos != std::string::npos) ? str.substr(0, pos) : str;
        if (!fragment.empty())  // ignore empty fragments
            fragments.push_back(fragment);
        str.erase(0, fragment.length() + delimiter.length());
    }
    return fragments;
}

bool has_suffix(const char* str, const char* suffix) {
    if (str == NULL || suffix == NULL)
        return false;
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len)
        return false;
    if (suffix_len == 0)
        return true;
    return strncmp(str + str_len - suffix_len, suffix, suffix_len) == 0;
}

bool trim_suffix_if_exists(std::string& str, const char* suffix) {
    if (!has_suffix(str.c_str(), suffix))
        return false;
    size_t suffix_len = strlen(suffix);
    str.erase(str.size() - suffix_len, suffix_len);
    return true;
}

// Returns true if current_time is past target_time
bool check_expired_time(timespec current_time, timespec target_time) {
    if (current_time.tv_sec > target_time.tv_sec) {
        return true;
    }
    return (current_time.tv_sec < target_time.tv_sec) ? false : (current_time.tv_nsec > (target_time.tv_nsec + 1000000));
    // ^^TIMEOUT TIME HASN'T BEEN REACHED^^     ^^SEC VALUE IS EQUAL, SO CHECK NANOSEC VALUE BUT WITH 1MS BUFFER^^
}

long int get_elapsed_time(timespec current_time, timespec target_time) {
    return ((target_time.tv_nsec - current_time.tv_nsec) / 1000000) + ((target_time.tv_sec - current_time.tv_sec) * 1000);
}

// Returns the current time as a double
double get_raw_timestamp(void) {
    timespec now;
    int rc = clock_gettime(CLOCK_MONOTONIC, &now);
    if (rc != 0)
        return 0.0;
    return (double)(now.tv_sec) + ((double)now.tv_nsec) / 1000000000.0;
}

// Takes a timespec and increments it with a millisecond value, dealing with overflow appropriately
void increment_timespec_ms(timespec& time_struct, int ms_inc) {
    long int s_inc = ms_inc / ONE_SECOND_IN_TERMS_OF_MS;                        // Get the number of whole seconds in millisecond increment
    time_struct.tv_sec += s_inc;                                                // Increment timespec by number of whole seconds
    ms_inc %= ONE_SECOND_IN_TERMS_OF_MS;                                        // Isolate the non-whole-second component of the millisecond increment
    long int ns_inc = (long int)ms_inc * MS_TO_NS_MULTIPLIER;                   // Since timespec is in terms of nanoseconds, put increment in terms of nanoseconds
    long int ns_left_in_sec = ONE_SECOND_IN_TERMS_OF_NS - time_struct.tv_nsec;  // Test to see if timespec can handle the nanosecond increment without overflowing
    if (ns_left_in_sec <= ns_inc) {                                             // Time increment causes overflow
        long int ns_overflow = ns_inc - ns_left_in_sec;
        ++time_struct.tv_sec;               // Carry the 1
        time_struct.tv_nsec = ns_overflow;  // Add the remainder
    } else {                                // Time increment does not cause overflow
        time_struct.tv_nsec += ns_inc;
    }
}

/**
 * @brief Converts an integer countdown of seconds to a string countdown of the format MINUTES_LEFT:SECONDS_LEFT.
 *
 * @param seconds_left The number of seconds left in the countdown.
 * @param start_value The starting value of the timer in seconds.
 * @return std::string The amount of time left in the countdown formatted as MINUTES_LEFT:SECONDS_LEFT.
 */
std::string seconds_to_timer_string(int seconds_left, int start_value) {
    // if timer is not counting down (still at its starting value), display dashes
    if (seconds_left == start_value) {
        return std::string("--:--");
    }
    int display_minutes = seconds_left / 60;
    int display_seconds = seconds_left % 60;
    std::string timer_display;
    try {
        timer_display = string_format("%02d:%02d", display_minutes, display_seconds);
    } catch (const std::exception& e) {
        timer_display = "Timer Formatting Error";
        FPS_ERROR_LOG("Error formatting timer display: %s\n", e.what());
    }
    return timer_display;
}

// Send the event using this fims instance
void emit_event(const char* source, const char* message, int severity) {
    // Creating buffer here so that classes that call emit_event don't need to have a buffer
    auto buf = fmt::memory_buffer();

    bufJSON_StartObject(buf);  // JSON_object {
    bufJSON_AddString(buf, "source", source);
    bufJSON_AddString(buf, "message", message);
    bufJSON_AddNumber(buf, "severity", severity);
    bufJSON_EndObject(buf);  // } JSON_object

    bufJSON_RemoveTrailingComma(buf);
    std::string body = to_string(buf);
#ifndef FPS_TEST_MODE
    p_fims->Send("post", "/events", NULL, body.c_str());
#endif
}

// Returns integer from input declaring which single bit to be high (e.g. 0 > 1, 2 > 4, 3 > 8 etc)
int get_int_from_bit_position(int bit_position) {
    int integer = 1;

    if (bit_position < 0)  // return 0 for invalid numbers
        return 0;

    return integer << bit_position;  // bit-shift integer by bit_position
}

/**
 * @brief Adds a clothed version of data to a naked object.
 * @param object naked object
 * @return cJSON* new clothed object
 */
cJSON* clothe_naked_cJSON(cJSON* object) {
    cJSON* clothed_variable = cJSON_CreateObject();
    cJSON_AddItemToObject(clothed_variable, "value", object);
    return clothed_variable;
}

/**
 * @brief
 * Utility function that can be applied to an object to see if it is of a cJSON type and therefore naked.
 *
 * @param object
 * @return true is naked --- false is not naked
 */
bool is_naked(cJSON* object) {
    return object->type == cJSON_Number || object->type == cJSON_True || object->type == cJSON_False || object->type == cJSON_String;
}

/**
 * @brief
 * Checks if a cJSON object is clothed. If not clothes it.
 *
 * @param fimsbody contains object as child
 * @param object contains clothed value as a child, or is naked value
 * @param name used to get object from fimsbody
 * @return cJSON* updates fimsbody as well
 */
cJSON* grab_naked_or_clothed(cJSON& fimsbody, cJSON* object, const char* name) {
    cJSON* value = NULL;
    if ((object = cJSON_GetObjectItem(&fimsbody, name))) {
        if ((value = cJSON_GetObjectItem(object, "value"))) {
            return object;
        } else if (is_naked(object)) {
            object = cJSON_DetachItemFromObject(&fimsbody, name);
            value = clothe_naked_cJSON(object);
            cJSON_AddItemToObject(&fimsbody, name, value);
            object = cJSON_GetObjectItem(&fimsbody, name);
            return value;
        }
    }
    return NULL;
}

/**
 * @brief
 * This function takes in a set and if it is naked updates the fimsbody such that it is then clothed.
 * It also conveniently checks that the value is of an expected cJSON type
 *
 * @param fimsbody contains current_setpoint as a child
 * @param current_setpoint is either naked value or containt a clothed value child
 * @param CJSONType expected type of value
 * @param name used to get current_setpoint from fimsbody and then replace it
 * @return cJSON* updates fimsbody as well
 */
cJSON* grab_naked_or_clothed_and_check_type(cJSON& fimsbody, cJSON* current_setpoint, int CJSONType, const char* name) {
    cJSON* value = NULL;
    if ((current_setpoint = cJSON_GetObjectItem(&fimsbody, name))) {
        if ((value = cJSON_GetObjectItem(current_setpoint, "value")) && value->type == CJSONType) {
            return current_setpoint;
        } else if (current_setpoint->type == CJSONType) {
            current_setpoint = cJSON_DetachItemFromObject(&fimsbody, name);
            value = clothe_naked_cJSON(current_setpoint);
            cJSON_AddItemToObject(&fimsbody, name, value);
            current_setpoint = cJSON_GetObjectItem(&fimsbody, name);
            return current_setpoint;
        }
    }
    return NULL;
}

// tolerance compares two numbers and returns true if the actual value is within tolerance percent of tolerance value
bool get_tolerance(float tolerance_value, float actual_value, int tolerance_percent) {
    bool in_range = ((tolerance_value + (tolerance_value * tolerance_percent * .01)) >= actual_value && (tolerance_value - (tolerance_value * tolerance_percent * .01)) <= actual_value);
    return in_range;
}

// function to build a curve from given points
void set_curve_points(Fims_Object* curve_points, std::vector<std::pair<float, float>>& curve_vector) {
    int num_options = curve_points->num_options;
    int num_points = num_options / 2;
    bool curve_points_valid = false;

    // Options list is formatted as (x0,y0,x1,y1,x2,y2...)

    if (num_options <= 2)
        FPS_WARNING_LOG("Not enough curve points\n");
    else if (num_options % 2 == 1)
        FPS_WARNING_LOG("Odd number of curve points\n");
    else
        curve_points_valid = true;

    for (int i = 0; curve_points_valid && i < num_options; i += 2) {
        curve_vector.push_back(std::make_pair(curve_points->options_value[i].value_float, curve_points->options_value[i + 1].value_float));

        // Check that x-coordinates are monotonically increasing
        if ((i > 0) && (curve_vector[i / 2].first <= curve_vector[(i / 2) - 1].first)) {
            curve_points_valid = false;
            FPS_WARNING_LOG("Independent variable in curve points is not monotonically increasing\n");
        }
    }

    if (!curve_points_valid) {
        // If curve points are invalid, return x={0,1,2..} and y={0,0,0..}
        // Arrays need to be length specified in num_options,
        // padded with an extra element in case of odd number
        for (int i = 0; i < num_points; i++) {
            curve_vector[i].first = i;
            curve_vector[i].second = 0;
        }
    }

    for (int i = 0; i < num_points; i++) {
        // FPS_INFO_LOG("Curve points: x[%i]=%f, y[%i]=%f \n", i, curve_vector[i].first, i, curve_vector[i].second);
    }
}

// function to determine output of a curve interpolated from given points
float get_curve_cmd(float ind_var, std::vector<std::pair<float, float>>& curve_points) {
    float cmd = 0;
    int n = curve_points.size();

    // Output cmd is fit to curve via y=mx+b, where m=(y1-y2)/(x1-x2) and b=y1-m*x1 or b=y2-m*x2 --> y=m*(x-x1)+y1
    // If input is outside of curve_points range, set output based on outermost 2 curve points
    if (ind_var < curve_points[0].first) {
        cmd = ((curve_points[1].second - curve_points[0].second) / (curve_points[1].first - curve_points[0].first)) * (ind_var - curve_points[0].first) + curve_points[0].second;
    } else if (ind_var >= curve_points[n - 1].first) {
        cmd = ((curve_points[n - 1].second - curve_points[n - 2].second) / (curve_points[n - 1].first - curve_points[n - 2].first)) * (ind_var - curve_points[n - 1].first) + curve_points[n - 1].second;
    } else {
        for (int i = 0; i < n - 1; i++) {
            if (ind_var >= curve_points[i].first && ind_var < curve_points[i + 1].first) {
                cmd = ((curve_points[i + 1].second - curve_points[i].second) / (curve_points[i + 1].first - curve_points[i].first)) * (ind_var - curve_points[i + 1].first) + curve_points[i + 1].second;
                break;
            }
        }
    }
    return cmd;
}

// check a float value against a min and max and return in-range value
float range_check(float value, float max, float min) {
    return (value < max) ? ((value > min) ? value : min) : max;
}

// check an int value against a min and max and return in-range value
int range_check(int value, int max, int min) {
    return (value < max) ? ((value > min) ? value : min) : max;
}

// check a value against 0 and return 0 if it is negative
float zero_check(float value) {
    return (value < 0) ? 0 : value;
}

/**
 * Check a value against 0 and return 0 if it is positive
 * @param value The value to check
 * @return The value if it is negative, else zero
 */
float less_than_zero_check(float value) {
    return (value > 0) ? 0 : value;
}

// Captures all output written to stdout
// Used for capturing FPS_DEBUG_LOG output from tests
// MUST call release_stdout() to enable writing to stdout again
// see https://stackoverflow.com/questions/15155314/redirect-stdout-and-stderr-to-the-same-file-and-restore-it
void capture_stdout() {
    fflush(stdout);

    // save the stdout state
    stdout_save = dup(STDOUT_FILENO);
    // redirect stdout to null pointer
    freopen("NUL", "a", stdout);
    released = false;
}

// Releases stdout for writing again
void release_stdout(bool print) {
    if (released)
        return;
    // restore the previous state of stdout
    dup2(stdout_save, STDOUT_FILENO);
    if (print) {
        // Read the stdout that was captured
        FILE* fp = fopen("NUL", "r");
        char buff[2048];
        while (fgets(buff, 2048, fp))
            printf(buff);
    }
    close(stdout_save);
    remove("NUL");
    released = true;
}

// Return whether two floating point numbers are within tolerance
// Used for rounding inaccuracies when working with doubles and floats
bool near(float a, float b, float tolerance) {
    return fabsf(a - b) < fabsf(tolerance);
}

// Return whether a is less than b, or the two numbers are within tolerance
// Used for rounding inaccuracies when working with doubles and floats
bool less_than_or_near(float a, float b, float tolerance) {
    return (a <= b) || near(a, b, tolerance);
}

// Return whether a is greater than b, or the two numbers are within tolerance
// Used for rounding inaccuracies when working with doubles and floats
bool greater_than_or_near(float a, float b, float tolerance) {
    return (a >= b) || near(a, b, tolerance);
}

void free_fims_msg(fims_message* msg) {
    for (unsigned int i = 0; i < msg->nfrags; i++)
        free(msg->pfrags[i]);
    p_fims->free_message(msg);
}

bool send_buffer_to(const char* uri, fmt::memory_buffer& buff) {
    if (uri == NULL) {
        FPS_ERROR_LOG("Cannot send buffer to NULL URI.");
        return false;
    }
    if (buff.size() == 0) {
        FPS_ERROR_LOG("Given empty buffer to send to %s.", uri);
        return false;
    }
    std::string body = to_string(buff);
    p_fims->Send("set", uri, NULL, body.c_str());
    return true;
}
