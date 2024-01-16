### Ess Controller v 10.1  Loader : files, templates, site , (client even) 

This document accompanies the v10.1_05 training video

## Introduction

The ess_controller relies on a reasonably complex series of config files.
These files can be in the form of direct variable mappings for relatively unstructured dat spaces.
The files can also provide a means of organizing the controller into a more structured layout.
ESS,BMS,RACK,PCS systems can be defined.

When the ess wants to turn off the bms system the object hierarchy allows the ESS to turn off the BMS and the BMS to turn off each RACK associated with it.

```

              A Sample System Hierarchy
                                                                ESS
                                                                 |
                                   +-----------------------------+-----------------------------------------+
                                   |                                                                       |                                             
                                  BMS                                                                     PCS
                                   |                                                                       |
        +--------------------------+--------------------------+                                     +------+-------+
        |                          |                          |                                     |              |                           
       BMS_1                      BMS_2                      BMS_3                                 PCS_1          PCS_2 
        |                          |                          |
  +-----+----------+        +------+----------+        +------+----------+      
  |     |          |        |      |          |        |      |          |      
RACK_1 RACK_2 ... RACK_n   RACK_1 RACK_2 ... RACK_n   RACK_1 RACK_2 ... RACK_n


```

It turns out that, to completely define such a hierachy you only need to know who you are, and who your parent is.



The site controller interface is a special case. Not wanting to duplicate effort the ess_controller sets 
itself up directly from the modbus_server.json.

This means that the ess_controller can keep in sync with the site controller data map.

## Example System Config Loader

Here is an exmple loader setup.

```
"/config/load": {       
         "ess_controller": {
          "value":false,
          "type":"master",
          "file":"ess_controller",
          "aname":"ess",
          "final":"ess_final",
          "new_options":[
             { "file":"sierra_ess_controls",            "aname":"ess"                   },
             { "load":"sierra_bms_manager",             "aname":"bms",  "pname":"ess"   },
             { "load":"sierra_bms_manager_modbus",      "aname":"bms",  "pname":"ess"   },
             { "file":"sierra_bms_controls",            "aname":"bms",  "pname":"ess"   },

             { "file":"sierra_pcs_manager",             "aname":"pcs",  "pname":"ess" },
             { "file":"siera_pcs_modbus_data",         "aname":"pcs",  "pname":"ess"  },
             { "file":"siera_pcs_controls",            "aname":"pcs",  "pname":"ess"  },

             { "site":"sierra_ess_server_tmpl",      "uri":"/config/ctmpl:sierra_ess_server_tmpl", "aname":"site", "pname":"ess"  }
          ]
        }
    }
```

This loader lives in the "/config/load" data table. You can have as many loaders as you need to define the system.
Other tables "/config/cfile" and "/config/ctmpl" are used to  hold information about any loader files or templates sent to the system.

```
fims_send -m get -r /$$ -u /ess/full/config/load| jq
{
  "ess_controller": {
    "value": true,
    "aname": "ess",
    "configDone": true,
    "file": "ess_controller",
    "final": "ess_final",
    "loadComplete": true,
    "options": [
      {   
          "file": "ess_manager","aname": "ess" },
          "load": "pcs_manager","aname": "ess" }
      }
    ]
  }
}
```

This information is useful in that it shows the md5 sum of the loaded config segment 
And it shows teh time "reqResp" when the file was loaded.
This table has been rearranged to show the loading order.

```
fims_send -m get -r /$$ -u /ess/full/config/cfile| jq
{
  "ess_init":       { "value": true,"aname": "ess",
            "file": "ess_init", "md5sum": "71b8b95d4db31a97ca3a1f8908b80b46", "reqCount": 4, "reqResp":  16.75565505027771, "reqTimeout": 21.64471411705017},
  "ess_controller": { "value": true,
    "md5sum": "f4205679cfdd319ba8bc99346934e6fd",                                            "reqResp":  16.773460149765015                               },
  "ess_manager": {"value": true, "md5sum": "289a13dc5da275ed2b5f2e94b0a58d9f",                "reqResp": 16.795735120773315 },
  "ess_final": { "value": true, 
            "file": "ess_final", "md5sum": "f8d15f58fe48e5cfa40793727547541d", "reqCount": 4, "reqResp": 33.31118607521057, "reqTimeout": 37.24470806121826}
}

```

In these cases no template has been setup.
## What is a loader

A loader will order the sequence of document retrieval from the dbi system.
A loader can also request a "final" file to start all the processes after the loading process.
The config system is split up into one or more files.
If a loader defines sub loaders then all the processes must complete before teh loader value turns true and the "final" file is recovered.
When running with the dbi system, the loader recovers the files in order. requssting the "next" file after the previous file has been processed.


(one or more templates)

Each loader can request one or more layered templates.

This structure can process a connected chain of systems.

Typical example Cells can belong to Modules

Modules belong to Racks
Racks belong to a Bms
Bms's belongs to the Ess.

The important thing about creating templates is that you can create multiple identical components with unique names.


Here is a typical but complex single level template structure with racks and hvac units.
Note that rack 1 has 1 hval unit abd the other racks have two. 
The "mult" and "shift" operations  allow a match up for the correct numberin.

```
file risen_bms_manager.json

{
    "/config/load": {
        "risen_bms_manager": {
            "value":false,
            "file":"risen_bms_manager",
            "pname":"ess",
            "aname":"bms",
            "new_options":[
              {
                 "tmpl":"risen_bms_template",  "pname":"bms", "amname":"##RACK_ID##",
                  "from":1, "to":18,
                  "reps":[
                          {"replace":"##RACK_ID##",   "with":"rack_{:02d}"},
                          {"replace":"##RACK_NUM##",  "with":"{:02d}"},
                          {"replace": "##AC_1_ID##",  "with": "hvac_{:02d}", "mult":2         },
                          {"replace": "##AC_1_NUM##", "with": "{:02d}",      "mult":2         },
                          {"replace": "##AC_2_ID##",  "with": "hvac_{:02d}", "mult":2, "add":1},
                          {"replace": "##AC_2_NUM##", "with": "{:02d}",      "mult":2, "add":1}
                      ]
                  }
              ]
        }
      }
}
```

This is a rather extensive template for a risen rack system.


