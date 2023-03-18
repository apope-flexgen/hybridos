Asset Manager Variables/URI Documentation

Author: Guy Howard, Camden Levinson

Date Created: 04/15/20

Date Modified: 07/21/20
    * datatype: 
    * uri: 
    * control: 
    * description: 

# HybridOS Asset Blocks
   - Asset Manager
      - Generator
      - Feeders 
      - Storage (ESS)
      - Solar
      - Configuration (.json)

# Asset Configuration per site - overall structure
   - JSON Asset data definition
      - Fixed asset configuration parameters
      - Assets
        - Components
          - Component ID
          - Variables...
          - UI controls...

# /assets

## Notes concerning behavior of data defined in base class

1. Any variables defined in the base class but not defined
as a part of an asset in the .json configuration file, assets.json,
will not be included in any activity in HybridOS_Control,
regardless of activity on FIMS.

2. Most data with uris starting with '/components' will be
published to '/assets' with the component id used to determine
which asset owns that data.  For example, a datum published
by an Ess with the uri '/components/sungrow_ess_1/voltage_dc'
will be published to '/assets/ess/ess_1/voltage_dc' by HybridOS_Control
for status display on HMI. The name 'voltage_dc' in both
/components and /assets versions is defined in assets.json.
The names are _not_ required to be identical.

## /generators
### /asset type configuration
* id: start_value - 1
    * datatype: int
    * uri: N/A - value - see ui_controls 'start' variable
    * control: yes
    * description: value transmitted to asset component to start asset
* id: stop_value - 0
    * datatype: int
    * uri: N/A - value
    * control: yes
    * description: value to stop asset
* id: grid_form_cmd - 1
    * datatype: 
    * uri: 
    * control: 
    * description: 
* id: grid_follow_cmd - 0
    * datatype: 
    * uri: 
    * control: 
    * description:
#### Example
        "start_value": 1,
        "stop_value": 0,
        "grid_form_cmd": 1,
        "grid_follow_cmd": 0,
        "asset_instance":
### /asset characteristics configuration                
* id: id
    * datatype: string
    * uri: used as a component in uri construction and as an internal identifier
    * control: no
    * description: identifier used in Asset Manager for asset instance
* id: name": 
    * datatype: string
    * uri: also used as a component in uri construction
    * control: no
    * description: string used for UI display
* id: rated_active_power_kw
    * datatype: float
    * uri: /assets/gen_[n]/rated_active_power_kw
    * control: no
    * description: asset rated active power kW 
* id: rated_reactive_power_kvar
    * datatype: float
    * uri: /assets/gen_[n]/rated_reactive_power_kvar
    * control: no
    * description: asset rated reactive power kW - configuration
* id: rated_apparent_power_kva
    * datatype: float
    * uri: /assets/gen_[n]/rated_apparent_power_kva
    * control: no
    * description: asset rated active power kW 
* id: slew_rate
    * datatype: int
    * uri: 
    * control: 
    * description: not used
* id: stopped_status_mask
    * datatype: hex string
    * uri: N/A
    * control: no
    * description: bit pattern used to detect stopped status value sent from components. To support a value, set the bit
    in the position given by the value to high, counting up from 0. Multiple valid values are supported, from 0 to 63. Examples:
    ```
    status: 0     binary mask: 0001 (position 0 true)      hex string: 0x01
    status: 0, 3  binary mask: 1001 (positions 0, 3 true)  hex string: 0x09
    ```
* id: running_status_mask
    * datatype: hex string
    * uri: N/A
    * control: no
    * description: bit pattern used to detect running status value sent from components. To support a value, set the bit
    in the position given by the value to high, counting up from 0. Multiple valid values are supported, from 0 to 63. Examples:
    ```
    status: 1     binary mask: 0010 (position 1 true)      hex string: 0x02
    status: 1, 2  binary mask: 0110 (positions 1, 2 true)  hex string: 0x06
    ```
* id: starting_status_mask
    * datatype: double
    * uri: N/A
    * control: no
    * description: bit pattern used to detect starting status message from asset
* id: stopping_status_mask
    * datatype: double
    * uri: N/A
    * control: no
    * description: bit pattern used to detect starting status message from asset

#### Example
    "id": "gen_1",
    "name": "Cat CG170-16 - Gen 1",
    "rated_active_power_kw": 1542.0,
    "rated_reactive_power_kvar": 1156.0,
    "rated_apparent_power_kva": 1927.0,
    "slew_rate": 200,
    "stopped_status_mask":128,
    "running_status_mask":8,
    "starting_status_mask":0x0F,
    "stopping_status_mask":0x40,
### /running mode setpoint array                
* id: siteState
    * datatype: enum/int
    * uri: N/A
    * control: yes
    * description: one of following running modes: {Init, runMode1, runMode2, Shutdown}
* id: add_asset_timer_setpoint
    * datatype: int
    * uri: N/A
    * control: no
    * description: period in seconds that asset must be stopped before it can be started
* id: remove_asset_timer_setpoint
    * datatype: int
    * uri: N/A
    * control: no
    * description: period in seconds that asset must be running before it can be started
* id: max_load_threshold
    * datatype: int
    * uri: N/A
    * control: no
    * description: asset power production in kW above which another asset will be requested to share the load.
* id: min_load_threshold": 40,
    * datatype: int
    * uri: N/A
    * control: no
    * description: asset power production in kW below which an asset will be requested to stop.
* id: start_priority": 1,
    * datatype: int
    * uri: N/A
    * control: 
    * description: 
* id: stop_priority": 1,
    * datatype: int
    * uri: N/A
    * control: 
    * description: 
* id: run_time_setpoint": 1,
    * datatype: int
    * uri: N/A
    * control: 
    * description: 
