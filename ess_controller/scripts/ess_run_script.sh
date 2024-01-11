#!/bin/bash
# script to run ess_controller 
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
    source="none"
    echo -e "no interface source defined."
    #exit 1
fi

if [ ! -d "$config_dir" ]; then 
  echo -e "config directory: $config_dir"; 
  echo -e "config directory not found making it."; 
  sudo mkdir -p $config_dir; 
fi

echo -e "##### HYBRIDOS RUN #####"
echo -e "config directory: $(readlink --canonicalize $config_dir)"
echo -e "component source: $source"
echo -e "starting system..."
 
# LEVEL 1: start influxdb, mongodb, fims_server
sleep 3s; sudo influxd 2> /dev/null &
sleep 3s; sudo mongod --config /etc/mongod.conf
sleep 3s; $bin_dir/fims/fims_server &

# check for config/ess_controller
if [ -d "~/dev/ess_controller/configs/ess_controller" ]; then
    echo -e "##### setting up config dir from dev #####"
    sudo cp -a ~/dev/ess_controller/configs/ess_controller $config_dir
fi

# LEVEL 2: start storage, metrics, events
if [ -d "$config_dir/storage" ]; then
    echo -e "##### starting storage #####"
    sleep 3s; $bin_dir/mcp/FlexGenMCP $bin_dir/mcp/config/mcp_storage.json &
fi
if [ -d "$config_dir/metrics" ]; then
    echo -e "##### starting metrics #####"
    sleep 3s; $bin_dir/mcp/FlexGenMCP $bin_dir/mcp/config/mcp_metrics.json &
fi
if [ -d "$config_dir/events" ]; then 
    echo -e "##### starting events #####"
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
if [ -d "$config_dir/ess_controller/run" ]; then
    sleep 3s; $bin_dir/mcp/FlexGenMCP $bin_dir/mcp/config/mcp_hybridos_controller.json &
else
    if [ -f "~/dev/ess_controller/build/release/ess_controller" ]; then
        echo -e "##### running ess_controller  #####"
        ~/dev/ess_controller/build/release/ess_controller $config_dir
    fi
fi
if [ -d "$config_dir/modbus_server/run" ]; then
    sleep 3s; sudo $bin_dir/mcp/FlexGenMCP $bin_dir/mcp/config/mcp_modbus_server.json &
fi

# LEVEL 4: start web_server, web_ui
if [ -d "$config_dir/web_ui" ]; then
    sleep 3s; sudo $bin_dir/web_server/web_server $bin_dir/web_ui/ $config_dir/web_ui $config_dir/web_server &
fi

