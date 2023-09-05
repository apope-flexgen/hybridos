/**
 * Feeder_Manager.h
 * Header for Feeder-specific Manager class
 * Refactored from Asset_Manager.h
 *
 * Created on Sep 30th, 2020
 *      Author: Jack Shade (jnshade)
 */

#ifndef FEEDER_MANAGER_H_
#define FEEDER_MANAGER_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Asset_Feeder.h>
#include <Types.h>
#include <macros.h>
#include <Type_Manager.h>

class Feeder_Manager : public Type_Manager {
protected:
    float sync_frequency_offset;

    Asset_Feeder* pPointOfInterConnect;
    Asset_Feeder* pSyncFeeder;

    std::vector<Asset_Feeder*> pFeeder;

public:
    Feeder_Manager();
    virtual ~Feeder_Manager();

    // Internal configuration functions
    void configure_base_class_list(void) override;
    bool configure_type_manager(Type_Configurator* configurator) override;
    Asset* build_new_asset(void) override;
    void append_new_asset(Asset*) override;

    bool get_poi_feeder_state(void);
    bool set_poi_feeder_state_open();
    bool set_poi_feeder_state_closed();
    bool get_poi_feeder_close_permissive_state(void);

    float get_poi_gridside_frequency(void);
    float get_poi_gridside_avg_voltage(void);
    float get_poi_power_factor();

    bool get_sync_feeder_status(void);
    float get_sync_feeder_gridside_frequency(void);
    float get_sync_frequency_offset(void);
    float get_sync_feeder_gridside_avg_voltage(void);
    bool set_sync_feeder_close_permissive_remove(void);
    bool set_sync_feeder_close_permissive(void);

    Asset_Feeder* validate_feeder_id(const char* feeder);
    bool get_feeder_state(Asset_Feeder* feeder);
    bool get_utility_status(Asset_Feeder* feeder);
    float get_feeder_active_power(const char* feeder);
    float get_feeder_reactive_power(const char* feeder);
    float get_feeder_nameplate_active_power(const char* feeder);

    float get_avg_ac_voltage(const char* feeder);

    bool set_feeder_state_open(Asset_Feeder* feeder);
    bool set_feeder_state_closed(Asset_Feeder* feeder);

    void set_poi_target_active_power(float);

    bool aggregate_feeder_data(void);

    void generate_asset_type_summary_json(fmt::memory_buffer& buf, const char* const var = NULL) override;

    void process_asset_data();
    // No feeder data in update_asset_data()

    const std::string get_poi_id(void);
    // float get_poi_potential_active_power(void);
    float get_poi_max_potential_active_power(void);
    float get_poi_min_potential_active_power(void);
    float get_poi_rated_kW(void);
};

#endif /* FEEDER_MANAGER_H_ */
