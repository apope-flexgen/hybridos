/*
 * Site_Manager.cpp
 *
 *  Created on: Sep 5, 2018
 *      Author: jcalcagni
 */

/* C Standard Library Dependencies */
#include "Types.h"
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
/* C++ Standard Library Dependencies */
#include <stdexcept>
#include <string>
#include <limits>
#include <vector>
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
#include <fims/libfims.h>
/* Local Internal Dependencies */
#include <Sequence.h>
#include <Site_Manager.h>
#include <Data_Endpoint.h>
#include <Site_Controller_Utils.h>

extern Data_Endpoint* data_endpoint;

Site_Manager::Site_Manager(Version* release_version) {
    if (release_version != NULL) {
        release_version_tag.value.set(release_version->get_tag());
        release_version_commit.value.set(release_version->get_commit());
        release_version_build.value.set(release_version->get_build());
    }

    current_state = Init;
    check_current_state = Init;
    step_change = true;
    path_change = true;
    sequence_reset = true;
    pAssets = NULL;
    pFims = NULL;
    standby_ess_latch = false;
    standby_solar_latch = false;
    num_ess_available = 0;
    num_ess_running = 0;
    num_solar_available = 0;
    num_solar_running = 0;
    num_gen_available = 0;
    num_gen_running = 0;
    num_gen_active = 0;
    prev_ess_kW_cmd = 0.0;
    prev_feeder_kW_cmd = 0.0;
    prev_gen_kW_cmd = 0.0;
    prev_solar_kW_cmd = 0.0;
    prev_ess_kVAR_cmd = 0.0;
    prev_gen_kVAR_cmd = 0.0;
    prev_solar_kVAR_cmd = 0.0;
    prev_asset_pf_cmd = 0.0;
    asset_pf_flag = false;
    prev_asset_pf_flag = true;
    current_runmode1_kW_feature = -1;
    current_runmode2_kW_feature = -1;
    current_runmode1_kVAR_feature = -1;
    prev_reactive_power_feature_cmd = 0.0f;

    for (int i = 0; i < 64; i++) {
        active_fault_array[i] = false;
    }
    for (int i = 0; i < 64; i++) {
        active_alarm_array[i] = false;
    }
    num_path_faults = 0;
    num_path_alarms = 0;

    watchdog_old_pet = 0;

    clock_gettime(CLOCK_MONOTONIC, &current_time);
    clock_gettime(CLOCK_MONOTONIC, &exit_target_time);
    clock_gettime(CLOCK_MONOTONIC, &heartbeat_timer);
    clock_gettime(CLOCK_MONOTONIC, &watchdog_timeout);
    increment_timespec_ms(watchdog_timeout, watchdog_duration_ms.value.value_int);
    clock_gettime(CLOCK_MONOTONIC, &time_to_clear_fault_status_flags);

    clear_fault_status_flags = false;

    // Initialize variable id's of variables that are not in variables.json but still receive FIMS sets
    cops_heartbeat.set_variable_id("cops_heartbeat");
}

void Site_Manager::configure_feature_objects(void) {
    //
    // Run Mode 1 Active Power Features
    //
    // Energy Arbitrage
    energy_arbitrage_feature.feature_vars = {
        &energy_arb_obj.price,         &energy_arb_obj.threshold_charge_1, &energy_arb_obj.threshold_charge_2, &energy_arb_obj.threshold_dischg_1, &energy_arb_obj.threshold_dischg_2, &energy_arb_obj.soc_min_limit,
        &energy_arb_obj.soc_max_limit, &energy_arb_obj.max_charge_1,       &energy_arb_obj.max_charge_2,       &energy_arb_obj.max_dischg_1,       &energy_arb_obj.max_dischg_2,
    };
    energy_arbitrage_feature.summary_vars = {
        &energy_arb_obj.threshold_charge_2, &energy_arb_obj.threshold_charge_1, &energy_arb_obj.threshold_dischg_1, &energy_arb_obj.threshold_dischg_2, &energy_arb_obj.price,
    };
    // Manual Power Mode
    manual_power_mode.feature_vars = { &manual_solar_kW_cmd, &manual_ess_kW_cmd, &manual_gen_kW_cmd, &manual_solar_kW_slew_rate, &manual_ess_kW_slew_rate, &manual_gen_kW_slew_rate };
    // Frequency Response Mode
    frequency_response.get_feature_vars(frequency_response_feature.feature_vars);
    frequency_response.get_summary_vars(frequency_response_feature.summary_vars);

    ess_calibration.feature_vars = {
        &ess_calibration_kW_cmd,       &ess_calibration_soc_limits_enable, &ess_calibration_min_soc_limit, &ess_calibration_max_soc_limit, &ess_calibration_voltage_limits_enable, &ess_calibration_min_voltage_limit, &ess_calibration_max_voltage_limit,
        &ess_calibration_num_setpoint, &ess_calibration_num_limited,       &ess_calibration_num_zero
    };
    // Copy SoC balancing factor to be preserved in Site Manager and overwritten in Asset Manager when this feature is enabled
    ess_soc_balancing_factor = pAssets->get_soc_balancing_factor();
    // Disable all features as part of initial configuration in case any are accidentally enabled
    for (auto feature : runmode1_kW_features_list)
        feature->enable_flag.value.set(false);

    //
    // Run Mode 2 Active Power Features
    //
    // Generator Charge
    generator_charge.feature_vars = { &generator_charge_additional_buffer };

    // Disable all features as part of initial configuration in case any are accidentally enabled
    for (auto feature : runmode2_kW_features_list)
        feature->enable_flag.value.set(false);

    //
    // Charge features
    //
    // Charge Dispatch
    charge_dispatch.feature_vars = {
        &charge_dispatch_kW_command,  // include in this list as a status is required for the feature to display
        &charge_dispatch_solar_enable_flag,
        &charge_dispatch_gen_enable_flag,
        &charge_dispatch_feeder_enable_flag,
    };
    charge_dispatch.available = true;  // charge dispatch is a special case in that it should always be enabled
    charge_dispatch.enable_flag.value.value_bool = true;
    charge_dispatch.toggle_ui_enabled(true);
    // Charge Control
    charge_control.feature_vars = {
        &ess_charge_control_kW_request, &ess_charge_control_target_soc, &ess_charge_control_kW_limit, &ess_charge_control_charge_disable, &ess_charge_control_discharge_disable,
    };

    //
    // Run Mode 1 Reactive Power Features
    //
    // Watt-Var Mode
    watt_var.feature_vars = { &watt_var_points };
    // Reactive Setpoint Mode
    reactive_setpoint.feature_vars = { &reactive_setpoint_kVAR_cmd, &reactive_setpoint_kVAR_slew_rate };
    // Power Factor Mode
    power_factor.feature_vars = { &power_factor_cmd };
    // Constant Power Factor Mode
    constant_power_factor.feature_vars = {
        &constant_power_factor_setpoint, &constant_power_factor_lagging_limit, &constant_power_factor_leading_limit, &constant_power_factor_absolute_mode, &constant_power_factor_lagging_direction,
    };
    // Disable all features as part of initial configuration in case any are accidentally enabled
    for (auto feature : runmode1_kVAR_features_list)
        feature->enable_flag.value.set(false);

    //
    // Run Mode 2 Reactive Power Features
    //
    // no runmode2 reactive power features yet

    //
    // Standalone Power Features
    //
    // POI Limits
    active_power_poi_limits.feature_vars = {
        &active_power_poi_limits_max_kW,         &active_power_poi_limits_min_kW,         &active_power_soc_poi_limits_enable,      &active_power_soc_poi_target_soc,
        &active_power_soc_poi_limits_low_min_kW, &active_power_soc_poi_limits_low_max_kW, &active_power_soc_poi_limits_high_min_kW, &active_power_soc_poi_limits_high_max_kW,
    };
    active_power_poi_limits.summary_vars = { &active_power_poi_limits_max_kW, &active_power_poi_limits_min_kW };
    // PFR
    pfr.feature_vars = {
        &pfr_deadband, &pfr_droop_percent, &pfr_offset_hz, &pfr_site_nominal_hz, &site_frequency, &pfr_status_flag, &pfr_nameplate_kW, &pfr_limits_max_kW, &pfr_limits_min_kW,
    };
    pfr.summary_vars = { &pfr_status_flag, &site_frequency };
    // Watt-Watt
    watt_watt.feature_vars = { &watt_watt_points };
    // LDSS
    ldss.feature_vars = {
        &ldss_priority_setting,     &ldss_max_load_threshold_percent, &ldss_min_load_threshold_percent, &ldss_warmup_time, &ldss_cooldown_time, &ldss_start_gen_time, &ldss_stop_gen_time,
        &ldss_enable_soc_threshold, &ldss_max_soc_threshold_percent,  &ldss_min_soc_threshold_percent,
    };
    // Load Shed
    load_shed.feature_vars = {
        &load_shed_value,        &load_shed_max_value,         &load_shed_min_value,         &load_shed_max_shedding_threshold, &load_shed_high_threshold,         &load_shed_low_threshold,
        &load_shed_spare_ess_kw, &load_shed_increase_timer_ms, &load_shed_decrease_timer_ms, &load_shed_increase_display_timer, &load_shed_decrease_display_timer,
    };
    // Solar Shed
    solar_shed.feature_vars = {
        &solar_shed_value,        &solar_shed_max_value,         &solar_shed_min_value,         &solar_shed_max_shedding_threshold, &solar_shed_high_threshold,         &solar_shed_low_threshold,
        &solar_shed_spare_ess_kw, &solar_shed_increase_timer_ms, &solar_shed_decrease_timer_ms, &solar_shed_increase_display_timer, &solar_shed_decrease_display_timer,
    };
    // ESS Discharge Prevention
    ess_discharge_prevention.feature_vars = { &edp_soc };
    // Aggregated Asset Limit
    agg_asset_limit.feature_vars = { &agg_asset_limit_kw };
    // Closed Loop Control
    reactive_power_closed_loop.feature_vars = {
        &reactive_power_closed_loop_step_size_kW,
        &reactive_power_closed_loop_default_offset,
        &reactive_power_closed_loop_min_offset,
        &reactive_power_closed_loop_max_offset,
        &reactive_power_closed_loop_total_correction,
        &reactive_power_closed_loop_steady_state_deadband_kW,
        &reactive_power_closed_loop_regulation_deadband_kW,
        &reactive_power_closed_loop_update_rate_ms,
        &reactive_power_closed_loop_decrease_timer_ms,
        &reactive_power_closed_loop_increase_timer_ms,
    };
    reactive_power_poi_limits.feature_vars = {
        &reactive_power_poi_limits_min_kVAR,
        &reactive_power_poi_limits_max_kVAR,
    };
    reactive_power_poi_limits.summary_vars = {
        &reactive_power_poi_limits_min_kVAR,
        &reactive_power_poi_limits_max_kVAR,
    };

    //
    // Site Operation Features
    //
    // Watchdog
    watchdog_feature.feature_vars = {
        &watchdog_duration_ms,
        &watchdog_pet,
        &heartbeat_counter,
        &heartbeat_duration_ms,
    };
}

// gets called just when the user clicks the Features tab. publish_all() subsequently updates the values
void Site_Manager::build_JSON_features(fmt::memory_buffer& buf) {
    bufJSON_StartObject(buf);  // JSON_object {
    bufJSON_AddId(buf, "summary");
    build_JSON_features_summary(buf, true);
    bufJSON_AddId(buf, "active_power");
    build_JSON_features_active_power(buf);
    bufJSON_AddId(buf, "reactive_power");
    build_JSON_features_reactive_power(buf);
    bufJSON_AddId(buf, "standalone_power");
    build_JSON_features_standalone_power(buf);
    bufJSON_AddId(buf, "site_operation");
    build_JSON_features_site_operation(buf);
    bufJSON_EndObject(buf);  // } JSON_object
}

void Site_Manager::build_JSON_features_summary(fmt::memory_buffer& buf, bool clothed, const char* const var) {
    if (var == NULL)
        bufJSON_StartObject(buf);  // JSON_object {
    bufJSON_AddStringCheckVar(buf, "name", "Features Summary", var);

    // mode status
    runmode1_kW_mode_status.add_to_JSON_buffer(buf, var, clothed);
    runmode2_kW_mode_status.add_to_JSON_buffer(buf, var, clothed);
    runmode1_kVAR_mode_status.add_to_JSON_buffer(buf, var, clothed);
    runmode2_kVAR_mode_status.add_to_JSON_buffer(buf, var, clothed);

    // status
    site_frequency.add_to_JSON_buffer(buf, var, clothed);

    // summary vars for standalone/active/reactive power features
    add_feature_group_summary_vars_to_JSON_buffer(runmode1_kW_features_list, buf, var);
    add_feature_group_summary_vars_to_JSON_buffer(runmode2_kW_features_list, buf, var);
    add_feature_group_summary_vars_to_JSON_buffer(charge_features_list, buf, var);
    add_feature_group_summary_vars_to_JSON_buffer(runmode1_kVAR_features_list, buf, var);
    add_feature_group_summary_vars_to_JSON_buffer(runmode2_kVAR_features_list, buf, var);
    add_feature_group_summary_vars_to_JSON_buffer(standalone_power_features_list, buf, var);
    add_feature_group_summary_vars_to_JSON_buffer(site_operation_features_list, buf, var);

    if (var == NULL)
        bufJSON_EndObject(buf);  // } JSON_object
}

void Site_Manager::build_JSON_features_active_power(fmt::memory_buffer& buf, const char* const var) {
    if (var == NULL)
        bufJSON_StartObject(buf);  // JSON_object {
    bufJSON_AddStringCheckVar(buf, "name", "Active Power Management", var);

    // feature selection
    available_features_runmode1_kW_mode.add_to_JSON_buffer(buf, var);
    available_features_runmode2_kW_mode.add_to_JSON_buffer(buf, var);
    runmode1_kW_mode_status.add_to_JSON_buffer(buf, var);
    runmode1_kW_mode_cmd.add_to_JSON_buffer(buf, var);
    runmode2_kW_mode_status.add_to_JSON_buffer(buf, var);
    runmode2_kW_mode_cmd.add_to_JSON_buffer(buf, var);

    // active power features feature variables
    add_feature_group_feature_vars_to_JSON_buffer(runmode1_kW_features_list, buf, var);
    add_feature_group_feature_vars_to_JSON_buffer(runmode2_kW_features_list, buf, var);
    add_feature_group_feature_vars_to_JSON_buffer(charge_features_list, buf, var);

    // data_status
    site_frequency.add_to_JSON_buffer(buf, var);
    site_kW_load.add_to_JSON_buffer(buf, var);
    site_kW_load_inclusion.add_to_JSON_buffer(buf, var);
    feature_kW_demand.add_to_JSON_buffer(buf, var);
    site_kW_demand.add_to_JSON_buffer(buf, var);
    asset_priority_runmode1.add_to_JSON_buffer(buf, var);
    asset_priority_runmode2.add_to_JSON_buffer(buf, var);
    site_kW_charge_production.add_to_JSON_buffer(buf, var);
    site_kW_discharge_production.add_to_JSON_buffer(buf, var);
    total_site_kW_rated_charge.add_to_JSON_buffer(buf, var);
    total_site_kW_rated_discharge.add_to_JSON_buffer(buf, var);
    total_site_kW_charge_limit.add_to_JSON_buffer(buf, var);
    total_site_kW_discharge_limit.add_to_JSON_buffer(buf, var);
    if (pAssets->get_num_ess_parsed() > 0) {
        ess_kW_cmd.add_to_JSON_buffer(buf, var);
        max_potential_ess_kW.add_to_JSON_buffer(buf, var);
        min_potential_ess_kW.add_to_JSON_buffer(buf, var);
        rated_ess_kW.add_to_JSON_buffer(buf, var);
        ess_actual_kW.add_to_JSON_buffer(buf, var);
        // TODO: variables shared by active power features get added multiple times
        // but incorrectly, creating invalid json. Pulled out this variable as a quick fix
        soc_avg_running.add_to_JSON_buffer(buf, var);
    }
    if (pAssets->get_num_feeder_parsed() > 0) {
        feeder_kW_cmd.add_to_JSON_buffer(buf, var);
        max_potential_feeder_kW.add_to_JSON_buffer(buf, var);
        min_potential_feeder_kW.add_to_JSON_buffer(buf, var);
        rated_feeder_kW.add_to_JSON_buffer(buf, var);
        feeder_actual_kW.add_to_JSON_buffer(buf, var);
    }
    if (pAssets->get_num_gen_parsed() > 0) {
        gen_kW_cmd.add_to_JSON_buffer(buf, var);
        max_potential_gen_kW.add_to_JSON_buffer(buf, var);
        min_potential_gen_kW.add_to_JSON_buffer(buf, var);
        rated_gen_kW.add_to_JSON_buffer(buf, var);
        gen_actual_kW.add_to_JSON_buffer(buf, var);
    }
    if (pAssets->get_num_solar_parsed() > 0) {
        solar_kW_cmd.add_to_JSON_buffer(buf, var);
        max_potential_solar_kW.add_to_JSON_buffer(buf, var);
        min_potential_solar_kW.add_to_JSON_buffer(buf, var);
        rated_solar_kW.add_to_JSON_buffer(buf, var);
        solar_actual_kW.add_to_JSON_buffer(buf, var);
    }
    if (var == NULL)
        bufJSON_EndObject(buf);  // } JSON_object
}

void Site_Manager::build_JSON_features_reactive_power(fmt::memory_buffer& buf, const char* const var) {
    if (var == NULL)
        bufJSON_StartObject(buf);  // JSON_object {
    bufJSON_AddStringCheckVar(buf, "name", "Reactive Power Management", var);

    // feature selection
    available_features_runmode1_kVAR_mode.add_to_JSON_buffer(buf, var);
    runmode1_kVAR_mode_status.add_to_JSON_buffer(buf, var);
    runmode1_kVAR_mode_cmd.add_to_JSON_buffer(buf, var);

    // reactive power features feature variables
    add_feature_group_feature_vars_to_JSON_buffer(runmode1_kVAR_features_list, buf, var);
    add_feature_group_feature_vars_to_JSON_buffer(runmode2_kVAR_features_list, buf, var);

    // data_status
    site_kVAR_demand.add_to_JSON_buffer(buf, var);
    if (pAssets->get_num_ess_parsed() > 0) {
        ess_kVAR_cmd.add_to_JSON_buffer(buf, var);
        potential_ess_kVAR.add_to_JSON_buffer(buf, var);
        ess_actual_kVAR.add_to_JSON_buffer(buf, var);
    }
    if (pAssets->get_num_gen_parsed() > 0) {
        gen_kVAR_cmd.add_to_JSON_buffer(buf, var);
        potential_gen_kVAR.add_to_JSON_buffer(buf, var);
        gen_actual_kVAR.add_to_JSON_buffer(buf, var);
    }
    if (pAssets->get_num_feeder_parsed() > 0) {
        feeder_actual_kVAR.add_to_JSON_buffer(buf, var);
        feeder_actual_pf.add_to_JSON_buffer(buf, var);
    }
    if (pAssets->get_num_solar_parsed() > 0) {
        solar_kVAR_cmd.add_to_JSON_buffer(buf, var);
        potential_solar_kVAR.add_to_JSON_buffer(buf, var);
        solar_actual_kVAR.add_to_JSON_buffer(buf, var);
    }

    if (var == NULL)
        bufJSON_EndObject(buf);  // } JSON_object
}

void Site_Manager::build_JSON_features_standalone_power(fmt::memory_buffer& buf, const char* const var) {
    if (var == NULL)
        bufJSON_StartObject(buf);  // JSON_object {
    bufJSON_AddStringCheckVar(buf, "name", "Standalone Power Features", var);

    // standalone power features feature variables
    add_feature_group_feature_vars_to_JSON_buffer(standalone_power_features_list, buf, var);

    if (var == NULL)
        bufJSON_EndObject(buf);  // } JSON_object
}

void Site_Manager::build_JSON_features_site_operation(fmt::memory_buffer& buf, const char* const var) {
    if (var == NULL)
        bufJSON_StartObject(buf);  // JSON_object {
    bufJSON_AddStringCheckVar(buf, "name", "Site Operation Features", var);

    // site operation features feature variables
    power_priority_flag.add_to_JSON_buffer(buf, var);
    allow_ess_auto_restart.add_to_JSON_buffer(buf, var);
    allow_solar_auto_restart.add_to_JSON_buffer(buf, var);
    allow_gen_auto_restart.add_to_JSON_buffer(buf, var);
    add_feature_group_feature_vars_to_JSON_buffer(site_operation_features_list, buf, var);

    if (var == NULL)
        bufJSON_EndObject(buf);  // } JSON_object
}

void Site_Manager::build_JSON_site(fmt::memory_buffer& buf) {
    bufJSON_StartObject(buf);  // JSON_object {
    bufJSON_AddId(buf, "summary");
    build_JSON_site_summary(buf, true);
    bufJSON_AddId(buf, "operation");
    build_JSON_site_operation(buf);
    bufJSON_AddId(buf, "configuration");
    build_JSON_site_configuration(buf);

    bufJSON_EndObject(buf);  // } JSON_object
}

void Site_Manager::build_JSON_site_summary(fmt::memory_buffer& buf, bool clothed, const char* const var) {
    if (var == NULL)
        bufJSON_StartObject(buf);  // JSON_object {

    bufJSON_AddStringCheckVar(buf, "name", "Site Summary", var);

    // ops status
    site_state.add_to_JSON_buffer(buf, var, clothed);
    site_state_enum.add_to_JSON_buffer(buf, var, clothed);
    site_status.add_to_JSON_buffer(buf, var, clothed);
    input_source_status.add_to_JSON_buffer(buf, var, clothed);
    disable_flag.add_to_JSON_buffer(buf, var, clothed);
    enable_flag.add_to_JSON_buffer(buf, var, clothed);
    clear_faults_flag.add_to_JSON_buffer(buf, var, clothed);
    active_faults.add_to_JSON_buffer(buf, var, clothed);
    active_alarms.add_to_JSON_buffer(buf, var, clothed);
    fault_status_flag.add_to_JSON_buffer(buf, var, clothed);
    alarm_status_flag.add_to_JSON_buffer(buf, var, clothed);

    if (pAssets->get_num_ess_parsed() > 0) {
        ess_kW_cmd.add_to_JSON_buffer(buf, var, clothed);
        ess_kVAR_cmd.add_to_JSON_buffer(buf, var, clothed);
        ess_instant_discharge.add_to_JSON_buffer(buf, var, clothed);
        ess_instant_charge_grid.add_to_JSON_buffer(buf, var, clothed);
        ess_instant_charge_pv.add_to_JSON_buffer(buf, var, clothed);
    }
    if (pAssets->get_num_gen_parsed() > 0) {
        gen_kW_cmd.add_to_JSON_buffer(buf, var, clothed);
        gen_kVAR_cmd.add_to_JSON_buffer(buf, var, clothed);
    }
    if (pAssets->get_num_feeder_parsed() > 0) {
        feeder_kW_cmd.add_to_JSON_buffer(buf, var, clothed);
    }
    if (pAssets->get_num_solar_parsed() > 0) {
        solar_kW_cmd.add_to_JSON_buffer(buf, var, clothed);
        solar_kVAR_cmd.add_to_JSON_buffer(buf, var, clothed);
    }

    if (var == NULL)
        bufJSON_EndObject(buf);  // } JSON_object
}

