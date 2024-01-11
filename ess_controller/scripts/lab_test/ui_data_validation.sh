#!/bin/sh

# Small test script that will populate the modbus registers in CATL BMS and PE PCS
# Once populated, the values should be seen in the ESS Controller UI and Site Controller UI

fSend=/usr/local/bin/fims/fims_send

# Helper function for doing a fims pub
#
# $1 = uri and/or variable name (ex.: /components/catl_mbmu_control_r or /components/catl_mbmu_control_r/mbmu_max_cell_voltage)
# $2 = value to set
fimsPub()
{
    $fSend -m pub -u $1 $2
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
    fimsPub "/components/catl_mbmu_summary_r/mbmu_current" 20003
    fimsPub "/components/catl_mbmu_summary_r/mbmu_max_cell_voltage" 15
    fimsPub "/components/catl_mbmu_summary_r/mbmu_min_cell_voltage" 5
    fimsPub "/components/catl_mbmu_summary_r/mbmu_avg_cell_voltage" 10
    fimsPub "/components/catl_mbmu_summary_r/mbmu_max_cell_temperature" 60
    fimsPub "/components/catl_mbmu_summary_r/mbmu_min_cell_temperature" 20
    fimsPub "/components/catl_mbmu_summary_r/mbmu_avg_cell_temperature" 35
    fimsPub "/components/catl_bms_ems_r/bms_max_charge_allowed" 540000
    fimsPub "/components/catl_bms_ems_r/bms_max_discharge_allowed" 230000
    fimsPub "/components/catl_bms_ems_r/bms_remain_charge_energy" 4300
    fimsPub "/components/catl_bms_ems_r/bms_remain_discharge_energy" 1700
    fimsPub "/components/catl_bms_ems_r/num_hv_subsystem" 9

    # Initialize pcs data
    fimsPub "/components/pcsm_internal_visualization/current_status" 6
    fimsPub "/components/pcsm_grid_visualization/active_power" 15000
    fimsPub "/components/pcsm_grid_visualization/reactive_power" 12000
    fimsPub "/components/pcsm_grid_visualization/apparent_power" 20000
    fimsPub "/components/pcsm_control/p_p_reference" 10
    fimsPub "/components/pcsm_control/q_q_reference" 20
    fimsPub "/components/pcsm_grid_visualization/grid_voltage_rs" 15
    fimsPub "/components/pcsm_grid_visualization/grid_voltage_st" 5
    fimsPub "/components/pcsm_grid_visualization/grid_voltage_tr" 10
    fimsPub "/components/pcsm_grid_visualization/grid_current_1" 4
    fimsPub "/components/pcsm_grid_visualization/grid_current_2" 6
    fimsPub "/components/pcsm_grid_visualization/grid_current_3" 8
    fimsPub "/components/pcsm_grid_visualization/grid_frequency" 54
    fimsPub "/components/pcsm_dc_inputs/vdc_bus_1" 23
    fimsPub "/components/pcsm_dc_inputs/dc_total_i_input" 8
}

initializeData

echo "Initialization done. Check ESS Controller and Site Controller UI."