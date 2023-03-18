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

class Value_Object
{
    public:
    Value_Object();
    Value_Object(int val);
    ~Value_Object();

    void set(int   value);
    void set(bool  value);
    void set(float value);
    void set(uint64_t value);
    void set(const char* value);
    void set(std::string value);
    void set(Value_Object &new_value);
    const char* print();
    bool add_value_to_JSON_buffer(fmt::memory_buffer &buf);
    bool add_naked_value_to_JSON_buffer(fmt::memory_buffer &buf);

    int   type;
    int   value_int;
    char* value_string;
    float value_float;
    uint64_t value_bit_field;
    uint64_t value_mask;
    bool  value_bool;

    private:
    char*  print_buffer;
};

#endif /* INCLUDE_VALUE_OBJECT_H_ */
