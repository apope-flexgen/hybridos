#!/bin/sh 

# Script used to check bms data values from modbus interface

fSend=/usr/local/bin/fims/fims_send

# Helper function for doing a fims set
#
# $1 = uri and/or variable name (ex.: /components/catl_mbmu_control_r or /components/catl_mbmu_control_r/mbmu_max_cell_voltage)
# $2 = value to set
fimsSet()
{
    $fSend -m set -u $1 $2 -r /me | jq
}

# Helper function for doing a fims pub
#
# $1 = uri and/or variable name (ex.: /components/catl_mbmu_control_r or /components/catl_mbmu_control_r/mbmu_max_cell_voltage)
# $2 = value to set
fimsPub()
{
    $fSend -m pub -u $1 $2
}

# Helper function for doing a fims get
#
# $1 = uri and/or variable name (ex.: /components/catl_mbmu_control_r or /components/catl_mbmu_control_r/mbmu_max_cell_voltage)
fimsGet()
{
    $fSend -m get -u $1 -r /me | jq
}

# Initialize data before testing
# Note: comment out function call below if raw data values should be read from the hardware unit
initializeData()
{
    echo "Initializing data..."

    # Initialize mbmu data
    fimsPub "/components/catl_bms_ems_r/bms_poweron" 1
    fimsPub "/components/catl_bms_ems_r/bms_status" 1
    fimsPub "/components/catl_mbmu_summary_r/mbmu_soc" 50
    fimsPub "/components/catl_mbmu_summary_r/mbmu_soh" 70
    fimsPub "/components/catl_mbmu_summary_r/mbmu_voltage" 10
    fimsPub "/components/catl_mbmu_summary_r/mbmu_current" 20
    fimsPub "/components/catl_mbmu_summary_r/mbmu_max_cell_voltage" 15
    fimsPub "/components/catl_mbmu_summary_r/mbmu_min_cell_voltage" 5
    fimsPub "/components/catl_mbmu_summary_r/mbmu_avg_cell_voltage" 10
    fimsPub "/components/catl_mbmu_summary_r/mbmu_max_cell_temperature" 60
    fimsPub "/components/catl_mbmu_summary_r/mbmu_min_cell_temperature" 20
    fimsPub "/components/catl_mbmu_summary_r/mbmu_avg_cell_temperature" 35
    fimsPub "/components/catl_bms_ems_r/bms_max_charge_allowed" 54
    fimsPub "/components/catl_bms_ems_r/bms_max_discharge_allowed" 23
    fimsPub "/components/catl_bms_ems_r/bms_remain_charge_energy" 43
    fimsPub "/components/catl_bms_ems_r/bms_remain_discharge_energy" 17
    fimsPub "/components/catl_bms_ems_r/num_hv_subsystem" 9

    # Initialize sbmu data
    fimsPub "/components/catl_sbmu_1/sbmu_soc" 65
    fimsPub "/components/catl_sbmu_1/sbmu_soh" 73
    fimsPub "/components/catl_sbmu_1/sbmu_voltage" 14
    fimsPub "/components/catl_sbmu_1/sbmu_max_cell_voltage" 32
    fimsPub "/components/catl_sbmu_1/sbmu_min_cell_voltage" 3
    fimsPub "/components/catl_sbmu_1/sbmu_avg_cell_voltage" 16
    fimsPub "/components/catl_sbmu_1/sbmu_max_cell_temperature" 57
    fimsPub "/components/catl_sbmu_1/sbmu_min_cell_temperature" 23
    fimsPub "/components/catl_sbmu_1/sbmu_avg_cell_temperature" 40
    fimsPub "/components/catl_sbmu_1/sbmu_max_charge_current" 5
    fimsPub "/components/catl_sbmu_1/sbmu_max_discharge_current" 2

    fimsPub "/components/catl_sbmu_2/sbmu_soc" 65
    fimsPub "/components/catl_sbmu_2/sbmu_soh" 73
    fimsPub "/components/catl_sbmu_2/sbmu_voltage" 14
    fimsPub "/components/catl_sbmu_2/sbmu_max_cell_voltage" 32
    fimsPub "/components/catl_sbmu_2/sbmu_min_cell_voltage" 3
    fimsPub "/components/catl_sbmu_2/sbmu_avg_cell_voltage" 16
    fimsPub "/components/catl_sbmu_2/sbmu_max_cell_temperature" 57
    fimsPub "/components/catl_sbmu_2/sbmu_min_cell_temperature" 23
    fimsPub "/components/catl_sbmu_2/sbmu_avg_cell_temperature" 40
    fimsPub "/components/catl_sbmu_2/sbmu_max_charge_current" 5
    fimsPub "/components/catl_sbmu_2/sbmu_max_discharge_current" 2

    fimsPub "/components/catl_sbmu_3/sbmu_soc" 65
    fimsPub "/components/catl_sbmu_3/sbmu_soh" 73
    fimsPub "/components/catl_sbmu_3/sbmu_voltage" 14
    fimsPub "/components/catl_sbmu_3/sbmu_max_cell_voltage" 32
    fimsPub "/components/catl_sbmu_3/sbmu_min_cell_voltage" 3
    fimsPub "/components/catl_sbmu_3/sbmu_avg_cell_voltage" 16
    fimsPub "/components/catl_sbmu_3/sbmu_max_cell_temperature" 57
    fimsPub "/components/catl_sbmu_3/sbmu_min_cell_temperature" 23
    fimsPub "/components/catl_sbmu_3/sbmu_avg_cell_temperature" 40
    fimsPub "/components/catl_sbmu_3/sbmu_max_charge_current" 5
    fimsPub "/components/catl_sbmu_3/sbmu_max_discharge_current" 2

    fimsPub "/components/catl_sbmu_4/sbmu_soc" 65
    fimsPub "/components/catl_sbmu_4/sbmu_soh" 73
    fimsPub "/components/catl_sbmu_4/sbmu_voltage" 14
    fimsPub "/components/catl_sbmu_4/sbmu_max_cell_voltage" 32
    fimsPub "/components/catl_sbmu_4/sbmu_min_cell_voltage" 3
    fimsPub "/components/catl_sbmu_4/sbmu_avg_cell_voltage" 16
    fimsPub "/components/catl_sbmu_4/sbmu_max_cell_temperature" 57
    fimsPub "/components/catl_sbmu_4/sbmu_min_cell_temperature" 23
    fimsPub "/components/catl_sbmu_4/sbmu_avg_cell_temperature" 40
    fimsPub "/components/catl_sbmu_4/sbmu_max_charge_current" 5
    fimsPub "/components/catl_sbmu_4/sbmu_max_discharge_current" 2

    fimsPub "/components/catl_sbmu_5/sbmu_soc" 65
    fimsPub "/components/catl_sbmu_5/sbmu_soh" 73
    fimsPub "/components/catl_sbmu_5/sbmu_voltage" 14
    fimsPub "/components/catl_sbmu_5/sbmu_max_cell_voltage" 32
    fimsPub "/components/catl_sbmu_5/sbmu_min_cell_voltage" 3
    fimsPub "/components/catl_sbmu_5/sbmu_avg_cell_voltage" 16
    fimsPub "/components/catl_sbmu_5/sbmu_max_cell_temperature" 57
    fimsPub "/components/catl_sbmu_5/sbmu_min_cell_temperature" 23
    fimsPub "/components/catl_sbmu_5/sbmu_avg_cell_temperature" 40
    fimsPub "/components/catl_sbmu_5/sbmu_max_charge_current" 5
    fimsPub "/components/catl_sbmu_5/sbmu_max_discharge_current" 2

    fimsPub "/components/catl_sbmu_6/sbmu_soc" 65
    fimsPub "/components/catl_sbmu_6/sbmu_soh" 73
    fimsPub "/components/catl_sbmu_6/sbmu_voltage" 14
    fimsPub "/components/catl_sbmu_6/sbmu_max_cell_voltage" 32
    fimsPub "/components/catl_sbmu_6/sbmu_min_cell_voltage" 3
    fimsPub "/components/catl_sbmu_6/sbmu_avg_cell_voltage" 16
    fimsPub "/components/catl_sbmu_6/sbmu_max_cell_temperature" 57
    fimsPub "/components/catl_sbmu_6/sbmu_min_cell_temperature" 23
    fimsPub "/components/catl_sbmu_6/sbmu_avg_cell_temperature" 40
    fimsPub "/components/catl_sbmu_6/sbmu_max_charge_current" 5
    fimsPub "/components/catl_sbmu_6/sbmu_max_discharge_current" 2

    fimsPub "/components/catl_sbmu_7/sbmu_soc" 65
    fimsPub "/components/catl_sbmu_7/sbmu_soh" 73
    fimsPub "/components/catl_sbmu_7/sbmu_voltage" 14
    fimsPub "/components/catl_sbmu_7/sbmu_max_cell_voltage" 32
    fimsPub "/components/catl_sbmu_7/sbmu_min_cell_voltage" 3
    fimsPub "/components/catl_sbmu_7/sbmu_avg_cell_voltage" 16
    fimsPub "/components/catl_sbmu_7/sbmu_max_cell_temperature" 57
    fimsPub "/components/catl_sbmu_7/sbmu_min_cell_temperature" 23
    fimsPub "/components/catl_sbmu_7/sbmu_avg_cell_temperature" 40
    fimsPub "/components/catl_sbmu_7/sbmu_max_charge_current" 5
    fimsPub "/components/catl_sbmu_7/sbmu_max_discharge_current" 2

    fimsPub "/components/catl_sbmu_8/sbmu_soc" 65
    fimsPub "/components/catl_sbmu_8/sbmu_soh" 73
    fimsPub "/components/catl_sbmu_8/sbmu_voltage" 14
    fimsPub "/components/catl_sbmu_8/sbmu_max_cell_voltage" 32
    fimsPub "/components/catl_sbmu_8/sbmu_min_cell_voltage" 3
    fimsPub "/components/catl_sbmu_8/sbmu_avg_cell_voltage" 16
    fimsPub "/components/catl_sbmu_8/sbmu_max_cell_temperature" 57
    fimsPub "/components/catl_sbmu_8/sbmu_min_cell_temperature" 23
    fimsPub "/components/catl_sbmu_8/sbmu_avg_cell_temperature" 40
    fimsPub "/components/catl_sbmu_8/sbmu_max_charge_current" 5
    fimsPub "/components/catl_sbmu_8/sbmu_max_discharge_current" 2

    fimsPub "/components/catl_sbmu_9/sbmu_soc" 65
    fimsPub "/components/catl_sbmu_9/sbmu_soh" 73
    fimsPub "/components/catl_sbmu_9/sbmu_voltage" 14
    fimsPub "/components/catl_sbmu_9/sbmu_max_cell_voltage" 32
    fimsPub "/components/catl_sbmu_9/sbmu_min_cell_voltage" 3
    fimsPub "/components/catl_sbmu_9/sbmu_avg_cell_voltage" 16
    fimsPub "/components/catl_sbmu_9/sbmu_max_cell_temperature" 57
    fimsPub "/components/catl_sbmu_9/sbmu_min_cell_temperature" 23
    fimsPub "/components/catl_sbmu_9/sbmu_avg_cell_temperature" 40
    fimsPub "/components/catl_sbmu_9/sbmu_max_charge_current" 5
    fimsPub "/components/catl_sbmu_9/sbmu_max_discharge_current" 2

}

