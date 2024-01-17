cfg_dir=/home/docker/hybridos/sandbox/asset_controls/config/functional_ess_psm/ess-controller/config
psm_dir=/home/docker/hybridos/sandbox/asset_controls/config/functional_ess_psm/psm
# cfg_dir=config_psm
scripts_dir=/home/docker/hybridos/sandbox/asset_controls/scripts
bin_dir=/usr/local/bin
# psm_dir=integration_dev/config/psm/config

# pkill fims_server
pkill -f web_server
# pkill dbi
pkill -f echo
pkill -f psm
pkill -f ess_controller
# pkill metrics

if [ $# -gt 0 ]
then
    exit
fi

sleep 1s

# fims_server &

if ! pgrep -af mongod
then
    echo "launching mongodb..."
    mongod --config /etc/mongod.conf &
    sleep 1s
    # Load a permanent, default user to the database
    mongo hybridos_authentication $scripts_dir/mongo_users.js

    dbi &
    # $scripts_dir/dbi_old.sh $cfg_dir
fi

if ! pgrep -af dbi
then
    echo "Starting DBI"
    dbi &
fi
$scripts_dir/dbi.sh $cfg_dir ess_controller
$scripts_dir/dbi.sh $cfg_dir web_ui
# $scripts_dir/dbi.sh $psm_dir psm

$bin_dir/psm $psm_dir/psm/psm.json &

$bin_dir/echo $psm_dir/echo/ess_1_bms_modbus_echo.json &
# $bin_dir/echo $psm_dir/echo/ess_1_pcs_modbus_echo.json &
$bin_dir/go_metrics $psm_dir/echo/ess_1_pcs_modbus_echo.json &


$bin_dir/web_server $bin_dir/web_ui $cfg_dir/web_ui $cfg_dir/web_server &

cp $cfg_dir/ess_controller/*_file.json /usr/local/etc/config/ess_controller
file_names=`ls -1 "$cfg_dir/ess_controller" | grep _file.json | sed -e 's/\.json$//'`
num_files=`ls -1 "$cfg_dir/ess_controller" | grep _file.json | wc -l`
count=0
for f in $file_names
do
    ((count+=1))
    if (( $count==$num_files ))
    then
        echo -e "launching last ess $f with configs..."
        $bin_dir/ess_controller -f $f
    else
        echo -e "launching $f with configs..."
        $bin_dir/ess_controller -f $f &
    fi
done

# sleep 20

# $bin_dir/ess_controller -f ess_1_file


# cp $cfg_dir/ess_controller/ess_file.json /usr/local/etc/config/ess_controller
