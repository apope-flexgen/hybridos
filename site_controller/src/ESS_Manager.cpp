/**
 * ESS_Manager.cpp
 * Source for ESS-specific Manager class
 * Refactored from Asset_Manager.cpp
 *
 * Created on Sep 30th, 2020
 *      Author: Jack Shade (jnshade)
 */

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <ESS_Manager.h>
#include <Configurator.h>
#include <Site_Controller_Utils.h>
#include "Path.h"

extern fims* p_fims;

/****************************************************************************************/
ESS_Manager::ESS_Manager() : Type_Manager(ESS_TYPE_ID) {
    numEssControllable = 0;
    numEssStartable = 0;
    numEssStandby = 0;

    essTotalActivePowerkW = 0.0;
    essTotalReactivePowerkVAR = 0.0;
    essTotalApparentPowerkVA = 0.0;
    essTotalUncontrollableActivePowerkW = 0.0;

    essTotalMaxPotentialActivePower = 0.0;
    essTotalMinPotentialActivePower = 0.0;
    essTotalPotentialReactivePower = 0.0;

    essTotalRatedActivePower = 0.0;
    essTotalRatedReactivePower = 0.0;
    essTotalRatedApparentPower = 0.0;

    essTotalNameplateActivePower = 0.0;
    essTotalNameplateReactivePower = 0.0;
    essTotalNameplateApparentPower = 0.0;

    essTargetActivePowerkW = 0.0;
    essTargetReactivePowerkVAR = 0.0;

    controllableEssMinSoc = 0.0;
    controllableEssAvgSoc = 0.0;
    controllableEssMaxSoc = 0.0;

    runningEssMinSoc = 0.0;
    runningEssAvgSoc = 0.0;
    runningEssMaxSoc = 0.0;

    parsedEssMinSoc = 0.0;
    parsedEssAvgSoc = 0;
    parsedEssMaxSoc = 0;

    entering_soc_range = 0.0;
    leaving_soc_range = 0.0;
    far_soc_range = 0.0;
    charge_control_divisor = 0.0;

    soc_status = OnTarget;

    essTotalChargeableEnergykWh = 0.0;
    essTotalChargeablePowerkW = 0.0;
    essTotalDischargeableEnergykWh = 0.0;
    essTotalDischargeablePowerkW = 0.0;

    grid_forming_voltage_slew = 0.0;
}

/****************************************************************************************/
ESS_Manager::~ESS_Manager() {
    if (!pEss.empty())
        for (size_t j = 0; j < pEss.size(); j++)
            if (pEss[j] != NULL)
                delete pEss[j];
}

int ESS_Manager::get_num_ess_startable(void) {
    return numEssStartable;
}

int ESS_Manager::get_num_ess_in_standby(void) {
    return numEssStandby;
}

int ESS_Manager::get_num_ess_controllable(void) {
    return numEssControllable;
}

float ESS_Manager::get_ess_total_active_power(void) {
    return essTotalActivePowerkW;
}

float ESS_Manager::get_ess_total_reactive_power(void) {
    return (essTotalReactivePowerkVAR);
}

float ESS_Manager::get_ess_total_uncontrollable_active_power(void) {
    return essTotalUncontrollableActivePowerkW;
}

float ESS_Manager::get_ess_total_max_potential_active_power(void) {
    return essTotalMaxPotentialActivePower;
}

float ESS_Manager::get_ess_total_min_potential_active_power(void) {
    return essTotalMinPotentialActivePower;
}

float ESS_Manager::get_ess_total_potential_reactive_power(void) {
    return essTotalPotentialReactivePower;
}

float ESS_Manager::get_ess_total_rated_active_power(void) {
    return essTotalRatedActivePower;
}

float ESS_Manager::get_ess_total_rated_reactive_power(void) {
    return essTotalRatedReactivePower;
}

float ESS_Manager::get_ess_total_rated_apparent_power(void) {
    return essTotalRatedApparentPower;
}

float ESS_Manager::get_total_kW_charge_limit(void) {
    return total_kW_charge_limit;
}

float ESS_Manager::get_total_kW_discharge_limit(void) {
    return total_kW_discharge_limit;
}

float ESS_Manager::get_total_chargeable_power_kW(void) {
    return essTotalChargeablePowerkW;
}

