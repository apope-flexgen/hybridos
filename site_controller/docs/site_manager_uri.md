Site Manager URI Documentation

Author: Kyle Brezina

Date Created: 03/11/19

Date Modified: 03/11/19


# /site

* id: local_remote_source_flag
  * datatype: bool
  * uri: /site/operation
  * control: yes
  * description: determines if remote (modbus) or local (UI) controller has control

* id: local_disable_flag
  * datatype: bool
  * uri: /site/operation
  * control: yes
  * description: local controller site disable control

* id: local_enable_flag
  * datatype: bool
  * uri: /site/operation
  * control: yes
  * description: local controller site enable control

* id: local_clear_faults
  * datatype: bool
  * uri: /site/operation
  * control: yes
  * description: local controller site clear faults control

* id: site_state
  * datatype: string
  * uri: /site/operation, /site/summary
  * control: no
  * description: the current state string name (Init, Ready, Startup, Running, Shutdown, Error)

* id: running_status_flag
  * datatype: bool
  * uri: /site/operation
  * control: no
  * description: boolean indicator for whether or not site state is Running

* id: local_remote_source_status
  * datatype: string
  * uri: /site/operation, /site/summary
  * control: no
  * description: string representation of which (remote (modbus) or local (UI)) controller has control

* id: present_disable_flag
  * datatype: bool
  * uri: /site/operation, /site/summary
  * control: no
  * description: status of site disable control (either remote or local)
  
* id: present_enable_flag
  * datatype: bool
  * uri: /site/operation, /site/summary
  * control: no
  * description: status of site enable control (either remote or local)

* id: present_clear_faults
  * datatype: bool
  * uri: /site/operation, /site/summary
  * control: no
  * description: status of site clear faults control (either remote or local)
  
* id: remote_disable_flag
  * datatype: bool
  * uri: /site/operation
  * control: yes 
  * description: remote controller site disable control

* id: remote_enable_flag
  * datatype: bool
  * uri: /site/operation
  * control: yes
  * description: remote controller site enable control

* id: remote_clear_faults
  * datatype: bool
  * uri: /site/operation
  * control: yes
  * description: remote controller site enable control
  
* id: ESS_active_power_cmd
  * datatype: float
  * uri: /site/operation, /site/summary
  * control: no
  * description: site active power command sent to ESS assets

* id: active_faults
  * datatype: enum/int
  * uri: /site/operation, /site/summary
  * control: no
  * description: array of strings identifying active or latched site faults (faults cause site shutdown)

* id: active_alarms
  * datatype: enum/int
  * uri: /site/operation, /site/summary
  * control: no
  * description: array of strings identifying active or latched site alarms (alarms do not cause site shutdown)
  
* id: fault_status_flag
  * datatype: bool
  * uri: /site/operation
  * control: no
  * description: boolean indication of whether any site faults are active or latched

* id: alarm_status_flag
  * datatype: bool
  * uri: /site/operation
  * control: no
  * description: boolean indication of whether any site alarms are active or latched

* id: ESS_instant_discharge
  * datatype: float
  * uri: /site/summary
  * control: no
  * description: KPI calculation, total instantaneous active power in discharge direction
  
* id: ESS_instant_charge_grid
  * datatype: float
  * uri: /site/summary
  * control: no
  * description: KPI calculation, total instantaneous active power in charge direction from grid
  
* id: ESS_instant_charge_pv
  * datatype: float
  * uri: /site/summary
  * control: no
  * description: KPI calculation, total instantaneous active power in charge direction from PV/solar

# /features

## /features/energy_arbitrage

* id: local_threshold_charge_2
  * datatype: float
  * uri: /features/energy_arbitrage
  * control: yes
  * description: local controller max charge price threshold
  
* id: local_threshold_charge_1
  * datatype: float
  * uri: /features/energy_arbitrage
  * control: yes
  * description: local controller normal charge price threshold

* id: local_threshold_dischg_1
  * datatype: float
  * uri: /features/energy_arbitrage
  * control: yes
  * description: local controller normal discharge price threshold

* id: local_threshold_dischg_2
  * datatype: float
  * uri: /features/energy_arbitrage
  * control: yes
  * description: local controller max discharge price threshold
  
