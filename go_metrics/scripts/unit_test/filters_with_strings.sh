#!/bin/bash

logname=filters_with_strings


gitdir=$(cd `dirname $0` && cd ../.. && pwd)
source $gitdir/scripts/test_funcs.sh filters_with_strings
timeout=15



test()
{

    ### TEST 1 ###
    usetime 2
    echo "Test fims get"
    get="fims_send -m get -r /$$ -u /components/ess_01"
    res=`$get`
    echo $res

    ### TEST 2 ###

    fims_send -m pub -u /components/ess_01 '{
                                                "pcs_1_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_2_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_3_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_4_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_5_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_6_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_7_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_8_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_9_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_10_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_11_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_12_online_flag": [{"value": 1, "string":"Online"}]
                                            }'

    usetime 1
    expect='{"online_count":12}'
    get="fims_send -m get -r /$$ -u /components/ess_01"
    runget $expect $get


    ### TEST 3 ###

    fims_send -m pub -u /components/ess_01 '{
                                                "pcs_1_online_flag": [{"value": 1, "string":"Offline"}],
                                                "pcs_2_online_flag": [{"value": 1, "string":"Offline"}],
                                                "pcs_3_online_flag": [{"value": 1, "string":"Offline"}],
                                                "pcs_4_online_flag": [{"value": 1, "string":"Offline"}],
                                                "pcs_5_online_flag": [{"value": 1, "string":"Offline"}],
                                                "pcs_6_online_flag": [{"value": 1, "string":"Offline"}],
                                                "pcs_7_online_flag": [{"value": 1, "string":"Offline"}],
                                                "pcs_8_online_flag": [{"value": 1, "string":"Offline"}],
                                                "pcs_9_online_flag": [{"value": 1, "string":"Offline"}],
                                                "pcs_10_online_flag": [{"value": 1, "string":"Offline"}],
                                                "pcs_11_online_flag": [{"value": 1, "string":"Offline"}],
                                                "pcs_12_online_flag": [{"value": 1, "string":"Offline"}]
                                            }'

    usetime 1
    expect='{"online_count":0}'
    get="fims_send -m get -r /$$ -u /components/ess_01"
    runget $expect $get


    ### TEST 4 ###

    fims_send -m pub -u /components/ess_01  '{
                                                "pcs_1_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_2_online_flag": [{"value": 1, "string":"Offline"}],
                                                "pcs_3_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_4_online_flag": [{"value": 1, "string":"Offline"}],
                                                "pcs_5_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_6_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_7_online_flag": [{"value": 1, "string":"Offline"}],
                                                "pcs_8_online_flag": [{"value": 1, "string":"Offline"}],
                                                "pcs_9_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_10_online_flag": [{"value": 1, "string":"Offline"}],
                                                "pcs_11_online_flag": [{"value": 1, "string":"Offline"}],
                                                "pcs_12_online_flag": [{"value": 1, "string":"Offline"}]
                                            }'

    usetime 1
    expect='{"online_count":5}'
    get="fims_send -m get -r /$$ -u /components/ess_01"
    runget $expect $get

    
    ### DONE ###
    usetime $timeleft


    echo "test complete"
}

menu $@