#!/bin/sh 
# p wilshire 02-16-2022
# Script to demo using SchedItemOpt to send multiple 

# if(av->gotParam("targfunc"))
#     {
#         tfun = av->getcParam("targfunc");
#         if(tfun)
#         {            
#             if (ams)
#             {
#                 res1 = vm->getFunc(vmap, ams->name.c_str(), tfun);
#             }
#             else if (ais)
#             {
#                 res1 = vm->getFunc(vmap, ais->name.c_str(), tfun);
#             }
#             // use the default using essName if needed
#             if(!res1)
#                 res1 = vm->getFunc(vmap, essName, tfun);
#         }
#     }
#    if (cjuri && cjuri->valuestring && targVal)
#             {
#                 if(debug)FPS_PRINT_INFO( " idx [{}] targVal >>  value to uri ->[{}] "
#                     , idx++
#                     , cjuri->valuestring
#                     );
#                 std::string tv;
#                 if(targVar)
#                 {
#                     tv = fmt::format("{}{}",cjuri->valuestring,targVar);
#                 }
#                 else
#                 {
#                     tv = fmt::format("{}",cjuri->valuestring);
#                 }     
#                 avi = vm->setVal(vmap, tv.c_str(), nullptr, av, "targVal");

#             } 
# if (res1)
#             {
#                 amFunc = reinterpret_cast<myAvfun_t> (res1);
#                 if(ams)
#                     amFunc(vmap, ams->amap, ams->name.c_str(), p_fims, avi?avi:av);
#                 if(ais)
#                     amFunc(vmap, ais->amap, ais->name.c_str(), p_fims, avi?avi:av);

#                 if(debug)FPS_PRINT_INFO( " idx [{}] @@@@@ @@@@ function [{}] uri ->[{}] func [{}]"
#                     , idx++
#                     , avi?avi->getfName():av->getfName()
#                     , tfun
#                     );
#             }
            
echo " Set up  ItemOpts"


#exit 0

fims_send -m set -r/$$ -u /ess/system/bms  '
{
    "set_time":{
        "value":false,
        "debug": false,
        "ifChanged":false,
        "enabled": false,
        "aname":"ess",
        "targVar": "xxx",
        "targfunc":"SendTime",
        "new_options":[
            {"uri":"/controls/bms/rack_01:","value":{"value":0}},
            {"uri":"/controls/bms/rack_02:","value":{"value":0}},
            {"uri":"/controls/bms/rack_03:","value":{"value":0}},
            {"uri":"/controls/bms/rack_04:","value":{"value":0}},
            {"uri":"/controls/bms/rack_05:","value":{"value":0}},
            {"uri":"/controls/bms/rack_06:","value":{"value":0}},
            {"uri":"/controls/bms/rack_07:","value":{"value":0}},
            {"uri":"/controls/bms/rack_08:","value":{"value":0}},
            {"uri":"/controls/bms/rack_09:","value":{"value":0}}
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
    "set_time":{
        "value": true,
        "enabled": true,
        "targVar": "CurrentTime"
        }
}
' | jq


fims_send -m get -r/$$ -u /ess/full/controls/bms  | jq


