
/usr/local/bin/fims/fims_send -m get -r /$$ -u /dbi/ess_controller/status/bms/DemoTotalCharge  | jq
# {
#   "value": 1234,
#   "EnableDbiUpdate": true,
#   "UpdateTimeCfg": 5,
#   "UpdateTimeRemain": 5,
#   "dbiStatus": "OK",
#   "debug": true,
#   "tLast": 529.2387190000009,
#   "actions": {
#     "onSet": [
#       {
#         "func": [
#           {
#             "amap": "bms",
#             "func": "CheckDbiVar"
#           }
#         ]
#       }
#     ]
#   }
# }

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/status/bms '
            { "DemoTotalCharge": { "value": 0.0,"dbiStatus": "init", "debug":true, 
                "actions": {"onSet": [ {"func": [{ "func": "CheckDbiVar", "amap": "bms" }]}]}}}'


/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/dbi/status/bms '
           { "DemoTotalCharge": { "value": 0.0,"dbiStatus": "init", "debug":true, 
             "actions": {"onSet": [ {"func": [{ "func": "CheckDbiResp", "amap": "bms" }]}]}}}' 

/usr/local/bin/fims/fims_send -m get -r /ess/dbi/status/bms/DemoTotalCharge  -u /dbi/ess_controller/status/bms/DemoTotalCharge  | jq

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/reload/CheckDbiVar_DemoTotalCharge_reload  0

/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/dbi/status/bms/DemoTotalCharge  | jq

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/full/status/bms/DemoTotalCharge  1 | jq
