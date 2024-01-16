
#!/bin/sh 

# Tests the HandlePower function (monitoring the state of the active current setpoint) in test_ess.cpp
# Note: Run test_ess first before running this shell script
echo "setting up structure ess ->pcs_1 ->pcs_2 ->bms_1 bms_2"
# fims_send -m set  -u /ess/cfg/cfile/ess/ess_limits ' {
#     "amname":"ess",
#     "/limits/ess":{
#         "tMaxPower":345000
#     }
# }'
fims_send -m set  -u /ess/cfg/cfile/ess/ess1_limits ' {
    "pname":"ess",
    "amname":"ess_1"
}'
# ,
#     "/limits/ess_1":{
#         "tMaxPower":345000
#     }
fims_send -m set  -u /ess/cfg/cfile/ess/ess2_limits ' {
    "pname":"ess",
    "amname":"ess_2"
}'
# ,
#     "/limits/ess_2":{
#         "tMaxPower":345000
#     }
# fims_send -m set  -u /ess/cfg/cfile/ess/bms_1_limits ' {
#     "pname":"ess",
#     "amname":"bms_1",
#     "/limits/bms_1":{
#         "tMaxPower":345000
#     }
# }'
# fims_send -m set  -u /ess/cfg/cfile/ess/bms_2_limits ' {
#     "pname":"ess",
#     "amname":"bms_2",
#     "/limits/bms_2":{
#         "tMaxPower":345000
#     }
# }'

# fims_send -m set  -u /ess/cfg/cfile/ess/pcs_1_limits ' {
#     "pname":"ess",
#     "amname":"pcs_1",
#     "/limits/pcs_1":{
#         "MaxPCSChargePower": 10,
#         "MaxPCSDischargePower":-10,
#         "MaxPCSReactivePower": -10,
#         "PCSLimitsChanged": false,
#         "RatedApparentPower": -10,
#         "tMaxPower":345000
#     }
# }'

# fims_send -m set  -u /ess/cfg/cfile/ess/pcs_2_limits ' {
#     "pname":"ess",
#     "amname":"pcs_2",
#     "/limits/pcs_2":{
#         "tMaxPower": 345000
#     }
# }'

fims_send -m set -r /$$ -u /ess/config/ess_1/enable false
fims_send -m set -r /$$ -u /ess/config/ess_2/enable false
echo "initializing commands"
fims_send -m set -r /$$ -u /ess/system/commands ' {
    "runESSOpts":	{
        "value": false,
        "enabled": false,
        "targav": "/system/commands:run",
        "new_options":[ 
            {"uri":"/sched/ess_1:every100ms_ess_1", "aname":"ess", "value":0, "every":0.1, "offset":0},
            {"uri":"/sched/ess_2:every100ms_ess_2", "aname":"ess", "value":0, "every":0.1, "offset":0.05}
        ],
        "actions":	{
            "onSet":[{"func":[{"amap": "ess", "func":"SchedItemOpts"}]}]
        }
    }
}'
fims_send -m set -r /$$ -u /ess/sched/ess_1 ' {
    "every100ms_ess_1":{
        "value":1,
        "note": "Run power command handler",
        "enable": "/config/ess_1:enable",
        "ess":"ess_1",
        "bms":"bms_1",
        "pcs":"pcs_1",
        "debug": false,
        "actions":{
            "onSet":[{
                "func":[
                    {"amap": "ess_1", "func":"HandlePowerCmd"}
                ]
            }]
        }
    }
}'
fims_send -m set -r /$$ -u /ess/sched/ess_2 ' {
    "every100ms_ess_2":{
        "value":1,
        "note": "Run power command handler",
        "enable": "/config/ess_2:enable",
        "ess":"ess_2",
        "bms":"bms_2",
        "pcs":"pcs_2",
        "debug": false,
        "actions":{
            "onSet":[{
                "func":[
                    {"amap": "ess_2", "func":"HandlePowerCmd"}
                ]
            }]
        }
    }
}'
fims_send -m set -r /$$ -u /ess/system/commands ' {
    "run": {
        "help": "run a schedule var needs the uri to be set ",
        "value":0,
        "ifChanged":false,
        "enabled":true, 
        "actions":{"onSet":[{"func":[{"amap": "ess", "func":"RunSched"}]}]}
    },
    "runESSOpts":	{
        "value": true,
        "enabled": true
    }
}'
# fims_send -m set -r /$$ -u /ess/system/commands/runESSOpts@enabled true
# fims_send -m set -r /$$ -u /ess/system/commands/runESSOpts true

read -rsn1 -p"Press any key to enable HPC 1";echo
fims_send -m set -r /$$ -u /ess/config/ess_1/enable true

read -rsn1 -p"Press any key to send initial variables 1";echo

