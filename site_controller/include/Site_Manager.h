/*
 * Site_Manager.h
 *
 *  Created on: Sep 5, 2018
 *      Author: jcalcagni
 */
#ifndef SITE_MANAGER_H_
#define SITE_MANAGER_H_

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
#include <spdlog/fmt/fmt.h>
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Asset_Manager.h>
#include <Value_Object.h>
#include <Input_Sources.h>
#include <Slew_Object.h>
#include <Variable_Regulator.h>
#include <Asset_Cmd_Object.h>
#include <Fims_Object.h>
#include <Sequence.h>
#include <macros.h>
#include <Types.h>
#include <version.h>
#include <Config_Validation_Result.h>
#include <Features/Feature.h>
#include <Features/Active_Power_Setpoint.h>
#include <Features/AVR.h>
#include <Features/Active_Power_Closed_Loop_Control.h>
#include <Features/Target_SOC.h>
#include <Features/Watt_Var.h>
#include <Features/Reactive_Setpoint.h>
#include <Features/Direct_Power_Factor.h>
#include <Features/Constant_Power_Factor.h>
#include <Features/Active_Power_POI_Limits.h>
#include <Features/Reactive_Power_POI_Limits.h>
#include <Features/Watt_Watt.h>
#include <Features/LDSS.h>
#include <Features/Load_Shed.h>
#include <Features/Solar_Shed.h>
#include <Features/ESS_Discharge_Prevention.h>
#include <Features/Aggregated_Asset_Limit.h>
#include <Features/Reactive_Power_Closed_Loop_Control.h>
#include <Features/Watchdog.h>
#include <Features/Charge_Dispatch.h>
#include <Features/Charge_Control.h>
#include <Features/Energy_Arbitrage.h>
#include <Features/Frequency_Response.h>
#include <Features/ESS_Calibration.h>
#include <Features/Generator_Charge.h>
#include <Features/Manual.h>
class Site_Manager {
    ////////////////////////////////////////////////////////////////////////////////////////
    //                              SITE MANAGER CONFIGURATION                            //
    ////////////////////////////////////////////////////////////////////////////////////////
public:
    Site_Manager(Version* release_version);
    bool configure(Asset_Manager* man, fims* fim, cJSON* sequenceRoot, cJSON* varRoot, bool* primary_controller);

protected:
    bool parse_variables(cJSON* object);
    std::pair<Fims_Object, Config_Validation_Result> parse_field_defaults(cJSON* JSON_defaults);
    std::pair<cJSON*, Config_Validation_Result> parse_flatten_vars(cJSON* JSON_object);
    void post_configure_initialize_features();

    ////////////////////////////////////////////////////////////////////////////////////////
    //                              PUBLISHING & FIMS MANAGEMENT                          //
    ////////////////////////////////////////////////////////////////////////////////////////
public:
    void fims_data_parse(fims_message* msg);
    void publish_all_FIMS();

protected:
    void send_FIMS(const char* method, const char* uri, const char* replyto, const char* body);
    void ui_configuration(void);
    void build_JSON_site(fmt::memory_buffer& buf);
    void build_JSON_site_summary(fmt::memory_buffer& buf, bool clothed, const char* const var = NULL);
    void build_JSON_site_operation(fmt::memory_buffer& buf, const char* const var = NULL);
    void build_JSON_site_configuration(fmt::memory_buffer& buf, const char* const var = NULL);
    void build_JSON_site_cops(fmt::memory_buffer& buf, const char* const var = NULL);
    void build_JSON_site_input_sources(fmt::memory_buffer& buf, const char* const var = NULL);
    void build_JSON_features(fmt::memory_buffer& buf);
    void build_JSON_features_summary(fmt::memory_buffer& buf, bool clothed, const char* const var = NULL);
    void build_JSON_features_active_power(fmt::memory_buffer& buf, const char* const var = NULL);
    void build_JSON_features_reactive_power(fmt::memory_buffer& buf, const char* const var = NULL);
    void build_JSON_features_standalone_power(fmt::memory_buffer& buf, const char* const var = NULL);
    void build_JSON_features_site_operation(fmt::memory_buffer& buf, const char* const var = NULL);
    bool* is_primary;  // Indicates whether this is the primary controller (true) or running in shadow mode (false)
    fims* pFims;
    fmt::memory_buffer send_FIMS_buf;  // Reusable and resizable string buffer used by the send_FIMS function

