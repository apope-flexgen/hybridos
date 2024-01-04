# capture hybridos modules list
hybridos=(
    'influx'
    'mongo'
    'fims'
    'site_controller'
    'fleet_manager'
    'dnp3_interface'
    'modbus_interface'
    'mcp'
    'metrics'
    'events'
    'dbi'
    'web_apps'
    'cops'
    'scheduler'
    'ftd'
    'cloud_sync'
    'dts'
    'twins'
    'echo'
    'config'
    'config_fleetman'
    'config_powercloud'
    'scripts'
    'washer'
    'hybridos'
)

printf "%-24s" "SERVICE"
printf "%-16s" "PID"
printf "%-24s\n" "PROCESS NAME"
echo "------------------------------------------------------------"
# traverse through hybridos modules
for i in "${hybridos[@]}"; do
    case $i in
    config | scripts | modbus_interface | dnp3_interface ) # ignored repositories
        ;;
    # web ui doesn't get its own process, so search the web_server arguments
    web_ui )
        printf "%-24s" "$i:"
        pid=$(pgrep -anf "$i" | awk '{print $1}')
        pname=$(pgrep -anf "$i" | awk '{print $2}')
        if [ -z "$pid" ]
        then
            printf "%-40s\n" ""
        else
            printf "%-16s" "$pid"
            printf "%-24s\n" "$pname"
        fi
        ;;
    * )
        printf "%-24s" "$i:"
        pid=$(pgrep -an "$i" | awk '{print $1}')
        pname=$(pgrep -an "$i" | awk '{print $2}')
        if [ -z "$pid" ]
        then
            printf "%-40s\n" ""
        else
            printf "%-16s" "$pid"
            printf "%-24s\n" "$pname"
        fi
        ;;
    esac
done

printf "%-24s" "dnp3:"
IFS=$'\n'
dnp3_list=( $(pgrep -af dnp3) )
for i in "${dnp3_list[@]}"; do
    pid=$(echo $i | awk '{print $1}')
    # to distinguish between processes, get the command line argument rather than the process name
    pname=$(echo $i | awk '{print $3}')
    if [ -z "$pid" ]
    then
        printf "%-40s\n" ""
    else
        printf "%-16s" "$pid"
        printf "%-24s\n" "$pname"
    fi
    if [ $i != ${dnp3_list[-1]} ];
    then
        printf "%-24s" ""
    fi
done
# newline if empty
if [ ${#dnp3_list[@]} -eq 0 ];
then
    printf "\n"
fi


printf "%-24s" "modbus:"
IFS=$'\n'
modbus_list=( $(pgrep -af modbus) )
for i in "${modbus_list[@]}"; do
    pid=$(echo $i | awk '{print $1}')
    # to distinguish between processes, get the command line argument rather than the process name
    pname=$(echo $i | awk '{print $3}')
    if [ -z "$pid" ]
    then
        printf "%-40s\n" ""
    else
        printf "%-16s" "$pid"
        printf "%-24s\n" "$pname"
    fi
    if [ $i != ${modbus_list[-1]} ];
    then
        printf "%-24s" ""
    fi
done
# newline if empty
if [ ${#modbus_list[@]} -eq 0 ];
then
    printf "\n"
fi
