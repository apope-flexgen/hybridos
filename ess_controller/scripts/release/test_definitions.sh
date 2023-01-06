# test definition file 
# p. Wilshire 10/22/2021
#             10/23/2021
#             10/27/2021
#              3/18/2022
#$1 shift 
#$2 shiftnum
#$3 add num
# default value can be overridden by the test 
TargetIp="172.17.0.3"
TestedBy="Phil Wilshire"
#sendGetSSH /components/pcs_0_parameter_setting  off_grid_voltage_setpoint 456  " check pcs_0    off_grid_voltage " 

sendGetSSH ()
{
    setSSHListen  $1   $2      0.1  3  " listen to    $1 $2      " 
    setLocalVal   $1   $2  $3       3  " set local value $2 -> $3"    $3
    getSSHListen  $1   $2      0.1     " get ssh listen for $2   "    $3    "Body:    {\"value\":$3}"
}

#setSSHListen /components/pcs_0_parameter_setting  off_grid_voltage_setpoint 10  0.1 0.1 /assets/pcs/pcs_1/voltage_command     456    456
#             $1                                   $2                        $3  $4
#setSSHListen /components/pcs_0_parameter_setting  off_grid_voltage_setpoint 10  0.1  "listen to     off_grd_voltage " 

setSSHListen()
{
  descs[idx]=$5

  acts[idx]=$(cat<<EOF
ssh root@${TargetIp} "timeout $4 fims_listen -u $1  > /tmp/listen_$2 2>&1&"
EOF
)
resps[idx]=$(cat<<EOF
EOF
)
  sleeps[idx]="$3" 
  idx=$(($idx+1))
}   
#getSSHListen /components/pcs_0_parameter_setting  off_grid_voltage_setpoint 456 /assets/pcs/pcs_1/voltage_command     456    456

getSSHListen()
{
  descs[idx]=$4

  acts[idx]=$(cat<<EOF
ssh root@${TargetIp} "cat  /tmp/listen_$2" | grep -A 3 $2 | grep value
EOF
)
resps[idx]=$(cat<<EOF
$6
EOF
)
  sleeps[idx]="$3" 
  idx=$(($idx+1))
}   

setLocalVal()
{
  descs[idx]=$5

  acts[idx]=$(cat<<EOF
fims_send -m set  -u $1 '{"$2":{"value":$3}}'
EOF
)
resps[idx]=$(cat<<EOF
EOF
)
  sleeps[idx]="$4" 
  idx=$(($idx+1))
}   

testFault ()
{
    setSSHVal    $1                      $2     $5           1.2      " >>>>>>>>>>>  fault $4 value  $5 expect $6"
    getAsset  /ess$4                            "$6"          0.1     "        >> get  $4   ->     $6"
}

testFaultVal ()
{
    setSSHVal    $1                      $2     $5           1.2      " >>>>>>>>>>>  fault $4 value  $5 expect $6"
    getAssetVal  /ess$4                            $6          0.1     "        >> get  $4   ->     $6"
}

getAvar ()
{
  sys_xxx=`fims_send -m get -r /$$ -u $2`
  echo "$sys_xxx"
}


# getAssetVal  /assets/ess/summary/version   newversion   
function getAssetVal()
{
  descs[idx]=$4

  acts[idx]=$(cat<<EOF
  ${FimsDir}fims_send -m get -r /$$ -u $1 
    | jq 
EOF
)
  resps[idx]=$(cat<<EOF
$2
EOF
)
  sleeps[idx]="$3" 
  idx=$(($idx+1))
}


# getAsset  /assets/ess/summary/version   newversion   0.1 "check an asset value"
function getAsset()
{
  descs[idx]=$4

  acts[idx]=$(cat<<EOF
  ${FimsDir}fims_send -m get -r /$$ -u $1 
    | jq 
EOF
)
  resps[idx]=$(cat<<EOF
"$2"
EOF
)
  sleeps[idx]="$3" 
  idx=$(($idx+1))
}