```
file:risen_bms_template.json
{
    "/notes/@@RACK_ID@@": {
        "note1":"This is the battery management system (BMS) config template file for Risen",
        "note1a": "/assets/bms/@@RACK_ID@@            - defines the web UI for the battery rack. Battery rack-related statuses, controls, and faults/alarms are displayed on the UI",
        "note1b": "/config/@@RACK_ID@@                - defines configurable variables used in the ESS Controller source code",
        "note1d": "/status/@@RACK_ID@@                - defines the status variables for the battery rack",
        "note1e": "/components/[id]                   - defines the interface for retrieving data for the battery rack from the BMS hardware and converting the data into a useable format (ex.: scaling, remap, etc.)",
        "note1f": "/links/@@RACK_ID@@                 - defines mapping of external interface components to internal variables",
        "note1g": "/vlinks/@@RACK_ID@@                - allows two variables to share the same value",
        "note1i": "/schedule/wake_monitor/@@RACK_ID@@ - periodically runs a function for a particular variable (default to 100ms; wake_monitor to be removed)",
        "note1j": "/faults/@@RACK_ID@@                - defines battery rack-related faults reported by the ESS Controller",
        "note1k": "/alarms/@@RACK_ID@@                - defines battery rack-related alarms reported by the ESS Controller",
        "note2":"To retrieve data from the BMS hardware, the modbus communication protocol will be used. To connect via modbus, use the modbus client (bms_modbus_client.json)"
    },
    "/system/@@RACK_ID@@": {
        "id": "@@RACK_ID@@",
        "name": "Risen BMS Rack @@RACK_NUM@@"
    },
    "/config/@@RACK_ID@@": {
        "Pubs": "/assets/bms/@@RACK_ID@@",
        "BlockSets": "/status/ess, /status/bms, /status/@@RACK_ID@@",
        "AlarmDestination": "/assets/bms/@@RACK_ID@@:alarms",
        "FaultDestination": "/assets/bms/@@RACK_ID@@:faults",
        "NoFaultMsg": "Normal",
        "NoAlarmMsg": "Normal"
    },
    "/assets/bms/@@RACK_ID@@": {
        "name": "Risen Battery Rack @@RACK_NUM@@",
        "alarms": {
            "name": "Alarms",
            "value": 0,
            "options": [],
            "unit": "",
            "scaler": 0,
            "enabled": true,
            "ui_type": "alarm",
            "type": "number"
        },
        "faults": {
            "name": "Faults",
            "value": 0,
            "options": [],
            "unit": "",
            "scaler": 0,
            "enabled": true,
            "ui_type": "alarm",
            "type": "number"
        },
        "system_state": {
            "name": "Battery Status",
            "value": "N/A",
            "unit": "",
            "scaler": 0,
            "enabled": true,
            "ui_type": "status",
            "type": "string"
        },
        "charge_discharge_state": {
            "name": "Charge/Discharge Status",
            "value": "N/A",
            "unit": "",
            "scaler": 0,
            "enabled": true,
            "ui_type": "status",
            "type": "string"
        },
        "dc_contactor_status": {
            "name": "Contactor Status",
            "value": "Open",
            "unit": "",
            "scaler": 0,
            "enabled": true,
            "ui_type": "status",
            "type": "string"
        },
        "enable_status": {
            "name": "Enable Status",
            "value": "N/A",
            "unit": "",
            "scaler": 0,
            "enabled": true,
            "ui_type": "status",
            "type": "string"
        },
        "soc": {
            "name": "State of Charge",
            "value": 0,
            "unit": "%",
            "scaler": 1,
            "enabled": false,
            "ui_type": "status",
            "type": "number"
        },
        "soh": {
            "name": "State of Health",
            "value": 0,
            "unit": "%",
            "scaler": 1,
            "enabled": false,
            "ui_type": "status",
            "type": "number"
        },
        "system_voltage": {
            "name": "System Voltage",
            "value": 0,
            "unit": "V",
            "scaler": 1,
            "enabled": true,
            "ui_type": "status",
            "type": "number"
        },
        "system_current": {
            "name": "System Current",
            "value": 0,
            "unit": "A",
            "scaler": 1,
            "enabled": true,
            "ui_type": "status",
            "type": "number"
        },
        "system_power": {
            "name": "System Power",
            "value": 0,
            "unit": "kW",
            "scaler": 1,
            "enabled": true,
            "ui_type": "status",
            "type": "number"
        },
        "max_cell_voltage": {
            "name": "Max Cell Voltage",
            "value": 0,
            "unit": "V",
            "scaler": 1,
            "enabled": false,
            "ui_type": "status",
            "type": "number"
        },
        "min_cell_voltage": {
            "name": "Min Cell Voltage",
            "value": 0,
            "unit": "V",
            "scaler": 1,
            "enabled": false,
            "ui_type": "status",
            "type": "number"
        },
        "avg_cell_voltage": {
            "name": "Avg Cell Voltage",
            "value": 0,
            "unit": "V",
            "scaler": 1,
            "enabled": false,
            "ui_type": "status",
            "type": "number"
        },
        "max_cell_temp": {
            "name": "Max Cell Temperature",
            "value": 0,
            "unit": "°C",
            "scaler": 1,
            "enabled": false,
            "ui_type": "status",
            "type": "number"
        },
        "min_cell_temp": {
            "name": "Min Cell Temperature",
            "value": 0,
            "unit": "°C",
            "scaler": 1,
            "enabled": false,
            "ui_type": "status",
            "type": "number"
        },
        "avg_cell_temp": {
            "name": "Avg Cell Temperature",
            "value": 0,
            "unit": "°C",
            "scaler": 1,
            "enabled": false,
            "ui_type": "status",
            "type": "number"
        },
        "max_charge_current": {
            "name": "Max Charge Current Allowed",
            "value": 0,
            "unit": "A",
            "scaler": 1,
            "enabled": true,
            "ui_type": "status",
            "type": "number"
        },
        "max_discharge_current": {
            "name": "Max Discharge Current Allowed",
            "value": 0,
            "unit": "A",
            "scaler": 1,
            "enabled": true,
            "ui_type": "status",
            "type": "number"
        },
        "hvac_1_unit_running_status": {
            "name": "HVAC 1 Unit Running Status",
            "value": "N/A",
            "unit": "",
            "scaler": 0,
            "enabled": true,
            "ui_type": "status",
            "type": "string"
        },
        "hvac_1_temperature": {
            "name": "HVAC 1 Temperature",
            "value": 0,
            "unit": "°C",
            "scaler": 1,
            "enabled": false,
            "ui_type": "status",
            "type": "number"
        },
        "hvac_2_unit_running_status": {
            "name": "HVAC 2 Unit Running Status",
            "value": "N/A",
            "unit": "",
            "scaler": 0,
            "enabled": true,
            "ui_type": "status",
            "type": "string"
        },
        "hvac_2_temperature": {
            "name": "HVAC 2 Temperature",
            "value": 0,
            "unit": "°C",
            "scaler": 1,
            "enabled": false,
            "ui_type": "status",
            "type": "number"
        },
        "maint_mode": {
            "name": "Maintenance Mode",
            "value": false,
            "unit": "",
            "scaler": 0,
            "enabled": true,
            "ui_type": "control",
            "type": "enum_slider",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"uri":	"/assets/bms/@@RACK_ID@@:clear_faults@enabled"}
                    ]
                }]
            },
            "options": [
                {"name": "No", "return_value": false},
                {"name": "Yes","return_value": true}
            ]
        },
        "enable_rack": {
            "name": "Enable Rack",
            "value": false,
            "unit": "",
            "scaler": 0,
            "enabled": false,
            "ui_type": "control",
            "type": "enum_button",
            "actions": {
                "onSet": [{
                    "func": [
                        {"ifChanged": false, "func": "LogInfo", "amap": "ess"}
                    ],
                    "remap": [
                        {"inValue": true, "ifChanged": false, "uri": "/controls/@@RACK_ID@@:EnableRack@triggerCmd", "outValue": true},
                        {"inValue": true, "ifChanged": false, "uri": "/controls/@@RACK_ID@@:EnableRack",            "outValue": 1}
                    ]
                }]
            },
            "options": [
                {"name": "No", "return_value": false},
                {"name": "Yes","return_value": true}
            ]
        },
        "disable_rack": {
            "name": "Disable Rack",
            "value": false,
            "unit": "",
            "scaler": 0,
            "enabled": false,
            "ui_type": "control",
            "type": "enum_button",
            "actions": {
                "onSet": [{
                    "func": [
                        {"ifChanged": false, "func": "LogInfo", "amap": "ess"}
                    ],
                    "remap": [
                        {"inValue": true, "ifChanged": false, "uri": "/controls/@@RACK_ID@@:DisableRack@triggerCmd", "outValue": true},
                        {"inValue": true, "ifChanged": false, "uri": "/controls/@@RACK_ID@@:DisableRack",            "outValue": 2}
                    ]
                }]
            },
            "options": [
                {"name": "No", "return_value": false},
                {"name": "Yes","return_value": true}
            ]
        },
        "clear_faults": {
            "name": "Clear Faults",
            "value": false,
            "unit": "",
            "scaler": 0,
            "enabled": false,
            "ui_type": "control",
            "type": "enum_button",
            "actions": { 
                "onSet":[{
                    "remap":[
                        {"ifChanged": false, "outValue":"Clear","uri":"/faults/@@RACK_ID@@:clear_faults"},
                        {"ifChanged": false, "outValue":"Clear","uri":"/alarms/@@RACK_ID@@:clear_alarms"}
                    ]
                }]
            },
            "options": [
                {"name": "Clear Faults", "return_value": "Clear"}
            ]
        }
    },
    "/vlinks/@@RACK_ID@@": {
        "@@RACK_ID@@_system_state_ui":          { "value": "/assets/bms/@@RACK_ID@@:system_state",                      "vlink": "/status/@@RACK_ID@@:SystemState"},
        "@@RACK_ID@@_charge_discharge_state_ui":{ "value": "/assets/bms/@@RACK_ID@@:charge_discharge_state",            "vlink": "/status/@@RACK_ID@@:ChargeDischargeState"},
        "@@RACK_ID@@_soc_ui":                   { "value": "/assets/bms/@@RACK_ID@@:soc",                               "vlink": "/status/@@RACK_ID@@:RackSOC"    },
        "@@RACK_ID@@_soh_ui":                   { "value": "/assets/bms/@@RACK_ID@@:soh",                               "vlink": "/status/@@RACK_ID@@:RackSOH"    },
        "@@RACK_ID@@_voltage_ui":               { "value": "/assets/bms/@@RACK_ID@@:system_voltage",                    "vlink": "/status/@@RACK_ID@@:RackVoltage"},
        "@@RACK_ID@@_current_ui":               { "value": "/assets/bms/@@RACK_ID@@:system_current",                    "vlink": "/status/@@RACK_ID@@:RackCurrent"},
        "@@RACK_ID@@_power_ui":                 { "value": "/assets/bms/@@RACK_ID@@:system_power",                      "vlink": "/status/@@RACK_ID@@:RackPower"},
        "@@RACK_ID@@_max_cell_voltage_ui":      { "value": "/assets/bms/@@RACK_ID@@:max_cell_voltage",                  "vlink": "/status/@@RACK_ID@@:RackMaxCellVoltage"},
        "@@RACK_ID@@_min_cell_voltage_ui":      { "value": "/assets/bms/@@RACK_ID@@:min_cell_voltage",                  "vlink": "/status/@@RACK_ID@@:RackMinCellVoltage"},
        "@@RACK_ID@@_avg_cell_voltage_ui":      { "value": "/assets/bms/@@RACK_ID@@:avg_cell_voltage",                  "vlink": "/status/@@RACK_ID@@:RackAvgCellVoltage"},
        "@@RACK_ID@@_max_cell_temp_ui":         { "value": "/assets/bms/@@RACK_ID@@:max_cell_temp",                     "vlink": "/status/@@RACK_ID@@:RackMaxCellTemp"},
        "@@RACK_ID@@_min_cell_temp_ui":         { "value": "/assets/bms/@@RACK_ID@@:min_cell_temp",                     "vlink": "/status/@@RACK_ID@@:RackMinCellTemp"},
        "@@RACK_ID@@_avg_cell_temp_ui":         { "value": "/assets/bms/@@RACK_ID@@:avg_cell_temp",                     "vlink": "/status/@@RACK_ID@@:RackAvgCellTemp"},
        "@@RACK_ID@@_max_charge_current_ui":    { "value": "/assets/bms/@@RACK_ID@@:max_charge_current",                "vlink": "/status/@@RACK_ID@@:RackMaxChargeCurrent"},
        "@@RACK_ID@@_max_discharge_current_ui": { "value": "/assets/bms/@@RACK_ID@@:max_discharge_current",             "vlink": "/status/@@RACK_ID@@:RackMaxDischargeCurrent"}
    },

    "/schedule/wake_monitor/@@RACK_ID@@":{
        "/status/@@RACK_ID@@:RackPower":          { "enable": true, "rate":0.1, "amap": "@@RACK_ID@@", "func":"CalculateVar"},
        "/status/@@RACK_ID@@:OverCurrentFault":   { "enable": true, "rate":0.1, "amap": "@@RACK_ID@@", "func":"CalculateVar"},
        "/status/@@RACK_ID@@:UnderCurrentFault":  { "enable": true, "rate":0.1, "amap": "@@RACK_ID@@", "func":"CalculateVar"},
        "/status/@@RACK_ID@@:OverCurrentAlarm":   { "enable": true, "rate":0.1, "amap": "@@RACK_ID@@", "func":"CalculateVar"},
        "/status/@@RACK_ID@@:UnderCurrentAlarm":  { "enable": true, "rate":0.1, "amap": "@@RACK_ID@@", "func":"CalculateVar"},
        "/status/@@RACK_ID@@:OverCurrentDerate":  { "enable": true, "rate":0.1, "amap": "@@RACK_ID@@", "func":"CalculateVar"},
        "/status/@@RACK_ID@@:UnderCurrentDerate": { "enable": true, "rate":0.1, "amap": "@@RACK_ID@@", "func":"CalculateVar"},
        "/status/@@RACK_ID@@:DCClosed":           { "enable": true, "rate":0.1, "amap": "@@RACK_ID@@", "func":"CalculateVar"},
        "/status/@@RACK_ID@@:RackCurrentFilt":    { "enable": true, "rate":0.1, "amap": "@@RACK_ID@@", "func":"CalculateVar"},
        "/status/@@RACK_ID@@:EnableRackCtrl":     { "enable": true, "rate":0.1, "amap": "@@RACK_ID@@", "func":"CalculateVar"},
        "/status/@@RACK_ID@@:DisableRackCtrl":    { "enable": true, "rate":0.1, "amap": "@@RACK_ID@@", "func":"CalculateVar"},

        "/controls/@@RACK_ID@@:EnableRack":           { "enable": true, "rate":0.1, "amap": "@@RACK_ID@@", "func":"HandleCmd"},
        "/controls/@@RACK_ID@@:DisableRack":          { "enable": true, "rate":0.1, "amap": "@@RACK_ID@@", "func":"HandleCmd"},
        "/controls/@@RACK_ID@@:VerifyRackEnabled":    { "enable": true, "rate":0.1, "amap": "@@RACK_ID@@", "func":"HandleCmd"},
        "/controls/@@RACK_ID@@:VerifyRackDisabled":   { "enable": true, "rate":0.1, "amap": "@@RACK_ID@@", "func":"HandleCmd"},

        "/status/@@RACK_ID@@:RackCurrent":            { "enable": true, "rate":0.1, "amap": "@@RACK_ID@@", "func":"CheckMonitorVar"},
        "/status/@@RACK_ID@@:RackCurrentCheckDerate": { "enable": true, "rate":0.1, "amap": "@@RACK_ID@@", "func":"CheckMonitorVar"}
    },
    "/controls/@@RACK_ID@@": {
        "EnableRack": {
            "value": 1,
            "note": "Enable command to send to battery rack if the rack is disabled and the dc contactors are opened",
            "cmdVar": "/components/bms_@@RACK_ID@@_controls:rack_enable",
            "checkCmdTimeout": 30,
            "sendCmdTimeout": 30,
            "numVars": 3,
            "variable1": "/status/@@RACK_ID@@:RackEnableStatus",
            "variable2": "/status/@@RACK_ID@@:DCClosed",
            "variable3": "/status/@@RACK_ID@@:FaultCnt",
            "useExpr": true,
            "expression": "not {1} and not {2} and {3} <= 0",
            "actions": {
                "onSet": [{"func": [{"func": "HandleCmd", "amap": "@@RACK_ID@@"}]}]
            }
        },
        "DisableRack": {
            "value": 2,
            "note": "Disable command to send to battery rack if the rack is enabled and the dc contactors are opened",
            "cmdVar": "/components/bms_@@RACK_ID@@_controls:rack_enable",
            "checkCmdTimeout": 30,
            "sendCmdTimeout": 30,
            "numVars": 3,
            "variable1": "/status/@@RACK_ID@@:RackEnableStatus",
            "variable2": "/status/@@RACK_ID@@:DCClosed",
            "variable3": "/status/@@RACK_ID@@:FaultCnt",
            "useExpr": true,
            "expression": "{1} and not {2} and {3} <= 0",
            "actions": {
                "onSet": [{"func": [{"func": "HandleCmd", "amap": "@@RACK_ID@@"}]}]
            }
        },

        "VerifyRackEnabled": {
            "value": 0,
            "note": "Verify that the battery rack is now enabled when the enable rack command is sent",
            "enableAlert": false,
            "sendCmdHoldTimeout": 3,
            "sendCmdTimeout": 30,
            "numVars": 1,
            "variable1": "/status/@@RACK_ID@@:RackEnableStatus",
            "useExpr": true,
            "expression": "{1}",
            "actions": {
                "onSet": [{"func": [{"func": "HandleCmd", "amap": "@@RACK_ID@@"}]}]
            }
        },
        "VerifyRackDisabled": {
            "value": 0,
            "note": "Verify that the battery rack is now disabled when the disable rack command is sent",
            "enableAlert": false,
            "sendCmdHoldTimeout": 3,
            "sendCmdTimeout": 30,
            "numVars": 1,
            "variable1": "/status/@@RACK_ID@@:RackEnableStatus",
            "useExpr": true,
            "expression": "not {1}",
            "actions": {
                "onSet": [{"func": [{"func": "HandleCmd", "amap": "@@RACK_ID@@"}]}]
            }
        }
    },
    "/status/@@RACK_ID@@": {
        "note": "These are component status vars",
        "SystemState": "Normal",
        "FaultCnt": 0,
        "AlarmCnt": 0,
        "RackSOC": 0,
        "RackSOH": 0,
        "RackVoltage": 0,
        "RackMaxCellVoltage": 0,
        "RackMinCellVoltage": 0,
        "RackAvgCellVoltage": 0,
        "RackMaxCellTemp": 0,
        "RackMinCellTemp": 0,
        "RackAvgCellTemp": 0,
        "RackMaxChargeCurrent": 0,
        "RackMaxDischargeCurrent": 0,
        "rack_precharge_phase": "Normal",
        "pos_contactor_state": 0,
        "pre_contactor_state": "Normal",
        "neg_contactor_state": 0,
        "disconnector_state": "Normal",
        "ChargeDischargeState": "Normal",
        "RackEnableStatus": {
            "value": false,
            "actions": {
                "onSet":[{
                    "remap":[
                        {"inValue": true, "uri": "/assets/bms/@@RACK_ID@@:enable_status", "outValue": "Enabled"},
                        {"inValue": false, "uri": "/assets/bms/@@RACK_ID@@:enable_status", "outValue": "Disabled"}
                    ]
                }]
            }
        },
        "@@AC_1_ID@@_unit_running_status": "Normal",
        "@@AC_1_ID@@_internal_fan_status": "Normal",
        "@@AC_1_ID@@_external_fan_status": "Normal",
        "@@AC_1_ID@@_compressor_status": "Normal",
        "@@AC_1_ID@@_emergency_air_fan_state": "Normal",
        "@@AC_1_ID@@_electrical_heating_running_status": "Normal",

        "@@AC_2_ID@@_unit_running_status": "Normal",
        "@@AC_2_ID@@_internal_fan_status": "Normal",
        "@@AC_2_ID@@_external_fan_status": "Normal",
        "@@AC_2_ID@@_compressor_status": "Normal",
        "@@AC_2_ID@@_emergency_air_fan_state": "Normal",
        "@@AC_2_ID@@_electrical_heating_running_status": "Normal",

        "RackPower": {
            "value": 0,
            "numVars": 2,
            "variable1": "/status/@@RACK_ID@@:RackVoltage",
            "variable2": "/status/@@RACK_ID@@:RackCurrent",
            "scale": 1000,
            "operation": "*"
        },
        "RackCurrent": {
            "value": 0,
            "EnableMaxValCheck": false,
            "EnableMinValCheck": false,
            "AlarmTimeout": 0.4,
            "MinAlarmTimeout": 1.75,
            "MaxAlarmTimeout": 1.75,
            "MinRecoverTimeout": 0,
            "MaxRecoverTimeout": 0,
            "actions": {
                "onSet": [{"func": [{"func": "CheckMonitorVar", "amap": "@@RACK_ID@@"}]}]
            }
        },
        "RackCurrentFilt": {
            "value": 0,
            "useExpr": true,
            "numVars": 4,
            "variable1": "/status/@@RACK_ID@@:DCClosed",
            "variable2": "/status/@@RACK_ID@@:RackCurrent",
            "variable3": "/status/bms:BMSCurrent",
            "variable4": "/status/bms:NumRacksInService",
            "expression": "if({1}, {2}, ({3} / {4}))",
            "actions": {
                "onSet":[{"remap":[{"uri": "/status/@@RACK_ID@@:RackCurrentCoerced"}]}]
            }
        },
        "RackCurrentCoerced": 0,
        "RackCurrentCheckDerate": {
            "value": 0,
            "EnableMaxValCheck": true,
            "EnableMinValCheck": true,
            "EnableFaultCheck": false,
            "EnableAlert": false,
            "AlarmTimeout": 0.5,
            "MinRecoverTimeout": 1.0,
            "MaxRecoverTimeout": 1.0,
            "actions": {
                "onSet": [{"func": [{"func": "CheckMonitorVar", "amap": "@@RACK_ID@@"}]}]
            }
        },
        "UnderCurrentFault": {
            "value": 0,
            "useExpr": true,
            "numVars": 1, 
            "variable1": "/status/@@RACK_ID@@:RackMaxChargeCurrent",
            "expression": "{1} * 1.05",
            "actions": {
                "onSet":[{"remap":[{"uri": "/status/@@RACK_ID@@:RackCurrent@MinFaultThreshold"}]}]
            }
        },
        "OverCurrentFault": {
            "value": 0,
            "useExpr": true,
            "numVars": 1, 
            "variable1": "/status/@@RACK_ID@@:RackMaxDischargeCurrent",
            "expression": "{1} * 1.05",
            "actions": {
                "onSet":[{"remap":[{"uri": "/status/@@RACK_ID@@:RackCurrent@MaxFaultThreshold"}]}]
            }
        },
        "UnderCurrentAlarm": {
            "value": 0,
            "numVars": 2, 
            "variable1": "/status/@@RACK_ID@@:RackMaxChargeCurrent",
            "variable2": "/status/pcs:DerateThresholdHigh",
            "operation": "*",
            "actions": {
                "onSet":[{
                    "remap":[
                        {"uri": "/status/@@RACK_ID@@:RackCurrent@MinAlarmThreshold"},
                        {"uri": "/status/@@RACK_ID@@:RackCurrent@MinResetValue"}
                    ]
                }]
            }
        },
        "OverCurrentAlarm": {
            "value": 0,
            "numVars": 2, 
            "variable1": "/status/@@RACK_ID@@:RackMaxDischargeCurrent",
            "variable2": "/status/pcs:DerateThresholdHigh",
            "operation": "*",
            "actions": {
                "onSet":[{
                    "remap":[
                        {"uri": "/status/@@RACK_ID@@:RackCurrent@MaxAlarmThreshold"},
                        {"uri": "/status/@@RACK_ID@@:RackCurrent@MaxResetValue"}
                    ]
                }]
            }
        },
        "UnderCurrentDerate": {
            "value": 0,
            "numVars": 2, 
            "variable1": "/status/@@RACK_ID@@:RackMaxChargeCurrent",
            "variable2": "/status/pcs:DerateThresholdLow",
            "operation": "*",
            "actions": {
                "onSet":[{
                    "remap":[
                        {"uri": "/status/@@RACK_ID@@:RackCurrentCheckDerate@MinAlarmThreshold"},
                        {"uri": "/status/@@RACK_ID@@:RackCurrentCheckDerate@MinResetValue"}
                    ]
                }]
            }
        },
        "OverCurrentDerate": {
            "value": 0,
            "numVars": 2, 
            "variable1": "/status/@@RACK_ID@@:RackMaxDischargeCurrent",
            "variable2": "/status/pcs:DerateThresholdLow",
            "operation": "*",
            "actions": {
                "onSet":[{
                    "remap":[
                        {"uri": "/status/@@RACK_ID@@:RackCurrentCheckDerate@MaxAlarmThreshold"},
                        {"uri": "/status/@@RACK_ID@@:RackCurrentCheckDerate@MaxResetValue"}
                    ]
                }]
            }
        },
        "DCClosed": {
            "value": 0,
            "resetChange": false,
            "useExpr": true,
            "numVars": 2,
            "variable1": "/status/@@RACK_ID@@:pos_contactor_state",
            "variable2": "/status/@@RACK_ID@@:neg_contactor_state",
            "expression": "{1} and {2}",
            "note": "To modify: enable/disable monitoring check for RackCurrent if bms dc contactors are closed and pcs is running",
            "actions": { 
                "onSet":[{
                    "remap":[
                        {"inValue": 0,  "uri":"/status/@@RACK_ID@@:RackCurrent@EnableMaxValCheck",            "outValue": false},
                        {"inValue": 1,  "uri":"/status/@@RACK_ID@@:RackCurrent@EnableMaxValCheck",            "outValue": true},
                        {"inValue": 0,  "uri":"/status/@@RACK_ID@@:RackCurrent@EnableMinValCheck",            "outValue": false},
                        {"inValue": 1,  "uri":"/status/@@RACK_ID@@:RackCurrent@EnableMinValCheck",            "outValue": true},
                        {"inValue": 0,  "uri":"/status/@@RACK_ID@@:RackCurrentCheckDerate@EnableMaxValCheck", "outValue": false},
                        {"inValue": 1,  "uri":"/status/@@RACK_ID@@:RackCurrentCheckDerate@EnableMaxValCheck", "outValue": true},
                        {"inValue": 0,  "uri":"/status/@@RACK_ID@@:RackCurrentCheckDerate@EnableMinValCheck", "outValue": false},
                        {"inValue": 1,  "uri":"/status/@@RACK_ID@@:RackCurrentCheckDerate@EnableMinValCheck", "outValue": true},

                        {"inValue": 0,  "uri":"/assets/bms/@@RACK_ID@@:dc_contactor_status",                  "outValue": "Open"},
                        {"inValue": 1,  "uri":"/assets/bms/@@RACK_ID@@:dc_contactor_status",                  "outValue": "Closed"}
                    ]
                }]
            }
        },
        "ClearFaultsDone": {
            "value": false,
            "note": "Set to true if all alarms/faults have been cleared using process_sys_alarm function. If true, reset ESS alarm/fault registers to 0",
            "actions": { 
                "onSet":[{
                    "remap":[
                        {"inValue": true, "ifChanged": false, "outValue": 0, "uri": "/site/ess_ls:@@RACK_ID@@_warnings"},
                        {"inValue": true, "ifChanged": false, "outValue": 0, "uri": "/site/ess_ls:@@RACK_ID@@_alarms"},
                        {"inValue": true, "ifChanged": false, "outValue": 0, "uri": "/site/ess_ls:@@RACK_ID@@_critical_alarms"},
                        {"inValue": true, "ifChanged": false, "outValue": 0, "uri": "/site/ess_ls:@@RACK_ID@@_alarm_info"},
                        {"inValue": true, "ifChanged": false, "outValue": 0, "uri": "/site/ess_ls:@@RACK_ID@@_hvac_1_alarms1"},
                        {"inValue": true, "ifChanged": false, "outValue": 0, "uri": "/site/ess_ls:@@RACK_ID@@_hvac_1_alarms2"},
                        {"inValue": true, "ifChanged": false, "outValue": 0, "uri": "/site/ess_ls:@@RACK_ID@@_hvac_2_alarms1"},
                        {"inValue": true, "ifChanged": false, "outValue": 0, "uri": "/site/ess_ls:@@RACK_ID@@_hvac_2_alarms2"},
                        {"inValue": true, "ifChanged": false, "outValue": 0, "uri": "/site/ess_ls:@@RACK_ID@@_fss_alarms"},
                        {"inValue": true, "ifChanged": false, "outValue": 0, "uri": "/site/ess_ls:@@RACK_ID@@_step_in_failure_alarms"}
                    ]
                }]
            }
        },
        "EnableRackCtrl": {
            "value": false,
            "useExpr": true,
            "numVars": 3,
            "variable1": "/status/@@RACK_ID@@:RackEnableStatus",
            "variable2": "/assets/bms/@@RACK_ID@@:maint_mode",
            "variable3": "/status/bms:DCClosed",
            "expression": "not {1} and {2} and {3} == 0",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue":false, "uri":"/assets/bms/@@RACK_ID@@:enable_rack@enabled",   "outValue":false},
                        {"inValue":true,  "uri":"/assets/bms/@@RACK_ID@@:enable_rack@enabled",   "outValue":true}
                    ]
                }]
            }
        },
        "DisableRackCtrl": {
            "value": false,
            "useExpr": true,
            "numVars": 3,
            "variable1": "/status/@@RACK_ID@@:RackEnableStatus",
            "variable2": "/assets/bms/@@RACK_ID@@:maint_mode",
            "variable3": "/status/bms:DCClosed",
            "expression": "{1} and {2} and {3} == 0",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue":false, "uri":"/assets/bms/@@RACK_ID@@:disable_rack@enabled",   "outValue":false},
                        {"inValue":true,  "uri":"/assets/bms/@@RACK_ID@@:disable_rack@enabled",   "outValue":true}
                    ]
                }]
            }
        },
        "EnableRackSuccess": {
            "value": false,
            "note": "Validation variable is set if the command value in /controls/bms:EnableRack is successfully/unsuccessfully set",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": true,  "ifChanged": false, "uri": "/controls/@@RACK_ID@@:VerifyRackEnabled@triggerCmd", "outValue": true},
                        {"inValue": true,  "ifChanged": false, "uri": "/controls/@@RACK_ID@@:VerifyRackEnabled",            "outValue": 0},

                        {"inValue": false, "ifChanged": false, "uri": "/alarms/@@RACK_ID@@:enable_cmd",                     "outValue": "Failed to send/verify enable command (1) to battery rack"}
                    ]
                }]
            }
        },
        "DisableRackSuccess": {
            "value": false,
            "note": "Validation variable is set if the command value in /controls/bms:DisableRack is successfully/unsuccessfully set",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": true,  "ifChanged": false, "uri": "/controls/@@RACK_ID@@:VerifyRackDisabled@triggerCmd", "outValue": true},
                        {"inValue": true,  "ifChanged": false, "uri": "/controls/@@RACK_ID@@:VerifyRackDisabled",            "outValue": 0},

                        {"inValue": false, "ifChanged": false, "uri": "/alarms/@@RACK_ID@@:disable_cmd",                     "outValue": "Failed to send/verify disable command (2) to battery rack"}
                    ]
                }]
            }
        },
        "VerifyRackEnabledSuccess": {
            "value": false,
            "note": "If the battery rack failed to transition to enabled state after an elasped time, send an alarm",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/@@RACK_ID@@:enable_failure", "outValue": "Failed to enable battery rack"}
                    ]
                }]
            }
        },
        "VerifyRackDisabledSuccess": {
            "value": false,
            "note": "If the battery rack failed to transition to disabled state after an elasped time, send an alarm",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/@@RACK_ID@@:disable_failure", "outValue": "Failed to disable battery rack"}
                    ]
                }]
            }
        }
    },
    "/components/bms_@@RACK_ID@@_controls": {
        "rack_enable": 0,
        "open_contactor": 0,
        "close_contactor": 0
    },
    "/components/bms_@@RACK_ID@@_info": {
        "rack_run_state": {
            "value": 0,
            "actions": {
                "onSet": [{
                    "enum": [
                        { "shift": 0,"mask": 255,"inValue": 0,"uri": "/status/@@RACK_ID@@:SystemState", "outValue": "Normal"},
                        { "shift": 0,"mask": 255,"inValue": 1,"uri": "/status/@@RACK_ID@@:SystemState", "outValue": "No charge"},
                        { "shift": 0,"mask": 255,"inValue": 2,"uri": "/status/@@RACK_ID@@:SystemState", "outValue": "No discharge"},
                        { "shift": 0,"mask": 255,"inValue": 3,"uri": "/status/@@RACK_ID@@:SystemState", "outValue": "Standby"},
                        { "shift": 0,"mask": 255,"inValue": 4,"uri": "/status/@@RACK_ID@@:SystemState", "outValue": "Stop"}
                    ]
                }]
            }
        },
        "rack_precharge_phase": {
            "value": 0,
            "actions": {
                "onSet": [{
                    "enum": [
                        { "shift": 0,"mask": 255,"inValue": 0,"uri": "/status/@@RACK_ID@@:rack_precharge_phase", "outValue": "Disconnected"},
                        { "shift": 0,"mask": 255,"inValue": 1,"uri": "/status/@@RACK_ID@@:rack_precharge_phase", "outValue": "Start connection"},
                        { "shift": 0,"mask": 255,"inValue": 2,"uri": "/status/@@RACK_ID@@:rack_precharge_phase", "outValue": "Connecting"},
                        { "shift": 0,"mask": 255,"inValue": 3,"uri": "/status/@@RACK_ID@@:rack_precharge_phase", "outValue": "Connected"},
                        { "shift": 0,"mask": 255,"inValue": 4,"uri": "/status/@@RACK_ID@@:rack_precharge_phase", "outValue": "Connection fail"}
                    ]
                }]
            }
        },
        "rack_connection_state": {
            "value": 0,
            "note": "0: Opened, 1: Closed",
            "note1": "Bit0: Positive Contactor State",
            "note2": "Bit1: PreContactor State",
            "note3": "Bit2: Negative Contactor State",
            "note4": "Bit3: Disconnector State",
            "actions": {
                "onSet": [{
                    "enum": [
                        { "shift": 0,"mask": 1,"inValue": 0,"uri": "/status/@@RACK_ID@@:pos_contactor_state", "outValue": 0},
                        { "shift": 0,"mask": 1,"inValue": 1,"uri": "/status/@@RACK_ID@@:pos_contactor_state", "outValue": 1},
                        { "shift": 1,"mask": 1,"inValue": 0,"uri": "/status/@@RACK_ID@@:pre_contactor_state", "outValue": "Opened"},
                        { "shift": 1,"mask": 1,"inValue": 1,"uri": "/status/@@RACK_ID@@:pre_contactor_state", "outValue": "Closed"},
                        { "shift": 2,"mask": 1,"inValue": 0,"uri": "/status/@@RACK_ID@@:neg_contactor_state", "outValue": 0},
                        { "shift": 2,"mask": 1,"inValue": 1,"uri": "/status/@@RACK_ID@@:neg_contactor_state", "outValue": 1},
                        { "shift": 3,"mask": 1,"inValue": 0,"uri": "/status/@@RACK_ID@@:disconnector_state", "outValue": "Opened"},
                        { "shift": 3,"mask": 1,"inValue": 1,"uri": "/status/@@RACK_ID@@:disconnector_state", "outValue": "Closed"}
                    ]
                }]
            }
        },
        "rack_alarm_info": {
            "value": 0,
            "note": "0: Normal, 1: Fault",
            "note1": "Bit0: BMU Hardware Fault",
            "note2": "Bit1: BCU Hardware Fault",
            "note3": "Bit2: Fuse Protector Fault",
            "note4": "Bit3: Contactor Adhesion Fault",
            "note5": "Bit4: BMU Communication Fault",
            "note6": "Bit5: BAU Communication Fault",
            "note7": "Bit6: Current Sensor Fault",
            "note8": "Bit7: IMD Fault",
            "note9": "Bit8: Disconnector Open when Rack Enabled",
            "note10": "Bit9-15: Reserved",
            "actions": {
                "onSet": [{
                    "enum": [
                        { "shift": 0,"mask": 1,"inValue": 0,"uri": "/faults/@@RACK_ID@@:bmu_hardware", "outValue": "Normal"},
                        { "shift": 0,"mask": 1,"inValue": 1,"uri": "/faults/@@RACK_ID@@:bmu_hardware", "outValue": "Fault"},
                        { "shift": 1,"mask": 1,"inValue": 0,"uri": "/faults/@@RACK_ID@@:bcu_hardware", "outValue": "Normal"},
                        { "shift": 1,"mask": 1,"inValue": 1,"uri": "/faults/@@RACK_ID@@:bcu_hardware", "outValue": "Fault"},
                        { "shift": 2,"mask": 1,"inValue": 0,"uri": "/faults/@@RACK_ID@@:fuse_protector", "outValue": "Normal"},
                        { "shift": 2,"mask": 1,"inValue": 1,"uri": "/faults/@@RACK_ID@@:fuse_protector", "outValue": "Fault"},
                        { "shift": 3,"mask": 1,"inValue": 0,"uri": "/faults/@@RACK_ID@@:contactor_adhesion", "outValue": "Normal"},
                        { "shift": 3,"mask": 1,"inValue": 1,"uri": "/faults/@@RACK_ID@@:contactor_adhesion", "outValue": "Fault"},
                        { "shift": 4,"mask": 1,"inValue": 0,"uri": "/faults/@@RACK_ID@@:bmu_comms", "outValue": "Normal"},
                        { "shift": 4,"mask": 1,"inValue": 1,"uri": "/faults/@@RACK_ID@@:bmu_comms", "outValue": "Fault"},
                        { "shift": 5,"mask": 1,"inValue": 0,"uri": "/faults/@@RACK_ID@@:bau_comms", "outValue": "Normal"},
                        { "shift": 5,"mask": 1,"inValue": 1,"uri": "/faults/@@RACK_ID@@:bau_comms", "outValue": "Fault"},
                        { "shift": 6,"mask": 1,"inValue": 0,"uri": "/faults/@@RACK_ID@@:current_sensor", "outValue": "Normal"},
                        { "shift": 6,"mask": 1,"inValue": 1,"uri": "/faults/@@RACK_ID@@:current_sensor", "outValue": "Fault"}

                        { "shift": 0,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_alarm_info[0]", "outValue": true},
                        { "shift": 1,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_alarm_info[1]", "outValue": true},
                        { "shift": 2,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_alarm_info[2]", "outValue": true},
                        { "shift": 3,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_alarm_info3[]", "outValue": true},
                        { "shift": 4,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_alarm_info[4]", "outValue": true},
                        { "shift": 5,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_alarm_info[5]", "outValue": true},
                        { "shift": 6,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_alarm_info[6]", "outValue": true},
                        { "shift": 7,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_alarm_info[7]", "outValue": true},
                        { "shift": 8,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_alarm_info[8]", "outValue": true}
                    ]
                }]
            }
        },
        "rack_warning": {
            "value": 0,
            "note": "0: Normal, 1: Warning",
            "note1": "Bit0: RackVolHigh Warn",
            "note2": "Bit1: RackVolLow Warn",
            "note3": "Bit2: CellVolHigh Warn",
            "note4": "Bit3: CellVolLow Warn",
            "note5": "Bit4: DsgOverCurr Warn",
            "note6": "Bit5: ChgOverCurr Warn",
            "note7": "Bit6: DsgTempHigh Warn",
            "note8": "Bit7: DsgTempLow Warn",
            "note9": "Bit8: ChgTempHigh Warn",
            "note10": "Bit9: ChgTempLow Warn",
            "note11": "Bit10: InsulationLow Warn",
            "note12": "Bit11: TerminalTempHigh Warn",
            "note13": "Bit12: HVBTempHigh Warn",
            "note14": "Bit13: CellVolDiffHigh Warn",
            "note15": "Bit14: CellTempDiffHigh Warn",
            "actions": {
                "onSet": [{
                    "enum": [
                        { "shift": 0,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:rack_volt_high_warn", "outValue": "Normal"},
                        { "shift": 0,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:rack_volt_high_warn", "outValue": "Warning"},
                        { "shift": 1,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:rack_volt_low_warn", "outValue": "Normal"},
                        { "shift": 1,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:rack_volt_low_warn", "outValue": "Warning"},
                        { "shift": 2,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:cell_volt_high_warn", "outValue": "Normal"},
                        { "shift": 2,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:cell_volt_high_warn", "outValue": "Warning"},
                        { "shift": 3,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:cell_volt_low_warn", "outValue": "Normal"},
                        { "shift": 3,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:cell_volt_low_warn", "outValue": "Warning"},
                        { "shift": 4,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:dsg_over_curr_warn", "outValue": "Normal"},
                        { "shift": 4,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:dsg_over_curr_warn", "outValue": "Warning"},
                        { "shift": 5,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:chg_over_curr_warn", "outValue": "Normal"},
                        { "shift": 5,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:chg_over_curr_warn", "outValue": "Warning"},
                        { "shift": 6,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:dsg_temp_high_warn", "outValue": "Normal"},
                        { "shift": 6,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:dsg_temp_high_warn", "outValue": "Warning"}

                        { "shift": 0,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_warnings[0]", "outValue": true},
                        { "shift": 1,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_warnings[1]", "outValue": true},
                        { "shift": 2,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_warnings[2]", "outValue": true},
                        { "shift": 3,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_warnings[3]", "outValue": true},
                        { "shift": 4,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_warnings[4]", "outValue": true},
                        { "shift": 5,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_warnings[5]", "outValue": true},
                        { "shift": 6,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_warnings[6]", "outValue": true}
                    ]
                }]
            }
        },
        "rack_alarm": {
            "value": 0,
            "note": "0: Normal, 1: Warning",
            "note1": "Bit0: RackVolHigh Alarm",
            "note2": "Bit1: RackVolLow Alarm",
            "note3": "Bit2: CellVolHigh Alarm",
            "note4": "Bit3: CellVolLow Alarm",
            "note5": "Bit4: DsgOverCurr Alarm",
            "note6": "Bit5: ChgOverCurr Alarm",
            "note7": "Bit6: DsgTempHigh Alarm",
            "note8": "Bit7: DsgTempLow Alarm",
            "note9": "Bit8: ChgTempHigh Alarm",
            "note10": "Bit9: ChgTempLow Alarm",
            "note11": "Bit10: InsulationLow Alarm",
            "note12": "Bit11: TerminalTempHigh Alarm",
            "note13": "Bit12: HVBTempHigh Alarm",
            "note14": "Bit13: CellVolDiffHigh Alarm",
            "note15": "Bit14: CellTempDiffHigh Alarm",
            "actions": {
                "onSet": [{
                    "enum": [
                        { "shift": 0,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:rack_volt_high_alarm", "outValue": "Normal"},
                        { "shift": 0,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:rack_volt_high_alarm", "outValue": "Alarm"},
                        { "shift": 1,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:rack_volt_low_alarm", "outValue": "Normal"},
                        { "shift": 1,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:rack_volt_low_alarm", "outValue": "Alarm"},
                        { "shift": 2,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:cell_volt_high_alarm", "outValue": "Normal"},
                        { "shift": 2,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:cell_volt_high_alarm", "outValue": "Alarm"},
                        { "shift": 3,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:cell_volt_low_alarm", "outValue": "Normal"},
                        { "shift": 3,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:cell_volt_low_alarm", "outValue": "Alarm"},
                        { "shift": 4,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:dsg_over_curr_alarm", "outValue": "Normal"},
                        { "shift": 4,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:dsg_over_curr_alarm", "outValue": "Alarm"},
                        { "shift": 5,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:chg_over_curr_alarm", "outValue": "Normal"},
                        { "shift": 5,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:chg_over_curr_alarm", "outValue": "Alarm"},
                        { "shift": 6,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:dsg_temp_high_alarm", "outValue": "Normal"},
                        { "shift": 6,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:dsg_temp_high_alarm", "outValue": "Alarm"}

                        { "shift": 0,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_alarms[0]", "outValue": true},
                        { "shift": 1,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_alarms[1]", "outValue": true},
                        { "shift": 2,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_alarms[2]", "outValue": true},
                        { "shift": 3,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_alarms[3]", "outValue": true},
                        { "shift": 4,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_alarms[4]", "outValue": true},
                        { "shift": 5,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_alarms[5]", "outValue": true},
                        { "shift": 6,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_alarms[6]", "outValue": true}
                    ]
                }]
            }
        },
        "rack_critical_alarm": {
            "value": 0,
            "note": "0: Normal, 1: Warning",
            "note1": "Bit0: RackVolHigh Critical Alarm",
            "note2": "Bit1: RackVolLow Critical Alarm",
            "note3": "Bit2: CellVolHigh Critical Alarm",
            "note4": "Bit3: CellVolLow Critical Alarm",
            "note5": "Bit4: DsgOverCurr Critical Alarm",
            "note6": "Bit5: ChgOverCurr Critical Alarm",
            "note7": "Bit6: DsgTempHigh Critical Alarm",
            "note8": "Bit7: DsgTempLow Critical Alarm",
            "note9": "Bit8: ChgTempHigh Critical Alarm",
            "note10": "Bit9: ChgTempLow Critical Alarm",
            "note11": "Bit10: InsulationLow Critical Alarm",
            "note12": "Bit11: TerminalTempHigh Critical Alarm",
            "note13": "Bit12: HVBTempHigh Critical Alarm",
            "note14": "Bit13: CellVolDiffHigh Critical Alarm",
            "note15": "Bit14: CellTempDiffHigh Critical Alarm",
            "actions": {
                "onSet": [{
                    "enum": [
                        { "shift": 0,"mask": 1,"inValue": 0,"uri": "/faults/@@RACK_ID@@:rack_volt_high", "outValue": "Normal"},
                        { "shift": 0,"mask": 1,"inValue": 1,"uri": "/faults/@@RACK_ID@@:rack_volt_high", "outValue": "Critical Alarm"},
                        { "shift": 1,"mask": 1,"inValue": 0,"uri": "/faults/@@RACK_ID@@:rack_volt_low", "outValue": "Normal"},
                        { "shift": 1,"mask": 1,"inValue": 1,"uri": "/faults/@@RACK_ID@@:rack_volt_low", "outValue": "Critical Alarm"},
                        { "shift": 2,"mask": 1,"inValue": 0,"uri": "/faults/@@RACK_ID@@:cell_volt_high", "outValue": "Normal"},
                        { "shift": 2,"mask": 1,"inValue": 1,"uri": "/faults/@@RACK_ID@@:cell_volt_high", "outValue": "Critical Alarm"},
                        { "shift": 3,"mask": 1,"inValue": 0,"uri": "/faults/@@RACK_ID@@:cell_volt_low", "outValue": "Normal"},
                        { "shift": 3,"mask": 1,"inValue": 1,"uri": "/faults/@@RACK_ID@@:cell_volt_low", "outValue": "Critical Alarm"},
                        { "shift": 4,"mask": 1,"inValue": 0,"uri": "/faults/@@RACK_ID@@:dsg_over_curr", "outValue": "Normal"},
                        { "shift": 4,"mask": 1,"inValue": 1,"uri": "/faults/@@RACK_ID@@:dsg_over_curr", "outValue": "Critical Alarm"},
                        { "shift": 5,"mask": 1,"inValue": 0,"uri": "/faults/@@RACK_ID@@:chg_over_curr", "outValue": "Normal"},
                        { "shift": 5,"mask": 1,"inValue": 1,"uri": "/faults/@@RACK_ID@@:chg_over_curr", "outValue": "Critical Alarm"},
                        { "shift": 6,"mask": 1,"inValue": 0,"uri": "/faults/@@RACK_ID@@:dsg_temp_high", "outValue": "Normal"},
                        { "shift": 6,"mask": 1,"inValue": 1,"uri": "/faults/@@RACK_ID@@:dsg_temp_high", "outValue": "Critical Alarm"}

                        { "shift": 0,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_critical_alarms[0]", "outValue": true},
                        { "shift": 1,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_critical_alarms[1]", "outValue": true},
                        { "shift": 2,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_critical_alarms[2]", "outValue": true},
                        { "shift": 3,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_critical_alarms[3]", "outValue": true},
                        { "shift": 4,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_critical_alarms[4]", "outValue": true},
                        { "shift": 5,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_critical_alarms[5]", "outValue": true},
                        { "shift": 6,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_critical_alarms[6]", "outValue": true}
                    ]
                }]
            }
        },
        "rack_step_in_failure": {
            "value": 0,
            "note": "0: Normal, 1: Warning",
            "note1": "Bit0: Rack Vol Diff Large",
            "note2": "Bit1: SMS Confirmation Overtime",
            "actions": {
                "onSet": [{
                    "enum": [
                        { "shift": 0,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:rack_vol_diff_large", "outValue": "Normal"},
                        { "shift": 0,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:rack_vol_diff_large", "outValue": "Warning"},
                        { "shift": 1,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:sms_confirmation_overtime", "outValue": "Normal"},
                        { "shift": 1,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:sms_confirmation_overtime", "outValue": "Warning"}
                    ]
                }]
            }
        },
        "precharge_total_vol": 0,
        "rack_voltage": {"value":0,"actions": { "onSet":[{"remap":[{"uri":"/status/@@RACK_ID@@:RackVoltage"}]}]}},
        "rack_current": {"value":0,"actions": { "onSet":[{"remap":[{"uri":"/status/@@RACK_ID@@:RackCurrent"}]}]}},
        "rack_state": {"value": 0,"actions": {"onSet": [{
                    "enum": [
                        { "shift": 0,"mask": 255,"inValue": 0,"uri": "/status/@@RACK_ID@@:ChargeDischargeState", "outValue": "Idle"},
                        { "shift": 0,"mask": 255,"inValue": 1,"uri": "/status/@@RACK_ID@@:ChargeDischargeState", "outValue": "Discharging"},
                        { "shift": 0,"mask": 255,"inValue": 2,"uri": "/status/@@RACK_ID@@:ChargeDischargeState", "outValue": "Charging"}
                    ]
                }]
            }
        },
        "rack_soc": {"value":0,"actions": { "onSet":[{"remap":[{"uri":"/status/@@RACK_ID@@:RackSOC"}]}]}},
        "rack_soh": {"value":0,"actions": { "onSet":[{"remap":[{"uri":"/status/@@RACK_ID@@:RackSOH"}]}]}},
        "rack_insulation": 0,
        "rack_positive_insulation": 0,
        "rack_negative_insulation": 0,
        "rack_max_charge_current": {"value":0,"actions": { "onSet":[{"remap":[{"uri":"/status/@@RACK_ID@@:RackMaxChargeCurrent"      }]}]}},
        "rack_max_discharge_current": {"value":0,"actions": { "onSet":[{"remap":[{"uri":"/status/@@RACK_ID@@:RackMaxDischargeCurrent"}]}]}},
        "rack_max_cell_voltage":      {"value":0,"actions": { "onSet":[{"remap":[{"uri":"/status/@@RACK_ID@@:RackMaxCellVoltage"     }]}]}},
        "rack_min_cell_voltage":      {"value":0,"actions": { "onSet":[{"remap":[{"uri":"/status/@@RACK_ID@@:RackMinCellVoltage"     }]}]}},
        "rack_max_cell_temperature":  {"value":0,"actions": { "onSet":[{"remap":[{"uri":"/status/@@RACK_ID@@:RackMaxCellTemp"   }]}]}},
        "rack_min_cell_temperature":  {"value":0,"actions": { "onSet":[{"remap":[{"uri":"/status/@@RACK_ID@@:RackMinCellTemp"   }]}]}},
        "rack_avg_voltage":           {"value":0,"actions": { "onSet":[{"remap":[{"uri":"/status/@@RACK_ID@@:RackAvgCellVoltage"}]}]}},
        "rack_avg_temperature":       {"value":0,"actions": { "onSet":[{"remap":[{"uri":"/status/@@RACK_ID@@:RackAvgCellTemp"   }]}]}}
    },
    "/components/hvac_params": {
        "@@RACK_ID@@_remote_ac_on": 0,
        "@@RACK_ID@@_cooling_temp_setting": 0,
        "@@RACK_ID@@_cooling_setpoint_upper_limit": 0,
        "@@RACK_ID@@_ht_warning_setting": 0,
        "@@RACK_ID@@_lt_warning_setting": 0,
        "@@RACK_ID@@_interior_air_fan_stop_setting": 0,
        "@@RACK_ID@@_heating_temp_setting": 0,
        "@@RACK_ID@@_heating_setpoint_lower_limit": 0,
        "@@RACK_ID@@_dehumidity_setting": 0
    },
    "/components/hvac_info": {
        "@@AC_1_ID@@_unit_running_status": {
            "value": 0,
            "actions": {
                "onSet": [{
                    "enum": [
                        { "shift": 0,"mask": 255,"inValue": 0,"uri": "/assets/bms/@@RACK_ID@@:hvac_1_unit_running_status", "outValue": "Stop"},
                        { "shift": 0,"mask": 255,"inValue": 1,"uri": "/assets/bms/@@RACK_ID@@:hvac_1_unit_running_status", "outValue": "Running"}
                    ]
                }]
            }
        },
        "@@AC_1_ID@@_inside_return_temp": {
            "value": 0,
            "actions": {
                "onSet": [{
                    "remap": [
                        {"uri": "/assets/bms/@@RACK_ID@@:hvac_1_temperature"}
                    ]
                }]
            }
        },
        "@@AC_1_ID@@_alarms_1": {
            "value": 0,
            "actions": {
                "onSet": [{
                    "enum": [
                        { "shift": 0,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_1_high_temp", "outValue": "Normal"},
                        { "shift": 0,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_1_high_temp", "outValue": "Warning"},
                        { "shift": 1,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_1_low_temp", "outValue": "Normal"},
                        { "shift": 1,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_1_low_temp", "outValue": "Warning"},
                        { "shift": 2,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_1_high_humidity", "outValue": "Normal"},
                        { "shift": 2,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_1_high_humidity", "outValue": "Warning"},
                        { "shift": 3,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_1_low_humidity", "outValue": "Normal"},
                        { "shift": 3,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_1_low_humidity", "outValue": "Warning"},
                        { "shift": 4,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_1_heat_exchanger_coil_frost", "outValue": "Normal"},
                        { "shift": 4,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_1_heat_exchanger_coil_frost", "outValue": "Warning"},
                        { "shift": 5,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_1_exhaust_temp_high", "outValue": "Normal"},
                        { "shift": 5,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_1_exhaust_temp_high", "outValue": "Warning"},
                        { "shift": 6,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_1_heat_exchanger_coil_temp_sensing_failure", "outValue": "Normal"},
                        { "shift": 6,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_1_heat_exchanger_coil_temp_sensing_failure", "outValue": "Warning"}

                        { "shift": 0,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_@@AC_1_ID@@_alarms[0]", "outValue": true},
                        { "shift": 1,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_@@AC_1_ID@@_alarms[1]", "outValue": true},
                        { "shift": 2,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_@@AC_1_ID@@_alarms[2]", "outValue": true},
                        { "shift": 3,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_@@AC_1_ID@@_alarms[3]", "outValue": true},
                        { "shift": 4,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_@@AC_1_ID@@_alarms[4]", "outValue": true},
                        { "shift": 5,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_@@AC_1_ID@@_alarms[5]", "outValue": true},
                        { "shift": 6,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_@@AC_1_ID@@_alarms[6]", "outValue": true}
                    ]
                }]
            }
        },
        "@@AC_1_ID@@_alarms_2": {
            "value": 0,
            "actions": {
                "onSet": [{
                    "enum": [
                        { "shift": 0,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_1_emergency_air_fan_failure", "outValue": "Normal"},
                        { "shift": 0,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_1_emergency_air_fan_failure", "outValue": "Warning"},
                        { "shift": 1,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_1_high_air_pressure", "outValue": "Normal"},
                        { "shift": 1,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_1_high_air_pressure", "outValue": "Warning"},
                        { "shift": 2,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_1_low_air_pressure", "outValue": "Normal"},
                        { "shift": 2,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_1_low_air_pressure", "outValue": "Warning"},
                        { "shift": 3,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_1_flood", "outValue": "Normal"},
                        { "shift": 3,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_1_flood", "outValue": "Warning"},
                        { "shift": 4,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_1_smoke", "outValue": "Normal"},
                        { "shift": 4,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_1_smoke", "outValue": "Warning"},
                        { "shift": 5,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_1_door_open", "outValue": "Normal"},
                        { "shift": 5,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_1_door_open", "outValue": "Warning"},
                        { "shift": 6,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_1_high_air_pressure_lockdown", "outValue": "Normal"},
                        { "shift": 6,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_1_high_air_pressure_lockdown", "outValue": "Warning"}

                        { "shift": 0,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_@@AC_1_ID@@_alarms[16]", "outValue": true},
                        { "shift": 1,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_@@AC_1_ID@@_alarms[17]", "outValue": true},
                        { "shift": 2,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_@@AC_1_ID@@_alarms[18]", "outValue": true},
                        { "shift": 3,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_@@AC_1_ID@@_alarms[19]", "outValue": true},
                        { "shift": 4,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_@@AC_1_ID@@_alarms[20]", "outValue": true},
                        { "shift": 5,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_@@AC_1_ID@@_alarms[21]", "outValue": true},
                        { "shift": 6,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_@@AC_1_ID@@_alarms[22]", "outValue": true},
                        { "shift": 7,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:@@RACK_ID@@_@@AC_1_ID@@_alarms[23]", "outValue": true}
                    ]
                }]
            }
        },
        "@@AC_1_ID@@_alarms_3": {
            "value": 0,
            "actions": {
                "onSet": [{
                    "enum": [
                        { "shift": 0,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_1_dc_low_voltage", "outValue": "Normal"},
                        { "shift": 0,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_1_dc_low_voltage", "outValue": "Warning"}
                    ]
                }]
            }
        },

        "@@AC_2_ID@@_unit_running_status": {
            "value": 0,
            "actions": {
                "onSet": [{
                    "enum": [
                        { "shift": 0,"mask": 255,"inValue": 0,"uri": "/assets/bms/@@RACK_ID@@:hvac_2_unit_running_status", "outValue": "Stop"},
                        { "shift": 0,"mask": 255,"inValue": 1,"uri": "/assets/bms/@@RACK_ID@@:hvac_2_unit_running_status", "outValue": "Running"}
                    ]
                }]
            }
        },
        "@@AC_2_ID@@_inside_return_temp": {
            "value": 0,
            "actions": {
                "onSet": [{
                    "remap": [
                        {"uri": "/assets/bms/@@RACK_ID@@:hvac_2_temperature"}
                    ]
                }]
            }
        },
        "@@AC_2_ID@@_alarms_1": {
            "value": 0,
            "actions": {
                "onSet": [{
                    "enum": [
                        { "shift": 0,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_2_high_temp", "outValue": "Normal"},
                        { "shift": 0,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_2_high_temp", "outValue": "Warning"},
                        { "shift": 1,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_2_low_temp", "outValue": "Normal"},
                        { "shift": 1,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_2_low_temp", "outValue": "Warning"},
                        { "shift": 2,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_2_high_humidity", "outValue": "Normal"},
                        { "shift": 2,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_2_high_humidity", "outValue": "Warning"},
                        { "shift": 3,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_2_low_humidity", "outValue": "Normal"},
                        { "shift": 3,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_2_low_humidity", "outValue": "Warning"},
                        { "shift": 4,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_2_heat_exchanger_coil_frost", "outValue": "Normal"},
                        { "shift": 4,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_2_heat_exchanger_coil_frost", "outValue": "Warning"},
                        { "shift": 5,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_2_exhaust_temp_high", "outValue": "Normal"},
                        { "shift": 5,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_2_exhaust_temp_high", "outValue": "Warning"},
                        { "shift": 6,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_2_heat_exchanger_coil_temp_sensing_failure", "outValue": "Normal"},
                        { "shift": 6,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_2_heat_exchanger_coil_temp_sensing_failure", "outValue": "Warning"},
                        { "shift": 7,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_2_exterior_temp_sensing_failure", "outValue": "Normal"},
                        { "shift": 7,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_2_exterior_temp_sensing_failure", "outValue": "Warning"}
                    ]
                }]
            }
        },
        "@@AC_2_ID@@_alarms_2": {
            "value": 0,
            "actions": {
                "onSet": [{
                    "enum": [
                        { "shift": 0,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_2_emergency_air_fan_failure", "outValue": "Normal"},
                        { "shift": 0,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_2_emergency_air_fan_failure", "outValue": "Warning"},
                        { "shift": 1,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_2_high_air_pressure", "outValue": "Normal"},
                        { "shift": 1,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_2_high_air_pressure", "outValue": "Warning"},
                        { "shift": 2,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_2_low_air_pressure", "outValue": "Normal"},
                        { "shift": 2,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_2_low_air_pressure", "outValue": "Warning"},
                        { "shift": 3,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_2_flood", "outValue": "Normal"},
                        { "shift": 3,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_2_flood", "outValue": "Warning"},
                        { "shift": 4,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_2_smoke", "outValue": "Normal"},
                        { "shift": 4,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_2_smoke", "outValue": "Warning"},
                        { "shift": 5,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_2_door_open", "outValue": "Normal"},
                        { "shift": 5,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_2_door_open", "outValue": "Warning"}
                    ]
                }]
            }
        },
        "@@AC_2_ID@@_alarms_3": {
            "value": 0,
            "actions": {
                "onSet": [{
                    "enum": [
                        { "shift": 0,"mask": 1,"inValue": 0,"uri": "/alarms/@@RACK_ID@@:hvac_2_dc_low_voltage", "outValue": "Normal"},
                        { "shift": 0,"mask": 1,"inValue": 1,"uri": "/alarms/@@RACK_ID@@:hvac_2_dc_low_voltage", "outValue": "Warning"}
                    ]
                }]
            }
        }
    },

    "/faults/@@RACK_ID@@": {
        "clear_faults": {
            "value": "Normal",
            "type": "fault",
            "actions": {
                "onSet": [{"func": [{"func": "process_sys_alarm", "amap": "@@RACK_ID@@"}]}]
            }
        },
        "rack_volt_high":            {"value":"Normal","type":"fault","actions":{"onSet":[{"func":[{"func":"process_sys_alarm","amap":"@@RACK_ID@@"}]}]}},
        "rack_volt_low":             {"value":"Normal","type":"fault","actions":{"onSet":[{"func":[{"func":"process_sys_alarm","amap":"@@RACK_ID@@"}]}]}},
        "cell_volt_high":            {"value":"Normal","type":"fault","actions":{"onSet":[{"func":[{"func":"process_sys_alarm","amap":"@@RACK_ID@@"}]}]}},
        "cell_volt_low":             {"value":"Normal","type":"fault","actions":{"onSet":[{"func":[{"func":"process_sys_alarm","amap":"@@RACK_ID@@"}]}]}},
        "dsg_over_curr":             {"value":"Normal","type":"fault","actions":{"onSet":[{"func":[{"func":"process_sys_alarm","amap":"@@RACK_ID@@"}]}]}},
        "chg_over_curr":             {"value":"Normal","type":"fault","actions":{"onSet":[{"func":[{"func":"process_sys_alarm","amap":"@@RACK_ID@@"}]}]}}

    },
    "/alarms/@@RACK_ID@@": {
        "clear_alarms": {
            "value": "Normal",
            "type": "alarm",
            "actions": {
                "onSet": [{"func": [{"func": "process_sys_alarm", "amap": "@@RACK_ID@@"}]}]
            }
        },
        "rack_volt_high_warn":         {"value":"Normal","type":"alarm","actions":{"onSet":[{"func":[{"func":"process_sys_alarm","amap":"@@RACK_ID@@"}]}]}},
        "rack_volt_low_warn":          {"value":"Normal","type":"alarm","actions":{"onSet":[{"func":[{"func":"process_sys_alarm","amap":"@@RACK_ID@@"}]}]}},
        "cell_volt_high_warn":         {"value":"Normal","type":"alarm","actions":{"onSet":[{"func":[{"func":"process_sys_alarm","amap":"@@RACK_ID@@"}]}]}},
        "cell_volt_low_warn":          {"value":"Normal","type":"alarm","actions":{"onSet":[{"func":[{"func":"process_sys_alarm","amap":"@@RACK_ID@@"}]}]}},
        "dsg_over_curr_warn":          {"value":"Normal","type":"alarm","actions":{"onSet":[{"func":[{"func":"process_sys_alarm","amap":"@@RACK_ID@@"}]}]}}
    }
}
```


## Manual Loading

These two commands can simulate the loader (for testing)

```

fims_send -m set -f configs/dbi/ess_controller5/risen_bms_manager.json -u /ess/cfg/cfile/ess/risen_bms_manager
fims_send -m set -f configs/dbi/ess_controller5/risen_bms_template.json -u /ess/cfg/ctmpl/ess/risen_bms_template

```

And the load operation checked 

```
 fims_send -m get -r /$$ -u /ess/naked/config/load | jq
{
  "/config/load": {
    "ess_controller": true,
    "risen_bms_manager": true
  }
}
```


## What is a template ??

A template is a file that is going to be processed by the system before bebg loaded into the system config.
Normally template files are processed with layered expaansion rules , using variable substitutions to create multiple instances of a config layout.
This process is disussed in the next video.

The whole file is read into a parameter called "body".
Then the whole file is processed by a special handler and inserted into the system datamap.

The file, in this case, is an exact copy of the same file used to configure the  modbus_server interface to the site controller. 
Change data in one and the other follows,