void Site_Manager::build_JSON_site_operation(fmt::memory_buffer& buf, const char* const var) {
    if (var == NULL)
        bufJSON_StartObject(buf);  // JSON_object {

    bufJSON_AddStringCheckVar(buf, "name", "Operation", var);

    // ops status
    site_state.add_to_JSON_buffer(buf, var);
    site_state_enum.add_to_JSON_buffer(buf, var);
    site_status.add_to_JSON_buffer(buf, var);
    running_status_flag.add_to_JSON_buffer(buf, var);
    disable_flag.add_to_JSON_buffer(buf, var);
    standby_flag.add_to_JSON_buffer(buf, var);
    enable_flag.add_to_JSON_buffer(buf, var);
    clear_faults_flag.add_to_JSON_buffer(buf, var);
    active_faults.add_to_JSON_buffer(buf, var);
    active_alarms.add_to_JSON_buffer(buf, var);
    fault_status_flag.add_to_JSON_buffer(buf, var);
    alarm_status_flag.add_to_JSON_buffer(buf, var);

    if (var == NULL)
        bufJSON_EndObject(buf);  // } JSON_object
}

void Site_Manager::build_JSON_site_configuration(fmt::memory_buffer& buf, const char* const var) {
    if (var == NULL)
        bufJSON_StartObject(buf);  // JSON_object {
    bufJSON_AddStringCheckVar(buf, "name", "Configuration", var);

    // config status
    // input source status
    input_source_status.add_to_JSON_buffer(buf, var);
    // asset slews
    if (pAssets->get_num_ess_parsed() > 0) {
        ess_kVAR_slew_rate.add_to_JSON_buffer(buf, var);
    }
    if (pAssets->get_num_gen_parsed() > 0) {
        gen_kVAR_slew_rate.add_to_JSON_buffer(buf, var);
    }
    if (pAssets->get_num_solar_parsed() > 0) {
        solar_kVAR_slew_rate.add_to_JSON_buffer(buf, var);
    }
    // config flags
    invert_poi_kW.add_to_JSON_buffer(buf, var);
    // reserved bools
    reserved_bool_1.add_to_JSON_buffer(buf, var);
    reserved_bool_2.add_to_JSON_buffer(buf, var);
    reserved_bool_3.add_to_JSON_buffer(buf, var);
    reserved_bool_4.add_to_JSON_buffer(buf, var);
    reserved_bool_5.add_to_JSON_buffer(buf, var);
    reserved_bool_6.add_to_JSON_buffer(buf, var);
    reserved_bool_7.add_to_JSON_buffer(buf, var);
    reserved_bool_8.add_to_JSON_buffer(buf, var);
    reserved_bool_9.add_to_JSON_buffer(buf, var);
    reserved_bool_10.add_to_JSON_buffer(buf, var);
    reserved_bool_11.add_to_JSON_buffer(buf, var);
    reserved_bool_12.add_to_JSON_buffer(buf, var);
    reserved_bool_13.add_to_JSON_buffer(buf, var);
    reserved_bool_14.add_to_JSON_buffer(buf, var);
    reserved_bool_15.add_to_JSON_buffer(buf, var);
    reserved_bool_16.add_to_JSON_buffer(buf, var);
    reserved_float_1.add_to_JSON_buffer(buf, var);
    reserved_float_2.add_to_JSON_buffer(buf, var);
    reserved_float_3.add_to_JSON_buffer(buf, var);
    reserved_float_4.add_to_JSON_buffer(buf, var);
    reserved_float_5.add_to_JSON_buffer(buf, var);
    reserved_float_6.add_to_JSON_buffer(buf, var);
    reserved_float_7.add_to_JSON_buffer(buf, var);
    reserved_float_8.add_to_JSON_buffer(buf, var);

    if (var == NULL)
        bufJSON_EndObject(buf);  // } JSON_object
}

void Site_Manager::build_JSON_site_cops(fmt::memory_buffer& buf, const char* const var) {
    if (var == NULL)
        bufJSON_StartObject(buf);  // JSON_object {
    // cops status
    bufJSON_AddNumberCheckVar(buf, "cops_heartbeat", cops_heartbeat.value.value_int, var);
    bufJSON_AddNumberCheckVar(buf, "pid", process_id.value.value_int, var);
    bufJSON_AddStringCheckVar(buf, "version_tag", release_version_tag.value.value_string.c_str(), var);
    bufJSON_AddStringCheckVar(buf, "version_commit", release_version_commit.value.value_string.c_str(), var);
    bufJSON_AddStringCheckVar(buf, "version_build", release_version_build.value.value_string.c_str(), var);

    if (var == NULL)
        bufJSON_EndObject(buf);  // } JSON_object
}

void Site_Manager::build_JSON_site_input_sources(fmt::memory_buffer& buf, const char* const var) {
    if (var == NULL)
        bufJSON_StartObject(buf);  // JSON_object {
    input_sources.add_to_JSON_buffer(buf, var);
    if (var == NULL)
        bufJSON_EndObject(buf);  // } JSON_object
}

/**
 * gets called from HybridOS_Control.  Has Asset_Manager pointer and sequences and variables file pointers
 *
 * @param primary_controller pointer to the bool indicating whether the system is currently
                            the primary controller. This pointer will be passed to Site_Manager
                            and down to the Asset Instances, and can be modified through the
                            fims endpoint /site/operation/primary_controller true/false
                            Testing will now require this pointer to be set.
 */
bool Site_Manager::configure(Asset_Manager* man, fims* fim, cJSON* sequenceRoot, cJSON* varRoot, bool* primary_controller) {
    pAssets = man;
    pFims = fim;
    send_FIMS_buf = fmt::memory_buffer();
    is_primary = primary_controller;

    FPS_DEBUG_LOG("site manager function 'configure' called\n");

    // process sequences.json
    if (sequenceRoot != NULL) {
        FPS_DEBUG_LOG("sequences response received from db \n");
        cJSON* JSON_all_sequences = cJSON_GetObjectItem(sequenceRoot, "sequences");
        if (JSON_all_sequences == NULL) {
            FPS_ERROR_LOG("no sequences found \n");
            return false;
        }
        for (int i = 0; i < NUM_STATES; i++) {
            // get first sequence, ready sequence
            cJSON* JSON_single_sequence = cJSON_GetObjectItem(JSON_all_sequences, state_name[i]);
            if (JSON_single_sequence == NULL) {
                FPS_ERROR_LOG("no %s sequence found \n", state_name[i]);
                return false;
            }
            // create a Sequence class instance for each state and set Site_Manager pointer to access Site_Manager vars/functions
            sequences.emplace_back(this);
            Sequence& current_sequence = sequences.back();

            if (!current_sequence.parse_sequence(JSON_single_sequence, (states)i)) {
                FPS_ERROR_LOG("Failed to parse %s sequence.\n", state_name[i]);
                return false;
            }
        }
    }

    else {
        FPS_ERROR_LOG("no sequences response or invalid JSON found \n");
        return false;
    }

    // process variables.json
    if (varRoot != NULL) {
        FPS_DEBUG_LOG("variables response received from db \n");
        if (!parse_variables(varRoot)) {
            FPS_ERROR_LOG("Failed to parse variables file.\n");
            return false;
        }
    } else {
        FPS_ERROR_LOG("no variables response or invalid JSON found \n");
        return false;
    }

    // alarm options were configured into alarms variable. allocate memory in active_alarms to handle all alarm types
    active_alarms.options_name.resize(alarms.options_name.size());
    active_alarms.options_value.resize(alarms.options_value.size());

    // fault options were configured into faults variable. allocate memory in active_faults to handle all fault types
    active_faults.options_name.resize(faults.options_name.size());
    active_faults.options_value.resize(faults.options_value.size());

    // Point feature objects at their owned variables
    configure_feature_objects();
    // Read which active power features are available for runmode1 and configure them on the UI
    if (!configure_runmode1_kW_features()) {
        FPS_ERROR_LOG("Site_Manager::configure ~ Error configuring runmode1 active power features!\n");
        return false;
    }
    // Read which active power features are available for runmode2 and configure them on the UI
    if (!configure_runmode2_kW_features()) {
        FPS_ERROR_LOG("Site_Manager::configure ~ Error configuring runmode2 active power features!\n");
        return false;
    }
    // Read which reactive power features are available for runmode1 and configure them on the UI
    if (!configure_runmode1_kVAR_features()) {
        FPS_ERROR_LOG("Site_Manager::configure ~ Error configuring runmode1 reactive power features!\n");
        return false;
    }
    // Read which reactive power features are available for runmode2 and configure them on the UI
    if (!configure_runmode2_kVAR_features()) {
        FPS_ERROR_LOG("Site_Manager::configure ~ Error configuring runmode2 reactive power features!\n");
        return false;
    }
    // Read which standalone power features are available and configure them on the UI
    if (!configure_standalone_power_features()) {
        FPS_ERROR_LOG("Site_Manager::configure ~ Error configuring standalone power features!\n");
        return false;
    }
    // Read which site operation features are available and configure them on the UI
    if (!configure_site_operation_features()) {
        FPS_ERROR_LOG("Site_Manager::configure ~ Error configuring site operation features!\n");
        return false;
    }
    configure_persistent_settings_pairs();

    // Record the PID
    process_id.value.set(getpid());

    // set the primary_controller status to the value configured in variables
    // Will still be updated by COPS if it is running
    *is_primary = configured_primary.value.value_bool;
    return true;
}

void Site_Manager::parse_default_vals(cJSON* JSON_defaults, Fims_Object& default_vals) {
    cJSON* name = cJSON_GetObjectItem(JSON_defaults, "name");
    if (name == NULL || name->valuestring == NULL)
        throw std::runtime_error("did not find name field");
    default_vals.set_name(name->valuestring);

    cJSON* unit = cJSON_GetObjectItem(JSON_defaults, "unit");
    if (unit == NULL || unit->valuestring == NULL)
        throw std::runtime_error("did not find unit field");
    default_vals.set_unit(unit->valuestring);

    cJSON* ui_type = cJSON_GetObjectItem(JSON_defaults, "ui_type");
    if (ui_type == NULL || ui_type->valuestring == NULL)
        throw std::runtime_error("did not find ui_type field");
    default_vals.set_ui_type(ui_type->valuestring);

    cJSON* type = cJSON_GetObjectItem(JSON_defaults, "type");
    if (type == NULL || type->valuestring == NULL)
        throw std::runtime_error("did not find type field");
    default_vals.set_type(type->valuestring);

    cJSON* var_type = cJSON_GetObjectItem(JSON_defaults, "var_type");
    cJSON* value = cJSON_GetObjectItem(JSON_defaults, "value");
    if (var_type == NULL || value == NULL || var_type->valuestring == NULL)
        throw std::runtime_error("did not find var_type and/or value fields");

    if (strcmp(var_type->valuestring, "Float") == 0)
        default_vals.value.set((float)value->valuedouble);
    else if (strcmp(var_type->valuestring, "Bool") == 0)
        default_vals.value.set(value->type == cJSON_True);
    else if (strcmp(var_type->valuestring, "String") == 0)
        default_vals.value.set(value->valuestring);
    else if (strcmp(var_type->valuestring, "Int") == 0)
        default_vals.value.set(value->valueint);
    else
        throw std::runtime_error("invalid var_type field");

    cJSON* scaler = cJSON_GetObjectItem(JSON_defaults, "scaler");
    if (scaler == NULL || scaler->type != cJSON_Number)
        throw std::runtime_error("did not find scaler field");
    default_vals.scaler = scaler->valueint;

    cJSON* ui_enabled = cJSON_GetObjectItem(JSON_defaults, "ui_enabled");
    if (ui_enabled == NULL || ui_enabled->type > cJSON_True)
        throw std::runtime_error("did not find ui_enabled field");
    default_vals.ui_enabled = (ui_enabled->type == cJSON_True);

    cJSON* multiple_inputs = cJSON_GetObjectItem(JSON_defaults, "multiple_inputs");
    if (multiple_inputs == NULL || multiple_inputs->type > cJSON_True)
        throw std::runtime_error("did not find multiple_inputs field");
    default_vals.multiple_inputs = (multiple_inputs->type == cJSON_True);

    // optional std::string can handle being unconfigured
    cJSON* write_uri = cJSON_GetObjectItem(JSON_defaults, "write_uri");
    if (write_uri != NULL && write_uri->valuestring != NULL)
        default_vals.write_uri = write_uri->valuestring;

    // default options not supported
    default_vals.num_options = 0;
}

// all variables to be set via variables.json
bool Site_Manager::parse_variables(cJSON* object) {
    cJSON* JSON_variable_object = cJSON_GetObjectItem(object, "variables");
    if (JSON_variable_object == NULL) {
        FPS_ERROR_LOG("variable object is missing from file! \n");
        return false;
    }

    // Create a cJSON object for a flattened variables object with the variables as top-level items
    // This way, variables aren't stuck to a particular category
    cJSON* JSON_flat_vars = cJSON_CreateObject();
    try {
        parse_flatten_vars(JSON_variable_object, JSON_flat_vars);
    } catch (std::exception& ex) {
        FPS_ERROR_LOG(ex.what());
        return false;
    }

    // extract defaults JSON object
    cJSON* JSON_defaults = cJSON_GetObjectItem(JSON_flat_vars, "defaults");
    if (JSON_defaults == NULL) {
        FPS_ERROR_LOG("No defaults found in variables file.\n");
        return false;
    }

    // parse defaults JSON object into Fims_Object
    Fims_Object default_vals;
    try {
        parse_default_vals(JSON_defaults, default_vals);
    } catch (const std::exception& e) {
        FPS_ERROR_LOG("Failed to parse variable defaults from variables.json: %s \n", e.what());
        return false;
    }

    // parse input_sources array
    cJSON* JSON_input_sources = cJSON_GetObjectItem(JSON_flat_vars, "input_sources");
    if (JSON_input_sources != NULL) {
        try {
            input_sources.parse_json_obj(JSON_input_sources);
        } catch (const std::exception& e) {
            FPS_ERROR_LOG("Failed to parse input_sources array: %s \n", e.what());
            return false;
        }
    }

    // TODO: once frequency response is a subclass of Feature, remove this special configuration function call
    // parse frequency_response object
    cJSON* JSON_frequency_response = cJSON_GetObjectItem(JSON_flat_vars, "frequency_response");
    if (JSON_frequency_response == NULL) {
        FPS_ERROR_LOG("No frequency_response found in variables file.\n");
        return false;
    }
    if (!frequency_response.parse_json_config(JSON_frequency_response, is_primary, &input_sources, default_vals, multi_input_command_vars)) {
        FPS_ERROR_LOG("Failed to parse frequency_response object.");
        return false;
    }

    // parse all Site_Manager-owned variables.json variables
    for (auto& variable_id_pair : variable_ids) {
        cJSON* JSON_variable = cJSON_GetObjectItem(JSON_flat_vars, variable_id_pair.second.c_str());
        if (JSON_variable == NULL) {
            FPS_ERROR_LOG("Could not find variable with ID %s in variables.json", variable_id_pair.second.c_str());
            return false;
        }
        if (!variable_id_pair.first->configure(variable_id_pair.second, is_primary, &input_sources, JSON_variable, default_vals, multi_input_command_vars)) {
            FPS_ERROR_LOG("Failed to parse variable with ID %s", variable_id_pair.second.c_str());
            return false;
        }
    }

    // parse feature-owned variables for all features
    for (auto features_list : {
             &runmode1_kW_features_list,
             &runmode2_kW_features_list,
             &charge_features_list,
             &runmode1_kVAR_features_list,
             &runmode2_kVAR_features_list,
             &standalone_power_features_list,
             &site_operation_features_list,
         }) {
        for (auto feature : *features_list) {
            if (!feature->parse_json_config(JSON_flat_vars, is_primary, &input_sources, default_vals, multi_input_command_vars)) {
                return false;
            }
        }
    }

    cJSON_Delete(JSON_flat_vars);

    // input_source_status string should be programmatically set based on the name of the currently selected input source.
    // initial value of input_source_status in variables.json is not actually used since it is too easy for configurator to forget to match it with the initially selected input source.
    // it is only present to satisfy parser
    input_source_status.value.set(input_sources.get_name_of_input(input_sources.get_selected_input_source_index()));

    return true;
}

/**
 * @brief Parses the organized variables in variables.json into a flattened JSON object with all variables at the same level.
 *
 * @param JSON_object The unflattened JSON variables object
 * @param JSON_flat_vars The JSON object which will receive the resulting flattened variables as cJSON references
 * @throw std::runtime_error if a duplicate variable is encountered
 */
void Site_Manager::parse_flatten_vars(cJSON* JSON_object, cJSON* JSON_flat_vars) {
    // Recursively search through the json and add all variables to JSON_flat_vars
    cJSON* var_candidate;
    cJSON_ArrayForEach(var_candidate, JSON_object) {
        // input_sources is an array of special config data, as opposed to all other variables which follow the same exact format
        // handle input_sources separately
        if (var_candidate == NULL || var_candidate->string == NULL) {
            throw std::runtime_error("Unlabeled object found in variables.json");
        } else if (strcmp(var_candidate->string, "frequency_response") == 0) {
            if (!cJSON_HasObjectItem(JSON_flat_vars, var_candidate->string)) {
                cJSON_AddItemReferenceToObject(JSON_flat_vars, var_candidate->string, var_candidate);
            } else {
                throw std::runtime_error("Duplicate variable " + std::string(var_candidate->string) + " found in variables.json.\n");
            }
        } else if (strcmp(var_candidate->string, "input_sources") == 0) {
            if (!cJSON_HasObjectItem(JSON_flat_vars, var_candidate->string)) {
                cJSON_AddItemReferenceToObject(JSON_flat_vars, var_candidate->string, var_candidate);
            } else {
                throw std::runtime_error("Duplicate variable " + std::string(var_candidate->string) + " found in variables.json.\n");
            }
        } else {
            // Variables are identified as objects without any subobjects as items
            bool hasObjectitem = false;
            cJSON* item;
            cJSON_ArrayForEach(item, var_candidate) {
                if (cJSON_IsObject(item)) {
                    hasObjectitem = true;
                    break;
                }
            }
            if (!hasObjectitem) {
                // Check for duplicates and add the variable to the flattened object
                if (!cJSON_HasObjectItem(JSON_flat_vars, var_candidate->string))
                    cJSON_AddItemReferenceToObject(JSON_flat_vars, var_candidate->string, var_candidate);
                else
                    throw std::runtime_error("Duplicate variable " + std::string(var_candidate->string) + " found in variables.json.\n");
            }
            // If not a variable, it's a category and we recurse
            else {
                try {
                    parse_flatten_vars(var_candidate, JSON_flat_vars);
                }
                // rethrow any exception
                catch (std::exception& ex) {
                    throw;
                }
            }
        }
    }
}

