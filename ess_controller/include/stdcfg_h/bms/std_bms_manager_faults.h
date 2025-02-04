const char* std_bms_manager_faults_s = R"JSON(
{
    "/faults/##BMS_ID##": {
        "clear_faults": {
            "value": "N/A",
            "type": "fault",
            "numVars": 13,
            "variable1"  : "HeartbeatRead"    ,
            "variable2"  : "MinCellVoltage"   ,
            "variable3"  : "MaxCellVoltage"   ,
            "variable4"  : "CellVoltageDelta" ,
            "variable5"  : "MinCellTemp"      ,
            "variable6"  : "MaxCellTemp"      ,
            "variable7"  : "CellTempDelta"    ,
            "variable8"  : "DCVoltage"        ,
            "variable9"  : "DCCurrent"        ,
            "variable10" : "DCPower"          ,
            "variable11" : "SOC"              ,
            "variable12" : "SOH"              ,
            "variable13" : "RacksInService"   ,
            "actions": {
                "onSet": [
                    {"func": [{"func": "process_sys_alarm", "amap": "##BMS_ID##"}]}
                ]
            }
        },
        "MonitorVarFault": {
            "value": "N/A",
            "actions": {
                "onSet":[{
                    "remap":[
                        {"inValue": "MinCellVoltage_limit_min"   , "uri":"/faults/site:fg_bms_faults_1[0]"  , "outValue": true},
                        {"inValue": "MaxCellVoltage_limit_max"   , "uri":"/faults/site:fg_bms_faults_1[1]"  , "outValue": true},
                        {"inValue": "CellVoltageDelta_limit_max" , "uri":"/faults/site:fg_bms_faults_1[2]"  , "outValue": true},
                        {"inValue": "MinCellTemp_limit_min"      , "uri":"/faults/site:fg_bms_faults_1[3]"  , "outValue": true},
                        {"inValue": "MaxCellTemp_limit_max"      , "uri":"/faults/site:fg_bms_faults_1[4]"  , "outValue": true},
                        {"inValue": "CellTempDelta_limit_max"    , "uri":"/faults/site:fg_bms_faults_1[5]"  , "outValue": true},
                        {"inValue": "DCVoltage_limit_min"        , "uri":"/faults/site:fg_bms_faults_1[6]"  , "outValue": true},
                        {"inValue": "DCVoltage_limit_max"        , "uri":"/faults/site:fg_bms_faults_1[7]"  , "outValue": true},
                        {"inValue": "DCCurrent_limit_min"        , "uri":"/faults/site:fg_bms_faults_1[8]"  , "outValue": true},
                        {"inValue": "DCCurrent_limit_max"        , "uri":"/faults/site:fg_bms_faults_1[9]"  , "outValue": true},
                        {"inValue": "DCPower_limit_min"          , "uri":"/faults/site:fg_bms_faults_1[10]" , "outValue": true},
                        {"inValue": "DCPower_limit_max"          , "uri":"/faults/site:fg_bms_faults_1[11]" , "outValue": true},
                        {"inValue": "SOC_limit_min"              , "uri":"/faults/site:fg_bms_faults_1[12]" , "outValue": true},
                        {"inValue": "SOC_limit_max"              , "uri":"/faults/site:fg_bms_faults_1[13]" , "outValue": true},
                        {"inValue": "SOH_limit_min"              , "uri":"/faults/site:fg_bms_faults_1[14]" , "outValue": true},
                        {"inValue": "RacksInService_limit_min"   , "uri":"/faults/site:fg_bms_faults_1[15]" , "outValue": true}
                    ]
                }]
            }
        }
    },
    "/alarms/##BMS_ID##": {
        "clear_alarms": {
            "value": "N/A",
            "type": "alarms",
            "numVars": 13,
            "variable1" : "HeartbeatRead"   ,
            "variable2" : "MinCellVoltage"  ,
            "variable3" : "MaxCellVoltage"  ,
            "variable4" : "CellVoltageDelta",
            "variable5" : "MinCellTemp"     ,
            "variable6" : "MaxCellTemp"     ,
            "variable7" : "CellTempDelta"   ,
            "variable8" : "DCVoltage"       ,
            "variable9": "DCCurrent"       ,
            "variable10": "DCPower"         ,
            "variable11": "SOC"             ,
            "variable12": "SOH"             ,
            "variable13": "RacksInService"  ,
            "actions": {
                "onSet": [
                    {"func": [{"func": "process_sys_alarm", "amap": "##BMS_ID##"}]}
                ]
            }
        },
        "ClearFaultsCmd"          : {"value": "Normal","type": "alarm","actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "##BMS_ID##"}]}]}},
        "CloseContactorsCmd"      : {"value": "Normal","type": "alarm","actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "##BMS_ID##"}]}]}},
        "CloseContactorsCmdVerify": {"value": "Normal","type": "alarm","actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "##BMS_ID##"}]}]}},
        "OpenContactorsCmd"       : {"value": "Normal","type": "alarm","actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "##BMS_ID##"}]}]}},
        "OpenContactorsCmdVerify" : {"value": "Normal","type": "alarm","actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "##BMS_ID##"}]}]}},
        "MonitorVarAlarm": {
            "value": "N/A",
            "actions": {
                "onSet":[{
                    "remap":[
                        {"inValue": "MinCellVoltage_limit_min"   , "uri":"/alarms/site:fg_bms_alarms_1[0]"  , "outValue": true},
                        {"inValue": "MaxCellVoltage_limit_max"   , "uri":"/alarms/site:fg_bms_alarms_1[1]"  , "outValue": true},
                        {"inValue": "CellVoltageDelta_limit_max" , "uri":"/alarms/site:fg_bms_alarms_1[2]"  , "outValue": true},
                        {"inValue": "MinCellTemp_limit_min"      , "uri":"/alarms/site:fg_bms_alarms_1[3]"  , "outValue": true},
                        {"inValue": "MaxCellTemp_limit_max"      , "uri":"/alarms/site:fg_bms_alarms_1[4]"  , "outValue": true},
                        {"inValue": "CellTempDelta_limit_max"    , "uri":"/alarms/site:fg_bms_alarms_1[5]"  , "outValue": true},
                        {"inValue": "DCVoltage_limit_min"        , "uri":"/alarms/site:fg_bms_alarms_1[6]"  , "outValue": true},
                        {"inValue": "DCVoltage_limit_max"        , "uri":"/alarms/site:fg_bms_alarms_1[7]"  , "outValue": true},
                        {"inValue": "DCCurrent_limit_min"        , "uri":"/alarms/site:fg_bms_alarms_1[8]"  , "outValue": true},
                        {"inValue": "DCCurrent_limit_max"        , "uri":"/alarms/site:fg_bms_alarms_1[9]"  , "outValue": true},
                        {"inValue": "DCPower_limit_min"          , "uri":"/alarms/site:fg_bms_alarms_1[10]" , "outValue": true},
                        {"inValue": "DCPower_limit_max"          , "uri":"/alarms/site:fg_bms_alarms_1[11]" , "outValue": true},
                        {"inValue": "SOC_limit_min"              , "uri":"/alarms/site:fg_bms_alarms_1[12]" , "outValue": true},
                        {"inValue": "SOC_limit_max"              , "uri":"/alarms/site:fg_bms_alarms_1[13]" , "outValue": true},
                        {"inValue": "SOH_limit_min"              , "uri":"/alarms/site:fg_bms_alarms_1[14]" , "outValue": true},
                        {"inValue": "RacksInService_limit_min"   , "uri":"/alarms/site:fg_bms_alarms_1[15]" , "outValue": true}
                    ]
                }]
            }
        }
    }
}
)JSON";
