/*
 * Value_Object.cpp
 *
 *  Created on: Nov 5, 2019
 *      Author: kbrezina
 */

/* C Standard Library Dependencies */
#include <cstring>
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Asset_Manager.h>
#include <Value_Object.h>
#include <Site_Controller_Utils.h>

Value_Object::Value_Object() {
    type = Invalid;
    value_int = 0;
    value_float = 0.0;
    value_bool = false;
    value_bit_field = 0;
    value_mask = 0;
}

Value_Object::Value_Object(int value) {
    type = Invalid;
    value_int = 0;
    value_float = 0.0;
    value_bool = false;
    value_bit_field = 0;
    value_mask = 0;
    set(value);
}

void Value_Object::set(bool value) {
    type = Bool;
    value_bool = value;
}

void Value_Object::set(int value) {
    type = Int;
    value_int = value;
}

void Value_Object::set(float value) {
    type = Float;
    value_float = value;
}

void Value_Object::set(uint64_t value) {
    type = Bit_Field;
    value_bit_field = value;
}

void Value_Object::set(const char* value) {
    type = String;
    value_string = value;
}

void Value_Object::set(std::string value) {
    type = String;
    value_string = value;
}

void Value_Object::set(Value_Object& new_value) {
    switch (type) {
        case Bool:
            value_bool = new_value.value_bool;
            break;
        case Int:
            value_int = new_value.value_int;
            break;
        case Float:
            value_float = new_value.value_float;
            break;
        case Bit_Field:
            value_bit_field = new_value.value_bit_field;
            break;
        case String:
            value_string = new_value.value_string;
            break;
        default:
            break;
    };
}

/**
 * Construct the string representation of the value
 */
std::string Value_Object::print() {
    std::string formatted_string;
    switch (type) {
        case Bool:
            formatted_string = value_bool ? "true" : "false";
            break;
        case Int:
            formatted_string = std::to_string(value_int);
            break;
        case Float:
            formatted_string = std::to_string(value_float);
            break;
        case String:
            formatted_string = value_string;
            break;
        case Bit_Field:
            formatted_string = std::to_string(value_bit_field);
            break;
        case Status:
            formatted_string = std::to_string(value_bit_field);
            break;
        default:
            formatted_string = "Invalid Type";
    };
    return formatted_string;
}

/**
 * Determine the appropriate value based on the type of the variable, then add it to the constructed memory buffer.
 * Currently used for rounding (re)active_power_setpoint's values, and these controls will alway be clothed. This
 * functionality is not available for the equivalent add_naked_value_to_JSON_buffer() for this reason.
 * @param buf Reference to the buffer to which the value will be added
 * @param round_float Optional variable defaulting to false. If true, round the value of the float to the nearest integer
 */
bool Value_Object::add_value_to_JSON_buffer(fmt::memory_buffer& buf, bool round_float) {
    switch (type) {
        case Bool:
            bufJSON_AddBool(buf, "value", value_bool);
            break;
        case Int:
            bufJSON_AddNumber(buf, "value", value_int);
            break;
        case Float:
            bufJSON_AddNumber(buf, "value", round_float ? round(value_float) : value_float);
            break;
        case String:
            bufJSON_AddString(buf, "value", value_string.c_str());
            break;
        default:
            FPS_ERROR_LOG("Invalid type %d cannot be added to object.", type);
            return false;
    };
    return true;
}

bool Value_Object::add_naked_value_to_JSON_buffer(fmt::memory_buffer& buf) {
    switch (type) {
        case Bool:
            FORMAT_TO_BUF(buf, R"({})", value_bool);
            break;
        case Int:
            FORMAT_TO_BUF(buf, R"({})", value_int);
            break;
        case Float:
            FORMAT_TO_BUF(buf, R"({})", value_float);
            break;
        case String:
            FORMAT_TO_BUF(buf, R"({})", value_string);
            break;
        default:
            FPS_ERROR_LOG("Invalid type %d cannot be added to object.", type);
            return false;
    };
    return true;
}
