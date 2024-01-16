#!/bin/sh
# script to supply all the junk the test loader needs
# inspect the first config loader 
SYS=/ess
pause=1
wait_pause()
{
   if [ "$pause" == "1" ] ; then
      echo -n " press enter to continue " && read in
   fi
}
echo
echo "inspect sample_ess_no_json" 
 cat configs/FlexEss/flex/sample_ess_no_json

echo
echo "inspect the top part of ess_manager" 
wait_pause
 head -30  configs/FlexEss/flex/ess_manager
 
echo
echo "inspect the top part of risen_bms_manager" 
wait_pause
 head -35  configs/FlexEss/flex/risen_bms_manager

echo
echo "inspect the top part of pe_pcs_manager" 
wait_pause
 head -40  configs/FlexEss/flex/pe_pcs_manager


echo
echo "inspect the top part of site_manager" 
wait_pause
 head -40  configs/FlexEss/flex/site_manager

echo
echo "inspect the top part of site_modbus" 
wait_pause
 head -40  configs/FlexEss/flex/site_modbus

