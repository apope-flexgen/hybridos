System Date and Time

Author: Jimmy Nguyen

Date Created: 
* 03/09/2021

Date Modified: 

# Overview
The ESS Controller manages its own date and time using the `chrono` and `ctime` libraries. In addition to keeping track of its date and time, the ESS Controller can also send out the date and time to assets that need the date and time populated periodically (ex.: CATL BMS) via fims set/pub.  

Also, the ESS Controller can send out heartbeats to assets as well to ensure communications are active between the ESS Controller and assets that need the heartbeat populated periodically.

The variables that the ESS Controller will set the date, time, and heartbeat are the following:
* `Heartbeat`
* `Sec`
* `Min`
* `Hour`
* `Day`
* `Month`
* `Year`

# Configuration
Here is an example of how to configure the date, time, and heartbeat variables to be updated by the ESS Controller:

```json
    "/links/bms": {
        "Heartbeat": {
            "value": "/components/catl_ems_bms_rw:ems_heartbeat"
        },
        "Sec": {
            "value": "/components/catl_ems_bms_rw:ems_rtc_sec"
        },
        "Min": {
            "value": "/components/catl_ems_bms_rw:ems_rtc_minute"
        },
        "Hour": {
            "value": "/components/catl_ems_bms_rw:ems_rtc_hour"
        },
        "Day": {
            "value": "/components/catl_ems_bms_rw:ems_rtc_day"
        },
        "Month": {
            "value": "/components/catl_ems_bms_rw:ems_rtc_month"
        },
        "Year": {
            "value": "/components/catl_ems_bms_rw:ems_rtc_year"
        }
    }
```

In the configuration above, the ESS Controller variables (ex.: Heartbeat) are linked to the modbus registers (ex.: ems_heartbeat), and whenever the system date, time, and heartbeat are updated in the ESS Controller, the modbus registers that are linked to the internal variables will also receive the same update as well.  

If the asset does not need its date, time, and/or heartbeat registers updated, then do not include the configuration items shown above.

## Configure Heartbeat
Different systems may have a different maximum value for the heartbeat (ex.: CATL BMS ems_heartbeat is between 0-255). Currently, the ESS Controller heartbeat is periodically incremented, and the heartbeat value is between 0-60)

Here is an example of how to adjust the maximum value of the heartbeat in the ESS Controller config:

TODO: add max heartbeat limit as parameter

# Data Validation
While the ESS Controller is running, to check the system date, time, and heartbeat, you can do the following:
1. Fims interface  
    * ESS Controller variables
      * `/usr/local/bin/fims/fims_send -m get -u /status/ess/Heartbeat -r /me`
      * `/usr/local/bin/fims/fims_send -m get -u /status/ess/Sec -r /me`
      * `/usr/local/bin/fims/fims_send -m get -u /status/ess/Min -r /me`
      * `/usr/local/bin/fims/fims_send -m get -u /status/ess/Hour -r /me`
      * `/usr/local/bin/fims/fims_send -m get -u /status/ess/Day -r /me`
      * `/usr/local/bin/fims/fims_send -m get -u /status/ess/Month -r /me`
      * `/usr/local/bin/fims/fims_send -m get -u /status/ess/Year -r /me`  
    * Modbus variables (CATL BMS as an example)  
      * `/usr/local/bin/fims/fims_send -m get -u /ess/components/catl_ems_bms_r/ems_heartbeat -r /me`
      * `/usr/local/bin/fims/fims_send -m get -u /ess/components/catl_ems_bms_r/ems_rtc_sec -r /me`
      * `/usr/local/bin/fims/fims_send -m get -u /ess/components/catl_ems_bms_r/ems_rtc_minute -r /me`
      * `/usr/local/bin/fims/fims_send -m get -u /ess/components/catl_ems_bms_r/ems_rtc_hour -r /me`
      * `/usr/local/bin/fims/fims_send -m get -u /ess/components/catl_ems_bms_r/ems_rtc_day -r /me`
      * `/usr/local/bin/fims/fims_send -m get -u /ess/components/catl_ems_bms_r/ems_rtc_month -r /me`
      * `/usr/local/bin/fims/fims_send -m get -u /ess/components/catl_ems_bms_r/ems_rtc_year -r /me`
