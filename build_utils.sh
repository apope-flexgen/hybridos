#!/bin/bash

name=hybridos

components=(fims fims_relay ess_controller site_controller fleet_manager modbus_interface dnp3_interface washer dbi scheduler cops ftd cloud_sync dts psm echo overwatch events metrics go_metrics web_apps)
meta=(ess_controller_meta site_controller_meta fleet_manager_meta psm_meta)

ess_controller_meta=(fims ess_controller modbus_interface dnp3_interface dbi ftd cloud_sync overwatch events metrics go_metrics web_apps)
site_controller_meta=(fims site_controller modbus_interface dnp3_interface dbi scheduler cops ftd cloud_sync dts overwatch events metrics go_metrics web_apps)
fleet_manager_meta=(fims fleet_manager modbus_interface dnp3_interface washer dbi scheduler cops ftd cloud_sync dts echo overwatch events metrics go_metrics web_apps)
psm_meta=(fims ess_controller modbus_interface dnp3_interface dbi ftd cloud_sync psm echo overwatch go_metrics)
