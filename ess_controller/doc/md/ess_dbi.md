ESS Controller Database Interface

Author: Jimmy Nguyen

Date Created: 
* 03/26/2021

Date Modified: 

# Overview
The ESS Controller is able to save and recover data from a database, which is managed by the database interface (dbi) module.  

On system startup, the ESS Controller can retrieve a particular data point from the dbi, and if the data exists, then that data will be used for initialization. After system startup, the ESS Controller can continue to send updates to the dbi after a certain period of time.   

For more information on how to use the dbi, check out the [dbi documentation](https://github.com/flexgen-power/dbi/blob/dev/documentation/Database_Interface_documentation.md). 

Note: All dbi data items used by the ESS Controller will be stored in `/dbi/ess_controller`.

# Configuration
Here is an example of how to configure the variables to be saved and retrieved from the dbi:

```json
"/controls/bms" : {
    "DemoChargeCurrent": {
        "value" :0.0,
        "dbiStatus":"init",
        "EnableDbiUpdate": true,
        "UpdateTimeCfg": 5,
        "actions": {
            "onSet": [
                {"func": [{"func": "CheckVar", "amap": "bms"}]}
            ]
        }
    }
},
```
In the example above, `/controls/bms:DemoChargeCurrent` is the data item that we would like to store in the database. All of the values, parameters, and other configuration items associated with the data item will be stored and periodically updated in the database.  

```json
"/dbi/controls/bms" : {
    "DemoChargeCurrent": {
        "value" :-999999,
        "dbiSet": false
    }
},
```
In the example above, `/dbi/controls/bms:DemoChargeCurrent` is the dbi response variable. The response variable will either contain the data item retrieved from the dbi on system startup or the data item to update the variable referencing the dbi.  

```json
"/schedule/wake_monitor/bms": {
    "/controls/bms:DemoChargeCurrent": { "enable": true, "rate":0.1, "amap": "bms", "func":"CheckDbiVar"},
    "/dbi/controls/bms:DemoChargeCurrent": { "enable": true, "rate":0.1, "amap": "bms", "func":"CheckDbiResp"}
}
```
In the example above, both `/controls/bms:DemoChargeCurrent` and `/dbi/controls/bms:DemoChargeCurrent` will run a function periodically using the ESS Controller wake monitor system.
* `CheckDbiVar` is used to monitor the variable referencing the dbi (`/control/bms:DemoChargeCurrent`) and send an update to the dbi after a certain period of time
    * Parameters initialized/used by variable referencing dbi:
        * `EnableDbiUpdate`- enable/disable update to dbi for a paricular variable
            * Defaults to true if not defined in config
        * `UpdateTimeCfg` - the configurable amount of time to take before sending an update to dbi
            * Defaults to 5 seconds if not defined in config
        * `UpdateTimeRemain` - the remaining amount of time left before sending an update to dbi
        * `dbiStatus` - the current state of the dbi variable
            * `init` - on system startup, send a fims get request to the dbi to recover data
            * `OK` - system startup is complete, so periodically send update to dbi
        * `tLast` - the last time that was set for the variable
        
* `CheckDbiResp` is used to monitor the dbi response variable and update the variable referencing the dbi
    * Parameters initialized/used by variable referencing dbi:
        * `dbiSet` - update the variable referencing the dbi if set to true
            * Set to true if only parameters and other config items for the variable referencing the dbi needs to be updated
            * Can be omitted if only the `value` of the variable needs to be updated

# Data Validation
While the ESS Controller is running, to check the variables stored in the ESS Controller and the dbi, you can do the following:
1. Fims interface  
    * `/usr/local/bin/fims/fims_send -m get -u /ess/full/controls/bms -r /me`
    * `/usr/local/bin/fims/fims_send -m get -u /ess/full/dbi/controls/bms -r /me`
    * `/usr/local/bin/fims/fims_send -m get -u /dbi/ess_controller/controls/bms -r /me`

To update the dbi, you can do the following:
1. Fims interface
    * `/usr/local/bin/fims/fims_send -m set -u /ess/dbi/controls/bms '{"DemoChargeCurrent":{"value":2,"UpdateTimeCfg":7}}' -r /me`
        * This will update the variable referencing the dbi, which will then update the dbi perodically (if `EnableDbiUpdate` is true for the variable referencing the dbi)
    * `/usr/local/bin/fims/fims_send -m get -u /dbi/ess_controller/controls/bms -r /ess/dbi/controls/bms`  
        * This will redirect the data retrieved from the dbi (`/dbi/ess_controller/controls/bms`) to the dbi response variable (`/ess/dbi/controls/bms`)
    * `/usr/local/bin/fims/fims_send -m set -u /dbi/ess_controller/controls/bms/DemoChargeCurrent/UpdateTimeCfg '8'`
        * This will directly update the data stored in the dbi (see dbi documentation for more info on how to update the dbi data)
        * If `EnableDbiUpdate` is set to true for the variable referencing the dbi, then the direct changes to the dbi will be overwritten. To avoid this, either terminate the ESS Controller running process or set `EnableDbiUpdate` to false and then redirect the data in the dbi to the dbi response variable