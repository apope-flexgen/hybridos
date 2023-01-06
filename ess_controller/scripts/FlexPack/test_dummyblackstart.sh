#!/bin/sh
echo  load in flexpack commands
sh scripts/FlexPack/all_command.sh
echo load the blackstart test config
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands/loadCfg '
             {"value":"someval","config":"blackstarttest.json"}'

echo check the open_all_breakers status
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/controls/ess/open_all_breakers | jq

echo send the openAllBreakers commands
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/controls/ess/openAllBreakers true

echo check the open_all_breakers status
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/controls/ess/open_all_breakers | jq 

echo send the breaker closed responses
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/status/ess/breaker_52_M1_open true

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/status/ess/breaker_52_M2_open true
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/status/ess/breaker_52_G1_open true
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/status/ess/breaker_52_G2_open true
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/status/ess/breaker_52_G3_open true
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/status/ess/breaker_52_D1_open true
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/status/ess/breaker_52_D2_open true
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/status/ess/breaker_52_M_open true

echo check the completion
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/controls/ess/open_all_breakers | jq 

echo check the next stage
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/reload/ess | jq 
