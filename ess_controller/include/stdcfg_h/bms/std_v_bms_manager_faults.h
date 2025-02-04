const char* std_v_bms_manager_faults_s = R"JSON(
{
    "/faults/bms": {
        "clear_faults": {
            "value": "N/A",
            "type": "fault",
            "numVars": 0,
            "actions": {
                "onSet": [
                    {"func": [{"func": "process_sys_alarm", "amap": "bms"}]},
                    {"remap":[
                        {"inValue": "Clear", "ifChanged": false, "uri":"/faults/site:fg_bms_faults_1", "outValue": 0},
                        {"inValue": "Clear", "ifChanged": false, "uri":"/faults/site:fg_bms_faults_2", "outValue": 0}
                    ]}
                ]
                
            }
        }
    },
    "/alarms/bms": {
        "clear_alarms": {
            "value": "N/A",
            "type": "alarms",
            "numVars": 0,
            "actions": {
                "onSet": [
                    {"func": [{"func": "process_sys_alarm", "amap": "bms"}]},
                    {"remap":[
                        {"inValue": "Clear", "ifChanged": false, "uri":"/alarms/site:fg_bms_alarms_1"         , "outValue": 0},
                        {"inValue": "Clear", "ifChanged": false, "uri":"/alarms/site:fg_bms_alarms_2"         , "outValue": 0},
                        {"inValue": "Clear", "ifChanged": false, "uri":"/alarms/site:fg_bms_control_alarms_1" , "outValue": 0},
                        {"inValue": "Clear", "ifChanged": false, "uri":"/alarms/site:fg_bms_control_alarms_2" , "outValue": 0}
                    ]}
                ]
            }
        }
    }
}
)JSON";
