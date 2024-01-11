#!/bin/sh
#variables are voltage, current, power, soc, num_batteries, num_per_rack, num_per_mbmu
num_batteries=50
num_per_rack=8
num_per_mbmu=9
full_soc=3528000.0
# ​#current=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/status/bms/sbmu_7/BMScurrent`
read_soc=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/components/catl_mbmu_summary_r/mbmu_soc`
read_voltage=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/components/catl_mbmu_summary_r/mbmu_voltage`
read_current=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/components/catl_mbmu_summary_r/mbmu_current`
time_dbl=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/status/ess/tNow`
read_max_mbmu=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/components/catl_mbmu_summary_r/mbmu_max_cell_temperature`
echo "soc = $read_soc"
echo "voltage = $read_voltage"
echo "current = $read_current"
echo "time = $time_dbl"
echo "max cell = $read_max_mbmu"
time_plus=`echo "$time_dbl + 1" | bc`
echo "time_plus = $time_plus"
if [ "$read_soc" = "0" ]; then
    write_soc=42.5 
    /usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/catl_mbmu_summary_r/mbmu_soc $write_soc
    echo "soc = $write_soc"
fi
if [ "$read_voltage" = "0" ]; then
    write_voltage=1324.5 
    /usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/catl_mbmu_summary_r/mbmu_voltage $write_voltage
    echo "voltage = $write_voltage"
fi
if [ "$current" = "0" ]; then
    write_current=280
    /usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/catl_mbmu_summary_r/mbmu_current $write_current
    echo "current = $write_current"
fi
read_soc=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/components/catl_mbmu_summary_r/mbmu_soc`
read_voltage=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/components/catl_mbmu_summary_r/mbmu_voltage`
current=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/components/catl_mbmu_summary_r/mbmu_current`
read_time=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/status/ess/tNow`
echo "read_time = $read_time"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/status/ess/lastNow $read_time
while [ 1 ] ; do
    sleep 2
    read_time=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/status/ess/tNow`
    last_time=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/status/ess/lastNow`
    volts=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/components/catl_mbmu_summary_r/mbmu_voltage`
    current=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/components/catl_mbmu_summary_r/mbmu_current`
    read_soc=42.5
    time_diff=`echo "$read_time - $last_time" | bc -l `
    echo "time_diff = $time_diff"
    delta_soc=`echo "$volts * $current * $time_diff* 100.0/$full_soc" | bc -l `
    new_soc=`echo "$read_soc - ($delta_soc/100.0)" | bc -l`
    echo "time_diff = $time_diff current=$current volts=$volts delta_soc = $delta_soc new_soc = $new_soc"
    /usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/status/ess/lastNow $read_time
    /usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/catl_mbmu_summary_r/mbmu_soc $new_soc
    sleep 2
    read_time=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/status/ess/tNow`
    last_time=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/status/ess/lastNow`
    volts=800
    current=180
    read_soc=50
    time_diff=`echo "$read_time - $last_time" | bc -l `
    echo "time_diff = $time_diff"
    delta_soc=`echo "$volts * $current * $time_diff* 100.0/$full_soc" | bc -l `
    new_soc=`echo "$read_soc - ($delta_soc/100.0)" | bc -l`
    echo "time_diff = $time_diff current=$current volts=$volts delta_soc = $delta_soc new_soc = $new_soc"
    /usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/status/ess/lastNow $read_time
    /usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/catl_mbmu_summary_r/mbmu_soc $new_soc
done