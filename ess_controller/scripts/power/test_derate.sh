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
./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":6}'
./fims_send -m set -r /me -u /ess/components/catl_sbmu_1 '{"sbmu_master_positive":1}'
./fims_send -m set -r /me -u /ess/components/catl_sbmu_1 '{"sbmu_master_negative":1}'
echo -e "Initializing variables based on lab readings..."
./fims_send -m set -u /ess/controls/ess '{"ActivePowerSetpoint":0}'
./fims_send -m set -u /ess/controls/ess '{"ReactivePowerSetpoint":0}'
./fims_send -m set -u /ess/controls/ess '{"PowerPriority":"value":"p"}}'
./fims_send -m set -u /ess/status/bms '{"MaxBMSChargeCurrent":-2335}'
./fims_send -m set -u /ess/status/bms '{"MaxBMSDischargeCurrent":2334}'
./fims_send -m set -u /ess/status/bms '{"BMSVoltage":1436.2}'
./fims_send -m set -u /ess/status/pcs '{"MaxPCSActivePower":100}'
./fims_send -m set -u /ess/status/pcs '{"MaxPCSReactivePower":100}'
./fims_send -m set -u /ess/status/pcs '{"MaxPCSApparentPower":100}'
./fims_send -m set -u /ess/status/pcs '{"NumRunningModules":6}'
./fims_send -m set -u /ess/components/catl_sbmu_1/sbmu_current 2000
./fims_send -m set -u /ess/components/catl_sbmu_2/sbmu_current 2000
./fims_send -m set -u /ess/components/catl_sbmu_3/sbmu_current 2000
./fims_send -m set -u /ess/components/catl_sbmu_4/sbmu_current 2000
./fims_send -m set -u /ess/components/catl_sbmu_5/sbmu_current 2000
./fims_send -m set -u /ess/components/catl_sbmu_6/sbmu_current 2000
./fims_send -m set -u /ess/components/catl_sbmu_7/sbmu_current 2000
./fims_send -m set -u /ess/components/catl_sbmu_8/sbmu_current 2000
./fims_send -m set -u /ess/components/catl_sbmu_9/sbmu_current 2000
./fims_send -m set -u /ess/components/catl_mbmu_summary_r/mbmu_current 20000
./fims_send -m set -u /ess/components/catl_bms_ems_r '{"bms_poweron":1}'
./fims_send -m set -u /ess/components/catl_bms_ems_r '{"num_hv_subsystem":9}'
./fims_send -m set -u /ess/status/sbmu_1/SBMUMaxDischargeCurrent 2334
./fims_send -m set -u /ess/status/sbmu_1/SBMUMaxChargeCurrent '{"value":-2334}'
./fims_send -m get -r /me -u /ess/full/status/bms/OverCurrentD1 | jq
sleep 1 # give enough time for setpoints to be initialize

./fims_send -m set -r /me -u /ess/controls/ess/ActivePowerSetpoint 3510
sleep 1
echo -e "Overcurrent high"
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrentCheckDerate 2334.1
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrent 2334.1
sleep 3
echo -e "Overcurrent low"
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrentCheckDerate 2311
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrent 2311
sleep 3
echo -e "Normal current range"
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrentCheckDerate 2220
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrent 2220
sleep 3
echo -e "Overcurrent low again"
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrentCheckDerate 2311
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrent 2311
sleep 1
echo -e "MaxDeratedDischargeCmd:"
./fims_send -m get -r /me -u /ess/status/pcs/MaxDeratedDischargeCmd | jq
echo -e "MaxDeratedChargeCmd:"
./fims_send -m get -r /me -u /ess/status/pcs/MaxDeratedChargeCmd | jq
sleep 3
echo -e "Lowering current limit"
./fims_send -m set -u /ess/status/sbmu_1/SBMUMaxDischargeCurrent 2310
sleep 3
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrentCheckDerate 2210
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrent 2210
sleep 3
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrentCheckDerate 2264
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrent 2264
sleep 1
echo -e "MaxDeratedDischargeCmd:"
./fims_send -m get -r /me -u /ess/status/pcs/MaxDeratedDischargeCmd | jq
echo -e "MaxDeratedChargeCmd:"
./fims_send -m get -r /me -u /ess/status/pcs/MaxDeratedChargeCmd | jq
sleep 3
./fims_send -m set -r /me -u /ess/controls/ess/ActivePowerSetpoint 351

./fims_send -m set -r /me -u /ess/controls/ess/ActivePowerSetpoint '{"value":-3510}'
sleep 1
echo -e "Overcurrent high"
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrentCheckDerate '{"value":-2334.1}'
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrent '{"value":-2334.1}'
sleep 3
echo -e "Overcurrent low"
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrentCheckDerate '{"value":-2311}'
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrent '{"value":-2311}'
sleep 3
echo -e "Normal current range"
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrentCheckDerate '{"value":-2220}'
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrent '{"value":-2220}'
sleep 3
echo -e "Overcurrent low again"
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrentCheckDerate '{"value":-2311}'
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrent '{"value":-2311}'
sleep 1
echo -e "MaxDeratedDischargeCmd:"
./fims_send -m get -r /me -u /ess/status/pcs/MaxDeratedDischargeCmd | jq
echo -e "MaxDeratedChargeCmd:"
./fims_send -m get -r /me -u /ess/status/pcs/MaxDeratedChargeCmd | jq
sleep 3
echo -e "Lowering current limit"
./fims_send -m set -u /ess/status/sbmu_1/SBMUMaxChargeCurrent '{"value":-2310}'
sleep 3
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrentCheckDerate '{"value":-2210}'
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrent '{"value":-2210}'
sleep 3
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrentCheckDerate '{"value":-2264}'
./fims_send -m set -r /me -u /ess/status/sbmu_1/SBMUCurrent '{"value":-2264}'
sleep 1
echo -e "MaxDeratedDischargeCmd:"
./fims_send -m get -r /me -u /ess/status/pcs/MaxDeratedDischargeCmd | jq
echo -e "MaxDeratedChargeCmd:"
./fims_send -m get -r /me -u /ess/status/pcs/MaxDeratedChargeCmd | jq
sleep 3

