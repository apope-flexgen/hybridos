#!/bin/bash
#"name":"test_example_4.sh",
#"author":"Phil Wilshire",
#"desc":"test templats etc ",
#"date":"04_21_2023"

# this script has these commands
# "start " a fims _listen log for seconds and send the fims stuff
# "expect" show us what is expected
# "read log"  read the log file

testname=example_4


gitdir=$(cd `dirname $0` && cd ../.. && pwd)
source $gitdir/scripts/test_funcs.sh basic
timeout=20



test()
{
    usetime 2
    #fims_send -m pub -u /test/expect '{"test_7.00":"/some/selected/output pulse_on:true"}'
    # echo "Test fims get"
    # get="fims_send -m get -r /$$ -u /some/output"
    # res=`$get`
    # echo $res

    fims_send -m pub -u /components/feeder_52m1/start 0 
    fims_send -m pub -u /components/feeder_52m1/stop 0 
    fims_send -m pub -u /components/feeder_52m1/selector 0 
    
    #echo "========> running ps"
    #ps ax | grep go_metrics

    usetime 2
    #echo "Test fims get"
    #get="fims_send -m get -r /$$ -u /some/state/output"
    #res=`$get`
    #echo $res

    expect='{"last_start":0,"last_stop":0,"system_state":"Stopped"}'
    get="fims_send -m get -r /$$ -u /some/state/output"
    runget $expect $get



    fims_send -m pub -u /components/feeder_52m1 '{
         "start": 0,
         "stop": 0,
         "selector": 1

         }'
    usetime 2
    expect='{"last_start":0,"last_stop":0,"system_state":"Standby"}'
    get="fims_send -m get -r /$$ -u /some/state/output"
    runget $expect $get

    fims_send -m pub -u /components/feeder_52m1 '{
         "start": 1,
         "stop": 0,
         "selector": 1

         }'
    usetime 2
    expect='{"last_start":1,"last_stop":0,"system_state":"Run"}'
    get="fims_send -m get -r /$$ -u /some/state/output"
    runget $expect $get

    fims_send -m pub -u /components/feeder_52m1 '{
         "start": 0,
         "stop": 0,
         "selector": 1

         }'
    usetime 2
    expect='{"last_start":0,"last_stop":0,"system_state":"Run"}'
    get="fims_send -m get -r /$$ -u /some/state/output"
    runget $expect $get

    fims_send -m pub -u /components/feeder_52m1 '{
         "start": 0,
         "stop": 1,
         "selector": 1

         }'
    usetime 2
    expect='{"last_start":0,"last_stop":1,"system_state":"Standby"}'
    get="fims_send -m get -r /$$ -u /some/state/output"
    runget $expect $get

    fims_send -m pub -u /components/feeder_52m1 '{
         "start": 0,
         "stop": 0,
         "selector": 0

         }'
    usetime 2
    expect='{"last_start":0,"last_stop":0,"system_state":"Stopped"}'
    get="fims_send -m get -r /$$ -u /some/state/output"
    runget $expect $get

    usetime $timeleft

    echo "test complete"
    #read_fims_log

}

menu $@