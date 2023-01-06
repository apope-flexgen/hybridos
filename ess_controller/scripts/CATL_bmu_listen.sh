/usr/local/bin/fims/fims_listen -s /components/catl_BMU | grep Body | sed "s/Body://g" | jq
