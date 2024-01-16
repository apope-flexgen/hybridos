System Logging

Author: Jimmy Nguyen

Date Created: 01/06/2021

Date Modified:

# Overview
The ESS Controller will log events that have occurred, such as communications failures or successes, monitoring activities, system startup and shutdown processes, etc.

Currently, the ESS Controller will use an assetVar for logging, where the value of the log assetVar will be the name of the log file (configurable). There will also be an enablePerf parameter attached to the log assetVar, which is used to enable/disable logging (see example below).

### Config Example:
```json
"/logs/ess": {
    "CheckAmCommsLog": {
        "value": "file:CheckAmCommsLog.txt",
        "enablePerf": true
    }
}
```

# Implementation
The log variable is located in /logs. The name of the log variable is the assetVar used by the ESS Controller. The assetVar is used to access the log file, which is stored as a value. The log file name can be changed if needed via fims. Otherwise, the default log file name will use the name of the function where logging is used (ex.: CheckAmComms).

### Configuration:
While the ESS Controller is running, to change the log file name, you can do the following example:
1. Fims interface
    * /usr/local/bin/fims/fims_send -m set -u /ess/logs/ess '{"CheckAmCommsLog":"file:CheckAmCommsLog_test.txt"}' -r /me
    * /usr/local/bin/fims/fims_send -m set -u /reload/ess '{"CheckAmComms": 0}'
