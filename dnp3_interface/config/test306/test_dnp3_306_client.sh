#!/bin/sh 
# 02_03_2023
# mini test script to check the 306 hotfix release

fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT true


fims_send -m set -u /local_client/_system '{"debug":3}' 
sleep 3
fims_send -m set -u /local_client/_system '{"debug":0}' 

fims_send -m get -r /$$ -u /sites/brp_sierra/SETPOINT

fims_send -m get -r /$$ -u /sites/brp_sierra

fims_send -m set -u /sites/brp_sierra/SETPOINT 1234
fims_send -m set -u /sites/brp_sierra/SETVAL 4321
sleep 0.2
fims_send -m set -u /sites/brp_sierra '{
                                        "SETPOINT": 123,
                                        "SETVAL": 203,
                                        "voltage":23456,
                                        "current":-2345,
                                         "charge":334455}'

fims_send -m set -u /sites/brp_sierra/_system '{"debug":0}'
fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT LATCH_ON 
fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT LATCH_OFF 
fims_send -m set -u /sites/brp_sierra/_system '{"debug":2}'
fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT 3
fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT 4
fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT true
fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT false
fims_send -m set -u /sites/brp_sierra/_system '{"debug":0}'


fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT  '{"value":3}'            #=>     true
fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT  '{"value":4}'            #=>    false

fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT  '{"value":true}'         #=>     true
fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT  '{"value":false}'        #=>    false

fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT  '{"value":"LATCH_ON"}'   #=>     true
fims_send -m set -u /sites/brp_sierra/TESTCROB_DEFAULT  '{"value":"LATCH_OFF"}'  #=>    false


#    &&  fims_send -m set -u /sites/brp_sierra/_system '{"debug":0}'

timeout 2 fims_listen

#results on client
#Method:       pub
#Uri:          /sites/brp_sierra
#ReplyTo:      (null)
#Process Name: DNP3_M_brp_sierra
#Username:     root
#Body:         {"52_2_LOW_SIDE_CIRCUIT_BREAKER":false,
#                 "U1_UNIT_LOCAL_REMOTE_CONTROL":false,
#                  "U1_UNIT_AUTHORITY_SWITCH_ISO":false,
#                   "GOV_BLOCK":false,"BESS_IDLE":false,
#                   "BESS_NOT_IDLE":false,
#                   "U1_UGMW_GEN7":0,
#                   "U1_UGMV_GEN7":0,
#                   "U1_UOLL_GENX":0,
#                   "U1_UOHL_GENX":0,
#                   "U1_CTLFDBK_GENX":0,
#                   "GOV_DRP":0,"GOV_DB":0,
#                   "OPER_RR":0,
##                   "NUMBER_OF_ONLINE_INVERTORS":0,"SOC_GEN_MWHX":0,
#                   "MXENERGY_GEN_MWHX":0,
#                   "Timestamp":"02-02-2023 03:20:01.100873"}
#Timestamp:    2023-02-02 03:20:01.101102


# results on server ...

#Method:       set
#Uri:          /sites/brp_sierra/SETPOINT
#ReplyTo:      (null)
#Process Name: DNP3_O_brp_sierra
#Username:     root
#Body:         1234
#Timestamp:    2023-02-02 03:16:47.586176

#Method:       set
#Uri:          /sites/brp_sierra
#ReplyTo:      (null)
#Process Name: DNP3_O_brp_sierra
#Username:     root
#Body:         {"SETVAL":4321}
#Timestamp:    2023-02-02 03:16:47.983894

#Method:       set
#Uri:          /sites/brp_sierra/SETPOINT
#ReplyTo:      (null)
#Process Name: DNP3_O_brp_sierra
#Username:     root
#Body:         123
#Timestamp:    2023-02-02 03:16:47.998877

#Method:       set
#Uri:          /sites/brp_sierra
#ReplyTo:      (null)
#Process Name: DNP3_O_brp_sierra
#Username:     root
#Body:         {"SETVAL":203,"voltage":23456,"current":-2345,"charge":334455}
#Timestamp:    2023-02-02 03:16:48.494119
