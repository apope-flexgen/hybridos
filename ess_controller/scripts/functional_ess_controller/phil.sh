




read -n1 -p "Press any key to set up sched function";echo

/usr/local/bin/fims_send -m set -u /ess/cfg/cfile/bms/start_contactors '
{
    "amname": "bms",
    "pname": "ess",
    "/assets/bms/summary": {
        "close_contactors": {
            "actions": {
                "onSet": [
                    {
                        "func": [
                            {
                                "aname": "bms",
                                "func": "LocalStartBMS"
                            }
                        ]
                    }
                ]
            },
            "enabled": true,
            "ifChanged": false,
            "name": "Start/Stop/Standby Command",
            "value": 0
        },
        "open_contactors": {
            "actions": {
                "onSet": [
                    {
                        "func": [
                            {
                                "aname": "bms",
                                "func": "LocalStopBMS"
                            }
                        ]
                    }
                ]
            },
            "enabled": true,
            "ifChanged": false,
            "name": "Start/Stop/Standby Command",
            "value": 0
        },
        "maint_mode": {
            "value": true
        }
    }
}
'

#  /ess/assets/bms/summary '
# {
#     "close_contactors": {
#         "actions": {
#             "onSet": [
#                 {
#                     "func": [
#                         {
#                             "aname": "bms",
#                             "func": "LocalStartBMS"
#                         }
#                     ]
#                 }
#             ]
#         },
#         "enabled": true,
#         "ifChanged": false,
#         "name": "Start/Stop/Standby Command",
#         "value": 0
#     },
#     "open_contactors": {
#         "actions": {
#             "onSet": [
#                 {
#                     "func": [
#                         {
#                             "aname": "bms",
#                             "func": "LocalStopBMS"
#                         }
#                     ]
#                 }
#             ]
#         },
#         "enabled": true,
#         "ifChanged": false,
#         "name": "Start/Stop/Standby Command",
#         "value": 0
#     },
#     "maint_mode": {
#         "value": true
#     }
# } '



# /usr/local/bin/fims_send -m set -r /$$ -u /ess/ '



# /usr/local/bin/fims_send -m set -r /$$ -u /ess/controls/bms  '
# {
#     "CloseContactors": {
#         "value": 0,
#         "cmdVar": "/components/bms:close_contactors",
#         "checkCmdTimeout": 3,
#         "sendCmdTimeout": 3,
#         "triggerCmd": false
#     },
#     "OpenContactors": {
#         "value": 0,
#         "cmdVar": "/components/bms:open_contactors",
#         "checkCmdTimeout": 3,
#         "sendCmdTimeout": 3,
#         "triggerCmd": false
#     }
# }'

# # echo -e "\n\nset up run command."
# fims_send -m set -r /$$ -u /ess/system/commands '
# {
#     "run":{
#         "value":"test",
#         "ifChanged":false, 
#         "enabled":true, 
#         "actions":{
#             "onSet":[{
#                 "func":[{
#                     "amap":"bms",
#                     "func":"RunSched"
#                 }]
#             }]
#         }
#     }
# }'


# # ESS config variables
# /usr/local/bin/fims_send -m set -r /$$ -u /ess/config/bms  '
# {
#     "enable": false
# } '

# # Set up on scheduler
# /usr/local/bin/fims_send -m set -r /$$ -u /ess/sched/bms  '
# {
#     "LocalStartBMS_bms": {
#         "actions": {
#             "onSet": [
#                 {
#                     "func": [
#                         {
#                             "aname": "bms",
#                             "func": "LocalStartBMS"
#                         }
#                     ]
#                 }
#             ]
#         },
#         "enable": "/config/bms:enable",
#         "value": 1
#     },
#     "LocalStopBMS_bms": {
#         "actions": {
#             "onSet": [
#                 {
#                     "func": [
#                         {
#                             "aname": "bms",
#                             "func": "LocalStopBMS"
#                         }
#                     ]
#                 }
#             ]
#         },
#         "enable": "/config/bms:enable",
#         "value": 1
#     }
# } '

# fims_send -m set -r /$$ -u /ess/system/commands '
# {
#     "runOpts_bms": {
#         "actions": {
#             "onSet": [
#                 {
#                     "func": [
#                         {
#                             "amap": "bms",
#                             "func": "SchedItemOpts"
#                         }
#                     ]
#                 }
#             ]
#         },
#         "enabled": true,
#         "note1": "Sets schedule variables (defined in /sched/bms) to /system/commands:run",
#         "note2": "This allows the ESS Controller to kick off scheduled tasks based on the provided variables",
#         "options": [
#             {
#                 "aname": "bms",
#                 "value":1, 
#                 "uri":"/sched/bms:LocalStartBMS_bms",
#                 "every":0.1
#             },
#             {
#                 "aname": "bms",
#                 "value":1, 
#                 "uri":"/sched/bms:LocalStopBMS_bms",
#                 "every":0.1
#             }
#         ],
#         "targav": "/system/commands:run",
#         "value": true
#     }
# }
# '

# /usr/local/bin/fims_send -m set -r /$$ -u /ess/config/bms/enable true


# read -n1 -p "Press any key to send true to /assets/bms/summary/close_contactors";echo

# # BMS UI variables
# /usr/local/bin/fims_send -m set -r /$$ -u /assets/bms/summary/close_contactors true