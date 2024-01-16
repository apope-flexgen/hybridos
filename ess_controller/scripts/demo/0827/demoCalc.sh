#!/bin/sh
###########################################################################
##
## Patch file to add the Power Aux Calculation to the running system
## can be added at any time after startup. 
##
##  Author: Phil Wilshire
##  Date:    08/27/2010
##  Version: 1.0
##  Sites : Tx100_NF Tx100_BC
##  Date Deployed :
##
##  
###########################################################################

#
# Set up the working variables
echo
echo  "Set up the working variables Pac Pdc"
fims_send -m set -r /$$ -u /flex/full/status/pcs '
{
  "Pac": {
    "value": 500
  },
 "Pdc": {
    "value": 1000
  }
}'  | jq

# Set up the Calcuation,
# this is attached to the output "/status/pcs:Paux"
# The varable value is updated whenever the CalculateVar function is activated for "/status/pcs:Paux" 
# the value here is not used but is set when the calculation has finished.
# note that when the calculation has completed the "remap" action will then send 
# the value to the output variable /site/ess_hs:AuxPower.
#
# this could be optimized more by adding the calculation directly to the output variable.
# 
echo
echo "setting up /status/pcs:Paux value 0 to hold calculation result"
fims_send -m set -r /$$ -u /flex/full/status/pcs '
{
  "Paux": {
    "value": 1,
    "useExpr": true,
    "numVars":3,
    "variable1":"/status/pcs:Pdc",
    "variable2":"/config/pcs:PdcEff",
    "variable3":"/status/pcs:Pac",
    "expression": "({1} * {2}) - {3}",
    "ifChanged":false,
    "debug":true,
    "actions": {
      "onSet": [{
          "remap": [
            {"amap": "pcs","uri": "/site/ess_hs:AuxPower"}
          ]}]}
  }
}' | jq

# Set up the config variable
echo
echo "setting up /config/pcs:PdcEff value 0.85"
fims_send -m set -r /$$ -u /flex/full/config/pcs '
{
 "PdcEff": {
    "value": 0.85
  }
}' | jq

# echo
# echo  "Set up the working variables Pac Pdc once again"
# fims_send -m set -r /$$ -u /flex/full/status/pcs '
# {
#   "Pac": {
#     "value": 500
#   },
#  "Pdc": {
#     "value": 1000
#   }
# }'

#
# Set up the control variable /control/pcs:runPaux (Used for testing) 
# this is a way of triggering the calculation in a test mode.
# the inAv is a FlexPack extesion so you'll need release v1.0.1+ or FlexPack
#
echo
echo "setting up /control/pcs:runPaux"
fims_send -m set -r /$$ -u /flex/full/control/pcs '
{
 "runPaux": {
    "value": 0.0,
    "ifChanged":false,
    "debug":true,
    "actions": {
      "onSet": [{ 
          "func": [
            {"inAv":"/status/pcs:Paux","amap": "pcs","func": "CalculateVar"}
          ]}]}
  }
}' | jq
#
# here are the modbus inputs mapped (this time) into the /status/pcs area  
# note, currently the actions are overwritten so any actions attached 
# to the designated inputs have to be included in this object
#  
echo
echo "setting up variables to capture the modbus inputs : /components/pcs_modbus_input"
fims_send -m set -r /$$ -u /flex/full/components/pcs_modbus_input '
{
    "dc_power":{
        "value":0,
        "actions":{
           "onSet": [
           {
                "remap": [
                {"amap": "pcs","uri": "/status/pcs:Pdc"}
           ]}]}
    },
    "ac_power":{
        "value":0,
        "actions": {
        "onSet": [{
          "remap": [
              {"amap": "pcs","uri": "/status/pcs:Pac"}
          ]}]}
    }
}' | jq

#  Using Links
#  Another way to directly map incoming mobus objects used "Links"
#  This , currently (v1.0.0), has to be done during the initial configuration
#    since the link evaluation is hard coded to occur after the config file load.
# FlexPack ( v1.0.1 and up) has a dynamic option with a little more control.
# this allows links to be evaluated at any time.
# the link work by adding the name Pdc to the pcs map and make it a pointer or reference
# to  /components/pcs_modbus_input:dc_power.
# this works with no added code so its fast.
# The var "Pdc" is added (if needed) into pcs mapping space.
# The "local" variable "/status/pcs:Pdc" can be created and this, in turn will also allow
# the variable to be available for internal mapping.
# Note you cannot specify actions and parameters for"/status/pcs:Pdc" in any conifig file if it used as a "linked" variable.
# the link option discards the "/status/pcs:Pdc" already created during config load and replaces the variable referenced
# by the "/components/pcs_modbus_input:dc_power" variable.
# After the link has been established , any changes to either 
#   "/status/pcs:Pdc" or "/components/pcs_modbus_input:dc_power" will affect both components. 
echo
echo 'set up links (not needed for this demo)'
fims_send -m set -r /$$ -u /flex/links/pcs '
{
    "Pdc":{"value":"/components/pcs_modbus_input:dc_power"},
    "Pac":{"value":"/components/pcs_modbus_input:ac_power"}
}' | jq

# ess_controller v1.0.0 uses the "wake_monitor" system to run all the scheduled 
# fixed monitorin functions.
# this , currently, has to be configured and set up in the startup config file.
# The following entry will do that.
# Just add this to the pcs_manager.json file.
# note that the configs accumulate. So this entry does not affect any other items alread placed in 
# /schedule/wake_monitor/pcs
# Actually you can add ths dynamically after start up. YOu just have to  trigger a "reload" in the 100mS function.
#
echo
echo 'set up wake up (not needed for this demo)'
fims_send -m set -r /$$ -u /flex/schedule/wake_monitor/pcs '
{
    "/status/pcs:Paux": { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"}
}' | jq

echo
echo  "Set up the working variables Pac Pdc"
fims_send -m set -r /$$ -u /flex/full/status/pcs '
{
  "Pac": {
    "value": 500
  },
 "Pdc": {
    "value": 1000
  }
}' | jq
# here we try and test the system.
# In this example a "set" to "/control/pcs/runPaux" causes the 
# Action 
echo
echo "now trigger a test run of the calculation"
fims_send -m set -r /$$ -u /flex/control/pcs/runPaux  1
#{"runPaux":1237}
echo
echo "show the interim value /status/pcs:Paux"
fims_send -m get -r /$$ -u /flex/status/pcs/Paux

echo 
echo "show the output /site/ess_hs:AuxPower"
fims_send -m get -r /$$ -u /flex/site/ess_hs/AuxPower
# expect 99.15

#sh scripts/demo/0827/demoCalc.sh
#{"Pac":{"value":500},"Pdc":{"value":1000},"PdcEff":{"value":0.85},"runPaux":{"value":0.85,"ifChanged":false,"debug":true,"actions":{"onSet":[{"func":[{"inAv":"/status/pcs:Paux","amap":"flex","func":"CalculateVar"}]}]}},"Paux":{"value":0.1,"includeCurrVal":true,"useExpr":true,"numVars":3,"variable1":"/status/pcs:Pdc","variable2":"/status/pcs:PdcEff","variable3":"/status/pcs:Pac","expression":"{1} * {2} - {3}","ifChanged":false,"debug":true,"actions":{"onSet":[{"remap":[{"amap":"pcs","uri":"/site/ess_hs:PowerAux"}]}]}}}
#{"runPaux":1237}
#look at out interim value /site/pcs/Paux
#99.15
#look at output /site/ess_hs
#99.15


