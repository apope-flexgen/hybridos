/*
 * Asset.h
 *
 *  Created on: May 9, 2018
 *      Author: jcalcagni
 */

#ifndef ASSET_H_
#define ASSET_H_

/* C Standard Library Dependencies */
#include <cstdio>
#include <cmath>
#include <limits>
/* C++ Standard Library Dependencies */
#include <map>
#include <list>
#include <unordered_map>
#include <iterator>
#include <algorithm>
/* External Dependencies */
#include <cjson/cJSON.h>
/* System Internal Dependencies */
#include <fims/libfims.h>
/* Local Internal Dependencies */
#include <Slew_Object.h>
#include <Value_Object.h>
#include <Fims_Object.h>
#include <macros.h>
#include <Types.h>
class Type_Configurator;

struct statusData {
    int64_t* data_int;
    char** data_str;
};

// Used for UI controls (i.e. start asset, maintenance active power setpoint, etc.).
struct fimsCtl {
    fimsCtl();
    ~fimsCtl();
    bool makeJSONObject(fmt::memory_buffer& buf, const char* const var = NULL, bool configurable_asset = false);
    bool configure(cJSON* varJson, jsonBuildOption cJoption, void* display, valueType numType, displayType uitype, bool varEnabled = true, bool boolStr = false);
    void* pDisplay;
    char* varName;
    char* unit;
    displayType uiType;
    bool enabled;
    bool boolString;
    valueType vt;
    jsonBuildOption options;
    int scaler;
    char* reg_name;
    char* obj_name;
    bool configured;
};

// TODO: move this section into separate header files
void build_on_off_option(fmt::memory_buffer&);
void build_yes_no_option(fmt::memory_buffer&);
void build_close_option(fmt::memory_buffer&);
void build_open_option(fmt::memory_buffer&);
void build_reset_option(fmt::memory_buffer&);
std::string build_uri(std::string comp, char* reg);

class Asset {
public:
    Asset ();
    virtual ~Asset() = default;

    // configuration
    bool configure(Type_Configurator* configurator);

    // status
    std::string get_name(void);
    std::string get_id(void);
    const char* get_asset_type(void) const;
    const std::string get_comp_name(int) const;

    bool is_available(void);
    bool is_running(void);
    bool is_controllable(void);
    bool in_maint_mode(void);
    bool in_standby(void);

    uint64_t get_status(void) const;

    int get_num_comps(void) const;

    float get_rated_active_power(void) const;
    float get_rated_reactive_power(void) const;
    float get_rated_apparent_power(void) const;

    float get_active_power(void) const;
    float get_reactive_power(void) const;
    float get_apparent_power(void) const;

    float get_max_potential_active_power(void);
    float get_min_potential_active_power(void);
    float get_max_limited_active_power(void);
    float get_potential_reactive_power(void);
    int get_active_power_slew_rate(void);
    float get_active_power_slew_max_target(void);
    float get_active_power_slew_min_target(void);
    float get_active_power_slew_target(float target);

    float get_voltage_l1_l2(void) const;
    float get_voltage_l2_l3(void) const;
    float get_voltage_l3_l1(void) const;
    float get_voltage_l1_n(void) const;
    float get_voltage_l2_n(void) const;
    float get_voltage_l3_n(void) const;
    float get_voltage_avg_line_to_line(void) const;
    float get_voltage_avg_line_to_neutral(void) const;
    float get_current_l1(void) const;
    float get_current_l2(void) const;
    float get_current_l3(void) const;
    float get_current_avg(void) const;
    float get_power_factor(void) const;
    bool get_watchdog_fault(void) const;
    void set_reactive_power_priority(bool priority);

    demandMode get_demand_mode(void) const;

    // internal functions
    virtual void process_asset(); // process incoming component data
    virtual void set_raw_status() = 0;
    bool process_watchdog_status();

    // pure virtual functions that child classes must implement
    virtual void update_asset() = 0;        // update outgoing component data
    virtual void send_to_components() = 0;  // send data to components (send_to_comp_uri)