* id: stop_time_setpoint": 1,
    * datatype: int
    * uri: N/A
    * control: 
    * description: 
* id: demand_control": "Indirect",
    * datatype: 
    * uri: N/A
    * control: 
    * description: 
* id: update_rate_ms": 4000
    * datatype: int
    * uri: N/A
    * control: 
    * description: 
#### Example
"modesInit":
{
    "siteState":"Init",
    "add_asset_timer_setpoint": 15,
    "remove_asset_timer_setpoint": 45,
    "max_load_threshold": 90,
    "min_load_threshold": 40,
    "start_priority": 1,
    "stop_priority": 1,
    "run_time_setpoint": 1,
    "stop_time_setpoint": 1,
    "demand_control": "Indirect",
    "update_rate_ms": 4000
},
{
    "siteState":"runMode1",
    "add_asset_timer_setpoint": 15,
    "remove_asset_timer_setpoint": 45,
    "max_load_threshold": 90,
    "min_load_threshold": 40,
    "start_priority": 1,
    "stop_priority": 1,
    "run_time_setpoint": 1,
    "stop_time_setpoint": 1,
    "demand_control": "Direct",
    "update_rate_ms": 4000
},
{
    "siteState":"runMode2",
    "add_asset_timer_setpoint": 15,
    "remove_asset_timer_setpoint": 45,
    "max_load_threshold": 90,
    "min_load_threshold": 40,
    "start_priority": 1,
    "stop_priority": 1,
    "run_time_setpoint": 1,
    "stop_time_setpoint": 1,
    "demand_control": "Indirect",
    "update_rate_ms": 4000
},
{
    "siteState":"shutDown",
    "add_asset_timer_setpoint": 15,
    "remove_asset_timer_setpoint": 45,
    "max_load_threshold": 90,
    "min_load_threshold": 40,
    "start_priority": 1,
    "stop_priority": 1,
    "run_time_setpoint": 1,
    "stop_time_setpoint": 1,
    "demand_control": "Indirect",
    "update_rate_ms": 4000
}

### /components                
                [
                    {
                        "component_id": "easygen_3500xt_1",
#### /variables                        
                        {
                            "current_l1":
                            {
                                "name": "L1 AC Current",
                                "register_id": "ac_current_l1",
                                "value": null,
                                "scaler": 1,
                                "unit": "A",
                                "twins_id": "i"
                            },
                            "current_l2":
                            {
                                "name": "L2 AC Current",
                                "register_id": "ac_current_l2",
                                "value": null,
                                "scaler": 1,
                                "unit": "A",
                                "twins_id": "i"
                            },
                            "current_l3":
                            {
                                "name": "L3 AC Current",
                                "register_id": "ac_current_l3",
                                "value": null,
                                "scaler": 1,
                                "unit": "A",
                                "twins_id": "i"
                            },
                            "voltage_l1_l2":
                            {
                                "name": "L1-L2 AC Voltage",
                                "register_id": "ac_voltage_l1_l2",
                                "value": null,
                                "scaler": 1,
                                "unit": "V",
                                "twins_id": "v"
                            },
                            "voltage_l2_l3":
                            {
                                "name": "L2-L3 AC Voltage",
                                "register_id": "ac_voltage_l2_l3",
                                "value": null,
                                "scaler": 1,
                                "unit": "V",
                                "twins_id": "v"
                            },
                            "voltage_l3_l1":
                            {
                                "name": "L3-L1 AC Voltage",
                                "register_id": "ac_voltage_l3_l1",
                                "value": null,
                                "scaler": 1,
                                "unit": "V",
                                "twins_id": "v"
                            },
                            "frequency_avg_rms":
                            {
                                "name": "Frequency",
                                "register_id": "frequency",
                                "value": null,
                                "scaler": 1,
                                "unit": "Hz",
                                "twins_id": "f"
                            },
                            "active_power":
                            {
                                "name": "AC Active Power",
                                "register_id": "active_power",
                                "value": null,
                                "scaler": 1000,
                                "unit": "W",
                                "twins_id": "p"
                            },
                            "reactive_power":
                            {
                                "name": "AC Reactive Power",
                                "register_id": "reactive_power",
                                "value": null,
                                "scaler": 1000,
                                "unit": "VAR",
                                "twins_id": "q"
                            },
                            "apparent_power":
                            {
                                "name": "AC Apparent Power",
                                "register_id": "apparent_power",
                                "value": null,
                                "scaler": 1000,
                                "unit": "VA",
                                "twins_id": "s"
                            },
                            "power_factor":
                            {
                                "name": "Power Factor",
                                "register_id": "power_factor",
                                "value": null,
                                "scaler": 1,
                                "unit": "%",
                                "twins_id": "pf"
                            },
                            "status":
                            {
                                "name": "Status",
                                "register_id": "status",
                                "value": null,
                                "scaler": null,
                                "unit": null
                            },
                            "disable":
                            {
                                "name": "Disable",
                                "register_id": "disable",
                                "value": null,
                                "scaler": null,
                                "unit": null
                            },
                            "alarms":
                            {
                                "name": "Alarms",
                                "register_id": "alarms",
                                "value": null,
                                "scaler": null,
                                "unit": null
                            },
                            "faults":
                            {
                                "name": "Faults",
                                "register_id": "faults",
                                "value": null,
                                "scaler": null,
                                "unit": null
                            },
                            "active_power_setpoint":
                            {
                                "name": "Active Power Setpoint",
                                "register_id": "active_power_setpoint",
                                "value": null,
                                "scaler": 1000,
                                "unit": "W",
                                "twins_id": "pcmd"
                            },
                            "reactive_power_setpoint":
                            {
                                "name": "Reactive Power Setpoint",
                                "register_id": "reactive_power_setpoint",
                                "value": null,
                                "scaler": 1000,
                                "unit": "VAR",
                                "twins_id": "qcmd"
                            },
                            "grid_mode":
                            {
                                "name": "Grid Mode",
                                "register_id": "control_word_2",
                                "value": null,
                                "scaler": null,
                                "unit": null,
                                "twins_id":"ctrlword2"
                            },
                            "grid_forming":
                            {
                                "name": "Grid Forming",
                                "register_id": "grid_forming",
                                "value": null,
                                "scaler": null,
                                "unit": null,
                                "twins_id":"gridforming"
                            }
                        },
#### ui controls                        
                        {
                            "maint_mode":
                            {
                                "name": "Maintenance Mode",
                                "register_id": null,
                                "value": null,
                                "scaler": null,
                                "unit": null
                            },
                            "start":
                            {
                                "name": "Start",
                                "register_id": "control_word_1",
                                "value": null,
                                "scaler": null,
                                "unit": null,
                                "twins_id": "ctrlword1"
                            },
                            "stop":
                            {
                                "name": "Stop",
                                "register_id": "control_word_1",
                                "value": null,
                                "scaler": null,
                                "unit": null,
                                "twins_id": "ctrlword1"
                            },
                            "clear_faults":
                            {
                                "name": "Clear Faults",
                                "register_id": "clear_faults",
                                "value": null,
                                "scaler": null,
                                "unit": null
                            },
                            "maint_active_power_setpoint":
                            {
                                "name": "Active Power Setpoint",
                                "register_id": null,
                                "value": null,
                                "scaler": 1000,
                                "unit": "W",
                                "twins_id": "pcmd"
                            },
                            "maint_reactive_power_setpoint":
                            {
                                "name": "Reactive Power Setpoint",
                                "register_id": null,
                                "value": null,
                                "scaler": 1000,
                                "unit": "VAR",
                                "twins_id": "qcmd"
                            }
                        }
                    }
                ]
            }
