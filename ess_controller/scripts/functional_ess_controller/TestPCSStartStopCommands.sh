# Function to display countdown
countdown() {
    local seconds=$1
    while [ $seconds -gt 0 ]; do
        echo -ne "$seconds seconds\033[0K\r"
        sleep 1
        ((seconds--))
    done
}


# Start

/usr/local/bin/fims_send -m set -r /$$ -u /components/sungrow_pcs_controls/state_cmd 5
sleep 1
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/accept_commands false
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/conditions_met false

read -n1 -p "Press any key to start PCS Start Test 1";echo
/usr/local/bin/fims_send -m set -r /$$ -u /assets/pcs/summary/start true
echo "Expected Failure on CheckCmd (Start) in"
countdown 10

/usr/local/bin/fims_send -m set -r /$$ -u /components/sungrow_pcs_controls/state_cmd 5
read -n1 -p "Press any key to start PCS Start Test 2";echo
/usr/local/bin/fims_send -m set -r /$$ -u /assets/pcs/summary/start true
echo "Setting accept_commands to true in"
countdown 7
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/accept_commands true
echo "Expected success on CheckCmd (Start)"
echo "Expected failure on SendCmd (VerifyStart) in"
countdown 5

/usr/local/bin/fims_send -m set -r /$$ -u /components/sungrow_pcs_controls/state_cmd 5
read -n1 -p "Press any key to start PCS Start Test 3";echo
/usr/local/bin/fims_send -m set -r /$$ -u /assets/pcs/summary/start true
echo "Expected success on CheckCmd (Start) in"
countdown 5
echo "Setting conditions_met to true in"
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/conditions_met true
countdown 3
echo "Expected success on SendCmd (VerifyStart)"



# Stop

/usr/local/bin/fims_send -m set -r /$$ -u /components/sungrow_pcs_controls/state_cmd 7
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/accept_commands false
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/conditions_met false

/usr/local/bin/fims_send -m set -r /$$ -u /components/sungrow_pcs_controls/state_cmd 7
sleep 1
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/accept_commands false
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/conditions_met false

read -n1 -p "Press any key to start PCS Stop Test 1";echo
/usr/local/bin/fims_send -m set -r /$$ -u /assets/pcs/summary/stop true
echo "Expected Failure on CheckCmd (Stop) in"
countdown 10

/usr/local/bin/fims_send -m set -r /$$ -u /components/sungrow_pcs_controls/state_cmd 7
read -n1 -p "Press any key to start PCS Stop Test 2";echo
/usr/local/bin/fims_send -m set -r /$$ -u /assets/pcs/summary/stop true
echo "Setting accept_commands to true in"
countdown 7
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/accept_commands true
echo "Expected success on CheckCmd (Stop)"
echo "Expected failure on SendCmd (VerifyStop) in"
countdown 5

/usr/local/bin/fims_send -m set -r /$$ -u /components/sungrow_pcs_controls/state_cmd 7
read -n1 -p "Press any key to start PCS Stop Test 3";echo
/usr/local/bin/fims_send -m set -r /$$ -u /assets/pcs/summary/stop true
echo "Expected success on CheckCmd (Stop) in"
countdown 5
echo "Setting conditions_met to true in"
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/conditions_met true
countdown 3
echo "Expected success on SendCmd (VerifyStop)"

# Standby

/usr/local/bin/fims_send -m set -r /$$ -u /components/sungrow_pcs_controls/state_cmd 5
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/accept_commands false
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/conditions_met false

/usr/local/bin/fims_send -m set -r /$$ -u /components/sungrow_pcs_controls/state_cmd 5
sleep 1
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/accept_commands false
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/conditions_met false

read -n1 -p "Press any key to start PCS Standby Test 1";echo
/usr/local/bin/fims_send -m set -r /$$ -u /assets/pcs/summary/standby true
echo "Expected Failure on CheckCmd (Standby) in"
countdown 10

/usr/local/bin/fims_send -m set -r /$$ -u /components/sungrow_pcs_controls/state_cmd 5
read -n1 -p "Press any key to start PCS Standby Test 2";echo
/usr/local/bin/fims_send -m set -r /$$ -u /assets/pcs/summary/standby true
echo "Setting accept_commands to true in"
countdown 7
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/accept_commands true
echo "Expected success on CheckCmd (Standby)"
echo "Expected failure on SendCmd (VerifyStandby) in"
countdown 5

/usr/local/bin/fims_send -m set -r /$$ -u /components/sungrow_pcs_controls/state_cmd 5
read -n1 -p "Press any key to start PCS Standby Test 3";echo
/usr/local/bin/fims_send -m set -r /$$ -u /assets/pcs/summary/standby true
echo "Expected success on CheckCmd (Standby) in"
countdown 5
echo "Setting conditions_met to true in"
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/conditions_met true
countdown 3
echo "Expected success on SendCmd (VerifyStandby)"