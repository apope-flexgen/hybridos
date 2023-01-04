#!/bin/sh

for i in {1..1000}
do
fims_send -f  /home/build/git_build/ess_controller/configs/sample_siteController_modbus_server.json  -m set -u /some/fims/thing
done

