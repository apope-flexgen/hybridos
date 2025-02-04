const char* std_site_reporter_s = R"JSON(
{
    "/controls/ess": {
        "SiteControllerHeartbeat"  : 0,
        "ClearFaults"              : 0,
        "BMSDCContactors"          : 0,
        "StartStop"                : 0,
        "GridMode"                 : 0,
        "ActivePowerCmd"           : 0,
        "ReactivePowerCmd"         : 0,
        "ActivePowerRampRate"      : 0,
        "RectivePowerRampRate"     : 0,
        "OffGridFrequencySetpoint" : 0,
        "OffGridVoltageSetpoint"   : 0
    },

    "/controls/site": {
        "site_controller_heartbeat"   : {"value": 0, "enable": "/config/ess:enable"             , "actions": {"onSet": [{"remap":[{"ifChanged": false, "uri": "/controls/ess:SiteControllerHeartbeat"  }]}]}},
        "clear_faults"                : {"value": 0, "enable": "/config/ess:site_control_enable", "actions": {"onSet": [{"remap":[{"ifChanged": false, "uri": "/controls/ess:ClearFaults"              }]}]}},
        "bms_dc_contactor_control"    : {"value": 0, "enable": "/config/ess:site_control_enable", "actions": {"onSet": [{"remap":[{"ifChanged": false, "uri": "/controls/ess:BMSDCContactors"          }]}]}},
        "start_stop_standby_command"  : {"value": 0, "enable": "/config/ess:site_control_enable", "actions": {"onSet": [{"remap":[{"ifChanged": false, "uri": "/controls/ess:StartStop"                }]}]}},
        "on_off_grid_mode_setting"    : {"value": 0, "enable": "/config/ess:site_control_enable", "actions": {"onSet": [{"remap":[{"ifChanged": false, "uri": "/controls/ess:GridMode"                 }]}]}},
        "active_power_setpoint"       : {"value": 0, "enable": "/config/ess:site_control_enable", "actions": {"onSet": [{"remap":[{"ifChanged": false, "uri": "/controls/ess:ActivePowerCmd"           }]}]}},
        "reactive_power_setpoint"     : {"value": 0, "enable": "/config/ess:site_control_enable", "actions": {"onSet": [{"remap":[{"ifChanged": false, "uri": "/controls/ess:ReactivePowerCmd"         }]}]}},
        "active_power_ramp_rate"      : {"value": 0, "enable": "/config/ess:site_control_enable", "actions": {"onSet": [{"remap":[{"ifChanged": false, "uri": "/controls/ess:ActivePowerRampRate"      }]}]}},
        "reactive_power_ramp_rate"    : {"value": 0, "enable": "/config/ess:site_control_enable", "actions": {"onSet": [{"remap":[{"ifChanged": false, "uri": "/controls/ess:ReactivePowerRampRate"    }]}]}},
        "off_grid_frequency_setpoint" : {"value": 0, "enable": "/config/ess:site_control_enable", "actions": {"onSet": [{"remap":[{"ifChanged": false, "uri": "/controls/ess:OffGridFrequencySetpoint" }]}]}},
        "off_grid_voltage_setpoint"   : {"value": 0, "enable": "/config/ess:site_control_enable", "actions": {"onSet": [{"remap":[{"ifChanged": false, "uri": "/controls/ess:OffGridVoltageSetpoint"   }]}]}}
    },

    "/status/site": {
        "site_control_enable"      : false,
        "dc_contactors_closed_bool": false
    },

    "/vlinks/ess": {
        "ess_controller_heartbeat" : {"value": "/status/site:ess_controller_heartbeat" , "vlink": "/status/ess:Heartbeat"           },
        "site_control_enable"      : {"value": "/status/site:site_control_enable"      , "vlink": "/config/ess:site_control_enable" },
        "bess_fault"               : {"value": "/status/site:bess_fault"               , "vlink": "/status/ess:IsFaulted"           },
        "bess_alarm"               : {"value": "/status/site:bess_alarm"               , "vlink": "/status/ess:IsAlarming"          },
        "bess_fire_fault"          : {"value": "/status/site:bess_fire_fault"          , "vlink": "/status/ess:FireFault"           },
        "bess_door_alarm"          : {"value": "/status/site:bess_door_alarm"          , "vlink": "/status/ess:DoorAlarm"           },
        "bess_e_stop_fault"        : {"value": "/status/site:bess_e_stop_fault"        , "vlink": "/status/ess:EStopFault"          },

        "chargeable_power"    : {"value": "/status/site:max_charge_power"    , "vlink": "/limits/ess:MaxChargePower"    },
        "dischargeable_power" : {"value": "/status/site:max_discharge_power" , "vlink": "/limits/ess:MaxDischargePower" },

        "bms_comms_ok"                      : {"value": "/status/site:bms_comms_ok"                      , "vlink": "/status/bms:CommsOK"                       },
        "bms_fault"                         : {"value": "/status/site:bms_fault"                         , "vlink": "/status/bms:IsFaulted"                     },
        "bms_alarm"                         : {"value": "/status/site:bms_alarm"                         , "vlink": "/status/bms:IsAlarming"                    },
        "dc_contactors_closed"              : {"value": "/status/site:dc_contactors_closed"              , "vlink": "/status/bms:DCClosed"                      },
        "battery_soc"                       : {"value": "/status/site:battery_soc"                       , "vlink": "/status/bms:SOC"                           },
        "battery_soh"                       : {"value": "/status/site:battery_soh"                       , "vlink": "/status/bms:SOH"                           },
        "dc_current"                        : {"value": "/status/site:dc_current"                        , "vlink": "/status/bms:DCCurrent"                     },
        "dc_voltage"                        : {"value": "/status/site:dc_voltage"                        , "vlink": "/status/bms:DCVoltage"                     },
        "dc_power"                          : {"value": "/status/site:dc_power"                          , "vlink": "/status/bms:DCPower"                       },
        "max_cell_temp"                     : {"value": "/status/site:max_cell_temp"                     , "vlink": "/status/bms:MaxCellTemp"                   },
        "avg_cell_temp"                     : {"value": "/status/site:avg_cell_temp"                     , "vlink": "/status/bms:AvgCellTemp"                   },
        "min_cell_temp"                     : {"value": "/status/site:min_cell_temp"                     , "vlink": "/status/bms:MinCellTemp"                   },
        "cell_temp_delta"                   : {"value": "/status/site:cell_temp_delta"                   , "vlink": "/status/bms:CellTempDelta"                 },
        "max_cell_voltage"                  : {"value": "/status/site:max_cell_voltage"                  , "vlink": "/status/bms:MaxCellVoltage"                },
        "avg_cell_voltage"                  : {"value": "/status/site:avg_cell_voltage"                  , "vlink": "/status/bms:AvgCellVoltage"                },
        "min_cell_voltage"                  : {"value": "/status/site:min_cell_voltage"                  , "vlink": "/status/bms:MinCellVoltage"                },
        "cell_voltage_delta"                : {"value": "/status/site:cell_voltage_delta"                , "vlink": "/status/bms:CellVoltageDelta"              },
        "racks_in_service"                  : {"value": "/status/site:racks_in_service"                  , "vlink": "/status/bms:RacksInService"                },
        "racks_total"                       : {"value": "/status/site:racks_total"                       , "vlink": "/status/bms:RacksTotal"                    },
        "bms_dc_contactor_control_feedback" : {"value": "/status/site:bms_dc_contactor_control_feedback" , "vlink": "/status/bms:BMSDCContactorControlFeedback" },
        "chargeable_energy"                 : {"value": "/status/site:chargeable_energy"                 , "vlink": "/status/bms:ChargeableEnergy"              },
        "dischargeable_energy"              : {"value": "/status/site:dischargeable_energy"              , "vlink": "/status/bms:DischargeableEnergy"           },

        "pcs_comms_ok"                               : {"value": "/status/site:pcs_comms_ok"                               , "vlink": "/status/pcs:CommsOK"                               },
        "pcs_fault"                                  : {"value": "/status/site:pcs_fault"                                  , "vlink": "/status/pcs:IsFaulted"                             },
        "pcs_alarm"                                  : {"value": "/status/site:pcs_alarm"                                  , "vlink": "/status/pcs:IsAlarming"                            },
        "system_state"                               : {"value": "/status/site:system_state"                               , "vlink": "/status/pcs:SystemState"                           },
        "l1_l2_voltage"                              : {"value": "/status/site:l1_l2_voltage"                              , "vlink": "/status/pcs:L1L2Voltage"                           },
        "l2_l3_voltage"                              : {"value": "/status/site:l2_l3_voltage"                              , "vlink": "/status/pcs:L2L3Voltage"                           },
        "l3_l1_voltage"                              : {"value": "/status/site:l3_l1_voltage"                              , "vlink": "/status/pcs:L3L1Voltage"                           },
        "l1_current"                                 : {"value": "/status/site:l1_current"                                 , "vlink": "/status/pcs:L1Current"                             },
        "l2_current"                                 : {"value": "/status/site:l2_current"                                 , "vlink": "/status/pcs:L2Current"                             },
        "l3_current"                                 : {"value": "/status/site:l3_current"                                 , "vlink": "/status/pcs:L3Current"                             },
        "power_factor"                               : {"value": "/status/site:power_factor"                               , "vlink": "/status/pcs:PowerFactor"                           },
        "frequency"                                  : {"value": "/status/site:frequency"                                  , "vlink": "/status/pcs:Frequency"                             },
        "active_power"                               : {"value": "/status/site:active_power"                               , "vlink": "/status/pcs:ActivePower"                           },
        "reactive_power"                             : {"value": "/status/site:reactive_power"                             , "vlink": "/status/pcs:ReactivePower"                         },
        "apparent_power"                             : {"value": "/status/site:apparent_power"                             , "vlink": "/status/pcs:ApparentPower"                         },
        "pcs_dc_voltage"                             : {"value": "/status/site:pcs_dc_voltage"                             , "vlink": "/status/pcs:PCSDCVoltage"                          },
        "pcs_dc_current"                             : {"value": "/status/site:pcs_dc_current"                             , "vlink": "/status/pcs:PCSDCCurrent"                          },
        "pcs_dc_power"                               : {"value": "/status/site:pcs_dc_power"                               , "vlink": "/status/pcs:PCSDCPower"                            },
        "max_igbt_temperature"                       : {"value": "/status/site:max_igbt_temperature"                       , "vlink": "/status/pcs:MaxIGBTTemperature"                    },
        "grid_mode"                                  : {"value": "/status/site:grid_mode"                                  , "vlink": "/status/pcs:GridMode"                              },
        "modules_online"                             : {"value": "/status/site:modules_online"                             , "vlink": "/status/pcs:ModulesOnline"                         },
        "modules_available"                          : {"value": "/status/site:modules_available"                          , "vlink": "/status/pcs:ModulesAvailable"                      },
        "active_power_setpoint_feedback"             : {"value": "/status/site:active_power_setpoint_feedback"             , "vlink": "/status/pcs:ActivePowerSetpointFeedback"           },
        "reactive_power_setpoint_feedback"           : {"value": "/status/site:reactive_power_setpoint_feedback"           , "vlink": "/status/pcs:ReactivePowerSetpointFeedback"         },
        "active_power_ramp_rate_setpoint_feedback"   : {"value": "/status/site:active_power_ramp_rate_setpoint_feedback"   , "vlink": "/status/pcs:ActivePowerRampRateSetpointFeedback"   },
        "reactive_power_ramp_rate_setpoint_feedback" : {"value": "/status/site:reactive_power_ramp_rate_setpoint_feedback" , "vlink": "/status/pcs:ReactivePowerRampRateSetpointFeedback" },
        "off_grid_frequency_setpoint_feedback"       : {"value": "/status/site:off_grid_frequency_setpoint_feedback"       , "vlink": "/status/pcs:OffGridFrequencySetpointFeedback"      },
        "off_grid_voltage_setpoint_feedback"         : {"value": "/status/site:off_grid_voltage_setpoint_feedback"         , "vlink": "/status/pcs:OffGridVoltageSetpointFeedback"        }
    },

    "/alarms/site": {
        "fg_bess_alarms"          : 0,
        "fg_bms_alarms_1"         : 0,
        "fg_bms_alarms_2"         : 0,
        "fg_pcs_alarms_1"         : 0,
        "fg_pcs_alarms_2"         : 0,
        "fg_bms_control_alarms_1" : 0,
        "fg_bms_control_alarms_2" : 0,
        "fg_pcs_control_alarms_1" : 0,
        "fg_pcs_control_alarms_2" : 0
    },

    "/faults/site": {
        "fg_bess_comms_faults" : 0,
        "fg_bess_faults"       : 0,
        "fg_bms_faults_1"      : 0,
        "fg_bms_faults_2"      : 0,
        "fg_pcs_faults_1"      : 0,
        "fg_pcs_faults_2"      : 0
    }
}
)JSON";
