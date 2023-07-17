ESS Controller Configuration Options and Features

Author: Jimmy Nguyen

Date Created: 
* 03/09/2021

Date Modified: 

# Overview
In the ESS Controller configuration, there are various options and features that can be defined for a specific variable to perform a specific task during runtime. For example, a variable can have the `remap` option can be used to set the target variable to a specific value in response to a fims set/pub. The `remap` option can use the `offset` feature to scale the value to set to the target variable as well as the `uri` feature to define the location and name of the target variable to set the value to.

Note that the options and features presented below are part of the `onSet` action.  

# Configuration
Here are the list of actions and associated options available in the ESS Controller that can be included in the config:
* `remap`
  * Description: Sets the target variable to a new value in response to a fims set/pub
  * Feature(s): 
    * `offset` - scale the value to set to the target variable
    * `uri` - the location and name of the variable to set to
    * `inValue` - the value received in response to a fims set/pub
      * Note: If omitted, `remap` will use `value` or `outValue` (if defined) to set the target variable
    * `outValue` - the value to set to the target variable in response to a fims set/pub
      * Note: If omitted, `remap` will use the `value` or `inValue` (if defined) to set the target variable
    * `fims` - perform a fims operation in response to a fims set/pub
      * set: ```{ "fims": "set", "uri": "/location_name/variable_name" }```
      * pub: ```{ "fims": "pub", "uri": "/location_name/variable_name" }```
      * get: ```{ "fims": "get", "replyto": "/destination_name/variable_name, "uri": "/location_name/variable_name" }```
  * Example(s):
    * `offset`:
        ```json
            "/components/catl_bms_ems_r": {
                "bms_max_discharge_allowed": {
                    "value": 0,
                    "actions": { 
                        "onSet":[{
                            "remap":[
                                {"offset": 20000,"uri":"/assets/bms/summary:max_discharge_power"}
                            ]
                        }]
                    }
                }
            }
        ```
        * In the example above, `offset` will scale the `value` to set to `uri` by 20000
    * `inValue` and `outValue`:
        ```json
            "/assets/pcs/summary": {
                "shutdown": {
                    "name": "Shutdown",
                    "value": false,
                    "unit": "",
                    "scaler": 0,
                    "enabled": false,
                    "ui_type": "control",
                    "type": "enum_button",
                    "actions":	{
                        "onSet":	[{
                            "remap":	[
                                {"inValue": true, "uri": "/status/pcs:HardShutdown", "outValue": true},
                                {"inValue": true, "uri": "/sched/pcs:schedShutdownPCS@endTime", "outValue": 0},
                                {"inValue": true, "uri": "/sched/pcs:schedShutdownPCS", "outValue": "schedShutdownPCS"},
                            ]
                        }]
                    }
                }
            }
        ```
        * Whenever the variable `shutdown` is changed, the `inValue` will be set to the same value as `value`, and the target variable defined in `uri` will be set to the value defined in `outValue`. 
        * If `inValue` is omitted, then `value` will be used instead
    * `fims`:
        ```json
            "/assets/pcs/summary": {
                "active_power_gradient": {
                    "name": "Active Power Ramp Rate (%/s)",
                    "value": 10,
                    "unit": "%/s",
                    "scaler": 1,
                    "enabled": false,
                    "ui_type": "control",
                    "type": "number",
                    "options": [],
                    "actions": {
                        "onSet": [{
                            "remap":[
                                {"fims": "set", "uri": "/components/pcsm_gradients:rise_grad_p"},
                                {"fims": "set", "uri": "/components/pcsm_gradients:drop_grad_p"},
                                {"fims": "set", "uri": "/components/pcsm_gradients:start_grad_p"},
                                {"fims": "set", "uri": "/components/pcsm_gradients:stop_grad_p"}
                            ]
                        }]
                    }
                }
            }
        ```
        * Whenever the variable `active_power_gradient` is changed, a fims set will be trigger, in which the target variable defined in `uri` will be set to the `value` of `active_power_gradient`
