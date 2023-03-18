Site Manager - variables.json Documentation

Author: Kyle Brezina

Date Created: 03/14/19

Date Modified: 05/18/21


# example variables.json file:

<pre><code>
{
	"variables":
	{
		"example_variable_id":
		{
			"name": "Example Variable",
			"unit": "",
			"ui_type": "status",
			"type": "enum",
			"var_type": "Bool",
			"value": 0.0,
			"scaler": 1,
			"enabled": true,
			"num_options": 0,
			"options": 
			[
				{
					"name": "False Case String",
					"value": false
				},
				{
					"name": "True Case String",
					"value": true
				}
			]
		}
	}
}
</code></pre>	


# descriptions:

* ___variables___ - name of the highest-level JSON object and cannot be modified
	* ___example_variable_id___ - the variable identifier.  this corresponds to a Site Manager variable and cannot be modified.
		* ___name___ - string name for the variable.  may be displayed on the UI
		* ___unit___ - string name for the variables units of measure.  may be displayed on the UI
		* ___ui_type___ - determines what type of variable format to use when displaying on the UI.  options are:
			* ___status___ - default.  variable will display on the UI as a status variable.
			* ___control___ - variable will will display on the UI as a control variable.
			* ___fault___ - reserved for "active_faults".
			* ___alarm___ - reserved for "active_alarms".
			* ___none___ - variable will not display on the UI.
		* ___type___ - TBD
		* ___var_type___ - determines what type of variable format to use in Site Manager.  options are:
			* ___Float___ - default. 32-bit float
			* ___Int___  - 32-bit integer
			* ___Bool___ - boolean
			* ___String___ - string (dynamically allocated char*).
		* ___value___ - sets the default initial value for the variable, corresponding to the "var_type"
		* ___scaler___ - declares the scale of the value WRT the "unit" 
		* ___enabled___ - for controls, sets whether the control is enabled or disabled on the UI.
		* ___num_options___ - indicates how many entries are used in the "options" array.  use 0 for empty "options" array.
		* ___options___ - used when additional information is needed about the variable.  e.g. individual fault names for "active_faults" and button state names for boolean controls.
			* ___name___ - string name associated with the corresponding "value".
			* ___value___ - value associated with the corresponding "name".  
	

# appendix: 

* ___defaults___ - this variable contains all default values for each JSON object parameter for reference.  if a variable has the default value, it is not necessary to list that parameter for that variable.
* ___reserved_bool_X___ - X = 1-16.  boolean variable that can be used for site-specific configurations
* ___reserved_float_X___ - X = 1-8.  float variable that can be used for site-specific configurations
* ___local_remote_source_flag___ - determines if remote (modbus) or local (UI) controller has control
* ___local_remote_source_status___ - string representation of which (remote (modbus) or local (UI)) controller has control
* ___local_enable_flag___ - local controller site enable control
* ___local_disable_flag___ - local controller site disable control
* ___local_standby_flag___ - local controller site standby mode control
* ___local_clear_faults___ - local controller site clear faults control
* ___remote_enable_flag___ - remote controller site enable control
* ___remote_disable_flag___ - remote controller site disable control
* ___remote_standby_flag___ - remote controller site standby mode control
* ___remote_clear_faults___ - remote controller site enable control
* ___present_enable_flag___ - status of site enable control (either remote or local)
* ___present_disable_flag___ - status of site disable control (either remote or local)
* ___present_standby_flag___ - status of site standby flag control (either remote or local)
* ___present_clear_faults___ - status of site clear faults control (either remote or local)
* ___faults___ - placeholder for all potential site faults that active faults are derived from
* ___active_faults___ - array of strings identifying active or latched site faults (faults cause site shutdown)
* ___alarms___ - placeholder for all potential site alarms that active alarms are dericed from
* ___active_alarms___ - array of strings identifying active or latched site alarms (alarms do not cause site shutdown)
* ___site_state___ - the current state string name (Init, Ready, Startup, Running, Shutdown, Error)
* ___site_status___ - the current path string name and step string name (unique, in sequences.json)
* ___running_status_flag___ - boolean indicator for whether or not site state is Running
* ___alarm_status_flag___ - boolean indication of whether any site alarms are active or latched
* ___fault_status_flag___ - boolean indication of whether any site faults are active or latched
* ___exit_timer___ - timeout in milliseconds for failed sequence step (will fault)
* ___asset_priority_runmode1___ - numeric indicator for RunMode1 asset type priority (for active power distribution).
	* 0 = solar, gen, ess, feeder 
	* 1 = solar, feeder, ess, gen
	* 2 = gen, ess, solar, feeder
