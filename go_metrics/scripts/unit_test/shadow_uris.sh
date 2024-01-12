#!/bin/bash

logname=shadow_uris


gitdir=$(cd `dirname $0` && cd ../.. && pwd)
source $gitdir/scripts/test_funcs.sh shadow_uris
timeout=15



test()
{

    ### TEST 1 ###
    usetime 2
    echo "Test fims get"
    get="fims_send -m get -r /$$ -u /metrics/site_status"
    res=`$get`
    echo $res

    ### TEST 2 ###

    fims_send -m pub -u /features/reactive_power '{"value":5,"runmode1_kVAR_mode_cmd":{"value":3}}'
    fims_send -m pub -u /features/active_power '{"value":125,"runmode1_kW_mode_cmd":{"value":23,"nested_value":{"value":33,"other_value":2}}}'
    

    usetime 2
    echo "Test fims_send -m pub -u /features/reactive_power '{"value":5,"runmode1_kVAR_mode_cmd":{"value":3}}'"
    get="fims_send -m get -r /$$ -u /metrics/site_status"
    res=`$get`
    echo $res

    expect='{"active_power_command_feedback":125,"active_power_command_mode_feedback":23,"active_power_operating_mode":23,"reactive_power_command_feedback":5,"reactive_power_operating_mode":3,"znested_value":33,"zznested_value":2}'
    get="fims_send -m get -r /$$ -u /metrics/site_status"
    runget $expect $get


    ### TEST 3 ###
    fims_send -m pub -u /features/reactive_power '{"value":1,"runmode1_kVAR_mode_cmd":{"value":2}}'
    fims_send -m pub -u /features/active_power '{"value":3,"runmode1_kW_mode_cmd":{"value":4,"nested_value":{"value":5,"other_value":6}}}'
    

    usetime 2
    echo "Test fims_send -m pub -u /features/reactive_power '{"value":1,"runmode1_kVAR_mode_cmd":{"value":2}}'"
    get="fims_send -m get -r /$$ -u /metrics/site_status"
    res=`$get`
    echo $res

    expect='{"active_power_command_feedback":3,"active_power_command_mode_feedback":4,"active_power_operating_mode":4,"reactive_power_command_feedback":1,"reactive_power_operating_mode":2,"znested_value":5,"zznested_value":6}'
    get="fims_send -m get -r /$$ -u /metrics/site_status"
    runget $expect $get

    
    ### DONE ###
    usetime $timeleft


    echo "test complete"
}

menu $@