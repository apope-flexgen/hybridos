#!/bin/sh 
# p wilshire 02-16-2022
# Script to demo using SchedItemOpt to send multiple values to the same assetVar

# // use inline value
#             if (cjuri && cjuri->valuestring && cjval)
#             {
#                 if(debug)FPS_PRINT_INFO( " idx [{}] uri >>>  value to uri ->[{}] "
#                     , idx++
#                     , cjuri->valuestring
#                     );
#                 avi = vm->setValfromCj(vmap, cjuri->valuestring, nullptr, cjval);
#             }
#             if(tav)
#             {
#                 if(debug)FPS_PRINT_INFO( " idx [{}] tav >>> cji  to uri [{}] "
#                     , idx++
#                     , tav->getfName()
#                     );
#                 vm->setValfromCj(vmap, tav->getfName(), nullptr, cji);
#             }
            
echo " Set up  ItemOpts"


#exit 0

fims_send -m set -r/$$ -u /ess/system/bms  '
{
    "set_var":{
        "value":false,
        "debug": false,
        "ifChanged":false,
        "enabled": false,
        "targVar": "ChargeCurrent",
        "targVal": 2300,
        "new_options":[
            {"uri":"/controls/bms/rack_01:"},
            {"uri":"/controls/bms/rack_02:"},
            {"uri":"/controls/bms/rack_03:"},
            {"uri":"/controls/bms/rack_04:"},
            {"uri":"/controls/bms/rack_05:"},
            {"uri":"/controls/bms/rack_06:"},
            {"uri":"/controls/bms/rack_07:"},
            {"uri":"/controls/bms/rack_08:"}
        ],
        "actions":{
            "onSet": [{
                "func":[{"debug":true,"func":"SchedItemOpts","amap":"ess"}]
            }]
        }
    }
}
' | jq


fims_send -m set -r/$$ -u /ess/system/bms  '
{
    "set_var":{
        "value":true,
        "enabled": true,
        "targVar": "ChargeCurrent",
        "targVal": 2300
        }
}
' | jq

fims_send -m set -r/$$ -u /ess/system/bms  '
{
    "set_var":{
        "value":true,
        "enabled": true,
        "targVar": "DischargeCurrent",
        "targVal": -2300
        }
}
' | jq

fims_send -m get -r/$$ -u /ess/full/controls/bms  | jq