    ////////////////////////////////////////////////////////////////////////////////////////
    //                                 ALARM & FAULT HANDLING                             //
    ////////////////////////////////////////////////////////////////////////////////////////
public:
    void set_faults(int fault_number);
    void set_alarms(int alarm_number);
    bool get_faults() const;
    bool get_alarms() const;
    // Used by path to determine if fault has already been detected
    bool get_active_faults(int index);
    bool get_active_alarms(int index);

protected:
    void clear_fault_registers(void);
    void clear_alarms(int alarm_number);
    void clear_faults();
    void build_active_faults();
    void build_active_alarms();
    bool clear_fault_status_flags;
    timespec time_to_clear_fault_status_flags;
    bool active_fault_array[64];
    bool active_alarm_array[64];
    int num_path_faults;
    int num_path_alarms;

    ////////////////////////////////////////////////////////////////////////////////////////
    //                               SEQUENCES, STATES, & PATHS                           //
    ////////////////////////////////////////////////////////////////////////////////////////
public:
    void process_state();

    // Sequence functions and helpers
    bool call_sequence_functions(const char* target_asset, const char* asset_cmd, Value_Object* value, int tolerance_percent);
    void handle_reserved_variable_sequence_functions(const char* cmd, Value_Object* value, int tolerance_percent, bool& command_found, bool& return_value);
    void handle_ess_sequence_functions(const char* cmd, Value_Object* value, int tolerance_percent, bool& command_found, bool& return_value);
    void handle_solar_sequence_functions(const char* cmd, Value_Object* value, int tolerance_percent, bool& command_found, bool& return_value);
    void handle_gen_sequence_functions(const char* cmd, Value_Object* value, int tolerance_percent, bool& command_found, bool& return_value);
    void handle_feed_sequence_functions(const char* cmd, Value_Object* value, bool& command_found, bool& return_value);
    void handle_config_sequence_functions(const char* cmd, Value_Object* value, bool& command_found, bool& return_value);
    void handle_feat_sequence_functions(const char* cmd, Value_Object* value, bool& command_found);
    void handle_specific_feed_sequence_functions(const char* cmd, Value_Object* value, const char* target_asset, bool& command_found, bool& return_value);

    void set_site_status(const char*);

    // Sequences_Status contains (times, states, and other sequences info)
    Sequences_Status sequences_status;

    Fims_Object get_reserved_bool_1();
    Fims_Object get_reserved_bool_2();
    Fims_Object get_reserved_bool_3();
    Fims_Object get_reserved_bool_4();
    Fims_Object get_reserved_bool_5();
    Fims_Object get_reserved_bool_6();
    Fims_Object get_reserved_bool_7();
    Fims_Object get_reserved_bool_8();
    Fims_Object get_reserved_bool_9();
    Fims_Object get_reserved_bool_10();
    Fims_Object get_reserved_bool_11();
    Fims_Object get_reserved_bool_12();
    Fims_Object get_reserved_bool_13();
    Fims_Object get_reserved_bool_14();
    Fims_Object get_reserved_bool_15();
    Fims_Object get_reserved_bool_16();

protected:
    void get_values();
    void set_values();
    void check_state();
    void init_state();
    void ready_state();
    void startup_state();
    void runmode1_state();
    void runmode2_state();
    void standby_state();
    void shutdown_state();
    void error_state();
    bool set_state(states state_request);
    std::vector<Sequence> sequences;

