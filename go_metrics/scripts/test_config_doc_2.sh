#!/bin/bash
#"name":"config_doc_2.json",
#"author":"Phil Wilshire",
#"desc":"test the readme doc 2",
#"date":"04_14_2023"

# this script has these commands
# "start " a fims _listen log for seconds and send the fims stuff
# "expect" show us what is expected
# "read log"  resd the log file

logname=config_doc_2
cfgname=config_doc_2
timeout=10


basedir=/home/docker/git/go_metrics
start ()
{
    #start config_doc_1 config_doc_1 5
    echo " logname $logname cfg $cfgname timeout $timeout"
    mkdir -p ${basedir}/logs/fims
    mkdir -p ${basedir}/logs/run
    timeout $timeout fims_listen > ${basedir}/logs/fims/$logname 2>&1&
    echo running ${basedir}/logs/fims/$logname
    timeout $timeout gometrics ${basedir}/test/configs/${cfgname}.json > ${basedir}/logs/run/$logname 2>&1&
    echo "running gometrics ${basedir}/test/configs/${cfgname}.json log  ${basedir}/logs/run/$logname"
    test

}

test()
{
    fims_send -m pub -u /test/step '{"test_2.01":"Look at what the templates did"}'

    sleep 1

    fims_send -m pub -u /test/step '{"test_2.02":"/components/feeder_52m1/v1 0"}'
    fims_send -m pub -u /test/expect '{"test_2.02":"/some/vars/var_output 5"}'
    fims_send -m pub -u /components/feeder_52m1/v1 0
    sleep 1
    fims_send -m pub -u /test/step '{"test_2.03":"/components/feeder_52m1/v1 25"}'
    fims_send -m pub -u /test/expect '{"test_2.03":"/some/vars/var_output 5"}'
    fims_send -m pub -u /components/feeder_52m1/v1 25
    sleep 1

    fims_send -m pub -u /test/step '{"test_2.04":"/components/feeder_52m1/v2 25"}'
    fims_send -m pub -u /test/expect '{"test_2.04":"/some/vars/var_output 30"}'
    fims_send -m pub -u /components/feeder_52m1/v2 25
    sleep 1

    fims_send -m pub -u /test/step '{"test_2.05":"/components/feeder_52m1/v2.1 15"}'
    fims_send -m pub -u /test/expect '{"test_2.05":"/some/vars/var_output 45"}'
    fims_send -m pub -u /components/feeder_52m1/v2.1 15
    sleep 1

    # do we need this ?
    #fims_send -m pub -u /test/step '{"test_2.05":"/components/feeder_52m1/v2.1@enable false"}'
    #fims_send -m pub -u /test/expect '{"test_2.05":"/some/vars/var_output 45"}'
    #fims_send -m pub -u /components/feeder_52m1/v2.1 15
    #sleep 1

    # we have  this assumes default enbled == false ( to be chananged to true)
    fims_send -m pub -u /test/step '{"test_2.06":"/components/feeder_52m1/v2.3/enabled true"}'
    fims_send -m pub -u /test/expect '{"test_2.06":"/some/vars/enabled_var_output 2314"}'
    fims_send -m pub -u /components/feeder_52m1/v2.3 2314
    fims_send -m pub -u /components/feeder_52m1/v2.3/enabled true
    sleep 1

    # enum and bitfield do not , yet accept inputs
    fims_send -m pub -u /test/step '{"test_2.05":"/components/feeder_52m1/v1 1234.5"}'
    fims_send -m pub -u /components/feeder_52m1/v1 0
    sleep 1

    # moving on to test the echo functionality
    fims_send -m pub -u /test/step '{"test_2.06":"/components/feeder/power_factor 345"}'
    fims_send -m pub -u /test/expect '{"test_2.06":"/components/sel_735/pf 345"}'
    fims_send -m pub -u /components/feeder/power_factor 345
    sleep 1

    # moving on to test the echo functionality
    fims_send -m pub -u /test/step '{"test_2.07":"/components/feeder/tv1voltage_l1 222"}'
    fims_send -m pub -u /test/expect '{"test_2.07":"/components/sel_735/tv1 222"}'
    fims_send -m pub -u /components/feeder/tv1voltage_l1 345
    sleep 1

    echo "test complete"
}

expect()
{
     cat ${basedir}/logs/fims_expect/$logname
}


read_fims_log_nots()
{
    cat ${basedir}/logs/fims/$logname | grep -v "Timestamp"
}
read_fims_log()
{
    echo fims_log  ${basedir}/logs/fims/${logname}
    cat ${basedir}/logs/fims/$logname
}

read_run_log()
{
    cat ${basedir}/logs/run/$logname
}

read_config()
{
    echo config  : ${basedir}/test/configs/${cfgname}.json
    cat ${basedir}/test/configs/${cfgname}.json
}

if [ "$1"  ==  "start" ] ; then
    start $@
fi

if [ "$1"  ==  "read_fims" ] ; then
    read_fims_log $@
fi
if [ "$1"  ==  "read_fims_nots" ] ; then
    read_fims_log_nots $@
fi

if [ "$1"  ==  "read_run" ] ; then
    read_run_log $@
fi

if [ "$1"  ==  "config" ] ; then
    read_config $@
fi

if [ "$1"  ==  "expect" ] ; then
    expect $@
fi
