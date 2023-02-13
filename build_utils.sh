#!/bin/bash

name=hybridos

components=(fims ess_controller site_controller fleet_manager modbus_interface dnp3_interface washer dbi scheduler cops ftd cloud_sync dts twins echo overwatch events metrics web_server web_ui)
meta=(ess_controller_meta site_controller_meta fleet_manager_meta twins_meta)

ess_controller_meta=(fims ess_controller modbus_interface dnp3_interface dbi ftd cloud_sync overwatch events metrics web_server web_ui)
site_controller_meta=(fims site_controller modbus_interface dnp3_interface dbi scheduler cops ftd cloud_sync dts overwatch events metrics web_server web_ui)
fleet_manager_meta=(fims fleet_manager modbus_interface dnp3_interface dbi scheduler cops ftd cloud_sync dts overwatch events metrics web_server web_ui)
twins_meta=(fims modbus_interface dnp3_interface dbi twins echo overwatch)