* id: present_threshold_charge_2
  * datatype: float
  * uri: /features/energy_arbitrage, features/summary
  * control: no
  * description: status of max charge price threshold (either remote or local)
  
* id: present_threshold_charge_1
  * datatype: float
  * uri: /features/energy_arbitrage, features/summary
  * control: no
  * description: status of normal charge price threshold (either remote or local)

* id: present_threshold_dischg_1
  * datatype: float
  * uri: /features/energy_arbitrage, features/summary
  * control: no
  * description: status of normal discharge price threshold (either remote or local)

* id: present_threshold_dischg_2
  * datatype: float
  * uri: /features/energy_arbitrage, features/summary
  * control: no
  * description: status of max discharge price threshold (either remote or local)		

* id: remote_threshold_charge_2
  * datatype: float
  * uri: /features/energy_arbitrage
  * control: yes
  * description: remote controller max charge price threshold
  
* id: remote_threshold_charge_1
  * datatype: float
  * uri: /features/energy_arbitrage
  * control: yes
  * description: remote controller normal charge price threshold

* id: remote_threshold_dischg_1
  * datatype: float
  * uri: /features/energy_arbitrage
  * control: yes
  * description: remote controller normal discharge price threshold

* id: remote_threshold_dischg_2
  * datatype: float
  * uri: /features/energy_arbitrage
  * control: yes
  * description: remote controller max discharge price threshold
  
* id: soc_avg_running
  * datatype: float
  * uri: /features/energy_arbitrage
  * control: no
  * description: average state of charge for running ESS assets

* id: price
  * datatype: float
  * uri: /features/energy_arbitrage
  * control: yes
  * description: current price ($/MWh) supplied by remote controller

* id: avail_ESS_charge_kW
  * datatype: float
  * uri: /features/energy_arbitrage
  * control: no
  * description: maximum charge power available from ESS assets
  
* id: avail_ESS_dischg_kW
  * datatype: float
  * uri: /features/energy_arbitrage
  * control: no
  * description: maximum discharge power available from ESS assets

* id: soc_min_running
  * datatype: float
  * uri: /features/energy_arbitrage
  * control: no
  * description: minimum state of charge for running ESS assets

* id: soc_max_running
  * datatype: float
  * uri: /features/energy_arbitrage
  * control: no
  * description: maximum state of charge for running ESS assets
  
* id: soc_min_limit
  * datatype: float
  * uri: /features/energy_arbitrage
  * control: runtime
  * description: minimum state of charge limit for energy arbitrage normal discharge price threshold
  
* id: soc_max_limit
  * datatype: float
  * uri: /features/energy_arbitrage
  * control: runtime
  * description: maximum state of charge limit for energy arbitrage normal charge price threshold

* id: _energy_arb_power
  * datatype: float
  * uri: /features/energy_arbitrage
  * control: no
  * description: internal variable, active power command after energy arbitrage algorithm

## /features/manual_mode

* id: local_manual_flag
  * datatype: bool
  * uri: /features/manual_mode
  * control: yes
  * description: local controller manual or automatic mode control

* id: local_manual_status
  * datatype: string
  * uri: /features/manual_mode
  * control: no 
  * description: local controller string representation (Manual, Automatic) for manual or automatic mode

* id: local_manual_power
  * datatype: float
  * uri: /features/manual_mode
  * control: yes
  * description: local controller manual power numeric control
  
* id: local_chg_dischg_flag
  * datatype: bool
  * uri: /features/manual_mode
  * control: yes
  * description: local controller charge/discharge direction for manual power numeric control

* id: local_chg_dischg_status
  * datatype: string
  * uri: /features/manual_mode
  * control: no
  * description: local controller string representation (Charge, Discharge) for charge/discharge direction of manual power numeric control

* id: present_manual_flag
  * datatype: bool
  * uri: /features/manual_mode
  * control: no
  * description: manual or automatic mode control (either remote or local)
  
* id: present_manual_status
  * datatype: string
  * uri: /features/summary
  * control: no
  * description: string representation (Manual, Automatic) for manual or automatic mode (either remote or local)
  
* id: present_manual_power
  * datatype: float
  * uri: /features/manual_mode
  * control: no
  * description: manual power numeric control (either remote or local)

