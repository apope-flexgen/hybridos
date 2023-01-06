/usr/local/bin/fims/fims_send -m set -r/$$ -u/flex/components/ems_running_info '{
        "ac_comms_state_1": {
            "value": 0,
            "actions": {
                "onSet": [{
                    "enum": [
                        { "shift": 0,"mask": 255,"inValue": 0,"uri": "/alarms/hvac:hvac_comms_1", "outValue": "Normal"},
                        { "shift": 0,"mask": 1,  "inValue": 1,"uri": "/alarms/hvac:hvac_comms_1", "outValue": "HVAC 1 Communication Abnormal"},
                        { "shift": 1,"mask": 1,  "inValue": 1,"uri": "/alarms/hvac:hvac_comms_1", "outValue": "HVAC 2 Communication Abnormal"},
                        { "shift": 2,"mask": 1,  "inValue": 1,"uri": "/alarms/hvac:hvac_comms_1", "outValue": "HVAC 3 Communication Abnormal"},
                        { "shift": 3,"mask": 1,  "inValue": 1,"uri": "/alarms/hvac:hvac_comms_1", "outValue": "HVAC 4 Communication Abnormal"},
                        { "shift": 4,"mask": 1,  "inValue": 1,"uri": "/alarms/hvac:hvac_comms_1", "outValue": "HVAC 5 Communication Abnormal"},
                        { "shift": 5,"mask": 1,  "inValue": 1,"uri": "/alarms/hvac:hvac_comms_1", "outValue": "HVAC 6 Communication Abnormal"},
                        { "shift": 6,"mask": 1,  "inValue": 1,"uri": "/alarms/hvac:hvac_comms_1", "outValue": "HVAC 7 Communication Abnormal"},
                        { "shift": 7,"mask": 1,  "inValue": 1,"uri": "/alarms/hvac:hvac_comms_1", "outValue": "HVAC 8 Communication Abnormal"},
                        { "shift": 8,"mask": 1,  "inValue": 1,"uri": "/alarms/hvac:hvac_comms_1", "outValue": "HVAC 9 Communication Abnormal"},
                        { "shift": 9,"mask": 1,  "inValue": 1,"uri": "/alarms/hvac:hvac_comms_1", "outValue": "HVAC 10 Communication Abnormal"},
                        { "shift": 10,"mask": 1, "inValue": 1,"uri": "/alarms/hvac:hvac_comms_1", "outValue": "HVAC 11 Communication Abnormal"},
                        { "shift": 11,"mask": 1, "inValue": 1,"uri": "/alarms/hvac:hvac_comms_1", "outValue": "HVAC 12 Communication Abnormal"},
                        { "shift": 12,"mask": 1, "inValue": 1,"uri": "/alarms/hvac:hvac_comms_1", "outValue": "HVAC 13 Communication Abnormal"},
                        { "shift": 13,"mask": 1, "inValue": 1,"uri": "/alarms/hvac:hvac_comms_1", "outValue": "HVAC 14 Communication Abnormal"},
                        { "shift": 14,"mask": 1, "inValue": 1,"uri": "/alarms/hvac:hvac_comms_1", "outValue": "HVAC 15 Communication Abnormal"},
                        { "shift": 15,"mask": 1, "inValue": 1,"uri": "/alarms/hvac:hvac_comms_1", "outValue": "HVAC 16 Communication Abnormal"}
                    ]
                }]
            }
        }
}'

