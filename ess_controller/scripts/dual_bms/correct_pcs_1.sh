#!/bin/sh
# p wilshire 03-09-2022

fims_send -m set -r /$$ -u /ess/full/status/pcs_1/FaultShutdown '
{
    "value": false,
    "actions": {
      "onSet":[{
          "remap":[
              {                 "uri":    "/status/pcs_1:FaultShutdownSeen"},
              {                 "uri":    "/bms_1/status/pcs_1:FaultShutdown", "fims":"set"},
              {"inValue": true, "uri": "/status/pcs_1:FaultShutdownisTrue"},
              {"inVar": "/status/pcs_1:FaultValue",  "uri": "/status/pcs_1:FaultShutdownmatchFaultValue"},
              {"inAv":  "/status/pcs_1:FaultInValue", "uri": "/status/pcs_1:FaultinValue"},
              {"inValue": true,  "uri": "/status/pcs_1:HardShutdown",             "outValue": false},
              {"inValue": false, "uri": "/status/pcs_1:FullShutdownTime",        "outTime": 5},
              {"inValue": true,  "uri": "/sched/pcs_1:schedShutdownPCS@endTime", "outValue": 0},
              {"inValue": true,  "uri": "/sched/pcs_1:schedShutdownPCS",       "outValue": "schedShutdownPCS"},
              {"enable": "/sched/pcs_1:FaultEnable", "uri": "/sched/pcs_1:schedShutdownESS1", "outValue": "schedShutdownESS"},
              {                  "uri": "/sched/pcs_1:schedShutdownESS2", "outValue": "schedShutdownESS"}
          ]
      }]
    }
}'


