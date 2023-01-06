#/bin/sh
# p. wlshire 03-09-2022
# simple simulate modbus data

fims_send -m pub  -u /components/ess/pcs_running_info '{
        "status":"init",
        "current":295,
        "voltage":2325,
        "alarms": 0,
        "faults":0
    }'


