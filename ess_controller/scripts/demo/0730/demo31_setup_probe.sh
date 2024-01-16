#!/bin/sh
#   need id probeBms temp table /probe/bms  and output table /components/probe
# sh demo31_setup_probe.sh  probeBms /probe/bms /components/probe 0.5
probeID=$1
probeTable=$2
pubTable=$3
probeTime=$4

#test override
probeID=probeBms
probeTable=/probe/bms
probeTime=0.5
pubTable=/components/probe
clear

echo setupcommand to allow us to create links to variables 
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
{
  "Link":{"value":"test","help": "link two variables","ifChanged":false, "enabled":true, 
                      "actions":{ "onSet":[{"func":[{"func":"MakeLink"}]}] }
          },
  "run":{"value":"test", "help": "run a schedule var", "ifChanged":false, "enabled":true, 
                      "actions":{ "onSet":[{"func":[{"func":"RunSched"}]}] }
          },
  "stop":{"value":"test","help": "stop a schedule var","ifChanged":false, "enabled":true, 
                      "actions":{ "onSet":[{"func":[{"func":"StopSched"}]}] }
         }            
}' | jq

echo set up the pub function for the probe
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/control/pubs "
    { \"${probeID}\":{\"value\":1,\"table\":\"${probeTable}\",\"sendas\":\"${pubTable}\",
      \"enabled\":true, \"actions\":{\"onSet\":[{\"func\":[{\"func\":\"RunPub\"}]}]}}}" | jq

echo -n " press enter to continue " && read in

clear
echo
echo  tell the scheduler to trigger probeBms every 0.5 seconds
echo '/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands/run '

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands/run "
     {\"value\":22,\"uri\":\"/control/pubs:${probeID}\",\"every\":${probeTime},\"offset\":0,\"debug\":0}" | jq

clear
echo
echo check the probe pubs
#/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/control/pubs | jq
sleep 1


#echo
#echo check the probe pubs
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/control/pubs | jq
echo '/usr/local/bin/fims/fims_listen'

#wait_pause