* ___asset_priority_runmode2___ - numeric indicator for RunMode2 asset type priority (for active power distribution).
* ___ess_kW_cmd___ - active power command for ESS assets
* ___gen_kW_cmd___ - active power command for generator assets
* ___solar_kW_cmd___ - active power command for solar assets
* ___feeder_kW_cmd___ - active power command for feeder assets
* ___ess_kVAR_cmd___ - reactive power command for ESS assets
* ___gen_kVAR_cmd___ - reactive power command for generator assets
* ___solar_kVAR_cmd___ - reactive power command for solar assets
* ___site_kW_demand___ - site total active power demand calculation
* ___site_kW_load___ - site active power load calculation (sum of controllable assets' measured power outputs)
* ___site_kVAR_demand___ - site total reactive power demand calculation
* ___grid_target_kW_cmd___ - active power command for grid target mode (all assets will regulate active power to maintain this target value at POI)
* ___start_first_gen_kW___ - kW threshold to check for issuing start generator asset request to asset manager
* ___start_first_ess_kW___ - kW threshold to check for issuing start ess asset request to asset manager
* ___start_first_solar_kW___ - kW threshold to check for issuing start solar asset request to asset manager
* ___ess_kVAR_slew_rate___ - ESS reactive power slew rate, kVAR/s
* ___gen_kVAR_slew_rate___ - generator reactive power slew rate, kVAR/s
* ___solar_kVAR_slew_rate___ - solar reactive power slew rate, kVAR/s
* ___power_priority_flag___ - false for active power priority, true for reactive power priority
* ___ESS_instant_discharge___ - KPI calculation, total instantaneous active power in discharge direction
* ___ESS_instant_charge_grid___ - KPI calculation, total instantaneous active power in charge direction from grid
* ___ESS_instant_charge_pv___ - KPI calculation, total instantaneous active power in charge direction from PV/solar
* ___ess_actual_kW___ - total aggregate measured active power from ESS assets
* ___gen_actual_kW___ - total aggregate measured active power from generator assets
* ___solar_actual_kW___ - total aggregate measured active power from solar assets
* ___feeder_actual_kW___ - total aggregate measured active power from POI feeder
* ___ess_actual_kVAR___ - total aggregate measured reactive power from ESS assets
* ___gen_actual_kVAR___ - total aggregate measured reactive power from generator assets
* ___solar_actual_kVAR___ - total aggregate measured reactive power from solar assets
* ___feeder_actual_kVAR___ - total aggregate measured reactive power from POI feeder

* ___energy_arb_enable_flag___ - set this variable true if site uses energy arbitrage feature
* ___price___ - current price ($/MWh) supplied by remote controller
* ___local_threshold_charge_1___ - local controller normal charge price threshold
* ___local_threshold_charge_2___ - local controller max charge price threshold
* ___local_threshold_dischg_1___ - local controller normal discharge price threshold
* ___local_threshold_dischg_2___ - local controller max discharge price threshold
* ___remote_threshold_charge_1___ - remote controller normal charge price threshold
* ___remote_threshold_charge_2___ - remote controller max charge price threshold
* ___remote_threshold_dischg_1___ - remote controller normal discharge price threshold
* ___remote_threshold_dischg_2___ - remote controller max discharge price threshold
* ___present_threshold_charge_1___ - status of normal charge price threshold (either remote or local)
* ___present_threshold_charge_2___ - status of max charge price threshold (either remote or local)
* ___present_threshold_dischg_1___ - status of normal discharge price threshold (either remote or local)
* ___present_threshold_dischg_2___ - status of max discharge price threshold (either remote or local)
* ___soc_min_all___ - minimum state of charge for all ESS assets
* ___soc_max_all___ - maximum state of charge for all ESS assets
* ___soc_avg_all___ - average state of charge for all ESS assets
* ___soc_min_running___ - minimum state of charge for running ESS assets
* ___soc_max_running___ - maximum state of charge for running ESS assets
* ___soc_avg_running___ - average state of charge for running ESS assets
* ___soc_min_limit___ - minimum state of charge limit for energy arbitrage normal discharge price threshold
* ___soc_max_limit___ - maximum state of charge limit for energy arbitrage normal charge price threshold
* ___max_charge_1___ - site normal charge setpoint for ESS assets
* ___max_charge_2___ - site maximum charge setpoint for ESS assets
* ___max_dischg_1___ - site normal discharge setpoint for ESS assets
* ___max_dischg_2___ - site maximum discharge setpoint for ESS assets
* ____energy_arb_power___ - internal variable, active power command after energy arbitrage algorithm
* ___local_manual_power___ - local controller manual power numeric control
* ___remote_manual_power___ - remote controller manual power numeric control
* ___present_manual_power___ - manual power numeric control (either remote or local)
* ___local_manual_flag___ - local controller manual or automatic mode control
* ___local_manual_status___ - local controller string representation (Manual, Automatic) for manual or automatic mode
* ___local_chg_dischg_flag___ - local controller charge/discharge direction for manual power numeric control
* ___local_chg_dischg_status___ - local controller string representation (Charge, Discharge) for charge/discharge direction of manual power numeric control
* ___remote_manual_flag___ - remote controller manual or automatic mode control
* ___remote_manual_status___ - remote controller string representation (Manual, Automatic) for manual or automatic mode
* ___remote_chg_dischg_flag___ - remote controller charge/discharge direction for manual power numeric control
* ___remote_chg_dischg_status___ - remote controller string representation (Charge, Discharge) for charge/discharge direction of manual power numeric control	
* ___present_manual_flag___ - manual or automatic mode control (either remote or local)
* ___present_manual_status___ - string representation (Manual, Automatic) for manual or automatic mode (either remote or local)
* ___present_chg_dischg_flag___ - charge/discharge direction for manual power numeric control (either remote or local)
* ___present_chg_dischg_status___ - string representation (Charge, Discharge) for charge/discharge direction of manual power numeric control (either remote or local)
* ___max_potential_ess_kW___ - maximum total instantaneous available active power from ESS asset
* ___max_potential_gen_kW___ - maximum total instantaneous available active power from generator asset
* ___max_potential_solar_kW___ - maximum total instantaneous available active power from solar asset
* ___max_potential_feeder_kW___ - maximum total instantaneous available active power from POI feeder
* ___min_potential_ess_kW___ - minimum total instantaneous available active power from ESS asset
* ___min_potential_gen_kW___ - minimum total instantaneous available active power from generator asset
* ___min_potential_solar_kW___ - minimum total instantaneous available active power from solar asset
* ___min_potential_feeder_kW___ - minimum total instantaneous available active power from POI feeder
* ___potential_ess_kVAR___ - maximum total instantaneous available reactive power from ESS asset
* ___potential_gen_kVAR___ - maximum total instantaneous available reactive power from generator asset
* ___potential_solar_kVAR___ - maximum total instantaneous available reactive power from solar asset
* ___curtailment_target___ - site active power maximum output limit, used in site power limit algorithms when curtailment is enabled
* ___curtailment_enable_flag___ - control to enable curtailment algorithm
* ___inverter_output_percent___ - percent of utilized output capacity for site solar production, used in curtailment algorithm
* ___curtailment_price___ - price used for "free power check" in curtailment algorithm
* ___curtailment_enable_feedback_flag___ - feedback for curtailment_enable_flag
* ___curtailment_status_flag___ - indication of whether curtailment algorithm has impacted ess_kW_cmd
* ___curtailment_free_power_kW_slew_rate___ - /rate at which curtailment free power slew occurs (delta x per second)
* ____slew_free_power___ - current slew value for active_power_cmd
* ___frequency_droop_target___ - LEGACY CODE, site active power maximum output limit, used in site power limit algorithms when frequency_droop is enabled
* ___frequency_droop_enable_flag___ - LEGACY CODE, control to enable frequency droop algorithm
* ___frequency_droop_enable_feedback_flag___ - LEGACY CODE, feedback for frequency_droop_enable_flag
* ___frequency_droop_status_flag___ -  LEGACY CODE, indication of whether frequency droop algorithm has impacted 
* ___pfr_deadband___ - window within which PFR algorithm will not adjust power.  window is nominal Hz +/- deadband 
* ___pfr_droop_percent___ - the percent of nominal Hz that will result in a 100% droop response
* ___pfr_offset_hz___ - offset to adjust pfr_actual_hz for manually testing PFR algorithm
* ___pfr_nominal_hz___ - nominal bus frequency
* ___pfr_actual_hz___ - instantaneous site frequency, used in PFR algorithm
* ___pfr_status_flag___ - indication of whether PFR algorithm has impacted ESS_active_power_cmd
* ___pfr_enable_flag___ - control to enable PFR algorithm
* ___pfr_nameplate_kW___ - maximum power output for PFR algorithm
* ____pfr_power___ - internal variable, active power command after PFR algorithm
* ___avr_enable_flag___ - enable active voltage response algorithm
* ___avr_target_voltage___ - target voltage setpoint for active voltage response algorithm
* ___ess_charge_control_kW_request___ - charge kW request from control algorithm
* ____ess_charge_control_kW_command___ - actual kW output from charge control algorithm
* ___ess_charge_control_enable_flag___ - enable boolean for charge control algorithm
* ___ess_charge_control_solar_charge_only_flag___ - when true, only potential solar will be used for charging
* ___ess_charge_control_solar_dischg_priority_flag___ - when true, maximizing solar power production will take precedence over ess discharge requests
* ___local_ess_charge_control_target_soc___ - local control for target state of charge in charge control algorithm
* ___remote_ess_charge_control_target_soc___ - remote control for target state of charge in charge control algorithm
* ___present_ess_charge_control_target_soc___ - active control feedback for target state of charge in charge control algorithm
* ___ess_charge_control_kW_limit___ - kW limit for for ESS, applied to both charge and discharge 
* ___ess_charge_control_feeder_enable_flag___ - when false, do not use feeder as source to charge ESS
* ___ess_charge_support_enable_flag___ - when true, ESS will charge as needed to compensate when other asset types cannot slew down fast enough to meet a reduced site demand - only available in runmode1 (grid-tied)
* ___grid_target_mode_enable_flag___ - boolean to enable demand management
* ___target_soc_mode_enable_flag___ - boolean to enable target soc mode
* ___manual_mode_enable_flag___ - boolean to enable manual mode
* ___local_manual_solar_kW_cmd___ - local manual setpoint for solar active power
* ___remote_manual_solar_kW_cmd___ - remote manual setpoint for solar active power
* ___present_manual_solar_kW_cmd___ - feedback for manual setpoint for solar active power
* ___local_manual_ess_kW_cmd___ - local manual setpoint for ESS active power
* ___remote_manual_ess_kW_cmd___ - remote manual setpoint for ESS active power
* ___present_manual_ess_kW_cmd___ - feedback for manual setpoint for ESS active power
* ___export_target_mode_enable_flag___ - boolean to enable export target mode 
* ___local_export_target_kW_cmd___ - local export target mode active power command
* ___remote_export_target_kW_cmd___ - remote export target mode active power command
* ___present_export_target_kW_cmd___ - feedback for export target mode active power command
* ___export_target_max_soc___ - export target mode maximum ESS state of charge setpoint
* ___export_target_min_soc___ - export target mode minimum ESS state of charge setpoint
* ___export_target_kW_slew_rate___ - export target mode active power slew rate setpoint
* ___available_features_kW_mode___ - list of all site controller available features.  can be masked to only enable a subset
* ___local_features_kW_mode_cmd___ - local control word for active power features
* ___remote_features_kW_mode_cmd___ - remote control word for active power features
* ___present_features_kW_mode_cmd___ - feedback for active power features
* ___features_kW_mode_status___ - string display feedback for active power features
* ___local_features_kVAR_mode_cmd___ - local control word for reactive power features
* ___remote_features_kVAR_mode_cmd___ - remote control word for reactive power features
* ___present_features_kVAR_mode_cmd___ - feedback for reactive power features
* ___features_kVAR_mode_status___ - string display feedback for reactive power features
* ___reactive_setpoint_mode_enable_flag___ - boolean to enable reactive setpoint mode
* ___local_reactive_setpoint_kVAR_cmd___ - local reactive setpoint mode power command
* ___remote_reactive_setpoint_kVAR_cmd___ - remote reactive setpoint mode power command
* ___present_reactive_setpoint_kVAR_cmd___ - feedback for reactive setpoint mode power command
* ___reactive_setpoint_kVAR_slew_rate___ - slew rate for power command
* ___active_voltage_mode_enable_flag___ - enable control for active voltage mode
* ___active_voltage_deadband___ - deadband within which algorithm wont respond
* ___active_voltage_droop_percent___ - percent of rated to apply per v deviation
* ___local_active_voltage_cmd___ - local active voltage command for active voltage mode
* ___remote_active_voltage_cmd___ - remote active voltage command for active voltage mode
* ___present_active_voltage_cmd___ - feedback for active voltage command for active voltage mode
* ___active_voltage_actual_volts___ - measured instantaneous voltage to calculated deviation with
* ___active_voltage_status_flag___ - boolean flag, true when response is nonzeroreactive power
* ___active_voltage_rated_kVAR___ - rated kVAR for calculating response kVAR
* ___power_factor_mode_enable_flag___ - enable control for power factor mode
* ___local_power_factor_cmd___ - local power factor command
* ___remote_power_factor_cmd___ - remote power factor command
* ___present_power_factor_cmd___ - feedback for power factor command
* ___watt_pf_mode_enable_flag___ - enable control for watt power factor mode
* ___watt_var_mode_enable_flag___ - enable control for watt var mode
* ___watt_var_points___ - points array for watt var mode
* ___watt_watt_adjustment_enable_flag___ - enable control for watt watt adjustment protection
* ___watt_watt_points___ - points array for watt watt adjustment protection
* ___peak_power_limit_status_flag___ - boolean status true when peak power limit occurs on POI
* ___site_frequency___ - actual measurement in Hz of site's frequency to be continuously updated
* ___fr_mode_enable_flag___ - enable control for frequency response mode
* ___fr_site_nominal_hz___ - target frequency that the system wants the site to be at
* ___fr_baseload_kW_cmd___ - amount of power that will be added to the compensation power as an offset
* ___fr_inactive_kW_cmd___ - compensation power when there is no frequency trigger event
* ___fr_total_kW_cmd___ - output of frequency response algorithm and is the sum of baseload power and compensation power
* ___fr_UF_enable_flag___ - enable control to allow frequency response to change compensation power during an underfrequency deviation event
* ___fr_UF_hz_trigger_percent___ - the percentage of nominal frequency that site frequency must shift by to begin an underfrequency deviation event
* ___fr_UF_trigger_time___ - the max amount of time that an underfrequency trigger event will output compensation power before reseting to inactive compensation power for cooldown
* ___fr_UF_cooldown_status___ - status bool showing if the system is cooling down from an underfrequency trigger event or not
* ___fr_UF_cooldown_time___ - the amount of time the system must cool off after an underfrequency trigger event before being allowed to enter a new underfrequency trigger event
* ___fr_UF_droop_bypass_flag___ - control bool which, when true, engages FFR or FRRS mode for underfrequency events (depending on slew override). When false, system is in PFR mode
* ___fr_UF_status_flag___ - status bool showing if underfrequency deviation event is happening or not
* ___fr_UF_active_kW_cmd___ - compensation power during an underfrequency trigger event
* ___fr_UF_droop_percent___ - the percentage devation of nominal frequency that site frequency must fall by to reach rated power output. Only applicable in PFR mode
* ___fr_UF_droop_limit_flag___ - control bool that, when true, does not allow power output to exceed rated power in underfrequency trigger event. Only applicable in PFR mode
* ___fr_UF_hz_recover_percent___ - the percentage deviation of nominal frequency that site frequency must be within to begin a recovery state from underfrequency event
* ___fr_UF_hz_instant_recover_percent___ - the percentage deviation of nominal frequency that site frequency must be within to skip underfrequency recovery state and immediately end trigger event
* ___fr_UF_recover_time___ - the amount of time an underfrequency recovery state must last before trigger event ends
* ___fr_UF_recover_latch___ - control bool that, when true, will keep an underfrequency recovery state active even if site frequency drifts out of recovery frequency band
* ___fr_UF_slew_rate___ - controls how quickly the frequency response algorithm can change the output power during underfrequency event
* ___fr_UF_slew_override_flag___ - control bool that, when true, will override the internal ESS slew rate for underfrequency events (will not override the algorithm's slew rate)
* ___fr_OF_enable_flag___ - enable control to allow frequency response to change compensation power during an overfrequency deviation event
* ___fr_OF_hz_trigger_percent___ - the percentage of nominal frequency that site frequency must shift by to begin an overfrequency deviation event
* ___fr_OF_trigger_time___ - the max amount of time that an overfrequency trigger event will output compensation power before reseting to inactive compensation power for cooldown
* ___fr_OF_cooldown_status___ - status bool showing if the system is cooling down from an overfrequency trigger event or not
* ___fr_OF_cooldown_time___ - the amount of time the system must cool off after an overfrequency trigger event before being allowed to enter a new overfrequency trigger event
* ___fr_OF_droop_bypass_flag___ - control bool which, when true, engages FFR or FRRS mode for overfrequency events (depending on slew override). When false, system is in PFR mode
* ___fr_OF_status_flag___ - status bool showing if overfrequency deviation event is happening or not
* ___fr_OF_active_kW_cmd___ - compensation power during an overfrequency trigger event
* ___fr_OF_droop_percent___ - the percentage devation of nominal frequency that site frequency must rise by to reach rated power output. Only applicable in PFR mode
* ___fr_OF_droop_limit_flag___ - control bool that, when true, does not allow power output to exceed rated power in overfrequency trigger event. Only applicable in PFR mode
* ___fr_OF_hz_recover_percent___ - the percentage deviation of nominal frequency that site frequency must be within to begin a recovery state from overfrequency event
* ___fr_OF_hz_instant_recover_percent___ - the percentage deviation of nominal frequency that site frequency must be within to skip overfrequency recovery state and immediately end trigger event
* ___fr_OF_recover_time___ - the amount of time an overfrequency recovery state must last before trigger event ends
* ___fr_OF_recover_latch___ - control bool that, when true, will keep an overfrequency recovery state active even if site frequency drifts out of recovery frequency band
* ___fr_OF_slew_rate___ - controls how quickly the frequency response algorithm can change the output power during overfrequency event
* ___fr_OF_slew_override_flag___ - control bool that, when true, will override the internal ESS slew rate for overfrequency events (will not override the algorithm's slew rate)
* ___watchdog_enable___ - control bool that, when true, enables the watchdog timer feature
* ___watchdog_duration_ms___ - length of time watchdog timer will count down before barking
* ___watchdog_pet___ - an int to be written to that will signal a reset to the watchdog timer
* ___heartbeat_counter___ - an int that increments periodically and gets broadcasted out to signal that a connection is still present
* ___poi_limits_enable___ - POI limit protection enable command
* ___poi_limits_max_kW___ - POI limit protection - maximum active power allowed through POI
* ___poi_limits_min_kW___ - POI limit protection - minimum active power allowed through POI
* ___invert_poi_kW___ - enable to invert the POI active power sign 
* ___cops_heartbeat___ - cops heartbeat provided here