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
#include <memory>
/* External Dependencies */
#include <spdlog/fmt/fmt.h>
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Asset_Manager.h>
#include <Value_Object.h>
#include <Input_Sources.h>
#include <Slew_Object.h>
#include <Variable_Regulator.h>
#include <Frequency_Response.h>
#include <Asset_Cmd_Object.h>
#include <Fims_Object.h>
#include <Energy_Arbitrage.h>
#include <macros.h>
#include <version.h>

class Sequence;

struct Feature {
    Feature();
    std::vector<Fims_Object*> feature_vars; // all variables associated with this feature
    std::vector<Fims_Object*> summary_vars; // only variables that are desired to be displayed in summary
    Fims_Object* enable_flag;               // true if this feature is currently being executed in algorithms
    bool available;                         // true if this feature is available to the customer to turn on
    void add_feature_vars_to_JSON_buffer(fmt::memory_buffer &buf, const char* const var = NULL);
    void add_summary_vars_to_JSON_buffer(fmt::memory_buffer &buf, const char* const var = NULL);
    void toggle_ui_enabled(bool);
};

class Site_Manager
{
    ////////////////////////////////////////////////////////////////////////////////////////
    //                              SITE MANAGER CONFIGURATION                            //
    ////////////////////////////////////////////////////////////////////////////////////////
public:
    Site_Manager(Version* release_version);
    virtual ~Site_Manager();
    bool configure(Asset_Manager* man, fims* fim, cJSON* sequenceRoot, cJSON* varRoot, bool* primary_controller);
protected:
    bool parse_variables(cJSON* object);
    void parse_default_vals(cJSON* JSON_defaults, Fims_Object &default_vals);
    void parse_flatten_vars(cJSON* JSON_object, cJSON* JSON_flat_vars);
    void configure_feature_objects();
    
    ////////////////////////////////////////////////////////////////////////////////////////
    //                              PUBLISHING & FIMS MANAGEMENT                          //
    ////////////////////////////////////////////////////////////////////////////////////////
public:
    void fims_data_parse(fims_message* msg);
    void publish_all_FIMS();
protected:
    void send_FIMS(const char* method, const char* uri, const char* replyto, const char* body);
    void ui_configuration(void);
    void build_JSON_site(fmt::memory_buffer &buf);
    void build_JSON_site_summary(fmt::memory_buffer &buf, bool clothed, const char* const var = NULL);
    void build_JSON_site_operation(fmt::memory_buffer &buf, const char* const var = NULL);
    void build_JSON_site_configuration(fmt::memory_buffer &buf, const char* const var = NULL);
    void build_JSON_site_cops(fmt::memory_buffer &buf, const char* const var = NULL);
    void build_JSON_site_input_sources(fmt::memory_buffer &buf, const char* const var = NULL);
    void build_JSON_features(fmt::memory_buffer &buf);
    void build_JSON_features_summary(fmt::memory_buffer &buf, bool clothed, const char* const var = NULL);
    void build_JSON_features_active_power(fmt::memory_buffer &buf, const char* const var = NULL);
    void build_JSON_features_reactive_power(fmt::memory_buffer &buf, const char* const var = NULL);
    void build_JSON_features_standalone_power(fmt::memory_buffer &buf, const char* const var = NULL);
    void build_JSON_features_site_operation(fmt::memory_buffer &buf, const char* const var = NULL);
    bool* is_primary; // Indicates whether this is the primary controller (true) or running in shadow mode (false)
    fims* pFims;
    fmt::memory_buffer send_FIMS_buf; // Reusable and resizable string buffer used by the send_FIMS function

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
    bool call_sequence_functions(const char* target_asset, const char* asset_cmd, Value_Object* value, int tolerance_percent);
    void set_site_status(const char*);
    timespec current_time, exit_target_time, debounce_target_time;
    states current_state, check_current_state;
    bool step_change, path_change; //true whenever a step or path changes
    bool sequence_reset;  //indication that current sequence has changed. used to handle interrupted sequences
    bool get_reserved_bool_1();
    bool get_reserved_bool_2();
    bool get_reserved_bool_3();
    bool get_reserved_bool_4();
    bool get_reserved_bool_5();
    bool get_reserved_bool_6();
    bool get_reserved_bool_7();
    bool get_reserved_bool_8();
    bool get_reserved_bool_9();
    bool get_reserved_bool_10();
    bool get_reserved_bool_11();
    bool get_reserved_bool_12();
    bool get_reserved_bool_13();
    bool get_reserved_bool_14();
    bool get_reserved_bool_15();
    bool get_reserved_bool_16();

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
    Sequence* sequences[NUM_STATES];

    ////////////////////////////////////////////////////////////////////////////////////////
    //                                  SITE CONFIGURATION                                //
    ////////////////////////////////////////////////////////////////////////////////////////
public:
    Input_Source_List input_sources;
protected:
    std::vector<Fims_Object*> multi_input_command_vars;
    Fims_Object input_source_status;
    Fims_Object power_priority_flag; //false for active power priority, true for reactive power priority
    Fims_Object invert_poi_kW; // if the on-site poi reports power with a polarity reverse of expected, this flag can signal Site Manager to flip the polarity

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
    Fims_Object site_state, site_status;
    Fims_Object running_status_flag, alarm_status_flag, fault_status_flag;
    Fims_Object exit_timer;  //timeout for failed sequence step in ms
    Fims_Object asset_priority_runmode1, asset_priority_runmode2; // Enumeratred integer indiciating the priority of the assets in meeting the demand
                                                                  // See Types.h "priority" enum for the list of available values
    Fims_Object site_kW_demand, site_kW_load;  //site total kW demand and load calculation
    Fims_Object site_kW_load_interval_ms; // Duration of site load to track
    Fims_Object site_kW_load_inclusion; // Whether the feature includes site load
    Fims_Object site_kVAR_demand;  //total site kVAR demand (from reactive power features)
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
    Fims_Object ess_kVAR_slew_rate; // kVAR/s
    Fims_Object start_first_ess_kW;  //kW thresholds to check for issuing start asset request to asset manager
    Fims_Object ess_kW_cmd, ess_kVAR_cmd;
    int num_ess_available, num_ess_running, num_ess_controllable;
    float prev_ess_kW_cmd, prev_ess_kVAR_cmd;
    bool standby_ess_latch; //boolean latch to prevent multiple standby function calls
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
    Fims_Object gen_kVAR_slew_rate; // kVAR/s
    Fims_Object start_first_gen_kW;  //kW thresholds to check for issuing start asset request to asset manager
    Fims_Object start_first_gen_soc; //min soc to start first generator
    Fims_Object gen_kW_cmd, gen_kVAR_cmd;
    int num_gen_available, num_gen_running, num_gen_active;
    float prev_gen_kW_cmd, prev_gen_kVAR_cmd;
    Slew_Object gen_kVAR_cmd_slew;
    //
    // Solar
    //
    Fims_Object max_potential_solar_kW, min_potential_solar_kW, rated_solar_kW, potential_solar_kVAR;
    Fims_Object solar_actual_kW, solar_actual_kVAR;
    Fims_Object solar_kVAR_slew_rate; // kVAR/s
    Fims_Object start_first_solar_kW;  //kW thresholds to check for issuing start asset request to asset manager
    Fims_Object solar_kW_cmd, solar_kVAR_cmd;
    int num_solar_available, num_solar_running;
    float prev_solar_kW_cmd, prev_solar_kVAR_cmd;
    bool standby_solar_latch; //boolean latch to prevent multiple standby function calls
    Slew_Object solar_kVAR_cmd_slew;