float ESS_Manager::get_total_dischargeable_power_kW(void) {
    return essTotalDischargeablePowerkW;
}

float ESS_Manager::get_total_chargeable_energy_kWh(void) {
    return essTotalChargeableEnergykWh;
}

float ESS_Manager::get_total_dischargeable_energy_kWh(void) {
    return essTotalDischargeableEnergykWh;
}

float ESS_Manager::get_asset_active_power_setpoint(int essIndex) {
    return pEss[essIndex]->get_active_power_setpoint_control();
}

float ESS_Manager::get_asset_reactive_power_setpoint(int essIndex) {
    return pEss[essIndex]->get_reactive_power_setpoint_control();
}

float ESS_Manager::get_running_ess_soc_max(void) {
    return runningEssMaxSoc;
}

float ESS_Manager::get_running_ess_soc_min(void) {
    return runningEssMinSoc;
}

float ESS_Manager::get_running_ess_soc_avg(void) {
    return runningEssAvgSoc;
}

float ESS_Manager::get_controllable_ess_soc_min(void) {
    return controllableEssMinSoc;
}

float ESS_Manager::get_controllable_ess_soc_max(void) {
    return controllableEssMaxSoc;
}

float ESS_Manager::get_controllable_ess_soc_avg(void) {
    return controllableEssAvgSoc;
}

float ESS_Manager::get_all_ess_soc_max(void) {
    return parsedEssMaxSoc;
}

float ESS_Manager::get_all_ess_soc_min(void) {
    return parsedEssMinSoc;
}

float ESS_Manager::get_all_ess_soc_avg(void) {
    return parsedEssAvgSoc;
}

float ESS_Manager::get_ess_total_nameplate_active_power(void) {
    return essTotalNameplateActivePower;
}

float ESS_Manager::get_ess_total_nameplate_reactive_power(void) {
    return essTotalNameplateReactivePower;
}

float ESS_Manager::get_ess_total_nameplate_apparent_power(void) {
    return essTotalNameplateApparentPower;
}

float ESS_Manager::get_soc_balancing_factor(void) {
    return socBalancingFactor;
}

std::vector<int> ESS_Manager::get_setpoint_statuses(void) {
    std::vector<int> setpoint_statuses(3);
    for (auto ess : pEss)
        setpoint_statuses[ess->get_setpoint_status()]++;
    return setpoint_statuses;
}

float ESS_Manager::get_pcs_nominal_voltage_setting(void) {
    return pEss[0]->get_pcs_nominal_voltage_setting();
}

bool ESS_Manager::set_pcs_nominal_voltage_setting(float mPcsNominalVoltageSetting) {
    FPS_DEBUG_LOG("Setting PCS nominal voltage setting %f\n", mPcsNominalVoltageSetting);

    for (int i = 0; i < numParsed; i++)
        pEss[i]->set_pcs_nominal_voltage_setting(mPcsNominalVoltageSetting);
    return true;
}

void ESS_Manager::set_all_ess_grid_form(void) {
    for (int i = 0; i < numParsed; i++)
        pEss[i]->set_grid_mode(FORMING);
}

void ESS_Manager::set_all_ess_grid_follow(void) {
    for (int i = 0; i < numParsed; i++)
        pEss[i]->set_grid_mode(FOLLOWING);
}

bool ESS_Manager::enter_standby_all_ess(void) {
    for (int i = 0; i < numParsed; i++) {
        if (pEss[i]->is_controllable())
            pEss[i]->enter_standby();
    }
    return true;
}

bool ESS_Manager::exit_standby_all_ess(void) {
    for (int i = 0; i < numParsed; i++) {
        if (pEss[i]->in_standby() && pEss[i]->is_available())
            pEss[i]->exit_standby();
    }
    return true;
}

bool ESS_Manager::start_all_ess(void) {
    FPS_DEBUG_LOG("\nESS_Manager::start_all_ess()\n");
    for (int i = 0; i < numParsed; i++) {
        if (pEss[i]->is_available() && !pEss[i]->is_running())
            pEss[i]->start();
    }
    return true;
}

bool ESS_Manager::stop_all_ess(void) {
    FPS_DEBUG_LOG("\nESS_Manager::stop_all_ess()\n");
    for (int i = 0; i < numParsed; i++) {
        if (pEss[i]->is_controllable())
            pEss[i]->stop();
    }
    return true;
}