# Comment this function call out if modbus registers should not be written to for testing
#initializeData

echo "Getting mbmu data..."

echo -n "BMSPowerOn: "                        && fimsGet "/status/bms/BMSPowerOn"
echo -n "BMSStatus: "                         && fimsGet "/status/bms/BMSStatus"
echo -n "mbmu_soc: "                          && fimsGet "/ess/components/catl_mbmu_summary_r/mbmu_soc"
echo -n "mbmu_soh: "                          && fimsGet "/ess/components/catl_mbmu_summary_r/mbmu_soh"
echo -n "mbmu_voltage: "                      && fimsGet "/ess/components/catl_mbmu_summary_r/mbmu_voltage"
echo -n "BMSCurrent: "                        && fimsGet "/status/bms/BMSCurrent"
echo -n "mbmu_max_cell_voltage: "             && fimsGet "/ess/components/catl_mbmu_summary_r/mbmu_max_cell_voltage"
echo -n "mbmu_min_cell_voltage: "             && fimsGet "/ess/components/catl_mbmu_summary_r/mbmu_min_cell_voltage"
echo -n "mbmu_avg_cell_voltage: "             && fimsGet "/ess/components/catl_mbmu_summary_r/mbmu_avg_cell_voltage"
echo -n "mbmu_max_cell_temperature: "         && fimsGet "/ess/components/catl_mbmu_summary_r/mbmu_max_cell_temperature"
echo -n "mbmu_min_cell_temperature: "         && fimsGet "/ess/components/catl_mbmu_summary_r/mbmu_min_cell_temperature"
echo -n "mbmu_avg_cell_temperature: "         && fimsGet "/ess/components/catl_mbmu_summary_r/mbmu_avg_cell_temperature"
echo -n "bms_max_charge_allowed: "            && fimsGet "/ess/components/catl_bms_ems_r/bms_max_charge_allowed"
echo -n "bms_max_discharge_allowed: "         && fimsGet "/ess/components/catl_bms_ems_r/bms_max_discharge_allowed"
echo -n "bms_remain_charge_energy: "          && fimsGet "/ess/components/catl_bms_ems_r/bms_remain_charge_energy"
echo -n "bms_remain_discharge_energy: "       && fimsGet "/ess/components/catl_bms_ems_r/bms_remain_discharge_energy"
echo -n "num_hv_subsystem: "                  && fimsGet "/ess/components/catl_bms_ems_r/num_hv_subsystem"
echo


