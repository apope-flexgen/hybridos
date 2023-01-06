startup=true
shutdown=false
standby=false
gpiofault=false
cd /usr/local/bin/fims
./fims_send -m set -r /me -u /ess/assets/pcs/summary/start false
./fims_send -m set -r /me -u /ess/assets/bms/summary/start false
./fims_send -m set -r /me -u /ess/assets/pcs/summary/stop false
./fims_send -m set -r /me -u /ess/assets/bms/summary/stop false
./fims_send -m set -r /me -u /ess/components/gpio/EStop false
./fims_send -m set -r /me -u /ess/components/pcs_registers_fast/vdc_bus_1 1400
./fims_send -m set -r /me -u /ess/components/pcs_registers_fast/num_running_modules 6
./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r/bms_heartbeat@EnableStateCheck false
./fims_send -m set -r /me -u /ess/components/pcs_registers_fast/seconds@EnableStateCheck false
./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r/num_hv_subsystem@EnableFaultCheck false
./fims_send -m set -r /me -u /ess/status/bms/BMSMinCellTemp@EnableFaultCheck false
./fims_send -m set -r /me -u /ess/assets/bms/summary/maint_mode true
./fims_send -m set -r /me -u /ess/assets/pcs/summary/maint_mode true
./fims_send -m set -r /me -u /ess/status/pcs/MaxPCSActivePower 100
./fims_send -m set -r /me -u /ess/status/pcs/MaxPCSReactivePower 100
./fims_send -m set -r /me -u /ess/status/pcs/MaxPCSApparentPower 100
./fims_send -m set -r /me -u /ess/controls/ess/ActivePowerSetpoint 0
sleep 0.2
./fims_send -m set -r /me -u /ess/site/ess_hs/clear_faults 1
sleep 0.2
./fims_send -m set -r /me -u /ess/site/ess_hs/clear_faults 0
if [ "$startup" = true ] ; then
    echo "PCS Off, BMS Off, Batteries normal"
    ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":2}'                 # PCS is ready
    ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_status":1}'   # BMS is powered off batteries normal
    ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":0}'
    echo "Waiting 5 seconds"
    sleep 1
    echo "BMS Start command"
    ./fims_send -m set -r /me -u /ess/assets/bms/summary/start true
    sleep 1
    echo "BMS Turning On"
    ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":1}'
    echo "Waiting 5 seconds"
    sleep 5
    echo "PCS Start command, wait 5 seconds"
    ./fims_send -m set -r /me -u /ess/assets/pcs/summary/start true
    sleep 5
    echo "PCS Going to Standby"
    ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":3}'
    sleep 1
    echo "PCS Going to Self-Diagnosis (90 seconds irl)"
    ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":18}'
    sleep 5
    echo "PCS Going to Precharge"
    ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":3}'
    sleep 3
    echo "PCS Going to Ready"
    ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":4}'
    sleep 5
    echo "PCS Going to Running"
    ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":6}'
    sleep 5
fi
if [ "$shutdown" = true ] ; then
    echo "PCS Running, BMS On, Batteries normal"
    ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_status":1}'
    ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":1}'
    ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":6}'
    ./fims_send -m set -r /me -u /ess/status/bms/BMSCurrentCheckStop 100
    ./fims_send -m set -r /me -u /ess/controls/ess/ActivePowerSetpoint 3000
    echo "Waiting 3 seconds"
    sleep 3
    echo "PCS Stop Command"
    ./fims_send -m set -r /me -u /ess/assets/pcs/summary/shutdown true
    echo "Waiting 5 seconds"
    sleep 5
    echo "BMS Current going low"
    ./fims_send -m set -r /me -u /ess/status/bms/BMSCurrentCheckStop 20
    sleep 5
    echo "PCS Going to Stop"
    ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":7}'
    sleep 3
    echo "PCS Going to Ready"
    ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":4}'
    sleep 3
    echo "BMS Current going low"
    ./fims_send -m set -r /me -u /ess/status/bms/BMSCurrentCheckStop 20
    sleep 5
    echo "PCS Going to Off"
    ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":2}'
    sleep 3
    echo "PCS Going to Discharge"
    ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":8}'
    sleep 5
    echo "PCS Going to Off"
    ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":2}'
    sleep 3
fi
if [ "$standby" = true ] ; then
    echo "PCS Running, BMS On, Batteries normal"
    ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_status":1}'
    ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":1}'
    ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":6}'
    ./fims_send -m set -r /me -u /ess/status/bms/BMSCurrentCheckStop 100
    echo "Waiting 3 seconds"
    sleep 3
    echo "PCS Stop Command"
    ./fims_send -m set -r /me -u /ess/assets/pcs/summary/stop true
    sleep 5
    echo "PCS Going to Stop"
    ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":7}'
    sleep 3
    echo "PCS Going to Ready"
    ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":4}'
    sleep 3
    echo "BMS Current going low"
    ./fims_send -m set -r /me -u /ess/status/bms/BMSCurrentCheckStop 20
    sleep 1
fi
if [ "$gpiofault" = true ] ; then
    echo "PCS Running, BMS On, Batteries normal"
    ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_status":1}'
    ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":1}'
    ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":6}'
    ./fims_send -m set -r /me -u /ess/status/bms/BMSCurrentCheckStop 100
    echo "Waiting 3 seconds"
    sleep 3
    echo "GPIO Fault"
    ./fims_send -m set -r /me -u /ess/components/gpio/EStop true
    sleep 5
    echo "PCS Going to Stop"
    ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":7}'
    sleep 3
    echo "PCS Going to Ready"
    ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":4}'
    sleep 3
    echo "BMS Current going low"
    ./fims_send -m set -r /me -u /ess/status/bms/BMSCurrentCheckStop 20
    sleep 5
    echo "PCS Going to Off"
    ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":2}'
    sleep 3
    echo "PCS Going to Discharge"
    ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":8}'
    sleep 5
    echo "PCS Going to Off"
    ./fims_send -m set -r /me -u /ess/components/pcs_registers_fast '{"current_status":2}'
    sleep 3
fi
