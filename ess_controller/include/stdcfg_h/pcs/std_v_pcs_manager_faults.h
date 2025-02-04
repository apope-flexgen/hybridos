const char* std_v_pcs_manager_faults_s = R"JSON(
{
    "/faults/pcs": {
        "clear_faults": {
            "value": "Normal",
            "type": "fault",
            "numVars": 16,
            "variable1": "HeartbeatRead",
            "variable2": "PCSDCVoltage",
            "variable3": "PCSDCCurrent",
            "variable4": "PCSDCPower",
            "variable5": "L1L2Voltage",
            "variable6": "L2L3Voltage",
            "variable7": "L3L1Voltage",
            "variable8": "L1Current",
            "variable9": "L2Current",
            "variable10": "L3Current",
            "variable11": "ActivePower",
            "variable12": "ReactivePower",
            "variable13": "ApparentPower",
            "variable14": "Frequency",
            "variable15": "ModulesOnline",
            "variable16": "MaxIGBTTemperature",
            "actions": {
                "onSet": [
                    {"func": [{"func": "process_sys_alarm", "amap": "pcs"}]},
                    {"remap":[
                        {"inValue": "Clear", "ifChanged": false, "uri":"/faults/site:fg_pcs_faults_1", "outValue": 0},
                        {"inValue": "Clear", "ifChanged": false, "uri":"/faults/site:fg_pcs_faults_2", "outValue": 0}
                    ]}
                ]
            }
        },
        "MonitorVarFault": {
            "value": "N/A",
            "actions": {
                "onSet":[{
                    "remap":[
                        {"inValue": "PCSDCVoltage_limit_min"  , "uri":"/faults/site:fg_pcs_faults_1[0]"  , "outValue": true},
                        {"inValue": "PCSDCVoltage_limit_max"  , "uri":"/faults/site:fg_pcs_faults_1[1]"  , "outValue": true},
                        {"inValue": "PCSDCCurrent_limit_min"  , "uri":"/faults/site:fg_pcs_faults_1[2]"  , "outValue": true},
                        {"inValue": "PCSDCCurrent_limit_max"  , "uri":"/faults/site:fg_pcs_faults_1[3]"  , "outValue": true},
                        {"inValue": "PCSDCPower_limit_min"    , "uri":"/faults/site:fg_pcs_faults_1[4]"  , "outValue": true},
                        {"inValue": "PCSDCPower_limit_max"    , "uri":"/faults/site:fg_pcs_faults_1[5]"  , "outValue": true},
                        {"inValue": "L1L2Voltage_limit_max"   , "uri":"/faults/site:fg_pcs_faults_1[6]"  , "outValue": true},
                        {"inValue": "L2L3Voltage_limit_max"   , "uri":"/faults/site:fg_pcs_faults_1[6]"  , "outValue": true},
                        {"inValue": "L3L1Voltage_limit_max"   , "uri":"/faults/site:fg_pcs_faults_1[6]"  , "outValue": true},
                        {"inValue": "L1L2Voltage_limit_min"   , "uri":"/faults/site:fg_pcs_faults_1[7]"  , "outValue": true},
                        {"inValue": "L2L3Voltage_limit_min"   , "uri":"/faults/site:fg_pcs_faults_1[7]"  , "outValue": true},
                        {"inValue": "L3L1Voltage_limit_min"   , "uri":"/faults/site:fg_pcs_faults_1[7]"  , "outValue": true},
                        {"inValue": "L1Current_limit_min"     , "uri":"/faults/site:fg_pcs_faults_1[8]"  , "outValue": true},
                        {"inValue": "L2Current_limit_min"     , "uri":"/faults/site:fg_pcs_faults_1[8]"  , "outValue": true},
                        {"inValue": "L3Current_limit_min"     , "uri":"/faults/site:fg_pcs_faults_1[8]"  , "outValue": true},
                        {"inValue": "L1Current_limit_max"     , "uri":"/faults/site:fg_pcs_faults_1[9]"  , "outValue": true},
                        {"inValue": "L2Current_limit_max"     , "uri":"/faults/site:fg_pcs_faults_1[9]"  , "outValue": true},
                        {"inValue": "L3Current_limit_max"     , "uri":"/faults/site:fg_pcs_faults_1[9]"  , "outValue": true},
                        {"inValue": "ActivePower_limit_min"   , "uri":"/faults/site:fg_pcs_faults_1[10]" , "outValue": true},
                        {"inValue": "ActivePower_limit_max"   , "uri":"/faults/site:fg_pcs_faults_1[11]" , "outValue": true},
                        {"inValue": "ReactivePower_limit_min" , "uri":"/faults/site:fg_pcs_faults_1[12]" , "outValue": true},
                        {"inValue": "ReactivePower_limit_max" , "uri":"/faults/site:fg_pcs_faults_1[13]" , "outValue": true},
                        {"inValue": "ApparentPower_limit_min" , "uri":"/faults/site:fg_pcs_faults_1[14]" , "outValue": true},
                        {"inValue": "ApparentPower_limit_max" , "uri":"/faults/site:fg_pcs_faults_1[15]" , "outValue": true},

                        {"inValue": "Frequency_limit_min"          , "uri":"/faults/site:fg_pcs_faults_2[0]" , "outValue": true},
                        {"inValue": "Frequency_limit_max"          , "uri":"/faults/site:fg_pcs_faults_2[1]" , "outValue": true},
                        {"inValue": "ModulesOnline_limit_min"      , "uri":"/faults/site:fg_pcs_faults_2[2]" , "outValue": true},
                        {"inValue": "MaxIGBTTemperature_limit_max" , "uri":"/faults/site:fg_pcs_faults_2[3]" , "outValue": true}
                    ]
                }]
            }
        }
    },

    "/alarms/pcs": {
        "clear_alarms": {
            "value": "Normal",
            "type": "alarms",
            "numVars": 16,
            "variable1": "HeartbeatRead",
            "variable2": "PCSDCVoltage",
            "variable3": "PCSDCCurrent",
            "variable4": "PCSDCPower",
            "variable5": "L1L2Voltage",
            "variable6": "L2L3Voltage",
            "variable7": "L3L1Voltage",
            "variable8": "L1Current",
            "variable9": "L2Current",
            "variable10": "L3Current",
            "variable11": "ActivePower",
            "variable12": "ReactivePower",
            "variable13": "ApparentPower",
            "variable14": "Frequency",
            "variable15": "ModulesOnline",
            "variable16": "MaxIGBTTemperature",
            "actions": {
                "onSet": [
                    {"func": [{"func": "process_sys_alarm", "amap": "pcs"}]},
                    {"remap":[
                        {"inValue": "Clear", "ifChanged": false, "uri":"/alarms/site:fg_pcs_alarms_1"         , "outValue": 0},
                        {"inValue": "Clear", "ifChanged": false, "uri":"/alarms/site:fg_pcs_alarms_2"         , "outValue": 0},
                        {"inValue": "Clear", "ifChanged": false, "uri":"/alarms/site:fg_pcs_control_alarms_1" , "outValue": 0},
                        {"inValue": "Clear", "ifChanged": false, "uri":"/alarms/site:fg_pcs_control_alarms_2" , "outValue": 0}
                    ]}
                ]
            }
        },
        "MonitorVarAlarm": {
            "value": "N/A",
            "actions": {
                "onSet":[{
                    "remap":[
                        {"inValue": "PCSDCVoltage_limit_min"  , "uri":"/alarms/site:fg_pcs_alarms_1[0]"  , "outValue": true},
                        {"inValue": "PCSDCVoltage_limit_max"  , "uri":"/alarms/site:fg_pcs_alarms_1[1]"  , "outValue": true},
                        {"inValue": "PCSDCCurrent_limit_min"  , "uri":"/alarms/site:fg_pcs_alarms_1[2]"  , "outValue": true},
                        {"inValue": "PCSDCCurrent_limit_max"  , "uri":"/alarms/site:fg_pcs_alarms_1[3]"  , "outValue": true},
                        {"inValue": "PCSDCPower_limit_min"    , "uri":"/alarms/site:fg_pcs_alarms_1[4]"  , "outValue": true},
                        {"inValue": "PCSDCPower_limit_max"    , "uri":"/alarms/site:fg_pcs_alarms_1[5]"  , "outValue": true},
                        {"inValue": "L1L2Voltage_limit_max"   , "uri":"/alarms/site:fg_pcs_alarms_1[6]"  , "outValue": true},
                        {"inValue": "L2L3Voltage_limit_max"   , "uri":"/alarms/site:fg_pcs_alarms_1[6]"  , "outValue": true},
                        {"inValue": "L3L1Voltage_limit_max"   , "uri":"/alarms/site:fg_pcs_alarms_1[6]"  , "outValue": true},
                        {"inValue": "L1L2Voltage_limit_min"   , "uri":"/alarms/site:fg_pcs_alarms_1[7]"  , "outValue": true},
                        {"inValue": "L2L3Voltage_limit_min"   , "uri":"/alarms/site:fg_pcs_alarms_1[7]"  , "outValue": true},
                        {"inValue": "L3L1Voltage_limit_min"   , "uri":"/alarms/site:fg_pcs_alarms_1[7]"  , "outValue": true},
                        {"inValue": "L1Current_limit_min"     , "uri":"/alarms/site:fg_pcs_alarms_1[8]"  , "outValue": true},
                        {"inValue": "L2Current_limit_min"     , "uri":"/alarms/site:fg_pcs_alarms_1[8]"  , "outValue": true},
                        {"inValue": "L3Current_limit_min"     , "uri":"/alarms/site:fg_pcs_alarms_1[8]"  , "outValue": true},
                        {"inValue": "L1Current_limit_max"     , "uri":"/alarms/site:fg_pcs_alarms_1[9]"  , "outValue": true},
                        {"inValue": "L2Current_limit_max"     , "uri":"/alarms/site:fg_pcs_alarms_1[9]"  , "outValue": true},
                        {"inValue": "L3Current_limit_max"     , "uri":"/alarms/site:fg_pcs_alarms_1[9]"  , "outValue": true},
                        {"inValue": "ActivePower_limit_min"   , "uri":"/alarms/site:fg_pcs_alarms_1[10]" , "outValue": true},
                        {"inValue": "ActivePower_limit_max"   , "uri":"/alarms/site:fg_pcs_alarms_1[11]" , "outValue": true},
                        {"inValue": "ReactivePower_limit_min" , "uri":"/alarms/site:fg_pcs_alarms_1[12]" , "outValue": true},
                        {"inValue": "ReactivePower_limit_max" , "uri":"/alarms/site:fg_pcs_alarms_1[13]" , "outValue": true},
                        {"inValue": "ApparentPower_limit_min" , "uri":"/alarms/site:fg_pcs_alarms_1[14]" , "outValue": true},
                        {"inValue": "ApparentPower_limit_max" , "uri":"/alarms/site:fg_pcs_alarms_1[15]" , "outValue": true},

                        {"inValue": "Frequency_limit_min"          , "uri":"/alarms/site:fg_pcs_alarms_2[0]" , "outValue": true},
                        {"inValue": "Frequency_limit_max"          , "uri":"/alarms/site:fg_pcs_alarms_2[1]" , "outValue": true},
                        {"inValue": "ModulesOnline_limit_min"      , "uri":"/alarms/site:fg_pcs_alarms_2[2]" , "outValue": true},
                        {"inValue": "MaxIGBTTemperature_limit_max" , "uri":"/alarms/site:fg_pcs_alarms_2[3]" , "outValue": true}
                    ]
                }]
            }
        },
        "ActivePowerCmdPrecheck"           : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "ActivePowerCmdVerify"             : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "ActivePowerRampRatePrecheck"      : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "ActivePowerRampRateVerify"        : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "ClearFaultsPrecheck"              : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "GridFollowPQPrecheck"             : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "GridFollowPQVerify"               : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "GridFormVFPrecheck"               : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "GridFormVFVerify"                 : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "GridFormVSGPrecheck"              : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "GridFormVSGVerify"                : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "OffGridFrequencySetpointPrecheck" : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "OffGridFrequencySetpointVerify"   : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "OffGridVoltageSetpointPrecheck"   : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "OffGridVoltageSetpointVerify"     : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "ReactivePowerCmdPrecheck"         : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "ReactivePowerCmdVerify"           : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "ReactivePowerRampRatePrecheck"    : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "ReactivePowerRampRateVerify"      : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "StandbyPrecheck"                  : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "StandbyVerify"                    : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "StartPrecheck"                    : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "StartVerify"                      : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "StopPrecheck"                     : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}},
        "StopVerify"                       : {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "pcs"}]}]}}
    }
}
)JSON";
