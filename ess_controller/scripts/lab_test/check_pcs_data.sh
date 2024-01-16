#!/bin/sh 

# Script used to check pcs data values from modbus interface

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
    fimsPub "/components/pcsm_internal_visualization/current_status" 1
    fimsPub "/components/pcsm_grid_visualization/active_power" 15
    fimsPub "/components/pcsm_grid_visualization/reactive_power" 12
    fimsPub "/components/pcsm_grid_visualization/apparent_power" 20
    fimsPub "/components/pcsm_control/p_p_reference" 10
    fimsPub "/components/pcsm_control:q_q_reference" 20
    fimsPub "/components/pcsm_grid_visualization/grid_voltage_rs" 15
    fimsPub "/components/pcsm_grid_visualization/grid_voltage_st" 5
    fimsPub "/components/pcsm_grid_visualization/grid_voltage_tr" 10
    fimsPub "/components/pcsm_grid_visualization/grid_current_1" 4
    fimsPub "/components/pcsm_grid_visualization/grid_current_2" 6
    fimsPub "/components/pcsm_grid_visualization/grid_current_3" 8
    fimsPub "/components/pcsm_grid_visualization/grid_frequency" 54
    fimsPub "/components/pcsm_dc_inputs/vdc_bus_1" 23
    fimsPub "/components/pcsm_dc_inputs/dc_1_i_input" 8
}

# Comment this function call out if modbus registers should not be written to for testing
#initializeData

echo "Getting pcs data..."

echo -n "status: "                  && fimsGet "/ess/components/pcsm_internal_visualization/current_status"
echo -n "active_power: "            && fimsGet "/ess/components/pcsm_grid_visualization/active_power"
echo -n "reactive_power: "          && fimsGet "/ess/components/pcsm_grid_visualization/reactive_power"
echo -n "apparent_power: "          && fimsGet "/ess/components/pcsm_grid_visualization/apparent_power"
echo -n "active_power_command: "    && fimsGet "/ess/components/pcsm_control/p_p_reference"
echo -n "reactive_power_command: "  && fimsGet "/ess/components/pcsm_control:q_q_reference"
echo -n "voltage_l1_l2: "           && fimsGet "/ess/components/pcsm_grid_visualization/grid_voltage_rs"
echo -n "voltage_l2_l3: "           && fimsGet "/ess/components/pcsm_grid_visualization/grid_voltage_st"
echo -n "voltage_l3_l1: "           && fimsGet "/ess/components/pcsm_grid_visualization/grid_voltage_tr"
echo -n "current_l1: "              && fimsGet "/ess/components/pcsm_grid_visualization/grid_current_1"
echo -n "current_l2: "              && fimsGet "/ess/components/pcsm_grid_visualization/grid_current_2"
echo -n "current_l3: "              && fimsGet "/ess/components/pcsm_grid_visualization/grid_current_3"
echo -n "frequency: "               && fimsGet "/ess/components/pcsm_grid_visualization/grid_frequency"
echo -n "voltage_dc: "              && fimsGet "/ess/components/pcsm_dc_inputs/vdc_bus_1"
echo -n "current_dc: "              && fimsGet "/ess/components/pcsm_dc_inputs/dc_1_i_input"