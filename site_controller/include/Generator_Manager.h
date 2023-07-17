/**
 * Generator_Manager.h
 * Header for Generator-specific Manager class
 * Refactored from Asset_Manager.h
 * 
 * Created on Sep 30th, 2020
 *      Author: Jack Shade (jnshade)
 */

#ifndef GENERATOR_MANAGER_H_
#define GENERATOR_MANAGER_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
#include <list>
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Asset_Generator.h>
#include <Types.h>
#include <macros.h>
#include <Type_Manager.h>

class Generator_Manager: public Type_Manager
{
protected:
    int numGenControllable;

    // gen power aggregates
    float genTotalActivePowerkW;
    float genTotalReactivePowerkVAR;
    float genTotalApparentPowerkVA;
    float genTotalUncontrollableActivePowerkW;

    float genTotalMaxPotentialActivePower;
    float genTotalMinPotentialActivePower;
    float genTotalPotentialReactivePower;

    float genTotalRatedActivePower;
    float total_kW_discharge_limit;

    float genTotalNameplateActivePower;

    float genTargetActivePowerkW;
    float genTargetReactivePowerkVAR;

    std::vector<Asset_Generator*> pGens;

    LDSS ldss;

public:
    Generator_Manager();
    virtual ~Generator_Manager();

    // Internal configuration functions
    void configure_base_class_list(void) override;
    bool configure_type_manager(Type_Configurator* configurator) override;
    Asset* build_new_asset(void) override;
    void append_new_asset(Asset*) override;

    int get_num_gens_stopped(void);
    int get_num_gen_controllable(void);
    void set_min_generators_active(int);

    float get_gen_total_active_power(void);
    float get_gen_total_reactive_power(void);
    float get_gen_total_uncontrollable_active_power(void);
    float get_gen_total_max_potential_active_power(void);
    float get_gen_total_min_potential_active_power(void);
    float get_gen_total_potential_reactive_power(void);
    float get_gen_total_nameplate_active_power(void);

    float get_gen_total_rated_active_power(void);
    float get_total_kW_discharge_limit(void);

    void set_all_gen_grid_form(void);
    void set_all_gen_grid_follow(void);

    void update_ldss_settings(LDSS_Settings &settings);

    bool direct_start_gen(void);
    void start_all_gen(void);
    bool stop_all_gen(void);

    bool aggregate_gen_data(void);

    void generate_asset_type_summary_json(fmt::memory_buffer &buf, const char* const var = NULL) override;

    bool calculate_gen_power(void);

    void process_asset_data(void);
    void update_asset_data(void);

    bool set_gen_target_active_power(float);
    bool set_gen_target_reactive_power(float);

    void start_first_gen(bool enable);

    void set_first_gen_is_starting_flag(bool locked);
    void make_gen_highest_start_priority(Asset_Generator*);
    void make_gen_highest_stop_priority(Asset_Generator*);
    void enable_ldss(bool flag);

    void handle_faulted_gens(void);
};

#endif /* GENERATOR_MANAGER_H_ */
