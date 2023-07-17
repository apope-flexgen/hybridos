# ESS Controller v10.1 - UI Configurations and Data Publishes

## Table of Contents
1. [Overview](#Overview)
2. [Configure UI Data Publishes](#Configure-UI-Data-Publishes)
3. [Configure Dashboard Page](#Configure-Dashboard-Page)
4. [Configure Assets Page](#Configure-Assets-Page)
5. [Tips](#Tips)

## Overview
This document contains information on how to configure the ESS Controller to publish UI-related data to the web server and how to configure the Dashboard and Assets pages that are currently supported for v10.1.

The information is related to what was discussed in both `UI Data Publishes` and `Dashboard and Assets Configuration` videos.

Note that all of the configurations discussed below are stored in MongoDB. We use the database interface (DBI) module to send/retrieve configuration data and other data objects to/from the database.

## Configure UI Data Publishes
Currently, the web server listens/sends data in/to `/assets`, which is the base source URI that contains data to display on the UI.

If the BMS is involved, then typically, UI data are contained in `/assets/bms/summary` and `/assets/bms/<asset_instance>`. If the PCS is involved, then typically, UI data are contained in `/assets/pcs/summary` and `/assets/pcs/<asset_instance>`
* `<asset_instance>` is the asset managed by the asset manager (e.g.: bms, pcs)
  * For example, `/assets/bms/rack_1` would indicate UI data includes battery rack 1 data

As an example, let's say we want to display BMS data on the UI. To display this particular set of data, we'll publish data contained in `/assets/bms/summary` to the UI. Let's say we want to publish this data every 100 milliseconds (ms).

Here is a configuration example that contains the data we want to display on the UI:
```json
"/assets/bms/summary": {
    "name": "BMS Manager",
    "alarms": {
        "value": 0,
        "options": [],
        "enabled": true
    },
    "faults": {
        "value": 0,
        "options": [],
        "enabled": true
    },
    "power_state": {
        "name": "Power State",
        "value": "OFF",
        "enabled": true
    },
    "system_state": {
        "name": "Batteries Status",
        "value": "INIT",
        "enabled": true
    },
    "com_status": {
        "name": "Communication Status",
        "value": "INIT",
        "enabled": true
    },
    "soc": {
        "name": "State of Charge",
        "value": 0,
        "enabled": false
    },

    ...
}
```

And here is a configuration example for publishing data to the UI:
```json
    "/system/commands": {
        "runOpts": {
            "value": false,
            "enabled": false,
            "targav": "/system/commands:run",
            "options": [
                {"uri":"/sched/bms:pubAssets_bms","aname":"bms","value":0,"every":0.1,"offset":0}
            ],
            "actions": {
                "onSet":[{"func":[{"func":"SchedItemOpts"}]}]
            }
        }
    },
    "/sched/bms": {
        "pubAssets_bms": {
            "value": 0,
            "mode": "ui",
            "table": "/assets/bms/summary",
            "actions": {
                "onSet":[{"func":[{"func":"RunPub"}]}]
            }
        }
    }
```
* Note:
  * In `/sched/bms:pubAssets_bms`, we are publishing `/assets/bms/summary` data
    * Notice `_bms` is appended to the variable name in the URI. A scheduled item should contain a unique name to properly run scheduled tasks
  * In `/system/commands:runOpts`, we are sending data in `/sched/bms:pubAssets_bms` to `/system/commands:run`, which will trigger the scheduler to set to `/sched/bms:pubAssets_bms` every 100 ms, which triggers `RunPub` and publishes data
    * Notice `enabled` is set to false. This means we're not setting `/sched/bms:pubAssets_bms` data defined in `options` to `/system/commands:run`. In other words, we're not triggering the scheduler defined in `/system/commands:run`

Finally, here are configuration examples for configuring and triggering the scheduler:
```json
    "/system/commands": {
        "run": {
            "help": "run a schedule var needs the uri to be set ",
            "value":0,
            "ifChanged":false,
            "enabled":true, 
            "actions":{
                "onSet":[{"func":[{"func":"RunSched"}]}]
            }
        },
        "runOpts": {
            "value": true,
            "enabled": true
        }
    }
```
* Note:
  * `/system/commands:run` is configured to run `RunSched`, which is an interface to a scheduler. This scheduler allows us to publish UI data at a defined rate, for example
  * `/system/commands:runOpts@enabled` and `/system/commands:runOpts@value` are set to true. This is to allow 
    * `enabled` is used to enable our action. For example, `SchedItemOpts` function is enabled for `/system/commands:runOpts`
    * `value` is used to trigger the `SchedItemOpts` function defined in the `actions` field in `/system/commands:runOpts`

Once configuration is complete, we should expect to see the following fims publishes:
```
$ fims_listen -u /assets/bms/summary
Method:  pub
Uri:     /assets/bms/summary
ReplyTo: (null)
Body:    {"alarms":{"value":0,"options":[],"enabled":true},"avg_cell_temp":{"value":50,"enabled":true},"avg_cell_voltage":{"value":3.437,"enabled":true},"clear_faults":{"value":false,"options":[{"name":"Clear Faults","return_value":"Clear"}],"enabled":false},"com_status":{"value":"ONLINE","enabled":true},"faults":{"value":0,"options":[],"enabled":true},"maint_mode":{"value":false,"options":[{"name":"No","return_value":false},{"name":"Yes","return_value":true}],"enabled":true},"max_cell_temp":{"value":51,"enabled":true},"max_cell_voltage":{"value":3.452,"enabled":true},"max_charge_current":{"value":0,"enabled":true},"max_charge_power":{"value":-40.819,"enabled":true},"max_discharge_current":{"value":0,"enabled":true},"max_discharge_power":{"value":91.519,"enabled":true},"min_cell_temp":{"value":49,"enabled":true},"min_cell_voltage":{"value":3.421,"enabled":true},"name":{"value":"BMS Manager","enabled":true},"power_state":{"value":"Off Ready","enabled":true},"remain_charge_energy":{"value":1674,"enabled":true},"remain_discharge_energy":{"value":1674,"enabled":true},"soc":{"value":50,"enabled":true},"soh":{"value":100,"enabled":true},"start":{"value":false,"options":[{"name":"No","return_value":false},{"name":"Yes","return_value":true}],"enabled":false},"status":{"value":0,"enabled":true},"stop":{"value":false,"options":[{"name":"No","return_value":false},{"name":"Yes","return_value":true}],"enabled":false},"system_current":{"value":0,"enabled":true},"system_power":{"value":0,"enabled":true},"system_state":{"value":"Normal","enabled":true},"system_voltage":{"value":1306.2,"enabled":true},"units_connected":{"value":0,"enabled":true}}
Timestamp:   2022-03-08 19:01:22.669999
Method:  pub
Uri:     /assets/bms/summary
ReplyTo: (null)
Body:    {"alarms":{"value":0,"options":[],"enabled":true},"avg_cell_temp":{"value":50,"enabled":true},"avg_cell_voltage":{"value":3.437,"enabled":true},"clear_faults":{"value":false,"options":[{"name":"Clear Faults","return_value":"Clear"}],"enabled":false},"com_status":{"value":"ONLINE","enabled":true},"faults":{"value":0,"options":[],"enabled":true},"maint_mode":{"value":false,"options":[{"name":"No","return_value":false},{"name":"Yes","return_value":true}],"enabled":true},"max_cell_temp":{"value":51,"enabled":true},"max_cell_voltage":{"value":3.452,"enabled":true},"max_charge_current":{"value":0,"enabled":true},"max_charge_power":{"value":-40.819,"enabled":true},"max_discharge_current":{"value":0,"enabled":true},"max_discharge_power":{"value":91.519,"enabled":true},"min_cell_temp":{"value":49,"enabled":true},"min_cell_voltage":{"value":3.421,"enabled":true},"name":{"value":"BMS Manager","enabled":true},"power_state":{"value":"Off Ready","enabled":true},"remain_charge_energy":{"value":1674,"enabled":true},"remain_discharge_energy":{"value":1674,"enabled":true},"soc":{"value":50,"enabled":true},"soh":{"value":100,"enabled":true},"start":{"value":false,"options":[{"name":"No","return_value":false},{"name":"Yes","return_value":true}],"enabled":false},"status":{"value":0,"enabled":true},"stop":{"value":false,"options":[{"name":"No","return_value":false},{"name":"Yes","return_value":true}],"enabled":false},"system_current":{"value":0,"enabled":true},"system_power":{"value":0,"enabled":true},"system_state":{"value":"Normal","enabled":true},"system_voltage":{"value":1306.2,"enabled":true},"units_connected":{"value":0,"enabled":true}}
Timestamp:   2022-03-08 19:01:22.770033
```
* Note:
  * In `Body`, `alarms` and `faults` variables contain `value`, `options`, and `enabled` fields
    * The `options` field contains alarm/fault messages to display on the UI
  * In `Body`, the rest of the UI variables contain `value` and `enabled` fields
    * The `value` field contains the value to display on the UI
    * The `enabled` field is used to allow the web server to enable/disable UI controls
  * The ESS Controller publishes `/assets/bms/summary` data every 100 ms (compare `Timestamp` in publishes)
  * Additional fields such as unit, scale, and name are included in a separate configuration data object (e.g.: dashboard, assets), which is now managed by the web server, not the ESS Controller

## Configure Dashboard Page
Dashboard configuration data is stored in `/dbi/ui_config/dashboard`
* To access this in the command line, you can run:
  * `$ fims_send -m get -r /me -u /dbi/ui_config/dashboard | jq`
* To save the configuration data in a file in the command line, you can run:
  * `$ fims_send -m get -r /me -u /dbi/ui_config/dashboard | jq >> /path/to/config/file.json`
* To upload the configuration data in a file to the database in the command line, you can run:
  * `$ fims_send -m set -u /me -u /dbi/ui_config/dashboard -f /path/to/config/file.json`

Here is a configuration example for the dashboard:
```json
{
  "_doc": "dashboard",
  "_id": "61aa4b2e3196d866e85e7cb9",
  "_version": "2021-12-03T18:14:59.934Z",
  "data": [
    {
      "info": {
        "baseURI": "",
        "batteryView": true,
        "batteryViewSourceURI": "/assets",
        "batteryViewURI": "/soc",
        "isTemplate": true,
        "items": [
          {
            "name": "Rack 1",
            "uri": "/bms/sbmu_1"
          },
          {
            "name": "Rack 2",
            "uri": "/bms/sbmu_2"
          },

          ...
        ],
        "name": "CATL BMS",
        "sourceURIs": [
          "/assets"
        ]
      },
      "status": [
        {
          "name": "SOH",
          "scalar": "1",
          "sourceURI": "/assets",
          "units": "%",
          "uri": "/soh"
        },
        {
          "name": "System Voltage",
          "scalar": "1",
          "sourceURI": "/assets",
          "units": "V",
          "uri": "/system_voltage"
        },
        {
          "name": "Contactor Status",
          "scalar": null,
          "sourceURI": "/assets",
          "units": "",
          "uri": "/dc_contactor_status"
        },

        ...
      ]
    }
  ]
}
```

## Configure Assets Page
Assets configuration data is stored in `/dbi/ui_config/assets`
* To access this in the command line, you can run:
  * `$ fims_send -m get -r /me -u /dbi/ui_config/assets | jq`
* To save the configuration data in a file in the command line, you can run:
  * `$ fims_send -m get -r /me -u /dbi/ui_config/assets | jq >> /path/to/config/file.json`
* To upload the configuration data in a file to the database in the command line, you can run:
  * `$ fims_send -m set -u /me -u /dbi/ui_config/assets -f /path/to/config/file.json`

Here is a configuration example for assets:
```json
{
  "data": [
    {
      "alarms": {
        "alarmFields": [],
        "faultFields": []
      },
      "controls": [
        {
          "inputType": "switch",
          "name": "Maintenance Mode",
          "uri": "/maint_mode"
        },
        {
          "inputType": "button",
          "name": "Clear Faults",
          "uri": "/clear_faults"
        }
      ],
      "info": {
        "asset": "BMS",
        "baseURI": "/bms",
        "extension": "/sbmu_",
        "hasSummary": true,
        "name": "CATL BMS",
        "numberOfItems": "9",
        "sourceURI": "/assets",
        "alarmFields": [
          "alarms"
        ],
        "faultFields": [
          "faults"
        ]
      },
      "statuses": [
        {
          "name": "State of Charge",
          "scalar": "1",
          "units": "%",
          "uri": "/soc"
        },
        {
          "name": "State of Health",
          "scalar": "1",
          "units": "%",
          "uri": "/soh"
        },

        ...
      ],
      "summary": [
        {
          "name": "Power State",
          "scalar": "0",
          "units": "",
          "uri": "/power_state"
        },
        {
          "name": "Batteries Status",
          "scalar": "0",
          "units": "",
          "uri": "/system_state"
        },

        ...
      ],
      "summaryControls": [
        {
          "inputType": "switch",
          "name": "Maintenance Mode",
          "uri": "/maint_mode"
        },
        {
          "inputType": "button",
          "name": "Close DC Contactor",
          "uri": "/start"
        },

        ...
      ]
    }
  ]
}
```


## Tips
### ESS Controller
To get the variable's value, parameters, and actions, run:
* `$ fims_send -m get -r /me -u /ess/full/<uri_and_variable_name>`
  * e.g.: `$fims_send -m get -r /me -u /ess/full/sched/bms/pubAssets_bms | jq`

To get a list of scheduled items, run:
* `$ fims_send -m get -r /me -u /ess/full/schlist | jq`
