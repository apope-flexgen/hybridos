
/usr/local/bin/fims/fims_listen -s /components/catl_ems_bms_rw | grep Body | sed 's/Body://g' | jq

