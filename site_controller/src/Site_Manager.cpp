/*
 * Site_Manager.cpp
 *
 *  Created on: Sep 5, 2018
 *      Author: jcalcagni
 */

/* C Standard Library Dependencies */
#include "Types.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
/* C++ Standard Library Dependencies */
#include <stdexcept>
#include <string>
#include <vector>
#include <set>
/* External Dependencies */
/* System Internal Dependencies */
#include <Logger.h>
#include <fims/libfims.h>
/* Local Internal Dependencies */
#include <Sequence.h>
#include <Site_Manager.h>
#include <Data_Endpoint.h>
#include <Site_Controller_Utils.h>
#include <Reference_Configs.h>

extern Data_Endpoint* data_endpoint;

Site_Manager::Site_Manager(Version* release_version) {
    if (release_version != NULL) {
        release_version_tag.value.set(release_version->get_tag());
        release_version_commit.value.set(release_version->get_commit());
        release_version_build.value.set(release_version->get_build());
    }

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
    asset_pf_flag = false;
    prev_asset_pf_flag = true;
    current_runmode1_kW_feature = -1;
    current_runmode2_kW_feature = -1;
    current_runmode1_kVAR_feature = -1;

    for (int i = 0; i < 64; i++) {
        active_fault_array[i] = false;
    }
    for (int i = 0; i < 64; i++) {
        active_alarm_array[i] = false;
    }
    num_path_faults = 0;
    num_path_alarms = 0;

    clock_gettime(CLOCK_MONOTONIC, &sequences_status.current_time);
    clock_gettime(CLOCK_MONOTONIC, &sequences_status.exit_target_time);
    clock_gettime(CLOCK_MONOTONIC, &time_to_clear_fault_status_flags);

    clear_fault_status_flags = false;

    // Initialize variable id's of variables that are not in variables.json but still receive FIMS sets
    cops_heartbeat.set_variable_id("cops_heartbeat");
}

// Perform any initialization of feature objects which must occur after configuration is parsed
void Site_Manager::post_configure_initialize_features(void) {
    //
    // Run Mode 1 Active Power Features
    //

    // Copy SoC balancing factor to be preserved in Site Manager and overwritten in Asset Manager when this feature is enabled
    ess_soc_balancing_factor = pAssets->get_soc_balancing_factor();
    // Disable all features as part of initial configuration in case any are accidentally enabled
    for (auto feature : runmode1_kW_features_list)
        feature->enable_flag.value.set(false);

    //
    // Run Mode 2 Active Power Features
    //

    // Disable all features as part of initial configuration in case any are accidentally enabled
    for (auto feature : runmode2_kW_features_list)
        feature->enable_flag.value.set(false);

    //
    // Charge features
    //
    // charge dispatch is a special case in that it should always be available and enabled
    charge_dispatch.enable_flag.value.value_bool = true;
    charge_dispatch.toggle_ui_enabled(true);

    //
    // Run Mode 1 Reactive Power Features
    //
    //
    // Disable all features as part of initial configuration in case any are accidentally enabled
    for (auto feature : runmode1_kVAR_features_list)
        feature->enable_flag.value.set(false);
    // AVR feature_vars depend on configuration, so initialize feature_vars now
    avr.initialize_feature_vars();

    //
    // Run Mode 2 Reactive Power Features
    //
    // no runmode2 reactive power features yet

    //
    // Standalone Power Features
    //
    // send Generator Manager the initial LDSS settings
    set_ldss_variables();

    // Frequency Response Mode
    // frequency response feature vars must be loaded after configuration parsing to
    // properly load all frequency response component feature vars
    frequency_response.get_feature_vars(frequency_response.feature_vars);
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

    // include site_frequency since PFR uses it
    site_frequency.add_to_JSON_buffer(buf, var);

    if (var == NULL)
        bufJSON_EndObject(buf);  // } JSON_object
}

void Site_Manager::build_JSON_features_site_operation(fmt::memory_buffer& buf, const char* const var) {
    if (var == NULL) {
        bufJSON_StartObject(buf);  // JSON_object {
    }
    bufJSON_AddStringCheckVar(buf, "name", "Site Operation Features", var);

    // site operation features feature variables
    power_priority_flag.add_to_JSON_buffer(buf, var);
    allow_ess_auto_restart.add_to_JSON_buffer(buf, var);
    allow_solar_auto_restart.add_to_JSON_buffer(buf, var);
    allow_gen_auto_restart.add_to_JSON_buffer(buf, var);
    add_feature_group_feature_vars_to_JSON_buffer(site_operation_features_list, buf, var);

    if (var == NULL) {
        bufJSON_EndObject(buf);  // } JSON_object
    }
}

void Site_Manager::build_JSON_site(fmt::memory_buffer& buf) {
    bufJSON_StartObject(buf);  // JSON_object {
    bufJSON_AddId(buf, "summary");
    build_JSON_site_summary(buf, true);
    bufJSON_AddId(buf, "operation");
    build_JSON_site_operation(buf);
    bufJSON_AddId(buf, "configuration");
    build_JSON_site_configuration(buf);
    bufJSON_AddId(buf, "input_sources");
    build_JSON_site_input_sources(buf, nullptr);

    bufJSON_EndObject(buf);  // } JSON_object
}

