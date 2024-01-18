#!/bin/bash

# Run with "watch -n 0.1 /path/to/watch.sh"

echo -n "BMS Status: " && fims_send -m get -r /$$ -u /components/bms_1/status_0_value
echo -n "BMS String: " && fims_send -m get -r /$$ -u /components/bms_1/status_0_string
echo -n "BMS fault: " && fims_send -m get -r /$$ -u /components/bms_1/fault
echo -n "BMS ctrlword1: " && fims_send -m get -r /$$ -u /components/bms_1/ctrlword1
echo -n "BMS SOC: " && fims_send -m get -r /$$ -u /components/bms_1/soc_value
echo -n "BMS VDC: " && fims_send -m get -r /$$ -u /components/bms_1/vdc_value
echo -n "BMS dccontactorclosecmd: " && fims_send -m get -r /$$ -u /components/bms_1/dccontactorclosecmd
echo -n "BMS dccontactoropencmd: " && fims_send -m get -r /$$ -u /components/bms_1/dccontactoropencmd
echo -n "BMS dccontactor: " && fims_send -m get -r /$$ -u /components/bms_1/dccontactor
echo -n "BMS oncmd: " && fims_send -m get -r /$$ -u /components/bms_1/oncmd
echo -n "BMS offcmd: " && fims_send -m get -r /$$ -u /components/bms_1/offcmd
echo -n "BMS on: " && fims_send -m get -r /$$ -u /components/bms_1/on
echo
echo -n "PCS Status: " && fims_send -m get -r /$$ -u /components/pcs_1/status_0_value
echo -n "PCS String: " && fims_send -m get -r /$$ -u /components/pcs_1/status_0_string
echo -n "PCS fault: " && fims_send -m get -r /$$ -u /components/pcs_1/fault
echo -n "PCS ctrlword1: " && fims_send -m get -r /$$ -u /components/pcs_1/ctrlword1
echo -n "PCS VDC: " && fims_send -m get -r /$$ -u /components/pcs_1/vdc
echo -n "PCS dccontactor: " && fims_send -m get -r /$$ -u /components/pcs_1/dccontactor
echo -n "PCS accontactor: " && fims_send -m get -r /$$ -u /components/pcs_1/dccontactor
echo -n "PCS oncmd: " && fims_send -m get -r /$$ -u /components/pcs_1/oncmd
echo -n "PCS offcmd: " && fims_send -m get -r /$$ -u /components/pcs_1/offcmd
echo -n "PCS on: " && fims_send -m get -r /$$ -u /components/pcs_1/on
echo -n "PCS P: " && fims_send -m get -r /$$ -u /components/pcs_1/p
