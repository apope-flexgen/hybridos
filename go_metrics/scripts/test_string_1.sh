#!/bin/bash
#"name":"string_1.json",
#"author":"Phil Wilshire",
#"desc":"test the Int function",
#"date":"04_18_2023"

# this script has these commands
# "start " a fims _listen log for seconds and send the fims stuff
# "expect" show us what is expected
# "read log"  resd the log file

logname=string_1
cfgname=examples/type_conversion/string
timeout=20
timeused=0


basedir=/home/docker/git/go_metrics

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
    fims_send -m pub -u /components/int1 -- -1
    fims_send -m pub -u /components/uint1 1
    fims_send -m pub -u /components/string1 '"string1"'
    fims_send -m pub -u /components/bool1 true

    usetime 2
    echo ' expect '
    echo '{"bool_to_string":"true","float_to_string":"34.560000","int_to_string":"-1","string_to_string":"string1","uint_to_string":"1"}'
    #get  {"bool_to_bool":true,"float_to_bool":true,"int_to_bool":true,"string_to_bool":true,"uint_to_bool":true}
    fims_send -m get -r /$$ -u /some/output

    fims_send -m pub -u /components '{
         "float1" :0,
         "int1" :0,
         "uint1" :34,
         "string1" :"false",
         "bool1" :false
         }'
    fims_send -m pub -u /components/float1 0
    fims_send -m pub -u /components/int1 0
    fims_send -m pub -u /components/uint1 34
    fims_send -m pub -u /components/string1 '"false"'
    fims_send -m pub -u /components/bool1 false
    usetime 2
    echo ' expect '
    echo '{"bool_to_string":"false","float_to_string":"0.000000","int_to_string":"0","string_to_string":"false","uint_to_string":"34"}'
    #get  {"bool_to_bool":true,"float_to_bool":true,"int_to_bool":true,"string_to_bool":true,"uint_to_bool":true}
    fims_send -m get -r /$$ -u /some/output

    fims_send -m pub -u /components '{
         "float1" :0,
         "int1" :34,
         "uint1" :0,
         "string1" :"true",
         "bool1" :false
         }'

    usetime 2
    echo ' expect '
    echo '{"bool_to_string":"false","float_to_string":"0.000000","int_to_string":"34","string_to_string":"true","uint_to_string":"0"}'
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
