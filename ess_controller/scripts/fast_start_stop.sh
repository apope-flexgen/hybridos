max_val=1300

for (( val=1200; val<=$(( $max_val -1 )); val++ ))
do


/usr/local/bin/fims/fims_send -m set -u /components/clou_ess_1_high_speed_hr "{\"start_stop\":{\"value\":$val}}"
#val=`expr $val + 1`

done
