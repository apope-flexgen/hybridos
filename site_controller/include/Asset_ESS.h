/*
 * Asset_ESS.h
 *
 *  Created on: August 14, 2018
 *      Author: kbrezina
 */

#ifndef ASSET_ESS_H_
#define ASSET_ESS_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Asset.h>

class Asset_ESS : public Asset {
public:
    Asset_ESS();

    // control
    bool start(void);
    bool stop(void);
    bool enter_standby(void);
    bool exit_standby(void);

    bool close_bms_contactors(void);
    bool open_bms_contactors(void);

    bool set_autobalancing(bool status);

    void set_grid_mode(gridMode mode);
    void set_power_mode(powerMode mode);

    void set_active_power_setpoint(float setpoint, bool use_strict_limits);
    void set_reactive_power_setpoint(float);
    void set_power_factor_setpoint(float);

    void set_voltage_slew_setpoint(float);
    void set_voltage_setpoint(float);
    void set_frequency_setpoint(float);
    void set_pcs_nominal_voltage_setting(float);
    void set_calibration_vars(ESS_Calibration_Settings settings);

    // status
    int get_soh(void);

    float get_active_power_setpoint(void);
    // control_setpoint with the updated active power
    float get_active_power_setpoint_control(void);
    float get_reactive_power_setpoint(void);
    // control_setpoint with the updated reactive power
    float get_reactive_power_setpoint_control(void);
    float get_power_factor_setpoint(void);

    float get_voltage_slew_setpoint(void);
    float get_pcs_nominal_voltage_setting(void);

    float get_chargeable_power(void);
    float get_dischargeable_power(void);
    float get_chargeable_energy(void);
    float get_dischargeable_energy(void);
    float get_min_limited_active_power(void);
    float get_max_limited_active_power(void);
    float get_soc(void);
    float get_max_temp(void);
    setpoint_states get_setpoint_status(void);

    gridMode get_grid_mode(void);

    // internal functions
    void process_asset();
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
    bool replace_typed_raw_fims_vars() override;
    int start_value;
    int stop_value;
    int enter_standby_value;
    int exit_standby_value;
    int grid_forming_value;
    int grid_following_value;
    setpoint_states setpoint_status;

    int bms_control_close;
    int bms_control_open;
    int bms_control_reset;
    bool dc_contactor_restriction;  // Bool indicating whether contactors must be closed in order to send maintenance mode commands

    int reactive_power_mode_value;
    int power_factor_mode_value;

    float rated_chargeable_power;
    float rated_dischargeable_power;

    float chgSocBegin;                  // Beginning of high end SoC derating. Chargeable power is limited towards zero when above this value
    float chgSocEnd;                    // End of high end SoC derating. Chargeable power is zero when at or above this value
    float dischgSocBegin;               // Beginning of low end SoC derating. Dischargeable power is limited towards zero when below this value
    float dischgSocEnd;                 // End of low end SoC derating. Dischargeable power is zero when at or below this value
    float maxRawSoc;                    // Highest (raw) SoC value allowed based on configuration as part of derating. Typically 97 and disabled with 100
    float minRawSoc;                    // Lowest (raw) SoC value allowed based on configuration as part of derating. Typically 4 and disabled with 0
    float rated_capacity;               // configurable battery base capacity
    bool calibration_flag;              // Flag indicating in calibration mode
    bool limits_override_flag;          // Bypass soc limits to publish raw value
    bool maint_limits_override_flag;    // Flag tracking maint mode sets to limits override only, not sets from site manager features
    bool soc_limits_flag;               // Enable soc-based limits on (dis)chargeable power
    float chargeable_soc_limit;         // Chargeable power set to 0 when above this soc
    float dischargeable_soc_limit;      // Dischargeable power set to 0 when below this soc
    bool voltage_limits_flag;           // Enable voltage-based limits on (dis)chargeable power
    float chargeable_voltage_limit;     // Chargeable power set to chargeable_min_limit_kW when voltage_max above this limit
    float dischargeable_voltage_limit;  // Dischargeable power set to dischargeable_min_limit_kW when voltage_min below this limit
    float chargeable_min_limit_kW;      // Configurable floor for chargeable power. Derating will not go below this value
    float dischargeable_min_limit_kW;   // Configurable floor for dischargeable power. Derating will not go below this value
    float raw_calibration_setpoint;     // Calibration setpoint command by the feature, used as a reference for the setpoint status