* id: present_chg_dischg_flag
  * datatype: bool
  * uri: /features/manual_mode
  * control: no
  * description: charge/discharge direction for manual power numeric control (either remote or local)

* id: present_chg_dischg_status
  * datatype: string
  * uri: /features/manual_mode
  * control: no
  * description: string representation (Charge, Discharge) for charge/discharge direction of manual power numeric control (either remote or local)

* id: remote_manual_flag
  * datatype: bool
  * uri: /features/manual_mode
  * control: yes
  * description: remote controller manual or automatic mode control

* id: remote_manual_status
  * datatype: string
  * uri: /features/manual_mode
  * control: no 
  * description: remote controller string representation (Manual, Automatic) for manual or automatic mode

* id: remote_manual_power
  * datatype: float
  * uri: /features/manual_mode
  * control: yes
  * description: remote controller manual power numeric control
  
* id: remote_chg_dischg_flag
  * datatype: bool
  * uri: /features/manual_mode
  * control: yes
  * description: remote controller charge/discharge direction for manual power numeric control

* id: remote_chg_dischg_status
  * datatype: string
  * uri: /features/manual_mode
  * control: no
  * description: remote controller string representation (Charge, Discharge) for charge/discharge direction of manual power numeric control	

## /features/curtailment

* id: curtailment_enable_flag
  * datatype: bool
  * uri: /features/curtailment, /features/summary
  * control: yes
  * description: control to enable curtailment algorithm
  
* id: curtailment_target
  * datatype: float
  * uri: /features/curtailment, /features/summary
  * control: yes
  * description: site active power maximum output limit, used in site power limit algorithms when curtailment is enabled

* id: inverter_output_percent
  * datatype: float
  * uri: /features/curtailment
  * control: yes
  * description: percent of utilized output capacity for site solar production, used in curtailment algorithm
  
* id: curtailment_price
  * datatype: float
  * uri: /features/curtailment
  * control: runtime
  * description: price used for "free power check" in curtailment algorithm

* id: curtailment_enable_feedback_flag
  * datatype: bool
  * uri: /features/curtailment
  * control: no
  * description: feedback for curtailment_enable_flag

* id: curtailment_status_flag
  * datatype: bool
  * uri: /features/curtailment
  * control: no
  * description: indication of whether curtailment algorithm has impacted ESS_active_power_cmd
  
* id: interconnect_max_power
  * datatype: float
  * uri: /features/curtailment
  * control: runtime
  * description: maximum site active power limit
  
* id: interconnect_active_power
  * datatype: float
  * uri: /features/curtailment
  * control: yes
  * description: instantaneous site active power,  written from remote controller for limit algorithms
  
* id: interconnect_avail_power
  * datatype: float
  * uri: .features/curtailment
  * control: no
  * description: calculated available active power that site can produce before reaching interconnect_max_power limit
  
* id: _slew_free_power
  * datatype: float
  * uri: /features/curtailment
  * control: no
  * description: internal variable, slewed free active power calculation

## /features/frequency_droop

* id: frequency_droop_enable_flag
  * datatype: bool
  * uri: /features/frequency_droop
  * control: yes
  * description: control to enable frequency droop algorithm
  
* id: frequency_droop_target
  * datatype: float
  * uri: /features/frequency_droop
  * control: yes
  * description: site active power maximum output limit, used in site power limit algorithms when frequency_droop is enabled
  
* id: frequency_droop_enable_feedback_flag
  * datatype: bool
  * uri: /features/frequency_droop
  * control: no
  * description: feedback for frequency_droop_enable_flag
  
* id: frequency_droop_status_flag
  * datatype: bool
  * uri: /features/frequency_droop
  * control: no
  * description: indication of whether frequency droop algorithm has impacted ESS_active_power_cmd
  
* id: pfr_status_flag
  * datatype: bool
  * uri: /features/frequency_droop, /features/summary
  * control: no
  * description: indication of whether PFR algorithm has impacted ESS_active_power_cmd
  
* id: _pfr_power
  * datatype: float
  * uri: /features/frequency_droop
  * control: no
  * description: internal variable, active power command after PFR algorithm
  
* id: avr_target_voltage
  * datatype: float
  * uri: /features/frequency_droop, /features/summary
  * control: yes
  * description: Automatic Voltage Response target voltage written by remote controller and passed to ESS assets for reactive power compensation