bool ESS_Manager::close_all_bms_contactors() {
    for (int i = 0; i < numParsed; i++) {
        if (!pEss[i]->is_running() && !pEss[i]->in_maint_mode())
            pEss[i]->close_bms_contactors();
    }
    return true;
}

bool ESS_Manager::open_all_bms_contactors() {
    for (int i = 0; i < numParsed; i++) {
        if (!pEss[i]->is_running() && !pEss[i]->in_maint_mode())
            pEss[i]->open_bms_contactors();
    }
    return true;
}

bool ESS_Manager::set_ess_target_active_power(float desiredkW) {
    essTargetActivePowerkW = desiredkW;
    return true;
}

bool ESS_Manager::set_ess_target_reactive_power(float desiredkW) {
    essTargetReactivePowerkVAR = desiredkW;
    return true;
}

void ESS_Manager::set_ess_voltage_setpoint(float setpoint) {
    for (int i = 0; i < numParsed; i++) {
        if (!pEss[i]->in_maint_mode())
            pEss[i]->set_voltage_setpoint(setpoint);
    }
}

void ESS_Manager::set_ess_frequency_setpoint(float setpoint) {
    for (int i = 0; i < numParsed; i++) {
        if (!pEss[i]->in_maint_mode())
            pEss[i]->set_frequency_setpoint(setpoint);
    }
}

/**
 * @brief Set the variables used for ESS Calibration mode passed as a struct
 *
 * @param settings Struct containing all Calibration settings
 */
void ESS_Manager::set_all_ess_calibration_vars(ESS_Calibration_Settings settings) {
    socBalancingFactor = settings.balancing_factor;
    balancePowerDistribution = settings.power_dist_flag;

    for (auto ess : pEss)
        ess->set_calibration_vars(settings);
}

void ESS_Manager::set_ess_reactive_kvar_mode(void) {
    for (int i = 0; i < numParsed; i++) {
        if (!pEss[i]->in_maint_mode())
            pEss[i]->set_power_mode(REACTIVEPWR);
    }
}

void ESS_Manager::set_ess_pwr_factor_mode(void) {
    for (int i = 0; i < numParsed; i++) {
        if (!pEss[i]->in_maint_mode())
            pEss[i]->set_power_mode(PWRFCTR);
    }
}
/**
 * New function to return the maintenance mode status
 * @param index The index of the instance in pEss
 */
bool ESS_Manager::in_maint_mode(int index) {
    // Range check for mismatched number of assets across types
    if (index < 0 || index >= numParsed)
        return false;

    return pEss[index]->in_maint_mode();
}

void ESS_Manager::set_ess_target_power_factor(float pwr_factor) {
    for (int i = 0; i < numParsed; i++)
        pEss[i]->set_power_factor_setpoint(pwr_factor);
}

float ESS_Manager::get_grid_forming_voltage_slew(void) {
    return grid_forming_voltage_slew;
}

void ESS_Manager::set_grid_forming_voltage_slew(float slope) {
    for (int i = 0; i < numParsed; i++) {
        pEss[i]->set_voltage_slew_setpoint(slope);
    }
}

