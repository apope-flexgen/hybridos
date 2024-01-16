ESS Variable Calculator

Author: Jimmy Nguyen

Date Created: 
* 02/26/2021

Date Modified: 

# Overview
The ESS Controller is able to communicate and send/receive data from hardware devices (ex.: CATL BMS, Power Electronics PCS) using modbus configuration files and the modbus interface.  

The ESS Controller can have access to registers defined in the modbus configuration, however, there may be instances where the ESS Controller wants to use a specific register, but that register does not exist in the configuration.
* Ex.: CATL BMS does not have a register for system power, but the ESS Controller may want to use this register to display data to the UI or other internal use cases  

The ESS Controller provides a simple calculator function that'll use arbitrary number of variables, perform scaling if specified, and set the result to the variable of interest  
* Ex.: For system power, we'll take the product of system voltage and system current

# Configuration
Here is an example of how to configure the ESS calculator:
```json
"/status/bms": {
    "BMSPower": {
        "value": 0,
        "numVars": 2,
        "variable1": "mbmu_voltage",
        "variable2": "BMSCurrent",
        "scale": 1000,
        "operation": "*"
    }
}
```
Note:  
* `numVars` is the number of variables to include in calculations
* `variable[int]` is the name of the variable to include in calcuations
  * Ex.: 
    * If `numVars` = 4, then we should have `variable1`, `variable2`, `variable3`, `variable4`
    * If `numVars` = N, then we should have `variable1`, `variable2`, ... , `variableN`
  * If the variable does not exist in the data map, then the variable is created before calculations
  * If the number of variables defined is not the same as `numVars`, then the ESS Controller will skip calculations
* `scale` is used to change the resulting value after calcuations
  * Optional parameter
  * Ex.: If system power is 5000.0 W and `scale` is 1000, then system power is now 5.0 kW
* `operation` is the math operation to use for calcuations
  * The math operations currently available are:
    * Addition (+)
    * Subtraction (-)
    * Multiplication (*)
    * Division (/)
    * Average (avg)


Here is a way to allow the function to run periodically, even with no changes to the variable: 
```json
"/schedule/wake_monitor/bms":{
    "/status/bms:BMSPower": { "enable": true, "rate":0.1, "func":"CalculateVar"}
}
```
Note:
* `enable` determines whether the function should run periodically for a specific variable
* `rate` is the time to wait before making another function call (not used at the moment)
* `func` allows the ESS Controller to periodically call the function (ex.: `CalculateVar`)

# Data Validation
While the ESS Controller is running, to check if the calcuator function is working, you can do the following:
1. Fims interface
    * `/usr/local/bin/fims/fims_send -m get -u /ess/full/status/bms/BMSPower -r /me | jq`
    * `/usr/local/bin/fims/fims_send -m pub -u /components/catl_mbmu_summary_r/mbmu_voltage 2`
    * `/usr/local/bin/fims/fims_send -m pub -u /components/catl_mbmu_summary_r/mbmu_current 20020`
    * `/usr/local/bin/fims/fims_send -m get -u /status/bms/BMSPower -r /me`