    ////////////////////////////////////////////////////////////////////////////////////////
    //                                         FEATURES                                   //
    ////////////////////////////////////////////////////////////////////////////////////////
public:
    Asset_Cmd_Object asset_cmd;
protected:
    void update_power_feature_selections();
    void read_site_feature_controls();
    void ui_disable_feature_group_feature_vars(std::vector<Feature*> &feature_list);
    void add_feature_group_feature_vars_to_JSON_buffer(const std::vector<Feature*> &feature_list, fmt::memory_buffer &buf, const char* const var = NULL);
    void add_feature_group_summary_vars_to_JSON_buffer(const std::vector<Feature*> &feature_list, fmt::memory_buffer &buf, const char* const var = NULL);

    ////////////////////////////////////////////////////////////////////////////////////////
    //                            RUN MODE 1 ACTIVE POWER FEATURES                        //
    ////////////////////////////////////////////////////////////////////////////////////////
    //
    // Feature selection
    //
    void process_runmode1_kW_feature();
    bool configure_runmode1_kW_features();
    void set_asset_cmd_variables();
    void set_volatile_asset_cmd_variables();
    void calculate_total_site_kW_limits();
    void remove_active_poi_corrections_from_slew_targets();
    Fims_Object available_features_runmode1_kW_mode;
    Fims_Object runmode1_kW_mode_cmd;
    Fims_Object runmode1_kW_mode_status;
    int current_runmode1_kW_feature; // follows the Fims_Object runmode1_kW_mode_cmd to identify when it gets changed
    uint64_t available_runmode1_kW_features_mask;
    uint64_t runmode1_kW_features_charge_control_mask = 2; // Mask indicating which active power features utilize charge control
    std::vector<Feature*> runmode1_kW_features_list;
    float prev_active_power_feature_cmd;
    Fims_Object feature_kW_demand;
    Fims_Object site_kW_charge_production;      // Charge up to this amount during dispatch
    Fims_Object site_kW_discharge_production;   // Discharge up to this amount during dispatch
    Fims_Object total_site_kW_rated_charge;     // (Controllable) Rated active power charge of the site
    Fims_Object total_site_kW_rated_discharge;  // (Controllable) Rated active power discharge of the site
    float total_asset_kW_charge_limit;          // Minimum total charge production allowed by assets
    float total_asset_kW_discharge_limit;       // Maximum total discharge production allowed by assets;
    Fims_Object total_site_kW_charge_limit;     // Minimum total charge production allowed by the site based on available power and feature limitations
    Fims_Object total_site_kW_discharge_limit;  // Maximum total discharge production allowed by the site based on available power and feature limitations
    //
    // Energy Arbitrage
    //
public:
    Energy_Arbitrage energy_arb_obj;
protected:
    Feature energy_arbitrage_feature;
    void energy_arbitrage_helper();
    Fims_Object energy_arb_enable_flag;
    //
    // Target SOC Mode
    //
    Feature target_soc;
    Fims_Object target_soc_mode_enable_flag;
    Fims_Object target_soc_load_enable_flag;
    //
    // Site Export Target Mode
    //
    Feature site_export_target;
public:
    Slew_Object export_target_kW_cmd_slew;
protected:
    Fims_Object export_target_mode_enable_flag;
    Fims_Object export_target_load_enable_flag;
    Fims_Object export_target_kW_cmd;
    Fims_Object export_target_kW_slew_rate;
    //
    // Manual Power Mode
    //
    Feature manual_power_mode;
    Fims_Object manual_mode_enable_flag;
    Fims_Object manual_solar_kW_cmd; //manual setpoint for solar KW
    Fims_Object manual_ess_kW_cmd; //manual setpoint for ess kW
    //
    // Grid Target Mode
    //
    Feature grid_target;
    Fims_Object grid_target_mode_enable_flag;
    Fims_Object grid_target_kW_cmd; //offset in grid target mode applied to load calculation to regulate grid to a value
    Fims_Object ess_charge_support_enable_flag; //this enables ESS to charge to support underloaded grid - only available in runmode1 (grid-tied)
    //
    // Frequency Response Mode
    //
protected:
    Frequency_Response frequency_response;
    Feature frequency_response_feature;
    void execute_frequency_response_feature();
    Fims_Object frequency_response_enabled; // boolean flag indicating if the Frequency Response active power mode is currently enabled
    //
    // Absolute Power Mode
    //
    Feature absolute_ess;
    Fims_Object absolute_ess_kW_cmd;
    Fims_Object absolute_ess_enable_flag;
    Fims_Object absolute_ess_direction_flag; // Direction of the charge command
    //
    // ESS Calibration Mode
    //
public:
    Feature ess_calibration;
    void get_ess_calibration_variables();
    void set_ess_calibration_variables();
    Fims_Object ess_calibration_enable_flag;
    Fims_Object ess_calibration_kW_cmd;
    Fims_Object ess_calibration_soc_limits_enable;
    Fims_Object ess_calibration_min_soc_limit;
    Fims_Object ess_calibration_max_soc_limit;
    Fims_Object ess_calibration_voltage_limits_enable;
    Fims_Object ess_calibration_min_voltage_limit;
    Fims_Object ess_calibration_max_voltage_limit;
    Fims_Object ess_calibration_num_setpoint;
    Fims_Object ess_calibration_num_limited;
    Fims_Object ess_calibration_num_zero;

