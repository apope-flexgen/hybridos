#!/bin/sh 
# p wilshire 02-16-2022
# Script to demo using SchedItemOpt to send multiple values to the same assetVar

# // use inline value
#   
# if(av->gotParam("targav"))
#     {
#         turi = av->getcParam("targav");
#         if(turi)
#             tav  = vm->getVar(vmap, turi, nullptr);
#     }
#             if(tav)
#             {
#                 if(debug)FPS_PRINT_INFO( " idx [{}] tav >>> cji  to uri [{}] "
#                     , idx++
#                     , tav->getfName()
#                     );
#                 vm->setValfromCj(vmap, tav->getfName(), nullptr, cji);
#             }

#
# sends each option to the targav

echo " Set up  ItemOpts"


#exit 0

fims_send -m set -r/$$ -u /ess/system/bms  '
{
    "set_tav":{
        "value":false,
        "debug": false,
        "ifChanged":false,
        "enabled": false,
        "targav": "/controls/ess:some_command",
        "new_options":[
            {"value":3452,"uri":"/controls/bms/rack_01:MaxChargeCurrent","limit":4567},
            {"value":3452,"uri":"/controls/bms/rack_02:MaxChargeCurrent","limit":4567},
            {"value":3452,"uri":"/controls/bms/rack_03:MaxChargeCurrent","limit":4567},
            {"value":3452,"uri":"/controls/bms/rack_04:MaxChargeCurrent","limit":4567},
            {"value":3452,"uri":"/controls/bms/rack_05:MaxChargeCurrent","limit":4567},
            {"value":3452,"uri":"/controls/bms/rack_06:MaxChargeCurrent","limit":4567}
        ],
        "actions":{
            "onSet": [{
                "func":[{"debug":true,"func":"SchedItemOpts","amap":"ess"}]
            }]
        }
    }
}
' | jq


fims_send -m set -r/$$ -u /ess/controls/ess  '
{
    "some_command":123
}' | jq

fims_send -m set -r/$$ -u /ess/system/bms  '
{
    "set_tav":{
        "value":true,
        "enabled": true
        }
}
' | jq


fims_send -m get -r/$$ -u /ess/full/controls/ess  | jq