bool ESS_Manager::aggregate_ess_data(void) {
    // FPS_DEBUG_LOG("Processing ESS data; ESS_Manager::aggregate_ess_data\n");

    // clear member variables
    numAvail = 0;
    numRunning = 0;
    numEssControllable = 0;
    numEssStartable = 0;
    numEssStandby = 0;
    num_in_local_mode = 0;

    parsedEssMinSoc = 101;
    controllableEssMinSoc = 101;
    runningEssMinSoc = 101;
    parsedEssMaxSoc = 0.0;
    controllableEssMaxSoc = 0.0;
    runningEssMaxSoc = 0.0;
    parsedEssAvgSoc = 0.0;
    controllableEssAvgSoc = 0.0;
    runningEssAvgSoc = 0.0;

    essTotalNameplateActivePower = 0.0;
    essTotalNameplateReactivePower = 0.0;
    essTotalNameplateApparentPower = 0.0;

    essTotalActivePowerkW = 0.0;
    essTotalReactivePowerkVAR = 0.0;
    essTotalApparentPowerkVA = 0.0;
    essTotalUncontrollableActivePowerkW = 0.0;

    essTotalChargeablePowerkW = 0.0;
    essTotalDischargeablePowerkW = 0.0;

    essTotalChargeableEnergykWh = 0.0;
    essTotalDischargeableEnergykWh = 0.0;

    essTotalMaxPotentialActivePower = 0.0;
    essTotalMinPotentialActivePower = 0.0;
    essTotalPotentialReactivePower = 0.0;

    essTotalRatedActivePower = 0.0;
    essTotalRatedReactivePower = 0.0;
    essTotalRatedApparentPower = 0.0;

    total_kW_charge_limit = 0.0f;
    total_kW_discharge_limit = 0.0f;

    // aggregate ess
    for (int i = 0; i < numParsed; i++) {
        // TODO: stop this. stop this now.
        if (i == 0)
            grid_forming_voltage_slew = pEss[i]->get_voltage_slew_setpoint();

        parsedEssMinSoc = std::min(parsedEssMinSoc, pEss[i]->get_soc());
        parsedEssMaxSoc = std::max(parsedEssMaxSoc, pEss[i]->get_soc());
        parsedEssAvgSoc += pEss[i]->get_soc();

        // aggregate nameplate values
        // TODO: nameplate power values are based on site configuration, move to static section
        essTotalNameplateActivePower += pEss[i]->get_rated_active_power();
        essTotalNameplateReactivePower += pEss[i]->get_rated_reactive_power();
        essTotalNameplateApparentPower += pEss[i]->get_rated_apparent_power();

        if (pEss[i]->is_available())
            numAvail++;

        if (pEss[i]->is_running()) {
            numRunning++;

            runningEssMinSoc = std::min(runningEssMinSoc, pEss[i]->get_soc());
            runningEssMaxSoc = std::max(runningEssMaxSoc, pEss[i]->get_soc());
            runningEssAvgSoc += pEss[i]->get_soc();
        }

        if (pEss[i]->is_available() && !pEss[i]->is_running())
            numEssStartable++;

        if (pEss[i]->is_available() && pEss[i]->in_standby())
            numEssStandby++;

        if (pEss[i]->is_controllable()) {
            numEssControllable++;

            controllableEssMinSoc = std::min(controllableEssMinSoc, pEss[i]->get_soc());
            controllableEssMaxSoc = std::max(controllableEssMaxSoc, pEss[i]->get_soc());
            controllableEssAvgSoc += pEss[i]->get_soc();

            essTotalActivePowerkW += pEss[i]->get_active_power();
            essTotalReactivePowerkVAR += pEss[i]->get_reactive_power();
            essTotalApparentPowerkVA += pEss[i]->get_apparent_power();

            essTotalChargeablePowerkW += pEss[i]->get_chargeable_power();
            essTotalDischargeablePowerkW += pEss[i]->get_dischargeable_power();

            essTotalMaxPotentialActivePower += pEss[i]->get_max_potential_active_power();
            essTotalMinPotentialActivePower += pEss[i]->get_min_potential_active_power();

            essTotalPotentialReactivePower += pEss[i]->get_potential_reactive_power();

            essTotalRatedActivePower += pEss[i]->get_rated_active_power();
            essTotalRatedReactivePower += pEss[i]->get_rated_reactive_power();
            essTotalRatedApparentPower += pEss[i]->get_rated_apparent_power();

            essTotalChargeableEnergykWh += pEss[i]->get_chargeable_energy();
            essTotalDischargeableEnergykWh += pEss[i]->get_dischargeable_energy();

            total_kW_charge_limit += pEss[i]->get_min_limited_active_power();
            total_kW_discharge_limit += pEss[i]->get_max_limited_active_power();
        } else {
            // only add to uncontrollable power if ESS is uncontrollable for reasons besides a modbus disconnect
            if (!pEss[i]->get_watchdog_fault()) {
                essTotalUncontrollableActivePowerkW += pEss[i]->get_active_power();
            }
        }

        if (pEss[i]->is_in_local_mode())
            num_in_local_mode++;
    }

    // update ess default soc minimum
    controllableEssMinSoc = (controllableEssMinSoc == 101) ? 0 : controllableEssMinSoc;
    parsedEssMinSoc = (parsedEssMinSoc == 101) ? 0 : parsedEssMinSoc;
    runningEssMinSoc = (runningEssMinSoc == 101) ? 0 : runningEssMinSoc;

    FPS_DEBUG_LOG("ESS_Manager::aggregate_ess_data essTotalActivePowerkW: %f essTotalMaxPotentialActivePower: %f essTotalMinPotentialActivePower: %f\n", essTotalActivePowerkW,
                  essTotalMaxPotentialActivePower, essTotalMinPotentialActivePower);

    // assign all values used by asset manager and asset
    parsedEssAvgSoc = (numParsed != 0) ? (parsedEssAvgSoc / (float)numParsed) : 0;
    controllableEssAvgSoc = (numEssControllable != 0) ? (controllableEssAvgSoc / (float)numEssControllable) : 0;
    runningEssAvgSoc = (numRunning != 0) ? (runningEssAvgSoc / (float)numRunning) : 0;

    return false;
}

