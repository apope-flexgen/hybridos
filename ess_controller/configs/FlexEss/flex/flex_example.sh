#!/bin/sh 

#load in the ess controller spec 

fims_send -f  configs/FlexEss/flex/sample_ess_no_json -m set -u /flex/cfg/cfile/flex/sample_ess_no_json

# set up the first file
fims_send -f  configs/FlexEss/flex/ess_controller -m set -u /flex/cfg/cfile/flex/ess_controller

# set up the ess manager
fims_send -f  configs/FlexEss/flex/ess_manager -m set -u /flex/cfg/cfile/flex/ess_manager

# set up the ess_manager template  .. this will cause it to evaluate the template
fims_send -f  configs/FlexEss/flex/ess_manager_tmpl -m set -u /flex/cfg/ctmpl/flex/ess_manager_tmpl

# these work I think
# set up the bms manager
fims_send -f  configs/FlexEss/flex/bms_manager -m set -u /flex/cfg/cfile/flex/bms_manager

# set up the bms_manager template  .. this will cause it to evaluate the template
fims_send -f  configs/FlexEss/flex/bms_manager_tmpl -m set -u /flex/cfg/ctmpl/flex/bms_manager_tmpl


#TODO 
# set up the pcs manager
fims_send -f  configs/FlexEss/flex/pcs_manager -m set -u /flex/cfg/cfile/flex/pcs_manager

# set up the pcs_manager template  .. this will cause it to evaluate the template
fims_send -f  configs/FlexEss/flex/pcs_manager_tmpl -m set -u /flex/cfg/ctmpl/flex/pcs_manager_tmpl

#TODO
# set up the site_controller
fims_send -f  configs/FlexEss/flex/site_controller -m set -u /flex/cfg/cfile/flex/site_controller

# set up the pcs_manager template  .. this will cause it to evaluate the template
#fims_send -f  configs/FlexEss/flex/pcs_manager_tmpl -m set -u /flex/cfg/ctmpl/flex/pcs_manager_tmpl

# check the result amap
fims_send  -m get -r /$$ -u /flex/full/amap | jq

#TODO send in ess_final which must 
# run links
# run vlinks
# run schedule
