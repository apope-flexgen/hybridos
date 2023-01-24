/*
 * Value_Object.h
 *
 *  Created on: Nov 5, 2019
 *      Author: kbrezina
 */

#ifndef INCLUDE_VALUE_OBJECT_H_
#define INCLUDE_VALUE_OBJECT_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */

class Value_Object
{
    public:
    Value_Object();
    ~Value_Object();

    void set(int   value);
    void set(bool  value);
    void set(float value);
    void set(const char* value);
    const char* print();

    int   type;
    int   value_int;
    char* value_string;
    float value_float;
    bool  value_bool;

    private:
    char*  print_buffer;
};

#endif /* INCLUDE_VALUE_OBJECT_H_ */
