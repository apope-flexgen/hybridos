#!/bin/bash
#turn on pcs_1 and dcdc_1 and set both to 300W discharge

/usr/local/bin/fims_send -m set -r /me -u /components/bms_1/ctrlword1 1
/usr/local/bin/fims_send -m set -r /me -u /components/bms_2/ctrlword1 1
/usr/local/bin/fims_send -m set -r /me -u /components/dcdc_1/ctrlword1 1
/usr/local/bin/fims_send -m set -r /me -u /components/pcs_1/ctrlword1 1
/usr/local/bin/fims_send -m set -r /me -u /components/pcs_1/pcmd '{"value":300}'
/usr/local/bin/fims_send -m set -r /me -u /components/dcdc_1/pcmd '{"value":300}'

