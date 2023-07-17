#!/bin/sh 
# mini test script to check the 306 hotfix release server site
# P. Miller 02_03_2023



fims_send -m pub -u /sites/brp_sierra/SETPOINT 1234
fims_send -m pub -u /sites/brp_sierra/SETVAL 4321
sleep 0.2

pub1 ()
{
fims_send -m pub -u /sites/brp_sierra '{
                 "52_2_LOW_SIDE_CIRCUIT_BREAKER":true,
                 "U1_UNIT_LOCAL_REMOTE_CONTROL":false,
                  "U1_UNIT_AUTHORITY_SWITCH_ISO":true,
                   "GOV_BLOCK":false,"BESS_IDLE":false,
                   "BESS_NOT_IDLE":false,
                   "U1_UGMW_GEN7":123,
                   "U1_UGMV_GEN7":234,
                   "U1_UOLL_GENX":345,
                   "U1_UOHL_GENX":111,
                   "U1_CTLFDBK_GENX":456,
                   "GOV_DRP":0,"GOV_DB":444,
                   "OPER_RR":23,
                   "NUMBER_OF_ONLINE_INVERTORS":4,
		   "SOC_GEN_MWHX":0,
                   "MXENERGY_GEN_MWHX":0
                   }'
}

pub2 ()
{
fims_send -m pub -u /sites/brp_sierra '{
                 "52_2_LOW_SIDE_CIRCUIT_BREAKER":false,
                 "U1_UNIT_LOCAL_REMOTE_CONTROL":true,
                  "U1_UNIT_AUTHORITY_SWITCH_ISO":false,
                   "GOV_BLOCK":false,"BESS_IDLE":true,
                   "BESS_NOT_IDLE":true,
                   "U1_UGMW_GEN7":1,
                   "U1_UGMV_GEN7":2,
                   "U1_UOLL_GENX":3,
                   "U1_UOHL_GENX":1,
                   "U1_CTLFDBK_GENX":4,
                   "GOV_DRP":0,"GOV_DB":4,
                   "OPER_RR":2,
                   "NUMBER_OF_ONLINE_INVERTORS":0,
		   "SOC_GEN_MWHX":1,
                   "MXENERGY_GEN_MWHX":0
                   }'
}

pub()
{
 pub1
 sleep 2
 pub2
 sleep 2
}

pub
pub
pub
pub


## results on client 

#Method:       pub
#Uri:          /sites/brp_sierra
#ReplyTo:      (null)
#Process Name: DNP3_M_brp_sierra
#Username:     root
#Body:         {"52_2_LOW_SIDE_CIRCUIT_BREAKER":true,"U1_UNIT_LOCAL_REMOTE_CONTROL":false,"U1_UNIT_AUTHORITY_SWITCH_ISO":true,"GOV_BLOCK":false,"BESS_IDLE":false,"BESS_NOT_IDLE":false,"U1_UGMW_GEN7":123,"U1_UGMV_GEN7":234,"U1_UOLL_GENX":345,"U1_UOHL_GENX":111,"U1_CTLFDBK_GENX":456,"GOV_DRP":0,"GOV_DB":444,"OPER_RR":23,"NUMBER_OF_ONLINE_INVERTORS":4,"SOC_GEN_MWHX":0,"MXENERGY_GEN_MWHX":0,"Timestamp":"02-02-2023 03:37:39.330627"}
#Timestamp:    2023-02-02 03:37:39.330939

#Method:       pub
#Uri:          /sites/brp_sierra
#ReplyTo:      (null)
#Process Name: DNP3_M_brp_sierra
#Username:     root
#Body:         {"52_2_LOW_SIDE_CIRCUIT_BREAKER":false,"U1_UNIT_LOCAL_REMOTE_CONTROL":true,"U1_UNIT_AUTHORITY_SWITCH_ISO":false,"GOV_BLOCK":false,"BESS_IDLE":true,"BESS_NOT_IDLE":true,"U1_UGMW_GEN7":1,"U1_UGMV_GEN7":2,"U1_UOLL_GENX":3,"U1_UOHL_GENX":1,"U1_CTLFDBK_GENX":4,"GOV_DRP":0,"GOV_DB":4,"OPER_RR":2,"NUMBER_OF_ONLINE_INVERTORS":0,"SOC_GEN_MWHX":1,"MXENERGY_GEN_MWHX":0,"Timestamp":"02-02-2023 03:37:40.331381"}
#Timestamp:    2023-02-02 03:37:40.331646
