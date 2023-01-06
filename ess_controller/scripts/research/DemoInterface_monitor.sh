#!/bin/sh
# demo of a controller interface running the monitor

PCS_ID=pcs_1



# echo  did we load it
# fims_send -m get  -r /$$ -u /ess/naked/config/cfile | jq

# echo Question ... how do we enable the "start button" ....
# fims_send -m get  -r /$$ -u /ess/full/assets/pcs/pcs_1/start  | jq | grep -i "start\|enable"


# echo hint we need to monitor the status 



# echo we already have  /assets/pcs/pcs_1:maint_mode as  true
# echo we need /status/bms:DCClosed to be true
# echo we need /status/pcs:SystemState to be Off or Ready

# echo we also need something to check StartEnable every so often.

echo  Demointerface_monitor loads the instructions for checking pcs:StartEnable

        # "/status/pcs:PCSHeartbeat":              { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CheckMonitorVar"},
        # "/status/pcs:ActivePower":               { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CheckMonitorVar"},
        # "/status/pcs:DCVoltage":                 { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CheckMonitorVar"},

        # "/status/pcs:ActivePowerCmd_adjusted":   { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"},
        # "/status/pcs:ReactivePowerCmd_adjusted": { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"},
        # "/status/pcs:ApparentPower":             { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"},
        # "/status/pcs:MaxPCSApparentPower":       { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"},
        # "/status/pcs:DischargeCalibrationCalc":  { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"},
        # "/status/pcs:ChargeCalibrationCalc":     { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"},
        # "/status/pcs:ShutdownEnable":            { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"},
        # "/status/pcs:StandbyEnable":             { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"},
        # "/status/pcs:GridFormEnable":            { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"},
        # "/status/pcs:GridFollowEnable":          { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"},
        # "/status/pcs:PCSHeartbeatCheckEnable":   { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"}

fims_send -m set  -u /ess/cfg/cfile/ess/pcs_monitor '{
"pname":"ess",
"amname":"pcs",
"/schedule/wake_monitor/pcs":{
        "/status/pcs:StartEnable":               { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"}
    }
}'

echo then we load the instructions for running the monitor

fims_send -m set  -u /ess/cfg/cfile/ess/pcs_monitor_run '{
    "/controls/pcs":{   
        "enable_run":false
    },

    "/system/commands": {
        "runMon":{
            "value":0,
            "aname":"pcs",
            "monitor":"wake_monitor",
            "enable":"/controls/pcs:enable",
            "help": "load the wake monitor setup system",
            "actions":{"onSet":[{"func":[{"func":"RunMonitor","aname":"ess"}]}]}
        },
        "run":{
            "value":"test",
            "uri":"/system/commands:runMon",
            "every":1,
            "enable":"/controls/pcs:enable_runxx",
            "help": "run the runMon schedule var",
            "actions":{"onSet":[{"func":[{"func":"RunSched"}]}]}
        }
    }
}'

echo lets allow it to run

fims_send -m set  -u /ess/controls/pcs/enable true

sleep 1

echo check it
fims_send -m get  -r/$$ -u /ess/full/system/commands/runMon | jq
sleep 1
fims_send -m get  -r/$$ -u /ess/full/system/commands/runMon | jq

echo any clues on how to get the StartEnable to work

fims_send -m get  -r/$$ -u /ess/full/status/pcs/StartEnable | jq

echo -n "/status/bms/DCClosed          " 
fims_send -m get  -r/$$ -u /ess/full/status/bms/DCClosed | jq | grep -i "\"value"

echo -n "/assets/pcs/pcs_1/maint_mode  " 
fims_send -m get  -r/$$ -u /ess/full/assets/pcs/pcs_1/maint_mode | jq | grep -i "\"value"

echo -n "/status/pcs/SystemState       "
fims_send -m get  -r/$$ -u /ess/full/status/pcs/SystemState | jq | grep -i "\"value"

echo hint ">>>>" fims_send -m set  -r/$$ -u /ess/full/status/pcs/SystemState \'\"Off\"\'
#fims_send -m get  -r /$$ -u /ess/naked/config/cfile | jq


echo So once start is enabled what does it do ..

echo '
"remap": [ { "inValue": true, "outValue": true,    "uri": "/controls/pcs:CheckIfOffGrid@triggerCmd" },
           { "inValue": true, "outValue": 0,       "uri": "/controls/pcs:CheckIfOffGrid",
                        "note": "Check if the PCS is in off-grid mode. If so, send 480 V / 60 Hz commands to the PCS before starting up the PCS" },
           { "inValue": true, "outValue": true,    "uri": "/controls/pcs:StartCmd@triggerCmd" },
           { "inValue": true, "outValue": 207,     "uri": "/controls/pcs:StartCmd",
                        "note": "Send start command to PCS" }
         ]
'
echo
echo ">>>>>>>>>>>>" How do  we trigger start 
echo
echo ">>>>>>>>>>>>" How do  we setup CheckIfOffGrid ??  ... nah ... we turn that off for this system
echo
echo

echo '
"remap": [ { "enable":"/config/pcs:BlackStart", "inValue": true, "outValue": true,    "uri": "/controls/pcs:CheckIfOffGrid@triggerCmd" },
           { "enable":"/config/pcs:BlackStart", "inValue": true, "outValue": 0,       "uri": "/controls/pcs:CheckIfOffGrid",
                        "note": "if BlackStart is true, Check if the PCS is in off-grid mode. If so, send 480 V / 60 Hz commands to the PCS before starting up the PCS" },
           { "inValue": true, "outValue": true,    "uri": "/controls/pcs:StartCmd@triggerCmd" },
           { "inValue": true, "outValue": 207,     "uri": "/controls/pcs:StartCmd",
                        "note": "Send start command to PCS" }
         ]
'
echo 
echo \"start\" has the following UI options .....
echo '
"options": [
                {"name": "No", "return_value": false},
                {"name": "Yes","return_value": true}
            ]
'

echo To start the system 
echo hint ">>>>" fims_send -m set  -r/me -u /ess/assets/pcs/pcs_1/start  true
echo 
echo hint ">>>>" fims_send -m get -r/me -u /ess/full/controls/pcs "| jq"

echo 
echo So far so good , we have triggered StartCmd but now it needs to do something.




echo ">>>>>>>>>>>>>>>>>>>" now load DemoInerface_StartCmd
#fims_send -m get  -r /$$ -u /ess/naked/config/cfile | jq