echo "Getting sbmu_1 data..."

echo -n "sbmu_soc: "                          && fimsGet "/ess/components/catl_sbmu_1/sbmu_soc"
echo -n "sbmu_soh: "                          && fimsGet "/ess/components/catl_sbmu_1/sbmu_soh"
echo -n "sbmu_voltage: "                      && fimsGet "/ess/components/catl_sbmu_1/sbmu_voltage"
echo -n "sbmu_max_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_1/sbmu_max_cell_voltage"
echo -n "sbmu_min_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_1/sbmu_min_cell_voltage"
echo -n "sbmu_avg_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_1/sbmu_avg_cell_voltage"
echo -n "sbmu_max_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_1/sbmu_max_cell_temperature"
echo -n "sbmu_min_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_1/sbmu_min_cell_temperature"
echo -n "sbmu_avg_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_1/sbmu_avg_cell_temperature"
echo -n "sbmu_max_charge_current: "           && fimsGet "/ess/components/catl_sbmu_1/sbmu_max_charge_current"
echo -n "sbmu_max_discharge_current: "        && fimsGet "/ess/components/catl_sbmu_1/sbmu_max_discharge_current"
echo


echo "Getting sbmu_2 data..."

echo -n "sbmu_soc: "                          && fimsGet "/ess/components/catl_sbmu_2/sbmu_soc"
echo -n "sbmu_soh: "                          && fimsGet "/ess/components/catl_sbmu_2/sbmu_soh"
echo -n "sbmu_voltage: "                      && fimsGet "/ess/components/catl_sbmu_2/sbmu_voltage"
echo -n "sbmu_max_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_2/sbmu_max_cell_voltage"
echo -n "sbmu_min_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_2/sbmu_min_cell_voltage"
echo -n "sbmu_avg_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_2/sbmu_avg_cell_voltage"
echo -n "sbmu_max_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_2/sbmu_max_cell_temperature"
echo -n "sbmu_min_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_2/sbmu_min_cell_temperature"
echo -n "sbmu_avg_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_2/sbmu_avg_cell_temperature"
echo -n "sbmu_max_charge_current: "           && fimsGet "/ess/components/catl_sbmu_2/sbmu_max_charge_current"
echo -n "sbmu_max_discharge_current: "        && fimsGet "/ess/components/catl_sbmu_2/sbmu_max_discharge_current"
echo


