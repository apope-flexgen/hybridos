#pragma once

const char* std_v_pcs_manager_controls_s = R"JSON(
{
    "/status/bms": {
        "DCClosedStatus": "N/A",
        "CurrentCheckDone": false
    },

    "/schedule/wake_monitor/pcs":{
        "/controls/pcs:ActivePowerCmd"                 : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:ActivePowerRampRate"            : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:ClearFaults"                    : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:GridFollowPQ"                   : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:GridFormVF"                     : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:GridFormVSG"                    : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:OffGridFrequencySetpoint"       : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:OffGridVoltageSetpoint"         : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:ReactivePowerCmd"               : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:ReactivePowerRampRate"          : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:Standby"                        : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:Start"                          : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:Stop"                           : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:VerifyActivePowerCmd"           : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:VerifyActivePowerRampRate"      : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:VerifyGridFollowPQ"             : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:VerifyGridFormVF"               : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:VerifyGridFormVSG"              : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:VerifyOffGridFrequencySetpoint" : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:VerifyOffGridVoltageSetpoint"   : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:VerifyReactivePowerCmd"         : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:VerifyReactivePowerRampRate"    : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:VerifyStandby"                  : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:VerifyStart"                    : {"func": "HandleCmd", "amap": "pcs"},
        "/controls/pcs:VerifyStop"                     : {"func": "HandleCmd", "amap": "pcs"},
        
        "/controls/pcs:ActivePowerCmdPass"   : {"func": "CalculateVar"},
        "/controls/pcs:ReactivePowerCmdPass" : {"func": "CalculateVar"},

        "/status/pcs:ActivePowerRampRateEnable"             : {"func": "CalculateVar"},
        "/status/pcs:ActivePowerSetpointEnable"             : {"func": "CalculateVar"},
        "/status/pcs:ClearFaultsEnable"                     : {"func": "CalculateVar"},
        "/status/pcs:GridFollowPQEnable"                    : {"func": "CalculateVar"},
        "/status/pcs:GridFormVFEnable"                      : {"func": "CalculateVar"},
        "/status/pcs:GridFormVSGEnable"                     : {"func": "CalculateVar"},
        "/status/pcs:ReactivePowerRampRateEnable"           : {"func": "CalculateVar"},
        "/status/pcs:ReactivePowerSetpointEnable"           : {"func": "CalculateVar"},
        "/status/pcs:StandbyEnable"                         : {"func": "CalculateVar"},
        "/status/pcs:StartEnable"                           : {"func": "CalculateVar"},
        "/status/pcs:StopEnable"                            : {"func": "CalculateVar"}
    },
    "/controls/pcs": {
        "ActivePowerSetpoint": {
            "actions": {
                "onSet": [{
                    "func": [
                        {"func": "HandlePowerCmd"},
                        {
                            "amap": "pcs",
                            "inAv": "/controls/pcs:ActivePowerCmdPass",
                            "func": "CalculateVar"
                        }
                    ]
                }]
            }
        },
        "ReactivePowerSetpoint": 0,
        "ActivePowerCmdReal": 0,
        "ReactivePowerCmdReal": 0,
        "Start": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 5,
            "useExpr": true,
            "numVars": 2,
            "variable1": "/status/pcs:SystemStateStatus",
            "variable2": "/status/bms:DCClosedStatus",
            "expression": "({1} == Stop or {1} == Standby) and {2} == Closed"
        },
        "VerifyStart": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 30,
            "numVars": 1,
            "variable1": "/status/pcs:SystemStateStatus",
            "useExpr": true,
            "expression": "{1} == Run"
        },
        "Standby": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 5,
            "useExpr": true,
            "numVars": 2,
            "variable1": "/status/pcs:SystemStateStatus",
            "variable2": "/status/bms:DCClosedStatus",
            "expression": " {1} == Stop and {2} == Closed"
        },
        "VerifyStandby": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 30,
            "numVars": 1,
            "variable1": "/status/pcs:SystemStateStatus",
            "useExpr": true,
            "expression": "{1} == Standby"
        },
        "Stop": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 10,
            "useExpr": true,
            "numVars": 2,
            "variable1": "/status/bms:CurrentCheckDone",
            "variable2": "/status/pcs:GridModeStatus",
            "expression": "{1} or {2} != FollowPQ"
        },
        "VerifyStop": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 30,
            "numVars": 1,
            "variable1": "/status/pcs:SystemStateStatus",
            "useExpr": true,
            "expression": "{1} == Stop or {1} == Fault"
        },
        "GridFollowPQ": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 5,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/status/pcs:SystemStateStatus",
            "expression": "{1} == Stop or {1} == Fault"
        },
        "VerifyGridFollowPQ": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 30,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/status/pcs:GridModeStatus",
            "expression": "{1} == FollowPQ"
        },
        "GridFormVF": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 5,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/status/pcs:SystemStateStatus",
            "expression": "{1} == Stop or {1} == Fault"
        },
        "VerifyGridFormVF": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 30,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/status/pcs:GridModeStatus",
            "expression": "{1} == FormVF"
        },
        "GridFormVSG": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 5,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/status/pcs:SystemStateStatus",
            "expression": "{1} == Stop or {1} == Fault"
        },
        "VerifyGridFormVSG": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 30,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/status/pcs:GridModeStatus",
            "expression": "{1} == FormVSG"
        },
        "ActivePowerCmdPass": {
            "value": 0,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/controls/pcs:ActivePowerCmdReal",
            "expression": "{1}",
            "actions": {
                "onSet": [
                    {"remap": [
                        {"ifChanged": false, "uri": "/controls/pcs:ActivePowerCmd@triggerCmd", "outValue": true},
                        {"ifChanged": false, "uri": "/controls/pcs:ActivePowerCmd"                             }
                    ]}
                ]
            }
        },
        "ActivePowerCmd": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 5,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/status/pcs:SystemStateStatus",
            "expression": "{1} != Stop or {1} != Fault",
            "actions": {
                "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
            }
        },
        "VerifyActivePowerCmd": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 30,
            "useExpr": true,
            "numVars": 3,
            "variable1": "/controls/pcs:ActivePowerCmd",
            "variable2": "/status/pcs:ActivePower",
            "variable3": "/config/pcs:PowerMargin",
            "expression": "({1} - {3}) <= {2} <= ({1} + {3})"
        },
        "ReactivePowerCmdPass": {
            "value": 0,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/controls/pcs:ReactivePowerCmdReal",
            "expression": "{1}",
            "actions": {
                "onSet": [
                    {"remap": [
                        {"ifChanged": false, "uri": "/controls/pcs:ReactivePowerCmd@triggerCmd", "outValue": true},
                        {"ifChanged": false, "uri": "/controls/pcs:ReactivePowerCmd"                             }
                    ]}
                ]
            }
        },
        "ReactivePowerCmd": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 5,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/status/pcs:SystemStateStatus",
            "expression": "{1} != Stop or {1} != Fault",
            "actions": {
                "onSet": [ {"func": [ {"func": "HandleCmd", "amap": "pcs"} ]}]
            }
        },
        "VerifyReactivePowerCmd": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 30,
            "useExpr": true,
            "numVars": 3,
            "variable1": "/controls/pcs:ReactivePowerCmd",
            "variable2": "/status/pcs:ReactivePower",
            "variable3": "/config/pcs:PowerMargin",
            "expression": "({1} - {3}) <= {2} <= ({1} + {3})"
        },
        "ActivePowerRampRate": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 5,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/controls/pcs:ActivePowerRampRate",
            "expression": "{1} > 0",
            "actions": {
                "onSet": [ {
                    "func": [ {"func": "HandleCmd", "amap": "pcs"} ]
                }]
            }
        },
        "VerifyActivePowerRampRate": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 30,
            "useExpr": true,
            "numVars": 3,
            "variable1": "/controls/pcs:ActivePowerRampRate",
            "variable2": "/status/pcs:ActivePowerRampRateSetpointFeedback",
            "variable3": "/config/pcs:RampRateMargin",
            "expression": "({1} - {3}) <= {2} <= ({1} + {3})"
        },
        "ReactivePowerRampRate": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 5,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/controls/pcs:ReactivePowerRampRate",
            "expression": "{1} > 0",
            "actions": {
                "onSet": [ {
                    "func": [ {"func": "HandleCmd", "amap": "pcs"} ]
                }]
            }
        },
        "VerifyReactivePowerRampRate": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 30,
            "useExpr": true,
            "numVars": 3,
            "variable1": "/controls/pcs:ReactivePowerRampRate",
            "variable2": "/status/pcs:ReactivePowerRampRateSetpointFeedback",
            "variable3": "/config/pcs:RampRateMargin",
            "expression": "({1} - {3}) <= {2} <= ({1} + {3})"
        },
        "OffGridFrequencySetpoint": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 5,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/controls/pcs:OffGridFrequencySetpoint",
            "expression": "{1} > 0",
            "actions": {
                "onSet": [ {
                    "func": [ {"func": "HandleCmd", "amap": "pcs"} ]
                }]
            }
        },
        "VerifyOffGridFrequencySetpoint": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 30,
            "useExpr": true,
            "numVars": 3,
            "variable1": "/controls/pcs:OffGridFrequencySetpoint",
            "variable2": "/status/pcs:OffGridFrequencySetpointFeedback",
            "variable3": "/config/pcs:OffGridFrequencyMargin",
            "expression": "({1} - {3}) <= {2} <= ({1} + {3})",
            "debug": true
        },
        "OffGridVoltageSetpoint": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 5,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/controls/pcs:OffGridVoltageSetpoint",
            "expression": "{1} > 0",
            "actions": {
                "onSet": [ {
                    "func": [ {"func": "HandleCmd", "amap": "pcs"} ]
                }]
            }
        },
        "VerifyOffGridVoltageSetpoint": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 30,
            "useExpr": true,
            "numVars": 3,
            "variable1": "/controls/pcs:OffGridVoltageSetpoint",
            "variable2": "/status/pcs:OffGridVoltageSetpointFeedback",
            "variable3": "/config/pcs:OffGridVoltageMargin",
            "expression": "({1} - {3}) <= {2} <= ({1} + {3})"
        },
        "ClearFaults": {
            "value": 0,
            "enableAlert": true
        }
    },

    "/status/pcs": {
        "StartEnable": {
            "value": 0,
            "useExpr": true,
            "numVars": 3,
            "variable1": "/status/bms:DCClosedStatus",
            "variable2": "/assets/pcs/summary:maint_mode",
            "variable3": "/status/pcs:SystemStateStatus",
            "expression": "{1} == Closed and {2} and ({3} == Stop or {3} == Standby)",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue":0, "uri":"/assets/pcs/summary:start@enabled", "outValue":false},
                        {"inValue":1, "uri":"/assets/pcs/summary:start@enabled", "outValue":true }
                    ]
                }]
            }
        },
        "StandbyEnable": {
            "value": 0,
            "useExpr": true,
            "numVars": 3,
            "variable1": "/status/bms:DCClosedStatus",
            "variable2": "/assets/pcs/summary:maint_mode",
            "variable3": "/status/pcs:SystemStateStatus",
            "expression": "{1} == Closed and {2} and {3} == Stop",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue":0, "uri":"/assets/pcs/summary:standby@enabled", "outValue":false},
                        {"inValue":1, "uri":"/assets/pcs/summary:standby@enabled", "outValue":true }
                    ]
                }]
            }
        },
        "StopEnable": {
            "value": 0,
            "useExpr": true,
            "numVars": 3,
            "variable1": "/assets/pcs/summary:maint_mode",
            "variable2": "/status/pcs:SystemStateStatus",
            "variable3": "/status/pcs:GridModeStatus",
            "expression": "({1} and {2} != Stop) or {3} != FollowPQ",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue":0, "uri":"/assets/pcs/summary:stop@enabled", "outValue":false},
                        {"inValue":1, "uri":"/assets/pcs/summary:stop@enabled", "outValue":true}
                    ]
                }]
            }
        },
        "GridFollowPQEnable": {
            "value": 0,
            "numVars": 3,
            "variable1": "/assets/pcs/summary:maint_mode",
            "variable2": "/status/pcs:SystemStateStatus",
            "variable3": "/status/pcs:GridModeStatus",
            "useExpr": true,
            "expression": "{1} and ({2} == Stop or {2} == Fault) and {3} != FollowPQ",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue":0, "uri":"/assets/pcs/summary:grid_follow_pq@enabled", "outValue":false},
                        {"inValue":1, "uri":"/assets/pcs/summary:grid_follow_pq@enabled", "outValue":true }
                    ]
                }]
            }
        },
        "GridFormVFEnable": {
            "value": 0,
            "numVars": 3,
            "variable1": "/assets/pcs/summary:maint_mode",
            "variable2": "/status/pcs:SystemStateStatus",
            "variable3": "/status/pcs:GridModeStatus",
            "useExpr": true,
            "expression": "{1} and ({2} == Stop or {2} == Fault) and {3} == FollowPQ",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue":0, "uri":"/assets/pcs/summary:grid_form_vf@enabled", "outValue":false},
                        {"inValue":1, "uri":"/assets/pcs/summary:grid_form_vf@enabled", "outValue":true }
                    ]
                }]
            }
        },
        "GridFormVSGEnable": {
            "value": 0,
            "numVars": 3,
            "variable1": "/assets/pcs/summary:maint_mode",
            "variable2": "/status/pcs:SystemStateStatus",
            "variable3": "/status/pcs:GridModeStatus",
            "useExpr": true,
            "expression": "{1} and ({2} == Stop or {2} == Fault) and {3} == FollowPQ",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue":0, "uri":"/assets/pcs/summary:grid_form_vsg@enabled", "outValue":false},
                        {"inValue":1, "uri":"/assets/pcs/summary:grid_form_vsg@enabled", "outValue":true }
                    ]
                }]
            }
        },
        "ActivePowerSetpointEnable": {
            "value": 0,
            "numVars": 2,
            "variable1": "/assets/pcs/summary:maint_mode",
            "variable2": "/status/pcs:SystemStateStatus",
            "useExpr": true,
            "expression": "{1} and ({2} != Stop and {2} != Fault)",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": 0, "uri":"/assets/pcs/summary:active_power_setpoint@enabled", "outValue": false},
                        {"inValue": 1, "uri":"/assets/pcs/summary:active_power_setpoint@enabled", "outValue": true }
                    ]
                }]
            }
        },
        "ReactivePowerSetpointEnable": {
            "value": 0,
            "numVars": 2,
            "variable1": "/assets/pcs/summary:maint_mode",
            "variable2": "/status/pcs:SystemStateStatus",
            "useExpr": true,
            "expression": "{1} and ({2} != Stop and {2} != Fault)",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue":0, "uri":"/assets/pcs/summary:reactive_power_setpoint@enabled", "outValue": false},
                        {"inValue":1, "uri":"/assets/pcs/summary:reactive_power_setpoint@enabled", "outValue": true }
                    ]
                }]
            }
        },
        "ActivePowerRampRateEnable": {
            "value": 0,
            "numVars": 1,
            "variable1": "/assets/pcs/summary:maint_mode",
            "useExpr": true,
            "expression": "{1}",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": 0, "uri":"/assets/pcs/summary:active_power_ramp_rate@enabled", "outValue": false},
                        {"inValue": 1, "uri":"/assets/pcs/summary:active_power_ramp_rate@enabled", "outValue": true }
                    ]
                }]
            }
        },
        "ReactivePowerRampRateEnable": {
            "value": 0,
            "numVars": 1,
            "variable1": "/assets/pcs/summary:maint_mode",
            "useExpr": true,
            "expression": "{1}",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue":0, "uri":"/assets/pcs/summary:reactive_power_ramp_rate@enabled", "outValue": false},
                        {"inValue":1, "uri":"/assets/pcs/summary:reactive_power_ramp_rate@enabled", "outValue": true }
                    ]
                }]
            }
        },
        "ClearFaultsEnable": {
            "value": 0,
            "useExpr": true,
            "numVars": 2,
            "variable1": "/status/pcs:IsFaulted",
            "variable2": "/status/pcs:IsAlarming",
            "expression": "{1} or {2}",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue":0, "uri":"/assets/pcs/summary:clear_faults@enabled", "outValue": false},
                        {"inValue":1, "uri":"/assets/pcs/summary:clear_faults@enabled", "outValue": true }
                    ]
                }]
            }
        },
        "StartSuccess": {
            "value": false,
            "actions": {
                "onSet": [
                    {"remap": [
                        {"inValue": true, "ifChanged": false, "uri": "/controls/pcs:VerifyStart@triggerCmd" , "outValue": true},

                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_1[1]" , "outValue": true                                                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:StartPrecheck"                    , "outValue": "Command preconditions were not met, the command was not sent. Details can be found in the ESS Events log."}
                    ]}
                ]
            }
        },
        "VerifyStartSuccess": {
            "value": false,
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_1[2]" , "outValue": true                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:StartVerify"              , "outValue": "Command was sent but the expected result was not observed before timeout."}
                    ]
                }]
            }
        },
        "StandbySuccess": {
            "value": false,
            "actions": {
                "onSet": [
                    {"remap": [
                        {"inValue": true, "ifChanged": false, "uri": "/controls/pcs:VerifyStandby@triggerCmd", "outValue": true},

                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_1[3]" , "outValue": true                                                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:StandbyPrecheck"                  , "outValue": "Command preconditions were not met, the command was not sent. Details can be found in the ESS Events log."}
                    ]}
                ]
            }
        },
        "VerifyStandbySuccess": {
            "value": false,
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_1[4]" , "outValue": true                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:StandbyVerify"            , "outValue": "Command was sent but the expected result was not observed before timeout."}
                    ]
                }]
            }
        },
        "StopSuccess": {
            "value": false,
            "actions": {
                "onSet": [
                    {"remap": [
                        {"inValue": true, "ifChanged": false, "uri": "/controls/pcs:VerifyStop@triggerCmd"       , "outValue": true },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_1[5]" , "outValue": true                                                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:StopPrecheck"                , "outValue": "Command preconditions were not met, the command was not sent. Details can be found in the ESS Events log."}
                    ]}
                ]
            }
        },
        "VerifyStopSuccess": {
            "value": false,
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_1[6]" , "outValue": true                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:StopVerify"               , "outValue": "Command was sent but the expected result was not observed before timeout."}
                    ]
                }]
            }
        },
        "GridFollowPQSuccess": {
            "value": false,
            "actions": {
                "onSet": [
                    {"remap": [
                        {"inValue": true, "ifChanged": false, "uri": "/controls/pcs:VerifyGridFollowPQ@triggerCmd", "outValue": true},

                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_1[7]" , "outValue": true                                                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:GridFollowPQPrecheck"             , "outValue": "Command preconditions were not met, the command was not sent. Details can be found in the ESS Events log."}
                    ]}
                ]
            }
        },
        "VerifyGridFollowPQSuccess": {
            "value": false,
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_1[8]" , "outValue": true                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:GridFollowPQVerify"       , "outValue": "Command was sent but the expected result was not observed before timeout."}
                    ]
                }]
            }
        },
        "GridFormVFSuccess": {
            "value": false,
            "actions": {
                "onSet": [
                    {"remap": [
                        {"inValue": true, "ifChanged": false, "uri": "/controls/pcs:VerifyGridFormVF@triggerCmd", "outValue": true},

                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_1[9]" , "outValue": true                                                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:GridFormVFPrecheck"               , "outValue": "Command preconditions were not met, the command was not sent. Details can be found in the ESS Events log."}
                    ]}
                ]
            }
        },
        "VerifyGridFormVFSuccess": {
            "value": false,
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_1[10]" , "outValue": true                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:GridFormVFVerify"          , "outValue": "Command was sent but the expected result was not observed before timeout."}
                    ]
                }]
            }
        },
        "GridFormVSGSuccess": {
            "value": false,
            "actions": {
                "onSet": [
                    {"remap": [
                        {"inValue": true, "ifChanged": false, "uri": "/controls/pcs:VerifyGridFormVSG@triggerCmd", "outValue": true},

                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_1[11]" , "outValue": true                                                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:GridFormVSGPrecheck"               , "outValue": "Command preconditions were not met, the command was not sent. Details can be found in the ESS Events log."}
                    ]}
                ]
            }
        },
        "VerifyGridFormVSGSuccess": {
            "value": false,
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_1[12]" , "outValue": true                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:GridFormVSGVerify"         , "outValue": "Command was sent but the expected result was not observed before timeout."}
                    ]
                }]
            }
        },
        "ActivePowerCmdSuccess": {
            "value": false,
            "actions": {
                "onSet": [
                    {"remap": [
                        {"inValue": true, "ifChanged": false, "uri": "/controls/pcs:VerifyActivePowerCmd@triggerCmd", "outValue": true},

                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_2[0]" , "outValue": true                                                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:ActivePowerCmdPrecheck"           , "outValue": "Command preconditions were not met, the command was not sent. Details can be found in the ESS Events log."}
                    ]}
                ]
            }
        },
        "VerifyActivePowerCmdSuccess": {
            "value": false,
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_2[1]" , "outValue": true                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:ActivePowerCmdVerify"     , "outValue": "Command was sent but the expected result was not observed before timeout."}
                    ]
                }]
            }
        },
        "ReactivePowerCmdSuccess": {
            "value": false,
            "actions": {
                "onSet": [
                    {"remap": [
                        {"inValue": true, "ifChanged": false, "uri": "/controls/pcs:VerifyReactivePower@triggerCmd", "outValue": true},

                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_2[2]" , "outValue": true                                                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:ReactivePowerCmdPrecheck"            , "outValue": "Command preconditions were not met, the command was not sent. Details can be found in the ESS Events log."}
                    ]}
                ]
            }
        },
        "VerifyReactivePowerCmdSuccess": {
            "value": false,
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_2[3]" , "outValue": true                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:ReactivePowerCmdVerify"      , "outValue": "Command was sent but the expected result was not observed before timeout."}
                    ]
                }]
            }
        },
        "ActivePowerRampRateSuccess": {
            "value": false,
            "actions": {
                "onSet": [
                    {"remap": [
                        {"inValue": true, "ifChanged": false, "uri": "/controls/pcs:VerifyActivePowerRampRate@triggerCmd", "outValue": true},

                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_2[4]" , "outValue": true                                                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:ActivePowerRampRatePrecheck" , "outValue": "Command preconditions were not met, the command was not sent. Details can be found in the ESS Events log."}
                    ]}
                ]
            }
        },
        "VerifyActivePowerRampRateSuccess": {
            "value": false,
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_2[5]" , "outValue": true                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:ActivePowerRampRateVerify"   , "outValue": "Command was sent but the expected result was not observed before timeout."}
                    ]
                }]
            }
        },
        "ReactivePowerRampRateSuccess": {
            "value": false,
            "actions": {
                "onSet": [
                    {"remap": [
                        {"inValue": true, "ifChanged": false, "uri": "/controls/pcs:VerifyReactivePowerRampRate@triggerCmd", "outValue": true},

                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_2[6]"   , "outValue": true                                                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:ReactivePowerRampRatePrecheck" , "outValue": "Command preconditions were not met, the command was not sent. Details can be found in the ESS Events log."}
                    ]}
                ]
            }
        },
        "VerifyReactivePowerRampRateSuccess": {
            "value": false,
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_2[7]" , "outValue": true                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:ReactivePowerRampRateVerify" , "outValue": "Command was sent but the expected result was not observed before timeout."}
                    ]
                }]
            }
        },
        "OffGridFrequencySetpointSuccess": {
            "value": false,
            "actions": {
                "onSet": [
                    {"remap": [
                        {"inValue": true, "ifChanged": false, "uri": "/controls/pcs:VerifyOffGridFrequencySetpoint@triggerCmd", "outValue": true},

                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_2[8]" , "outValue": true                                                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:OffGridFrequencySetpointPrecheck" , "outValue": "Command preconditions were not met, the command was not sent. Details can be found in the ESS Events log."}
                    ]}
                ]
            }
        },
        "VerifyOffGridFrequencySetpointSuccess": {
            "value": false,
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_2[9]" , "outValue": true                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:OffGridFrequencySetpointVerify"   , "outValue": "Command was sent but the expected result was not observed before timeout."}
                    ]
                }]
            }
        },
        "OffGridVoltageSetpointSuccess": {
            "value": false,
            "actions": {
                "onSet": [
                    {"remap": [
                        {"inValue": true, "ifChanged": false, "uri": "/controls/pcs:VerifyOffGridVoltageSetpoint@triggerCmd", "outValue": true},

                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_2[10]"   , "outValue": true                                                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:OffGridVoltageSetpointPrecheck" , "outValue": "Command preconditions were not met, the command was not sent. Details can be found in the ESS Events log."}
                    ]}
                ]
            }
        },
        "VerifyOffGridVoltageSetpointSuccess": {
            "value": false,
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_2[11]" , "outValue": true                                                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:OffGridVoltageSetpointVerify" , "outValue": "Command was sent but the expected result was not observed before timeout."}
                    ]
                }]
            }
        },
        "ClearFaultsSuccess": {
            "value": false,
            "actions": {
                "onSet": [
                    {"remap": [
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_pcs_control_alarms_1[0]" , "outValue": true},
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:ClearFaultsPrecheck" , "outValue": "Command preconditions were not met, the command was not sent. Details can be found in the ESS Events log." },
                        
                        {"inValue": true , "ifChanged": false, "uri": "/faults/pcs:clear_faults"  , "outValue": "Clear" },
                        {"inValue": true , "ifChanged": false, "uri": "/alarms/pcs:clear_alarms"  , "outValue": "Clear" },
                        {"inValue": true , "ifChanged": false, "uri": "/status/pcs:FaultShutdown" , "outValue": false   }
                    ]}
                ]
            }
        }
    }
}
)JSON";