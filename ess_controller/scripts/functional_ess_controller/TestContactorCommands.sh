# Function to display countdown
countdown() {
    local seconds=$1
    while [ $seconds -gt 0 ]; do
        echo -ne "$seconds seconds\033[0K\r"
        sleep 1
        ((seconds--))
    done
}

/usr/local/bin/fims_send -m set -r /$$ -u /components/catl_bms_1_controls/ems_cmd 3
sleep 1
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/accept_commands false
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/conditions_met false

read -n1 -p "Press any key to start CloseContactors Test 1";echo
/usr/local/bin/fims_send -m set -r /$$ -u /assets/bms/summary/close_contactors true
echo "Expected Failure on CheckCmd in"
countdown 10

/usr/local/bin/fims_send -m set -r /$$ -u /components/catl_bms_1_controls/ems_cmd 3
read -n1 -p "Press any key to start CloseContactors Test 2";echo
/usr/local/bin/fims_send -m set -r /$$ -u /assets/bms/summary/close_contactors true
echo "Setting accept_commands to true in"
countdown 7
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/accept_commands true
echo "Expected success on CheckCmd"
echo "Expected failure on SendCmd in"
countdown 5

/usr/local/bin/fims_send -m set -r /$$ -u /components/catl_bms_1_controls/ems_cmd 3
read -n1 -p "Press any key to start CloseContactors Test 3";echo
/usr/local/bin/fims_send -m set -r /$$ -u /assets/bms/summary/close_contactors true
echo "Expected success on CheckCmd in"
countdown 5
echo "Setting conditions_met to true in"
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/conditions_met true
countdown 3
echo "Expected success on SendCmd"


/usr/local/bin/fims_send -m set -r /$$ -u /components/catl_bms_1_controls/ems_cmd 2
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/accept_commands false
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/conditions_met false

read -n1 -p "Press any key to start OpenContactors Test 1";echo
/usr/local/bin/fims_send -m set -r /$$ -u /assets/bms/summary/open_contactors true
echo "Expected Failure on CheckCmd in"
countdown 10

/usr/local/bin/fims_send -m set -r /$$ -u /components/catl_bms_1_controls/ems_cmd 2
read -n1 -p "Press any key to start OpenContactors Test 2";echo
/usr/local/bin/fims_send -m set -r /$$ -u /assets/bms/summary/open_contactors true
echo "Setting accept_commands to true in"
countdown 7
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/accept_commands true
echo "Expected success on CheckCmd"
echo "Expected failure on SendCmd in"
countdown 5

/usr/local/bin/fims_send -m set -r /$$ -u /components/catl_bms_1_controls/ems_cmd 2
read -n1 -p "Press any key to start OpenContactors Test 3";echo
/usr/local/bin/fims_send -m set -r /$$ -u /assets/bms/summary/open_contactors true
echo "Expected success on CheckCmd in"
countdown 5
echo "Setting conditions_met to true in"
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/conditions_met true
countdown 3
echo "Expected success on SendCmd"