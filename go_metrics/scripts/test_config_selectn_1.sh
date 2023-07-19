#!/bin/bash
#"name":"config_selectn_1.json",
#"author":"Phil Wilshire",
#"desc":"test the selectn function",
#"date":"04_17_2023"

# this script has these commands
# "start " a fims _listen log for seconds and send the fims stuff
# "expect" show us what is expected
# "read log"  resd the log file

logname=config_selectn_1
cfgname=config_selectn_1
timeout=15


basedir=/home/docker/git/go_metrics
start ()
{
    #start config_doc_1 config_doc_1 5
    echo " logname $logname cfg $cfgname timeout $timeout"
    mkdir -p ${basedir}/logs/fims
    mkdir -p ${basedir}/logs/run
    timeout $timeout fims_listen > ${basedir}/logs/fims/$logname 2>&1&
    echo running ${basedir}/logs/fims/$logname
    timeout $timeout go_metrics ${basedir}/test/configs/${cfgname}.json > ${basedir}/logs/run/$logname 2>&1&
    echo "running gometrics ${basedir}/test/configs/${cfgname}.json log  ${basedir}/logs/run/$logname"
    test

}

test()
{
    sleep 1
    fims_send -m pub -u /test/step '{"test_3.00":"observer iniital values"}'
    fims_send -m pub -u /test/expect '{"test_3.00":"/some/selected/output selection123 33"}'
    sleep 1.1

    fims_send -m pub -u /test/step '{"test_3.01":"set up iniital values"}'
    fims_send -m pub -u /test/expect '{"test_3.00":"/some/selected/output selection123 333"}'
    fims_send -m set -u /components/feeder_52m1/v1 111
    fims_send -m set -u /components/feeder_52m1/v2 222
    fims_send -m set -u /components/feeder_52m1/v3 333
    fims_send -m set -u /components/feeder_52m1/v4 444
    fims_send -m set -u /components/feeder_52m1/v5 555

    sleep 1
    fims_send -m pub -u /test/step '{"test_3.02":"/components/feeder_52m1/selection_index 1"}'
    fims_send -m pub -u /test/expect '{"test_3.00":"/some/selected/output selection123 111"}'
    fims_send -m pub -u /components/feeder_52m1/selection_index 1

    sleep 1
    fims_send -m pub -u /test/step '{"test_3.02":"/components/feeder_52m1/selection_index 2"}'
    fims_send -m pub -u /test/expect '{"test_3.00":"/some/selected/output selection123 222"}'
    fims_send -m pub -u /components/feeder_52m1/selection_index 2

    sleep 1
    fims_send -m pub -u /test/step '{"test_3.02":"/components/feeder_52m1/selection_index 3"}'
    fims_send -m pub -u /test/expect '{"test_3.00":"/some/selected/output selection123 333"}'
    fims_send -m pub -u /components/feeder_52m1/selection_index 3


    sleep 1
    fims_send -m pub -u /test/step '{"test_3.03":"/components/feeder_52m1/selection_index 100"}'
    fims_send -m pub -u /test/expect '{"test_3.03":"/some/selected/output selection123 111"}'
    fims_send -m pub -u /components/feeder_52m1/selection_index 100

    sleep 1
    fims_send -m pub -u /test/step '{"test_3.04":"/components/feeder_52m1/selection_index 4"}'
    fims_send -m pub -u /test/expect '{"test_3.04":"/some/selected/output selection123 444"}'
    fims_send -m pub -u /components/feeder_52m1/selection_index 4

    sleep 1
    fims_send -m pub -u /test/step '{"test_3.04":"/components/feeder_52m1/selection_index 4"}'
    fims_send -m pub -u /test/expect '{"test_3.04":"/some/selected/output selection123 444"}'
    fims_send -m pub -u /components/feeder_52m1/selection_index 4

    sleep 1
    fims_send -m pub -u /test/step '{"test_3.05":"/components/feeder_52m1/selection_index -1"}'
    fims_send -m pub -u /test/expect '{"test_3.05":"/some/selected/output selection123 444"}'
    fims_send -m pub -u /components/feeder_52m1/selection_index -- -1

    sleep 1
    fims_send -m pub -u /test/step '{"test_3.06":"/components/feeder_52m1/selection_index 1"}'
    fims_send -m pub -u /test/expect '{"test_3.06":"/some/selected/output selection123 111"}'
    fims_send -m pub -u /components/feeder_52m1/selection_index 1

    sleep 1


    echo "test complete"
    read_fims_log

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

help ()
{
    echo " help : show this help"
    echo " start : start the test"
    echo " read_fims : read the fims capture"
    echo " result : read the test result"
    echo " config : show config"
    echo " expect : show expected result"
}

if [ "$1"  ==  "" ] ; then
    help $@
fi

if [ "$1"  ==  "help" ] ; then
    help $@
fi

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

if [ "$1"  ==  "result" ] ; then
    read_fims_log $@
fi

if [ "$1"  ==  "log" ] ; then
    read_run_log $@
fi

if [ "$1"  ==  "config" ] ; then
    read_config $@
fi

if [ "$1"  ==  "expect" ] ; then
    expect $@
fi