    ////////////////////////////////////////////////////////////////////////////////////
    //                          RUN MODE 2 ACTIVE POWER FEATURES                      //
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Feature selection
    //
    void process_runmode2_kW_feature();
    bool configure_runmode2_kW_features();
    Fims_Object available_features_runmode2_kW_mode;
    Fims_Object runmode2_kW_mode_cmd;
    Fims_Object runmode2_kW_mode_status;
    int current_runmode2_kW_feature; // follows the Fims_Object runmode2_kW_mode_cmd to identify when it gets changed
    uint64_t available_runmode2_kW_features_mask;
    std::vector<Feature*> runmode2_kW_features_list;
    //
    // No islanded/runmode2 active power features yet
    //
    
    ///////////////////////////////////////////////////////////////////
    //                          CHARGE FEATURES                      //
    ///////////////////////////////////////////////////////////////////
    std::vector<Feature*> charge_features_list; // Separate list for charge dispatch and charge control allowing them to be enabled in parallel
    //
    // Charge Dispatch - Always enabled 
    //
    Feature charge_dispatch;
    Fims_Object _charge_dispatch_enable_flag;
    Fims_Object charge_dispatch_kW_command; //actual kW output from charge control algorithm
    Fims_Object charge_dispatch_solar_enable_flag; //when true, use solar as a source for ESS charge
    Fims_Object charge_dispatch_gen_enable_flag; //when true, use gen as a source for ESS charge
    Fims_Object charge_dispatch_feeder_enable_flag;  //when true, use the feeder as a source for ESS charge
    //
    // Charge Control - Enabled based on active power feature charge_control mask
    //
    void check_charge_control(void);
    Feature charge_control;
    Fims_Object _ess_charge_control_enable_flag;
    Fims_Object ess_charge_control_kW_request; //charge kW request from control algorithm
    Fims_Object ess_charge_control_target_soc;  //control for target state of charge in charge control algorithm
    Fims_Object ess_charge_control_kW_limit; //kW limit for ESS, applied to both charge and discharge
    Fims_Object ess_charge_control_charge_disable;
    Fims_Object ess_charge_control_discharge_disable;

    ////////////////////////////////////////////////////////////////////////////////////////
    //                           RUN MODE 1 REACTIVE POWER FEATURES                       //
    ////////////////////////////////////////////////////////////////////////////////////////
    //
    // Feature selection
    //
    void process_runmode1_kVAR_feature();
    bool configure_runmode1_kVAR_features();
    void remove_reactive_poi_corrections_from_slew_targets();
    Fims_Object available_features_runmode1_kVAR_mode;
    // TODO: make these Fims Objects so they can be configured and exposed on fims
    float total_site_kVAR_rated_charge;     // (Controllable) Rated reactive power charge of the site
    float total_site_kVAR_rated_discharge;  // (Controllable) Rated reactive power discharge of the site
    Fims_Object runmode1_kVAR_mode_cmd;
    Fims_Object runmode1_kVAR_mode_status;
    int current_runmode1_kVAR_feature; // follows the Fims_Object runmode1_kVAR_mode_cmd to identify when it gets changed
    uint64_t available_runmode1_kVAR_features_mask;
    std::vector<Feature*> runmode1_kVAR_features_list;
    float prev_reactive_power_feature_cmd; // Previous kVAR demand set by the reactive power feature, used to track how much the demand is changing
    //
    // Watt-VAR Mode
    //
    Feature watt_var;
public:
    void watt_var_mode(float active_power, std::vector<std::pair<float,float>>& watt_var_curve);
    std::vector<std::pair<float,float>> watt_var_curve;
protected:
    Fims_Object watt_var_mode_enable_flag;
    Fims_Object watt_var_points;
    float watt_var_correction; // Internal variable tracking the amount of correction applied by watt_var
    //
    // Reactive Power Setpoint Mode
    //
    Feature reactive_setpoint;
    Slew_Object reactive_setpoint_kVAR_cmd_slew;
    Fims_Object reactive_setpoint_mode_enable_flag;
    Fims_Object reactive_setpoint_kVAR_cmd;
    Fims_Object reactive_setpoint_kVAR_slew_rate;
    //
    // Power Factor Mode
    //
    Feature power_factor;
    float prev_asset_pf_cmd;
    bool asset_pf_flag, prev_asset_pf_flag; //track the change to/from any mode that uses power factor cmds to enable assets to follow them
    Fims_Object power_factor_mode_enable_flag;
    Fims_Object power_factor_cmd;  //pf-setpoint
    //
    // Constant Power Factor Mode
    //
    Feature constant_power_factor;
    void execute_constant_power_factor();                   // Feature function
    Fims_Object constant_power_factor_mode_enable_flag;     // Enable flag
    Fims_Object constant_power_factor_setpoint;             // Power Factor setpoint. Sign is used if bidirectional mode, only magnitude in absolute mode
    Fims_Object constant_power_factor_lagging_limit;        // -1.0 < pf < lagging_limit for negative commands
    Fims_Object constant_power_factor_leading_limit;        //  1.0 > pf > leading_limit for positive commands
    Fims_Object constant_power_factor_absolute_mode;        // Absolute value mode, with sign determined by the direction flag:
    Fims_Object constant_power_factor_lagging_direction;    // Direction flag. True is negative aka lagging, and false is positive aka leading
    //
    // Active Voltage Mode
    //
    Feature active_voltage;
    Fims_Object active_voltage_mode_enable_flag;
    Fims_Object active_voltage_deadband;  //deadband within which algorithm wont respond
    Fims_Object active_voltage_droop_percent;  //percent of rated to apply per v deviation
    Fims_Object active_voltage_cmd; //nominal voltage target
    Fims_Object active_voltage_actual_volts;  // actual voltage to calculated deviation with
    Fims_Object active_voltage_status_flag;  //true flag when response is nonzero
    Fims_Object active_voltage_rated_kVAR;  //rated kVAR for calculating response kVAR

