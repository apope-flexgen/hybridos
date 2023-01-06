#!/bin/sh

#
# file 300.1_test_actions.sh
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


#$1 desc 
#$2 act
#$3 resp
#$4 outfile

#run_test $test1_desc $test1_act $test1_resp $outfile
num_passed=0
num_failed=0
fails=()

tfile=./scripts/release/test_definitions.sh
. $tfile

rtest="300.1_test_bitset"
# load in the test setup defs...
deffile=./scripts/release/${rtest}.txt
. $deffile

mkdir -p /var/log/tests/output
outfile=/var/log/tests/output/${rtest}_results.txt
logfile=/var/log/tests/output/${rtest}_log.txt


rm -f $outfile
test_header $outfile

rm -f $logfile
test_header $logfile

fidx=1

run_tests $fidx $idx

ix=$(($idx-1))
test_tail $outfile

echo
echo

cat $outfile

exit




