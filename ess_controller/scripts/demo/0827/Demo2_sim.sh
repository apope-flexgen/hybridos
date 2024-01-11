
#!/bin/sh

# simple test script to make sure the cel voltage reporting is working.

echo "First load up the Sim Information"
sh scripts/FlexPack/load_rizen_rack.sh | jq
echo 
echo 
fims_send -m get -r /$$ -u /flex/full//bms_rack_02_info/rack_voltage

sh scripts/FlexPack/load_rizen_sim.sh
fims_send -m set -r /$$ -u /flex/full/status/bms/rack_02_mod_01_cell_02/cell_voltage 16.4
fims_send -m set -r /$$ -u /flex/full/status/bms/rack_02_mod_02_cell_02/cell_voltage 1.4
fims_send -m set -r /$$ -u /flex/full/status/bms/rack_02_mod_02_cell_02/cell_voltage 1.4
fims_send -m set -r /$$ -u /flex/full/status/bms/rack_02_mod_02_cell_02/cell_voltage 11.4
fims_send -m set -r /$$ -u /flex/full/status/bms/rack_02_mod_01_cell_02/cell_voltage 11.4
# watch -n 0.5 'fims_send -m get -r /$$ -u /flex/full/components/bms_rack_02_info/rack_voltage |jq'
fims_send -m get -r /$$ -u /flex/full/components/bms_rack_02_info/rack_voltage |jq

fims_send -m get -r /$$ -u /flex/full/status/bms/rack_02_mod_01_cell_02/cell_voltage 11.4
fims_send -m get -r /$$ -u /flex/full/status/bms/rack_02_mod_01/mod_voltage |jq

fims_send -m get -r /$$ -u /flex/full/status/bms/rack_02_mod_01_cell_03/cell_voltage |jq
fims_send -m get -r /$$ -u /flex/full/status/bms/Voltage |jq

#{
#  "cell_voltage": {
#    "value": 3.421
#  }
#}
