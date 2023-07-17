/*
 * Asset_Cmd_Object.h
 *
 *  Created on: Nov 5, 2019
 *      Author: kbrezina
 */

#ifndef INCLUDE_ASSET_CMD_OBJECT_H_
#define INCLUDE_ASSET_CMD_OBJECT_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */

enum discharge_type
{
    LOAD, REQUESTS, DEMAND
};

// Types of load compensation available
enum load_compensation
{
    NO_COMPENSATION,    // No load compensation being provided
    LOAD_OFFSET,             // Always increase discharge to offset load
    LOAD_MINIMUM             // Only increase discharge if load is not met
};

class Asset_Cmd_Object
{
    public:

    struct Asset_Cmd_Data
    {
        Asset_Cmd_Data();

        float calculate_source_kW_contribution(float cmd);
        float calculate_unused_charge_kW();
        float calculate_unused_dischg_kW();

        float kW_request;       // Power requested of this asset by the active power feature and its standalone modifications
        float saved_kW_request; // Record of the kW_request before its modification
        float kW_cmd;           // Power commanded of this asset after the active power dispatch step
        float saved_kW_cmd;     // Record of the kW_cmd before its modification
        float actual_kW;        // Actual active power measured by the asset
        float start_first_kW;   // Power above which the asset should be started
        bool start_first_flag;  // Flag indicating whether to enable the start_first_kW check

        float min_potential_kW; //minimum possible instantaneous active power available from asset
        float max_potential_kW; //maximum possible instantaneous active power available from asset

        float kVAR_cmd;         // Power commanded of this asset after the reactive power dispatch step
        float actual_kVAR;      // Actual reactive power measured by the asset
        float potential_kVAR;   // maximum possible instantaneous reactive power available from the asset
    };

    Asset_Cmd_Object();
    ~Asset_Cmd_Object();

    float calculate_net_poi_export();
    float poi_cmd; // Command desired at the POI. Used by closed loop control to avoid potential feedback loops with site demand
    void calculate_site_kW_load();
    float calculate_additional_load_compensation();
    void track_unslewed_load(load_compensation method_of_compensation);
    void track_slewed_load(load_compensation method_of_compensation, float unslewed_demand, float additional_load, Slew_Object& feature_slew);
    void calculate_feature_kW_demand(int asset_priority);
    void calculate_total_potential_kVAR();
    void calculate_site_kW_production_limits();
    bool determine_ess_load_requirement(int asset_priority);
    float dispatch_discharge_requests(int asset_priority, float cmd);
    float dispatch_site_kW_discharge_cmd(int asset_priority, float cmd, discharge_type command_type);
    float dispatch_site_kW_charge_cmd(int asset_priority, bool solar_source_flag, bool gen_source_flag, bool feeder_source_flag);
    float ess_overload_support(float grid_target_kW_cmd);
    void target_soc_mode(bool load_requirement);
    void site_export_target_mode(bool load_enable_flag, Slew_Object* export_target_slew, float export_target_kW_cmd);
    void manual_mode(float manual_ess_kW_cmd, float manual_solar_kW_cmd);
    void grid_target_mode(float grid_target_kW_cmd);
    void absolute_ess(bool chg_dischg_flag, float absolute_ess_kW_cmd);
    void ess_calibration_mode(float ess_calibration_kW_cmd, int num_ess_controllable);
    void dispatch_reactive_power();
    void reactive_setpoint_mode(Slew_Object* reactive_setpoint_slew, float reactive_setpoint_kVAR_cmd);
    void constant_power_factor_mode(float power_factor_setpoint, bool power_factor_direction);
    void active_voltage_mode(float deadband, float cmd, float actual_volts, float droop_percent, float rated_kVAR);
    
    //accessor functions
    float get_site_kW_demand();
    float get_site_kW_load();
    bool get_site_kW_load_inclusion();
    load_compensation get_load_method();
    float get_additional_load_compensation();
    float get_feature_kW_demand();
    float get_site_kW_charge_production();
    float get_site_kW_discharge_production();
    float get_site_kVAR_demand();
    float get_ess_kW_request();
    float get_gen_kW_request();
    float get_solar_kW_request();

    void set_site_kW_demand(float value);
    void preserve_uncorrected_site_kW_demand();
    void set_site_kW_load(float value);
    void set_additional_load_compensation(float value); // test endpoint only
    void create_site_kW_load_buffer(int size);
    void set_load_compensation_method(load_compensation method);
    void set_feature_kW_demand(float value);
    void set_site_kW_charge_production(float value);
    void set_site_kW_discharge_production(float value);
    void set_site_kVAR_demand(float value);
    void set_ess_kW_request(float value);
    void set_gen_kW_request(float value);
    void set_solar_kW_request(float value);

