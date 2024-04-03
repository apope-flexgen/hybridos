# Run this script on the client container when the client is connected to the server.
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
gcom_dnp3_client client_01.json > /dev/null 2>&1 &

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

# Force all values on the client
echo "Sending command to client..."
echo "fims_send -m set -u /local_client/components/test/_force"
fims_send -m set -u /local_client/components/test/_force
echo
sleep 2

# Force all input values to something else:
echo "Sending command to client..."
echo -e "fims_send -m set -u /local_client/components/test '{\"analog_in_0\":2,\"analog_in_1\":3,\"analog_in_2\":4,\"analog_in_3\":5,\"analog_in_4\":6,\"analog_in_5\":7,\"analog_out_0\":8,\"analog_out_1\":9,\"analog_out_2\":10,\"analog_out_3\":11,\"analog_out_4\":12,\"analog_out_5\":13,\"analog_out_6\":14,\"analog_out_7\":15,\"binary_in_0\":false,\"binary_in_1\":true,\"binary_out_0\":false,\"binary_out_1\":true,\"binary_out_2\":false,\"binary_out_3\":true,\"counter_0\":16,\"counter_1\":17,\"counter_2\":18}'"
fims_send -m set -u /local_client/components/test '{"analog_in_0":2,"analog_in_1":3,"analog_in_2":4,"analog_in_3":5,"analog_in_4":6,"analog_in_5":7,"analog_out_0":8,"analog_out_1":9,"analog_out_2":10,"analog_out_3":11,"analog_out_4":12,"analog_out_5":13,"analog_out_6":14,"analog_out_7":15,"binary_in_0":false,"binary_in_1":true,"binary_out_0":false,"binary_out_1":true,"binary_out_2":false,"binary_out_3":true,"counter_0":16,"counter_1":17,"counter_2":18}'
echo


# Check that sets came out on the server
echo "If you are running a fims_listen on the SERVER CONTAINER, check for the following sets:"
echo
echo '"analog_out_0":8,"analog_out_1":9,"analog_out_2":10,"analog_out_3":11,"analog_out_4":12,"analog_out_5":13,"analog_out_6":14,"analog_out_7":15'
echo '"binary_out_0":false,"binary_out_1":true,"binary_out_2":false,"binary_out_3":true'
echo
echo "Press enter to continue..."
read
echo

# Confirm forced data is being published by client
echo 'Expect:       {"analog_in_0":2,"analog_in_1":3,"analog_in_2":4,"analog_in_3":5,"analog_in_4":6,"analog_in_5":7,"analog_out_0":8,"analog_out_1":9,"analog_out_2":10,"analog_out_3":11,"analog_out_4":12,"analog_out_5":13,"analog_out_6":14,"analog_out_7":15,"binary_in_0":false,"binary_in_1":true,"binary_out_0":false,"binary_out_1":true,"binary_out_2":false,"binary_out_3":true,"counter_0":16,"counter_1":17,"counter_2":18}'
echo -n Got   :
stdbuf -o0 fims_listen -u /components/test -n 1 | stdbuf -o0 grep Body | stdbuf -o0 awk -F "Body:" '{print $2}'
echo
echo "Press enter to continue..."
read
echo

# Fill the server with dummy data
echo "Please send the following command to the server on the SERVER CONTAINER"
echo
echo -e "fims_send -m pub -u /components/test '{\"analog_in_0\":1,\"analog_in_1\":2,\"analog_in_2\":3,\"analog_in_3\":4,\"analog_in_4\":5,\"analog_in_5\":6,\"analog_out_0\":7,\"analog_out_1\":8,\"analog_out_2\":9,\"analog_out_3\":10,\"analog_out_4\":11,\"analog_out_5\":12,\"analog_out_6\":13,\"analog_out_7\":14,\"binary_in_0\":true,\"binary_in_1\":true,\"binary_out_0\":true,\"binary_out_1\":true,\"binary_out_2\":true,\"binary_out_3\":true,\"counter_0\":15,\"counter_1\":16,\"counter_2\":17}'"
echo
echo "Press enter to continue..."
read
echo

# Confirm forced data is still on client
echo 'Expect:       {"analog_in_0":2,"analog_in_1":3,"analog_in_2":4,"analog_in_3":5,"analog_in_4":6,"analog_in_5":7,"analog_out_0":8,"analog_out_1":9,"analog_out_2":10,"analog_out_3":11,"analog_out_4":12,"analog_out_5":13,"analog_out_6":14,"analog_out_7":15,"binary_in_0":false,"binary_in_1":true,"binary_out_0":false,"binary_out_1":true,"binary_out_2":false,"binary_out_3":true,"counter_0":16,"counter_1":17,"counter_2":18}'
echo -n Got   :
stdbuf -o0 fims_listen -u /components/test -n 1 | stdbuf -o0 grep Body | stdbuf -o0 awk -F "Body:" '{print $2}'
echo
echo "Press enter to continue..."
read
echo

# Unforce all values on the client
echo "Sending command to client..."
echo "fims_send -m set -u /local_client/components/test/_unforce"
fims_send -m set -u /local_client/components/test/_unforce
echo
sleep 2

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

# Attempt to force all input values to something else (should not work)
echo "Sending command to client..."
echo -e "fims_send -m set -u /local_client/components/test '{\"analog_in_0\":2,\"analog_in_1\":3,\"analog_in_2\":4,\"analog_in_3\":5,\"analog_in_4\":6,\"analog_in_5\":7,\"analog_out_0\":8,\"analog_out_1\":9,\"analog_out_2\":10,\"analog_out_3\":11,\"analog_out_4\":12,\"analog_out_5\":13,\"analog_out_6\":14,\"analog_out_7\":15,\"binary_in_0\":false,\"binary_in_1\":true,\"binary_out_0\":false,\"binary_out_1\":true,\"binary_out_2\":false,\"binary_out_3\":true,\"counter_0\":16,\"counter_1\":17,\"counter_2\":18}'"
fims_send -m set -u /local_client/components/test '{"analog_in_0":2,"analog_in_1":3,"analog_in_2":4,"analog_in_3":5,"analog_in_4":6,"analog_in_5":7,"analog_out_0":8,"analog_out_1":9,"analog_out_2":10,"analog_out_3":11,"analog_out_4":12,"analog_out_5":13,"analog_out_6":14,"analog_out_7":15,"binary_in_0":false,"binary_in_1":true,"binary_out_0":false,"binary_out_1":true,"binary_out_2":false,"binary_out_3":true,"counter_0":16,"counter_1":17,"counter_2":18}'
echo
sleep 2

# Check that sets came out on the server
echo "If you are running a fims_listen on the SERVER CONTAINER, confirm that no sets were sent out recently."
echo
echo "Press enter to continue..."
read
echo

# Confirm dummy data is still on client container
echo 'Expect:       {"analog_in_0":1,"analog_in_1":2,"analog_in_2":3,"analog_in_3":4,"analog_in_4":5,"analog_in_5":6,"analog_out_0":7,"analog_out_1":8,"analog_out_2":9,"analog_out_3":10,"analog_out_4":11,"analog_out_5":12,"analog_out_6":13,"analog_out_7":14,"binary_in_0":true,"binary_in_1":true,"binary_out_0":true,"binary_out_1":true,"binary_out_2":true,"binary_out_3":true,"counter_0":15,"counter_1":16,"counter_2":17}'
echo -n Got   :
stdbuf -o0 fims_listen -u /components/test -n 1 | stdbuf -o0 grep Body | stdbuf -o0 awk -F "Body:" '{print $2}'
echo
echo "Press enter to continue..."
read
echo