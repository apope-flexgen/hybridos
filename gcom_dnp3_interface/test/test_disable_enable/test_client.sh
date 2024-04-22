## This script is intended to test if points that are disabled ON THE CLIENT SIDE are removed
## from pubs ON THE CLIENT SIDE and are also not being sent out as sets on the SERVER SIDE.

# Run this script on the client container after you have started the server (but not the client).
#
# The client should NOT be actively running before running this script.
#
# Make sure to follow any instructions as they appear on the screen.


# Make sure the server is running
echo "Please make sure that the server is running on the SERVER CONTAINER before continuing."
echo "Press enter to continue..."
read
echo

# Also maybe run a fims listen
echo "You may also want to run a fims_listen on the SERVER CONTAINER to check that sets are coming through."
echo "Press enter to continue..."
read
echo

# Start the client
# gcom_dnp3_client client_01.json > /dev/null 2>&1 &
gcom_dnp3_client client_01.json &

# Fill the server with dummy data
echo "Please send the following command to the server on the SERVER CONTAINER"
echo
echo -e "fims_send -m pub -u /components/test '{\"analog_in_0\":1,\"analog_in_1\":2,\"analog_in_2\":3,\"analog_in_3\":4,\"analog_in_4\":5,\"analog_in_5\":6,\"analog_out_0\":7,\"analog_out_1\":8,\"analog_out_2\":9,\"analog_out_3\":10,\"analog_out_4\":11,\"analog_out_5\":12,\"analog_out_6\":13,\"analog_out_7\":14,\"binary_in_0\":true,\"binary_in_1\":true,\"binary_out_0\":true,\"binary_out_1\":true,\"binary_out_2\":true,\"binary_out_3\":true,\"counter_0\":15,\"counter_1\":16,\"counter_2\":17}'"
echo
echo "Press enter to continue..."
read
echo

# Confirm dummy data is there
echo 'Expect:       {"analog_in_0":1,"analog_in_1":2,"analog_in_2":3,"analog_in_3":4,"analog_in_4":5,"analog_in_5":6,"analog_out_0":7,"analog_out_1":8,"analog_out_2":9,"analog_out_3":10,"analog_out_4":11,"analog_out_5":12,"analog_out_6":13,"analog_out_7":14,"binary_in_0":true,"binary_in_1":true,"binary_out_0":true,"binary_out_1":true,"binary_out_2":true,"binary_out_3":true,"counter_0":15,"counter_1":16,"counter_2":17}'
echo -n Got   :
stdbuf -o0 fims_listen -u /components/test -n 1 | stdbuf -o0 grep Body | stdbuf -o0 awk -F "Body:" '{print $2}'
echo
echo "Press enter to continue..."
read
echo

# Disable analog_out_0 on the client
echo "Sending command to client..."
echo "fims_send -m set -u /components/test/analog_out_0/_disable"
fims_send -m set -u /components/test/analog_out_0/_disable
echo
sleep 2

# Disable analog_in_0 on the client
echo "Sending command to client..."
echo "fims_send -m set -u /components/test/analog_in_0/_disable"
fims_send -m set -u /components/test/analog_in_0/_disable
echo
sleep 2

# Attempt to send set to analog_out_0
echo "Sending command to client..."
echo -e "fims_send -m set -u /components/test '{\"analog_out_0\":8,\"analog_out_1\":9}'"
fims_send -m set -u /components/test '{"analog_out_0":8,"analog_out_1":9}'
echo


# Check that sets came out on the server
echo "If you are running a fims_listen on the SERVER CONTAINER, check for the following sets:"
echo
echo 'fims_send -m set -u /components/test/analog_out_1 9'
echo
echo 'Also confirm that a set WAS NOT sent to /components/test/analog_out_0'
echo
echo "Press enter to continue..."
read
echo

# Confirm analog_out_0 data is not being published by client
echo 'Expect:       {"analog_in_1":2,"analog_in_2":3,"analog_in_3":4,"analog_in_4":5,"analog_in_5":6,"analog_out_1":9,"analog_out_2":9,"analog_out_3":10,"analog_out_4":11,"analog_out_5":12,"analog_out_6":13,"analog_out_7":14,"binary_in_0":true,"binary_in_1":true,"binary_out_0":true,"binary_out_1":true,"binary_out_2":true,"binary_out_3":true,"counter_0":15,"counter_1":16,"counter_2":17}'
echo -n Got   :
stdbuf -o0 fims_listen -u /components/test -n 1 | stdbuf -o0 grep Body | stdbuf -o0 awk -F "Body:" '{print $2}'
echo
echo "Press enter to continue..."
read
echo

# Enable analog_out_0 on the client
echo "Sending command to client..."
echo "fims_send -m set -u /components/test/analog_out_0/_enable"
fims_send -m set -u /components/test/analog_out_0/_enable
echo
sleep 2

# Enable analog_in_0 on the client
echo "Sending command to client..."
echo "fims_send -m set -u /components/test/analog_in_0/_enable"
fims_send -m set -u /components/test/analog_in_0/_enable
echo
sleep 2

# Attempt to send set to analog_out_0
echo "Sending command to client..."
echo -e "fims_send -m set -u /components/test '{\"analog_out_0\":8,\"analog_out_1\":9}'"
fims_send -m set -u /components/test '{"analog_out_0":8,"analog_out_1":9}'
echo


# Check that sets came out on the server
echo "If you are running a fims_listen on the SERVER CONTAINER, check for the following sets:"
echo
echo 'fims_send -m set -u /components/test/analog_out_0 8'
echo 'fims_send -m set -u /components/test/analog_out_1 9'
echo
echo "Press enter to continue..."
read
echo

# Confirm analog_out_0 data is again being published by client
echo 'Expect:       {"analog_in_0":1,"analog_in_1":2,"analog_in_2":3,"analog_in_3":4,"analog_in_4":5,"analog_in_5":6,"analog_out_1":8,"analog_out_1":9,"analog_out_2":9,"analog_out_3":10,"analog_out_4":11,"analog_out_5":12,"analog_out_6":13,"analog_out_7":14,"binary_in_0":true,"binary_in_1":true,"binary_out_0":true,"binary_out_1":true,"binary_out_2":true,"binary_out_3":true,"counter_0":15,"counter_1":16,"counter_2":17}'
echo -n Got   :
stdbuf -o0 fims_listen -u /components/test -n 1 | stdbuf -o0 grep Body | stdbuf -o0 awk -F "Body:" '{print $2}'
echo
echo "Press enter to continue..."
read
echo

pkill gcom_dnp3_clien