echo "Getting sbmu_3 data..."

echo -n "sbmu_soc: "                          && fimsGet "/ess/components/catl_sbmu_3/sbmu_soc"
echo -n "sbmu_soh: "                          && fimsGet "/ess/components/catl_sbmu_3/sbmu_soh"
echo -n "sbmu_voltage: "                      && fimsGet "/ess/components/catl_sbmu_3/sbmu_voltage"
echo -n "sbmu_max_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_3/sbmu_max_cell_voltage"
echo -n "sbmu_min_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_3/sbmu_min_cell_voltage"
echo -n "sbmu_avg_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_3/sbmu_avg_cell_voltage"
echo -n "sbmu_max_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_3/sbmu_max_cell_temperature"
echo -n "sbmu_min_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_3/sbmu_min_cell_temperature"
echo -n "sbmu_avg_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_3/sbmu_avg_cell_temperature"
echo -n "sbmu_max_charge_current: "           && fimsGet "/ess/components/catl_sbmu_3/sbmu_max_charge_current"
echo -n "sbmu_max_discharge_current: "        && fimsGet "/ess/components/catl_sbmu_3/sbmu_max_discharge_current"
echo


echo "Getting sbmu_4 data..."

echo -n "sbmu_soc: "                          && fimsGet "/ess/components/catl_sbmu_4/sbmu_soc"
echo -n "sbmu_soh: "                          && fimsGet "/ess/components/catl_sbmu_4/sbmu_soh"
echo -n "sbmu_voltage: "                      && fimsGet "/ess/components/catl_sbmu_4/sbmu_voltage"
echo -n "sbmu_max_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_4/sbmu_max_cell_voltage"
echo -n "sbmu_min_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_4/sbmu_min_cell_voltage"
echo -n "sbmu_avg_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_4/sbmu_avg_cell_voltage"
echo -n "sbmu_max_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_4/sbmu_max_cell_temperature"
echo -n "sbmu_min_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_4/sbmu_min_cell_temperature"
echo -n "sbmu_avg_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_4/sbmu_avg_cell_temperature"
echo -n "sbmu_max_charge_current: "           && fimsGet "/ess/components/catl_sbmu_4/sbmu_max_charge_current"
echo -n "sbmu_max_discharge_current: "        && fimsGet "/ess/components/catl_sbmu_4/sbmu_max_discharge_current"
echo


