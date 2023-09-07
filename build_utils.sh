#!/bin/bash

name=hybridos

components=(fims fims_relay ess_controller site_controller fleet_manager modbus_interface dnp3_interface washer dbi scheduler cops ftd cloud_sync dts twins echo overwatch events metrics go_metrics web_apps)
meta=(ess_controller_meta site_controller_meta fleet_manager_meta twins_meta)

ess_controller_meta=(fims fims_relay ess_controller modbus_interface dnp3_interface dbi ftd cloud_sync overwatch events metrics go_metrics web_apps)
site_controller_meta=(fims fims_relay site_controller modbus_interface dnp3_interface dbi scheduler cops ftd cloud_sync dts overwatch events metrics go_metrics web_apps)
fleet_manager_meta=(fims fims_relay fleet_manager modbus_interface dnp3_interface washer dbi scheduler cops ftd cloud_sync dts echo overwatch events metrics go_metrics web_apps)
twins_meta=(fims fims_relay ess_controller modbus_interface dnp3_interface dbi ftd cloud_sync twins echo overwatch go_metrics)