    ///////////////////////////////////////////////////////////////////////////////////
    //                       RUN MODE 2 REACTIVE POWER FEATURES                      //
    ///////////////////////////////////////////////////////////////////////////////////
    //
    // Feature selection
    //
    bool configure_runmode2_kVAR_features();
    Fims_Object available_features_runmode2_kVAR_mode;
    Fims_Object runmode2_kVAR_mode_cmd;
    Fims_Object runmode2_kVAR_mode_status;
    int current_runmode2_kVAR_feature; // follows the Fims_Object runmode2_kVAR_mode_cmd to identify when it gets changed
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
    bool configure_standalone_power_features();
    std::vector<Feature*> standalone_power_features_list;
    //
    // POI Limits
    // When on, site demand and ess charge/discharge requests will be limited so POI does not exceed its rated limit
    //
    Feature active_power_poi_limits;
    void apply_active_power_poi_limits();
    Fims_Object active_power_poi_limits_enable;
    Fims_Object active_power_poi_limits_max_kW; //max POI kW value allowed when POI Limits enabled
    Fims_Object active_power_poi_limits_min_kW; //min POI kW value allowed when POI Limits enabled
    Fims_Object active_power_soc_poi_limits_enable; //ESS specific, soc-based POI limits
    Fims_Object active_power_soc_poi_target_soc; //threshold below which "under" limits are applied and above which "over" limits are applied
    Fims_Object active_power_soc_poi_limits_low_min_kW; //min POI limit when soc <= soc_poi_target_soc
    Fims_Object active_power_soc_poi_limits_low_max_kW; //max POI limit when soc <= soc_poi_target_soc
    Fims_Object active_power_soc_poi_limits_high_min_kW; //min POI limit when soc <= soc_poi_target_soc
    Fims_Object active_power_soc_poi_limits_high_max_kW; //max POI limit when soc <= soc_poi_target_soc

    //
    // Reactive Power POI Limits
    // When on, site kVAR demand will be limited so POI does not exceed its rated limit
    //
    Feature reactive_power_poi_limits;
    void apply_reactive_power_poi_limits();
    Fims_Object reactive_power_poi_limits_enable;
    Fims_Object reactive_power_poi_limits_min_kVAR; //min POI kVAR value allowed when POI Limits enabled
    Fims_Object reactive_power_poi_limits_max_kVAR; //max POI kVAR value allowed when POI Limits enabled

    //
    // PFR
    //
    Feature pfr;
    void primary_frequency_response(float kW_cmd);
    Fims_Object pfr_enable_flag;
    Fims_Object pfr_deadband; //deadband in Hz for pfr algorithm
    Fims_Object pfr_droop_percent; //droop percentage for pfr algorithm
    Fims_Object pfr_status_flag; //boolean indicates if pfr is actively changing system power output
    Fims_Object pfr_nameplate_kW; //maximum power output for PFR algorithm
    Fims_Object pfr_limits_min_kW; //min PFR kW scaler used during a frequency event
    Fims_Object pfr_limits_max_kW; //max PFR kW scaler used during a frequency event
    Fims_Object pfr_site_nominal_hz; //frequency value that site should be at in ideal conditions
    Fims_Object pfr_offset_hz; //test input that gets added to site frequency to simulate frequency deviation events
    //
    // Watt-Watt
    //
    Feature watt_watt;
    void watt_watt_poi_adjustment(void);
    std::vector<std::pair<float,float>> watt_watt_curve;
    Fims_Object watt_watt_adjustment_enable_flag;
    Fims_Object watt_watt_points;
    float watt_watt_correction; // Internal variable tracking the amount of correction applied by watt_watt
    //
    // LDSS
    //
    Feature ldss;
    void set_ldss_variables();
    Fims_Object ldss_enable_flag;
    Fims_Object ldss_priority_setting; //static or dynamic priorities
    Fims_Object ldss_max_load_threshold_percent, ldss_min_load_threshold_percent;
    Fims_Object ldss_warmup_time, ldss_cooldown_time, ldss_start_gen_time, ldss_stop_gen_time; // LDSS timers
    //
    // Load Shed
    //
    Feature load_shed;
    void calculate_load_shed();
    Variable_Regulator load_shed_calculator; // Used to hold and calculate the load shed value
    Fims_Object load_shed_enable; 
    Fims_Object load_shed_value; // Contains write uri for load shedder
    Fims_Object load_shed_max_value; // Max load shed value
    Fims_Object load_shed_min_value; // Min load shed value
    Fims_Object load_shed_max_shedding_threshold; // If SOC falls below this value, set the load shed value to max
    Fims_Object load_shed_high_threshold; // Threshold on ess (dischargeable power - measured power) to decrease shed value
    Fims_Object load_shed_decrease_timer_ms; // Time above threshold before decreasing load shed value
    Fims_Object load_shed_low_threshold; // Threshold on ess (dischargeable power - measured power) to increase shed value
    Fims_Object load_shed_increase_timer_ms; // Time below threshold before increasing load shed value
    Fims_Object load_shed_increase_display_timer; // load_shed_increase_timer_ms converted to a MIN:SEC display string
    Fims_Object load_shed_decrease_display_timer; // load_shed_decrease_timer_ms converted to a MIN:SEC display string
    Fims_Object load_shed_spare_ess_kw; // ess dischargeable kw - ess current discharging kw, AKA how much more discharge power the ess can handle
    //
    // Solar Shed
    //
    Feature solar_shed;
    void calculate_solar_shed();
    Variable_Regulator solar_shed_calculator; // Used to hold and calculate the solar shed value
    Fims_Object solar_shed_enable; 
    Fims_Object solar_shed_value; // Current solar shed value
    Fims_Object solar_shed_max_value; // Max solar shed value
    Fims_Object solar_shed_min_value; // Min solar shed value
    Fims_Object solar_shed_max_shedding_threshold; // If SOC rises above this value, set the solar shed value to max
    Fims_Object solar_shed_high_threshold; // Threshold on ess (chargeable power - measured charging power) to decrease shed value
    Fims_Object solar_shed_decrease_timer_ms; // Time above threshold before decreasing solar shed value
    Fims_Object solar_shed_low_threshold; // Threshold on ess (chargeable power - measured charging power) to increase shed value
    Fims_Object solar_shed_increase_timer_ms; // Time below threshold before increasing solar shed value
    Fims_Object solar_shed_increase_display_timer; // solar_shed_increase_timer_ms converted to a MIN:SEC display string
    Fims_Object solar_shed_decrease_display_timer; // solar_shed_decrease_timer_ms converted to a MIN:SEC display string
    Fims_Object solar_shed_spare_ess_kw; // ess chargeable kw - ess current charging kw, AKA how much more charge power the ess can handle
    //
    // ESS Discharge Prevention
    //
    Feature ess_discharge_prevention;
    void prevent_ess_discharge();
    Fims_Object edp_enable; // ess discharge prevention enable
    Fims_Object edp_soc; // target soc at or below which the ess will not be able to discharge
    //
    // Aggregated Asset Limit (solar-dependent ess discharge limit)
    //
    Feature agg_asset_limit;
    void apply_aggregated_asset_limit(float uncontrolled_ess_kw, float uncontrolled_solar_kw);
    Fims_Object agg_asset_limit_enable;
    Fims_Object agg_asset_limit_kw;
    //
    // Active Power Closed Loop Control
    //
    Feature active_power_closed_loop;
    void calculate_active_power_closed_loop_offset();
    Variable_Regulator active_power_closed_loop_regulator; // Used to hold and calculate the offset value
    Fims_Object active_power_closed_loop_enable; 
    Fims_Object active_power_closed_loop_default_offset; // Default offset
    Fims_Object active_power_closed_loop_min_offset; // Min offset
    Fims_Object active_power_closed_loop_max_offset; // Max offset
    Fims_Object active_power_closed_loop_step_size_kW; // Step size (in kW) multiplied against offset to get the total correction to apply
    Fims_Object active_power_closed_loop_total_correction; // Total correction applied by active closed loop control
    Fims_Object active_power_closed_loop_steady_state_deadband_kW; // If difference in commands deviates beyond this value, reset to default value
    Fims_Object active_power_closed_loop_regulation_deadband_kW; // Deadband threshold based on feeder rated power (in kW) outside which POI values are inaccurate
    Fims_Object active_power_closed_loop_update_rate_ms; // Number of timer updates per second
    Fims_Object active_power_closed_loop_decrease_timer_ms; // Time above regulation deadband before decreasing offset value
    Fims_Object active_power_closed_loop_increase_timer_ms; // Time below regulation deadband before increasing offset value
    Fims_Object _active_power_closed_loop_manual_offset_kW; // Internal debug offset used only for testing
    //
    // Reactive Power Closed Loop Control
    //
    Feature reactive_power_closed_loop;
    void calculate_reactive_power_closed_loop_offset();
    Variable_Regulator reactive_power_closed_loop_regulator; // Used to hold and calculate the offset value
    Fims_Object reactive_power_closed_loop_enable; 
    Fims_Object reactive_power_closed_loop_default_offset; // Default offset
    Fims_Object reactive_power_closed_loop_min_offset; // Min offset
    Fims_Object reactive_power_closed_loop_max_offset; // Max offset
    Fims_Object reactive_power_closed_loop_step_size_kW; // Step size (in kW) multiplied against offset to get the total correction to apply
    Fims_Object reactive_power_closed_loop_total_correction; // Total correction applied by reactive closed loop control
    Fims_Object reactive_power_closed_loop_steady_state_deadband_kW; // If difference in commands deviates beyond this value, reset to default value
    Fims_Object reactive_power_closed_loop_regulation_deadband_kW; // Deadband threshold based on feeder rated power (in kW) outside which POI values are inaccurate
    Fims_Object reactive_power_closed_loop_update_rate_ms; // Number of timer updates per second
    Fims_Object reactive_power_closed_loop_decrease_timer_ms; // Time above regulation deadband before decreasing offset value
    Fims_Object reactive_power_closed_loop_increase_timer_ms; // Time below regulation deadband before increasing offset value