## /feeders
### /asset type configuration
* id: poi_feeder
    * datatype: int
    * uri: 
    * control: yes
    * description: 
* id: sync_feeder
    * datatype: int
    * uri: 
    * control: yes
    * description: 
* id: sync_frequency_offset
    * datatype: float
    * uri: 
    * control: no
    * description: 
#### Example
            "poi_feeder": 0,
            "sync_feeder": 2,
            "sync_frequency_offset": 0.10,

            "asset_instances":
### /asset characteristics   
* id: id
    * datatype: string
    * uri: used as a component in uri construction and as an internal identifier
    * control: no
    * description: identifier used in Asset Manager for asset instance
* id: name": 
    * datatype: string
    * uri: also used as a component in uri construction
    * control: no
    * description: string used for UI display
* id: rated_active_power_kw
    * datatype: float
    * uri: /assets/feeder_[n]/rated_active_power_kw
    * control: no
    * description: asset rated active power kW 
* id: rated_reactive_power_kvar
    * datatype: float
    * uri: /assets/feeder_[n]/rated_reactive_power_kvar
    * control: no
    * description: asset rated reactive power kW - configuration
* id: rated_apparent_power_kva
    * datatype: float
    * uri: /assets/feeder_[n]/rated_apparent_power_kva
    * control: no
    * description: asset rated active power kW 
* id: slew_rate
    * datatype: int
    * uri: 
    * control: 
    * description: not used       
* id: value_close
    * datatype: int
    * uri: 
    * control: yes
    * description: value that closes the feeder 
* id: value_close_permissive
    * datatype: int
    * uri: 
    * control: yes
    * description: value that gives access to asset to close feeder
* id: value_close_permissive_remove
    * datatype: int
    * uri: 
    * control: yes
    * description: value that removes access to asset to close feeder
* id: value_open
    * datatype: int
    * uri: 
    * control: yes
    * description: value that opens the feeder       
* id: value_open_permissive
    * datatype: int
    * uri: 
    * control: yes
    * description: value that gives access to asset to open feeder            
* id: value_reset
    * datatype: int
    * uri: 
    * control: yes
    * description: value that resets the feeder
#### Example
                    "id": "feed_1",
                    "name": "Utility Feed - 52-M1",
                    "nominal_voltage": 12000.0,
                    "nominal_frequency": 60.0,
                    "rated_active_power_kw": 9950,
                    "rated_reactive_power_kvar": 500000.0,
                    "rated_apparent_power_kva": 500000.0,
                    "slew_rate": 5000000,
                    "value_close": 1,
                    "value_close_permissive": 1,
                    "value_close_permissive_remove": 0,
                    "value_open": 0,
                    "value_open_permissive": 0,
                    "value_reset": 7
### /running mode setpoint array
* id: siteState
    * datatype: enum/int
    * uri: N/A
    * control: yes
    * description: one of following running modes: {Init, runMode1, runMode2 Shutdown}
* id: demand_control": "Indirect",
    * datatype: 
    * uri: N/A
    * control: 
    * description:          
#### Example             
                    "modesInit":
                    [
                        {
                            "siteState":"Init",
                            "demand_control": "Indirect"
                        },
                        {
                            "siteState":"runMode1",
                            "demand_control": "Indirect"
                        },
                        {
                            "siteState":"runMode2",
                            "demand_control": "Indirect"
                        },
                        {
                            "siteState":"shutDown",
                            "demand_control": "Indirect"
                        }
                    ],
