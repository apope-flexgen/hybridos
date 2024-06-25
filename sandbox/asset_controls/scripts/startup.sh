cfg_dir=/home/docker/hybridos/sandbox/asset_controls/config/BRBinPSM/ess_controller
ui_dir=$cfg_dir
psm_dir=/home/docker/hybridos/sandbox/asset_controls/config/BRBinPSM/psm

scripts_dir=/home/docker/hybridos/sandbox/asset_controls/scripts
bin_dir=/usr/local/bin

runESS=true
runUI=false
runPSM=true
runPSMUI=false
runMecho=true
dbiOnly=false

# pkill fims_server
pkill -f web_server
# pkill dbi
pkill -f echo
pkill -f psm
pkill -f twins
pkill -f ess_controller
pkill go_metrics

if [ $# -gt 0 ]
then
    exit
fi

sleep 1s

if ! pgrep -af mongod
then
    echo "launching mongodb..."
    mongod --config /etc/mongod.conf &
    sleep 1s
    # Load a permanent, default user to the database
    mongo hybridos_authentication $scripts_dir/mongo_users.js
    dbi &
fi

if ! pgrep -af dbi
then
    echo "Starting DBI"
    dbi &
fi

if [ "$runUI" = true ]; then
    $scripts_dir/dbi.sh $ui_dir web_ui
    # Check the exit status of the function
    if [ $? -ne 0 ]; then
        echo "UI DBI Load failed, exiting"
        exit 1
    fi
fi

if [ "$runESS" = true ]; then
    $scripts_dir/dbi.sh $cfg_dir ess_controller
    if [ $? -ne 0 ]; then
        echo "ESS Controller DBI Load failed, exiting"
        exit 1
    fi
fi

if [ "$runPSM" = true ]; then
    $scripts_dir/dbi.sh $psm_dir psm
    if [ $? -ne 0 ]; then
        echo "PSM DBI Load failed, exiting"
        exit 1
    fi
fi

if [ "$runPSMUI" = true ]; then
    $scripts_dir/dbi.sh $psm_dir web_ui
    if [ $? -ne 0 ]; then
        echo "PSM UI DBI Load failed, exiting"
        exit 1
    fi
fi

if [ "$runUI" = true ]; then
    echo "Starting ESSC UI"
    $bin_dir/web_server $bin_dir/web_ui $ui_dir/web_ui $ui_dir/web_server &
fi

if [ "$runESS" = true ]; then
    cp $cfg_dir/ess_controller/*_file.json /usr/local/etc/config/ess_controller
    echo "Starting ESSC"
    $bin_dir/ess_controller -f ess_file&
fi

if [ "$runPSM" = true ]; then
    echo "Starting PSM"
    $bin_dir/psm&
fi

if [ "$runPSMUI" = true ]; then
    echo "Starting PSM UI"
    $bin_dir/web_server $bin_dir/web_ui $psm_dir/web_ui $psm_dir/web_server &
fi

if [ "$runMecho" = true ]; then
    go_metrics $psm_dir/go_metrics/bms_1_ess-controller_pge_modbus_server_go_metrics.json&
    go_metrics $psm_dir/go_metrics/pcs_ess-controller_pge_modbus_server_go_metrics.json&
    # go_metrics $psm_dir/go_metrics/bms_1_catl_enerc_plus_metrics.json&
    # go_metrics $psm_dir/go_metrics/bms_2_catl_enerc_plus_metrics.json&
    # $bin_dir/echo $psm_dir/echo/ess_1_bms_modbus_echo.json&
    # $bin_dir/echo $psm_dir/echo/ess_1_pcs_modbus_echo.json&
fi