#!/usr/bin/sh


#Note script originally used SOC, SOH, Voltage. Changed to MaxCellVoltage, MinCellVoltage, MaxCellTemp for demo. Variable names unchanged.
red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`
ess="ess"

echo setup AllLocks command
echo "set up /system/commands:locks"; echo
/usr/local/bin/fims_send -m set -r /me -u /$ess/system/commands '
         {"locks":{"value":"test",
                    "help": "lock all locked tables",
                    "ifChanged":false, "enabled":false,
                    "debug": true,
                    "actions":{"onSet":[{"func":[{"func":"runAllLocks"}]}]}}}'

echo "Set up /system/bms for test purposes"; echo
/usr/local/bin/fims_send -m set -r /me -u /$ess/status/bms '
      {"Voltage": {"MaxAlarmThreshold": 0, "MinAlarmThreshold": 0, "MaxResetValue": 0, "MaxSetVal": 20}
      }'

MAXAL_START=$(/usr/local/bin/fims_send -m get -r /me -u /$ess/naked/status/bms/Voltage:MaxAlarmThreshold)
echo $MAXAL_START
MINAL_START=$(/usr/local/bin/fims_send -m get -r /me -u /$ess/naked/status/bms/Voltage:MinAlarmThreshold)
MAXRE_START=$(/usr/local/bin/fims_send -m get -r /me -u /$ess/naked/status/bms/Voltage:MaxResetValue)



echo "MaxAlarmThreshold is: ${green}$MAXAL_START${reset}. MinAlarmThreshold is: ${green}$MINAL_START${reset}. MaxResetValue is: ${green}$MAXRE_START${reset} "

read -rsn1 -p"Press any key to continue";echo
echo "Enable locking. Expect no locks added since config locking table not yet configured"; echo
/usr/local/bin/fims_send -m set -r /me -u /$ess/system/commands/locks '
      {"value":"test", "enabled":true}'

echo "Update values in /system/bms. Expect variables to be unlocked and values to change"
echo "Set up /system/bms for test purposes"; echo
/usr/local/bin/fims_send -m set -r /me -u /$ess/status/bms/Voltage@MaxAlarmThreshold ' 
      {"value": 60}'
/usr/local/bin/fims_send -m set -r /me -u /$ess/status/bms/Voltage@MinAlarmThreshold ' 
      {"value": -3}'
/usr/local/bin/fims_send -m set -r /me -u /$ess/status/bms/Voltage@MaxResetValue ' 
      {"value": 4}'

FAIL=false
soctext=$green
sohtext=$green
voltagetext=$green
MAXAL_UPDT=$(/usr/local/bin/fims_send -m get -r /me -u /$ess/status/bms/Voltage:MaxAlarmThreshold)
MINAL_UPDT=$(/usr/local/bin/fims_send -m get -r /me -u /$ess/status/bms/Voltage:MinAlarmThreshold)
MAXRE_UPDT=$(/usr/local/bin/fims_send -m get -r /me -u /$ess/status/bms/Voltage:MaxResetValue)

if [ $MAXAL_UPDT -ne 4 ]
then
soctext=$red
FAIL=true
fi

if [ $MINAL_UPDT -ne 3 ]
then
sohtext=$red
FAIL=true
fi

if [ $MAXRE_UPDT -ne 60 ]
then
voltagetext=$red
FAIL=true
fi

echo "##################"
if $FAIL
then
echo -e "${red}    TEST FAIL${reset}"
else
echo -e "${green}    TEST PASS${reset}"
fi
echo "##################"; echo

echo "Expected MaxCellVoltage: 4. MinCellVoltage: 3. MaxCellTemp: 60"
echo -e "Actual MaxCellVoltage: ${soctext}$MAXAL_UPDT${reset}. MinCellVoltage: ${sohtext}$MINAL_UPDT${reset}. MaxCellTemp: ${voltagetext}$MAXRE_UPDT${reset}."

read -rsn1 -p"Press any key to continue";echo
echo "Create /config/locks table with entry {\"/status/bms\": {\"value\":true}}"; echo
/usr/local/bin/fims_send -m set -r /me -u /$ess/config/locks '
      {"/status/bms": {"value":true}}'

echo "Re-run runAllLocks. Expect locks added to entries in /status/bms"; echo
/usr/local/bin/fims_send -m set -r /me -u /$ess/system/commands/locks '
      {"value":"test"}'

read -rsn1 -p"Press any key to continue";echo
echo "Attempt to change values for entries in /status/bms. Expect all to be locked"; echo
/usr/local/bin/fims_send -m set -r /me -u /$ess/status/bms ' 
      {"MaxCellTemp": {"value": 65},
        "MinCellVoltage": {"value": 2},
        "MaxCellVoltage": {"value": 5}
      }'
#echo "Expect MaxCellTemp: 0, MinCellVoltage: 0, MaxCellVoltage:0"; echo
/usr/local/bin/fims_send -m get -r /me -u /$ess/status/bms | jq

FAIL=false
soctext=$green
sohtext=$green
voltagetext=$green
SOC_NEW=$(/usr/local/bin/fims_send -m get -r /me -u /$ess/naked/status/bms/MaxCellVoltage)
SOH_NEW=$(/usr/local/bin/fims_send -m get -r /me -u /$ess/naked/status/bms/MinCellVoltage)
VOLTAGE_NEW=$(/usr/local/bin/fims_send -m get -r /me -u /$ess/naked/status/bms/MaxCellTemp)

if [ $SOC_NEW -ne $SOC_UPDT ]
then
soctext=$red
FAIL=true
fi

if [ $SOH_NEW -ne $SOH_UPDT ]
then
sohtext=$red
FAIL=true
fi

if [ $VOLTAGE_NEW -ne $VOLTAGE_UPDT ]
then
voltagetext=$red
FAIL=true
fi

echo "##################"
if $FAIL
then
echo -e "${red}    TEST FAIL${reset}"
else
echo -e "${green}    TEST PASS${reset}"
fi
echo "##################"; echo

echo "Expected MaxCellVoltage: $SOC_UPDT. MinCellVoltage: $SOH_UPDT. MaxCellTemp: $VOLTAGE_UPDT"
echo -e "Actual MaxCellVoltage: ${soctext}$SOC_NEW${reset}. MinCellVoltage: ${sohtext}$SOH_NEW${reset}. MaxCellTemp: ${voltagetext}$VOLTAGE_NEW${reset}."


read -rsn1 -p"Press any key to continue";echo
echo "Unlock MaxCellVoltage from /status/bms"; echo
/usr/local/bin/fims_send -m set -r /me -u /$ess/config/locks '
      {"/status/bms:MaxCellVoltage": {"value":false}}'

echo "Re-run runAllLocks. Expect locks added to entries in /status/bms, then MaxCellVoltage unlocked"; echo
/usr/local/bin/fims_send -m set -r /me -u /$ess/system/commands/locks '
      {"value":"test"}'

read -rsn1 -p"Press any key to continue";echo
echo "Attempt to change values for entries in /status/bms. Expect all except MaxCellVoltage to be locked"; echo
/usr/local/bin/fims_send -m set -r /me -u /$ess/status/bms ' 
      {"MaxCellTemp": {"value": 65},
        "MinCellVoltage": {"value": 2},
        "MaxCellVoltage": {"value": 5}
      }'

FAIL=false
soctext=$green
sohtext=$green
voltagetext=$green
SOC_NEW=$(/usr/local/bin/fims_send -m get -r /me -u /$ess/naked/status/bms/MaxCellVoltage)
SOH_NEW=$(/usr/local/bin/fims_send -m get -r /me -u /$ess/naked/status/bms/MinCellVoltage)
VOLTAGE_NEW=$(/usr/local/bin/fims_send -m get -r /me -u /$ess/naked/status/bms/MaxCellTemp)
if [ $SOC_NEW -ne 4.5 ]
then
soctext=$red
FAIL=true
fi

if [ $SOH_NEW -ne $SOH_UPDT ]
then
sohtext=$red
FAIL=true
fi

if [ $VOLTAGE_NEW -ne $VOLTAGE_UPDT ]
then
voltagetext=$red
FAIL=true
fi

echo "##################"
if $FAIL
then
echo -e "${red}    TEST FAIL${reset}"
else
echo -e "${green}    TEST PASS${reset}"
fi
echo "##################"; echo

echo "Expected MaxCellVoltage: 5. MinCellVoltage: 3. MaxCellTemp: 60"
echo -e "Actual MaxCellVoltage: ${soctext}$SOC_NEW${reset}. MinCellVoltage: ${sohtext}$SOH_NEW${reset}. MaxCellTemp: ${voltagetext}$VOLTAGE_NEW${reset}."

#echo "Expect Voltage: 1500, SOH: 100, SOC: 90"; echo
/usr/local/bin/fims_send -m get -r /me -u /$ess/status/bms | jq

#"/config/locks":{
#
#      "/status/bms": true,
#}