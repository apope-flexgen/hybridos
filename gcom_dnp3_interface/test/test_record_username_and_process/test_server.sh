#!/bin/bash

# run this from hybridos/gcom_dnp3_interface/test/test_record_username_and_process

scripts_dir=local
bin_dir=/usr/local/bin

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

echo "Starting gcom_dnp3_server"
gcom_dnp3_server server_01.json > /dev/null 2>&1 &
sleep 5

## Look at what is currently in /components/test/analog_out_4
echo 'fims_send -m get -u /local_server/components/test/analog_out_4/_full -r /$$'
echo 'Expect:'
echo '  "value_updated_by_process": ""'
echo '  "value_updated_by_username": ""'
echo 'Got   : '
fims_send -m get -u /local_server/components/test/analog_out_4/_full -r /$$ | jq | grep -e "value_updated_by_process" -e "value_updated_by_username"
sleep 2
echo

## Look at what is currently in /components/test/analog_in_4
echo 'fims_send -m get -u /local_server/components/test/analog_in_4/_full -r /$$'
echo 'Expect:'
echo '  "value_updated_by_process": ""'
echo '  "value_updated_by_username": ""'
echo 'Got   : '
fims_send -m get -u /local_server/components/test/analog_in_4/_full -r /$$ | jq | grep -e "value_updated_by_process" -e "value_updated_by_username"
sleep 2
echo

## Send sets to gcom_dnp3_server
echo "Sending sets..."
echo "fims_send -m set -n stephanie -u /components/test/analog_out_4 25"
echo "fims_send -m set -n stephanie -u /components/test/analog_in_4 25"
fims_send -m set -n stephanie -u /components/test/analog_out_4 25
fims_send -m set -n stephanie -u /components/test/analog_in_4 25
sleep 2
echo

## Look at what is currently in /components/test/analog_out_4
## (Shouldn't change)
echo 'fims_send -m get -u /local_server/components/test/analog_out_4/_full -r /$$'
echo 'Expect:'
echo '  "value_updated_by_process": ""'
echo '  "value_updated_by_username": ""'
echo 'Got   : '
fims_send -m get -u /local_server/components/test/analog_out_4/_full -r /$$ | jq | grep -e "value_updated_by_process" -e "value_updated_by_username"
sleep 2
echo

## Look at what is currently in /components/test/analog_in_4
## (Shouldn't change)
echo 'fims_send -m get -u /local_server/components/test/analog_in_4/_full -r /$$'
echo 'Expect:'
echo '  "value_updated_by_process": ""'
echo '  "value_updated_by_username": ""'
echo 'Got   : '
fims_send -m get -u /local_server/components/test/analog_in_4/_full -r /$$ | jq | grep -e "value_updated_by_process" -e "value_updated_by_username"
sleep 2
echo

## Send sets to gcom_dnp3_server (this is handled differently so we need another test)
echo "Sending sets..."
echo "fims_send -m set -n not_me -u /components/test '{"analog_out_4":25, "analog_in_4":25}'"
fims_send -m set -n not_me -u /components/test '{"analog_out_4":25, "analog_in_4":25}'
sleep 2
echo

## Look at what is currently in /components/test/analog_out_4
## (Shouldn't change)
echo 'fims_send -m get -u /local_server/components/test/analog_out_4/_full -r /$$'
echo 'Expect:'
echo '  "value_updated_by_process": ""'
echo '  "value_updated_by_username": ""'
echo 'Got   : '
fims_send -m get -u /local_server/components/test/analog_out_4/_full -r /$$ | jq | grep -e "value_updated_by_process" -e "value_updated_by_username"
sleep 2
echo

## Look at what is currently in /components/test/analog_in_4
## (Shouldn't change)
echo 'fims_send -m get -u /local_server/components/test/analog_in_4/_full -r /$$'
echo 'Expect:'
echo '  "value_updated_by_process": ""'
echo '  "value_updated_by_username": ""'
echo 'Got   : '
fims_send -m get -u /local_server/components/test/analog_in_4/_full -r /$$ | jq | grep -e "value_updated_by_process" -e "value_updated_by_username"
sleep 2
echo

## Send sets to gcom_dnp3_server (this is handled differently so we need another test)
echo "Sending pub..."
echo "fims_send -m pub -n someone_like_me -u /components/test '{"analog_out_4":25, "analog_in_4":25}'"
fims_send -m pub -n someone_like_me -u /components/test '{"analog_out_4":25, "analog_in_4":25}'
sleep 2
echo

## Look at what is currently in /components/test/analog_out_4
echo 'fims_send -m get -u /local_server/components/test/analog_out_4/_full -r /$$'
echo 'Expect:'
echo '  "value_updated_by_process": "fims_send"'
echo '  "value_updated_by_username": "someone_like_me"'
echo 'Got   : '
fims_send -m get -u /local_server/components/test/analog_out_4/_full -r /$$ | jq | grep -e "value_updated_by_process" -e "value_updated_by_username"
sleep 2
echo

## Look at what is currently in /components/test/analog_in_4
echo 'fims_send -m get -u /local_server/components/test/analog_in_4/_full -r /$$'
echo 'Expect:'
echo '  "value_updated_by_process": "fims_send"'
echo '  "value_updated_by_username": "someone_like_me"'
echo 'Got   : '
fims_send -m get -u /local_server/components/test/analog_in_4/_full -r /$$ | jq | grep -e "value_updated_by_process" -e "value_updated_by_username"
sleep 2
echo

## Send sets to gcom_dnp3_server
echo "Sending pubs..."
echo "fims_send -m pub -n another_one_like_me -u /components/test/analog_out_4 25"
echo "fims_send -m pub -n another_one_like_me -u /components/test/analog_in_4 25"
fims_send -m pub -n another_one_like_me -u /components/test/analog_out_4 25
fims_send -m pub -n another_one_like_me -u /components/test/analog_in_4 25
sleep 2
echo

## Look at what is currently in /components/test/analog_out_4
echo 'fims_send -m get -u /local_server/components/test/analog_out_4/_full -r /$$'
echo 'Expect:'
echo '  "value_updated_by_process": "fims_send"'
echo '  "value_updated_by_username": "another_one_like_me"'
echo 'Got   : '
fims_send -m get -u /local_server/components/test/analog_out_4/_full -r /$$ | jq | grep -e "value_updated_by_process" -e "value_updated_by_username"
sleep 2
echo

## Look at what is currently in /components/test/analog_in_4
echo 'fims_send -m get -u /local_server/components/test/analog_in_4/_full -r /$$'
echo 'Expect:'
echo '  "value_updated_by_process": "fims_send"'
echo '  "value_updated_by_username": "another_one_like_me"'
echo 'Got   : '
fims_send -m get -u /local_server/components/test/analog_in_4/_full -r /$$ | jq | grep -e "value_updated_by_process" -e "value_updated_by_username"
sleep 2
echo

pkill gcom_dnp3_serv