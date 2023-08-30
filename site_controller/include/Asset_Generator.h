/*
 * Asset_Generator.h
 *
 *  Created on: May 9, 2018
 *      Author: jcalcagni
 */

#ifndef ASSET_GENERATOR_H_
#define ASSET_GENERATOR_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Asset.h>
#include <LDSS.h>

class LDSS;

class Asset_Generator : public Asset {
public:
    Asset_Generator();
    virtual ~Asset_Generator();

    // control
    bool start(void);
    bool stop(void);

    void set_grid_mode(gridMode);

    void set_active_power_setpoint(float);
    void set_reactive_power_setpoint(float);

    void set_stopping_flag(bool flag);
    void set_balanced(bool flag);

    // status
    bool get_start_status(void);
    bool get_stop_status(void);

    // used by Asset Manager to distribute power commands
    bool is_balanced(void);
    bool is_stopping(void);

    bool is_starting(void) const;

    bool is_stopped(void);  // raw status from the component

    float get_active_power_setpoint(void);
    float get_reactive_power_setpoint(void);
    // control_setpoints with updated power values
    float get_active_power_setpoint_control(void);
    float get_reactive_power_setpoint_control(void);

    gridMode get_grid_mode(void);

    // internal functions
    void process_asset(void);
    void update_asset(void);
    void send_to_components(void) override;
    bool handle_set(std::string uri, cJSON& body);
    bool generate_asset_ui(fmt::memory_buffer&, const char* const var = NULL) override;

protected:
    // configuration
    void set_required_variables(void);
    bool configure_typed_asset_instance_vars(Type_Configurator* configurator);
    bool configure_ui_controls(Type_Configurator* configurator);
    bool configure_typed_asset_fims_vars(std::map<std::string, Fims_Object*>* const asset_var_map);
    uint64_t starting_status_mask;
    uint64_t stopping_status_mask;

    int start_value;
    int stop_value;
    int grid_forming_value;
    int grid_following_value;

    // setpoints
    Fims_Object* grid_mode_setpoint;

    // control
    float maint_active_power_setpoint;
    float maint_reactive_power_setpoint;

    fimsCtl start_ctl;
    fimsCtl stop_ctl;
    fimsCtl clear_faults_ctl;
    fimsCtl start_next_ctl;
    fimsCtl stop_next_ctl;
    fimsCtl maint_active_power_setpoint_ctl;
    fimsCtl maint_reactive_power_setpoint_ctl;

    // status
    bool isBalanced;
    bool isStopping;
    bool isStarting;
    bool isStopped;

    // uris
    std::string uri_start;
    std::string uri_stop;

    // internal functions
    bool send_grid_mode(void);

    bool send_active_power_setpoint(void);
    bool send_reactive_power_setpoint(void);

    void process_potential_active_power(void) override;

    //////////////////////////////////////////////////
    ////////////////// LDSS //////////////////////////
    //////////////////////////////////////////////////
public:
    //
    // accessor functions
    //
    // gets
    bool get_block_ldss_static_starts_flag(void);
    bool get_block_ldss_static_stops_flag(void);
    bool is_warmup_timer_active(void) const;
    bool is_cooldown_timer_active(void) const;
    bool get_cooling_down_flag(void);
    bool get_warming_up_flag(void);
    int get_warmup_timer(void) const;
    int get_cooldown_timer(void) const;
    int get_dynamic_start_priority(void) const;
    int get_dynamic_stop_priority(void) const;
    int get_static_start_priority(void) const;
    int get_static_stop_priority(void) const;
    // sets
    void set_cooling_down_flag(bool);
    void set_warming_up_flag(bool);
    void block_starts_based_on_static_priority(bool);
    void block_stops_based_on_static_priority(bool);
    bool set_dynamic_start_priority(int);
    bool set_dynamic_stop_priority(int);
    void set_static_start_priority(int);
    void set_static_stop_priority(int);
    //
    // operational methods
    //
    void reset_warmup_timer();
    void reset_cooldown_timer();
    void tick_warmup_timer();
    void tick_cooldown_timer();
    //
    // configuration
    //
    LDSS* ldss;

protected:
    // flags
    bool block_ldss_static_starts;
    bool block_ldss_static_stops;
    bool cooling_down;
    bool warming_up;
    // timers
    int cooldown_countdown;
    int warmup_countdown;
    // priorities
    int static_start_priority;
    int static_stop_priority;
    int dynamic_start_priority;
    int dynamic_stop_priority;
};

#endif /* ASSET_GENERATOR_H_ */