### /components                     
                    [
                        {
                            "component_id": "sel-351-7a",
#### /variables                             
                            {
                                "breaker_status":
                                {
                                    "name": "Breaker Status",
                                    "register_id": "breaker_status",
                                    "value": null,
                                    "scaler": null,
                                    "unit": null,
                                    "twins_id": "closed"
                                },
                                "breaker_tripped":
                                {
                                    "name": "Breaker Tripped",
                                    "register_id": "tripped",
                                    "value": null,
                                    "scaler": null,
                                    "unit": null,
                                    "twins_id": "closed"
                                },
                                "breaker_close_permissive":
                                {
                                    "name": "Breaker Close Permissive",
                                    "register_id": "breaker_close_permissive",
                                    "value": null,
                                    "scaler": null,
                                    "unit": null
                                },
                                "breaker_close_permissive_remove":
                                {
                                    "name": "Breaker Close Permissive Remove",
                                    "register_id": "breaker_close_permissive_remove",
                                    "value": null,
                                    "scaler": null,
                                    "unit": null
                                },
                                "current_l1":
                                {
                                    "name": "L1 AC Current",
                                    "register_id": "ac_current_l1",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "A",
                                    "twins_id": "i"
                                },
                                "current_l2":
                                {
                                    "name": "L2 AC Current",
                                    "register_id": "ac_current_l2",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "A",
                                    "twins_id": "i"
                                },
                                "current_l3":
                                {
                                    "name": "L3 AC Current",
                                    "register_id": "ac_current_l3",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "A",
                                    "twins_id": "i"
                                },
                                "voltage_l1":
                                {
                                    "name": "L1 AC Voltage",
                                    "register_id": "voltage_l1",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "V",
                                    "twins_id": "v1"
                                },
                                "voltage_l2":
                                {
                                    "name": "L2 AC Voltage",
                                    "register_id": "voltage_l2",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "V",
                                    "twins_id": "v1"
                                },
                                "voltage_l3":
                                {
                                    "name": "L3 AC Voltage",
                                    "register_id": "voltage_l3",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "V",
                                    "twins_id": "v1"
                                },
                                "grid_voltage_l1":
                                {
                                    "name": "L1 Grid AC Voltage",
                                    "register_id": "grid_voltage_l1",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "V",
                                    "twins_id": "v2"
                                },
                                "grid_voltage_l2":
                                {
                                    "name": "L2 Grid AC Voltage",
                                    "register_id": "grid_voltage_l2",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "V",
                                    "twins_id": "v2"
                                },
                                "grid_voltage_l3":
                                {
                                    "name": "L3 Grid AC Voltage",
                                    "register_id": "grid_voltage_l3",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "V",
                                    "twins_id": "v2"
                                },
                                "active_power":
                                {
                                    "name": "AC Active Power",
                                    "register_id": "active_power",
                                    "value": null,
                                    "scaler": 1000,
                                    "unit": "W",
                                    "twins_id": "p"
                                },
                                "reactive_power":
                                {
                                    "name": "AC Reactive Power",
                                    "register_id": "reactive_power",
                                    "value": null,
                                    "scaler": 1000,
                                    "unit": "VAR",
                                    "twins_id": "q"
                                },
                                "apparent_power":
                                {
                                    "name": "AC Apparent Power",
                                    "register_id": "apparent_power",
                                    "value": null,
                                    "scaler": 1000,
                                    "unit": "VA",
                                    "twins_id": "s"
                                },
                                "frequency":
                                {
                                    "name": "Site Frequency",
                                    "register_id": "frequency",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "Hz",
                                    "twins_id": "f1"
                                },
                                "grid_frequency":
                                {
                                    "name": "Grid Frequency",
                                    "register_id": "grid_frequency",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "Hz",
                                    "twins_id": "f2"
                                },
                                "phase_angle_delta":
                                {
                                    "name": "Grid-Site Phase Angle Delta",
                                    "register_id": "phase_angle_delta",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "Hz"
                                },
                                "power_factor":
                                {
                                    "name": "Power Factor",
                                    "register_id": "power_factor",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "%",
                                    "twins_id": "pf"
                                }
                            },
#### ui controls                            
                            {
                                "maint_mode":
                                {
                                    "name": "Maintenance Mode",
                                    "register_id": null,
                                    "scaler": null,
                                    "unit": null
                                },
                                "breaker_open":
                                {
                                    "name": "Breaker Open (Utility)",
                                    "register_id": "relay_command",
                                    "value": null,
                                    "scaler": null,
                                    "unit": null,
                                    "twins_id": "ctrlword1"
                                },
                                "breaker_close":
                                {
                                    "name": "Breaker Close (Utility)",
                                    "register_id": "relay_command",
                                    "value": null,
                                    "scaler": null,
                                    "unit": null,
                                    "twins_id": "ctrlword1"
                                },
                                "breaker_reset":
                                {
                                    "name": "Breaker Reset (Utility)",
                                    "register_id": "relay_reset",
                                    "value": null,
                                    "scaler": null,
                                    "unit": null,
                                    "twins_id": "ctrlword1"
                                }
                            }
                        }
                    ]
                }
            ]
        },
## /ess
### /asset type configuration
* id: nominal_voltage
    * datatype: float
    * uri: 
    * control: no
    * description: 
* id: nominal_frequency
    * datatype: float
    * uri: /components/grid/fcmd
    * control: no
    * description: 
* id: start_value
    * datatype: int
    * uri: N/A - value - see ui_controls 'start' variable
    * control: yes
    * description: value transmitted to asset component to start asset
* id: stop_value
    * datatype: int
    * uri: N/A - value
    * control: yes
    * description: value to stop asset
* id: DC_contactor_open
    * datatype: int
    * uri: 
    * control: yes
    * description: if DC switch is open
* id: DC_contactor_closed
    * datatype: int
    * uri: 
    * control: yes
    * description: if DC switch is closed
* id: DC_contactor_reset
    * datatype: int
    * uri: 
    * control: yes
    * description: if DC switch is being reset
* id: DC_contactor_restriction
    * datatype: bool
    * uri:
    * control:
    * description: if contactors must be closed to issue maintenance control commands
