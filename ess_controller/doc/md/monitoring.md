System Monitoring

Author: Phil Wilshire, Jimmy Nguyen

Date Created: 
* 01/06/2021

Date Modified: 
* 02/26/2021 - updated documentation for existing sections; added section on communications check
* 01/29/2021 - added details on configuration (how to set/get the monitor variable via fims)
* 01/28/2021 - added more description for limits and states  
* 01/20/2021 - added more documentation for monitor configs

# Overview
As part of a protection strategy, the ESS Controller provides a way to monitor a list of variables, send out alarms, and
shutdown the system if the variable reaches over/under the protection thresholds.

There are two ways the ESS Controller can keep track of the monitor variable:
* Run the check monitoring function periodically
* Run the check monitoring function whenever the ESS Controller detects a fims set or pub to the monitor variable
  * In other words, any changes in the value of the monitor variable will cause the check monitor function to be called



# Configuration
Here are the different ways the variable can be monitored:

1) Limits (check if value > max threshold)
```json
 "/components/catl_mbmu_control_r": {
    "mbmu_max_cell_voltage": {
        "value": 0,
        "EnableFaultCheck": true,
        "EnableMaxValCheck": true,
        "MaxAlarmThresholdValue": 25.4,
        "MaxFaultThresholdValue": 28,
        "MaxResetValue": 22.4,
        "MaxAlarmTimeout": 2.5,
        "MaxFaultTimeout": 5.5,
        "MaxRecoverTimeout": 1.4,
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "func": "CheckMonitorVar",
                            "amap": "bms"
                        }
                    ]
                }
            ]
        }
    },
 }
```

2) Limits (check if value < min threshold)
```json
 "/components/catl_mbmu_control_r": {
    "mbmu_min_cell_voltage": {
        "value": 0,
        "EnableFaultCheck": true,
        "EnableMinValCheck": true,
        "MinAlarmThresholdValue": 25.4,
        "MinFaultThresholdValue": 28,
        "MinResetValue": 22.4,
        "MinAlarmTimeout": 2.5,
        "MinFaultTimeout": 5.5,
        "MinRecoverTimeout": 1.4,
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "func": "CheckMonitorVar",
                            "amap": "bms"
                        }
                    ]
                }
            ]
        }
    },
 }
```

3) Limits (check if value > max threshold or value < min threshold)
```json
    "/components/pcsm_dc_inputs": {
        "vdc_bus_1": {
            "value": 0,
			
            "EnableFaultCheck": true,
            "EnableMaxValCheck": true,
            "EnableMinValCheck": true,

            "MinAlarmThreshold": 875,
            "MaxAlarmThreshold": 3000,
            "MinFaultThreshold": 850,
            "MaxFaultThreshold": 4000,
            "MinResetValue": 875,
            "MaxResetValue": 1000,

            "MinAlarmTimeout": 1.1,
            "MaxAlarmTimeout": 1.1,
            "MinFaultTimeout": 1.1,
            "MaxFaultTimeout": 1.1,
            "MinRecoverTimeout": 1.1,
            "MaxRecoverTimeout": 1.1,
            "actions": {
                "onSet": [
                    {
                        "func": [
                            {
                                "func": "CheckMonitorVar",
                                "amap": "pcs"
                            }
                        ]
                    }
                ]
            }
        }
    },
```

4) States (check if value is changing)
```json
    "/site/ess_hs": {
        "life": {
            "value": 0,
            "EnableStateCheck": true,
            "Type": "int",
            "AlarmTimeout": 5,
            "FaultTimeout": 10,
            "RecoverTimeout": 1,
            "actions": {
                "onSet": [
                    {
                        "func": [
                            {
                                "func": "CheckMonitorVar",
                                "amap": "site"
                            }
                        ]
                    }
                ]
            }
        }
    },
```
Note:
* `actions` and `onSet` will allow the ESS Controller to respond if the value of the variable we're working with has changed (via fims set or pub).  
* `func` allows the ESS Controller to call the function (ex.: `CheckMonitorVar`) if the variable has changed
* `amap` specifies the data map to use in the function (ex.: `site`)  