// If the hybridOS controller discover that a message was sent to /features uri this will handle those messages
// example fims message$ ./fims_send -m set -u /features/price '{"value": 100}'
// example fims message$ ./fims_send -m get -u /features -r /anydestination
void Site_Manager::fims_data_parse(fims_message* msg) {
    // verify message is for Site_Manager and mark if the URI is a features or site URI
    uriType uType;
    if (strncmp(msg->pfrags[0], "features", strlen("features")) == 0)
        uType = features_uri;
    else if (strncmp(msg->pfrags[0], "site", strlen("site")) == 0)
        uType = site_uri;
    else {
        FPS_ERROR_LOG("FIMS message passed to Site_Manager::fims_data_parse, but did not begin with /features or /site. \n");
        return;
    }

    if (strcmp(msg->method, "set") == 0) {
        // First ensure the set is valid before echoing it to the storage db module
        bool valid_set = false;
        // Message to writeout for setpoint if valid
        cJSON* body_JSON = cJSON_Parse(msg->body);
        if (body_JSON == NULL) {
            FPS_ERROR_LOG("fims message body is NULL or incorrectly formatted: (%s) \n", msg->body);
            return;
        }
        cJSON* body_value = cJSON_GetObjectItem(body_JSON, "value");
        int body_type = (body_value) ? body_value->type : body_JSON->type;
        float body_float = (body_value) ? body_value->valuedouble : body_JSON->valuedouble;  // float type for body value
        int body_int = (body_value) ? body_value->valueint : body_JSON->valueint;
        bool body_bool = (body_type == cJSON_False) ? false : true;

        // TODO: once all set handling uses the below msg_value, remove the above body_float, body_int, body_bool declarations
        cJSON* p_msg_value = body_value ? body_value : body_JSON;
        if (p_msg_value == NULL) {
            FPS_ERROR_LOG("Failed to parse a value from fims message body: (%s) \n", msg->body);
            return;
        }
        cJSON msg_value = *p_msg_value;

        if (msg->nfrags == 3)  // This is where you would allow user to set parameters
        {
            if (uType == features_uri)  // all /feature sets for features with handler functions here
            {
                if (strncmp(msg->pfrags[1], "active_power", strlen("active_power")) == 0) {
                    for (auto feature : runmode1_kW_features_list) {
                        feature->handle_fims_set(msg->pfrags[2], msg_value);
                    }
                    for (auto feature : runmode2_kW_features_list) {
                        feature->handle_fims_set(msg->pfrags[2], msg_value);
                    }
                    valid_set = true;
                } else if (strncmp(msg->pfrags[1], "reactive_power", strlen("reactive_power")) == 0) {
                    for (auto feature : runmode1_kVAR_features_list) {
                        feature->handle_fims_set(msg->pfrags[2], msg_value);
                    }
                    for (auto feature : runmode2_kVAR_features_list) {
                        feature->handle_fims_set(msg->pfrags[2], msg_value);
                    }
                    valid_set = true;
                } else if (strncmp(msg->pfrags[1], "standalone_power", strlen("standalone_power")) == 0) {
                    for (auto feature : standalone_power_features_list) {
                        feature->handle_fims_set(msg->pfrags[2], msg_value);
                    }
                    valid_set = true;
                } else if (strncmp(msg->pfrags[1], "site_operation", strlen("site_operation")) == 0) {
                    for (auto feature : site_operation_features_list) {
                        feature->handle_fims_set(msg->pfrags[2], msg_value);
                    }
                    valid_set = true;
                }
            }
            // TODO: remove the following if blocks when all features have fims handler functions
            if (body_type == cJSON_Number)  // process all numeric sets here
            {
                if (uType == features_uri)  // all /feature numeric sets here
                {
                    if (strncmp(msg->pfrags[1], "active_power", strlen("active_power")) == 0) {
                        frequency_response.handle_fims_set(*msg);

                        runmode1_kW_mode_cmd.set_fims_masked_int(msg->pfrags[2], body_int, available_runmode1_kW_features_mask);
                        runmode2_kW_mode_cmd.set_fims_masked_int(msg->pfrags[2], body_int, available_runmode2_kW_features_mask);
                        asset_priority_runmode1.set_fims_int(msg->pfrags[2], range_check(body_int, NUM_ASSET_PRIORITIES, 0));
                        asset_priority_runmode2.set_fims_int(msg->pfrags[2], range_check(body_int, NUM_ASSET_PRIORITIES, 0));
                        site_kW_load_interval_ms.set_fims_int(msg->pfrags[2], range_check(body_int, 1, 60000));  // Interval between 1ms and 60s

                        // Manual Mode
                        manual_solar_kW_cmd.set_fims_float(msg->pfrags[2], body_float);
                        manual_ess_kW_cmd.set_fims_float(msg->pfrags[2], body_float);
                        manual_gen_kW_cmd.set_fims_float(msg->pfrags[2], body_float);
                        // Update slew rates
                        if (manual_solar_kW_slew_rate.set_fims_int(msg->pfrags[2], range_check(body_int, 100000000, 1)))
                            manual_solar_kW_slew.set_slew_rate(manual_solar_kW_slew_rate.value.value_int);
                        if (manual_ess_kW_slew_rate.set_fims_int(msg->pfrags[2], range_check(body_int, 100000000, 1)))
                            manual_ess_kW_slew.set_slew_rate(manual_ess_kW_slew_rate.value.value_int);
                        if (manual_gen_kW_slew_rate.set_fims_int(msg->pfrags[2], range_check(body_int, 100000000, 1)))
                            manual_gen_kW_slew.set_slew_rate(manual_gen_kW_slew_rate.value.value_int);

                        // Energy_Arbitrage
                        energy_arb_obj.threshold_charge_1.set_fims_float(msg->pfrags[2], body_float);
                        energy_arb_obj.threshold_charge_2.set_fims_float(msg->pfrags[2], body_float);
                        energy_arb_obj.threshold_dischg_1.set_fims_float(msg->pfrags[2], body_float);
                        energy_arb_obj.threshold_dischg_2.set_fims_float(msg->pfrags[2], body_float);
                        energy_arb_obj.max_charge_1.set_fims_float(msg->pfrags[2], body_float);
                        energy_arb_obj.max_charge_2.set_fims_float(msg->pfrags[2], body_float);
                        energy_arb_obj.max_dischg_1.set_fims_float(msg->pfrags[2], body_float);
                        energy_arb_obj.max_dischg_2.set_fims_float(msg->pfrags[2], body_float);
                        energy_arb_obj.price.set_fims_float(msg->pfrags[2], body_float);

                        // ESS Calibration
                        ess_calibration_min_voltage_limit.set_fims_float(msg->pfrags[2], fabsf(body_float));
                        ess_calibration_max_voltage_limit.set_fims_float(msg->pfrags[2], fabsf(body_float));
                        ess_calibration_kW_cmd.set_fims_float(msg->pfrags[2], body_float);

                        // write if valid % range
                        if (body_float >= 0 && body_float <= 100) {
                            ess_calibration_min_soc_limit.set_fims_float(msg->pfrags[2], body_float);
                            ess_calibration_max_soc_limit.set_fims_float(msg->pfrags[2], body_float);
                            ess_charge_control_target_soc.set_fims_float(msg->pfrags[2], body_float);
                        }

                        // Charge Control
                        ess_charge_control_kW_limit.set_fims_float(msg->pfrags[2], fabsf(body_float));

                        valid_set = true;
                    } else if (strncmp(msg->pfrags[1], "reactive_power", strlen("reactive_power")) == 0) {
                        runmode1_kVAR_mode_cmd.set_fims_masked_int(msg->pfrags[2], body_int, available_runmode1_kVAR_features_mask);
                        reactive_setpoint_kVAR_cmd.set_fims_float(msg->pfrags[2], body_float);
                        reactive_setpoint_kVAR_slew_rate.set_fims_int(msg->pfrags[2], body_int);
                        power_factor_cmd.set_fims_float(msg->pfrags[2], zero_check(body_float > 1 ? 1 : body_float));
                        constant_power_factor_setpoint.set_fims_float(msg->pfrags[2], body_float);
                        constant_power_factor_lagging_limit.set_fims_float(msg->pfrags[2], body_float);
                        constant_power_factor_leading_limit.set_fims_float(msg->pfrags[2], body_float);
                        valid_set = true;
                    } else if (strncmp(msg->pfrags[1], "standalone_power", strlen("standalone_power")) == 0) {
                        // poi limits
                        active_power_poi_limits_max_kW.set_fims_float(msg->pfrags[2], fabsf(body_float));
                        active_power_poi_limits_min_kW.set_fims_float(msg->pfrags[2], -1 * fabsf(body_float));
                        active_power_soc_poi_target_soc.set_fims_float(msg->pfrags[2], fabsf(body_float));
                        active_power_soc_poi_limits_low_min_kW.set_fims_float(msg->pfrags[2], -1 * fabsf(body_float));
                        active_power_soc_poi_limits_low_max_kW.set_fims_float(msg->pfrags[2], fabsf(body_float));
                        active_power_soc_poi_limits_high_min_kW.set_fims_float(msg->pfrags[2], -1 * fabsf(body_float));
                        active_power_soc_poi_limits_high_max_kW.set_fims_float(msg->pfrags[2], fabsf(body_float));
                        // Reactive poi limits
                        reactive_power_poi_limits_min_kVAR.set_fims_float(msg->pfrags[2], -1 * fabsf(body_float));
                        reactive_power_poi_limits_max_kVAR.set_fims_float(msg->pfrags[2], fabsf(body_float));
                        // pfr
                        pfr_offset_hz.set_fims_float(msg->pfrags[2], body_float);
                        pfr_limits_min_kW.set_fims_float(msg->pfrags[2], -1 * fabsf(body_float));
                        pfr_limits_max_kW.set_fims_float(msg->pfrags[2], fabsf(body_float));
                        pfr_droop_percent.set_fims_float(msg->pfrags[2], body_float);
                        pfr_deadband.set_fims_float(msg->pfrags[2], body_float);
                        // ldss
                        ldss_priority_setting.set_fims_int(msg->pfrags[2], body_int);
                        ldss_warmup_time.set_fims_int(msg->pfrags[2], body_int);
                        ldss_cooldown_time.set_fims_int(msg->pfrags[2], body_int);
                        ldss_start_gen_time.set_fims_int(msg->pfrags[2], body_int);
                        ldss_stop_gen_time.set_fims_int(msg->pfrags[2], body_int);
                        ldss_max_load_threshold_percent.set_fims_float(msg->pfrags[2], range_check(body_float, 100.0f, ldss_min_load_threshold_percent.value.value_float));
                        ldss_min_load_threshold_percent.set_fims_float(msg->pfrags[2], range_check(body_float, ldss_max_load_threshold_percent.value.value_float, 0.0f));
                        ldss_max_soc_threshold_percent.set_fims_float(msg->pfrags[2], range_check(body_float, 100.0f, ldss_min_soc_threshold_percent.value.value_float));
                        ldss_min_soc_threshold_percent.set_fims_float(msg->pfrags[2], range_check(body_float, ldss_max_soc_threshold_percent.value.value_float, 0.0f));
                        start_first_gen_soc.set_fims_float(msg->pfrags[2], range_check(body_float, 100.0f, 0.0f));
                        // ess discharge prevention
                        edp_soc.set_fims_float(msg->pfrags[2], range_check(body_float, 100.0f, 0.0f));

                        reactive_power_closed_loop_min_offset.set_fims_int(msg->pfrags[2], -1 * fabs(body_int));
                        reactive_power_closed_loop_max_offset.set_fims_int(msg->pfrags[2], fabs(body_int));
                        reactive_power_closed_loop_step_size_kW.set_fims_float(msg->pfrags[2], fabsf(body_float));
                        reactive_power_closed_loop_steady_state_deadband_kW.set_fims_float(msg->pfrags[2], fabsf(body_float));
                        reactive_power_closed_loop_regulation_deadband_kW.set_fims_float(msg->pfrags[2], fabsf(body_float));
                        valid_set = true;
                    } else if (strncmp(msg->pfrags[1], "site_operation", strlen("site_operation")) == 0) {
                        watchdog_pet.set_fims_int(msg->pfrags[2], fabsf(body_int));
                        // UI configured timer duration
                        watchdog_duration_ms.set_fims_int(msg->pfrags[2], fabsf(body_int));

                        // watchdog_pet only invalid cops writeout endpoint, all others true
                        valid_set = (strcmp(msg->pfrags[2], "watchdog_pet") != 0);
                    }
                } else if (uType == site_uri)  // all numeric sets for /site
                {
                    // addition from NULL
                    if (strncmp(msg->pfrags[1], "cops", strlen("cops")) == 0) {
                        cops_heartbeat.set_fims_int(msg->pfrags[2], body_int);
                    } else if (strncmp(msg->pfrags[1], "configuration", strlen("configuration")) == 0) {
                        reserved_float_1.set_fims_float(msg->pfrags[2], body_float);
                        reserved_float_2.set_fims_float(msg->pfrags[2], body_float);
                        reserved_float_3.set_fims_float(msg->pfrags[2], body_float);
                        reserved_float_4.set_fims_float(msg->pfrags[2], body_float);
                        reserved_float_5.set_fims_float(msg->pfrags[2], body_float);
                        reserved_float_6.set_fims_float(msg->pfrags[2], body_float);
                        reserved_float_7.set_fims_float(msg->pfrags[2], body_float);
                        reserved_float_8.set_fims_float(msg->pfrags[2], body_float);
                        valid_set = true;
                    } else if (strcmp(msg->pfrags[1], "debug") == 0) {
                        if ((strcmp(msg->pfrags[2], "state") == 0) && (static_cast<states>(body_int) != current_state)) {
                            set_state(static_cast<states>(body_int));
                            valid_set = true;
                        }
                    }
                }
            } else if (body_type == cJSON_False || body_type == cJSON_True)  // process all boolean sets here
            {
                if (uType == features_uri)  // all /features boolean sets here
                {
                    if (strncmp(msg->pfrags[1], "active_power", strlen("active_power")) == 0) {
                        frequency_response.handle_fims_set(*msg);
                        ess_calibration_soc_limits_enable.set_fims_bool(msg->pfrags[2], body_bool);
                        ess_calibration_voltage_limits_enable.set_fims_bool(msg->pfrags[2], body_bool);
                        // Charge Control
                        ess_charge_control_charge_disable.set_fims_bool(msg->pfrags[2], body_bool);
                        ess_charge_control_discharge_disable.set_fims_bool(msg->pfrags[2], body_bool);
                        // Charge Dispatch
                        charge_dispatch_solar_enable_flag.set_fims_bool(msg->pfrags[2], body_bool);
                        charge_dispatch_gen_enable_flag.set_fims_bool(msg->pfrags[2], body_bool);
                        charge_dispatch_feeder_enable_flag.set_fims_bool(msg->pfrags[2], body_bool);
                        valid_set = true;
                    } else if (strncmp(msg->pfrags[1], "reactive_power", strlen("reactive_power")) == 0) {
                        constant_power_factor_absolute_mode.set_fims_bool(msg->pfrags[2], body_bool);
                        constant_power_factor_lagging_direction.set_fims_bool(msg->pfrags[2], body_bool);
                    } else if (strncmp(msg->pfrags[1], "standalone_power", strlen("standalone_power")) == 0) {
                        // Feature enable flags
                        for (auto feature : standalone_power_features_list)
                            if (feature->available)
                                feature->enable_flag.set_fims_bool(msg->pfrags[2], body_bool);
                        // Additional feature options
                        active_power_soc_poi_limits_enable.set_fims_bool(msg->pfrags[2], body_bool);
                        valid_set = true;
                    } else if (strncmp(msg->pfrags[1], "site_operation", strlen("site_operation")) == 0) {
                        // Feature enable flags
                        for (auto feature : site_operation_features_list)
                            if (feature->available)
                                feature->enable_flag.set_fims_bool(msg->pfrags[2], body_bool);

                        power_priority_flag.set_fims_bool(msg->pfrags[2], body_bool);
                        allow_ess_auto_restart.set_fims_bool(msg->pfrags[2], body_bool);
                        allow_solar_auto_restart.set_fims_bool(msg->pfrags[2], body_bool);
                        allow_gen_auto_restart.set_fims_bool(msg->pfrags[2], body_bool);
                        valid_set = true;
                    }
                } else if (uType == site_uri)  // all /site boolean set here
                {
                    if ((strncmp(msg->pfrags[1], "operation", strlen("operation")) == 0)) {
                        // Check is_primary status (not a Fims_Object, set here)
                        if (strcmp(msg->pfrags[2], "primary_controller") == 0) {
                            *is_primary = body_bool;

                            emit_event("Site", "Assumed primary controller", INFO_ALERT);
                        }
                        // Special endpoint indicating there has been an update in DBI settings
                        // This endpoint is only used if Site_Controller starts before COPS and missed DBI changes after its configuration
                        // Else the call from COPS is missed here but DBI updates will be read when this controller first configures
                        else if (strcmp(msg->pfrags[2], "dbi_update") == 0) {
                            // Wait for DBI to update before requesting the new data
                            sleep(1);
                            if (!data_endpoint->setpoint_readin()) {
                                FPS_ERROR_LOG("Site_Manager failed to read in latest configuration data from primary");
                                emit_event("Site", "Failed to read in new configuration from primary controller after failover event", ALARM_ALERT);
                                return;
                            }
                            emit_event("Site", "Read new configuration from primary controller after failover event", INFO_ALERT);
                        } else {
                            enable_flag.set_fims_bool(msg->pfrags[2], body_bool);
                            disable_flag.set_fims_bool(msg->pfrags[2], body_bool);
                            standby_flag.set_fims_bool(msg->pfrags[2], body_bool);
                            clear_faults_flag.set_fims_bool(msg->pfrags[2], body_bool);
                            // Support everything except clear faults
                            if (strncmp(msg->pfrags[2], "clear_faults_flag", strlen("clear_faults_flag")) != 0)
                                valid_set = true;
                        }
                    } else if (strncmp(msg->pfrags[1], "input_sources", strlen("input_sources")) == 0) {
                        std::string new_selected_input = input_sources.set_source_enable_flag(msg->pfrags[2], body_bool);
                        input_source_status.value.set(new_selected_input);
                        valid_set = true;
                    } else if ((strncmp(msg->pfrags[1], "configuration", strlen("configuration")) == 0)) {
                        invert_poi_kW.set_fims_bool(msg->pfrags[2], body_bool);
                        reserved_bool_1.set_fims_bool(msg->pfrags[2], body_bool);
                        reserved_bool_2.set_fims_bool(msg->pfrags[2], body_bool);
                        reserved_bool_3.set_fims_bool(msg->pfrags[2], body_bool);
                        reserved_bool_4.set_fims_bool(msg->pfrags[2], body_bool);
                        reserved_bool_5.set_fims_bool(msg->pfrags[2], body_bool);
                        reserved_bool_6.set_fims_bool(msg->pfrags[2], body_bool);
                        reserved_bool_7.set_fims_bool(msg->pfrags[2], body_bool);
                        reserved_bool_8.set_fims_bool(msg->pfrags[2], body_bool);
                        reserved_bool_9.set_fims_bool(msg->pfrags[2], body_bool);
                        reserved_bool_10.set_fims_bool(msg->pfrags[2], body_bool);
                        reserved_bool_11.set_fims_bool(msg->pfrags[2], body_bool);
                        reserved_bool_12.set_fims_bool(msg->pfrags[2], body_bool);
                        reserved_bool_13.set_fims_bool(msg->pfrags[2], body_bool);
                        reserved_bool_14.set_fims_bool(msg->pfrags[2], body_bool);
                        reserved_bool_15.set_fims_bool(msg->pfrags[2], body_bool);
                        reserved_bool_16.set_fims_bool(msg->pfrags[2], body_bool);
                        valid_set = true;
                    } else if ((strncmp(msg->pfrags[1], "debug", strlen("debug")) == 0)) {
                        if (strcmp(msg->pfrags[2], "fault") == 0) {
                            if (body_bool)
                                set_faults(0);
                            else
                                clear_faults();
                        }
                    }
                }
            } else
                FPS_WARNING_LOG("features set message body type not expected. type: %d \n", body_type);
        } else
            FPS_WARNING_LOG("site manager set message error, null body found");

        // Write out if the set matched one of the above categories
        if (valid_set) {
            std::string uri = "/" + std::string(msg->pfrags[0]);
            // All site/feature settings that are valid are persistent currently
            data_endpoint->setpoint_writeout(uri, msg->pfrags[2], &body_JSON);
        }
        cJSON_Delete(body_JSON);
    } else if (strcmp(msg->method, "get") == 0) {
        if (msg->replyto == NULL)
            FPS_ERROR_LOG("site manager get message error, no replyto\n");
        else if (msg->nfrags == 1 || msg->nfrags == 2 || msg->nfrags == 3)
            send_FIMS("set", msg->replyto, NULL, msg->uri);
        else
            FPS_ERROR_LOG("site manager get message error, invalid URI\n");
    } else {
        // method not currently supported, could be post or del
        if (msg->replyto != NULL)
            pFims->Send("set", msg->replyto, NULL, "Error, unsupported method for URI.");
    }
}

void Site_Manager::send_FIMS(const char* method, const char* uri, const char* replyto, const char* body) {
    // Clear buffer for use
    send_FIMS_buf.clear();

    // Disable publish if second controller (shadow mode)
    // Only allow the heartbeat response to be sent to COPS
    if (!*is_primary && strcmp(uri, "/cops/heartbeat/site_controller") != 0)
        return;

    // borrowed from libfims, get URI segments
    const char** pfrags = NULL;
    unsigned int nfrags = 0;
    int count = 0;
    int offset[MAX_URI_DEPTH];
    for (int i = 0; body[i] != '\0' && count < MAX_URI_DEPTH; i++) {
        if (body[i] == '/') {
            offset[count] = i;
            count++;
        }
    }
    nfrags = count;
    if (count > 0 && count < MAX_URI_DEPTH)
        pfrags = new const char*[count];
    else {
        FPS_ERROR_LOG("Invalid number of segments in site controller URI\n");
        return;
    }
    for (int i = 0; i < count; i++)
        pfrags[i] = body + (offset[i] + 1);

    if (nfrags == 1)  // get 1 fragment object
    {
        if (strncmp(pfrags[0], "features", strlen("features")) == 0)
            build_JSON_features(send_FIMS_buf);
        else if (strncmp(pfrags[0], "site", strlen("site")) == 0)
            build_JSON_site(send_FIMS_buf);
    } else if (nfrags == 2 || nfrags == 3)  // get 2 or 3 fragment object
    {
        // If 3 fragments, only generate the desired single variable
        const char* var = NULL;
        if (nfrags == 3)
            var = pfrags[2];

        if (strncmp(pfrags[0], "features", strlen("features")) == 0) {
            if (strncmp(pfrags[1], "summary", strlen("summary")) == 0)
                // Build naked json
                build_JSON_features_summary(send_FIMS_buf, false, var);
            else if ((strncmp(pfrags[1], "active_power", strlen("active_power")) == 0))
                build_JSON_features_active_power(send_FIMS_buf, var);
            else if ((strncmp(pfrags[1], "reactive_power", strlen("reactive_power")) == 0))
                build_JSON_features_reactive_power(send_FIMS_buf, var);
            else if ((strncmp(pfrags[1], "standalone_power", strlen("standalone_power")) == 0))
                build_JSON_features_standalone_power(send_FIMS_buf, var);
            else if ((strncmp(pfrags[1], "site_operation", strlen("site_operation")) == 0))
                build_JSON_features_site_operation(send_FIMS_buf, var);
        } else if (strncmp(pfrags[0], "site", strlen("site")) == 0) {
            if (strncmp(pfrags[1], "summary", strlen("summary")) == 0)
                // build naked json
                build_JSON_site_summary(send_FIMS_buf, false, var);
            else if (strncmp(pfrags[1], "operation", strlen("operation")) == 0)
                build_JSON_site_operation(send_FIMS_buf, var);
            else if (strncmp(pfrags[1], "configuration", strlen("configuration")) == 0)
                build_JSON_site_configuration(send_FIMS_buf, var);
            else if (strncmp(pfrags[1], "cops", strlen("cops")) == 0)
                build_JSON_site_cops(send_FIMS_buf, var);
            else if (strncmp(pfrags[1], "input_sources", strlen("input_sources")) == 0)
                build_JSON_site_input_sources(send_FIMS_buf, var);
        }
    }
    bufJSON_RemoveTrailingComma(send_FIMS_buf);
    std::string string_body = to_string(send_FIMS_buf);

    delete[] pfrags;

    if (string_body.empty())  // no match, print error and return
    {
        FPS_ERROR_LOG("Site Manager FIMS message error - URI not matched %s\n", body);
        return;
    }

    // send fims message
    pFims->Send(method, uri, replyto, string_body.c_str());
}

void Site_Manager::publish_all_FIMS() {
    send_FIMS("pub", "/site/summary", NULL, "/site/summary");
    send_FIMS("pub", "/site/operation", NULL, "/site/operation");
    send_FIMS("pub", "/site/configuration", NULL, "/site/configuration");
    send_FIMS("pub", "/site/input_sources", NULL, "/site/input_sources");
    send_FIMS("pub", "/features/summary", NULL, "/features/summary");
    send_FIMS("pub", "/features/active_power", NULL, "/features/active_power");
    send_FIMS("pub", "/features/reactive_power", NULL, "/features/reactive_power");
    send_FIMS("pub", "/features/standalone_power", NULL, "/features/standalone_power");
    send_FIMS("pub", "/features/site_operation", NULL, "/features/site_operation");
}

