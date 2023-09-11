/*
 * Asset_Solar.h
 *
 *  Created on: August 14, 2018
 *      Author: kbrezina
 */

#ifndef ASSET_SOLAR_H_
#define ASSET_SOLAR_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Asset.h>

class Asset_Solar : public Asset {
public:
    Asset_Solar();

    // control
    bool start(void);
    bool stop(void);
    bool enter_standby(void);
    bool exit_standby(void);

    void set_active_power_setpoint(float);
    void set_reactive_power_setpoint(float);
    void set_power_factor_setpoint(float);

    void set_potential_active_power_limit(float limit);

    void set_power_mode(powerMode);

    // status
    float get_active_power_setpoint(void);
    float get_reactive_power_setpoint(void);
    float get_power_factor_setpoint(void);

    // internal functions
    void process_asset(void);
    void set_raw_status() override;
    const char* get_status_string() const override;
    void update_asset(void);
    void send_to_components(void) override;
    bool handle_set(std::string uri, cJSON& body);
    bool generate_asset_ui(fmt::memory_buffer&, const char* const var = NULL) override;

protected:
    // configuration
    void set_required_variables(void);
    bool configure_typed_asset_instance_vars(Type_Configurator* configurator);
    bool configure_ui_controls(Type_Configurator* configurator);
    bool configure_typed_asset_fims_vars(Type_Configurator* configurator);
    int start_value;
    int stop_value;
    int enter_standby_value;
    int exit_standby_value;

    int reactive_power_mode_value;
    int power_factor_mode_value;

    // setpoints
    Fims_Object power_mode_setpoint;
    Fims_Object power_factor_setpoint;
    Fims_Object status;

    // control
    float maint_active_power_setpoint;
    float maint_reactive_power_setpoint;

    // status
    bool curtailed_status;

    float curtailed_active_power_limit;

    fimsCtl stop_ctl;
    fimsCtl start_ctl;
    fimsCtl clear_faults_ctl;
    fimsCtl enter_standby_ctl;
    fimsCtl exit_standby_ctl;
    fimsCtl maint_active_power_setpoint_ctl;
    fimsCtl maint_reactive_power_setpoint_ctl;

    // uris
    std::string uri_start;
    std::string uri_stop;
    std::string uri_enter_standby;
    std::string uri_exit_standby;

    // internal functions
    void process_potential_active_power(void) override;

    bool send_active_power_setpoint(void);
    bool send_reactive_power_setpoint(void);
    bool send_power_factor_setpoint(void);
    bool send_power_mode(void);
};

#endif /* ASSET_SOLAR_H_ */