* id: chg_soc_begin
    * datatype: float
    * uri: 
    * control: 
    * description: 
* id: chg_soc_end
    * datatype: float
    * uri: 
    * control: 
    * description: 
* id: dischg_soc_begin
    * datatype: float
    * uri: 
    * control: 
    * description: 
* id: dischg_soc_end
    * datatype: float
    * uri: 
    * control: 
    * description: 
* id: min_raw_soc
    * datatype: float
    * uri: 
    * control: 
    * description: 
* id: max_raw_soc
    * datatype: float
    * uri: 
    * control: 
    * description: 
* id: min_renewable_chg
    * datatype: float
    * uri: 
    * control: 
    * description:  
* id: grid_form_cmd
    * datatype: int
    * uri: 
    * control: yes
    * description: 
* id: grid_follow_cmd
    * datatype: int
    * uri: 
    * control: yes
    * description: 
* id: entering_soc_range
    * datatype: float
    * uri: 
    * control: 
    * description: 
* id: leaving_soc_range
    * datatype: float
    * uri: 
    * control: 
    * description: 
* id: far_soc_range
    * datatype: float
    * uri: 
    * control: 
    * description: 
* id: charge_control_divisor
    * datatype: float
    * uri: 
    * control: 
    * description: 
#### Example
            "nominal_voltage": 480.0,
            "nominal_frequency": 60.0,
            "start_value": 1,
            "stop_value": 0,
            "DC_contactor_open": 1,
            "DC_contactor_closed": 2,
            "DC_contactor_reset": 3,
            "chg_soc_begin": 90.0,
            "chg_soc_end": 100.0,
            "dischg_soc_begin": 10.0,
            "dischg_soc_end": 0.0,
            "min_raw_soc":1.5,
            "max_raw_soc":98.5,
            "min_renewable_chg":75,
            "grid_form_cmd":1,
            "grid_follow_cmd":0,
            "entering_soc_range": 0.1,
            "leaving_soc_range": 0.5,
            "far_soc_range": 10.0,
            "charge_control_divisor": 11.1,

            "asset_instances":
### /asset characteristics configuration                
* id: id
    * datatype: string
    * uri: used as a component in uri construction and as an internal identifier
    * control: no
    * description: identifier used in Asset Manager for asset instance
* id: name": 
    * datatype: string
    * uri: also used as a component in uri construction
    * control: no
    * description: string used for UI display
* id: watchdog_enabled 
    * datatype: boolean
    * uri: 
    * control: no
    * description: whether watchdog is enabled or not
* id: watchdog_timeout_ms
    * datatype: int
    * uri: 
    * control: yes
    * description: timeout for when watchdog looks for a change
* id: rated_active_power_kw
    * datatype: float
    * uri: /assets/ess_[n]/rated_active_power_kw
    * control: no
    * description: asset rated active power kW 
* id: rated_reactive_power_kvar
    * datatype: float
    * uri: /assets/ess_[n]/rated_reactive_power_kvar
    * control: no
    * description: asset rated reactive power kW - configuration
* id: rated_apparent_power_kva
    * datatype: float
    * uri: /assets/ess_[n]/rated_apparent_power_kva
    * control: no
    * description: asset rated active power kW 
* id: slew_rate
    * datatype: int
    * uri: 
    * control: slew rate of asset
    * description: not used
* id: throttle_timeout_fast_ms
    * datatype: double
    * uri: 
    * control: yes
    * description: millisecond timeout for sending info to components. lower timeout value than throttle_timeout_slow_ms for higher send rate
* id: throttle_timeout_slow_ms
    * datatype: double
    * uri: 
    * control: yes
    * description: millisecond timeout for sending info to components. higher timeout value than throttle_timeout_fast_ms for lower send rate
* id: deadband_percent
    * datatype: float
    * uri: 
    * control: yes
    * description: a percent above and below a specific value