// Get all interface variables (from asset manager, FIMS).
void Site_Manager::get_values() {
    FPS_DEBUG_LOG("Getting Asset Manager Values; Site_Manager::get_values.\n");

    // update clock for any timers
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    // update site/feature control variables based on values received from local/remote source
    read_site_feature_controls();

    // configure which UI buttons are enabled and update variables based on values received from UI
    update_power_feature_selections();

    // update any variables as needed
    max_potential_ess_kW.value.set(pAssets->get_ess_total_max_potential_active_power());
    min_potential_ess_kW.value.set(pAssets->get_ess_total_min_potential_active_power());
    max_potential_gen_kW.value.set(pAssets->get_gen_total_max_potential_active_power());
    min_potential_gen_kW.value.set(pAssets->get_gen_total_min_potential_active_power());
    max_potential_solar_kW.value.set(pAssets->get_solar_total_max_potential_active_power());
    min_potential_solar_kW.value.set(pAssets->get_solar_total_min_potential_active_power());
    max_potential_feeder_kW.value.set(pAssets->get_poi_max_potential_active_power());
    min_potential_feeder_kW.value.set(pAssets->get_poi_min_potential_active_power());
    site_frequency.value.set(pAssets->get_poi_gridside_frequency());

    rated_ess_kW.value.set(pAssets->get_ess_total_rated_active_power());
    rated_feeder_kW.value.set(pAssets->get_poi_nameplate_active_power());
    rated_gen_kW.value.set(pAssets->get_gen_total_active_power());
    rated_solar_kW.value.set(pAssets->get_solar_total_rated_active_power());

    total_site_kW_rated_charge.value.set(-1.0f * rated_ess_kW.value.value_float);
    total_site_kW_rated_discharge.value.set(rated_ess_kW.value.value_float + rated_gen_kW.value.value_float + rated_solar_kW.value.value_float);
    total_site_kVAR_rated_charge = -1.0f * pAssets->get_ess_total_rated_reactive_power();
    // TODO: add gen power to calculation when it supports reactive power production from features
    total_site_kVAR_rated_discharge = pAssets->get_ess_total_rated_reactive_power() + pAssets->get_solar_total_rated_reactive_power();
    total_asset_kW_charge_limit = pAssets->get_total_kW_charge_limit();
    total_asset_kW_discharge_limit = pAssets->get_total_kW_discharge_limit();
    total_site_kW_charge_limit.value.set(total_asset_kW_charge_limit);
    total_site_kW_discharge_limit.value.set(total_asset_kW_discharge_limit);

    potential_ess_kVAR.value.set(pAssets->get_ess_total_potential_reactive_power());
    potential_gen_kVAR.value.set(pAssets->get_gen_total_potential_reactive_power());
    potential_solar_kVAR.value.set(pAssets->get_solar_total_potential_reactive_power());

    reactive_setpoint_kVAR_cmd_slew.set_slew_rate(reactive_setpoint_kVAR_slew_rate.value.value_int);

    // Update LDSS values
    set_ldss_variables();

    // update load/solar shedder values
    load_shed_spare_ess_kw.value.set(pAssets->get_ess_total_dischargeable_power_kW() - pAssets->get_ess_total_active_power());
    solar_shed_spare_ess_kw.value.set(pAssets->get_ess_total_chargeable_power_kW() - zero_check(-1 * pAssets->get_ess_total_active_power()));

    soc_min_all.value.set(pAssets->get_all_ess_soc_min());
    soc_max_all.value.set(pAssets->get_all_ess_soc_max());
    soc_avg_all.value.set(pAssets->get_all_ess_soc_avg());
    soc_min_running.value.set(pAssets->get_ess_soc_min());
    soc_max_running.value.set(pAssets->get_ess_soc_max());
    soc_avg_running.value.set(pAssets->get_ess_soc_avg());

    ess_actual_kW.value.set(pAssets->get_ess_total_active_power());
    gen_actual_kW.value.set(pAssets->get_gen_total_active_power());
    solar_actual_kW.value.set(pAssets->get_solar_total_active_power());
    feeder_actual_kW.value.set((invert_poi_kW.value.value_bool ? -1.0f : 1.0f) * pAssets->get_feeder_active_power(pAssets->get_poi_id().c_str()));

    ess_actual_kVAR.value.set(pAssets->get_ess_total_reactive_power());
    gen_actual_kVAR.value.set(pAssets->get_gen_total_reactive_power());
    solar_actual_kVAR.value.set(pAssets->get_solar_total_reactive_power());
    // Invert using the same flag as active power
    feeder_actual_kVAR.value.set((invert_poi_kW.value.value_bool ? -1.0f : 1.0f) * pAssets->get_feeder_reactive_power(pAssets->get_poi_id().c_str()));
    feeder_actual_pf.value.set(pAssets->get_poi_power_factor());

    // frequency response values needed
    pfr_nameplate_kW.value.set(pAssets->get_ess_total_nameplate_active_power());

    // active voltage variables set
    active_voltage_regulation.actual_volts.value.set(pAssets->get_poi_gridside_avg_voltage());
    active_voltage_regulation.rated_kVAR.value.set(pAssets->get_ess_total_nameplate_reactive_power() + pAssets->get_solar_total_nameplate_reactive_power());

    num_ess_available = pAssets->get_num_ess_avail();
    num_ess_running = pAssets->get_num_ess_running();
    num_ess_controllable = pAssets->get_num_ess_controllable();
    num_solar_available = pAssets->get_num_solar_avail();
    num_solar_running = pAssets->get_num_solar_running();
    num_gen_available = pAssets->get_num_gen_avail();
    num_gen_running = pAssets->get_num_gen_running();
    num_gen_active = pAssets->get_num_gen_controllable();

    // update asset_cmd w external variables for state machine
    asset_cmd.additional_load_compensation = 0.0f;
    asset_cmd.poi_cmd = 0.0f;
    asset_cmd.ess_data.actual_kW = ess_actual_kW.value.value_float;
    asset_cmd.feeder_data.actual_kW = feeder_actual_kW.value.value_float;
    asset_cmd.gen_data.actual_kW = gen_actual_kW.value.value_float;
    asset_cmd.solar_data.actual_kW = solar_actual_kW.value.value_float;

    asset_cmd.ess_data.actual_kVAR = ess_actual_kVAR.value.value_float;
    asset_cmd.gen_data.actual_kVAR = gen_actual_kVAR.value.value_float;
    asset_cmd.solar_data.actual_kVAR = solar_actual_kVAR.value.value_float;

    asset_cmd.ess_data.max_potential_kW = max_potential_ess_kW.value.value_float;
    asset_cmd.feeder_data.max_potential_kW = max_potential_feeder_kW.value.value_float;
    asset_cmd.gen_data.max_potential_kW = max_potential_gen_kW.value.value_float;
    asset_cmd.solar_data.max_potential_kW = max_potential_solar_kW.value.value_float;

    asset_cmd.ess_data.min_potential_kW = min_potential_ess_kW.value.value_float;
    asset_cmd.feeder_data.min_potential_kW = min_potential_feeder_kW.value.value_float;
    asset_cmd.gen_data.min_potential_kW = min_potential_gen_kW.value.value_float;
    asset_cmd.solar_data.min_potential_kW = min_potential_solar_kW.value.value_float;

    asset_cmd.ess_data.potential_kVAR = potential_ess_kVAR.value.value_float;
    asset_cmd.gen_data.potential_kVAR = potential_gen_kVAR.value.value_float;
    asset_cmd.solar_data.potential_kVAR = potential_solar_kVAR.value.value_float;

    asset_cmd.ess_data.start_first_kW = start_first_ess_kW.value.value_float;
    asset_cmd.gen_data.start_first_kW = start_first_gen_kW.value.value_float;
    asset_cmd.solar_data.start_first_kW = start_first_solar_kW.value.value_float;

    asset_cmd.ess_data.auto_restart_flag = allow_ess_auto_restart.value.value_bool;
    asset_cmd.gen_data.auto_restart_flag = allow_gen_auto_restart.value.value_bool;
    asset_cmd.solar_data.auto_restart_flag = allow_solar_auto_restart.value.value_bool;

    // reset all asset kW and kVAR cmds to 0
    asset_cmd.reset_kW_dispatch();
    asset_cmd.reset_kVAR_dispatch();

    // calculate total potential kVAR
    asset_cmd.calculate_total_potential_kVAR();

    // calculate site load and write it to UI
    asset_cmd.calculate_site_kW_load();
    site_kW_load_inclusion.value.set(false);
    site_kW_load.value.set(asset_cmd.site_kW_load);

    // Reset CLC correction reporting vars
    active_power_closed_loop.total_correction.value.set(0.0f);
    reactive_power_closed_loop_total_correction.value.set(0.0f);

    // KPI values
    update_ess_kpi_values();

    get_ess_calibration_variables();

    // bidirectional function call (get charge_control_kW_request and send target_soc to algorithm)
    ess_charge_control_kW_request.value.set(pAssets->charge_control(ess_charge_control_target_soc.value.value_float, ess_charge_control_charge_disable.value.value_bool, ess_charge_control_discharge_disable.value.value_bool));
}

// set all interface variables (to asset manager, FIMS)
void Site_Manager::set_values() {
    FPS_DEBUG_LOG("Setting Asset Manager Values; Site_Manager::set_values.\n");

    // if Clear Faults button was pressed a second ago, clear the flags now. Delayed to give Asset Manager time to clear component faults
    if (clear_fault_status_flags && check_expired_time(current_time, time_to_clear_fault_status_flags)) {
        clear_fault_registers();
    }

    // if Asset Manager cleared asset faults, clear them from site alarms
    if ((num_path_faults == 0) && (active_alarm_array[2] == true)) {
        clear_alarms(2);
    }

    // if Asset Manager cleared asset alarms, clear them from site alarms
    if ((num_path_alarms == 0) && (active_alarm_array[1] == true)) {
        clear_alarms(1);
    }

    // update asset slew instances
    ess_kVAR_cmd_slew.update_slew_target(ess_kVAR_cmd.value.value_float);
    gen_kVAR_cmd_slew.update_slew_target(gen_kVAR_cmd.value.value_float);
    solar_kVAR_cmd_slew.update_slew_target(solar_kVAR_cmd.value.value_float);
    remove_active_poi_corrections_from_slew_targets();
    remove_reactive_poi_corrections_from_slew_targets();
    manual_solar_kW_slew.update_slew_target(solar_kW_cmd.value.value_float);
    manual_ess_kW_slew.update_slew_target(ess_kW_cmd.value.value_float);
    manual_gen_kW_slew.update_slew_target(gen_kW_cmd.value.value_float);

    pAssets->set_reactive_power_priority(power_priority_flag.value.value_bool);

    // if any command variables have changed, write them to Asset Manager
    pAssets->start_first_gen(asset_cmd.gen_data.start_first_flag);

    if (prev_ess_kW_cmd != ess_kW_cmd.value.value_float) {
        pAssets->set_ess_target_active_power(ess_kW_cmd.value.value_float);
        prev_ess_kW_cmd = ess_kW_cmd.value.value_float;
    }

    FPS_DEBUG_LOG("Site_Manager::set_values ess_kW_cmd: %f\n", ess_kW_cmd.value.value_float);

    if (prev_gen_kW_cmd != gen_kW_cmd.value.value_float) {
        pAssets->set_gen_target_active_power(gen_kW_cmd.value.value_float);
        prev_gen_kW_cmd = gen_kW_cmd.value.value_float;
    }
    if (prev_solar_kW_cmd != solar_kW_cmd.value.value_float) {
        pAssets->set_solar_target_active_power(solar_kW_cmd.value.value_float);
        prev_solar_kW_cmd = solar_kW_cmd.value.value_float;
    }
    if (prev_feeder_kW_cmd != feeder_kW_cmd.value.value_float) {
        pAssets->set_poi_target_active_power(feeder_kW_cmd.value.value_float);
        prev_feeder_kW_cmd = feeder_kW_cmd.value.value_float;
    }
    if (prev_ess_kVAR_cmd != ess_kVAR_cmd.value.value_float) {
        pAssets->set_ess_target_reactive_power(ess_kVAR_cmd.value.value_float);
        prev_ess_kVAR_cmd = ess_kVAR_cmd.value.value_float;
    }
    if (prev_gen_kVAR_cmd != gen_kVAR_cmd.value.value_float) {
        pAssets->set_gen_target_reactive_power(gen_kVAR_cmd.value.value_float);
        prev_gen_kVAR_cmd = gen_kVAR_cmd.value.value_float;
    }
    if (prev_solar_kVAR_cmd != solar_kVAR_cmd.value.value_float) {
        pAssets->set_solar_target_reactive_power(solar_kVAR_cmd.value.value_float);
        prev_solar_kVAR_cmd = solar_kVAR_cmd.value.value_float;
    }
    if (prev_asset_pf_cmd != power_factor_cmd.value.value_float) {
        pAssets->set_ess_target_power_factor(power_factor_cmd.value.value_float);
        pAssets->set_solar_target_power_factor(power_factor_cmd.value.value_float);
        prev_asset_pf_cmd = power_factor_cmd.value.value_float;
    }
    if (prev_asset_pf_flag != asset_pf_flag) {
        prev_asset_pf_flag = asset_pf_flag;
        if (asset_pf_flag) {
            pAssets->set_ess_pwr_factor_mode();
            pAssets->set_solar_pwr_factor_mode();
        } else {
            pAssets->set_ess_reactive_kvar_mode();
            pAssets->set_solar_reactive_kvar_mode();
        }
    }

    // update active faults and alarms before fims publish
    build_active_faults();
    build_active_alarms();

    // update vars as needed
    active_voltage_regulation.status_flag.value.set(false);
    load_shed_increase_display_timer.value.set(load_shed_calculator.get_increase_display_timer());
    load_shed_decrease_display_timer.value.set(load_shed_calculator.get_decrease_display_timer());
    solar_shed_increase_display_timer.value.set(solar_shed_calculator.get_increase_display_timer());
    solar_shed_decrease_display_timer.value.set(solar_shed_calculator.get_decrease_display_timer());

    set_ess_calibration_variables();
    // Update local variables used in UI reporting based on changes from RunMode
    set_asset_cmd_variables();

    // Set/clear ui_enable flags
    ui_configuration();
}

/**
 * In the power pipeline, the RunMode will modify it's local copies of Site Manager variables
 * Ensure these changes are captured in Site Manager as well for accurate reporting on Fims and in the UI
 * This function is used for variables whose final values can be captured at the end of the power pipeline
 */
void Site_Manager::set_asset_cmd_variables() {
    min_potential_ess_kW.value.set(asset_cmd.ess_data.min_potential_kW);
    max_potential_ess_kW.value.set(asset_cmd.ess_data.max_potential_kW);

    min_potential_feeder_kW.value.set(asset_cmd.feeder_data.min_potential_kW);
    max_potential_feeder_kW.value.set(asset_cmd.feeder_data.max_potential_kW);

    min_potential_gen_kW.value.set(asset_cmd.gen_data.min_potential_kW);
    max_potential_gen_kW.value.set(asset_cmd.gen_data.max_potential_kW);

    min_potential_solar_kW.value.set(asset_cmd.solar_data.min_potential_kW);
    max_potential_solar_kW.value.set(asset_cmd.solar_data.max_potential_kW);

    site_kW_load_inclusion.value.set(asset_cmd.get_site_kW_load_inclusion());
    site_kW_demand.value.set(asset_cmd.site_kW_demand);
    site_kVAR_demand.value.set(asset_cmd.site_kVAR_demand);
}

/**
 * In the power pipeline, the RunMode will modify it's local copies of Site Manager variables
 * Ensure these changes are captured in Site Manager as well for accurate reporting on Fims and in the UI
 * This function is used for variables whose final values can only be preserved at a specific time before they are modified
 */
void Site_Manager::set_volatile_asset_cmd_variables() {
    feature_kW_demand.value.set(asset_cmd.feature_kW_demand);
    site_kW_charge_production.value.set(asset_cmd.site_kW_charge_production);
    site_kW_discharge_production.value.set(asset_cmd.site_kW_discharge_production);
}

/**
 * The output of standalone features will be included in the asset commands. In the case of POI correction features,
 * this will throw off the slew target of active power features that utilize their own slew rates.
 *
 * Remove the corrections instead to get a clean slew target that's also limited by asset potentials. This function
 * handles the following active power feature slews: Export Target
 * by removing the following POI corrections: Active Power Closed Loop Control, Watt-Watt
 */
void Site_Manager::remove_active_poi_corrections_from_slew_targets() {
    // TODO: fix for mixed asset commands
    // i.e. a -10MW charge cmd could produce asset cmds of -10MW ESS, +10MW Solar, resulting in a target of 0MW
    // We would only get passed this with a sufficient slew rate that can reach -10MW from net 0MW
    float slew_target = ess_kW_cmd.value.value_float + solar_kW_cmd.value.value_float + gen_kW_cmd.value.value_float;
    // Remove corrections in reverse of the order they were dispatched
    if (watt_watt.enable_flag.value.value_bool)
        slew_target -= watt_watt_correction;
    if (active_power_closed_loop.enable_flag.value.value_bool)
        slew_target -= active_power_closed_loop.total_correction.value.value_float;
    // Reset feature slews
    active_power_setpoint_mode.kW_slew.update_slew_target(slew_target);
}

/**
 * The output of standalone features will be included in the asset commands. In the case of POI correction features,
 * this will throw off the slew target of reactive power features that utilize their own slew rates.
 *
 * Remove the corrections instead to get a clean slew target that's also limited by asset potentials. This function
 * handles the following reactive power feature slews: Reactive Setpoint
 * by removing the following POI corrections: Reactive Power Closed Loop Control
 */
void Site_Manager::remove_reactive_poi_corrections_from_slew_targets() {
    // Start the aggregate of site production
    // Gen and feeder are not supported by reactive setpoint currently
    float slew_target = ess_kVAR_cmd.value.value_float + solar_kVAR_cmd.value.value_float;
    if (reactive_power_closed_loop.enable_flag.value.value_bool)
        slew_target -= reactive_power_closed_loop_total_correction.value.value_float;
    // Reset feature slews
    reactive_setpoint_kVAR_cmd_slew.update_slew_target(slew_target);
}

/**
 * Calculate the final site production limits to be reported
 */
void Site_Manager::calculate_total_site_kW_limits() {
    total_site_kW_charge_limit.value.set(std::max(total_site_kW_charge_limit.value.value_float, total_asset_kW_charge_limit));
    total_site_kW_discharge_limit.value.set(std::min(total_site_kW_discharge_limit.value.value_float, total_asset_kW_discharge_limit));
}

// set/clear ui_enable flags
void Site_Manager::ui_configuration(void) {
    // site disabled
    if (disable_flag.value.value_bool) {
        enable_flag.ui_enabled = false;
        set_enable_flag(false);
    } else {
        // hide start button if not in ready state
        enable_flag.ui_enabled = (current_state == Ready);
        // Manually set the enable flag false for all states except Init
        // This allows a persistent settings set to be received on startup but all other redundant enable sets to be ignored
        // In most cases a persistent settings start should be delayed 10 seconds, but this provides additional tolerance if it's sent too early
        set_enable_flag(enable_flag.value.value_bool && current_state == Init);
    }

    disable_flag.ui_enabled = true;
    runmode1_kW_mode_cmd.ui_enabled = true;
    runmode1_kVAR_mode_cmd.ui_enabled = true;
    runmode2_kW_mode_cmd.ui_enabled = true;
    runmode2_kVAR_mode_cmd.ui_enabled = true;
    active_power_poi_limits.enable_flag.ui_enabled = true;
    watchdog_feature.enable_flag.ui_enabled = true;
    watchdog_duration_ms.ui_enabled = watchdog_feature.enable_flag.value.value_bool;
    ess_charge_control_kW_limit.ui_enabled = true;

    // disable clear faults button if no faults or alarms
    clear_faults_flag.ui_enabled = fault_status_flag.value.value_bool || alarm_status_flag.value.value_bool || num_path_faults != 0 || num_path_alarms != 0;

    // charge features - vars enabled if feature is enabled
    for (auto feature : charge_features_list) {
        feature->toggle_ui_enabled(feature->enable_flag.value.value_bool);
    }
    // standalone power features - vars enabled if feature is enabled
    for (auto feature : standalone_power_features_list) {
        // One-off logic to invert POI limits based on its subfeature, soc-based limits
        if (feature == &active_power_poi_limits) {
            // For the main POI limits feature, enable its variables based on the enable status of the feature and the inverse of soc-based limits enable status
            bool poi_limits_enabled_status = feature->enable_flag.value.value_bool && !active_power_soc_poi_limits_enable.value.value_bool;
            // For soc-based POI limits, enable its variables based on the enable status of the feature and the subfeature enable status
            bool soc_limits_enabled_status = feature->enable_flag.value.value_bool && active_power_soc_poi_limits_enable.value.value_bool;

            // Apply to all variables in the feature
            feature->toggle_ui_enabled(poi_limits_enabled_status);
            // Unique behavior for sub feature control: if feature is enabled it will always be enabled
            active_power_soc_poi_limits_enable.ui_enabled = feature->enable_flag.value.value_bool;
            // Update soc-based subfeature to mirror it's enabled status
            active_power_soc_poi_target_soc.ui_enabled = soc_limits_enabled_status;
            active_power_soc_poi_limits_low_min_kW.ui_enabled = soc_limits_enabled_status;
            active_power_soc_poi_limits_low_max_kW.ui_enabled = soc_limits_enabled_status;
            active_power_soc_poi_limits_high_min_kW.ui_enabled = soc_limits_enabled_status;
            active_power_soc_poi_limits_high_max_kW.ui_enabled = soc_limits_enabled_status;
        }
        // Default behavior for other features
        else
            feature->toggle_ui_enabled(feature->enable_flag.value.value_bool);
    }
    // site operation features - vars enabled if feature is enabled
    for (auto feature : site_operation_features_list) {
        feature->toggle_ui_enabled(feature->enable_flag.value.value_bool);
    }
}

// update values of site/feature controls
void Site_Manager::read_site_feature_controls() {
    // get the index of the highest-priority input that is enabled
    uint input_selection = input_sources.get_selected_input_source_index();

    // update all multiple-input command variables with the proper input selection
    for (auto& var : multi_input_command_vars) {
        var->update_present_register(input_selection);
    }

    // any input is allowed to raise the clear faults flag, even if it is not the currently selected input
    for (auto& input_flag : clear_faults_flag.inputs) {
        if (input_flag.value_bool) {
            clear_faults_flag.value.set(true);
            break;
        }
    }
}

// any configuration related to UI display
void Site_Manager::update_power_feature_selections() {
    // if runmode1 active power feature command has changed, switch runmode1 active power features
    if (current_runmode1_kW_feature != runmode1_kW_mode_cmd.value.value_int) {
        // turn off old gc active power feature unless this is first iteration or Disabled
        if (current_runmode1_kW_feature >= 0 && current_runmode1_kW_feature < (int)runmode1_kW_features_list.size())
            runmode1_kW_features_list[current_runmode1_kW_feature]->enable_flag.value.set(false);
        // turn on new gc active power feature (unless Disabled selected)
        if (runmode1_kW_mode_cmd.value.value_int < (int)runmode1_kW_features_list.size())
            runmode1_kW_features_list[runmode1_kW_mode_cmd.value.value_int]->enable_flag.value.set(true);
        current_runmode1_kW_feature = runmode1_kW_mode_cmd.value.value_int;
        // set ui_enabled to false for all feature vars of all gc active power features
        ui_disable_feature_group_feature_vars(runmode1_kW_features_list);
        // set ui_enabled to true for all feature vars of chosen gc active power feature (unless Disabled selected)
        if (runmode1_kW_mode_cmd.value.value_int < (int)runmode1_kW_features_list.size())
            runmode1_kW_features_list[current_runmode1_kW_feature]->toggle_ui_enabled(true);

        // Set charge control enable status if utilized by the feature based on the active power charge control mask
        bool charge_control_enabled = (runmode1_kW_features_charge_control_mask) & (((uint64_t)1) << runmode1_kW_mode_cmd.value.value_int);
        charge_control.enable_flag.value.set(charge_control_enabled);
        charge_control.toggle_ui_enabled(charge_control_enabled);
    }
    // One-off logic to disable soc and voltage based limits variables unless their enable flags are true
    if (runmode1_kW_features_list[current_runmode1_kW_feature] == &ess_calibration) {
        bool soc_limits_enabled = ess_calibration.enable_flag.value.value_bool && ess_calibration_soc_limits_enable.value.value_bool;
        ess_calibration_min_soc_limit.ui_enabled = soc_limits_enabled;
        ess_calibration_max_soc_limit.ui_enabled = soc_limits_enabled;
        bool voltage_limits_enabled = ess_calibration.enable_flag.value.value_bool && ess_calibration_voltage_limits_enable.value.value_bool;
        ess_calibration_min_voltage_limit.ui_enabled = voltage_limits_enabled;
        ess_calibration_max_voltage_limit.ui_enabled = voltage_limits_enabled;
    }

    // if runmode2 active power feature command has changed, switch runmode2 active power features
    if (current_runmode2_kW_feature != runmode2_kW_mode_cmd.value.value_int) {
        // turn off old runmode2 active power feature unless this is first iteration or Disabled
        if (current_runmode2_kW_feature >= 0 && current_runmode2_kW_feature < (int)runmode2_kW_features_list.size())
            runmode2_kW_features_list[current_runmode2_kW_feature]->enable_flag.value.set(false);
        // turn on new runmode2 active power feature (unless Disabled selected)
        if (runmode2_kW_mode_cmd.value.value_int < (int)runmode2_kW_features_list.size())
            runmode2_kW_features_list[runmode2_kW_mode_cmd.value.value_int]->enable_flag.value.set(true);
        current_runmode2_kW_feature = runmode2_kW_mode_cmd.value.value_int;
        // set ui_enabled to false for all feature vars of all runmode2 active power features
        ui_disable_feature_group_feature_vars(runmode2_kW_features_list);
        // set ui_enabled to true for all feature vars of chosen runmode2 active power feature (unless Disabled selected)
        if (runmode2_kW_mode_cmd.value.value_int < (int)runmode2_kW_features_list.size())
            runmode2_kW_features_list[current_runmode2_kW_feature]->toggle_ui_enabled(true);
    }

    // if runmode1 reactive power feature command has changed, switch runmode1 reactive power features
    if (current_runmode1_kVAR_feature != runmode1_kVAR_mode_cmd.value.value_int) {
        // turn off old gc reactive power feature unless this is first iteration
        if (current_runmode1_kVAR_feature >= 0 && current_runmode1_kVAR_feature < (int)runmode1_kVAR_features_list.size())
            runmode1_kVAR_features_list[current_runmode1_kVAR_feature]->enable_flag.value.set(false);
        // turn on new gc reactive power feature (unless Disabled selected)
        if (runmode1_kVAR_mode_cmd.value.value_int < (int)runmode1_kVAR_features_list.size())
            runmode1_kVAR_features_list[runmode1_kVAR_mode_cmd.value.value_int]->enable_flag.value.set(true);
        current_runmode1_kVAR_feature = runmode1_kVAR_mode_cmd.value.value_int;
        // set ui_enabled to false for all feature vars of all gc reactive power features
        ui_disable_feature_group_feature_vars(runmode1_kVAR_features_list);
        // set ui_enabled to true for all feature vars of chosen gc reactive power feature (unless Disabled selected)
        if (runmode1_kVAR_mode_cmd.value.value_int < (int)runmode1_kVAR_features_list.size())
            runmode1_kVAR_features_list[current_runmode1_kVAR_feature]->toggle_ui_enabled(true);
    }

    // if runmode2 reactive power feature command has changed, switch runmode2 reactive power features
    if (current_runmode2_kVAR_feature != runmode2_kVAR_mode_cmd.value.value_int) {
        // turn off old runmode2 reactive power feature unless this is first iteration
        if (current_runmode2_kVAR_feature >= 0 && current_runmode2_kVAR_feature < (int)runmode2_kVAR_features_list.size())
            runmode2_kVAR_features_list[current_runmode2_kVAR_feature]->enable_flag.value.set(false);
        // turn on new runmode2 reactive power feature (unless Disabled selected)
        if (runmode2_kVAR_mode_cmd.value.value_int < (int)runmode2_kVAR_features_list.size())
            runmode2_kVAR_features_list[runmode2_kVAR_mode_cmd.value.value_int]->enable_flag.value.set(true);
        current_runmode2_kVAR_feature = runmode2_kVAR_mode_cmd.value.value_int;
        // set ui_enabled to false for all feature vars of all runmode2 reactive power features
        ui_disable_feature_group_feature_vars(runmode2_kVAR_features_list);
        // set ui_enabled to true for all feature vars of chosen runmode2 reactive power feature (unless Disabled selected)
        if (runmode2_kVAR_mode_cmd.value.value_int < (int)runmode2_kVAR_features_list.size())
            runmode2_kVAR_features_list[current_runmode2_kVAR_feature]->toggle_ui_enabled(true);
    }

    // set features mode string for UI
    runmode1_kW_mode_status.value.set(available_features_runmode1_kW_mode.options_name[runmode1_kW_mode_cmd.value.value_int]);
    runmode2_kW_mode_status.value.set(available_features_runmode2_kW_mode.options_name[runmode2_kW_mode_cmd.value.value_int]);
    runmode1_kVAR_mode_status.value.set(available_features_runmode1_kVAR_mode.options_name[runmode1_kVAR_mode_cmd.value.value_int]);
    runmode2_kVAR_mode_status.value.set(available_features_runmode2_kVAR_mode.options_name[runmode2_kVAR_mode_cmd.value.value_int]);
}