* `enum`
  * Description: Sets the target variable to a new value in response to a fims set/pub. Same as `remap`, except `enum` supports bit shifting and masking. `enum` does not support scaling
  * Feature(s): 
    * `uri` - the location and name of the variable to set to
    * `inValue` - the value received in response to a fims set/pub
    * `outValue` - the value to set to the target variable in response to a fims set/pub
    * `shift` - shift the bits in `inValue` to the left
    * `mask` - defines which bits in `inValue` to keep
  * Example(s):
    ```json
        "/components/catl_mbmu_sum_r": {
            "mbmu_warning_21": {
                "value": 0,
                "actions": {
                    "onSet": [{
                       "enum": [
                            { "shift": 0,"mask": 1,"inValue": 0,"uri": "/alarms/bms:current_overlimit", "outValue": "Normal"},
                            { "shift": 0,"mask": 1,"inValue": 1,"uri": "/alarms/bms:current_overlimit", "outValue": "Warn"},
                            { "shift": 1,"mask": 1,"inValue": 0,"uri": "/alarms/bms:single_cell_voltage_overlimit", "outValue": "Normal"},
                            { "shift": 1,"mask": 1,"inValue": 1,"uri": "/alarms/bms:single_cell_voltage_overlimit", "outValue": "Warn"},
                            { "shift": 2,"mask": 1,"inValue": 0,"uri": "/alarms/bms:single_cell_temp_overlimit", "outValue": "Normal"},
                            { "shift": 2,"mask": 1,"inValue": 1,"uri": "/alarms/bms:single_cell_temp_overlimit", "outValue": "Warn"}
                        ]
                    }]
                }
            }
        }
    ```
* `func`
  * Description: Runs the function in the ESS Controller for a particular asset whenever there is a fims set/pub
  * Feature(s): 
    * `func` - runs the function in the ESS Controller in response to a fims set/pub
    * `amap` - the data map associated with a particular asset to use in the function
  * Example(s):
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
* `limits`
  * Description: Sets the variable to the value in `low` if the current value is less than the lower limit value or `high` if the current value is greater than the upper limit value
  * Feature(s): 
    * `low` - the lower limit value
    * `high` - the upper limit value
  * Example(s):
    ```json
        "/controls/pcs": {
            "PStartGradient": {
                "value": 10.0,
                "actions": {
                    "onSet": [ 
                        {
                            "limits": [
                                {
                                    "low": 0.1,
                                    "high": 3000.0
                                } 
                            ]
                        }
                    ]
                }
            }
        }
    ```
    * If `value` in `/control/pcs` is less than `low`, then `value` will be set to the value in `low`
    * If `value` in `/control/pcs` is greater than `high`, then `value` will be set to the value in `high`
* `bitset`
  * Description: Sets the bit value to the target variable's bit
  * Feature(s): 
    * `uri` - the location of the variable to set to
    * `var` - the name of the variable to set to
    * `bit` - the bit to set
    * `soloBit` - if false, set the bit stored in `value`. If true, set the bit stored in `bit`
  * Example(s):
    ```json
        "/controls/pcs": {
            "QMode_Q": {
                "value": true,
                "actions": {
                    "onSet": [{
                        "bitset": [
                            {
                                "bit": 1,
                                "uri": "/controls/pcs",
                                "var": "ctrlword_qmode",
                                "soloBit": true
                            }
                        ]
                    }]
                }
            }
        }
    ```
* `bitfield`
  * Description: Sets the bit value to the target variable
  * Feature(s): 
    * `uri` - the location and name of the variable to set to
    * `bit` - the bit to set
    * `outValue` - the value to set to the target variable in response to a fims set/pub
  * Example(s):
    ```json
        "/components/ess":{
            "ctrlword1cfg":{
                "note1":"//mask 3  bit 0   0000000000000001       oncmd",
                "note2":"//mask 3  bit 1   0000000000000010       kacclosecmd",
                "note3":"//mask 48 bit 4   0000000000010000       offcmd",
                "note4":"//mask 48 bit 5   0000000000100000       kacopencmd",
                "value":0,
                "actions":{
                    "onSet":[{
                        "bitfield":[
                            { "bit": 0, "uri": "/controls/ess:OnCmd",           "outValue": true },
                            { "bit": 1, "uri": "/controls/ess:OffCmd",          "outValue": true },
                            { "bit": 2, "uri": "/controls/ess:StandbyCmd",      "outValue": true },
                            { "bit": 3, "uri": "/controls/ess:readyOkSetCmd",   "outValue": true },
                            { "bit": 4, "uri": "/controls/ess:readyOkClearCmd", "outValue": true }
                        ]
                    }]                 
                }
            }
        }
    ```
* `bitmap`
  * Description: Sets the bit value to the target variable
  * Feature(s): 
    * `uri` - the location and name of the variable to set to
    * `bit` - the bit to set
    * `outValue` - the value to set to the target variable in response to a fims set/pub
    * `shift` - shift the bits in `bit` to the left
    * `mask` - defines which bits in `bit` to keep
  * Example(s):
    ```json
       "/components/test_vlink": {
            "test_status": {
                "value": 23,
                "actions": {
                    "onSet":[{
                        "bitmap":[
                            { "shift": 0,"mask": 7,"uri": "/site/ess_hs:test_bitmap"}
                        ]
                    }]
                }
            }
       }
    ```