#!/bin/bash
#"name":"config_pulse_1.json",
#"author":"Phil Wilshire",
#"desc":"test the pulse function",
#"date":"04_17_2023"

# this script has these commands
# "start " a fims _listen log for seconds and send the fims stuff
# "expect" show us what is expected
# "read log"  resd the log file

logname=logic_and_1
cfgname=logic/and
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
    timeout $timeout go_metrics ${basedir}/examples/${cfgname}.json > ${basedir}/logs/run/$logname 2>&1&
    echo "running gometrics ${basedir}/examples/${cfgname}.json log  ${basedir}/logs/run/$logname"
    test

}

test()
{
    fims_send -m pub -u /test/step '{"test_1.00":"observe logic and values"}'
    fims_send -m pub -u /test/expect '{"test_1.00":"/some/output/bool_output1 "}'
    sleep 1

    fims_send -m pub -u /components/int1  0
    fims_send -m pub -u /components/int2  1
    fims_send -m pub -u /components/bool1  true
    fims_send -m pub -u /components/bool2  false

    sleep 1

    fims_send -m pub -u /test/step '{"test_1.01":"observe logic and values"}'
    fims_send -m pub -u /test/expect '{"test_1.01":"/some/output/bool_output1 "}'
    fims_send -m pub -u /components/int1  1
    fims_send -m pub -u /components/int2  1
    fims_send -m pub -u /components/bool1  true
    fims_send -m pub -u /components/bool2  false
    sleep 1.1

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
