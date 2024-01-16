HBVAL=1
# continue until $n equals 5
while [ $HBVAL -le 60 ]
do
	HBVAL=$(( HBVAL+1 ))	 # increments $n
	/usr/local/bin/fims/fims_send -m set -u /components/catl_ems_bms_rw -r/$$ "{\"ems_heartbeat\":{\"value\":$HBVAL}}"
	/usr/local/bin/fims/fims_send -m set -u /components/catl_ems_bms_rw -r/$$ "{\"ems_cmd\":{\"value\":3}}"
	sleep 2
	#/usr/local/bin/fims/fims_send -m set -u /components/catl_ems_bms_rw -r/$$ "{\"ems_cmd\":{\"value\":0}}"
        sleep 2
done

