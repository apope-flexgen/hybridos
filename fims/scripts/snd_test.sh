#!/bin/sh

for i in {1..1000}
do
fims_send -f  /home/build/git_build/ess_controller/configs/ess_config.json  -m set -u /some/fims/thing
done

