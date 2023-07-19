# !/bin/bash

echo Setting value to 1
fims_send -m set -u /var_2 1
sleep 5
echo Setting value to 2
fims_send -m set -u /var_2 2
sleep 5
echo Setting value to 3
fims_send -m set -u /var_2 3
sleep 5
echo Setting value to 4
fims_send -m set -u /var_2 4
echo done