// Read which runmode1 kW features are available and configure them on the UI
bool Site_Manager::configure_runmode1_kW_features(void) {
    // add only the runmode1 kW features covered by the available_features_runmode1_kW_mode mask
    bool initial_gc_kW_feature_found = false;
    int num_features = (int)runmode1_kW_features_list.size();
    if (available_features_runmode1_kW_mode.num_options != num_features + 1)  // +1 because of Default
    {
        FPS_ERROR_LOG("available_features_runmode1_kW_mode does not have all modes listed. Expected %d, got %d.\n", num_features + 1, available_features_runmode1_kW_mode.num_options);
        return false;
    }
    try {
        available_runmode1_kW_features_mask = (uint64_t)std::stoul(available_features_runmode1_kW_mode.value.value_string, NULL, 16);
    } catch (...) {
        FPS_ERROR_LOG("Error parsing available_runmode1_kW_features_mask\n");
        return false;
    }
    for (int i = 0; i <= num_features; ++i)  // <= so Disabled is included
    {
        if (available_runmode1_kW_features_mask & ((uint64_t)1) << i) {
            // mark feature as available to customer as long as it is not "Disabled" option
            if (i != num_features)
                runmode1_kW_features_list[i]->available = true;

            // if initial runmode1 kW feature is found, mark that it passed the mask
            if (i == runmode1_kW_mode_cmd.value.value_int)
                initial_gc_kW_feature_found = true;

            // add feature to list of available runmode1 kW feature commands
            runmode1_kW_mode_cmd.options_name.push_back(available_features_runmode1_kW_mode.options_name[i]);
            runmode1_kW_mode_cmd.options_value.push_back(Value_Object(i));
            runmode1_kW_mode_cmd.num_options++;

            // Set charge control available if utilized by any feature
            if (runmode1_kW_features_charge_control_mask & ((uint64_t)1) << i)
                charge_control.available = true;
        } else {
            if (i != num_features)
                runmode1_kW_features_list[i]->enable_flag.value.set(false);
        }
    }

    // do not show control on UI if only one option
    if (runmode1_kW_mode_cmd.num_options < 2) {
        runmode1_kW_mode_cmd.set_ui_type("none");
    }

    // Define the load buffer with a single entry for each 10ms iteration
    asset_cmd.create_site_kW_load_buffer(site_kW_load_interval_ms.value.value_int * MS_TO_uS_MULTIPLIER / STATE_MACHINE_INTERVAL_uS);

    // if the initial runmode1 kW feature was not in available features mask, kill configuration since this is fatal error
    if (!initial_gc_kW_feature_found) {
        FPS_ERROR_LOG("INITIAL RUN MODE 1 ACTIVE POWER FEATURE MISCONFIGURED!\n");
        FPS_ERROR_LOG("Initial runmode1 kW feature does not pass available runmode1 kW features mask.\n");
        return false;
    }

    return true;
}

// Read which runmode2 active power features are available and configure them on the UI
bool Site_Manager::configure_runmode2_kW_features(void) {
    // add only the runmode2 kW features covered by the available_features_runmode2_kW_mode mask
    bool initial_runmode2_kW_feature_found = false;
    int num_features = (int)runmode2_kW_features_list.size();
    if (available_features_runmode2_kW_mode.num_options != num_features + 1)  // +1 because of Default
    {
        FPS_ERROR_LOG("available_features_runmode2_kW_mode does not have all modes listed. Expected %d, got %d.\n", num_features + 1, available_features_runmode2_kW_mode.num_options);
        return false;
    }
    try {
        available_runmode2_kW_features_mask = (uint64_t)std::stoul(available_features_runmode2_kW_mode.value.value_string, NULL, 16);
    } catch (...) {
        FPS_ERROR_LOG("Error parsing available_runmode2_kW_features_mask\n");
        return false;
    }
    for (int i = 0; i <= num_features; ++i)  // <= so Disabled is included
    {
        if (available_runmode2_kW_features_mask & ((uint64_t)1) << i) {
            // mark feature as available to customer as long as it is not "Disabled" option
            if (i != num_features)
                runmode2_kW_features_list[i]->available = true;

            // if initial runmode2 kW feature is found, mark that it passed the mask
            if (i == runmode2_kW_mode_cmd.value.value_int)
                initial_runmode2_kW_feature_found = true;

            // add feature to list of available runmode2 kW feature commands
            runmode2_kW_mode_cmd.options_name.push_back(available_features_runmode2_kW_mode.options_name[i]);
            runmode2_kW_mode_cmd.options_value.push_back(Value_Object(i));
            runmode2_kW_mode_cmd.num_options++;
        } else {
            if (i != num_features)
                runmode2_kW_features_list[i]->enable_flag.value.set(false);
        }
    }

    // do not show control on UI if only one option
    if (runmode2_kW_mode_cmd.num_options < 2) {
        runmode2_kW_mode_cmd.set_ui_type("none");
    }

    // if the initial runmode2 kW feature was not in available features mask, kill configuration since this is fatal error
    if (!initial_runmode2_kW_feature_found) {
        FPS_ERROR_LOG("INITIAL RUNMODE2 ACTIVE POWER FEATURE MISCONFIGURED!\n");
        FPS_ERROR_LOG("Initial runmode2 kW feature does not pass available runmode2 kW features mask.\n");
        return false;
    }

    return true;
}

// Read which runmode1 kVAR features are available and configure them on the UI
bool Site_Manager::configure_runmode1_kVAR_features(void) {
    // add only the runmode1 kVAR features covered by the available_features_runmode1_kVAR_mode mask
    bool initial_gc_kVAR_feature_found = false;
    int num_features = (int)runmode1_kVAR_features_list.size();
    if (available_features_runmode1_kVAR_mode.num_options != num_features + 1)  // +1 because of Default
    {
        FPS_ERROR_LOG("available_features_runmode1_kVAR_mode does not have all modes listed. Expected %d, got %d.\n", num_features + 1, available_features_runmode1_kVAR_mode.num_options);
        return false;
    }
    try {
        available_runmode1_kVAR_features_mask = (uint64_t)std::stoul(available_features_runmode1_kVAR_mode.value.value_string, NULL, 16);
    } catch (...) {
        FPS_ERROR_LOG("Error parsing available_runmode1_kVAR_features_mask\n");
        return false;
    }
    for (int i = 0; i <= num_features; ++i)  // <= so Disabled is included
    {
        if (available_runmode1_kVAR_features_mask & ((uint64_t)1) << i) {
            // mark feature as available to customer as long as it is not "Disabled" option
            if (i != num_features)
                runmode1_kVAR_features_list[i]->available = true;

            // if initial runmode1 kVAR feature is found, mark that it passed the mask
            if (i == runmode1_kVAR_mode_cmd.value.value_int)
                initial_gc_kVAR_feature_found = true;

            // add feature to list of available runmode1 kVAR feature commands
            runmode1_kVAR_mode_cmd.options_name.push_back(available_features_runmode1_kVAR_mode.options_name[i]);
            runmode1_kVAR_mode_cmd.options_value.push_back(Value_Object(i));
            runmode1_kVAR_mode_cmd.num_options++;
        } else {
            if (i != num_features)
                runmode1_kVAR_features_list[i]->enable_flag.value.set(false);
        }
    }

    // do not show control on UI if only one option
    if (runmode1_kVAR_mode_cmd.num_options < 2) {
        runmode1_kVAR_mode_cmd.set_ui_type("none");
    }

    // if the initial runmode1 kVAR feature was not in available features mask, kill configuration since this is fatal error
    if (!initial_gc_kVAR_feature_found) {
        FPS_ERROR_LOG("INITIAL RUN MODE 1 REACTIVE POWER FEATURE MISCONFIGURED!\n");
        FPS_ERROR_LOG("Initial runmode1 kVAR feature does not pass available reactive power features mask.\n");
        return false;
    }
    return true;
}

// Read which runmode2 kVAR features are available and configure them on the UI
bool Site_Manager::configure_runmode2_kVAR_features(void) {
    // add only the runmode2 kVAR features covered by the available_features_runmode2_kVAR_mode mask
    bool initial_gc_kVAR_feature_found = false;
    int num_features = (int)runmode2_kVAR_features_list.size();
    if (available_features_runmode2_kVAR_mode.num_options != num_features + 1)  // +1 because of Default
    {
        FPS_ERROR_LOG("available_features_runmode2_kVAR_mode does not have all modes listed. Expected %d, got %d.\n", num_features + 1, available_features_runmode2_kVAR_mode.num_options);
        return false;
    }
    try {
        available_runmode2_kVAR_features_mask = (uint64_t)std::stoul(available_features_runmode2_kVAR_mode.value.value_string, NULL, 16);
    } catch (...) {
        FPS_ERROR_LOG("Error parsing available_runmode2_kVAR_features_mask\n");
        return false;
    }
    for (int i = 0; i <= num_features; ++i)  // <= so Disabled is included
    {
        if (available_runmode2_kVAR_features_mask & ((uint64_t)1) << i) {
            // mark feature as available to customer as long as it is not "Disabled" option
            if (i != num_features)
                runmode2_kVAR_features_list[i]->available = true;

            // if initial runmode2 kVAR feature is found, mark that it passed the mask
            if (i == runmode2_kVAR_mode_cmd.value.value_int)
                initial_gc_kVAR_feature_found = true;

            // add feature to list of available runmode2 kVAR feature commands
            runmode2_kVAR_mode_cmd.options_name.push_back(available_features_runmode2_kVAR_mode.options_name[i]);
            runmode2_kVAR_mode_cmd.options_value.push_back(Value_Object(i));
            runmode2_kVAR_mode_cmd.num_options++;
        } else {
            if (i != num_features)
                runmode2_kVAR_features_list[i]->enable_flag.value.set(false);
        }
    }

    // do not show control on UI if only one option
    if (runmode2_kVAR_mode_cmd.num_options < 2) {
        runmode2_kVAR_mode_cmd.set_ui_type("none");
    }

    // if the initial runmode2 kVAR feature was not in available features mask, kill configuration since this is fatal error
    if (!initial_gc_kVAR_feature_found) {
        FPS_ERROR_LOG("INITIAL RUNMODE2 REACTIVE POWER FEATURE MISCONFIGURED!\n");
        FPS_ERROR_LOG("Initial runmode2 kVAR feature does not pass available active power features mask.\n");
        return false;
    }
    return true;
}

/**
 * Read which standalone power features are available and configure them on the UI.
 * @return True if configuration was successful, false if not.
 */
bool Site_Manager::configure_standalone_power_features(void) {
    int num_features = (int)standalone_power_features_list.size();
    if (available_features_standalone_power.num_options != num_features) {
        FPS_ERROR_LOG("available_features_standalone_power does not have all modes listed. Expected %d, got %d.\n", num_features, available_features_standalone_power.num_options);
        return false;
    }
    // add only the standalone power features covered by the available_features_standalone_power mask
    try {
        available_standalone_power_features_mask = (uint64_t)std::stoul(available_features_standalone_power.value.value_string, NULL, 16);
    } catch (...) {
        FPS_ERROR_LOG("Error parsing available_standalone_power_features_mask\n");
        return false;
    }
    for (int i = 0; i < (int)standalone_power_features_list.size(); ++i) {
        if (available_standalone_power_features_mask & ((uint64_t)1) << i) {
            // mark feature as available to customer
            standalone_power_features_list[i]->available = true;
        } else {
            standalone_power_features_list[i]->enable_flag.value.set(false);
        }
    }

    // send Generator Manager the initial LDSS settings
    set_ldss_variables();
    return true;
}

/**
 * Read which site operation features are available and configure them on the UI.
 * @return True if configuration was successful, false if not.
 */
bool Site_Manager::configure_site_operation_features(void) {
    int num_features = (int)site_operation_features_list.size();
    if (available_features_site_operation.num_options != num_features) {
        FPS_ERROR_LOG("available_features_site_operation does not have all modes listed. Expected %d, got %d.\n", num_features, available_features_site_operation.num_options);
        return false;
    }
    // add only the site operation features covered by the available_features_site_operation mask
    try {
        available_site_operation_features_mask = (uint64_t)std::stoul(available_features_site_operation.value.value_string, NULL, 16);
    } catch (...) {
        FPS_ERROR_LOG("Error parsing available_site_operation_features_mask\n");
        return false;
    }
    for (int i = 0; i < (int)site_operation_features_list.size(); ++i) {
        if (available_site_operation_features_mask & ((uint64_t)1) << i) {
            // mark feature as available to customer
            site_operation_features_list[i]->available = true;
        } else {
            site_operation_features_list[i]->enable_flag.value.set(false);
        }
    }
    return true;
}

/**
 * Add any additional persistent settings entries once the site has configured
 * TODO: In the future these pairs should be denoted by variables.json configuration
 *       But for the time being simply add any additional multiple inputs variables to the hardcoded list
 */
void Site_Manager::configure_persistent_settings_pairs() {
    for (size_t i = 0; i < enable_flag.inputs.size(); ++i) {
        std::string input_id = std::string(enable_flag.get_variable_id()) + "_" + enable_flag.input_source_settings->get_uri_suffix_of_input(i);
        data_endpoint->opposite_setpoints.find("disable_flag")->second.push_back(input_id);
    }
}

void Site_Manager::update_ess_kpi_values() {
    // tax credit KPI
    float interconnect_kW = feeder_actual_kW.value.value_float;
    float ess_kW = ess_actual_kW.value.value_float;
    float ess_discharge_kW = 0;
    float pv_kW = interconnect_kW - ess_kW;
    float ess_grid_kW = 0;
    float ess_pv_kW = 0;

    // get ess grid and pv portions
    // ess is charging and exceeds PV production
    if ((ess_kW < 0) && (fabsf(ess_kW) > pv_kW)) {
        if (pv_kW > 0)  // there is PV production
        {
            ess_grid_kW = fabsf(ess_kW) - pv_kW;
            ess_pv_kW = pv_kW;
        } else  // no PV production (avoid sensing error noise)
            ess_grid_kW = fabsf(ess_kW);
    }
    // PV production exceeds ess charging amount
    else if ((ess_kW < 0) && (fabsf(ess_kW) < pv_kW))
        ess_pv_kW = fabsf(ess_kW);

    // get ess discharge only
    if (ess_kW > 0)
        ess_discharge_kW = ess_kW;

    ess_instant_discharge.value.set(ess_discharge_kW);
    ess_instant_charge_grid.value.set(ess_grid_kW);
    ess_instant_charge_pv.value.set(ess_pv_kW);
}

void Site_Manager::set_faults(int fault_number) {
    char event_message[SHORT_MSG_LEN];

    active_fault_array[fault_number] = true;
    snprintf(event_message, SHORT_MSG_LEN, "Fault: %s", faults.options_name[fault_number].c_str());
    FPS_ERROR_LOG("%s", event_message);

    emit_event("Site", event_message, FAULT_ALERT);
    fault_status_flag.value.value_bool = true;
}

void Site_Manager::set_alarms(int alarm_number) {
    active_alarm_array[alarm_number] = true;

#ifndef FPS_TEST_MODE
    // Causes seg faults in test mode as options names undefined
    char event_message[SHORT_MSG_LEN];
    snprintf(event_message, SHORT_MSG_LEN, "Alarm: %s", alarms.options_name[alarm_number].c_str());
    FPS_ERROR_LOG("%s", event_message);
    emit_event("Site", event_message, ALARM_ALERT);
#endif

    alarm_status_flag.value.value_bool = true;
}

/*
    If there are asset alarms/faults that are cleared but still flagged, this function can unflag them
    If the cleared asset alarms/faults are the only site alarms, this function will also clear site alarm flag
*/
void Site_Manager::clear_alarms(int alarm_number) {
    active_alarm_array[alarm_number] = false;
    if (!get_alarms()) {
        alarm_status_flag.value.set(false);
    }
}

bool Site_Manager::get_faults() const {
    for (int i = 0; i < 64; i++) {
        if (active_fault_array[i])
            return true;
    }
    return false;
}

bool Site_Manager::get_alarms() const {
    for (int i = 0; i < 64; i++) {
        if (active_alarm_array[i])
            return true;
    }
    return false;
}

bool Site_Manager::get_active_faults(int index) {
    return active_fault_array[index];
}

bool Site_Manager::get_active_alarms(int index) {
    return active_alarm_array[index];
}

void Site_Manager::clear_faults() {
    // Tell user Clear Faults button was pressed
    FPS_ERROR_LOG("Site Manager clear faults executed");
    emit_event("Site", "Clear faults executed", INFO_ALERT);

    // Tell Asset Manager to do its part in clearing faults, including clearing component faults
    pAssets->site_clear_faults();

    // Delay clearing faults for 1 second to give Asset Manager time to clear component faults. Otherwise cleared bits would see asset fault and reset
    clear_fault_status_flags = true;
    clock_gettime(CLOCK_MONOTONIC, &time_to_clear_fault_status_flags);
    increment_timespec_ms(time_to_clear_fault_status_flags, 1000);
}

void Site_Manager::clear_fault_registers() {
    // Turn off clear faults bool so this function only executes once per Clear Faults button press
    clear_fault_status_flags = false;

    // Clear fault and alarm indicator flags
    fault_status_flag.value.set(false);
    alarm_status_flag.value.set(false);

    // Clear all faults and arrays
    for (int i = 0; i < 64; i++) {
        active_fault_array[i] = false;
        active_alarm_array[i] = false;
    }
}

/**
 * Set the enable flag and all of its multiple inputs false
 * @param value the value to set
 */
void Site_Manager::set_enable_flag(bool value) {
    enable_flag.value.set(value);
    for (auto& input_flag : enable_flag.inputs) {
        input_flag.set(value);
    }
}

void Site_Manager::build_active_faults() {
    int count = 0;
    active_faults.value.value_bit_field = 0;
    for (int i = 0; i < faults.num_options; i++) {
        if (active_fault_array[i]) {
            active_faults.options_name[i] = faults.options_name[i];
            active_faults.options_value[i].set(faults.options_value[i].value_int);
            active_faults.value.value_bit_field |= 1 << i;
            count++;
        }
    }
    active_faults.value.set(count);
    active_faults.num_options = count;
}

void Site_Manager::build_active_alarms() {
    int count = 0;
    active_alarms.value.value_bit_field = 0;
    for (int i = 0; i < alarms.num_options; i++) {
        if (active_alarm_array[i]) {
            active_alarms.options_name[i] = alarms.options_name[i];
            active_alarms.options_value[i].set(alarms.options_value[i].value_int);
            active_alarms.value.value_bit_field |= 1 << i;
            count++;
        }
    }
    active_alarms.value.set(count);
    active_alarms.num_options = count;
}

bool Site_Manager::set_state(states state_request) {
    FPS_INFO_LOG("Site Manager function 'set_state' called \n");
    if (state_request != current_state)
        current_state = state_request;
    return true;
}

void Site_Manager::set_site_status(const char* message) {
    site_status.value.set(std::string(message));
}