# setAsset  /status/ess/build   newversion  0.1 "Set an asset (bool or numerical) value"
function setAssetVal()
{
  descs[idx]=$4

  acts[idx]=$(cat<<EOF
  ${FimsDir}fims_send -m set -r /$$ -u $1 '{
      "value": $2
}' 
    | jq | grep $2
EOF
)
  resps[idx]=$(cat<<EOF
    "value": $2
EOF
)
  sleeps[idx]="$3" 
  idx=$(($idx+1))
}


# setAssetValNR  /assets/ess/summary/status   21   0.1  "Set an asset value with no reply"   
function setAssetValNR()
{
  descs[idx]=$4

  acts[idx]=$(cat<<EOF
  ${FimsDir}fims_send -m set  -u $1 '{
      "value": $2
}' 
EOF
)
  resps[idx]=$(cat<<EOF
EOF
)
  sleeps[idx]="$3" 
  idx=$(($idx+1))
}

# setAsset  /status/ess/build   newversion  0.1 "Set an asset (test) value"
function setAsset()
{
  descs[idx]=$4

  acts[idx]=$(cat<<EOF
  ${FimsDir}fims_send -m set -r /$$ -u $1 '{
      "value": "$2"
}' 
    | jq | grep $2
EOF
)
  resps[idx]=$(cat<<EOF
    "value": "$2"
EOF
)
  sleeps[idx]="$3" 
  idx=$(($idx+1))
}

setSSHVal()
{
  descs[idx]=$5

  acts[idx]=$(cat<<EOF
ssh root@${TargetIp} "fims_send -m pub  -u $1 '{\"$2\":$3}'"
EOF
)
resps[idx]=$(cat<<EOF
EOF
)
  sleeps[idx]="$4" 
  idx=$(($idx+1))
}   

#xxx=`addMask 4  1  1`
function addMask()
{
    xfoo=`echo "$2 * 2^$1" | bc`
    xfum=`echo "$xfoo + $3" | bc`
    echo "$xfum"
}

#for i in {START..END}
function run_tests()
{
    if [ "$logfile" = "" ] ; then
      logfile=/dev/null
    fi 
    for ((foo=$1; foo<$2;++foo)); do
    #for foo in {$1..$2}; do
     run_test "${descs[$foo]}" "${acts[$foo]}" "${resps[$foo]}" "$outfile" "$logfile"
     sleep ${sleeps[$foo]} 
    done
# ix=$(($idx-1))
}

#$1 desc
#$2 act
#$3 resp
#$4 outfile
#$5 logfile
act_res=()

function run_test()
{
    echo " >>>>>> test${foo}  $1"
    #echo " >>>>>> $2"
    act_resp=$(eval $2)

    echo " #####################"     >> $5
    echo " test${foo} [$1]"           >> $5
    echo " #####################"     >> $5
    echo " request ["                 >> $5
    echo "$2"                         >> $5
    echo " ]"                         >> $5
    echo " #######"                   >> $5
    echo " act_resp ["                >> $5
    echo "$act_resp"                  >> $5
    echo " ]"                         >> $5
    echo " #######"                   >> $5
    echo " exp_resp ["                >> $5
    echo "$3"                         >> $5
    echo " ]"                         >> $5
    echo " #######"                   >> $5
 
    #echo " act_resp >> $act_resp"   
    #echo " exp_resp >> $3"  
    act_res[${foo}]=$act_resp 
    if [ "$act_resp" = "$3" ] ; then
        echo  " test${foo} [passed]  >> $1" >> $4
        num_passed=$(($num_passed+1))
    else
        echo " test${foo} [failed] >> $1 "  >> $4
        echo " act_resp >> [$act_resp]"  >> $4 
        echo " exp_resp >> [$3]"         >> $4
        num_failed=$(($num_failed+1))
        fails+=($foo)
    fi

    #echo -n " >>>>>>>>>" >> $outfile
    #check_resp "$test1_desc" "$act_resp" "$test1_resp" >> $outfile
      #echo " >>>>>> $1"
      #act_resp=$(eval $2)    
      #echo -n " >>>>>>>>>" >> $4
      #check_resp "$1" "$act_resp" "$3" >> $4
}