void ESS_Manager::generate_asset_type_summary_json(fmt::memory_buffer& buf, const char* const var) {
    if (var == NULL) {
        bufJSON_StartObject(buf);  // summary {
    }

    bufJSON_AddStringCheckVar(buf, "name", "ESS Summary", var);
    bufJSON_AddNumberCheckVar(buf, "num_ess_available", numAvail, var);
    bufJSON_AddNumberCheckVar(buf, "num_ess_running", numRunning, var);
    bufJSON_AddNumberCheckVar(buf, "num_ess_controllable", numEssControllable, var);
    bufJSON_AddNumberCheckVar(buf, "num_ess_in_local_mode", num_in_local_mode, var);
    bufJSON_AddNumberCheckVar(buf, "ess_total_active_power", essTotalActivePowerkW, var);
    bufJSON_AddNumberCheckVar(buf, "ess_total_reactive_power", essTotalReactivePowerkVAR, var);
    bufJSON_AddNumberCheckVar(buf, "ess_total_apparent_power", essTotalApparentPowerkVA, var);
    bufJSON_AddNumberCheckVar(buf, "ess_chargeable_power", essTotalChargeablePowerkW, var);
    bufJSON_AddNumberCheckVar(buf, "ess_dischargeable_power", essTotalDischargeablePowerkW, var);
    bufJSON_AddNumberCheckVar(buf, "ess_chargeable_energy", essTotalChargeableEnergykWh, var);
    bufJSON_AddNumberCheckVar(buf, "ess_dischargeable_energy", essTotalDischargeableEnergykWh, var);
    bufJSON_AddNumberCheckVar(buf, "grid_forming_voltage_slew", grid_forming_voltage_slew, var);
    bufJSON_AddNumberCheckVar(buf, "ess_num_alarmed", get_num_alarmed(), var);
    bufJSON_AddNumberCheckVar(buf, "ess_num_faulted", get_num_faulted(), var);

    // Marked for deprecation
    bufJSON_AddNumberCheckVar(buf, "ess_total_alarms", get_num_active_alarms(), var);
    bufJSON_AddNumberCheckVar(buf, "ess_total_faults", get_num_active_faults(), var);

    // SOC variables added
    bufJSON_AddNumberCheckVar(buf, "soc_min_all", get_all_ess_soc_min(), var);
    bufJSON_AddNumberCheckVar(buf, "soc_max_all", get_all_ess_soc_max(), var);
    bufJSON_AddNumberCheckVar(buf, "soc_avg_all", get_all_ess_soc_avg(), var);
    bufJSON_AddNumberCheckVar(buf, "soc_min_running", get_running_ess_soc_min(), var);
    bufJSON_AddNumberCheckVar(buf, "soc_max_running", get_running_ess_soc_max(), var);
    bufJSON_AddNumberCheckVar(buf, "soc_avg_running", get_running_ess_soc_avg(), var);

    // controllable SOC variables added
    bufJSON_AddNumberCheckVar(buf, "soc_min_controllable", get_controllable_ess_soc_min(), var);
    bufJSON_AddNumberCheckVar(buf, "soc_max_controllable", get_controllable_ess_soc_max(), var);
    bufJSON_AddNumberCheckVar(buf, "soc_avg_controllable", get_controllable_ess_soc_avg(), var);

    if (var == NULL) {
        bufJSON_EndObjectNoComma(buf);  // } summary
    }
}

/*
 * @brief Set all ESS reactive_power_setpoint 0. Will only apply to those controllable.
 */
