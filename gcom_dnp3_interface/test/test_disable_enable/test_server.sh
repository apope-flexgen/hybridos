## This script is intended to test if points that are disabled ON THE SERVER SIDE
## are not being sent out as sets on the SERVER SIDE.

# Run this script on the server container after you have started the client (but not the server).
#
# The server should NOT be actively running before running this script.
#
# Make sure to follow any instructions as they appear on the screen.


# Make sure the client is running
echo "Please make sure that the client is running on the CLIENT CONTAINER before continuing."
echo "Press enter to continue..."
read
echo

# Also maybe run a fims listen
echo "You may also want to run a fims_listen on the SERVER CONTAINER to check that sets are coming through."
echo "Press enter to continue..."
read
echo

# Start the server
gcom_dnp3_server server_01.json > /dev/null 2>&1 &

# Fill the server with dummy data
echo "Sending command to server..."
echo
echo 'fims_send -m pub -u /components/test {"analog_in_0":1,"analog_in_1":2,"analog_in_2":3,"analog_in_3":4,"analog_in_4":5,"analog_in_5":6,"analog_out_0":7,"analog_out_1":8,"analog_out_2":9,"analog_out_3":10,"analog_out_4":11,"analog_out_5":12,"analog_out_6":13,"analog_out_7":14,"binary_in_0":true,"binary_in_1":true,"binary_out_0":true,"binary_out_1":true,"binary_out_2":true,"binary_out_3":true,"counter_0":15,"counter_1":16,"counter_2":17}'
fims_send -m pub -u /components/test '{"analog_in_0":1,"analog_in_1":2,"analog_in_2":3,"analog_in_3":4,"analog_in_4":5,"analog_in_5":6,"analog_out_0":7,"analog_out_1":8,"analog_out_2":9,"analog_out_3":10,"analog_out_4":11,"analog_out_5":12,"analog_out_6":13,"analog_out_7":14,"binary_in_0":true,"binary_in_1":true,"binary_out_0":true,"binary_out_1":true,"binary_out_2":true,"binary_out_3":true,"counter_0":15,"counter_1":16,"counter_2":17}'
echo

# Send some sets through the client to make sure that sets are working
echo "Please send the following command to the client on the CLIENT CONTAINER"
echo "Then confirm that the output matches what the expected values below."
echo
echo -e "fims_send -m set -u /components/test '{\"analog_out_0\":1,\"analog_out_2\":1,\"analog_out_4\":1,\"analog_out_6\":1,\"binary_out_0\":false,\"binary_out_2\":false}'"
echo
echo 'Expect sets to appear on server container:'
echo '{"analog_out_0":1,"analog_out_2":1,"analog_out_4":1,"analog_out_6":1,"binary_out_0":false,"binary_out_2":false}'
echo "Press enter to continue..."
read
echo

# Disable analog_out_0 on the server
echo "Sending command to server..."
echo "fims_send -m set -u /components/test/analog_out_0/_disable"
fims_send -m set -u /components/test/analog_out_0/_disable
echo
sleep 2

# Send some sets through the client to make sure that sets are working
echo "Please send the following command to the client on the CLIENT CONTAINER"
echo
echo -e "fims_send -m set -u /components/test '{\"analog_out_0\":9,\"analog_out_2\":10}'"
echo
echo "Press enter to continue..."
read

# Check that sets came out on the server
echo "If you are running a fims_listen on the SERVER CONTAINER, check for the following sets:"
echo
echo 'fims_send -m set -u /components/test/analog_out_2 10'
echo
echo 'Also confirm that a set WAS NOT sent to /components/test/analog_out_0'
echo
echo "Press enter to continue..."
read
echo

# Enable analog_out_0 on the server
echo "Sending command to server..."
echo "fims_send -m set -u /components/test/analog_out_0/_enable"
fims_send -m set -u /components/test/analog_out_0/_enable
echo
sleep 2

# Send some sets through the client to make sure that sets are working
echo "Please send the following command to the client on the CLIENT CONTAINER"
echo
echo -e "fims_send -m set -u /components/test '{\"analog_out_0\":3,\"analog_out_2\":3}'"
echo
echo "Press enter to continue..."
read

# Check that sets came out on the server
echo "If you are running a fims_listen on the SERVER CONTAINER, check for the following sets:"
echo
echo 'fims_send -m set -u /components/test/analog_out_0 3'
echo
echo 'fims_send -m set -u /components/test/analog_out_2 3'
echo
echo "Press enter to continue..."
read
echo

pkill gcom_dnp3_serv