    ////////////////////////////////////////////////////////////////////////////////////////
    //                                  SITE CONFIGURATION                                //
    ////////////////////////////////////////////////////////////////////////////////////////
public:
    Input_Source_List input_sources;

protected:
    std::vector<Fims_Object*> multi_input_command_vars;
    Fims_Object input_source_status;
    Fims_Object power_priority_flag;  // false for active power priority, true for reactive power priority
    Fims_Object invert_poi_kW;        // if the on-site poi reports power with a polarity reverse of expected, this flag can signal Site Manager to flip the polarity

    ////////////////////////////////////////////////////////////////////////////////////////
    //                                    SITE OPERATION                                  //
    ////////////////////////////////////////////////////////////////////////////////////////
    void dispatch_active_power(int asset_priority);
    void set_asset_power_commands();
    Fims_Object reserved_bool_1, reserved_bool_2, reserved_bool_3, reserved_bool_4, reserved_bool_5, reserved_bool_6, reserved_bool_7, reserved_bool_8;
    Fims_Object reserved_bool_9, reserved_bool_10, reserved_bool_11, reserved_bool_12, reserved_bool_13, reserved_bool_14, reserved_bool_15, reserved_bool_16;
    Fims_Object reserved_float_1, reserved_float_2, reserved_float_3, reserved_float_4, reserved_float_5, reserved_float_6, reserved_float_7, reserved_float_8;
    Fims_Object enable_flag, disable_flag, standby_flag, clear_faults_flag;
    Fims_Object faults, active_faults;
    Fims_Object alarms, active_alarms;
    Fims_Object site_state, site_state_enum, site_status;
    Fims_Object running_status_flag, alarm_status_flag, fault_status_flag;
    Fims_Object exit_timer;                                        // timeout for failed sequence step in ms
    Fims_Object asset_priority_runmode1, asset_priority_runmode2;  // Enumeratred integer indiciating the priority of the assets in meeting the demand
                                                                   // See Types.h "priority" enum for the list of available values
    Fims_Object site_kW_demand, site_kW_load;                      // site total kW demand and load calculation
    Fims_Object site_kW_load_interval_ms;                          // Duration of site load to track
    Fims_Object site_kW_load_inclusion;                            // Whether the feature includes site load
    Fims_Object site_kVAR_demand;                                  // total site kVAR demand (from reactive power features)
    Fims_Object site_frequency;
    Fims_Object configured_primary;
    void set_enable_flag(bool value);

    ////////////////////////////////////////////////////////////////////////////////////////
    //                                          COPS                                      //
    ////////////////////////////////////////////////////////////////////////////////////////
protected:
    Fims_Object cops_heartbeat;
    Fims_Object process_id;
    Fims_Object release_version_tag;
    Fims_Object release_version_commit;
    Fims_Object release_version_build;

