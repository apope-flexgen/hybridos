## Interim Test Environemnt

Pending the development of the full system test infrastructure and to allow more complex sequences to be verified a "simple" shell based 
test infrastructure has been developed.
This is intended to be seamlessly transferable to the dev-ops test environment when that is devloped.


The Shell based system is desined to be simple to impliment and deploy, yet provide an essential verification and test environment for code development.


## Basic principles

The system sends a series of fims messages to the ess_controller and collects the responses.
The responses are filtered and then compared with expected responses.
If the filtered responses match then the test passes.

A series of tests and responses are contained in a test definition file.
The test execution script sources the definition file and sets up arrays to run the tests.
The test is simply executed as follows:-

```
#$1 desc
#$2 act
#$3 resp
#$4 outfile
#$5 logfile
#
# globals 
# num_passed, num_failed
# fails() an array

function run_test()
{
    echo " >>>>>> $1"
    #echo " >>>>>> $2"
    act_resp=$(eval $2)

    echo " #####################"  >> $5
    echo " test [$1]"              >> $5
    echo " request [$2]"           >> $5
    echo " act_resp [$act_resp]"   >> $5
    echo " exp_resp [$3]"          >> $5
    
    # this a  very simple a == b comparison test  
    if [ "$act_resp" = "$3" ] ; then
        echo  " test [passed]  >> $1" >> $4
        num_passed=$(($num_passed+1))
    else
        echo " test [failed] >> $1 "  >> $4
        echo " act_resp >> [$act_resp]"  >> $4
        echo " exp_resp >> [$3]"         >> $4
        num_failed=$(($num_failed+1))
        fails+=($foo)
    fi

}

```

## Sample Def file

This is a sample def file

```
# defs for 401.1_test_dbi_var.sh
# p. wilshire
# 10/22/2021
#
# the test script dots in this to run the config set up by 01.1
# it can be tricky to match exact responses so use grep -v to filter out stuff that changes regardless.
# also the responses will be in alpha order.
#

FimsDir=/usr/local/bin/

descs=()
acts=()
resps=()

#######################################################################
descs[0]=" >> test dbi var "
#######################################################################
idx=1
sleeps[idx]="0.1" 
idx=$(($idx+1))

descs[idx]=" >> setup dbi response vars first "
acts[idx]=$(cat<<EOF
${FimsDir}fims_send -m set  -r /$$ -u /ess/full/dbi/controls/bms ' 
{
    "DemoChargeCurrent":{"value": -9999,"dbiSet":false,
       "actions": {"onSet": [{"func": [{"amap": "ess","func": "CheckDbiResp"}]}]}
    }
}' | jq
EOF
)
resps[idx]=$(cat<<EOF
{
    "DemoChargeCurrent":{"value": -9999,"dbiSet":false,
       "actions": {"onSet": [{"func": [{"amap": "ess","func": "CheckDbiResp"}]}]}
    }
}
EOF
)
sleeps[idx]="0.1" 
idx=$(($idx+1))


descs[idx]=" >> setup Dbivar DemoChargeCurrent"
acts[idx]=$(cat<<EOF
${FimsDir}fims_send -m set  -u /dbi/ess_controller/dbivars_controls_bms_DemoChargeCurrent '
{
    "value": 5555
}'
EOF
)
resps[idx]=$(cat<<EOF
EOF
)
sleeps[idx]="0.1" 
idx=$(($idx+1))

descs[idx]=" >> test Dbivar DemoChargeCurrent"
acts[idx]=$(cat<<EOF
${FimsDir}fims_send -m get -r /$$  -u /dbi/ess_controller/dbivars_controls_bms_DemoChargeCurrent 
EOF
)
resps[idx]=$(cat<<EOF
5555
EOF
)
sleeps[idx]="0.1" 
idx=$(($idx+1))

#
# the 0 value should be replaced by the dbi value 5555
#
descs[idx]=" >> setup Demo Current + Voltage "
acts[idx]=$(cat<<EOF
${FimsDir}fims_send -m set  -u /ess/full/dbi/controls/bms '
{
    "DemoChargeCurrent":
    {
        "value": 0,
        "dbiStatus":"init",
        "debug":1,
        "actions": 
        {
            "onSet": [{"func": [{"amap": "ess","func": "CheckDbiVar"}]}]
        }
    },
    "DemoDischargeCurrent":
    {
        "value": 0,
        "dbiStatus":"init",
        "debug":1,
        "actions": 
        {
            "onSet": [{"func": [{"amap": "ess","func": "CheckDbiVar"}]}]
        }
    },
    "DemoVoltage":
    {
        "value": 0,
        "dbiStatus":"init",
        "debug":1,
        "actions": 
        {
            "onSet": [{"func": [{"amap": "ess","func": "CheckDbiVar"}]}]
        }
    }
}'
EOF
)
resps[idx]=$(cat<<EOF
EOF
)
#allow time for dbi to respond the init status should trigger a dbi read.


descs[idx]=" >> trigger Demo Current "
acts[idx]=$(cat<<EOF
${FimsDir}fims_send -m set  -u /ess/full/controls/bms '
{
    "DemoChargeCurrent":
    {
        "value": 234
    }
}'
EOF
)
resps[idx]=$(cat<<EOF
EOF
)
sleeps[idx]="1" 
idx=$(($idx+1))

descs[idx]=" >> recover Demo Current "
acts[idx]=$(cat<<EOF
${FimsDir}fims_send -m get -r /$$  -u /ess/full/controls/bms/DemoChargeCurrent | jq | grep -v tLast
EOF
)
resps[idx]=$(cat<<EOF
{
  "DemoChargeCurrent": {
    "value": 234
  }
}
EOF
)
sleeps[idx]="0.1" 
idx=$(($idx+1))

```