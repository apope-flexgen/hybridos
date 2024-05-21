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
#include <vector>
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Slew_Object.h>

enum discharge_type {
    LOAD,      // Discharging to account for load
    REQUESTS,  // Discharging to account for specific asset type requests
    DEMAND,    // Discharging to account for remaining demanded discharge
};

// Types of load compensation available
enum load_compensation {
    NO_COMPENSATION,  // No load compensation being provided
    LOAD_OFFSET,      // Always increase discharge to offset load
    LOAD_MINIMUM      // Only increase discharge if load is not met
};

/**
 * An Asset_Cmd_Object is a mutable representation of the site's state, which
 * is modified in calculations to determine what the new state should be.
 */
struct Asset_Cmd_Object {
public:
    struct Asset_Cmd_Data {
        Asset_Cmd_Data();

        float calculate_source_kW_contribution(float cmd);
        float calculate_unused_charge_kW();
        float calculate_unused_dischg_kW();

        float kW_request;        // Power requested of this asset by the active power feature and its standalone modifications
        float saved_kW_request;  // Record of the kW_request before its modification
        float kW_cmd;            // Power commanded of this asset after the active power dispatch step
        float saved_kW_cmd;      // Record of the kW_cmd before its modification
        float actual_kW;         // Actual active power measured by the asset
        float start_first_kW;    // Power above which the asset should be started
        bool start_first_flag;   // Flag indicating whether to enable the start_first_kW check
        bool auto_restart_flag;  // Flag indicating whether to allow asset to start at all

        float min_potential_kW;  // minimum possible instantaneous active power available from asset
        float max_potential_kW;  // maximum possible instantaneous active power available from asset

        float kVAR_cmd;        // Power commanded of this asset after the reactive power dispatch step
        float actual_kVAR;     // Actual reactive power measured by the asset
        float potential_kVAR;  // maximum possible instantaneous reactive power available from the asset
    };

    Asset_Cmd_Object();
    ~Asset_Cmd_Object();

    float calculate_net_poi_export();
    void calculate_site_kW_load();
    void calculate_feature_kW_demand(int asset_priority);
    void calculate_total_potential_kVAR();
    bool determine_ess_load_requirement(int asset_priority);
    float dispatch_site_kW_discharge_cmd(int asset_priority, float cmd, discharge_type command_type);
    float dispatch_site_kW_charge_cmd(int asset_priority, bool solar_source_flag, bool gen_source_flag, bool feeder_source_flag);
    void dispatch_reactive_power();

    bool get_site_kW_load_inclusion();

    void preserve_uncorrected_site_kW_demand();
    void create_site_kW_load_buffer(int size);
    void set_load_compensation_method(load_compensation method);

    // Asset_Cmd_Data accessor functions
    float get_limited_ess_kW_cmd();
    float get_limited_feeder_kW_cmd();
    float get_limited_gen_kW_cmd();
    float get_limited_solar_kW_cmd();
    float get_total_available_charge_kW();
    float get_total_available_discharge_kW();
    float get_total_available_charge_kVAR();
    float get_total_available_discharge_kVAR();

    void save_state();
    void restore_state();
    void reset_kW_dispatch();
    void reset_kVAR_dispatch();

    Asset_Cmd_Data ess_data, feeder_data, gen_data, solar_data;

    float poi_cmd;                       // Command desired at the POI. Used by closed loop control to avoid potential feedback loops with site demand
    float site_kW_load;                  // Rolling average calculation of site load. + means power flowing INTO site. - means power flowing OUT of site.
    std::vector<float> load_buffer;      // Circular buffer for tracking configurable duration of load
    int load_iterator;                   // Position in the load circular buffer
    load_compensation load_method;       // Whether the feature production is accounting for load
    float additional_load_compensation;  // Tracks load compensation not included in initial asset requests or feature demand
    float feature_kW_demand;             // Demand commanded from the active power feature. + means power flowing OUT of site. - means power flowing INTO site.
    float site_kW_demand;                // Demand after standalone feature modifications
    float uncorrected_site_kW_demand;    // Preserved reference to the demand prior to any POI corrections (CLC, watt-watt)
    float site_kVAR_demand;              // Demand kVAR for assets
    float total_potential_kVAR;
    float site_kW_charge_production;     // Charge up to this amount during dispatch
    float site_kW_discharge_production;  // Discharge up to this amount during dispatch

private:
    std::vector<Asset_Cmd_Data*> list_assets_by_discharge_priority(int asset_priority, std::vector<Asset_Cmd_Data*> exclusions = {});
    void remove_asset_types(std::vector<Asset_Cmd_Data*>& data_list, std::vector<Asset_Cmd_Data*> asset_types);
    float aggregate_unused_kW(std::vector<Asset_Cmd_Data*> const& data_list);
};

namespace asset_cmd_utils {
float track_slewed_load(load_compensation load_method, float site_kW_demand, float unslewed_demand, float additional_load_compensation, const Slew_Object& feature_slew);
float calculate_additional_load_compensation(load_compensation load_method, float site_kW_load, float site_kW_demand, float ess_kW_request, float gen_kW_request, float solar_kW_request);

struct site_kW_production_limits {
    float site_kW_charge_production;
    float site_kW_discharge_production;
};
site_kW_production_limits calculate_site_kW_production_limits(float ess_kW_request, float gen_kW_request, float solar_kW_request, load_compensation load_method, float additional_load_compensation, float feature_kW_demand, float site_kW_demand);
}  // namespace asset_cmd_utils

#endif /* INCLUDE_ASSET_CMD_OBJECT_H_ */
