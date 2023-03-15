#!/bin/sh
#
# file 601.1_run_tests.sh
#
# iniitally all the tests will divert their output to a file which is used to set up the 
# test responses.
#
# this will update dbi every time the value is written with a change
# dbiStatus == "init" to also force a dbiWrite
# once that has been processes that value will change to "OK"
# EnableDbiUpdate must also be set to true
# if dbiStatus == "once"
#   EnableDbiUpdate will be set to false after a single update.
if [ -f /home/config/sim/setup.txt ] ; then
    . /home/config/sim/setup.txt
fi

if [ -f  ~/config_ess/install/configs/sim/setup.txt ] ; then 
    . ~/config_ess/install/configs/sim/setup.txt
    echo "using ~/config_ess/install/configs/sim/setup.txt"
fi
#

#run_test $test1_desc $test1_act $test1_resp $outfile
function run_test()
{
    echo " >>>>>> $1"
    #echo " >>>>>> $2"
    act_resp=$(eval $2)
    #echo " act_resp >> $act_resp"   
    #echo " exp_resp >> $3"   

    # this needs to be a python program
    if [ "$act_resp" = "$3" ] ; then
        echo  " test [passed]  >> $1" >> $4
    else
        echo " test [failed] >> $1 "  >> $4
        echo " act_resp >> [$act_resp]"  >> $4 
        echo " exp_resp >> [$3]"         >> $4
    fi
}


# load in the test setup defs...
deffile=./scripts/release/601.1_run_tests.txt
. $deffile

mkdir -p /var/log/tests/output
outfile=/var/log/tests/output/601.1_run_tests_results.txt

rm -f $outfile
echo -n " >>>>>>tests resuts  >>>" > $outfile
echo " ${descs[0]}" >> $outfile
date                             >> $outfile
ip a | grep -A 4 eth0 | grep "inet " | cut -d '/' -f1 >> $outfile
echo "#########################" >> $outfile
echo "                         " >> $outfile

if [ -f build/release/ess_controller ] ; then
    build/release/ess_controller -v >>$outfile
    echo "#########################" >> $outfile
    echo "                         " >> $outfile
fi

fidx=1
echo " idx $idx  ${descs[0]}"

for ((foo=$fidx; foo<$idx;++foo)); do
  run_test "${descs[$foo]}" "${acts[$foo]}" "${resps[$foo]}" "$outfile"
done


echo
echo

cat $outfile
exit