bool Site_Manager::call_sequence_functions(const char* target_asset, const char* cmd, Value_Object* value, int tolerance_percent) {
    bool command_found = true;
    bool return_value = true;

    // this cmd will pass and move to next step
    if (strcmp(target_asset, "bypass") == 0)
        return true;
    // this cmd will move to next path (requires step to have path switch
    else if (strcmp(target_asset, "new_path") == 0)
        return false;
    // all generic boolean and floats for site-specific configuration
    else if (strcmp(target_asset, "reserved") == 0) {
        if (strcmp(cmd, "bool_1") == 0)
            return_value = (reserved_bool_1.value.value_bool == value->value_bool);
        else if (strcmp(cmd, "bool_2") == 0)
            return_value = (reserved_bool_2.value.value_bool == value->value_bool);
        else if (strcmp(cmd, "bool_3") == 0)
            return_value = (reserved_bool_3.value.value_bool == value->value_bool);
        else if (strcmp(cmd, "bool_4") == 0)
            return_value = (reserved_bool_4.value.value_bool == value->value_bool);
        else if (strcmp(cmd, "bool_5") == 0)
            return_value = (reserved_bool_5.value.value_bool == value->value_bool);
        else if (strcmp(cmd, "bool_6") == 0)
            return_value = (reserved_bool_6.value.value_bool == value->value_bool);
        else if (strcmp(cmd, "bool_7") == 0)
            return_value = (reserved_bool_7.value.value_bool == value->value_bool);
        else if (strcmp(cmd, "bool_8") == 0)
            return_value = (reserved_bool_8.value.value_bool == value->value_bool);
        else if (strcmp(cmd, "bool_9") == 0)
            return_value = (reserved_bool_9.value.value_bool == value->value_bool);
        else if (strcmp(cmd, "bool_10") == 0)
            return_value = (reserved_bool_10.value.value_bool == value->value_bool);
        else if (strcmp(cmd, "bool_11") == 0)
            return_value = (reserved_bool_11.value.value_bool == value->value_bool);
        else if (strcmp(cmd, "bool_12") == 0)
            return_value = (reserved_bool_12.value.value_bool == value->value_bool);
        else if (strcmp(cmd, "bool_13") == 0)
            return_value = (reserved_bool_13.value.value_bool == value->value_bool);
        else if (strcmp(cmd, "bool_14") == 0)
            return_value = (reserved_bool_14.value.value_bool == value->value_bool);
        else if (strcmp(cmd, "bool_15") == 0)
            return_value = (reserved_bool_15.value.value_bool == value->value_bool);
        else if (strcmp(cmd, "bool_16") == 0)
            return_value = (reserved_bool_16.value.value_bool == value->value_bool);
        else if (strcmp(cmd, "float_1") == 0)
            return_value = ((value->type == Float) && get_tolerance(value->value_float, reserved_float_1.value.value_float, tolerance_percent));
        else if (strcmp(cmd, "float_2") == 0)
            return_value = ((value->type == Float) && get_tolerance(value->value_float, reserved_float_2.value.value_float, tolerance_percent));
        else if (strcmp(cmd, "float_3") == 0)
            return_value = ((value->type == Float) && get_tolerance(value->value_float, reserved_float_3.value.value_float, tolerance_percent));
        else if (strcmp(cmd, "float_4") == 0)
            return_value = ((value->type == Float) && get_tolerance(value->value_float, reserved_float_4.value.value_float, tolerance_percent));
        else if (strcmp(cmd, "float_5") == 0)
            return_value = ((value->type == Float) && get_tolerance(value->value_float, reserved_float_5.value.value_float, tolerance_percent));
        else if (strcmp(cmd, "float_6") == 0)
            return_value = ((value->type == Float) && get_tolerance(value->value_float, reserved_float_6.value.value_float, tolerance_percent));
        else if (strcmp(cmd, "float_7") == 0)
            return_value = ((value->type == Float) && get_tolerance(value->value_float, reserved_float_7.value.value_float, tolerance_percent));
        else if (strcmp(cmd, "float_8") == 0)
            return_value = ((value->type == Float) && get_tolerance(value->value_float, reserved_float_8.value.value_float, tolerance_percent));
        else
            command_found = false;
    }
    // put all ess commands here
    else if (strcmp(target_asset, "ess") == 0) {
        if (strcmp(cmd, "get_num_ess_available") == 0)
            return_value = ((value->type == Float) && get_tolerance(value->value_float, num_ess_available, tolerance_percent));
        else if (strcmp(cmd, "get_num_ess_running") == 0)
            return_value = ((value->type == Float) && get_tolerance(value->value_float, num_ess_running, tolerance_percent));
        else if (strcmp(cmd, "get_num_ess_controllable") == 0)
            return_value = ((value->type == Float) && get_tolerance(value->value_float, num_ess_controllable, tolerance_percent));
        else if (strcmp(cmd, "get_ess_active_power") == 0)
            return_value = ((value->type == Float) && get_tolerance(value->value_float, ess_actual_kW.value.value_float, tolerance_percent));
        else if (strcmp(cmd, "start_all_ess") == 0)
            return_value = pAssets->start_all_ess();
        else if (strcmp(cmd, "stop_all_ess") == 0)
            return_value = pAssets->stop_all_ess();
        else if (strcmp(cmd, "set_all_ess_grid_form") == 0)
            pAssets->set_all_ess_grid_form();
        else if (strcmp(cmd, "set_all_ess_grid_follow") == 0)
            pAssets->set_all_ess_grid_follow();
        else if (strcmp(cmd, "set_voltage_slope") == 0)
            pAssets->set_grid_forming_voltage_slew(value->value_float);
        else if (strcmp(cmd, "open_contactors") == 0)
            pAssets->open_all_bms_contactors();
        else if (strcmp(cmd, "close_contactors") == 0)
            pAssets->close_all_bms_contactors();
        else if (strcmp(cmd, "synchronize_ess") == 0)
            return_value = synchronize_ess();
        else if (strcmp(cmd, "has_faults") == 0)
            return_value = ((pAssets->get_num_active_faults(ESS) > 0) == value->value_bool);
        else if (strcmp(cmd, "allow_auto_restart") == 0)
            allow_ess_auto_restart.value.set(value->value_bool);
        else if (strcmp(cmd, "controllable_soc_above") == 0)
            return_value = ((value->type == Float) && soc_avg_running.value.value_float > value->value_float);
        else if (strcmp(cmd, "controllable_soc_below") == 0)
            return_value = ((value->type == Float) && soc_avg_running.value.value_float < value->value_float);
        else if (strcmp(cmd, "all_soc_above") == 0)
            return_value = ((value->type == Float) && soc_avg_all.value.value_float > value->value_float);
        else if (strcmp(cmd, "all_soc_below") == 0)
            return_value = ((value->type == Float) && soc_avg_all.value.value_float < value->value_float);
        else
            command_found = false;
    } else if (strcmp(target_asset, "gen") == 0) {
        // put all gen commands here
        if (strcmp(cmd, "get_num_gen_available") == 0)
            return_value = ((value->type == Float) && get_tolerance(value->value_float, num_gen_available, tolerance_percent));
        else if (strcmp(cmd, "get_num_gen_running") == 0)
            return_value = ((value->type == Float) && get_tolerance(value->value_float, num_gen_running, tolerance_percent));
        else if (strcmp(cmd, "get_num_gen_active") == 0)
            return_value = ((value->type == Float) && get_tolerance(value->value_float, num_gen_active, tolerance_percent));
        else if (strcmp(cmd, "min_generators_active") == 0)
            pAssets->set_min_generators_active(value->value_float);
        else if (strcmp(cmd, "direct_start_gen") == 0) {
            return_value = pAssets->direct_start_gen();
        } else if (strcmp(cmd, "start_all_gen") == 0) {
            return_value = true;
            pAssets->start_all_gen();
        } else if (strcmp(cmd, "stop_all_gen") == 0)
            return_value = pAssets->stop_all_gen();
        else if (strcmp(cmd, "set_all_gen_grid_follow") == 0)
            pAssets->set_all_gen_grid_follow();
        else if (strcmp(cmd, "set_all_gen_grid_form") == 0)
            pAssets->set_all_gen_grid_form();
        else if (strcmp(cmd, "has_faults") == 0)
            return_value = ((pAssets->get_num_active_faults(GENERATORS) > 0) == value->value_bool);
        else if (strcmp(cmd, "allow_auto_restart") == 0)
            allow_gen_auto_restart.value.set(value->value_bool);  // set the config bool for set_values
        else
            command_found = false;
    }
    // put all solar commands here
    else if (strcmp(target_asset, "solar") == 0) {
        if (strcmp(cmd, "get_num_solar_available") == 0)
            return_value = ((value->type == Float) && get_tolerance(value->value_float, num_solar_available, tolerance_percent));
        else if (strcmp(cmd, "get_num_solar_running") == 0)
            return_value = ((value->type == Float) && get_tolerance(value->value_float, num_solar_running, tolerance_percent));
        else if (strcmp(cmd, "start_all_solar") == 0)
            return_value = pAssets->start_all_solar();
        else if (strcmp(cmd, "stop_all_solar") == 0)
            return_value = pAssets->stop_all_solar();
        else if (strcmp(cmd, "has_faults") == 0)
            return_value = ((pAssets->get_num_active_faults(SOLAR) > 0) == value->value_bool);
        else if (strcmp(cmd, "allow_auto_restart") == 0)
            allow_solar_auto_restart.value.set(value->value_bool);
        else
            command_found = false;
    }
    // put all GENERAL feeder commands here
    else if (strcmp(target_asset, "feeder") == 0) {
        // sync breaker state
        if (strcmp(cmd, "get_sync_feeder_state") == 0)
            return_value = pAssets->get_sync_feeder_status() ? (value->type == Bool) && (value->value_bool == true) : (value->type == Bool) && (value->value_bool == false);
        // get poi breaker state
        else if (strcmp(cmd, "get_poi_feeder_state") == 0) {
            bool poi_state = pAssets->get_poi_feeder_state();  // DEBUG
            return_value = (value->type == Bool) && (value->value_bool == poi_state);
        }
        // set poi breaker state open
        else if (strcmp(cmd, "set_poi_feeder_state_open") == 0)
            return_value = pAssets->set_poi_feeder_state_open();
        // set poi breaker state closed
        else if (strcmp(cmd, "set_poi_feeder_state_closed") == 0)
            return_value = pAssets->set_poi_feeder_state_closed();
        else if (strcmp(cmd, "set_sync_feeder_close_permissive_remove") == 0)
            return_value = pAssets->set_sync_feeder_close_permissive_remove();
        else if (strcmp(cmd, "set_sync_feeder_close_permissive") == 0)
            return_value = pAssets->set_sync_feeder_close_permissive();
        else
            command_found = false;
    }
    // put all site state/operation commands here
    else if (strcmp(target_asset, "config") == 0) {
        if (strcmp(cmd, "clear_faults") == 0)
            clear_faults();
        else if (strcmp(cmd, "get_standby_flag") == 0)
            return_value = standby_flag.value.value_bool == value->value_bool;  // get standby flag status
        else
            command_found = false;
    }
    // feature commands
    else if (strcmp(target_asset, "features") == 0) {
        if (strcmp(cmd, "reset_load_shed") == 0) {
            int configured_shed_value = int(value->value_float);
            if (configured_shed_value < load_shed_min_value.value.value_int || configured_shed_value > load_shed_max_value.value.value_int) {
                FPS_ERROR_LOG("Invalid load shed value %d provided, must be between %d and %d\n", configured_shed_value, load_shed_min_value.value.value_int, load_shed_max_value.value.value_int);
                command_found = false;
            } else {
                load_shed_value.value.set(configured_shed_value);
                load_shed_calculator.offset = configured_shed_value;
            }
        } else if (strcmp(cmd, "enable_ldss") == 0)
            ldss.enable_flag.value.set(value->value_bool);
        else
            command_found = false;
    }

    // check for specific feeder id for setting and getting specific feeder states
    else {
        // specific feeder id is used in place of target_asset
        Asset_Feeder* found_feeder = pAssets->validate_feeder_id(target_asset);
        if (found_feeder == nullptr) {
            FPS_ERROR_LOG("feeder ID: %s was not found in the assets list\n", target_asset);
            command_found = false;
        } else {
            // set_feeder_state_open
            if (strcmp(cmd, "set_feeder_state_open") == 0) {
                return_value = pAssets->set_feeder_state_open(found_feeder);
            }
            // set_feeder_state_closed
            else if (strcmp(cmd, "set_feeder_state_closed") == 0) {
                return_value = pAssets->set_feeder_state_closed(found_feeder);
            }
            // get_feeder_state : return feeder (as target_asset) state as true for closed (on) and false for open (off)
            else if (strcmp(cmd, "get_feeder_state") == 0) {
                bool fdr_state = pAssets->get_feeder_state(found_feeder);
                return_value = (value->type == Bool) && (value->value_bool == fdr_state);
            }
            // get_feeder_utility_status : return feeder (as target_asset) utility status as true for online and false for offline
            else if (strcmp(cmd, "get_utility_status") == 0) {
                bool fdr_state = pAssets->get_feeder_utility_status(found_feeder);
                return_value = (value->type == Bool) && (value->value_bool == fdr_state);
            } else
                command_found = false;
        }
    }

    // no command executed
    if (command_found == false) {
        FPS_ERROR_LOG("call_function command not found. target_asset: %s cmd: %s \n", target_asset, cmd);
        return false;
    }

    return return_value;
}

void Site_Manager::check_state(void) {
    Sequence current_sequence = sequences[current_state];
    Path current_path = current_sequence.paths[current_sequence.current_path_index];

    // if faulted or shutdown cmd, enter shutdown state
    if (current_state != Init && (current_sequence.check_faults() || (disable_flag.value.value_bool)))
        current_state = Shutdown;

    // check if alarms are present
    current_sequence.check_alarms();

    // count number of asset faults and asset alarms
    num_path_faults = current_path.num_active_faults;
    num_path_alarms = current_path.num_active_alarms;

    // boolean running status check
    running_status_flag.value.set((current_state == RunMode1) || current_state == RunMode2);

    // if new state detected, init vars as needed
    if (check_current_state != current_state) {
        char event_message[SHORT_MSG_LEN];
        FPS_INFO_LOG("Site Manager state change to: %s", state_name[current_state]);
        snprintf(event_message, SHORT_MSG_LEN, "State changed to %s", state_name[current_state]);
        emit_event("Site", event_message, STATUS_ALERT);
        sequences[check_current_state].sequence_bypass = false;  // ensure previous state executes next time its called
        check_current_state = current_state;
        site_state.value.set(state_name[current_state]);
        site_state_enum.value.set(current_state);
        sequence_reset = true;
    }
}

void Site_Manager::process_state(void) {
    FPS_DEBUG_LOG("\n***HybridOS Step 3: Get and Set Asset Manager Data.\nIn Site_Manager::process_state.\n");

    // get all interface variables
    get_values();

    // if clear faults is true, call clear_faults function
    if (clear_faults_flag.value.value_bool) {
        clear_faults();

        // after clearing faults, lower the clear faults flag, including any inputs
        clear_faults_flag.value.set(false);
        for (auto& input_flag : clear_faults_flag.inputs) {
            input_flag.set(false);
        }
    }

    // confirm connection with master controller (if this feature is enabled)
    if (watchdog_feature.enable_flag.value.value_bool)
        watchdog();

    // determine which state to run
    check_state();

    // run the sequence for the current state
    sequences[current_state].call_sequence();

    // run the current state function
    switch (current_state) {
        // run the current state function
        case Init:
            init_state();
            break;
        case Ready:
            ready_state();
            break;
        case Startup:
            startup_state();
            break;
        case RunMode1:
            runmode1_state();
            break;
        case RunMode2:
            runmode2_state();
            break;
        case Standby:
            standby_state();
            break;
        case Shutdown:
            shutdown_state();
            break;
        case Error:
            error_state();
            break;
        default:
            current_state = Error;
    }

    // set all interface variables
    set_values();
}

// initialization state - run once at program boot to initialize variables
void Site_Manager::init_state(void) {
    // FPS_ERROR_LOG("site manager Init State executed \n");
    emit_event("Site", "System initialized", INFO_ALERT);
    current_state = Ready;

    // set internal vars to 0
    ess_kW_cmd.value.set(0.0f);
    gen_kW_cmd.value.set(0.0f);
    solar_kW_cmd.value.set(0.0f);
    feeder_kW_cmd.value.set(0.0f);
    site_kW_demand.value.set(0.0f);
    site_kW_load.value.set(0.0f);
    ess_kVAR_cmd.value.set(0.0f);
    gen_kVAR_cmd.value.set(0.0f);
    solar_kVAR_cmd.value.set(0.0f);
    active_power_setpoint_mode.kW_slew.reset_slew_target();
    manual_solar_kW_slew.reset_slew_target();
    manual_ess_kW_slew.reset_slew_target();
    manual_gen_kW_slew.reset_slew_target();
    gen_kVAR_cmd_slew.reset_slew_target();
    ess_kVAR_cmd_slew.reset_slew_target();
    solar_kVAR_cmd_slew.reset_slew_target();
    reactive_setpoint_kVAR_cmd_slew.reset_slew_target();

    // initialize watt_var points
    set_curve_points(&watt_var_points, watt_var_curve);
    // initialize watt_watt points
    set_curve_points(&watt_watt_points, watt_watt_curve);

    // initialize load shed Variable_Regulator
    load_shed_calculator.min_offset = load_shed_min_value.value.value_int;
    load_shed_calculator.max_offset = load_shed_max_value.value.value_int;
    // the default case is set to the maximum offset configured
    load_shed_calculator.offset = load_shed_calculator.max_offset;
    load_shed_calculator.default_offset = load_shed_calculator.max_offset;
    if (load_shed_value.value.value_int != load_shed_calculator.offset) {
        load_shed_value.value.set(load_shed_calculator.offset);
        load_shed_value.send_to_component(true);
    }
    load_shed_calculator.set_default_condition(load_shed_max_shedding_threshold.value.value_float, Variable_Regulator::VALUE_BELOW);
    load_shed_calculator.set_control_high_threshold(load_shed_high_threshold.value.value_float);
    load_shed_calculator.set_decrease_timer_duration_ms(load_shed_decrease_timer_ms.value.value_int);
    load_shed_calculator.set_control_low_threshold(load_shed_low_threshold.value.value_float);
    load_shed_calculator.set_increase_timer_duration_ms(load_shed_increase_timer_ms.value.value_int);

    // initialize solar shed Variable_Regulator
    solar_shed_calculator.min_offset = solar_shed_min_value.value.value_int;
    solar_shed_calculator.max_offset = solar_shed_max_value.value.value_int;
    solar_shed_calculator.offset = solar_shed_calculator.max_offset;
    solar_shed_calculator.default_offset = solar_shed_calculator.max_offset;
    solar_shed_value.value.set(solar_shed_calculator.offset);
    solar_shed_calculator.set_default_condition(solar_shed_max_shedding_threshold.value.value_float, Variable_Regulator::VALUE_ABOVE);
    solar_shed_calculator.set_control_high_threshold(solar_shed_high_threshold.value.value_float);
    solar_shed_calculator.set_decrease_timer_duration_ms(solar_shed_decrease_timer_ms.value.value_int);
    solar_shed_calculator.set_control_low_threshold(solar_shed_low_threshold.value.value_float);
    solar_shed_calculator.set_increase_timer_duration_ms(solar_shed_increase_timer_ms.value.value_int);

    // initialize active closed loop control Variable_Regulator
    active_power_closed_loop.regulator.min_offset = active_power_closed_loop.min_offset.value.value_int;
    active_power_closed_loop.regulator.max_offset = active_power_closed_loop.max_offset.value.value_int;
    active_power_closed_loop.regulator.offset = active_power_closed_loop.default_offset.value.value_int;
    active_power_closed_loop.regulator.default_offset = active_power_closed_loop.default_offset.value.value_int;
    // Set up update rate (updates per second) based on the millisecond rate given, with the fastest rate being 10ms
    active_power_closed_loop.update_rate_ms.value.set(std::max(active_power_closed_loop.update_rate_ms.value.value_int, 10));
    active_power_closed_loop.regulator.set_update_rate(1000 / active_power_closed_loop.update_rate_ms.value.value_int);
    // Set up steady state deadband condition as false for any value above the deadband
    active_power_closed_loop.regulator.set_default_condition(active_power_closed_loop.steady_state_deadband_kW.value.value_float, Variable_Regulator::VALUE_ABOVE);
    // Set up regulation deadband condition as false for any value above the 0.5% accuracy deadband compared to the POI
    active_power_closed_loop.regulator.set_control_high_threshold(active_power_closed_loop.regulation_deadband_kW.value.value_float);
    active_power_closed_loop.regulator.set_decrease_timer_duration_ms(active_power_closed_loop.decrease_timer_ms.value.value_int);
    // Set up regulation deadband condition as false for any value below the 0.5% accuracy deadband compared to the POI
    active_power_closed_loop.regulator.set_control_low_threshold(-1.0f * active_power_closed_loop.regulation_deadband_kW.value.value_float);
    active_power_closed_loop.regulator.set_increase_timer_duration_ms(active_power_closed_loop.increase_timer_ms.value.value_int);

    // initialize reactive closed loop control Variable_Regulator
    reactive_power_closed_loop_regulator.min_offset = reactive_power_closed_loop_min_offset.value.value_int;
    reactive_power_closed_loop_regulator.max_offset = reactive_power_closed_loop_max_offset.value.value_int;
    reactive_power_closed_loop_regulator.offset = reactive_power_closed_loop_default_offset.value.value_int;
    reactive_power_closed_loop_regulator.default_offset = reactive_power_closed_loop_default_offset.value.value_int;
    // Set up update rate (updates per second) based on the millisecond rate given, with the fastest rate being 10ms
    reactive_power_closed_loop_update_rate_ms.value.set(std::max(reactive_power_closed_loop_update_rate_ms.value.value_int, 10));
    reactive_power_closed_loop_regulator.set_update_rate(1000 / reactive_power_closed_loop_update_rate_ms.value.value_int);
    // Set up steady state deadband condition as false for any value above the deadband
    reactive_power_closed_loop_regulator.set_default_condition(reactive_power_closed_loop_steady_state_deadband_kW.value.value_float, Variable_Regulator::VALUE_ABOVE);
    // Set up regulation deadband condition as false for any value above the accuracy deadband compared to the POI
    reactive_power_closed_loop_regulator.set_control_high_threshold(reactive_power_closed_loop_regulation_deadband_kW.value.value_float);
    reactive_power_closed_loop_regulator.set_decrease_timer_duration_ms(reactive_power_closed_loop_decrease_timer_ms.value.value_int);
    // Set up regulation deadband condition as false for any value below the accuracy deadband compared to the POI
    reactive_power_closed_loop_regulator.set_control_low_threshold(-1.0f * reactive_power_closed_loop_regulation_deadband_kW.value.value_float);
    reactive_power_closed_loop_regulator.set_increase_timer_duration_ms(reactive_power_closed_loop_increase_timer_ms.value.value_int);

    // initialize internal variables with config vars
    active_power_setpoint_mode.kW_slew.set_slew_rate(active_power_setpoint_mode.kW_slew_rate.value.value_int);
    manual_solar_kW_slew.set_slew_rate(manual_solar_kW_slew_rate.value.value_int);
    manual_ess_kW_slew.set_slew_rate(manual_ess_kW_slew_rate.value.value_int);
    manual_gen_kW_slew.set_slew_rate(manual_gen_kW_slew_rate.value.value_int);
    ess_kVAR_cmd_slew.set_slew_rate(ess_kVAR_slew_rate.value.value_int);
    gen_kVAR_cmd_slew.set_slew_rate(gen_kVAR_slew_rate.value.value_int);
    solar_kVAR_cmd_slew.set_slew_rate(solar_kVAR_slew_rate.value.value_int);

    // lower first_gen_is_starting in case it was not cleared by a started generator in startup.
    pAssets->set_first_gen_is_starting_flag(false);
}

// error state - should never run :)
void Site_Manager::error_state(void) {
    // set internal vars to 0
    // FPS_ERROR_LOG("site manager Error State executed \n");
}

// ready state - site is not faulted, and not yet running. waiting for start cmd
void Site_Manager::ready_state(void) {
    // if enable cmd is true and disable cmd is false, move to startup state
    if (enable_flag.value.value_bool && !disable_flag.value.value_bool) {
        FPS_INFO_LOG("site manager received enable command \n");
        current_state = Startup;
    }
}

// startup state - all startup logic triggered from sequences
void Site_Manager::startup_state(void) {
    return;
}

