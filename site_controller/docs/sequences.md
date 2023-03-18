Site Manager - sequences.json Documentation

Author: Kyle Brezina

Date Created: 03/13/19

Date Modified: 03/13/19


# example sequences.json file:

<pre><code>
{	  
    "sequences":
    {
        "Init":
        {
            "sequence_name": "Init",
            "paths":
            [
                {
                    "path_name": "Init",
                    "return_id": "Ready",
                    "timeout": 15,
                    "active_faults": 
                    [
                        {
                            "asset_fault_name": "/bypass",
                            "asset_active_faults": [ 0 ]
                        }
                    ],
                    "steps":
                    [
                        {
                            "step_name": "Set State Variables",
                            "entry_actions": 
                            {
                                "route": "/bypass",
                                "value": true
                            },
                            "exit_conditions": 
                            {
                                "route": "/bypass",
                                "value": true,
                                "debounce_timer": 1000
                            }
                        },
                        {
                            "step_name": "Smart Breaker Check",
                            "path_switch": true,
                            "next_path": 2,
                            "entry_actions": 
                            {
                                "route": "/bypass",
                                "value": true
                            },
                            "exit_conditions": 
                            {
                                "route": "/bypass",
                                "value": true
                            }
                        },
                    ]
                }
            ]
        },
</code></pre>


# descriptions:

* ___sequences___ - name of the highest-level JSON object and cannot be modified
	* ___Init___ - the sequence identifier.  a single sequence may be created for each of the following states/identifiers: [___Init___, ___Ready___, ___Startup___, ___RunMode1___, ___RunMode2___, ___Shutdown___, ___Error___]
		* ___sequence_name___ - string name for the sequence.  no real requirements here (arguably redundant, debug use only)
        * ___paths___ - a group of steps to execute in the sequence.  multiple paths can be linked together with conditional logic
            * ___path_name___ - unique string name for the path 
            * ___return_id___ - contains the sequence identifier that will be called after this sequence finishes
            * ___timeout___ - path will fail and exit to shutdown sequence if this timeout timer expires before all path steps are completed.  default timeout is disabled.
            * ___active_faults___ - contains an array of entries.  each entry checks a specific fault for a specific value.  if that value is found, a sequence fault is asserted.
                * ___asset_fault_name___ - corresponds to a specific fault.  see appendix for full list.
                * ___asset_active_faults___ - contains an array of numbers.  only the first value is used at this time.  these will be made available in the logic for the specific fault and its use may vary.  
            * ___steps___ - name of the JSON object and cannot be modified.  it contains an array of steps.  each step will perform an action, check for an exit condition and proceed to the next step.   after the last step, the "return_id" sequence is called.
                * ___step_name___ - string name for the step.  no real requirements here (arguably redundant, debug use only) 	
                * ___path_switch___ - when true, this step will move to the new designated path if the step return is true.  if the step return is false, step will move to next step in the path.	default path_switch is false
                * ___next_path___ - only valid when "path_switch" is true. determines which path the step will move to if the step returns true.  path must be in current sequence, and paths are numbered in order, 0 index.
                * ___entry_actions___ - contains necessary information to call specific functions.  see appendix for full list. 
                    * ___route___ - iIdentifies a specific function call
                    * ___value___ - sends this value, if necessary, to the specific function call
                * ___exit_conditions___	- contains necessary information to call specific functions.  see appendix for full list.
                    * ___route___ - identifies a specific function call
                    * ___value___ - used to check the return value from the function call, and its use may vary.
                    * ___tolerance___ - may be used in the case of numeric values.  the function's return value must be within the perecent tolerance of the return value for the exit condition to complete.
                    * ___debounce_timer___ - a timer, in milliseconds.  the function call must return an in range value consistently for this period of time for the exit condition to complete. default debounce is disabled (0ms)
				

# appendix:

## active_faults

* ___/bypass___ - never returns a fault.  it is necessary to use this when no other "active_fault" entries are present.
* ___/test_fault___ - always returns a fault.  used for internal debugging.
* ___/assets___
    * ___/get_any_ess_faults___ -  if return value > 0, fault is asserted.
    * ___/get_masked_ess_faults___ -  a bitmask comparison is done between the ESS fault return value and "asset_active_faults" value.  a match will assert a fault.
    * ___/get_num_ess_running___ - returns the number of ESS assets running.  this value must be greaten than or equal to "asset_active_faults" value.
    * ___/get_num_ess_available___ - returns the number of ESS assets available.  this value must be greaten than or equal to "asset_active_faults" value.
    * ___/get_num_assets_controllable___ - calculates the number of each asset type running. if ALL (Gen, ESS, Solar) asset types report no controllable asset instances, fault is asserted.

## entry_actions and exit_conditions

* ___/bypass___ - always proceeds to the next step in the path.  it is necessary to use this when no other "entry_actions" are present.
* ___/new_path___ - issues command to start all ESS Assets.
* ___/reserved___ - all generic boolean and floats for site-specific configuration. (written from external source)
    * ___bool_X___ - X = 1-16.  boolean type.
    * ___float_X___ - X = 1-8.  float type. 
* ___/ess___
    * ___/get_num_ess_available___ - returns the number of ESS assets available.  this value must be within tolerance.
    * ___/get_num_ess_running___ - returns the number of ESS assets running.  this value must be within tolerance.
    * ___/get_ess_active_power___ - returns the instantaneous measured ESS Asset active power (kW).  this value must be within tolerance.
    * ___/start_all_ess___ - issues command to start all ESS assets.
    * ___/stop_all_ess___ - issues command to stop all ESS assets.
    * ___/set_all_ess_grid_form___ - issues command to set all ESS into grid-forming mode.
    * ___/set_all_ess_grid_follow___ - issues command to set all ESS into grid-following mode.
    * ___/set_voltage_slope___ - issues command to set ESS grid-forming voltage slew (V/s).
    * ___/open_contactors___ - issues command to open all BMS contactors.
    * ___/close_contactors___ - issues command to close all BMS contactors.
    * ___/synchronize_ess___ - calls the ESS synchronization function to match V, Hz across a breaker
* ___/gen___
    * ___/get_num_gen_available___ - returns the number of generator assets available.  this value must be within tolerance.
    * ___/get_num_gen_running___ - returns the number of generator assets running.  this value must be within tolerance.
    * ___/get_num_gen_active___ - returns the number of generator assets active.  this value must be within tolerance.
    * ___/min_generators_active___ - sets the required minimum number of active generators.  
    * ___/direct_start_gen___ - issues command to start a single generator asset.
    * ___/start_all_gen___ - issues command to start all generator assets.
    * ___/set_all_gen_grid_follow___ - issues command to set all generators into grid-following mode.
    * ___/set_all_gen_grid_form___ - issues command to set all generators into grid-forming mode.
* ___/solar___
    * ___/get_num_solar_available___ - returns the number of solar assets available.  this value must be within tolerance.
    * ___/get_num_solar_running___ - returns the number of solar assets running.  this value must be within tolerance.
    * ___/start_all_solar___ - issues command to start all solar assets.
    * ___/stop_all_solar___ - issues command to stop all solar assets.
* ___/feeder___
    * ___/get_poi_feeder_state___ - boolean comparison between POI feeder state and "value".  returns true if POI is closed and "value" is true OR if POI is open and "value" is false.
    * ___/set_sync_feeder_close_permissive_remove___ - issues command to remove close permissive to sync feeder.
    * ___/set_sync_feeder_close_permissive___ - issues command to assert close permissive to sync feeder.
* ___/config___
    * ___/clear_faults___ - clears any latched site manager faults
    * ___/set_addremove_timers_enabled___ - enables LDSS (and its timers).
    * ___/site_state_init___ - notifies asset manager that site manager state is ___Init___ or its mode specific variable selection.
    * ___/site_state_runmode1___ - notifies asset manager that site manager state is ___RunMode1___ for its mode specific variable selection.
    * ___/site_state_runmode2___ - notifies asset manager that site manager state is ___RunMode2___ for its mode specific variable selection.
    * ___/enable_grid_target_kW_cmd___ - enables or disables "Grid Target kW Cmd" register on UI, determined by boolean "value"
    * ___/get_standby_flag___ - compares standby_flag Fims_Object variable boolean status to "value" and returns true if they match