echo "Getting sbmu_5 data..."

echo -n "sbmu_soc: "                          && fimsGet "/ess/components/catl_sbmu_5/sbmu_soc"
echo -n "sbmu_soh: "                          && fimsGet "/ess/components/catl_sbmu_5/sbmu_soh"
echo -n "sbmu_voltage: "                      && fimsGet "/ess/components/catl_sbmu_5/sbmu_voltage"
echo -n "sbmu_max_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_5/sbmu_max_cell_voltage"
echo -n "sbmu_min_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_5/sbmu_min_cell_voltage"
echo -n "sbmu_avg_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_5/sbmu_avg_cell_voltage"
echo -n "sbmu_max_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_5/sbmu_max_cell_temperature"
echo -n "sbmu_min_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_5/sbmu_min_cell_temperature"
echo -n "sbmu_avg_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_5/sbmu_avg_cell_temperature"
echo -n "sbmu_max_charge_current: "           && fimsGet "/ess/components/catl_sbmu_5/sbmu_max_charge_current"
echo -n "sbmu_max_discharge_current: "        && fimsGet "/ess/components/catl_sbmu_5/sbmu_max_discharge_current"
echo


echo "Getting sbmu_6 data..."

echo -n "sbmu_soc: "                          && fimsGet "/ess/components/catl_sbmu_6/sbmu_soc"
echo -n "sbmu_soh: "                          && fimsGet "/ess/components/catl_sbmu_6/sbmu_soh"
echo -n "sbmu_voltage: "                      && fimsGet "/ess/components/catl_sbmu_6/sbmu_voltage"
echo -n "sbmu_max_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_6/sbmu_max_cell_voltage"
echo -n "sbmu_min_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_6/sbmu_min_cell_voltage"
echo -n "sbmu_avg_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_6/sbmu_avg_cell_voltage"
echo -n "sbmu_max_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_6/sbmu_max_cell_temperature"
echo -n "sbmu_min_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_6/sbmu_min_cell_temperature"
echo -n "sbmu_avg_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_6/sbmu_avg_cell_temperature"
echo -n "sbmu_max_charge_current: "           && fimsGet "/ess/components/catl_sbmu_6/sbmu_max_charge_current"
echo -n "sbmu_max_discharge_current: "        && fimsGet "/ess/components/catl_sbmu_6/sbmu_max_discharge_current"
echo