void Site_Manager::build_JSON_site_summary(fmt::memory_buffer& buf, bool clothed, const char* const var) {
    if (var == NULL)
        bufJSON_StartObject(buf);  // JSON_object {

    bufJSON_AddStringCheckVar(buf, "name", "Site Summary", var);

    // ops status
    site_state.add_to_JSON_buffer(buf, var, clothed);
    site_state_enum.add_to_JSON_buffer(buf, var, clothed);
    alt_site_state_enum.add_to_JSON_buffer(buf, var, clothed);
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
    alt_site_state_enum.add_to_JSON_buffer(buf, var);
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
    if (var == NULL) {
        bufJSON_StartObject(buf);  // JSON_object {
    }
    bufJSON_AddStringCheckVar(buf, "name", "Configuration", var);

    // config status
    // input source status
    input_source_status.add_to_JSON_buffer(buf, var);

    configured_primary.add_to_JSON_buffer(buf, var);
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
    if (var == nullptr) {
        bufJSON_StartObject(buf);  // JSON_object {
        bufJSON_AddStringCheckVar(buf, "name", "Input Sources", var);
        for (auto& ctl : input_sources_controls) {
            ctl.add_to_JSON_buffer(buf);
        }
        bufJSON_EndObject(buf);  // } JSON_object
    } else {
        const size_t length = strnlen(var, MAX_VARIABLE_LENGTH);
        if (length == MAX_VARIABLE_LENGTH) {
            FPS_WARNING_LOG("Input Source variable id of longer than %d characters. This is not supported. Skipping.", MAX_VARIABLE_LENGTH);
            return;
        }
        for (auto& ctl : input_sources_controls) {
            if (strncmp(ctl.get_variable_id(), var, MAX_VARIABLE_LENGTH - 1) == 0) {
                ctl.add_to_JSON_buffer(buf, var);
            }
        }
    }
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

            if (!current_sequence.parse(JSON_single_sequence, (states)(i))) {
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

    // input_source_status string should be programmatically set based on the name of the currently selected input source.
    // initial value of input_source_status in variables.json is not actually used since it is too easy for configurator to forget to match it with the initially selected
    // input source. it is only present to satisfy parser
    if (input_sources.get_num_sources() > 0)
        input_source_status.value.set(input_sources.get_name_of_input(input_sources.get_selected_input_source_index()));

    // post-configuration initialization of features
    post_configure_initialize_features();

    configure_persistent_settings_pairs();

    // Record the PID
    process_id.value.set(getpid());

    // set the primary_controller status to the value configured in variables
    // Will still be updated by COPS if it is running
    *is_primary = configured_primary.value.value_bool;
    return true;
}

/**
 * @brief Parses the field defaults object in variables.json which contains default configuration at the level of
 * fields within a variable's configuration. For example, we might use the field defaults to specify that the
 * variables with unspecified var_type will default to Float.
 *
 * @param JSON_defaults The field defaults JSON object
 * @return A pair of <defaults_object, validation_result> where validation_result describes whether or not the config was valid
 */
std::pair<Fims_Object, Config_Validation_Result> Site_Manager::parse_field_defaults(cJSON* JSON_defaults) {
    Fims_Object field_defaults;

    cJSON* name = cJSON_GetObjectItem(JSON_defaults, "name");
    if (name == NULL || name->valuestring == NULL)
        return std::make_pair(field_defaults, Config_Validation_Result(false, "did not find name field"));
    field_defaults.set_name(name->valuestring);

    cJSON* unit = cJSON_GetObjectItem(JSON_defaults, "unit");
    if (unit == NULL || unit->valuestring == NULL)
        return std::make_pair(field_defaults, Config_Validation_Result(false, "did not find unit field"));
    field_defaults.set_unit(unit->valuestring);

    cJSON* ui_type = cJSON_GetObjectItem(JSON_defaults, "ui_type");
    if (ui_type == NULL || ui_type->valuestring == NULL)
        return std::make_pair(field_defaults, Config_Validation_Result(false, "did not find ui_type field"));
    field_defaults.set_ui_type(ui_type->valuestring);

    cJSON* type = cJSON_GetObjectItem(JSON_defaults, "type");
    if (type == NULL || type->valuestring == NULL)
        return std::make_pair(field_defaults, Config_Validation_Result(false, "did not find type field"));
    field_defaults.set_type(type->valuestring);

    cJSON* var_type = cJSON_GetObjectItem(JSON_defaults, "var_type");
    cJSON* value = cJSON_GetObjectItem(JSON_defaults, "value");
    if (var_type == NULL || value == NULL || var_type->valuestring == NULL)
        return std::make_pair(field_defaults, Config_Validation_Result(false, "did not find var_type and/or value fields"));

    if (strcmp(var_type->valuestring, "Float") == 0)
        field_defaults.value.set((float)value->valuedouble);
    else if (strcmp(var_type->valuestring, "Bool") == 0)
        field_defaults.value.set(value->type == cJSON_True);
    else if (strcmp(var_type->valuestring, "String") == 0)
        field_defaults.value.set(value->valuestring);
    else if (strcmp(var_type->valuestring, "Int") == 0)
        field_defaults.value.set(value->valueint);
    else
        return std::make_pair(field_defaults, Config_Validation_Result(false, "invalid var_type field"));

    cJSON* scaler = cJSON_GetObjectItem(JSON_defaults, "scaler");
    if (scaler == NULL || scaler->type != cJSON_Number)
        return std::make_pair(field_defaults, Config_Validation_Result(false, "did not find scaler field"));
    field_defaults.scaler = scaler->valueint;

    cJSON* ui_enabled = cJSON_GetObjectItem(JSON_defaults, "ui_enabled");
    if (ui_enabled == NULL || ui_enabled->type > cJSON_True)
        return std::make_pair(field_defaults, Config_Validation_Result(false, "did not find ui_enabled field"));
    field_defaults.ui_enabled = (ui_enabled->type == cJSON_True);

    cJSON* multiple_inputs = cJSON_GetObjectItem(JSON_defaults, "multiple_inputs");
    if (multiple_inputs == NULL || multiple_inputs->type > cJSON_True)
        return std::make_pair(field_defaults, Config_Validation_Result(false, "did not find multiple_inputs field"));
    field_defaults.multiple_inputs = (multiple_inputs->type == cJSON_True);

    // optional std::string can handle being unconfigured
    cJSON* write_uri = cJSON_GetObjectItem(JSON_defaults, "write_uri");
    if (write_uri != NULL && write_uri->valuestring != NULL)
        field_defaults.write_uri = write_uri->valuestring;

    return std::make_pair(field_defaults, Config_Validation_Result(true));
}

// Configure variables based on the given variables.json configuration object
bool Site_Manager::parse_variables(cJSON* variables_config_object) {
    Config_Validation_Result validation_result = Config_Validation_Result(true);

    std::set<std::string> unrecognized_variable_ids;  // filled with all variable ids in configuration, ids are removed as they are recognized

    // get the default variables.json configuration reference
    Reference_Configs reference_configs;
    cJSON* JSON_default_config_variable_object = cJSON_GetObjectItem(reference_configs.variables_json_defaults,
                                                                     "variables");  // Reference_Configs ensures "variables" object exists
    std::pair<cJSON*, Config_Validation_Result> JSON_flat_default_parse_result = parse_flatten_vars(JSON_default_config_variable_object);
    cJSON* JSON_flat_default_config = JSON_flat_default_parse_result.first;
    if (!JSON_flat_default_parse_result.second.is_valid_config || JSON_flat_default_config == NULL) {
        FPS_ERROR_LOG("There is something wrong with this build. We failed to flatten the static default variables.json: " + JSON_flat_default_parse_result.second.brief);
        exit(1);
    }

    // get the base variables object
    cJSON* JSON_variable_object = cJSON_GetObjectItem(variables_config_object, "variables");
    if (JSON_variable_object == NULL) {
        FPS_ERROR_LOG("variable object is missing from file!");
        return false;
    }

    // create a cJSON object for a flattened variables object with the variables as top-level items;
    // this way, variables aren't stuck to a particular category
    std::pair<cJSON*, Config_Validation_Result> JSON_flat_vars_parse_result = parse_flatten_vars(JSON_variable_object);
    cJSON* JSON_flat_vars = JSON_flat_vars_parse_result.first;
    if (!JSON_flat_vars_parse_result.second.is_valid_config) {
        FPS_ERROR_LOG("Failed to flatten variables.json: %s", JSON_flat_vars_parse_result.second.brief);
        return false;
    }

    // store the ids of all of the variables that have been configured
    cJSON* var;
    cJSON_ArrayForEach(var, JSON_flat_vars) {
        unrecognized_variable_ids.insert(var->string);
    }

    // extract field defaults JSON object
    unrecognized_variable_ids.erase("defaults");
    cJSON* JSON_field_defaults = cJSON_GetObjectItem(JSON_flat_vars, "defaults");
    bool is_default_JSON_field_defaults = false;
    if (JSON_field_defaults == NULL) {
        JSON_field_defaults = cJSON_GetObjectItem(JSON_flat_default_config, "defaults");
        if (JSON_field_defaults == NULL) {
            FPS_ERROR_LOG("No \"defaults\" variable found in variables.json configuration or static default configuration.");
            // we cannot try to configure the rest of the variables if we have no field defaults
            // we should never hit this error, since field defaults should have valid default configuration
            return false;
        }
        is_default_JSON_field_defaults = true;
    }

    std::pair<Fims_Object, Config_Validation_Result> JSON_field_defaults_parse_result = parse_field_defaults(JSON_field_defaults);
    Fims_Object field_defaults = JSON_field_defaults_parse_result.first;
    if (!JSON_field_defaults_parse_result.second.is_valid_config) {
        FPS_ERROR_LOG("Failed to parse variable defaults from variables.json: %s", JSON_field_defaults_parse_result.second.brief);
        return false;
    }
    if (is_default_JSON_field_defaults) {
        validation_result.INFO_details.push_back(
            Result_Details(fmt::format("Defaulting missing variable \"defaults\" in variables.json configuration to: value: {}, ui_type: {}.", field_defaults.value.print(),
                                       field_defaults.get_ui_type())));
    }

    // parse input_sources array
    unrecognized_variable_ids.erase("input_sources");
    cJSON* JSON_input_sources = cJSON_GetObjectItem(JSON_flat_vars, "input_sources");
    bool is_default_JSON_input_sources = false;
    if (JSON_input_sources == NULL) {
        JSON_input_sources = cJSON_GetObjectItem(JSON_flat_default_config, "input_sources");
        if (JSON_input_sources == NULL) {
            FPS_ERROR_LOG("No \"input_sources\" variable found in variables.json configuration or static default configuration.");
            // we cannot try to configure the rest of the variables if we don't know what the input sources list should be (even if it should be empty)
            // we should never hit this error, since input sources should have valid default configuration
            return false;
        }
        is_default_JSON_input_sources = true;
    }
    Config_Validation_Result JSON_input_sources_parse_result = input_sources.parse_json_obj(JSON_input_sources);
    if (!JSON_input_sources_parse_result.is_valid_config) {
        FPS_ERROR_LOG("Failed to parse input_sources array: %s", JSON_input_sources_parse_result.brief);
        return false;
    }
    if (is_default_JSON_input_sources) {
        std::string default_input_sources_description = "[";
        for (uint i = 0; i < input_sources.get_num_sources(); i++) {
            default_input_sources_description += input_sources.get_name_of_input(i) + (i < input_sources.get_num_sources() - 1 ? ", " : "");
        }
        default_input_sources_description += "]";
        validation_result.INFO_details.push_back(
            Result_Details(fmt::format("Defaulting missing variable \"input_sources\" in variables.json configuration to: {}.", default_input_sources_description)));
    }

    // go ahead and resize the vector to be exactly what we need
    input_sources_controls.resize(input_sources.input_sources.size());

    // now that we have our input_sources we can configure the associated fims_objects
    size_t i = 0;
    for (const auto &in : input_sources.input_sources) {
        std::string control_ui_type_str;
        switch (in->control_ui_type) {
            case UI_Type::ALARM:
                control_ui_type_str = "alarm"; 
                break;
            case UI_Type::FAULT:
                control_ui_type_str = "fault";
                break;
            case UI_Type::STATUS:
                control_ui_type_str = "status";
                break;
            case UI_Type::CONTROL:
                control_ui_type_str = "control";
                break;
            case UI_Type::NONE:
                control_ui_type_str = "none";
                break;
        }
        try {
            auto buf = fmt::memory_buffer();
            bufJSON_StartObject(buf);                                   // {
            bufJSON_AddString(buf, "name", in->name.c_str());                   // "name": <name>,
            bufJSON_AddString(buf, "type", "enum_slider");                      // "type": "enum_slider",
            bufJSON_AddString(buf, "ui_type", control_ui_type_str.c_str());     // "ui_type": "control",
            bufJSON_AddBool(buf, "value", in->enabled);                         // "value": <enabled>,
            bufJSON_AddString(buf, "var_type", "Bool");                         // "var_type": "Bool",
            bufJSON_AddBool(buf, "multiple_inputs", false);                     // "multiple_inputs": false 
            bufJSON_EndObjectNoComma(buf);                              // }
            input_sources_controls.at(i).set_ui_type(in->ui_type);
            auto *cjson = cJSON_Parse(fmt::to_string(buf).c_str());
            input_sources_controls.at(i).configure(
                    in->uri_suffix, // this will become the variable id
                    is_primary, 
                    &input_sources, // not acutally used multiple_inputs is false
                    cjson,
                    field_defaults, 
                    multi_input_command_vars // not acutally used multiple_inputs is false
                    );
            cJSON_Delete(cjson);
        } catch (std::out_of_range &e) {
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("Attempting to access out of bounds when configuring fimsctls for input_sources.")));
            validation_result.is_valid_config = false;
        }
        i++;
    }

    // parse all feature-independent variables.json variables
    for (auto& variable_id_pair : variable_ids) {
        std::string id = variable_id_pair.second;
        unrecognized_variable_ids.erase(id);
        bool is_default_JSON_variable = false;

        cJSON* JSON_variable = cJSON_GetObjectItem(JSON_flat_vars, id.c_str());
        if (JSON_variable == NULL) {
            JSON_variable = cJSON_GetObjectItem(JSON_flat_default_config, id.c_str());
            if (JSON_variable == NULL) {
                validation_result.ERROR_details.push_back(Result_Details(fmt::format("Required variable \"{}\" is missing from variables.json configuration.", id)));
                validation_result.is_valid_config = false;
                continue;
            } else {
                is_default_JSON_variable = true;
            }
        }
        if (!variable_id_pair.first->configure(variable_id_pair.second, is_primary, &input_sources, JSON_variable, field_defaults, multi_input_command_vars)) {
            validation_result.ERROR_details.push_back(Result_Details(fmt::format("Failed to parse variable with ID {}", variable_id_pair.second)));
            validation_result.is_valid_config = false;
        }
        if (is_default_JSON_variable) {
            validation_result.INFO_details.push_back(
                Result_Details(fmt::format("Defaulting missing variable \"{}\" in variables.json configuration to: value: {}, ui_type: {}.", id,
                                           variable_id_pair.first->value.print(), variable_id_pair.first->get_ui_type())));
        }
    }

    bool has_valid_available_features_config = true;
    // determine which features are available
    // read which active power features are available for runmode1 and configure them on the UI
    if (!configure_available_runmode1_kW_features_list()) {
        validation_result.ERROR_details.push_back(Result_Details("Error configuring available runmode1 active power features!"));
        validation_result.is_valid_config = false;
        has_valid_available_features_config = false;
    }
    // read which active power features are available for runmode2 and configure them on the UI
    if (!configure_available_runmode2_kW_features_list()) {
        validation_result.ERROR_details.push_back(Result_Details("Error configuring available runmode2 active power features!"));
        validation_result.is_valid_config = false;
        has_valid_available_features_config = false;
    }
    // read which reactive power features are available for runmode1 and configure them on the UI
    if (!configure_available_runmode1_kVAR_features_list()) {
        validation_result.ERROR_details.push_back(Result_Details("Error configuring available runmode1 reactive power features!"));
        validation_result.is_valid_config = false;
        has_valid_available_features_config = false;
    }
    // read which reactive power features are available for runmode2 and configure them on the UI
    if (!configure_available_runmode2_kVAR_features_list()) {
        validation_result.ERROR_details.push_back(Result_Details("Error configuring available runmode2 reactive power features!"));
        validation_result.is_valid_config = false;
        has_valid_available_features_config = false;
    }
    // read which standalone power features are available and configure them on the UI
    if (!configure_available_standalone_power_features_list()) {
        validation_result.ERROR_details.push_back(Result_Details("Error configuring available standalone power features!"));
        validation_result.is_valid_config = false;
        has_valid_available_features_config = false;
    }
    // read which site operation features are available and configure them on the UI
    if (!configure_available_site_operation_features_list()) {
        validation_result.ERROR_details.push_back(Result_Details("Error configuring available site operation features!"));
        validation_result.is_valid_config = false;
        has_valid_available_features_config = false;
    }
    // if the watchdog is available above then try to parse it's configuration
    if (watchdog_feature.available) {
        watchdog_feature.parse_json_config(JSON_flat_vars, is_primary, &input_sources, field_defaults, multi_input_command_vars);
    }
    // charge dispatch should always be available
    charge_dispatch.available = true;

    // parse feature-owned variables for all available features
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
            // we still mark recognized variables for features that are not available
            for (auto id : feature->get_variable_ids_list()) {
                unrecognized_variable_ids.erase(id);
            }

            if (!has_valid_available_features_config) {
                continue;
            }
            Config_Validation_Result feature_validation_result = feature->parse_json_config(JSON_flat_vars, is_primary, &input_sources, field_defaults,
                                                                                            multi_input_command_vars);
            if (!feature->available) {
                // if a feature is not available AND enabled it is an error.
                if (feature->enable_flag.value.value_bool == true) {
                    validation_result.ERROR_details.push_back(
                        Result_Details(fmt::format("Enable flag \"{}\" was set as true but the feature is not available.", feature->enable_flag.get_name())));
                    validation_result.is_valid_config = false;
                }
                continue;
            }
            validation_result.absorb(feature_validation_result);
        }
    }

    cJSON_Delete(JSON_flat_vars);
    cJSON_Delete(JSON_flat_default_config);

    // add a warning message for all variables that weren't recognized
    if (!unrecognized_variable_ids.empty()) {
        for (auto unrecognized_var : unrecognized_variable_ids) {
            validation_result.WARNING_details.push_back(
                Result_Details(fmt::format("Encountered unrecognized variable \"{}\" in variables.json configuration.", unrecognized_var)));
        }
    }

    // report the per-variable results of configuration parsing if there weren't more pressing errors that resulted in an early return
    validation_result.log_details();

    if (!validation_result.is_valid_config) {
        return false;
    }

    return true;
}

