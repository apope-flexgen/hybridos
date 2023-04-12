#!/bin/bash
#
#   USAGE: capture state of machine imaging after running MaaS and initial Ansible provisioning
#
#   SYNTAX: ./server_status.sh <hostname> <server_type> <status>
#   EX: ./server_status.sh brp-fleet-manager-01 fleet_manager installed (for prod servers)
#       ./server_status.sh brp-zapata1-site-controller-01 site_controller shipped (for servers shipped to prod but not installed on site)
#       ./server_status.sh brp-northfork-ess-controller-01 ess_controller lab (for servers imaged for QA use)

set -xe

hostname=$1
service_name=$2
status=$3

if [ -z $hostname ]
then
    echo "Server type not defined!"
    exit 1
else
    echo "Hostname is $hostname"
fi
if [ -z $service_name ]
then 
    echo "Server type not defined!"
    exit 1
else
    echo "Server type is $service_name"
fi

if [ -z $status ]
then
    echo "Status not defined!"
    exit 1
else
    echo "Status is $status"
fi

# site_name=$(echo $hostname | cut -d'-' -f 2)
middle="-"
output_file_suffix=".log"
output_file=/home/hybridos/$hostname$middle$status$output_file_suffix
touch $output_file

echo "HOSTNAME" > $output_file
echo $(hostname) >> $output_file
echo >> $output_file

echo "DATE AND TIME" >> $output_file
echo "$(date)" >> $output_file
echo "$(timedatectl | grep "Time zone")" >> $output_file
echo >> $output_file

echo "OS VERSION INFO" >> $output_file
echo "$(cat /etc/os-release | grep PRETTY_NAME | cut -d'=' -f2)" >> $output_file
echo "Kernel version: $(uname -r)" >> $output_file
echo >> $output_file

echo "SELINUX MODE" >> $output_file
echo "$(getenforce)" >> $output_file
echo >> $output_file

echo "DRIVE PARTITIONING" >> $output_file
echo "df -h output:" >> $output_file
echo "$(df -h)" >> $output_file
echo "lsblk output:" >> $output_file
echo "$(lsblk)" >> $output_file
echo >> $output_file

echo "NETWORK CONFIGURATION" >> $output_file
echo "ip address output:" >> $output_file
echo "$(ip a)" >> $output_file
echo "ip route output:" >> $output_file
echo "$(ip r)" >> $output_file
echo "route -n output:" >> $output_file
echo "$(route -n)" >> $output_file
echo >> $output_file

echo "FIREWALL PORT CONFIGURATION" >> $output_file
echo "$(firewall-cmd --list-all)" >> $output_file
echo >> $output_file

hybridos_meta=$(rpm -qa ${service_name}_meta)
hybridos_pm=$(rpm -qa ${service_name}_pm)
component_list=(fims hybridos_controller mcp ess_controller site_controller fleet_manager modbus_interface dnp3_interface washer dbi scheduler cops ftd cloud_sync dts twins echo overwatch storage influxdb_setup events metrics web_server web_ui)
third_party_list=(cjson grafana influxdb libmodbus mongodb-org mongodb-org-mongos mongodb-org-server mongodb-org-shell mongodb-org-tools opendnp3 parallel_hashmap simdjson spdlog spscqueue telegraf tscns)

echo "FLEXGEN SOFTWARE VERSION INSTALLED" >> $output_file
if [ -z $hybridos_meta ] # if a meta package is not installed, then check for pm
then
    if [ -z $hybridos_pm ]
    then
        echo $hybridos_pm >> $output_file
    fi
    echo "$(rpm -qa ${component_list[@]})" >> $output_file
else
    echo $hybridos_meta >> $output_file
fi
echo >> $output_file
echo "THIRD PARTY SOFTWARE" >> $output_file
echo "$(rpm -qa ${third_party_list[@]})" >> $output_file
echo >> $output_file

echo "PACKAGE REPOS" >> $output_file
echo "$(yum repolist)" >> $output_file
echo >> $output_file

echo "CONFIG ARTIFACTS" >> $output_file
echo "$(tree /home/hybridos/config)" >> $output_file
echo >> $output_file

echo "TONY'S FANCY COMMAND" >> $output_file
echo "$(systemctl | grep -E 'fims|influxdb|mongod|grafana|dbi|fleet_manager|_controller|scheduler|cops|events|web_server|ftd|cloud_sync|dts|storage|overwatch|gpio|modbus|dnp3|metrics|echo|twins|UNIT')" >> $output_file
echo >> $output_file

if [ -d "/usr/local/bin/fims" ]; then
    fims_list=$(/usr/local/bin/fims/fims_send -m get -r /$$ -u /dbi/show_collections)
else
    fims_list=$(/usr/local/bin/fims_send -m get -r /$$ -u /dbi/show_collections)
fi

if rpm -qa | grep -q "dbi"; then
    echo "dbi RPM is installed"
    echo "DBI DATA FROM FIMS" >> $output_file
    echo $fims_list | sed 's/"//g;s/[][]//g' | tr "," "\n" | while read LINE
    do
        tmp=$(/usr/local/bin/fims_send -m get -r /$$ -u /dbi/$LINE/show_documents)
        echo "$LINE : $tmp" >> $output_file
    done
    echo >> $output_file
else
    echo "dbi RPM is not installed"
fi

echo "INFLUXDB DATABASES" >> $output_file
echo "$(influx -execute 'show databases')" >> $output_file