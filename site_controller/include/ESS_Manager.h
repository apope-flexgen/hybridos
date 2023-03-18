/**
 * ESS_Manager.h
 * Header for ESS-specific Manager class
 * Refactored from Asset_Manager.h
 * 
 * Created on Sep 30th, 2020
 *      Author: Jack Shade (jnshade)
 */

#ifndef ESS_MANAGER_H_
#define ESS_MANAGER_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Asset_ESS.h>
#include <Types.h>
#include <macros.h>
#include <Type_Manager.h>

enum SOC_State
{
    OnTarget, BelowTarget, FarBelowTarget, AboveTarget, FarAboveTarget
};

struct SOC_Balancing_Data {
    Asset_ESS* ess;
    float contributionFactor;
    float powerLimit;
};

class ESS_Manager: public Type_Manager
{
protected:
    int numEssControllable;
    int numEssStartable;
    int numEssStandby;

    // ess power aggregates
    float essTotalActivePowerkW;
    float essTotalReactivePowerkVAR;
    float essTotalApparentPowerkVA;
    float essTotalUncontrollableActivePowerkW;

    float essTotalMaxPotentialActivePower;
    float essTotalMinPotentialActivePower;
    float essTotalPotentialReactivePower;

    float essTotalRatedActivePower;
    float essTotalRatedReactivePower;
    float essTotalRatedApparentPower;
    float total_kW_charge_limit;
    float total_kW_discharge_limit;

    float essTotalNameplateActivePower;
    float essTotalNameplateReactivePower;
    float essTotalNameplateApparentPower;

    float essTargetActivePowerkW;
    float essTargetReactivePowerkVAR;

    float controllableEssMinSoc;
    float controllableEssAvgSoc;
    float controllableEssMaxSoc;

    float parsedEssMinSoc;
    float parsedEssAvgSoc;
    float parsedEssMaxSoc;

    float socBalancingFactor;
    bool balancePowerDistribution = true;

    float entering_soc_range;
    float leaving_soc_range;
    float far_soc_range;
    float charge_control_divisor;

    bool firstAssetStartEss;

    std::vector<Asset_ESS*> pEss;

    float essTotalChargeableEnergykWh;
    float essTotalChargeablePowerkW;
    float essTotalDischargeableEnergykWh;
    float essTotalDischargeablePowerkW;

    float grid_forming_voltage_slew;

    SOC_State soc_status;

public:
    ESS_Manager();
    virtual ~ESS_Manager();

    // Internal configuration functions
    void configure_base_class_list(void) override;
    bool configure_type_manager(Type_Configurator* configurator) override;
    Asset* build_new_asset(void) override;
    void append_new_asset(Asset*) override;

    int get_num_ess_startable(void);
    int get_num_ess_controllable(void);
    int get_num_ess_in_standby(void);

    float get_ess_soc_max(void);
    float get_ess_soc_min(void);
    float get_ess_soc_avg(void);

    float get_all_ess_soc_max(void);
    float get_all_ess_soc_min(void);
    float get_all_ess_soc_avg(void);

    float get_pcs_nominal_voltage_setting(void);
    bool set_pcs_nominal_voltage_setting(float mPcsNominalVoltageSetting);

    float get_ess_total_active_power(void);
    float get_ess_total_reactive_power(void);
    float get_ess_total_uncontrollable_active_power(void);

    float get_ess_total_max_potential_active_power(void);
    float get_ess_total_min_potential_active_power(void);
    float get_ess_total_potential_reactive_power(void);

    float get_ess_total_rated_active_power(void);
    float get_ess_total_rated_reactive_power(void);
    float get_ess_total_rated_apparent_power(void);

    float get_ess_total_nameplate_active_power(void);
    float get_ess_total_nameplate_reactive_power(void);
    float get_ess_total_nameplate_apparent_power(void);

    float get_soc_balancing_factor(void);

    float get_total_chargeable_power_kW(void);
    float get_total_dischargeable_power_kW(void);
    float get_total_chargeable_energy_kWh(void);
    float get_total_dischargeable_energy_kWh(void);
    float get_total_kW_charge_limit(void);
    float get_total_kW_discharge_limit(void);
    float get_asset_active_power_setpoint(int);
    float get_asset_reactive_power_setpoint(int);
    std::vector<int> get_setpoint_statuses(void);

    bool set_ess_target_active_power(float);
    bool set_ess_target_reactive_power(float);

    void set_ess_voltage_setpoint(float setpoint);
    void set_ess_frequency_setpoint(float setpoint);
    void set_all_ess_calibration_vars(ESS_Calibration_Settings settings);

    void set_all_ess_grid_form(void);
    void set_all_ess_grid_follow(void);

    bool enter_standby_all_ess(void);
    bool exit_standby_all_ess(void);
    bool start_all_ess(void);
    bool stop_all_ess(void);

    bool close_all_bms_contactors();
    bool open_all_bms_contactors();

    void set_ess_reactive_kvar_mode(void);
    void set_ess_pwr_factor_mode(void);
    float get_grid_forming_voltage_slew(void);
    bool in_maint_mode(int index);

    void set_ess_target_power_factor(float pwr_factor);
    void set_grid_forming_voltage_slew(float slope);

    bool aggregate_ess_data(void);

    void generate_asset_type_summary_json(fmt::memory_buffer &buf, const char* const var = NULL) override;

    void calculate_ess_reactive_power(void);
    bool calculate_ess_active_power(void);
    float charge_control(float target_soc_desired, bool charge_disable, bool discharge_disable);

    void process_asset_data(void);
    void update_asset_data(void);

    void set_ess_clear_faults(void);

    // Friend classes for testing
    friend class Site_Manager_Mock;
};

#endif /* ESS_MANAGER_H_ */
