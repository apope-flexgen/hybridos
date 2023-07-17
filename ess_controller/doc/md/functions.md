ESS Controller Functions

Author: Jimmy Nguyen

Date Created: 
* 02/26/2021

Date Modified: 

# Overview
Internally, the ESS Controller maintains a function map that contains the name of the function to use and the asset name (ex.: ess, bms, pcs, sbmu) that the function is associated with. The function names can be defined in the configuration file, and if the function exists in the ESS Controller's function map, then the function can run.

Functions that are in the data map can run in three different ways:
* Scheduler
  * The function will be called either continuously or at a fixed number of times
  * Ex.:
```json
"/sched/pcs": {
    "schedStartupPCS":{
        "value":    "schedStartupPCS",
        "actions":{
            "onSet": [{ 
                "func": [
                    {"func": "HandleSchedLoad", "amap": "pcs"}
                ]
            }]
        },
        "uri":      "/sched/pcs:schedStartupPCS",
        "fcn":      "StartupPCS",
        "id":       "schedStartupPCS",
        "amap":     "pcs",
        "refTime":  0.269,
        "runAfter": 0.270,
        "repTime":  0.100,
        "endTime":  0.001
    }
}
```

* Wake monitor
  * The function will be called continuously by the ESS Controller
  * Note:
    * `enable` determines whether the function should run periodically for a specific variable
    * `rate` is the time to wait before making another function call (not used at the moment)
    * `func` allows the ESS Controller to periodically call the function (ex.: `CalculateVar`)
  * Ex.:
```json
"/schedule/wake_monitor/pcs":{
    "/components/pcsm_dc_inputs:vdc_bus_1": { "enable": true, "rate":1, "func":"CheckMonitorVar"},
    "/components/pcsm_general:seconds": { "enable": true, "rate":1, "func":"CheckMonitorVar"},
    "/status/pcs:ActivePowerCmd_adjusted": { "enable": true, "rate":0.1, "func":"CalculateVar"},
    "/status/pcs:ReactivePowerCmd_adjusted": { "enable": true, "rate":0.1, "func":"CalculateVar"}
}
```

* In response to a fims set/pub
  * The function will be called once every time there is a fims set/pub on the variable
  * Note:
    * `actions` and `onSet` will allow the ESS Controller to respond if the value of the variable we're working with has changed (via fims set or pub).  
    * `func` allows the ESS Controller to call the function (ex.: `CheckMonitorVar`) if the variable has changed
    * `amap` specifies the data map to use in the function (ex.: `pcs`)  
  * Ex.:
```json
"/components/pcsm_general": {
    "seconds": {
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
                            "amap": "pcs"
                        }
                    ]
                }
            ]
        }
    }
}
```

----

## Functions
Here are the list of functions available in the ESS Controller that can be included in the config:
* `CheckMonitorVar`
  * Description: Monitors the variable. Sends out alarm and/or shutdown the system if abnormal value is found after certain amount of time
  * Available to: ess, bms, pcs, sbmu
  * Reference: [monitoring.md](monitoring.md)
* `CalculateVar`
  * Description: Perform math operations and sets the results to the variable of interest
  * Available to: ess, bms, pcs, sbmu
  * Reference: [ess_calculator.md](ess_calculator.md)
* `MathMovAvg`
  * Description: Extracts moving Average data from an incoming data value.
  * Available to: ess, bms, pcs, sbmu
  * Reference: [MathMovAvg.md](MathMovAvg.md)
* `process_sys_alarm`
  * Description: Process the system faults and alarms read by the ESS Controller. Clears fault/alarm messages if the `clear_faults` variable is set to `Clear`
  * Available to: ess, bms, pcs, sbmu
  * Reference: [alarms.md](alarms.md)
* `StartupPCS`
  * Description: Startup the PCS by the start command from the UI or fims
  * Avaliable to: pcs
* `ShutdownPCS`
  * Description: Shutdown the PCS either by the shutdown command from the UI or fims or by a fault condition from the ESS Controller
  * Available to: pcs
* `StartupBMS`
  * Description: Startup the BMS by the start command from the UI or fims
  * Avaliable to: bms
* `ShutdownBMS`
  * Description: Shutdown the BMS either by the shutdown command from the UI or fims or by a fault condition from the ESS Controller
  * Available to: bms
* `SendClearFaultCmd`
  * Description: Sends out a clear fault command to the hardware device modbus register
  * Available to: bms, pcs, gpio