void ESS_Manager::zero_all_controllable_ess_reactive_power(void) {
    for (int i = 0; i < numParsed; i++) {
        if (pEss[i]->is_controllable() && pEss[i]->get_reactive_power_setpoint() != 0) {
            pEss[i]->set_reactive_power_setpoint(0);
        }
    }
}

/*
 * @brief Set all ESS reactive_power_setpoint to the maximum possible output. Will only apply to those controllable.
 */
void ESS_Manager::all_controllable_ess_max_reactive_power(void) {
    for (int i = 0; i < numParsed; i++) {
        if (pEss[i]->is_controllable() && pEss[i]->get_reactive_power_setpoint() != pEss[i]->get_rated_reactive_power()) {
            pEss[i]->set_reactive_power_setpoint(pEss[i]->get_rated_reactive_power());
        }
    }
}

/*
 * @brief Set all ESS reactive_power_setpoint to the minimum possible output. Will only apply to those controllable.
 */
void ESS_Manager::all_controllable_ess_min_reactive_power(void) {
    for (int i = 0; i < numParsed; i++) {
        if (pEss[i]->is_controllable() && pEss[i]->get_reactive_power_setpoint() != -1 * pEss[i]->get_rated_reactive_power()) {
            pEss[i]->set_reactive_power_setpoint(-1 * pEss[i]->get_rated_reactive_power());
        }
    }
}

/*
 * @brief Based on the target/potential reactive power distribute reactive power commands appropriately.
 */
void ESS_Manager::calculate_ess_reactive_power(void) {
    // no reactive power requested
    if (essTargetReactivePowerkVAR == 0 || essTotalPotentialReactivePower == 0) {
        zero_all_controllable_ess_reactive_power();
        return;
    }

    // full reactive power requested
    if (fabsf(essTargetReactivePowerkVAR) >= essTotalRatedReactivePower) {
        essTargetReactivePowerkVAR > 0 ? all_controllable_ess_max_reactive_power() : all_controllable_ess_min_reactive_power();
        return;
    }

    // TODO(unknown): do something smart for requested is larger than available
    // for now do same as if the reactive power is available
    // There is reactive power that can be handled by available
    for (int i = 0; i < numParsed; i++) {
        if (pEss[i]->is_controllable()) {
            float weightedShare = (pEss[i]->get_potential_reactive_power() == 0)
                                      ? 0
                                      : essTargetReactivePowerkVAR / (essTotalPotentialReactivePower / pEss[i]->get_potential_reactive_power());
            if (pEss[i]->get_reactive_power_setpoint() != weightedShare) {
                pEss[i]->set_reactive_power_setpoint(weightedShare);
            }
        }
    }
}

