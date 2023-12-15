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

sleep 1
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/accept_commands true

/usr/local/bin/fims_send -m set -r /$$ -u /mecho/bms_conditions_met false
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/pcs_conditions_met false


read -n1 -p "Press any key to start - Site Start Test 1";echo
/usr/local/bin/fims_send -m set -u /ess/site/ess/site_status_control_command 1
echo "Expected success on CheckCmd (CloseContactors)"
echo "Expected failure on SendCmd (VerifyCloseContactors) in"
countdown 5


read -n1 -p "Press any key to start - Site Start Test 2";echo
/usr/local/bin/fims_send -m set -u /ess/site/ess/site_status_control_command 1
echo "Expected success on CheckCmd (CloseContactors) in"
countdown 5
echo "Setting bms_conditions_met to true in"
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/bms_conditions_met true
countdown 3
echo "Expected success on SendCmd (VerifyCloseContactors)"
echo "Expected success on CheckCmd (Start)"
echo "Expected failure on SendCmd (VerifyStart) in"



read -n1 -p "Press any key to start - Site Start Test 3";echo
/usr/local/bin/fims_send -m set -u /ess/site/ess/site_status_control_command 1
echo "Expected success on CheckCmd (Start) in"
countdown 5
echo "Setting pcs_conditions_met to true in"
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/pcs_conditions_met true
countdown 3
echo "Expected success on SendCmd (VerifyStart)"







# Stop

/usr/local/bin/fims_send -m set -r /$$ -u /mecho/bms_conditions_met false
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/pcs_conditions_met false


read -n1 -p "Press any key to start - Site Stop Test 1";echo
/usr/local/bin/fims_send -m set -u /ess/site/ess/site_status_control_command 0
echo "Expected success on CheckCmd (Stop)"
echo "Expected failure on SendCmd (VerifyStop) in"
countdown 5


read -n1 -p "Press any key to start - Site Stop Test 2";echo
/usr/local/bin/fims_send -m set -u /ess/site/ess/site_status_control_command 0
echo "Expected success on CheckCmd (Stop) in"
countdown 5
echo "Setting bms_conditions_met to true in"
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/pcs_conditions_met true
countdown 3
echo "Expected success on SendCmd (VerifyStop)"
echo "Expected success on CheckCmd (OpenContactors)"
echo "Expected failure on SendCmd (VerifyOpenContactors) in"



read -n1 -p "Press any key to start - Site Stop Test 3";echo
/usr/local/bin/fims_send -m set -u /ess/site/ess/site_status_control_command 0
echo "Expected success on CheckCmd (OpenContactors) in"
countdown 5
echo "Setting pcs_conditions_met to true in"
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/bms_conditions_met true
countdown 3
echo "Expected success on SendCmd (VerifyOpenContactors)"








# Standby


# Coming from an Stop state

/usr/local/bin/fims_send -m set -r /$$ -u /mecho/bms_conditions_met false
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/pcs_conditions_met false


read -n1 -p "Press any key to start - Site Standby Test 1";echo
/usr/local/bin/fims_send -m set -u /ess/site/ess/site_status_control_command 2
echo "Expected success on CheckCmd (CloseContactors)"
echo "Expected failure on SendCmd (VerifyCloseContactors) in"
countdown 5


read -n1 -p "Press any key to start - Site Standby Test 2";echo
/usr/local/bin/fims_send -m set -u /ess/site/ess/site_status_control_command 2
echo "Expected success on CheckCmd (CloseContactors) in"
countdown 5
echo "Setting bms_conditions_met to true in"
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/bms_conditions_met true
countdown 3
echo "Expected success on SendCmd (VerifyCloseContactors)"
echo "Expected success on CheckCmd (Standby)"
echo "Expected failure on SendCmd (VerifyStandby) in"



read -n1 -p "Press any key to start - Site Standby Test 3";echo
/usr/local/bin/fims_send -m set -u /ess/site/ess/site_status_control_command 2
echo "Expected success on CheckCmd (Standby) in"
countdown 5
echo "Setting pcs_conditions_met to true in"
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/pcs_conditions_met true
countdown 3
echo "Expected success on SendCmd (VerifyStandby)"


# Coming right from an Start state

echo "Starting System to test Standby from an On State"
/usr/local/bin/fims_send -m set -u /ess/site/ess/site_status_control_command 1
sleep 10

/usr/local/bin/fims_send -m set -r /$$ -u /mecho/pcs_conditions_met false


read -n1 -p "Press any key to start - Site Standby Test 4";echo
/usr/local/bin/fims_send -m set -u /ess/site/ess/site_status_control_command 2
echo "Expected success on CheckCmd (Standby)"
echo "Expected failure on SendCmd (VerifyStandby) in"
countdown 5



read -n1 -p "Press any key to start - Site Standby Test 5";echo
/usr/local/bin/fims_send -m set -u /ess/site/ess/site_status_control_command 2
echo "Expected success on CheckCmd (Standby) in"
countdown 5
echo "Setting pcs_conditions_met to true in"
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/pcs_conditions_met true
countdown 3
echo "Expected success on SendCmd (VerifyStandby)"