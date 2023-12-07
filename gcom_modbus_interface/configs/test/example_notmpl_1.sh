/usr/local/bin/fims_send -m pub -u /components/comp1 '{ "01":{"value":2},"02":{"value":2},"03":{"value":2},"04":{"value":2},"10":{"value":2},"13":{"value":2}}'&
#/usr/local/bin/fims_echo -u /components/comp2 -b '{"24_decode_id":{"value":0},"25":{"value":0}}'&
#/usr/local/bin/fims_echo -u /components/xyz_ip_device_id -b '{"26":{"value":0}}'&