// distributes essTargetActivePowerkW (set by Site Manager) to the ESS asset instances
// uses SOC balancing algorithm to decide how much power each ESS gets
bool ESS_Manager::calculate_ess_active_power(void) {
    // we do not want to change essTargetActivePowerkW,
    // so powerToAssign is the value we use to keep track of assigned power
    float powerToAssign = essTargetActivePowerkW;

    // identify ESSs that are controllable and can take a power command
    std::vector<SOC_Balancing_Data> essAssetsToAssignPowerTo;
    for (auto it = pEss.begin(); it != pEss.end(); ++it) {
        if ((*it)->is_controllable()) {
            // calculate contribution factor: soc^b if discharging or (100-soc)^b if charging
            SOC_Balancing_Data balancingData;
            balancingData.ess = *it;
            // use LIMITED power here as a way to account for reactive_power_priority
            // NOTE: get_min_limited_active_power() returns negative, but we want positive flip it here no need for new func
            balancingData.powerLimit = powerToAssign >= 0 ? (*it)->get_max_limited_active_power() : -1 * (*it)->get_min_limited_active_power();
            float socMargin = powerToAssign > 0 ? (*it)->get_soc() : (100 - (*it)->get_soc());
            // If at 0% or 100% soc but there is still (dis)chargeable power available, continue to allow the ESS to (dis)charge
            if (socMargin == 0.0 && balancingData.powerLimit != 0.0) {
                socMargin = 1.0f;
            }
            balancingData.contributionFactor = powf(socMargin, socBalancingFactor);
            essAssetsToAssignPowerTo.push_back(balancingData);
        }
    }

    // iterate in balancing algorithm until all power assignments are within limits
    bool powerLimitViolated;
    do {
        // Reset powerLimitViolated flag for each iteration attempt
        powerLimitViolated = false;

        // calculate K, the power gain that will insure sum of distributed power will equal powerToAssign
        // K = the total power to assign divided by the sum of each ESS's contribution factor
        float aggregateContribution = 0;
        for (auto it = essAssetsToAssignPowerTo.begin(); it != essAssetsToAssignPowerTo.end(); ++it) {
            aggregateContribution += it->contributionFactor;
        }
        float k = ((aggregateContribution == 0) ? 0 : (powerToAssign / aggregateContribution));

        // set each ESS's active power setpoint to K times its contribution factor
        for (auto it = essAssetsToAssignPowerTo.begin(); it != essAssetsToAssignPowerTo.end(); ++it) {
            float setpoint = k * it->contributionFactor;
            // if calculated setpoint is over ESS instance power limit, set it to its power limit, reduce powerToAssign, and remove ESS from algorithm.
            // must reiterate through algorithm if power limit violated
            if (balancePowerDistribution && fabsf(setpoint) > it->powerLimit) {
                float signed_power_limit = powerToAssign > 0 ? it->powerLimit : -1 * it->powerLimit;
                it->ess->set_active_power_setpoint(signed_power_limit, false);
                powerToAssign -= it->ess->get_active_power_setpoint_control();
                essAssetsToAssignPowerTo.erase(it);
                powerLimitViolated = true;
                break;
            }
            // if calculated setpoint is within ESS instance power limit, or a feature has disabled the power distribution balancing, give the setpoint to the ESS
            it->ess->set_active_power_setpoint(setpoint, false);
        }
    } while (powerLimitViolated);

    return true;
}

float ESS_Manager::charge_control(float target_soc_desired, bool charge_disable, bool discharge_disable) {
    // set boundary values
    float target_soc = target_soc_desired;
    float leaving_target_below = target_soc - leaving_soc_range;
    float leaving_target_above = target_soc + leaving_soc_range;
    float returning_target_below = target_soc - entering_soc_range;
    float returning_target_above = target_soc + entering_soc_range;
    float far_below_target = target_soc - far_soc_range;
    float far_above_target = target_soc + far_soc_range;

    float current_soc = controllableEssAvgSoc;

    // update soc_state
    switch (soc_status) {
        case FarAboveTarget:
            if (current_soc < far_below_target) {
                soc_status = FarBelowTarget;
            } else if (current_soc < leaving_target_below) {
                soc_status = BelowTarget;
            } else if (current_soc < returning_target_above) {
                soc_status = OnTarget;
            } else if (current_soc < far_above_target) {
                soc_status = AboveTarget;
            }
            break;
        case AboveTarget:
            if (current_soc > far_above_target) {
                soc_status = FarAboveTarget;
            } else if (current_soc < far_below_target) {
                soc_status = FarBelowTarget;
            } else if (current_soc < leaving_target_below) {
                soc_status = BelowTarget;
            } else if (current_soc < returning_target_above) {
                soc_status = OnTarget;
            }
            break;
        case OnTarget:
            if (current_soc > far_above_target) {
                soc_status = FarAboveTarget;
            } else if (current_soc > leaving_target_above) {
                soc_status = AboveTarget;
            } else if (current_soc < far_below_target) {
                soc_status = FarBelowTarget;
            } else if (current_soc < leaving_target_below) {
                soc_status = BelowTarget;
            }
            break;
        case BelowTarget:
            if (current_soc > far_above_target) {
                soc_status = FarAboveTarget;
            } else if (current_soc > leaving_target_above) {
                soc_status = AboveTarget;
            } else if (current_soc > returning_target_below) {
                soc_status = OnTarget;
            } else if (current_soc < far_below_target) {
                soc_status = FarBelowTarget;
            }
            break;
        case FarBelowTarget:
            if (current_soc > far_above_target) {
                soc_status = FarAboveTarget;
            } else if (current_soc > leaving_target_above) {
                soc_status = AboveTarget;
            } else if (current_soc > returning_target_below) {
                soc_status = OnTarget;
            } else if (current_soc > far_below_target) {
                soc_status = BelowTarget;
            }
            break;
    }

    // take action based on updated soc_status
    float charge_request = 0.0;
    switch (soc_status) {
            // For FarAboveTarget and AboveTarget, only discharge if discharge is not disabled, else return 0
        case FarAboveTarget:
            charge_request = !discharge_disable * essTotalDischargeablePowerkW;
            break;
        case AboveTarget: {
            charge_request = !discharge_disable * essTotalDischargeablePowerkW * ((current_soc - returning_target_below) / charge_control_divisor);
            charge_request = (charge_request > essTotalDischargeablePowerkW) ? essTotalDischargeablePowerkW : charge_request;
            break;
        }
        case OnTarget:
            charge_request = 0.0;
            break;
            // For BelowTarget and FarBelowTarget, only charge if charge is not disabled, else return 0
        case BelowTarget: {
            charge_request = !charge_disable * (-1) * essTotalChargeablePowerkW * ((returning_target_above - current_soc) / charge_control_divisor);
            charge_request = (charge_request < (-1 * essTotalChargeablePowerkW)) ? (-1 * essTotalChargeablePowerkW) : charge_request;
            break;
        }
        case FarBelowTarget:
            charge_request = !charge_disable * (-1.0) * essTotalChargeablePowerkW;
            break;
    }

    return charge_request;
}

