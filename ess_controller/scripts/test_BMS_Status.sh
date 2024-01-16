#!/bin/sh
# script to cycle the bms_status
#bms_status
#                 "Initial status",
#                 "Normal status",
#                 "Full Charge status",
#                 "Full Discharge status",
#                 "Warning status",
#                 "Fault status" 

#mbmu_status
#		"Initialize",
#               "Normal",
#               "Full Charge",
#               "Full Discharge",
#               "Warning Status",
#               "Fault Status"
#

echo " setting BMS Iniitalize status"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /components/catl_bms_mbmu_status_r/bms_status '"Initial status"'


sleep 1
echo " setting BMS Normal status"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /components/catl_bms_mbmu_status_r/bms_status '"Normal status"'

sleep 1
echo " setting BMS Full Charge status"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /components/catl_bms_mbmu_status_r/bms_status '"Full Charge status"'

sleep 1
echo " setting BMS Full Discharge status"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /components/catl_bms_mbmu_status_r/bms_status '"Full Discharge status"'


sleep 1
echo " setting BMS Warning status"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /components/catl_bms_mbmu_status_r/bms_status '"Warning status"'

sleep 1
echo " setting BMS Fault status"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /components/catl_bms_mbmu_status_r/bms_status '"Fault status"'

sleep 1
echo " setting BMS Normal status again"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /components/catl_bms_mbmu_status_r/bms_status '"Normal status"'

sleep 1
echo " setting MBMU Iniitalize"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /components/catl_mbmu_stat_r/mbmu_status '"Initialize"'

sleep 1
echo " setting MBMU Normal"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /components/catl_mbmu_stat_r/mbmu_status '"Normal"'

sleep 1
echo " setting MBMU Full Charge"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /components/catl_mbmu_status_r/mbmu_status '"Full Charge"'

sleep 1
echo " setting MBMU Full Discharge"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /components/catl_mbmu_stat_r/mbmu_status '"Full Discharge"'


sleep 1
echo " setting MBMU Warning"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /components/catl_mbmu_stat_r/mbmu_status '"Warning"'

sleep 1
echo " setting MBMU Fault"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /components/catl_mbmu_stat_r/mbmu_status '"Fault"'

sleep 1
echo " setting MBMU Normal"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /components/catl_mbmu_stat_r/mbmu_status '"Normal"'
