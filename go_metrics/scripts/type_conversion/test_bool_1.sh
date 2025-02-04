#!/bin/bash
#"name":"bool_1.json",
#"author":"Phil Wilshire",
#"desc":"test the Bool function",
#"date":"04_18_2023"

# this script has these commands
# "start " a fims _listen log for seconds and send the fims stuff
# "expect" show us what is expected
# "read log"  resd the log file

logname=bool_1
cfgname=examples/type_conversion/bool_1
timeout=20
timeused=0


basedir=$(cd `dirname $0` && cd ../.. && pwd)

start ()
{
    #start config_doc_1 config_doc_1 5
    echo " logname $logname cfg $cfgname timeout $timeout"
    mkdir -p ${basedir}/logs/fims
    mkdir -p ${basedir}/logs/run
    timeout $timeout fims_listen > ${basedir}/logs/fims/$logname 2>&1&
    echo running ${basedir}/logs/fims/$logname
    timeout $timeout go_metrics ${basedir}/${cfgname}.json > ${basedir}/logs/run/$logname 2>&1&
    echo "timeout $timeout go_metrics ${basedir}/${cfgname}.json log  ${basedir}/logs/run/$logname"
    test

}

usetime()
{

    echo -n sleep for $1
    to=$1
    sleep $to
    timeused=$(($timeused + $to))    
    timeleft=$(($timeout - $timeused))
    echo " timeleft now  $timeleft"
}

test()
{
    usetime 2
    fims_send -m pub -u /test/step '{"test_7.00":"type bool "}'
    #fims_send -m pub -u /test/expect '{"test_7.00":"/some/selected/output pulse_on:true"}'
    fims_send -m pub -u /components/float1 34.56
    fims_send -m pub -u /components/float2 -- -1
    fims_send -m pub -u /components/float3 1
    fims_send -m pub -u /components/string1 '"string1"'
    fims_send -m pub -u /components/string2 true
    echo ' expect '
    echo '{"bool_to_bool":true,"float_to_bool":true,"int_to_bool":true,"string_to_bool":true,"uint_to_bool":false}'
    #get  {"bool_to_bool":true,"float_to_bool":true,"int_to_bool":true,"string_to_bool":true,"uint_to_bool":true}
    fims_send -m get -r /$$ -u /some/output

    usetime 2
    fims_send -m pub -u /components '{
         "float1" :0,
         "float2" :0,
         "float3" :34,
         "string1" :"false",
         "string2" :false
         }'
    echo ' expect '
    echo '{"bool_to_bool":false,"float_to_bool":false,"int_to_bool":false,"string_to_bool":false,"uint_to_bool":true}'
    fims_send -m get -r /$$ -u /some/output

    usetime 2
    fims_send -m pub -u /components '{
         "float1" :0,
         "float2" :34,
         "float3" :34,
         "string1" :"true",
         "string2" :false
         }'

    echo ' expect '
    echo '{"bool_to_bool":false,"float_to_bool":false,"int_to_bool":true,"string_to_bool":false,"uint_to_bool":true}'
    fims_send -m get -r /$$ -u /some/output
    usetime $timeleft


    echo "test complete"
    #read_fims_log

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