/**
 * @brief Parses the organized variables in variables.json into a flattened JSON object with all variables at the same level
 * (there are specific hardcoded variables which require subobjects and won't be flattened completely).
 *
 * @param JSON_object The unflattened JSON variables object
 * @return A pair of <flattened_object, validation_result> where validation_result describes whether or not the config was valid
 */
std::pair<cJSON*, Config_Validation_Result> Site_Manager::parse_flatten_vars(cJSON* JSON_object) {
    cJSON* JSON_flat_vars = cJSON_CreateObject();

    // recursively search through the json and add all variables to JSON_flat_vars
    std::function<Config_Validation_Result(cJSON*, cJSON*)> flatten_vars_recursive_lambda;
    flatten_vars_recursive_lambda = [&flatten_vars_recursive_lambda](cJSON* JSON_object, cJSON* JSON_flat_vars) -> Config_Validation_Result {
        cJSON* var_candidate;
        cJSON_ArrayForEach(var_candidate, JSON_object) {
            // input_sources is an array of special config data, as opposed to all other variables which follow the same exact format
            // handle input_sources separately
            if (var_candidate == NULL || var_candidate->string == NULL) {
                return Config_Validation_Result(false, "Unlabeled object found in variables.json");
            } else if (strcmp(var_candidate->string, "frequency_response") == 0) {
                if (!cJSON_HasObjectItem(JSON_flat_vars, var_candidate->string)) {
                    cJSON_AddItemReferenceToObject(JSON_flat_vars, var_candidate->string, var_candidate);
                } else {
                    return Config_Validation_Result(false, "Duplicate variable " + std::string(var_candidate->string) + " found in variables.json.");
                }
            } else if (strcmp(var_candidate->string, "input_sources") == 0) {
                if (!cJSON_HasObjectItem(JSON_flat_vars, var_candidate->string)) {
                    cJSON_AddItemReferenceToObject(JSON_flat_vars, var_candidate->string, var_candidate);
                } else {
                    return Config_Validation_Result(false, "Duplicate variable " + std::string(var_candidate->string) + " found in variables.json.");
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
                        return Config_Validation_Result(false, "Duplicate variable " + std::string(var_candidate->string) + " found in variables.json.");
                }
                // If not a variable, it's a category and we recurse
                else {
                    Config_Validation_Result recursive_result = flatten_vars_recursive_lambda(var_candidate, JSON_flat_vars);
                    // propagate up any error
                    if (!recursive_result.is_valid_config) {
                        return recursive_result;
                    }
                }
            }
        }
        return Config_Validation_Result(true);
    };
    Config_Validation_Result validation_result = flatten_vars_recursive_lambda(JSON_object, JSON_flat_vars);

    return std::make_pair(JSON_flat_vars, validation_result);
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
        bool should_writeout_setpoint = false;
        // Message to writeout for setpoint if valid
        cJSON* body_JSON = cJSON_Parse(msg->body);
        if (body_JSON == NULL) {
            FPS_ERROR_LOG("fims message body is NULL or incorrectly formatted: (%s) \n", msg->body);
            return;
        }

        // Parse the value from the message
        cJSON* body_value = cJSON_GetObjectItem(body_JSON, "value");
        cJSON* p_msg_value = body_value ? body_value : body_JSON;
        if (p_msg_value == NULL) {
            FPS_ERROR_LOG("Failed to parse a value from fims message body: (%s) \n", msg->body);
            return;
        }
        cJSON msg_value = *p_msg_value;

        int body_type = msg_value.type;
        float body_float = msg_value.valuedouble;  // float type for body value
        int body_int = msg_value.valueint;
        bool body_bool = (body_type == cJSON_False) ? false : true;

        if (msg->nfrags == 3)  // This is where you would allow user to set parameters
        {
            if (uType == features_uri)  // all /features sets here
            {
                if (strncmp(msg->pfrags[1], "active_power", strlen("active_power")) == 0) {
                    runmode1_kW_mode_cmd.set_fims_masked_int(msg->pfrags[2], body_int, available_runmode1_kW_features_mask);
                    runmode2_kW_mode_cmd.set_fims_masked_int(msg->pfrags[2], body_int, available_runmode2_kW_features_mask);

                    for (auto feature : runmode1_kW_features_list) {
                        feature->handle_fims_set(msg->pfrags[2], msg_value);
                    }
                    for (auto feature : runmode2_kW_features_list) {
                        feature->handle_fims_set(msg->pfrags[2], msg_value);
                    }
                    // charge control feature variables appear under the active power uri
                    for (auto feature : charge_features_list) {
                        feature->handle_fims_set(msg->pfrags[2], msg_value);
                    }

                    asset_priority_runmode1.set_fims_int(msg->pfrags[2], range_check(body_int, NUM_ASSET_PRIORITIES, 0));
                    asset_priority_runmode2.set_fims_int(msg->pfrags[2], range_check(body_int, NUM_ASSET_PRIORITIES, 0));
                    site_kW_load_interval_ms.set_fims_int(msg->pfrags[2], range_check(body_int, 1, 60000));  // Interval between 1ms and 60s

                    should_writeout_setpoint = true;
                } else if (strncmp(msg->pfrags[1], "reactive_power", strlen("reactive_power")) == 0) {
                    runmode1_kVAR_mode_cmd.set_fims_masked_int(msg->pfrags[2], msg_value.valueint, available_runmode1_kVAR_features_mask);

                    for (auto feature : runmode1_kVAR_features_list) {
                        feature->handle_fims_set(msg->pfrags[2], msg_value);
                    }
                    for (auto feature : runmode2_kVAR_features_list) {
                        feature->handle_fims_set(msg->pfrags[2], msg_value);
                    }

                    should_writeout_setpoint = true;
                } else if (strncmp(msg->pfrags[1], "standalone_power", strlen("standalone_power")) == 0) {
                    for (auto feature : standalone_power_features_list) {
                        feature->handle_fims_set(msg->pfrags[2], msg_value);
                    }

                    // start_first_gen_soc is included as a standalone_power variable due to LDSS
                    if (msg_value.type == cJSON_Number) {
                        start_first_gen_soc.set_fims_float(msg->pfrags[2], range_check(msg_value.valuedouble, 100.0f, 0.0f));
                    }

                    should_writeout_setpoint = true;
                } else if (strncmp(msg->pfrags[1], "site_operation", strlen("site_operation")) == 0) {
                    for (auto feature : site_operation_features_list) {
                        feature->handle_fims_set(msg->pfrags[2], msg_value);
                    }

                    if (msg_value.type == cJSON_False || msg_value.type == cJSON_True) {
                        power_priority_flag.set_fims_bool(msg->pfrags[2], body_bool);
                        allow_ess_auto_restart.set_fims_bool(msg->pfrags[2], body_bool);
                        allow_solar_auto_restart.set_fims_bool(msg->pfrags[2], body_bool);
                        allow_gen_auto_restart.set_fims_bool(msg->pfrags[2], body_bool);

                    }

                    should_writeout_setpoint = true;
                    if (msg_value.type == cJSON_Number) {
                        // watchdog_pet only invalid cops writeout endpoint, all others true
                        should_writeout_setpoint = (strcmp(msg->pfrags[2], "watchdog_pet") != 0);
                    }
                }
            } else if (body_type == cJSON_Number)  // process all numeric sets here
            {
                if (uType == site_uri)  // all numeric sets for /site
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
                        if (ess_kVAR_slew_rate.set_fims_int(msg->pfrags[2], range_check(body_int, 100000000, 1))) {
                            ess_kVAR_cmd_slew.set_slew_rate(ess_kVAR_slew_rate.value.value_int);
                        }
                        if (gen_kVAR_slew_rate.set_fims_int(msg->pfrags[2], range_check(body_int, 100000000, 1))) {
                            gen_kVAR_cmd_slew.set_slew_rate(gen_kVAR_slew_rate.value.value_int);
                        }
                        if (solar_kVAR_slew_rate.set_fims_int(msg->pfrags[2], range_check(body_int, 100000000, 1))) {
                            solar_kVAR_cmd_slew.set_slew_rate(solar_kVAR_slew_rate.value.value_int);
                        }
                        should_writeout_setpoint = true;
                    } else if (strcmp(msg->pfrags[1], "debug") == 0) {
                        if ((strcmp(msg->pfrags[2], "state") == 0) && (static_cast<states>(body_int) != sequences_status.current_state)) {
                            set_state(static_cast<states>(body_int));
                            should_writeout_setpoint = true;
                        }
                    }
                }
            } else if (body_type == cJSON_False || body_type == cJSON_True)  // process all boolean sets here
            {
                if (uType == site_uri)  // all /site boolean set here
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
                                should_writeout_setpoint = true;
                        }
                    } 
                    else if (strncmp(msg->pfrags[1], "input_sources", strlen("input_sources")) == 0) {
                        std::string new_selected_input;
                        new_selected_input = input_sources.set_source_enable_flag(msg->pfrags[2], body_bool);
                        // sychronize the control bools
                        for (size_t i = 0; i < input_sources.input_sources.size(); i++) {
                            input_sources_controls.at(i).value.set(input_sources.input_sources.at(i)->enabled);
                            FPS_DEBUG_LOG("input_sources_controls: %s: %d -- input_sources: %s: %d", input_sources_controls.at(i).get_name(), input_sources_controls.at(i).value.value_bool,
                                    input_sources.input_sources.at(i)->name, input_sources.input_sources.at(i)->enabled);
                        }
                        input_source_status.value.set(new_selected_input);
                        should_writeout_setpoint = true;
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
                        should_writeout_setpoint = true;
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
        } else {
            FPS_WARNING_LOG("site manager set message error, unexpected number of fragments %d instead of 3.", msg->nfrags);
            if (msg->replyto != NULL)
                p_fims->Send("set", msg->replyto, NULL, "Invalid URI");
        }

        // Write out if the set matched one of the above categories
        if (should_writeout_setpoint) {
            std::string uri = "/" + std::string(msg->pfrags[0]);
            // All site/feature settings that are valid are persistent currently
            data_endpoint->setpoint_writeout(uri, msg->pfrags[2], &body_JSON);
        }

        if (msg->replyto != NULL)
            pFims->Send("set", msg->replyto, NULL, msg->body);

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

