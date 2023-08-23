import { AssetsResponse } from './responses';
import { Assets } from './dtos/asset.dto';
import { AddLayout } from '../bff/Layouts/dto/layout.dto';

export const ASSETS_RESPONSE: AssetsResponse = {
    assets: 'Example Assets Response'
};

// Modified example config file from SMT project
export const ASSETS_POST: Assets = {
    data: [{
        "info": {
            "asset": "PCS",
            "assetKey": "pcs",
            "itemName": "PCS",
            "hasSummary": true,
            "baseURI": "/pcs",
            "extension": "/pcs_",
            "name": "PCS",
            "numberOfItems": "2",
            "sourceURI": "/assets",
            "alarmFields": [ "alarms" ],
            "faultFields": [ "faults" ]
        },
        "alarms": {
            "alarmFields": [],
            "faultFields": []
        },
        "allControls": [],
        "controls": [],
        "statuses": [
            {"name": "Active Power"         ,"uri": "/active_power"         , "units": "kW"   , "scalar": ""},
            {"name": "Reactive Power"       ,"uri": "/reactive_power"       , "units": "kVAR" , "scalar": ""},
            {"name": "Apparent Power"       ,"uri": "/apparent_power"       , "units": "kVA"  , "scalar": ""},
            {"name": "PCS DC Voltage"       ,"uri": "/pcs_dc_voltage"       , "units": "V"    , "scalar": ""},
            {"name": "PCS DC Current"       ,"uri": "/pcs_dc_current"       , "units": "A"    , "scalar": ""},
            {"name": "PCS DC Power"         ,"uri": "/pcs_dc_power"         , "units": "kW"   , "scalar": ""},
        ],
        "summaryControls": [
            { "inputType": "switch", "name": "Maintenance Mode"                           , "uri": "/maint_mode"               },
            { "inputType": "button", "name": "Start"                                      , "uri": "/start"                    },
            { "inputType": "button", "name": "Stop"                                       , "uri": "/stop"                     },
            { "inputType": "number", "name": "Active Power Setpoint (kW)"                 , "uri": "/active_power_setpoint"    },
            { "inputType": "number", "name": "Reactive Power Setpoint (kVAr)"             , "uri": "/reactive_power_setpoint"  },
            { "inputType": "button", "name": "Clear Faults"                               , "uri": "/clear_faults"             }
        ],
        "summary": [
            {"name": "Comms OK"             , "uri": "/comms_ok_status"      , "units": ""     , "scalar": ""},
            {"name": "System State"         , "uri": "/system_state_status"  , "units": ""     , "scalar": ""},
            {"name": "Grid Mode"            , "uri": "/grid_mode_status"     , "units": ""     , "scalar": ""},
            {"name": "Active Power"         , "uri": "/active_power"         , "units": "kW"   , "scalar": ""},
            {"name": "Reactive Power"       , "uri": "/reactive_power"       , "units": "kVAR" , "scalar": ""},
            {"name": "Apparent Power"       , "uri": "/apparent_power"       , "units": "kVA"  , "scalar": ""},
            {"name": "PCS DC Voltage"       , "uri": "/pcs_dc_voltage"       , "units": "V"    , "scalar": ""},
            {"name": "PCS DC Current"       , "uri": "/pcs_dc_current"       , "units": "A"    , "scalar": ""},
            {"name": "PCS DC Power"         , "uri": "/pcs_dc_power"         , "units": "kW"   , "scalar": ""},
            
            {"name": "Active Power Setpoint [Feedback]"             ,"uri": "/active_power_setpoint_feedback"             , "units": "kW"     , "scalar": ""},
            {"name": "Reactive Power Setpoint [Feedback]"           ,"uri": "/reactive_power_setpoint_feedback"           , "units": "kVAR"   , "scalar": ""},
        ]
    }]
};

export const ASSETS_REDUCED: AddLayout = {
    data: [{
        info: {
            name: "PCS",
            key: "pcs_tab",
        }
    }]
};

export const ASSETS_POST_DUPLICATE: Assets = {
    data: [
        {
            "info": {
                "asset": "PCS",
                "assetKey": "pcs",
                "itemName": "PCS",
                "hasSummary": true,
                "baseURI": "/pcs",
                "extension": "/pcs_",
                "name": "PCS",
                "numberOfItems": "2",
                "sourceURI": "/assets",
                "alarmFields": [ "alarms" ],
                "faultFields": [ "faults" ]
            },
            "alarms": {
                "alarmFields": [],
                "faultFields": []
            },
            "allControls": [],
            "controls": [],
            "statuses": [],
            "summaryControls": [],
            "summary": []
        },
        {
            "info": {
                "asset": "PCS",
                "assetKey": "pcs",
                "itemName": "PCS",
                "hasSummary": true,
                "baseURI": "/pcs",
                "extension": "/pcs_",
                "name": "PCS",
                "numberOfItems": "2",
                "sourceURI": "/assets",
                "alarmFields": [ "alarms" ],
                "faultFields": [ "faults" ]
            },
            "alarms": {
                "alarmFields": [],
                "faultFields": []
            },
            "allControls": [],
            "controls": [],
            "statuses": [],
            "summaryControls": [],
            "summary": []
        }
    ]
};

export const ASSETS_REDUCED_DUPLICATE: AddLayout = {
    data: [
        {
            info: {
                name: "PCS",
                key: "pcs_tab",
            }
        },
        {
            info: {
                name: "PCS",
                key: "pcs_tab_2",
            }
        }
]};