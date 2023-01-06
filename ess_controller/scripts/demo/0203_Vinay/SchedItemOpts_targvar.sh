#!/bin/sh 
# p wilshire 02-16-2022
# Script to demo using SchedItemOpt to send multiple values to the same assetVar

# // if we have a value then set it
# // useAV will use the incoming value
# if (cjuri && cjuri->valuestring && targVal)
# {
#     if(debug)FPS_PRINT_INFO( " idx [{}] targVal >>  value to uri ->[{}] "
#         , idx++
#         , cjuri->valuestring
#         );
#     std::string tv;
#     if(targVar)
#     {
#         tv = fmt::format("{}{}",cjuri->valuestring,targVar);
#     }
#     else
#     {
#         tv = fmt::format("{}",cjuri->valuestring);
#     }     
#     avi = vm->setVal(vmap, tv.c_str(), nullptr, av, "targVal");
# }

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


