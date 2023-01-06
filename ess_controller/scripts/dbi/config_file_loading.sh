#!/bin/sh
# script to supply all the junk the test loader needs
Sys=ess
pause=0
if [ $# > 1 ] ; then
   pause=1
fi


wait_pause()
{
   if [ "$pause" == "1" ] ; then
      echo -n " press enter to continue " && read in
   fi
}

#wait_pause

#exit


echo
echo "send ess_config_risen_pe and inspect /config/load" 
 fims_send -f configs/FlexEss/flex/ess_config_risen_pe.json -m set  -u /ess/cfg/cfile/ess/ess_config_risen_pe
wait_pause
 fims_send -m get -r /$$ -u /ess/full/config/load/ess_controller | jq
#  {
#   "name": "sample config for ess",
#   "ess_controller": {
#     "value": false,
#     "aname": "ess",
#     "file": "ess_controller",  << next send this 
#     "final": "ess_final",
#     "type": "master",
#     "options": [
#       {
#         "load": "ess_manager",  << then send this 
#         "aname": "ess",
#         "summary": "ess/summary",
#         "svec": "ess/ess_"
#       },
#       {
#         "load": "risen_bms_manager",
#         "aname": "bms",
#         "pname": "ess",
#         "summary": "bms/summary",
#         "svec": "bms/rack_"
#       },
#       {
#         "load": "pe_pcs_manager",
#         "aname": "pcs",
#         "pname": "ess",
#         "summary": "pcs/summary",
#         "svec": "pcs/module_"
#       },
#       {
#         "load": "site_manager",
#         "aname": "site",
#         "pname": "ess"
#       },
#       {
#         "site": "site_modbus",
#         "uri": "/config/tmpl:site_modbus",
#         "aname": "site",
#         "pname": "ess"
#       }
#     ]
#   }
# }

# {
#   "ess_config_risen_pe": {
#     "value": true,
#     "md5sum": "73eee01d2ce833c5f1150bca6292fbfc",  << we got this one 
#     "reqResp": 20.824570999997377
#   },
#   "ess_config_risen_sungrow": {   << ignore his one it was requested bt default
#     "value": false,
#     "aname": "ess",
#     "file": "ess_config_risen_sungrow",
#     "reqCount": 5,
#     "reqTimeout": 26.003687999997055
#   },
#   "ess_controller": {
#     "value": false,
#     "aname": "ess",
#     "file": "ess_controller",
#     "reqCount": 5,
#     "reqTimeout": 49.00370000000112
#   },
#   "ess_manager": {
#     "value": false,
#     "aname": "ess",
#     "file": "ess_manager",
#     "reqCount": 5,
#     "reqTimeout": 49.00370000000112
#   },
#   "pe_pcs_manager": {
#     "value": false,
#     "aname": "pcs",
#     "file": "pe_pcs_manager",
#     "pname": "ess",
#     "reqCount": 5,
#     "reqTimeout": 49.00370000000112
#   },
#   "risen_bms_manager": {
#     "value": false,
#     "aname": "bms",
#     "file": "risen_bms_manager",
#     "pname": "ess",
#     "reqCount": 5,
#     "reqTimeout": 49.00370000000112
#   },
#   "site_manager": {
#     "value": false,
#     "aname": "site",
#     "file": "site_manager",
#     "pname": "ess",
#     "reqCount": 5,
#     "reqTimeout": 49.00370000000112
#   }
# }


echo "next send  ess_controller (no crash version)" 
wait_pause
 fims_send -f configs/FlexEss/flex/ess_controller.json -m set  -u /ess/cfg/cfile/ss/ess_controller


echo "next send  ess_manager load file " 
wait_pause
 fims_send -f configs/FlexEss/flex/ess_manager.json -m set        -u /ess/cfg/cfile/ess/ess_manager

#  "ess_manager": {
#     "value": true,
#     "aname": "ess",
#     "file": "ess_manager",
#     "md5sum": "aee2cc18428e4cb735400809c3ec5f24",
#     "reqCount": 5,
#     "reqResp": 350.870640000001,
#     "reqTimeout": 49.00370000000112
#   },

# see the new loader 
# it needs a template
#   "ess_manager": {
#     "value": false,
#     "aname": "ess",
#     "file": "ess_manager",
#     "options": [
#       {
#         "tmpl": "ess_manager_tmpl",
#         "pname": "ess",
#         "amname": "##ESS_ID##",
#         "from": 1,
#         "to": 1,
#         "reps": [
#           {
#             "replace": "##ESS_ID##",
#             "with": "ess_{:d}"
#           },
#           {
#             "replace": "##ESS_NUM##",
#             "with": "{:d}"
#           }
#         ]
#       }
#     ]
#   }
# }

echo "next send  ess_manager tmpl file " 
wait_pause
 fims_send -f configs/FlexEss/flex/ess_manager_tmpl.json -m set    -u /ess/cfg/ctmpl/ess/ess_manager_tmpl

# we have one happy manager,  template has been expanded
#
#   "ess_manager": {
#     "value": true,
#     "aname": "ess",
#     "configDone": true,
#     "file": "ess_manager",
#     "loadComplete": true,
#     "options": [
#       {
#         "tmpl": "ess_manager_tmpl",
#         "pname": "ess",
#         "amname": "##ESS_ID##",
#         "from": 1,
#         "to": 1,
#         "reps": [
#           {
#             "replace": "##ESS_ID##",
#             "with": "ess_{:d}"
#           },
#           {
#             "replace": "##ESS_NUM##",
#             "with": "{:d}"
#           }
#         ]
#       }
#     ]
#   }
# }
#  fims_send -m get -r /$$ -u /ess/full/amap | jq
# {
#   "aMs": [
#     {
#       "name": "ess",
#       "aMs": 1,
#       "aIs": 0
#     },
#     {
#       "name": "ess_1",
#       "p-name": "ess",
#       "aMs": 0,
#       "aIs": 0
#     }
#   ]
# }

echo "inspect load:ess_controller " 
wait_pause
 fims_send -m get -r /$$ -u /ess/full/config/load/ess_controller | jq

echo "inspect load:ess_manager " 
wait_pause
 fims_send -m get -r /$$ -u /ess/full/config/load/ess_manager | jq

echo "next send  pe_pcs_manager " 
wait_pause
 fims_send -f configs/FlexEss/flex/pe_pcs_manager.json -m set     -u /ess/cfg/cfile/ess/pe_pcs_manager

#   "pe_pcs_manager": {
#     "value": false,
#     "aname": "pcs",
#     "file": "pe_pcs_manager",
#     "pname": "ess",
#     "options": [
#       {
#         "tmpl": "pe_pcs_template",
#         "pname": "pcs",
#         "amname": "##PCS_ID##",
#         "from": 1,
#         "to": 5,
#         "reps": [
#           {
#             "replace": "##PCS_ID##",
#             "with": "module_{:02d}"
#           },
#           {
#             "replace": "##PCS_NUM##",
#             "with": "{:02d}"
#           }
#         ]
#       }
#     ]
#   }
# }

echo "and the pe_pcs_manager tmpl" 
wait_pause
 fims_send -f configs/FlexEss/flex/pe_pcs_template.json -m set     -u /ess/cfg/ctmpl/ess/pe_pcs_template


echo "inspect load:pe_pcs_manager " 
wait_pause
 fims_send -m get -r /$$ -u /ess/full/config/load/pe_pcs_manager | jq


echo "next send  rizen_bms_manager " 
wait_pause
 fims_send -f configs/FlexEss/flex/risen_bms_manager.json -m set     -u /ess/cfg/cfile/ess/risen_bms_manager

 echo "next send  risen_bms_templates ; note this takes a few seconds to load"
wait_pause
 fims_send -f configs/FlexEss/flex/risen_bms_template.json -m set  -u /ess/cfg/ctmpl/ess/risen_bms_template

sleep 1
 echo "waiting 1"

sleep 1
 echo "waiting 2"

sleep 1
 echo "waiting 3"

sleep 1
 echo "waiting 4"

echo "inspect load:risen_bms_manager " 
wait_pause
 fims_send -m get -r /$$ -u /ess/full/config/load/risen_bms_manager | jq

echo "next send  site_manager " 
wait_pause
 fims_send -f configs/FlexEss/flex/site_manager.json -m set     -u /ess/cfg/cfile/ess/site_manager

echo "inspect load:site_manager " 
wait_pause
 fims_send -m get -r /$$ -u /ess/full/config/load/site_manager | jq

wait_pause

echo "next send  site controller modbus maps as a template " 
wait_pause

 fims_send -f configs/FlexEss/flex/site_modbus.json -m set  -u /ess/cfg/ctmpl/ess/site_modbus

echo "inspect load:ess_controller " 
wait_pause
 fims_send -m get -r /$$ -u /ess/full/config/load/ess_controller | jq



exit


