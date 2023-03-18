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

class Asset_Cmd_Object
{
    public:

    struct Asset_Cmd_Data
    {
        Asset_Cmd_Data();

        float calculate_source_kW_contribution(float cmd);
        float calculate_unused_kW();

        float kW_request;
        float kW_cmd;
        float actual_kW;
        float start_first_kW;
        bool start_first_flag;

        float min_potential_kW;  //minimum possible instantaneous active power available from asset
        float max_potential_kW;  //maximum possible instantaneous active power available from asset

        float kVAR_cmd;
        float actual_kVAR;
        float potential_kVAR;
    };

    Asset_Cmd_Object();
    ~Asset_Cmd_Object();

    void calculate_site_kW_load();
    void calculate_feature_kW_demand(int asset_priority);
    void calculate_total_potential_kVAR();
    float calculate_internal_load_compensation(int asset_priority);
    float calculate_internal_charge_compensation(int asset_priority);
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
    void active_voltage_mode(float deadband, float cmd, float actual_volts, float droop_percent, float rated_kVAR);
    
    //accessor functions
    float get_site_kW_demand();
    float get_site_kW_load();
    bool get_site_kW_load_inclusion();
    float get_feature_kW_demand();
    float get_site_kW_charge_production();
    float get_site_kW_discharge_production();
    float get_site_kVAR_demand();
    float get_ess_kW_request();
    float get_gen_kW_request();
    float get_solar_kW_request();

    void set_site_kW_demand(float value);
    void set_site_kW_load(float value);
    void create_site_kW_load_buffer(int size);
    void set_site_kW_load_inclusion(bool value);
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

    void reset_kW_cmd();
    void reset_kVAR_cmd();

    private:

    Asset_Cmd_Data ess_data, feeder_data, gen_data, solar_data;

    std::vector<Asset_Cmd_Data*> list_assets_by_discharge_priority(int asset_priority, std::vector<Asset_Cmd_Data*> exclusions = {});
    void remove_asset_types(std::vector<Asset_Cmd_Data*> &data_list, const std::vector<Asset_Cmd_Data*> asset_types);
    float aggregate_unused_kW(std::vector<Asset_Cmd_Data*> const &data_list);

    float site_kW_load;                 // Rolling average calculation of site load. + means power flowing INTO site. - means power flowing OUT of site.
    std::vector<float> load_buffer;     // Circular buffer for tracking configurable duration of load
    int load_iterator;                  // Position in the load circular buffer
    bool site_kW_load_inclusion;        // Whether the feature production is accounting for load
    float feature_kW_demand;            // Demand commanded from the active power feature. + means power flowing OUT of site. - means power flowing INTO site.
    float site_kW_demand;               // Demand after standalone feature modifications
    float prev_site_kW_demand;          // Last unique site demand
    float site_kVAR_demand;             // Demand kVAR for assets
    float total_potential_kVAR;
    float site_kW_charge_production;    // Charge up to this amount during dispatch
    float site_kW_discharge_production; // Discharge up to this amount during dispatch
};

#endif /* INCLUDE_ASSET_CMD_OBJECT_H_ */
