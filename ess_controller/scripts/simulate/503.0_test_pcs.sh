#!/binsh
# start the pcs simulator
#
# the personality (PowerElctronics or Sungrow)
# can be added after the main system starts.
# the PCS simultor needs to know the tables for the pub operations
# those will be provided or overridden by the personality module.
# we'll set up some defaults here perhaps
#
SIM=flex


echo setup run command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/system/commands '
         {"run":{"value":"test",
                  "help": "run a schedule var",
                   "ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"func":"RunSched"}]}]}}}'

#wait_pause

echo setup stop command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/system/commands '
         {"stop":{"value":"test",
                   "help": "stop a schedule var",
                    "ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"func":"StopSched"}]}]}}}'


# example set up the pub functions for the scheduler
#/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/control/pubs '
#                    { "pubBmsUIHs":{"value":1,"table":"/assets/bms/summary",
#                                  "enabled":true, "actions":{"onSet":[{"func":[{"func":"RunPub"}]}]}}}'

#echo run Bms Publish operations
#/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands/run '
#                    {"value":22,"uri":"/control/pubs:pubBmsUIHs","every":1.0,"offset":0,"debug":0}'


/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/schedule/ess '
{
    "addSchedItem": {
      "value": "None",
      "actions": {
        "onSet": [
          {
            "func": [
              {
                "amap": "ess",
                "func": "HandleSchedItem"
              }
            ]
          }
        ]
      }
    }
}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/sched/pcs '
{ 
    "SimPcs":{
        "value":"dummy",
         "PcsFastPub":"/components/pcs_registers_fast",
         "PcsFastPubRate":100,
         "PcsSlowPub":"/components/pcs_registers_slow",
         "PcsSlowPubRate":2000
         }
}'

# /usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/control/pubs '
#                     { "pubBmsHs":{"value":1,
#                                   "table":"/site/bms_hs",
#                                   "enabled":true, "actions":
#                         {"onSet":[{"func":[{"func":"RunPub"}]}]}}}'


/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/schedule/ess '
{ 
    "addSchedItem":{
        "value":"SimPcs",
        "var":"/sched/pcs:simHandlePcs",
        "debug":1,
        "amap":"pcs",
        "uri":"/sched/pcs:SimPcs", 
        "fcn":"SimHandlePcs",
        "refTime":0.200,
        "runTime":0.200,
        "repTime":1.000,
        "endTime":0
}}
'
# set up a test value for pub
/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/components/pcs_registers_fast '
{ 
    "test_value":{
        "value": 0
         }
}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/sched/pcs '
{ 
    "PcsFastPub":{
        "value":"dummy",
         "table":"/components/pcs_registers_fast",
         "enabled":false
         }
}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/schedule/ess '
{ 
    "addSchedItem":{
        "value":"PcsFastPub",
        "var":"/sched/pcs:pcsFastPub",
        "debug":1,
        "amap":"pcs",
        "uri":"/sched/pcs:PcsFastPub", 
        "fcn":"FastPub",
        "refTime":0.200,
        "runTime":0.200,
        "repTime":1.000,
        "endTime":0
}}
'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/sched/pcs '
{ 
    "PcsSlowPub":{
        "value":"dummy",
         "table":"/components/pcs_registers_slow",
         "enabled":false
         }
}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/schedule/ess '
{ 
    "addSchedItem":{
        "value":"PcsSlowPub",
        "var":"/sched/pcs:pcsSlowPub",
        "debug":1,
        "amap":"pcs",
        "uri":"/sched/pcs:PcsSlowPub", 
        "fcn":"FastPub",
        "refTime":0.200,
        "runTime":0.200,
        "repTime":5.000,
        "endTime":0
}}
'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/sched/pcs '
{ 
    "PcsFastPub":{
        "value":"dummy",
        "table":"/components/pcs_registers_fast",
        "enabled":false,
        "endTime":1
        },
    "PcsSlowPub":{
        "value":"dummy",
        "table":"/components/pcs_registers_slow",
        "enabled":false,
        "endTime":1
         }
}'
