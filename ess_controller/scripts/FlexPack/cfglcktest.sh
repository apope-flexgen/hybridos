/usr/local/bin/fims_send -m set -r /me -u /ess/status/bms ' 
      {
        "Voltage": {
            "value": 0,
            "EnableFaultCheck": true,
            "EnableMaxValCheck": false,
            "EnableMinValCheck": false,

            "MaxAlarmThreshold": 1450,
            "MaxFaultThreshold": 1500,
            "MaxResetValue": 1400,

            "MaxAlarmTimeout": 0.5,
            "MaxFaultTimeout": 0.5,
            "MaxRecoverTimeout": 1.5,

            "MinAlarmThreshold": 1000,
            "MinFaultThreshold": 800,
            "MinResetValue": 1300,

            "MinAlarmTimeout": 0.5,
            "MinFaultTimeout": 0.2,
            "MinRecoverTimeout": 1.5,
            "actions": {
                "onSet": [{"func": [{"func": "CheckMonitorVar","amap": "bms"}]}]
            }
       
        },
        "SOH": {
            "value": 100,
            "EnableFaultCheck": true,
            "EnableMinValCheck": false,

            "MinAlarmThreshold": 30,
            "MinFaultThreshold": 25,
            "MinResetValue": 30,

            "MinAlarmTimeout": 5.5,
            "MinFaultTimeout": 2.5,
            "MinRecoverTimeout": 1.4,
            "actions": {
                "onSet": [{"func": [{"func": "CheckMonitorVar","amap": "bms"}]}]
            }
        },
        "SOC": {
            "value": 0,
			
            "EnableFaultCheck": true,
            "EnableMinValCheck": true,
            "MinAlarmThreshold": 10.0,
            "MinFaultThreshold": 5.0,
            "AlarmTimeout": 5,
            "FaultTimeout": 2,
            "actions": {
                "onSet": [{"func": [{"func": "CheckMonitorVar","amap": "bms"}]}]
            }'