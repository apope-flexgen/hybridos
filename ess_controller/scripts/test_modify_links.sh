 #!/bin/sh
 # simple script to demo the dynamic relink feature of the ess_controller
 #
 # in the HeartBeat function ther is this setup code

 #amap["todSec"]               = vmp->setLinkVal(vmap, aname, "/status",    "todSec",                ival);

 
 # This code would default the bms_1/todSec to be sent to /status/bms_i/bms_todSec

 # this is not what we want on the real system to we use a "link" to relocate the bms_todSec output to
 # /components/pcr_:HeartBeatSec
 # here is the config file segment that does that
 #
#  "/links/bms_1": {
#      "bms_todSec": {
#      "value": "/components/pcr_1:HeartBeatSec"
##     }
#   },
#
# When the system starts The HandleESSHeartBeat function sets itself up
# it adds items into the system config if they are not already defined.
#
#The presence of the link overrides the hard coded configuration in the code.
# on a complex system with stringent downtime penalties we do NOT want to shut down the whole system  and disrupt comminications to mak small changes.
# if we are not happy with the compiles in or the configuered assignment of this link then we can readjut it.
#The following command will  reset the link to a different place.


/usr/local/bin/fims/fims_send -m set  -r/$$ -u /links/bms_1 '{\"bms_todSec\":\"/components/pcr_1:TheOtherPlace\"}"

# This will readjust the link. and yes, the final system will save that adjustment in nonvolatile memory.
#
#But the HandleBMSHeartBeat function has to be told  to reevaluate it links
# the (soon to be reworked) junky code at the headof every function handles that.
# we have a control variable "HandleBMSHeartBeat" that will cause the Function to remap its connections the system. Set that to 0 to cold start the function or 1
# to warm start it and recalculate all its links

# so the sequence is :

/usr/local/bin/fims/fims_send -m set  -r/$$ -u /links/bms_1 '{"bms_todSec":"/components/pcr_1:TheOtherPlace"}'

/usr/local/bin/fims/fims_send -m set  -r/$$ -u /config/bms_1 '{"HandleBMSHeartBeat":1}'


# /usr/local/bin/fims/fims_send -m get  -r/$$ -u /links/bms_1/bms_todSec 
#{
#  "value": "/components/pcr_1:TheOtherPlace"
#}

#then get the new values siting in their new location(s)
/usr/local/bin/fims/fims_send -m get  -r/$$ -u /components/pcr_1 | jq
/usr/local/bin/fims/fims_listen -s /components/pcr_1
