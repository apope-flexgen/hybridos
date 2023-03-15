/*
 * Fims_Object.h
 *
 *  Created on: Nov 5, 2019
 *      Author: kbrezina
 */

#ifndef INCLUDE_FIMS_OBJECT_H_
#define INCLUDE_FIMS_OBJECT_H_
#include <cjson/cJSON.h>
#include </home/vagrant/git/dnp3_interface/include/Value_Object.h>

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */

class Fims_Object
{
public:
    Fims_Object();
    virtual ~Fims_Object();
    cJSON* build_JSON_Object();
    void add_to_Object(cJSON* object);
    void set_id(const char* _id);
    void set_name(const char* _name);
    void set_unit(const char* _unit);
    void set_ui_type(const char* _ui_type);
    void set_type(const char* _type);
    void set_fims_float(const char* uri_endpoint, float body_float);
    void set_fims_int(const char* uri_endpoint, int body_int);
    void set_fims_bool(const char* uri_endpoint, bool body_bool);
    const char* get_id() const;
    const char* get_name()const;
    const char* get_unit()const;
    const char* get_ui_type()const;
    const char* get_type()const;

    char** options_name;
    Value_Object** options_value;
    int scaler;
    int num_options;
    bool enabled;
    Value_Object value;

private:
    char* id;
    char* name;
    char* unit;
    char* ui_type;  //enum, number, string, faults, alarms, none
    char* type;  //int, float, bool, string
};

#endif /* INCLUDE_FIMS_OBJECT_H_ */
