/*
 * Value_Object.h
 *
 *  Created on: Nov 5, 2019
 *      Author: kbrezina
 */

#ifndef INCLUDE_VALUE_OBJECT_H_
#define INCLUDE_VALUE_OBJECT_H_

/* C Standard Library Dependencies */
#include <cstdint>
/* C++ Standard Library Dependencies */
#include <string>
/* External Dependencies */
#include <spdlog/fmt/fmt.h>
/* System Internal Dependencies */
/* Local Internal Dependencies */

class Value_Object {
public:
    Value_Object();
    Value_Object(int val);

    /** @cond exclude from doxygen comments. These functions are all over the place and can crowd diagrams.*/
    void set(int value);
    void set(bool value);
    void set(float value);
    void set(uint64_t value);
    void set(const char* value);
    void set(std::string value);
    void set(Value_Object& new_value);
    /** @endcond */
    std::string print();
    bool add_value_to_JSON_buffer(fmt::memory_buffer& buf, bool round_float = false);
    bool add_naked_value_to_JSON_buffer(fmt::memory_buffer& buf);

    int type;
    int value_int;
    std::string value_string;
    float value_float;
    uint64_t value_bit_field;
    uint64_t value_mask;
    bool value_bool;
};

// Creates a Value_Object of the specified type using the given value
template <class T>
Value_Object create_Value_Object(T value) {
    Value_Object obj;
    obj.set(value);
    return obj;
}

#endif /* INCLUDE_VALUE_OBJECT_H_ */