/**
 * Run Mode 1 is one of two running modes and is typically considered to be the "grid tied"
 * running mode. In this function, runmode1 active/reactive power features, runmode1
 * standalone power features, and protection operations are executed.
 */
void Site_Manager::runmode1_state(void) {
    // Ensure runmode1 solar curtailment is enabled
    if (num_solar_running > 0)
        pAssets->set_solar_curtailment_enabled(true);

    // if the currently selected runmode1 kW feature uses charge control, set asset_cmd internal ess charge kW request variable (and limit it)
    if (charge_control.enable_flag.value.value_bool)
        asset_cmd.ess_data.kW_request = range_check(ess_charge_control_kW_request.value.value_float, ess_charge_control_kW_limit.value.value_float, -1 * ess_charge_control_kW_limit.value.value_float);

    // Active Power Features

    // all runmode1 active power features contained here
    process_runmode1_kW_feature();

    // when enabled, Aggregated Asset Limit will not allow controllable ESS to cause all ESS+Solar to exceed feature limit
    if (agg_asset_limit.enable_flag.value.value_bool)
        apply_aggregated_asset_limit(pAssets->get_ess_total_uncontrollable_active_power(), pAssets->get_solar_total_uncontrollable_active_power());

    // when enabled, ESS Discharge Prevention feature will not allow ESS to discharge if average SOC is below the configured EDP SoC threshold
    if (ess_discharge_prevention.enable_flag.value.value_bool)
        prevent_ess_discharge();

    // call PFR to adjust site_kW_demand and ess_kW_request as needed
    if (pfr.enable_flag.value.value_bool)
        primary_frequency_response(asset_cmd.site_kW_demand);

    // limit power values based on the amount of power that the POI can legally/physically handle
    if (active_power_poi_limits.enable_flag.value.value_bool)
        apply_active_power_poi_limits();

    // Preserve demand prior to POI corrections
    asset_cmd.preserve_uncorrected_site_kW_demand();

    // reinit internal debug value
    charge_dispatch_kW_command.value.set(0.0f);

    // adjust power request to account for transformer losses at the poi
    if (watt_watt.enable_flag.value.value_bool)
        watt_watt_poi_adjustment();

    // close the loop on active power, taking into account the value at the POI and making adjustments as needed
    if (active_power_closed_loop.enable_flag.value.value_bool)
        active_power_closed_loop.execute(asset_cmd, feeder_actual_kW.value.value_float, total_site_kW_rated_charge.value.value_float, total_site_kW_rated_discharge.value.value_float);
    // Reset when disabled
    else
        active_power_closed_loop.regulator.reset();

    // dispatch the active power request to the various assets
    dispatch_active_power(asset_priority_runmode1.value.value_int);

    // charge ess to unload gen + solar during their rampdown if they're too slow
    if (active_power_setpoint_mode.ess_charge_support_enable_flag.value.value_bool) {
        active_power_setpoint_mode.charge_support_execute(asset_cmd);
    }

    // Reactive Power Features

    // all runmode1 reactive power features contained here
    process_runmode1_kVAR_feature();

    if (reactive_power_poi_limits.enable_flag.value.value_bool)
        apply_reactive_power_poi_limits();

    // close the loop on reactive power, taking into account the value at the POI and making adjustments as needed
    if (reactive_power_closed_loop.enable_flag.value.value_bool)
        calculate_reactive_power_closed_loop_offset();
    // Reset when disabled
    else
        reactive_power_closed_loop_regulator.reset();

    // dispatch the reactive power request to the various assets
    asset_cmd.dispatch_reactive_power();

    // set power cmds after all algorithms have opportunity to execute
    set_asset_power_commands();

    // Calculate final reported site limits
    calculate_total_site_kW_limits();

    // start first gen if ESS SOC < setpoint
    pAssets->start_first_gen((asset_cmd.gen_data.start_first_flag || (soc_avg_running.value.value_float <= start_first_gen_soc.value.value_float)) && asset_cmd.gen_data.auto_restart_flag);

    // start any available ESS stopped or in standby
    if (asset_cmd.ess_data.auto_restart_flag)
        pAssets->start_available_ess();

    // start any available Solar stopped or in standby
    if (asset_cmd.solar_data.auto_restart_flag)
        pAssets->start_available_solar();

    // enter standby mode
    if (standby_flag.value.value_bool) {
        standby_ess_latch = false;
        standby_solar_latch = false;
        current_state = Standby;
    }
}

/**
 * Run Mode 2 is one of two running modes and is typically considered to be the "islanded"
 * running mode. In this function, runmode2 active power features, runmode2 standalone
 * power features, and protection operations are executed.
 */
void Site_Manager::runmode2_state(void) {
    // Ensure runmode1 solar curtailment is disabled
    if (num_solar_running > 0)
        pAssets->set_solar_curtailment_enabled(false);

    // update solar shed value
    if (solar_shed.enable_flag.value.value_bool)
        calculate_solar_shed();

    // all runmode2 active power features contained here
    process_runmode2_kW_feature();

    // divide the active power demand between the various assets
    dispatch_active_power(asset_priority_runmode2.value.value_int);

    // update load shed value
    if (load_shed.enable_flag.value.value_bool)
        calculate_load_shed();

    // after deciding which assets get how much power, send out the power commands
    set_asset_power_commands();

    // start any available ESS stopped or in standby
    if (asset_cmd.ess_data.auto_restart_flag)
        pAssets->start_available_ess();

    // start any available Solar stopped or in standby
    if (asset_cmd.solar_data.auto_restart_flag)
        pAssets->start_available_solar();

    // start first gen if ESS SOC < setpoint
    pAssets->start_first_gen((asset_cmd.gen_data.start_first_flag || (soc_avg_running.value.value_float <= start_first_gen_soc.value.value_float)) && asset_cmd.gen_data.auto_restart_flag);
}

// standby state - site is running, but no power features enabled.
void Site_Manager::standby_state(void) {
    // reset internal variables
    site_kW_demand.value.set(0.0f);
    ess_kW_cmd.value.set(0.0f);
    feeder_kW_cmd.value.set(0.0f);
    gen_kW_cmd.value.set(0.0f);
    solar_kW_cmd.value.set(0.0f);
    ess_kVAR_cmd.value.set(ess_kVAR_cmd_slew.get_slew_target(0.0f));
    gen_kVAR_cmd.value.set(gen_kVAR_cmd_slew.get_slew_target(0.0f));
    solar_kVAR_cmd.value.set(solar_kVAR_cmd_slew.get_slew_target(0.0f));

    // after power cmds ramp to 0, send standby mode cmd (also if num_controllable > 0)
    if (prev_solar_kW_cmd == 0 && prev_solar_kVAR_cmd == 0 && (!standby_solar_latch || pAssets->get_num_solar_controllable() > 0)) {
        pAssets->enter_standby_all_solar();
        standby_solar_latch = true;
    }

    if (prev_ess_kW_cmd == 0 && prev_ess_kVAR_cmd == 0 && (!standby_ess_latch || pAssets->get_num_ess_controllable() > 0)) {
        pAssets->enter_standby_all_ess();
        standby_ess_latch = true;
    }

    // leaving standby
    if (!standby_flag.value.value_bool) {
        // disable standby mode for assets
        pAssets->exit_standby_all_ess();
        pAssets->exit_standby_all_solar();

        // TODO:future consideration - is logic needed to determine which RunMode to enter
        current_state = RunMode1;
    }

    return;
}

// shutdown state - occurs when shutdown cmd or fault occur
void Site_Manager::shutdown_state(void) {
    // reset internal variables
    ess_kW_cmd.value.set(0.0f);
    gen_kW_cmd.value.set(0.0f);
    solar_kW_cmd.value.set(0.0f);
    feeder_kW_cmd.value.set(0.0f);
    site_kW_demand.value.set(0.0f);
    site_kW_load.value.set(0.0f);
    ess_kVAR_cmd.value.set(0.0f);
    gen_kVAR_cmd.value.set(0.0f);
    solar_kVAR_cmd.value.set(0.0f);
    active_power_setpoint_mode.kW_slew.reset_slew_target();
    manual_solar_kW_slew.reset_slew_target();
    manual_ess_kW_slew.reset_slew_target();
    manual_gen_kW_slew.reset_slew_target();
    gen_kVAR_cmd_slew.reset_slew_target();
    ess_kVAR_cmd_slew.reset_slew_target();
    solar_kVAR_cmd_slew.reset_slew_target();
    reactive_setpoint_kVAR_cmd_slew.reset_slew_target();

    // Reset load shed value to max
    load_shed_calculator.offset = load_shed_calculator.max_offset;
    if (load_shed_value.value.value_int != load_shed_calculator.offset) {
        load_shed_value.value.set(load_shed_calculator.offset);
        load_shed_value.send_to_component(true);
    }

    // Reset solar shed value to max
    solar_shed_calculator.offset = solar_shed_calculator.max_offset;
    solar_shed_value.value.set(solar_shed_calculator.offset);

    // Reset closed loop control value to default
    active_power_closed_loop.regulator.offset = active_power_closed_loop.default_offset.value.value_float;

    // tell dbi not to execute start cmd if restart / power cycle happens
    if (data_endpoint != NULL && enable_flag.value.value_bool != false) {
        data_endpoint->turn_off_start_cmd();
    }

    if (sequences[Shutdown].sequence_bypass == true)
        current_state = Ready;

    // set all inputs to enable_flag to false
    clear_faults_flag.value.set(false);
    for (auto& input_flag : clear_faults_flag.inputs) {
        input_flag.set(false);
    }

    // set start asset flags false since algorithm isnt running to check
    asset_cmd.ess_data.start_first_flag = false;
    asset_cmd.gen_data.start_first_flag = false;
    asset_cmd.solar_data.start_first_flag = false;

    return;
    // FPS_ERROR_LOG("site manager Shutdown State executed \n");
}

/**
 * When in runmode1 mode, calls the selected active power feature that will determine the target
 * active power output of assets and modify variables such as site_kW_demand and ess_kW_request.
 *
 * The active power features listed in this function are grid-compatible, whereas non-listed active power
 * features are only allowed during islanded/runmode2 mode.
 */
void Site_Manager::process_runmode1_kW_feature() {
    // TARGET SOC MODE set a site demand based on load, and provide a minimum for solar and ess discharge
    if (target_soc.enable_flag.value.value_bool) {
        target_soc.execute(asset_cmd);
    }
    // SITE EXPORT TARGET MODE takes a single power command that is distributed to solar and ess, using dispatch and charge control
    else if (active_power_setpoint_mode.enable_flag.value.value_bool) {
        active_power_setpoint_mode.execute(asset_cmd);
    }
    // MANUAL MODE takes an ESS kW cmd, solar kW cmd, generator kW cmd, ESS slew rate, solar slew rate, and generator slew rate and routes those commands through dispatch and charge control
    else if (manual_power_mode.enable_flag.value.value_bool) {
        asset_cmd.manual_mode(manual_ess_kW_cmd.value.value_float, manual_solar_kW_cmd.value.value_float, manual_gen_kW_cmd.value.value_float, &manual_ess_kW_slew, &manual_solar_kW_slew, &manual_gen_kW_slew);
    }
    // FR MODE (frequency response) - output or absorb additional power if frequency deviates by a set amount
    else if (frequency_response_feature.enable_flag.value.value_bool) {
        execute_frequency_response_feature();
    }
    // ENERGY ARBITRAGE MODE determines storage charge/discharge based on current price and thresholds
    else if (energy_arbitrage_feature.enable_flag.value.value_bool) {
        energy_arbitrage_helper();
    }
    // ESS Calibration Mode takes an ESS kW_cmd and routes it to each ESS without balancing, and with optional soc and voltage limits
    else if (ess_calibration.enable_flag.value.value_bool) {
        asset_cmd.ess_calibration_mode(ess_calibration_kW_cmd.value.value_float, pAssets->get_num_ess_controllable());
    }

    asset_cmd.calculate_feature_kW_demand(asset_priority_runmode1.value.value_int);
}

// run frequency response algorithm
void Site_Manager::execute_frequency_response_feature() {
    // get inputs
    Frequency_Response_Inputs ins{
        asset_cmd.ess_data.max_potential_kW, asset_cmd.ess_data.min_potential_kW, get_ess_total_rated_active_power(), site_frequency.value.value_float, current_time,
    };
    // call frequency response algorithm
    Frequency_Response_Outputs outs = frequency_response.aggregate_response_components(ins);
    // set outputs
    asset_cmd.ess_data.max_potential_kW = outs.ess_max_potential;
    asset_cmd.ess_data.min_potential_kW = outs.ess_min_potential;
    asset_cmd.site_kW_demand = outs.output_kw;
    // Fully curtail solar. Even if configured to be larger, the UF response will not exceed the ESS (and gen) output and will not request from solar
    asset_cmd.solar_data.max_potential_kW = 0;
    // This feature does not track load
    asset_cmd.set_load_compensation_method(NO_COMPENSATION);
}

void Site_Manager::energy_arbitrage_helper() {
    Energy_Arbitrage_Inputs Energy_Arbitrage_Parameters{
        soc_avg_running.value.value_float,
        asset_cmd.solar_data.max_potential_kW,
    };

    Energy_Arbitrage_Output outputs = energy_arb_obj.energy_arbitrage(Energy_Arbitrage_Parameters);
    if (!active_alarm_array[3] && outputs.error_code == 1) {
        set_alarms(3);
    } else {
        asset_cmd.solar_data.max_potential_kW = outputs.solar_max_potential_kW;
        // Feature output assigned to ESS
        asset_cmd.ess_data.kW_request = outputs.ess_kW_power;
        // Solar fully uncurtailed, but reserve a portion equal to ESS charge request (negative feature output) if present
        asset_cmd.solar_data.kW_request = outputs.solar_kW_request;
    }
    // This feature tracks load at a minimum
    asset_cmd.load_method = LOAD_MINIMUM;
    asset_cmd.additional_load_compensation = asset_cmd_utils::calculate_additional_load_compensation(asset_cmd.load_method, asset_cmd.site_kW_load, asset_cmd.site_kW_demand, asset_cmd.ess_data.kW_request, asset_cmd.gen_data.kW_request,
                                                                                                     asset_cmd.solar_data.kW_request);
    asset_cmd.site_kW_demand += asset_cmd.additional_load_compensation;
}

/**
 * When in runmode2 mode, calls the selected active power feature that will determine the target
 * active power output of all aggregated assets and modify variables such as site_kW_demand and
 * ess_kW_request.
 *
 * The active power features listed in this function are islanded-compatible, whereas non-listed active power
 * features are only allowed during runmode1 mode.
 */
void Site_Manager::process_runmode2_kW_feature() {
    // Base case behavior
    // site_kW_demand should start by covering the site load
    asset_cmd.load_method = LOAD_MINIMUM;
    asset_cmd.additional_load_compensation = asset_cmd_utils::calculate_additional_load_compensation(asset_cmd.load_method, asset_cmd.site_kW_load, asset_cmd.site_kW_demand, asset_cmd.ess_data.kW_request, asset_cmd.gen_data.kW_request,
                                                                                                     asset_cmd.solar_data.kW_request);
    asset_cmd.site_kW_demand += asset_cmd.additional_load_compensation;

    // Request full charge which may be satisfied based on availability of other assets
    // If no other assets available, ESS will discharge to compensate for load
    asset_cmd.ess_data.kW_request = asset_cmd.ess_data.min_potential_kW;

    // Feature selection. Currently, only Generator Charge is offered
    if (generator_charge.enable_flag.value.value_bool)
        run_generator_charge();

    asset_cmd.calculate_feature_kW_demand(asset_priority_runmode2.value.value_float);
}

/**
 * Calculate a generator command based on the current ESS active power and solar shedding buffers.
 * This feature is entirely coupled with solar shedding and LDSS
 */
void Site_Manager::run_generator_charge() {
    // ESS should be controlled by Solar + Gen, leaving enough room for solar to always increase (high threshold) if it wants
    // Determine if solar needs room to increase (remember min_offset = no shedding = full solar output)
    float solar_additional_buffer = (solar_shed_calculator.offset == solar_shed_calculator.min_offset) ? 0.0f : solar_shed_high_threshold.value.value_float;
    // Determine how much solar power will actually go to the ESS, and not load
    float actual_solar_compensation = asset_cmd.solar_data.max_potential_kW - asset_cmd.site_kW_load;
    // Determine the ESS's unsatisfied charge capability that can be handled by the generator
    float available_ess_after_solar = zero_check(zero_check(-1.0f * asset_cmd.ess_data.min_potential_kW) - actual_solar_compensation);
    // Further reduce this value by the buffers
    float calc_gen_limit = zero_check(available_ess_after_solar - solar_additional_buffer - generator_charge_additional_buffer.value.value_float);
    // Take the final value as the command, and make sure gen power does not exceed this value
    float max_gen_limit = std::min(calc_gen_limit, asset_cmd.gen_data.max_potential_kW);
    asset_cmd.gen_data.max_potential_kW = max_gen_limit;
    max_potential_gen_kW.value.set(max_gen_limit);
}

/**
 * Calls the selected reactive power feature that will determine the target reactive power output
 * of all aggregated assets and modify variables such as site_kVAR_demand.
 */
void Site_Manager::process_runmode1_kVAR_feature() {
    // ACTIVE VOLTAGE REGULATION MODE will sink or supply power based on voltage deviation
    if (active_voltage_regulation.enable_flag.value.value_bool) {
        // active voltage regulation function
        active_voltage_regulation.execute(asset_cmd, asset_pf_flag);
    }
    // WATT-VAR MODE sets kVAr command based on where actual active power sits on watt_var_points curve
    else if (watt_var.enable_flag.value.value_bool) {
        // watt-var function
        watt_var_mode(ess_kW_cmd.value.value_float + solar_kW_cmd.value.value_float + gen_kW_cmd.value.value_float, watt_var_curve);

        // this mode does not use power factor control
        asset_pf_flag = false;
    }
    // REACTIVE POWER SETPOINT MODE takes a single reactive power setpoint and passes it on for asset distribution
    else if (reactive_setpoint.enable_flag.value.value_bool) {
        asset_cmd.reactive_setpoint_mode(&reactive_setpoint_kVAR_cmd_slew, reactive_setpoint_kVAR_cmd.value.value_float);

        // this mode does not use power factor control
        asset_pf_flag = false;
    }

    // POWER FACTOR MODE = tbd
    else if (power_factor.enable_flag.value.value_bool) {
        asset_cmd.site_kVAR_demand = 0;

        // this mode does uses power factor control
        asset_pf_flag = true;
    }
    // Constant Power Factor Mode
    else if (constant_power_factor.enable_flag.value.value_bool) {
        execute_constant_power_factor();
        // this mode does not use power factor control (it will if it is ever implemented)
        asset_pf_flag = false;
    } else  // TODO:use another feature to set
    {
        asset_cmd.site_kVAR_demand = 0;

        // this mode does not use power factor control
        asset_pf_flag = false;
    }

    // set UI/FIMS var equal to internal calculation for site demand
    site_kVAR_demand.value.set(asset_cmd.site_kVAR_demand);
}

// Helper function for executing constant power factor mode
void Site_Manager::execute_constant_power_factor() {
    // Ensure limits are bounded
    constant_power_factor_lagging_limit.value.set(range_check(-1.0f * std::abs(constant_power_factor_lagging_limit.value.value_float), 0.0f, -1.0f));
    constant_power_factor_leading_limit.value.set(range_check(std::abs(constant_power_factor_leading_limit.value.value_float), 1.0f, 0.0f));
    // Establish command direction
    bool cpf_direction;
    // If direction flag is enabled, use it to determine direction
    if (constant_power_factor_absolute_mode.value.value_bool)
        cpf_direction = constant_power_factor_lagging_direction.value.value_bool;
    else
        cpf_direction = std::signbit(constant_power_factor_setpoint.value.value_float);

    // Apply the appropriate limit based on direction (negative is lagging)
    if (cpf_direction) {
        // Explicitly set direction of the setpoint
        constant_power_factor_setpoint.value.set(-1.0f * std::abs(constant_power_factor_setpoint.value.value_float));
        // Bound the setpoint using lagging limit, -1.0 < pf < lagging limit
        constant_power_factor_setpoint.value.set(range_check(constant_power_factor_setpoint.value.value_float, constant_power_factor_lagging_limit.value.value_float, -1.0f));
    } else {
        // Explicitly set direction of the setpoint
        constant_power_factor_setpoint.value.set(std::abs(constant_power_factor_setpoint.value.value_float));
        // Bound the setpoint using leading limit < pf < 1.0
        constant_power_factor_setpoint.value.set(range_check(constant_power_factor_setpoint.value.value_float, 1.0f, constant_power_factor_leading_limit.value.value_float));
    }
    asset_cmd.constant_power_factor_mode(constant_power_factor_setpoint.value.value_float, cpf_direction);
}

/**
 * Adjusts site demand based on the legal/physical/etc power limits of the point of interface (poi), with poi_limits_min_kW limiting the
 * power that can be imported into the site (negative), and poi_limits_max_kW limiting the power that can be exported from the site (positive).
 * Largely works to reconcile the expected value at the POI and the site production needed to achieve this expected value.
 *
 * If tracked by the active power feature and enabled based on configuration, load (including uncontrollable) is implicit in the site demand received and is expected to
 * cancel out with site discharge production. As such, it should be removed from the demand to get the true expected value at the POI.
 */
void Site_Manager::apply_active_power_poi_limits() {
    float max_poi_limit = active_power_poi_limits_max_kW.value.value_float;
    float min_poi_limit = active_power_poi_limits_min_kW.value.value_float;

    // soc-based POI limits are enabled, overwriting the default POI limits
    if (active_power_soc_poi_limits_enable.value.value_bool) {
        // Only apply under limits if ESS are below the configured threshold
        if (soc_avg_running.value.value_float <= active_power_soc_poi_target_soc.value.value_float) {
            max_poi_limit = active_power_soc_poi_limits_low_max_kW.value.value_float;
            min_poi_limit = active_power_soc_poi_limits_low_min_kW.value.value_float;
            // Only apply over limits if ESS are above the configured threshold
        } else {
            max_poi_limit = active_power_soc_poi_limits_high_max_kW.value.value_float;
            min_poi_limit = active_power_soc_poi_limits_high_min_kW.value.value_float;
        }
    }

    // Limit explicit POI request in addition to demand
    asset_cmd.poi_cmd = range_check(asset_cmd.poi_cmd, max_poi_limit, min_poi_limit);

    // POI limits are asymmetric due to there being a single source of POI charge and multiple sources of POI discharge
    // Apply the charge POI limit
    asset_cmd.feeder_data.max_potential_kW = std::min(asset_cmd.feeder_data.max_potential_kW, -1.0f * min_poi_limit);
    asset_cmd.determine_ess_load_requirement(asset_priority_runmode1.value.value_int);

    // Limit site demand, tracking load if enabled to ensure an accurate value at the POI
    float min_limit_with_load = min_poi_limit + asset_cmd.get_site_kW_load_inclusion() * asset_cmd.site_kW_load;
    float max_limit_with_load = max_poi_limit + asset_cmd.get_site_kW_load_inclusion() * asset_cmd.site_kW_load;
    asset_cmd.site_kW_demand = range_check(asset_cmd.site_kW_demand, max_limit_with_load, min_limit_with_load);

    // Apply to reported site limits. Cancelling does not need to be calculated as only one of charge/discharge will be active at a time
    // Load must be included in all cases as the total limit represents the value that will be seen at the POI
    total_site_kW_charge_limit.value.set(range_check(total_site_kW_charge_limit.value.value_float + asset_cmd.site_kW_load, max_poi_limit, min_poi_limit));
    total_site_kW_discharge_limit.value.set(range_check(total_site_kW_discharge_limit.value.value_float + asset_cmd.site_kW_load, max_poi_limit, min_poi_limit));
}

