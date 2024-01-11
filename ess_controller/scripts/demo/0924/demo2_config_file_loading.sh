#!/bin/sh
# script to supply all the junk the test loader needs
Sys=ess
pause=0
wait_pause()
{
   if [ "$pause" == "1" ] ; then
      echo -n " press enter to continue " && read in
   fi
}
echo
echo "supply ess_config_risen_pe" 
 fims_send -f configs/FlexEss/flex/ess_config_risen_pe.json -m set  -u /ess/cfg/cfile/ess/ess_config_risen_pe
 fims_send -m get -r /$$ -u /ess/full/config | jq

echo "next send  site controller modbus maps " 
wait_pause

 fims_send -f configs/FlexEss/flex/site_modbus.json -m set  -u /ess/cfg/ctmpl/ess/site_modbus
 fims_send -m get -r /$$ -u /ess/full/config | jq
 echo "next send  ess_manager " 
 echo "           rizen_bms_manager " 
 echo "           pe_pcs_manager " 
 echo "           site_manager " 
wait_pause

 fims_send -f configs/FlexEss/flex/ess_manager.json -m set        -u /ess/cfg/cfile/ess/ess_manager
 fims_send -f configs/FlexEss/flex/risen_bms_manager.json -m set  -u /ess/cfg/cfile/ess/risen_bms_manager
 fims_send -f configs/FlexEss/flex/pe_pcs_manager.json -m set     -u /ess/cfg/cfile/ess/pe_pcs_manager
 fims_send -f configs/FlexEss/flex/site_manager.json -m set       -u /ess/cfg/cfile/ess/site_manager
 fims_send -m get -r /$$ -u /ess/full/config/load | jq
 #fims_send -m get -r /$$ -u /ess/full/config/tmpl | jq

 echo "next send  ess_controller "
wait_pause

 fims_send -f configs/FlexEss/flex/ess_controller.json -m set  -u /ess/cfg/cfile/ss/ess_controller
 fims_send -m get -r /$$ -u /ess/full/config/load | jq
 fims_send -m get -r /$$ -u /ess/full/config/cfile | jq
 #fims_send -m get -r /$$ -u /ess/config/tmpl | jq

 echo "next send  templates "
wait_pause

 fims_send -f configs/FlexEss/flex/ess_manager_tmpl.json -m set    -u /ess/cfg/ctmpl/ess/ess_manager_tmpl
 fims_send -f configs/FlexEss/flex/pe_pcs_template.json -m set     -u /ess/cfg/ctmpl/ess/pe_pcs_template
 fims_send -f configs/FlexEss/flex/risen_bms_template.json -m set  -u /ess/cfg/ctmpl/ess/risen_bms_template
 #fims_send -m get -r /$$ -u /ess/config/tmpl | jq

 echo "inspect the asset mapping "
wait_pause

 fims_send -m get -r /$$ -u /ess/amap | jq

 echo "inspect site/ess_hs , site/ess_hs "
wait_pause

 fims_send -m get -r /$$ -u /ess/full/site/ess_ls | jq
 fims_send -m get -r /$$ -u /ess/full/site/ess_hs | jq