    ////////////////////////////////////////////////////////////////////////////////////////
    //                           ASSET-SPECIFIC VARIABLES/FUNCTIONS                       //
    ////////////////////////////////////////////////////////////////////////////////////////
public:
    Asset_Manager* pAssets;
    //
    // ESS
    //
    float get_ess_total_rated_active_power(void);

protected:
    void update_ess_kpi_values();
    bool synchronize_ess(void);
    Fims_Object soc_min_all, soc_max_all, soc_avg_all, soc_min_running, soc_max_running, soc_avg_running;
    Fims_Object max_potential_ess_kW, min_potential_ess_kW, rated_ess_kW, potential_ess_kVAR;
    Fims_Object ess_instant_discharge, ess_instant_charge_grid, ess_instant_charge_pv;
    Fims_Object ess_actual_kW, ess_actual_kVAR;
    Fims_Object ess_kVAR_slew_rate;      // kVAR/s
    Fims_Object start_first_ess_kW;      // kW thresholds to check for issuing start asset request to asset manager
    Fims_Object allow_ess_auto_restart;  // prevent asset from starting up once stopped
    Fims_Object ess_kW_cmd, ess_kVAR_cmd;
    int num_ess_available, num_ess_running, num_ess_controllable;
    float prev_ess_kW_cmd, prev_ess_kVAR_cmd;
    bool standby_ess_latch;  // boolean latch to prevent multiple standby function calls
    Slew_Object ess_kVAR_cmd_slew;
    float ess_soc_balancing_factor;
    //
    // Feeder
    //
    Fims_Object max_potential_feeder_kW, min_potential_feeder_kW, rated_feeder_kW;
    Fims_Object feeder_actual_kW, feeder_actual_kVAR, feeder_kW_cmd, feeder_actual_pf;
    float prev_feeder_kW_cmd;
    //
    // Generator
    //
    Fims_Object max_potential_gen_kW, min_potential_gen_kW, rated_gen_kW;
    Fims_Object potential_gen_kVAR;
    Fims_Object gen_actual_kW, gen_actual_kVAR;
    Fims_Object gen_kVAR_slew_rate;      // kVAR/s
    Fims_Object start_first_gen_kW;      // kW thresholds to check for issuing start asset request to asset manager
    Fims_Object start_first_gen_soc;     // min soc to start first generator
    Fims_Object allow_gen_auto_restart;  // prevent asset from starting up once stopped
    Fims_Object gen_kW_cmd, gen_kVAR_cmd;
    int num_gen_available, num_gen_running, num_gen_active;
    float prev_gen_kW_cmd, prev_gen_kVAR_cmd;
    Slew_Object gen_kVAR_cmd_slew;
    //
    // Solar
    //
    Fims_Object max_potential_solar_kW, min_potential_solar_kW, rated_solar_kW, potential_solar_kVAR;
    Fims_Object solar_actual_kW, solar_actual_kVAR;
    Fims_Object solar_kVAR_slew_rate;      // kVAR/s
    Fims_Object start_first_solar_kW;      // kW thresholds to check for issuing start asset request to asset manager
    Fims_Object allow_solar_auto_restart;  // prevent asset from starting from a stopped state
    Fims_Object solar_kW_cmd, solar_kVAR_cmd;
    int num_solar_available, num_solar_running;
    float prev_solar_kW_cmd, prev_solar_kVAR_cmd;
    bool standby_solar_latch;  // boolean latch to prevent multiple standby function calls
    Slew_Object solar_kVAR_cmd_slew;

    ////////////////////////////////////////////////////////////////////////////////////////
    //                                         FEATURES                                   //
    ////////////////////////////////////////////////////////////////////////////////////////
public:
    Asset_Cmd_Object asset_cmd;

protected:
    void update_power_feature_selections();
    void read_site_feature_controls();
    void ui_disable_feature_group_feature_vars(std::vector<Feature*>& feature_list);
    void add_feature_group_feature_vars_to_JSON_buffer(const std::vector<Feature*>& feature_list, fmt::memory_buffer& buf, const char* const var = NULL);
    void add_feature_group_summary_vars_to_JSON_buffer(const std::vector<Feature*>& feature_list, fmt::memory_buffer& buf, const char* const var = NULL);

