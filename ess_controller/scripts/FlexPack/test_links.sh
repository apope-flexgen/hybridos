#!/bin/sh
#wait_pause
echo setup Link command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
        {"link":
                {"value":"test",
                    "help": "link two vars",
                    "ifChanged":false, 
                    "enabled":true,
                    "debug":true,
                    "actions":{"onSet":[{"func":[{"func":"RunLinks"}]}]}
                }
        }'


/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/links/rack_6 '
    {
        "Year": {
            "value": "/components/catl_rack_6_ems_bms_rw:ems_rtc_year",
            "linkvar": "/status/rack_6:year",
            "defval": 0,
            "pname": "/bms/rack_6"
        }
    }'


echo test the linkvar
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/links/rack_6 

echo run the linkvar command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '{"link":{"value":"test","amap": "rack_6"}}'

echo test the linked variable value , should be 0
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/status/rack_6/year

echo now set the incoming variable to 2021

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/catl_rack_6_ems_bms_rw/ems_rtc_year 2021

echo test the changed value in the linked variable 
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/status/rack_6/year