    // FIMS
    bool handle_get(fims_message *pmsg);
    bool add_asset_data_to_buffer(fmt::memory_buffer &buf, bool clothed);
    bool add_variable_to_buffer(std::string uri, const char* variable, fmt::memory_buffer &buf);
    virtual bool handle_set(std::string uri, cJSON &body) = 0;
    bool handle_generic_asset_controls_set(std::string uri, cJSON &body);
    virtual bool generate_asset_ui(fmt::memory_buffer &buf, const char* const var = NULL) = 0;

protected:
    // Indicates whether this is the primary controller (true) or running in shadow mode (false)
    bool* is_primary;
    // configuration
    bool parse_variable(cJSON* var_json, std::string comp_id);
    void var_maps_insert(Fims_Object* variable, std::map<std::string, std::vector<Fims_Object*>>* const component_var_map);
    std::string build_asset_variable_uri(const char* var);
    bool validate_config();
    bool configure_common_asset_instance_vars(Type_Configurator* configurator);
    bool configure_component_vars(Type_Configurator* configurator);
    bool configure_common_asset_fims_vars(Type_Configurator* configurator);
    bool configure_single_fims_var(Fims_Object* fims_var, const char* var_id, Type_Configurator* configurator, valueType type=Float, float default_float=0.0, int default_int=0, bool default_bool=false, bool comes_from_component=true, const char* var_name="", const char* var_units="", int var_scaler=1);
    void update_fims_var(Fims_Object* fims_var, valueType type, float default_float, int default_int, bool default_bool, const char* var_id="", const char* var_name="", const char* var_units="", int var_scaler=1);
    bool configure_watchdog_vars();
    void add_dynamic_variables_to_maps(Type_Configurator* configurator);
    virtual bool configure_typed_asset_fims_vars(Type_Configurator* configurator) = 0;
    /**
     * Here is where the connection between asset and component var maps is severed, so that the raw values will still be sourced from
     * components, while the calculated values will be sourced from the asset's calculated variables
     */
    virtual bool replace_typed_raw_fims_vars() { return true; };
    virtual bool configure_typed_asset_instance_vars(Type_Configurator* configurator) = 0;
    virtual bool configure_ui_controls(Type_Configurator* configurator) = 0;
    std::string name;
    std::string asset_id;
    std::list<const char*> required_variables;
    // Map of all asset uris to unique Fims_Objects
    std::map<std::string, Fims_Object*> asset_var_map;
    // Map of all Fims_Objects that are dynamically determined by configuration
    // These are variables that do not have hard-coded references as members of the Asset
    std::unordered_map<std::string, Fims_Object> dynamic_variables;

    // All hard-coded member variables of the Asset
    // control points
    Fims_Object active_power_setpoint;
    Fims_Object reactive_power_setpoint;
    Fims_Object active_power;
    Fims_Object reactive_power;
    Fims_Object apparent_power;
    // status points
    Fims_Object voltage_l1_l2; // Line 1 - Line 2
    Fims_Object voltage_l2_l3; // Line 2 - Line 3
    Fims_Object voltage_l3_l1; // Line 3 - Line 1
    Fims_Object voltage_l1_n; // Line 1 - N
    Fims_Object voltage_l2_n; // Line 2 - N
    Fims_Object voltage_l3_n; // Line 3 - N
    Fims_Object current_l1;
    Fims_Object current_l2;
    Fims_Object current_l3;
    Fims_Object power_factor;
    Fims_Object watchdog_heartbeat;
    Fims_Object component_connected;
    Fims_Object watchdog_status;

    std::vector<std::string> compNames;
    const char* statusStrings[MAX_STATUS_BITS];

    uint32_t numAssetComponents;

    uint64_t running_status_mask;
    uint64_t standby_status_mask;  // Mask indicating the standby status value
    uint64_t stopped_status_mask;

    int prev_watchdog_heartbeat;
    int watchdog_timeout_ms;
    bool watchdog_enable;
    bool fims_timeout;
    timespec fims_timer;
    bool check_fims_timeout(void);