    ////////////////////////////////////////////////////////////////////////////////////////
    //                            RUN MODE 1 ACTIVE POWER FEATURES                        //
    ////////////////////////////////////////////////////////////////////////////////////////
    //
    // Feature selection
    //
    void process_runmode1_kW_feature();
    bool configure_available_runmode1_kW_features_list();
    void set_asset_cmd_variables();
    void set_volatile_asset_cmd_variables();
    void calculate_total_site_kW_limits();
    void remove_active_poi_corrections_from_slew_targets();
    Fims_Object available_features_runmode1_kW_mode;
    Fims_Object runmode1_kW_mode_cmd;
    Fims_Object runmode1_kW_mode_status;
    int current_runmode1_kW_feature;  // follows the Fims_Object runmode1_kW_mode_cmd to identify when it gets changed
    uint64_t available_runmode1_kW_features_mask;
    uint64_t runmode1_kW_features_charge_control_mask = 2;  // Mask indicating which active power features utilize charge control
    std::vector<Feature*> runmode1_kW_features_list = std::vector<Feature*>{
        &energy_arbitrage, &target_soc, &active_power_setpoint_mode, &manual_power_mode, &ess_calibration,
    };
    Fims_Object feature_kW_demand;
    Fims_Object site_kW_charge_production;      // Charge up to this amount during dispatch
    Fims_Object site_kW_discharge_production;   // Discharge up to this amount during dispatch
    Fims_Object total_site_kW_rated_charge;     // (Controllable) Rated active power charge of the site
    Fims_Object total_site_kW_rated_discharge;  // (Controllable) Rated active power discharge of the site
    float total_asset_kW_charge_limit;          // Minimum total charge production allowed by assets
    float total_asset_kW_discharge_limit;       // Maximum total discharge production allowed by assets;
    Fims_Object total_site_kW_charge_limit;     // Minimum total charge production allowed by the site based on available power and feature limitations
    Fims_Object total_site_kW_discharge_limit;  // Maximum total discharge production allowed by the site based on available power and feature limitations

protected:
    features::Energy_Arbitrage energy_arbitrage;

    features::Target_SOC target_soc;

    features::Active_Power_Setpoint active_power_setpoint_mode;

    features::Manual manual_power_mode;

public:
    features::ESS_Calibration ess_calibration;
    void get_ess_calibration_variables();
    void set_ess_calibration_variables();

    ////////////////////////////////////////////////////////////////////////////////////
    //                          RUN MODE 2 ACTIVE POWER FEATURES                      //
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Feature selection
    //
    void process_runmode2_kW_feature();
    bool configure_available_runmode2_kW_features_list();
    Fims_Object available_features_runmode2_kW_mode;
    Fims_Object runmode2_kW_mode_cmd;
    Fims_Object runmode2_kW_mode_status;
    int current_runmode2_kW_feature;  // follows the Fims_Object runmode2_kW_mode_cmd to identify when it gets changed
    uint64_t available_runmode2_kW_features_mask;
    std::vector<Feature*> runmode2_kW_features_list = {
        &generator_charge,
    };

    features::Generator_Charge generator_charge;

    ///////////////////////////////////////////////////////////////////
    //                          CHARGE FEATURES                      //
    ///////////////////////////////////////////////////////////////////
    std::vector<Feature*> charge_features_list = {
        // Separate list for charge dispatch and charge control allowing them to be enabled in parallel
        &charge_dispatch,
        &charge_control,
    };
    // Charge Dispatch - Always enabled
    features::Charge_Dispatch charge_dispatch;

    // Charge Control - Enabled based on active power feature charge_control mask
    features::Charge_Control charge_control;

    ////////////////////////////////////////////////////////////////////////////////////////
    //                           RUN MODE 1 REACTIVE POWER FEATURES                       //
    ////////////////////////////////////////////////////////////////////////////////////////
    //
    // Feature selection
    //
    void process_runmode1_kVAR_feature();
    bool configure_available_runmode1_kVAR_features_list();
    void remove_reactive_poi_corrections_from_slew_targets();
    Fims_Object available_features_runmode1_kVAR_mode;
    // TODO: make these Fims Objects so they can be configured and exposed on fims
    float total_site_kVAR_rated_charge;     // (Controllable) Rated reactive power charge of the site
    float total_site_kVAR_rated_discharge;  // (Controllable) Rated reactive power discharge of the site
    Fims_Object runmode1_kVAR_mode_cmd;
    Fims_Object runmode1_kVAR_mode_status;
    int current_runmode1_kVAR_feature;  // follows the Fims_Object runmode1_kVAR_mode_cmd to identify when it gets changed
    uint64_t available_runmode1_kVAR_features_mask;
    std::vector<Feature*> runmode1_kVAR_features_list = {
        &avr, &watt_var, &reactive_setpoint, &power_factor, &constant_power_factor,
    };