/**
 * Adjusts site demand based on the legal/physical/etc power limits of the point of interface (poi), with reactive_power_poi_limits_min_kVAR limiting the
 * power that can be imported into the site (negative), and reactive_power_poi_limits_max_kVAR limiting the power that can be exported from the site (positive).
 *
 * No load is considered so there is no requirement to solve for the value at the POI. The limits are simply applied as is
 */
void Site_Manager::apply_reactive_power_poi_limits() {
    float min_poi_limit = reactive_power_poi_limits_min_kVAR.value.value_float;
    float max_poi_limit = reactive_power_poi_limits_max_kVAR.value.value_float;

    asset_cmd.site_kVAR_demand = range_check(asset_cmd.site_kVAR_demand, max_poi_limit, min_poi_limit);
}

/**
 * Processes the request active power from the active power feature calculation by splitting the request
 * between all asset types.
 * @param asset_priority An enumerated integer selecting the order in which assets should be assigned power.
 */
void Site_Manager::dispatch_active_power(int asset_priority) {
    // Extract the total charge and discharge available for dispatch
    asset_cmd_utils::site_kW_production_limits site_kW_prod_limits = asset_cmd_utils::calculate_site_kW_production_limits(asset_cmd.ess_data.kW_request, asset_cmd.gen_data.kW_request, asset_cmd.solar_data.kW_request, asset_cmd.load_method,
                                                                                                                          asset_cmd.additional_load_compensation, asset_cmd.feature_kW_demand, asset_cmd.site_kW_demand);
    asset_cmd.site_kW_charge_production = site_kW_prod_limits.site_kW_charge_production;
    asset_cmd.site_kW_discharge_production = site_kW_prod_limits.site_kW_discharge_production;

    // Capture variable values for UI/FIMS reporting before their modification by dispatch
    set_volatile_asset_cmd_variables();

    // Process and dispatch discharge production to meet load, if appropriate
    float aggregate_dispatch = 0.0f;
    // Handle load first if it's tracked separately from demand (MINIMUM use case). Otherwise it will be handled later in demand
    if (asset_cmd.load_method == LOAD_MINIMUM)
        aggregate_dispatch += asset_cmd.dispatch_site_kW_discharge_cmd(asset_priority, asset_cmd.site_kW_load, LOAD);

    // Process and dispatch charge production
    aggregate_dispatch += asset_cmd.dispatch_site_kW_charge_cmd(asset_priority, charge_dispatch_solar_enable_flag.value.value_bool, charge_dispatch_gen_enable_flag.value.value_bool, charge_dispatch_feeder_enable_flag.value.value_bool);

    // Process and dispatch remaining discharge production to each asset type
    aggregate_dispatch += asset_cmd.dispatch_site_kW_discharge_cmd(asset_priority, asset_cmd.site_kW_discharge_production, REQUESTS);
    aggregate_dispatch += asset_cmd.dispatch_site_kW_discharge_cmd(asset_priority, asset_cmd.site_kW_discharge_production, DEMAND);

    // set internal debug value to power from charge control
    charge_dispatch_kW_command.value.set(aggregate_dispatch);
}

/**
 * After all power calculation/adjustment algorithms have had an opportunity to execute,
 * set each asset type's command with limits applied.
 */
void Site_Manager::set_asset_power_commands() {
    // set power cmds after all algorithms have opportunity to execute
    ess_kW_cmd.value.set(asset_cmd.get_limited_ess_kW_cmd());
    feeder_kW_cmd.value.set(asset_cmd.get_limited_feeder_kW_cmd());
    gen_kW_cmd.value.set(asset_cmd.get_limited_gen_kW_cmd());
    solar_kW_cmd.value.set(asset_cmd.get_limited_solar_kW_cmd());
    ess_kVAR_cmd.value.set(ess_kVAR_cmd_slew.get_slew_target(asset_cmd.ess_data.kVAR_cmd));
    gen_kVAR_cmd.value.set(gen_kVAR_cmd_slew.get_slew_target(asset_cmd.gen_data.kVAR_cmd));
    solar_kVAR_cmd.value.set(solar_kVAR_cmd_slew.get_slew_target(asset_cmd.solar_data.kVAR_cmd));
}

// W-VAr mode function
void Site_Manager::watt_var_mode(float active_power, std::vector<std::pair<float, float>>& curve) {
    float reference_site_demand = asset_cmd.site_kVAR_demand;
    float new_site_demand = get_curve_cmd(active_power, curve);
    watt_var_correction = new_site_demand - reference_site_demand;

    asset_cmd.site_kVAR_demand = new_site_demand;
}

void Site_Manager::primary_frequency_response(float kW_cmd) {
    // include offset test input in case it is non-zero
    float freq_input = site_frequency.value.value_float + pfr_offset_hz.value.value_float;

    // get correct reference_hz for overhz or underhz event (nominal hz + or - deadband)
    float reference_hz = (freq_input > pfr_site_nominal_hz.value.value_float) ? pfr_site_nominal_hz.value.value_float + pfr_deadband.value.value_float : pfr_site_nominal_hz.value.value_float - pfr_deadband.value.value_float;

    // delta frequency deviation calculation
    float delta_hz;

    // delta calculation for overfrequency event
    if (freq_input > pfr_site_nominal_hz.value.value_float)
        // if outside pass positive delta. otherwise 0.
        delta_hz = freq_input > reference_hz ? freq_input - reference_hz : 0;
    // delta calculation for underfrequency event
    else
        // if outside the deadband, pass negative delta. otherwise 0.
        delta_hz = freq_input < reference_hz ? freq_input - reference_hz : 0;

    // no PFR event or invalid droop percent
    if (delta_hz == 0 || pfr_droop_percent.value.value_float <= 0) {
        pfr_status_flag.value.value_bool = false;
        return;
    }

    // pfr event detected, set pfr status true
    pfr_status_flag.value.value_bool = true;

    // delta frequency deviation scaled by percent slope and nominal frequency (=0 if no PFR event)
    float response_percent = (delta_hz / pfr_site_nominal_hz.value.value_float) / (.01 * pfr_droop_percent.value.value_float);

    // calculate charge/discharge limits from min/max potential
    float chg_kW_limit = less_than_zero_check(pfr_limits_min_kW.value.value_float);
    float dischg_kW_limit = zero_check(pfr_limits_max_kW.value.value_float);

    // Calculate the charge magnitude that should be used, original command direction gives us the sign to use
    float delta_chg = (kW_cmd < 0) ? fabsf(pfr_limits_min_kW.value.value_float) : pfr_limits_max_kW.value.value_float;

    // calculate pfr offset request based on overfrequency or underfrequency event
    float response_kW = -1 * response_percent * delta_chg;

    // Get the direction of the original command, or use sign of response if 0
    float base_cmd = (kW_cmd != 0) ? kW_cmd : response_kW;

    // charge direction power limit (cannot change direction)
    if (base_cmd > 0)
        chg_kW_limit = 0;
    // discharge direction power limit
    else
        dischg_kW_limit = 0;

    // Bound modified site demand by configured limits
    float calculated_response = range_check(kW_cmd + response_kW, dischg_kW_limit, chg_kW_limit);

    // Modify request and demand based on the degree of change calculated
    asset_cmd.site_kW_demand = calculated_response;
    total_site_kW_charge_limit.value.set(range_check(total_site_kW_charge_limit.value.value_float + response_kW, 0.0f, pfr_limits_min_kW.value.value_float));
    total_site_kW_discharge_limit.value.set(range_check(total_site_kW_discharge_limit.value.value_float + response_kW, pfr_limits_max_kW.value.value_float, 0.0f));
}

bool Site_Manager::synchronize_ess() {
    pAssets->set_ess_voltage_setpoint(pAssets->get_sync_feeder_gridside_avg_voltage());
    pAssets->set_ess_frequency_setpoint(pAssets->get_sync_feeder_gridside_frequency() - pAssets->get_sync_frequency_offset());  //-pAssets->get_sync_feeder_frequency_offset());

    return pAssets->get_sync_feeder_status();
}

/**
 * @brief Get ESS Calibration Mode variables from Assets
 */
void Site_Manager::get_ess_calibration_variables() {
    std::vector<int> ess_setpoint_statuses = pAssets->get_ess_setpoint_statuses();
    ess_calibration_num_setpoint.value.set(ess_setpoint_statuses[ACCEPTED]);
    ess_calibration_num_limited.value.set(ess_setpoint_statuses[LIMITED]);
    ess_calibration_num_zero.value.set(ess_setpoint_statuses[ZERO]);
}

/**
 * @brief Write ESS Calibration Mode varibles to Assets, passed as struct
 *
 */
void Site_Manager::set_ess_calibration_variables() {
    ESS_Calibration_Settings settings;
    settings.calibration_flag = ess_calibration.enable_flag.value.value_bool;
    // Soc balancing always disabled in this mode
    settings.balancing_factor = (float)!ess_calibration.enable_flag.value.value_bool * ess_soc_balancing_factor;
    // ESS power distribution balancing always disabled in this mode
    settings.power_dist_flag = !ess_calibration.enable_flag.value.value_bool;
    // limits override always enabled in this mode
    settings.limits_override = ess_calibration.enable_flag.value.value_bool;
    settings.soc_limits_flag = ess_calibration_soc_limits_enable.value.value_bool;
    settings.min_soc_limit = ess_calibration_min_soc_limit.value.value_float;
    settings.max_soc_limit = ess_calibration_max_soc_limit.value.value_float;
    settings.voltage_limits = ess_calibration_voltage_limits_enable.value.value_bool;
    settings.min_voltage_limit = ess_calibration_min_voltage_limit.value.value_float;
    settings.max_voltage_limit = ess_calibration_max_voltage_limit.value.value_float;
    // Write through the feature setpoint as well so we have a reference of whether it's being met for each asset
    settings.raw_feature_setpoint = ess_calibration_kW_cmd.value.value_float;
    pAssets->set_ess_calibration_vars(settings);
}

// Checks for watchdog timeout and increments heartbeat counter
void Site_Manager::watchdog() {
    // If the watchdog has received a new pet from the master controller
    if (watchdog_old_pet != watchdog_pet.value.value_int) {
        watchdog_old_pet = watchdog_pet.value.value_int;                                // Update old pet
        clock_gettime(CLOCK_MONOTONIC, &watchdog_timeout);                              // Get current time
        increment_timespec_ms(watchdog_timeout, watchdog_duration_ms.value.value_int);  // Reset watchdog expiration time
    }
    // If no new pet and the watchdog has timed out
    else if (check_expired_time(current_time, watchdog_timeout)) {
        dogbark();  // Execute watchdog failure responses
    }

    // If it is time for the heart to send out another beat (signaling to master controller that site_controller is still operational)
    if (check_expired_time(current_time, heartbeat_timer)) {
        ++heartbeat_counter.value.value_int;                                            // Update counter that will be published for master controller
        clock_gettime(CLOCK_MONOTONIC, &heartbeat_timer);                               // Get current time
        increment_timespec_ms(heartbeat_timer, heartbeat_duration_ms.value.value_int);  // Reset heartbeat timer
    }
}

// Executes watchdog failure reponses
void Site_Manager::dogbark() {
    // If watchdog has already barked, don't bark again
    if (!active_alarm_array[4]) {
        set_alarms(4);
    }
}

// Adjusts power requests to account for transformer losses at the POI
void Site_Manager::watt_watt_poi_adjustment() {
    float reference_site_demand = asset_cmd.site_kW_demand;
    float new_site_demand = get_curve_cmd(asset_cmd.site_kW_demand, watt_watt_curve);
    watt_watt_correction = new_site_demand - reference_site_demand;

    // Set asset cmd value
    asset_cmd.site_kW_demand = new_site_demand;
}

/**
 * Aggregates LDSS feature settings and passes them down to update the feature.
 */
void Site_Manager::set_ldss_variables() {
    auto ldss_settings = LDSS_Settings();
    ldss_settings.enabled = ldss.enable_flag.value.value_bool;
    ldss_settings.priority_setting = (LDSS_Priority_Setting)ldss_priority_setting.value.value_int;
    ldss_settings.max_load_threshold_percent = ldss_max_load_threshold_percent.value.value_float;
    ldss_settings.min_load_threshold_percent = ldss_min_load_threshold_percent.value.value_float;
    ldss_settings.warmup_time = ldss_warmup_time.value.value_int;
    ldss_settings.cooldown_time = ldss_cooldown_time.value.value_int;
    ldss_settings.start_gen_time = ldss_start_gen_time.value.value_int;
    ldss_settings.stop_gen_time = ldss_stop_gen_time.value.value_int;
    ldss_settings.enable_soc_thresholds_flag = ldss_enable_soc_threshold.value.value_bool;
    ldss_settings.min_soc_percent = ldss_min_soc_threshold_percent.value.value_float;
    ldss_settings.max_soc_percent = ldss_max_soc_threshold_percent.value.value_float;
    pAssets->update_ldss_settings(std::move(ldss_settings));
}

// Updates the load shed value and writes out the value if it changes
void Site_Manager::calculate_load_shed() {
    int prev_load_shed = load_shed_calculator.offset;
    // load shed value depends on soc and (dischargeable power - measured power)
    load_shed_calculator.regulate_variable_offset(soc_avg_running.value.value_float, load_shed_spare_ess_kw.value.value_float);
    int curr_load_shed = load_shed_calculator.offset;
    // Write out the value on a change
    if (curr_load_shed != prev_load_shed) {
        load_shed_value.value.set(curr_load_shed);
        load_shed_value.send_to_component(true);
    }
}

// Updates the solar shed value and curtails solar power accordingly
void Site_Manager::calculate_solar_shed() {
    // solar shed value depends on soc and (chargeable power - measured charging power)
    solar_shed_calculator.regulate_variable_offset(soc_avg_running.value.value_float, solar_shed_spare_ess_kw.value.value_float);
    solar_shed_value.value.set(solar_shed_calculator.offset);
    // set solar max potential active power based on shed value
    // linearly map max shed to 0% power and min shed to 100% power
    float percent_shedding = ((float)solar_shed_calculator.max_offset - solar_shed_calculator.offset) / (solar_shed_calculator.max_offset - solar_shed_calculator.min_offset);
    float solar_max = pAssets->get_solar_total_rated_active_power() * percent_shedding;
    // max solar power is still limited by existing max limit
    solar_max = std::min(solar_max, asset_cmd.solar_data.max_potential_kW);

    asset_cmd.solar_data.max_potential_kW = solar_max;
    max_potential_solar_kW.value.value_float = solar_max;
}

// Updates the closed loop control offset value to correct for inaccuracies in reactive power at the POI
void Site_Manager::calculate_reactive_power_closed_loop_offset() {
    // Always invert POI, using the convention that positive power flows into the site and negative power flows out of the site at the POI
    float poi_value = -1.0f * feeder_actual_kVAR.value.value_float;
    float current_cmd = asset_cmd.site_kVAR_demand;

    // Make sure a step size is configured to prevent divide by zero -> NaN
    if (reactive_power_closed_loop_step_size_kW.value.value_float == 0.0f)
        reactive_power_closed_loop_step_size_kW.value.value_float = 1.0f;
    // Reset offset limits
    reactive_power_closed_loop_regulator.min_offset = reactive_power_closed_loop_min_offset.value.value_int;
    reactive_power_closed_loop_regulator.max_offset = reactive_power_closed_loop_max_offset.value.value_int;

    // First calculate remaining available power for negative corrections
    float negative_charge_increase = less_than_zero_check(total_site_kVAR_rated_charge - current_cmd);
    float negative_discharge_decrease = std::min(total_site_kVAR_rated_discharge, zero_check(current_cmd));
    float negative_power_available = negative_charge_increase - negative_discharge_decrease;
    // Next calculate remaining available power for positive corrections
    float positive_charge_decrease = std::max(total_site_kVAR_rated_charge, less_than_zero_check(current_cmd));
    float positive_discharge_increase = zero_check(total_site_kVAR_rated_discharge - current_cmd);
    float positive_power_available = positive_discharge_increase - positive_charge_decrease;

    // Then calculate the min/max offset
    int min_allowable_offset = negative_power_available / reactive_power_closed_loop_step_size_kW.value.value_float;
    int max_allowable_offset = positive_power_available / reactive_power_closed_loop_step_size_kW.value.value_float;
    // Finally update regulator limits
    if (reactive_power_closed_loop_regulator.min_offset < min_allowable_offset)
        reactive_power_closed_loop_regulator.min_offset = min_allowable_offset;
    if (reactive_power_closed_loop_regulator.max_offset > max_allowable_offset)
        reactive_power_closed_loop_regulator.max_offset = max_allowable_offset;

    // Regulate based on configured deadbands compared to difference in dispatched commands (steady state) and difference in command and POI (accuracy)
    bool command_accepted = reactive_power_closed_loop_regulator.regulate_variable_offset(std::abs(current_cmd - prev_reactive_power_feature_cmd), poi_value - current_cmd);

    // TODO: if the offset is beyond the new min/max, reset it so it doesn't get stuck
    //       in the future we should count up/down until we're within range again, but changing the regulator itself carries more risk
    reactive_power_closed_loop_regulator.offset = range_check(reactive_power_closed_loop_regulator.offset, reactive_power_closed_loop_regulator.max_offset, reactive_power_closed_loop_regulator.min_offset);

    // Take the product of the offset and step size as the total correction applied to the reactive power feature's command
    reactive_power_closed_loop_total_correction.value.set(reactive_power_closed_loop_regulator.offset * reactive_power_closed_loop_step_size_kW.value.value_float);
    asset_cmd.site_kVAR_demand += reactive_power_closed_loop_total_correction.value.value_float;
    // Preserve the reactive power feature command to be dispatched, but only if closed loop control had a chance to act on the command
    if (command_accepted)
        prev_reactive_power_feature_cmd = current_cmd;
}

/**
 * @brief When the average running ESS SoC is below a configured value,
 * sets the ESS kW limits to be no more than 0kW, disallowing discharges.
 *
 */
void Site_Manager::prevent_ess_discharge() {
    if (soc_avg_running.value.value_float <= edp_soc.value.value_float) {
        if (asset_cmd.ess_data.max_potential_kW > 0) {
            max_potential_ess_kW.value.set(0.0f);
            asset_cmd.ess_data.max_potential_kW = max_potential_ess_kW.value.value_float;

            // Adjusted the amount of ESS power considered available for discharge
            total_asset_kW_discharge_limit -= pAssets->get_ess_total_kW_discharge_limit();
        }
        if (asset_cmd.ess_data.min_potential_kW > 0) {
            min_potential_ess_kW.value.set(0.0f);
            asset_cmd.ess_data.min_potential_kW = min_potential_ess_kW.value.value_float;
        }
    }
}

/**
 * @brief This feature exclusively limits ESS, and exclusively limits it in the Discharge direction.
 * Sums the aggregate of solar output and the output of any ESS in maintenance mode (uncontrolled).
 * Sets the controllable ESS limit such that the controllable ESS will not cause the total of all
 * solar+ESS to exceed the feature limit. If the solar and/or uncontrollable ESS exceeds the feature
 * limit by themselves, that is allowed.
 *
 * Note for context: This feature was originally developed to satisfy a specific customer request where
 * solar is expected to always be below the limit, so just implementing logic to limit the ESS is
 * satisfactory.
 *
 */
void Site_Manager::apply_aggregated_asset_limit(float uncontrolled_ess_kw, float uncontrolled_solar_kw) {
    float solar_power = asset_cmd.solar_data.actual_kW + uncontrolled_solar_kw;
    float ess_limit = zero_check(agg_asset_limit_kw.value.value_float - solar_power - uncontrolled_ess_kw);

    // if calculated limit is below current limit, lower the current limit
    if (ess_limit < asset_cmd.ess_data.max_potential_kW) {
        max_potential_ess_kW.value.set(zero_check(ess_limit));
        asset_cmd.ess_data.max_potential_kW = max_potential_ess_kW.value.value_float;

// Adjusted the amount of ESS power considered available for discharge
#ifndef FPS_TEST_MODE
        total_asset_kW_discharge_limit -= pAssets->get_ess_total_kW_discharge_limit() + ess_limit;
#endif

        // if new max is below what the min was, lower the min as well
        if (asset_cmd.ess_data.min_potential_kW > max_potential_ess_kW.value.value_float) {
            min_potential_ess_kW.value.set(max_potential_ess_kW.value.value_float);
            asset_cmd.ess_data.min_potential_kW = min_potential_ess_kW.value.value_float;
        }
    }
}

Fims_Object Site_Manager::get_reserved_bool_1() {
    return reserved_bool_1;
}

Fims_Object Site_Manager::get_reserved_bool_2() {
    return reserved_bool_2;
}

Fims_Object Site_Manager::get_reserved_bool_3() {
    return reserved_bool_3;
}

Fims_Object Site_Manager::get_reserved_bool_4() {
    return reserved_bool_4;
}

Fims_Object Site_Manager::get_reserved_bool_5() {
    return reserved_bool_5;
}

Fims_Object Site_Manager::get_reserved_bool_6() {
    return reserved_bool_6;
}

Fims_Object Site_Manager::get_reserved_bool_7() {
    return reserved_bool_7;
}

Fims_Object Site_Manager::get_reserved_bool_8() {
    return reserved_bool_8;
}

Fims_Object Site_Manager::get_reserved_bool_9() {
    return reserved_bool_9;
}

Fims_Object Site_Manager::get_reserved_bool_10() {
    return reserved_bool_10;
}

Fims_Object Site_Manager::get_reserved_bool_11() {
    return reserved_bool_11;
}

Fims_Object Site_Manager::get_reserved_bool_12() {
    return reserved_bool_12;
}

Fims_Object Site_Manager::get_reserved_bool_13() {
    return reserved_bool_13;
}

Fims_Object Site_Manager::get_reserved_bool_14() {
    return reserved_bool_14;
}

Fims_Object Site_Manager::get_reserved_bool_15() {
    return reserved_bool_15;
}

Fims_Object Site_Manager::get_reserved_bool_16() {
    return reserved_bool_16;
}

float Site_Manager::get_ess_total_rated_active_power() {
    return pAssets->get_ess_total_rated_active_power();
}

// set ui_enabled to false for all feature vars of all features of the given feature set
void Site_Manager::ui_disable_feature_group_feature_vars(std::vector<Feature*>& feature_list) {
    for (auto it : feature_list) {
        it->toggle_ui_enabled(false);
    }
}

// add all feature vars from a list of features to cJSON object
void Site_Manager::add_feature_group_feature_vars_to_JSON_buffer(const std::vector<Feature*>& feature_list, fmt::memory_buffer& buf, const char* const var) {
    for (auto it : feature_list) {
        // only add features that are configured to be available
        if (it->available)
            it->add_feature_vars_to_JSON_buffer(buf, var);
    }
}

// add all summary vars from a list of features to cJSON object
void Site_Manager::add_feature_group_summary_vars_to_JSON_buffer(const std::vector<Feature*>& feature_list, fmt::memory_buffer& buf, const char* const var) {
    for (auto it : feature_list) {
        // only add features that are configured to be available
        if (it->available) {
            it->enable_flag.add_to_JSON_buffer(buf, var, false);
            it->add_summary_vars_to_JSON_buffer(buf, var);
        }
    }
}
