cd /usr/local/bin/fims
./fims_send -m set -r /me -u /ess/components/pcs_registers_fast/vdc_bus_1 1400
./fims_send -m set -r /me -u /ess/components/pcs_registers_fast/num_running_modules 6
./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r/bms_heartbeat@EnableStateCheck false
./fims_send -m set -r /me -u /ess/components/pcs_registers_fast/seconds@EnableStateCheck false
./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r/num_hv_subsystem@EnableFaultCheck false
./fims_send -m set -r /me -u /ess/status/bms/BMSMinCellTemp@EnableFaultCheck false
./fims_send -m set -r /me -u /ess/assets/bms/summary/maint_mode true
./fims_send -m set -r /me -u /ess/assets/pcs/summary/maint_mode true
sleep 0.2
./fims_send -m set -r /me -u /ess/site/ess_hs/clear_faults 1
sleep 0.2
./fims_send -m set -r /me -u /ess/site/ess_hs/clear_faults 0
./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":1}'
./fims_send -m set -u /ess/components/catl_bms_ems_r '{"num_hv_subsystem":9}'
./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":6}'
./fims_send -m set -r /me -u /ess/components/catl_sbmu_1 '{"sbmu_master_positive":1}'
./fims_send -m set -r /me -u /ess/components/catl_sbmu_1 '{"sbmu_master_negative":1}'
./fims_send -m set -r /me -u /ess/components/catl_sbmu_2 '{"sbmu_master_positive":2}'
./fims_send -m set -r /me -u /ess/components/catl_sbmu_2 '{"sbmu_master_negative":2}'
./fims_send -m set -r /me -u /ess/components/catl_sbmu_3 '{"sbmu_master_positive":3}'
./fims_send -m set -r /me -u /ess/components/catl_sbmu_3 '{"sbmu_master_negative":3}'
./fims_send -m set -r /me -u /ess/components/catl_sbmu_4 '{"sbmu_master_positive":4}'
./fims_send -m set -r /me -u /ess/components/catl_sbmu_4 '{"sbmu_master_negative":4}'
./fims_send -m set -r /me -u /ess/components/catl_sbmu_5 '{"sbmu_master_positive":5}'
./fims_send -m set -r /me -u /ess/components/catl_sbmu_5 '{"sbmu_master_negative":5}'
./fims_send -m set -r /me -u /ess/components/catl_sbmu_6 '{"sbmu_master_positive":6}'
./fims_send -m set -r /me -u /ess/components/catl_sbmu_6 '{"sbmu_master_negative":6}'
./fims_send -m set -r /me -u /ess/components/catl_sbmu_7 '{"sbmu_master_positive":7}'
./fims_send -m set -r /me -u /ess/components/catl_sbmu_7 '{"sbmu_master_negative":7}'
./fims_send -m set -r /me -u /ess/components/catl_sbmu_8 '{"sbmu_master_positive":8}'
./fims_send -m set -r /me -u /ess/components/catl_sbmu_8 '{"sbmu_master_negative":8}'
./fims_send -m set -r /me -u /ess/components/catl_sbmu_9 '{"sbmu_master_positive":9}'
./fims_send -m set -r /me -u /ess/components/catl_sbmu_9 '{"sbmu_master_negative":9}'
echo -e "Initializing variables based on lab readings..."
./fims_send -m set -u /ess/controls/ess '{"ActivePowerSetpoint":0}'
./fims_send -m set -u /ess/controls/ess '{"ReactivePowerSetpoint":0}'
./fims_send -m set -u /ess/controls/ess '{"PowerPriority":"value":"p"}}'
./fims_send -m set -u /ess/status/bms '{"MaxBMSChargeCurrent":-2335}'
./fims_send -m set -u /ess/status/bms '{"MaxBMSDischargeCurrent":2526.03}'
./fims_send -m set -u /ess/status/bms '{"BMSVoltage":1327}'
./fims_send -m set -u /ess/status/pcs '{"MaxPCSActivePower":100}'
./fims_send -m set -u /ess/status/pcs '{"MaxPCSReactivePower":100}'
./fims_send -m set -u /ess/status/pcs '{"MaxPCSApparentPower":100}'
./fims_send -m set -u /ess/status/pcs '{"NumRunningModules":6}'
./fims_send -m set -u /ess/status/sbmu_1/SBMUMaxDischargeCurrent 2334
./fims_send -m set -u /ess/status/sbmu_2/SBMUMaxDischargeCurrent 2334
./fims_send -m set -u /ess/status/sbmu_3/SBMUMaxDischargeCurrent 2334
./fims_send -m set -u /ess/status/sbmu_4/SBMUMaxDischargeCurrent 2334
./fims_send -m set -u /ess/status/sbmu_5/SBMUMaxDischargeCurrent 2334
./fims_send -m set -u /ess/status/sbmu_6/SBMUMaxDischargeCurrent 2334
./fims_send -m set -u /ess/status/sbmu_7/SBMUMaxDischargeCurrent 2334
./fims_send -m set -u /ess/status/sbmu_8/SBMUMaxDischargeCurrent 2334
./fims_send -m set -u /ess/status/sbmu_9/SBMUMaxDischargeCurrent 2334
./fims_send -m set -u /ess/status/sbmu_1/SBMUMaxChargeCurrent '{"value":-2334}'
./fims_send -m set -u /ess/status/sbmu_2/SBMUMaxChargeCurrent '{"value":-2334}'
./fims_send -m set -u /ess/status/sbmu_3/SBMUMaxChargeCurrent '{"value":-2334}'
./fims_send -m set -u /ess/status/sbmu_4/SBMUMaxChargeCurrent '{"value":-2334}'
./fims_send -m set -u /ess/status/sbmu_5/SBMUMaxChargeCurrent '{"value":-2334}'
./fims_send -m set -u /ess/status/sbmu_6/SBMUMaxChargeCurrent '{"value":-2334}'
./fims_send -m set -u /ess/status/sbmu_7/SBMUMaxChargeCurrent '{"value":-2334}'
./fims_send -m set -u /ess/status/sbmu_8/SBMUMaxChargeCurrent '{"value":-2334}'
./fims_send -m set -u /ess/status/sbmu_9/SBMUMaxChargeCurrent '{"value":-2334}'
./fims_send -m set -u /ess/components/catl_sbmu_1/sbmu_current 2260.44
./fims_send -m set -u /ess/components/catl_sbmu_2/sbmu_current 2260.44
./fims_send -m set -u /ess/components/catl_sbmu_3/sbmu_current 2260.44
./fims_send -m set -u /ess/components/catl_sbmu_4/sbmu_current 2271.8
./fims_send -m set -u /ess/components/catl_sbmu_5/sbmu_current 2260.44
./fims_send -m set -u /ess/components/catl_sbmu_6/sbmu_current 2260.44
./fims_send -m set -u /ess/components/catl_sbmu_7/sbmu_current 2260.44
./fims_send -m set -u /ess/components/catl_sbmu_8/sbmu_current 2249.8
./fims_send -m set -u /ess/components/catl_sbmu_9/sbmu_current 2260.44
./fims_send -m set -u /ess/status/bms/BMSCurrent 2343.96
sleep 0.2 # give enough time for setpoints to be initialize

