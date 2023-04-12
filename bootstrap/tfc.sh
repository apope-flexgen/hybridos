#!/bin/bash
# Tony Olivo's Fancy Command

watch -n 1 "systemctl list-units | grep -E 'fims|influxdb|mongod|grafana|dbi|fleet_manager|_controller|scheduler|cops|events|web_server|ftd|cloud_sync|dts|storage|overwatch|gpio|modbus|dnp3|metrics|echo|twins|UNIT' | grep -v '\.slice'"
