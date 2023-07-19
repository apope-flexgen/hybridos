# !/bin/bash

echo Setting value to 1
fims_send -m set -u /var_1 1
sleep 2
echo Setting value to 2
fims_send -m set -u /var_1 2
sleep 2
echo Setting value to 3
fims_send -m set -u /var_1 3
sleep 2
echo Setting value to 4
fims_send -m set -u /var_1 4
sleep 2
echo Setting value to 5
fims_send -m set -u /var_1 5
sleep 2
echo Setting value to 6
fims_send -m set -u /var_1 6
sleep 2
echo Setting value to 7
fims_send -m set -u /var_1 7
echo done