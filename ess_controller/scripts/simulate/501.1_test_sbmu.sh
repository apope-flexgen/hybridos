/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/schedule/ess '
{ 
    "addSchedItem":{
        "value":"SimSbmu",
        "uri":"/sched/ess:SimSbmu", 
        "fcn":"SimHandleSbmu","refTime":0.200,"runTime":0.200,"repTime":1.000,"endTime":0
}}
'