#!/bin/bash

echo "PCS 1 ~~~~~~~~~~~~~~~~~~~"
echo -n "              On: " && /usr/local/bin/fims_send -m get -r /$$ -u /components/pcs_1/on
echo -n "             Pdc: " && /usr/local/bin/fims_send -m get -r /$$ -u /components/pcs_1/pdc
echo
# echo "ESS 2 ~~~~~~~~~~~~~~~~~~"
# echo -n "              On: " && $fims_get /components/twins_ess_2/on
# echo -n "     GridForming: " && $fims_get /components/twins_ess_2/gridforming
# echo -n "           Power: " && $fims_get /components/twins_ess_2/p
# echo -n "            Freq: " && $fims_get /components/twins_ess_2/f
# echo
# echo "XFMR ~~~~~~~~~~~~~~~~~~"
# echo -n "              V2: " && $fims_get /components/tx_grid/v2
# echo -n "           Power: " && $fims_get /components/tx_grid/p
# echo
# echo "Load ~~~~~~~~~~~~~~~~~~"
# echo -n "              On: " && $fims_get /components/biomass_plant_load/on
# echo -n "           Power: " && $fims_get /components/biomass_plant_load/p
# echo
# echo "POI Feeder ~~~~~~~~~~~~"
# echo -n "          Closed: " && $fims_get /components/twins_shared_poi/closed
# echo -n "           Power: " && $fims_get /components/twins_shared_poi/p