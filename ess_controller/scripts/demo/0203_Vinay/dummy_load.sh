#!/bin/sh
# p wilshire 02-13-2022
# basic script to set up a bunch of components and publish them
# look for "repTime" to change the pub rate. I'll find a better way of doing that.
# ess_controller -x
# sh dummy_load.sh
# fims_listen
# after a couple of seconds the pubs start.
# values are all the same for now but I'm working on that next


/usr/local/bin/fims_send -m set -r /$$ -u /ess/config/load  '
{
    "test_publish" :{
        "value": false,
        "file":"dummy_load",
        "pname":"ess",
        "aname":"bms",
        "new_options": [
            {
                "tmpl":"dummy_load_tmpl_1", "pname":"bms", "amname":"##BMS_ID##",
                "from":1, "to": 10,
                "reps":[
                    {"replace":"##BMS_ID##", "with":"bms_{:02d}"},
                    {"replace":"##BMS_NUM##", "with":"{:02d}"}
                ]
            },
            {
                "tmpl":"dummy_load_tmpl_2", "pname":"##BMS_ID##", "amname":"##BMS_ID##_##RACK_ID##",
                "from":1, "to": 5,
                "reps":[
                    {"replace":"##RACK_ID##", "with":"rack_{:02d}"},
                    {"replace":"##RACK_NUM##", "with":"{:02d}"}
                ]
            }
            
        ]
    }
} '
#exit 0

/usr/local/bin/fims_send -m set  -u /ess/cfg/cfile/ess/dummy_load  '
{
    "/status/bms":{
        "SystemStatus":"Running"
    },
    "/system/commands": {
        "runBmsMon": {
            "value": "test",
            "ifChanged":false,
            "enabled":true,
            "amname":"bms",
            "mname":"wake_monitor",
            "runChild":false,
            "debug":true,
            "actions":{ "onSet":[{"func":[{"debug":true,"func":"RunMonitorList"}]}]}
        }
    },
 
    "/system/test":{
        "test_opts":	{
            "value": true,
            "help": "sends each option to the targ av", 
            "debug":false,
            "enabled":false,
            "ifChanged": false,
            "targVal": 3456,
            "xtargav":"/test/targ:av",
            "xtargfunc": "HandleSchedItem",
            "amap":	"ess",
            "new_options":[ 
                {"uri":"/sched/ess:essSystemInit", "value":{"value":1234,"state":"start"}}
            ],
            "actions":	{"onSet":	[{"func":	[{"debug":true,"func":	"SchedItemOpts","amap":	"ess","initDone":true}]}]}
        }
    }
}'
/usr/local/bin/fims_send -m get  -r/$$ -u /ess/full/config/cfile/dummy_load  | jq

#exit 0

/usr/local/bin/fims_send -m set  -u /ess/cfg/ctmpl/ess/dummy_load_tmpl_1  '
{
    "/components/##BMS_ID##":{
        "SystemStatus":"Running",
        "SOC":45.6,
        "Current":280,
        "Voltage":120
    },

    "/schedule/wake_monitor/bms" : {
        "/schedule/##BMS_ID##:run_pub@enabled":{"debug":true,"value":false,"amap":"##BMS_ID##","enable":true,"func":"SendTrue","rate":0.1},
        "/schedule/##BMS_ID##:run_pub":{"debug":true,"value":false,"amap":"##BMS_ID##","enable":true,"func":"SendTime","rate":0.1}
    },

    "/schedule/##BMS_ID##":{
        "run_pub":{
            "value":"##BMS_ID##:run_pub",
            "enabled":true,
            "endTime":0,
            "fcn":"RunPub",
            "refTime":0,
            "repTime":0.1,
            "table":"/components/##BMS_ID##",
            "uri":"/schedule/##BMS_ID##:run_pub",
            "id":"##BMS_ID##:run_pub",
            "actions":{ "onSet":[{"func":[{"debug":true,"amap":"ess","func":"HandleSchedLoad"}]}]}
        }
    }

}'

/usr/local/bin/fims_send -m set  -u /ess/cfg/ctmpl/ess/dummy_load_tmpl_2  '
{
    "/components/##BMS_ID##": {
        "##BMS_ID##_##RACK_ID##_SystemStatus":"Running",
        "##BMS_ID##_##RACK_ID##_SOC":45.6,
        "##BMS_ID##_##RACK_ID##_Current":280,
        "##BMS_ID##_##RACK_ID##_Voltage":120
    },
    "/system/test":{
        "test_opts":	{
            "value": true,
            "options":[ 
                {"uri":"/system/test/ess:##BMS_ID##_##RACK_ID##_test", "xvalue":{"value":1234,"state":"start"}}
            ]
        }
    }
}'
/usr/local/bin/fims_send -m set  -u /ess/cfg/cfile/ess/system_test  '
{
    "/system/test":{
        "test_opts":	{
            "debug": true,
            "enabled": true
            }
        }
    }
}'
/usr/local/bin/fims_send -m set  -u /ess/cfg/cfile/ess/system_test2  '
{
    "/system/test":{
        "test_opts":	{
            "value": 4444
            }
        }
    }
}'