    ////////////////////////////////////////////////////////////////////////
    //                         SITE OPERATION FEATURES                    //
    ////////////////////////////////////////////////////////////////////////
    //
    // feature selection
    //
    Fims_Object available_features_site_operation;
    uint64_t available_site_operation_features_mask;
    bool configure_site_operation_features();
    void configure_persistent_settings_pairs();
    std::vector<Feature*> site_operation_features_list;
    //
    // Watchdog Feature
    //
    Feature watchdog_feature;
    void watchdog();
    void dogbark();
    int watchdog_old_pet;
    timespec heartbeat_timer, watchdog_timeout;
    Fims_Object watchdog_enable;
    Fims_Object watchdog_duration_ms;
    Fims_Object watchdog_pet;
    Fims_Object heartbeat_counter;
    Fims_Object heartbeat_duration_ms;

    ////////////////////////////////////////////////////////////////////////
    //                              VARIABLE IDs                          //
    ////////////////////////////////////////////////////////////////////////
    std::vector<std::pair<Fims_Object*, std::string>> variable_ids = {
        {&input_source_status, "input_source_status"},
        {&enable_flag, "enable_flag"},
        {&disable_flag, "disable_flag"},
        {&standby_flag, "standby_flag"},
        {&clear_faults_flag, "clear_faults_flag"},
        {&active_faults, "active_faults"},
        {&active_alarms, "active_alarms"},
        {&site_state, "site_state"},
        {&site_status, "site_status"},
        {&running_status_flag, "running_status_flag"},
        {&alarm_status_flag, "alarm_status_flag"},
        {&fault_status_flag, "fault_status_flag"},
        {&reserved_bool_1, "reserved_bool_1"},
        {&reserved_bool_2, "reserved_bool_2"},
        {&reserved_bool_3, "reserved_bool_3"},
        {&reserved_bool_4, "reserved_bool_4"},
        {&reserved_bool_5, "reserved_bool_5"},
        {&reserved_bool_6, "reserved_bool_6"},
        {&reserved_bool_7, "reserved_bool_7"},
        {&reserved_bool_8, "reserved_bool_8"},
        {&reserved_bool_9, "reserved_bool_9"},
        {&reserved_bool_10, "reserved_bool_10"},
        {&reserved_bool_11, "reserved_bool_11"},
        {&reserved_bool_12, "reserved_bool_12"},
        {&reserved_bool_13, "reserved_bool_13"},
        {&reserved_bool_14, "reserved_bool_14"},
        {&reserved_bool_15, "reserved_bool_15"},
        {&reserved_bool_16, "reserved_bool_16"},
        {&reserved_float_1, "reserved_float_1"},
        {&reserved_float_2, "reserved_float_2"},
        {&reserved_float_3, "reserved_float_3"},
        {&reserved_float_4, "reserved_float_4"},
        {&reserved_float_5, "reserved_float_5"},
        {&reserved_float_6, "reserved_float_6"},
        {&reserved_float_7, "reserved_float_7"},
        {&reserved_float_8, "reserved_float_8"},
        {&ess_kVAR_slew_rate, "ess_kVAR_slew_rate"},
        {&gen_kVAR_slew_rate, "gen_kVAR_slew_rate"},
        {&solar_kVAR_slew_rate, "solar_kVAR_slew_rate"},
        {&power_priority_flag, "power_priority_flag"},
        {&invert_poi_kW, "invert_poi_kW"},
        {&ess_instant_discharge, "ess_instant_discharge"},
        {&ess_instant_charge_grid, "ess_instant_charge_grid"},
        {&ess_instant_charge_pv, "ess_instant_charge_pv"},
        {&faults, "faults"},
        {&alarms, "alarms"},
        {&exit_timer, "exit_timer"},
        {&asset_priority_runmode1, "asset_priority_runmode1"},
        {&asset_priority_runmode2, "asset_priority_runmode2"},
        {&start_first_gen_kW, "start_first_gen_kW"},
        {&start_first_ess_kW, "start_first_ess_kW"},
        {&start_first_solar_kW, "start_first_ess_kW"},
        {&soc_min_all, "soc_min_all"},
        {&soc_max_all, "soc_max_all"},
        {&soc_avg_all, "soc_avg_all"},
        {&soc_min_running, "soc_min_running"},
        {&soc_max_running, "soc_max_running"},
        {&soc_avg_running, "soc_avg_running"},
        {&available_features_standalone_power, "available_features_standalone_power"},
        {&available_features_site_operation, "available_features_site_operation"},
        {&available_features_runmode2_kW_mode, "available_features_runmode2_kW_mode"},
        {&runmode2_kW_mode_cmd, "runmode2_kW_mode_cmd"},
        {&runmode2_kW_mode_status, "runmode2_kW_mode_status"},
        {&available_features_runmode2_kVAR_mode, "available_features_runmode2_kVAR_mode"},
        {&runmode2_kVAR_mode_cmd, "runmode2_kVAR_mode_cmd"},
        {&runmode2_kVAR_mode_status, "runmode2_kVAR_mode_status"},
        {&configured_primary, "configured_primary"},
        {&energy_arb_enable_flag, "energy_arb_enable_flag"},
        {&_charge_dispatch_enable_flag, "_charge_dispatch_enable_flag"},
        {&_ess_charge_control_enable_flag, "_ess_charge_control_enable_flag"},
        {&grid_target_mode_enable_flag, "grid_target_mode_enable_flag"},
        {&target_soc_mode_enable_flag, "target_soc_mode_enable_flag"},
        {&manual_mode_enable_flag, "manual_mode_enable_flag"},
        {&absolute_ess_enable_flag, "absolute_ess_enable_flag"},
        {&export_target_mode_enable_flag, "export_target_mode_enable_flag"},
        {&ess_calibration_enable_flag, "ess_calibration_enable_flag"},
        {&reactive_setpoint_mode_enable_flag, "reactive_setpoint_mode_enable_flag"},
        {&active_voltage_mode_enable_flag, "active_voltage_mode_enable_flag"},
        {&power_factor_mode_enable_flag, "power_factor_mode_enable_flag"},
        {&constant_power_factor_mode_enable_flag, "constant_power_factor_mode_enable_flag"},
        {&watt_var_mode_enable_flag, "watt_var_mode_enable_flag"},
        {&frequency_response_enabled, "fr_mode_enable_flag"},
        {&ess_kW_cmd, "ess_kW_cmd"},
        {&gen_kW_cmd, "gen_kW_cmd"},
        {&solar_kW_cmd, "solar_kW_cmd"},
        {&feeder_kW_cmd, "feeder_kW_cmd"},
        {&site_kW_demand, "site_kW_demand"},
        {&site_kW_load, "site_kW_load"},
        {&site_kW_load_inclusion, "site_kW_load_inclusion"},
        {&site_kW_load_interval_ms, "site_kW_load_interval_ms"},
        {&site_frequency, "site_frequency"},
        {&grid_target_kW_cmd, "grid_target_kW_cmd"},
        {&ess_actual_kW, "ess_actual_kW"},
        {&gen_actual_kW, "gen_actual_kW"},
        {&solar_actual_kW, "solar_actual_kW"},
        {&feeder_actual_kW, "feeder_actual_kW"},
        {&max_potential_ess_kW, "max_potential_ess_kW"},
        {&min_potential_ess_kW, "min_potential_ess_kW"},
        {&rated_ess_kW, "rated_ess_kW"},
        {&max_potential_gen_kW, "max_potential_gen_kW"},
        {&min_potential_gen_kW, "min_potential_gen_kW"},
        {&rated_gen_kW, "rated_gen_kW"},
        {&max_potential_solar_kW, "max_potential_solar_kW"},
        {&min_potential_solar_kW, "min_potential_solar_kW"},
        {&rated_solar_kW, "rated_solar_kW"},
        {&max_potential_feeder_kW, "max_potential_feeder_kW"},
        {&min_potential_feeder_kW, "min_potential_feeder_kW"},
        {&rated_feeder_kW, "rated_feeder_kW"},
        {&pfr_limits_min_kW, "pfr_limits_min_kW"},
        {&pfr_limits_max_kW, "pfr_limits_max_kW"},
        {&pfr_site_nominal_hz, "pfr_site_nominal_hz"},
        {&pfr_offset_hz, "pfr_offset_hz"},
        {&ess_charge_support_enable_flag, "ess_charge_support_enable_flag"},
        {&energy_arb_obj.price, "price"},
        {&energy_arb_obj.threshold_charge_1, "threshold_charge_1"},
        {&energy_arb_obj.threshold_charge_2, "threshold_charge_2"},
        {&energy_arb_obj.threshold_dischg_1, "threshold_dischg_1"},
        {&energy_arb_obj.threshold_dischg_2, "threshold_dischg_2"},
        {&energy_arb_obj.soc_min_limit, "soc_min_limit"},
        {&energy_arb_obj.soc_max_limit, "soc_max_limit"},
        {&energy_arb_obj.max_charge_1, "max_charge_1"},
        {&energy_arb_obj.max_charge_2, "max_charge_2"},
        {&energy_arb_obj.max_dischg_1, "max_dischg_1"},
        {&energy_arb_obj.max_dischg_2, "max_dischg_2"},
        {&charge_dispatch_kW_command, "charge_dispatch_kW_command"},
        {&charge_dispatch_gen_enable_flag, "charge_dispatch_gen_enable_flag"},
        {&charge_dispatch_solar_enable_flag, "charge_dispatch_solar_enable_flag"},
        {&charge_dispatch_feeder_enable_flag, "charge_dispatch_feeder_enable_flag"},
        {&ess_charge_control_kW_request, "ess_charge_control_kW_request"},
        {&ess_charge_control_kW_limit, "ess_charge_control_kW_limit"},
        {&ess_charge_control_target_soc, "ess_charge_control_target_soc"},
        {&ess_charge_control_charge_disable, "ess_charge_control_charge_disable"},
        {&ess_charge_control_discharge_disable, "ess_charge_control_discharge_disable"},
        {&target_soc_load_enable_flag, "target_soc_load_enable_flag"},
        {&manual_solar_kW_cmd, "manual_solar_kW_cmd"},
        {&manual_ess_kW_cmd, "manual_ess_kW_cmd"},
        {&absolute_ess_kW_cmd, "absolute_ess_kW_cmd"},
        {&absolute_ess_direction_flag, "absolute_ess_direction_flag"},
        {&export_target_kW_cmd, "export_target_kW_cmd"},
        {&export_target_load_enable_flag, "export_target_load_enable_flag"},
        {&export_target_kW_slew_rate, "export_target_kW_slew_rate"},
        {&ess_calibration_kW_cmd, "ess_calibration_kW_cmd"},
        {&ess_calibration_soc_limits_enable, "ess_calibration_soc_limits_enable"},
        {&ess_calibration_min_soc_limit, "ess_calibration_min_soc_limit"},
        {&ess_calibration_max_soc_limit, "ess_calibration_max_soc_limit"},
        {&ess_calibration_voltage_limits_enable, "ess_calibration_voltage_limits_enable"},
        {&ess_calibration_min_voltage_limit, "ess_calibration_min_voltage_limit"},
        {&ess_calibration_max_voltage_limit, "ess_calibration_max_voltage_limit"},
        {&ess_calibration_num_setpoint, "ess_calibration_num_setpoint"},
        {&ess_calibration_num_limited, "ess_calibration_num_limited"},
        {&ess_calibration_num_zero, "ess_calibration_num_zero"},
        {&available_features_runmode1_kW_mode, "available_features_runmode1_kW_mode"},
        {&runmode1_kW_mode_cmd, "runmode1_kW_mode_cmd"},
        {&runmode1_kW_mode_status, "runmode1_kW_mode_status"},
        {&feature_kW_demand, "feature_kW_demand"},
        {&site_kW_charge_production, "site_kW_charge_production"},
        {&site_kW_discharge_production, "site_kW_discharge_production"},
        {&total_site_kW_rated_charge, "total_site_kW_rated_charge"},
        {&total_site_kW_rated_discharge, "total_site_kW_rated_discharge"},
        {&total_site_kW_charge_limit, "total_site_kW_charge_limit"},
        {&total_site_kW_discharge_limit, "total_site_kW_discharge_limit"},
        {&ess_kVAR_cmd, "ess_kVAR_cmd"},
        {&gen_kVAR_cmd, "gen_kVAR_cmd"},
        {&solar_kVAR_cmd, "solar_kVAR_cmd"},
        {&site_kVAR_demand, "site_kVAR_demand"},
        {&ess_actual_kVAR, "ess_actual_kVAR"},
        {&gen_actual_kVAR, "gen_actual_kVAR"},
        {&solar_actual_kVAR, "solar_actual_kVAR"},
        {&feeder_actual_kVAR, "feeder_actual_kVAR"},
        {&feeder_actual_pf, "feeder_actual_pf"},
        {&potential_ess_kVAR, "potential_ess_kVAR"},
        {&potential_gen_kVAR, "potential_gen_kVAR"},
        {&potential_solar_kVAR, "potential_solar_kVAR"},
        {&available_features_runmode1_kVAR_mode, "available_features_runmode1_kVAR_mode"},
        {&runmode1_kVAR_mode_cmd, "runmode1_kVAR_mode_cmd"},
        {&runmode1_kVAR_mode_status, "runmode1_kVAR_mode_status"},
        {&reactive_setpoint_kVAR_cmd, "reactive_setpoint_kVAR_cmd"},
        {&reactive_setpoint_kVAR_slew_rate, "reactive_setpoint_kVAR_slew_rate"},
        {&active_voltage_deadband, "active_voltage_deadband"},
        {&active_voltage_droop_percent, "active_voltage_droop_percent"},
        {&active_voltage_cmd, "active_voltage_cmd"},
        {&active_voltage_actual_volts, "active_voltage_actual_volts"},
        {&active_voltage_status_flag, "active_voltage_status_flag"},
        {&active_voltage_rated_kVAR, "active_voltage_rated_kVAR"},
        {&power_factor_cmd, "power_factor_cmd"},
        {&constant_power_factor_setpoint, "constant_power_factor_setpoint"},
        {&constant_power_factor_lagging_limit, "constant_power_factor_lagging_limit"},
        {&constant_power_factor_leading_limit, "constant_power_factor_leading_limit"},
        {&constant_power_factor_absolute_mode, "constant_power_factor_absolute_mode"},
        {&constant_power_factor_lagging_direction, "constant_power_factor_lagging_direction"},
        {&watt_var_points, "watt_var_points"},
        {&start_first_gen_soc, "start_first_gen_soc"},
        {&pfr_enable_flag, "pfr_enable_flag"},
        {&pfr_deadband, "pfr_deadband"},
        {&pfr_droop_percent, "pfr_droop_percent"},
        {&pfr_status_flag, "pfr_status_flag"},
        {&pfr_nameplate_kW, "pfr_nameplate_kW"},
        {&watt_watt_adjustment_enable_flag, "watt_watt_adjustment_enable_flag"},
        {&watt_watt_points, "watt_watt_points"},
        {&active_power_poi_limits_enable, "active_power_poi_limits_enable"},
        {&active_power_poi_limits_max_kW, "active_power_poi_limits_max_kW"},
        {&active_power_poi_limits_min_kW, "active_power_poi_limits_min_kW"},
        {&active_power_soc_poi_limits_enable, "active_power_soc_poi_limits_enable"},
        {&active_power_soc_poi_target_soc, "active_power_soc_poi_target_soc"},
        {&active_power_soc_poi_limits_low_min_kW, "active_power_soc_poi_limits_low_min_kW"},
        {&active_power_soc_poi_limits_low_max_kW, "active_power_soc_poi_limits_low_max_kW"},
        {&active_power_soc_poi_limits_high_min_kW, "active_power_soc_poi_limits_high_min_kW"},
        {&active_power_soc_poi_limits_high_max_kW, "active_power_soc_poi_limits_high_max_kW"},
        {&reactive_power_poi_limits_enable, "reactive_power_poi_limits_enable"},
        {&reactive_power_poi_limits_min_kVAR, "reactive_power_poi_limits_min_kVAR"},
        {&reactive_power_poi_limits_max_kVAR, "reactive_power_poi_limits_max_kVAR"},
        {&ldss_enable_flag, "ldss_enable_flag"},
        {&ldss_priority_setting, "ldss_priority_setting"},
        {&ldss_start_gen_time, "ldss_start_gen_time"},
        {&ldss_stop_gen_time, "ldss_stop_gen_time"},
        {&ldss_max_load_threshold_percent, "ldss_max_load_threshold_percent"},
        {&ldss_min_load_threshold_percent, "ldss_min_load_threshold_percent"},
        {&ldss_warmup_time, "ldss_warmup_time"},
        {&ldss_cooldown_time, "ldss_cooldown_time"},
        {&load_shed_enable, "load_shed_enable"},
        {&load_shed_value, "load_shed_value"},
        {&load_shed_max_value, "load_shed_max_value"},        
        {&load_shed_min_value, "load_shed_min_value"},
        {&load_shed_max_shedding_threshold, "load_shed_max_shedding_threshold"},
        {&load_shed_high_threshold, "load_shed_high_threshold"},
        {&load_shed_decrease_timer_ms, "load_shed_decrease_timer_ms"},
        {&load_shed_low_threshold, "load_shed_low_threshold"},
        {&load_shed_increase_timer_ms, "load_shed_increase_timer_ms"},
        {&load_shed_increase_display_timer, "load_shed_increase_display_timer"},
        {&load_shed_decrease_display_timer, "load_shed_decrease_display_timer"},
        {&load_shed_spare_ess_kw, "load_shed_spare_ess_kw"},
        {&solar_shed_enable, "solar_shed_enable"},
        {&solar_shed_value, "solar_shed_value"},
        {&solar_shed_max_value, "solar_shed_max_value"},
        {&solar_shed_min_value, "solar_shed_min_value"},
        {&solar_shed_max_shedding_threshold, "solar_shed_max_shedding_threshold"},
        {&solar_shed_high_threshold, "solar_shed_high_threshold"},
        {&solar_shed_decrease_timer_ms, "solar_shed_decrease_timer_ms"},
        {&solar_shed_low_threshold, "solar_shed_low_threshold"},
        {&solar_shed_increase_timer_ms, "solar_shed_increase_timer_ms"},
        {&solar_shed_increase_display_timer, "solar_shed_increase_display_timer"},
        {&solar_shed_decrease_display_timer, "solar_shed_decrease_display_timer"},
        {&solar_shed_spare_ess_kw, "solar_shed_spare_ess_kw"},
        {&active_power_closed_loop_enable, "active_power_closed_loop_enable"},
        {&active_power_closed_loop_default_offset, "active_power_closed_loop_default_offset"},
        {&active_power_closed_loop_min_offset, "active_power_closed_loop_min_offset"},
        {&active_power_closed_loop_max_offset, "active_power_closed_loop_max_offset"},
        {&active_power_closed_loop_step_size_kW, "active_power_closed_loop_step_size_kW"},
        {&active_power_closed_loop_total_correction, "active_power_closed_loop_total_correction"},
        {&active_power_closed_loop_steady_state_deadband_kW, "active_power_closed_loop_steady_state_deadband_kW"},
        {&active_power_closed_loop_regulation_deadband_kW, "active_power_closed_loop_regulation_deadband_kW"},
        {&active_power_closed_loop_update_rate_ms, "active_power_closed_loop_update_rate_ms"},
        {&active_power_closed_loop_decrease_timer_ms, "active_power_closed_loop_decrease_timer_ms"},
        {&active_power_closed_loop_increase_timer_ms, "active_power_closed_loop_increase_timer_ms"},
        {&_active_power_closed_loop_manual_offset_kW, "_active_power_closed_loop_manual_offset_kW"},
        {&reactive_power_closed_loop_enable, "reactive_power_closed_loop_enable"},
        {&reactive_power_closed_loop_default_offset, "reactive_power_closed_loop_default_offset"},
        {&reactive_power_closed_loop_min_offset, "reactive_power_closed_loop_min_offset"},
        {&reactive_power_closed_loop_max_offset, "reactive_power_closed_loop_max_offset"},
        {&reactive_power_closed_loop_step_size_kW, "reactive_power_closed_loop_step_size_kW"},
        {&reactive_power_closed_loop_total_correction, "reactive_power_closed_loop_total_correction"},
        {&reactive_power_closed_loop_steady_state_deadband_kW, "reactive_power_closed_loop_steady_state_deadband_kW"},
        {&reactive_power_closed_loop_regulation_deadband_kW, "reactive_power_closed_loop_regulation_deadband_kW"},
        {&reactive_power_closed_loop_update_rate_ms, "reactive_power_closed_loop_update_rate_ms"},
        {&reactive_power_closed_loop_decrease_timer_ms, "reactive_power_closed_loop_decrease_timer_ms"},
        {&reactive_power_closed_loop_increase_timer_ms, "reactive_power_closed_loop_increase_timer_ms"},
        {&edp_enable, "ess_discharge_prevention_enable"},
        {&edp_soc, "ess_discharge_prevention_soc"},
        {&agg_asset_limit_enable, "agg_asset_limit_enable"},
        {&agg_asset_limit_kw, "agg_asset_limit_kw"},
        {&watchdog_enable, "watchdog_enable"},
        {&watchdog_duration_ms, "watchdog_duration_ms"},
        {&watchdog_pet, "watchdog_pet"},
        {&heartbeat_counter, "heartbeat_counter"},
        {&heartbeat_duration_ms, "heartbeat_duration_ms"}
    };
};

#endif /* SITEMANAGER_H_ */

