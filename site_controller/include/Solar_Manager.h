/**
 * Solar_Manager.h
 * Header for Solar-specific Manager class
 * Refactored from Asset_Manager.h
 * 
 * Created on Sep 30th, 2020
 *      Author: Jack Shade (jnshade)
 */

#ifndef SOLAR_MANAGER_H_
#define SOLAR_MANAGER_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Asset_Solar.h>
#include <Type_Manager.h>

enum solar_curtailment_states
{
    no_curtailment, partial_curtailment, full_curtailment
};
const char curtailment_strings[][20] = { "No Curtailment", "Partial Curtailment", "Full Curtailment"};

class Solar_Manager: public Type_Manager
{
protected:
    int numSolarControllable;
    int numSolarStartable;
    int numSolarStandby;

    // solar power aggregates
    float lastTotalActivePowerkW;
    float solarTotalActivePowerkW;
    float solarTotalReactivePowerkVAR;
    float solarTotalApparentPowerkVA;
    float solarTotalUncontrollableActivePowerkW;

    float solarTotalMaxPotentialActivePower;
    float solarTotalMinPotentialActivePower;
    float solarTotalPotentialReactivePower;
    float solarTotalPotentialApparentPower;

    float solarTotalRatedActivePower;
    float solarTotalRatedReactivePower;
    float solarTotalRatedApparentPower;
    float total_kW_discharge_limit;

    float solarTotalNameplateActivePower;
    float solarTotalNameplateReactivePower;
    float solarTotalNameplateApparentPower;

    float solarTargetActivePowerkW;
    float solarTargetReactivePowerkVAR;

    bool firstAssetStartSolar;

    std::vector<Asset_Solar*> pSolar;

    bool solar_curtailment_enabled;
    solar_curtailment_states solar_curtailment_state;
    int solar_prime_index;
    float curtailment_deadband_percentage;

    // Internal functions
    void calculate_solar_target_active_power(void);
    void calculate_solar_target_reactive_power(void);
    std::tuple<float, float> calculate_solar_reactive_power_commands(void);
    void calculate_solar_total_potential_active_power(void);
    int solar_prime_inverter_selection(void);
    solar_curtailment_states find_next_curtailment_state(void);
    bool aggregate_solar_data(void);
    void generate_asset_type_summary_json(fmt::memory_buffer &buf, const char* const var = NULL) override;

public:
    Solar_Manager();
    virtual ~Solar_Manager();

    // Configuration functions
    void configure_base_class_list(void) override;
    bool configure_type_manager(Type_Configurator* configurator) override;
    Asset* build_new_asset(void) override;
    void append_new_asset(Asset*) override;

    int get_num_solar_startable(void);
    int get_num_solar_controllable(void);
    int get_num_solar_in_standby(void);

    float get_solar_total_active_power(void);
    float get_solar_total_reactive_power(void);
    float get_solar_total_uncontrollable_active_power(void);

    float get_solar_total_max_potential_active_power(void);
    float get_solar_total_min_potential_active_power(void);
    float get_solar_total_potential_reactive_power(void);

    float get_solar_total_rated_active_power(void);
    float get_solar_total_rated_reactive_power(void);
    float get_solar_total_rated_apparent_power(void);
    float get_total_kW_discharge_limit(void);

    float get_solar_total_nameplate_active_power(void);
    float get_solar_total_nameplate_reactive_power(void);
    float get_solar_total_nameplate_apparent_power(void);

    bool enter_standby_all_solar(void);
    bool exit_standby_all_solar(void);
    bool start_all_solar(void);
    bool stop_all_solar(void);

    void set_solar_target_active_power(float);
    void set_solar_target_reactive_power(float);
    void set_solar_reactive_kvar_mode(int index);
    void set_solar_pwr_factor_mode(int index);
    void set_solar_target_power_factor(float);

    void process_asset_data(void);
    void update_asset_data(void);

    void set_solar_clear_faults(void);

    void start_first_solar(bool enable);

    void set_solar_curtailment_enabled(bool enable);
};

#endif /* SOLAR_MANAGER_H_ */
