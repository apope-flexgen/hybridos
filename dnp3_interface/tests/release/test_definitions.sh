# test definition file 
# p. Wilshire 10/22/2021
#             10/23/2021
#             10/27/2021
#$1 shift 
#$2 shiftnum
#$3 add num

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
    echo ">>>>>> $rtest : tests resuts  >>>"   > $1
    echo " $rtest: ${descs[0]}"               >> $1
    echo -n "  Test Date: "                   >> $1
    date                                      >> $1
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