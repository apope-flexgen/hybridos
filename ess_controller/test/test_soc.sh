#!/bin/sh
#variables are voltage, current, power, soc, num_batteries, num_per_rack, num_per_mbmu

num_batteries=50
num_per_rack=8
num_per_mbmu=9
full_soc=3528000
time=1

#current=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/status/bms/sbmu_7/BMScurrent`
#read_soc=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/components/catl_mbmu_summary_r/mbmu_soc 42`
#time_dbl=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/status/ess/tNow`
#echo $read_soc

#if [ "$read_soc" = "0" ]; then
#    write_soc=425
#    /usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/catl_mbmu_summary_r/mbmu_soc $write_soc
#fi

read_soc=89
voltage=35
read_total_power=$(($read_soc * $full_soc))
echo $read_total_power
read_current=$(($read_total_power / $num_batteries / $num_per_rack / $num_per_mbmu / $voltage / $time / 10))
echo $read_current

#voltage=$(($voltage * 10))
#current=250
power=$(($voltage * $read_current * $time))

total_power=$(($power * $num_batteries * $num_per_rack * $num_per_mbmu))
echo $total_power
total_power=$(($total_power * 10))
soc=$(($total_power / $full_soc))
echo $soc | bc


