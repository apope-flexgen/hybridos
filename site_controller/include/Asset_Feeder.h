/*
 * Asset_Grid.h
 *
 *  Created on: August 14, 2018
 *      Author: kbrezina
 */

#ifndef ASSET_FEEDER_H_
#define ASSET_FEEDER_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Asset.h>

class Asset_Feeder: public Asset
{
public:
    Asset_Feeder();
    virtual ~Asset_Feeder();

    // configuration
    bool validate_poi_feeder_configuration(Type_Configurator* configurator);

    // control
    bool breaker_open(void);
    bool breaker_close(void);
    bool breaker_close_permissive(void);
    bool breaker_close_permissive_remove(void);
    void breaker_reset(void);

    void set_active_power_setpoint(float);

    // status
    bool get_breaker_status(void);
    bool get_utility_status(void);
    float get_gridside_avg_voltage(void);
    float get_gridside_frequency(void);
    float get_power_factor();

    // internal functions
    void process_asset(bool* status);
    void process_potential_active_power() override;
    void update_asset(void);
    void send_to_components(void) override;
    bool handle_set(std::string uri, cJSON &body);
    bool generate_asset_ui(fmt::memory_buffer&, const char* const var = NULL) override;
    
protected:
    // configuration
    void set_required_variables(void);
    bool configure_typed_asset_instance_vars(Type_Configurator* configurator);
    bool configure_ui_controls(Type_Configurator* configurator);
    bool configure_typed_asset_fims_vars(std::map <std::string, Fims_Object*> * const asset_var_map);
    int open_value;
    int close_value;
    int close_permissive_value;
    int close_permissive_remove_value;
    int reset_value;

    // status points
    Fims_Object* grid_voltage_l1;
    Fims_Object* grid_voltage_l2;
    Fims_Object* grid_voltage_l3;
    Fims_Object* grid_frequency;
    Fims_Object* breaker_status;
    Fims_Object* utility_status;    // Register tracking the status of the utility for sites that support it

    // control
    fimsCtl breaker_open_ctl;
    fimsCtl breaker_close_ctl;
    fimsCtl breaker_close_perm_ctl;
    fimsCtl breaker_close_perm_remove_ctl;
    fimsCtl breaker_reset_ctl;

    // status
    bool breaker_close_permissive_status;

    // uris
    std::string uri_breaker_open;
    std::string uri_breaker_close;
    std::string uri_breaker_close_permissive;
    std::string uri_breaker_close_permissive_remove;
    std::string uri_breaker_reset;
};

#endif /* ASSET_FEEDER_H_ */
