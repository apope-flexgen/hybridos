#!/bin/sh 
# test SchedItemOpts
#!/bin/sh
# basic tester for SchedItemOpts
# either send a number of  avs to a function
# or run a function against a number of av's
# 
#  build/release/FlexEss  -c sample_ess_no_json -n flex -s ":/components:/system:/site:
#
fims_send -m set -r /$$ -u /flex/test/sched ' {
    "sched_func":	{
        "value": true,
        "debug":true,
        "ifChanged": false,
        "targfunc": "HandleSchedItem",
        "amap":	"flex",
        "options":[ 
            {"uri":"/sched/ess:essSystemInit"},
            {"uri":"/sched/ess:every100mSP1"},
            {"uri":"/sched/ess:every100mSP2"},
            {"uri":"/sched/ess:every100mSP3"},
            {"uri":"/sched/ess:fastPub"},
            {"uri":"/sched/ess:slowPub"}
        ],
        "actions":	{
            "onSet":	[{
                "func":	[{
                        "debug":true,
                        "func":	"SchedItemOpts",
                        "amap":	"flex",
                        "initDone":true
                    }]
        
            }]
        }
    },
    "sched_av":	{
        "value": true,
        "debug":true,
        "ifChanged": false,
        "targav": "/test/av:aTestAv",
        "options":[ 
            {"value":1,"uri":"/sched/ess:essSystemInit"},
            {"value":2,"uri":"/sched/ess:every100mSP1"},
            {"value":3,"uri":"/sched/ess:every100mSP2"},
            {"value":4,"uri":"/sched/ess:every100mSP3"},
            {"value":5,"uri":"/sched/ess:fastPub"},
            {"value":6,"uri":"/sched/ess:slowPub"}
        ],
        "actions":	{
            "onSet":	[{
                "func":	[{
                        "debug":true,
                        "func":	"SchedItemOpts",
                        "amap":	"flex",
                        "initDone":true
                    }]
        
            }]
        }
    }

}'

fims_send -m set -r /$$ -u /flex/test/sched ' {
"sched_items2":	{
    "value": true
}
}'