echo "Getting sbmu_7 data..."

echo -n "sbmu_soc: "                          && fimsGet "/ess/components/catl_sbmu_7/sbmu_soc"
echo -n "sbmu_soh: "                          && fimsGet "/ess/components/catl_sbmu_7/sbmu_soh"
echo -n "sbmu_voltage: "                      && fimsGet "/ess/components/catl_sbmu_7/sbmu_voltage"
echo -n "sbmu_max_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_7/sbmu_max_cell_voltage"
echo -n "sbmu_min_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_7/sbmu_min_cell_voltage"
echo -n "sbmu_avg_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_7/sbmu_avg_cell_voltage"
echo -n "sbmu_max_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_7/sbmu_max_cell_temperature"
echo -n "sbmu_min_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_7/sbmu_min_cell_temperature"
echo -n "sbmu_avg_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_7/sbmu_avg_cell_temperature"
echo -n "sbmu_max_charge_current: "           && fimsGet "/ess/components/catl_sbmu_7/sbmu_max_charge_current"
echo -n "sbmu_max_discharge_current: "        && fimsGet "/ess/components/catl_sbmu_7/sbmu_max_discharge_current"
echo


echo "Getting sbmu_8 data..."

echo -n "sbmu_soc: "                          && fimsGet "/ess/components/catl_sbmu_8/sbmu_soc"
echo -n "sbmu_soh: "                          && fimsGet "/ess/components/catl_sbmu_8/sbmu_soh"
echo -n "sbmu_voltage: "                      && fimsGet "/ess/components/catl_sbmu_8/sbmu_voltage"
echo -n "sbmu_max_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_8/sbmu_max_cell_voltage"
echo -n "sbmu_min_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_8/sbmu_min_cell_voltage"
echo -n "sbmu_avg_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_8/sbmu_avg_cell_voltage"
echo -n "sbmu_max_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_8/sbmu_max_cell_temperature"
echo -n "sbmu_min_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_8/sbmu_min_cell_temperature"
echo -n "sbmu_avg_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_8/sbmu_avg_cell_temperature"
echo -n "sbmu_max_charge_current: "           && fimsGet "/ess/components/catl_sbmu_8/sbmu_max_charge_current"
echo -n "sbmu_max_discharge_current: "        && fimsGet "/ess/components/catl_sbmu_8/sbmu_max_discharge_current"
echo


echo "Getting sbmu_9 data..."

echo -n "sbmu_soc: "                          && fimsGet "/ess/components/catl_sbmu_9/sbmu_soc"
echo -n "sbmu_soh: "                          && fimsGet "/ess/components/catl_sbmu_9/sbmu_soh"
echo -n "sbmu_voltage: "                      && fimsGet "/ess/components/catl_sbmu_9/sbmu_voltage"
echo -n "sbmu_max_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_9/sbmu_max_cell_voltage"
echo -n "sbmu_min_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_9/sbmu_min_cell_voltage"
echo -n "sbmu_avg_cell_voltage: "             && fimsGet "/ess/components/catl_sbmu_9/sbmu_avg_cell_voltage"
echo -n "sbmu_max_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_9/sbmu_max_cell_temperature"
echo -n "sbmu_min_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_9/sbmu_min_cell_temperature"
echo -n "sbmu_avg_cell_temperature: "         && fimsGet "/ess/components/catl_sbmu_9/sbmu_avg_cell_temperature"
echo -n "sbmu_max_charge_current: "           && fimsGet "/ess/components/catl_sbmu_9/sbmu_max_charge_current"
echo -n "sbmu_max_discharge_current: "        && fimsGet "/ess/components/catl_sbmu_9/sbmu_max_discharge_current"
echo