sleep 1
echo -e "Max Discharge Power Est:"
./fims_send -m get -r /me -u /ess/status/bms/MaxBMSDischargePowerEst | jq
echo -e "Max Charge Power Est:"
./fims_send -m get -r /me -u /ess/status/bms/MaxBMSChargePowerEst | jq
sleep 2

./fims_send -m set -u /ess/status/bms '{"MaxBMSChargeCurrent":-2358}'
./fims_send -m set -u /ess/status/bms '{"BMSVoltage":1421}'
./fims_send -m set -u /ess/components/catl_sbmu_1/sbmu_current 1889
./fims_send -m set -u /ess/components/catl_sbmu_2/sbmu_current 1890
./fims_send -m set -u /ess/components/catl_sbmu_3/sbmu_current 1892
./fims_send -m set -u /ess/components/catl_sbmu_4/sbmu_current 1856
./fims_send -m set -u /ess/components/catl_sbmu_5/sbmu_current 1889
./fims_send -m set -u /ess/components/catl_sbmu_6/sbmu_current 1889
./fims_send -m set -u /ess/components/catl_sbmu_7/sbmu_current 1896
./fims_send -m set -u /ess/components/catl_sbmu_8/sbmu_current 1887
./fims_send -m set -u /ess/components/catl_sbmu_9/sbmu_current 1896
./fims_send -m set -u /ess/status/bms/BMSCurrent '{"value":-1016}'
sleep 0.2 # give enough time for setpoints to be initialize

sleep 1
echo -e "Max Discharge Power Est:"
./fims_send -m get -r /me -u /ess/status/bms/MaxBMSDischargePowerEst | jq
echo -e "Max Charge Power Est:"
./fims_send -m get -r /me -u /ess/status/bms/MaxBMSChargePowerEst | jq
sleep 3

./fims_send -m set -u /ess/components/catl_sbmu_1/sbmu_current 2000
./fims_send -m set -u /ess/components/catl_sbmu_2/sbmu_current 2000
./fims_send -m set -u /ess/components/catl_sbmu_3/sbmu_current 2000
./fims_send -m set -u /ess/components/catl_sbmu_4/sbmu_current 2000
./fims_send -m set -u /ess/components/catl_sbmu_5/sbmu_current 2000
./fims_send -m set -u /ess/components/catl_sbmu_6/sbmu_current 2000
./fims_send -m set -u /ess/components/catl_sbmu_7/sbmu_current 2000
./fims_send -m set -u /ess/components/catl_sbmu_8/sbmu_current 2000
./fims_send -m set -u /ess/components/catl_sbmu_9/sbmu_current 2000
./fims_send -m set -u /ess/status/bms/BMSCurrent 0