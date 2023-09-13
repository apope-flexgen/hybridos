# this script runs the ess_controller and loads configs the new way

cfg_dir=/home/docker/local/config_c2c
# cfg_dir=config_ess
# cfg_dir=integration_dev/config/ess_controller/config
scripts_dir=local
bin_dir=/usr/local/bin
twins_dir=integration_dev/config/twins/config
pkill fims_server
pkill web_server
pkill dbi
# pkill metrics

if [ $# -gt 0 ]
then
    exit
fi

sleep 1s

fims_server &

if ! pgrep -af mongod
then
    echo "launching mongodb..."
    mongod --config /etc/mongod.conf &
    sleep 1s
    # Load a permanent, default user to the database
    mongo hybridos_authentication $scripts_dir/mongo_users.js

    dbi &
    $scripts_dir/dbi.sh $cfg_dir
fi

if ! pgrep -af dbi
then
    echo "Starting DBI"
    dbi &
fi
$scripts_dir/dbi.sh $cfg_dir

# $bin_dir/echo $cfg_dir/echo/sce_catl_bms_1_modbus_echo.json &
# $bin_dir/echo $cfg_dir/echo/sce_catl_bms_2_modbus_echo.json &
# $bin_dir/echo $cfg_dir/echo/sce_pe_pcs_modbus_echo.json &

# $bin_dir/twins $cfg_dir/twins/ &

# $bin_dir/web_server $bin_dir/web_ui $cfg_dir/web_ui $cfg_dir/web_server &

# cp $cfg_dir/ess_controller/*_file.json /usr/local/etc/config/ess_controller
# file_names=`ls -1 "$cfg_dir/ess_controller" | grep _file.json | sed -e 's/\.json$//'`
# num_files=`ls -1 "$cfg_dir/ess_controller" | grep _file.json | wc -l`
# count=0
# for f in $file_names
# do
#     ((count+=1))
#     if (( $count==$num_files ))
#     then
#         echo -e "launching last ess $f with configs..."
#         $bin_dir/ess_controller -f $f
#     else
#         echo -e "launching $f with configs..."
#         $bin_dir/ess_controller -f $f &
#     fi
# done

cp $cfg_dir/ess_controller/ess_file.json /usr/local/etc/config/ess_controller
$bin_dir/ess_controller -f ess_file &
# $bin_dir/metrics $cfg_dir/metrics/metrics.json &
# $bin_dir/metrics $cfg_dir/metrics/metrics_northfork.json &
# $bin_dir/metrics $cfg_dir/metrics/metrics_magnolia.json &

# $bin_dir/echo $cfg_dir/echo/pe_pcs_ui_echo.json &
# $bin_dir/echo $cfg_dir/echo/catl_bms_ui_echo.json &
# $bin_dir/echo $cfg_dir/echo/dcdc_ui_echo.json &
# $bin_dir/echo $cfg_dir/echo/pv_ui_echo.json &
# $bin_dir/echo $cfg_dir/echo/echo_all.json &