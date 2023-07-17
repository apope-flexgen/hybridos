#!/bin/sh 
# 02_14_2023

# mini test script to check negative floats from client -> server and server -> client

# this config file is set up using gherpick.py

# config entry


    # "system": {
    #     "name": "FlexGen HybridOS",
    #     "protocol": "DNP3",
    #     "version": "0.2",
    #     "base_uri": "/sites",
    #     "id": "hoth",
    #     "debug": 0,
    #     "xip_address": "10.10.1.31",
    #     "ip_address": "172.17.0.2",
    #     "local_uri": "/local_server",
    #     "port": 20000,
    #     "unsol": 1,
    #     "event_pub": true,
    #     "xformat": "naked",
    #     "note": "this system must have a timeout to trigger the variable timeout",
    #     "timeout": 1000,
    #     "frequency": 500

            # "type": "analog",
            # "map": [
            #     {
            #         "id": "analog_1",
            #         "offset": 0,
            #         "scale": 1,
            #         "Xformat": "clothed",
            #         "timeout": 5,  <<< this one is in seconds maybe it should be in ms ???
            #         "uri": "/sites/analog",
            #         "name": "Analog1G1V1"
            #     },

dnp3_server /home/docker/configs/server/dnp3_server_test_306b.json

#on server 
fims_send -m pub -u /sites/analog/analog_1 -- -1234

#look at pubs on client

# Method:       pub
# Uri:          /test/analog
# ReplyTo:      (null)
# Process Name: DNP3_M_hoth
# Username:     root
# Body:         {"analog_1":-1234,"analog_2":0,"Timestamp":"02-14-2023 20:42:46.865206"}
# Timestamp:    2023-02-14 20:42:46.865482

fims_send -m pub -u /sites/analog/analog_1 -- 1234

# Method:       pub
# Uri:          /test/analog
# ReplyTo:      (null)
# Process Name: DNP3_M_hoth
# Username:     root
# Body:         {"analog_1":1234,"analog_2":0,"Timestamp":"02-14-2023 20:44:17.941799"}
# Timestamp:    2023-02-14 20:44:17.942077


#Client to Server test.
client
dnp3_client /home/docker/configs/client/dnp3_client_test_306b.json

server
dnp3_client /home/docker/configs/server/dnp3_server_test_306b.json

#start a fims_listen on the server 

fims_listen > /tmp/fimsListen.log 2>&1&

#on the client

fims_send -m set -u /testcli/testop/TestOPF32toSite -- 1234

fims_send -m set -u /testcli/testop/TestOPF32toSite -- -1

fims_send -m set -u /testcli/testop/TestOPF32toSite -- 80000

fims_send -m set -u /testcli/testop/TestOPF32toSite -- -80000

# Process Name: DNP3_O_hoth
# Username:     root
# Body:         {"source":"DNP3_server:hoth","message":"state change [OPEN]\n","severity":1}
# Timestamp:    2023-02-14 20:52:48.550136

# Method:       set
# Uri:          /testsrv/testop/TestOPF32toSite
# ReplyTo:      (null)
# Process Name: DNP3_O_hoth
# Username:     root
# Body:         1234
# Timestamp:    2023-02-14 20:53:11.249009

# Method:       set
# Uri:          /testsrv/testop/TestOPF32toSite
# ReplyTo:      (null)
# Process Name: DNP3_O_hoth
# Username:     root
# Body:         -1
# Timestamp:    2023-02-14 20:53:11.250371

# Method:       set
# Uri:          /testsrv/testop/TestOPF32toSite
# ReplyTo:      (null)
# Process Name: DNP3_O_hoth
# Username:     root
# Body:         80000
# Timestamp:    2023-02-14 20:53:11.251293

# Method:       set
# Uri:          /testsrv/testop/TestOPF32toSite
# ReplyTo:      (null)
# Process Name: DNP3_O_hoth
# Username:     root
# Body:         -80000
# Timestamp:    2023-02-14 20:53:



#clothed testing

fims_send -m set -u /testcli/testop/TestOPF32toSiteCl -- 1234

fims_send -m set -u /testcli/testop/TestOPF32toSiteCl -- -1

fims_send -m set -u /testcli/testop/TestOPF32toSiteCl -- 80000

fims_send -m set -u /testcli/testop/TestOPF32toSiteCl -- -80000

# fims_listen output
# Method:       set
# Uri:          /testsrv/testop/TestOPF32toSiteCl
# ReplyTo:      (null)
# Process Name: DNP3_O_hoth
# Username:     root
# Body:         {"value":1234}
# Timestamp:    2023-02-14 20:56:32.27204

# Method:       set
# Uri:          /testsrv/testop/TestOPF32toSiteCl
# ReplyTo:      (null)
# Process Name: DNP3_O_hoth
# Username:     root
# Body:         {"value":-1}
# Timestamp:    2023-02-14 20:56:32.29007

