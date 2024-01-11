#!/bin/sh 
# p wilshire 02-16-2022
# Script to demo using SchedItemOpt to send multiple values to different uri's

# // use inline value
#             if (cjuri && cjuri->valuestring && cjval)
#             {
#                 if(debug)FPS_PRINT_INFO( " idx [{}] uri >>>  value to uri ->[{}] "
#                     , idx++
#                     , cjuri->valuestring
#                     );
#                 avi = vm->setValfromCj(vmap, cjuri->valuestring, nullptr, cjval);
#             }
            
echo " Set up  ItemOpts"


#exit 0

fims_send -m set -r/$$ -u /ess/system/bms  '
{
    "set_uri":{
        "value":false,
        "debug": false,
        "ifChanged":false,
        "enabled": false,
        "new_options":[
            {"uri":"/controls/bms/rack_01:CloseContactorCmd","value":{"value":false,"state":"Open","enabled":true}},
            {"uri":"/controls/bms/rack_02:CloseContactorCmd","value":{"value":false,"state":"Open","enabled":true}},
            {"uri":"/controls/bms/rack_03:CloseContactorCmd","value":{"value":false,"state":"Open","enabled":true}},
            {"uri":"/controls/bms/rack_04:CloseContactorCmd","value":{"value":false,"state":"Open","enabled":true}},
            {"uri":"/controls/bms/rack_05:CloseContactorCmd","value":{"value":false,"state":"Open","enabled":true}},
            {"uri":"/controls/bms/rack_06:CloseContactorCmd","value":{"value":false,"state":"Open","enabled":true}},
            {"uri":"/controls/bms/rack_07:CloseContactorCmd","value":{"value":false,"state":"Open","enabled":true}},
            {"uri":"/controls/bms/rack_08:CloseContactorCmd","value":{"value":false,"state":"Open","enabled":true}}
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
    "set_uri":{
        "value":true,
        "enabled": true
        }
}
' | jq


fims_send -m get -r/$$ -u /ess/full/controls/bms  | jq


