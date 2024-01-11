#!/bin/sh
# start the fast pubs for Sungrow
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/schedule/ess '
{
     "addSchedItem":{
         "value":"PcsFastPub",
         "var":"/sched/pcs:pcsFastPub",
         "debug":1,
         "amap":"pcs",
         "uri":"/sched/pcs:PcsFastPub",
         "fcn":"FastPub",
         "refTime":0.200,
         "runTime":0.200,
         "repTime":2.000,
         "endTime":0
 }
 }
 '