* id: running_status_mask
    * datatype: hex string
    * uri: N/A
    * control: no
    * description: bit pattern used to detect running status value sent from components. To support a value, set the bit
    in the position given by the value to high, counting up from 0. Multiple valid values are supported, from 0 to 63. Examples:
    ```
    status: 1     binary mask: 0010 (position 1 true)      hex string: 0x02
    status: 1, 2  binary mask: 0110 (positions 1, 2 true)  hex string: 0x06
### /asset characteristics
                    "id": "ess_1",
                    "name": "BESS Inverter Block 01",
					"watchdog_enable": false,
					"watchdog_timeout_ms": 1500,
                    "rated_active_power_kw": 1250.0,
                    "rated_apparent_power_kva": 1250.0,
                    "rated_reactive_power_kvar": 1250.0,
                    "system_rated_chargeable_power": 1500,
                    "system_rated_dischargeable_power": 1400,
                    "slew_rate": 10000,
					"throttle_timeout_ms": 100,
					"deadband_percent": 0.025,
                    "running_status_mask": 0x02,
### /running mode setpoint array      
* id: siteState
    * datatype: enum/int
    * uri: N/A
    * control: yes
    * description: one of following running modes: {Init, runMode1, runMode2 Shutdown}
* id: demand_control": "Indirect",
    * datatype: 
    * uri: N/A
    * control: 
    * description: 
#### Example
                    "modesInit":
                    [
                        {
                            "siteState":"Init",
                            "demand_control": "Indirect"
                        },
                        {
                            "siteState":"runMode1",
                            "demand_control": "Indirect"
                        },
                        {
                            "siteState":"runMode2",
                            "demand_control": "Indirect"
                        },
                        {
                            "siteState":"shutDown",
                            "demand_control": "Indirect"
                        }
                    ],
### /components                    
                    [
                        {
                            "component_id": "flexgen_ess",
#### /variables                             
                            {
                                "voltage_setpoint":
                                {
                                    "name": "Voltage Setpoint",
                                    "register_id": "voltage_setpoint",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "V"
                                },
                                "frequency_setpoint":
                                {
                                    "name": "Frequency Setpoint",
                                    "register_id": "frequency_setpoint",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "Hz"
                                },
                                "grid_mode":
                                {
                                    "name": "Grid Mode",
                                    "register_id": "control_word_2",
                                    "value": null,
                                    "scaler": 1,
                                    "twins_id":"ctrlword2"
                                },
                                "voltage_dc":
                                {
                                    "name": "ESM DC Link Voltage",
                                    "register_id": "vdc_low",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "V",
                                    "twins_id": "vdc"
                                },
                                "soc":
                                {
                                    "name": "State of Charge",
                                    "register_id": "soc",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "%",
                                    "twins_id": "soc"
                                },
                                "max_temp":
                                {
                                    "name": "Ambient Temp",
                                    "register_id": "ambient_temp",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "C"
                                },
                                "status":
                                {
                                    "name": "Status",
                                    "register_id": "status",
                                    "value": null,
                                    "scaler": null,
                                    "unit": null,
                                    "twins_id": "status"
                                },
                                "alarms":
                                {
                                    "name": "Alarms",
                                    "register_id": "alarms",
                                    "value": null,
                                    "scaler": null,
                                    "unit": null
                                },
                                "faults":
                                {
                                    "name": "Faults",
                                    "register_id": "faults",
                                    "value": null,
                                    "scaler": null,
                                    "unit": null
                                },
                                "voltage_l1_n":
                                {
                                    "name": "L1 AC Voltage",
                                    "register_id": "ac_voltage_l1",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "V",
                                    "twins_id": "v"
                                },
                                "voltage_l2_n":
                                {
                                    "name": "L2 AC Voltage",
                                    "register_id": "ac_voltage_l2",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "V",
                                    "twins_id": "v"
                                },
                                "voltage_l3_n":
                                {
                                    "name": "L3 AC Voltage",
                                    "register_id": "ac_voltage_l3",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "V",
                                    "twins_id": "v"
                                },
                                "current_l1":
                                {
                                    "name": "L1 AC Current",
                                    "register_id": "ac_current_l1",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "A",
                                    "twins_id": "i"
                                },
                                "current_l2":
                                {
                                    "name": "L2 AC Current",
                                    "register_id": "ac_current_l2",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "A",
                                    "twins_id": "i"
                                },
                                "current_l3":
                                {
                                    "name": "L3 AC Current",
                                    "register_id": "ac_current_l3",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "A",
                                    "twins_id": "i"
                                },
                                "active_power":
                                {
                                    "name": "Active Power",
                                    "register_id": "active_power",
                                    "value": null,
                                    "scaler": 1000,
                                    "unit": "W",
                                    "twins_id": "p"
                                },
                                "reactive_power":
                                {
                                    "name": "Reactive Power",
                                    "register_id": "reactive_power",
                                    "value": null,
                                    "scaler": 1000,
                                    "unit": "VAR",
                                    "twins_id": "q"
                                },
                                "apparent_power":
                                {
                                    "name": "Apparent Power",
                                    "register_id": "apparent_power",
                                    "value": null,
                                    "scaler": 1000,
                                    "unit": "VA",
                                    "twins_id": "s"
                                },
                                "frequency":
                                {
                                    "name": "Frequency",
                                    "register_id": "frequency",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "Hz",
                                    "twins_id": "f"
                                },
                                "system_chargeable_power":
                                {
                                    "name": "System Chargeable Power",
                                    "register_id": "system_chargeable_power",
                                    "value": null,
                                    "scaler": 1000,
                                    "unit": "W",
                                    "twins_id": "pcharge"
                                },
                                "system_dischargeable_power":
                                {
                                    "name": "System Dischargeable Power",
                                    "register_id": "system_dischargeable_power",
                                    "value": null,
                                    "scaler": 1000,
                                    "unit": "W",
                                    "twins_id": "pdischarge"
                                },
                                "active_power_setpoint":
                                {
                                    "name": "Active Power Setpoint",
                                    "register_id": "active_power_setpoint",
                                    "value": null,
                                    "scaler": 1000,
                                    "unit": "W",
                                    "twins_id": "pcmd"
                                },
                                "reactive_power_setpoint":
                                {
                                    "name": "Reactive Power Setpoint",
                                    "register_id": "reactive_power_setpoint",
                                    "value": null,
                                    "scaler": 1000,
                                    "unit": "W",
                                    "twins_id": "qcmd"
                                },
								"connected":
								{
									"name": "Modbus Client Connection Status",
									"register_id": "connected",
									"value": null,
									"scaler": null,
									"unit": null
								}
                            },
#### ui controls
                            {
                                "maint_mode":
                                {
                                    "name": "Maintenance Mode",
                                    "register_id": null,
                                    "scaler": null,
                                    "unit": null
                                },
                                "start":
                                {
                                    "name": "Start",
                                    "register_id": "control_word_1",
                                    "scaler": null,
                                    "unit": null,
                                    "twins_id": "ctrlword1"
                                },
                                "stop":
                                {
                                    "name": "Stop",
                                    "register_id": "control_word_1",
                                    "scaler": null,
                                    "unit": null,
                                    "twins_id": "ctrlword1"
                                },
                                "clear_faults":
                                {
                                    "name": "Clear Faults",
                                    "register_id": "clear_faults",
                                    "value": null,
                                    "scaler": null,
                                    "unit": null
                                },
                                "enter_standby":
                                {
                                    "name": "Enter Standby",
                                    "register_id": "standby_mode",
                                    "scaler": null,
                                    "unit": null,
                                    "twins_id": "ctrlword1"
                                },
                                "exit_standby":
                                {
                                    "name": "Exit Standby",
                                    "register_id": "standby_mode",
                                    "scaler": null,
                                    "unit": null,
                                    "twins_id": "ctrlword1"
                                },
                                "close_dc_contactors":
                                {
                                    "name": "Close DC Contactors",
                                    "register_id": "control_word_2",
                                    "scaler": null,
                                    "unit": null,
                                    "twins_id": "ctrlword2"
                                },
                                "open_dc_contactors":
                                {
                                    "name": "Open DC Contactors",
                                    "register_id": "control_word_2",
                                    "scaler": null,
                                    "unit": null,
                                    "twins_id": "ctrlword2"
                                },
                                "maint_active_power_setpoint":
                                {
                                    "name": "Active Power Setpoint",
                                    "register_id": null,
                                    "scaler": 1000,
                                    "unit": "W",
                                    "twins_id": "pcmd"
                                },
                                "maint_reactive_power_setpoint":
                                {
                                    "name": "Reactive Power Setpoint",
                                    "register_id": null,
                                    "scaler": 1000,
                                    "unit": "VAR",
                                    "twins_id": "qcmd"
                                },
								"connected":
								{
									"name": "Connect",
									"register_id": "connected",
									"value": null,
									"scaler": null,
									"unit": null
								}
                            }
                        },
						{
							"component_id": "ess_1_dummy",
							"variables":
							{
								"modbus_heartbeat":
								{
									"name": "Modbus Heartbeat",
									"register_id": "modbus_heartbeat",
									"value": null,
									"scaler": 1,
									"twins_id": "modbus_heartbeat"
								},
								"component_connected":
								{
									"name": "Component Connection Status",
									"register_id": "component_connected",
									"value": null,
									"scaler": null,
									"unit": null
								},
								"modbus_connected":
								{
									"name": "Connection Status",
									"register_id": "modbus_connected",
									"value": null,
									"scaler": null,
									"unit": null
								}
							}
						}
                    ]
                }
            ]
        },
## /solar
### /asset type configuration
* id: start_value
    * datatype: int
    * uri: N/A - value - see ui_controls 'start' variable
    * control: yes
    * description: value transmitted to asset component to start asset
* id: stop_value
    * datatype: int
    * uri: N/A - value
    * control: yes
    * description: value to stop asset
* id: deadband_percentage
    * datatype: float
    * uri:
    * control: yes
    * description: a percent above and below a specific value
#### Example
            "start_value": 1467,
            "stop_value": 1749,
            "deadband_percentage": 0.05,

            "asset_instances":
### /asset characteristics
* id: id
    * datatype: string
    * uri: used as a component in uri construction and as an internal identifier
    * control: no
    * description: identifier used in Asset Manager for asset instance
* id: name": 
    * datatype: string
    * uri: also used as a component in uri construction
    * control: no
    * description: string used for UI display
* id: nominal_voltage
    * datatype: float
    * uri: 
    * control: no
    * description: assets nominal voltage
* id: nominal_frequency
    * datatype: float
    * uri: /components/grid/fcmd
    * control: no
    * description: assets nominal frequrncy
* id: rated_active_power_kw
    * datatype: float
    * uri: /assets/solar_[n]/rated_active_power_kw
    * control: no
    * description: asset rated active power kW 
* id: rated_reactive_power_kvar
    * datatype: float
    * uri: /assets/solar_[n]/rated_reactive_power_kvar
    * control: no
    * description: asset rated reactive power kW - configuration
* id: rated_apparent_power_kva
    * datatype: float
    * uri: /assets/solar_[n]/rated_apparent_power_kva
    * control: no
    * description: asset rated active power kW 
* id: slew_rate
    * datatype: int
    * uri: 
    * control: 
    * description: not used
* id: running_status_mask
    * datatype: hex string
    * uri: N/A
    * control: no
    * description: bit pattern used to detect running status value sent from components. To support a value, set the bit
    in the position given by the value to high, counting up from 0. Multiple valid values are supported, from 0 to 63. Examples:
    ```
    status: 1     binary mask: 0010 (position 1 true)      hex string: 0x02
    status: 1, 2  binary mask: 0110 (positions 1, 2 true)  hex string: 0x06
