/*
 * Asset_Manager.h
 *
 *  Created on: Sep 4, 2018
 *      Author: ghoward
 */

#ifndef ASSET_MANAGER_H_
#define ASSET_MANAGER_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Asset_ESS.h>
#include <Asset_Generator.h>
#include <Asset_Feeder.h>
#include <Asset_Solar.h>
#include <ESS_Manager.h>
#include <Feeder_Manager.h>
#include <Generator_Manager.h>
#include <Solar_Manager.h>
#include <Value_Object.h>
#include <Fims_Object.h>
#include <Types.h>

class Asset_Manager {
public:
    Asset_Manager();
    ~Asset_Manager();

    // init methods
    int debugLoopCount;
    bool asset_create(cJSON* pJsonRoot, bool* primary_controller);

    void set_min_generators_active(int);

    void start_available_solar(void);
    bool start_all_solar(void);
    bool stop_all_solar(void);
    bool enter_standby_all_solar(void);
    bool exit_standby_all_solar(void);

    void set_gen_clear_faults(void);
    void set_ess_clear_faults(void);
    void set_solar_clear_faults(void);
    void set_feeder_clear_faults(void);

    void start_available_ess(void);
    bool start_all_ess(void);
    bool stop_all_ess(void);
    bool close_all_bms_contactors(void);
    bool open_all_bms_contactors(void);
    bool enter_standby_all_ess(void);
    bool exit_standby_all_ess(void);

    bool set_poi_feeder_state_open();
    bool set_poi_feeder_state_closed();
    bool set_feeder_state_open(Asset_Feeder* target_feeder);
    bool set_feeder_state_closed(Asset_Feeder* target_feeder);

    bool set_gen_target_active_power(float);
    bool set_ess_target_active_power(float);
    bool set_poi_target_active_power(float);
    void set_solar_target_active_power(float);

    bool set_pcs_nominal_voltage_setting(float mPcsNominalVoltageSetting);
    bool site_clear_faults(void);

    // operational methods
    float charge_control(float target_soc_desired, bool charge_disable, bool discharge_disable);

    void fims_data_parse(fims_message* pmsg);
    void process_asset_data(void);
    void update_asset_data(void);
    void send_to_components(void);

    void enable_ldss(bool);
    void start_first_gen(bool enable);
    void set_first_gen_is_starting_flag(bool flag);

    // status methods
    int get_num_ess_avail(void);
    int get_num_solar_avail(void);
    int get_num_gen_avail(void);
    int get_num_feeder_avail(void);

    int get_num_ess_parsed(void);
    int get_num_solar_parsed(void);
    int get_num_gen_parsed(void);
    int get_num_feeder_parsed(void);

    int get_num_ess_running(void);
    int get_num_solar_running(void);
    int get_num_gen_running(void);
    int get_num_feeder_running(void);

    int get_num_ess_startable(void);
    int get_num_solar_startable(void);

    int get_num_ess_controllable(void);
    int get_num_solar_controllable(void);
    int get_num_gen_controllable(void);

    int get_num_ess_in_standby(void);
    int get_num_solar_in_standby(void);

    Asset_Feeder* validate_feeder_id(const char* feeder_ID);
    bool get_feeder_state(Asset_Feeder* target_feeder);
    bool get_feeder_utility_status(Asset_Feeder* target_feeder);

    float get_ess_soc_max(void);
    float get_ess_soc_min(void);
    float get_ess_soc_avg(void);

    float get_all_ess_soc_max(void);
    float get_all_ess_soc_min(void);
    float get_all_ess_soc_avg(void);

    float get_pcs_nominal_voltage_setting(void);
    float get_avg_ac_voltage(const char* feeder_ID);

    float get_ess_total_active_power(void);
    float get_ess_total_reactive_power(void);
    float get_ess_total_uncontrollable_active_power(void);

    float get_ess_total_max_potential_active_power(void);
    float get_ess_total_min_potential_active_power(void);
    float get_ess_total_potential_reactive_power(void);

    float get_ess_total_rated_active_power(void);
    float get_ess_total_rated_reactive_power(void);
    float get_ess_total_rated_apparent_power(void);

    float get_ess_total_kW_charge_limit(void);
    float get_ess_total_kW_discharge_limit(void);

    float get_ess_total_nameplate_active_power(void);
    float get_ess_total_nameplate_reactive_power(void);
    float get_ess_total_nameplate_apparent_power(void);

    float get_soc_balancing_factor(void);

    float get_solar_total_active_power(void);
    float get_solar_total_reactive_power(void);
    float get_solar_total_uncontrollable_active_power(void);

    float get_solar_total_max_potential_active_power(void);
    float get_solar_total_min_potential_active_power(void);
    float get_solar_total_potential_reactive_power(void);

    float get_solar_total_rated_active_power(void);
    float get_solar_total_rated_reactive_power(void);
    float get_solar_total_rated_apparent_power(void);

