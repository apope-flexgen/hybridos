# set up start and stop command to put something on sched and set up dataMap control

# echo -e "\n\nset up run command."
/usr/local/bin/fims_send -m set -r /$$ -u /ess/system/commands '
         {"run":{"value":"test",
                  "help": "run a schedule var",
                   "ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"amap":"ess","func":"RunSched"}]}]}}}'

# echo -e "\n\nsetup stop command."
/usr/local/bin/fims_send -m set -r /$$ -u /ess/system/commands '
         {"stop":{"value":"test",
                   "help": "stop a schedule var",
                    "ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"amap":"ess","func":"StopSched"}]}]}}}'


# set up the pub functions for the assets scheduler 
/usr/local/bin/fims_send -m set -r /$$ -u /ess/full/control/dataMaps '
                    { "data_maps":{"value":1,"table":"/assets",
                                  "enabled":true, "actions":{"onSet":[{"func":[{"amap":"ess", "func":"runDataMaps"}]}]}}}'


# USING NON-SCHEDULER FUNCS 

# This sets up the system to process the dataMap setup demo code
# fims_send -m set -r /$$ -u /ess/demo/code '{
#     "runExtCode":{
#         "value":0,
#         "actions":{
#             "onSet":[
#                 {"func":
#                     [
#                         {"func":"runDataMaps", "inValue":0},
#                         {"func":"runDataMaps"}
#                     ]
#                 }
#             ]
#         }
#     }
# }'

# this wil set up two bms_racks each with 2 modules
# fims_send -m set -r /$$ -u /ess/demo/code '{"runExtCode":{"value":15,"cmd":"setup_demo","pname":"rack","num_racks":2, "num_modules":2}}'