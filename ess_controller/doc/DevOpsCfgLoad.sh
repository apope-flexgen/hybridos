#!/bin/sh
# file to load the configs for the devops training part 1
# first test the files
tdir=~/v1.1.0/ess_controller/doc/
cat ${tdir}DevOpsEss.json | jq
cat ${tdir}DevOpsPcs.json | jq
cat ${tdir}DevOpsBms.json | jq
cat ${tdir}DevOpsSite.json | jq
cat ${tdir}DevOpsFinal.json | jq
echo
echo " #################### end of json tests"
echo
fims_send -m set -f  ${tdir}DevOpsEss.json -u /ess/cfg/cfile/ess/DevOpsEss
fims_send -m set -f  ${tdir}DevOpsBms.json -u /ess/cfg/cfile/ess/DevOpsBms
fims_send -m set -f  ${tdir}DevOpsPcs.json -u /ess/cfg/cfile/ess/DevOpsPcs
fims_send -m set -f  ${tdir}DevOpsSite.json -u /ess/cfg/cfile/ess/DevOpsSite
fims_send -m set -f  ${tdir}DevOpsFinal.json -u /ess/cfg/cfile/ess/DevOpSFinal
#fims_send -m get -r /$$ -u /ess/full/notes/ess
fims_send -m get -r /$$ -u /ess/full/config/cfile | jq
fims_send -m get -r /$$ -u /ess/full/amap | jq

exit

{
    "/notes/site": {
        "note" :" These are owned by the site manager"
    },
    "/site/ess_hs":{ 
        "commands":{
            "value":0,
            "actions":{
                "onSet":[{
                    "enum":[
                        {"inValue":3,"uri":"/system/pcs:status","outValue":"Stop"},
                        {"inValue":2,"uri":"/system/pcs:status","outValue":"Standby"},
                        {"inValue":1,"uri":"/system/pcs:status","outValue":"Run"}
        
                        ]
                    }]        
            }
        }
    }
}
