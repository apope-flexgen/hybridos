echo test 1  timeout expected
/usr/local/bin/fims/fims_send -m get -r /$$ -u /components/catl_ems_to_bms_rw

echo test 2
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/components/catl_ems_to_bms_rw

echo test 3
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/components/catl_ems_to_bms_rw/ems_heartbeat

echo test 4
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/catl_ems_to_bms_rw '{"ems_test":{"value":1234}}'

echo test 5
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/components/catl_ems_to_bms_rw/ems_test

echo test 6
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/components/catl_ems_to_bms_rw

echo test 7
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/components/catl_ems_to_bms_rw| jq

echo test 8 timout expected
/usr/local/bin/fims/fims_send -m get -r /$$ -u /components/catl_ems_to_bms_rw/ems_test

echo test 9 timeout expected (pub)
/usr/local/bin/fims/fims_send -m pub -r /$$ -u /ess/components/catl_ems_to_bms_rw '{"ems_test":{"value":51234}}'

echo test 10 timeout expected
/usr/local/bin/fims/fims_send -m get -r /$$ -u /components/catl_ems_to_bms_rw/ems_test

echo test 11
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/components/catl_ems_to_bms_rw/ems_test


exit


#results

test 1
Receive Timeout.
test 2
{"ems_cmd":{"value":0},"ems_heartbeat":{"value":39},"ems_rtc_day":{"value":6},"ems_rtc_hour":{"value":11},"ems_rtc_minute":{"value":35},"ems_rtc_month":{"value":1},"ems_rtc_second":{"value":39},"ems_rtc_year":{"value":2021},"ems_test":{"value":51234}}
test 3
39
test 4
{"ems_test":{"value":1234}}
test 5
1234
test 6
{"ems_cmd":{"value":0},"ems_heartbeat":{"value":39},"ems_rtc_day":{"value":6},"ems_rtc_hour":{"value":11},"ems_rtc_minute":{"value":35},"ems_rtc_month":{"value":1},"ems_rtc_second":{"value":39},"ems_rtc_year":{"value":2021},"ems_test":{"value":1234}}
test 7
{
  "ems_cmd": {
    "value": 0
  },
  "ems_heartbeat": {
    "value": 39
  },
  "ems_rtc_day": {
    "value": 6
  },
  "ems_rtc_hour": {
    "value": 11
  },
  "ems_rtc_minute": {
    "value": 35
  },
  "ems_rtc_month": {
    "value": 1
  },
  "ems_rtc_second": {
    "value": 39
  },
  "ems_rtc_year": {
    "value": 2021
  },
  "ems_test": {
    "value": 1234
  }
}
test 8
Receive Timeout.
test 9
Receive Timeout.
test 10
Receive Timeout.
test 11
51234

