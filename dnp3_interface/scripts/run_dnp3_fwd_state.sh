#!/bin/sh
# p wilshire 02-07-2022
# send /components/sel_735/active_power every second


#Uri:     /components/sel_651r
#ReplyTo: (null)
#Body:    {"breaker_status":
#{"value":false,"flags":1,"sflags":"ONLINE","stime":"2022-03-12 02:13:16.162","ltime":"2022-03-12 02:13:16.161","etime":"2022-03-12 02:13:16.162"},"grid_voltage_l1_l2":0,"grid_voltage_l2_l3":0,"grid_voltage_l3_l1":0,"Timestamp":"03-12-2022 02:13:16.165709"}

while [ 1 ] ; do

   echo "false"
   fims_send -m pub -u /components/sel_651r '{
        "breaker_status": {
                   "value":false,"flags":1,
                    "stime":"2022-03-12 02:13:16.162",
                    "ltime":"2022-03-12 02:13:16.161",
                    "etime":"2022-03-12 02:13:16.162"}
    }'

   sleep 5
   echo "true"
   fims_send -m pub -u /components/sel_651r '{
        "breaker_status": {
                   "value":true,"flags":4,
                    "stime":"2022-03-12 02:13:16.162",
                    "ltime":"2022-03-12 02:13:16.161",
                    "etime":"2022-03-12 02:13:16.162"}
    }'
   sleep 5
done

