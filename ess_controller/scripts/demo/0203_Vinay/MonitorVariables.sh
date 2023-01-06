#!/bin/sh
# p wilshire 02/03/2022
# demo script to set up monitoring and alarms 
#Lets look at an example.

# Set up a monitor command
fims_send  -m set -r /$$ -u /ess/full/system/commands '
{
    "runEssMon":{
            "value":"test",
            "help": "run a wake_monitor list",
            "ifChanged":false, 
            "aname":"ess", 
            "mname": "wake_monitor", 
            "enabled":false, 
            "runChild":false,
            "actions":{"onSet":[{"func":[{"func":"RunMonitorList"}]}]}
            }
}'
# we can start the task now even though we have nothing to monitor

fims_send  -m set -r /$$ -u /ess/full/system/commands '
{
    "runEssMon":{"value":"test","enabled":true}
}'

# we will need a monitir list to stop the system complaining
echo "set up a monitor list"

fims_send  -m set -r /$$ -u /ess/full/schedule/wake_monitor/ess '
{
        "/state/ess:Current":             { "enabled": true, "rate": 0.1, "amap": "ess", "func": "CheckMonitorVar"},
        "/state/ess:Power":               { "enabled": true, "rate": 0.1, "amap": "ess", "func": "CheckMonitorVar"}
}'

echo " set up the target variables"
fims_send  -m set -r /$$ -u /ess/full/state/ess/Current 1234
fims_send  -m set -r /$$ -u /ess/full/state/ess/Power 2000

echo " look at what's happening"
fims_send  -m get -r /$$ -u /ess/full/state/ess | jq

echo "run it again"

fims_send  -m set -r /$$ -u /ess/full/system/commands/runEssMon  1

echo " do this to see what's changed"
echo "fims_send  -m get -r /$$ -u /ess/full/status/ess | jq"

# set up some limits , thes are normally defined in the configs

fims_send  -m set -r /$$ -u /ess/full/state/ess/Current@MaxResetValue '{"value":1900}'
fims_send  -m set -r /$$ -u /ess/full/state/ess/Current@MinResetValue '{"value":-1900}'
fims_send  -m set -r /$$ -u /ess/full/state/ess/Current@MaxFaultThreshold '{"value":2200}'
fims_send  -m set -r /$$ -u /ess/full/state/ess/Current@MinFaultThreshold '{"value":-2200}'
fims_send  -m set -r /$$ -u /ess/full/state/ess/Current@MaxAlarmThreshold '{"value":2000}'
fims_send  -m set -r /$$ -u /ess/full/state/ess/Current@MinAlarmThreshold '{"value":-2000}'
fims_send  -m set -r /$$ -u /ess/full/state/ess/Current@EnableMaxValCheck true 
fims_send  -m set -r /$$ -u /ess/full/state/ess/Current@EnableMinValCheck true 


# now exceed them
fims_send  -m set -r /$$ -u /ess/full/state/ess/Current 2010
fims_send  -m set -r /$$ -u /ess/full/system/commands/runEssMon  1
fims_send  -m set -r /$$ -u /ess/full/state/ess/Current '{"value":-2000}'
fims_send  -m set -r /$$ -u /ess/full/system/commands/runEssMon  1

# we are generating faults so how do we clear them

fims_send  -m set -r /$$ -u /ess/faults/ess '
 {
    "clear_faults": {
    "value": "default",
    "type": "fault",
    "actions": {
            "onSet": [{
                "func": [
                    {"enable":"/sched/ess:enable","enabled":false, "func": "process_sys_alarm", "amap": "ess"}
                    ]
                }]
        }
    }
}'

fims_send  -m set -r /$$ -u /ess/full/sched/ess/enable true 

fims_send  -m set -r /$$ -u /ess/full/state/ess '
{
  "Current": {
    "value": 290
  },
  "Power": {
    "value": 500
  },
  "Volts": {
    "value": 1356
  }
}'

fims_send  -m set -r /$$ -u /ess/full/faults/ess '
{
  "clear_faults": {
    "value": "NoVal",
    "defComp": "/state/ess",
    "numVars": 3,
    "type": "fault",
    "variable1": "Current",
    "variable2": "Volts",
    "variable3": "Power",
    "actions": {
      "onSet": [
        {
          "func": [
            {
              "amap": "ess",
              "enable": "/sched/ess:enable",
              "func": "process_sys_alarm"
            }
          ]
        }
      ]
    }
  }
}'

# test this with
fims_send  -m get -r /$$ -u /ess/amap/ess | jq

# which should look like

# {
#   "amap": {
#     "AlarmCnt": {
#       "value": 0
#     },
#     "AlarmDestination": {
#       "value": "/assets/ess/summary:alarms"
#     },
#     "ClearFaultsDone": {
#       "value": true
#     },
#     "FaultCnt": {
#       "value": 0
#     },
#     "FaultDestination": {
#       "value": "/assets/ess/summary:faults"
#     },
#     "FaultShutdown": {
#       "value": false
#     },
#     "NoAlarmMsg": {
#       "value": "Normal"
#     },
#     "NoFaultMsg": {
#       "value": "Normal"
#     },
#     "myCurrent": {
#       "value": 290
#     },
#     "myPower": {
#       "value": 500
#     },
#     "myVolts": {
#       "value": 1356
#     },
#     "ess_config": {
#       "value": 154.00299200008158,
#       "active": true,
#       "amap": true,
#       "debug": 0,
#       "enabled": true,
#       "endTime": 0,
#       "fcn": "RunTarg",
#       "reftime": 0,
#       "repTime": 1,
#       "runCnt": 154,
#       "runEnd": 0,
#       "runTime": 155.002331505,
#       "targ": "/config/control:ess_config",
#       "uri": "/config/control:ess_config"
#     }
#   },
#   "links": {
#     "AlarmCnt": {
#       "value": "/status/ess:AlarmCnt"
#     },
#     "AlarmDestination": {
#       "value": "/config/ess:AlarmDestination"
#     },
#     "ClearFaultsDone": {
#       "value": "/status/ess:ClearFaultsDone"
#     },
#     "FaultCnt": {
#       "value": "/status/ess:FaultCnt"
#     },
#     "FaultDestination": {
#       "value": "/config/ess:FaultDestination"
#     },
#     "FaultShutdown": {
#       "value": "/status/ess:FaultShutdown"
#     },
#     "NoAlarmMsg": {
#       "value": "/config/ess:NoAlarmMsg"
#     },
#     "NoFaultMsg": {
#       "value": "/config/ess:NoFaultMsg"
#     }
#   }
# }
fims_send  -m set -r /$$ -u /ess/full/alarms/ess '
{
  "clear_alarms": {
    "value": "NoVal",
    "defComp": "/state/ess",
    "numVars": 3,
    "type": "alarm",
    "variable1": "Current",
    "variable2": "Volts",
    "variable3": "Power",
    "actions": {
      "onSet": [
        {
          "func": [
            {
              "amap": "ess",
              "enable": "/sched/ess:enable",
              "func": "process_sys_alarm"
            }
          ]
        }
      ]
    }
  }
}'

# so now we can clear faults and alarms....
fims_send  -m set -r /$$ -u /ess/full/alarms/ess/clear_alarms '"Clear"'
fims_send  -m set -r /$$ -u /ess/full/alarms/ess/clear_faults '"Clear"'
