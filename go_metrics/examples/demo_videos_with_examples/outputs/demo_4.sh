# !/bin/bash

echo Setting value to 1
fims_send -m set -u /var_4 1
sleep 5
echo Setting value to 2
fims_send -m set -u /var_4 2
sleep 5
echo Setting value to 10
fims_send -m set -u /var_4 10
sleep 5
echo Setting value to 11
fims_send -m set -u /var_4 11
sleep 5
echo Setting value to 12
fims_send -m set -u /var_4 12
sleep 5
echo Setting value to 20
fims_send -m set -u /var_4 20
echo done