#### Example
                    "id": "solar_1",
                    "name": "Solar Inverter 01",
                    "nominal_voltage": 480.0,
                    "nominal_frequency": 60.0,
                    "rated_active_power_kw": 65.0,
                    "rated_reactive_power_kvar": 65.0,
                    "rated_apparent_power_kva": 65,
                    "slew_rate": 100,
                    "running_status_mask":0x022,
### /running mode setpoint array
* id: siteState
    * datatype: enum/int
    * uri: N/A
    * control: yes
    * description: one of following running modes: {Init, runMode1, runMode2 Shutdown}
* id: demand_control": "Indirect",
    * datatype: 
    * uri: N/A
    * control: 
    * description:       
#### Example               
                    "modesInit":
                    [
                        {
                            "siteState":"Init",
                            "demand_control": "Indirect"
                        },
                        {
                            "siteState":"runMode1",
                            "demand_control": "Indirect"
                        },
                        {
                            "siteState":"runMode2",
                            "demand_control": "Indirect"
                        },
                        {
                            "siteState":"shutDown",
                            "demand_control": "Indirect"
                        }
                    ],
### /components
                    [
                        {
                            "component_id": "solarpower_1",
#### /variables
                            {
                                "active_power_setpoint":
                                {
                                    "name": "Active Power Limit",
                                    "register_id": "active_power_setpoint",
                                    "value": null,
                                    "scaler": 1000,
                                    "unit": "W",
                                    "twins_id": "plim"
                                },
                                "reactive_power_setpoint":
                                {
                                    "name": "Reactive Power Setpoint",
                                    "register_id": "reactive_power_setpoint",
                                    "value": null,
                                    "scaler": 1000,
                                    "unit": "W",
                                    "twins_id": "qcmd"
                                },
                                "voltage_dc":
                                {
                                    "name": "DC Voltage",
                                    "register_id": "voltage_dc",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "V"
                                },
                                "current_dc":
                                {
                                    "name": "DC Current",
                                    "register_id": "current_dc",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "A"
                                },
                                "clear_faults":
                                {
                                    "name": "Clear Faults",
                                    "register_id": "clear_faults",
                                    "value": null,
                                    "scaler": null,
                                    "unit": null
                                },
                                "status":
                                {
                                    "name": "Status",
                                    "register_id": "status",
                                    "value": null,
                                    "scaler": null,
                                    "unit": null,
                                    "twins_id": "status"
                                },
                                "alarms":
                                {
                                    "name": "Alarms",
                                    "register_id": "alarms",
                                    "value": null,
                                    "scaler": null,
                                    "unit": null
                                },
                                "faults":
                                {
                                    "name": "Faults",
                                    "register_id": "faults",
                                    "value": null,
                                    "scaler": null,
                                    "unit": null
                                },
                                "voltage_l1":
                                {
                                    "name": "L1 AC Voltage",
                                    "register_id": "voltage_l1",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "V",
                                    "twins_id": "v"
                                },
                                "voltage_l2":
                                {
                                    "name": "L2 AC Voltage",
                                    "register_id": "voltage_l2",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "V"
                                },
                                "voltage_l3":
                                {
                                    "name": "L3 AC Voltage",
                                    "register_id": "voltage_l3",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "V"
                                },
                                "current_l1":
                                {
                                    "name": "L1 AC Current",
                                    "register_id": "current_l1",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "A",
                                    "twins_id": "i"
                                },
                                "current_l2":
                                {
                                    "name": "L2 AC Current",
                                    "register_id": "current_l2",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "A",
                                    "twins_id": "i"
                                },
                                "current_l3":
                                {
                                    "name": "L3 AC Current",
                                    "register_id": "current_l3",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "A",
                                    "twins_id": "i"
                                },
                                "active_power":
                                {
                                    "name": "Active Power",
                                    "register_id": "active_power",
                                    "value": null,
                                    "scaler": 1000,
                                    "unit": "W",
                                    "twins_id": "p"
                                },
                                "reactive_power":
                                {
                                    "name": "Reactive Power",
                                    "register_id": "reactive_power",
                                    "value": null,
                                    "scaler": 1000,
                                    "unit": "VAR",
                                    "twins_id": "q"
                                },
                                "apparent_power":
                                {
                                    "name": "Apparent Power",
                                    "register_id": "apparent_power",
                                    "value": null,
                                    "scaler": 1000,
                                    "unit": "VA",
                                    "twins_id": "s"
                                },
                                "frequency":
                                {
                                    "name": "Frequency",
                                    "register_id": "frequency",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "Hz",
                                    "twins_id": "f"
                                },
                                "wind_speed":
                                {
                                    "name": "Wind Speed",
                                    "register_id": "wind_speed",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "mph"
                                },
                                "wind_direction":
                                {
                                    "name": "Wind Direction",
                                    "register_id": "wind_direction",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "degrees"
                                },
                                "air_temp":
                                {
                                    "name": "Air Temperature",
                                    "register_id": "air_temp",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "degrees-Celsius"
                                },
                                "humidity":
                                {
                                    "name": "Relative Humidity",
                                    "register_id": "humidity",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "%"
                                },
                                "array_temp":
                                {
                                    "name": "Array Temperature",
                                    "register_id": "array_temp",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "degrees-Celsius"
                                },
                                "irradiance":
                                {
                                    "name": "Solar Irradiance",
                                    "register_id": "irradiance",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "W/m^2"
                                },
                                "air_pressure":
                                {
                                    "name": "Air Pressure",
                                    "register_id": "air_pressure",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "hPa"
                                },
                                "precip_rate":
                                {
                                    "name": "Precipitation Rate",
                                    "register_id": "precip_rate",
                                    "value": null,
                                    "scaler": 1,
                                    "unit": "in/h"
                                },
                                "weather":
                                {
                                    "name": "Weather",
                                    "register_id": "weather",
                                    "value": null,
                                    "scaler": null,
                                    "unit": null
                                }
                            },
#### ui controls
                             {
                                "maint_mode":
                                {
                                    "name": "Maintenance Mode",
                                    "register_id": null,
                                    "scaler": null,
                                    "unit": null
                                },
                                "start":
                                {
                                    "name": "Start",
                                    "register_id": "start_stop",
                                    "scaler": null,
                                    "unit": null,
                                    "twins_id": "ctrlword1"
                                },
                                "stop":
                                {
                                    "name": "Stop",
                                    "register_id": "start_stop",
                                    "scaler": null,
                                    "unit": null,
                                    "twins_id": "ctrlword1"
                                },
                                "clear_faults":
                                {
                                    "name": "Clear Faults",
                                    "register_id": "clear_faults",
                                    "value": null,
                                    "scaler": null,
                                    "unit": null
                                },
                                "maint_active_power_setpoint":
                                {
                                    "name": "Active Power Setpoint",
                                    "register_id": null,
                                    "scaler": 1000,
                                    "unit": "W",
                                    "twins_id": "pcmd"
                                },
                                "maint_reactive_power_setpoint":
                                {
                                    "name": "Reactive Power Setpoint",
                                    "register_id": null,
                                    "scaler": 1000,
                                    "unit": "VAR",
                                    "twins_id": "qcmd"
                                }
                            }
                        }
                    ]
                }
            ]
        }
    }
}