# Method:       set
# Uri:          /testsrv/testop/TestOPF32toSiteCl
# ReplyTo:      (null)
# Process Name: DNP3_O_hoth
# Username:     root
# Body:         {"value":80000}
# Timestamp:    2023-02-14 20:56:32.35313

# Method:       set
# Uri:          /testsrv/testop/TestOPF32toSiteCl
# ReplyTo:      (null)
# Process Name: DNP3_O_hoth
# Username:     root
# Body:         {"value":-80000}
# Timestamp:    2023-02-14 20:56:33.948672

# server gets using local_server
fims_send -m get -r /$$ -u /local_server/testsrv/testop/TestOPF32toSiteCl
#{"TestOPF32toSiteCl":{"value":80000},"Timestamp":"02-14-2023 21:01:44.990904"}

fims_send -m get -r /$$ -u /local_server/testsrv/testop/TestOPF32toSite
#{"TestOPF32toSite":-80000,"Timestamp":"02-14-2023 21:02:21.628966"}

#Client gets
# note these may well be wrong
fims_send -m set -u /testcli/testop/TestOPF32toSiteCl -- -80000
fims_send -m get -r /$$ -u /testcli/testop/TestOPF32toSiteCl
#{"TestOPF32toSiteCl":{"value":-80000}}
fims_send -m set -u /testcli/testop/TestOPF32toSiteCl -- 80000
fims_send -m get -r /$$ -u /testcli/testop/TestOPF32toSiteCl
#{"TestOPF32toSiteCl":{"value":80000}}


sh-4.2# fims_send -m set -u /testcli/testop/TestOPF32toSite -- 80000
sh-4.2# fims_send -m get -r /$$ -u /testcli/testop/TestOPF32toSite
#{"TestOPF32toSite":80000}
sh-4.2# fims_send -m set -u /testcli/testop/TestOPF32toSite -- -80000
sh-4.2# fims_send -m get -r /$$ -u /testcli/testop/TestOPF32toSite
#{"TestOPF32toSite":-80000}



#on the server
fims_send -m pub -u /sites/analog/analog_1 -- -1234
#on the client
 fims_send -m get -r /$$ -u /local_client/test/analog/analog_1
#{"analog_1":-1234}

fims_send -m pub -u /sites/analog/analog_1 -- 8910
fims_send -m get -r /$$ -u /local_client/test/analog/analog_1
#{"analog_1":8910}
 fims_send -m get -r /$$ -u /local_client/clothed/test/analog/analog_1
#{"analog_1":{"value":8910}}

server::fims_send -m pub -u /sites/analog/analog_1 -- -910

client::fims_send -m get -r /$$ -u /local_client/clothed/test/analog/analog_1
#{"analog_1":{"value":-910}}



#batchedSets client setup

# this may not be working as desired
# 
# config client and server 
# "system":{
#       "format": "naked",
#         "frequency": 1000,
#         "batchSets":1500  << must be set on both client and server all pubbed vars in server must have "notBatched":true,
#
#     },
            # "type": "AnOPF32",
            # "map": [
            #     {
            #         "id": "TestOPF32toSite",
            #         "offset": 0,
            #         "name": "Test OUTPut F32 To Site",
            #         "unit": "MW",
            #         "signed": true,
            #         "notBatched":true,
            #         "uri": "/testcli/testop"
            #     },
            #     {
            #         "id": "TestOPF32toSiteCl",
            #         "offset": 1,
            #         "name": "Test OUTPut F32 To Site",
            #         "unit": "MW",
            #         "format": "clothed",
            #         "signed": true,
            #         "notBatched":false,
            #         "uri": "/testcli/testop"
            #     },
            #                     {
            #         "id": "TestOPF32toSite_2",
            #         "offset": 2,
            #         "name": "Test OUTPut F32 To Site #2",
            #         "unit": "MW",
            #         "signed": true,
            #         "notBatched":false,
            #         "uri": "/testcli/testop"
            #     }
            # ]



#client: fims_send -m set -u /testcli/testop '{"TestOPF32toSite": -80000,"TestOPF32toSiteCl": -80000,"TestOPF32toSite_2": 1234}'

#server:fims_listen

# Method:       set
# Uri:          /testsrv/testop
# ReplyTo:      (null)
# Process Name: DNP3_O_hoth
# Username:     root
# Body:         {"TestOPF32toSite":-80000}
# Timestamp:    2023-02-14 21:30:54.469390

# Method:       set
# Uri:          /testsrv/testop
# ReplyTo:      (null)
# Process Name: DNP3_O_hoth
# Username:     root
# Body:         {"TestOPF32toSiteCl":{"value":-80000},"TestOPF32toSite_2":1234}
# Timestamp:    2023-02-14 21:30:55.29269