void Site_Manager::send_FIMS(const char* method, const char* uri, const char* replyto, const char* body_retrieval_uri) {
    // Clear buffer for use
    send_FIMS_buf.clear();

    // Disable publish if second controller (shadow mode)
    // Only allow the heartbeat response to be sent to COPS
    if (!*is_primary && strcmp(uri, "/cops/heartbeat/site_controller") != 0 && strcmp(uri, "/site/configuration/configured_primary") != 0)
        return;

    // borrowed from libfims, get URI segments
    const char** pfrags = NULL;
    unsigned int nfrags = 0;
    int count = 0;
    int offset[MAX_URI_DEPTH];
    for (int i = 0; body_retrieval_uri[i] != '\0' && count < MAX_URI_DEPTH; i++) {
        if (body_retrieval_uri[i] == '/') {
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
        pfrags[i] = body_retrieval_uri + (offset[i] + 1);

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
            else if (strncmp(pfrags[1], "input_sources", strlen("input_sources")) == 0) {
                build_JSON_site_input_sources(send_FIMS_buf, var);
            }
        }
    }
    bufJSON_RemoveTrailingComma(send_FIMS_buf);
    std::string string_body = to_string(send_FIMS_buf);

    delete[] pfrags;

    if (string_body.empty())  // no match, print error and return
    {
        FPS_ERROR_LOG("Site Manager FIMS message error - URI not matched %s\n", body_retrieval_uri);
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
    clock_gettime(CLOCK_MONOTONIC, &sequences_status.current_time);

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

    reactive_setpoint.kVAR_cmd_slew.set_slew_rate(reactive_setpoint.kVAR_slew_rate.value.value_int);

    // Update LDSS values
    set_ldss_variables();

    // update load/solar shedder values
    load_shed.load_shed_spare_ess_kw.value.set(pAssets->get_ess_total_dischargeable_power_kW() - pAssets->get_ess_total_active_power());
    solar_shed.solar_shed_spare_ess_kw.value.set(pAssets->get_ess_total_chargeable_power_kW() - zero_check(-1 * pAssets->get_ess_total_active_power()));

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

    // automatic voltage variables set
    avr.actual_volts.value.set(pAssets->get_poi_gridside_avg_voltage());
    avr.status_flag.value.set(false);

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
    reactive_power_closed_loop.total_correction.value.set(0.0f);

    // KPI values
    update_ess_kpi_values();

    get_ess_calibration_variables();

    // bidirectional function call (get charge_control_kW_request and send target_soc to algorithm)
    charge_control.kW_request.value.set(pAssets->charge_control(charge_control.target_soc.value.value_float, charge_control.charge_disable.value.value_bool,
                                                                charge_control.discharge_disable.value.value_bool));
}

// set all interface variables (to asset manager, FIMS)
void Site_Manager::set_values() {
    FPS_DEBUG_LOG("Setting Asset Manager Values; Site_Manager::set_values.\n");

    // if Clear Faults button was pressed a second ago, clear the flags now. Delayed to give Asset Manager time to clear component faults
    if (clear_fault_status_flags && check_expired_time(sequences_status.current_time, time_to_clear_fault_status_flags)) {
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
    remove_reactive_poi_corrections_from_slew_targets();
    manual_power_mode.manual_solar_kW_slew.update_slew_target(solar_kW_cmd.value.value_float);
    manual_power_mode.manual_ess_kW_slew.update_slew_target(ess_kW_cmd.value.value_float);
    manual_power_mode.manual_gen_kW_slew.update_slew_target(gen_kW_cmd.value.value_float);

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
    if (power_factor.prev_asset_pf_cmd != power_factor.power_factor_cmd.value.value_float) {
        pAssets->set_ess_target_power_factor(power_factor.power_factor_cmd.value.value_float);
        pAssets->set_solar_target_power_factor(power_factor.power_factor_cmd.value.value_float);
        power_factor.prev_asset_pf_cmd = power_factor.power_factor_cmd.value.value_float;
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
    load_shed.load_shed_increase_display_timer.value.set(load_shed.load_shed_calculator.get_increase_display_timer());
    load_shed.load_shed_decrease_display_timer.value.set(load_shed.load_shed_calculator.get_decrease_display_timer());
    solar_shed.solar_shed_increase_display_timer.value.set(solar_shed.solar_shed_calculator.get_increase_display_timer());
    solar_shed.solar_shed_decrease_display_timer.value.set(solar_shed.solar_shed_calculator.get_decrease_display_timer());

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
        slew_target -= reactive_power_closed_loop.total_correction.value.value_float;
    // Reset feature slews
    reactive_setpoint.kVAR_cmd_slew.update_slew_target(slew_target);
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
        enable_flag.ui_enabled = (sequences_status.current_state == Ready);
        // Manually set the enable flag false for all states except Init
        // This allows a persistent settings set to be received on startup but all other redundant enable sets to be ignored
        // In most cases a persistent settings start should be delayed 10 seconds, but this provides additional tolerance if it's sent too early
        set_enable_flag(enable_flag.value.value_bool && sequences_status.current_state == Init);
    }

    disable_flag.ui_enabled = true;
    runmode1_kW_mode_cmd.ui_enabled = true;
    runmode1_kVAR_mode_cmd.ui_enabled = true;
    runmode2_kW_mode_cmd.ui_enabled = true;
    runmode2_kVAR_mode_cmd.ui_enabled = true;
    active_power_poi_limits.enable_flag.ui_enabled = true;
    watchdog_feature.enable_flag.ui_enabled = true;
    watchdog_feature.watchdog_duration_ms.ui_enabled = watchdog_feature.enable_flag.value.value_bool;
    charge_control.kW_limit.ui_enabled = true;

    // disable clear faults button if no faults or alarms
    clear_faults_flag.ui_enabled = fault_status_flag.value.value_bool || alarm_status_flag.value.value_bool || num_path_faults != 0 || num_path_alarms != 0;

    // charge features - vars enabled if feature is enabled
    for (auto feature : charge_features_list) {
        feature->toggle_ui_enabled(feature->enable_flag.value.value_bool);
    }
    // standalone power features - vars enabled if feature is enabled
    for (auto feature : standalone_power_features_list) {
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
        bool soc_limits_enabled = ess_calibration.enable_flag.value.value_bool && ess_calibration.soc_limits_enable.value.value_bool;
        ess_calibration.min_soc_limit.ui_enabled = soc_limits_enabled;
        ess_calibration.max_soc_limit.ui_enabled = soc_limits_enabled;
        bool cell_voltage_limits_enabled = ess_calibration.enable_flag.value.value_bool && ess_calibration.cell_voltage_limits_enable.value.value_bool;
        ess_calibration.min_cell_voltage_limit.ui_enabled = cell_voltage_limits_enabled;
        ess_calibration.max_cell_voltage_limit.ui_enabled = cell_voltage_limits_enabled;
        bool rack_voltage_limits_enabled = ess_calibration.enable_flag.value.value_bool && ess_calibration.rack_voltage_limits_enable.value.value_bool;
        ess_calibration.min_rack_voltage_limit.ui_enabled = rack_voltage_limits_enabled;
        ess_calibration.max_rack_voltage_limit.ui_enabled = rack_voltage_limits_enabled;
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
    runmode1_kW_mode_status.value.set(available_features_runmode1_kW_mode.options_map[runmode1_kW_mode_cmd.value.value_int].first);
    runmode2_kW_mode_status.value.set(available_features_runmode2_kW_mode.options_map[runmode2_kW_mode_cmd.value.value_int].first);
    runmode1_kVAR_mode_status.value.set(available_features_runmode1_kVAR_mode.options_map[runmode1_kVAR_mode_cmd.value.value_int].first);
    runmode2_kVAR_mode_status.value.set(available_features_runmode2_kVAR_mode.options_map[runmode2_kVAR_mode_cmd.value.value_int].first);
}

// Read which runmode1 kW features are available and configure them on the UI.
// Also configures the availability of Charge Control
bool Site_Manager::configure_available_runmode1_kW_features_list(void) {
    // add only the runmode1 kW features covered by the available_features_runmode1_kW_mode mask
    bool initial_gc_kW_feature_found = false;
    size_t num_features = runmode1_kW_features_list.size();
    if (available_features_runmode1_kW_mode.options_map.size() != num_features + 1)  // +1 because of Default
    {
        FPS_ERROR_LOG("available_features_runmode1_kW_mode does not have all modes listed. Expected %d, got %d.\n", num_features + 1,
                      available_features_runmode1_kW_mode.options_map.size());
        return false;
    }
    try {
        available_runmode1_kW_features_mask = (uint64_t)std::stoul(available_features_runmode1_kW_mode.value.value_string, NULL, 16);
    } catch (...) {
        FPS_ERROR_LOG("Error parsing available_runmode1_kW_features_mask\n");
        return false;
    }
    for (size_t i = 0; i <= num_features; ++i)  // <= so Disabled is included
    {
        if (available_runmode1_kW_features_mask & ((uint64_t)1) << i) {
            // mark feature as available to customer as long as it is not "Disabled" option
            if (i != num_features)
                runmode1_kW_features_list[i]->available = true;

            // if initial runmode1 kW feature is found, mark that it passed the mask
            if ((int)i == runmode1_kW_mode_cmd.value.value_int)
                initial_gc_kW_feature_found = true;

            // add feature to list of available runmode1 kW feature commands
            runmode1_kW_mode_cmd.options_map[i] = std::pair<std::string, Value_Object>(available_features_runmode1_kW_mode.options_map[i].first, i);

            // Set charge control available if utilized by any feature
            if (runmode1_kW_features_charge_control_mask & ((uint64_t)1) << i)
                charge_control.available = true;
        }
    }

    // do not show control on UI if only one option
    if (runmode1_kW_mode_cmd.options_map.size() < 2) {
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
bool Site_Manager::configure_available_runmode2_kW_features_list(void) {
    // add only the runmode2 kW features covered by the available_features_runmode2_kW_mode mask
    bool initial_runmode2_kW_feature_found = false;
    size_t num_features = runmode2_kW_features_list.size();
    if (available_features_runmode2_kW_mode.options_map.size() != num_features + 1)  // +1 because of Default
    {
        FPS_ERROR_LOG("available_features_runmode2_kW_mode does not have all modes listed. Expected %d, got %d.\n", num_features + 1,
                      available_features_runmode2_kW_mode.options_map.size());
        return false;
    }
    try {
        available_runmode2_kW_features_mask = (uint64_t)std::stoul(available_features_runmode2_kW_mode.value.value_string, NULL, 16);
    } catch (...) {
        FPS_ERROR_LOG("Error parsing available_runmode2_kW_features_mask\n");
        return false;
    }
    for (size_t i = 0; i <= num_features; ++i)  // <= so Disabled is included
    {
        if (available_runmode2_kW_features_mask & ((uint64_t)1) << i) {
            // mark feature as available to customer as long as it is not "Disabled" option
            if (i != num_features)
                runmode2_kW_features_list[i]->available = true;

            // if initial runmode2 kW feature is found, mark that it passed the mask
            if ((int)i == runmode2_kW_mode_cmd.value.value_int)
                initial_runmode2_kW_feature_found = true;

            // add feature to list of available runmode2 kW feature commands
            runmode2_kW_mode_cmd.options_map[i] = std::pair<std::string, Value_Object>(available_features_runmode2_kW_mode.options_map[i].first, i);
        }
    }

    // do not show control on UI if only one option
    if (runmode2_kW_mode_cmd.options_map.size() < 2) {
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
bool Site_Manager::configure_available_runmode1_kVAR_features_list(void) {
    // add only the runmode1 kVAR features covered by the available_features_runmode1_kVAR_mode mask
    bool initial_gc_kVAR_feature_found = false;
    size_t num_features = runmode1_kVAR_features_list.size();
    if (available_features_runmode1_kVAR_mode.options_map.size() != num_features + 1)  // +1 because of Default
    {
        FPS_ERROR_LOG("available_features_runmode1_kVAR_mode does not have all modes listed. Expected %d, got %d.\n", num_features + 1,
                      available_features_runmode1_kVAR_mode.options_map.size());
        return false;
    }
    try {
        available_runmode1_kVAR_features_mask = (uint64_t)std::stoul(available_features_runmode1_kVAR_mode.value.value_string, NULL, 16);
    } catch (...) {
        FPS_ERROR_LOG("Error parsing available_runmode1_kVAR_features_mask\n");
        return false;
    }
    for (size_t i = 0; i <= num_features; ++i)  // <= so Disabled is included
    {
        if (available_runmode1_kVAR_features_mask & ((uint64_t)1) << i) {
            // mark feature as available to customer as long as it is not "Disabled" option
            if (i != num_features)
                runmode1_kVAR_features_list[i]->available = true;

            // if initial runmode1 kVAR feature is found, mark that it passed the mask
            if ((int)i == runmode1_kVAR_mode_cmd.value.value_int)
                initial_gc_kVAR_feature_found = true;

            // add feature to list of available runmode1 kVAR feature commands
            runmode1_kVAR_mode_cmd.options_map[i] = std::pair<std::string, Value_Object>(available_features_runmode1_kVAR_mode.options_map[i].first, i);
        }
    }

    // do not show control on UI if only one option
    if (runmode1_kVAR_mode_cmd.options_map.size() < 2) {
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
bool Site_Manager::configure_available_runmode2_kVAR_features_list(void) {
    // add only the runmode2 kVAR features covered by the available_features_runmode2_kVAR_mode mask
    bool initial_gc_kVAR_feature_found = false;
    size_t num_features = runmode2_kVAR_features_list.size();
    if (available_features_runmode2_kVAR_mode.options_map.size() != num_features + 1)  // +1 because of Default
    {
        FPS_ERROR_LOG("available_features_runmode2_kVAR_mode does not have all modes listed. Expected %d, got %d.\n", num_features + 1,
                      available_features_runmode2_kVAR_mode.options_map.size());
        return false;
    }
    try {
        available_runmode2_kVAR_features_mask = (uint64_t)std::stoul(available_features_runmode2_kVAR_mode.value.value_string, NULL, 16);
    } catch (...) {
        FPS_ERROR_LOG("Error parsing available_runmode2_kVAR_features_mask\n");
        return false;
    }
    for (size_t i = 0; i <= num_features; ++i)  // <= so Disabled is included
    {
        if (available_runmode2_kVAR_features_mask & ((uint64_t)1) << i) {
            // mark feature as available to customer as long as it is not "Disabled" option
            if (i != num_features)
                runmode2_kVAR_features_list[i]->available = true;

            // if initial runmode2 kVAR feature is found, mark that it passed the mask
            if ((int)i == runmode2_kVAR_mode_cmd.value.value_int)
                initial_gc_kVAR_feature_found = true;

            // add feature to list of available runmode2 kVAR feature commands
            runmode2_kVAR_mode_cmd.options_map[i] = std::pair<std::string, Value_Object>(available_features_runmode2_kVAR_mode.options_map[i].first, i);
        }
    }

    // do not show control on UI if only one option
    if (runmode2_kVAR_mode_cmd.options_map.size() < 2) {
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
bool Site_Manager::configure_available_standalone_power_features_list(void) {
    size_t num_features = standalone_power_features_list.size();
    if (available_features_standalone_power.options_map.size() != num_features) {
        FPS_ERROR_LOG("available_features_standalone_power does not have all modes listed. Expected %d, got %d.\n", num_features,
                      available_features_standalone_power.options_map.size());
        return false;
    }
    // add only the standalone power features covered by the available_features_standalone_power mask
    try {
        available_standalone_power_features_mask = (uint64_t)std::stoul(available_features_standalone_power.value.value_string, NULL, 16);
    } catch (...) {
        FPS_ERROR_LOG("Error parsing available_standalone_power_features_mask\n");
        return false;
    }
    for (size_t i = 0; i < standalone_power_features_list.size(); ++i) {
        if (available_standalone_power_features_mask & ((uint64_t)1) << i) {
            // mark feature as available to customer
            standalone_power_features_list[i]->available = true;
        }
    }

    return true;
}

/**
 * Read which site operation features are available and configure them on the UI.
 * @return True if configuration was successful, false if not.
 */
bool Site_Manager::configure_available_site_operation_features_list(void) {
    size_t num_features = site_operation_features_list.size();
    if (available_features_site_operation.options_map.size() != num_features) {
        FPS_ERROR_LOG("available_features_site_operation does not have all modes listed. Expected %d, got %d.\n", num_features,
                      available_features_site_operation.options_map.size());
        return false;
    }
    // add only the site operation features covered by the available_features_site_operation mask
    try {
        available_site_operation_features_mask = (uint64_t)std::stoul(available_features_site_operation.value.value_string, NULL, 16);
    } catch (...) {
        FPS_ERROR_LOG("Error parsing available_site_operation_features_mask\n");
        return false;
    }
    for (size_t i = 0; i < site_operation_features_list.size(); ++i) {
        if (available_site_operation_features_mask & ((uint64_t)1) << i) {
            // mark feature as available to customer
            site_operation_features_list[i]->available = true;
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

/**
 * Track the fault with the given fault_number
 * @param fault_number Index of the fault message within the faults options array
 */
void Site_Manager::set_faults(int fault_number) {
    char event_message[SHORT_MSG_LEN];

    // Make sure the fault number is in range. The range of available options will be determined by the configuration received for faults
    // TODO: make faults default to filling all 32 possible options values
    std::string fault_message = "internal fault tracking error";
    auto fault_it = faults.options_map.find(fault_number);
    if (fault_it != faults.options_map.end()) {
        fault_message = fault_it->second.first;
    }

    active_fault_array[fault_number] = true;
    snprintf(event_message, SHORT_MSG_LEN, "Fault: %s", fault_message.c_str());
    FPS_ERROR_LOG("%s", event_message);

    emit_event("Site", event_message, FAULT_ALERT);
    fault_status_flag.value.value_bool = true;
}

/**
 * Track the alarm with the given alarm_number
 * @param alarm_number Index of the alarm message within the alarms options array
 */
void Site_Manager::set_alarms(int alarm_number) {
    char event_message[SHORT_MSG_LEN];

    // Make sure the alarm number is in range. The range of available options will be determined by the configuration received for alarms
    // TODO: make alarms default to filling all 32 possible options values
    std::string alarm_message = "internal alarm tracking error";
    auto alarm_it = alarms.options_map.find(alarm_number);
    if (alarm_it != alarms.options_map.end()) {
        alarm_message = alarm_it->second.first;
    }

    active_alarm_array[alarm_number] = true;
    snprintf(event_message, SHORT_MSG_LEN, "Alarm: %s", alarm_message.c_str());
    FPS_ERROR_LOG("%s", event_message);

    emit_event("Site", event_message, ALARM_ALERT);
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
    for (size_t i = 0; i < faults.options_map.size(); i++) {
        if (active_fault_array[i]) {
            active_faults.options_map[i] = faults.options_map[i];
            active_faults.value.value_bit_field |= uint64_t(1) << i;
            count++;
        }
    }
    active_faults.value.set(count);
}

void Site_Manager::build_active_alarms() {
    int count = 0;
    active_alarms.value.value_bit_field = 0;
    for (size_t i = 0; i < alarms.options_map.size(); i++) {
        if (active_alarm_array[i]) {
            active_alarms.options_map[i] = alarms.options_map[i];
            active_alarms.value.value_bit_field |= uint64_t(1) << i;
            count++;
        }
    }
    active_alarms.value.set(count);
}

bool Site_Manager::set_state(states state_request) {
    FPS_INFO_LOG("Site Manager function 'set_state' called \n");
    if (state_request != sequences_status.current_state)
        sequences_status.current_state = state_request;
    return true;
}

void Site_Manager::set_site_status(const char* message) {
    site_status.value.set(std::string(message));
}

void Site_Manager::handle_reserved_variable_sequence_functions(const char* cmd, Value_Object* value, int tolerance_percent, bool& command_found, bool& return_value) {
    if (strcmp(cmd, "bool_1") == 0) {
        return_value = (reserved_bool_1.value.value_bool == value->value_bool);
    } else if (strcmp(cmd, "bool_2") == 0) {
        return_value = (reserved_bool_2.value.value_bool == value->value_bool);
    } else if (strcmp(cmd, "bool_3") == 0) {
        return_value = (reserved_bool_3.value.value_bool == value->value_bool);
    } else if (strcmp(cmd, "bool_4") == 0) {
        return_value = (reserved_bool_4.value.value_bool == value->value_bool);
    } else if (strcmp(cmd, "bool_5") == 0) {
        return_value = (reserved_bool_5.value.value_bool == value->value_bool);
    } else if (strcmp(cmd, "bool_6") == 0) {
        return_value = (reserved_bool_6.value.value_bool == value->value_bool);
    } else if (strcmp(cmd, "bool_7") == 0) {
        return_value = (reserved_bool_7.value.value_bool == value->value_bool);
    } else if (strcmp(cmd, "bool_8") == 0) {
        return_value = (reserved_bool_8.value.value_bool == value->value_bool);
    } else if (strcmp(cmd, "bool_9") == 0) {
        return_value = (reserved_bool_9.value.value_bool == value->value_bool);
    } else if (strcmp(cmd, "bool_10") == 0) {
        return_value = (reserved_bool_10.value.value_bool == value->value_bool);
    } else if (strcmp(cmd, "bool_11") == 0) {
        return_value = (reserved_bool_11.value.value_bool == value->value_bool);
    } else if (strcmp(cmd, "bool_12") == 0) {
        return_value = (reserved_bool_12.value.value_bool == value->value_bool);
    } else if (strcmp(cmd, "bool_13") == 0) {
        return_value = (reserved_bool_13.value.value_bool == value->value_bool);
    } else if (strcmp(cmd, "bool_14") == 0) {
        return_value = (reserved_bool_14.value.value_bool == value->value_bool);
    } else if (strcmp(cmd, "bool_15") == 0) {
        return_value = (reserved_bool_15.value.value_bool == value->value_bool);
    } else if (strcmp(cmd, "bool_16") == 0) {
        return_value = (reserved_bool_16.value.value_bool == value->value_bool);
    } else if (strcmp(cmd, "float_1") == 0) {
        return_value = ((value->type == Float) && get_tolerance(value->value_float, reserved_float_1.value.value_float, tolerance_percent));
    } else if (strcmp(cmd, "float_2") == 0) {
        return_value = ((value->type == Float) && get_tolerance(value->value_float, reserved_float_2.value.value_float, tolerance_percent));
    } else if (strcmp(cmd, "float_3") == 0) {
        return_value = ((value->type == Float) && get_tolerance(value->value_float, reserved_float_3.value.value_float, tolerance_percent));
    } else if (strcmp(cmd, "float_4") == 0) {
        return_value = ((value->type == Float) && get_tolerance(value->value_float, reserved_float_4.value.value_float, tolerance_percent));
    } else if (strcmp(cmd, "float_5") == 0) {
        return_value = ((value->type == Float) && get_tolerance(value->value_float, reserved_float_5.value.value_float, tolerance_percent));
    } else if (strcmp(cmd, "float_6") == 0) {
        return_value = ((value->type == Float) && get_tolerance(value->value_float, reserved_float_6.value.value_float, tolerance_percent));
    } else if (strcmp(cmd, "float_7") == 0) {
        return_value = ((value->type == Float) && get_tolerance(value->value_float, reserved_float_7.value.value_float, tolerance_percent));
    } else if (strcmp(cmd, "float_8") == 0) {
        return_value = ((value->type == Float) && get_tolerance(value->value_float, reserved_float_8.value.value_float, tolerance_percent));
    } else {
        command_found = false;
    }
}
void Site_Manager::handle_ess_sequence_functions(const char* cmd, Value_Object* value, int tolerance_percent, bool& command_found, bool& return_value) {
    if (strcmp(cmd, "get_num_ess_available") == 0) {
        return_value = ((value->type == Float) && get_tolerance(value->value_float, num_ess_available, tolerance_percent));
    } else if (strcmp(cmd, "get_num_ess_running") == 0) {
        return_value = ((value->type == Float) && get_tolerance(value->value_float, num_ess_running, tolerance_percent));
    } else if (strcmp(cmd, "get_num_ess_controllable") == 0) {
        return_value = ((value->type == Float) && get_tolerance(value->value_float, num_ess_controllable, tolerance_percent));
    } else if (strcmp(cmd, "get_ess_active_power") == 0) {
        return_value = ((value->type == Float) && get_tolerance(value->value_float, ess_actual_kW.value.value_float, tolerance_percent));
    } else if (strcmp(cmd, "start_all_ess") == 0) {
        return_value = pAssets->start_all_ess();
    } else if (strcmp(cmd, "stop_all_ess") == 0) {
        return_value = pAssets->stop_all_ess();
    } else if (strcmp(cmd, "set_all_ess_grid_form") == 0) {
        pAssets->set_all_ess_grid_form();
    } else if (strcmp(cmd, "set_all_ess_grid_follow") == 0) {
        pAssets->set_all_ess_grid_follow();
    } else if (strcmp(cmd, "set_voltage_slope") == 0) {
        pAssets->set_grid_forming_voltage_slew(value->value_float);
    } else if (strcmp(cmd, "open_contactors") == 0) {
        pAssets->open_all_bms_contactors();
    } else if (strcmp(cmd, "close_contactors") == 0) {
        pAssets->close_all_bms_contactors();
    } else if (strcmp(cmd, "synchronize_ess") == 0) {
        return_value = synchronize_ess();
    } else if (strcmp(cmd, "has_faults") == 0) {
        return_value = ((pAssets->get_num_active_faults(ESS) > 0) == value->value_bool);
    } else if (strcmp(cmd, "allow_auto_restart") == 0) {
        allow_ess_auto_restart.value.set(value->value_bool);
    } else if (strcmp(cmd, "controllable_soc_above") == 0) {
        return_value = ((value->type == Float) && soc_avg_running.value.value_float > value->value_float);
    } else if (strcmp(cmd, "controllable_soc_below") == 0) {
        return_value = ((value->type == Float) && soc_avg_running.value.value_float < value->value_float);
    } else if (strcmp(cmd, "all_soc_above") == 0) {
        return_value = ((value->type == Float) && soc_avg_all.value.value_float > value->value_float);
    } else if (strcmp(cmd, "all_soc_below") == 0) {
        return_value = ((value->type == Float) && soc_avg_all.value.value_float < value->value_float);
    } else {
        command_found = false;
    }
}

void Site_Manager::handle_gen_sequence_functions(const char* cmd, Value_Object* value, int tolerance_percent, bool& command_found, bool& return_value) {
    if (strcmp(cmd, "get_num_gen_available") == 0) {
        return_value = ((value->type == Float) && get_tolerance(value->value_float, num_gen_available, tolerance_percent));
    } else if (strcmp(cmd, "get_num_gen_running") == 0) {
        return_value = ((value->type == Float) && get_tolerance(value->value_float, num_gen_running, tolerance_percent));
    } else if (strcmp(cmd, "get_num_gen_active") == 0) {
        return_value = ((value->type == Float) && get_tolerance(value->value_float, num_gen_active, tolerance_percent));
    } else if (strcmp(cmd, "min_generators_active") == 0) {
        pAssets->set_min_generators_active(value->value_float);
    } else if (strcmp(cmd, "direct_start_gen") == 0) {
        return_value = pAssets->direct_start_gen();
    } else if (strcmp(cmd, "start_all_gen") == 0) {
        return_value = true;
        pAssets->start_all_gen();
    } else if (strcmp(cmd, "stop_all_gen") == 0) {
        return_value = pAssets->stop_all_gen();
    } else if (strcmp(cmd, "set_all_gen_grid_follow") == 0) {
        pAssets->set_all_gen_grid_follow();
    } else if (strcmp(cmd, "set_all_gen_grid_form") == 0) {
        pAssets->set_all_gen_grid_form();
    } else if (strcmp(cmd, "has_faults") == 0) {
        return_value = ((pAssets->get_num_active_faults(GENERATORS) > 0) == value->value_bool);
    } else if (strcmp(cmd, "allow_auto_restart") == 0) {
        allow_gen_auto_restart.value.set(value->value_bool);  // set the config bool for set_values
    } else {
        command_found = false;
    }
}

void Site_Manager::handle_solar_sequence_functions(const char* cmd, Value_Object* value, int tolerance_percent, bool& command_found, bool& return_value) {
    if (strcmp(cmd, "get_num_solar_available") == 0) {
        return_value = ((value->type == Float) && get_tolerance(value->value_float, num_solar_available, tolerance_percent));
    } else if (strcmp(cmd, "get_num_solar_running") == 0) {
        return_value = ((value->type == Float) && get_tolerance(value->value_float, num_solar_running, tolerance_percent));
    } else if (strcmp(cmd, "start_all_solar") == 0) {
        return_value = pAssets->start_all_solar();
    } else if (strcmp(cmd, "stop_all_solar") == 0) {
        return_value = pAssets->stop_all_solar();
    } else if (strcmp(cmd, "has_faults") == 0) {
        return_value = ((pAssets->get_num_active_faults(SOLAR) > 0) == value->value_bool);
    } else if (strcmp(cmd, "allow_auto_restart") == 0) {
        allow_solar_auto_restart.value.set(value->value_bool);
    } else {
        command_found = false;
    }
}

void Site_Manager::handle_feed_sequence_functions(const char* cmd, Value_Object* value, bool& command_found, bool& return_value) {
    // sync breaker state
    if (strcmp(cmd, "get_sync_feeder_state") == 0) {
        return_value = pAssets->get_sync_feeder_status() ? (value->type == Bool) && (value->value_bool) : (value->type == Bool) && (!value->value_bool);
        // get poi breaker state
    } else if (strcmp(cmd, "get_poi_feeder_state") == 0) {
        bool poi_state = pAssets->get_poi_feeder_state();  // DEBUG
        return_value = (value->type == Bool) && (value->value_bool == poi_state);
    }
    // set poi breaker state open
    else if (strcmp(cmd, "set_poi_feeder_state_open") == 0) {
        return_value = pAssets->set_poi_feeder_state_open();
        // set poi breaker state closed
    } else if (strcmp(cmd, "set_poi_feeder_state_closed") == 0) {
        return_value = pAssets->set_poi_feeder_state_closed();
    } else if (strcmp(cmd, "set_sync_feeder_close_permissive_remove") == 0) {
        return_value = pAssets->set_sync_feeder_close_permissive_remove();
    } else if (strcmp(cmd, "set_sync_feeder_close_permissive") == 0) {
        return_value = pAssets->set_sync_feeder_close_permissive();
    } else {
        command_found = false;
    }
}

void Site_Manager::handle_config_sequence_functions(const char* cmd, Value_Object* value, bool& command_found, bool& return_value) {
    if (strcmp(cmd, "clear_faults") == 0) {
        clear_faults();
    } else if (strcmp(cmd, "get_standby_flag") == 0) {
        return_value = standby_flag.value.value_bool == value->value_bool;  // get standby flag status
    } else {
        command_found = false;
    }
}

void Site_Manager::handle_feat_sequence_functions(const char* cmd, Value_Object* value, bool& command_found) {
    if (strcmp(cmd, "reset_load_shed") == 0) {
        int configured_shed_value = static_cast<int>(value->value_float);
        if (configured_shed_value < load_shed.load_shed_min_value.value.value_int || configured_shed_value > load_shed.load_shed_max_value.value.value_int) {
            FPS_ERROR_LOG("Invalid load shed value %d provided, must be between %d and %d\n", configured_shed_value, load_shed.load_shed_min_value.value.value_int,
                          load_shed.load_shed_max_value.value.value_int);
            command_found = false;
        } else {
            load_shed.load_shed_value.value.set(configured_shed_value);
            load_shed.load_shed_calculator.offset = configured_shed_value;
        }
    } else if (strcmp(cmd, "enable_ldss") == 0) {
        ldss.enable_flag.value.set(value->value_bool);
    } else {
        command_found = false;
    }
}

void Site_Manager::handle_specific_feed_sequence_functions(const char* cmd, Value_Object* value, const char* target_asset, bool& command_found, bool& return_value) {
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
        } else {
            command_found = false;
        }
    }
}

bool Site_Manager::call_sequence_functions(const char* target_asset, const char* cmd, Value_Object* value, int tolerance_percent) {
    bool command_found = true;
    bool return_value = true;

    // this cmd will pass and move to next step
    if (strcmp(target_asset, "bypass") == 0) {
        return true;
        // this cmd will move to next path (requires step to have path switch
    }
    if (strcmp(target_asset, "new_path") == 0) {
        return false;
        // all generic boolean and floats for site-specific configuration
    }
    if (strcmp(target_asset, "reserved") == 0) {
        handle_reserved_variable_sequence_functions(cmd, value, tolerance_percent, command_found, return_value);
    } else if (strcmp(target_asset, "ess") == 0) {  // ###### ESS SECTION ######
        handle_ess_sequence_functions(cmd, value, tolerance_percent, command_found, return_value);
    } else if (strcmp(target_asset, "gen") == 0) {  // ###### GENERATOR SECTION ######
        handle_gen_sequence_functions(cmd, value, tolerance_percent, command_found, return_value);
    } else if (strcmp(target_asset, "solar") == 0) {  // ###### SOLAR SECTION ######
        handle_solar_sequence_functions(cmd, value, tolerance_percent, command_found, return_value);
    } else if (strcmp(target_asset, "feeder") == 0) {  // ###### FEEDER SECTION ######
        handle_feed_sequence_functions(cmd, value, command_found, return_value);
    } else if (strcmp(target_asset, "config") == 0) {  // ###### state/operation commands here ######
        handle_config_sequence_functions(cmd, value, command_found, return_value);
    } else if (strcmp(target_asset, "features") == 0) {  // ###### feature commands here ######
        handle_feat_sequence_functions(cmd, value, command_found);
    } else {  // check for specific feeder id for setting and getting specific feeder states
        handle_specific_feed_sequence_functions(cmd, value, target_asset, command_found, return_value);
    }

    // no command executed
    if (command_found == false) {
        FPS_ERROR_LOG("call_function command not found. target_asset: %s cmd: %s \n", target_asset, cmd);
        return false;
    }

    return return_value;
}

alt_states Site_Manager::get_alternate_site_state() const {
    switch (site_state_enum.value.value_int) {  
        case states::Init:
            return alt_states::TRANSIENT_INIT;
        case states::Ready:
            return alt_states::READY;
        case states::Startup:
            return alt_states::STARTUP;
        case states::RunMode1:
            return alt_states::RUNNING_FOLLOWING;
        case states::RunMode2:
            return alt_states::RUNNING_FORMING;
        case states::Standby:
            return alt_states::STANDBY_ERROR;
        case states::Shutdown:
            if (this->get_faults()) {
                return alt_states::FAULTED_STATE;
            } else {
                return alt_states::SHUTDOWN;
            }
        default:
            // unknown state
            return alt_states::STANDBY_ERROR;
    }
}

void Site_Manager::check_state(void) {
    Sequence& current_sequence = sequences[sequences_status.current_state];
    Path& current_path = current_sequence.paths[current_sequence.current_path_index];

    // if faulted or shutdown cmd, enter shutdown state
    if (sequences_status.current_state != Init && (current_sequence.check_faults() || (disable_flag.value.value_bool))) {
        sequences_status.current_state = Shutdown;
    }

    // check if alarms are present
    current_sequence.check_alarms();

    // count number of asset faults and asset alarms
    num_path_faults = current_path.num_active_faults;
    num_path_alarms = current_path.num_active_alarms;

    // boolean running status check
    running_status_flag.value.set((sequences_status.current_state == RunMode1) || sequences_status.current_state == RunMode2);

    // if new state detected, init vars as needed
    if (sequences_status.check_current_state != sequences_status.current_state) {
        char event_message[SHORT_MSG_LEN];
        FPS_INFO_LOG("Site Manager state change to: %s", state_name[sequences_status.current_state]);
        snprintf(event_message, SHORT_MSG_LEN, "State changed to %s", state_name[sequences_status.current_state]);
        emit_event("Site", event_message, STATUS_ALERT);
        sequences[sequences_status.check_current_state].sequence_bypass = false;  // ensure previous state executes next time its called
        sequences_status.check_current_state = sequences_status.current_state;
        site_state.value.set(state_name[sequences_status.current_state]);
        site_state_enum.value.set(sequences_status.current_state);
        sequences_status.sequence_reset = true;
    }

    // the only difference in the site_state_enum and the alt_site_state_enum is the faulted state
    // a faulted state is a substate of shutdown since any site lvl fault will place you in shutdown
    alt_site_state_enum.value.set(get_alternate_site_state());
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
    if (watchdog_feature.enable_flag.value.value_bool) {
        if (watchdog_feature.should_bark(sequences_status.current_time)) {
            dogbark();
        }
    }
    watchdog_feature.beat(sequences_status.current_time);

    // determine which state to run
    check_state();

    // run the sequence for the current state
    sequences[sequences_status.current_state].call_sequence(sequences_status);

    // run the current state function
    switch (sequences_status.current_state) {
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
            sequences_status.current_state = Error;
    }

    // set all interface variables
    set_values();
}

// initialization state - run once at program boot to initialize variables
void Site_Manager::init_state(void) {
    // FPS_ERROR_LOG("site manager Init State executed \n");
    emit_event("Site", "System initialized", INFO_ALERT);
    sequences_status.current_state = Ready;

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
    manual_power_mode.manual_solar_kW_slew.reset_slew_target();
    manual_power_mode.manual_ess_kW_slew.reset_slew_target();
    manual_power_mode.manual_gen_kW_slew.reset_slew_target();
    gen_kVAR_cmd_slew.reset_slew_target();
    ess_kVAR_cmd_slew.reset_slew_target();
    solar_kVAR_cmd_slew.reset_slew_target();
    reactive_setpoint.kVAR_cmd_slew.reset_slew_target();
    avr.kVAR_slew.reset_slew_target();

    // initialize watt_var points
    watt_var.init_curve();
    // initialize watt_watt points
    watt_watt.init_curve();

    // initialize load shed Variable_Regulator
    load_shed.init();

    // initialize solar shed Variable_Regulator
    solar_shed.init();

    // initialize active closed loop control Variable_Regulator
    active_power_closed_loop.regulator.min_offset = active_power_closed_loop.min_offset.value.value_int;
    active_power_closed_loop.regulator.max_offset = active_power_closed_loop.max_offset.value.value_int;
    active_power_closed_loop.regulator.offset = active_power_closed_loop.default_offset.value.value_int;
    active_power_closed_loop.regulator.default_offset = active_power_closed_loop.default_offset.value.value_int;
    // Set up update rate (updates per second) based on the millisecond rate given, with the fastest rate being 10ms
    active_power_closed_loop.update_rate_ms.value.set(std::max(active_power_closed_loop.update_rate_ms.value.value_int, 10));
    active_power_closed_loop.regulator.set_update_rate(1000 / static_cast<float>(active_power_closed_loop.update_rate_ms.value.value_int));
    // Set up steady state deadband condition as false for any value above the deadband
    active_power_closed_loop.regulator.set_default_condition(active_power_closed_loop.steady_state_deadband_kW.value.value_float, Variable_Regulator::VALUE_ABOVE);
    // Set up regulation deadband condition as false for any value above the 0.5% accuracy deadband compared to the POI
    active_power_closed_loop.regulator.set_control_high_threshold(active_power_closed_loop.regulation_deadband_kW.value.value_float);
    active_power_closed_loop.regulator.set_decrease_timer_duration_ms(active_power_closed_loop.decrease_timer_ms.value.value_int);
    // Set up regulation deadband condition as false for any value below the 0.5% accuracy deadband compared to the POI
    active_power_closed_loop.regulator.set_control_low_threshold(-1.0f * active_power_closed_loop.regulation_deadband_kW.value.value_float);
    active_power_closed_loop.regulator.set_increase_timer_duration_ms(active_power_closed_loop.increase_timer_ms.value.value_int);

    // initialize reactive closed loop control Variable_Regulator
    reactive_power_closed_loop.init();

    // initialize internal variables with config vars
    active_power_setpoint_mode.kW_slew.set_slew_rate(active_power_setpoint_mode.kW_slew_rate.value.value_int);
    manual_power_mode.manual_solar_kW_slew.set_slew_rate(manual_power_mode.manual_solar_kW_slew_rate.value.value_int);
    manual_power_mode.manual_ess_kW_slew.set_slew_rate(manual_power_mode.manual_ess_kW_slew_rate.value.value_int);
    manual_power_mode.manual_gen_kW_slew.set_slew_rate(manual_power_mode.manual_gen_kW_slew_rate.value.value_int);
    avr.kVAR_slew.set_slew_rate(avr.kVAR_slew_rate.value.value_int);
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
        sequences_status.current_state = Startup;
    }
}

// startup state - all startup logic triggered from sequences
void Site_Manager::startup_state(void) {}

/**
 * Run Mode 1 is one of two running modes and is typically considered to be the "grid tied"
 * running mode. In this function, runmode1 active/reactive power features, runmode1
 * standalone power features, and protection operations are executed.
 */
void Site_Manager::runmode1_state(void) {
    // Ensure runmode1 solar curtailment is enabled
    if (num_solar_running > 0) {
        pAssets->set_solar_curtailment_enabled(true);
    }

    // if the currently selected runmode1 kW feature uses charge control, set asset_cmd internal ess charge kW request variable (and limit it)
    if (charge_control.enable_flag.value.value_bool) {
        asset_cmd.ess_data.kW_request = range_check(charge_control.kW_request.value.value_float, charge_control.kW_limit.value.value_float,
                                                    -1 * charge_control.kW_limit.value.value_float);
    }

    // Active Power Features

    // all runmode1 active power features contained here
    process_runmode1_kW_feature();

    // when enabled, Aggregated Asset Limit will not allow controllable ESS to cause all ESS+Solar to exceed feature limit
    if (agg_asset_limit.enable_flag.value.value_bool) {
        agg_asset_limit.execute(asset_cmd, pAssets->get_ess_total_uncontrollable_active_power(), pAssets->get_solar_total_uncontrollable_active_power(),
                                max_potential_ess_kW.value.value_float, min_potential_ess_kW.value.value_float, total_asset_kW_discharge_limit,
                                pAssets->get_ess_total_kW_discharge_limit());
    }

    // when enabled, ESS Discharge Prevention feature will not allow ESS to discharge if average SOC is below the configured EDP SoC threshold
    if (ess_discharge_prevention.enable_flag.value.value_bool) {
        ess_discharge_prevention.execute(asset_cmd, soc_avg_running.value.value_float, max_potential_ess_kW.value.value_float, min_potential_ess_kW.value.value_float,
                                         pAssets->get_ess_total_kW_discharge_limit(), total_asset_kW_discharge_limit);
    }

    // FR MODE (frequency response) - output or absorb additional power if frequency deviates by a set amount
    if (frequency_response.enable_flag.value.value_bool) {
        // don't run if ess_discharge_prevention is enabled
        if (!ess_discharge_prevention.enable_flag.value.value_bool) {
            frequency_response.execute(asset_cmd, get_ess_total_rated_active_power(), site_frequency.value.value_float, sequences_status.current_time);
        }
    } else {
        frequency_response.clear_all_component_outputs();
    }

    // limit power values based on the amount of power that the POI can legally/physically handle
    if (active_power_poi_limits.enable_flag.value.value_bool) {
        active_power_poi_limits.execute(asset_cmd, soc_avg_running.value.value_float, asset_priority_runmode1.value.value_int, total_site_kW_charge_limit.value.value_float,
                                        total_site_kW_discharge_limit.value.value_float);
    }

    // Preserve demand prior to POI corrections
    asset_cmd.preserve_uncorrected_site_kW_demand();

    // reinit internal debug value
    charge_dispatch.kW_command.value.set(0.0f);

    // adjust power request to account for transformer losses at the poi
    if (watt_watt.enable_flag.value.value_bool) {
        watt_watt.execute(asset_cmd);
    }

    // close the loop on active power, taking into account the value at the POI and making adjustments as needed
    if (active_power_closed_loop.enable_flag.value.value_bool) {
        active_power_closed_loop.execute(asset_cmd, feeder_actual_kW.value.value_float, total_site_kW_rated_charge.value.value_float,
                                         total_site_kW_rated_discharge.value.value_float);
        // Reset when disabled
    } else {
        active_power_closed_loop.regulator.reset();
    }

    // dispatch the active power request to the various assets
    dispatch_active_power(asset_priority_runmode1.value.value_int);

    // charge ess to unload gen + solar during their rampdown if they're too slow
    if (active_power_setpoint_mode.ess_charge_support_enable_flag.value.value_bool) {
        active_power_setpoint_mode.charge_support_execute(asset_cmd);
    }

    // Reactive Power Features

    // all runmode1 reactive power features contained here
    process_runmode1_kVAR_feature();

    if (reactive_power_poi_limits.enable_flag.value.value_bool) {
        reactive_power_poi_limits.execute(asset_cmd);
    }

    // close the loop on reactive power, taking into account the value at the POI and making adjustments as needed
    if (reactive_power_closed_loop.enable_flag.value.value_bool) {
        reactive_power_closed_loop.execute(asset_cmd, feeder_actual_kVAR.value.value_float, total_site_kVAR_rated_charge, total_site_kVAR_rated_discharge);
        // Reset when disabled
    } else {
        reactive_power_closed_loop.regulator.reset();
    }

    // dispatch the reactive power request to the various assets
    asset_cmd.dispatch_reactive_power();

    // set power cmds after all algorithms have opportunity to execute
    set_asset_power_commands();

    // Calculate final reported site limits
    calculate_total_site_kW_limits();

    // start first gen if ESS SOC < setpoint
    pAssets->start_first_gen((asset_cmd.gen_data.start_first_flag || (soc_avg_running.value.value_float <= start_first_gen_soc.value.value_float)) &&
                             asset_cmd.gen_data.auto_restart_flag);

    // start any available ESS stopped or in standby
    if (asset_cmd.ess_data.auto_restart_flag) {
        pAssets->start_available_ess();
    }

    // start any available Solar stopped or in standby
    if (asset_cmd.solar_data.auto_restart_flag) {
        pAssets->start_available_solar();
    }

    // enter standby mode
    if (standby_flag.value.value_bool) {
        standby_ess_latch = false;
        standby_solar_latch = false;
        sequences_status.current_state = Standby;
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
        solar_shed.execute(asset_cmd, soc_avg_running.value.value_float, pAssets->get_solar_total_rated_active_power(), max_potential_solar_kW.value.value_float);

    // all runmode2 active power features contained here
    process_runmode2_kW_feature();

    // divide the active power demand between the various assets
    dispatch_active_power(asset_priority_runmode2.value.value_int);

    // update load shed value
    if (load_shed.enable_flag.value.value_bool)
        load_shed.execute(soc_avg_running.value.value_float);

    // after deciding which assets get how much power, send out the power commands
    set_asset_power_commands();

    // start any available ESS stopped or in standby
    if (asset_cmd.ess_data.auto_restart_flag)
        pAssets->start_available_ess();

    // start any available Solar stopped or in standby
    if (asset_cmd.solar_data.auto_restart_flag)
        pAssets->start_available_solar();

    // start first gen if ESS SOC < setpoint
    pAssets->start_first_gen((asset_cmd.gen_data.start_first_flag || (soc_avg_running.value.value_float <= start_first_gen_soc.value.value_float)) &&
                             asset_cmd.gen_data.auto_restart_flag);
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
        sequences_status.current_state = RunMode1;
    }
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
    manual_power_mode.manual_solar_kW_slew.reset_slew_target();
    manual_power_mode.manual_ess_kW_slew.reset_slew_target();
    manual_power_mode.manual_gen_kW_slew.reset_slew_target();
    gen_kVAR_cmd_slew.reset_slew_target();
    ess_kVAR_cmd_slew.reset_slew_target();
    solar_kVAR_cmd_slew.reset_slew_target();
    reactive_setpoint.kVAR_cmd_slew.reset_slew_target();
    avr.kVAR_slew.reset_slew_target();

    // Reset load shed value to max
    load_shed.reset();

    // Reset solar shed value to max
    solar_shed.reset();

    // Reset closed loop control value to default
    active_power_closed_loop.regulator.offset = active_power_closed_loop.default_offset.value.value_float;

    // tell dbi not to execute start cmd if restart / power cycle happens
    if (data_endpoint != NULL && enable_flag.value.value_bool != false) {
        data_endpoint->turn_off_start_cmd();
    }

    if (sequences[Shutdown].sequence_bypass == true)
        sequences_status.current_state = Ready;

    // set all inputs to enable_flag to false
    clear_faults_flag.value.set(false);
    for (auto& input_flag : clear_faults_flag.inputs) {
        input_flag.set(false);
    }

    // set start asset flags false since algorithm isnt running to check
    asset_cmd.ess_data.start_first_flag = false;
    asset_cmd.gen_data.start_first_flag = false;
    asset_cmd.solar_data.start_first_flag = false;
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
        active_power_setpoint_mode.execute(asset_cmd, this->total_site_kW_rated_discharge.value.value_float, this->total_site_kW_rated_charge.value.value_float);
    }
    // MANUAL MODE takes an ESS kW cmd, solar kW cmd, generator kW cmd, ESS slew rate, solar slew rate, and generator slew rate and routes those commands through dispatch and
    // charge control
    else if (manual_power_mode.enable_flag.value.value_bool) {
        manual_power_mode.execute(asset_cmd);
    }
    // ENERGY ARBITRAGE MODE determines storage charge/discharge based on current price and thresholds
    else if (energy_arbitrage.enable_flag.value.value_bool) {
        if (!energy_arbitrage.execute(asset_cmd, soc_avg_running.value.value_float, active_alarm_array)) {
            set_alarms(energy_arbitrage.alarm_index);
        }
    }
    // ESS Calibration Mode takes an ESS kW_cmd and routes it to each ESS without balancing, and with optional soc and voltage limits
    else if (ess_calibration.enable_flag.value.value_bool) {
        ess_calibration.execute(asset_cmd, pAssets->get_num_ess_controllable());
    }

    asset_cmd.calculate_feature_kW_demand(asset_priority_runmode1.value.value_int);

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
    asset_cmd.additional_load_compensation = asset_cmd_utils::calculate_additional_load_compensation(asset_cmd.load_method, asset_cmd.site_kW_load, asset_cmd.site_kW_demand,
                                                                                                     asset_cmd.ess_data.kW_request, asset_cmd.gen_data.kW_request,
                                                                                                     asset_cmd.solar_data.kW_request);
    asset_cmd.site_kW_demand += asset_cmd.additional_load_compensation;

    // Request full charge which may be satisfied based on availability of other assets
    // If no other assets available, ESS will discharge to compensate for load
    asset_cmd.ess_data.kW_request = asset_cmd.ess_data.min_potential_kW;

    // Feature selection. Currently, only Generator Charge is offered
    if (generator_charge.enable_flag.value.value_bool)
        generator_charge.execute(asset_cmd, solar_shed, max_potential_gen_kW.value.value_float);

    asset_cmd.calculate_feature_kW_demand(asset_priority_runmode2.value.value_float);
}

/**
 * Calls the selected reactive power feature that will determine the target reactive power output
 * of all aggregated assets and modify variables such as site_kVAR_demand.
 */
void Site_Manager::process_runmode1_kVAR_feature() {
    // AUTOMATIC VOLTAGE REGULATION MODE will sink or supply power based on voltage deviation
    if (avr.enable_flag.value.value_bool) {
        // automatic voltage regulation function
        avr.execute(asset_cmd, asset_pf_flag);
    }
    // WATT-VAR MODE sets kVAr command based on where actual active power sits on watt_var_points curve
    else if (watt_var.enable_flag.value.value_bool) {
        // watt-var function
        watt_var.execute(asset_cmd, ess_kW_cmd.value.value_float + solar_kW_cmd.value.value_float + gen_kW_cmd.value.value_float, asset_pf_flag);
    }
    // REACTIVE POWER SETPOINT MODE takes a single reactive power setpoint and passes it on for asset distribution
    else if (reactive_setpoint.enable_flag.value.value_bool) {
        reactive_setpoint.execute(asset_cmd, asset_pf_flag);
    }

    // POWER FACTOR MODE = tbd
    else if (power_factor.enable_flag.value.value_bool) {
        power_factor.execute(asset_cmd, asset_pf_flag);
    }
    // Constant Power Factor Mode
    else if (constant_power_factor.enable_flag.value.value_bool) {
        constant_power_factor.execute(asset_cmd, asset_pf_flag);
    } else  // TODO:use another feature to set
    {
        asset_cmd.site_kVAR_demand = 0;

        // this mode does not use power factor control
        asset_pf_flag = false;
    }

    // set UI/FIMS var equal to internal calculation for site demand
    site_kVAR_demand.value.set(asset_cmd.site_kVAR_demand);
}

/**
 * Processes the request active power from the active power feature calculation by splitting the request
 * between all asset types.
 * @param asset_priority An enumerated integer selecting the order in which assets should be assigned power.
 */
void Site_Manager::dispatch_active_power(int asset_priority) {
    // Extract the total charge and discharge available for dispatch
    asset_cmd_utils::site_kW_production_limits site_kW_prod_limits = asset_cmd_utils::calculate_site_kW_production_limits(
        asset_cmd.ess_data.kW_request, asset_cmd.gen_data.kW_request, asset_cmd.solar_data.kW_request, asset_cmd.load_method, asset_cmd.additional_load_compensation,
        asset_cmd.feature_kW_demand, asset_cmd.site_kW_demand);
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
    aggregate_dispatch += asset_cmd.dispatch_site_kW_charge_cmd(asset_priority, charge_dispatch.solar_enable_flag.value.value_bool,
                                                                charge_dispatch.gen_enable_flag.value.value_bool, charge_dispatch.feeder_enable_flag.value.value_bool);

    // Process and dispatch remaining discharge production to each asset type
    aggregate_dispatch += asset_cmd.dispatch_site_kW_discharge_cmd(asset_priority, asset_cmd.site_kW_discharge_production, REQUESTS);
    aggregate_dispatch += asset_cmd.dispatch_site_kW_discharge_cmd(asset_priority, asset_cmd.site_kW_discharge_production, DEMAND);

    // set internal debug value to power from charge control
    charge_dispatch.kW_command.value.set(aggregate_dispatch);
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

bool Site_Manager::synchronize_ess() {
    pAssets->set_ess_voltage_setpoint(pAssets->get_sync_feeder_gridside_avg_voltage());
    pAssets->set_ess_frequency_setpoint(pAssets->get_sync_feeder_gridside_frequency() -
                                        pAssets->get_sync_frequency_offset());  //-pAssets->get_sync_feeder_frequency_offset());

    return pAssets->get_sync_feeder_status();
}

/**
 * @brief Get ESS Calibration Mode variables from Assets
 */
void Site_Manager::get_ess_calibration_variables() {
    std::vector<int> ess_setpoint_statuses = pAssets->get_ess_setpoint_statuses();
    ess_calibration.num_setpoint.value.set(ess_setpoint_statuses[ACCEPTED]);
    ess_calibration.num_limited.value.set(ess_setpoint_statuses[LIMITED]);
    ess_calibration.num_zero.value.set(ess_setpoint_statuses[ZERO]);
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
    settings.soc_protection_buffers_disable = ess_calibration.enable_flag.value.value_bool;
    settings.soc_limits_flag = ess_calibration.soc_limits_enable.value.value_bool;
    settings.min_soc_limit = ess_calibration.min_soc_limit.value.value_float;
    settings.max_soc_limit = ess_calibration.max_soc_limit.value.value_float;
    settings.cell_voltage_limits = ess_calibration.cell_voltage_limits_enable.value.value_bool;
    settings.min_cell_voltage_limit = ess_calibration.min_cell_voltage_limit.value.value_float;
    settings.max_cell_voltage_limit = ess_calibration.max_cell_voltage_limit.value.value_float;
    settings.rack_voltage_limits = ess_calibration.rack_voltage_limits_enable.value.value_bool;
    settings.min_rack_voltage_limit = ess_calibration.min_rack_voltage_limit.value.value_float;
    settings.max_rack_voltage_limit = ess_calibration.max_rack_voltage_limit.value.value_float;
    // Write through the feature setpoint as well so we have a reference of whether it's being met for each asset
    settings.raw_feature_setpoint = ess_calibration.kW_cmd.value.value_float;
    pAssets->set_ess_calibration_vars(settings);
}

// Executes watchdog failure reponses
void Site_Manager::dogbark() {
    // If watchdog has already barked, don't bark again
    if (!active_alarm_array[4]) {
        set_alarms(4);
    }
}

/**
 * Aggregates LDSS feature settings and passes them down to update the feature.
 */
void Site_Manager::set_ldss_variables() {
    auto ldss_settings = LDSS_Settings();
    ldss_settings.enabled = ldss.enable_flag.value.value_bool;
    ldss_settings.priority_setting = (LDSS_Priority_Setting)ldss.ldss_priority_setting.value.value_int;
    ldss_settings.max_load_threshold_percent = ldss.ldss_max_load_threshold_percent.value.value_float;
    ldss_settings.min_load_threshold_percent = ldss.ldss_min_load_threshold_percent.value.value_float;
    ldss_settings.warmup_time = ldss.ldss_warmup_time.value.value_int;
    ldss_settings.cooldown_time = ldss.ldss_cooldown_time.value.value_int;
    ldss_settings.start_gen_time = ldss.ldss_start_gen_time.value.value_int;
    ldss_settings.stop_gen_time = ldss.ldss_stop_gen_time.value.value_int;
    ldss_settings.enable_soc_thresholds_flag = ldss.ldss_enable_soc_threshold.value.value_bool;
    ldss_settings.min_soc_percent = ldss.ldss_min_soc_threshold_percent.value.value_float;
    ldss_settings.max_soc_percent = ldss.ldss_max_soc_threshold_percent.value.value_float;
    pAssets->update_ldss_settings(std::move(ldss_settings));
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
