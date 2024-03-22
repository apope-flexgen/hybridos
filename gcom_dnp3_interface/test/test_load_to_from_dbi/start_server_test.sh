#!/bin/bash

# run this from hybridos/gcom_dnp3_interface/test/test_load_to_from_dbi

scripts_dir=local
bin_dir=/usr/local/bin

pkill dbi
pkill mongod
pkill gcom_dnp3_clien
pkill gcom_dnp3_serv

if [ $# -gt 0 ]
then
    exit
fi

sleep 1s

## start fims_server
if ! pgrep -af fims_server
then
fims_server &
fi

## start mongod
if ! pgrep -af mongod
then
    echo "launching mongodb..."
    mongod --config /etc/mongod.conf &
    sleep 1s
fi

## start dbi
if ! pgrep -af dbi
then
    echo "Starting DBI"
    dbi &
fi
sleep 2
echo

## delete everything for the client and the server
echo "fims_send -m del -u /dbi/test_client"
echo "fims_send -m del -u /dbi/test_server"
echo
fims_send -m del -u /dbi/test_client
fims_send -m del -u /dbi/test_server

gcom_dnp3_server server_01.json > /dev/null 2>&1 &

## Look at what is currently in /dbi/test_server/saved_registers (should be nothing if this is the first run)
echo 'fims_send -m get -u /dbi/test_server/saved_registers -r /$$'
fims_send -m get -u /dbi/test_server/saved_registers -r /$$
sleep 2.5
echo

## Look at what is currently in /some/uri (should be 0 if nothing is currently in dbi)
echo 'fims_send -m get -u /local_server/some/uri -r /$$'
fims_send -m get -u /local_server/some/uri -r /$$
sleep 2
echo

## Look at what is currently in /another/uri (should be 0 if nothing is currently in dbi)
echo 'fims_send -m get -u /local_server/another/uri -r /$$'
fims_send -m get -u /local_server/another/uri -r /$$
sleep 2
echo

## Look at what is currently in /c/c (should be 0 if nothing is currently in dbi)
echo 'fims_send -m get -u /local_server/c/c -r /$$'
fims_send -m get -u /local_server/c/c -r /$$
sleep 2
echo

## Look at what is currently in /d/d (should be 0 if nothing is currently in dbi)
echo 'fims_send -m get -u /local_server/d/d -r /$$'
fims_send -m get -u /local_server/d/d -r /$$
sleep 2
echo

## Look at what is currently in /e/e (should be 0 if nothing is currently in dbi)
echo 'fims_send -m get -u /local_server/e/e -r /$$'
fims_send -m get -u /local_server/e/e -r /$$
sleep 2
echo

## Look at what is currently in /some/uri (should be 0 if nothing is currently in dbi)
echo 'fims_send -m get -u /local_server/components/test -r /$$'
fims_send -m get -u /local_server/components/test -r /$$
sleep 2
echo

## Send a bunch of sets to gcom_dnp3_server
echo "sending pubs"
fims_send -m pub -u /a/a '{"analog_in_0": 100,"analog_in_2": -100,"binary_in_0": true,"counter_0": 50}'
fims_send -m pub -u /d/d '{"binary_in_1": true}'
fims_send -m pub -u /b/b '{"analog_in_1": 2,"analog_in_3": -2,"analog_in_4": -2,"analog_in_5": -2,"counter_1": 51}'
fims_send -m pub -u /e/e '{"counter_2": 20}'
fims_send -m pub -u /some/uri '{"analog_out_0": 100}'
fims_send -m pub -u /another/uri '{"analog_out_1": 23}'
fims_send -m pub -u /a/a '{"analog_out_2": 2,"analog_out_4": 2.3,"analog_out_6": 5.11,"binary_out_1":true,"binary_out_2":true}'
fims_send -m pub -u /b/b '{"analog_out_3": 2,"analog_out_5": 2.3,"analog_out_7": 5.11}'
fims_send -m pub -u /c/c '{"binary_out_3":true}'
fims_send -m pub -u /components/test '{"binary_out_0":true}'
sleep 10

## Look at what is currently in /dbi/test_server/saved_registers (should be 25)
echo 'fims_send -m get -u /dbi/test_server/saved_registers -r /$$'
echo 'Expect: data on /some/uri, /another/uri, /a/a, /b/b, /c/c, /d/d, /e/e, and /components/test'
echo -n 'Got   : '
fims_send -m get -u /dbi/test_server/saved_registers -r /$$
sleep 2
echo

## Look at what is currently in /some/uri
echo 'fims_send -m get -u /local_server/some/uri -r /$$'
echo 'Expect: {"analog_out_0":100}'
echo -n 'Got   : '
fims_send -m get -u /local_server/some/uri -r /$$
sleep 2
echo

## Look at what is currently in /another/uri
echo 'fims_send -m get -u /local_server/another/uri -r /$$'
echo 'Expect: {"analog_out_1":23}'
echo -n 'Got   : '
fims_send -m get -u /local_server/another/uri -r /$$
sleep 2
echo

## Look at what is currently in /a/a
echo 'fims_send -m get -u /local_server/a/a -r /$$'
echo 'Expect: {"analog_in_0":100, "analog_in_2":-100, "analog_out_2":2, "analog_out_4":2.299999952316284, "analog_out_6":5.11, "binary_in_0":true, "binary_out_1":true, "binary_out_2":true, "counter_0":50}'
echo -n 'Got   : '
fims_send -m get -u /local_server/a/a -r /$$
sleep 2
echo

## Look at what is currently in /b/b
echo 'fims_send -m get -u /local_server/b/b -r /$$'
echo 'Expect: {"analog_in_1":2, "analog_in_3":-2, "analog_in_4":-2, "analog_in_5":-2, "analog_out_3":2, "analog_out_5":2.299999952316284, "analog_out_7":5.11, "counter_1":51}'
echo -n 'Got   : '
fims_send -m get -u /local_server/b/b -r /$$
sleep 2
echo

## Look at what is currently in /c/c
echo 'fims_send -m get -u /local_server/c/c -r /$$'
echo 'Expect: {"binary_out_3":true}'
echo -n 'Got   : '
fims_send -m get -u /local_server/c/c -r /$$
sleep 2
echo

## Look at what is currently in /d/d
echo 'fims_send -m get -u /local_server/d/d -r /$$'
echo 'Expect: {"binary_in_1":true}'
echo -n 'Got   : '
fims_send -m get -u /local_server/d/d -r /$$
sleep 2
echo

## Look at what is currently in /e/e
echo 'fims_send -m get -u /local_server/e/e -r /$$'
echo 'Expect: {"counter_2":20}'
echo -n 'Got   : '
fims_send -m get -u /local_server/e/e -r /$$
sleep 2
echo

## Look at what is currently in /components/test
echo 'fims_send -m get -u /local_server/components/test -r /$$'
echo 'Expect: {"binary_out_0":true}'
echo -n 'Got   : '
fims_send -m get -u /local_server/components/test -r /$$
sleep 2
echo


echo "Killing dnp3_server..."
pkill gcom_dnp3_serv
sleep 5
echo
echo "Please confirm no instances of gcom_dnp3_server are running."
echo
echo "ps ax | grep gcom_dnp3_server"
ps ax | grep gcom_dnp3_server
echo
echo "Restarting dnp3_server..."
gcom_dnp3_server server_01.json > /dev/null 2>&1 &
sleep 5

## Look at what is currently in /dbi/test_server/saved_registers (should be 25)
echo 'fims_send -m get -u /dbi/test_server/saved_registers -r /$$'
echo 'Expect: data on /some/uri, /another/uri, /a/a, /b/b, /c/c, /d/d, /e/e, and /components/test'
echo -n 'Got   : '
fims_send -m get -u /dbi/test_server/saved_registers -r /$$
sleep 2
echo

## Look at what is currently in /some/uri
echo 'fims_send -m get -u /local_server/some/uri -r /$$'
echo 'Expect: {"analog_out_0":100}'
echo -n 'Got   : '
fims_send -m get -u /local_server/some/uri -r /$$
sleep 2
echo

## Look at what is currently in /another/uri
echo 'fims_send -m get -u /local_server/another/uri -r /$$'
echo 'Expect: {"analog_out_1":23}'
echo -n 'Got   : '
fims_send -m get -u /local_server/another/uri -r /$$
sleep 2
echo

## Look at what is currently in /a/a
echo 'fims_send -m get -u /local_server/a/a -r /$$'
echo 'Expect: {"analog_in_0":100, "analog_in_2":-100, "analog_out_2":2, "analog_out_4":2.299999952316284, "analog_out_6":5.11, "binary_in_0":true, "binary_out_1":true, "binary_out_2":true, "counter_0":50}'
echo -n 'Got   : '
fims_send -m get -u /local_server/a/a -r /$$
sleep 2
echo

## Look at what is currently in /b/b
echo 'fims_send -m get -u /local_server/b/b -r /$$'
echo 'Expect: {"analog_in_1":2, "analog_in_3":-2, "analog_in_4":-2, "analog_in_5":-2, "analog_out_3":2, "analog_out_5":2.299999952316284, "analog_out_7":5.11, "counter_1":51}'
echo -n 'Got   : '
fims_send -m get -u /local_server/b/b -r /$$
sleep 2
echo

## Look at what is currently in /c/c
echo 'fims_send -m get -u /local_server/c/c -r /$$'
echo 'Expect: {"binary_out_3":true}'
echo -n 'Got   : '
fims_send -m get -u /local_server/c/c -r /$$
sleep 2
echo

## Look at what is currently in /d/d
echo 'fims_send -m get -u /local_server/d/d -r /$$'
echo 'Expect: {"binary_in_1":true}'
echo -n 'Got   : '
fims_send -m get -u /local_server/d/d -r /$$
sleep 2
echo

## Look at what is currently in /e/e
echo 'fims_send -m get -u /local_server/e/e -r /$$'
echo 'Expect: {"counter_2":20}'
echo -n 'Got   : '
fims_send -m get -u /local_server/e/e -r /$$
sleep 2
echo

## Look at what is currently in /components/test
echo 'fims_send -m get -u /local_server/components/test -r /$$'
echo 'Expect: {"binary_out_0":true}'
echo -n 'Got   : '
fims_send -m get -u /local_server/components/test -r /$$
sleep 2
echo

pkill gcom_dnp3_serv