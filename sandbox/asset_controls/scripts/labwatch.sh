# echo -n "Voltage:              " &&  fims_send -m get -r /me -u /ess/control/actions_ess_BatteryRackBalancing_bms_1/rack_1/voltage
# echo -n "Voltage, unfiltered:  " && fims_send -m get -r /me -u /ess/control/actions_ess_bwf_bms_1_rack_1_BWFilter/ButterworthFilter/input
# echo -n "Current:              " && fims_send -m get -r /me -u /ess/status/bms_1_rack_1/DCCurrent
# echo -n "PCmd:                 " && fims_send -m get -r /me -u /ess/control/actions_ess_BatteryRackBalancing_bms_1/BatteryBalancing/PCmd
# echo -n "PActl:                " && fims_send -m get -r /me -u /ess/status/pcs_1/ActivePower
# echo -n "SOC:                  " && fims_send -m get -r /me -u /ess/status/bms/SOC


echo -n "Overall PCMD:            " && fims_send -m get -r /me -u /ess/controls/pcs_1/ActivePowerSetpoint
echo -n "Overall PACTL:           " && fims_send -m get -r /me -u /ess/status/pcs_1/ActivePower
echo -n "PCS System State:        " && fims_send -m get -r /me -u /ess/status/pcs/SystemState
echo -n "MaxCellVolt              " && fims_send -m get -r /me -u /ess/status/bms/MaxCellVoltage
echo -n "MinCellVolt              " && fims_send -m get -r /me -u /ess/status/bms/MinCellVoltage

echo
echo "Rack 1 ====================="
echo -n "    Contactor Status:     " && fims_send -m get -r /me -u /ess/status/bms_1_rack_1/DCClosed
echo -n "    Voltage:              " &&  fims_send -m get -r /me -u /ess/status/bms_1_rack_1/DCVoltageBeforeBusBar
echo -n "    Current:              " && fims_send -m get -r /me -u /ess/status/bms_1_rack_1/DCCurrent
echo -n "    SOC:                  " && fims_send -m get -r /me -u /ess/status/bms_1_rack_1/SOC
echo
echo "Rack 2 ====================="
echo -n "    Contactor Status:     " && fims_send -m get -r /me -u /ess/status/bms_1_rack_2/DCClosed
echo -n "    Voltage:              " &&  fims_send -m get -r /me -u /ess/status/bms_1_rack_2/DCVoltageBeforeBusBar
echo -n "    Current:              " && fims_send -m get -r /me -u /ess/status/bms_1_rack_2/DCCurrent
echo -n "    SOC:                  " && fims_send -m get -r /me -u /ess/status/bms_1_rack_2/SOC
echo
echo "Rack 3 ====================="
echo -n "    Contactor Status:     " && fims_send -m get -r /me -u /ess/status/bms_1_rack_3/DCClosed
echo -n "    Voltage:              " &&  fims_send -m get -r /me -u /ess/status/bms_1_rack_3/DCVoltageBeforeBusBar
echo -n "    Current:              " && fims_send -m get -r /me -u /ess/status/bms_1_rack_3/DCCurrent
echo -n "    SOC:                  " && fims_send -m get -r /me -u /ess/status/bms_1_rack_3/SOC
echo
echo "Rack 4 ====================="
echo -n "    Contactor Status:     " && fims_send -m get -r /me -u /ess/status/bms_1_rack_4/DCClosed
echo -n "    Voltage:              " &&  fims_send -m get -r /me -u /ess/status/bms_1_rack_4/DCVoltageBeforeBusBar
echo -n "    Current:              " && fims_send -m get -r /me -u /ess/status/bms_1_rack_4/DCCurrent
echo -n "    SOC:                  " && fims_send -m get -r /me -u /ess/status/bms_1_rack_4/SOC
echo
echo "Rack 5 ====================="
echo -n "    Contactor Status:     " && fims_send -m get -r /me -u /ess/status/bms_1_rack_5/DCClosed
echo -n "    Voltage:              " &&  fims_send -m get -r /me -u /ess/status/bms_1_rack_5/DCVoltageBeforeBusBar
echo -n "    Current:              " && fims_send -m get -r /me -u /ess/status/bms_1_rack_5/DCCurrent
echo -n "    SOC:                  " && fims_send -m get -r /me -u /ess/status/bms_1_rack_5/SOC
echo