    //Asset_Dmd_Data accessor functions
    float get_limited_ess_kW_cmd();
    float get_limited_feeder_kW_cmd();
    float get_limited_gen_kW_cmd();
    float get_limited_solar_kW_cmd();
    float get_ess_kW_cmd();
    float get_feeder_kW_cmd();
    float get_gen_kW_cmd();
    float get_solar_kW_cmd();
    float get_ess_actual_kW();
    float get_feeder_actual_kW();
    float get_gen_actual_kW();
    float get_solar_actual_kW();
    float get_ess_uncontrollable_kW();
    float get_gen_uncontrollable_kW();
    float get_solar_uncontrollable_kW();
    float get_total_uncontrollable_kW();
    float get_ess_max_potential_kW();
    float get_feeder_max_potential_kW();
    float get_gen_max_potential_kW();
    float get_solar_max_potential_kW();
    float get_ess_min_potential_kW();
    float get_feeder_min_potential_kW();
    float get_gen_min_potential_kW();
    float get_solar_min_potential_kW();
    float get_ess_kVAR_cmd();
    float get_gen_kVAR_cmd();
    float get_solar_kVAR_cmd();
    bool get_ess_start_first_flag();
    bool get_gen_start_first_flag();
    bool get_solar_start_first_flag();
    float get_total_available_charge_kW();
    float get_total_available_discharge_kW();
    float get_total_available_charge_kVAR();
    float get_total_available_discharge_kVAR();

    void set_ess_kW_cmd(float value);
    void set_feeder_kW_cmd(float value);
    void set_gen_kW_cmd(float value);
    void set_solar_kW_cmd(float value);
    void set_ess_actual_kW(float value);
    void set_feeder_actual_kW(float value);
    void set_gen_actual_kW(float value);
    void set_solar_actual_kW(float value);
    void set_ess_uncontrollable_kW(float value);
    void set_gen_uncontrollable_kW(float value);
    void set_solar_uncontrollable_kW(float value);
    void set_ess_actual_kVAR(float value);
    void set_gen_actual_kVAR(float value);
    void set_solar_actual_kVAR(float value);
    void set_ess_max_potential_kW(float value);
    void set_feeder_max_potential_kW(float value);
    void set_gen_max_potential_kW(float value);
    void set_solar_max_potential_kW(float value);
    void set_ess_min_potential_kW(float value);
    void set_feeder_min_potential_kW(float value);
    void set_gen_min_potential_kW(float value);
    void set_solar_min_potential_kW(float value);
    void set_ess_kVAR_cmd(float value);
    void set_gen_kVAR_cmd(float value);
    void set_solar_kVAR_cmd(float value);
    void set_ess_potential_kVAR(float value);
    void set_gen_potential_kVAR(float value);
    void set_solar_potential_kVAR(float value);
    void set_ess_start_first_kW(float value);
    void set_gen_start_first_kW(float value);
    void set_solar_start_first_kW(float value);
    void set_ess_start_first_flag(bool value);
    void set_gen_start_first_flag(bool value);
    void set_solar_start_first_flag(bool value);

    void save_state();
    void restore_state();
    void reset_kW_dispatch();
    void reset_kVAR_dispatch();

    private:

    Asset_Cmd_Data ess_data, feeder_data, gen_data, solar_data;
    std::vector<Asset_Cmd_Data*> list_assets_by_discharge_priority(int asset_priority, std::vector<Asset_Cmd_Data*> exclusions = {});
    void remove_asset_types(std::vector<Asset_Cmd_Data*> &data_list, const std::vector<Asset_Cmd_Data*> asset_types);
    float aggregate_unused_kW(std::vector<Asset_Cmd_Data*> const &data_list);

    float site_kW_load;                 // Rolling average calculation of site load. + means power flowing INTO site. - means power flowing OUT of site.
    std::vector<float> load_buffer;     // Circular buffer for tracking configurable duration of load
    int load_iterator;                  // Position in the load circular buffer
    load_compensation load_method;      // Whether the feature production is accounting for load
    float additional_load_compensation; // Tracks load compensation not included in initial asset requests or feature demand
    float feature_kW_demand;            // Demand commanded from the active power feature. + means power flowing OUT of site. - means power flowing INTO site.
    float site_kW_demand;               // Demand after standalone feature modifications
    float prev_site_kW_demand;          // Last unique site demand
    float uncorrected_site_kW_demand;   // Preserved reference to the demand prior to any POI corrections (CLC, watt-watt)
    float site_kVAR_demand;             // Demand kVAR for assets
    float total_potential_kVAR;
    float site_kW_charge_production;    // Charge up to this amount during dispatch
    float site_kW_discharge_production; // Discharge up to this amount during dispatch
};

#endif /* INCLUDE_ASSET_CMD_OBJECT_H_ */
