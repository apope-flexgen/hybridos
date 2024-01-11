#!/bin/sh

# script to set up and test the BMS Heasrtbeat
# will not work.... we send out the heartbeat

/usr/local/bin/fims/fims_send -m pub  -u /components/catl_ems_bms_rw/ems_heartbeat 222
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_ems_bms_rw/ems_heartbeat 223
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_ems_bms_rw/ems_heartbeat 224
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_ems_bms_rw/ems_heartbeat 225
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_ems_bms_rw/ems_heartbeat 226
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_ems_bms_rw/ems_heartbeat 222
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_ems_bms_rw/ems_heartbeat 223
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_ems_bms_rw/ems_heartbeat 224
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_ems_bms_rw/ems_heartbeat 225
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_ems_bms_rw/ems_heartbeat 226
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_ems_bms_rw/ems_heartbeat 222
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_ems_bms_rw/ems_heartbeat 223

echo "# trigger alarm"
sleep 3   
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_ems_bms_rw/ems_heartbeat 224


echo " # trigger fault"
sleep 7

echo "# recover"
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_ems_bms_rw/ems_heartbeat 225
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_ems_bms_rw/ems_heartbeat 226
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_ems_bms_rw/ems_heartbeat 222
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_ems_bms_rw/ems_heartbeat 223
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_ems_bms_rw/ems_heartbeat 224
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_ems_bms_rw/ems_heartbeat 225