echo "Initializing ess_1 variables"
echo -n "setting /limits/bms_1:ChargePowerLimit        " && /usr/local/bin/fims_send -r /$$ -m set -u /ess/limits/bms_1/ChargePowerLimit '{"value":-1475}'
echo -n "setting /limits/bms_1:DischargePowerLimit     " && /usr/local/bin/fims_send -r /$$ -m set -u /ess/limits/bms_1/DischargePowerLimit 1490
echo -n "setting /limits/pcs_1:RatedActivePower        " && /usr/local/bin/fims_send -r /$$ -m set -u /ess/config/pcs_1/RatedActivePower 1495
echo -n "setting /limits/pcs_1:RatedReactivePower      " && /usr/local/bin/fims_send -r /$$ -m set -u /ess/config/pcs_1/RatedReactivePower 1000
echo -n "setting /limits/pcs_1:RatedApparentPower      " && /usr/local/bin/fims_send -r /$$ -m set -u /ess/config/pcs_1/RatedApparentPower 0
echo -n "triggering /limits/pcs_1:RatedApparentPower   " && /usr/local/bin/fims_send -r /$$ -m set -u /ess/config/pcs_1/RatedApparentPower 2125
echo
sleep 0.2

read -rsn1 -p"Press any key to enable HPC 2";echo
fims_send -m set -r /$$ -u /ess/config/ess_2/enable true

read -rsn1 -p"Press any key to send initial variables 2";echo

echo "Initializing ess_2 variables"
echo -n "setting /limits/bms_2:ChargePowerLimit        " && /usr/local/bin/fims_send -r /$$ -m set -u /ess/limits/bms_2/ChargePowerLimit '{"value":-1500}'
echo -n "setting /limits/bms_2:DischargePowerLimit     " && /usr/local/bin/fims_send -r /$$ -m set -u /ess/limits/bms_2/DischargePowerLimit 1600
echo -n "setting /limits/pcs_2:RatedActivePower        " && /usr/local/bin/fims_send -r /$$ -m set -u /ess/config/pcs_2/RatedActivePower 4000
echo -n "setting /limits/pcs_2:RatedReactivePower      " && /usr/local/bin/fims_send -r /$$ -m set -u /ess/config/pcs_2/RatedReactivePower 2000
echo -n "setting /limits/pcs_2:RatedApparentPower      " && /usr/local/bin/fims_send -r /$$ -m set -u /ess/config/pcs_2/RatedApparentPower 0
echo -n "triggering /limits/pcs_2:RatedApparentPower   " && /usr/local/bin/fims_send -r /$$ -m set -u /ess/config/pcs_2/RatedApparentPower 3125
echo
sleep 0.2

read -rsn1 -p"Press any key to check variables";echo

# Check state
echo "Check variables"
echo -n "getting /limits/ess                           " && /usr/local/bin/fims_send -r /$$ -m get -u /ess/limits | jq
echo
sleep 0.2

read -rsn1 -p"Press any key to test Discharge Limiting";echo
fims_send -r /$$ -m set -u /ess/controls/pcs_1/ActivePowerSetpoint 1500

read -rsn1 -p"Press any key to test Charge Limiting";echo
fims_send -r /$$ -m set -u /ess/controls/pcs_1/ActivePowerSetpoint '{"value":-1510}'

read -rsn1 -p"Press any key to test Reactive Power Limiting";echo
fims_send -r /$$ -m set -u /ess/controls/pcs_1/ActivePowerSetpoint 0
fims_send -r /$$ -m set -u /ess/controls/pcs_1/ReactivePowerSetpoint 1200

read -rsn1 -p"Press any key to test negative Reactive Power Limiting";echo
fims_send -r /$$ -m set -u /ess/controls/pcs_1/ReactivePowerSetpoint '{"value":-1210}'

read -rsn1 -p"Press any key to test Apparent Power Limiting";echo
fims_send -r /$$ -m set -u /ess/controls/pcs_1/ActivePowerSetpoint 2000
fims_send -r /$$ -m set -u /ess/controls/pcs_1/ReactivePowerSetpoint 1000

read -rsn1 -p"Press any key to test Apparent Power Limiting with negative P";echo
fims_send -r /$$ -m set -u /ess/controls/pcs_1/ActivePowerSetpoint '{"value":-2000}'

read -rsn1 -p"Press any key to test Apparent Power Limiting with negated Q";echo
fims_send -r /$$ -m set -u /ess/controls/pcs_1/ReactivePowerSetpoint '{"value":-1210}'

read -rsn1 -p"Press any key to test Apparent Power Limiting with positive P";echo
fims_send -r /$$ -m set -u /ess/controls/pcs_1/ActivePowerSetpoint 2000

read -rsn1 -p"Press any key to Change Priority";echo
fims_send -r /$$ -m set -u /ess/controls/pcs_1/PowerPriority '{"value":"q"}'

read -rsn1 -p"Press any key to test Apparent Power Limiting";echo
fims_send -r /$$ -m set -u /ess/controls/pcs_1/ReactivePowerSetpoint 1000

read -rsn1 -p"Press any key to test Apparent Power Limiting with negative P";echo
fims_send -r /$$ -m set -u /ess/controls/pcs_1/ActivePowerSetpoint '{"value":-2000}'

read -rsn1 -p"Press any key to test Apparent Power Limiting with negated Q";echo
fims_send -r /$$ -m set -u /ess/controls/pcs_1/ReactivePowerSetpoint '{"value":-1210}'
