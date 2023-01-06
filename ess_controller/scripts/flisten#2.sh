
/usr/local/bin/fims/fims_listen -s /components/catl_bms_ems_r | grep Body | sed 's/Body://g' | jq
