#!/bin/bash

echo "PCS 1 ~~~~~~~~~~~~~~~~~~~"
echo -n "              On: " && /usr/local/bin/fims_send -m get -r /$$ -u /components/pcs_1/on
echo -n "             Pdc: " && /usr/local/bin/fims_send -m get -r /$$ -u /components/pcs_1/pdc
echo
echo "BMS 1 ~~~~~~~~~~~~~~~~~~"
echo -n "              On: " && /usr/local/bin/fims_send -m get -r /$$ -u /components/bms_1/on
# echo -n "     GridForming: " &/usr/local/bin/fims_send -m get -r /$$ -u /components/bms_1/gridforming
echo -n "           Power: " && /usr/local/bin/fims_send -m get -r /$$ -u /components/bms_1/p
echo -n "             SOC: " && /usr/local/bin/fims_send -m get -r /$$ -u /components/bms_1/soc_value
# echo -n "            Freq: " && $fims_get /components/bms_1/f
echo
echo "DCDC 1 ~~~~~~~~~~~~~~~~~~~"
echo -n "              On: " && /usr/local/bin/fims_send -m get -r /$$ -u /components/dcdc_1/on
echo -n "             Pdc: " && /usr/local/bin/fims_send -m get -r /$$ -u /components/dcdc_1/pdc2
echo

echo "DC Bus ~~~~~~~~~~~~~~~~~~~"
echo -n "             Pdc: " && /usr/local/bin/fims_send -m get -r /$$ -u /components/dcbus/pdc
# echo -n "           Power: " && /usr/local/bin/fims_send -m get -r /$$ -u /components/twins_shared_poi/p
echo
echo "BMS 2 ~~~~~~~~~~~~~~~~~~"
echo -n "              On: " && /usr/local/bin/fims_send -m get -r /$$ -u /components/bms_2/on
# echo -n "     GridForming: " && /usr/local/bin/fims_send -m get -r /$$ -u /components/bms_1/gridforming
echo -n "           Power: " && /usr/local/bin/fims_send -m get -r /$$ -u /components/bms_2/p
echo -n "             SOC: " && /usr/local/bin/fims_send -m get -r /$$ -u /components/bms_2/soc_value

# echo -n "            Freq: " && /usr/local/bin/fims_send -m get -r /$$ -u /components/bms_1/f