    features::Watt_Var watt_var;

protected:
    features::Reactive_Setpoint reactive_setpoint;

    features::Direct_Power_Factor power_factor;
    bool asset_pf_flag, prev_asset_pf_flag;  // track the change to/from any mode that uses power factor cmds to enable assets to follow them

    features::Constant_Power_Factor constant_power_factor;

    features::AVR avr;

    ///////////////////////////////////////////////////////////////////////////////////
    //                       RUN MODE 2 REACTIVE POWER FEATURES                      //
    ///////////////////////////////////////////////////////////////////////////////////
    //
    // Feature selection
    //
    bool configure_available_runmode2_kVAR_features_list();
    Fims_Object available_features_runmode2_kVAR_mode;
    Fims_Object runmode2_kVAR_mode_cmd;
    Fims_Object runmode2_kVAR_mode_status;
    int current_runmode2_kVAR_feature;  // follows the Fims_Object runmode2_kVAR_mode_cmd to identify when it gets changed
    uint64_t available_runmode2_kVAR_features_mask;
    std::vector<Feature*> runmode2_kVAR_features_list;
    //
    // No islanded/runmode2 kVAR features yet
    //

    ////////////////////////////////////////////////////////////////////////////////////////
    //                                  STANDALONE POWER FEATURES                         //
    ////////////////////////////////////////////////////////////////////////////////////////
    //
    // feature selection
    //
    Fims_Object available_features_standalone_power;
    uint64_t available_standalone_power_features_mask;
    bool configure_available_standalone_power_features_list();
    std::vector<Feature*> standalone_power_features_list = {
        &active_power_poi_limits, &frequency_response, &watt_watt, &ldss, &load_shed, &solar_shed, &ess_discharge_prevention, &agg_asset_limit, &active_power_closed_loop, &reactive_power_closed_loop, &reactive_power_poi_limits,
    };

    features::Active_Power_POI_Limits active_power_poi_limits;

    features::Reactive_Power_POI_Limits reactive_power_poi_limits;

    features::Frequency_Response frequency_response;

    features::Watt_Watt watt_watt;

    features::LDSS ldss;
    void set_ldss_variables();

    features::Load_Shed load_shed;

    features::Solar_Shed solar_shed;

    features::ESS_Discharge_Prevention ess_discharge_prevention;

    features::Aggregated_Asset_Limit agg_asset_limit;

    features::Active_Power_Closed_Loop_Control active_power_closed_loop;

    features::Reactive_Power_Closed_Loop_Control reactive_power_closed_loop;

    ////////////////////////////////////////////////////////////////////////
    //                         SITE OPERATION FEATURES                    //
    ////////////////////////////////////////////////////////////////////////
    //
    // feature selection
    //
    Fims_Object available_features_site_operation;
    uint64_t available_site_operation_features_mask;
    bool configure_available_site_operation_features_list();
    void configure_persistent_settings_pairs();
    std::vector<Feature*> site_operation_features_list = {
        &watchdog_feature,
    };

    features::Watchdog watchdog_feature;
    void dogbark();

