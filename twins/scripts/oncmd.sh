#!/bin/bash
#turn on pcs_1 and dcdc_1 and set both to 300W discharge

# /usr/local/bin/fims_send -m set -r /me -u /components/bms_1/ctrlword1 2
# /usr/local/bin/fims_send -m set -r /me -u /components/bms_2/disablefault true
# /usr/local/bin/fims_send -m set -r /me -u /components/bms_1/disablefault true
# /usr/local/bin/fims_send -m set -r /me -u /components/bms_2/ctrlword1 2
# /usr/local/bin/fims_send -m set -r /me -u /components/dcdc_1/oncmd true
# /usr/local/bin/fims_send -m set -r /me -u /components/pcs_1/ctrlword1 2
# /usr/local/bin/fims_send -m set -r /me -u /components/pcs_1/pcmd '{"value":300}'
# /usr/local/bin/fims_send -m set -r /me -u /components/dcdc_1/pcmd '{"value":300}'


# these commands are used for setting up ess ramp rate functional testing with twins
/usr/local/bin/fims_send -m set -u /components/ess1/rampenable true

/usr/local/bin/fims_send -m set -u /components/ess1/pramprise 10
/usr/local/bin/fims_send -m set -u /components/ess1/prampdrop 20
/usr/local/bin/fims_send -m set -u /components/ess1/prampstart 15
/usr/local/bin/fims_send -m set -u /components/ess1/prampstop 25

/usr/local/bin/fims_send -m set -u /components/ess1/qramprise 17.5
/usr/local/bin/fims_send -m set -u /components/ess1/qrampdrop 12
/usr/local/bin/fims_send -m set -u /components/ess1/qrampstart 10
/usr/local/bin/fims_send -m set -u /components/ess1/qrampstop 35

/usr/local/bin/fims_send -m set -u /components/ess1/soc 50
/usr/local/bin/fims_send -m set -u /components/ess1/phigh 5000
/usr/local/bin/fims_send -m set -u /components/ess1/qhigh 5000
/usr/local/bin/fims_send -m set -u /components/ess1/pcharge 5000
/usr/local/bin/fims_send -m set -u /components/ess1/pdischarge 5000
/usr/local/bin/fims_send -m set -u /components/ess1/cap 2000

# /usr/local/bin/fims_send -m set -u /components/ess1/oncmd true


