#!/bin/bash

logname=filters_galore


gitdir=$(cd `dirname $0` && cd ../.. && pwd)
source $gitdir/scripts/test_funcs.sh filters_galore
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
                                                "pcs_12_online_flag": [{"value": 1, "string":"Online"}],
                                                "random_1_flag_1": 1,
                                                "random_2_flag_1": 2,
                                                "random_3_flag_1": 3,
                                                "random_4_flag_1": 4,
                                                "random_5_flag_1": 5,
                                                "random_6_flag_1": 6,
                                                "random_7_flag_1": 7,
                                                "random_8_flag_1": 8,
                                                "random_9_flag_1": 9,
                                                "random_10_flag_1": 10,
                                                "random_11_flag_1": 11,
                                                "random_12_flag_1": 12,
                                                "random_1_flag_2": 0,
                                                "random_2_flag_2": 0,
                                                "random_3_flag_2": 0,
                                                "random_4_flag_2": 0,
                                                "random_5_flag_2": 0,
                                                "random_6_flag_2": 0,
                                                "random_7_flag_2": 0,
                                                "random_8_flag_2": 0,
                                                "random_9_flag_2": 0,
                                                "random_10_flag_2": 0,
                                                "random_11_flag_2": 0,
                                                "random_12_flag_2": 0,
                                                "bitfield_3_1":3,
                                                "bitfield_3_2":2,
                                                "bitfield_3_3":3,
                                                "bitfield_4_1":2,
                                                "bitfield_4_2":2,
                                                "bitfield_4_3":2,
                                                "bitfield_4_4":2,
                                                "bitfield_5_1":3,
                                                "bitfield_5_2":5,
                                                "bitfield_5_3":3,
                                                "bitfield_5_4":5,
                                                "bitfield_5_5":3
                                            }'

    usetime 1
    expect='{"index_filter_fail_3_4_5_sum":0,"index_filter_fail_5_4_3_sum":0,"index_filter_flag_1_sum":78,"index_filter_flag_2_count":12,"index_filter_flag_2_sum":78,"index_filter_flag_3_count":1,"index_filter_flag_3_sum":3,"online_count":12,"online_int_count":12,"random_flag_1_count":12,"random_flag_2_count":12,"type_filter_2_sum":1}'
    get="fims_send -m get -r /$$ -u /components/ess_01"
    runget $expect $get


    ### TEST 3 ###

    fims_send -m pub -u /components/ess_01 '{
                                                "pcs_1_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_2_online_flag": [{"value": 0, "string":"Online"}],
                                                "pcs_3_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_4_online_flag": [{"value": 0, "string":"Online"}],
                                                "pcs_5_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_6_online_flag": [{"value": 0, "string":"Online"}],
                                                "pcs_7_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_8_online_flag": [{"value": 0, "string":"Online"}],
                                                "pcs_9_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_10_online_flag": [{"value": 0, "string":"Online"}],
                                                "pcs_11_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_12_online_flag": [{"value": 0, "string":"Online"}],
                                                "random_1_flag_1": -1,
                                                "random_2_flag_1": -1,
                                                "random_3_flag_1": -1,
                                                "random_4_flag_1": -1,
                                                "random_5_flag_1": -1,
                                                "random_6_flag_1": -1,
                                                "random_7_flag_1": -1,
                                                "random_8_flag_1": -1,
                                                "random_9_flag_1": -1,
                                                "random_10_flag_1": -1,
                                                "random_11_flag_1": -1,
                                                "random_12_flag_1": -1,
                                                "random_1_flag_2": 2,
                                                "random_2_flag_2": 2,
                                                "random_3_flag_2": 2,
                                                "random_4_flag_2": 2,
                                                "random_5_flag_2": 2,
                                                "random_6_flag_2": 2,
                                                "random_7_flag_2": 2,
                                                "random_8_flag_2": 2,
                                                "random_9_flag_2": 2,
                                                "random_10_flag_2": 2,
                                                "random_11_flag_2": 2,
                                                "random_12_flag_2": 2,
                                                "bitfield_3_1":3,
                                                "bitfield_3_2":2,
                                                "bitfield_3_3":3,
                                                "bitfield_4_1":2,
                                                "bitfield_4_2":2,
                                                "bitfield_4_3":2,
                                                "bitfield_4_4":2,
                                                "bitfield_5_1":3,
                                                "bitfield_5_2":5,
                                                "bitfield_5_3":3,
                                                "bitfield_5_4":5,
                                                "bitfield_5_5":3
                                                }'

    usetime 1
    expect='{"index_filter_fail_3_4_5_sum":0,"index_filter_fail_5_4_3_sum":0,"index_filter_flag_1_sum":6,"index_filter_flag_2_count":6,"index_filter_flag_2_sum":-6,"index_filter_flag_3_count":0,"index_filter_flag_3_sum":0,"online_count":12,"online_int_count":12,"random_flag_1_count":12,"random_flag_2_count":12,"type_filter_2_sum":0}'
    get="fims_send -m get -r /$$ -u /components/ess_01"
    runget $expect $get


    ### TEST 4 ###

    fims_send -m pub -u /components/ess_01 '{
                                                "pcs_1_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_2_online_flag": [{"value": 2, "string":"Standby"}],
                                                "pcs_3_online_flag": [{"value": 0, "string":"Offline"}],
                                                "pcs_4_online_flag": [{"value": 3, "string":"Fault"}],
                                                "pcs_5_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_6_online_flag": [{"value": 2, "string":"Standby"}],
                                                "pcs_7_online_flag": [{"value": 0, "string":"Offline"}],
                                                "pcs_8_online_flag": [{"value": 3, "string":"Fault"}],
                                                "pcs_9_online_flag": [{"value": 1, "string":"Online"}],
                                                "pcs_10_online_flag": [{"value": 2, "string":"Standby"}],
                                                "pcs_11_online_flag": [{"value": 0, "string":"Offline"}],
                                                "pcs_12_online_flag": [{"value": 3, "string":"Fault"}],
                                                "random_1_flag_1": 1,
                                                "random_2_flag_1": 2,
                                                "random_3_flag_1": 3,
                                                "random_4_flag_1": 4,
                                                "random_5_flag_1": 5,
                                                "random_6_flag_1": 6,
                                                "random_7_flag_1": 7,
                                                "random_8_flag_1": 8,
                                                "random_9_flag_1": 9,
                                                "random_10_flag_1": 10,
                                                "random_11_flag_1": 11,
                                                "random_12_flag_1": 12,
                                                "random_1_flag_2": 0,
                                                "random_2_flag_2": 0,
                                                "random_3_flag_2": 0,
                                                "random_4_flag_2": 0,
                                                "random_5_flag_2": 0,
                                                "random_6_flag_2": 0,
                                                "random_7_flag_2": 0,
                                                "random_8_flag_2": 0,
                                                "random_9_flag_2": 0,
                                                "random_10_flag_2": 0,
                                                "random_11_flag_2": 0,
                                                "random_12_flag_2": 0,
                                                "bitfield_3_1":3,
                                                "bitfield_3_2":2,
                                                "bitfield_3_3":3,
                                                "bitfield_4_1":2,
                                                "bitfield_4_2":2,
                                                "bitfield_4_3":2,
                                                "bitfield_4_4":2,
                                                "bitfield_5_1":3,
                                                "bitfield_5_2":5,
                                                "bitfield_5_3":3,
                                                "bitfield_5_4":5,
                                                "bitfield_5_5":3
                                            }'

    usetime 1
    expect='{"index_filter_fail_3_4_5_sum":0,"index_filter_fail_5_4_3_sum":0,"index_filter_flag_1_sum":33,"index_filter_flag_2_count":6,"index_filter_flag_2_sum":33,"index_filter_flag_3_count":0,"index_filter_flag_3_sum":0,"online_count":3,"online_int_count":12,"random_flag_1_count":12,"random_flag_2_count":12,"type_filter_2_sum":1}'
    get="fims_send -m get -r /$$ -u /components/ess_01"
    runget $expect $get


    ### DONE ###
    usetime $timeleft


    echo "test complete"
}

menu $@