// HybridOS Step 2: Process Asset Data
void ESS_Manager::process_asset_data() {
    if (numParsed > 0) {
        for (int i = 0; i < numParsed; i++) {
            // Update Asset level data with map values
            pEss[i]->process_asset();
        }
        aggregate_ess_data();
    }
}

// HybridOS Step 4: Update Asset Data
void ESS_Manager::update_asset_data(void) {
    calculate_ess_active_power();
    calculate_ess_reactive_power();
    for (int i = 0; i < numParsed; i++)
        pEss[i]->update_asset();
}

/****************************************************************************************/
/*
    Overriding configuration functions
*/
void ESS_Manager::configure_base_class_list() {
    pAssets.assign(pEss.begin(), pEss.end());
}

Asset* ESS_Manager::build_new_asset(void) {
    Asset_ESS* asset = new Asset_ESS;
    if (asset == NULL) {
        FPS_ERROR_LOG("There is something wrong with this build. Ess %zu: Memory allocation error.", pEss.size() + 1);
        exit(1);
    }
    numParsed++;
    return asset;
}

void ESS_Manager::append_new_asset(Asset* asset) {
    pEss.push_back(dynamic_cast<Asset_ESS*>(asset));
}

// After configuring individual asset instances, this function finishes configuring the ESS Manager
Config_Validation_Result ESS_Manager::configure_type_manager(Type_Configurator* configurator) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    cJSON* essRoot = configurator->asset_type_root;

    cJSON* object = cJSON_HasObjectItem(essRoot, "soc_balancing_factor") ? cJSON_GetObjectItem(essRoot, "soc_balancing_factor") : NULL;
    if (object == NULL) {
        validation_result.is_valid_config = false;
        validation_result.ERROR_details.push_back(
            Result_Details(fmt::format("{}: failed to find soc_balancing_factor in config.", configurator->p_manager->get_asset_type_id())));
    }
    socBalancingFactor = (float)object->valuedouble;

    // Optional Charge Control SoC ranges
    object = cJSON_HasObjectItem(essRoot, "entering_soc_range") ? cJSON_GetObjectItem(essRoot, "entering_soc_range") : NULL;
    if (object != NULL)
        entering_soc_range = (float)object->valuedouble;

    object = cJSON_HasObjectItem(essRoot, "leaving_soc_range") ? cJSON_GetObjectItem(essRoot, "leaving_soc_range") : NULL;
    if (object != NULL)
        leaving_soc_range = (float)object->valuedouble;

    object = cJSON_HasObjectItem(essRoot, "far_soc_range") ? cJSON_GetObjectItem(essRoot, "far_soc_range") : NULL;
    if (object != NULL)
        far_soc_range = (float)object->valuedouble;

    object = cJSON_HasObjectItem(essRoot, "charge_control_divisor") ? cJSON_GetObjectItem(essRoot, "charge_control_divisor") : NULL;
    if (object != NULL)
        charge_control_divisor = (float)object->valuedouble;

    return validation_result;
}
/****************************************************************************************/