function test_header()
{
    rm -f $1
    echo "#################################################################"   > $1
    echo ">>>>>> $rtest : tests resuts  >>>"  >> $1
    echo "#################################################################"  >> $1
    echo "                                 "  >> $1 
    echo "   $rtest: ${descs[0]}"             >> $1
    echo "                         "          >> $1
    echo -n "  Test Date: "                   >> $1
    date                                      >> $1
    echo -n "  Tested By: "                   >> $1
    echo " $TestedBy "                         >> $1

    echo -n "  Test System Ip: "              >> $1
    ip a | grep -A 4 eth0 | grep "inet " | cut -d '/' -f1 | cut -d ' ' -f6 >> $1
    echo -n "  Test System Os: "              >> $1
    uname -a                                  >> $1
    echo "#################################"  >> $1
    echo "                         "          >> $1

    if [ -f build/release/ess_controller ] ; then
        build/release/ess_controller -v  >> $1
        echo "                         " >> $1
        echo "#########################" >> $1
        echo "                         " >> $1
    fi


    #last=34
    #first=2
    #fidx=1
    #echo " idx $idx  descs[1] = ${descs[1]}"
}

function test_tail()
{
    echo "#########################"                              >> $1
    echo "######## $rtest: passed $num_passed out of $ix "        >> $1
    if [ $num_passed = $ix ] ; then
      echo "#########################"                            >> $1
      echo "######## $rtest: all tests passed "                   >> $1
    else
      echo "#########################"                            >> $1
      echo "######## $rtest: failed $num_failed out of $ix "      >> $1
      echo "#########################" >> $1
      echo "########                 ${fails[@]} "                >> $1
      echo "######## $rtest: failed tests"                        >> $1
      for ff  in ${fails[@]} ; do
        echo " test$ff -->  ${descs[$ff]}"                        >> $1
        done
    fi
      echo "####################################################" >> $1
}
#setVar DemoChargeCurrent 4321 0.1 " >> set the controlled var to 4321"
function setVar()
{
  descs[idx]=$4

  acts[idx]=$(cat<<EOF
  ${FimsDir}fims_send -m set -r /$$ -u /ess/full/controls/bms '{"$1": $2}' 
    | jq | grep $1
EOF
)
  resps[idx]=$(cat<<EOF
  "$1": $2
EOF
)
  sleeps[idx]="$3" 
  idx=$(($idx+1))
}
function nsetVar()
{
  descs[idx]=$4

  acts[idx]=$(cat<<EOF
  ${FimsDir}fims_send -m set -r /$$ -u /ess/full/controls/bms/$1 $2 
    | jq | grep $1
EOF
)
  resps[idx]=$(cat<<EOF
  "$1": $2
EOF
)
  sleeps[idx]="$3" 
  idx=$(($idx+1))
}

#setVarTxt DemoChargeCurrent init 0.1 " >> set the controlled var to 4321"

function setVarTxt()
{
  descs[idx]=$4

  acts[idx]=$(cat<<EOF
  ${FimsDir}fims_send -m set -r /$$ -u /ess/full/controls/bms "{\"$1\":\"$2\"}" 
    | jq | grep $1
EOF
)
  resps[idx]=$(cat<<EOF
  "$1": "$2"
EOF
)
  sleeps[idx]="$3" 
  idx=$(($idx+1))
}
#getVar DemoChargeCurrent 4321 0.1 " >> check the controlled value 4321"

function getVar()
{
  descs[idx]=$4

  acts[idx]=$(cat<<EOF
  ${FimsDir}fims_send -m get -r /$$ -u /ess/controls/bms/$1 
    | jq 
EOF
)
  resps[idx]=$(cat<<EOF
$2
EOF
)
  sleeps[idx]="$3" 
  idx=$(($idx+1))
}

#getVar DemoChargeCurrent 4321 0.1 " >> check the controlled value 4321"

function getParam()
{
  descs[idx]=$5

  acts[idx]=$(cat<<EOF
  ${FimsDir}fims_send -m get -r /$$ -u /ess/full/controls/bms/$1 
    | jq | grep $2
EOF
)
  resps[idx]=$(cat<<EOF
    "$2": $3
EOF
)
  sleeps[idx]="$4" 
  idx=$(($idx+1))
}