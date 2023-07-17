#!/bin/sh 
# 02_14_2023

# mini test script to check a variable update timeout 

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
            #         "xvariation": "Group1Var1",
            #         "xevariation": "Group2Var3",
            #         "Xevariation": "Group2Var2",
            #         "xclazz": 1,
            #         "uri": "/sites/analog",
            #         "name": "Analog1G1V1"
            #     },

dnp3_server /home/docker/configs/server/dnp3_server_test_306a.json
#repeat this twice
timeout 10 watch -n 0.5  'fims_send -m pub -u /sites/analog/analog_1 1234'


#Server log

# >>>>>> Num Uris found 6 .

# >>>>>>>>>> Uri [0] [/sites/hoth]
# >>>>>>>>>> Uri [1] [/local_server]
# >>>>>>>>>> Uri [2] [/sites/analog]
# >>>>>>>>>> Uri [3] [/sites/binary]
# >>>>>>>>>> Uri [4] [/sites/binaryOS]
# >>>>>>>>>> Uri [5] [/testsrv/testop]
#  ConfigureDatabase >>  analogOS size = [1]
#  ConfigureDatabase >>  counter size = [1]
#  ConfigureDatabase >>  binary size = [3]
# fps channel state change: [OPENING]
# Outstation setup OK id [hoth] master 1 station 10
# DNP3 Server Setup complete: Entering main loop.
#  Time [1.043] State Init  db [binary_1Cl] last [-1.000] timeout [0.000]
#  Time [1.043] State Init  db [binaryOS_10v2] last [-1.000] timeout [0.000]
#  Time [31.353] Variable Update Failed;  db [analog_1] last [25.938] timeout [5.000]
# fps channel state change: [OPEN]
#  Time [40.280] Variable Update Restored; db [analog_1] last [25.938] timeout [5.000]
#  Time [55.613] Variable Update Failed;  db [analog_1] last [49.894] timeout [5.000]