    // control
    float maint_active_power_setpoint;
    float maint_reactive_power_setpoint;

    fimsCtl start_ctl;
    fimsCtl stop_ctl;
    fimsCtl clear_faults_ctl;
    fimsCtl enter_standby_ctl;
    fimsCtl exit_standby_ctl;
    fimsCtl limits_override_ctl;
    fimsCtl autobalancing_enable_ctl;
    fimsCtl autobalancing_disable_ctl;
    fimsCtl maint_active_power_setpoint_ctl;
    fimsCtl maint_reactive_power_setpoint_ctl;
    fimsCtl connected_ctl;
    fimsCtl disconnected_ctl;
    fimsCtl open_dc_contactors_ctl;
    fimsCtl close_dc_contactors_ctl;

    Write_Rate_Throttle active_power_setpoint_throttle;
    Write_Rate_Throttle reactive_power_setpoint_throttle;
    Write_Rate_Throttle start_command_throttle;
    Write_Rate_Throttle stop_command_throttle;

    // status
    bool energy_configured;  // Flag to indicate whether chargeable/dischargeable_energy was configured

    Fims_Object grid_mode_setpoint;
    Fims_Object power_mode_setpoint;
    Fims_Object power_factor_setpoint;
    Fims_Object voltage_slew_setpoint;  // units in %/s
    Fims_Object voltage_setpoint;
    Fims_Object frequency_setpoint;
    // TODO: these are Sungrow specific fields, remove and cleanup
    Fims_Object pcs_a_nominal_voltage_setpoint;
    Fims_Object pcs_b_nominal_voltage_setpoint;
    // status points
    Fims_Object soh;  // battery state of health
    Fims_Object chargeable_power;
    Fims_Object dischargeable_power;
    Fims_Object chargeable_power_raw;
    Fims_Object dischargeable_power_raw;
    Fims_Object soc;      // battery state of charge
    Fims_Object soc_raw;  // unscaled battery state of charge from asset
    Fims_Object chargeable_energy;
    Fims_Object dischargeable_energy;
    Fims_Object chargeable_energy_raw;
    Fims_Object dischargeable_energy_raw;
    Fims_Object max_temp;  // battery max temperature
    Fims_Object min_temp;  // battery min temperature
    Fims_Object racks_in_service;
    Fims_Object dc_contactors_closed;
    Fims_Object autobalancing_status;  // Status of the autobalancing register
    Fims_Object voltage_min;
    Fims_Object voltage_max;
    Fims_Object status;

    // uris
    std::string uri_start;
    std::string uri_stop;
    std::string uri_enter_standby;
    std::string uri_exit_standby;
    std::string uri_open_dc_contacts;
    std::string uri_close_dc_contacts;
    std::string uri_set_autobalancing;

    // internal functions
    void process_potential_active_power(void) override;

    float process_soc(float);

    bool send_grid_mode(void);
    bool send_power_mode(void);

    bool send_active_power_setpoint(void);
    bool send_reactive_power_setpoint(void);
    bool send_power_factor_setpoint(void);

    bool send_voltage_slew_setpoint(void);
    bool send_voltage_setpoint(void);
    bool send_frequency_setpoint(void);

    bool send_pcs_nominal_voltage_setting(void);

    // Friend classes
    friend class ESS_Manager_Mock;
};

#endif /* ASSET_ESS_H_ */