    ////////////////////////////////////////////////////////////////////////
    //                              VARIABLE IDs                          //
    ////////////////////////////////////////////////////////////////////////
    std::vector<std::pair<Fims_Object*, std::string>> variable_ids = {
        { &input_source_status, "input_source_status" },
        { &enable_flag, "enable_flag" },
        { &disable_flag, "disable_flag" },
        { &standby_flag, "standby_flag" },
        { &clear_faults_flag, "clear_faults_flag" },
        { &active_faults, "active_faults" },
        { &active_alarms, "active_alarms" },
        { &site_state, "site_state" },
        { &site_state_enum, "site_state_enum" },
        { &site_status, "site_status" },
        { &running_status_flag, "running_status_flag" },
        { &alarm_status_flag, "alarm_status_flag" },
        { &fault_status_flag, "fault_status_flag" },
        { &reserved_bool_1, "reserved_bool_1" },
        { &reserved_bool_2, "reserved_bool_2" },
        { &reserved_bool_3, "reserved_bool_3" },
        { &reserved_bool_4, "reserved_bool_4" },
        { &reserved_bool_5, "reserved_bool_5" },
        { &reserved_bool_6, "reserved_bool_6" },
        { &reserved_bool_7, "reserved_bool_7" },
        { &reserved_bool_8, "reserved_bool_8" },
        { &reserved_bool_9, "reserved_bool_9" },
        { &reserved_bool_10, "reserved_bool_10" },
        { &reserved_bool_11, "reserved_bool_11" },
        { &reserved_bool_12, "reserved_bool_12" },
        { &reserved_bool_13, "reserved_bool_13" },
        { &reserved_bool_14, "reserved_bool_14" },
        { &reserved_bool_15, "reserved_bool_15" },
        { &reserved_bool_16, "reserved_bool_16" },
        { &reserved_float_1, "reserved_float_1" },
        { &reserved_float_2, "reserved_float_2" },
        { &reserved_float_3, "reserved_float_3" },
        { &reserved_float_4, "reserved_float_4" },
        { &reserved_float_5, "reserved_float_5" },
        { &reserved_float_6, "reserved_float_6" },
        { &reserved_float_7, "reserved_float_7" },
        { &reserved_float_8, "reserved_float_8" },
        { &ess_kVAR_slew_rate, "ess_kVAR_slew_rate" },
        { &gen_kVAR_slew_rate, "gen_kVAR_slew_rate" },
        { &solar_kVAR_slew_rate, "solar_kVAR_slew_rate" },
        { &power_priority_flag, "power_priority_flag" },
        { &invert_poi_kW, "invert_poi_kW" },
        { &ess_instant_discharge, "ess_instant_discharge" },
        { &ess_instant_charge_grid, "ess_instant_charge_grid" },
        { &ess_instant_charge_pv, "ess_instant_charge_pv" },
        { &faults, "faults" },
        { &alarms, "alarms" },
        { &exit_timer, "exit_timer" },
        { &asset_priority_runmode1, "asset_priority_runmode1" },
        { &asset_priority_runmode2, "asset_priority_runmode2" },
        { &start_first_gen_kW, "start_first_gen_kW" },
        { &allow_gen_auto_restart, "allow_gen_auto_restart" },
        { &start_first_ess_kW, "start_first_ess_kW" },
        { &allow_ess_auto_restart, "allow_ess_auto_restart" },
        { &start_first_solar_kW, "start_first_solar_kW" },
        { &allow_solar_auto_restart, "allow_solar_auto_restart" },
        { &soc_min_all, "soc_min_all" },
        { &soc_max_all, "soc_max_all" },
        { &soc_avg_all, "soc_avg_all" },
        { &soc_min_running, "soc_min_running" },
        { &soc_max_running, "soc_max_running" },
        { &soc_avg_running, "soc_avg_running" },
        { &available_features_standalone_power, "available_features_standalone_power" },
        { &available_features_site_operation, "available_features_site_operation" },
        { &available_features_runmode2_kW_mode, "available_features_runmode2_kW_mode" },
        { &runmode2_kW_mode_cmd, "runmode2_kW_mode_cmd" },
        { &runmode2_kW_mode_status, "runmode2_kW_mode_status" },
        { &available_features_runmode2_kVAR_mode, "available_features_runmode2_kVAR_mode" },
        { &runmode2_kVAR_mode_cmd, "runmode2_kVAR_mode_cmd" },
        { &runmode2_kVAR_mode_status, "runmode2_kVAR_mode_status" },
        { &configured_primary, "configured_primary" },
        { &ess_kW_cmd, "ess_kW_cmd" },
        { &gen_kW_cmd, "gen_kW_cmd" },
        { &solar_kW_cmd, "solar_kW_cmd" },
        { &feeder_kW_cmd, "feeder_kW_cmd" },
        { &site_kW_demand, "site_kW_demand" },
        { &site_kW_load, "site_kW_load" },
        { &site_kW_load_inclusion, "site_kW_load_inclusion" },
        { &site_kW_load_interval_ms, "site_kW_load_interval_ms" },
        { &site_frequency, "site_frequency" },
        { &ess_actual_kW, "ess_actual_kW" },
        { &gen_actual_kW, "gen_actual_kW" },
        { &solar_actual_kW, "solar_actual_kW" },
        { &feeder_actual_kW, "feeder_actual_kW" },
        { &max_potential_ess_kW, "max_potential_ess_kW" },
        { &min_potential_ess_kW, "min_potential_ess_kW" },
        { &rated_ess_kW, "rated_ess_kW" },
        { &max_potential_gen_kW, "max_potential_gen_kW" },
        { &min_potential_gen_kW, "min_potential_gen_kW" },
        { &rated_gen_kW, "rated_gen_kW" },
        { &max_potential_solar_kW, "max_potential_solar_kW" },
        { &min_potential_solar_kW, "min_potential_solar_kW" },
        { &rated_solar_kW, "rated_solar_kW" },
        { &max_potential_feeder_kW, "max_potential_feeder_kW" },
        { &min_potential_feeder_kW, "min_potential_feeder_kW" },
        { &rated_feeder_kW, "rated_feeder_kW" },
        { &available_features_runmode1_kW_mode, "available_features_runmode1_kW_mode" },
        { &runmode1_kW_mode_cmd, "runmode1_kW_mode_cmd" },
        { &runmode1_kW_mode_status, "runmode1_kW_mode_status" },
        { &feature_kW_demand, "feature_kW_demand" },
        { &site_kW_charge_production, "site_kW_charge_production" },
        { &site_kW_discharge_production, "site_kW_discharge_production" },
        { &total_site_kW_rated_charge, "total_site_kW_rated_charge" },
        { &total_site_kW_rated_discharge, "total_site_kW_rated_discharge" },
        { &total_site_kW_charge_limit, "total_site_kW_charge_limit" },
        { &total_site_kW_discharge_limit, "total_site_kW_discharge_limit" },
        { &ess_kVAR_cmd, "ess_kVAR_cmd" },
        { &gen_kVAR_cmd, "gen_kVAR_cmd" },
        { &solar_kVAR_cmd, "solar_kVAR_cmd" },
        { &site_kVAR_demand, "site_kVAR_demand" },
        { &ess_actual_kVAR, "ess_actual_kVAR" },
        { &gen_actual_kVAR, "gen_actual_kVAR" },
        { &solar_actual_kVAR, "solar_actual_kVAR" },
        { &feeder_actual_kVAR, "feeder_actual_kVAR" },
        { &feeder_actual_pf, "feeder_actual_pf" },
        { &potential_ess_kVAR, "potential_ess_kVAR" },
        { &potential_gen_kVAR, "potential_gen_kVAR" },
        { &potential_solar_kVAR, "potential_solar_kVAR" },
        { &available_features_runmode1_kVAR_mode, "available_features_runmode1_kVAR_mode" },
        { &runmode1_kVAR_mode_cmd, "runmode1_kVAR_mode_cmd" },
        { &runmode1_kVAR_mode_status, "runmode1_kVAR_mode_status" },
        { &start_first_gen_soc, "start_first_gen_soc" },
    };
};

#endif /* SITEMANAGER_H_ */