    float get_solar_total_nameplate_active_power(void);
    float get_solar_total_nameplate_reactive_power(void);
    float get_solar_total_nameplate_apparent_power(void);

    float get_feeder_active_power(const char* feeder_ID);
    float get_feeder_reactive_power(const char* feeder_ID);

    float get_feeder_nameplate_active_power(const char* feeder_id);
    float get_poi_nameplate_active_power(void);

    float get_gen_total_active_power(void);
    float get_gen_total_reactive_power(void);
    float get_gen_total_uncontrollable_active_power(void);

    float get_gen_total_max_potential_active_power(void);
    float get_gen_total_min_potential_active_power(void);
    float get_gen_total_rated_active_power(void);
    float get_gen_total_potential_reactive_power(void);
    float get_gen_total_nameplate_active_power(void);

    float get_ess_total_chargeable_power_kW(void);
    float get_ess_total_dischargeable_power_kW(void);
    float get_ess_total_chargeable_energy_kWh(void);
    float get_ess_total_dischargeable_energy_kWh(void);
    std::vector<int> get_ess_setpoint_statuses(void);

    int get_num_active_faults(assetType type) const;
    int get_num_active_alarms(assetType type) const;
    bool check_asset_alert(std::pair<std::string, uint64_t>& alert);

    const std::string get_poi_id(void);
    bool get_poi_feeder_state(void);
    bool get_poi_feeder_close_permissive_state(void);
    float get_poi_gridside_frequency(void);
    float get_poi_gridside_avg_voltage(void);
    float get_poi_power_factor();
    float get_poi_max_potential_active_power(void);
    float get_poi_min_potential_active_power(void);

    bool get_sync_feeder_status(void);
    float get_sync_feeder_gridside_frequency(void);
    float get_sync_frequency_offset(void);
    float get_sync_feeder_gridside_avg_voltage(void);
    bool set_sync_feeder_close_permissive(void);
    bool set_sync_feeder_close_permissive_remove(void);

    void set_all_gen_grid_form(void);
    void set_all_gen_grid_follow(void);
    void set_all_ess_grid_form(void);
    void set_all_ess_grid_follow(void);
    void set_solar_curtailment_enabled(bool enable);

    bool set_gen_target_reactive_power(float);
    bool set_ess_target_reactive_power(float);
    bool set_solar_target_reactive_power(float desiredkW);

    void set_ess_target_power_factor(float);
    void set_solar_target_power_factor(float);

    void set_solar_reactive_kvar_mode(void);
    void set_ess_reactive_kvar_mode(void);
    void set_solar_pwr_factor_mode(void);
    void set_ess_pwr_factor_mode(void);

    void set_ess_voltage_setpoint(float setpoint);
    void set_ess_frequency_setpoint(float setpoint);

    void set_ess_calibration_vars(ESS_Calibration_Settings settings);
    void set_reactive_power_priority(bool priority);

    float get_grid_forming_voltage_slew(void);
    void set_grid_forming_voltage_slew(float slope);

    float get_total_kW_charge_limit(void);
    float get_total_kW_discharge_limit(void);

    void update_ldss_settings(LDSS_Settings&& settings);

    bool direct_start_gen(void);
    void start_all_gen(void);
    bool stop_all_gen(void);

    void publish_assets(void);

protected:
    // Indicates whether this is the primary controller (true) or running in shadow mode (false)
    bool* is_primary;

    fmt::memory_buffer send_FIMS_buf;  // Reusable and resizable string buffer used to send FIMS messages

    Type_Manager* get_type_manager(std::string type);
    Type_Manager* get_type_manager(const char* type);
    ESS_Manager* ess_manager;
    Feeder_Manager* feeder_manager;
    Generator_Manager* generator_manager;
    Solar_Manager* solar_manager;

    Type_Configurator* ess_configurator;
    Type_Configurator* feeder_configurator;
    Type_Configurator* generator_configurator;
    Type_Configurator* solar_configurator;

    // control variables
    std::map<std::string, std::vector<Fims_Object*>> component_var_map;
    std::map<std::string, Fims_Object*> asset_var_map;

    void handle_pubs(char** pfrags, int nfrags, char* body);
    void handle_pub_status_options(cJSON* cJcomp, Fims_Object* fimsComp, int varArraySize);
    void handle_pub_alarm_or_fault_options(cJSON* cJcomp, Fims_Object* fimsComp, int varArraySize);
    void handle_pub_other_options(cJSON* cJcomp, Fims_Object* fimsComp, int varArraySize);
    void handle_get(fims_message* pmsg);
    void handle_set(fims_message& msg);
    void handle_post(int nfrags, char* body);
    void handle_del(int nfrags, char* body);
    void send_all_asset_data(char* uri);

    void print_component_var_map();
    void print_asset_var_map();
    bool build_configurators(void);
};

#endif /* ASSET_MANAGER_H_ */
