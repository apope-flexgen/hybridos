# DNP3 and Modbus Telemetry Scripts

## Overview
There are Python scripts that can be used to check telemetry for the DNP3 client/server and the modbus client. Specifically, the scripts can report values from all registers defined in the DNP3 client/server and modbus client configuration files.

## Running the Script
The telemetry scripts can be found in `/home/scripts/utility` in the Docker containers. On the command line:
* To check DNP3 client telemetry, run `/home/scripts/utility/check_dnp3_client_telemetry.py /path/to/dnp3_client.json`
* To check DNP3 server telemetry, run `/home/scripts/utility/check_dnp3_server_telemetry.py /path/to/dnp3_server.json`
* To check modbus client telemetry, run `/home/scripts/utility/check_modbus_client_telemetry.py /path/to/modbus_client.json`