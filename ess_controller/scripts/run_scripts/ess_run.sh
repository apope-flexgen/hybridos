#!/bin/bash

bin_dir=/usr/local/bin
config_dir=/usr/local/etc/config

./ess_stop.sh # shutdown any running processes

# determine component source - modbus, twins, or both
if [ $# = 0 ] || [ "$1" = "--modbus" ]; then # default option
    source="modbus"
elif [ "$1" = "--twins" ]; then
    source="twins"
elif [ "$1" = "--both" ]; then
    source="both"
else
    echo -e "invalid source option."
    exit 1
fi

if [ ! -d "$config_dir" ]; then echo -e "config directory: $config_dir"; echo -e "config directory not found."; exit 1; fi

echo -e "##### ESS RUN #####"
echo -e "config directory: $(readlink --canonicalize $config_dir)"
echo -e "component source: $source"
echo -e "starting system..."

# LEVEL 1: start influxdb, mongodb, fims_server
sleep 3s; sudo influxd 2> /dev/null &
sleep 3s; sudo mongod --config /etc/mongod.conf
sleep 3s; $bin_dir/fims/fims_server &

# LEVEL 2: start storage, metrics, events
if [ -d "$config_dir/storage" ]; then
    sleep 3s; $bin_dir/mcp/FlexGenMCP $bin_dir/mcp/config/mcp_storage.json &
fi
if [ -d "$config_dir/metrics" ]; then
    sleep 3s; $bin_dir/mcp/FlexGenMCP $bin_dir/mcp/config/mcp_metrics.json &
fi
if [ -d "$config_dir/events" ]; then
    sleep 3s; $bin_dir/mcp/FlexGenMCP $bin_dir/mcp/config/mcp_events.json &
fi

# LEVEL 3: start modbus clients, hybridos_controller, modbus_servers
case $source in
    modbus )
        sleep 3s; $bin_dir/mcp/FlexGenMCP $bin_dir/mcp/config/mcp_modbus_client.json &
        ;;
    twins )
        sleep 3s; $bin_dir/twins $config_dir/ess_controller &
        ;;
    both )
        sleep 3s; $bin_dir/mcp/FlexGenMCP $bin_dir/mcp/config/mcp_modbus_client.json &
        sleep 3s; $bin_dir/twins $config_dir/ess_controller &
        ;;
    * )
        ;;
esac
if [ -d "$config_dir/ess_controller" ]; then
    sleep 3s; $bin_dir/mcp/FlexGenMCP $bin_dir/mcp/config/mcp_ess_controller.json &
fi
if [ -d "$config_dir/modbus_server" ]; then
    sleep 3s; sudo $bin_dir/mcp/FlexGenMCP $bin_dir/mcp/config/mcp_modbus_server.json &
fi
if [ -d "$config_dir/modbus_client" ]; then
    sleep 3s; sudo $bin_dir/mcp/FlexGenMCP $bin_dir/mcp/config/mcp_client_server.json &
fi

# LEVEL 4: start web_server, web_ui
if [ -d "$config_dir/web_ui" ]; then
    sleep 3s; sudo $bin_dir/web_server/web_server $bin_dir/web_ui/ $config_dir/web_ui $config_dir/web_server &
fi
