#! /bin/sh

# query with  /usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/assets/bms/summary/alarms | jq
# set with  /usr/local/bin/fims/fims_send -m pub -u /components/catl_mbmu_sum_r '{"mbmu_warning_11": 1234}'
# and this .. fims_send -m set -r /$$ -u /ess/warn/bms/alarms '"Clear"'
/usr/local/bin/fims/fims_send -m set  -u "/ess/assets/bms/summary/clear_faults" '"Clear"'
/usr/local/bin/fims/fims_send -m set  -u "/ess/assets/bms/summary/clear_faults" '"Clear"'


echo " First Check alarms" 
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/assets/bms/summary/alarms | jq

sleep 5

echo " Now Set some Alarms in mbmu_warning_11"
/usr/local/bin/fims/fims_send -m pub -u /components/catl_mbmu_sum_r '{"mbmu_warning_11": 0000}'
/usr/local/bin/fims/fims_send -m pub -u /components/catl_mbmu_sum_r '{"mbmu_warning_11": 1234}'


sleep 2
echo " Check alarms again" 
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/assets/bms/summary/alarms | jq


sleep 5

echo " Now Clear Alarms"
/usr/local/bin/fims/fims_send -m set -r /$$ -u "/ess/assets/bms/summary/clear_faults" '"Clear"'

sleep 2
echo " Check alarms again" 
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/assets/bms/summary/alarms | jq


sleep 2

echo " Here is the alarm input config" 
cat configs/bms_catl_template.json  | grep -A 33  mbmu_warning_11


echo " Here is the UI config " 
cat configs/bms_manager.json  | grep -A 20  clear_faults