Here is a way to allow the function to run periodically, even with no changes to the variable:  
```json
"/schedule/wake_monitor/bms":{
        "/components/catl_mbmu_control_r:mbmu_max_cell_voltage": { "enable": true, "rate":0.1, "func":"CheckMonitorVar"},
        "/components/catl_mbmu_control_r:mbmu_min_cell_voltage": { "enable": true, "rate":0.1, "func":"CheckMonitorVar"},
        "/components/catl_mbmu_control_r:mbmu_max_cell_temperature": { "enable": true, "rate":0.1, "func":"CheckMonitorVar"},
        "/components/catl_mbmu_control_r:mbmu_min_cell_temperature": { "enable": true, "rate":0.1, "func":"CheckMonitorVar"},
        "/components/catl_mbmu_control_r:mbmu_soc": { "enable": true, "rate":0.1, "func":"CheckMonitorVar"},
        "/components/catl_mbmu_control_r:mbmu_soh": { "enable": true, "rate":0.1, "func":"CheckMonitorVar"}
    },
```
Note:
* `enable` determines whether the function should run periodically for a specific variable
* `rate` is the time to wait before making another function call (not used at the moment)
* `func` allows the ESS Controller to periodically call the function (ex.: `CheckMonitorVar`)

----

## Communications Check
The monitoring function also allows a way to check whether the ESS Controller is communicating with other devices, such as the PCS, BMS, Site Controller, etc.  
If the variable we are monitoring is no longer changing, then the ESS Controller should send out an alarm to the web interface and the Site Controller (if the communication failure is not from the Site Controller) and shutdown the system.

Here is an example of how to configure communication check for a monitor variable:
```json
    "/site/ess_hs": {
        "life": {
            "value": 0,
            "EnableStateCheck": true,
            "EnableCommsCheck": true,
            "Type": "int",
            "AlarmTimeout": 5,
            "FaultTimeout": 10,
            "RecoverTimeout": 1,
            "actions": {
                "onSet": [
                    {
                        "func": [
                            {
                                "func": "CheckMonitorVar",
                                "amap": "site"
                            }
                        ]
                    }
                ]
            }
        }
    },
```

The `EnableCommsCheck` parameter is used to indicate whether the variable should be related to communications testing (ex.: checking if the ESS Controller can communicate with the BMS).  
* This is an optional parameter.

# Implementation

## Limits
The monitor function will check if the current value is over or under the threshold value.  
An alarm or a fault will be raised if the value stays above or below the alarm/fault threshold value for the duration of the alarm/fault time.  
A reset is raised (indicating return to normal operation) if the value is above/below the reset value and the monitored variable is already in an alarm and/or a fault state (ex.: `seenFault` and/or `seenAlarm`).

To enable limit check, set `EnableMaxValCheck` and/or `EnableMinValCheck` to true.  
To enable fault check (and allow the system to initiate shutdown process after fault condition), set `EnableFaultCheck` to true.

### Parameters
* `EnableFaultCheck` - enable/disable fault for a specific variable
* `EnableMaxValCheck` - enable/disable check for value > threshold
* `EnableMinValCheck` - enable/disable check for value < threshold
* `MaxAlarmThreshold` - if value > alarm threshold, an alarm is sent out
* `MinAlarmThreshold` - if value < alarm threshold, an alarm is sent out
* `MaxFaultThreshold` - if value > fault threshold, a fault is raised
* `MinFaultThreshold` - if value < fault threshold, a fault is raised
* `MinResetValue` - if value > reset val, alarm/fault is cleared
* `MaxResetValue` - if value < reset val, alarm/fault is cleared
* `MinAlarmTimeout` - the amount of time before sending out alarm if value < alarm threshold
* `MaxAlarmTimeout` - the amount of time before sending out alarm if value > alarm threshold
* `MinFaultTimeout` - the amount of time before raising a fault if value < fault threshold
* `MaxFaultTimeout` - the amount of time before raising a fault if value > fault threshold
* `MinRecoverTimeout` - the amount of time before clearing alarm/fault if value > reset val
* `MaxRecoverTimeout` - the amount of time before clearing alarm/fault if value < reset val
* `AlarmCnt` - the # of alarms that occurred for the variable                                       (not defined in config)
* `FaultCnt` - the # of faults that occurred for the variable                                       (not defined in config)
* `MinAlarmTime` - the remaining amount of time before sending out alarm if value < alarm threshold (not defined in config)
* `MaxAlarmTime` - the remaining amount of time before sending out alarm if value > alarm threshold (not defined in config)
* `MinFaultTime` - the remaining amount of time before raising a fault if value < fault threshold   (not defined in config)
* `MaxFaultTime` - the remaining amount of time before raising a fault if value > fault threshold   (not defined in config)
* `MinRecoverTime` - the remaining amount of time before clearing alarm/fault if value > reset val  (not defined in config)
* `MaxRecoverTime` - the remaining amount of time before clearing alarm/fault if value < reset val  (not defined in config)
* `seenMinAlarm` - indicates whether alarm has been seen for value < alarm threshold                (not defined in config)
* `seenMaxAlarm` - indicates whether alarm has been seen for value > alarm threshold                (not defined in config)
* `seenMinFault` - indicates whether fault has been seen for value < fault threshold                (not defined in config)
* `seenMaxFault` - indicates whether fault has been seen for value > fault threshold                (not defined in config)
* `seenMinAlarm` - indicates whether reset has been seen for value > reset val                      (not defined in config)
* `seenMaxAlarm` - indicates whether reset has been seen for value < reset val                      (not defined in config)
* `tLast` - the last time that was set for the variable                                             (not defined in config)

