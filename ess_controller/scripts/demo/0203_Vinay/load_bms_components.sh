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
    "racks_1_to_10" :{
        "value": false,
        "file":"test_load",
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

/usr/local/bin/fims_send -m set -r /$$ -u /ess/config/load  '
{
    "racks_11_and_12" :{
        "value": false,
        "file":"test_load",
        "pname":"ess",
        "aname":"bms",
        "new_options": [
            {
                "tmpl":"dummy_load_tmpl_1", "pname":"bms", "amname":"##BMS_ID##",
                "from":11, "to": 12,
                "reps":[
                    {"replace":"##BMS_ID##", "with":"bms_{:02d}"},
                    {"replace":"##BMS_NUM##", "with":"{:02d}"}
                ]
            },
            {
                "tmpl":"dummy_load_tmpl_2", "pname":"##BMS_ID##", "amname":"##BMS_ID##_##RACK_ID##",
                "from":1, "to": 8,
                "reps":[
                    {"replace":"##RACK_ID##", "with":"rack_{:02d}"},
                    {"replace":"##RACK_NUM##", "with":"{:02d}"}
                ]
            }
            
        ]
    }
} '

#exit 0

/usr/local/bin/fims_send -m set  -u /ess/cfg/cfile/ess/test_load  '
{
    "/docs/01":{
        "note01":" Demonstration of a multi level template setup",
        "note02":"    This is used to create a test output file for modbus testing",
        "note03":"    A number of BMS systems is created, each with a fixed number of racks",
        "note04":"    BMS systems can also be created with different numbers of racks using the same templates",
        "note05":"    The system then sets up the publish framework for the components using the wake_monitor framework",
        "note06":"       use set /schedule/bms_xx/run_pub@enabled true/false to turn on or off individual component pubs", 
        "note07":"    ",
        "note08":"    The system then sets up control framework using the enhanced SchedItemOpts feature",
        "note09":"       this creates a list of bms/rack combinations and allows the same value to be sent to a selected target av all members of the list",
        "note10":"           set /controls/all_racks/set_targ@enabled true turns on the control",
        "note11":"           set /controls/all_racks/set_targ@targVar  somevar  selects a rack variable",
        "note12":"           set /controls/all_racks/set_targ@targVal  somevalue  selects the intended  variable value",
        "note13":"           set /controls/all_racks/set_targ   somenumber   triggers the operation",
        "note14":"    ",
        "note15":"           use  /controls/all_bms/set_targ  to just target bms units "
    },
    "/status/bms":{
        "SystemStatus":"Running"
    },
    "/system/commands": {
        "runBmsMon": {
            "value": "test",
            "ifChanged":false,
            "enabled":true,
            "amname":"bms",
            "mname":"run_pubs",
            "runChild":false,
            "debug":true,
            "actions":{ "onSet":[{"func":[{"debug":true,"func":"RunMonitorList"}]}]}
        }
    },
    "/controls/all_bms":{
        "set_targ":	{
            "value": true,
            "help": "sends each option to the targ av", 
            "debug":false,
            "enabled":false,
            "ifChanged": false,
            "targVal": 3456,
            "targVar": "TestVar",
            "xtargav":"/test/targ:av",
            "xtargfunc": "HandleSchedItem",
            "amap":	"ess",
            "new_options":[ 
            ],
            "actions":	{"onSet":	[{"func":	[{"debug":true,"func":	"SchedItemOpts","amap":	"ess","initDone":true}]}]}
        }
    },
 
    "/controls/all_racks":{
        "set_targ":	{
            "value": true,
            "help": "sends each option to the targ av", 
            "debug":false,
            "enabled":false,
            "ifChanged": false,
            "targVal": 3456,
            "targVar": "TestVar",
            "xtargav":"/test/targ:av",
            "xtargfunc": "HandleSchedItem",
            "amap":	"ess",
            "new_options":[ 
            ],
            "actions":	{"onSet":	[{"func":	[{"debug":true,"func":	"SchedItemOpts","amap":	"ess","initDone":true}]}]}
        }
    }
}'
/usr/local/bin/fims_send -m get  -r/$$ -u /ess/full/config/cfile/test_load  | jq

#exit 0

/usr/local/bin/fims_send -m set  -u /ess/cfg/ctmpl/ess/dummy_load_tmpl_1  '
{
    "/components/##BMS_ID##":{
        "SystemStatus":"Running",
        "SOC":45.6,
        "Current":280,
        "Voltage":120
    },

    "/schedule/run_pubs/bms" : {
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
            "mode":"naked",
            "table":"/components/##BMS_ID##",
            "uri":"/schedule/##BMS_ID##:run_pub",
            "xid":"##BMS_ID##:run_pub",
            "actions":{ "onSet":[{"func":[{"debug":true,"amap":"ess","func":"HandleSchedLoad"}]}]}
        }
    },
    
    "/controls/all_bms":{
        "set_targ":	{
            "value": true,
            "options":[ 
                {"uri":"/components/##BMS_ID##:", "xvalue":{"value":1234,"state":"start"}}
            ]
        }
    }


}'

/usr/local/bin/fims_send -m set  -u /ess/cfg/ctmpl/ess/dummy_load_tmpl_2  '
{
    "/components/##BMS_ID##": {
        "##RACK_ID##_SystemStatus":"Running",
        "##RACK_ID##_SOC":45.6,
        "##RACK_ID##_Current":280,
        "##RACK_ID##_Voltage":120
    },
    
    "/controls/all_racks":{
        "set_targ":	{
            "value": true,
            "options":[ 
                {"uri":"/components/##BMS_ID##:##RACK_ID##_", "xvalue":{"value":1234,"state":"start"}}
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
