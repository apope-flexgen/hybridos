#!/bin/bash
#"name":"config_doc_1.json",
#"author":"Phil Wilshire",
#"desc":"test the readme",
#"date":"04_14_2023"

# this script has these commands
# "start logname cfgname xxx" a fims _listen log for seconds and send the fims stuff
# "expect" show us what is expected
# "read logname"  resd the log file


basedir=/home/docker/git/go_metrics
start ()
{
    #start config_doc_1 config_doc_1 5
    logname=config_doc_1
    cfgname=config_doc_1
    timeout=10
    echo " logname $logname cfg $cfgname timeout $timeout"
    mkdir -p ${basedir}/logs/fims
    mkdir -p ${basedir}/logs/run
    timeout $timeout fims_listen > ${basedir}/logs/fims/$logname 2>&1&
    echo running ${basedir}/logs/fims/$logname
    timeout $timeout gometrics ${basedir}/configs/${cfgname}.json > ${basedir}/logs/run/$logname 2>&1&
    echo "running gometrics ${basedir}/configs/${cfgname}.json log  ${basedir}/logs/run/$logname"
    test

}

test()
{
    fims_send -m pub -u /test/step 1.01
    fims_send -m pub -u /components/feeder_52m1/v1 0
    sleep 1
    fims_send -m pub -u /test/step 1.02
    fims_send -m pub -u /components/feeder_52m1/v1 1234.5
    sleep 1
    fims_send -m pub -u /test/step 1.03
    fims_send -m pub -u /components/feeder_52m1/v1 '{"value":1234.6}'
    sleep 1
    fims_send -m pub -u /test/step 1.04
    fims_send -m pub -u /components/feeder_52m1 '{"v1":1234.7}'
    sleep 1
    fims_send -m pub -u /test/step 1.05
    fims_send -m pub -u /components/feeder_52m1 '{"v1":{"value":1234.8}}'
    echo "test complete"
}

expect()
{
echo '
Method:       pub
Uri:          /test/step
ReplyTo:      (null)
Process Name: fims_send
Username:     root
Body:         1.01
Timestamp:    2023-04-14 14:45:45.184007

Method:       pub
Uri:          /components/feeder_52m1/v1
ReplyTo:      (null)
Process Name: fims_send
Username:     root
Body:         0
Timestamp:    2023-04-14 14:45:45.186528

Method:       pub
Uri:          /test/step
ReplyTo:      (null)
Process Name: fims_send
Username:     root
Body:         1.01
Timestamp:    2023-04-14 14:45:46.189840

Method:       pub
Uri:          /some/value/output
ReplyTo:      (null)
Process Name: fab
Username:     root
Body:         {"v1_times_5":0,"v2_times_5":0,"v3_times_5":0,"v4_times_5":0}
Timestamp:    2023-04-14 14:45:46.190054

Method:       pub
Uri:          /components/feeder_52m1/v1
ReplyTo:      (null)
Process Name: fims_send
Username:     root
Body:         1234.5
Timestamp:    2023-04-14 14:45:46.191770

Method:       pub
Uri:          /some/value/output
ReplyTo:      (null)
Process Name: fab
Username:     root
Body:         {"v1_times_5":6172.5,"v2_times_5":6172.5,"v3_times_5":6172.5,"v4_times_5":6172.5}
Timestamp:    2023-04-14 14:45:47.190383

Method:       pub
Uri:          /test/step
ReplyTo:      (null)
Process Name: fims_send
Username:     root
Body:         1.02
Timestamp:    2023-04-14 14:45:47.194868

Method:       pub
Uri:          /components/feeder_52m1/v1
ReplyTo:      (null)
Process Name: fims_send
Username:     root
Body:         {"value":1234.6}
Timestamp:    2023-04-14 14:45:47.196648

Method:       pub
Uri:          /some/value/output
ReplyTo:      (null)
Process Name: fab
Username:     root
Body:         {"v1_times_5":6173,"v2_times_5":6173,"v3_times_5":6173,"v4_times_5":6173}
Timestamp:    2023-04-14 14:45:48.190082

Method:       pub
Uri:          /test/step
ReplyTo:      (null)
Process Name: fims_send
Username:     root
Body:         1.03
Timestamp:    2023-04-14 14:45:48.199732

Method:       pub
Uri:          /components/feeder_52m1
ReplyTo:      (null)
Process Name: fims_send
Username:     root
Body:         {"v1":1234.7}
Timestamp:    2023-04-14 14:45:48.200996

Method:       pub
Uri:          /some/value/output
ReplyTo:      (null)
Process Name: fab
Username:     root
Body:         {"v1_times_5":6173.5,"v2_times_5":6173.5,"v3_times_5":6173.5,"v4_times_5":6173.5}
Timestamp:    2023-04-14 14:45:49.190336

Method:       pub
Uri:          /test/step
ReplyTo:      (null)
Process Name: fims_send
Username:     root
Body:         1.04
Timestamp:    2023-04-14 14:45:49.204068

Method:       pub
Uri:          /components/feeder_52m1
ReplyTo:      (null)
Process Name: fims_send
Username:     root
Body:         {"v1":{"value":1234.8}}
Timestamp:    2023-04-14 14:45:49.206016

Method:       pub
Uri:          /some/value/output
ReplyTo:      (null)
Process Name: fab
Username:     root
Body:         {"v1_times_5":6174,"v2_times_5":6174,"v3_times_5":6174,"v4_times_5":6174}
Timestamp:    2023-04-14 14:45:50.190604

Method:       pub
Uri:          /some/value/output
ReplyTo:      (null)
Process Name: fab
Username:     root
Body:         {"v1_times_5":6174,"v2_times_5":6174,"v3_times_5":6174,"v4_times_5":6174}
Timestamp:    2023-04-14 14:45:51.190802

Method:       pub
Uri:          /some/value/output
ReplyTo:      (null)
Process Name: fab
Username:     root
Body:         {"v1_times_5":6174,"v2_times_5":6174,"v3_times_5":6174,"v4_times_5":6174}
Timestamp:    2023-04-14 14:45:52.190258

Method:       pub
Uri:          /some/value/output
ReplyTo:      (null)
Process Name: fab
Username:     root
Body:         {"v1_times_5":6174,"v2_times_5":6174,"v3_times_5":6174,"v4_times_5":6174}
Timestamp:    2023-04-14 14:45:53.190538

Method:       pub
Uri:          /some/value/output
ReplyTo:      (null)
Process Name: fab
Username:     root
Body:         {"v1_times_5":6174,"v2_times_5":6174,"v3_times_5":6174,"v4_times_5":6174}
Timestamp:    2023-04-14 14:45:54.190605

'
}


read_fims_log_nots()
{
    logname=config_doc_1
    cfgname=config_doc_1
    timeout=10
    cat ${basedir}/logs/fims/$logname | grep -v "Timestamp"
}
read_fims_log()
{
    logname=config_doc_1
    cfgname=config_doc_1
    timeout=10
    cat ${basedir}/logs/fims/$logname
}

read_run_log()
{
    logname=config_doc_1
    cfgname=config_doc_1
    timeout=10
    cat ${basedir}/logs/run/$logname
}

read_config()
{
    logname=config_doc_1
    cfgname=config_doc_1
    timeout=10
    echo config  : ${basedir}/configs/${cfgname}.json
    cat ${basedir}/configs/${cfgname}.json
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

if [ "$1"  ==  "read_config" ] ; then
    read_config $@
fi

if [ "$1"  ==  "expect" ] ; then
    expect $@
fi