## State
The monitor function will check if the variable is constantly changing.  
An alarm or a fault will be raised if the value stays the same for the duration of the alarm/fault time.  
A reset is raised (indicating return to normal operation) if the value is changing (val != lastVal) and the monitored variable is already in an alarm and/or a fault state (ex.: `seenFault` and/or `seenAlarm`).  

The `Type` parameter is used to specify the data type of the variable.  
* If this parameter is left out of the config, then the default `Type` will be `int`

To enable state check, set `EnableStateCheck` to true.  

### Parameters
* `EnableStateCheck` - enable/disable state check for a specific variable
* `EnableCommsCheck` - enable/disable communications check for a specific variable
  * Note: `EnableStateCheck` must be `true` in order to allow the ESS Controller to perform additional actions related to communications failure
* `Type` - the variable data type (int, double, and string types are supported at the moment)
* `AlarmTimeout` - the amount of time before sending out alarm if value == last value
* `FaultTimeout` - the amount of time before raising a fault if value == last value
* `RecoverTimeout` - the amount of time before clearing alarm/fault if value != last value
* `AlarmCnt` - the # of alarms that occurred for the variable                                       (not defined in config)
* `FaultCnt` - the # of faults that occurred for the variable                                       (not defined in config)
* `AlarmTime` - the remaining amount of time before sending out alarm if value == last value        (not defined in config)
* `FaultTime` - the remaining amount of time before raising a fault if value == last value          (not defined in config)
* `RecoverTime` - the remaining amount of time before clearing alarm/fault if value != last value   (not defined in config)
* `seenAlarm` - indicates whether alarm has been seen for value == last value                       (not defined in config)
* `seenFault` - indicates whether fault has been seen for value == last value                       (not defined in config)
* `seenReset` - indicates whether reset has been seen for value != last value                       (not defined in config)
* `lastVal` - the last value that was read by the ess controller                                    (not defined in config)
* `tLast` - the last time that was set for the variable                                             (not defined in config)

# Data Validation
While the ESS Controller is running, to change the parameter(s) of the variable you are monitoring, you can do the following example:
1. Fims interface
    * `/usr/local/bin/fims/fims_send -m set -u /ess/components/catl_mbmu_control_r '{"mbmu_max_cell_voltage":{"value":0}}'`
    * `/usr/local/bin/fims/fims_send -m set -u /ess/components/catl_mbmu_control_r '{"mbmu_max_cell_voltage":{"EnableFaultCheck":false}}'`
    * `/usr/local/bin/fims/fims_send -m set -u /ess/components/catl_mbmu_control_r '{"mbmu_max_cell_voltage":{"EnableMaxValCheck":false}}'`
    * `/usr/local/bin/fims/fims_send -m set -u /ess/components/catl_mbmu_control_r '{"mbmu_max_cell_voltage":{"MaxAlarmThreshold":25.4}}'`
    * `/usr/local/bin/fims/fims_send -m set -u /ess/components/catl_mbmu_control_r '{"mbmu_max_cell_voltage":{"MaxFaultThreshold":28}}'`
    * `/usr/local/bin/fims/fims_send -m set -u /ess/components/catl_mbmu_control_r '{"mbmu_max_cell_voltage":{"MaxResetValue":22.4}}'`
    * `/usr/local/bin/fims/fims_send -m set -u /ess/components/catl_mbmu_control_r '{"mbmu_max_cell_voltage":{"MaxAlarmTimeout":2.5}}'`
    * `/usr/local/bin/fims/fims_send -m set -u /ess/components/catl_mbmu_control_r '{"mbmu_max_cell_voltage":{"MaxFaultTimeout":5.5}}'`
    * `/usr/local/bin/fims/fims_send -m set -u /ess/components/catl_mbmu_control_r '{"mbmu_max_cell_voltage":{"MaxRecoverTimeout":1.4}}'`
    * `/usr/local/bin/fims/fims_send -m set -u /ess/reload/bms '{"mbmu_max_cell_voltage_reload":0}'`
        * Note: Set the reload value < 0 to allow the variable to monitor to reset its parameters (ex.: `AlarmCnt`, `AlarmTime`)

To view value and all the parameter(s) you are monitoring, you can do the following example:
1. Fims interface
    * `/usr/local/bin/fims/fims_send -m get -u /ess/full/components/catl_mbmu_control_r/mbmu_max_cell_voltage -r /me | jq`