    float rated_active_power_kw;
    float rated_reactive_power_kvar;
    float rated_apparent_power_kva;
    float active_power_limit;
    bool reactive_power_priority;
    float nominal_voltage;
    float nominal_frequency;
    float numPhases;
    bool connected_rising_edge_detect;  // Variable that tells if an Asset is connected or disconnect from the modbus_client

    double throttle_timeout_fast_ms;
    double throttle_timeout_slow_ms;
    double throttle_deadband_percentage;

    const char* asset_type_id = "";
    statusType status_type;
    demandMode assetControl;

    // control
    fimsCtl maint_mode;
    fimsCtl lock_mode;
    Slew_Object active_power_slew;

    // status
    bool isAvail;
    bool isRunning;
    bool inMaintenance;
    bool inStandby;
    bool inLockdown;

    uint64_t internal_status;
    uint64_t raw_status;

    float max_potential_active_power;
    float min_potential_active_power;
    float min_limited_active_power;  // Active power values limited by reactive power when prioritized to maintain apparent power rating
    float max_limited_active_power;
    float potential_reactive_power;

    // uris
    std::string uri_clear_faults;

    // internal functions
    virtual void process_potential_active_power(void);
    virtual void process_potential_reactive_power(void);

    // component interface functions
    bool send_to_comp_uri(const char* value, const std::string& uri);
    bool send_to_comp_uri(int value, const std::string& uri);
    bool send_to_comp_uri(float value, const std::string& uri);
    bool send_to_comp_uri(bool value, const std::string& uri);
    bool json_object_send(std::string& value, const std::string& uri);

    bool send_setpoint(std::string uri, cJSON* valueObject);

    fmt::memory_buffer send_FIMS_buf;  // Reusable and resizable string buffer for generating FIMS messages

    ////////////////////////////////////////////////////////////////////////////////////////
    //                                 FAULT & ALARM HANDLING                             //
    ////////////////////////////////////////////////////////////////////////////////////////
public:
    void clear_alerts(void);
    int get_num_active_alarms(void) const;
    int get_num_active_faults(void) const;
    bool is_newly_faulted(void) const;
    bool check_alert(std::string& id, uint64_t& mask);
    bool check_fault(std::string& id, uint64_t& mask);
    bool check_alarm(std::string& id, uint64_t& mask);

protected:
    void lower_alert_bits(void);
    // fault values are latched, meaning even if a component starts reporting a previously-active
    //      fault as inactive, the asset will keep the fault value recorded as active until the
    //      user acknowledges the fault occurrence by pressing the Clear Faults button on either
    //      the Site page or the Assets page.
    // alarm values are saved, meaning the last value reported by the component is remembered and
    //      compared to the next value reported by the component. This comparison allows for
    //      keeping track of rising and falling edges.
    std::map<std::string, uint64_t> latched_faults, saved_alarms;
    // component_fault/alarm_registers are the Fims_Objects that receive alert statuses directly from components.
    // if a component starts reporting no alerts, these will be cleared by the component
    std::vector<Fims_Object*> component_fault_registers, component_alarm_registers;
    bool clearFaultsControlEnable;
    bool clear_fault_registers_flag;
    timespec clear_faults_time;
    // newly_faulted is true for a single iteration when an asset goes from not having any faults to having at least one fault
    bool newly_faulted;
    Fims_Object watchdog_fault;
    Fims_Object is_faulted;
    Fims_Object is_alarmed;

protected:
    // Friend classes
    friend class ESS_Manager_Mock;
    friend class Feeder_Manager_Mock;
    friend class Generator_Manager_Mock;
    friend class Solar_Manager_Mock;
};

class Write_Rate_Throttle {
public:
    Write_Rate_Throttle();
    virtual ~Write_Rate_Throttle();

    void reset(void);
    void configure(long timeout, float rated = 0, float deadband = 0);

    bool command_trigger(void);            // time throttle
    bool setpoint_trigger(float control);  // time and deadband throttle
    long current_timestamp(void);

private:
    // configuration
    float rated_power;
    long throttle_timeout;
    float deadband_percentage;

    // previous iteration variables
    long status_time;
    float control_feedback;
};

#endif /* ASSET_H_ */
