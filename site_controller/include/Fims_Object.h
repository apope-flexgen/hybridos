/*
 * Fims_Object.h
 *
 *  Created on: Nov 5, 2019
 *      Author: kbrezina
 */

#ifndef INCLUDE_FIMS_OBJECT_H_
#define INCLUDE_FIMS_OBJECT_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
#include <vector>
#include <string>
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Types.h>
#include <Input_Sources.h>
#include <Value_Object.h>

// Types of Fims_Objects:
//
// Multiple-Input Command Variables
//      Commands come from "above" site_controller (UI, Scheduler, DNP3, etc.) to the inputs vector. From there, the
//      highest-priority enabled input has its value piped to the value field.
//
// Single-Input Command Variables
//      Command comes from "above" site_controller directly to the value field.
//
// Output Status Variables
//      Contains the output of site_controller's calculations/algorithms. Cannot be controlled by an external source,
//      except through the inputs to the calculation/algorithm that outputs to the variable.
//
// Input/Output Component Variables
//      Value comes from "below" site_controller, specifically a component Modbus register, and represents the current
//      status of the register. Holding Registers can be controlled using the component_control_value field, but Input
//      Registers cannot be controlled.
class Fims_Object
{
public:
    Fims_Object();
    virtual ~Fims_Object();
    void build_JSON_Object(fmt::memory_buffer &buf, bool control2status, bool clothed, const char* const search_id = NULL);
    void add_to_JSON_buffer(fmt::memory_buffer &buf, const char* const search_id = NULL, bool clothed = true);
    void add_status_of_control_to_JSON_buffer(fmt::memory_buffer &buf, const char* const var = NULL, bool clothed = true);
    void set_component_uri(const char* _uri);
    void set_register_id(const char* _id);
    void set_variable_id(const char* _id);
    void set_variable_id(const std::string &id);
    void set_name(const char* _name);
    void set_unit(const char* _unit);
    void set_ui_type(const char* _ui_type);
    void set_value_type(valueType _value_type);
    void set_type(const char* _type);
    bool set_fims_float(const char* uri_endpoint, float body_float);
    bool set_fims_int(const char* uri_endpoint, int body_int);
    bool set_fims_bool(const char* uri_endpoint, bool body_bool);
    bool set_fims_bit_field(const char* uri_endpoint, uint64_t body_bit_field);
    bool set_fims_masked_int(const char* uri_endpoint, int body_int, uint64_t mask);
    const char* get_component_uri() const;
    const char* get_register_id() const;
    const char* get_variable_id() const;
    void clear_fims_bit_field(void);
    const char* get_name()const;
    const char* get_unit()const;
    const char* get_ui_type()const;
    const char* get_type()const;
    int get_scaler()const;
    bool send_to_component(bool use_write_uri = false, bool round_float = false);
    void update_present_register(uint input_source_index);
    void parse_json_config(cJSON* JSON_object, const Fims_Object &default_vals);
    bool configure(const std::string& var_id, bool* p_flag, Input_Source_List* input_sources, cJSON* JSON_config, const Fims_Object& default_vals, std::vector<Fims_Object*>& multi_input_command_vars);

    std::vector<std::string> options_name;
    std::vector<Value_Object> options_value;
    int default_status_value;
    std::string default_status_name;
    int scaler;
    int num_options;
    bool ui_enabled;
    std::string write_uri; // When called, write this object's value to this custom Fims URI
    bool* is_primary; // pointer to flag indicating if this Site Controller is primary controller (else it is secondary/shadow controller)

    // Indicates if the Fims_Object is a Multiple-Input Command Variable, meaning the inputs vector should be checked for input commands
    bool multiple_inputs;

    // For Multiple-Input Command Variables.
    // There is a specific Fims_Object called input_source_selection. It is always configured as a Multiple-Input Command Variable accepting
    // booleans. All Multiple-Input Command Variables programatically have the same number of inputs (same vector size) as input_source_selection,
    // so each Value_Object of the inputs vector pairs with the boolean Value_Object at the same index of input_source_selection's input vector.
    // That boolean indicates if the input represented by the index is enabled or not. The inputs are indexed in order of priority, so the
    // first input that is enabled is the input whose Value_Object should be forwarded to the Fims_Object::value field.
    std::vector<Value_Object> inputs;

    // pointer to the list of input sources so settings such as uri_suffix, ui_type, etc. can be accessed as needed
    Input_Source_List* input_source_settings;

    // for component variables
    // component_control_value holds the control value Site Controller wants to set a component control to.
    // for example, active_power_setpoint's "value" is the status of the asset's active power setpoint.
    //              its "component_control_value" is the value Site Controller wants the asset's active
    //              power setpoint to change to.
    Value_Object component_control_value;

    // current value. See comment above class Fims_Object for explanation about where the source of this value comes from.
    Value_Object value;

private:
    std::string variable_id;  // id that the UI expects. Used for both Site Manager and Asset Manager variables
    std::string register_id;  // id expected from a component's pub. Not used by Site Manager variables, only by Asset Manager variables
    std::string component_uri;// uri used by Asset Manager to send set commands to components
    std::string name;
    std::string unit;
    std::string ui_type;  //enum, number, string, faults, alarms, none
    std::string type;  //int, float, bool, string

    fmt::memory_buffer send_FIMS_buf; // Reusable and resizable string buffer used to send FIMS messages

    uint parse_input_source_index(std::string);
};

#endif /* INCLUDE_FIMS_OBJECT_H_ */
